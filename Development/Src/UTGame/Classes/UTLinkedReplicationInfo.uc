/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTLinkedReplicationInfo extends ReplicationInfo
	abstract
	native
	nativereplication;

var UTLinkedReplicationInfo NextReplicationInfo;

cpptext
{
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

replication
{
	// Variables the server should send to the client.
	if ( bNetInitial && (Role==ROLE_Authority) )
		NextReplicationInfo;
}

defaultproperties
{
	NetUpdateFrequency=1
}
