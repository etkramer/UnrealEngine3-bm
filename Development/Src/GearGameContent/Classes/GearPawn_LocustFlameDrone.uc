/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustFlameDrone extends GearPawn_LocustHunterArmorNoGrenades
	config(Pawn)
	implements(AIObjectAvoidanceInterface);

var repnotify GDO_FlameDroneTank FlameTanks;

/** General material used to control common pawn material parameters (e.g. burning) */
var protected transient MaterialInstanceConstant MIC_FlameTanks;


replication
{
	if(ROLE==ROLE_Authority)
		FlameTanks;
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'FlameTanks')
	{
		if(FlameTanks != none)
		{
			FlameTanks.SetHardAttach(true);
			FlameTanks.SetBase(self,,Mesh,'Flametanks');
			FlameTanks.FlameTankMesh.SetLightEnvironment( LightEnvironment );
			FlameTanks.FlameTankMesh.SetShadowParent( Mesh );

			if( MIC_FlameTanks == none )
			{
				MIC_FlameTanks = FlameTanks.FlameTankMesh.CreateAndSetMaterialInstanceConstant(0);
			}
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}
/**
---> AIobjectAvoidanceInterface
**/
function bool ShouldAvoid(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	if(FastTrace(Location, AskingAI.Pawn.Location,,true))
	{
		return true;
	}

	return false;
}
function bool ShouldEvade(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	// evade if it's really close
	if(VSizeSq(AskingAI.Pawn.Location - Location) < 300.f*300.f)
	{
		return true;
	}
	return false;
}
function bool ShouldRoadieRun(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	return true;
}


simulated event Destroyed()
{
	Super.Destroyed();
	if(FlameTanks != none)
	{
		FlameTanks.Destroy();
	}
}

simulated function SetupHelmet()
{
	Super.SetupHelmet();
}


/** This will create the MIC for this pawn's materials **/
simulated protected function InitMICPawnMaterial()
{
	Super.InitMICPawnMaterial();

	if(ROLE == ROLE_Authority)
	{
		// Attach flame tanks
		FlameTanks = Spawn(class'GDO_FlameDroneTank',self,,Location,,,true);
		FlameTanks.Init(self);
		FlameTanks.SetHardAttach(true);
		FlameTanks.SetBase(self,,Mesh,'Flametanks');
		FlameTanks.FlameTankMesh.SetLightEnvironment( LightEnvironment );
		FlameTanks.FlameTankMesh.SetShadowParent( Mesh );
	}

	MIC_FlameTanks = FlameTanks.FlameTankMesh.CreateAndSetMaterialInstanceConstant(0);
}


/** Heats up the skin by the given amount.  Skin heat constantly diminishes in Tick(). */
simulated function HeatSkin(float HeatIncrement)
{
	Super.HeatSkin( HeatIncrement );

	MIC_FlameTanks.SetScalarParameterValue('Heat', CurrentSkinHeat);
}

/** Char skin to the given absolute char amount [0..1].  Charring does not diminish over time. */
simulated function CharSkin(float CharIncrement)
{
	Super.CharSkin( CharIncrement );

	MIC_FlameTanks.SetScalarParameterValue('Burn', CurrentSkinChar);
}


function RemoveAndDropAttachments( Vector ApplyImpulse, class<DamageType> DamageType )
{
	if( FlameTanks != None )
	{
		FlameTanks.SetBase( None );
		FlameTanks.SetHidden( TRUE );
		FlameTanks.Destroy();
	}
}



defaultproperties
{
	HelmetType=class'Item_Helmet_LocustDroneFlame'
	ShoulderPadLeftType=None
	ShoulderPadRightType=None
	ControllerClass=class'GearAI_FlameDrone'

	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGameContent.GearWeap_FlameThrower'

	NoticedGUDSEvent=GUDEvent_NoticedDrone
}
