class GearPawn_LocustBrumak_ResistSmallArms extends GearPawn_LocustBrumak;

var() config float	ScaleDamageVsSmallArms;

function AdjustPawnDamage
(
	out	int					Damage,
		Pawn				InstigatedBy,
		Vector				HitLocation,
	out Vector				Momentum,
	class<GearDamageType>	GearDamageType,
	optional	out	TraceHitInfo		HitInfo
)
{
//	`log( self@GetFuncName()@GearDamageType@GearDamageType.default.WeaponID@Damage );
//	ScriptTrace();


	if( GearDamageType != None )
	{
		switch( GearDamageType.default.WeaponID )
		{
			case WC_Lancer:
			case WC_Snub:
			case WC_Longshot:
			case WC_Hammerburst:
			case WC_Boltock:
			case WC_LocustBurstPistol:
			case WC_Gnasher:
			case WC_Scorcher:
			case WC_Minigun:
				Damage *= ScaleDamageVsSmallArms;
				break;
		}
	}

//	`log( "..."@Damage );

	Super.AdjustPawnDamage( Damage, InstigatedBy, HitLocation, Momentum, GearDamageType, HitInfo );
}

defaultproperties
{

}