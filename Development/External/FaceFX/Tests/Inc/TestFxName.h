#ifndef TestFxName_H__
#define TestFxName_H__

#include "FxName.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"

UNITTEST( FxName, DefaultConstructor )
{
	OC3Ent::Face::FxName test;
	CHECK( test == OC3Ent::Face::FxName::NullName );
}

UNITTEST( FxName, Constructor )
{
	OC3Ent::Face::FxName test( "Object Name" );
#ifdef FX_DISALLOW_SPACES_IN_NAMES
	CHECK( test == "Object_Name" );
#else
	CHECK( test == "Object Name" );
#endif
}

UNITTEST( FxName, CopyConstructor )
{
	OC3Ent::Face::FxName test( "Object Name" );
	OC3Ent::Face::FxName copy( test );
#ifdef FX_DISALLOW_SPACES_IN_NAMES
	CHECK( copy == "Object_Name" );
#else
	CHECK( copy == "Object Name" );
#endif
}

UNITTEST( FxName, AssignmentOperator )
{
	OC3Ent::Face::FxName test( "Object Name" );
	OC3Ent::Face::FxName copy( "Dummy" );
	copy = test;

#ifdef FX_DISALLOW_SPACES_IN_NAMES
	CHECK( copy == "Object_Name" );
#else
	CHECK( copy == "Object Name" );
#endif
	CHECK( copy == test );
}

UNITTEST( FxName, EqualityWithName )
{
	OC3Ent::Face::FxName test1( "Head Bone" );
	OC3Ent::Face::FxName test2( "Head Bone" );
	OC3Ent::Face::FxName test3( "Unique Bone" );

	CHECK( test1 == test2 );
	CHECK( test1 != test3 );
}

UNITTEST( FxName, SetName )
{
	OC3Ent::Face::FxName test( "Head Bone" );
	test.SetName( "New Head Bone Name" );
#ifdef FX_DISALLOW_SPACES_IN_NAMES
	CHECK( test == "New_Head_Bone_Name" );
#else
	CHECK( test == "New Head Bone Name" );
#endif

	OC3Ent::Face::FxName dupe1( "BoneName" );
	OC3Ent::Face::FxName dupe2( "BoneName" );
	CHECK( dupe1 == dupe2 && dupe1 == "BoneName" && dupe2 == "BoneName" );

	dupe1.SetName( "NewName" );
	CHECK( dupe1 == "NewName" && dupe2 == "BoneName" );
}

UNITTEST( FxName, Rename )
{
	OC3Ent::Face::FxName test( "BoneName" );
	OC3Ent::Face::FxName test2( test );
	OC3Ent::Face::FxName test3( "ExistingBoneName" );

	CHECK( test == test2 );
	test2.Rename( "NewBoneName" );
	CHECK( test == test2 );
	test2.Rename( "ExistingBoneName" );
	CHECK( test == test2 );
	CHECK( test2 == test3 );
}

UNITTEST( FxName, GetAsString )
{
	OC3Ent::Face::FxName test( "Head Bone" );
#ifdef FX_DISALLOW_SPACES_IN_NAMES
	CHECK( test.GetAsString() == "Head_Bone" );
#else
	CHECK( test.GetAsString() == "Head Bone" );
#endif
}

UNITTEST( FxName, SimpleSerialization )
{
	FxName toSave("this is a test name.");
	FxName toSave2("this is a test name.");

	OC3Ent::Face::FxArchive directoryCreater(OC3Ent::Face::FxArchiveStoreNull::Create(), OC3Ent::Face::FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << toSave << toSave2;
	OC3Ent::Face::FxArchiveStoreMemory* savingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( NULL, 0 );
	OC3Ent::Face::FxArchive savingArchive( savingMemoryStore, OC3Ent::Face::FxArchive::AM_Save );
	savingArchive.Open();
	savingArchive.SetInternalDataState(directoryCreater.GetInternalData());
	savingArchive << toSave << toSave2;
	
	OC3Ent::Face::FxArchiveStoreMemory* loadingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( savingMemoryStore->GetMemory(), savingMemoryStore->GetSize() );
	OC3Ent::Face::FxArchive loadingArchive( loadingMemoryStore, OC3Ent::Face::FxArchive::AM_Load );
	loadingArchive.Open();

#ifdef NO_SAVE_VERSION
	CHECK(loadingArchive.IsValid() == FxFalse);
#else
	FxName loaded, loaded2;
	loadingArchive << loaded << loaded2;

	CHECK( toSave == loaded );
	CHECK( toSave2 == loaded );
	CHECK( loaded == loaded2 );
	CHECK( loaded.GetAsString() == loaded2.GetAsString() );
	CHECK( loaded.GetAsString() == toSave.GetAsString() );
	CHECK( loaded2.GetAsString() == toSave2.GetAsString() );
#endif
}

UNITTEST( FxName, ComplexSerialization )
{
	FxName toSave("this is a test name.");
	FxName toSave2("this is a test name.");

	OC3Ent::Face::FxArchive directoryCreater(OC3Ent::Face::FxArchiveStoreNull::Create(), OC3Ent::Face::FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << toSave << toSave2;
	OC3Ent::Face::FxArchiveStoreMemory* savingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( NULL, 0 );
	OC3Ent::Face::FxArchive savingArchive( savingMemoryStore, OC3Ent::Face::FxArchive::AM_Save );
	savingArchive.Open();
	OC3Ent::Face::FxArchiveStoreMemory* savingMemoryStore2 = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( NULL, 0 );
	OC3Ent::Face::FxArchive savingArchive2( savingMemoryStore2, OC3Ent::Face::FxArchive::AM_Save );
	savingArchive2.Open();
	savingArchive.SetInternalDataState(directoryCreater.GetInternalData());
	savingArchive2.SetInternalDataState(directoryCreater.GetInternalData());
	savingArchive << toSave << toSave2;
	savingArchive2 << toSave << toSave2;
	
	OC3Ent::Face::FxArchiveStoreMemory* loadingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( savingMemoryStore->GetMemory(), savingMemoryStore->GetSize() );
	OC3Ent::Face::FxArchive loadingArchive( loadingMemoryStore, OC3Ent::Face::FxArchive::AM_Load );
	loadingArchive.Open();

	OC3Ent::Face::FxArchiveStoreMemory* loadingMemoryStore2 = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( savingMemoryStore2->GetMemory(), savingMemoryStore2->GetSize() );
	OC3Ent::Face::FxArchive loadingArchive2( loadingMemoryStore2, OC3Ent::Face::FxArchive::AM_Load );
	loadingArchive2.Open();

#ifdef NO_SAVE_VERSION
	CHECK(loadingArchive.IsValid() == FxFalse);
	CHECK(loadingArchive2.IsValid() == FxFalse);
#else
	FxName loaded, loaded2;
	FxName otherLoaded, otherLoaded2;
	loadingArchive << loaded << loaded2;
	loadingArchive2 << otherLoaded << otherLoaded2;

	CHECK( loaded == loaded2 );
	CHECK( loaded == otherLoaded );
	CHECK( loaded == otherLoaded2 );
	CHECK( loaded2 == otherLoaded );
	CHECK( loaded2 == otherLoaded2 );
	CHECK( loaded.GetAsString() == loaded2.GetAsString() );
	CHECK( loaded.GetAsString() == toSave.GetAsString() );
	CHECK( loaded2.GetAsString() == toSave2.GetAsString() );
	CHECK( otherLoaded.GetAsString() == toSave.GetAsString() );
	CHECK( otherLoaded2.GetAsString() == toSave2.GetAsString() );
#endif
}

#endif
