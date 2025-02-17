#ifndef TestFxVec2_H__
#define TestFxVec2_H__

#include "FxVec2.h"
#include "FxArchive.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"

using namespace OC3Ent::Face;

UNITTEST( FxVec2, DefaultConstructor )
{
	FxVec2 test;
	CHECK( test.x == 0.0f && test.y == 0.0f  );
}

UNITTEST( FxVec2, Constructor )
{
	FxVec2 test( 1.0f, 5.2f );
	CHECK( test.x == 1.0f && test.y == 5.2f  );
}

UNITTEST( FxVec2, CopyConstruction )
{
	FxVec2 test( 1.0f, 5.2f );
	FxVec2 copy( test );
	CHECK( copy.x == 1.0f && copy.y == 5.2f );
}

UNITTEST( FxVec2, Assignment )
{
	FxVec2 test( 1.0f, 5.2f );
	FxVec2 copy = test;
	CHECK( copy.x == 1.0f && copy.y == 5.2f  );
}

UNITTEST( FxVec2, EqualityInequalityOperators )
{
	FxVec2 test( 1.0f, 5.2f);
	CHECK( test == FxVec2( 1.0f, 5.2f ) );
	CHECK( test != FxVec2( 1.1f, 5.23f ) );
}

UNITTEST( FxVec2, MultiplicationByScalar )
{
	FxVec2 test( 1.0f, 2.0f );
	CHECK( test * 2.0f == FxVec2( 2.0f, 4.0f ) );
	test *= 2.0f;
	CHECK( test == FxVec2( 2.0f, 4.0f ) );
}

UNITTEST( FxVec2, Addition )
{
	FxVec2 test1( 1.0f, 2.0f );
	FxVec2 test2( 3.0f, 2.0f );
	CHECK( test1 + test2 == FxVec2( 4.0f, 4.0f ) );
	test1 += test2;
	CHECK( test1 == FxVec2( 4.0f, 4.0f ) );
}

UNITTEST( FxVec2, Subtraction )
{
	FxVec2 test1( 1.0f, 2.0f );
	FxVec2 test2( 3.0f, 2.0f );
	CHECK( test1 - test2 == FxVec2( -2.0f, 0.0f ) );
	test1 -= test2;
	CHECK( test1 == FxVec2( -2.0f, 0.0f ) );
}

UNITTEST( FxVec2, Negation )
{
	FxVec2 test( 1.0f, 2.0f );
	CHECK( -test == FxVec2( -1.0f, -2.0f ) );
}

UNITTEST( FxVec2, Length )
{
	FxVec2 test( 1.0f, 2.0f );
	CHECK( test.Length() - 2.23606798f < FX_REAL_EPSILON );
}

UNITTEST( FxVec2, Normalize )
{
	FxVec2 test( 1.0f, 0.0f );
	CHECK( test.Normalize() == FxVec2( 1.0f, 0.0f ) );

	FxVec2 test2( 1.0f, 2.0f );
	CHECK( test2.Normalize() == 
		FxVec2( 0.447213595f, 0.894427191f ) );
	CHECK( FxAbs(1.0f - FxSqrt( FxSquare( test2.x ) + FxSquare( test2.y )) ) 
		< FX_REAL_EPSILON );
	CHECK( FxAbs(1.0f - test2.Length()) < FX_REAL_EPSILON );
}

UNITTEST( FxVec2, Lerp )
{
	FxVec2 start( 0.0f, 0.0f );
	FxVec2 end( 1.0f, 1.0f );

	CHECK( start.Lerp( end, 0.5 ) == FxVec2( 0.5f, 0.5f ) );
}

UNITTEST( FxVec2, Serialization )
{
	FxVec2 toSave( 0.5f, 0.1f );
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
	FxVec2 loaded;
	loadingArchive << loaded;

	CHECK( toSave == loaded );
#endif
}

#endif
