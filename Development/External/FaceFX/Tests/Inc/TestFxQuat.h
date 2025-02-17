#ifndef TestFxQuat_H__
#define TestFxQuat_H__

#include "FxQuat.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"
using namespace OC3Ent::Face;

UNITTEST( FxQuat, Constructor )
{
	FxQuat quat;
	CHECK( quat.w == 1.0f );
	CHECK( quat.x == 0.0f );
	CHECK( quat.y == 0.0f );
	CHECK( quat.z == 0.0f );
}

UNITTEST( FxQuat, OtherConstructor )
{
	FxQuat quat( 0.0f, 0.25f, 0.50f, 0.75f );
	CHECK( quat.w == 0.0f );
	CHECK( quat.x == 0.25f );
	CHECK( quat.y == 0.50f );
	CHECK( quat.z == 0.75f );
}

UNITTEST( FxQuat, CopyConstructor )
{
	FxQuat quat( 0.0f, 0.25f, 0.50f, 0.75f );
	FxQuat quat2( quat );
	CHECK( quat.w == quat2.w );
	CHECK( quat.x == quat2.x );
	CHECK( quat.y == quat2.y );
	CHECK( quat.z == quat2.z );
}

UNITTEST( FxQuat, Assignment )
{
	FxQuat quat( 0.0f, 0.25f, 0.50f, 0.75f );
	FxQuat quat2 = quat;
	CHECK( quat.w == quat2.w );
	CHECK( quat.x == quat2.x );
	CHECK( quat.y == quat2.y );
	CHECK( quat.z == quat2.z );
}

UNITTEST( FxQuat, Equality )
{
	FxQuat quat( 0.0f, 0.25f, 0.50f, 0.75f );
	FxQuat quat2( 0.0f, 0.25f, 0.50f, 0.75f );
	CHECK( quat == quat2 );
}

UNITTEST( FxQuat, Inequality )
{
	FxQuat quat( 0.0f, 0.25f, 0.50f, 0.75f );
	FxQuat quat2( 1.0f, 0.26f, 0.53f, 0.01f );
	CHECK( quat != quat2 );
}

UNITTEST( FxQuat, Multiplication )
{
	FxQuat q( 2.0f, -1.0f, 2.0f, 3.0f );
	FxQuat p( 3.0f, 1.0f, -2.0f, 1.0f );
	FxQuat r( 8.0f, -9.0f, -2.0f, 11.0f );
	CHECK( p * q == r );
}

UNITTEST( FxQuat, MultiplyEqual )
{
	FxQuat q( 2.0f, -1.0f, 2.0f, 3.0f );
	FxQuat p( 3.0f, 1.0f, -2.0f, 1.0f );
	FxQuat r( 8.0f, -9.0f, -2.0f, 11.0f );
	p *= q;
	CHECK( p == r );
}

UNITTEST( FxQuat, Inverse )
{
	FxQuat quat( 0.0f, 0.25f, 0.50f, 0.75f );
	FxQuat inv = quat.GetInverse();
	CHECK( inv.w == quat.w );
	CHECK( inv.x == -quat.x );
	CHECK( inv.y == -quat.y );
	CHECK( inv.z == -quat.z );
}

UNITTEST( FxQuat, InverseProperties )
{
	FxQuat q(1.0f,0.456f,0.567f,0.523f);
	q.Normalize();
	FxQuat q1 = q.GetInverse();
	FxQuat identQuat;
	CHECK( q * q1 == identQuat );
	CHECK( q1 * q == identQuat );
	CHECK( q * identQuat == q );
	CHECK( identQuat * q == q );
	CHECK( q1 * identQuat == q1 );
	CHECK( identQuat * q1 == q1 );
}

UNITTEST( FxQuat, Normalization )
{
	FxQuat q(0.0f,0.0f,0.0f,0.0f);
	q.Normalize();
	CHECK( q == FxQuat() );
}

UNITTEST( FxQuat, Alignment )
{
	//@todo
}

UNITTEST( FxQuat, Slerp )
{
	//@todo Why does having instrumentation turned on affect the results of
	//      slerp ever so slightly?!

//	FxQuat q( 0.5f, 0.015f, 0.021f, 0.012f );
//	q.Normalize();
//	FxQuat p(-1.0f,-1.999f,-1.999f,-1.999f);
//	p.Normalize();
//	FxQuat result = q.Slerp(p, 0.5f);
//	FxQuat r(-0.78222442f,-0.35843381f,-0.36577904f,-0.35476118f);
//	CHECK( result == r );

	FxQuat q1( 0.5f, 0.251f, 0.530f, 0.364f );
	q1.Normalize();
	FxQuat p1(1.0f,0.999f,0.999f,0.999f);
	p1.Normalize();
	FxQuat result1 = q1.Slerp(p1, 0.5f);
	FxQuat r1(0.54351521f,0.39718789f,0.56086469f,0.46347994f);
	CHECK( result1 == r1 );

	FxQuat q2( 0.5f, 0.251f, 0.530f, 0.364f );
	q2.Normalize();
	FxQuat p2( 0.5f, 0.251f, 0.530f, 0.364f );
	p2.Normalize();
	FxQuat result2 = q2.Slerp(p2, 0.5f);
	CHECK( result2 == q2 );
}

UNITTEST( FxQuat, Serialization )
{
	FxQuat toSave( 0.0f, 0.5f, 0.1f, 0.2f );
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
	FxQuat loaded;
	loadingArchive << loaded;
	
	CHECK( toSave.w == loaded.w );
	CHECK( toSave.x == loaded.x );
	CHECK( toSave.y == loaded.y );
	CHECK( toSave.z == loaded.z );
#endif
}

#endif
