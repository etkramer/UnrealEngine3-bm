
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_HumanBaseMP extends GearPawn_COGGear
	config(Pawn);

/**
 * All of the MP characters are going to need a couple of things set for them.  For one thing, they’ll need to
 * have their MP shader set on them (list forthcoming) and we’ll have to figure out what values we want for each
 * MP map for the rimlight-at-distance.
 **/
var protected MaterialInstanceConstant RimShaderMaterialSpecific;

var() LinearColor RimColor;
var() float RimDistance; // – Default is 1024
var() float EdgeWidth; //– Default is 0.80

simulated protected function InitMPRimShader()
{
	MPRimShader = new(Outer) class'MaterialInstanceConstant';
	MPRimShader.SetParent( RimShaderMaterialSpecific );
	Mesh.SetMaterial( 0, MPRimShader );

	SetUpRimShader( MPRimShader );

	if( ( HeadSlotStaticMesh != None ) && ( bHasHelmetOn == TRUE ) )
	{
		MPRimShaderHelmet = HeadSlotStaticMesh.CreateAndSetMaterialInstanceConstant(0);
		SetUpRimShader( MPRimShaderHelmet );
	}
}

simulated function float GetKnockdownZThreshold()
{
	return 512.f;
}

/** This will set the character specific rim shader values that have been set for the specific map **/
simulated protected function SetUpRimShader( out MaterialInstanceConstant TheMIC )
{
	// This will propagate down the inheritance chain to the appropriate instance shaders.
	TheMIC.SetVectorParameterValue( 'EmisMult', class'GearPerMapColorConfig'.default.DefaultRimShaderCOG.EmisMult );
	TheMIC.SetVectorParameterValue( 'TeamColor', class'GearPerMapColorConfig'.default.DefaultRimShaderCOG.TeamColor );
	TheMIC.SetVectorParameterValue( 'RimColor', class'GearPerMapColorConfig'.default.DefaultRimShaderCOG.RimColor );
}

simulated function ChainSawGore()
{
	BreakConstraint( vect(100,0,0), vect(0,10,0), 'b_MF_Spine_03' );
	BreakConstraint( vect(0,100,0), vect(0,0,10), 'b_MF_UpperArm_R' );
}

/** AnimSets list updated, post process them */
simulated function AnimSetsListUpdated()
{
	local int	i;

	// In MP, we want each mesh to use the animation height, so human players have the same height.
	for(i=0; i<Mesh.AnimSets.Length; i++)
	{
		Mesh.AnimSets[i].bAnimRotationOnly = FALSE;
	}
}

simulated function byte GetMPWeaponColorBasedOnClass()
{
	return 0;
}

defaultproperties
{
	RimColor=(R=0.1,G=10.0,B=12.0,A=0.0)
	RimDistance=1024.0f
	EdgeWidth=1.80f
}



