#ifndef TestFxKey_H__
#define TestFxKey_H__

#include "FxKey.h"
#include "FxArchive.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"
using namespace OC3Ent::Face;

UNITTEST( FxAnimKey, DefaultConstructor )
{
	FxAnimKey key;
	CHECK( key.GetTime() == 0.0 );
	CHECK( key.GetValue() == 0.0 );
	CHECK( key.GetSlopeIn() == 0.0 );
	CHECK( key.GetSlopeOut() == 0.0 );
}

UNITTEST( FxAnimKey, Constructor )
{
	FxAnimKey key( 1.25f, 0.5f, 0.1f, 0.2f );
	CHECK( key.GetTime() == 1.25f );
	CHECK( key.GetValue() == 0.5f );
	CHECK( key.GetSlopeIn() == 0.1f );
	CHECK( key.GetSlopeOut() == 0.2f );
}

UNITTEST( FxAnimKey, CopyConstructor )
{
	FxAnimKey key( 1.25f, 0.5f, 0.1f, 0.2f );
	FxAnimKey key2( key );
	CHECK( key2.GetTime() == 1.25f );
	CHECK( key2.GetValue() == 0.5f );
	CHECK( key2.GetSlopeIn() == 0.1f );
	CHECK( key2.GetSlopeOut() == 0.2f );
}

UNITTEST( FxAnimKey, Assignment )
{
	FxAnimKey key( 1.25f, 0.5f, 0.1f, 0.2f );
	FxAnimKey key2;
	key2 = key;
	CHECK( key2.GetTime() == 1.25f );
	CHECK( key2.GetValue() == 0.5f );
	CHECK( key2.GetSlopeIn() == 0.1f );
	CHECK( key2.GetSlopeOut() == 0.2f );
}

UNITTEST( FxAnimKey, Comparison )
{
	FxAnimKey key( 1.25f, 0.5f, 0.1f, 0.2f );
	FxAnimKey key2( 1.25f, 0.5f, 0.1f, 0.2f );
	FxAnimKey key3;
	CHECK( key != key3 );
	CHECK( key2 != key3 );
	CHECK( key == key2 );
}

UNITTEST( FxAnimKey, GetsAndSets )
{
	FxAnimKey key;
	key.SetTime( 1.25f );
	key.SetValue( 0.5f );
	key.SetSlopeIn( 0.1f );
	key.SetSlopeOut( 0.2f );
	CHECK( key.GetTime() == 1.25f );
	CHECK( key.GetValue() == 0.5f );
	CHECK( key.GetSlopeIn() == 0.1f );
	CHECK( key.GetSlopeOut() == 0.2f );
}

UNITTEST( FxAnimKey, Serialization )
{
	FxAnimKey toSave( 1.25f, 0.5f, 0.1f, 0.2f );
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
	FxAnimKey loaded;
	loadingArchive << loaded;

	CHECK( toSave == loaded );
#endif
}

#endif
