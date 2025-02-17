/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTWeaponPawn extends UTVehicleBase
	native(Vehicle)
	nativereplication
	notplaceable;

/** MyVehicleWeapon points to the weapon assoicated with this WeaponPawn and is replcated */
var repnotify UTVehicleWeapon MyVehicleWeapon;

/** MyVehicle points to the vehicle that houses this WeaponPawn and is replicated */
var repnotify UTVehicle MyVehicle;

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
 *   Statistics gathering
 */
function name GetVehicleDrivingStatName()
{
	local name VehicleStatName;

	VehicleStatName = name('DRIVING_'$MyVehicle.Class.name);
	return VehicleStatName;
}

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
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/**
 * Caculates the Camera position.  WeaponPawn's forward this to their vehicle
 *
 * @param	fDeltatime		How long since the last calculation
 * @param	out_CamLoc		The final camera location (out)
 * @param	out_CamRot		The final camera rotation (out)
 * @param	out_FOV			The final FOV to use (out)
 */

simulated function bool CalcCamera(float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV)
{
    local vector out_CamStart;

	if (MyVehicle != None && MySeatIndex > 0 && MySeatIndex < MyVehicle.Seats.length)
	{
		MyVehicle.VehicleCalcCamera(fDeltaTime, MySeatIndex, out_CamLoc, out_CamRot, out_CamStart);
		return true;
	}
	else
	{
		return Super.CalcCamera(fDeltaTime, out_CamLoc, out_CamRot, out_FOV);
	}
}

`if(`notdefined(ShippingPC))
exec function FixedView(string VisibleMeshes)
{
	local UTPawn P;

	// pass onto driver utpawn
	P = UTPawn(Driver);
	if (P != None)
	{
		P.FixedView(VisibleMeshes);
	}
}
`endif

simulated function ProcessViewRotation(float DeltaTime, out rotator out_ViewRotation, out rotator out_DeltaRot)
{
	Super.ProcessViewRotation(DeltaTime, out_ViewRotation, out_DeltaRot);
}

simulated function SetFiringMode(byte FiringModeNum)
{
	if (MyVehicle != None && MySeatIndex > 0 && MySeatIndex < MyVehicle.Seats.length)
	{
		MyVehicle.SeatFiringMode(MySeatIndex, FiringModeNum, false);
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
		MyVehicle.ChangeSeat(Controller, RequestedSeat);
}

/**
 * moves the camera in or out
 *
 * @Param	bIn		If true, we should zoom the camera in
 *
 */
simulated function AdjustCameraScale(bool bIn)
{
	if (MyVehicle != None)
	{
		MyVehicle.AdjustCameraScale(bIn);
	}
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
	if ( (MyVehicle != None) && MyVehicle.bAcceptTurretJump )
	{
		return MyVehicle.DoJump(bUpdating);
	}
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
	local UTPawn UTP;

	UTP = UTPawn(P);
	if (UTP != None)
	{
		UTP.SetWeaponAttachmentVisibility(false);
		if (MyVehicle.bAttachDriver)
		{
			UTP.SetCollision(false, false);
			UTP.bCollideWorld = false;
			UTP.SetHardAttach(true);
			UTP.SetLocation(Location);
			UTP.SetPhysics(PHYS_None);

			MyVehicle.SitDriver(UTP, MySeatIndex);
		}
	}
}

simulated event HoldGameObject(UTCarriedObject GameObj)
{
	if (MyVehicle != None)
	{
		MyVehicle.HoldGameObject(GameObj);
	}
}

simulated function FaceRotation(rotator NewRotation, float DeltaTime)
{
	SetRotation(NewRotation);
}

function bool DriverLeave(bool bForceLeave)
{
	local UTBot B;

	B = UTBot(Controller);
	if (Super.DriverLeave(bForceLeave))
	{
		// bot might have gotten in because of temporary orders through UTVehicle::PlayHorn(), so clear that now
		if (B != None)
		{
			B.ClearTemporaryOrders();
		}

		return true;
	}
	else
	{
		return false;
	}
}

function bool Died(Controller Killer, class<DamageType> DamageType, vector HitLocation)
{
	local PlayerController OldPC;

	OldPC = PlayerController(Controller);
	if (Super.Died(Killer, DamageType, HitLocation))
	{
		GotoState('Dying'); // so Controller will be disconnected, etc
		HandleDeadVehicleDriver();
		// have player view original vehicle instead
		if (OldPC != None && MyVehicle != None)
		{
			OldPC.SetViewTarget(MyVehicle);
		}
		Destroy();
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Pass HUD Rendering to the Vehicle
 */
function DisplayHud(UTHud Hud, Canvas Canvas, vector2D HudPOS, optional int SIndex)
{
	if ( MyVehicle != none )
	{
		MyVehicle.DisplayHud(Hud, Canvas, HUDPOS, MySeatIndex);
	}
}

simulated function GetQuickPickCells(UTHud Hud, out array<QuickPickCell> Cells, out int CurrentWeaponIndex)
{
	if ( MyVehicle != none )
	{
		MyVehicle.GetQuickPickCells(HUD, Cells, CurrentWeaponIndex);
	}
}

simulated function QuickPick(int Quad)
{
	if ( MyVehicle != none )
	{
		MyVehicle.QuickPick(Quad);
	}
}

simulated function ApplyWeaponEffects(int OverlayFlags, optional int SeatIndex)
{
	if (MyVehicle != None)
	{
		MyVehicle.ApplyWeaponEffects(OverlayFlags, MySeatIndex);
	}
}

`if(`notdefined(ShippingPC))
exec function EditUDmgFX(optional int Index)
{
	if (WorldInfo.NetMode == NM_Standalone)
	{
		class<UTInventory>(DynamicLoadObject("UTGameContent.UTUDamage", class'Class')).static.AddWeaponOverlay(UTGameReplicationInfo(WorldInfo.GRI));
		MyVehicle.ApplyWeaponEffects(1, MySeatIndex);
		ConsoleCommand("EditObject name=" $ string(MyVehicle) $ "_WeaponEffect_" $ Index);
	}
}
`endif

defaultproperties
{
	//@note: even though UTWeaponPawns don't usually have visible components, they must have bHidden=false so AI can see them

	Physics=PHYS_None
	bProjTarget=false
	InventoryManagerClass=class'UTInventoryManager'
	bOnlyRelevantToOwner=true

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
