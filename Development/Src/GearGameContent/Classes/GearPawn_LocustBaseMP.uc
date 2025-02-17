/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_LocustBaseMP extends GearPawn_LocustBase
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


simulated function ChainSawGore()
{
	BreakConstraint( vect(100,0,0), vect(0,10,0), 'b_MF_Spine_03' );
	BreakConstraint( vect(0,100,0), vect(0,0,10), 'b_MF_UpperArm_R' );
}


simulated function float GetKnockdownZThreshold()
{
	return 512.f;
}

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

/** This will set the character specific rim shader values that have been set for the specific map **/
simulated protected function SetUpRimShader( out MaterialInstanceConstant TheMIC )
{
	// This will propagate down the inheritance chain to the appropriate instance shaders.
	TheMIC.SetVectorParameterValue( 'EmisMult', class'GearPerMapColorConfig'.default.DefaultRimShaderLocust.EmisMult );
	TheMIC.SetVectorParameterValue( 'TeamColor', class'GearPerMapColorConfig'.default.DefaultRimShaderLocust.TeamColor );
	TheMIC.SetVectorParameterValue( 'RimColor', class'GearPerMapColorConfig'.default.DefaultRimShaderLocust.RimColor );
}


/** MP helmets don't fly off**/
simulated function RemoveAndSpawnAHelmet( Vector ApplyImpulse, class<DamageType> DamageType, bool bForced );

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
	return 1;
}

defaultproperties
{
	RimColor=(R=4.0,G=0.0,B=0.0,A=0.0)
	RimDistance=1024.0f
	EdgeWidth=1.80f
}
