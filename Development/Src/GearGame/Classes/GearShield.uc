/**
 * Base class for carryable shields.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearShield extends Inventory
	native(Weapon)
	config(Weapon)
	abstract;


/** Constant weapon ID, mainly used for FX lookup */
var const EWeaponClass WeaponID;

/** The skeletal mesh. */
var() const SkeletalMeshComponent		Mesh;
/** mesh used when deployed */
var SkeletalMeshComponent	DeployedMesh;

var() protected const float DropAngularVelocity;

var CanvasIcon				PickupIcon;

/** Name of the expand animation. */
var() Name					ExpandAnimName;
/** Name of the retract animation. */
var() Name					RetractAnimName;

/** True if the shield is in it's expanded state, false if retracted. */
var transient bool			bExpanded;

var protected const SoundCue	ShieldExpandSound;
var protected const SoundCue	ShieldRetractSound;
var protected const SoundCue	ShieldStickInGroundSound;
/** @fixme, needs hooked in*/
var protected const SoundCue	ShieldRemoveFromGroundSound;

var const SoundCue				ShieldDropSound;



/**
  * This is the MIC which will be the weapons's "material" once it is spawned.	We will then
  * be able to modify the colors on the fly!
 **/
var MaterialInstanceConstant MIC_WeaponSkin;

/**
 * This weapon's default emissive color.  The issue is that we can not have a struct of structs
 * and have the composed struct get the correct defaultproperties.	So we need to have some pain
 * here where we check for the uninited LinearColor and then init it the real defaults.
 * Find RonP or MartinS for the details on this.  We are going to fix thise post Gears as it
 * is a behavior affecting change.
**/
var LinearColor LC_EmisDefaultCOG;
var LinearColor LC_EmisDefaultLocust;


function DropFrom(vector StartLocation, vector StartVelocity)
{
	local GearDroppedPickup DP;

	if( DroppedPickupMesh != None && SkeletalMeshComponent(DroppedPickupMesh).PhysicsAsset == None )
	{
		`log( GetFuncName() @ Self @ "has no physics asset! doing a non physics drop...");
		Super.DropFrom(StartLocation, StartVelocity);
		return;
	}

	if( Instigator != None && Instigator.InvManager != None )
	{
		Instigator.InvManager.RemoveFromInventory(self);
	}

	// if cannot spawn a pickup, then destroy and quit
	if( DroppedPickupClass == None || DroppedPickupMesh == None )
	{
		Destroy();
		return;
	}

	DP = GearDroppedPickup(Spawn(DroppedPickupClass,,, StartLocation));
	if( DP == None )
	{
		// failed spawning the pickup, just destroy and quit
		Destroy();
		return;
	}

	DP.Inventory	= self;
	DP.InventoryClass = class;
	DP.Instigator = Instigator;
	DP.SetPickupMesh(DroppedPickupMesh);
	DP.CollisionComponent.SetRBAngularVelocity(VRand() * RandRange(-DropAngularVelocity,DropAngularVelocity),TRUE);
	DP.CollisionComponent.SetRBLinearVelocity(StartVelocity);
	DP.SetEmissiveColor( GearPawn(Instigator).GetMPWeaponColorBasedOnClass() );

	// align pickup mesh with current real mesh pos, for continuity
	DP.CollisionComponent.SetRBPosition(Mesh.GetPosition());
	DP.CollisionComponent.SetRBRotation(Mesh.GetRotation());

	if (bExpanded)
	{
		// @fixme, doesn't work due to physics asset reasons.
		RetractShieldMesh(SkeletalMeshComponent(DP.CollisionComponent));
	}

	Instigator = None;
}

function GivenTo(Pawn NewOwner, optional bool bDoNotActivate)
{
	local GearPawn P;

	Super.GivenTo(NewOwner, bDoNotActivate);

	P = GearPawn(NewOwner);
	if (P != None)
	{
		P.bCanMantle = false;
	}
}

reliable client function ClientGivenTo(Pawn NewOwner, bool bDoNotActivate)
{
	local GearPawn GP;

	Super.ClientGivenTo(NewOwner, bDoNotActivate);

	GP = GearPawn(NewOwner);
	if (GP != None)
	{
		GP.EquipShield(self);
	}
}

function ItemRemovedFromInvManager()
{
	local GearPawn P;

	P = GearPawn(Owner);
	if (P != None)
	{
		P.bCanMantle = P.default.bCanMantle;
	}
}

simulated final function bool CanDeployShield(optional out vector DropLoc)
{
	local Pawn P;
	local vector HitLocation, HitNormal, AdjustedDropLoc, Extent;

	DropLoc = Instigator.Location + vector(Instigator.Rotation) * 128.0;
	if (FastTrace(DropLoc - vect(0, 0, 200.0), DropLoc, vect(1, 1, 1)))
	{
		// can't drop here - would be floating
		return false;
	}
	else if (!FastTrace(DropLoc - vect(0,0,0.5) * Instigator.GetCollisionHeight(), Instigator.Location - vect(0,0,0.5) * Instigator.GetCollisionHeight()))
	{
		// can't drop here - wall in between
		return false;
	}
	else if (InterpActor(Instigator.Trace(HitLocation, HitNormal, DropLoc - vect(0, 0, 200.0), DropLoc, true, vect(1, 1, 1))) != None)
	{
		// can't plant shield on a mover
		return false;
	}
	else
	{
		AdjustedDropLoc = DropLoc;
		Extent.X = CylinderComponent(DroppedPickupClass.default.CollisionComponent).CollisionRadius;
		Extent.Y = Extent.X;
		Extent.Z = CylinderComponent(DroppedPickupClass.default.CollisionComponent).CollisionHeight;
		if (!FindSpot(Extent, AdjustedDropLoc) || AdjustedDropLoc != DropLoc)
		{
			return false;
		}
		else
		{
			foreach CollidingActors(class'Pawn', P, 32.0, DropLoc, true)
			{
				if (P != Instigator)
				{
					return false;
				}
			}

			return true;
		}
	}
}

function GearDroppedPickup_Shield Deploy()
{
	local GearDroppedPickup_Shield DP;
	local GearPawn GP;
	local vector DropLoc;

	if (!CanDeployShield(DropLoc))
	{
		return None;
	}

	GP = GearPawn(Instigator);
	if (GP != None)
	{
		// create a pickup
		DP = GearDroppedPickup_Shield(Spawn(DroppedPickupClass,,, DropLoc));
		if( DP == None )
		{
			return None;
		}

		// delete from inventory
		if (Instigator.InvManager != None)
		{
			Instigator.InvManager.RemoveFromInventory(self);
		}

		ShieldPlaySound(ShieldStickInGroundSound);

		// init params
		DP.Inventory = self;
		DP.InventoryClass = class;
		DP.Instigator = Instigator;

		// tell it to create cover
		DP.Deploy(GP);
		GP.AttachClass_Shield = None;
		Instigator = None;

		SetMaterialBasedOnTeam( DP.ShieldMeshComp, GP.GetMPWeaponColorBasedOnClass() );

		// and return it so the server can acquire the cover
		return DP;
	}
	else
	{
		return None;
	}
}

simulated protected function ShieldPlaySound(SoundCue Cue)
{
	if (Owner != None)
	{
		Owner.PlaySound(Cue, TRUE);
	}
}

/** Causes the shield to expand. */
simulated function Expand()
{
	ExpandShieldMesh(Mesh);
	ShieldPlaySound(ShieldExpandSound);
	bExpanded = TRUE;
}

/** Causes the shield to retract. */
simulated function Retract()
{
	RetractShieldMesh(Mesh);
	ShieldPlaySound(ShieldRetractSound);
	bExpanded = FALSE;
}


/** Does the expand on any generic shield mesh.  Used by the dropped pickup as well. */
static function ExpandShieldMesh(SkeletalMeshComponent SkelMesh)
{
	PlayShieldAnim(SkelMesh, default.ExpandAnimName);
	//@todo, play some audio, attached to the mesh
}

/** Does the retract on any generic shield mesh.  Used by the dropped pickup as well. */
static function RetractShieldMesh(SkeletalMeshComponent SkelMesh)
{
	PlayShieldAnim(SkelMesh, default.RetractAnimName);
	//@todo, play some audio, attached to the mesh
}


static protected function PlayShieldAnim(SkeletalMeshComponent SkelMesh, name AnimName, optional float Rate=1.f, optional bool bLooping=FALSE)
{
	local AnimNodeSequence AnimNode;

	AnimNode = AnimNodeSequence(SkelMesh.FindAnimNode('CustomAnimNode'));
		//AnimNodeSequence(SkelMesh.Animations);

	// Check if we can play animation
	if( AnimNode == None )
	{
		`log("GearShield::PlayShieldAnim - no animations :(");
		return;
	}

	// Set right animation sequence if needed
	if( AnimNode.AnimSeq == None || AnimNode.AnimSeq.SequenceName != AnimName )
	{
		AnimNode.SetAnim(AnimName);
	}

	if( AnimNode.AnimSeq == None )
	{
		`log("GearShield::PlayShieldAnim - AnimSeq == None" @ SkelMesh @ AnimName );
		return;
	}

	// Play Animation
	AnimNode.PlayAnim(bLooping, Rate);
}



//@see GearWeapon for duplicated version of this as it has a different hierarchy
simulated function SetMaterialBasedOnTeam( MeshComponent TheMesh, int TeamNum )
{
	local LinearColor LC;

	//`log( "SetMaterialBasedOnTeam" );
	if( MIC_WeaponSkin == None )
	{
		MIC_WeaponSkin = new(outer) class'MaterialInstanceConstant';
		MIC_WeaponSkin.SetParent( TheMesh.GetMaterial( 0 ) );
		TheMesh.SetMaterial( 0, MIC_WeaponSkin );
	}

	TeamNum = GearPawn(Instigator).GetMPWeaponColorBasedOnClass();

	if( (TheMesh != None) && ( MIC_WeaponSkin != None ) )
	{
		if( TeamNum == 0 )
		{
			LC = class'GearWeapon'.static.GetWeaponEmisColor_COG( WorldInfo, default.WeaponID );
		}
		else
		{
			LC = class'GearWeapon'.static.GetWeaponEmisColor_Locust( WorldInfo, default.WeaponID );
		}

		//`log( AttachClass @ "SetAttachmentVisibility LC " $ LC.A @ LC.R @ LC.G @ LC.B );
		MIC_WeaponSkin.SetVectorParameterValue( 'Weap_EmisColor', LC );
	}
}

function CreateCheckpointRecord(out InventoryRecord Record)
{
	Record.InventoryClassPath = PathName(Class);
}

/** the MP bot AI uses this to determine if it should delay its current action and pick this up when it gets close
 * currently the return value is used as a bool so just return 0 or 1
 */
static function float DetourWeight(Pawn Other, float PathWeight)
{
	local GearAI_TDM AI;
	local GearWeapon WeapBelt;

	// we want it if we're being shot and have a pistol
	AI = GearAI_TDM(Other.Controller);
	if (AI.IsUnderHeavyFire())
	{
		WeapBelt = GearWeapon(GearInventoryManager(Other.InvManager).GetInventoryInSlot(EASlot_Holster));
		return (WeapBelt != None && WeapBelt.HasAnyAmmo()) ? 1 : 0;
	}
	else
	{
		return 0;
	}
}

defaultproperties
{
	WeaponID=WC_Shield
	RespawnTime=30.f

	ExpandAnimName=Open
	RetractAnimName=Close

	Begin Object Class=SkeletalMeshComponent Name=ShieldMesh0
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		CollideActors=TRUE
		BlockRigidBody=TRUE
		RBChannel=RBCC_Default
		RBCollideWithChannels=(Default=TRUE,BlockingVolume=TRUE,EffectPhysics=TRUE,GameplayPhysics=TRUE,Pawn=FALSE)
	End Object
	Mesh=ShieldMesh0
	PickupFactoryMesh=ShieldMesh0
	DroppedPickupMesh=ShieldMesh0

	Begin Object Class=SkeletalMeshComponent Name=ShieldMesh1
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		CollideActors=TRUE
		BlockActors=true
		BlockRigidBody=TRUE
		RBChannel=RBCC_Default
		RBCollideWithChannels=(Default=TRUE,BlockingVolume=TRUE,EffectPhysics=TRUE,GameplayPhysics=TRUE,Pawn=FALSE)
	End Object
	DeployedMesh=ShieldMesh1

	DroppedPickupClass=class'GearDroppedPickup_Shield'

	DropAngularVelocity=64.0f

	PickupIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=0,V=420,UL=86,VL=92)

	LC_EmisDefaultCOG=(R=3,G=4,B=8,A=1.0)
	LC_EmisDefaultLocust=(R=60.0,G=1.0,B=0.1,A=1.0)
}
