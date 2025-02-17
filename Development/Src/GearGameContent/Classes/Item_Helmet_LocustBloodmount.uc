/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class Item_Helmet_LocustBloodmount extends Item_HelmetBase;


defaultproperties
{
	SpawnableData.Add( ( AttachSocketName="Helmet", TheMesh=StaticMesh'Locust_Bloodmount.Locust_Bloodmount_Helmet_SMShape' ) )
	BeingShotOffSound=SoundCue'Locust_Bloodmount_Efforts.BloodMount.BloodMount_HelmetRipOffCue'
	ImpactingGround=SoundCue'Locust_Bloodmount_Efforts.BloodMount.BloodMount_HelmetRipLandCue'
}



