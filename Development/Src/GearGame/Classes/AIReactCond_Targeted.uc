/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Targeted Reaction Condition - activates when an enemy in front of us has his crosshair on us
 */
class AIReactCond_Targeted extends AIReactCondition_Base
	native(AI);

var Pawn Shooter;
/** enemies within this range will qualify for evade */
var() float ShooterRangeThreshold;

cpptext
{
	virtual UBOOL ShouldActivateNative(AActor* Instigator, UAIReactChannel* OriginatingChannel);
}


event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	Super.Activate(EventInstigator, OriginatingChannel);
	if( Shooter != None )
	{
		// If in cover not exposed to the shooter
		if( MyGearPawn.IsInCover() && !IsCoverExposedToAnEnemy( GearAI_Cover(Outer).Cover, Shooter ) )
		{
			// Stop leaning out as a "dodge"
			if( MyGearPawn.IsLeaning() || MyGearPawn.IsPeeking(MyGearPawn.CoverAction) )
			{
				bFailedToFireFromCover = TRUE;
			}
		}
		// Otherwise, do the evade
		else
		if( InterpActor(MyGearPawn.Base) == None )
		{
			DoEvade( GetBestEvadeDir( MyGearPawn.Location, Shooter ), TRUE );
		}
	}
	else
	{
		`warn( self@GetFuncName()@"No Shooter"@Outer@EventInstigator );
	}
}


defaultproperties
{
	AutoSubscribeChannels(0)=SightPlayer
	ShooterRangeThreshold=1024
}



