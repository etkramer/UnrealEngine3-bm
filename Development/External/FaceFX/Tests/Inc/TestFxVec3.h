#ifndef TestFxVec3_H__
#define TestFxVec3_H__

#include "FxVec3.h"
#include "FxArchive.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"

using namespace OC3Ent::Face;

UNITTEST( FxVec3, DefaultConstructor )
{
	FxVec3 test;
	CHECK( test.x == 0.0f && test.y == 0.0f && test.z == 0.0f );
}

UNITTEST( FxVec3, Constructor )
{
	FxVec3 test( 1.0f, 5.2f, 3.14159f );
	CHECK( test.x == 1.0f && test.y == 5.2f && test.z == 3.14159f );
}

UNITTEST( FxVec3, CopyConstruction )
{
	FxVec3 test( 1.0f, 5.2f, 3.14159f );
	FxVec3 copy( test );
	CHECK( copy.x == 1.0f && copy.y == 5.2f && copy.z == 3.14159f );
}

UNITTEST( FxVec3, Assignment )
{
	FxVec3 test( 1.0f, 5.2f, 3.14159f );
	FxVec3 copy = test;
	CHECK( copy.x == 1.0f && copy.y == 5.2f && copy.z == 3.14159f );
}

UNITTEST( FxVec3, EqualityInequalityOperators )
{
	FxVec3 test( 1.0f, 5.2f, 3.14159f );
	CHECK( test == FxVec3( 1.0f, 5.2f, 3.14159f ) );
	CHECK( test != FxVec3( 1.1f, 5.23f, 3.14f ) );
}

UNITTEST( FxVec3, MultiplicationByScalar )
{
	FxVec3 test( 1.0f, 2.0f, 3.0f );
	CHECK( test * 2.0f == FxVec3( 2.0f, 4.0f, 6.0f ) );
	test *= 2.0f;
	CHECK( test == FxVec3( 2.0f, 4.0f, 6.0f ) );
}

UNITTEST( FxVec3, Addition )
{
	FxVec3 test1( 1.0f, 2.0f, 3.0f );
	FxVec3 test2( 3.0f, 2.0f, 1.0f );
	CHECK( test1 + test2 == FxVec3( 4.0f, 4.0f, 4.0f ) );
	test1 += test2;
	CHECK( test1 == FxVec3( 4.0f, 4.0f, 4.0f ) );
}

UNITTEST( FxVec3, Subtraction )
{
	FxVec3 test1( 1.0f, 2.0f, 3.0f );
	FxVec3 test2( 3.0f, 2.0f, 1.0f );
	CHECK( test1 - test2 == FxVec3( -2.0f, 0.0f, 2.0f ) );
	test1 -= test2;
	CHECK( test1 == FxVec3( -2.0f, 0.0f, 2.0f ) );
}

UNITTEST( FxVec3, Negation )
{
	FxVec3 test( 1.0f, 2.0f, 3.0f );
	CHECK( -test == FxVec3( -1.0f, -2.0f, -3.0f ) );
}

UNITTEST( FxVec3, Length )
{
	FxVec3 test( 1.0f, 2.0f, 3.0f );
	CHECK( test.Length() - 3.74165739f < FX_REAL_EPSILON );
}

UNITTEST( FxVec3, Normalize )
{
	FxVec3 test( 1.0f, 0.0f, 0.0f );
	CHECK( test.Normalize() == FxVec3( 1.0f, 0.0f, 0.0f ) );

	FxVec3 test2( 1.0f, 2.0f, 3.0f );
	CHECK( test2.Normalize() == 
		FxVec3( 0.267261242f, 0.534522483f, 0.801783725f ) );
	CHECK( FxAbs(1.0f - FxSqrt( FxSquare( test2.x ) + FxSquare( test2.y ) + 
		FxSquare( test2.z ) ) ) < FX_REAL_EPSILON );
	CHECK( FxAbs(1.0f - test2.Length()) < FX_REAL_EPSILON );
}

UNITTEST( FxVec3, Lerp )
{
	FxVec3 start( 0.0f, 0.0f, 0.0f );
	FxVec3 end( 1.0f, 1.0f, 1.0f );

	CHECK( start.Lerp( end, 0.5 ) == FxVec3( 0.5f, 0.5f, 0.5f ) );
}

UNITTEST( FxVec3, Serialization )
{
	FxVec3 toSave( 0.5f, 0.1f, 0.2f );
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
	FxVec3 loaded;
	loadingArchive << loaded;

	CHECK( toSave == loaded );
#endif
}

#endif
