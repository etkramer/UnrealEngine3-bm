#ifndef TestFxPlatform_H__
#define TestFxPlatform_H__

#include "FxPlatform.h"
#include "FxMath.h"
#include "FxArchive.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"
#include <cmath>

#ifdef _MSC_VER
	#if _MSC_VER > 1200
		using namespace std;
	#endif
#else
	using namespace std;
#endif

using namespace OC3Ent::Face;

UNITTEST( FxPlatform, DataTypeSizes )
{
	CHECK( 1 == sizeof(OC3Ent::Face::FxInt8) );
	CHECK( 1 == sizeof(OC3Ent::Face::FxUInt8) );
	CHECK( 2 == sizeof(OC3Ent::Face::FxInt16) );
	CHECK( 2 == sizeof(OC3Ent::Face::FxUInt16) );
	CHECK( 4 == sizeof(OC3Ent::Face::FxInt32) );
	CHECK( 4 == sizeof(OC3Ent::Face::FxUInt32) );

	CHECK( 4 == sizeof(OC3Ent::Face::FxReal) );
	CHECK( 8 == sizeof(OC3Ent::Face::FxDReal) );
	
#ifndef FX_USE_BUILT_IN_BOOL_TYPE
	CHECK( 4 == sizeof(OC3Ent::Face::FxBool) );
#endif
	
	CHECK( 1 == sizeof(OC3Ent::Face::FxChar) );
	CHECK( 1 == sizeof(OC3Ent::Face::FxUChar) );

	CHECK( 1 == sizeof(OC3Ent::Face::FxByte) );

	CHECK( 4 == sizeof(OC3Ent::Face::FxSize) );
}

UNITTEST( FxPlatform, Serialization )
{
	FxInt8 saveInt8 = -1;
	FxUInt8 saveUInt8 = 1;
	FxInt16 saveInt16 = -2;
	FxUInt16 saveUInt16 = 2;
	FxInt32 saveInt32 = -3;
	FxUInt32 saveUInt32 = 3;
	FxReal saveReal = 0.1f;
	FxDReal saveDReal = 0.2;
	FxBool saveBoolT = FxTrue;
	FxBool saveBoolF = FxFalse;
	FxChar saveChar = 'c';
	FxUChar saveUChar = 'A';
	FxByte saveByte = 'Z';
	FxSize saveSize = 32;

	FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << saveInt8 << saveUInt8 << saveInt16 << saveUInt16 
		             << saveInt32 << saveUInt32
		             << saveReal << saveDReal << saveBoolT << saveBoolF 
		             << saveChar << saveUChar << saveByte << saveSize;
	OC3Ent::Face::FxArchiveStoreMemory* savingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( NULL, 0 );
	OC3Ent::Face::FxArchive savingArchive( savingMemoryStore, OC3Ent::Face::FxArchive::AM_Save );
	savingArchive.Open();
	savingArchive.SetInternalDataState(directoryCreater.GetInternalData());
	
	savingArchive << saveInt8 << saveUInt8 << saveInt16 << saveUInt16 
		          << saveInt32 << saveUInt32
				  << saveReal << saveDReal << saveBoolT << saveBoolF 
				  << saveChar << saveUChar << saveByte << saveSize;
	
	OC3Ent::Face::FxArchiveStoreMemory* loadingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( savingMemoryStore->GetMemory(), savingMemoryStore->GetSize() );
	OC3Ent::Face::FxArchive loadingArchive( loadingMemoryStore, OC3Ent::Face::FxArchive::AM_Load );
	loadingArchive.Open();
#ifdef NO_SAVE_VERSION
	CHECK(loadingArchive.IsValid()==FxFalse);
#else
	FxInt8 loadInt8;
	FxUInt8 loadUInt8;
	FxInt16 loadInt16;
	FxUInt16 loadUInt16;
	FxInt32 loadInt32;
	FxUInt32 loadUInt32;
	FxReal loadReal;
	FxDReal loadDReal;
	FxBool loadBoolT;
	FxBool loadBoolF;
	FxChar loadChar;
	FxUChar loadUChar;
	FxByte loadByte;
	FxSize loadSize;
	
	loadingArchive << loadInt8 << loadUInt8 << loadInt16 << loadUInt16 
				   << loadInt32 << loadUInt32
				   << loadReal << loadDReal << loadBoolT << loadBoolF 
				   << loadChar << loadUChar << loadByte << loadSize;

	CHECK( loadInt8 == saveInt8 );
	CHECK( loadUInt8 == saveUInt8 );
	CHECK( loadInt16 == saveInt16 );
	CHECK( loadUInt16 == saveUInt16 );
	CHECK( loadInt32 == saveInt32 );
	CHECK( loadUInt32 == saveUInt32 );
	CHECK( loadReal == saveReal );
	CHECK( loadDReal == saveDReal );
	CHECK( loadBoolT == saveBoolT );
	CHECK( loadBoolF == saveBoolF );
	CHECK( loadChar == saveChar );
	CHECK( loadUChar == saveUChar );
	CHECK( loadByte == saveByte );
	CHECK( loadSize == saveSize );
#endif
}

UNITTEST( FxPlatform, Math )
{
	FxReal real = 0.0f;
	CHECK( FxSqrt( real ) == sqrt( real ) );
	FxDReal dreal = 0.0;
	CHECK( FxSqrt( dreal ) == sqrt( dreal ) );
	FxInt32 i = 5;
	CHECK( FxAbs( i ) == abs( i ) );
	CHECK( FxAbs( real ) == fabs( real ) );
	CHECK( FxAbs( dreal ) == fabs ( dreal ) );
	CHECK( fabs(FxAcos( real ) - acos( real )) < FX_REAL_EPSILON );
	CHECK( fabs(FxAcos( dreal ) - acos( dreal )) < FX_REAL_EPSILON );
	CHECK( fabs(FxSin( real ) - sin( real )) < FX_REAL_EPSILON );
	CHECK( fabs(FxSin( dreal ) - sin( dreal )) < FX_REAL_EPSILON );
}

#endif
