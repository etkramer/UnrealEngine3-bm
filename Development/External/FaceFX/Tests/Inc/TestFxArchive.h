#ifndef TestFxArchive_H__
#define TestFxArchive_H__

#include "FxArchive.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreFile.h"
#include "FxArchiveStoreMemory.h"

#include "FxString.h"
#include "FxNamedObject.h"

using namespace OC3Ent::Face;

UNITTEST( FxArchive, FileStorage )
{
	FxString myTestString( "testingFileStorage" );
	FxObject* someObject = new FxObject;
	FxObject* someOtherObject = NULL;
	FxInt32 someInt = 31337;
	FxReal someReal = 0.5;
	FxNamedObject* myNamedObject = new FxNamedObject();
	myNamedObject->SetName( "Wow!" );

	FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << myTestString << someObject << someInt
		<< someReal << myNamedObject << someOtherObject;
	FxArchive* myFileSaveArchive = 
		new FxArchive( FxArchiveStoreFile::Create("test.arc"), FxArchive::AM_Save );
	myFileSaveArchive->Open();
	myFileSaveArchive->SetInternalDataState(directoryCreater.GetInternalData());
	*myFileSaveArchive << myTestString << someObject << someInt
		<< someReal << myNamedObject << someOtherObject;

	myTestString.Clear();
	someInt = 0;
	someReal = 0.0;

	if( someObject )
	{
		delete someObject;
		someObject = NULL;
	}

	someOtherObject = NULL;

	if( myNamedObject )
	{
		delete myNamedObject;
		myNamedObject = NULL;
	}

	// Force file to flush and close.
	if( myFileSaveArchive )
	{
		delete myFileSaveArchive;
		myFileSaveArchive = NULL;
	}

	FxObject* myNamedObject2 = NULL;

	FxArchive myFileLoadArchive( FxArchiveStoreFile::Create("test.arc"), FxArchive::AM_Load );
	myFileLoadArchive.Open();
	myFileLoadArchive << myTestString << someObject << someInt
		<< someReal << myNamedObject2 << someOtherObject;

	CHECK( myTestString == "testingFileStorage" );
	CHECK( someObject != NULL );
	CHECK( someInt == 31337 );
	CHECK( someReal == 0.5 );
	CHECK( myNamedObject2->IsA( FxNamedObject::StaticClass() ) );
	CHECK( ((FxNamedObject*)myNamedObject2)->GetName() == "Wow!" );
	CHECK( someOtherObject == NULL );
    
	if( someObject )
	{
		delete someObject;
		someObject = NULL;
	}
	
	if( myNamedObject2 )
	{
		delete myNamedObject2;
		myNamedObject2 = NULL;
	}
}

UNITTEST( FxArchive, MemoryStorage )
{
	FxString myTestString( "testingMemoryStorage" );
	FxObject* someObject = new FxObject;
	FxObject* someOtherObject = NULL;
	FxInt32 someInt = 31337;
	FxReal someReal = 0.5;
	FxNamedObject* myNamedObject = new FxNamedObject();
	myNamedObject->SetName( "Wow!" );

	FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << myTestString << someObject << someInt
		<< someReal << myNamedObject << someOtherObject;
	FxArchiveStoreMemory* myMemoryStoreSave = FxArchiveStoreMemory::Create( NULL, 0 );
	FxArchive myMemorySaveArchive( myMemoryStoreSave, FxArchive::AM_Save );
	myMemorySaveArchive.Open();
	myMemorySaveArchive.SetInternalDataState(directoryCreater.GetInternalData());
	myMemorySaveArchive << myTestString << someObject << someInt
		<< someReal << myNamedObject << someOtherObject;
	
	myTestString.Clear();
	someInt = 0;
	someReal = 0.0;

	if( someObject )
	{
		delete someObject;
		someObject = NULL;
	}

	someOtherObject = NULL;

	if( myNamedObject )
	{
		delete myNamedObject;
		myNamedObject = NULL;
	}

	FxObject* myNamedObject2 = NULL;

	FxArchiveStoreMemory* myMemoryStoreLoad = 
		FxArchiveStoreMemory::Create( myMemoryStoreSave->GetMemory(), 
								      myMemoryStoreSave->GetSize() );
	FxArchive myMemoryLoadArchive( myMemoryStoreLoad, FxArchive::AM_Load );
	myMemoryLoadArchive.Open();
#ifdef NO_SAVE_VERSION
	CHECK(myMemoryLoadArchive.IsValid() == FxFalse);
#else
	myMemoryLoadArchive << myTestString << someObject << someInt
		<< someReal << myNamedObject2 << someOtherObject;

	CHECK( myTestString == "testingMemoryStorage" );
	CHECK( someObject != NULL );
	CHECK( someInt == 31337 );
	CHECK( someReal == 0.5 );
	CHECK( myNamedObject2->IsA( FxNamedObject::StaticClass() ) );
	CHECK( ((FxNamedObject*)myNamedObject2)->GetName() == "Wow!" );
	CHECK( someOtherObject == NULL );

	if( someObject )
	{
		delete someObject;
		someObject = NULL;
	}

	if( myNamedObject2 )
	{
		delete myNamedObject2;
		myNamedObject2 = NULL;
	}
#endif
}

#endif
