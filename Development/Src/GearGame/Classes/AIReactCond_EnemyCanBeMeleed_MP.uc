/** version of AIReactCond_EnemyCanBeMeleed modified for MP bot tactics */
class AIReactCond_EnemyCanBeMeleed_MP extends AIReactCond_EnemyCanBeMeleed;

function bool ShouldMeleeCharge(Actor EventInstigator, float RangeToEnemy)
{
	local GearPawn GP;
	local float Chance;
	local GearPRI PRI;

	// never charge if we are the leader, since we are too important for that sort of thing
	PRI = GearPRI(PlayerReplicationInfo);
	if (PRI.bIsLeader)
	{
		return false;
	}
	GP = GearPawn(EventInstigator);
	// use default implementation when attacking human on Casual (which makes sure human can see enemy coming at him, etc)
	if (GP != None && GP.IsHumanControlled() && PRI.Difficulty.default.DifficultyLevel == DL_Casual)
	{
		return Super.ShouldMeleeCharge(EventInstigator, RangeToEnemy);
	}
	// otherwise, scale our chance value by range to enemy, and roll the dice
	Chance = GetChanceToCharge(RangeToEnemy);
	// double chance to charge if enemy is holding a meatbag, since we'll have a hard time killing him any other way
	if (GP != None && GP.IsAKidnapper())
	{
		Chance *= 2.0;
	}
	if (FRand() < Chance)
	{
		// don't charge unless enemy has his back turned
		return ( GP != None && (vector(GP.GetViewRotation()) dot Normal(GP.Location - MyGearPawn.Location)) > VisibleDotThresh &&
			WorldInfo.TimeSeconds - LastShotAtTime < 2.0 && FastTrace(GP.Location, MyGearPawn.GetPawnViewLocation()) );
	}
	else
	{
		return false;
	}
}

defaultproperties
{
	VisibleDotThresh=0.0
}
