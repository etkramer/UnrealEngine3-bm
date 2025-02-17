
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustSpeedy extends GearPawn_LocustDrone
	config(Pawn);

/** Replace Roadie w/ Charge for Speedy Locust */
simulated function CacheAnimNodes()
{
	local AnimNodeSequence SeqNode;

	foreach Mesh.AllAnimNodes(class'AnimNodeSequence', SeqNode)
	{
		if( SeqNode.AnimSeqName == 'AR_RoadieRun_Fwd' )
		{
			SeqNode.SetAnim('AR_Charge_Fwd');
		}
	}

	Super.CacheAnimNodes();
}

DefaultProperties
{
	ControllerClass=class'GearAI_Speedy'
	HelmetType=class'Item_Helmet_LocustSpeedy'
	ShoulderPadLeftType=class'Item_ShoulderPad_LocustDiamond'
	ShoulderPadRightType=class'Item_ShoulderPad_LocustDiamond'

	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGame.GearWeap_LocustPistol'
	TurningRadius=64.0f
}
