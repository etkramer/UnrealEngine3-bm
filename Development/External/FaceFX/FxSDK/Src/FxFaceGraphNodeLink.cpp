//------------------------------------------------------------------------------
// This class defines a link between two nodes in an FxFaceGraph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxFaceGraphNodeLink.h"
#include "FxUtil.h"
#include "FxLinkFn.h"
#include "FxFaceGraphNode.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxFaceGraphNodeLinkVersion 0

FX_IMPLEMENT_CLASS(FxFaceGraphNodeLink, kCurrentFxFaceGraphNodeLinkVersion, FxObject)

FxFaceGraphNodeLink::FxFaceGraphNodeLink()
	: FxDataContainer()
	, _inputName(FxName::NullName)
	, _input(NULL)
	, _linkFnName(FxName::NullName)
	, _linkFnType(LFT_Invalid)
{
}

FxFaceGraphNodeLink::FxFaceGraphNodeLink( const FxName& inputName, 
	FxFaceGraphNode* inputNode, const FxName& linkFunctionName, 
	FxLinkFnType linkFunctionType, const FxLinkFnParameters& linkFunctionParams )
	: FxDataContainer()
	, _inputName(inputName)
	, _input(inputNode)
	, _linkFnName(linkFunctionName)
	, _linkFnType(linkFunctionType)
	, _linkFnParams(linkFunctionParams)
{
}

FxFaceGraphNodeLink::FxFaceGraphNodeLink( const FxFaceGraphNodeLink& other )
	: Super(other)
	, FxDataContainer(other)
	, _inputName(other._inputName)
	, _input(other._input)
	, _linkFnName(other._linkFnName)
	, _linkFnType(other._linkFnType)
	, _linkFnParams(other._linkFnParams)
{
}

FxFaceGraphNodeLink& FxFaceGraphNodeLink::operator=( const FxFaceGraphNodeLink& other )
{	
	if( this == &other ) return *this;
	Super::operator=(other);
	FxDataContainer::operator=(other);
	_inputName    = other._inputName;
	_input        = other._input;
	_linkFnName   = other._linkFnName;
	_linkFnType   = other._linkFnType;
	_linkFnParams = other._linkFnParams;
	return *this;
}

FxFaceGraphNodeLink::~FxFaceGraphNodeLink()
{
}

void FxFaceGraphNodeLink::SetNode( FxFaceGraphNode* inputNode )
{
	FxAssert(inputNode);
	if( inputNode )
	{
		_input     = inputNode;
		_inputName = _input->GetName();
	}
	else
	{
		_inputName = FxName::NullName;
	}
}

void FxFaceGraphNodeLink::SetLinkFnName( const FxName& linkFn )
{
	_linkFnName = linkFn;
	_linkFnType = FxLinkFn::FindLinkFunctionType(_linkFnName);
}

void FxFaceGraphNodeLink::SetLinkFn( const FxLinkFn* linkFn )
{
	FxAssert(linkFn);
	if( linkFn )
	{
		_linkFnType = linkFn->GetType();
		_linkFnName = linkFn->GetName();
	}
	else
	{
		_linkFnType = LFT_Invalid;
		_linkFnName = FxName::NullName;
	}
}

void FxFaceGraphNodeLink::SetLinkFnParams( const FxLinkFnParameters& linkFnParams )
{
	_linkFnParams = linkFnParams;
}

FxReal FxFaceGraphNodeLink::GetCorrectionFactor( void ) const
{
	if( _linkFnParams.parameters.Length() )
	{
		return _linkFnParams.parameters[0];
	}
	// This is FxCorrectiveLinkFn's default parameter 0 value.
	return 1.0f;
}

void FxFaceGraphNodeLink::SetCorrectionFactor( FxReal newValue )
{
	if( _linkFnParams.parameters.Length() < 1 )
	{
		_linkFnParams.parameters.PushBack(newValue);
	}
	else
	{
		_linkFnParams.parameters[0] = newValue;
	}
}

void FxFaceGraphNodeLink::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxFaceGraphNodeLink");

	arc << _inputName << _linkFnName << _linkFnParams;
	_linkFnType = FxLinkFn::FindLinkFunctionType(_linkFnName);
	FxAssert(LFT_Invalid != _linkFnType);
	if( LFT_Null == _linkFnType )
	{
		_linkFnParams.parameters.Clear();
	}
}

} // namespace Face

} // namespace OC3Ent
