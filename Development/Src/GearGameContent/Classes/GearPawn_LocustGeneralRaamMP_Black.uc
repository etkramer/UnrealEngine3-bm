/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustGeneralRaamMP_Black extends GearPawn_LocustGeneralRaamMP
	config(Pawn);

/*
simulated function SetUpRimShader()
{
	MPRimShader.SetVectorParameterValue( 'RimColor', WorldInfo.Gears_CharactersLocust.Locust_Raam_Black.RimColor );
	MPRimShader.SetScalarParameterValue( 'RimDistance', WorldInfo.Gears_CharactersLocust.Locust_Raam_Black.RimDistance );
	MPRimShader.SetScalarParameterValue( 'EdgeWidth', WorldInfo.Gears_CharactersLocust.Locust_Raam_Black.EdgeWidth );
}
*/


defaultproperties
{
	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.LOC_Raam_Shader_Black_MP'
}