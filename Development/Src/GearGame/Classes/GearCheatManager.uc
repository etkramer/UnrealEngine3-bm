/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// CheatManager
// Object within playercontroller that manages "cheat" commands
// only spawned in single player mode
//=============================================================================

class GearCheatManager extends CheatManager within GearPC
	native;

var transient protected bool bDebugGUDSStreaming;

struct native SpawnPawnInfo
{
	var	String			PawnName;
	var String			ClassName;
	var class<GearAI>	AIClass;
};
var	Array<SpawnPawnInfo>	SpawnPawnList;

// forces log output (even in FINAL_RELEASE) use with caution!
function native ForceLog(coerce string s);

/** Overridden to skip PendingClientWeaponSet stuff for debug weapons. */
simulated function Weapon GiveWeapon( String WeaponClassStr )
{
	local Weapon W;
	local class<Weapon> WeaponClass;

	WeaponClass = class<Weapon>(DynamicLoadObject(WeaponClassStr, class'Class'));
	if (Pawn.FindInventoryType(WeaponClass) == None)
	{
		W = super.GiveWeapon(WeaponClassStr);
		W.GotoState('Inactive');
		return W;
	}

	return Super.GiveWeapon(WeaponClassStr);
}

/** Give player all available weapons */
exec function AllWeapons()
{
	local bool						bOldWeaponCountEnforcement;
	local GearInventoryManager		GIM;

	if( (WorldInfo.NetMode!=NM_Standalone) || (Pawn == None) )
	{
		return;
	}

	// tell inv manager we're cheating and allowing >4 weapons
	GIM = GearInventoryManager(Pawn.InvManager);
	bOldWeaponCountEnforcement = GIM.bDebugIgnoreWeaponCountLimit;
	GIM.bDebugIgnoreWeaponCountLimit = TRUE;

	// COG Weapons
	GiveWeapon("GearGame.GearWeap_COGPistol");
	GiveWeapon("GearGame.GearWeap_AssaultRifle");
	GiveWeapon("GearGame.GearWeap_Shotgun");
	GiveWeapon("GearGame.GearWeap_SniperRifle");
	GiveWeapon("GearGameContent.GearWeap_Boomshot");
	GiveWeapon("GearGameContent.GearWeap_HOD");
	GiveWeapon("GearGameContent.GearWeap_FlameThrower");

	// Locust weapons
	GiveWeapon("GearGame.GearWeap_LocustPistol");
	GiveWeapon("GearGameContent.GearWeap_LocustBurstPistol");
	GiveWeapon("GearGame.GearWeap_LocustAssaultRifle");
	GiveWeapon("GearGameContent.GearWeap_Bow");

	// Grenades
	GiveWeapon("GearGame.GearWeap_FragGrenade");
	GiveWeapon("GearGameContent.GearWeap_InkGrenade");
	GiveWeapon("GearGame.GearWeap_SmokeGrenade");

	// back to weaponcount-enforcing behavior
	GIM.bDebugIgnoreWeaponCountLimit = bOldWeaponCountEnforcement;
}

exec function SetSticks(int StickConfig)
{
	GearPRI(PlayerReplicationInfo).StickConfig = EGearStickConfigOpts(StickConfig);
	UpdateControllerConfig();
}


/** Toggle between the normal control scheme and the alternate one that separates going into and out of cover */
exec function ToggleControls()
{
	SetAlternateControls(!bUseAlternateControls);
	ClientMessage( bUseAlternateControls ? "Alternate Controls" : "Normal Controls" );
}

/** Sets maximum ammo on all weapons */
exec function AllAmmo()
{
	local GearWeapon	Weap;

	if (Pawn == None)
		return;

	ForEach Pawn.InvManager.InventoryActors(class'GearWeapon', Weap)
	{
		Weap.SpareAmmoCount = Weap.GetMaxSpareAmmoSize();
		Weap.AmmoUsedCount	= 0;
	}
}

/**
 * DanS function for prototyping the OnlineCommunityContentInterface
 */
exec function InitCommunityContent()
{
	local OnlineCommunityContentInterface CommunityContent;
	`if(`notdefined(FINAL_RELEASE))
		local bool bWasSuccessful;
	`endif

	// Ask for the interface by name and cast to our well known type
	CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
	if (CommunityContent != None)
	{
		`if(`notdefined(FINAL_RELEASE))
			bWasSuccessful = CommunityContent.Init();
			`Log("CommunityContent.Init() returned "$bWasSuccessful);
		`else
			CommunityContent.Init();
		`endif
	}
}

/**
 * DanS function for prototyping the OnlineCommunityContentInterface
 */
exec function ExitCommunityContent()
{
	local OnlineCommunityContentInterface CommunityContent;

	// Ask for the interface by name and cast to our well known type
	CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
	if (CommunityContent != None)
	{
		CommunityContent.Exit();
		`Log("CommunityContent.Exit()");
	}
}

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
exec function ReadContentList()
{
	local OnlineCommunityContentInterface CommunityContent;
	`if(`notdefined(FINAL_RELEASE))
		local bool bWasSuccessful;
	`endif

	// Ask for the interface by name and cast to our well known type
	CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
	if (CommunityContent != None)
	{
		CommunityContent.AddReadContentListCompleteDelegate(ReadContentListComplete);
		`if(`notdefined(FINAL_RELEASE))
			bWasSuccessful = CommunityContent.ReadContentList(0, 0, 100);
			`Log("CommunityContent.ReadContentList() returned "$bWasSuccessful);
		`else
				CommunityContent.ReadContentList(0, 0, 100);
		`endif
	}
}

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
function ReadContentListComplete(bool bWasSuccessful)
{
	`Log("CommunityContent.ReadContentList() completed, result: "$bWasSuccessful);
}

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
exec function GetContentList()
{
	local OnlineCommunityContentInterface CommunityContent;
	local array<CommunityContentFile> ContentFiles;
	`if(`notdefined(FINAL_RELEASE))
		local bool bWasSuccessful;
	`endif

	// Ask for the interface by name and cast to our well known type
	CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
	if (CommunityContent != None)
	{
		`if(`notdefined(FINAL_RELEASE))
			bWasSuccessful = CommunityContent.GetContentList(0, ContentFiles);
			`Log("CommunityContent.GetContentList() returned "$bWasSuccessful);
		`else
			CommunityContent.GetContentList(0, ContentFiles);
		`endif
	}
}

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
exec function ReadFriendsContentList()
{
	local OnlineCommunityContentInterface CommunityContent;
	local array<OnlineFriend> Friends;
	`if(`notdefined(FINAL_RELEASE))
		local bool bWasSuccessful;
	`endif

	// Ask for the interface by name and cast to our well known type
	CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
	if (CommunityContent != None)
	{
		// Check for friends
		OnlineSub.PlayerInterface.GetFriendsList(0, Friends);
		if(Friends.length >= 1)
		{
			CommunityContent.AddReadFriendsContentListCompleteDelegate(ReadFriendsContentListComplete);
			`if(`notdefined(FINAL_RELEASE))
				bWasSuccessful = CommunityContent.ReadFriendsContentList(0, Friends, 0, 100);
				`Log("CommunityContent.ReadFriendsContentList() returned "$bWasSuccessful);
			`else
				CommunityContent.ReadFriendsContentList(0, Friends, 0, 100);
			`endif
		}
		else
		{
			`Log("No friends");
		}
	}
}

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
function ReadFriendsContentListComplete(bool bWasSuccessful)
{
	`Log("CommunityContent.ReadFriendsContentList() completed, result: "$bWasSuccessful);
}

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
exec function GetFriendsContentList()
{
	local OnlineCommunityContentInterface CommunityContent;
	local array<CommunityContentFile> ContentFiles;
	local array<OnlineFriend> Friends;
	`if(`notdefined(FINAL_RELEASE))
		local bool bWasSuccessful;
	`endif

	// Ask for the interface by name and cast to our well known type
	CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
	if (CommunityContent != None)
	{
		// Check for friends
		OnlineSub.PlayerInterface.GetFriendsList(0, Friends);
		if(Friends.length >= 1)
		{
			`if(`notdefined(FINAL_RELEASE))
				bWasSuccessful = CommunityContent.GetFriendsContentList(0, Friends[0], ContentFiles);
				`Log("CommunityContent.GetFriendsContentList() returned "$bWasSuccessful);
			`else
				CommunityContent.GetFriendsContentList(0, Friends[0], ContentFiles);
			`endif
		}
		else
		{
			`Log("No friends");
		}
	}
}

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
/*exec function UploadContent()
{
	local OnlineCommunityContentInterface CommunityContent;
	local int ContentType;
	local array<byte> Payload;
	local int PayloadLength;
	local int Index;
	`if(`notdefined(FINAL_RELEASE))
		local bool bWasSuccessful;
	`endif

	// Ask for the interface by name and cast to our well known type
	CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
	if (CommunityContent != None)
	{
		ContentType = 0;  // test content
		PayloadLength = 100;
		Payload.length = PayloadLength;
		for(Index = 0; Index < PayloadLength; Index++)
		{
			Payload[Index] = Index;
		}
		CommunityContent.AddUploadContentCompleteDelegate(UploadContentComplete);
		`if(`notdefined(FINAL_RELEASE))
			bWasSuccessful = CommunityContent.UploadContent(0,ContentType,Payload);
			`Log("CommunityContent.UploadContent() returned "$bWasSuccessful);
		`else
			CommunityContent.UploadContent(0,ContentType,Payload);
		`endif
	}
}*/

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
function UploadContentComplete(bool bWasSuccessful,CommunityContentFile UploadedFile)
{
	`Log("CommunityContent.UploadContent() completed for content id: "$UploadedFile.ContentId$", result: "$bWasSuccessful);
}

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
exec function DownloadContent()
{
	local OnlineCommunityContentInterface CommunityContent;
	local CommunityContentFile DownloadFile;
	`if(`notdefined(FINAL_RELEASE))
		local bool bWasSuccessful;
	`endif

	// Ask for the interface by name and cast to our well known type
	CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
	if (CommunityContent != None)
	{
		CommunityContent.AddDownloadContentCompleteDelegate(DownloadContentComplete);
		DownloadFile.ContentId = 56;
		`if(`notdefined(FINAL_RELEASE))
			bWasSuccessful = CommunityContent.DownloadContent(0,DownloadFile);
			`Log("CommunityContent.DownloadContent() returned "$bWasSuccessful);
		`else
			CommunityContent.DownloadContent(0,DownloadFile);
		`endif
	}
}

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
function DownloadContentComplete(bool bWasSuccessful,CommunityContentFile DownloadedFile)
{
	`Log("CommunityContent.DownloadContent() completed for content id: "$DownloadedFile.ContentId$", result: "$bWasSuccessful$", size: "$DownloadedFile.FileSize);
	`Log("Downloads: "$DownloadedFile.DownloadCount$", Average Rating: "$DownloadedFile.AverageRating);
	`Log("LastRatingGiven: "$DownloadedFile.LastRatingGiven$", RatingCount: "$DownloadedFile.RatingCount);
}

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
exec function GetPayload()
{
	local OnlineCommunityContentInterface CommunityContent;
	local CommunityContentFile File;
	`if(`notdefined(FINAL_RELEASE))
		local bool bWasSuccessful;
	`endif

	// Ask for the interface by name and cast to our well known type
	CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
	if (CommunityContent != None)
	{
		CommunityContent.AddGetContentPayloadCompleteDelegate(GetPayloadContentComplete);
		File.ContentId = 56;
		File.FileSize = 100;
		`if(`notdefined(FINAL_RELEASE))
			bWasSuccessful = CommunityContent.GetContentPayload(0,File);
			`Log("CommunityContent.GetContentPayload() returned "$bWasSuccessful);
		`else
			CommunityContent.GetContentPayload(0,File);
		`endif
	}
}

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
function GetPayloadContentComplete(bool bWasSuccessful,CommunityContentFile FileDownloaded,const out array<byte> Payload)
{
	local int Index;
	`Log("CommunityContent.GetContentPayload() completed for content id: "$FileDownloaded.ContentId$", result: "$bWasSuccessful$", size: "$Payload.length);
	for(Index = 0; Index < Payload.length; Index++)
	{
		if(Payload[Index] != Index)
		{
			`Log("!!Payload doesn't match what was uploaded!!");
		}
	}
}

/**
* DanS function for prototyping the OnlineCommunityContentInterface
*/
exec function RateContent()
{
	local OnlineCommunityContentInterface CommunityContent;
	local CommunityContentFile ContentFile;

	// Ask for the interface by name and cast to our well known type
	CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
	if (CommunityContent != None)
	{
		ContentFile.ContentId = 56;
		CommunityContent.RateContent(0,ContentFile,100);
	}
}

/** shortcut for the lazy :-) **/
exec function UA()
{
	UnlimitedAmmo();
}

/** Gives all carried weapons unlimited ammo */
exec function UnlimitedAmmo()
{
	local GearWeapon	Weap;

	if( (WorldInfo.NetMode!=NM_Standalone) || (Pawn == None) )
		return;

	ForEach Pawn.InvManager.InventoryActors(class'GearWeapon', Weap)
	{
		Weap.SetInfiniteMagazineSize();	// disable reloads
		Weap.SetInfiniteSpareAmmo();	// unlimited spare ammo
	}
}

/** shortcut for the lazy :) */
exec function UC()
{
	UnlimitedClips();
}

/** gives unlimited clips */
exec function UnlimitedClips()
{
	local GearWeapon	Weap;

	if( (WorldInfo.NetMode!=NM_Standalone) || (Pawn == None) )
		return;

	ClientMessage("Unlimited clips!");

	ForEach Pawn.InvManager.InventoryActors(class'GearWeapon', Weap)
	{
		Weap.SetInfiniteSpareAmmo();		// unlimited spare ammo
	}
}

/** shortcut for the lazy :) */
exec function UH()
{
	UnlimitedHealth();
}
/** Allows pawn to take damage, but never die. */
exec function UnlimitedHealth()
{
	local int i;
	local GearPawn APawn;
	local GearVehicle AVehicle;

	APawn = GearPawn(Pawn);
	if (APawn == None)
	{
		AVehicle = GearVehicle(Pawn);
		if (AVehicle != None)
		{
			for (i=0; i<AVehicle.Seats.length; i++)
			{
				APawn = GearPawn(AVehicle.Seats[i].StoragePawn);
				if (APawn != None)
				{
					APawn.bUnlimitedHealth = !APawn.bUnlimitedHealth;
					ClientMessage("UnlimitedHealth"@APawn@APawn.bUnlimitedHealth);
				}
			}
		}
	}
	else
	{
		APawn.bUnlimitedHealth = !APawn.bUnlimitedHealth;
		ClientMessage("UnlimitedHealth"@APawn@APawn.bUnlimitedHealth);
	}
}

exec function HUDHate()
{
	MyGearHUD.AddWeaponTakenMessage(PlayerReplicationInfo,class'GDT_ShieldBash');
	MyGearHUD.AddWeaponTakenMessage(PlayerReplicationInfo,class'GDT_FragGrenade');
	MyGearHUD.AddWeaponTakenMessage(PlayerReplicationInfo,class'GDT_FragGrenade');
}

exec function ChangeDifficulty(coerce string NewDifficulty)
{
	local class<DifficultySettings> Difficulty;
	if (NewDifficulty ~= "Casual")
	{
		Difficulty = class'DifficultySettings_Casual';
	}
	else if (NewDifficulty ~= "Normal")
	{
		Difficulty = class'DifficultySettings_Normal';
	}
	else if (NewDifficulty ~= "Hardcore")
	{
		Difficulty = class'DifficultySettings_Hardcore';
	}
	else if (NewDifficulty ~= "Insane")
	{
		Difficulty = class'DifficultySettings_Insane';
	}
	GearPRI(PlayerReplicationInfo).Difficulty = Difficulty;
	Difficulty.static.ApplyDifficultySettings(Outer);
	MyGearPawn.MessagePlayer("OK! changed difficulty to "@Difficulty);
}

/**
* This will give you cash in addition to what CheatManager gives you.
**/
exec function Loaded()
{
	if( WorldInfo.Netmode!=NM_Standalone )
		return;

	Super.Loaded();

	UC();

	Outer.bWeaponsLoaded = TRUE;
}

exec function SuperDamage()
{
	ClientMessage("Super damage!");
	MyGearPawn.SuperDamageMultiplier += 8.f;
}

exec function Badass()
{
	UC();
	SuperDamage();
	if (!bGodMode)
	{
		God();
	}
	ClientMessage("Badass!");
}

exec function GodLoaded()
{
	Loaded();
	if (!bGodMode)
	{
		God();
	}
}

exec function Invisible()
{
    Invis();
}
exec function Invis()
{
	local GearAI AI;
	local int Idx;

	bInvisible = !bInvisible;
	ClientMessage( "Invisiblility"@bInvisible );

	foreach WorldInfo.AllControllers( class'GearAI', AI )
	{
		Idx = AI.TargetList.Find( Pawn );
		if( Idx >= 0 )
		{
			AI.TargetList.Remove( Idx, 1 );
		}

		if( AI.Enemy == Pawn ||
			AI.FireTarget == Pawn )
		{
			AI.SelectTarget();
		}
	}
}
exec function IgnorePlayer() { Invis(); }

exec function CoverHead()
{
	local GearPawn GP;

	`log(self@GetFuncName());

	foreach WorldInfo.AllPawns(class'GearPawn', GP)
	{
		GP.Cringe();
	}
}


/** Test DBNO, test cubstomp */
exec function ForceDBNO(optional bool bHumansToo, optional bool bDBNOForeverCheat)
{
	local GearPawn GP;

	`log(self@GetFuncName());

	// Forces Pawns to stay DBNO and not auto revive/die
	if( bDBNOForeverCheat )
	{
		GearGame(WorldInfo.Game).bDBNOForeverCheat = TRUE;
	}

	foreach WorldInfo.AllPawns(class'GearPawn', GP)
	{
		ForcePawnToDBNO(GP,bHumansToo);
	}
}

function ForcePawnToDBNO(GearPawn InGP, bool bHumans)
{
	local PlayerController PC;

	foreach LocalPlayerControllers(class'PlayerController', PC)
	{
		break;
	}

	if( (!InGP.IsDBNO()) && (bHumans || GearPC(InGP.Controller) == None) && InGP.bCanDBNO && (InGP.Health > 0) )
	{
		InGP.EnterDBNO(PC, class'GearDamageType');
	}
}

exec function DBNOMyTarget()
{
	local Actor HitActor;
	local vector HitLocation;
	local vector HitNormal;
	local vector TraceStart;
	local vector TraceEnd;
	local Gearpawn GP;


	TraceStart = Pawn.GetPawnViewLocation();
	TraceEnd = TraceStart + Vector(Pawn.GetViewRotation()) * 1024.f;
	HitActor = Trace(Hitlocation,HitNormal,TraceEnd,TraceStart,true);
	if(HitActor == none)
	{
		ClientMessage("Trace didn't hit anything!");
		return;
	}

	GP = GearPawn(HitActor);
	if(GP != none)
	{
		ClientMessage("Forcing "$GP$" to DBNO.");
		ForcePawnToDBNO(GP,true);
	}
	else
	{
		ClientMessage("Trace Didn't hit a GearPawn!");
	}

}

exec function DBNOMe()
{
	ForcePawnToDBNO( MyGearPawn, TRUE );
}


exec function TestDueling()
{
	local AIController	AIController;
	local GearPawn		GP;
	local GearPC		GPC;

	`log(self@GetFuncName());

	AIController = SpawnLocust();
	GP = GearPawn(AIController.Pawn);

	// Throw all weapons
	GP.ThrowActiveWeapon();
	GP.DropExtraWeapons( class'GearDamageType' );

	// Give Chainsaws
	GP.CreateInventory(class'GearWeap_AssaultRifle');

	// Force that locust into a duel.
	foreach WorldInfo.LocalPlayerControllers(class'GearPC', GPC)
	{
		GPC.MyGearPawn.ServerDoSpecialMove(SM_ChainsawDuel_Leader, TRUE, GP);
	}
}

exec function TestWretchGrab()
{
	local AIController	AIController;
	local GearPawn		GP;
	local GearPC		GPC;

	`log(self@GetFuncName());

	AIController = SpawnPawn("wretch");
	GP = GearPawn(AIController.Pawn);

	// Force that locust into a duel.
	foreach WorldInfo.LocalPlayerControllers(class'GearPC', GPC)
	{
		GP.ServerDoSpecialMove(SM_Wretch_GrabAttack, TRUE, GPC.MyGearPawn);
	}
}

/**
 * Adjusts the free camera distance, for vid capture.
 */
exec function AdjustFreeCamDistance(float Amount)
{
	PlayerCamera.FreeCamDistance += Amount;
}
exec function AdjustFreeCamOffset(float XAmount,float YAmount,float ZAmount)
{
	PlayerCamera.FreeCamOffset.X += XAmount;
	PlayerCamera.FreeCamOffset.Y += YAmount;
	PlayerCamera.FreeCamOffset.Z += ZAmount;
}

simulated exec function TeleportToLocation( float x, float y, float z, int Yaw, int Pitch, int Roll )
{
	local vector        SetLocation;
	local rotator       SetRotation;
	GetPlayerViewPoint( SetLocation, SetRotation );
	SetLocation.X = x;
	SetLocation.Y = y;
	SetLocation.Z = z;
	SetRotation.Yaw = Yaw;

	// Most of the time I just want to set the Yaw and I want the Pitch and Roll to remain the same
	// so passing in -1 for Pitch and -1 for Roll will do that.  Other wise update them as well
	if(Roll !=-1)
	{
		SetRotation.Roll = Roll;
	}
	if(Pitch !=-1)
	{
		SetRotation.Pitch = Pitch;
	}
	ClientSetLocation(SetLocation,SetRotation);
}

/**
 * Toggles between reviving/normal for testing stuffs.
 */
exec function TestReviving( optional bool bDBNOForeverCheat )
{
	if( MyGearPawn.IsDBNO() )
	{
		ServerReviveSelf();
	}
	else
	{
		if (bDBNOForeverCheat)
		{
			GearGame(WorldInfo.Game).bDBNOForeverCheat = bDBNOForeverCheat;
			GearPawn(Pawn).EnterDBNO(Outer, class'GearDamageType');
		}
		else
		{
			MyGearPawn.PlayBurstDamageWarningSoundSemaphore = TRUE;
			MyGearPawn.TakeDamage(MyGearPawn.Health,None,MyGearPawn.Location,vect(0,0,1),class'GDT_Ballistic');
			MyGearPawn.PlayBurstDamageWarningSoundSemaphore = FALSE;
		}
	}
}

/** Revive DBNO players in the level, for testing. */
exec function Revive()
{
	local GearPawn	GP;

	GearGame(WorldInfo.Game).bDBNOForeverCheat = FALSE;

	foreach WorldInfo.AllPawns(class'GearPawn', GP)
	{
		if( GP.IsDBNO() )
		{
			GP.DoRevival(GP);
		}
	}
}

exec function AIRevive()
{
	local GearAI AI;
	local GearPC PC;

	foreach WorldInfo.AllControllers( class'GearAI', AI )
	{
		break;
	}
	foreach LocalPlayerControllers( class'GearPC', PC )
	{
		break;
	}

	if( AI != None && PC != None )
	{
		class'AICmd_Revive'.static.Revive( AI, GearPawn(PC.Pawn) );
	}
}

exec function AIExecute()
{
	local GearAI AI;
	local GearPC PC;

	foreach WorldInfo.AllControllers( class'GearAI', AI )
	{
		break;
	}
	foreach LocalPlayerControllers( class'GearPC', PC )
	{
		break;
	}

	if( AI != None && PC != None )
	{
		class'AICmd_Execute'.static.Execute( AI, GearPawn(PC.Pawn) );
	}
}


/**
* This will cause us to do the warefare type suicide.  So normal
* suicide will cause the pawn to DIE DIE DIE DIE.  This will emulate
* what happens in Gear when a player is killed.
*
**/
exec function GearSuicide()
{
	// go straight to death
	MyGearPawn.TakeDamage( 10000, None, vect(0,0,0), vect(0,0,0), class'GDT_ScriptedRagdoll' );
}

exec function VerifyNavList()
{
	WorldInfo.VerifyNavList();
}

exec function AddAIFilter( coerce string Filter )
{
	local GearAI AI;
	local int Idx;
	local Name N;

	N = Name(Filter);
	foreach WorldInfo.AllControllers(class'GearAI',AI)
	{
		Idx = AI.AILogFilter.Find( N );
		if( Idx < 0 )
		{
			AI.AILogFilter[AI.AILogFilter.Length] = N;
		}
	}
}
exec function RemoveAIFilter( coerce string Filter )
{
	local GearAI AI;
	local int Idx;
	local Name N;

	N = Name(Filter);
	foreach WorldInfo.AllControllers(class'GearAI',AI)
	{
		Idx = AI.AILogFilter.Find( N );
		if( Idx >= 0 )
		{
			Pawn.MessagePlayer( "Removed Filter"@Filter );
			AI.AILogFilter.Remove( Idx, 1 );
		}
	}
}


/** Console command for camera testing. */
exec function TestCameraTurn(int Arg)
{
	//GearPlayerCamera(PlayerCamera).BeginTurn(Arg, 0.f, 0.5f);
	GearPlayerCamera(PlayerCamera).GameplayCam.BeginTurn(0.f, Arg, 0.5f, 0.f, TRUE);
}

exec function Teleport()
{
	TeleportToPlayer();
}

exec function TestBlood()
{
    MyGearPawn.SpawnBloodPool();
}

/** This will show the current health recharge settings.  usefult for testing difficulty settings **/
exec function ShowHealthRecharge()
{
	`log( "HealthRechargePercentPerSecond: " $ MyGearPawn.HealthRechargePercentPerSecond );
	`log( "HealthRechargeDelay: " $ MyGearPawn.HealthRechargeDelay );
}

exec function SpawnDummy()
{
	SpawnLocust("",true);
}

exec function CmdTest()
{
    local GearAI AI;
    AI = GearAI(SpawnLocust("",true));
//    class'AICmd_MoveToGoal'.static.MoveToPoint( AI, Pawn.Location + vect(300,0,0) );
	class'AICmd_Move_Evade'.static.Evade( AI, SM_EvadeFwd );
}

/** Left this in for legacy reasons. */
exec function AIController SpawnLocust(optional string Type, optional bool bRetarded)
{
	return SpawnPawn(Type, bRetarded);
}

/** Can spawn friendlies or enemies. */
exec function AIController SpawnPawn(optional string Type, optional bool bRetarded)
{
	local NavigationPoint Nav;
	local Pawn NewPawn;
	local AIController NewAI;
	local class<Pawn> PawnClass;
	local class<AIController> AIClass;
	local INT	i;
	local bool	bFound;

	if (Pawn != None)
	{
		Nav = class'NavigationPoint'.static.GetNearestNavToActor(Pawn,,,128.f);
	}
	if (Nav == None)
	{
		Nav = class'NavigationPoint'.static.GetNearestNavToPoint(Pawn!=None?Pawn:self,Pawn!=None?Pawn.Location:Location);
	}
	if (Nav == None)
	{
		if (Pawn != None)
		{
			Nav = Pawn.Anchor;
		}
	}

	if (Nav != None)
	{
		// See if we can find a matching creature type.
		for(i=0; i<SpawnPawnList.Length; i++)
		{
			if( SpawnPawnList[i].PawnName ~= Type )
			{
				PawnClass = class<Pawn>(DynamicLoadObject(SpawnPawnList[i].ClassName,class'class'));
				AIClass = SpawnPawnList[i].AIClass;
				bFound = TRUE;
				break;
			}
		}

		if( !bFound )
		{
			PawnClass = class'GearPawn_LocustDrone';
			AIClass = class'GearAI_Locust';
		}

		if (bRetarded)
		{
			AIClass = class'GearAI';
		}
		if (PawnClass != None && AIClass != None)
		{

			`DLog("Spawning type:" @ Type @ "PawnClass:" @ PawnClass @ "AIClass:" @ AIClass @ "at" @ Nav);

			NewPawn = Spawn(PawnClass,,,Nav.Location,rotator(Pawn.Location - Nav.Location));
			if (NewPawn != None)
			{
				NewAI = Spawn(AIClass);
				if (NewAI != None)
				{
					if (AIClass == class'GearAI_COGGear')
					{
						NewAI.SetTeam(0);
					}
					else
					{
						NewAI.SetTeam(1);
					}

					NewAI.Possess(NewPawn,FALSE);
					NewPawn.AddDefaultInventory();

					if ( bRetarded && (GearAI(NewAI) != None) )
					{
						GearAI(NewAI).bAllowCombatTransitions = FALSE;
					}
				    return NewAI;
				}
				else
				{
					NewPawn.Destroy();
				}
			}
		}
	}
    return None;
}

exec function MeleeAI()
{
	local GearAI AI;
	SpawnDummy();
	foreach WorldInfo.AllControllers(class'GearAI',AI)
	{
		AI.SetEnemy(Pawn);
		AI.PushState('SubAction_LoopingMeleeAttack');
	}
}

exec function DumpCommandStacks()
{
	local GearAI AI;
	foreach WorldInfo.AllControllers(class'GearAI', AI )
	{
		AI.DumpCommandStack();
	}
}

exec function DumpPathConstraints()
{
	local GearAI AI;
	foreach WorldInfo.AllControllers(class'GearAI', AI )
	{
		AI.DumpPathConstraints();
	}
}

exec function SteveCheat()
{
	Badass();
	DebugSteve();
}

exec function DebugSteve()
{
	DebugAI( 'Default' );
	DebugAI( 'pathing' );
	DebugAI( 'enemy' );
}

exec function Banzai()
{
	class'AICmd_Attack_Banzai'.static.EveryoneAttack(WorldInfo);
}


exec function TestLensFX(string WhichFX)
{
	local GearPC GPC;

	foreach LocalPlayerControllers(class'GearPC', GPC)
	{
		if ( (WhichFX ~= "blood") || (WhichFX ~= "shotgun") )
		{
			GPC.ClientSpawnCameraLensEffect(class'Emit_CameraBlood_Shotgun');
		}
		else if (WhichFX ~= "fire")
		{
			GPC.ClientSpawnCameraLensEffect(class'Emit_CameraScorch');
		}
		else if ( (WhichFX ~= "drip") || (WhichFX ~= "chainsaw") )
		{
			GPC.ClientSpawnCameraLensEffect(class'Emit_CameraBlood_Chainsaw');
		}
		else if (WhichFX ~= "ink")
		{
			GPC.ClientSpawnCameraLensEffect(class'Emit_CameraInk');
		}
		else if (WhichFX ~= "hod")
		{
			GPC.ClientSpawnCameraLensEffect(class'Emit_CameraHOD');
		}
	}


}

exec function TestEnemySpottedGUDS()
{
	local GearPawn EnemyPawn, P;

	// find an enemy
	foreach WorldInfo.AllPawns(class'GearPawn', P)
	{
		if (!MyGearPawn.IsSameTeam(P))
		{
			EnemyPawn = P;
			break;
		}
	}

	if (EnemyPawn != None)
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_NoticedEnemyGeneric, Pawn, EnemyPawn);
	}
	else
	{
		`log("*** No enemies found.");
	}
}

/** Toggle GUDS debug info. */
exec function GUDDebug()
{
	GearGame(WorldInfo.Game).UnscriptedDialogueManager.bShowGUDDebugText = !GearGame(WorldInfo.Game).UnscriptedDialogueManager.bShowGUDDebugText;
	GearGame(WorldInfo.Game).UnscriptedDialogueManager.bDebugGUDEvents = GearGame(WorldInfo.Game).UnscriptedDialogueManager.bShowGUDDebugText;
	GearGame(WorldInfo.Game).UnscriptedDialogueManager.bDebugGUDSLogging = !GearGame(WorldInfo.Game).UnscriptedDialogueManager.bDebugGUDSLogging;
	ClientMessage("GUDS debugging set to"@GearGame(WorldInfo.Game).UnscriptedDialogueManager.bShowGUDDebugText);
};

exec function GUDLogging()
{
	GearGame(WorldInfo.Game).UnscriptedDialogueManager.bDebugGUDSLogging = !GearGame(WorldInfo.Game).UnscriptedDialogueManager.bDebugGUDSLogging;
	ClientMessage("GUDS logging set to"@GearGame(WorldInfo.Game).UnscriptedDialogueManager.bDebugGUDSLogging);
}

exec function ScreenshotMode()
{
	local GearPlayerInput_Screenshot NewInput;
	NewInput = new(Outer) class'GearPlayerInput_Screenshot';
	NewInput.OnInputObjectCreate();
	PushPlayerInput(NewInput);
	PlayerInput = NewInput;
	ClientMessage("Entered screenshot mode");
	`log("Entered screenshot mode");
}
//
///** @fixme: hack function, just for testing, will delete later */
//function CameraAnim GetTestCameraAnim(int Idx)
//{
//	switch (Idx)
//	{
//	case 0:
//		return CameraAnim'CameraAnimTest.CA_Test01';
//	case 1:
//		return CameraAnim'CameraAnimTest.CA_TranslationRotationTest';
//	case 2:
//		return CameraAnim'CameraAnimTest.CA_FOVTest';
//	case 3:
//		return CameraAnim'CameraAnimTest.CA_PPTest';
//	}
//	return None;
//}
//exec function TestCameraAnim(int Idx)
//{
//	local CameraAnim Anim;
//	Anim = GetTestCameraAnim(Idx);
//	if (Anim != None)
//	{
//		GearPlayerCamera(PlayerCamera).PlayCameraAnim(Anim,,, 0.2f, 0.2f);
//	}
//}
//exec function StopTestCameraAnim(int Idx)
//{
//	local CameraAnim Anim;
//	Anim = GetTestCameraAnim(Idx);
//	if (Anim != None)
//	{
//		GearPlayerCamera(PlayerCamera).StopAllCameraAnimsByType(Anim);
//	}
//}
//exec function TestCameraAnimLoop(int Idx)
//{
//	local CameraAnim Anim;
//	Anim = GetTestCameraAnim(Idx);
//	if (Anim != None)
//	{
//		GearPlayerCamera(PlayerCamera).PlayCameraAnim(Anim,,, 0.2f, 0.2f, TRUE);
//	}
//}

exec function TestWeather(Int Type)
{
	GearGRI(WorldInfo.GRI).SetCurrentWeather(EWeatherType(Type));
}

exec function Tentacle( int Idx )
{
	local GearPawn_LocustLeviathanBase P;
	foreach WorldInfo.AllPawns( class'GearPawn_LocustLeviathanBase', P )
	{
		P.StartTentacleAttack( Idx, Pawn );
	}
}

exec function UnlockAchievement( EGearAchievement AchievementId )
{
	`log("Unlocking achievement" @ `showenum(EGearAchievement,AchievementId));
	ClientUnlockAchievement(AchievementId);
}

exec function UnlockAllAchievements()
{
	local int Count;

	if (OnlineSub != None &&
		OnlineSub.PlayerInterfaceEx != None)
	{
		for (Count = 50; Count > 0; Count--)
		{
			// Read this title's achivements for player 0
			OnlineSub.PlayerInterfaceEx.UnlockAchievement(0,Count);
		}
	}
}

exec function UnlockAllGamerpics()
{
	OnlineSub.PlayerInterfaceEx.UnlockGamerPicture(0,1);
	OnlineSub.PlayerInterfaceEx.UnlockGamerPicture(0,2);
}

/** Used to read the achievements for a user */
exec function ReadAchievements( optional int TitleId )
{
	if (OnlineSub != None &&
		OnlineSub.PlayerInterfaceEx != None)
	{
		OnlineSub.PlayerInterfaceEx.AddReadAchievementsCompleteDelegate(0,OnReadAchievementsComplete);
		// Read this title's achivements for player 0
		OnlineSub.PlayerInterfaceEx.ReadAchievements(0,TitleId);
	}
}

/** Dumps the achievements that were read */
function OnReadAchievementsComplete(int TitleId)
{
	local int Index;
	local array<AchievementDetails> Details;

	OnlineSub.PlayerInterfaceEx.ClearReadAchievementsCompleteDelegate(0,OnReadAchievementsComplete);

	OnlineSub.PlayerInterfaceEx.GetAchievements(0,Details,TitleId);
	`Log("Found "$Details.Length$" achievements for titleId:" @ TitleId);
	`Log("");
	// Iterate through the achievements and dump them out
	for (Index = 0; Index < Details.Length; Index++)
	{
		`Log("Id = "$Details[Index].Id);
		`Log("Name = "$Details[Index].AchievementName);
		`Log("Description = "$Details[Index].Description);
		`Log("HowTo = "$Details[Index].HowTo);
		`Log("Worth = "$Details[Index].GamerPoints);
		`Log("bIsSecret = "$Details[Index].bIsSecret);
		`Log("bWasAchievedOnline = "$Details[Index].bWasAchievedOnline);
		`Log("bWasAchievedOffline = "$Details[Index].bWasAchievedOffline);
		`Log("");
	}
}

/** Used to read the achievements for a user */
exec function ReadGears1Achievements()
{
	if (OnlineSub != None &&
		OnlineSub.PlayerInterfaceEx != None)
	{
		OnlineSub.PlayerInterfaceEx.AddReadAchievementsCompleteDelegate(0,OnReadAchievementsComplete);
		// Read this title's achivements for player 0
		OnlineSub.PlayerInterfaceEx.ReadAchievements(0,0x4D530842);
	}
}

exec function DebugSaveProfile()
{
	SaveProfile();
}

/**
 * Sends the profile to MCP for stats aggregation
 */
exec function UploadProfile()
{
	local OnlineEventsInterface Uploader;
	local UniqueNetId UniqueId;

	if (OnlineSub != None)
	{
		// Ask for the interface by name and cast to our well known type
		Uploader = OnlineEventsInterface(OnlineSub.GetNamedInterface('McpUpload'));
		if (Uploader != None)
		{
			OnlineSub.PlayerInterface.GetUniquePlayerId(0,UniqueId);
			Uploader.UploadProfileData(UniqueId,OnlineSub.PlayerInterface.GetPlayerNickname(0),ProfileSettings);
		}
	}
}

/**
 * Sends the hardware information to MCP for stats aggregation
 */
exec function UploadHardwareData()
{
	local OnlineEventsInterface Uploader;
	local UniqueNetId UniqueId;

	if (OnlineSub != None)
	{
		// Ask for the interface by name and cast to our well known type
		Uploader = OnlineEventsInterface(OnlineSub.GetNamedInterface('McpUpload'));
		if (Uploader != None)
		{
			OnlineSub.PlayerInterface.GetUniquePlayerId(0,UniqueId);
			Uploader.UploadHardwareData(UniqueId,OnlineSub.PlayerInterface.GetPlayerNickname(0));
		}
	}
}

/**
 * Used to test the custom player list
 */
exec function CustomPlayerList()
{
	local array<UniqueNetId> Players;
	local PlayerReplicationInfo PRI;
	local UniqueNetId ZeroId;

	if (OnlineSub != None && OnlineSub.PlayerInterfaceEx != None)
	{
		if (WorldInfo.GRI != None)
		{
			foreach WorldInfo.GRI.PRIArray(PRI)
			{
				if (PRI.UniqueId != ZeroId)
				{
					Players.AddItem(PRI.UniqueId);
				}
			}
		}
		else
		{
			OnlineSub.PlayerInterface.GetUniquePlayerId(0,ZeroId);
			Players.AddItem(ZeroId);
		}
		OnlineSub.PlayerInterfaceEx.ShowCustomPlayersUI(0,Players,"Joe Rocks","No, really, Joe rocks!");
	}
}

/**
 * Used to test TMS file reading
 */
exec function ReadTMS()
{
	if (OnlineSub != None && OnlineSub.SystemInterface != None)
	{
		OnlineSub.SystemInterface.ReadTitleFile("*.*");
	}
}

/**
 * Used to test MCP file reading
 */
exec function ReadFileFromMCP()
{
	local OnlineTitleFileDownloadMcp Downloader;

	if (OnlineSub != None)
	{
		// Ask for the interface by name and cast to our well known type
		Downloader = OnlineTitleFileDownloadMcp(OnlineSub.GetNamedInterface('McpDownload'));
		if (Downloader != None)
		{
			Downloader.ReadTitleFile("iisstart.htm");
		}
	}
}

exec function ListEfforts(optional bool bAll)
{
	local array<GearSoundGroup> DumpedGroups;
	local class<GearPawn> PawnClass;
	local int i;

	if (bAll)
	{
		for(i=0; i<SpawnPawnList.Length; i++)
		{
			PawnClass = class<GearPawn>(DynamicLoadObject(SpawnPawnList[i].ClassName,class'class'));
			if ( (PawnClass.default.SoundGroup != None) && (DumpedGroups.Find(PawnClass.default.SoundGroup) == INDEX_NONE) )
			{
				PawnClass.default.SoundGroup.DumpMemoryUsage(false);
				DumpedGroups[DumpedGroups.length] = PawnClass.default.SoundGroup;
			}
		}
	}
	else
	{
		MyGearPawn.SoundGroup.DumpMemoryUsage(true);
	}

}

exec function AbortAllAICommands()
{
	local GearAI GAI;
	foreach WorldInfo.AllControllers(class'GearAI', GAI)
	{
		`log("Aborting all commands for "$GAI$"...");
		GAI.AbortCOmmand(GAI.CommandList);
	}
}

exec function DeleteEveryoneWhenStale()
{
	local GearAI GAI;
	foreach WorldInfo.AllControllers(class'GearAI', GAI)
	{
		GAI.SetEnableDeleteWhenStale(TRUE);
	}
}

function DumpInfoForAI(GearAI AI)
{
	local float DistFromPlayer, TimeSinceLastRender;
	DistFromPlayer = AI.GetMinDistanceToAnyPlayer();
	TimeSinceLastRender = TimeSInce(AI.Pawn.LastRenderTime);

	ForceLog("  ... "$AI$"(ActvCmd:"$AI.GetActionString()$")  -- DistFromPlayer:"@DistFromPlayer@"TimeSinceLastRender:"@TimeSinceLastRender@"Enemy:"@AI.Enemy@"AILoc:"@AI.Pawn.Location);
}

function DumpInfoForAITeam(EGearTeam Team,out int Out_Total,out int Out_WithEnemy,out int Out_Rendered)
{
	local GearAI GAI;
	local int Total,WithEnemy,Rendered;
	ForceLog("  "$Team$" AI:");
	foreach WorldInfo.AllControllers(class'GearAI', GAI)
	{
		if(GAI.GetTeamNum() != Team)
		{
			continue;
		}
		Total++;
		if(GAI.Enemy != none)
		{
			WithEnemy++;
		}
		if(TimeSince(GAI.Pawn.LastRenderTime) < 0.08f)
		{
			Rendered++;
		}
		DumpInfoForAI(GAI);
	}
	ForceLog("  "$team$" Totals: (HaveEnemy/RenderedRecently/Total) ("$WithEnemy$"/"$Rendered$"/"$Total$")");
	Out_Total += Total;
	Out_WithEnemy += WithEnemy;
	Out_Rendered += Rendered;
}
exec function DumpAIStats()
{
	local int Total,WithEnemy,Rendered;
	ForceLog("-------------         AI STATS         -------------");
	DumpInfoForAITeam(TEAM_COG,Total,WithEnemy,Rendered);
	DumpInfoForAITeam(TEAM_LOCUST,Total,WithEnemy,Rendered);
	ForceLog(" Overall Totals: (HaveEnemy/RenderedRecently/Total) ("$WithEnemy$"/"$Rendered$"/"$Total$")");
	ForceLog("----------------------------------------------------");

}

exec function FlushAILogs()
{
	local GearAI GAI;

	if(!WorldInfo.bPlayersOnly)
	{
		DebugFreezeGame();
	}
	foreach WorldInfo.AllControllers(class'GearAI', GAI)
	{
		if(GAI.AILogFile != none)
		{
			GAI.AILogFile.CloseLog();
			GAI.AILogFile.Destroy();
			GAI.AILogFile = none;
		}
	}
}

// MSSTART brantsch@microsoft.com - 05/09/2008: Debug Message functionality for MS tools
native static function MS_SendDebugMessage(string message);
// MSEND brantsch@microsoft.com - 05/09/2008: Debug Message functionality for MS tools

/** Unlocks all the chapters for all difficulties */
exec function UnlockAllChapters()
{
	local GearPC PC;
	local int ChapIndex;

	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		for (ChapIndex = 0; ChapIndex < CHAP_MAX; ChapIndex++)
		{
			PC.ClientUnlockChapter(EChapterPoint(ChapIndex),CHAP_MAX,DL_Insane);
		}
	}
	`log("Unlocked all Chapters for all difficulties");
}

/** Relock all the chapters */
exec function LockAllChapters()
{
	local GearPC PC;

	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		PC.ProfileSettings.SetProfileSettingValueInt(PC.ProfileSettings.UnlockedChaptersCasual1, 0);
		PC.ProfileSettings.SetProfileSettingValueInt(PC.ProfileSettings.UnlockedChaptersCasual2, 0);
		PC.ProfileSettings.SetProfileSettingValueInt(PC.ProfileSettings.UnlockedChaptersNormal1, 0);
		PC.ProfileSettings.SetProfileSettingValueInt(PC.ProfileSettings.UnlockedChaptersNormal2, 0);
		PC.ProfileSettings.SetProfileSettingValueInt(PC.ProfileSettings.UnlockedChaptersHard1, 0);
		PC.ProfileSettings.SetProfileSettingValueInt(PC.ProfileSettings.UnlockedChaptersHard2, 0);
		PC.ProfileSettings.SetProfileSettingValueInt(PC.ProfileSettings.UnlockedChaptersInsane1, 0);
		PC.ProfileSettings.SetProfileSettingValueInt(PC.ProfileSettings.UnlockedChaptersInsane2, 0);
		PC.ProfileSettings.SetProfileSettingValueInt(PC.ProfileSettings.UnlockedChapterAccess1, 0);
		PC.ProfileSettings.SetProfileSettingValueInt(PC.ProfileSettings.UnlockedChapterAccess2, 0);
		PC.SaveProfile();
	}
	`log("Locked all Chapters for all difficulties");
}

/** Unlocks all the chapters for a specific difficulty */
exec function UnlockChaptersDiff(EDifficultyLevel DiffType)
{
	local GearPC PC;
	local int ChapIndex;

	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		for (ChapIndex = 0; ChapIndex < CHAP_MAX; ChapIndex++)
		{
			PC.ClientUnlockChapter(EChapterPoint(ChapIndex),CHAP_MAX,DiffType);
		}
	}
	`log("Unlocked all Chapters for " $ DiffType);
}

/** Unlocks a chapter for a specific difficulty */
exec function UnlockChapter(EChapterPoint Chapter, EDifficultyLevel DiffType)
{
	local GearPC PC;

	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		PC.ClientUnlockChapter(Chapter,CHAP_MAX,DiffType);
	}
	`log("Unlocked chapter" @ Chapter @ "for" @ DiffType);
}

/** Sets values for progression achievements */
exec function SetProgression(EGearAchievement Achievement, int Value)
{
	local GearPC PC;
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		PC.ProfileSettings.SetCurrentProgressValueForAchievement(Achievement, Value);
	}
	`log("Progression for" @ Achievement @ "set to"@Value@"!");
}

/** Marks a chapter as complete for the coop achievements */
exec function CompleteCoopChapter(EChapterPoint Chapter)
{
	local GearPC PC;
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		PC.ProfileSettings.MarkCoopChapterComplete(Chapter, PC);
	}
	`log("Coop chapter" @ Chapter @ "completed!");
}

/** Marks a weapon as having killed with for testing the VarietyIsTheSpiceOfDeath achievement */
exec function CompleteWeaponKill(EGearWeaponType WeapType)
{
	local GearPC PC;
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		PC.ProfileSettings.MarkWeaponWeKilledWith(WeapType, PC);
	}
	`log("Weapon kill" @ WeapType @ "completed!");
}

/** Marks an execution as completed for testing the MultitalentedExecutioner achievement */
exec function CompleteExecution(EGearExecutionType ExecType)
{
	local GearPC PC;
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		PC.ProfileSettings.AttemptMarkForExecutionAchievement(ExecType, PC);
	}
	`log("Execution kill" @ ExecType @ "completed!");
}

/** Marks a training session as completed for testing the BackToTheBasics achievement */
exec function CompleteTraining(EGearTrainingType TrainType)
{
	local GearPC PC;
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		PC.ProfileSettings.MarkTrainingCompleted(TrainType, PC);
	}
	`log("Training type" @ TrainType @ "completed!");
}

/** Marks a map as completed for testing the AroundTheWorld achievement */
exec function CompleteMPMap(EGearMapsShipped MapType)
{
	local GearPC PC;
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		PC.ProfileSettings.UpdateMPMatchProgression(0, MapType, PC);
	}
	`log("MP Map" @ MapType @ "completed!");
}

/** Marks a Horde wave as completed for testing the Horde achievements */
exec function CompleteHordeWave(int Wave)
{
	local GearPC PC;
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		PC.ProfileSettings.MarkHordeWaveAsCompleted(Wave, PC);
	}
	`log("Horde wave" @ Wave @ "completed!");
}

exec function DebugFaceCam(optional bool bEnableGUDSBrowsing)
{
	bDebugFaceCam = TRUE;
	bDebugGUDBrowser = bEnableGUDSBrowsing;
	GearGame(WorldInfo.Game).UnscriptedDialogueManager.bEnableGUDSBrowsing = bEnableGUDSBrowsing;
	MyGearHud.bShowHUD = !(bDebugGUDBrowser || bDebugEffortBrowser);
}

exec function GUDSBrowser()
{
	bDebugGUDBrowser = !bDebugGUDBrowser;
	GearGame(WorldInfo.Game).UnscriptedDialogueManager.bEnableGUDSBrowsing = bDebugGUDBrowser;
	MyGearHud.bShowHUD = !bDebugGUDBrowser;
}

exec function EffortBrowser()
{
	bDebugEffortBrowser = !bDebugEffortBrowser;
	GearGame(WorldInfo.Game).UnscriptedDialogueManager.bEnableEffortsBrowsing = bDebugEffortBrowser;
	MyGearHud.bShowHUD = !bDebugEffortBrowser;
}

exec function DebugGUDSStreaming()
{
	bDebugGUDSStreaming = !bDebugGUDSStreaming;
	MyGearHud.SetDebugDraw( DebugDraw_GUDSStreaming, bDebugGUDSStreaming );
}
simulated function DebugDraw_GUDSStreaming( GearHUD_Base H )
{
	GearGame(WorldInfo.Game).UnscriptedDialogueManager.DrawDebugStreaming(H.Canvas);
}

/**
 * Sets the most restrictive NAT setting that can host
 *
 * @param the new NAT setting to use
 */
exec function SetCanHostVersusMatch(ENATType NatType)
{
	class'GearPartyGame_Base'.default.CanHostVersusNatType = NatType;
	class'GearPreGameLobbyGame_Base'.default.CanHostVersusNatType = NatType;
	class'GearMenuGame'.default.CanHostVersusNatType = NatType;
	GearMenuGame(WorldInfo.Game).CanHostVersusNatType = NatType;
	`Log("class'GearMenuGame'.default.CanHostVersusNatType = "$class'GearMenuGame'.default.CanHostVersusNatType);
}

/**
 * Sets the most restrictive NAT setting that can host partial parties
 *
 * @param the new NAT setting to use
 */
exec function SetCanHostPartyMatch(ENATType NatType)
{
	class'GearPartyGame_Base'.default.CanHostPartyNatType = NatType;
	class'GearPreGameLobbyGame_Base'.default.CanHostPartyNatType = NatType;
	class'GearMenuGame'.default.CanHostPartyNatType = NatType;
	GearMenuGame(WorldInfo.Game).CanHostPartyNatType = NatType;
	`Log("class'GearMenuGame'.default.CanHostPartyNatType = "$class'GearMenuGame'.default.CanHostPartyNatType);
}

/**
 * Sets the most restrictive NAT setting that can invite players
 *
 * @param the new NAT setting to use
 */
exec function SetCanInviteToMatch(ENATType NatType)
{
	class'GearPartyGame_Base'.default.CanInviteNatType = NatType;
	class'GearPreGameLobbyGame_Base'.default.CanInviteNatType = NatType;
	class'GearMenuGame'.default.CanInviteNatType = NatType;
	GearMenuGame(WorldInfo.Game).CanInviteNatType = NatType;
	`Log("class'GearMenuGame'.default.CanInviteNatType = "$class'GearMenuGame'.default.CanInviteNatType);
}

exec function LogSquadEnemyList()
{
	local Pawn E;
	local int Idx;
	local GearAI AI;

	`log( GetFuncName()@Squad );
	if( Squad != None )
	{
		Idx = 0;
		foreach Squad.AllEnemies( class'Pawn', E )
		{
			`log( "Iterator"@Idx@E );
			Idx++;
		}
		`log( "Total Squad Enemies"@Idx );

		foreach Squad.AllMembers( class'GearAI', AI )
		{
			`log( "AI Local List"@AI@AI.LocalEnemyList.Length );
			for( Idx = 0; Idx < AI.LocalEnemyList.Length; Idx++ )
			{
				`log( AI@"Local"@Idx@AI.LocalEnemyList[Idx].Pawn );
			}
		}
	}
}

exec function ShowUnviewedNavPaths()
{
	local int Value1, Value2, NavPathsNeedingViewingValue;
	local bool bMarked;

	ProfileSettings.GetProfileSettingValueInt( ProfileSettings.NavigationNeedsViewed, NavPathsNeedingViewingValue );

	// war journal
	bMarked = ProfileSettings.NavigationPathNeedsViewed(eNAVPATH_MainMenu_WarJournal);
	`log("WARJOURNAL -" @ `showvar(bMarked));

	// discoverable
	ProfileSettings.GetProfileSettingValueInt(ProfileSettings.DiscoverNeedsViewed1, Value1);
	ProfileSettings.GetProfileSettingValueInt(ProfileSettings.DiscoverNeedsViewed2, Value2);
	bMarked = ProfileSettings.NavigationPathNeedsViewed(eNAVPATH_WarJournal_Discover);
	`log("COLLECTABLES -" @ `showvar(bMarked) @ `showvar(Value1,DiscoverNeedsViewed1) @ `showvar(Value2,DiscoverNeedsViewed2));

	// unlockable
	ProfileSettings.GetProfileSettingValueInt(ProfileSettings.UnlockableNeedsViewed, Value1);
	bMarked = ProfileSettings.NavigationPathNeedsViewed(eNAVPATH_WarJournal_Unlocks);
	`log("UNLOCKABLES -" @ `showvar(bMarked) @ `showvar(Value1,UnlockableNeedsViewed));


	// achievement
	bMarked = ProfileSettings.NavigationPathNeedsViewed(eNAVPATH_WarJournal_Achieve);
	`log("ACHIEVEMENT -" @ `showvar(bMarked));

	bMarked = bMarked;
}

/** Show the discoverable scene for the discoverable */
exec function DisplayCollectible( EGearDiscoverableType DiscType )
{
	DiscoverType = DiscType;
	StartDiscoverableUI();
}

/** Debug hook to unlock various collectibles  needs to go he**/
exec function UnlockCollectable( EGearDiscoverableType DiscType )
{
	UnlockCollectible( DiscType );
}

/** Debug hook to unlock various collectibles **/
exec function UnlockCollectible( EGearDiscoverableType DiscType )
{
	if ( DiscType != eDISC_None )
	{
		`log(`location @ "unlocking collectable" @ GetEnum(enum'EGearDiscoverableType',DiscType));
		ClientPerformDiscoverableActions(DiscType);
	}
}

exec function UnlockAllCollectables()
{
	UnlockAllCollectibles();
}
exec function UnlockAllCollectibles()
{
	local byte i;
	local EGearDiscoverableType DiscType;

	if( ProfileSettings != None )
	{
		`log(`location @ "unlocking all collectibles");

		// Set and mark the profile
		for ( i = eDISC_None + 1; i < eDISC_MAX; i++ )
		{
			DiscType = EGearDiscoverableType(i);
			ProfileSettings.MarkDiscoverableAsFoundButNotViewed(DiscType, Outer, false);
		}

		SaveProfile();
	}
	else
	{
		`log(`location @ Outer @ "has no profile!");
	}
}

exec function LockAllCollectibles()
{
	if ( ProfileSettings != None )
	{
		`log(`location @ "locking all collectibles!");
		ProfileSettings.SetProfileSettingValueInt( ProfileSettings.DiscoverFound1, 0 );
		ProfileSettings.SetProfileSettingValueInt( ProfileSettings.DiscoverNeedsViewed1, 0 );
		ProfileSettings.SetProfileSettingValueInt( ProfileSettings.DiscoverFound2, 0 );
		ProfileSettings.SetProfileSettingValueInt( ProfileSettings.DiscoverNeedsViewed2, 0 );
		SaveProfile();
	}
}

exec function UnlockUnlockable( EGearUnlockable Unlockable )
{
	if ( ProfileSettings != None )
	{
		`log(`location @ "unlocking" @ GetEnum(enum'EGearUnlockable',Unlockable));
		ProfileSettings.MarkUnlockableAsUnlockedButNotViewed(Unlockable, Outer);
	}
	else
	{
		`log(`location @ Outer @ "has no profile!");
	}
}

exec function UnlockAllUnlockables()
{
	if( ProfileSettings != None )
	{
		`log(`location @ "Unlocking all unlockables NOT including DLC characters.");
		UnlockMultipleUnlockables(eUNLOCK_Character_DLC1);
	}
	else
	{
		`log(`location @ Outer @ "has no profile!");
	}
}

exec function UnlockAllUnlockablesDLC()
{
	if( ProfileSettings != None )
	{
		`log(`location @ "Unlocking all unlockables including DLC characters.");
		UnlockMultipleUnlockables(eUNLOCK_Character_None);
	}
	else
	{
		`log(`location @ Outer @ "has no profile!");
	}
}

private function UnlockMultipleUnlockables( EGearUnlockable MaxUnlockableValue )
{
	local byte i;

	if( ProfileSettings != None )
	{
		// Set and mark the profile
		for ( i = 0; i < MaxUnlockableValue; i++ )
		{
			ProfileSettings.MarkUnlockableAsUnlockedButNotViewed(EGearUnlockable(i), Outer);
		}
		SaveProfile();
	}
}

exec function ResetAchievements()
{
	if ( ProfileSettings != None )
	{
		`log(`location @ "re-locking all achievements");
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.COMPLETED_ACHIEVEMENT_MASK_A, 0);
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.COMPLETED_ACHIEVEMENT_MASK_B, 0);
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.UNVIEWED_ACHIEVEMENT_MASK_A, 0);
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.UNVIEWED_ACHIEVEMENT_MASK_B, 0);
	}
	else
	{
		`log(`location @ Outer @ "has no profile!");
	}
}

exec function ShowAchievementMasks()
{
	local int Value;

	if ( ProfileSettings != None )
	{
		`log(`location @ "showing all achievements");
		ProfileSettings.GetProfileSettingValueInt(ProfileSettings.COMPLETED_ACHIEVEMENT_MASK_A, Value);
		`log("COMPLETED ACHIEVEMENT MASK A:" @ Value);

		ProfileSettings.GetProfileSettingValueInt(ProfileSettings.COMPLETED_ACHIEVEMENT_MASK_B, Value);
		`log("COMPLETED ACHIEVEMENT MASK B:" @ Value);

		ProfileSettings.GetProfileSettingValueInt(ProfileSettings.UNVIEWED_ACHIEVEMENT_MASK_A, Value);
		`log("UNVIEWED ACHIEVEMENT MASK A:" @ Value);

		ProfileSettings.GetProfileSettingValueInt(ProfileSettings.UNVIEWED_ACHIEVEMENT_MASK_B, Value);
		`log("UNVIEWED ACHIEVEMENT MASK A:" @ Value);
	}
	else
	{
		`log(`location @ Outer @ "has no profile!");
	}
}

exec function ResetAllCollectibles()
{
	if ( ProfileSettings != None )
	{
		`log(`location @ "re-locking all collectibles");
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.DiscoverFound1, 0);
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.DiscoverNeedsViewed1, 0);
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.DiscoverFound2, 0);
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.DiscoverNeedsViewed2, 0);

		SaveProfile();
	}
	else
	{
		`log(`location @ Outer @ "has no profile!");
	}
}

exec function ResetUnlockables()
{
	if ( ProfileSettings != None )
	{
		`log(`location @ "re-locking all unlockables");
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.UnlockedUnlockables, 0);
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.UnlockableNeedsViewed, 0);

		SaveProfile();
	}
	else
	{
		`log(`location @ Outer @ "has no profile!");
	}
}

// MSTART- justinmc @microsoft.com- 06/25/2008: Enabling the playing of individual sound cues on command
native static function MS_GetSoundCueInfo();

exec function MS_ExtractSoundCues()
{
	class'GearCheatManager'.static.MS_GetSoundCueInfo();
}

exec function MS_PlaySoundCue(string soundclip, int MessageGUID = 0)
{
	local SoundCue ASound;
	local PlayerController PC;

	foreach WorldInfo.AllControllers(class'PlayerController', PC)
	{
		if(soundclip != "")
		{
			ASound = SoundCue(DynamicLoadObject(soundclip,class'SoundCue'));
			if(ASound != None)
			{
				PC.Kismet_ClientPlaySound(ASound,PC.Pawn,1.0f,1.0f,1.0f,FALSE,FALSE);
				class'GearCheatManager'.static.MS_SendDebugMessage("UNREAL!UDC!" $  string(Rand(500)) $  "|1|" $  string(MessageGUID) $  "|1");
			}
			else
			{
				class'GearCheatManager'.static.MS_SendDebugMessage("UNREAL!UDC!" $  string(Rand(500)) $  "|1|" $  string(MessageGUID) $  "|0");
			}
		}

	}
}

// MSEND- justinmc @microsoft.com- 06/25/2008: Enabling the playing of individual sound cues on command

// MSSTART- justinmc @microsoft.com- 07/02/2008: Exposing GearPC.CanRunToCover function
exec function MS_CanRunToCover(int MessageGUID = 0)
{
	local bool bResult;
	local CovPosInfo CoverCanRunTo;
	local int NoCameraAutoAlign;

	bResult = CanRunToCover(CoverCanRunTo, NoCameraAutoAlign);


	if(bResult == TRUE)
	{
		class'GearCheatManager'.static.MS_SendDebugMessage("UNREAL!UDC!" $  string(Rand(500)) $  "|1|" $  string(MessageGUID) $  "|1");
	}
	else
	{
		class'GearCheatManager'.static.MS_SendDebugMessage("UNREAL!UDC!" $  string(Rand(500)) $  "|1|" $  string(MessageGUID) $  "|0");
	}
}
// MSEND- justinmc @microsoft.com- 07/02/2008: Exposing GearPC.CanRunToCover function

exec function ResetNavIcons()
{
	if ( ProfileSettings != None )
	{
		`log(`location @ "resetting all nav attract icons");
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.NavigationNeedsViewed, 0);
		SaveProfile();
	}
	else
	{
		`log(`location @ Outer @ "has no profile!");
	}
}

/** exec for forcing the loading movie to play or stop - useful for cases where the loading movie is stuck on */
exec function ShowLoadingMovie( optional string EnableString )
{
	local bool bEnable;

	bEnable = true;
	if ( EnableString != "" )
	{
		bEnable = bool(EnableString);
	}

	ClientShowLoadingMovie(bEnable);
}

//@NOTE: these probably will not work right in COOP
exec function PrepMap( coerce name LevelName )
{
	`log( "Zoom Prepare:" @ LevelName );
	ClientPrepareMapChange( LevelName, TRUE, TRUE );
}
exec function CommitMap()
{
	ClientCommitMapChange();
}

exec function ShowDamage()
{
	class'GearGRI'.default.bDebugShowDamage = !class'GearGRI'.default.bDebugShowDamage;

	if (class'GearGRI'.default.bDebugShowDamage)
	{
		ClientMessage("ShowDamage ON");
	}
	else
	{
		ClientMessage("ShowDamage OFF");
	}
}



/** The if order must match the order in MeatflagClasses **/
exec function SetMeatFlag( name MeatFlagName )
{
	if( MeatFlagName == 'Franklin' )
	{
		GearGameCTM_Base(WorldInfo.Game).CurrMeatflagClass = GearGameCTM_Base(WorldInfo.Game).MeatflagClasses[0];
	}
	else if( MeatFlagName == 'OldMan' )
	{
		GearGameCTM_Base(WorldInfo.Game).CurrMeatflagClass = GearGameCTM_Base(WorldInfo.Game).MeatflagClasses[1];
	}
	else if( MeatFlagName == 'Drunk' )
	{
		GearGameCTM_Base(WorldInfo.Game).CurrMeatflagClass = GearGameCTM_Base(WorldInfo.Game).MeatflagClasses[2];
	}
	else
	{
		`log( "Invalid MeatFlagName.  Options are:  Franklin|OldMan|Drunk"  );
	}
}


// - - - Progress Notification - - -
exec function splitalert()
{
	local GearPC PC;

	foreach LocalPlayerControllers( class'GearPC', PC )
	{
		PC.AlertManager.Alert( eALERT_FriendAlert, "Testing", "Controller " $ string(PC) );
	}
}

exec function allachievements()
{
	local AchievementDetails AchievDetails;
	local float Current, Max;
	local int AchId;
	local AlertEvent EventToPush;

	Current = 33;
	Max = 100;

	for(AchId=0; AchId < eGA_MAX; AchId++)
	{
		OnlinePlayerData.AchievementsProvider.GetAchievementDetails(AchId, AchievDetails);
		EventToPush.Title = AchievDetails.AchievementName;
		`Log(AchievDetails.AchievementName);
		EventToPush.SubText = string( int(Current) ) $ "/" $ string( int(Max) );
		EventToPush.PercentComplete = Current/Max;
		EventToPush.CustomIcon=OnlinePlayerData.AchievementsProvider.GetAchievementIconPathName(AchId);

		AlertManager.Alert(eALERT_Progress, EventToPush.Title, EventToPush.SubText, EventToPush.PercentComplete, EventToPush.CustomIcon);
	}
}

exec function alert(EAlertType type, string Title, string Subtitle, float Progress, string CustomIcon)
{
	if (AlertManager == None)
	{
		`Log("Oh no! ProgressMgr == None.");
	}
	AlertManager.Alert(type, Title, Subtitle, Progress, CustomIcon);
}

exec function progtest( EGearAchievement Ach, int Current )
{
	local float Current_dump, Max;
	local string AchievementIcon;

	GetAchievementProgression(Ach, Current_dump, Max);
	AchievementIcon = OnlinePlayerData.AchievementsProvider.GetAchievementIconPathName(Ach);

	if (AlertManager.ShouldShowProgressionAlert(Ach, Current, Max))
	{
		`Log("Would show." @ string(Ach) @ string(Current) @ string(Max) );
		AlertManager.Alert(eALERT_Progress, "ProgressTest", string(Current)$"/"$string(Max), Current/Max, AchievementIcon);
	}
	else
	{
		`Log("No show." @ string(Ach) @ string(Current) @ string(Max) );
	}
}

exec function progstat( EGearAchievement Ach )
{
	local float Current, Max;
	local AchievementDetails AchievDetails;

	GetAchievementProgression(Ach, Current, Max);
	OnlinePlayerData.AchievementsProvider.GetAchievementDetails(Ach, AchievDetails);

	`Log("Achievement Stats:" @ AchievDetails.AchievementName @ "["$ string(Ach) $"]" @ string(Current) $ "/" $ string(Max) );
}

exec function supalert(string FriendName)
{
	AlertManager.Alert(eALERT_FriendAlert, "Invitation Sent", "What's up, " $ FriendName);
}

exec function supalertall(int NumFriends)
{
	AlertManager.Alert(eALERT_FriendAlert, "Invitation Sent", "What's up to " $ string(NumFriends) $ " e-friends.");
}

exec function loctest( string File, string Section, string LocString )
{
	//static function bool ShowUIMessage( name SceneTag, string Title, string Message, string Question, array<name> ButtonAliases, delegate<UIMessageBoxBase.OnOptionSelected> SelectionCallback, optional LocalPlayer ScenePlayerOwner, optional out UIMessageBoxBase out_CreatedScene, optional byte ForcedPriority )

	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local UIMessageBoxBase InstancedScene;

	ButtonAliases.AddItem('GenericAccept');

	GameSceneClient = class'UIRoot'.static.GetSceneClient();

	GameSceneClient.ClearUIMessageScene('LocTestingMessage');

	GameSceneClient.ShowUIMessage(	'LocTestingMessage', "Testing Loc",
									Localize( Section, LocString, File ), "", ButtonAliases, None, LocalPlayer(Player), InstancedScene);

	InstancedScene.bCloseOnLevelChange = false;
	InstancedScene.bExemptFromAutoClose = true;
	InstancedScene.SceneStackPriority++;
}

exec function TestSkorgeDuel(optional ESkorgeStage InStage, optional BYTE ForcedOutcome)
{
	local AIController AI;
	local GearPawn_LocustSkorgeBase	Skorge;
	local GearPC GPC;
	local AnimSet	MarcusSkorgeDuelAnimSet;

	AI = SpawnPawn("skorge", FALSE);
	Skorge = GearPawn_LocustSkorgeBase(AI.Pawn);

	if( Skorge == None )
	{
		`warn("Skorge couldn't be spawned!");
		return;
	}

	Skorge.Stage = InStage;
	Skorge.ForcedOutcome = ForcedOutcome;

	`DLog("Stage:" @ Skorge.Stage @ " (Staff=0,TwoStick=1,OneStick=2), Outcome:" @ Skorge.ForcedOutcome @ "(Normal=0,WIN=1,LOSE=2,DRAW=4)");

	// Force that locust into a duel.
	foreach WorldInfo.LocalPlayerControllers(class'GearPC', GPC)
	{
		if( GPC.MyGearPawn != None )
		{
			MarcusSkorgeDuelAnimSet = AnimSet(DynamicLoadObject("COG_MarcusFenix.Animations.Skorge_Chainsaw_Duel",class'AniMSet'));
			GPC.MyGearPawn.KismetAnimSets[0] = MarcusSkorgeDuelAnimSet;
			GPC.MyGearPawn.UpdateAnimSetList();
			Skorge.DoSpecialMove(SM_ChainsawDuel_Leader, TRUE, GPC.MyGearPawn);
			break;
		}
	}
}

exec function ListBrokenCover()
{
	local CoverLink Link;
	local int SlotIdx, ActionIdx;

	`log( GetFuncName()@"..." );

	for( Link = WorldInfo.CoverList; Link != None; Link = Link.NextCoverLink )
	{
		for( SlotIdx = 0; SlotIdx < Link.Slots.Length; SlotIdx++ )
		{
			if( Link.Slots[SlotIdx].CoverType == CT_Standing )
			{
				for( ActionIdx = 0; ActionIdx < Link.Slots[SlotIdx].Actions.Length; ActionIdx++ )
				{
					if( Link.Slots[SlotIdx].Actions[ActionIdx] == CA_PopUp )
					{
						`log( "Popup action on standing cover!!!!"@Link@SlotIdx@ActionIdx );
					}
				}
			}
		}
	}
}


exec function ForceTeleport()
{
	Pawn.SetLocation(vect(0,0,0));
}

/** JoeG function to display what's going on in matchmaking */
exec function ShowMMState()
{
	ConsoleCommand("displayall gearpartygame_base MatchmakingState");
	ConsoleCommand("displayall GearPartyGameSettings LocalizedStringSettings");
	ConsoleCommand("displayall PartyBeaconHost Reservations");
	ConsoleCommand("displayall PartyBeaconHost NumConsumedReservations");
	ConsoleCommand("displayall PartyBeaconHost PotentialTeamChoices");
}

exec function DisplayCoverClaims()
{
	local CoverLink CurLink;
	local int i;
	foreach AllActors(class'CoverLink',CurLink)
	{
		for(i=0;i<CurLink.Slots.length;i++)
		{
			if( CurLink.Slots[i].SlotOwner != none )
			{
				`log(CurLink@"Slot"@i@"("$CurLink.Slots[i].SlotMarker$") Owner"@CurLink.Slots[i].SlotOwner);
			}
		}
	}
}

defaultproperties
{
	SpawnPawnList.Add( (PawnName="boomer",ClassName="GearGameContent.GearPawn_LocustBoomer",AIClass=class'GearAI_Boomer') )
	SpawnPawnList.Add( (PawnName="gatlingboomer",ClassName="GearGameContent.GearPawn_LocustBoomerGatling",AIClass=class'GearAI_Boomer_Gatling') )
	SpawnPawnList.Add( (PawnName="grinder",ClassName="GearGameContent.GearPawn_LocustBoomerGatling",AIClass=class'GearAI_Boomer_Gatling') )
	SpawnPawnList.Add( (PawnName="flameboomer",ClassName="GearGameContent.GearPawn_LocustBoomerFlame",AIClass=class'GearAI_Boomer_Flame') )
	SpawnPawnList.Add( (PawnName="flailboomer",ClassName="GearGameContent.GearPawn_LocustBoomerFlail",AIClass=class'GearAI_Boomer_Shield') )
	SpawnPawnList.Add( (PawnName="mauler",ClassName="GearGameContent.GearPawn_LocustBoomerFlail",AIClass=class'GearAI_Boomer_Shield') )
	SpawnPawnList.Add( (PawnName="shieldboomer",ClassName="GearGameContent.GearPawn_LocustBoomerFlail",AIClass=class'GearAI_Boomer_Shield') )
	SpawnPawnList.Add( (PawnName="butcherboomer",ClassName="GearGameContent.GearPawn_LocustBoomerButcher",AIClass=class'GearAI_Boomer_Melee') )
	SpawnPawnList.Add( (PawnName="mechanicboomer",ClassName="GearGameContent.GearPawn_LocustBoomerMechanic",AIClass=class'GearAI_Boomer_Melee') )
	SpawnPawnList.Add( (PawnName="wretch",ClassName="GearGameContent.GearPawn_LocustWretch",AIClass=class'GearAI_Wretch') )
	SpawnPawnList.Add( (PawnName="darkwretch",ClassName="GearGameContent.GearPawn_LocustDarkWretch",AIClass=class'GearAI_Wretch') )
	SpawnPawnList.Add( (PawnName="theron",ClassName="GearGameContent.GearPawn_LocustTheron",AIClass=class'GearAI_Theron') )
	SpawnPawnList.Add( (PawnName="guard",ClassName="GearGameContent.GearPawn_LocustPalaceGuard",AIClass=class'GearAI_Theron') )
	SpawnPawnList.Add( (PawnName="palaceguard",ClassName="GearGameContent.GearPawn_LocustPalaceGuard",AIClass=class'GearAI_Theron') )
	SpawnPawnList.Add( (PawnName="kantus",ClassName="GearGameContent.GearPawn_LocustKantus",AIClass=class'GearAI_Kantus') )
	SpawnPawnList.Add( (PawnName="priest",ClassName="GearGameContent.GearPawn_LocustKantus",AIClass=class'GearAI_Kantus') )
	SpawnPawnList.Add( (PawnName="hunter",ClassName="GearGameContent.GearPawn_LocustHunterArmorNoGrenades",AIClass=class'GearAI_Hunter') )
	SpawnPawnList.Add( (PawnName="dom",ClassName="GearGame.GearPawn_COGDom",AIClass=class'GearAI_COGGear') )
	SpawnPawnList.Add( (PawnName="gus",ClassName="GearGameContent.GearPawn_COGGus",AIClass=class'GearAI_COGGear') )
	SpawnPawnList.Add( (PawnName="baird",ClassName="GearGameContent.GearPawn_COGBaird",AIClass=class'GearAI_COGGear') )
	SpawnPawnList.Add( (PawnName="dizzy",ClassName="GearGameContent.GearPawn_COGDizzy",AIClass=class'GearAI_COGGear') )
	SpawnPawnList.Add( (PawnName="tai",ClassName="GearGameContent.GearPawn_COGTai",AIClass=class'GearAI_COGGear') )
	SpawnPawnList.Add( (PawnName="hoffman",ClassName="GearGameContent.GearPawn_COGHoffman",AIClass=class'GearAI_COGGear') )
	SpawnPawnList.Add( (PawnName="marcus",ClassName="GearGame.GearPawn_COGMarcus",AIClass=class'GearAI_COGGear') )
	SpawnPawnList.Add( (PawnName="minh",ClassName="GearGameContent.GearPawn_COGMinh",AIClass=class'GearAI_COGGear') )
	SpawnPawnList.Add( (PawnName="carmine",ClassName="GearGameContent.GearPawn_COGCarmine",AIClass=class'GearAI_COGGear') )
	SpawnPawnList.Add( (PawnName="flamedrone",ClassName="GearGameContent.GearPawn_LocustFlameDrone",AIClass=class'GearAI_FlameDrone') )
	SpawnPawnList.Add( (PawnName="ticker",ClassName="GearGameContent.GearPawn_LocustTicker",AIClass=class'GearAI_Ticker') )
	SpawnPawnList.Add( (PawnName="sire",ClassName="GearGameContent.GearPawn_LocustSire",AIClass=class'GearAI_Sire') )
	SpawnPawnList.Add( (PawnName="skorge",ClassName="GearGameContent.GearPawn_LocustSkorge",AIClass=class'GearAI_Skorge') )
	SpawnPawnList.Add( (PawnName="bloodmount",ClassName="GearGameContent.GearPawn_LocustBloodMountWithDrone",AIClass=class'GearAI_Bloodmount') )
	SpawnPawnList.Add( (PawnName="rockworm",ClassName="GearGameContent.GearPawn_RockWorm",AIClass=class'GearAI_RockWorm') )
	SpawnPawnList.Add( (PawnName="nemaslug",ClassName="GearGameContent.GearPawn_LocustNemaSlug",AIClass=class'GearAI_NemaSlug') )
	SpawnPawnList.Add( (PawnName="securitybot",ClassName="GearGameContent.GearPawn_SecurityBotFlying",AIClass=class'GearAI_SecurityBotFlying') )
	SpawnPawnList.Add( (PawnName="reaver",ClassName="GearGameContent.Vehicle_Reaver",AIClass=class'GearAI_Reaver') )
	SpawnPawnList.Add( (PawnName="franklin",ClassName="GearGameContent.GearPawn_NPCFranklin",AIClass=class'GearAI_NPCCOG') )
	SpawnPawnList.Add( (PawnName="franklinnodreads",ClassName="GearGameContent.GearPawn_NPCCOGFranklinWithNoDreads",AIClass=class'GearAI_NPCCOG') )
	SpawnPawnList.Add( (PawnName="speedy",ClassName="GearGameContent.GearPawn_LocustSpeedy",AIClass=class'GearAI_Speedy') )

	SpawnPawnList.Add( (PawnName="dizzyMP",ClassName="GearGameContent.GearPawn_COGDizzyMP",AIClass=class'GearAI_COGGear') )
	SpawnPawnList.Add( (PawnName="domMP",ClassName="GearGameContent.GearPawn_COGDomMP",AIClass=class'GearAI_COGGear') )
	SpawnPawnList.Add( (PawnName="gusMP",ClassName="GearGameContent.GearPawn_COGGusMP",AIClass=class'GearAI_COGGear') )
	SpawnPawnList.Add( (PawnName="taiMP",ClassName="GearGameContent.GearPawn_COGTaiMP",AIClass=class'GearAI_COGGear') )
}

