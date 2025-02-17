#ifndef TestFxCurve_H__
#define TestFxCurve_H__

#include "FxAnimCurve.h"
#include "FxFaceGraph.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"
using namespace OC3Ent::Face;

UNITTEST( FxAnimCurve, Constructor )
{
	FxAnimCurve curve;
	CHECK( 0 == curve.GetNumKeys() );
}

UNITTEST( FxAnimCurve, CopyConstructor )
{
	FxAnimCurve curve;
	FxAnimKey key(0.0f, 0.0f, 0.0f, 0.0f);
	FxAnimKey key2(1.0f, 1.0f, 0.5f, 0.5f);
	curve.InsertKey(key);
	curve.InsertKey(key2);
	FxAnimCurve curve2(curve);
	CHECK( 2 == curve.GetNumKeys() );
	CHECK( curve.GetNumKeys() == curve2.GetNumKeys() );
	CHECK( curve.GetKey(0) == curve2.GetKey(0) );
	CHECK( curve.GetKey(1) == curve2.GetKey(1) );
}

UNITTEST( FxAnimCurve, Assignment )
{
	FxAnimCurve curve;
	FxAnimKey key(0.0f, 0.0f, 0.0f, 0.0f);
	FxAnimKey key2(1.0f, 1.0f, 0.5f, 0.5f);
	curve.InsertKey(key);
	curve.InsertKey(key2);
	FxAnimCurve curve2;
	curve2 = curve;
	CHECK( 2 == curve.GetNumKeys() );
	CHECK( curve.GetNumKeys() == curve2.GetNumKeys() );
	CHECK( curve.GetKey(0) == curve2.GetKey(0) );
	CHECK( curve.GetKey(1) == curve2.GetKey(1) );
}

UNITTEST( FxAnimCurve, GetNumKeys )
{
	FxAnimCurve curve;
	FxAnimKey key(0.0f, 1.0f, 0.0f, 0.0f);
	FxAnimKey key2(1.0f, 0.0f, 0.5f, 1.0f);
	curve.InsertKey(key);
	curve.InsertKey(key2);
	CHECK( 2 == curve.GetNumKeys() );
}

UNITTEST( FxAnimCurve, InsertKey )
{
	FxAnimCurve curve;
	FxAnimKey key(0.0f, 1.0f, 0.0f, 0.0f);
	FxAnimKey key2(1.0f, 0.0f, 0.5f, 1.0f);
	FxAnimKey key3(1.25f, 0.f, 0.f, 0.f);
	curve.InsertKey(key2);
	curve.InsertKey(key);
	curve.InsertKey(key3);
	CHECK( 3 == curve.GetNumKeys() );
	CHECK( curve.GetKey(0).GetTime() < curve.GetKey(1).GetTime() );
	CHECK( curve.GetKey(1).GetTime() < curve.GetKey(2).GetTime() );
	CHECK( curve.GetKey(0) == key );
	CHECK( curve.GetKey(1) == key2 );
	CHECK( curve.GetKey(2) == key3 );
}

UNITTEST( FxAnimCurve, RemoveKey )
{
	FxAnimCurve curve;
	FxAnimKey key(0.0f, 1.0f, 0.0f, 0.0f);
	FxAnimKey key2(1.0f, 0.0f, 0.5f, 1.0f);
	FxAnimKey key3(1.25f, 0.f, 0.f, 0.f);
	curve.InsertKey(key2);
	curve.InsertKey(key);
	curve.InsertKey(key3);
	CHECK( 3 == curve.GetNumKeys() );
	curve.RemoveKey(1) ;
	CHECK( 2 == curve.GetNumKeys() );
	CHECK( curve.GetKey(0).GetTime() < curve.GetKey(1).GetTime() );
	CHECK( curve.GetKey(0) == key );
	CHECK( curve.GetKey(1) == key3 );
}

UNITTEST( FxAnimCurve, RemoveAllKeys )
{
	FxAnimCurve curve;
	FxAnimKey key(0.0f, 1.0f, 0.0f, 0.0f);
	FxAnimKey key2(1.0f, 0.0f, 0.5f, 1.0f);
	FxAnimKey key3(1.25f, 0.f, 0.f, 0.f);
	curve.InsertKey(key2);
	curve.InsertKey(key);
	curve.InsertKey(key3);
	CHECK( 3 == curve.GetNumKeys() );
	curve.RemoveAllKeys();
	CHECK( 0 == curve.GetNumKeys() );
}

UNITTEST( FxAnimCurve, GetKey )
{
	FxAnimCurve curve;
	FxAnimKey key(0.0f, 1.0f, 0.0f, 0.0f);
	FxAnimKey key2(1.0f, 0.0f, 0.5f, 1.0f);
	FxAnimKey key3(1.25f, 0.f, 0.f, 0.f);
	curve.InsertKey(key2);
	curve.InsertKey(key);
	curve.InsertKey(key3);
	CHECK( 3 == curve.GetNumKeys() );
	const FxAnimKey& testKey = curve.GetKey(0);
	CHECK( testKey == key );
	CHECK( 3 == curve.GetNumKeys() );
}

UNITTEST( FxAnimCurve, FindKey )
{
	FxAnimCurve curve;
	FxAnimKey key(0.0f, 1.0f, 0.0f, 0.0f);
	FxAnimKey key2(1.0f, 0.0f, 0.5f, 1.0f);
	FxAnimKey key3(1.25f, 0.f, 0.f, 0.f);
	curve.InsertKey(key2);
	curve.InsertKey(key);
	curve.InsertKey(key3);
	
	CHECK( 0 == curve.FindKey(0.0f) );
	CHECK( 2 == curve.FindKey(1.25f) );
	CHECK( 1 == curve.FindKey(1.0f) );
	CHECK( FxInvalidIndex == curve.FindKey(-1.25f) );
	CHECK( FxInvalidIndex == curve.FindKey(10.0f) );
}

UNITTEST( FxAnimCurve, ModifyKey )
{
	FxAnimCurve curve;
	FxAnimKey key(0.0f, 1.0f, 0.0f, 0.0f);
	FxAnimKey key2(1.0f, 0.0f, 0.5f, 1.0f);
	FxAnimKey key3(1.25f, 0.f, 0.f, 0.f);
	curve.InsertKey(key2);
	curve.InsertKey(key);
	curve.InsertKey(key3);
	CHECK( 3 == curve.GetNumKeys() );
	key2.SetTime(2.0f);
	curve.ModifyKey(1, key2);
	CHECK( curve.GetKey(0) == key );
	CHECK( curve.GetKey(1) == key3);
	CHECK( curve.GetKey(2) == key2 );
}

UNITTEST( FxAnimCurve, FindTimeExtents )
{
	FxReal minTime = 0.0f;
	FxReal maxTime = 0.0f;

	// Use separate scopes to avoid conflicts of naming test variables the same
	// Check default case of no keys at all
	{
		FxAnimCurve noKeys;
		noKeys.FindTimeExtents(minTime, maxTime);
		CHECK( minTime == 0.0f );
		CHECK( maxTime == 0.0f );
	}

	// Check the case of one key (at any time, the curve should be that key's value)
	{
		FxAnimCurve oneKey;
		FxAnimKey key(1.0f, 0.5f, 0.0f, 0.0f);
		oneKey.InsertKey(key);
		oneKey.FindTimeExtents(minTime, maxTime);
		CHECK( minTime == 1.0f );
		CHECK( maxTime == 1.0f );
	}

	// Check the case of one piece-wise curve segment
	{
		FxAnimCurve twoKeys;
		FxAnimKey key(0.0f, 0.0f, 0.0f, 0.0f);
		FxAnimKey key2(1.0f, 1.0f, 0.0f, 0.0f);
		twoKeys.InsertKey(key);
		twoKeys.InsertKey(key2);
		twoKeys.FindTimeExtents(minTime, maxTime);
		CHECK( minTime == 0.0f );
		CHECK( maxTime == 1.0f );
	}

	// Check the case of multiple piece-wise curve segments
	{
		FxAnimCurve fourKeys;
		FxAnimKey key(0.0f, 0.0f, 0.0f, 0.0f);
		FxAnimKey key2(1.0f, 1.0f, 0.0f, 0.0f);
		FxAnimKey key3(2.0f, 0.0f, 0.0f, 0.0f);
		FxAnimKey key4(3.0f, 1.0f, 0.0f, 0.0f);
		// Insert them out of order to really test the system
		fourKeys.InsertKey(key3);
		fourKeys.InsertKey(key);
		fourKeys.InsertKey(key4);
		fourKeys.InsertKey(key2);
		fourKeys.FindTimeExtents(minTime, maxTime);
		CHECK( minTime == 0.0f );
		CHECK( maxTime == 3.0f );
	}
}

UNITTEST( FxAnimCurve, FindValueExtents )
{
	FxReal minValue = 0.0f;
	FxReal maxValue = 0.0f;

	// Use separate scopes to avoid conflicts of naming test variables the same
	// Check default case of no keys at all
	{
		FxAnimCurve noKeys;
		noKeys.FindValueExtents(minValue, maxValue);
		CHECK( minValue == 0.0f );
		CHECK( maxValue == 0.0f );
	}

	// Check the case of one key (at any time, the curve should be that key's value)
	{
		FxAnimCurve oneKey;
		FxAnimKey key(1.0f, 0.5f, 0.0f, 0.0f);
		oneKey.InsertKey(key);
		oneKey.FindValueExtents(minValue, maxValue);
		CHECK( minValue == 0.5f );
		CHECK( maxValue == 0.5f );
	}

	// Check the case of one piece-wise curve segment
	{
		FxAnimCurve twoKeys;
		FxAnimKey key(0.0f, 0.0f, 0.0f, 0.0f);
		FxAnimKey key2(1.0f, 1.0f, 0.0f, 0.0f);
		twoKeys.InsertKey(key);
		twoKeys.InsertKey(key2);
		twoKeys.FindValueExtents(minValue, maxValue);
		CHECK( minValue == 0.0f );
		CHECK( maxValue == 1.0f );
	}

	// Check the case of multiple piece-wise curve segments
	{
		FxAnimCurve fourKeys;
		FxAnimKey key(0.0f, 0.0f, 0.0f, 0.0f);
		FxAnimKey key2(1.0f, 1.0f, 0.0f, 0.0f);
		FxAnimKey key3(2.0f, 0.0f, 0.0f, 0.0f);
		FxAnimKey key4(3.0f, 1.0f, 0.0f, 0.0f);
		// Insert them out of order to really test the system
		fourKeys.InsertKey(key3);
		fourKeys.InsertKey(key);
		fourKeys.InsertKey(key4);
		fourKeys.InsertKey(key2);
		fourKeys.FindValueExtents(minValue, maxValue);
		CHECK( minValue == 0.0f );
		CHECK( maxValue == 1.0f );
	}
}

UNITTEST( FxAnimCurve, EvaluateAt )
{
	// Use separate scopes to avoid conflicts of naming test variables the same
	// Check default case of no keys at all
	{
		FxAnimCurve noKeys;
		CHECK( noKeys.EvaluateAt(-1.0f) == 0.0f );
		CHECK( noKeys.EvaluateAt(0.0f) == 0.0f );
		CHECK( noKeys.EvaluateAt(1.0f) == 0.0f );
		CHECK( noKeys.EvaluateAt(10.0f) == 0.0f );
	}

	// Check the case of one key (at any time, the curve should be that key's value)
	{
		FxAnimCurve oneKey;
		FxAnimKey key(1.0f, 0.5f, 0.0f, 0.0f);
		oneKey.InsertKey(key);
		CHECK( oneKey.EvaluateAt(-1.0f) == 0.5f );
		CHECK( oneKey.EvaluateAt(0.0f) == 0.5f );
		CHECK( oneKey.EvaluateAt(1.0f) == 0.5f );
		CHECK( oneKey.EvaluateAt(10.0f) == 0.5f );
	}

	// Check the case of one piece-wise curve segment
	{
		FxAnimCurve twoKeys;
		FxAnimKey key(0.0f, 0.0f, 0.0f, 0.0f);
		FxAnimKey key2(1.0f, 1.0f, 0.0f, 0.0f);
		twoKeys.InsertKey(key);
		twoKeys.InsertKey(key2);
		CHECK( twoKeys.EvaluateAt(-1.0f) == 0.0f );
		CHECK( twoKeys.EvaluateAt(0.0f) == 0.0f );
		CHECK( twoKeys.EvaluateAt(0.5f) == 0.5f );
		CHECK( twoKeys.EvaluateAt(1.0f) == 1.0f );
		CHECK( twoKeys.EvaluateAt(10.0f) == 1.0f );
	}

	// Check the case of times other than zero and one to test the parametric time calculation
	{
		FxAnimCurve twoKeys;
		FxAnimKey key(4.0f, 0.0f, 0.0f, 0.0f);
		FxAnimKey key2(8.0f, 1.0f, 0.0f, 0.0f);
		twoKeys.InsertKey(key);
		twoKeys.InsertKey(key2);
		CHECK( twoKeys.EvaluateAt(-1.0f) == 0.0f );
		CHECK( twoKeys.EvaluateAt(4.0f) == 0.0f );
		CHECK( twoKeys.EvaluateAt(6.0f) == 0.5f );
		CHECK( twoKeys.EvaluateAt(8.0f) == 1.0f );
		CHECK( twoKeys.EvaluateAt(10.0f) == 1.0f );
	}

	// Check the case of tangents other than zero
	{
		FxAnimCurve nonZeroDerivatives;
		FxAnimKey key(0.0f, 0.0f, 0.25f, 0.25f);
		FxAnimKey key2(1.0f, 1.0f, 0.5f, 0.5f);
		nonZeroDerivatives.InsertKey(key);
		nonZeroDerivatives.InsertKey(key2);
		CHECK( nonZeroDerivatives.EvaluateAt(-1.0f) == 0.0f );
		CHECK( nonZeroDerivatives.EvaluateAt(0.0f) == 0.0f );
		CHECK( nonZeroDerivatives.EvaluateAt(0.5f) == 0.46875f );
		CHECK( nonZeroDerivatives.EvaluateAt(1.0f) == 1.0f );
		CHECK( nonZeroDerivatives.EvaluateAt(10.0f) == 1.0f );
	}
	
	// Check the case of multiple piece-wise curve segments
	{
		FxAnimCurve fourKeys;
		FxAnimKey key(0.0f, 0.0f, 0.0f, 0.0f);
		FxAnimKey key2(1.0f, 1.0f, 0.0f, 0.0f);
		FxAnimKey key3(2.0f, 0.0f, 0.0f, 0.0f);
		FxAnimKey key4(3.0f, 1.0f, 0.0f, 0.0f);
		// Insert them out of order to really test the system
		fourKeys.InsertKey(key3);
		fourKeys.InsertKey(key);
		fourKeys.InsertKey(key4);
		fourKeys.InsertKey(key2);
		CHECK( fourKeys.EvaluateAt(-1.0f) == 0.0f );
		CHECK( fourKeys.EvaluateAt(0.0f) == 0.0f );
		CHECK( fourKeys.EvaluateAt(0.5f) == 0.5f );
		CHECK( fourKeys.EvaluateAt(1.0f) == 1.0f );
		CHECK( fourKeys.EvaluateAt(1.5f) == 0.5f );
		CHECK( fourKeys.EvaluateAt(2.0f) == 0.0f );
		CHECK( fourKeys.EvaluateAt(2.5f) == 0.5f );
		CHECK( fourKeys.EvaluateAt(3.0f) == 1.0f );
		// Check out-of-order evaluation
		CHECK( fourKeys.EvaluateAt(1.0f) == 1.0f );
		CHECK( fourKeys.EvaluateAt(10.0f) == 1.0f );
	}

	// Check the case of a key having a different incoming and outgoing slopes
	{
		FxAnimCurve threeKeys;
		FxAnimKey key(0.0f, 0.0f, 0.25f, 0.25f);
		FxAnimKey key2(1.0f, 1.0f, 0.5f, 1.0f);
		FxAnimKey key3(2.0f, 0.0f, 0.25f, 0.0f);
		// Insert them out of order to really test the system
		threeKeys.InsertKey(key3);
		threeKeys.InsertKey(key);
		threeKeys.InsertKey(key2);
		CHECK( threeKeys.EvaluateAt(-1.0f) == 0.0f );
		CHECK( threeKeys.EvaluateAt(0.0f) == 0.0f );
		CHECK( threeKeys.EvaluateAt(0.5f) == 0.46875f );
		CHECK( threeKeys.EvaluateAt(1.0f) == 1.0f );
		CHECK( threeKeys.EvaluateAt(1.5f) == 0.59375f );
		CHECK( threeKeys.EvaluateAt(2.0f) == 0.0f );
		CHECK( threeKeys.EvaluateAt(10.0f) == 0.0f );
	}
}

UNITTEST( FxAnimCurve, Serialization )
{
	// This test no longer works because of the new optimized key serialization.
	// The keys are stored at the FxAnim level instead of at the FxCurve level.
	/*FxAnimCurve toSave;
	FxAnimKey key(0.0f, 1.0f, 0.0f, 0.0f);
	FxAnimKey key2(1.0f, 0.0f, 0.5f, 1.0f);
	FxAnimKey key3(1.25f, 0.f, 0.f, 0.f);
	toSave.InsertKey(key2);
	toSave.InsertKey(key);
	toSave.InsertKey(key3);
	// Do a quick sanity check of the name table
	CHECK( toSave.GetName().GetAsString() == FxName::NullName.GetAsString() );
	CHECK( toSave.GetName() == FxName::NullName );
	toSave.SetName("TestCurve");
	CHECK( 3 == toSave.GetNumKeys() );

	OC3Ent::Face::FxArchive directoryCreater(OC3Ent::Face::FxArchiveStoreNull::Create(), OC3Ent::Face::FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	toSave.Serialize(directoryCreater);
	OC3Ent::Face::FxArchiveStoreMemory* savingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create(NULL, 0);
	OC3Ent::Face::FxArchive savingArchive(savingMemoryStore, OC3Ent::Face::FxArchive::AM_Save);
	savingArchive.Open();
	savingArchive.SetInternalDataState(directoryCreater.GetInternalData());
	toSave.Serialize(savingArchive);

	OC3Ent::Face::FxArchiveStoreMemory* loadingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create(savingMemoryStore->GetMemory(), savingMemoryStore->GetSize());
	OC3Ent::Face::FxArchive loadingArchive(loadingMemoryStore, OC3Ent::Face::FxArchive::AM_Load);
	loadingArchive.Open();

	FxAnimCurve loaded;
	loaded.Serialize(loadingArchive);

	CHECK( loaded.GetNumKeys() == toSave.GetNumKeys() );
	for( FxSize i = 0; i < loaded.GetNumKeys(); ++i )
	{
		CHECK( loaded.GetKey(i) == toSave.GetKey(i) );
	}
	CHECK( loaded.GetName() == toSave.GetName() );
	CHECK( loaded.GetName().GetAsString() == toSave.GetName().GetAsString() );
	CHECK( loaded.GetNumKeys() == toSave.GetNumKeys() );*/
}

#endif
