/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_SecurityNemacyst extends GearAI_Nemacyst
	native(AI);

/** GoW global macros */

var() float HoverHeight;
var	  float	OldAirSpeed;

cpptext
{
	virtual void GetLookDir(FVector& LookDir);
};


function Possess(Pawn NewPawn, bool bVehicleTransition)
{
	Super.Possess(NewPawn,bVehicleTransition);
	Pawn.Mesh.SetRotation(rot(0,0,0));
}

function FoundEnemyAndHomingIn()
{
	ReactionManager.SuppressChannel('Hearing');
	Enemy.MakeNoise(1.f);
	ReactionManager.UnSuppressChannel('Hearing');
	GearPawn_LocustNemacystBase(MyGearPawn).HomingInOnEnemy(Enemy);
}

state Action_Idle
{
	function HearNoise( float Loudness, Actor NoiseMaker, optional Name NoiseType )
	{
		//MessagePlayer(GetFuncname()@Loudness@NoiseMaker@NoiseType);
		if(NoiseType == 'PlayFireSound')
		{
			super(GearAI).HearNoise(Loudness,NoiseMaker,NoiseType);
		}		
	}

	function bool ShouldHoverUp()
	{
		local vector HitL, HitN,End;
		End = Pawn.Location;
		End.Z -= HoverHeight*2.0f;
		if (Pawn.Trace(HitL,HitN,End,Pawn.Location) != None)
		{
			return (VSize(HitL - Pawn.Location) < HoverHeight);
		}
		return FALSE;
	}

	function vector GetHoverDest()
	{
		local vector Dest;
		Dest = Pawn.Location;
		Dest.Z += HoverHeight;
		return Dest;
		
	}

	function OnAIMove(SeqAct_AIMove Action)
	{
		ReactionManager.UnSuppressAll();
		AbortCommand(none,class'AICmd_Nemacyst_WanderWithinVolume');
		Super.OnAIMove(Action);
	}

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );
	
	ReactionManager.SuppressAll();
	if( ShouldHoverUp() )
	{
		//debug
		`AILog("Hovering up...");

		MoveTo(GetHoverDest());

		Pawn.Acceleration = Vect(0,0,0);
		Sleep(0.25f);
		Goto('Begin');
	}
	`AILog("Post shouldhoverup");
	ReactionManager.UnSuppressAll();
	if(TetherActor == none)
	{
		Wander();
	}
	else
	{
		`AILog("Not wandering because I still have a tether, waiting for tether to be cleared...");
		OldAirSpeed = Pawn.AirSpeed;
		Pawn.AirSpeed = 50;
		MoveToward(TetherActor);
		Pawn.Acceleration = Vect(0,0,0);
		Pawn.AirSpeed = OldAirSpeed;
		while(TetherActor != none)
		{
			Sleep(0.25f);
		}
		

	}
	

	Pawn.Acceleration = Vect(0,0,0);
	Sleep(2.f);

	Goto('Begin');
}


defaultproperties
{

	// clear parents reactions
	DefaultReactConditions.Empty

	// investigate reaction (reacts to hearing)
	Begin Object Class=AIReactCond_GenericPushCommand Name=Investigate0
		AutoSubscribeChannels(0)=Hearing
		CommandClass=class'AICmd_Nemacyst_Investigate'
		MinTimeBetweenOutputsSeconds=3.0
	End Object
	DefaultReactConditions.Add(Investigate0)

	// home reaction (reacts to sight)
	Begin Object Class=AIReactCond_GenericPushCommand Name=GoAndKill0
		AutoSubscribeChannels(0)=Sight
		CommandClass=class'AICmd_Nemacyst_HomeInOnEnemy'
		MinTimeBetweenOutputsSeconds=1.0f
	End Object
	DefaultReactConditions.Add(GoAndKill0)

	HoverHeight=512.f
}
