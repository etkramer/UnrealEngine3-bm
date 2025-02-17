class GearRules_COGDmgTakenMult extends GearHordeExtendedGameRules;

function NetDamage(int OriginalDamage, out int Damage, Pawn Injured, Controller InstigatedBy, vector HitLocation, out vector Momentum, class<DamageType> DamageType)
{
	Super.NetDamage(OriginalDamage, Damage, Injured, InstigatedBy, HitLocation, Momentum, DamageType);

	if (Injured.GetTeamNum() == 0)
	{
		Damage *= Multiplier;
	}
}
