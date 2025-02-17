/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FUtils/FUTestBed.h"
#include "FUtils/FUObject.h"
#include "FUtils/FUObjectType.h"

///////////////////////////////////////////////////////////////////////////////
// Declare a few object and containers to be used by the tests belows
class FUTObject1 : public FUObject
{
	DeclareObjectType(FUObject);
public:
	FUTObject1(FUObjectContainer<>* container) { container->push_back(this); }
};
ImplementObjectType(FUTObject1);

class FUTObject2 : public FUObject
{
	DeclareObjectType(FUObject);
public:
	FUTObject2(FUObjectContainer<>* container) { container->push_back(this); }
};
ImplementObjectType(FUTObject2);

class FUTObject1Up : public FUTObject1
{
	DeclareObjectType(FUTObject1);
public:
	FUTObject1Up(FUObjectContainer<>* container) : FUTObject1(container) {}
};
ImplementObjectType(FUTObject1Up);

///////////////////////////////////////////////////////////////////////////////
class FUTSimple1 : public FUObject
{
	DeclareObjectType(FUObject);
};
class FUTSimple2 : public FUTSimple1
{
	DeclareObjectType(FUTSimple1);
};
class FUTSimple3 : public FUTSimple2
{
	DeclareObjectType(FUTSimple2);
};
class FUTSimple4 : public FUTSimple2
{
	DeclareObjectType(FUTSimple2);
};
ImplementObjectType(FUTSimple1);
ImplementObjectType(FUTSimple2);
ImplementObjectType(FUTSimple3);
ImplementObjectType(FUTSimple4);

///////////////////////////////////////////////////////////////////////////////
TESTSUITE_START(FUObject)

TESTSUITE_TEST(SimpleTracking)
	FUObjectContainer<> container;
	FUTObject1* obj1 = new FUTObject1(&container);
	FUTObject2* obj2 = new FUTObject2(&container);
	PassIf(obj1 != NULL && obj2 != NULL);
	PassIf(container.contains(obj1));
	PassIf(container.contains(obj2));
	delete obj1;
	PassIf(container.contains(obj2));
	FailIf(container.contains(obj1));
	delete obj2;
	PassIf(container.empty());
	obj2 = NULL; obj1 = NULL;

	// Verify that non-tracked objects are acceptable
	FUObject* obj3 = new FUObject();
	PassIf(container.empty());
	SAFE_DELETE(obj3);

TESTSUITE_TEST(RTTI)
	FUObjectContainer<> container;
	FUTObject1* obj1 = new FUTObject1(&container);
	FUTObject2* obj2 = new FUTObject2(&container);
	const FUObjectType& type1 = obj1->GetObjectType();
	const FUObjectType& type2 = obj2->GetObjectType();
	FailIf(type1 == type2);
	PassIf(type1 == FUTObject1::GetClassType());
	PassIf(type2 == FUTObject2::GetClassType());

	// Check out up-class RTTI
	FUTObject1Up* up = new FUTObject1Up(&container);
	PassIf(up->GetObjectType() == FUTObject1Up::GetClassType());
	FailIf(up->GetObjectType() == FUTObject1::GetClassType());
	FailIf(up->GetObjectType() == FUTObject2::GetClassType());
	PassIf(up->GetObjectType().Includes(FUTObject1::GetClassType()));
	PassIf(up->GetObjectType().Includes(obj1->GetObjectType()));
	PassIf(up->GetObjectType().GetParent() == FUTObject1::GetClassType());
	SAFE_DELETE(obj1);
	SAFE_DELETE(obj2);
	SAFE_DELETE(up);

TESTSUITE_TEST(MultiTracker)
	// Create one object and have multiple trackers tracking it.
	FUObjectList<> container1;
	FUObjectList<> container2;
	FUObjectList<> container3;

	FUObject* o = new FUObject();
	container1.push_back(o);
	container2.push_back(o);
	container3.push_back(o);
	PassIf(!container1.empty());
	PassIf(!container2.empty());
	PassIf(!container3.empty());
	
	// Now, release the object and verify that all the trackers have been informed
	delete o;
	PassIf(container1.empty());
	PassIf(container2.empty());
	PassIf(container3.empty());

TESTSUITE_TEST(MultiContainer)
	// Create a couple of objects and have multiple containers tracking them.
	FUObjectContainer<>* container1 = new FUObjectContainer<>();
	FUObjectContainer<>* container2 = new FUObjectContainer<>();
	FUObjectContainer<>* container3 = new FUObjectContainer<>();

	FUObject* o1 = container1->Add();
	FUObject* o2 = container2->Add();
	container1->push_back(o2);
	container2->push_back(o1);
	container3->push_back(o2);
	container3->insert(container3->begin(), o1);
	PassIf(container1->size() == 2);
	PassIf(container2->size() == 2);
	PassIf(container3->size() == 2);

	// Delete the first object. All the containers should be informed
	// and only the second object should still be trackers by the containers.
	delete o1;
	PassIf(container1->size() == 1 && container1->contains(o2));
	PassIf(container2->size() == 1 && container2->contains(o2));
	PassIf(container3->size() == 1 && container3->contains(o2));

	// Now, the tricky part. Delete the first container and verify
	// that the other containers are now empty.
	delete container1;
	PassIf(container2->empty());
	PassIf(container3->empty());

	// clean-up
	delete container2;
	delete container3;
	FUCheckMemory(return false);

TESTSUITE_TEST(ContainedObjectPointer)
	// Create a couple of objects and have multiple containers tracking them.
	FUObjectContainer<>* container = new FUObjectContainer<>();
	FUObjectPtr<> smartPointer;
	FUObject* obj = container->Add();
	smartPointer = obj;
	FailIf(smartPointer != obj);
	PassIf(smartPointer == obj);
	PassIf(obj == smartPointer);
	FailIf(obj != smartPointer);
	PassIf(smartPointer->GetObjectType() == FUObject::GetClassType());
	PassIf((*smartPointer).GetObjectType() == FUObject::GetClassType());

	// Delete the container and verify that the pointer got updated.
	delete container;
	PassIf(obj != smartPointer);
	PassIf(smartPointer == NULL);
	FailIf(smartPointer == obj);

TESTSUITE_TEST(ObjectReference)
	FUObjectPtr<>* smartPointer1 = new FUObjectPtr<>();
	FUObjectPtr<>* smartPointer2 = new FUObjectPtr<>();
	FUObjectRef<>* smartReference = new FUObjectRef<>();
	FUObject* testObj = new FUObject();

	// Assign the test object and verify that releasing the pointer doesn't affect
	// the other pointer or the reference.
	*smartPointer1 = testObj;
	*smartPointer2 = testObj;
	*smartReference = testObj;
	SAFE_DELETE(smartPointer1);
	PassIf(*smartReference == testObj);
	PassIf(*smartPointer2 == testObj);
	PassIf(smartReference->TracksObject(testObj));

	// Verify that when the reference is assigned something else: the pointers
	// get cleared properly.
	smartPointer1 = new FUObjectPtr<>(testObj);
	*smartReference = new FUObject();
	PassIf(*smartPointer1 == NULL);
	PassIf(*smartPointer2 == NULL);
	PassIf(*smartReference != NULL);

	// Verify that when the reference is deleted, the pointer do get cleared.
	*smartPointer1 = *smartReference;
	*smartPointer2 = *smartReference;
	PassIf(*smartPointer1 != NULL);
	PassIf(*smartPointer2 != NULL);
	PassIf(*smartPointer1 == *smartPointer2);
	SAFE_DELETE(smartReference);
	PassIf(*smartPointer1 == NULL);
	PassIf(*smartPointer2 == NULL);

	SAFE_DELETE(smartPointer1);
	SAFE_DELETE(smartPointer2);

TESTSUITE_TEST(ObjectContainerErase)
	// Create a couple of objects and have multiple containers tracking them.
	FUObjectList<> container1, container2;
	FUObject* o = new FUObject();
	container1.push_back(o);
	container2.push_back(o);
	container1.erase(o);
	container2.pop_back();
	SAFE_DELETE(o);

TESTSUITE_TEST(Shortcuts)
	FUTSimple1 t1;
	FUTSimple3 t3;
	FUTSimple4 t4;

	PassIf(t1.IsType(FUTSimple1::GetClassType()));
	PassIf(t1.HasType(FUObject::GetClassType()));
	FailIf(t1.IsType(FUObject::GetClassType()));

	PassIf(t3.IsType(FUTSimple3::GetClassType()));
	FailIf(t3.IsType(FUTSimple2::GetClassType()));
	PassIf(t3.HasType(FUTSimple2::GetClassType()));
	PassIf(t3.HasType(FUTSimple1::GetClassType()));
	PassIf(t3.HasType(FUObject::GetClassType()));

	PassIf(DynamicCast<FUTSimple2>(&t3) == &t3);
	PassIf(DynamicCast<FUTSimple3>(&t4) == NULL);

TESTSUITE_END
