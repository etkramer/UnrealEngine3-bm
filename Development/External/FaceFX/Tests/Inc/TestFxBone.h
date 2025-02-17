#ifndef TestFxBone_H__
#define TestFxBone_H__

#include "FxBone.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"
using namespace OC3Ent::Face;

UNITTEST( FxBone, Constructor )
{
	FxBone bone;
	CHECK( bone.GetName() == FxName::NullName );
	CHECK( bone.GetPos() == FxVec3() );
	CHECK( bone.GetRot() == FxQuat() );
	CHECK( bone.GetScale() == FxVec3() );
}

UNITTEST( FxBone, Equality )
{
	FxBone bone( "refBone", FxVec3(0.0f,0.0f,0.1f),
		          FxQuat(1.0f,0.1f,0.2f,0.0f), FxVec3(1.0f,1.0f,1.0f) );
	FxBone bone2( "refBone", FxVec3(0.0f,0.0f,0.1f),
				   FxQuat(1.0f,0.1f,0.2f,0.0f), FxVec3(1.0f,1.0f,1.0f) );
	CHECK( bone == bone2 );
}

UNITTEST( FxBone, SetGetPos )
{
	FxBone bone;
	bone.SetPos( FxVec3(0.0f,0.0f,0.1f) );
	CHECK( bone.GetPos() == FxVec3(0.0f,0.0f,0.1f) );
}

UNITTEST( FxBone, SetGetRot )
{
	FxBone bone;
	bone.SetRot( FxQuat(1.0f,0.1f,0.2f,0.0f) );
	CHECK( bone.GetRot() == FxQuat(1.0f,0.1f,0.2f,0.0f) );
}

UNITTEST( FxBone, SetGetScale )
{
	FxBone bone;
	bone.SetScale( FxVec3(1.0f,1.0f,1.0f) );
	CHECK( bone.GetScale() == FxVec3(1.0f,1.0f,1.0f) );
}

UNITTEST( FxBone, Serialization )
{
	FxBone toSave( "refBone", FxVec3(0.0f,0.0f,0.1f),
				    FxQuat(1.0f,0.1f,0.2f,0.0f), FxVec3(1.0f,1.0f,1.0f) );

	OC3Ent::Face::FxArchive directoryCreater(OC3Ent::Face::FxArchiveStoreNull::Create(), OC3Ent::Face::FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << toSave;
	OC3Ent::Face::FxArchiveStoreMemory* savingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( NULL, 0 );
	OC3Ent::Face::FxArchive savingArchive( savingMemoryStore, OC3Ent::Face::FxArchive::AM_Save );
	savingArchive.Open();
	savingArchive.SetInternalDataState(directoryCreater.GetInternalData());
	savingArchive << toSave;
	
	OC3Ent::Face::FxArchiveStoreMemory* loadingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( savingMemoryStore->GetMemory(), savingMemoryStore->GetSize() );
	OC3Ent::Face::FxArchive loadingArchive( loadingMemoryStore, OC3Ent::Face::FxArchive::AM_Load );
	loadingArchive.Open();

#ifdef NO_SAVE_VERSION
	CHECK(loadingArchive.IsValid() == FxFalse);
#else
	FxBone loaded;
	loadingArchive << loaded;

	CHECK( toSave == loaded );
#endif
}

#endif
