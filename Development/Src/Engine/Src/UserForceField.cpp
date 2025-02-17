/**
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */

#include "EnginePrivate.h"
#include "UserForceField.h"
#include "UserForceFieldShapeGroup.h"

#if WITH_NOVODEX
#include "UnNovodexSupport.h"

UserForceField* UserForceField::Create(NxForceField* forceField)
{
	UserForceField* result = new UserForceField;
	result->NxObject = forceField;
	result->MainGroup = UserForceFieldShapeGroup::Create(&forceField->getIncludeShapeGroup(), &forceField->getScene());
	forceField->userData = result;
	return result;
}

void UserForceField::Destroy()
{
	check(NxObject->getScene().isWritable());
	MainGroup->Destroy();
	NxObject->getScene().releaseForceField(*NxObject);
	delete this;
}

void UserForceField::addShapeGroup(UserForceFieldShapeGroup& ffsGroup)
{
	ffsGroup.GiveToForceField(this);
}

UserForceFieldShapeGroup & UserForceField::getIncludeShapeGroup()
{
	return *UserForceFieldShapeGroup::Convert(&NxObject->getIncludeShapeGroup());
}

#endif
