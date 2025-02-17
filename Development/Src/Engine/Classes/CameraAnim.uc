
/**
 *	CameraAnim: defines a pre-packaged animation to be played on a camera.
 * 	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class CameraAnim extends Object
	notplaceable
	native;

/** The InterpGroup that holds our actual interpolation data. */
var InterpGroup		CameraInterpGroup;

/** Length, in seconds. */
var const float		AnimLength;

/** AABB in local space. */
var const box		BoundingBox;


cpptext
{
protected:
	void CalcLocalAABB();

public:
	/** Overridden to calculate the bbox at save time. */
	virtual void PreSave();
	virtual void PostLoad();

	UBOOL CreateFromInterpGroup(class UInterpGroup* SrcGroup, class USeqAct_Interp* Interp);
	FBox GetAABB(FVector const& BaseLoc, FRotator const& BaseRot, FLOAT Scale) const;
};

defaultproperties
{
	AnimLength=3.f
}
