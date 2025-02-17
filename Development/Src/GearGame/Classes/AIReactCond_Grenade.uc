/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/** Grenade Reaction - listens for Projectile Warnings **/
class AIReactCond_Grenade extends AIReactCondition_Base;

/** GoW global macros */

event bool ShouldActivate( Actor DmgInst, AIReactChannel OrigChan )
{
	if(!Super.ShouldActivate(DmgInst,OrigChan))
	{
		return false;
	}

	if( MyGearPawn == None ||
		MyGearPawn.IsDoingASpecialMove() ||
		MyGearPawn.Health <= 0 ||
		MyGearPawn.IsDBNO() ||
		IsFriendlyPawn(DmgInst.Instigator) ||
		!bAllowCombatTransitions)
	{
		return false;
	}

	return true;
}

event Activate( Actor EventInstigator, AIReactChannel OriginatingChannel )
{
	local float Dist;
	local Projectile Proj;

	Super.Activate(EventInstigator,OriginatingChannel);
	Proj = Projectile(EventInstigator);

	// If we are not based on another pawn (ie reaver gunner)
	if( Pawn(Pawn.Base) == None )
	{
		//debug
		`AILog( GetFuncName()@Proj@VSize2D(MyGearPawn.Location - Proj.Location)@(Proj.DamageRadius * 1.5f)@FastTrace( MyGearPawn.Location, Proj.Location ) );

		if( FRand() <= ChanceToEvadeGrenade * GetEvadeChanceScale() )
		{
			Dist = VSize2D(MyGearPawn.Location - Proj.Location);
			if( Dist < Proj.DamageRadius * 1.5f &&
				FastTrace( MyGearPawn.Location, Proj.Location ) )
			{
				EvadeAwayFromPoint( Proj.Location );
			}
			else
			if( Dist < Proj.DamageRadius * 3f )
			{
				MyGearPawn.Cringe(); // Cover Head
			}
		}
	}


}


defaultproperties
{
	AutoSubscribeChannels(0)=WarnProjExplode
}
