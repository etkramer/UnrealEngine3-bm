/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearWeaponPawn extends GearVehicleBase
	native(Vehicle)
	nativereplication
	notplaceable;

/** MyVehicleWeapon points to the weapon assoicated with this WeaponPawn and is replcated */
var repnotify GearVehicleWeapon MyVehicleWeapon;

/** MyVehicle points to the vehicle that houses this WeaponPawn and is replicated */
var repnotify GearVehicle MyVehicle;

/** An index in to the Seats array of the vehicle housing this WeaponPawn.  It is replicated */
var repnotify int MySeatIndex;

replication
{
	if (Role == ROLE_Authority)
		MySeatIndex, MyVehicle, MyVehicleWeapon;
}

cpptext
{
	virtual void TickSpecial( FLOAT DeltaSeconds );
	virtual AVehicle* GetVehicleBase();
	INT* GetOptimizedRepList(BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel);
}

simulated native function vector GetTargetLocation(optional Actor RequestedBy, optional bool bRequestAlternateLoc) const;


/**
* General Debug information
*/
simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	local Canvas Canvas;

	Canvas = HUD.Canvas;

	super.DisplayDebug(HUD, out_YL, out_YPos);

	Canvas.SetPos(4,out_YPos);
	Canvas.DrawText("[WeaponPawn]");
	out_YPos+=out_YL;
	Canvas.SetPos(4,out_YPos);
	Canvas.DrawText("Owner:"@Owner);
	out_YPos+=out_YL;
	Canvas.SetPos(4,out_YPos);
	Canvas.DrawText("Vehicle:"@MyVehicleWeapon@MyVehicle);
	out_YPos+=out_YL;
	Canvas.SetPos(4,out_YPos);
	Canvas.DrawText("Rotation/Location:"@Rotation@Location);

	out_YPos+=out_YL;

	if (MyVehicle!=none)
	{
		MyVehicle.DisplayDebug(HUD, out_YL, out_YPos);
	}
}

/**
* We use NetNotify to signal that critical data has been replicated.  when the Vehicle, Weapon and SeatIndex have
* all arrived, we setup ourself up locally.
*
* @param	VarName		Name of the variable replicated
*/
simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'MyVehicle' || VarName == 'MyVehicleWeapon' || VarName == 'MySeatIndex')
	{
		if (MySeatIndex > 0 && MyVehicle != None && MySeatIndex < MyVehicle.Seats.length)
		{
			MyVehicle.Seats[MySeatIndex].SeatPawn = self;
			MyVehicle.Seats[MySeatIndex].Gun = MyVehicleWeapon;
			SetBase(MyVehicle);

			//Setup the PP effects for this guy
			//valid here because only the WeaponPawnOwner will ever get this
			ClientSetCameraEffect(Controller, true);
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function ProcessViewRotation(float DeltaTime, out rotator out_ViewRotation, out rotator out_DeltaRot)
{
	local int i, MaxDelta;
	local float MaxDeltaDegrees;

	if (WorldInfo.bUseConsoleInput && MyVehicle != None)
	{
		// clamp player rotation to turret rotation speed
		for (i = 0; i < MyVehicle.Seats[MySeatIndex].TurretControllers.length; i++)
		{
			MaxDeltaDegrees = FMax(MaxDeltaDegrees, MyVehicle.Seats[MySeatIndex].TurretControllers[i].LagDegreesPerSecond);
		}
		if (MaxDeltaDegrees > 0.0)
		{
			MaxDelta = int(MaxDeltaDegrees * 182.0444 * DeltaTime);
			out_DeltaRot.Pitch = (out_DeltaRot.Pitch >= 0) ? Min(out_DeltaRot.Pitch, MaxDelta) : Max(out_DeltaRot.Pitch, -MaxDelta);
			out_DeltaRot.Yaw = (out_DeltaRot.Yaw >= 0) ? Min(out_DeltaRot.Yaw, MaxDelta) : Max(out_DeltaRot.Yaw, -MaxDelta);
			out_DeltaRot.Roll = (out_DeltaRot.Roll >= 0) ? Min(out_DeltaRot.Roll, MaxDelta) : Max(out_DeltaRot.Roll, -MaxDelta);
		}
	}
	Super.ProcessViewRotation(DeltaTime, out_ViewRotation, out_DeltaRot);
}

simulated function SetFiringMode(byte FiringModeNum)
{
	if (MyVehicle != None && MySeatIndex > 0 && MySeatIndex < MyVehicle.Seats.length)
	{
		MyVehicle.SeatFiringMode(MySeatIndex, FiringModeNum, false);
	}
	if (Weapon != None)
	{
		Weapon.FireModeUpdated(FiringModeNum, false);
	}
}

/**
* We need to pass IncreantFlashCount calls to the controlling vehicle
*/
simulated function IncrementFlashCount( Weapon Who, byte FireModeNum )
{
	if (MyVehicle==none)
	{
		return;
	}

	MyVehicle.IncrementFlashCount(Who, FireModeNum);
}

/**
* We need to pass ClearFlashCount calls to the controlling vehicle
*/

simulated function ClearFlashCount(Weapon Who)
{
	if (MyVehicle==none)
	{
		return;
	}

	MyVehicle.ClearFlashCount(Who);
}

/**
* We need to pass SetFlashLocation calls to the controlling vehicle
*/

function SetFlashLocation( Weapon Who, byte FireModeNum, vector NewLoc )
{
	if (MyVehicle==none)
	{
		return;
	}

	MyVehicle.SetFlashLocation( Who, FireModeNum, NewLoc);
}

/**
* We need to pass ClearFlashLocation calls to the controlling vehicle
*/

simulated function ClearFlashLocation(Weapon Who)
{
	if (MyVehicle==none)
	{
		return;
	}
	MyVehicle.ClearFlashLocation(Who);

}

/**
* Called when the controller takes possession of the WeaponPawn.  Upon Possession, make sure the weapon heads
* to the right state and set the eye height.
*
* @param	C					the controller taking posession
* @param	bVehicleTransition	Will be true if this the pawn is entering/leaving a vehicle
*/

function PossessedBy(Controller C, bool bVehicleTransition)
{
	super.PossessedBy(C,bVehicleTransition);
	MyVehicleWeapon.ClientWeaponSet(false);
	SetBaseEyeHeight();
	Eyeheight = BaseEyeheight;
}

reliable client function ClientSetCameraEffect(Controller C, bool bEnabled)
{
	//Setup the post process effect
	//(for when Pawn is already in Vehicle and Controller possesses some time later)
	if (Vehicle_Centaur_Base(MyVehicle) != None)
	{
		Vehicle_Centaur_Base(MyVehicle).ClientSetCameraEffect(C, bEnabled);
	}
}

simulated function ClientRestart()
{
	Super.ClientRestart();

	if (Role < ROLE_Authority && Weapon == None)
	{
		ServerRequestWeapon();
	}
}

/** client requests server to force activate the current weapon */
reliable server function ServerRequestWeapon()
{
	if (Weapon != None)
	{
		Weapon.ClientWeaponSet(false);
	}
}

/**
* Called when the driver leaves the WeaponPawn.  We forward it along as a PassengerLeave call to the controlling
* vehicle.
*/
function DriverLeft()
{
	Super.DriverLeft();
	MyVehicle.PassengerLeave(MySeatIndex);
}

/**
request change to adjacent vehicle seat
*/
reliable server function ServerAdjacentSeat(int Direction, Controller C)
{
	MyVehicle.ServerAdjacentSeat(Direction, C);
}

reliable server function ServerChangeSeat(int RequestedSeat)
{
	if (MyVehicle!=none)
	{
		MyVehicle.ChangeSeat(Controller, RequestedSeat);
	}
}

/** Called when a client presses X while in the vehicle. */
reliable server function ServerPressedX()
{
	MyVehicle.PassengerPressedX(Controller);
}

/** Called from client to move to driver seat */
reliable server function ServerBecomeDriver()
{
	MyVehicle.PassengerBecomeDriver(Controller);
}

reliable server function ServerSendTwoFloats(float A, float B)
{
	MyVehicle.PassengerSentTwoFloats(A, B);
}

/**
* Called when the vehicle needs to place an exiting driver.  Forward the call
* to the controlling vehicle
*
* @param	ExitingDriver		The pawn that is exiting
*/

function bool PlaceExitingDriver(optional Pawn ExitingDriver)
{
	if ( ExitingDriver == None )
		ExitingDriver = Driver;

	if (MyVehicle!=none)
	{
		return MyVehicle.PlaceExitingDriver(ExitingDriver);
	}

	return false;
}

function DropToGround() {}
function AddVelocity( vector NewVelocity, vector HitLocation, class<DamageType> damageType, optional TraceHitInfo HitInfo ) {}
function JumpOffPawn() {}
singular event BaseChange() {}
function SetMovementPhysics() {}

function bool DoJump( bool bUpdating )
{
	return false;
}

/**
* @Returns the collision radius.  In this case we return that of the vehicle if it exists
*/
simulated function float GetCollisionRadius()
{
	if (MyVehicle!=none)
		return MyVehicle.GetCollisionRadius();
	else
		return super.GetCollisionRadius();
}

/**
* Set the Base Eye height.  We override it here to pull it from the CameraEyeHeight variable of in the seat array
*/
simulated function SetBaseEyeheight()
{
	BaseEyeHeight = MyVehicle.Seats[MySeatIndex].CameraEyeHeight;
}

/**
* Attach the Driver to the vehicle.  If he's visible, find him a place, otherwhise hind him
*
* @param	P		the Pawn the attach
*/
simulated function AttachDriver( Pawn P )
{
	local GearPawn WP;

	if (MyVehicle == None)
	{
		GotoState('PendingAttachDriver');
		return;
	}

	if( !MyVehicle.bAttachDriver )
		return;

	WP = GearPawn(P);
	if (WP!=none)
	{

		WP.SetCollision( false, false);
		WP.bCollideWorld = false;
		WP.SetHardAttach(true);
		WP.SetLocation( Location );
		WP.SetPhysics( PHYS_None );
		WP.Mesh.SetBlockRigidBody(FALSE);
		WP.Mesh.SetHasPhysicsAssetInstance(FALSE);

		MyVehicle.SitDriver( WP, MySeatIndex);
	}
}

simulated function FaceRotation(rotator NewRotation, float DeltaTime)
{
	SetRotation(NewRotation);
}

simulated function DetachDriver( Pawn P )
{
    local GearPawn WP;

    Super.DetachDriver(P);

    WP = GearPawn(P);
	if (WP != none)
	{
	   WP.Mesh.SetBlockRigidBody(TRUE);
	   WP.Mesh.SetHasPhysicsAssetInstance(TRUE);
	}
}

simulated function vector GetCameraWorstCaseLoc()
{
	// icky, hardcoding seat 1
	return MyVehicle.GetCameraWorstCaseLoc(1);
}

/** return TRUE if player is targeting in this weaponpawn, false otherwise */
simulated function bool ShouldTargetingModeZoomCamera()
{
	local GearPC WPC;

	if (MyVehicle != None)
	{
		WPC = GearPC(MyVehicle.Seats[MySeatIndex].SeatPawn.Controller);
		if (WPC != None)
		{
			return (WPC.VehicleReversePressedAmount > 0.f);
		}
	}

	return FALSE;
}

function bool Died(Controller Killer, class<DamageType> DamageType, vector HitLocation)
{
	local GearPC OldPC;

	OldPC = GearPC(Controller);
	if (Super.Died(Killer, DamageType, HitLocation))
	{
		GotoState('Dying'); // so Controller will be disconnected, etc
		HandleDeadVehicleDriver();
		// have player view original vehicle instead
		if (OldPC != None && MyVehicle != None)
		{
			OldPC.SetViewTarget(MyVehicle);
			OldPC.LastPawn = MyVehicle;
		}
		Destroy();
		return true;
	}
	else
	{
		return false;
	}
}

function bool TooCloseToAttack(Actor Other)
{
	local int NeededPitch;

	if (MyVehicle == None || Weapon == None || VSize(MyVehicle.Location - Other.Location) > 2500.0)
	{
		return false;
	}

	NeededPitch = rotator(Other.GetTargetLocation(self) - Weapon.GetPhysicalFireStartLoc()).Pitch & 65535;
	return MyVehicle.CheckTurretPitchLimit(NeededPitch, MySeatIndex);
}

simulated function bool AllowTargetting()
{
	return (MyVehicle != None && MyVehicle.AllowTargetting());
}

defaultproperties
{
	bKillDuringLevelTransition=TRUE

	//@note: even though GearWeaponPawns don't usually have visible components, they must have bHidden=false so AI can see them

	Physics=PHYS_None
		bProjTarget=false
		InventoryManagerClass=class'GearInventoryManager'
		bOnlyRelevantToOwner=false //@HACK: should be true, workaround for RideReaver attachment issues on clients

		// No Collision
		bCollideActors=false
		bCollideWorld=false

		Begin Object Name=CollisionCylinder
		CollisionRadius=0
		CollisionHeight=0
		BlockNonZeroExtent=false
		BlockZeroExtent=false
		BlockActors=false
		CollideActors=false
		BlockRigidBody=false
		End Object

		BaseEyeheight=180
		Eyeheight=180

		bIgnoreBaseRotation=true
		bStationary=true
		bFollowLookDir=true
		bTurnInPlace=true

		MySeatIndex=INDEX_NONE
}
