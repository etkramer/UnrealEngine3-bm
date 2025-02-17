/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/** Will trigger a stumble reaction when we take the proper type of damage **/
class AIReactCond_Stumble extends AIReactCondition_Base;

var ESpecialMove ActionToPush;
var bool bIgnoreLegShotStumbles;

event bool ShouldActivate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	local AIReactChan_Damage DmgChan;
	local Pawn DamageInstigator;
	local float Pct;
	local float DotP;
	local Vector	VectToEnemy, VectToKnown;

	if (!Super.ShouldActivate(EventInstigator,OriginatingChannel))
	{
		return FALSE;
	}

	DamageInstigator = Pawn(EventInstigator);

	DmgChan = AIReactChan_Damage(OriginatingChannel);

	VectToEnemy = Normal(DamageInstigator.Location-Pawn.Location);
	VectToKnown = Normal(DmgChan.LastInstigatorLoc - Pawn.Location);

	DotP = VectToEnemy DOT VectToKnown;


	ActionToPush = SM_None;

	if(MyGearPawn == none)
	{
		return FALSE;
	}

	if( !MyGearPawn.IsDoingASpecialMove() &&
		!MyGearPawn.IsInCover() &&
		 MyGearPawn.Health <= (MyGearPawn.DefaultHealth*0.85f) )
	{
		//`AILog(GetFuncName()@DamageType@class<GearDamageType>(damageType).default.bShouldCauseStumbles@class<GearDamageType>(damageType).default.StumblePercent);
		if( MyGearPawn.bCanDBNO && MyGearPawn.TookLegShot(DmgChan.HitInfo.BoneName) )
		{
			if( DmgChan.damageType.default.bShouldCauseStumbles )
			{
				Pct = DmgChan.damageType.default.StumblePercent;

				// increased chance to stumble if shot from behind
				if (DotP < 0.2f)
				{
					Pct *= 1.25f;
				}

				if( FRand() < Pct )
				{
					DamageReceivedInAction = 0;

					// only care if we actually want to do leg shot stumbles
					if(!bIgnoreLegShotStumbles)
					{
						// here we want to check the distance.  If the attacker is close then
						// use the SubAction_StumbleDownFromCloseRangeShot
						if( VSize(DamageInstigator.Location - Pawn.Location) < StumbleDownFromCloseRangeShotDistance )
						{
							ActionToPush = SM_StumbleGoDownFromCloseRangeShot;
						}
						else
						{
							ActionToPush = SM_StumbleGoDown;
						}
					}
				}
			}
		}
		else
		if( ClassIsChildOf( DmgChan.damageType, class'GDT_Melee' ) )
		{
			if( TimeSince( LastSecondaryActionTime ) > 5.f &&
				DamageReceivedInAction > MyGearPawn.DefaultHealth * 0.2f &&
				!MyGearPawn.IsDoingStumbleFromMelee() )
			{
				DamageReceivedInAction = 0;
				LastSecondaryActionTime = WorldInfo.TimeSeconds;

				ActionToPush = SM_StumbleFromMelee;
			}
		}
	}

	if( ActionToPush != SM_None )
	{
		return TRUE;
	}

	return FALSE;
}

event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel)
{
	Super.Activate(EventInstigator,OriginatingChannel);
	NotifyStumbleAction( ActionToPush );
}

defaultproperties
{
	bActivateWhenBasedOnInterpActor=FALSE
	AutoSubscribeChannels(0)=Damage
	bIgnoreLegShotStumbles=true
}
