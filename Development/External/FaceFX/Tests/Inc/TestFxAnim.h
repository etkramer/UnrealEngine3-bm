#ifndef TestFxAnim_H__
#define TestFxAnim_H__

#include "FxAnim.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"

UNITTEST( FxAnim, Constructor )
{
	FxAnim test;
	CHECK( test.GetNumAnimCurves() == 0 );
	CHECK( test.GetName() == FxName::NullName );

	test.SetName( "My First Anim" );
#ifdef FX_DISALLOW_SPACES_IN_NAMES
	CHECK( test.GetName() == "My_First_Anim" );
#else
	CHECK( test.GetName() == "My First Anim" );
#endif
}

UNITTEST( FxAnim, CopyConstruction )
{
	FxAnim test;
	test.SetName( "My First Anim" );

	FxAnim copy( test );
	CHECK( copy.GetName() == test.GetName() );
	CHECK( copy.GetNumAnimCurves() == test.GetNumAnimCurves() );
}

UNITTEST( FxAnim, Assignment )
{
	FxAnim test;
	test.SetName( "My First Anim" );

	FxAnim copy;
	copy = test;

	CHECK( copy.GetName() == test.GetName() );
	CHECK( copy.GetNumAnimCurves() == test.GetNumAnimCurves() );
}

UNITTEST( FxAnim, FindAnimCurve )
{
	FxAnim testAnim;
	FxAnimCurve testCurve;
	testAnim.SetName( "A Multi Curve Animation" );
	testCurve.SetName( "My First Curve" );
	testAnim.AddAnimCurve( testCurve );
	testCurve.SetName( "My Second Curve" );
	testAnim.AddAnimCurve( testCurve );
	testCurve.SetName( "My Third Curve" );
	testAnim.AddAnimCurve( testCurve );

	CHECK( testAnim.FindAnimCurve( "My First Curve" ) == 0 );
	CHECK( testAnim.FindAnimCurve( "My Second Curve" ) == 1 );
	CHECK( testAnim.FindAnimCurve( "My Third Curve" ) == 2 );
	CHECK( testAnim.FindAnimCurve( "The Curve I Did Not Add" ) == FxInvalidIndex );
}

UNITTEST( FxAnim, GetAnimCurve )
{
	FxAnim testAnim;
	FxAnimCurve testCurve;
	testAnim.SetName( "A One Curve Animation" );
	testCurve.SetName( "My First Curve" );

	FxAnimKey key( 0.0f, 1.0f, 0.0f, 0.0f );
	FxAnimKey key2( 1.0f, 0.0f, 0.5f, 1.0f );
	FxAnimKey key3( 1.25f, 0.f, 0.f, 0.f );
	testCurve.InsertKey( key );
	testCurve.InsertKey( key2 );
	testCurve.InsertKey( key3 );
	testAnim.AddAnimCurve( testCurve );

#ifdef FX_DISALLOW_SPACES_IN_NAMES
	CHECK( testAnim.GetAnimCurve( 0 ).GetName() == "My_First_Curve" );
#else
	CHECK( testAnim.GetAnimCurve( 0 ).GetName() == "My First Curve" );
#endif

	CHECK( testAnim.GetAnimCurve( 0 ).GetNumKeys() == 3 );
}

UNITTEST( FxAnim, AddAnimCurve )
{
	FxAnim testAnim;
	testAnim.SetName( "A One Curve Animation" );
	FxAnimCurve testCurve;
	CHECK( testAnim.GetNumAnimCurves() == 0 );

	FxAnimKey key( 0.0f, 1.0f, 0.0f, 0.0f );
	FxAnimKey key2( 1.0f, 0.0f, 0.5f, 1.0f );
	FxAnimKey key3( 1.25f, 0.f, 0.f, 0.f );
	testCurve.InsertKey( key );
	testCurve.InsertKey( key2 );
	testCurve.InsertKey( key3 );
	testAnim.AddAnimCurve( testCurve );
	CHECK( testAnim.GetNumAnimCurves() == 1 );
}

UNITTEST( FxAnim, RemoveCurve )
{
	FxAnim testAnim;
	testAnim.SetName( "A One Curve Animation" );
	FxAnimCurve testCurve;
	CHECK( testAnim.GetNumAnimCurves() == 0 );

	FxAnimKey key( 0.0f, 1.0f, 0.0f, 0.0f );
	FxAnimKey key2( 1.0f, 0.0f, 0.5f, 1.0f );
	FxAnimKey key3( 1.25f, 0.f, 0.f, 0.f );
	testCurve.InsertKey( key );
	testCurve.InsertKey( key2 );
	testCurve.InsertKey( key3 );
	testAnim.AddAnimCurve( testCurve );
	CHECK( testAnim.GetNumAnimCurves() == 1 );

	CHECK( testAnim.RemoveAnimCurve( testCurve.GetName() ) );
	CHECK( testAnim.GetNumAnimCurves() == 0 );
}

UNITTEST( FxAnim, Serialization )
{
    FxAnim toSave;
	FxAnimCurve curve;
	toSave.SetName( "My Serialized Anim" );
	curve.SetName(  "My Serialized Curve" );
	FxAnimKey key( 0.0f, 1.0f, 0.0f, 0.0f );
	FxAnimKey key2( 1.0f, 0.0f, 0.5f, 1.0f );
	FxAnimKey key3( 1.25f, 0.f, 0.f, 0.f );
	curve.InsertKey( key );
	curve.InsertKey( key2 );
	curve.InsertKey( key3 );
	toSave.AddAnimCurve( curve );

	OC3Ent::Face::FxArchive directoryCreater(OC3Ent::Face::FxArchiveStoreNull::Create(), OC3Ent::Face::FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	toSave.Serialize(directoryCreater);
	OC3Ent::Face::FxArchiveStoreMemory* savingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( NULL, 0 );
	OC3Ent::Face::FxArchive savingArchive( savingMemoryStore, OC3Ent::Face::FxArchive::AM_Save );
	savingArchive.Open();
	savingArchive.SetInternalDataState(directoryCreater.GetInternalData());
	toSave.Serialize( savingArchive );
	
	OC3Ent::Face::FxArchiveStoreMemory* loadingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( savingMemoryStore->GetMemory(), savingMemoryStore->GetSize() );
	OC3Ent::Face::FxArchive loadingArchive( loadingMemoryStore, OC3Ent::Face::FxArchive::AM_Load );
	loadingArchive.Open();

#ifdef NO_SAVE_VERSION
	CHECK(loadingArchive.IsValid() == FxFalse);
#else
	FxAnim loaded;
	loaded.Serialize( loadingArchive );

	CHECK( loaded.GetNumAnimCurves() == toSave.GetNumAnimCurves() );
	CHECK( loaded.GetName() == toSave.GetName() );
	CHECK( loaded.GetNameAsString() == toSave.GetNameAsString() );
	CHECK( loaded.GetAnimCurve(0).GetName() == toSave.GetAnimCurve(0).GetName() );
	CHECK( loaded.GetAnimCurve(0).GetNumKeys() == toSave.GetAnimCurve(0).GetNumKeys() );
	for( FxSize i = 0; i < toSave.GetAnimCurve(0).GetNumKeys(); ++i )
	{
		CHECK( loaded.GetAnimCurve(0).GetKey(i) == toSave.GetAnimCurve(0).GetKey(i) );
	}
#endif
}

UNITTEST( FxAnim, FindBoundsAndDuration )
{
	FxAnim testAnim;
	FxAnimCurve testCurve;
	testAnim.SetName( "A One Curve Animation" );
	testCurve.SetName( "My First Curve" );

	FxAnimKey key( -1.0f, 1.0f, 0.0f, 0.0f );
	FxAnimKey key2( 1.0f, 0.0f, 0.5f, 1.0f );
	FxAnimKey key3( 1.25f, 0.f, 0.f, 0.f );
	testCurve.InsertKey( key );
	testCurve.InsertKey( key2 );
	testCurve.InsertKey( key3 );
	testAnim.AddAnimCurve( testCurve );

#ifdef FX_DISALLOW_SPACES_IN_NAMES
	CHECK( testAnim.GetAnimCurve( 0 ).GetName() == "My_First_Curve" );
#else
	CHECK( testAnim.GetAnimCurve( 0 ).GetName() == "My First Curve" );
#endif
	CHECK( testAnim.GetAnimCurve( 0 ).GetNumKeys() == 3 );
	CHECK( FLOAT_EQUALITY( testAnim.GetStartTime(), -1.0f ) );
	CHECK( FLOAT_EQUALITY( testAnim.GetEndTime(), 1.25f ) );
	CHECK( FLOAT_EQUALITY( testAnim.GetDuration(), 2.25f ) );
}

UNITTEST( FxAnim, GetSetTimes )
{
	FxAnim testAnim;
	CHECK( FLOAT_EQUALITY( testAnim.GetBlendInTime(), 0.16f ) );
	CHECK( FLOAT_EQUALITY( testAnim.GetBlendOutTime(), 0.22f ) );
	testAnim.SetBlendInTime( 0.55f );
	testAnim.SetBlendOutTime( 2.37f );
	CHECK( FLOAT_EQUALITY( testAnim.GetBlendInTime(), 0.55f ) );
	CHECK( FLOAT_EQUALITY( testAnim.GetBlendOutTime(), 2.37f ) );
}

UNITTEST( FxAnimGroup, Constructor )
{
	FxAnimGroup test;
	CHECK( test.GetNumAnims() == 0 );
	CHECK( test.GetName() == FxName::NullName );

	test.SetName( "testGroup" );
	CHECK( test.GetName() == "testGroup" );
}

UNITTEST( FxAnimGroup, CopyConstruction )
{
	FxAnimGroup test;
	test.SetName( "testGroup" );

	FxAnimGroup test2( test );
	CHECK( test2.GetName() == test.GetName() );
	CHECK( test2.GetNumAnims() == test.GetNumAnims() );
}

UNITTEST( FxAnimGroup, Assignment )
{
	FxAnimGroup test;
	test.SetName( "testGroup" );

	FxAnimGroup test2;
	test2 = test;
	CHECK( test2.GetName() == test.GetName() );
	CHECK( test2.GetNumAnims() == test.GetNumAnims() );
}

UNITTEST( FxAnimGroup, FindAnim )
{
	FxAnimGroup testGroup;
	FxAnim testAnim;
	testAnim.SetName( "My First Anim" );
	CHECK( testGroup.AddAnim( testAnim ) == FxTrue );
	testAnim.SetName( "My Second Anim" );
	CHECK( testGroup.AddAnim( testAnim ) == FxTrue );
	testAnim.SetName( "My Third Anim" );
	CHECK( testGroup.AddAnim( testAnim ) == FxTrue );
	CHECK( testGroup.FindAnim( "My First Anim" ) == 0 );
	CHECK( testGroup.FindAnim( "My Second Anim" ) == 1 );
	CHECK( testGroup.FindAnim( "My Third Anim" ) == 2 );
	CHECK( testGroup.FindAnim( "The Anim I Did Not Add" ) == FxInvalidIndex );
}

UNITTEST( FxAnimGroup, GetAnim )
{
	FxAnimGroup testGroup;
	FxAnim testAnim;
	testAnim.SetName( "testAnim" );
	CHECK( testGroup.AddAnim( testAnim ) == FxTrue );	
	CHECK( testGroup.GetNumAnims() == 1 );
	CHECK( testGroup.GetAnim(0).GetName() == testAnim.GetName() );
	CHECK( testGroup.GetAnim(0).GetNumAnimCurves() == testAnim.GetNumAnimCurves() );
}

UNITTEST( FxAnimGroup, AddAnim )
{
	FxAnimGroup testGroup;
	FxAnim testAnim;
	testAnim.SetName( "testAnim" );
	CHECK( testGroup.AddAnim( testAnim ) == FxTrue );
	CHECK( testGroup.GetNumAnims() == 1 );
	CHECK( testGroup.GetAnim(0).GetName() == testAnim.GetName() );
	CHECK( testGroup.GetAnim(0).GetNumAnimCurves() == testAnim.GetNumAnimCurves() );
	CHECK( testGroup.AddAnim( testAnim ) == FxFalse );
}

UNITTEST( FxAnimGroup, RemoveAnim )
{
	FxAnimGroup testGroup;
	FxAnim testAnim;
	testAnim.SetName( "testAnim" );
	CHECK( testGroup.AddAnim( testAnim ) == FxTrue );
	CHECK( testGroup.GetNumAnims() == 1 );
	CHECK( testGroup.GetAnim(0).GetName() == testAnim.GetName() );
	CHECK( testGroup.GetAnim(0).GetNumAnimCurves() == testAnim.GetNumAnimCurves() );
	CHECK( testGroup.RemoveAnim( "NotAddedAnim" ) == FxFalse );
	CHECK( testGroup.RemoveAnim( testAnim.GetName() ) == FxTrue );
	CHECK( testGroup.GetNumAnims() == 0 );
}

UNITTEST( FxAnimGroup, Serialization )
{
	FxAnimGroup toSave;
	FxAnim anim;
	FxAnimCurve curve;
	FxAnimCurve curve2;
	toSave.SetName( "My Serialized AnimGroup" );
	anim.SetName( "My Serialized Anim" );
	curve.SetName( "My Serialized Curve" );
	FxAnimKey key( 0.0f, 1.0f, 0.0f, 0.0f );
	FxAnimKey key2( 1.0f, 0.0f, 0.5f, 1.0f );
	FxAnimKey key3( 1.25f, 0.f, 0.f, 0.f );
	curve.InsertKey( key );
	curve.InsertKey( key2 );
	curve.InsertKey( key3 );
	curve2.SetName( "My Serialized Curve 2" );
	FxAnimKey key4( 0.0f, 0.0f, 0.0f, 0.0f );
	FxAnimKey key5( 3.4f, 1.0f, 0.25f, 1.34f );
	FxAnimKey key6( 5.43f, 0.34f, 0.0f, 0.23f );
	FxAnimKey key7( 6.0f, 0.0f, 0.0f, 0.0f );
	curve.InsertKey( key4 );
	curve.InsertKey( key5 );
	curve.InsertKey( key6 );
	curve.InsertKey( key7 );
	anim.AddAnimCurve( curve );
	anim.AddAnimCurve( curve2 );
	
	toSave.AddAnim( anim );
	
	OC3Ent::Face::FxArchive directoryCreater(OC3Ent::Face::FxArchiveStoreNull::Create(), OC3Ent::Face::FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	toSave.Serialize(directoryCreater);
	OC3Ent::Face::FxArchiveStoreMemory* savingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( NULL, 0 );
	OC3Ent::Face::FxArchive savingArchive( savingMemoryStore, OC3Ent::Face::FxArchive::AM_Save );
	savingArchive.Open();
	savingArchive.SetInternalDataState(directoryCreater.GetInternalData());
	toSave.Serialize( savingArchive );

	OC3Ent::Face::FxArchiveStoreMemory* loadingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( savingMemoryStore->GetMemory(), savingMemoryStore->GetSize() );
	OC3Ent::Face::FxArchive loadingArchive( loadingMemoryStore, OC3Ent::Face::FxArchive::AM_Load );
	loadingArchive.Open();

#ifdef NO_SAVE_VERSION
	CHECK(loadingArchive.IsValid() == FxFalse);
#else
	FxAnimGroup loaded;
	loaded.Serialize( loadingArchive );

	CHECK( loaded.GetNumAnims() == toSave.GetNumAnims() );
	CHECK( loaded.GetName() == toSave.GetName() );
	CHECK( loaded.GetNameAsString() == toSave.GetNameAsString() );
	CHECK( loaded.GetAnim(0).GetName() == toSave.GetAnim(0).GetName() );
	CHECK( loaded.GetAnim(0).GetNumAnimCurves() == toSave.GetAnim(0).GetNumAnimCurves() );
	CHECK( loaded.GetAnim(0).GetAnimCurve(0).GetName() == toSave.GetAnim(0).GetAnimCurve(0).GetName() );
	CHECK( loaded.GetAnim(0).GetAnimCurve(0).GetNumKeys() == toSave.GetAnim(0).GetAnimCurve(0).GetNumKeys() );
	for( FxSize i = 0; i < toSave.GetAnim(0).GetAnimCurve(0).GetNumKeys(); ++i )
	{
		CHECK( loaded.GetAnim(0).GetAnimCurve(0).GetKey(i) == toSave.GetAnim(0).GetAnimCurve(0).GetKey(i) );
	}
	CHECK( loaded.GetAnim(0).GetAnimCurve(1).GetName() == toSave.GetAnim(0).GetAnimCurve(1).GetName() );
	CHECK( loaded.GetAnim(0).GetAnimCurve(1).GetNumKeys() == toSave.GetAnim(0).GetAnimCurve(1).GetNumKeys() );
	for( FxSize i = 0; i < toSave.GetAnim(0).GetAnimCurve(1).GetNumKeys(); ++i )
	{
		CHECK( loaded.GetAnim(0).GetAnimCurve(1).GetKey(i) == toSave.GetAnim(0).GetAnimCurve(1).GetKey(i) );
	}
#endif
}

#endif
