/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class DistributionFloatUniform extends DistributionFloat
	native
	collapsecategories
	hidecategories(Object)
	editinlinenew;
	
/** Low end of output float distribution. */
var()	float	Min;

/** High end of output float distribution. */
var()	float	Max;
	
cpptext
{
	virtual void PostLoad();

	virtual FLOAT GetValue( FLOAT F = 0.f, UObject* Data = NULL );
#if !CONSOLE
	/**
	 * Return the operation used at runtime to calculate the final value
	 */
	virtual ERawDistributionOperation GetOperation();
	
	/**
	 * Fill out an array of floats and return the number of elements in the entry
	 *
	 * @param Time The time to evaluate the distribution
	 * @param Values An array of values to be filled out, guaranteed to be big enough for 4 floats
	 * @return The number of elements (values) set in the array
	 */
	virtual DWORD InitializeRawEntry(FLOAT Time, FLOAT* Values);
#endif

	// FCurveEdInterface interface
	virtual INT		GetNumKeys();
	virtual INT		GetNumSubCurves();
	virtual FLOAT	GetKeyIn(INT KeyIndex);
	virtual FLOAT	GetKeyOut(INT SubIndex, INT KeyIndex);
	virtual void	GetInRange(FLOAT& MinIn, FLOAT& MaxIn);
	virtual void	GetOutRange(FLOAT& MinOut, FLOAT& MaxOut);
	virtual BYTE	GetKeyInterpMode(INT KeyIndex);
	virtual void	GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent);
	virtual FLOAT	EvalSub(INT SubIndex, FLOAT InVal);

	virtual INT		CreateNewKey(FLOAT KeyIn);
	virtual void	DeleteKey(INT KeyIndex);

	virtual INT		SetKeyIn(INT KeyIndex, FLOAT NewInVal);
	virtual void	SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal);
	virtual void	SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode);
	virtual void	SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent);
}

