/**
 * Info, the root of all information holding klasses.
 * Doesn't have any movement / collision related code.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Info extends Actor
	abstract
	hidecategories(Movement,Collision)
	native;

//------------------------------------------------------------------------------
// Structs for reporting server state data

struct transient native export KeyValuePair
{
	var() string Key;
	var() string Value;
};

struct transient native export PlayerResponseLine
{
	var() int PlayerNum;
	var() int PlayerID;
	var() string PlayerName;
	var() int Ping;
	var() int Score;
	var() int StatsID;
	var() array<KeyValuePair> PlayerInfo;

};

struct transient native export ServerResponseLine
{
	var() int ServerID;
	var() string IP;
	var() int Port;
	var() int QueryPort;
	var() string ServerName;
	var() string MapName;
	var() string GameType;
	var() int CurrentPlayers;
	var() int MaxPlayers;
	var() int Ping;

	var() array<KeyValuePair> ServerInfo;
	var() array<PlayerResponseLine> PlayerInfo;
};


defaultproperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	RemoteRole=ROLE_None
	bHidden=true
	bOnlyDirtyReplication=true
	bSkipActorPropertyReplication=true
}
