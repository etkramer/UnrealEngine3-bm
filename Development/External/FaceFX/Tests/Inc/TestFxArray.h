#ifndef TestFxArray_H__
#define TestFxArray_H__

#include "FxArray.h"
#include "FxArchive.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreMemory.h"

UNITTEST( FxArray, DefaultConstructor )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> test;
	CHECK( test.Length() == 0 );
	CHECK( test.Allocated() == 0 );
}

UNITTEST( FxArray, CopyConstructor )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> test;
	test.PushBack(0);
	test.PushBack(1);
	test.PushBack(2);

	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> copy( test );
	CHECK( copy.Length() == 3 );
	CHECK( copy[0] == test[0] && copy[1] == test[1] && copy[2] == test[2] );
}

UNITTEST( FxArray, AssignmentOperator )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> test;
	test.PushBack(0);
	test.PushBack(1);
	test.PushBack(2);

	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> copy;
	copy = test;
	CHECK( copy.Length() == 3 );
	CHECK( copy[0] == test[0] && copy[1] == test[1] && copy[2] == test[2] );
}

UNITTEST( FxArray, Length )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> test;
	CHECK( test.Length() == 0 );

	test.PushBack( 1 );
	test.PushBack( 1 );
	test.PushBack( 2 );
	test.PushBack( 3 );
	test.PushBack( 5 );
	test.PushBack( 8 );
	test.PushBack( 13 );
	CHECK( test.Length() == 7 );
}

UNITTEST( FxArray, Reserve )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> test;
	test.Reserve( 10 );
	CHECK( test.Allocated() == 10 );
	CHECK( test.Length() == 0 );
}

UNITTEST( FxArray, IsEmpty )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> test;
	CHECK( test.IsEmpty() );
	test.PushBack( 0 );
	CHECK( !test.IsEmpty() );
}

UNITTEST( FxArray, PushBack )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> test;
	test.PushBack( 0 );
	CHECK( test.Allocated() == 1 );
	CHECK( test.Length() == 1 );
	CHECK( test[0] == 0 );

	test.PushBack( 1 );
	test.PushBack( 2 );
	CHECK( test.Allocated() == 4 );
	CHECK( test.Length() == 3 );
	CHECK( test[1] == 1 && test[2] == 2 );

	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> biggerTest;
	biggerTest.Reserve( 100 );
	biggerTest.PushBack( 50 );
	biggerTest.PushBack( 55 );
	CHECK( biggerTest.Allocated() == 100 );
	CHECK( biggerTest.Length() == 2 );
	CHECK( biggerTest[0] == 50 );
	CHECK( biggerTest[1] == 55 );
}

UNITTEST( FxArray, Insert )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> test;
	test.Insert( 0, 0 );
	CHECK( test.Allocated() == 1 );
	CHECK( test.Length() == 1 );
	CHECK( test[0] == 0 );

	test.PushBack( 1 );
	test.PushBack( 3 );
	test.Insert( 2, 2 );
	CHECK( test.Length() == 4 );
	CHECK( test[0] == 0 && test[1] == 1 && test[2] == 2 && test[3] == 3 );

	test.Insert( test.Length(), 4 );
	CHECK( test[ test.Length() - 1 ] == 4 );
}

UNITTEST( FxArray, Swap )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> evens;
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> odds;
	
	evens.Reserve( 3 );
	evens.PushBack( 2 );
	evens.PushBack( 4 );
	evens.PushBack( 6 );

	odds.Reserve( 3 );
	odds.PushBack( 1 );
	odds.PushBack( 3 );
	odds.PushBack( 5 );

	evens.Swap(odds);

	CHECK( evens[0] == 1 && evens[1] == 3 && evens[2] == 5 );
	CHECK( odds[0] == 2 && odds[1] == 4 && odds[2] == 6 );
}

UNITTEST( FxArray, Clear )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> test;
	test.Reserve( 5 );
	test.PushBack( 0 );
	test.PushBack( 1 );
	test.Clear();

	CHECK( test.Length() == 0 && test.Allocated() == 0 );
}

UNITTEST( FxArray, Remove )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> test;
	test.Reserve( 5 );
	test.PushBack( 0 );
	test.PushBack( 1 );
	test.PushBack( 2 );
	test.PushBack( 3 );
	test.PushBack( 4 );
	test.PushBack( 5 );
	CHECK( test.Length() == 6 );
	test.Remove( 0 );

	CHECK( test.Length() == 5 );
	CHECK( test[0] == 1 && test[4] == 5 );

	test.Remove( test.Length() - 1 );

	CHECK( test.Length() == 4 );
	CHECK( test[0] == 1 && test[3] == 4 );
}

UNITTEST( FxArray, Find )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> test;
	test.PushBack( 1 );
	test.PushBack( 5 );
	test.PushBack( 10 );
	test.PushBack( 100 );

	CHECK( test.Find( 1 ) == 0 );
	CHECK( test.Find( 100 ) == 3 );
	CHECK( test.Find( 9999 ) == OC3Ent::Face::FxInvalidIndex );
}

UNITTEST( FxArray, UseAsStack ) // tests back and popback
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> stack;
	stack.Reserve( 3 );
	stack.PushBack( 0 );
	stack.PushBack( 1 );
	stack.PushBack( 2 );
	CHECK( stack.Back() == 2 );
	stack.PopBack();
	CHECK( stack.Back() == 1 );
	CHECK( stack.Length() == 2 );
}

UNITTEST( FxArray, SwapArrayBase )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> test;
	test.Reserve( 3 );
	test.PushBack( 0 );
	test.PushBack( 1 );
	test.PushBack( 2 );

	OC3Ent::Face::FxArrayBase<OC3Ent::Face::FxInt32> newBase(5);
	newBase._v[0] = 5;
	newBase._v[1] = 4;
	newBase._v[2] = 3;
	newBase._v[3] = 2;
	newBase._v[4] = 1;
	newBase._usedCount = 5;
	test.SwapArrayBase(newBase);

	CHECK( test.Length() == 5 );
	CHECK( test[0] == 5 );
	CHECK( test[4] == 1 );

}

UNITTEST( FxArray, Serialization )
{
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> toSave;
	toSave.Reserve( 4 );
	toSave.PushBack( 5 );
	toSave.PushBack( 15 );
	toSave.PushBack( 123 );
	toSave.PushBack( 5934 );

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
	OC3Ent::Face::FxArray<OC3Ent::Face::FxInt32> loaded;
	loadingArchive << loaded;

	CHECK( toSave.Length() == loaded.Length() );
	CHECK( toSave[0] == loaded[0] && toSave[1] == loaded[1] && toSave[2] == loaded[2] && toSave[3] == loaded[3] );
#endif
}

// Create a new class to help test the serialization of arrays of base class
// pointers.
#include "FxObject.h"
#include "FxNamedObject.h"
using namespace OC3Ent::Face;
class myTestArrayClass : public FxObject
{
	FX_DECLARE_CLASS(myTestArrayClass, FxObject);
public:
	myTestArrayClass() {}
	virtual ~myTestArrayClass() {}
};
FX_IMPLEMENT_CLASS(myTestArrayClass,0,FxObject);

UNITTEST( FxArray, SerializationOfArraysOfPointers )
{
	// Register the class.
	myTestArrayClass::StaticClass();

	OC3Ent::Face::FxObject* objectOne = OC3Ent::Face::FxObject::ConstructObject();
	OC3Ent::Face::FxNamedObject* objectTwo = OC3Ent::Face::FxCast<FxNamedObject>(OC3Ent::Face::FxNamedObject::ConstructObject());
	myTestArrayClass* objectThree = FxCast<myTestArrayClass>(myTestArrayClass::ConstructObject());

	OC3Ent::Face::FxArray<OC3Ent::Face::FxObject*> toSave;
	toSave.Reserve( 3 );
	toSave.PushBack( objectOne );
	toSave.PushBack( objectTwo );
	toSave.PushBack( objectThree );
	
	OC3Ent::Face::FxArchive directoryCreater(OC3Ent::Face::FxArchiveStoreNull::Create(), OC3Ent::Face::FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << toSave;
	OC3Ent::Face::FxArchiveStoreMemory* savingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( NULL, 0 );
	OC3Ent::Face::FxArchive savingArchive( savingMemoryStore, OC3Ent::Face::FxArchive::AM_Save );
	savingArchive.Open();
	savingArchive.SetInternalDataState(directoryCreater.GetInternalData());
	savingArchive << toSave;
		
	if( objectOne )
	{
		delete objectOne;
		objectOne = NULL;
	}

	if( objectTwo )
	{
		delete objectTwo;
		objectTwo = NULL;
	}

	if( objectThree )
	{
		delete objectThree;
		objectThree = NULL;
	}

	OC3Ent::Face::FxArchiveStoreMemory* loadingMemoryStore = 
		OC3Ent::Face::FxArchiveStoreMemory::Create( savingMemoryStore->GetMemory(), savingMemoryStore->GetSize() );
	OC3Ent::Face::FxArchive loadingArchive( loadingMemoryStore, OC3Ent::Face::FxArchive::AM_Load );
	loadingArchive.Open();

	OC3Ent::Face::FxArray<OC3Ent::Face::FxObject*> loaded;
    loadingArchive << loaded;

#ifdef NO_SAVE_VERSION
	CHECK(loadingArchive.IsValid() == FxFalse);
#else
	CHECK( toSave.Length() == loaded.Length() );
	CHECK( loaded[0] );
	CHECK( loaded[1] );
	CHECK( loaded[2] );
	CHECK( loaded[0]->IsA( OC3Ent::Face::FxObject::StaticClass() ) );
	CHECK( loaded[1]->IsA( OC3Ent::Face::FxNamedObject::StaticClass() ) );
	CHECK( loaded[2]->IsA( myTestArrayClass::StaticClass() ) );

	objectOne = static_cast<FxObject*>(loaded[0]);
	objectTwo = static_cast<FxNamedObject*>(loaded[1]);
	objectThree = static_cast<myTestArrayClass*>(loaded[2]);

	if( objectOne )
	{
		FxDelete(objectOne);
	}

	if( objectTwo )
	{
		FxDelete(objectTwo);
	}

	if( objectThree )
	{
		FxDelete(objectThree);
	}
#endif
}

#endif
