/**
 * GearEditor's factory types.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#ifndef _GEAREDITOR_FACTORIES_H_
#define _GEAREDITOR_FACTORIES_H_


class UGearBloodInfoFactoryNew : public UFactory
{
	DECLARE_CLASS(UGearBloodInfoFactoryNew,UFactory,CLASS_CollapseCategories,GearEditor);
	NO_DEFAULT_CONSTRUCTOR(UGearBloodInfoFactoryNew)
public:

	void StaticConstructor();
	/**
	* Initializes property values for intrinsic classes.  It is called immediately after the class default object
	* is initialized against its archetype, but before any objects of this class are created.
	*/
	void InitializeIntrinsicPropertyValues();
	UObject* FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn);
};


#endif // _GEAREDITOR_FACTORIES_H_




