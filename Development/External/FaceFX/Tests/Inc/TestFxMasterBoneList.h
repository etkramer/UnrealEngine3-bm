#ifndef TestFxMasterBoneList_H__
#define TestFxMasterBoneList_H__

#include "FxMasterBoneList.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"
using namespace OC3Ent::Face;

UNITTEST( FxMasterBoneList, Constructor )
{
	FxMasterBoneList mbl;
	CHECK( mbl.GetNumRefBones() == 0 );
}

UNITTEST( FxMasterBoneList, GetNumRefBones )
{
	FxMasterBoneList mbl;
	FxBone refBone( "refBone", FxVec3(), FxQuat(), FxVec3() );
	mbl.AddRefBone( refBone );
	CHECK( mbl.GetNumRefBones() == 1 );
}

UNITTEST( FxMasterBoneList, GetRefBone )
{
	FxMasterBoneList mbl;
	FxBone refBone( "refBone", FxVec3(), FxQuat(), FxVec3() );
	mbl.AddRefBone( refBone );
	CHECK( mbl.GetNumRefBones() == 1 );
	CHECK( mbl.GetRefBone(0) == refBone );
}

UNITTEST( FxMasterBoneList, AddRefBone )
{
	FxMasterBoneList mbl;
	FxBone refBone( "refBone", FxVec3(), FxQuat(), FxVec3() );
	mbl.AddRefBone( refBone );
	CHECK( mbl.GetNumRefBones() == 1 );
	CHECK( mbl.GetRefBone(0) == refBone );
	FxBone refBone2( "refBone2", FxVec3(), FxQuat(), FxVec3() );
	mbl.AddRefBone( refBone2 );
	CHECK( mbl.GetNumRefBones() == 2 );
	CHECK( mbl.GetRefBone(1) == refBone2 );
	FxBone refBone3( "refBone", FxVec3(), FxQuat(), FxVec3(1.0f,1.0f,1.0f) );
	mbl.AddRefBone( refBone3 );
	CHECK( mbl.GetNumRefBones() == 2 );
	CHECK( mbl.GetRefBone(0) == refBone3 );
	CHECK( mbl.GetRefBone(0) != refBone );
}

UNITTEST( FxMasterBoneList, SetRefBoneClientIndex )
{
	FxMasterBoneList mbl;
	FxBone refBone("refBone", FxVec3(), FxQuat(), FxVec3());
	mbl.AddRefBone(refBone);
	CHECK( mbl.GetNumRefBones() == 1 );
	CHECK( mbl.GetRefBone(0) == refBone );
	mbl.SetRefBoneClientIndex(0, 2);
	CHECK( mbl.GetRefBoneClientIndex(0) == 2 );
}

UNITTEST( FxMasterBoneList, LinkAll )
{
	//@todo
}

UNITTEST( FxMasterBoneList, GetBlendedBone )
{
	//@todo
}

UNITTEST( FxMasterBoneList, Serialization )
{
	FxBone refBone( "refBone", FxVec3(0.0f,0.0f,0.1f),
		FxQuat(1.0f,0.1f,0.2f,0.0f), FxVec3(1.0f,1.0f,1.0f) );
	FxMasterBoneList toSave;
	toSave.AddRefBone( refBone );
	CHECK( toSave.GetNumRefBones() == 1 );

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
	FxMasterBoneList loaded;
	loadingArchive << loaded;

	CHECK( toSave.GetNumRefBones() == loaded.GetNumRefBones() );
	CHECK( toSave.GetRefBone(0) == loaded.GetRefBone(0) );
#endif
}

#endif
