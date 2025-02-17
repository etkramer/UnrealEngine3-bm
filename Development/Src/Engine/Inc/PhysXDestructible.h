/*=============================================================================
	PhysXDestructible.h: Destructible Vertical Component.
	Copyright 2007-2008 AGEIA Technologies.
=============================================================================*/

#ifndef _PHYSXDESTRUCTIBLE_H_
#define _PHYSXDESTRUCTIBLE_H_

#include "Array.h"
#include "UnFracturedStaticMesh.h"
#include "EngineMeshClasses.h"

enum EPhysXDestructibleManagerFlag
{
    DMF_VisualizeSupport    =0,
    DMF_MAX                 =1,
};

class FPhysXDestructibleManager
{
public:
	FPhysXDestructibleManager();
	~FPhysXDestructibleManager();

	//	External interface
	void	SetMaxDynamicChunkCount( INT MaxChunkCount );
	void	SetDebrisLifetime( FLOAT Lifetime );
	void	TickManager( FLOAT DeltaTime );
	void	SetFlag( EPhysXDestructibleManagerFlag Flag, UBOOL bValue );
	UBOOL	GetFlag( EPhysXDestructibleManagerFlag Flag );

private:
	//	Private interface, used by PhysXDestructible classes
	UBOOL							RegisterActor( APhysXDestructibleActor * Actor );

	UPhysXDestructibleStructure *	CreateStructure( const TArray<APhysXDestructibleActor *> & Actors );
	UBOOL							RemoveStructure( UPhysXDestructibleStructure * Structure );

	UBOOL							RegisterChunk( FPhysXDestructibleChunk * Chunk );
	void							AddChunk( FPhysXDestructibleChunk * Chunk );
	struct FPhysXDestructibleChunk *RemoveFirstChunk();
	struct FPhysXDestructibleChunk *RemoveChunk( FPhysXDestructibleChunk * Chunk );
	void							CapDynamicChunkCount( INT ChunkCountCap );

	struct FIFOEntry
	{
		UPhysXDestructibleStructure *	Structure;
		INT	ChunkIndex;
		INT	NextEntry;
		INT	PrevEntry;
	}; 

	INT										DynamicChunkFIFOMax;
	INT										DynamicChunkFIFONum;
	FLOAT									DebrisLifetime;
	INT										FIFOHead;
	INT										FIFOTail;
	TArray<FIFOEntry>						FIFOEntries;

	TArray<UPhysXDestructibleStructure*>	Structures;
	TArray<UPhysXDestructibleStructure*>	StructureKillList;

    INT										Flags;

	friend class	UPhysXDestructibleStructure;
	friend class	APhysXDestructibleActor;
	friend class	APhysXDestructiblePart;
};

#endif // #ifndef _PHYSXDESTRUCTIBLE_H_
