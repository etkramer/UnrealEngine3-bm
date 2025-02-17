/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVoice extends UTLocalMessage
	abstract;

var Array<SoundNodeWave> TauntSounds;
var Array<SoundNodeWave> MatureTauntSounds;
var SoundNodeWave WeaponTauntSounds[20];
var Array<SoundNodeWave> EncouragementSounds;
var Array<SoundNodeWave> ManDownSounds;
var Array<SoundNodeWave> FlagKillSounds;
var Array<SoundNodeWave> OrbKillSounds;
var Array<SoundNodeWave> AckSounds;
var Array<SoundNodeWave> FriendlyFireSounds;
var Array<SoundNodeWave> GotYourBackSounds;
var Array<SoundNodeWave> NeedOurFlagSounds;
var Array<SoundNodeWave> SniperSounds;
var Array<SoundNodeWave> InPositionSounds;
var Array<SoundNodeWave> HaveFlagSounds;
var Array<SoundNodeWave> HaveOrbSounds;
var Array<SoundNodeWave> UnderAttackSounds;
var Array<SoundNodeWave> AreaSecureSounds;

var SoundNodeWave IncomingSound;
var SoundNodeWave EnemyOrbCarrierSound;
var SoundNodeWave EnemyFlagCarrierSound;
var SoundNodeWave EnemyFlagCarrierHereSound;
var SoundNodeWave EnemyFlagCarrierHighSound;
var SoundNodeWave EnemyFlagCarrierLowSound;
var SoundNodeWave MidfieldSound;
var SoundNodeWave GotOurFlagSound;

/** The set of taunt sounds that we pick from when a specific taunt animation is played. */
struct TauntAnimSound
{
	/** Matches the EmoteTag property of the EmoteInfo struct in FamilyInfo. */
	var	name	EmoteTag;

	/** Indexes into the TauntSounds array. */
	var array<int>	TauntSoundIndex;
};

/** Mapping from taunt animations to sounds to play. */
var array<TauntAnimSound>	TauntAnimSoundMap;

/** Offset into actor specific location speech array */
var int LocationSpeechOffset;

/** Index offsets for message groups */
const TAUNTINDEXSTART = 0;
const WEAPONTAUNTINDEXSTART = 100;
const ENCOURAGEMENTINDEXSTART = 200;
const MANDOWNINDEXSTART = 300;
const FLAGKILLINDEXSTART = 400;
const ORBKILLINDEXSTART = 500;
const ACKINDEXSTART = 600;
const FRIENDLYFIREINDEXSTART = 700;
const GOTYOURBACKINDEXSTART = 800;
const NEEDOURFLAGINDEXSTART = 900;
const SNIPERINDEXINDEXSTART = 1000;
const LOCATIONUPDATEINDEXSTART = 1100;
const INPOSITIONINDEXSTART = 1200;
const ENEMYSTATUSINDEXSTART = 1300;
const KILLEDVEHICLEINDEXSTART = 1400;
const ENEMYFLAGCARRIERINDEXSTART = 1500;
const HOLDINGFLAGINDEXSTART = 1600;
const AREASECUREINDEXSTART = 1700;
const UNDERATTACKINDEXSTART = 1800;
const GOTOURFLAGINDEXSTART = 1900;
const NODECONSTRUCTEDINDEXSTART = 2000;

static function int GetTauntMessageIndex(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype, class<DamageType> DamageType)
{
	local UTBot B;
	local int R, TauntLength;
	local class<UTDamageType> D;
	local UTGame G;

	if ( (FRand() < 0.5) && (UTBot(Sender) != None) )
	{
		G = UTGame(Sender.WorldInfo.Game);
		if ( G != None )
		{
			// look for weapon specific taunt
			D = class<UTDamageType>(DamageType);
			if ( (D != None) && (D.default.CustomTauntIndex >= 0) && (D.default.CustomTauntIndex < 20)
				&& (default.WeaponTauntSounds[D.default.CustomTauntIndex] != None)
				&& (G.WeaponTauntUsed[D.default.CustomTauntIndex] == 0) )
			{
				R = WEAPONTAUNTINDEXSTART + D.Default.CustomTauntIndex;
				G.WeaponTauntUsed[D.default.CustomTauntIndex] = 1;
				return R;
			}
		}
	}
	TauntLength = default.TauntSounds.Length;
	R = Rand(TauntLength);

	B = UTBot(Sender);
	if ( B == None )
	{
		return R + TAUNTINDEXSTART;
	}
	if ( R == B.LastTauntIndex )
	{
		R = (R+1)%TauntLength;
	}
	B.LastTauntIndex = R;

	return R + TAUNTINDEXSTART;
}

static function int GetEncouragementMessageIndex(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	if ( default.EncouragementSounds.Length == 0)
	{
		return -1;
	}
	return ENCOURAGEMENTINDEXSTART + Rand(default.EncouragementSounds.Length);
}

static function int GetManDownMessageIndex(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype, class<DamageType> DamageType)
{
	if ( default.ManDownSounds.Length == 0)
	{
		return -1;
	}
	return MANDOWNINDEXSTART + Rand(default.ManDownSounds.Length);
}

static function int GetFlagKillMessageIndex(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype, class<DamageType> DamageType)
{
	if ( (default.FlagKillSounds.Length == 0) || (FRand() < 0.6) )
	{
		return GetTauntMessageIndex(Sender, Recipient, MessageType, DamageType);
	}
	return FLAGKILLINDEXSTART + Rand(default.FlagKillSounds.Length);
}

static function int GetOrbKillMessageIndex(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype, class<DamageType> DamageType)
{
	if ( (default.OrbKillSounds.Length == 0) || (FRand() < 0.6) )
	{
		return GetTauntMessageIndex(Sender, Recipient, MessageType, DamageType);
	}
	return ORBKILLINDEXSTART + Rand(default.OrbKillSounds.Length);
}

static function int GetAckMessageIndex(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	if ( default.AckSounds.Length == 0)
	{
		return -1;
	}
	return ACKINDEXSTART + Rand(default.AckSounds.Length);
}

static function int GetFriendlyFireMessageIndex(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	if ( (default.FriendlyFireSounds.Length == 0) || (Recipient == None) || (UTPlayerController(Recipient.Owner) == None) )
	{
		return -1;
	}
	UTPlayerController(Recipient.Owner).LastFriendlyFireTime = Sender.WorldInfo.TimeSeconds;

	return FRIENDLYFIREINDEXSTART + Rand(default.FriendlyFireSounds.Length);
}

static function int GetGotYourBackMessageIndex(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	if ( default.GotYourBackSounds.Length == 0)
	{
		return -1;
	}
	return GOTYOURBACKINDEXSTART + Rand(default.GotYourBackSounds.Length);
}

static function int GetNeedOurFlagMessageIndex(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	if ( default.NeedOurFlagSounds.Length == 0)
	{
		return -1;
	}
	return NEEDOURFLAGINDEXSTART + Rand(default.NeedOurFlagSounds.Length);
}

static simulated function ClientReceive(
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	Super.ClientReceive(P, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);
	UTPlayerController(P).PlayAnnouncement(default.class, Switch, RelatedPRI_1, OptionalObject );
}

static function SoundNodeWave AnnouncementSound(int MessageIndex, Object OptionalObject, PlayerController PC)
{
	local UTPickupFactory F;
	local UTGameObjective O;
	local UTCTFFlag Flag;

	if ( MessageIndex < default.TauntSounds.Length )
	{
		return default.TauntSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < 20 )
	{
		return default.WeaponTauntSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < default.EncouragementSounds.Length )
	{
		return default.EncouragementSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < default.ManDownSounds.Length )
	{
		return default.ManDownSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < default.FlagKillSounds.Length )
	{
		return default.FlagKillSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < default.OrbKillSounds.Length )
	{
		return default.OrbKillSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < default.AckSounds.Length )
	{
		return default.AckSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < default.FriendlyFireSounds.Length )
	{
		return default.FriendlyFireSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < default.GotYourBackSounds.Length )
	{
		return default.GotYourBackSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < default.NeedOurFlagSounds.Length )
	{
		return default.NeedOurFlagSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < default.SniperSounds.Length )
	{
		return default.SniperSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < 100 )
	{
		if ( (OptionalObject == None) || (MessageIndex == 10) )
		{
			return default.MidFieldSound;
		}
		O = UTGameObjective(OptionalObject);
		if ( O != None )
		{
			return O.GetLocationSpeechFor(PC, default.LocationSpeechOffset, MessageIndex);
		}
		F = UTPickupFactory(OptionalObject);
		if ( F != None )
		{
			return (default.LocationSpeechOffset < F.LocationSpeech.Length) ? F.LocationSpeech[default.LocationSpeechOffset] : None;
		}
		return default.MidFieldSound;
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < default.InPositionSounds.Length )
	{
		return default.InPositionSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex == 0 )
	{
		// Enemy sound - "incoming", orb/flag carrier, or vehicle
		return EnemySound(PC, OptionalObject);
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex == 0 )
	{
		return KilledVehicleSound(PC, OptionalObject);
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < 100 )
	{
		// ping enemy flag carrier
		Flag = UTCTFFlag(OptionalObject);

		if ( (Flag != None) && PC.WorldInfo.GRI.OnSameTeam(Flag, PC) )
		{
			Flag.LastLocationPingTime = PC.WorldInfo.TimeSeconds;
		}
		// enemy flag carrier here
		if ( MessageIndex == 2 )
			return default.EnemyFlagCarrierHighSound;
		else if ( MessageIndex == 3)
			return default.EnemyFlagCarrierLowSound;

		return default.EnemyFlagCarrierHereSound;
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < 100 )
	{
		if ( MessageIndex < default.HaveOrbSounds.Length )
		{
			return default.HaveOrbSounds[MessageIndex];
		}
		MessageIndex -= 50;
		if ( MessageIndex < 0 )
		{
			return None;
		}
		if ( MessageIndex < default.HaveFlagSounds.Length )
		{
			return default.HaveFlagSounds[MessageIndex];
		}
		return None;
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < default.AreaSecureSounds.Length )
	{
		return default.AreaSecureSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex < default.UnderAttackSounds.Length )
	{
		return default.UnderAttackSounds[MessageIndex];
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	if ( MessageIndex == 0 )
	{
		return default.GotOurFlagSound;
	}
	MessageIndex -= 100;
	if ( MessageIndex < 0 )
	{
		return None;
	}
	return None;
}

static function SoundNodeWave EnemySound(PlayerController PC, object OptionalObject)
{
	local class<UTVehicle> VehicleClass;
	local UTPlayerReplicationInfo PRI;
	local UTPlayerController UTPC;
	local UTCTFFlag Flag;

	VehicleClass = class<UTVehicle>(OptionalObject);

	if ( VehicleClass == None )
	{
		PRI = UTPlayerReplicationInfo(OptionalObject);
		if ( (PRI == None) || !PRI.bHasFlag || (PC.WorldInfo.GRI == None) || (PC.WorldInfo.GRI.GameClass == None) )
		{
			UTPC = UTPlayerController(PC);
			if ( (UTPC != None) && (UTPC.WorldInfo.TimeSeconds - UTPC.LastIncomingMessageTime > 35) )
			{
				UTPC.LastIncomingMessageTime = UTPC.WorldInfo.TimeSeconds;
			return default.IncomingSound;
		}
			return None;
		}

		Flag = UTCTFFlag(PRI.GetFlag());
		if ( Flag != None )
		{
			Flag.LastLocationPingTime = PC.WorldInfo.TimeSeconds;
		}
		
		if ( default.LocationSpeechOffset < 3 )
		{
			return default.EnemyFlagCarrierSound;
		}

		// HACK since these voices can't give location
		if ( (UTPC != None) && (UTPC.WorldInfo.TimeSeconds - UTPC.LastIncomingMessageTime > 25) )
		{
			UTPC.LastIncomingMessageTime = UTPC.WorldInfo.TimeSeconds;
		return default.EnemyFlagCarrierSound;
	}
		return None;
	}

	if ( VehicleClass.default.EnemyVehicleSound.Length > default.LocationSpeechOffset )
	{
		return VehicleClass.default.EnemyVehicleSound[default.LocationSpeechOffset];
	}
	
	UTPC = UTPlayerController(PC);
	if ( (UTPC != None) && (UTPC.WorldInfo.TimeSeconds - UTPC.LastIncomingMessageTime > 35) )
	{
		UTPC.LastIncomingMessageTime = UTPC.WorldInfo.TimeSeconds;
		return default.IncomingSound;
	}
	return None;
}

static function string GetString(
	optional int Switch,
	optional bool bPRI1HUD,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	return "";
}

static function bool AllowVoiceMessage(name MessageType, UTPlayerController PC, PlayerController Recipient)
{
	local float CurrentTime;

	if ( PC.WorldInfo.NetMode == NM_Standalone )
		return true;

	CurrentTime = PC.WorldInfo.TimeSeconds;
	if ( CurrentTime - PC.OldMessageTime < 4 )
	{
		if ( (MessageType == 'TAUNT') || (CurrentTime - PC.OldMessageTime < 1) )
		{
			return false;
		}
	}
	if ( (Recipient != None) && Recipient.IsPlayerMuted(PC.PlayerReplicationInfo.UniqueID) )
	{
		return false;
	}
	if ( CurrentTime - PC.OldMessageTime < 6 )
		PC.OldMessageTime = CurrentTime + 3;
	else
		PC.OldMessageTime = CurrentTime;

	return true;
}

 /** Used to play a voice taunt that matches a taunt animation. */
 static function SendVoiceForTauntAnim(Controller Sender, Name EmoteTag, bool bOnlyTeam)
 {
	local UTPlayerController PC;
	local UTPlayerReplicationInfo SenderPRI;
	local int SpeechIndex;

	SenderPRI = UTPlayerReplicationInfo(Sender.PlayerReplicationInfo);
	if ( SenderPRI != None )
	{
		if ( Left(string(EmoteTag),5) ~= "Taunt" )
		{
			SpeechIndex = Rand(100);
		}
		else if ( EmoteTag == 'Encouragement' )
		{ 
			SpeechIndex = GetEncouragementMessageIndex(Sender, None, '');
		}
		else if ( EmoteTag == 'Ack' )
		{ 
			SpeechIndex = GetAckMessageIndex(Sender, None, '');
		}
		else if ( EmoteTag == 'InPosition' )
		{ 
			SpeechIndex = INPOSITIONINDEXSTART + Rand(default.InPositionSounds.Length);
		}
		else if ( EmoteTag == 'UnderAttack' )
		{ 
			SpeechIndex = UNDERATTACKINDEXSTART + Rand(default.UnderAttackSounds.Length);
		}
		else if ( EmoteTag == 'AreaSecure' )
		{ 
			SpeechIndex = AREASECUREINDEXSTART + Rand(default.AreaSecureSounds.Length);
		}

		foreach Sender.WorldInfo.AllControllers(class'UTPlayerController', PC)
		{
			if (!bOnlyTeam || Sender.WorldInfo.GRI.OnSameTeam(Sender, PC))
			{
				PC.ReceiveTauntMessage( SenderPRI, EmoteTag, SpeechIndex);
			}
		}
	}
 }

/** Used to play a voice taunt that matches a taunt animation. */
static function ClientPlayTauntAnim(UTPlayerController PC, PlayerReplicationInfo Sender, Name EmoteTag, int Seed)
{
	local int i, TauntIndex;

	if ( Seed < 100 )
	{
	// Look for this EmoteTag, to find appropriate set of voice taunts
	TauntIndex = -1;
	for(i=0; i<default.TauntAnimSoundMap.length; i++)
	{
		if (default.TauntAnimSoundMap[i].EmoteTag == EmoteTag)
		{
			// Pick one randomly
 				Seed = Min(Seed * default.TauntAnimSoundMap[i].TauntSoundIndex.length/100, default.TauntAnimSoundMap.Length);
				TauntIndex = TAUNTINDEXSTART + default.TauntAnimSoundMap[i].TauntSoundIndex[Seed];
			continue;
		}
	}
	}
	else
	{
		TauntIndex = Seed;
	}

	// If found one, play it
	if(TauntIndex != -1)
	{
		PC.ReceiveLocalizedMessage( default.Class, TauntIndex, Sender );
	}
}

/**
  *
  */
static function SendVoiceMessage(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype, class<DamageType> DamageType)
{
	local UTPlayerController PC, SenderPC, RecipientPC;
	local int MessageIndex;
	local bool bFoundFriendlyPlayer;
	local UTPlayerReplicationInfo SenderPRI;

	// Can message be sent?
	SenderPRI = UTPlayerReplicationInfo(Sender.PlayerReplicationInfo);
	if ( SenderPRI == None )
	{
		return;
	}
	SenderPC = UTPlayerController(Sender);
	RecipientPC = (Recipient != None) ? UTPlayerController(Recipient.Owner) : None;

	// early out if not sending to any players
	if ( (RecipientPC == None) && Sender.WorldInfo.Game.bTeamGame )
	{
		// make sure have players on my team
		foreach Sender.WorldInfo.AllControllers(class'UTPlayerController', PC)
		{
			if ( (Sender.PlayerReplicationInfo != None) && (PC.PlayerReplicationInfo != None) 
				&& (Sender.PlayerReplicationInfo.Team == PC.PlayerReplicationInfo.Team) && (Sender != PC) )
			{
				bFoundFriendlyPlayer = true;
				break;
			}
		}
		if ( !bFoundFriendlyPlayer )
		{
			return;
		}
	}
	if ( (SenderPC != None) && !AllowVoiceMessage(MessageType, SenderPC, RecipientPC) )
	{
		return;
	}

	MessageIndex = GetMessageIndex(Sender, Recipient, MessageType, DamageType);
	if ( MessageIndex == -1 )
	{
		// already handled special case (like status)
		return;
	}
	if ( Recipient != None )
	{
		if ( RecipientPC != None )
		{
			RecipientPC.ReceiveBotVoiceMessage(SenderPRI, MessageIndex, None);
		}
		return;
	}

	foreach Sender.WorldInfo.AllControllers(class'UTPlayerController', PC)
	{
		if ( ((PC == Sender) || !Sender.WorldInfo.Game.bTeamGame || Sender.WorldInfo.GRI.bMatchIsOver || ((Sender.PlayerReplicationInfo != None) && (PC.PlayerReplicationInfo != None) && (Sender.PlayerReplicationInfo.Team == PC.PlayerReplicationInfo.Team)))
			&& !PC.IsPlayerMuted(Sender.PlayerReplicationInfo.UniqueID) )
		{
				PC.ReceiveBotVoiceMessage(SenderPRI, MessageIndex, None);
		}
	}
}

static function int GetMessageIndex(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype, class<DamageType> DamageType)
{
    switch (Messagetype)
    {
		case 'TAUNT':
			return GetTauntMessageIndex(Sender, Recipient, MessageType, DamageType);

		case 'INJURED':
			InitCombatUpdate(Sender, Recipient, MessageType);
			return -1;

		case 'STATUS':
			InitStatusUpdate(Sender, Recipient, MessageType);
			return -1;

		case 'INCOMING':
		case 'INCOMINGVEHICLE':
			SendEnemyStatusUpdate(Sender, Recipient, MessageType);
			return -1;

		case 'LOCATION':
			SendLocationUpdate(Sender, Recipient, MessageType, UTGame(Sender.WorldInfo.Game), Sender.Pawn);
			return -1;

		case 'INPOSITION':
			SendInPositionMessage(Sender, Recipient, MessageType);
			return -1;

		case 'MANDOWN':
			return GetManDownMessageIndex(Sender, Recipient, MessageType, DamageType);

		case 'FRIENDLYFIRE':
			return GetFriendlyFireMessageIndex(Sender, Recipient, MessageType);

		case 'ENCOURAGEMENT':
			return GetEncouragementMessageIndex(Sender, Recipient, MessageType);

		case 'ORBKILL':
			return GetOrbKillMessageIndex(Sender, Recipient, MessageType, DamageType);

		case 'FLAGKILL':
			return GetFlagKillMessageIndex(Sender, Recipient, MessageType, DamageType);

		case 'ACK':
			return GetAckMessageIndex(Sender, Recipient, MessageType);

		case 'SNIPER':
			InitSniperUpdate(Sender, Recipient, MessageType);
			return -1;

		case 'GOTYOURBACK':
			return GetGotYourBackMessageIndex(Sender, Recipient, MessageType);

		case 'HOLDINGFLAG':
			SetHoldingFlagUpdate(Sender, Recipient, MessageType);
			return -1;

		case 'GOTOURFLAG':
			return GOTOURFLAGINDEXSTART;

		case 'NEEDOURFLAG':
			return GetNeedOurFlagMessageIndex(Sender, Recipient, MessageType);

		case 'ENEMYFLAGCARRIERHERE':
			SendEnemyFlagCarrierHereUpdate(Sender, Recipient, MessageType);
			return -1;

		case 'VEHICLEKILL':
			SendKilledVehicleMessage(Sender, Recipient, MessageType);
			return -1;
	}
	return -1;
}

static function InitStatusUpdate(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	local UTBot B;
	local name BotOrders;

	B = UTBot(Sender);
	if ( B != None )
	{
		if ( B.Pawn == None )
		{
			return;
		}
		BotOrders = B.GetOrders();
		if ( (BotOrders == 'defend') || (BotOrders == 'hold') )
		{
			if ( (UTDefensePoint(B.Pawn.Anchor) != None)
				|| ((B.Squad.SquadObjective != None) &&
					(VSizeSq(B.Pawn.Location - B.Squad.SquadObjective.Location) < Square(B.Squad.SquadObjective.BaseRadius))) )
			{
				InitCombatUpdate(Sender, Recipient, MessageType);
				return;
			}
		}
	}
	if ( SendLocationUpdate(Sender, Recipient, Messagetype, UTGame(Sender.WorldInfo.Game), Sender.Pawn, true) || (B == None) )
	{
		return;
	}

	InitCombatUpdate(Sender, Recipient, MessageType);
}

static function InitCombatUpdate(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	local int MessageIndex;
	local UTPlayerController PC;

	if ( Sender.Enemy == None )
	{
		if ( default.AreaSecureSounds.Length == 0 )
		{
			return;
		}
		MessageIndex = AREASECUREINDEXSTART + Rand(default.AreaSecureSounds.Length);
	}
	else
	{
		if ( default.UnderAttackSounds.Length == 0 )
		{
			return;
		}
		ForEach Sender.WorldInfo.AllControllers(class'UTPlayerController', PC )
		{
			if ( Sender.WorldInfo.TimeSeconds - PC.LastCombatUpdateTime < 25 )
			{
				return;
			}
			PC.LastCombatUpdateTime = Sender.WorldInfo.TimeSeconds;
			break;
		}
		MessageIndex = UNDERATTACKINDEXSTART + Rand(default.UnderAttackSounds.Length);
	}
	SendLocalizedMessage(Sender, Recipient, MessageType, MessageIndex);
}

static function SetHoldingFlagUpdate(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	local int MessageIndex;

	MessageIndex = HOLDINGFLAGINDEXSTART;
	if ( default.HaveFlagSounds.Length == 0 )
	{
		return;
	}
	MessageIndex += 50 + Rand(default.HaveFlagSounds.Length);
	SendLocalizedMessage(Sender, Recipient, MessageType, MessageIndex);
}

static function SendLocalizedMessage(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype, int MessageIndex, optional object LocationObject)
{
	local UTPlayerController PC;
	local UTPlayerReplicationInfo SenderPRI;

	SenderPRI = UTPlayerReplicationInfo(Sender.PlayerReplicationInfo);
	if ( SenderPRI == None )
	{
		return;
	}
	if ( Recipient != None )
	{
		PC = UTPlayerController(Recipient.Owner);
		if ( PC != None )
		{
			PC.ReceiveBotVoiceMessage(SenderPRI, MessageIndex, LocationObject);
		}
	}
	else
	{
		foreach Sender.WorldInfo.AllControllers(class'UTPlayerController', PC)
		{
			if ( (!Sender.WorldInfo.Game.bTeamGame || ((Sender.PlayerReplicationInfo != None) && (PC.PlayerReplicationInfo != None) && (Sender.PlayerReplicationInfo.Team == PC.PlayerReplicationInfo.Team)))
				&& !PC.IsPlayerMuted(Sender.PlayerReplicationInfo.UniqueID) )
			{
				PC.ReceiveBotVoiceMessage(SenderPRI, MessageIndex, LocationObject);
			}
		}
	}
}

static function SendEnemyFlagCarrierHereUpdate(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	local Actor LocationObject;
	local int MessageIndex;
	local UTGame G;
	local Pawn StatusPawn;
	local UTCarriedObject Flag;

	G = UTGame(Sender.WorldInfo.Game);
	StatusPawn = Sender.Enemy;
	if ( StatusPawn == None )
	{
		return;
	}
	Flag = UTPlayerReplicationInfo(StatusPawn.PlayerReplicationInfo).GetFlag();
	if ( Flag == None )
	{
		return;
	}

	if ( (G != None) && G.bTeamGame && (StatusPawn != None) && G.GetLocationFor(StatusPawn, LocationObject, MessageIndex, default.LocationSpeechOffset) )
	{
		if ( (Sender.WorldInfo.TimeSeconds - Flag.LastFlagSeeTime < 20)
			&& (MessageIndex == Flag.LastSeeMessageIndex) )
		{
			// don't repeat same flag carrier message too often
			return;
		}

		Flag.LastFlagSeeTime = Sender.WorldInfo.TimeSeconds;
		Flag.LastSeeMessageIndex = MessageIndex;

		MessageIndex += ENEMYFLAGCARRIERINDEXSTART;
		SendLocalizedMessage(Sender, Recipient, MessageType, MessageIndex, Flag);

		// also send location phrase if near/inside base
		if  ( MessageIndex < 1502 )
		{
			MessageIndex -= 400;
			SendLocalizedMessage(Sender, Recipient, MessageType, MessageIndex, LocationObject);
		}
	}
}

static function InitSniperUpdate(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	local int MessageIndex;
	local UTGame G;

	if ( default.SniperSounds.Length == 0 )
	{
		return;
	}
	MessageIndex = SNIPERINDEXINDEXSTART + Rand(default.SniperSounds.Length);
	SendLocalizedMessage(Sender, Recipient, MessageType, MessageIndex);

	// now play sniper location
	G = UTGame(Sender.WorldInfo.Game);
	if ( (G == None) || (G.Sniper == None) )
	{
		return;
	}
	SendLocationUpdate(Sender, Recipient, Messagetype, G, G.Sniper);
}

static function SendEnemyStatusUpdate(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	local object EnemyObject;
	local UTVehicle V;
	local int MessageIndex;

	if ( Sender.Enemy == None )
	{
		return;
	}

	// possibly say "incoming!" or identify if flag/orb carrier or big vehicle
	MessageIndex = ENEMYSTATUSINDEXSTART;
	EnemyObject = Sender.Enemy.PlayerReplicationInfo;
	if ( (EnemyObject == None) || !Sender.Enemy.PlayerReplicationInfo.bHasFlag )
	{
		// maybe send vehicle class instead
		V = UTVehicle(Sender.Enemy);
		if ( (V != None) && V.bHasEnemyVehicleSound && (V.EnemyVehicleSound.Length > default.LocationSpeechOffset) )
		{
			EnemyObject = V.class;
			V.LastEnemyWarningTime = Sender.WorldInfo.TimeSeconds;
		}
		else
		{
			V = None;
		}
	}
	SendLocalizedMessage(Sender, Recipient, MessageType, MessageIndex, EnemyObject);

	if ( (V == None) && !Sender.Enemy.PlayerReplicationInfo.bHasFlag )
	{
		// don't say "incoming" + location
		return;
	}

	// send enemy location update
	SendLocationUpdate(Sender,Recipient, Messagetype, UTGame(Sender.WorldInfo.Game), Sender.Enemy);
}

static function SendKilledVehicleMessage(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	local UTBot B;

	B = UTBot(Sender);
	if ( (B == None) || (B.KilledVehicleClass == None) )
	{
		return;
	}
	SendLocalizedMessage(Sender, Recipient, MessageType, KILLEDVEHICLEINDEXSTART, B.KilledVehicleClass);
}

static function SoundNodeWave KilledVehicleSound(PlayerController PC, object OptionalObject)
{
	local class<UTVehicle> VehicleClass;

	VehicleClass = class<UTVehicle>(OptionalObject);

	if ( VehicleClass == None )
	{
		return None;
	}

	return (VehicleClass.default.VehicleDestroyedSound.Length > default.LocationSpeechOffset)
				? VehicleClass.default.VehicleDestroyedSound[default.LocationSpeechOffset]
				: None;
}

static function bool SendLocationUpdate(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype, UTGame G, Pawn StatusPawn, optional bool bDontSendMidfield)
{
	local Actor LocationObject;
	local int MessageIndex;

	if ( (G != None) && G.bTeamGame && (StatusPawn != None) && G.GetLocationFor(StatusPawn, LocationObject, MessageIndex, default.LocationSpeechOffset) )
	{
		if ( bDontSendMidfield && (MessageIndex == 10) )
		{
			return false;
		}
		MessageIndex += LOCATIONUPDATEINDEXSTART;
		SendLocalizedMessage(Sender, Recipient, MessageType, MessageIndex, LocationObject);
		return true;
	}
	return false;
}

static function SendInPositionMessage(Controller Sender, PlayerReplicationInfo Recipient, Name Messagetype)
{
	if ( default.InPositionSounds.Length > 0)
	{
		SendLocalizedMessage(Sender, Recipient, MessageType, INPOSITIONINDEXSTART + Rand(default.InPositionSounds.Length));
	}
	InitCombatUpdate(Sender, Recipient, MessageType);
}

/**
 * Kill regular voice messages if doing banter, or if there are too many voice messages in front of them
 */
static function bool ShouldBeRemoved(UTQueuedAnnouncement MyAnnouncement, class<UTLocalMessage> NewAnnouncementClass, int NewMessageIndex)
{
	local UTQueuedAnnouncement A;
	local int VoiceMessageCount, MaxCount;

	if ( NewAnnouncementClass == class'UTScriptedVoiceMessage' )
	{
		return true;
	}
	if ( ClassIsChildOf(NewAnnouncementClass, class'UTVoice') )
	{
		// check how many voice messages are between me and end
		MaxCount = ((MyAnnouncement.MessageIndex >= LOCATIONUPDATEINDEXSTART) && (MyAnnouncement.MessageIndex < LOCATIONUPDATEINDEXSTART+100))
					? 0
					: 1;
		For ( A=MyAnnouncement.NextAnnouncement; A!=None; A=A.NextAnnouncement )
		{
			if ( ClassIsChildOf(A.AnnouncementClass, class'UTVoice') )
			{
				VoiceMessageCount++;
				if ( VoiceMessageCount > MaxCount )
				{
					return true;
				}
			}
		}
	}
	return false;
}


/*
 * Don't play voice message if banter is playing.
 *
 */
static function bool AddAnnouncement(UTAnnouncer Announcer, int MessageIndex, optional PlayerReplicationInfo PRI, optional Object OptionalObject)
{
	local UTQueuedAnnouncement A;

	For ( A=Announcer.Queue; A!=None; A=A.NextAnnouncement )
	{
		if ( A.AnnouncementClass == class'UTScriptedVoiceMessage' )
		{
			return false;
		}
	}

	super.AddAnnouncement(Announcer, MessageIndex, PRI, OptionalObject);
	return false;
}

defaultproperties
{
	bShowPortrait=true
	bIsConsoleMessage=false
	AnnouncementDelay=0.75
	AnnouncementPriority=-1
	AnnouncementVolume=2.0
}


