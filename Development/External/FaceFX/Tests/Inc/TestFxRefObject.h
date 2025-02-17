#ifndef TestFxRefObject_H__
#define TestFxRefObject_H__

#include "FxRefObject.h"
using namespace OC3Ent::Face;

UNITTEST( FxRefObject, FxRefObject )
{
	FxRefObject* myRefObject = new FxRefObject;
	CHECK( myRefObject != NULL );
	CHECK( myRefObject->GetReferenceCount() == 0 );
	
	FxUInt32 testRefCount = myRefObject->AddReference();
	CHECK( testRefCount == 1 );

	testRefCount = myRefObject->AddReference();
	testRefCount = myRefObject->AddReference();
	testRefCount = myRefObject->AddReference();

	CHECK( testRefCount == 4 );
	
	testRefCount = myRefObject->RemoveReference();
	testRefCount = myRefObject->RemoveReference();
	testRefCount = myRefObject->RemoveReference();

	CHECK( testRefCount == 1 );

	testRefCount = myRefObject->RemoveReference();

	CHECK( testRefCount == 0 );
}

#endif
