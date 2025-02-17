/**
 * SeqAct_WaitForLevelsVisible
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SeqAct_ForceMaterialMipsResident extends SeqAct_Latent
	native(Sequence);

cpptext
{
	virtual void Activated();
	virtual UBOOL UpdateOp(FLOAT DeltaTime);

	/** Apply the specified settings to the materials. */
	void ApplySettings( UBOOL bEnable, FLOAT Duration );
};

/** Time to enforce the bForceMiplevelsToBeResident for. 0.0 means infinite and you HAVE to make sure you turn this action off properly! */
var()	float						ForceDuration;

/** Array of Materials to set bForceMiplevelsToBeResident on their textures for the duration of this action. */
var()	array<MaterialInterface>	ForceMaterials;

/** Time left before we reset the flag. */
var		float						RemainingTime;

static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 1;
}

defaultproperties
{
	ObjName="Force Material Mips Resident"
	ObjCategory="Actor"

	ForceDuration = 1.f

	InputLinks(0)=(LinkDesc="Start")
	InputLinks(1)=(LinkDesc="Stop")

	VariableLinks.Empty
}
