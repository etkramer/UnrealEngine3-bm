#ifndef TestFxClass_H__
#define TestFxClass_H__

#include "FxClass.h"
#include "FxObject.h"

using namespace OC3Ent::Face;

UNITTEST( FxClass, FxClassInterface )
{
	FxClass* myTestClass = new FxClass( "myTestClass", 0, NULL, 8, NULL );

	CHECK( myTestClass->GetName() == "myTestClass" );
	CHECK( myTestClass->GetCurrentVersion() == 0 );
	CHECK( myTestClass->GetBaseClassDesc() == NULL );
	CHECK( myTestClass->GetSize() == 8 );
	
	FxObject* testConstructObject = myTestClass->ConstructObject();
	CHECK( testConstructObject == NULL );

	const FxClass* foundMyTestClass = FxClass::FindClassDesc( "myTestClass" );
	CHECK( foundMyTestClass != NULL );
	CHECK( foundMyTestClass == myTestClass );

	const FxClass* noClass = FxClass::FindClassDesc( "noClass" );
	CHECK( noClass == NULL );
}

// Create a simple class hierarchy for testing the RTTI facilities of
// FxClass.
class myTestBaseClass : public FxObject
{
	FX_DECLARE_CLASS(myTestBaseClass, FxObject);
public:
	myTestBaseClass() {}
	virtual ~myTestBaseClass() {}
};
FX_IMPLEMENT_CLASS(myTestBaseClass,0,FxObject);

class myTestChildClass : public myTestBaseClass
{
	FX_DECLARE_CLASS(myTestChildClass, myTestBaseClass);
public:
	myTestChildClass() {}
	virtual ~myTestChildClass() {}
};
FX_IMPLEMENT_CLASS(myTestChildClass,8,myTestBaseClass);

class myTestGrandchildClass : public myTestChildClass
{
	FX_DECLARE_CLASS(myTestGrandchildClass, myTestChildClass);
public:
	myTestGrandchildClass() {}
	virtual ~myTestGrandchildClass() {}
};
FX_IMPLEMENT_CLASS(myTestGrandchildClass,4,myTestChildClass);

UNITTEST( FxClass, RTTI )
{
	// Register the classes.
	myTestBaseClass::StaticClass();
	myTestChildClass::StaticClass();
	myTestGrandchildClass::StaticClass();

	myTestBaseClass* baseClass = new myTestBaseClass;
	myTestChildClass* childClass = new myTestChildClass;
	myTestGrandchildClass* grandchildClass = new myTestGrandchildClass;
	myTestBaseClass* dynGrandchildClass = new myTestGrandchildClass;
	
	CHECK( baseClass->IsA( myTestBaseClass::StaticClass() ) );
	CHECK( childClass->IsA( myTestChildClass::StaticClass() ) );
	CHECK( grandchildClass->IsA( myTestGrandchildClass::StaticClass() ) );
	CHECK( dynGrandchildClass->IsA( myTestGrandchildClass::StaticClass() ) );
	CHECK( dynGrandchildClass->IsKindOf( FxObject::StaticClass() ) );
	CHECK( dynGrandchildClass->IsKindOf( myTestGrandchildClass::StaticClass() ) );
	CHECK( baseClass->GetCurrentVersion() == 0 );
	CHECK( childClass->GetCurrentVersion() == 8 );
	CHECK( grandchildClass->GetCurrentVersion() == 4 );
	CHECK( dynGrandchildClass->GetCurrentVersion() == 4 );
	CHECK( FxClass::FindClassDesc( "myTestBaseClass" ) == myTestBaseClass::StaticClass() );
	CHECK( FxClass::FindClassDesc( "myTestChildClass" ) == myTestChildClass::StaticClass() );
	CHECK( FxClass::FindClassDesc( "myTestGrandchildClass" ) == myTestGrandchildClass::StaticClass() );
	CHECK( baseClass->GetClassDesc() == FxClass::FindClassDesc( "myTestBaseClass") );
	CHECK( childClass->GetClassDesc() == FxClass::FindClassDesc( "myTestChildClass") );
	CHECK( grandchildClass->GetClassDesc() == FxClass::FindClassDesc( "myTestGrandchildClass") );
	CHECK( dynGrandchildClass->GetClassDesc() == FxClass::FindClassDesc( "myTestGrandchildClass") );

	myTestBaseClass* testDynCreate = 
		static_cast<myTestBaseClass*>(dynGrandchildClass->GetClassDesc()->ConstructObject());
	CHECK( testDynCreate != NULL );
	CHECK( testDynCreate->IsA( myTestGrandchildClass::StaticClass() ) );
	CHECK( testDynCreate->GetCurrentVersion() == 4 );

	// Test dynamic type-safe casting of FxObjects.
	FxObject* castOne = FxCast<FxObject>(dynGrandchildClass);
	CHECK( castOne != NULL );
	myTestBaseClass* castTwo = FxCast<myTestBaseClass>(dynGrandchildClass);
	CHECK( castTwo != NULL );
	myTestGrandchildClass* castThree = FxCast<myTestGrandchildClass>(dynGrandchildClass);
	CHECK( castThree != NULL );
	FxNamedObject* failCast = FxCast<FxNamedObject>(dynGrandchildClass);
	CHECK( failCast == NULL );

	if( baseClass )
	{
		delete baseClass;
		baseClass = NULL;
	}

	if( childClass )
	{
		delete childClass;
		childClass = NULL;
	}

	if( grandchildClass )
	{
		delete grandchildClass;
		grandchildClass = NULL;
	}

	if( dynGrandchildClass )
	{
		delete dynGrandchildClass;
		dynGrandchildClass = NULL;
	}

	if( testDynCreate )
	{
		delete testDynCreate;
		testDynCreate = NULL;
	}
}

#endif
