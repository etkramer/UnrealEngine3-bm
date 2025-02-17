/*=============================================================================
	UnDistributions.cpp: Implementation of distribution classes.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

IMPLEMENT_CLASS(UDistributionFloatConstant);
IMPLEMENT_CLASS(UDistributionFloatConstantCurve);
IMPLEMENT_CLASS(UDistributionFloatUniform);
IMPLEMENT_CLASS(UDistributionFloatUniformCurve);
IMPLEMENT_CLASS(UDistributionFloatParameterBase);

IMPLEMENT_CLASS(UDistributionVectorConstant);
IMPLEMENT_CLASS(UDistributionVectorConstantCurve);
IMPLEMENT_CLASS(UDistributionVectorUniform);
IMPLEMENT_CLASS(UDistributionVectorUniformCurve);
IMPLEMENT_CLASS(UDistributionVectorParameterBase);

/*-----------------------------------------------------------------------------
	UDistributionFloatConstant implementation.
-----------------------------------------------------------------------------*/

FLOAT UDistributionFloatConstant::GetValue( FLOAT F, UObject* Data )
{
	return Constant;
}


//////////////////////// FCurveEdInterface ////////////////////////

INT UDistributionFloatConstant::GetNumKeys()
{
	return 1;
}

INT UDistributionFloatConstant::GetNumSubCurves()
{
	return 1;
}

FLOAT UDistributionFloatConstant::GetKeyIn(INT KeyIndex)
{
	check( KeyIndex == 0 );
	return 0.f;
}

FLOAT UDistributionFloatConstant::GetKeyOut(INT SubIndex, INT KeyIndex)
{
	check( SubIndex == 0 );
	check( KeyIndex == 0 );
	return Constant;
}

void UDistributionFloatConstant::GetInRange(FLOAT& MinIn, FLOAT& MaxIn)
{
	MinIn = 0.f;
	MaxIn = 0.f;
}

void UDistributionFloatConstant::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	MinOut = Constant;
	MaxOut = Constant;
}

BYTE UDistributionFloatConstant::GetKeyInterpMode(INT KeyIndex)
{
	check( KeyIndex == 0 );
	return CIM_Constant;
}

void UDistributionFloatConstant::GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent)
{
	check( SubIndex == 0 );
	check( KeyIndex == 0 );
	ArriveTangent = 0.f;
	LeaveTangent = 0.f;
}

FLOAT UDistributionFloatConstant::EvalSub(INT SubIndex, FLOAT InVal)
{
	check(SubIndex == 0);
	return Constant;
}

INT UDistributionFloatConstant::CreateNewKey(FLOAT KeyIn)
{	
	return 0;
}

void UDistributionFloatConstant::DeleteKey(INT KeyIndex)
{
	check( KeyIndex == 0 );
}

INT UDistributionFloatConstant::SetKeyIn(INT KeyIndex, FLOAT NewInVal)
{
	check( KeyIndex == 0 );
	return 0;
}

void UDistributionFloatConstant::SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal) 
{
	check( SubIndex == 0 );
	check( KeyIndex == 0 );
	Constant = NewOutVal;

	bIsDirty = TRUE;
}

void UDistributionFloatConstant::SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode) 
{
	check( KeyIndex == 0 );
}

void UDistributionFloatConstant::SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent)
{
	check( SubIndex == 0 );
	check( KeyIndex == 0 );
}

/*-----------------------------------------------------------------------------
	UDistributionFloatConstantCurve implementation.
-----------------------------------------------------------------------------*/

FLOAT UDistributionFloatConstantCurve::GetValue( FLOAT F, UObject* Data )
{
	return ConstantCurve.Eval(F, 0.f);
}

//////////////////////// FCurveEdInterface ////////////////////////

INT UDistributionFloatConstantCurve::GetNumKeys()
{
	return ConstantCurve.Points.Num();
}

INT UDistributionFloatConstantCurve::GetNumSubCurves()
{
	return 1;
}

FLOAT UDistributionFloatConstantCurve::GetKeyIn(INT KeyIndex)
{
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	return ConstantCurve.Points(KeyIndex).InVal;
}

FLOAT UDistributionFloatConstantCurve::GetKeyOut(INT SubIndex, INT KeyIndex)
{
	check( SubIndex == 0 );
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	return ConstantCurve.Points(KeyIndex).OutVal;
}

void UDistributionFloatConstantCurve::GetInRange(FLOAT& MinIn, FLOAT& MaxIn)
{
	if(ConstantCurve.Points.Num() == 0)
	{
		MinIn = 0.f;
		MaxIn = 0.f;
	}
	else
	{
		FLOAT Min = BIG_NUMBER;
		FLOAT Max = -BIG_NUMBER;
		for (INT Index = 0; Index < ConstantCurve.Points.Num(); Index++)
		{
			FLOAT Value = ConstantCurve.Points(Index).InVal;
			if (Value < Min)
			{
				Min = Value;
			}
			if (Value > Max)
			{
				Max = Value;
			}
		}
		MinIn = Min;
		MaxIn = Max;
	}
}

void UDistributionFloatConstantCurve::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	ConstantCurve.CalcBounds(MinOut, MaxOut, 0.f);
}

BYTE UDistributionFloatConstantCurve::GetKeyInterpMode(INT KeyIndex)
{
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	return ConstantCurve.Points(KeyIndex).InterpMode;
}

void UDistributionFloatConstantCurve::GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent)
{
	check( SubIndex == 0 );
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	ArriveTangent = ConstantCurve.Points(KeyIndex).ArriveTangent;
	LeaveTangent = ConstantCurve.Points(KeyIndex).LeaveTangent;
}

FLOAT UDistributionFloatConstantCurve::EvalSub(INT SubIndex, FLOAT InVal)
{
	check(SubIndex == 0);
	return ConstantCurve.Eval(InVal, 0.f);
}

INT UDistributionFloatConstantCurve::CreateNewKey(FLOAT KeyIn)
{
	FLOAT NewKeyOut = ConstantCurve.Eval(KeyIn, 0.f);
	INT NewPointIndex = ConstantCurve.AddPoint(KeyIn, NewKeyOut);
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;

	return NewPointIndex;
}

void UDistributionFloatConstantCurve::DeleteKey(INT KeyIndex)
{
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	ConstantCurve.Points.Remove(KeyIndex);
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;
}

INT UDistributionFloatConstantCurve::SetKeyIn(INT KeyIndex, FLOAT NewInVal)
{
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	INT NewPointIndex = ConstantCurve.MovePoint(KeyIndex, NewInVal);
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;

	return NewPointIndex;
}

void UDistributionFloatConstantCurve::SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal) 
{
	check( SubIndex == 0 );
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	ConstantCurve.Points(KeyIndex).OutVal = NewOutVal;
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;
}

void UDistributionFloatConstantCurve::SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode) 
{
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	ConstantCurve.Points(KeyIndex).InterpMode = NewMode;
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;
}

void UDistributionFloatConstantCurve::SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent)
{
	check( SubIndex == 0 );
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	ConstantCurve.Points(KeyIndex).ArriveTangent = ArriveTangent;
	ConstantCurve.Points(KeyIndex).LeaveTangent = LeaveTangent;

	bIsDirty = TRUE;
}


/** Returns TRUE if this curve uses legacy tangent/interp algorithms and may be 'upgraded' */
UBOOL UDistributionFloatConstantCurve::UsingLegacyInterpMethod() const
{
	return ConstantCurve.UsingLegacyInterpMethod();
}


/** 'Upgrades' this curve to use the latest tangent/interp algorithms (usually, will 'bake' key tangents.) */
void UDistributionFloatConstantCurve::UpgradeInterpMethod()
{
	if( UsingLegacyInterpMethod() )
	{
		ConstantCurve.UpgradeInterpMethod();

		bIsDirty = TRUE;
	}
}


/*-----------------------------------------------------------------------------
	UDistributionFloatUniform implementation.
-----------------------------------------------------------------------------*/
void UDistributionFloatUniform::PostLoad()
{
	if (GetLinker() && (GetLinker()->Ver() < VER_UNIFORM_DISTRIBUTION_BAKING_UPDATE))
	{
		bIsDirty = TRUE;
		MarkPackageDirty();
	}

	Super::PostLoad();
}

FLOAT UDistributionFloatUniform::GetValue( FLOAT F, UObject* Data )
{
	return Max + (Min - Max) * appSRand();
}

#if !CONSOLE
/**
 * Return the operation used at runtime to calculate the final value
 */
ERawDistributionOperation UDistributionFloatUniform::GetOperation()
{
	if (Min == Max)
	{
		// This may as well be a constant - don't bother doing the appSRand scaling on it.
		return RDO_None;
	}
	return RDO_Random;
}
	
/**
 * Fill out an array of floats and return the number of elements in the entry
 *
 * @param Time The time to evaluate the distribution
 * @param Values An array of values to be filled out, guaranteed to be big enough for 4 floats
 * @return The number of elements (values) set in the array
 */
DWORD UDistributionFloatUniform::InitializeRawEntry(FLOAT Time, FLOAT* Values)
{
	Values[0] = Min;
	Values[1] = Max;
	return 2;
}
#endif

//////////////////////// FCurveEdInterface ////////////////////////

INT UDistributionFloatUniform::GetNumKeys()
{
	return 1;
}

INT UDistributionFloatUniform::GetNumSubCurves()
{
	return 2;
}

FLOAT UDistributionFloatUniform::GetKeyIn(INT KeyIndex)
{
	check( KeyIndex == 0 );
	return 0.f;
}

FLOAT UDistributionFloatUniform::GetKeyOut(INT SubIndex, INT KeyIndex)
{
	check( SubIndex == 0 || SubIndex == 1);
	check( KeyIndex == 0 );
	return (SubIndex == 0) ? Min : Max;
}

void UDistributionFloatUniform::GetInRange(FLOAT& MinIn, FLOAT& MaxIn)
{
	MinIn = 0.f;
	MaxIn = 0.f;
}

void UDistributionFloatUniform::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	MinOut = Min;
	MaxOut = Max;
}

BYTE UDistributionFloatUniform::GetKeyInterpMode(INT KeyIndex)
{
	check( KeyIndex == 0 );
	return CIM_Constant;
}

void UDistributionFloatUniform::GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent)
{
	check( SubIndex == 0 || SubIndex == 1);
	check( KeyIndex == 0 );
	ArriveTangent = 0.f;
	LeaveTangent = 0.f;
}

FLOAT UDistributionFloatUniform::EvalSub(INT SubIndex, FLOAT InVal)
{
	check( SubIndex == 0 || SubIndex == 1);
	return (SubIndex == 0) ? Min : Max;
}

INT UDistributionFloatUniform::CreateNewKey(FLOAT KeyIn)
{	
	return 0;
}

void UDistributionFloatUniform::DeleteKey(INT KeyIndex)
{
	check( KeyIndex == 0 );
}

INT UDistributionFloatUniform::SetKeyIn(INT KeyIndex, FLOAT NewInVal)
{
	check( KeyIndex == 0 );
	return 0;
}

void UDistributionFloatUniform::SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal) 
{
	check( SubIndex == 0 || SubIndex == 1);
	check( KeyIndex == 0 );

	// We ensure that we can't move the Min past the Max.
	if(SubIndex == 0)
	{	
		Min = ::Min<FLOAT>(NewOutVal, Max);
	}
	else
	{
		Max = ::Max<FLOAT>(NewOutVal, Min);
	}

	bIsDirty = TRUE;
}

void UDistributionFloatUniform::SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode) 
{
	check( KeyIndex == 0 );
}

void UDistributionFloatUniform::SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent)
{
	check( SubIndex == 0 || SubIndex == 1);
	check( KeyIndex == 0 );
}

/*-----------------------------------------------------------------------------
	UDistributionFloatUniformCurve implementation.
-----------------------------------------------------------------------------*/
void UDistributionFloatUniformCurve::PostLoad()
{
	if (GetLinker() && (GetLinker()->Ver() < VER_UNIFORM_DISTRIBUTION_BAKING_UPDATE))
	{
		bIsDirty = TRUE;
		MarkPackageDirty();
	}

	Super::PostLoad();
}

FLOAT UDistributionFloatUniformCurve::GetValue(FLOAT F, UObject* Data)
{
	FVector2D Val = ConstantCurve.Eval(F, FVector2D(0.f, 0.f));
	return Val.X + (Val.Y - Val.X) * appSRand();
}

#if !CONSOLE
/**
 * Return the operation used at runtime to calculate the final value
 */
ERawDistributionOperation UDistributionFloatUniformCurve::GetOperation()
{
	if (ConstantCurve.Points.Num() == 1)
	{
		// Only a single point - so see if Min == Max
		FInterpCurvePoint<FVector2D>& Value = ConstantCurve.Points(0);
		if (Value.OutVal.X == Value.OutVal.Y)
		{
			// This may as well be a constant - don't bother doing the appSRand scaling on it.
			return RDO_None;
		}
	}
	return RDO_Random;
}

/**
 * Fill out an array of floats and return the number of elements in the entry
 *
 * @param Time The time to evaluate the distribution
 * @param Values An array of values to be filled out, guaranteed to be big enough for 4 floats
 * @return The number of elements (values) set in the array
 */
DWORD UDistributionFloatUniformCurve::InitializeRawEntry(FLOAT Time, FLOAT* Values)
{
	FVector2D MinMax = GetMinMaxValue(Time, NULL);
	Values[0] = MinMax.X;
	Values[1] = MinMax.Y;
	return 2;
}
#endif

FVector2D UDistributionFloatUniformCurve::GetMinMaxValue(FLOAT F, UObject* Data)
{
	return ConstantCurve.Eval(F, FVector2D(0.f, 0.f));
}

//////////////////////// FCurveEdInterface ////////////////////////
INT UDistributionFloatUniformCurve::GetNumKeys()
{
	return ConstantCurve.Points.Num();
}

INT UDistributionFloatUniformCurve::GetNumSubCurves()
{
	return 2;
}

FLOAT UDistributionFloatUniformCurve::GetKeyIn(INT KeyIndex)
{
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));
	return ConstantCurve.Points(KeyIndex).InVal;
}

FLOAT UDistributionFloatUniformCurve::GetKeyOut(INT SubIndex, INT KeyIndex)
{
	check((SubIndex >= 0) && (SubIndex < 2));
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));
	
	if (SubIndex == 0)
	{
		return ConstantCurve.Points(KeyIndex).OutVal.X;
	}
	else
	{
		return ConstantCurve.Points(KeyIndex).OutVal.Y;
	}
}

FColor UDistributionFloatUniformCurve::GetKeyColor(INT SubIndex, INT KeyIndex, const FColor& CurveColor)
{
	check((SubIndex >= 0) && (SubIndex < 2));
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));

	if (SubIndex == 0)
	{
		return FColor(255,0,0);
	}
	else
	{
		return FColor(0,255,0);
	}
}

void UDistributionFloatUniformCurve::GetInRange(FLOAT& MinIn, FLOAT& MaxIn)
{
	if (ConstantCurve.Points.Num() == 0)
	{
		MinIn = 0.f;
		MaxIn = 0.f;
	}
	else
	{
		FLOAT Min = BIG_NUMBER;
		FLOAT Max = -BIG_NUMBER;
		for (INT Index = 0; Index < ConstantCurve.Points.Num(); Index++)
		{
			FLOAT Value = ConstantCurve.Points(Index).InVal;
			if (Value < Min)
			{
				Min = Value;
			}
			if (Value > Max)
			{
				Max = Value;
			}
		}
		MinIn = Min;
		MaxIn = Max;
	}
}

void UDistributionFloatUniformCurve::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	FVector2D MinVec, MaxVec;
	ConstantCurve.CalcBounds(MinVec, MaxVec, FVector2D(0.f,0.f));
	MinOut = MinVec.GetMin();
	MaxOut = MaxVec.GetMax();
}

BYTE UDistributionFloatUniformCurve::GetKeyInterpMode(INT KeyIndex)
{
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));
	return ConstantCurve.Points(KeyIndex).InterpMode;
}

void UDistributionFloatUniformCurve::GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent)
{
	check((SubIndex >= 0) && (SubIndex < 2));
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));

	if (SubIndex == 0)
	{
		ArriveTangent = ConstantCurve.Points(KeyIndex).ArriveTangent.X;
		LeaveTangent = ConstantCurve.Points(KeyIndex).LeaveTangent.X;
	}
	else
	{
		ArriveTangent = ConstantCurve.Points(KeyIndex).ArriveTangent.Y;
		LeaveTangent = ConstantCurve.Points(KeyIndex).LeaveTangent.Y;
	}
}

FLOAT UDistributionFloatUniformCurve::EvalSub(INT SubIndex, FLOAT InVal)
{
	check((SubIndex >= 0) && (SubIndex < 2));

	FVector2D OutVal = ConstantCurve.Eval(InVal, FVector2D(0.f,0.f));

	if (SubIndex == 0)
	{
		return OutVal.X;
	}
	else
	{
		return OutVal.Y;
	}
}

INT UDistributionFloatUniformCurve::CreateNewKey(FLOAT KeyIn)
{	
	FVector2D NewKeyVal = ConstantCurve.Eval(KeyIn, FVector2D(0.f,0.f));
	INT NewPointIndex = ConstantCurve.AddPoint(KeyIn, NewKeyVal);
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;

	return NewPointIndex;
}

void UDistributionFloatUniformCurve::DeleteKey(INT KeyIndex)
{
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));
	ConstantCurve.Points.Remove(KeyIndex);
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;
}

INT UDistributionFloatUniformCurve::SetKeyIn(INT KeyIndex, FLOAT NewInVal)
{
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));
	INT NewPointIndex = ConstantCurve.MovePoint(KeyIndex, NewInVal);
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;

	return NewPointIndex;
}

void UDistributionFloatUniformCurve::SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal) 
{
	check((SubIndex >= 0) && (SubIndex < 2));
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));

	if (SubIndex == 0)
	{
		ConstantCurve.Points(KeyIndex).OutVal.X = NewOutVal;
	}
	else
	{
		ConstantCurve.Points(KeyIndex).OutVal.Y = NewOutVal;
	}

	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;
}

void UDistributionFloatUniformCurve::SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode) 
{
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));
	ConstantCurve.Points(KeyIndex).InterpMode = NewMode;
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;
}

void UDistributionFloatUniformCurve::SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent)
{
	check((SubIndex >= 0) && (SubIndex < 2));
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));

	if(SubIndex == 0)
	{
		ConstantCurve.Points(KeyIndex).ArriveTangent.X = ArriveTangent;
		ConstantCurve.Points(KeyIndex).LeaveTangent.X = LeaveTangent;
	}
	else
	{
		ConstantCurve.Points(KeyIndex).ArriveTangent.Y = ArriveTangent;
		ConstantCurve.Points(KeyIndex).LeaveTangent.Y = LeaveTangent;
	}

	bIsDirty = TRUE;
}


/** Returns TRUE if this curve uses legacy tangent/interp algorithms and may be 'upgraded' */
UBOOL UDistributionFloatUniformCurve::UsingLegacyInterpMethod() const
{
	return ConstantCurve.UsingLegacyInterpMethod();
}


/** 'Upgrades' this curve to use the latest tangent/interp algorithms (usually, will 'bake' key tangents.) */
void UDistributionFloatUniformCurve::UpgradeInterpMethod()
{
	if( UsingLegacyInterpMethod() )
	{
		ConstantCurve.UpgradeInterpMethod();

		bIsDirty = TRUE;
	}
}

/*-----------------------------------------------------------------------------
	UDistributionVectorConstant implementation.
-----------------------------------------------------------------------------*/

FVector UDistributionVectorConstant::GetValue(FLOAT F, UObject* Data, INT Extreme)
{
	switch (LockedAxes)
	{
    case EDVLF_XY:
		return FVector(Constant.X, Constant.X, Constant.Z);
    case EDVLF_XZ:
		return FVector(Constant.X, Constant.Y, Constant.X);
    case EDVLF_YZ:
		return FVector(Constant.X, Constant.Y, Constant.Y);
	case EDVLF_XYZ:
		return FVector(Constant.X);
    case EDVLF_None:
	default:
		return Constant;
	}
}

void UDistributionVectorConstant::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}


//////////////////////// FCurveEdInterface ////////////////////////

INT UDistributionVectorConstant::GetNumKeys()
{
	return 1;
}

INT UDistributionVectorConstant::GetNumSubCurves()
{
	switch (LockedAxes)
	{
    case EDVLF_XY:
    case EDVLF_XZ:
    case EDVLF_YZ:
		return 2;
	case EDVLF_XYZ:
		return 1;
	}
	return 3;
}

FLOAT UDistributionVectorConstant::GetKeyIn(INT KeyIndex)
{
	check( KeyIndex == 0 );
	return 0.f;
}

FLOAT UDistributionVectorConstant::GetKeyOut(INT SubIndex, INT KeyIndex)
{
	check( SubIndex >= 0 && SubIndex < 3);
	check( KeyIndex == 0 );
	
	if (SubIndex == 0)
	{
		return Constant.X;
	}
	else 
	if (SubIndex == 1)
	{
		if ((LockedAxes == EDVLF_XY) || (LockedAxes == EDVLF_XYZ))
		{
			return Constant.X;
		}
		else
		{
			return Constant.Y;
		}
	}
	else 
	{
		if ((LockedAxes == EDVLF_XZ) || (LockedAxes == EDVLF_XYZ))
		{
			return Constant.X;
		}
		else
		if (LockedAxes == EDVLF_YZ)
		{
			return Constant.Y;
		}
		else
		{
			return Constant.Z;
		}
	}
}

FColor UDistributionVectorConstant::GetKeyColor(INT SubIndex, INT KeyIndex, const FColor& CurveColor)
{
	check( SubIndex >= 0 && SubIndex < 3);
	check( KeyIndex == 0 );

	if(SubIndex == 0)
		return FColor(255,0,0);
	else if(SubIndex == 1)
		return FColor(0,255,0);
	else
		return FColor(0,0,255);
}

void UDistributionVectorConstant::GetInRange(FLOAT& MinIn, FLOAT& MaxIn)
{
	MinIn = 0.f;
	MaxIn = 0.f;
}

void UDistributionVectorConstant::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	FVector Local;

	switch (LockedAxes)
	{
    case EDVLF_XY:
		Local = FVector(Constant.X, Constant.X, Constant.Z);
		break;
    case EDVLF_XZ:
		Local = FVector(Constant.X, Constant.Y, Constant.X);
		break;
    case EDVLF_YZ:
		Local = FVector(Constant.X, Constant.Y, Constant.Y);
		break;
    case EDVLF_XYZ:
		Local = FVector(Constant.X);
		break;
	case EDVLF_None:
	default:
		Local = FVector(Constant);
		break;
	}

	MinOut = Local.GetMin();
	MaxOut = Local.GetMax();
}

BYTE UDistributionVectorConstant::GetKeyInterpMode(INT KeyIndex)
{
	check( KeyIndex == 0 );
	return CIM_Constant;
}

void UDistributionVectorConstant::GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent)
{
	check( SubIndex >= 0 && SubIndex < 3);
	check( KeyIndex == 0 );
	ArriveTangent = 0.f;
	LeaveTangent = 0.f;
}

FLOAT UDistributionVectorConstant::EvalSub(INT SubIndex, FLOAT InVal)
{
	check( SubIndex >= 0 && SubIndex < 3);
	return GetKeyOut(SubIndex, 0);
}

INT UDistributionVectorConstant::CreateNewKey(FLOAT KeyIn)
{	
	return 0;
}

void UDistributionVectorConstant::DeleteKey(INT KeyIndex)
{
	check( KeyIndex == 0 );
}

INT UDistributionVectorConstant::SetKeyIn(INT KeyIndex, FLOAT NewInVal)
{
	check( KeyIndex == 0 );
	return 0;
}

void UDistributionVectorConstant::SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal) 
{
	check( SubIndex >= 0 && SubIndex < 3);
	check( KeyIndex == 0 );

	if(SubIndex == 0)
		Constant.X = NewOutVal;
	else if(SubIndex == 1)
		Constant.Y = NewOutVal;
	else if(SubIndex == 2)
		Constant.Z = NewOutVal;

	bIsDirty = TRUE;
}

void UDistributionVectorConstant::SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode) 
{
	check( KeyIndex == 0 );
}

void UDistributionVectorConstant::SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent)
{
	check( SubIndex >= 0 && SubIndex < 3);
	check( KeyIndex == 0 );
}

// DistributionVector interface
/** GetRange - in the case of a constant curve, this will not be exact!				*/
void UDistributionVectorConstant::GetRange(FVector& OutMin, FVector& OutMax)
{
	OutMin	= Constant;
	OutMax	= Constant;
}

/*-----------------------------------------------------------------------------
	UDistributionVectorConstantCurve implementation.
-----------------------------------------------------------------------------*/

FVector UDistributionVectorConstantCurve::GetValue(FLOAT F, UObject* Data, INT Extreme)
{
	FVector Val = ConstantCurve.Eval(F, FVector(0.f));
	switch (LockedAxes)
	{
    case EDVLF_XY:
		return FVector(Val.X, Val.X, Val.Z);
    case EDVLF_XZ:
		return FVector(Val.X, Val.Y, Val.X);
    case EDVLF_YZ:
		return FVector(Val.X, Val.Y, Val.Y);
	case EDVLF_XYZ:
		return FVector(Val.X);
    case EDVLF_None:
	default:
		return Val;
	}
}

void UDistributionVectorConstantCurve::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}

//////////////////////// FCurveEdInterface ////////////////////////

INT UDistributionVectorConstantCurve::GetNumKeys()
{
	return ConstantCurve.Points.Num();
}

INT UDistributionVectorConstantCurve::GetNumSubCurves()
{
	switch (LockedAxes)
	{
    case EDVLF_XY:
    case EDVLF_XZ:
    case EDVLF_YZ:
		return 2;
	case EDVLF_XYZ:
		return 1;
	}
	return 3;
}

FLOAT UDistributionVectorConstantCurve::GetKeyIn(INT KeyIndex)
{
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	return ConstantCurve.Points(KeyIndex).InVal;
}

FLOAT UDistributionVectorConstantCurve::GetKeyOut(INT SubIndex, INT KeyIndex)
{
	check( SubIndex >= 0 && SubIndex < 3);
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	
	if (SubIndex == 0)
	{
		return ConstantCurve.Points(KeyIndex).OutVal.X;
	}
	else
	if(SubIndex == 1)
	{
		if ((LockedAxes == EDVLF_XY) || (LockedAxes == EDVLF_XYZ))
		{
			return ConstantCurve.Points(KeyIndex).OutVal.X;
		}

		return ConstantCurve.Points(KeyIndex).OutVal.Y;
	}
	else 
	{
		if ((LockedAxes == EDVLF_XZ) || (LockedAxes == EDVLF_XYZ))
		{
			return ConstantCurve.Points(KeyIndex).OutVal.X;
		}
		else
		if (LockedAxes == EDVLF_YZ)
		{
			return ConstantCurve.Points(KeyIndex).OutVal.Y;
		}

		return ConstantCurve.Points(KeyIndex).OutVal.Z;
	}
}

FColor UDistributionVectorConstantCurve::GetKeyColor(INT SubIndex, INT KeyIndex, const FColor& CurveColor)
{
	check( SubIndex >= 0 && SubIndex < 3);
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );

	if(SubIndex == 0)
		return FColor(255,0,0);
	else if(SubIndex == 1)
		return FColor(0,255,0);
	else
		return FColor(0,0,255);
}

void UDistributionVectorConstantCurve::GetInRange(FLOAT& MinIn, FLOAT& MaxIn)
{
	if( ConstantCurve.Points.Num() == 0 )
	{
		MinIn = 0.f;
		MaxIn = 0.f;
	}
	else
	{
		FLOAT Min = BIG_NUMBER;
		FLOAT Max = -BIG_NUMBER;
		for (INT Index = 0; Index < ConstantCurve.Points.Num(); Index++)
		{
			FLOAT Value = ConstantCurve.Points(Index).InVal;
			if (Value < Min)
			{
				Min = Value;
			}
			if (Value > Max)
			{
				Max = Value;
			}
		}
		MinIn = Min;
		MaxIn = Max;
	}
}

void UDistributionVectorConstantCurve::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	FVector MinVec, MaxVec;
	ConstantCurve.CalcBounds(MinVec, MaxVec, FVector(0.f));

	switch (LockedAxes)
	{
    case EDVLF_XY:
		MinVec.Y = MinVec.X;
		MaxVec.Y = MaxVec.X;
		break;
    case EDVLF_XZ:
		MinVec.Z = MinVec.X;
		MaxVec.Z = MaxVec.X;
		break;
    case EDVLF_YZ:
		MinVec.Z = MinVec.Y;
		MaxVec.Z = MaxVec.Y;
		break;
    case EDVLF_XYZ:
		MinVec.Y = MinVec.X;
		MinVec.Z = MinVec.X;
		MaxVec.Y = MaxVec.X;
		MaxVec.Z = MaxVec.X;
		break;
    case EDVLF_None:
	default:
		break;
	}

	MinOut = MinVec.GetMin();
	MaxOut = MaxVec.GetMax();
}

BYTE UDistributionVectorConstantCurve::GetKeyInterpMode(INT KeyIndex)
{
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	return ConstantCurve.Points(KeyIndex).InterpMode;
}

void UDistributionVectorConstantCurve::GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent)
{
	check( SubIndex >= 0 && SubIndex < 3);
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );

	if(SubIndex == 0)
	{
		ArriveTangent = ConstantCurve.Points(KeyIndex).ArriveTangent.X;
		LeaveTangent = ConstantCurve.Points(KeyIndex).LeaveTangent.X;
	}
	else if(SubIndex == 1)
	{
		ArriveTangent = ConstantCurve.Points(KeyIndex).ArriveTangent.Y;
		LeaveTangent = ConstantCurve.Points(KeyIndex).LeaveTangent.Y;
	}
	else if(SubIndex == 2)
	{
		ArriveTangent = ConstantCurve.Points(KeyIndex).ArriveTangent.Z;
		LeaveTangent = ConstantCurve.Points(KeyIndex).LeaveTangent.Z;
	}
}

FLOAT UDistributionVectorConstantCurve::EvalSub(INT SubIndex, FLOAT InVal)
{
	check( SubIndex >= 0 && SubIndex < 3);

	FVector OutVal = ConstantCurve.Eval(InVal, FVector(0.f));

	if(SubIndex == 0)
		return OutVal.X;
	else if(SubIndex == 1)
		return OutVal.Y;
	else
		return OutVal.Z;
}

INT UDistributionVectorConstantCurve::CreateNewKey(FLOAT KeyIn)
{	
	FVector NewKeyVal = ConstantCurve.Eval(KeyIn, FVector(0.f));
	INT NewPointIndex = ConstantCurve.AddPoint(KeyIn, NewKeyVal);
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;

	return NewPointIndex;
}

void UDistributionVectorConstantCurve::DeleteKey(INT KeyIndex)
{
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	ConstantCurve.Points.Remove(KeyIndex);
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;
}

INT UDistributionVectorConstantCurve::SetKeyIn(INT KeyIndex, FLOAT NewInVal)
{
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	INT NewPointIndex = ConstantCurve.MovePoint(KeyIndex, NewInVal);
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;

	return NewPointIndex;
}

void UDistributionVectorConstantCurve::SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal) 
{
	check( SubIndex >= 0 && SubIndex < 3);
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );

	if(SubIndex == 0)
		ConstantCurve.Points(KeyIndex).OutVal.X = NewOutVal;
	else if(SubIndex == 1)
		ConstantCurve.Points(KeyIndex).OutVal.Y = NewOutVal;
	else 
		ConstantCurve.Points(KeyIndex).OutVal.Z = NewOutVal;

	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;
}

void UDistributionVectorConstantCurve::SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode) 
{
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	
	ConstantCurve.Points(KeyIndex).InterpMode = NewMode;
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;
}

void UDistributionVectorConstantCurve::SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent)
{
	check( SubIndex >= 0 && SubIndex < 3);
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );

	if(SubIndex == 0)
	{
		ConstantCurve.Points(KeyIndex).ArriveTangent.X = ArriveTangent;
		ConstantCurve.Points(KeyIndex).LeaveTangent.X = LeaveTangent;
	}
	else if(SubIndex == 1)
	{
		ConstantCurve.Points(KeyIndex).ArriveTangent.Y = ArriveTangent;
		ConstantCurve.Points(KeyIndex).LeaveTangent.Y = LeaveTangent;
	}
	else if(SubIndex == 2)
	{
		ConstantCurve.Points(KeyIndex).ArriveTangent.Z = ArriveTangent;
		ConstantCurve.Points(KeyIndex).LeaveTangent.Z = LeaveTangent;
	}

	bIsDirty = TRUE;
}

// DistributionVector interface
/** GetRange - in the case of a constant curve, this will not be exact!				*/
void UDistributionVectorConstantCurve::GetRange(FVector& OutMin, FVector& OutMax)
{
	FVector MinVec, MaxVec;
	ConstantCurve.CalcBounds(MinVec, MaxVec, FVector(0.f));

	switch (LockedAxes)
	{
    case EDVLF_XY:
		MinVec.Y = MinVec.X;
		MaxVec.Y = MaxVec.X;
		break;
    case EDVLF_XZ:
		MinVec.Z = MinVec.X;
		MaxVec.Z = MaxVec.X;
		break;
    case EDVLF_YZ:
		MinVec.Z = MinVec.Y;
		MaxVec.Z = MaxVec.Y;
		break;
    case EDVLF_XYZ:
		MinVec.Y = MinVec.X;
		MinVec.Z = MinVec.X;
		MaxVec.Y = MaxVec.X;
		MaxVec.Z = MaxVec.X;
		break;
    case EDVLF_None:
	default:
		break;
	}

	OutMin = MinVec;
	OutMax = MaxVec;
}


/** Returns TRUE if this curve uses legacy tangent/interp algorithms and may be 'upgraded' */
UBOOL UDistributionVectorConstantCurve::UsingLegacyInterpMethod() const
{
	return ConstantCurve.UsingLegacyInterpMethod();
}


/** 'Upgrades' this curve to use the latest tangent/interp algorithms (usually, will 'bake' key tangents.) */
void UDistributionVectorConstantCurve::UpgradeInterpMethod()
{
	if( UsingLegacyInterpMethod() )
	{
		ConstantCurve.UpgradeInterpMethod();

		bIsDirty = TRUE;
	}
}


/*-----------------------------------------------------------------------------
	UDistributionVectorUniform implementation.
-----------------------------------------------------------------------------*/
void UDistributionVectorUniform::PostLoad()
{
	if (GetLinker() && (GetLinker()->Ver() < VER_UNIFORM_DISTRIBUTION_BAKING_UPDATE))
	{
		bIsDirty = TRUE;
		MarkPackageDirty();
	}

	if (GetLinker() && (GetLinker()->Ver() < VER_LOCKED_UNIFORM_DISTRIBUTION_BAKING))
	{
		if (LockedAxes != EDVLF_None)
		{
			bIsDirty = TRUE;
		}
	}

	Super::PostLoad();
}

FVector UDistributionVectorUniform::GetValue(FLOAT F, UObject* Data, INT Extreme)
{
	FVector LocalMax = Max;
	FVector LocalMin = Min;

	for (INT i = 0; i < 3; i++)
	{
		switch (MirrorFlags[i])
		{
		case EDVMF_Same:	LocalMin[i] =  LocalMax[i];		break;
		case EDVMF_Mirror:	LocalMin[i] = -LocalMax[i];		break;
		}
	}

	FLOAT fX;
	FLOAT fY;
	FLOAT fZ;

	UBOOL	bMin	= TRUE;
	if (bUseExtremes)
	{
		if (Extreme == 0)
		{
			if (appSRand() > 0.5f)
			{
				bMin	= FALSE;
			}
		}
		else
		if (Extreme > 0)
		{
			bMin	= FALSE;
		}
	}

	switch (LockedAxes)
	{
    case EDVLF_XY:
		if (bUseExtremes)
		{
			if (bMin)
			{
				fX = LocalMin.X;
				fZ = LocalMin.Z;
			}
			else
			{
				fX = LocalMax.X;
				fZ = LocalMax.Z;
			}
		}
		else
		{
			fX = LocalMax.X + (LocalMin.X - LocalMax.X) * appSRand();
			fZ = LocalMax.Z + (LocalMin.Z - LocalMax.Z) * appSRand();
		}
		fY = fX;
		break;
    case EDVLF_XZ:
		if (bUseExtremes)
		{
			if (bMin)
			{
				fX = LocalMin.X;
				fY = LocalMin.Y;
			}
			else
			{
				fX = LocalMax.X;
				fY = LocalMax.Y;
			}
		}
		else
		{
			fX = LocalMax.X + (LocalMin.X - LocalMax.X) * appSRand();
			fY = LocalMax.Y + (LocalMin.Y - LocalMax.Y) * appSRand();
		}
		fZ = fX;
		break;
    case EDVLF_YZ:
		if (bUseExtremes)
		{
			if (bMin)
			{
				fX = LocalMin.X;
				fY = LocalMin.Y;
			}
			else
			{
				fX = LocalMax.X;
				fY = LocalMax.Y;
			}
		}
		else
		{
			fX = LocalMax.X + (LocalMin.X - LocalMax.X) * appSRand();
			fY = LocalMax.Y + (LocalMin.Y - LocalMax.Y) * appSRand();
		}
		fZ = fY;
		break;
	case EDVLF_XYZ:
		if (bUseExtremes)
		{
			if (bMin)
			{
				fX = LocalMin.X;
			}
			else
			{
				fX = LocalMax.X;
			}
		}
		else
		{
			fX = LocalMax.X + (LocalMin.X - LocalMax.X) * appSRand();
		}
		fY = fX;
		fZ = fX;
		break;
    case EDVLF_None:
	default:
		if (bUseExtremes)
		{
			if (bMin)
			{
				fX = LocalMin.X;
				fY = LocalMin.Y;
				fZ = LocalMin.Z;
			}
			else
			{
				fX = LocalMax.X;
				fY = LocalMax.Y;
				fZ = LocalMax.Z;
			}
		}
		else
		{
			fX = LocalMax.X + (LocalMin.X - LocalMax.X) * appSRand();
			fY = LocalMax.Y + (LocalMin.Y - LocalMax.Y) * appSRand();
			fZ = LocalMax.Z + (LocalMin.Z - LocalMax.Z) * appSRand();
		}
		break;
	}

	return FVector(fX, fY, fZ);
}

#if !CONSOLE

/**
 * Return the operation used at runtime to calculate the final value
 */
ERawDistributionOperation UDistributionVectorUniform::GetOperation()
{
	if (Min == Max)
	{
		// This may as well be a constant - don't bother doing the appSRand scaling on it.
		return RDO_None;
	}
	// override the operation to use
	return bUseExtremes ? RDO_Extreme : RDO_Random;
}

/**
 * Fill out an array of vectors and return the number of elements in the entry
 *
 * @param Time The time to evaluate the distribution
 * @param Values An array of values to be filled out, guaranteed to be big enough for 2 vectors
 * @return The number of elements (values) set in the array
 */
DWORD UDistributionVectorUniform::InitializeRawEntry(FLOAT Time, FVector* Values)
{
	// get the locked/mirrored min and max
	Values[0] = GetMinValue();
	Values[1] = GetMaxValue();

	// two elements per value
	return 2;
}

#endif

FVector UDistributionVectorUniform::GetMinValue()
{
	FVector LocalMax = Max;
	FVector LocalMin = Min;

	for (INT i = 0; i < 3; i++)
	{
		switch (MirrorFlags[i])
		{
		case EDVMF_Same:	LocalMin[i] =  LocalMax[i];		break;
		case EDVMF_Mirror:	LocalMin[i] = -LocalMax[i];		break;
		}
	}

	FLOAT fX;
	FLOAT fY;
	FLOAT fZ;

	switch (LockedAxes)
	{
    case EDVLF_XY:
		fX = LocalMin.X;
		fY = LocalMin.X;
		fZ = LocalMin.Z;
		break;
    case EDVLF_XZ:
		fX = LocalMin.X;
		fY = LocalMin.Y;
		fZ = fX;
		break;
    case EDVLF_YZ:
		fX = LocalMin.X;
		fY = LocalMin.Y;
		fZ = fY;
		break;
	case EDVLF_XYZ:
		fX = LocalMin.X;
		fY = fX;
		fZ = fX;
		break;
    case EDVLF_None:
	default:
		fX = LocalMin.X;
		fY = LocalMin.Y;
		fZ = LocalMin.Z;
		break;
	}

	return FVector(fX, fY, fZ);
}

FVector UDistributionVectorUniform::GetMaxValue()
{
	FVector LocalMax = Max;

	FLOAT fX;
	FLOAT fY;
	FLOAT fZ;

	switch (LockedAxes)
	{
    case EDVLF_XY:
		fX = LocalMax.X;
		fY = LocalMax.X;
		fZ = LocalMax.Z;
		break;
    case EDVLF_XZ:
		fX = LocalMax.X;
		fY = LocalMax.Y;
		fZ = fX;
		break;
    case EDVLF_YZ:
		fX = LocalMax.X;
		fY = LocalMax.Y;
		fZ = fY;
		break;
	case EDVLF_XYZ:
		fX = LocalMax.X;
		fY = fX;
		fZ = fX;
		break;
    case EDVLF_None:
	default:
		fX = LocalMax.X;
		fY = LocalMax.Y;
		fZ = LocalMax.Z;
		break;
	}

	return FVector(fX, fY, fZ);
}

void UDistributionVectorUniform::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}

//////////////////////// FCurveEdInterface ////////////////////////

// We have 6 subs, 3 mins and three maxes. They are assigned as:
// 0,1 = min/max x
// 2,3 = min/max y
// 4,5 = min/max z

INT UDistributionVectorUniform::GetNumKeys()
{
	return 1;
}

INT UDistributionVectorUniform::GetNumSubCurves()
{
	switch (LockedAxes)
	{
    case EDVLF_XY:
    case EDVLF_XZ:
    case EDVLF_YZ:
		return 4;
	case EDVLF_XYZ:
		return 2;
	}
	return 6;
}

FLOAT UDistributionVectorUniform::GetKeyIn(INT KeyIndex)
{
	check( KeyIndex == 0 );
	return 0.f;
}

FLOAT UDistributionVectorUniform::GetKeyOut(INT SubIndex, INT KeyIndex)
{
	check( SubIndex >= 0 && SubIndex < 6 );
	check( KeyIndex == 0 );

	FVector LocalMax = Max;
	FVector LocalMin = Min;

	for (INT i = 0; i < 3; i++)
	{
		switch (MirrorFlags[i])
		{
		case EDVMF_Same:	LocalMin[i] =  LocalMax[i];		break;
		case EDVMF_Mirror:	LocalMin[i] = -LocalMax[i];		break;
		}
	}

	switch (LockedAxes)
	{
    case EDVLF_XY:
		LocalMin.Y = LocalMin.X;
		break;
    case EDVLF_XZ:
		LocalMin.Z = LocalMin.X;
		break;
    case EDVLF_YZ:
		LocalMin.Z = LocalMin.Y;
		break;
	case EDVLF_XYZ:
		LocalMin.Y = LocalMin.X;
		LocalMin.Z = LocalMin.X;
		break;
    case EDVLF_None:
	default:
		break;
	}

	switch (SubIndex)
	{
	case 0:		return LocalMin.X;
	case 1:		return LocalMax.X;
	case 2:		return LocalMin.Y;
	case 3:		return LocalMax.Y;
	case 4:		return LocalMin.Z;
	}
	return LocalMax.Z;
}

FColor UDistributionVectorUniform::GetKeyColor(INT SubIndex, INT KeyIndex, const FColor& CurveColor)
{
	check( SubIndex >= 0 && SubIndex < 6);
	check( KeyIndex == 0 );

	if(SubIndex == 0)
		return FColor(128,0,0);
	else if(SubIndex == 1)
		return FColor(255,0,0);
	else if(SubIndex == 2)
		return FColor(0,128,0);
	else if(SubIndex == 3)
		return FColor(0,255,0);
	else if(SubIndex == 4)
		return FColor(0,0,128);
	else
		return FColor(0,0,255);
}

void UDistributionVectorUniform::GetInRange(FLOAT& MinIn, FLOAT& MaxIn)
{
	MinIn = 0.f;
	MaxIn = 0.f;
}

void UDistributionVectorUniform::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	FVector LocalMax = Max;
	FVector LocalMin = Min;

	for (INT i = 0; i < 3; i++)
	{
		switch (MirrorFlags[i])
		{
		case EDVMF_Same:	LocalMin[i] =  LocalMax[i];		break;
		case EDVMF_Mirror:	LocalMin[i] = -LocalMax[i];		break;
		}
	}

	FVector LocalMin2;
	FVector LocalMax2;

	switch (LockedAxes)
	{
    case EDVLF_XY:
		LocalMin2 = FVector(LocalMin.X, LocalMin.X, LocalMin.Z);
		LocalMax2 = FVector(LocalMax.X, LocalMax.X, LocalMax.Z);
		break;
    case EDVLF_XZ:
		LocalMin2 = FVector(LocalMin.X, LocalMin.Y, LocalMin.X);
		LocalMax2 = FVector(LocalMax.X, LocalMax.Y, LocalMax.X);
		break;
    case EDVLF_YZ:
		LocalMin2 = FVector(LocalMin.X, LocalMin.Y, LocalMin.Y);
		LocalMax2 = FVector(LocalMax.X, LocalMax.Y, LocalMax.Y);
		break;
    case EDVLF_XYZ:
		LocalMin2 = FVector(LocalMin.X);
		LocalMax2 = FVector(LocalMax.X);
		break;
    case EDVLF_None:
	default:
		LocalMin2 = FVector(LocalMin.X, LocalMin.Y, LocalMin.Z);
		LocalMax2 = FVector(LocalMax.X, LocalMax.Y, LocalMax.Z);
		break;
	}

	MinOut = LocalMin2.GetMin();
	MaxOut = LocalMax2.GetMax();
}

BYTE UDistributionVectorUniform::GetKeyInterpMode(INT KeyIndex)
{
	check( KeyIndex == 0 );
	return CIM_Constant;
}

void UDistributionVectorUniform::GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent)
{
	check( SubIndex >= 0 && SubIndex < 6);
	check( KeyIndex == 0 );
	ArriveTangent = 0.f;
	LeaveTangent = 0.f;
}

FLOAT UDistributionVectorUniform::EvalSub(INT SubIndex, FLOAT InVal)
{
	return GetKeyOut(SubIndex, 0);
}

INT UDistributionVectorUniform::CreateNewKey(FLOAT KeyIn)
{	
	return 0;
}

void UDistributionVectorUniform::DeleteKey(INT KeyIndex)
{
	check( KeyIndex == 0 );
}

INT UDistributionVectorUniform::SetKeyIn(INT KeyIndex, FLOAT NewInVal)
{
	check( KeyIndex == 0 );
	return 0;
}

void UDistributionVectorUniform::SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal) 
{
	check( SubIndex >= 0 && SubIndex < 6 );
	check( KeyIndex == 0 );

	if(SubIndex == 0)
		Min.X = ::Min<FLOAT>(NewOutVal, Max.X);
	else if(SubIndex == 1)
		Max.X = ::Max<FLOAT>(NewOutVal, Min.X);
	else if(SubIndex == 2)
		Min.Y = ::Min<FLOAT>(NewOutVal, Max.Y);
	else if(SubIndex == 3)
		Max.Y = ::Max<FLOAT>(NewOutVal, Min.Y);
	else if(SubIndex == 4)
		Min.Z = ::Min<FLOAT>(NewOutVal, Max.Z);
	else
		Max.Z = ::Max<FLOAT>(NewOutVal, Min.Z);

	bIsDirty = TRUE;
}

void UDistributionVectorUniform::SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode) 
{
	check( KeyIndex == 0 );
}

void UDistributionVectorUniform::SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent)
{
	check( SubIndex >= 0 && SubIndex < 6);
	check( KeyIndex == 0 );
}

// DistributionVector interface
/** GetRange - in the case of a constant curve, this will not be exact!				*/
void UDistributionVectorUniform::GetRange(FVector& OutMin, FVector& OutMax)
{
	OutMin	= Min;
	OutMax	= Max;
}

//
//	UDistributionVectorUniformCurve
//
void UDistributionVectorUniformCurve::PostLoad()
{
	if (GetLinker() && (GetLinker()->Ver() < VER_UNIFORM_DISTRIBUTION_BAKING_UPDATE))
	{
		bIsDirty = TRUE;
		MarkPackageDirty();
	}

	if (GetLinker() && (GetLinker()->Ver() < VER_LOCKED_UNIFORM_DISTRIBUTION_BAKING))
	{
		if ((LockedAxes[0] != EDVLF_None) || (LockedAxes[1] != EDVLF_None))
		{
			bIsDirty = TRUE;
		}
	}

	Super::PostLoad();
}

FVector UDistributionVectorUniformCurve::GetValue(FLOAT F, UObject* Data, INT Extreme)
{
	FTwoVectors	Val = ConstantCurve.Eval(F, FTwoVectors());

	UBOOL	bMin	= TRUE;
	if (bUseExtremes)
	{
		if (Extreme == 0)
		{
			if (appSRand() > 0.5f)
			{
				bMin	= FALSE;
			}
		}
		else
		if (Extreme < 0)
		{
			bMin	= FALSE;
		}
	}

	LockAndMirror(Val);
	if  (bUseExtremes)
	{
		if (bMin)
		{
			return FVector(
				Val.v2.X,
				Val.v2.Y,
				Val.v2.Z);
		}
		else
		{
			return FVector(
				Val.v1.X,
				Val.v1.Y,
				Val.v1.Z);
		}
	}
	else
	{
		return FVector(
			Val.v1.X + (Val.v2.X - Val.v1.X) * appSRand(),
			Val.v1.Y + (Val.v2.Y - Val.v1.Y) * appSRand(),
			Val.v1.Z + (Val.v2.Z - Val.v1.Z) * appSRand());
	}
}

#if !CONSOLE

/**
 * Return the operation used at runtime to calculate the final value
 */
ERawDistributionOperation UDistributionVectorUniformCurve::GetOperation()
{
	if (ConstantCurve.Points.Num() == 1)
	{
		// Only a single point - so see if Min == Max
		FInterpCurvePoint<FTwoVectors>& Value = ConstantCurve.Points(0);
		if (Value.OutVal.v1 == Value.OutVal.v2)
		{
			// This may as well be a constant - don't bother doing the appSRand scaling on it.
			return RDO_None;
		}
	}
	return bUseExtremes ? RDO_Extreme : RDO_Random;
}

/**
 * Fill out an array of vectors and return the number of elements in the entry
 *
 * @param Time The time to evaluate the distribution
 * @param Values An array of values to be filled out, guaranteed to be big enough for 2 vectors
 * @return The number of elements (values) set in the array
 */
DWORD UDistributionVectorUniformCurve::InitializeRawEntry(FLOAT Time, FVector* Values)
{
	// get the min and max values at the current time (just Eval the curve)
	FTwoVectors MinMax = GetMinMaxValue(Time, NULL);
	// apply any axis locks and mirroring (in place)
	LockAndMirror(MinMax);

	// copy out the values
	Values[0] = MinMax.v1;
	Values[1] = MinMax.v2;

	// we wrote two elements
	return 2;
}

#endif

/**
 *	This function will retrieve the max and min values at the given time.
 */
FTwoVectors UDistributionVectorUniformCurve::GetMinMaxValue(FLOAT F, UObject* Data)
{
	return ConstantCurve.Eval(F, FTwoVectors());
}

/** These two functions will retrieve the Min/Max values respecting the Locked and Mirror flags. */
FVector UDistributionVectorUniformCurve::GetMinValue()
{
	check(!TEXT("Don't call me!"));
	return FVector(0.0f);
}

FVector UDistributionVectorUniformCurve::GetMaxValue()
{
	check(!TEXT("Don't call me!"));
	return FVector(0.0f);
}

// UObject interface
void UDistributionVectorUniformCurve::Serialize(FArchive& Ar)
{
	// No need to override old versions - this is a new class...
	Super::Serialize(Ar);
}

// FCurveEdInterface interface
INT UDistributionVectorUniformCurve::GetNumKeys()
{
	return ConstantCurve.Points.Num();
}

INT UDistributionVectorUniformCurve::GetNumSubCurves()
{
	INT Count = 0;
/***
	switch (LockedAxes[0])
	{
    case EDVLF_XY:	Count += 2;	break;
    case EDVLF_XZ:	Count += 2;	break;
    case EDVLF_YZ:	Count += 2;	break;
	case EDVLF_XYZ:	Count += 1;	break;
	default:		Count += 3;	break;
	}

	switch (LockedAxes[1])
	{
    case EDVLF_XY:	Count += 2;	break;
    case EDVLF_XZ:	Count += 2;	break;
    case EDVLF_YZ:	Count += 2;	break;
	case EDVLF_XYZ:	Count += 1;	break;
	default:		Count += 3;	break;
	}
***/
	Count = 6;
	return Count;
}

FLOAT UDistributionVectorUniformCurve::GetKeyIn(INT KeyIndex)
{
	check( KeyIndex >= 0 && KeyIndex < ConstantCurve.Points.Num() );
	return ConstantCurve.Points(KeyIndex).InVal;
}

FLOAT UDistributionVectorUniformCurve::GetKeyOut(INT SubIndex, INT KeyIndex)
{
	check((SubIndex >= 0) && (SubIndex < 6));
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));


	// Grab the value
	FInterpCurvePoint<FTwoVectors>	Point = ConstantCurve.Points(KeyIndex);

	FTwoVectors	Val	= Point.OutVal;
	LockAndMirror(Val);
	if ((SubIndex % 2) == 0)
	{
		return Val.v1[SubIndex / 2];
	}
	else
	{
		return Val.v2[SubIndex / 2];
	}
	check(!TEXT("Should not reach here!"));
	return 0.0f;
}

FColor UDistributionVectorUniformCurve::GetKeyColor(INT SubIndex, INT KeyIndex, const FColor& CurveColor)
{
	check((SubIndex >= 0) && (SubIndex < 6));
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));

	if (SubIndex == 0)
	{
		return FColor(255,0,0);
	}
	else 
	if (SubIndex == 1)
	{
		return FColor(128,0,0);
	}
	else
	if (SubIndex == 2)
	{
		return FColor(0,255,0);
	}
	else
	if (SubIndex == 3)
	{
		return FColor(0,128,0);
	}
	else
	if (SubIndex == 4)
	{
		return FColor(0,0,255);
	}
	else
	{
		return FColor(0,0,128);
	}
}

void UDistributionVectorUniformCurve::GetInRange(FLOAT& MinIn, FLOAT& MaxIn)
{
	if (ConstantCurve.Points.Num() == 0)
	{
		MinIn = 0.f;
		MaxIn = 0.f;
	}
	else
	{
		FLOAT Min = BIG_NUMBER;
		FLOAT Max = -BIG_NUMBER;
		for (INT Index = 0; Index < ConstantCurve.Points.Num(); Index++)
		{
			FLOAT Value = ConstantCurve.Points(Index).InVal;
			if (Value < Min)
			{
				Min = Value;
			}
			if (Value > Max)
			{
				Max = Value;
			}
		}
		MinIn = Min;
		MaxIn = Max;
	}
}

void UDistributionVectorUniformCurve::GetOutRange(FLOAT& MinOut, FLOAT& MaxOut)
{
	FTwoVectors	MinVec, MaxVec;

	ConstantCurve.CalcBounds(MinVec, MaxVec, FTwoVectors());
	LockAndMirror(MinVec);
	LockAndMirror(MaxVec);

	MinOut = ::Min<FLOAT>(MinVec.GetMin(), MaxVec.GetMin());
	MaxOut = ::Max<FLOAT>(MinVec.GetMax(), MaxVec.GetMax());
}

BYTE UDistributionVectorUniformCurve::GetKeyInterpMode(INT KeyIndex)
{
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));
	return ConstantCurve.Points(KeyIndex).InterpMode;
}

void UDistributionVectorUniformCurve::GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent)
{
	check((SubIndex >= 0) && (SubIndex < 6));
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));

	if (SubIndex == 0)
	{
		ArriveTangent = ConstantCurve.Points(KeyIndex).ArriveTangent.v1.X;
		LeaveTangent = ConstantCurve.Points(KeyIndex).LeaveTangent.v1.X;
	}
	else
	if (SubIndex == 1)
	{
		ArriveTangent = ConstantCurve.Points(KeyIndex).ArriveTangent.v2.X;
		LeaveTangent = ConstantCurve.Points(KeyIndex).LeaveTangent.v2.X;
	}
	else
	if (SubIndex == 2)
	{
		ArriveTangent = ConstantCurve.Points(KeyIndex).ArriveTangent.v1.Y;
		LeaveTangent = ConstantCurve.Points(KeyIndex).LeaveTangent.v1.Y;
	}
	else
	if (SubIndex == 3)
	{
		ArriveTangent = ConstantCurve.Points(KeyIndex).ArriveTangent.v2.Y;
		LeaveTangent = ConstantCurve.Points(KeyIndex).LeaveTangent.v2.Y;
	}
	else
	if (SubIndex == 4)
	{
		ArriveTangent = ConstantCurve.Points(KeyIndex).ArriveTangent.v1.Z;
		LeaveTangent = ConstantCurve.Points(KeyIndex).LeaveTangent.v1.Z;
	}
	else
	if (SubIndex == 5)
	{
		ArriveTangent = ConstantCurve.Points(KeyIndex).ArriveTangent.v2.Z;
		LeaveTangent = ConstantCurve.Points(KeyIndex).LeaveTangent.v2.Z;
	}
}

FLOAT UDistributionVectorUniformCurve::EvalSub(INT SubIndex, FLOAT InVal)
{
	check((SubIndex >= 0) && (SubIndex < 6));

	FTwoVectors Default;
	FTwoVectors OutVal = ConstantCurve.Eval(InVal, Default);
	LockAndMirror(OutVal);
	if ((SubIndex % 2) == 0)
	{
		return OutVal.v1[SubIndex / 2];
	}
	else
	{
		return OutVal.v2[SubIndex / 2];
	}
}

INT UDistributionVectorUniformCurve::CreateNewKey(FLOAT KeyIn)
{
	FTwoVectors NewKeyVal = ConstantCurve.Eval(KeyIn, FTwoVectors());
	INT NewPointIndex = ConstantCurve.AddPoint(KeyIn, NewKeyVal);
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;

	return NewPointIndex;
}

void UDistributionVectorUniformCurve::DeleteKey(INT KeyIndex)
{
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));
	ConstantCurve.Points.Remove(KeyIndex);
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;
}

INT UDistributionVectorUniformCurve::SetKeyIn(INT KeyIndex, FLOAT NewInVal)
{
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));
	INT NewPointIndex = ConstantCurve.MovePoint(KeyIndex, NewInVal);
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;

	return NewPointIndex;
}

void UDistributionVectorUniformCurve::SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal)
{
	check((SubIndex >= 0) && (SubIndex < 6));
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));

	FLOAT Value;

	FInterpCurvePoint<FTwoVectors>*	Point = &(ConstantCurve.Points(KeyIndex));
	check(Point);

	if (SubIndex == 0)
	{
		Value = ::Max<FLOAT>(NewOutVal, Point->OutVal.v2.X);
		Point->OutVal.v1.X = Value;
	}
	else
	if (SubIndex == 1)
	{
		Value = ::Min<FLOAT>(NewOutVal, Point->OutVal.v1.X);
		Point->OutVal.v2.X = Value;
	}
	else
	if (SubIndex == 2)
	{
		Value = ::Max<FLOAT>(NewOutVal, Point->OutVal.v2.Y);
		Point->OutVal.v1.Y = Value;
	}
	else
	if (SubIndex == 3)
	{
		Value = ::Min<FLOAT>(NewOutVal, Point->OutVal.v1.Y);
		Point->OutVal.v2.Y = Value;
	}
	else
	if (SubIndex == 4)
	{
		Value = ::Max<FLOAT>(NewOutVal, Point->OutVal.v2.Z);
		Point->OutVal.v1.Z = Value;
	}
	else
	if (SubIndex == 5)
	{
		Value = ::Min<FLOAT>(NewOutVal, Point->OutVal.v1.Z);
		Point->OutVal.v2.Z = Value;
	}

	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;
}

void UDistributionVectorUniformCurve::SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode)
{
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));

	ConstantCurve.Points(KeyIndex).InterpMode = NewMode;
	ConstantCurve.AutoSetTangents(0.f);

	bIsDirty = TRUE;
}

void UDistributionVectorUniformCurve::SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent)
{
	check((SubIndex >= 0) && (SubIndex < 6));
	check((KeyIndex >= 0) && (KeyIndex < ConstantCurve.Points.Num()));

	if (SubIndex == 0)
	{
		ConstantCurve.Points(KeyIndex).ArriveTangent.v1.X = ArriveTangent;
		ConstantCurve.Points(KeyIndex).LeaveTangent.v1.X = LeaveTangent;
	}
	else
	if (SubIndex == 1)
	{
		ConstantCurve.Points(KeyIndex).ArriveTangent.v2.X = ArriveTangent;
		ConstantCurve.Points(KeyIndex).LeaveTangent.v2.X = LeaveTangent;
	}
	else
	if (SubIndex == 2)
	{
		ConstantCurve.Points(KeyIndex).ArriveTangent.v1.Y = ArriveTangent;
		ConstantCurve.Points(KeyIndex).LeaveTangent.v1.Y = LeaveTangent;
	}
	else
	if (SubIndex == 3)
	{
		ConstantCurve.Points(KeyIndex).ArriveTangent.v2.Y = ArriveTangent;
		ConstantCurve.Points(KeyIndex).LeaveTangent.v2.Y = LeaveTangent;
	}
	else
	if (SubIndex == 4)
	{
		ConstantCurve.Points(KeyIndex).ArriveTangent.v1.Z = ArriveTangent;
		ConstantCurve.Points(KeyIndex).LeaveTangent.v1.Z = LeaveTangent;
	}
	else
	if (SubIndex == 5)
	{
		ConstantCurve.Points(KeyIndex).ArriveTangent.v2.Z = ArriveTangent;
		ConstantCurve.Points(KeyIndex).LeaveTangent.v2.Z = LeaveTangent;
	}

	bIsDirty = TRUE;
}


/** Returns TRUE if this curve uses legacy tangent/interp algorithms and may be 'upgraded' */
UBOOL UDistributionVectorUniformCurve::UsingLegacyInterpMethod() const
{
	return ConstantCurve.UsingLegacyInterpMethod();
}


/** 'Upgrades' this curve to use the latest tangent/interp algorithms (usually, will 'bake' key tangents.) */
void UDistributionVectorUniformCurve::UpgradeInterpMethod()
{
	if( UsingLegacyInterpMethod() )
	{
		ConstantCurve.UpgradeInterpMethod();

		bIsDirty = TRUE;
	}
}


void UDistributionVectorUniformCurve::LockAndMirror(FTwoVectors& Val)
{
	// Handle the mirror flags...
	for (INT i = 0; i < 3; i++)
	{
		switch (MirrorFlags[i])
		{
		case EDVMF_Same:		Val.v2[i]	=  Val.v1[i];	break;
		case EDVMF_Mirror:		Val.v2[i]	= -Val.v1[i];	break;
		}
	}

	// Handle the lock axes flags
	switch (LockedAxes[0])
	{
	case EDVLF_XY:
		Val.v1.Y	= Val.v1.X;
		break;
	case EDVLF_XZ:
		Val.v1.Z	= Val.v1.X;
		break;
	case EDVLF_YZ:
		Val.v1.Z	= Val.v1.Y;
		break;
	case EDVLF_XYZ:
		Val.v1.Y	= Val.v1.X;
		Val.v1.Z	= Val.v1.X;
		break;
	}

	switch (LockedAxes[0])
	{
	case EDVLF_XY:
		Val.v2.Y	= Val.v2.X;
		break;
	case EDVLF_XZ:
		Val.v2.Z	= Val.v2.X;
		break;
	case EDVLF_YZ:
		Val.v2.Z	= Val.v2.Y;
		break;
	case EDVLF_XYZ:
		Val.v2.Y	= Val.v2.X;
		Val.v2.Z	= Val.v2.X;
		break;
	}
}

// DistributionVector interface
/** GetRange - in the case of a constant curve, this will not be exact!				*/
void UDistributionVectorUniformCurve::GetRange(FVector& OutMin, FVector& OutMax)
{
	FTwoVectors	MinVec, MaxVec;

	ConstantCurve.CalcBounds(MinVec, MaxVec, FTwoVectors());
	LockAndMirror(MinVec);
	LockAndMirror(MaxVec);

	if (MinVec.v1.X < MaxVec.v1.X)	OutMin.X = MinVec.v1.X;
	else							OutMin.X = MaxVec.v1.X;
	if (MinVec.v1.Y < MaxVec.v1.Y)	OutMin.Y = MinVec.v1.Y;
	else							OutMin.Y = MaxVec.v1.Y;
	if (MinVec.v1.Z < MaxVec.v1.Z)	OutMin.Z = MinVec.v1.Z;
	else							OutMin.Z = MaxVec.v1.Z;

	if (MinVec.v2.X > MaxVec.v2.X)	OutMax.X = MinVec.v2.X;
	else							OutMax.X = MaxVec.v2.X;
	if (MinVec.v2.Y > MaxVec.v2.Y)	OutMax.Y = MinVec.v2.Y;
	else							OutMax.Y = MaxVec.v2.Y;
	if (MinVec.v2.Z > MaxVec.v2.Z)	OutMax.Z = MinVec.v2.Z;
	else							OutMax.Z = MaxVec.v2.Z;
}

//
//	UDistributionFloatParameterBase
//
FLOAT UDistributionFloatParameterBase::GetValue( FLOAT F, UObject* Data )
{
	FLOAT ParamFloat = 0.f;
	UBOOL bFoundParam = GetParamValue(Data, ParameterName, ParamFloat);
	if(!bFoundParam)
	{
		ParamFloat = Constant;
	}

	if(ParamMode == DPM_Direct)
	{
		return ParamFloat;
	}
	else if(ParamMode == DPM_Abs)
	{
		ParamFloat = Abs(ParamFloat);
	}

	FLOAT Gradient;
	if(MaxInput <= MinInput)
		Gradient = 0.f;
	else
		Gradient = (MaxOutput - MinOutput)/(MaxInput - MinInput);

	FLOAT ClampedParam = ::Clamp(ParamFloat, MinInput, MaxInput);
	FLOAT Output = MinOutput + ((ClampedParam - MinInput) * Gradient);

	return Output;
}

FVector UDistributionVectorParameterBase::GetValue( FLOAT F, UObject* Data, INT Extreme )
{
	FVector ParamVector(0.f);
	UBOOL bFoundParam = GetParamValue(Data, ParameterName, ParamVector);
	if(!bFoundParam)
	{
		ParamVector = Constant;
	}

	if(ParamModes[0] == DPM_Abs)
	{
		ParamVector.X = Abs(ParamVector.X);
	}

	if(ParamModes[1] == DPM_Abs)
	{
		ParamVector.Y = Abs(ParamVector.Y);
	}

	if(ParamModes[2] == DPM_Abs)
	{
		ParamVector.Z = Abs(ParamVector.Z);
	}

	FVector Gradient;
	if(MaxInput.X <= MinInput.X)
		Gradient.X = 0.f;
	else
		Gradient.X = (MaxOutput.X - MinOutput.X)/(MaxInput.X - MinInput.X);

	if(MaxInput.Y <= MinInput.Y)
		Gradient.Y = 0.f;
	else
		Gradient.Y = (MaxOutput.Y - MinOutput.Y)/(MaxInput.Y - MinInput.Y);

	if(MaxInput.Z <= MinInput.Z)
		Gradient.Z = 0.f;
	else
		Gradient.Z = (MaxOutput.Z - MinOutput.Z)/(MaxInput.Z - MinInput.Z);

	FVector ClampedParam;
	ClampedParam.X = ::Clamp(ParamVector.X, MinInput.X, MaxInput.X);
	ClampedParam.Y = ::Clamp(ParamVector.Y, MinInput.Y, MaxInput.Y);
	ClampedParam.Z = ::Clamp(ParamVector.Z, MinInput.Z, MaxInput.Z);

	FVector Output = MinOutput + ((ClampedParam - MinInput) * Gradient);

	if(ParamModes[0] == DPM_Direct)
	{
		Output.X = ParamVector.X;
	}

	if(ParamModes[1] == DPM_Direct)
	{
		Output.Y = ParamVector.Y;
	}

	if(ParamModes[2] == DPM_Direct)
	{
		Output.Z = ParamVector.Z;
	}

	return Output;
}

