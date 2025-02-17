//------------------------------------------------------------------------------
// Represents the transformations which may take place along links.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxLinkFn.h"
// This is here only to eat the pCustomCurve when serializing old versions of
// FxLinkFnParameters.
#include "FxAnimCurve.h"

namespace OC3Ent
{

namespace Face
{

FxLinkFnParameterDesc::FxLinkFnParameterDesc()
	: parameterDefaultValue(0.0f)
{
}

FxLinkFnParameterDesc::
FxLinkFnParameterDesc( const FxChar* name, FxReal defaultValue )
	: parameterName(name)
	, parameterDefaultValue(defaultValue)
{
}

#define kCurrentFxLinkFnParametersVersion 1

FxArchive& operator<<( FxArchive& arc, FxLinkFnParameters& linkFnParams )
{
	FxUInt16 version = arc.SerializeClassVersion("FxLinkFnParameters", FxTrue, kCurrentFxLinkFnParametersVersion);
	arc << linkFnParams.parameters;
	if( arc.IsLoading() && version < 1 )
	{
		// Eat the old pCustomCurve.
		FxAnimCurve* pCustomCurve = NULL;
		arc << pCustomCurve;
		if( pCustomCurve )
		{
			delete pCustomCurve;
			pCustomCurve = NULL;
		}
	}
	return arc;
}

#define kCurrentFxLinkFnVersion 0

FX_IMPLEMENT_CLASS(FxLinkFn, kCurrentFxLinkFnVersion, FxNamedObject)

FxArray<FxLinkFn*>* FxLinkFn::_linkFnTable = NULL;

FxLinkFn::FxLinkFn()
{
	_isCorrective = FxFalse;
}

FxLinkFn::~FxLinkFn()
{
}

// Update this when adding new link function types.
#define FX_NUM_LINK_FUNCTIONS 12
void FX_CALL FxLinkFn::Startup( void )
{
	if( !_linkFnTable )
	{
		FxLinkFn::StaticClass();

		_linkFnTable = static_cast<FxArray<FxLinkFn*>*>(FxAlloc(sizeof(FxArray<FxLinkFn*>), "LinkFnList"));
		FxDefaultConstruct(_linkFnTable);
		_linkFnTable->Reserve(FX_NUM_LINK_FUNCTIONS);

//------------------------------------------------------------------------------
// Initialize all link functions here.
//------------------------------------------------------------------------------
		FxNullLinkFn::StaticClass();

		FxLinearLinkFn::StaticClass();
		FxLinearLinkFn* linearLinkFn = new FxLinearLinkFn();
		FxLinkFn::AddLinkFunction(linearLinkFn);

		FxQuadraticLinkFn::StaticClass();
		FxQuadraticLinkFn* quadraticLinkFn = new FxQuadraticLinkFn();
		FxLinkFn::AddLinkFunction(quadraticLinkFn);

		FxCubicLinkFn::StaticClass();
		FxCubicLinkFn* cubicLinkFn = new FxCubicLinkFn();
		FxLinkFn::AddLinkFunction(cubicLinkFn);

		FxSqrtLinkFn::StaticClass();
		FxSqrtLinkFn* sqrtLinkFn = new FxSqrtLinkFn();
		FxLinkFn::AddLinkFunction(sqrtLinkFn);

		FxNegateLinkFn::StaticClass();
		FxNegateLinkFn* negateLinkFn = new FxNegateLinkFn();
		FxLinkFn::AddLinkFunction(negateLinkFn);

		FxInverseLinkFn::StaticClass();
		FxInverseLinkFn* inverseLinkFn = new FxInverseLinkFn();
		FxLinkFn::AddLinkFunction(inverseLinkFn);

		FxOneClampLinkFn::StaticClass();
		FxOneClampLinkFn* oneClampLinkFn = new FxOneClampLinkFn();
		FxLinkFn::AddLinkFunction(oneClampLinkFn);

		FxConstantLinkFn::StaticClass();
		FxConstantLinkFn* constantLinkFn = new FxConstantLinkFn();
		FxLinkFn::AddLinkFunction(constantLinkFn);

		FxCorrectiveLinkFn::StaticClass();
		FxCorrectiveLinkFn* correctiveLinkFn = new FxCorrectiveLinkFn();
		FxLinkFn::AddLinkFunction(correctiveLinkFn);

		// Deprecated the Custom link function.  In FaceFX 1.7+, all Custom link
		// functions are re-routed through the "null" link function.
		FxNullLinkFn* deprecatedCustomLinkFn = new FxNullLinkFn("custom");
		FxLinkFn::AddLinkFunction(deprecatedCustomLinkFn);

		// Deprecated the Perlin Noise link function.  In FaceFX 1.5+, all
		// Perlin Noise link functions are re-routed through the "null" link
		// function.
		FxNullLinkFn* deprecatedPerlinNoiseLinkFn = new FxNullLinkFn("perlin noise");
		FxLinkFn::AddLinkFunction(deprecatedPerlinNoiseLinkFn);

		FxClampedLinearLinkFn::StaticClass();
		FxClampedLinearLinkFn* clampedLinearLinkFn = new FxClampedLinearLinkFn();
		FxLinkFn::AddLinkFunction(clampedLinearLinkFn);
	}
}

void FX_CALL FxLinkFn::Shutdown( void )
{
	if( _linkFnTable )
	{
		// Delete all link functions in the link function table.
		FxSize numLinkFns = _linkFnTable->Length();
		for( FxSize i = 0; i < numLinkFns; ++i )
		{
			delete (*_linkFnTable)[i];
		}
		_linkFnTable->Clear();
		FxDelete(_linkFnTable);
        _linkFnTable = NULL;
	}
}

FxSize FX_CALL FxLinkFn::GetNumLinkFunctions( void )
{
	if( _linkFnTable )
	{
		return _linkFnTable->Length();
	}
	return 0;
}

const FxLinkFn* FX_CALL FxLinkFn::GetLinkFunction( FxSize index )
{
	if( _linkFnTable )
	{
		return (*_linkFnTable)[index];
	}
	return NULL;
}

const FxLinkFn* FX_CALL FxLinkFn::FindLinkFunction( const FxName& name )
{
	if( _linkFnTable )
	{
		FxSize numLinkFns = _linkFnTable->Length();
		for( FxSize i = 0; i < numLinkFns; ++i )
		{
			if( (*_linkFnTable)[i]->GetName() == name )
			{
				return (*_linkFnTable)[i];
			}
		}
	}
	return NULL;
}

const FxLinkFn* FX_CALL FxLinkFn::FindLinkFunctionByType( FxLinkFnType lfType )
{
	if( _linkFnTable )
	{
		FxSize numLinkFns = _linkFnTable->Length();
		for( FxSize i = 0; i < numLinkFns; ++i )
		{
			if( (*_linkFnTable)[i]->GetType() == lfType )
			{
				return (*_linkFnTable)[i];
			}
		}
	}
	return NULL;
}

FxLinkFnType FX_CALL FxLinkFn::FindLinkFunctionType( const FxName& name )
{
	const FxLinkFn* pLinkFn = FindLinkFunction(name);
	if( pLinkFn )
	{
		return pLinkFn->GetType();
	}
	return LFT_Invalid;
}

void FX_CALL FxLinkFn::AddLinkFunction( FxLinkFn* pLinkFn )
{
	if( pLinkFn )
	{
		if( !_linkFnTable )
		{
			FxAssert(!"Attempt to add FxLinkFn before FxLinkFn::Startup() called!");
		}
		const FxLinkFn* duplicateFn = FindLinkFunction(pLinkFn->GetName());
		if( duplicateFn != NULL )
		{
			FxAssert(!"Attempt to add duplicate FxLinkFn!");
		}
		_linkFnTable->PushBack(pLinkFn);
	}
}

//------------------------------------------------------------------------------
// Null.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxNullLinkFn, kCurrentFxLinkFnVersion, FxLinkFn)

FxNullLinkFn::FxNullLinkFn( const FxChar* name )
	: _fnDesc(" This link function type has been deprecated. ")
{
	SetName(name);
}

FxNullLinkFn::~FxNullLinkFn()
{
}

//------------------------------------------------------------------------------
// Linear.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxLinearLinkFn, kCurrentFxLinkFnVersion, FxLinkFn)

FxLinearLinkFn::FxLinearLinkFn()
	: _fnDesc(" y = m * x + b ")
{
	SetName("linear");
	_parameters.Reserve(2);
	_parameters.PushBack(FxLinkFnParameterDesc("m", 1.0f));
	_parameters.PushBack(FxLinkFnParameterDesc("b", 0.0f));
}

FxLinearLinkFn::~FxLinearLinkFn()
{
}

//------------------------------------------------------------------------------
// Quadratic.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxQuadraticLinkFn, kCurrentFxLinkFnVersion, FxLinkFn)

FxQuadraticLinkFn::FxQuadraticLinkFn()
	: _fnDesc(" y = a * x ^2 ")
{
	SetName("quadratic");
	_parameters.PushBack(FxLinkFnParameterDesc("a", 1.0f));
}

FxQuadraticLinkFn::~FxQuadraticLinkFn()
{
}

//------------------------------------------------------------------------------
// Cubic.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxCubicLinkFn, kCurrentFxLinkFnVersion, FxLinkFn)

FxCubicLinkFn::FxCubicLinkFn()
	: _fnDesc(" y = a * x ^3 ")
{
	SetName("cubic");
	_parameters.PushBack(FxLinkFnParameterDesc("a", 1.0f));
}

FxCubicLinkFn::~FxCubicLinkFn()
{
}

//------------------------------------------------------------------------------
// Square Root.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxSqrtLinkFn, kCurrentFxLinkFnVersion, FxLinkFn)

FxSqrtLinkFn::FxSqrtLinkFn()
	: _fnDesc(" y = a * sqrt( x ) ")
{
	SetName("square root");
	_parameters.PushBack(FxLinkFnParameterDesc("a", 1.0f));
}

FxSqrtLinkFn::~FxSqrtLinkFn()
{
}

//------------------------------------------------------------------------------
// Negate.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxNegateLinkFn, kCurrentFxLinkFnVersion, FxLinkFn)

FxNegateLinkFn::FxNegateLinkFn()
	: _fnDesc(" y = - x ")
{
	SetName("negate");
}

FxNegateLinkFn::~FxNegateLinkFn()
{
}

//------------------------------------------------------------------------------
// Inverse.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxInverseLinkFn, kCurrentFxLinkFnVersion, FxLinkFn)

FxInverseLinkFn::FxInverseLinkFn()
	: _fnDesc(" y = 1 / x ")
{
	SetName("inverse");
}

FxInverseLinkFn::~FxInverseLinkFn()
{
}

//------------------------------------------------------------------------------
// One Clamp.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxOneClampLinkFn, kCurrentFxLinkFnVersion, FxLinkFn)

FxOneClampLinkFn::FxOneClampLinkFn()
	: _fnDesc(" y = [1 if x <= 1] or [1 / x if x > 1) ")
{
	SetName("oneclamp");
}

FxOneClampLinkFn::~FxOneClampLinkFn()
{
}

//------------------------------------------------------------------------------
// Constant.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxConstantLinkFn, kCurrentFxLinkFnVersion, FxLinkFn)

FxConstantLinkFn::FxConstantLinkFn()
	: _fnDesc(" y = c ")
{
	SetName("constant");
	_parameters.PushBack(FxLinkFnParameterDesc("c", 1.0f));
}

FxConstantLinkFn::~FxConstantLinkFn()
{
}

//------------------------------------------------------------------------------
// Corrective.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxCorrectiveLinkFn, kCurrentFxLinkFnVersion, FxLinkFn)

FxCorrectiveLinkFn::FxCorrectiveLinkFn()
	: _fnDesc(" x corrects for y ")	
{
	SetName("corrective");
	_parameters.PushBack(FxLinkFnParameterDesc("Correction Factor", 1.0f));
	_isCorrective = FxTrue;
}

FxCorrectiveLinkFn::~FxCorrectiveLinkFn()
{
}

//------------------------------------------------------------------------------
// Clamped Linear.
//------------------------------------------------------------------------------
FX_IMPLEMENT_CLASS(FxClampedLinearLinkFn, kCurrentFxLinkFnVersion, FxLinkFn)

FxClampedLinearLinkFn::FxClampedLinearLinkFn()
	: _fnDesc(" y = m * x ")
{
	SetName("clamped linear");
	_parameters.Reserve(4);
	_parameters.PushBack(FxLinkFnParameterDesc("m", 1.0f));
	_parameters.PushBack(FxLinkFnParameterDesc("clampx", 0.0f));
	_parameters.PushBack(FxLinkFnParameterDesc("clampy", 0.0f));
	_parameters.PushBack(FxLinkFnParameterDesc("clampdir", 1.0f));
}

FxClampedLinearLinkFn::~FxClampedLinearLinkFn()
{
}

//------------------------------------------------------------------------------
// Add new link function types here.
//------------------------------------------------------------------------------

} // namespace Face

} // namespace OC3Ent
