class GearPawn_LocustBrumakPlayerBase extends GearPawn_LocustBrumakBase
	abstract;

var SoundCue MeleeImpactSound;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	if( LeftGunComponent != None )
	{
		Mesh.AttachComponentToSocket( LeftGunComponent, 'leftgun' );
		LeftGunComponent.SetShadowParent( Mesh );
		LeftGunComponent.SetLightEnvironment( LightEnvironment );
		LeftGunComponent.SetScale( DrawScale );
	}

	if( RightGunComponent != None )
	{
		Mesh.AttachComponentToSocket( RightGunComponent, 'rightgun' );
		RightGunComponent.SetShadowParent( Mesh );
		RightGunComponent.SetLightEnvironment( LightEnvironment );
		RightGunComponent.SetScale( DrawScale );
	}

	if( MaskComponent != None )
	{
		Mesh.AttachComponentToSocket( MaskComponent, 'head' );
		MaskComponent.SetShadowParent( Mesh );
		MaskComponent.SetLightEnvironment( LightEnvironment );
		MaskComponent.SetScale( DrawScale );
	}
}

function AddDefaultInventory();
simulated function PlayShoulderMeleeCameraAnim();

simulated function PlayWeaponSwitch(Weapon OldWeapon, Weapon NewWeapon)
{
	MyGearWeapon = GearWeapon(Weapon);
	AttachWeapon();
}

simulated function GearPC GetPC()
{
	if( Controller == None && HumanGunner != None )
	{
		return HumanGunner.GetPC();
	}
	return Super.GetPC();
}