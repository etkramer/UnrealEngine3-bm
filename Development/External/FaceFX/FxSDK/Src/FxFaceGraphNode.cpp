//------------------------------------------------------------------------------
// This abstract class implements a node in an FxFaceGraph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxFaceGraphNode.h"
#include "FxMath.h"
#include "FxUtil.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxFaceGraphNodeVersion 2

FX_IMPLEMENT_CLASS(FxFaceGraphNode, kCurrentFxFaceGraphNodeVersion, FxNamedObject)

FxFaceGraphNode::FxFaceGraphNode()
	: FxDataContainer()
	, _hasBeenVisited(FxFalse)
	, _min(0.0f)
	, _max(1.0f)
	, _inputOperation(IO_Sum)
	, _isPlaceable(FxTrue)
{
}

FxFaceGraphNode::~FxFaceGraphNode()
{
}

void FxFaceGraphNode::CopyData( FxFaceGraphNode* pOther )
{
	if( pOther )
	{
		pOther->SetName(GetName());
		pOther->SetMin(GetMin());
		pOther->SetMax(GetMax());
		pOther->SetNodeOperation(GetNodeOperation());
		FxSize numUserProperties = GetNumUserProperties();
		for( FxSize i = 0; i < numUserProperties; ++i )
		{
			pOther->ReplaceUserProperty(GetUserProperty(i));
		}
	}
}

void FxFaceGraphNode::SetMin( FxReal iMin )
{
	_min = iMin;
}

void FxFaceGraphNode::SetMax( FxReal iMax )
{
	_max = iMax;
}

void FxFaceGraphNode::SetNodeOperation( FxInputOp iNodeOperation )
{
	_inputOperation = iNodeOperation;
}

FxBool FxFaceGraphNode::AddInputLink( const FxFaceGraphNodeLink& newInputLink )
{
	_inputs.PushBack(newInputLink);
	return FxTrue;
}

void FxFaceGraphNode::ModifyInputLink( FxSize index, 
									   const FxFaceGraphNodeLink& inputLink )
{
	FxAssert( index < _inputs.Length() );
	_inputs[index] = inputLink;
}

void FxFaceGraphNode::RemoveInputLink( FxSize index )
{
	_inputs.Remove(index);
}

void FxFaceGraphNode::AddOutput( FxFaceGraphNode* pOutput )
{
	_outputs.PushBack(pOutput);
}

FxBool FxFaceGraphNode::IsPlaceable( void ) const
{
	return _isPlaceable;
}

void FxFaceGraphNode::AddUserProperty( const FxFaceGraphNodeUserProperty& userProperty )
{
	if( FxInvalidIndex == FindUserProperty(userProperty.GetName()) )
	{
		_userProperties.PushBack(userProperty);
	}
}

FxSize FxFaceGraphNode::GetNumUserProperties( void ) const
{
	return _userProperties.Length();
}

FxSize FxFaceGraphNode::FindUserProperty( const FxName& userPropertyName ) const
{
	FxSize numUserProperties = _userProperties.Length();	
	for( FxSize i = 0; i < numUserProperties; ++i )
	{
		if( _userProperties[i].GetName() == userPropertyName )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

const FxFaceGraphNodeUserProperty& 
FxFaceGraphNode::GetUserProperty( FxSize index ) const
{
	return _userProperties[index];
}

void FxFaceGraphNode::
ReplaceUserProperty( const FxFaceGraphNodeUserProperty& userProperty )
{
	FxSize index = FindUserProperty(userProperty.GetName());
	if( FxInvalidIndex != index )
	{
		_userProperties[index] = userProperty;
	}
}

void FxFaceGraphNode::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	FxUInt16 version = arc.SerializeClassVersion("FxFaceGraphNode");

	FxInt32 tempNodeOp = static_cast<FxInt32>(_inputOperation);
	arc << _min << _max << tempNodeOp << _inputs;
	_inputOperation = static_cast<FxInputOp>(tempNodeOp);

	if( version >= 1 )
	{
		arc << _userProperties;
	}
	if( version >= 2 )
	{
		FxSize numOutputs = _outputs.Length();
		arc << numOutputs;
		if( arc.IsLoading() )
		{
			_outputs.Reserve(numOutputs);
		}
	}
}

FxBool FxFaceGraphNode::_hasCycles( void ) const
{
	if( _hasBeenVisited )
	{
		return FxTrue;
	}
	else
	{
		_hasBeenVisited = FxTrue;
		FxBool inputHasCycles = FxFalse;
		FxSize numInputs = _inputs.Length();
		for( FxSize i = 0; i < numInputs; ++i )
		{
			inputHasCycles |= _inputs[i].GetNode()->_hasCycles();
		}
		_hasBeenVisited = FxFalse;
		return inputHasCycles;
	}
}

} // namespace Face

} // namespace OC3Ent
