/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Nemacyst extends GearAI
	native(AI);

/** GoW global macros */

var Pawn	LastEnemy;
var float	AlignToVelThreshold;
var bool	bRestoreAirSpeed;
var Rotator	SpinRot;
var float	SpinDir;
var bool	bLaunching;

var() config float HearingRadius;

var Volume_Nemacyst CurrentVolume;
/** Will find a nemacyst volume (if any) which our pawn is currently within */
final native function Volume_Nemacyst GetActiveNemacystVolume();

/** will attempt to find a new point which is within our current volume formation and is close to the direction we were already going */
native function bool SuggestNewMovePoint(out vector out_NewMovePt, vector TryThisDirFirst, optional float MoveDist=1024.f);

function Wander()
{
	`AILog("Wandering..");
	class'AICmd_Nemacyst_WanderWithinVolume'.static.InitCommand(self);
}

function HearNoise( float Loudness, Actor NoiseMaker, optional Name NoiseType );

function StopFiring();

function GearAINotifyTakeHit( Controller InstigatedBy, vector HitLocation, int Damage, class<DamageType> damageType, vector Momentum, TraceHitInfo HitInfo )
{
	HearNoise(1.f,InstigatedBy);
}

function DoLaunch()
{
	class'AICmd_Nemacyst_Launch'.static.Launch( self );
}

simulated function DrawDebug(GearHUD_Base HUD, Name Category)
{
	if(Category == 'Default')
	{
//		DrawDebugCoordinateSystem( Pawn.Location, Pawn.Rotation, 256 );

		DrawDebugLine(Pawn.Location,Pawn.Location + vector(Rotation) * 1024.f,0,255,0);
/*		if (Focus != None)
		{
			DrawDebugLine(Pawn.Location,Focus.Location,255,0,0);
		}
		DrawDebugLine(Pawn.Location,GetDestinationPosition(),128,0,128);*/
	}

	Super.DrawDebug(HUD,Category);
}

singular function bool StepAsideFor( Pawn ChkPawn )
{
	`if(`notdefined(FINAL_RELEASE))
		`log("Nemacyst stepping aside for someeone.. wtf?!?!");
		ScriptTrace();
	`endif
	return false;
}

function Tick(float DeltaTime)
{
	Super.Tick(DeltaTime);

	if( Pawn != None )
	{
		if( VSize(Pawn.Velocity) > AlignToVelThreshold )
		{
			SetFocalPoint( Pawn.Location + Normal(Pawn.Velocity) * 5000.f, TRUE );
		}
		else
		{
			SetFocalPoint( Pawn.Location + vect(0,0,5000.f), TRUE );
		}

		if( Pawn.bRollToDesired )
		{
			SpinRot.Roll += SpinDir * RotationRate.Roll * DeltaTime;

			Pawn.DesiredRotation = Rotator(GetFocalPoint() - Pawn.Location);
			Pawn.DesiredRotation.Roll = SpinRot.Roll;
		}

		// Restore default air speed smoothly
		if( bRestoreAirSpeed )
		{
			Pawn.AirSpeed = FInterpTo(Pawn.AirSpeed, Pawn.default.AirSpeed, DeltaTime, 4.f);
		}
	}
}

function FoundEnemyAndHomingIn(); // stub

function bool SetTether( Actor NewTetherActor, optional float NewTetherDistance, optional bool NewbDynamicTether, optional float NewTetherPersistDuration, optional bool bInteruptable, optional bool bIsValidCache )
{
	//debug
	`AILog( GetFuncName()@NewTetherActor@NewTetherDistance );

	TetherActor = NewTetherActor;
	return class'AICmd_Nemacyst_MoveToGoal'.static.InitCommandUserActor(self,NewTetherActor);
}

state Action_Idle
{
	function HearNoise( float Loudness, Actor NoiseMaker, optional Name NoiseType )
	{
		local Pawn	Heard;

		if( !bAllowCombatTransitions || bLaunching )
		{
			return;
		}

		if (Controller(NoiseMaker) != None)
		{
			Heard = Controller(NoiseMaker).Pawn;
		}
		else
		{
			Heard = Pawn(NoiseMaker);
		}

		//debug
		`AILog( "heard:"@Heard@VSize(Pawn.Location - Heard.Location)@HearingRadius@IsFriendlyPawn(Heard)@LineOfSightTo(Heard) );

		if( Heard != None &&
			Heard != Pawn &&
			VSize(Pawn.Location - Heard.Location) <= HearingRadius &&
			!IsFriendlyPawn(Heard) &&
			LineOfSightTo(Heard))
		{
			Enemy = Heard;

			// Home at enemy, but with a little randomization, so all the nemacysts aren't completely synchronized.
			SetTimer( FRand()*FRand(), FALSE, nameof(StartHomingAtEnemy) );
			//GotoState('Action_Homing');
		}
	}

	function bool ShouldHoverUp()
	{
		local vector HitL, HitN;
		if (Pawn.Trace(HitL,HitN,Pawn.Location - vect(0,0,1024.f),Pawn.Location) != None)
		{
			return (VSize(HitL - Pawn.Location) < 512.f);
		}
		return FALSE;
	}

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	if( ShouldHoverUp() )
	{
		//debug
		`AILog("Hovering up...");

		MoveTo(Pawn.Location + vect(0,0,256));

		Pawn.Acceleration = Vect(0,0,0);
		Sleep(0.25f);
		Goto('Begin');
	}

	Pawn.Acceleration = Vect(0,0,0);
	Sleep(2.f);

	Wander();
}

function StartHomingAtEnemy()
{
	if( Enemy != None)
	{
		class'AICmd_Nemacyst_HomeInOnEnemy'.static.InitCommand(self);
	}
}

function vector GetDestinationNearEnemy()
{
	local vector Loc1,Loc2,Loc;
	local float Dist;
	local float Speed;
	if (Enemy != None)
	{
		// try to get on same z level first
		if (Abs(Enemy.Location.Z - Pawn.Location.Z) > 256.f && VSize2D(Enemy.Location - Pawn.Location) > 256.f)
		{
			Loc1 = Pawn.Location;
			Loc1.Z = Enemy.Location.Z + 64.f;
			Loc1 += (Enemy.Location - Loc1) * 0.75f;

			Loc2 = (Enemy.Location + Pawn.Location) * 0.5f;
			Loc = (Loc1 + Loc2) * 0.5f;
		}
		else
		{
			// otherwise aim for their head
			Loc = Enemy.Location + vect(0,0,32.f);
		}
		//DrawDebugCoordinateSystem(Loc,rot(0,0,0),100.f,TRUE);
		// move a max of 256 units at a time
		Dist = VSize(Loc - Pawn.Location);
		Speed = VSize(Pawn.Velocity);
		if(Speed> 0 && (Dist/Speed < 2.25f || Dist < 512.f) && Enemy != LastEnemy)
		{
			LastEnemy = Enemy;
			if(GearPawn(Pawn) != none)
			{
				GearPawn(Pawn).BeginAttackRun(none);
			}
		}
		return (Pawn.Location + Normal(Loc - Pawn.Location) * FMin(Dist,256.f));
	}
	return Pawn.Location;
}

defaultproperties
{
	AlignToVelThreshold=40.f

	bIsPlayer=TRUE
	RotationRate=(Pitch=15000,Yaw=15000,Roll=20000)

	DefaultCommand=class'AICmd_Base_Nemacyst'

	// ignores default reactions
	DefaultReactConditions.Empty
	DefaultReactConditionClasses.Empty

	Begin Object Class=AIReactCond_EnemyVisibleForThresh Name=VisibleForTime0
		SeenThresh=1.25f
		OutputChannelName=EnemyVisibleForLongEnough
	End Object
	DefaultReactConditions.Add(VisibleForTime0)
	// when we've seen someone for logn enough push homeinonenemy
	Begin Object class=AIReactCond_GenericPushCommand name=GenComPush0
		AutoSubscribeChannels(0)=EnemyVisibleForLongEnough
		CommandClass=class'AICmd_Nemacyst_HomeInOnEnemy'
	End Object
	DefaultReactConditions.Add(GenComPush0)
	bCanRevive=false
	bCanExecute=false
}
