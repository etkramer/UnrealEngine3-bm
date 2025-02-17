class GameplayCam_BrumakGunnerTargeting extends GameplayCam_BrumakGunner;

/** returns camera mode desired FOV */
function float GetDesiredFOV( Pawn ViewedPawn )
{
	local GearWeapon			Wpn;

	Wpn = GearWeapon(GetGearPC().Pawn.Weapon);

	if (Wpn != None)
	{
		return Wpn.GetTargetingFOV(FOVAngle);
	}
	else
	{
		return FOVAngle;
	}
}

defaultproperties
{
	ViewOffset={(
		OffsetHigh=(X=-450,Y=0,Z=600),
		OffsetLow=(X=50,Y=0,Z=400),
		OffsetMid=(X=-200,Y=0,Z=450),
	)}
	WorstLocOffset=(X=0,Y=0,Z=150)
}
