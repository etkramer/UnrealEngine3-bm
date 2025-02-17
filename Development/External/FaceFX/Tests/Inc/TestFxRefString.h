#ifndef TestFxRefString_H__
#define TestFxRefString_H__

#include "FxRefString.h"
using namespace OC3Ent::Face;

UNITTEST( FxRefString, FxRefString )
{
	FxRefString* myRefString = new FxRefString;
	FxRefString* myOtherRefString = new FxRefString( "OtherRefString" );

	CHECK( myRefString != NULL );
	CHECK( myOtherRefString != NULL );
	CHECK( myRefString->GetReferenceCount() == 0 );
	CHECK( myOtherRefString->GetReferenceCount() == 0 );

	myRefString->AddReference();
	myOtherRefString->AddReference();

	myRefString->SetString( "OriginalRefString" );
	myRefString->AddReference();

	CHECK( myRefString->GetReferenceCount() == 2 );
	CHECK( myOtherRefString->GetReferenceCount() == 1 );

	CHECK( myRefString->GetString() == "OriginalRefString" );
	CHECK( myOtherRefString->GetString() == "OtherRefString" );

	FxUInt32 testRefCount = myOtherRefString->RemoveReference();
	CHECK( testRefCount == 0 );

	testRefCount = myRefString->RemoveReference();
	CHECK( testRefCount == 1 );

	testRefCount = myRefString->RemoveReference();
	CHECK( testRefCount == 0 );
}

#endif
