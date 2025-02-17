class GearAI_Brumak_Kadar extends GearAI_Brumak;

var config float NewDefaultHealth;

event Possess(Pawn NewPawn, bool bVehicleTransition)
{
	Super.Possess( NewPawn, bVehicleTransition );

	if( Brumak != None )
	{
		Brumak.DefaultHealth = NewDefaultHealth;
		Brumak.Health = Brumak.DefaultHealth;
	}
}

defaultproperties
{
	FirePattern.Empty
	FirePattern(0)=(PatternName="BoomBoom1",SuperGunFireTime=0.5f,DelayEndTime=2.f)	
	FirePattern(1)=(PatternName="BoomBoom2",SuperGunFireTime=0.5f,DelayEndTime=3.f)
	FirePattern(2)=(PatternName="BoomBoom3",SuperGunFireTime=0.5f,DelayEndTime=4.f)
	FirePattern(3)=(PatternName="BoomBoom4",SuperGunFireTime=0.5f,DelayEndTime=5.f)
}