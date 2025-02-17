/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDEntity.h"
#include "FCDocument/FCDPhysicsModel.h"
#include "FCDocument/FCDPhysicsModelInstance.h"
#include "FCDocument/FCDPhysicsRigidBody.h"
#include "FCDocument/FCDPhysicsRigidConstraint.h"
#include "FCDocument/FCDPhysicsRigidConstraintInstance.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDPhysicsRigidConstraintInstance);

FCDPhysicsRigidConstraintInstance::FCDPhysicsRigidConstraintInstance(FCDocument* document, FCDPhysicsModelInstance* _parent)
:	FCDEntityInstance(document, NULL, FCDEntity::PHYSICS_RIGID_CONSTRAINT), parent(_parent)
{
}

FCDPhysicsRigidConstraintInstance::~FCDPhysicsRigidConstraintInstance()
{
	parent = NULL;
}


// Load the geometry instance from the COLLADA document
FUStatus FCDPhysicsRigidConstraintInstance::LoadFromXML(xmlNode* instanceNode)
{
	FUStatus status = FCDEntityInstance::LoadFromXML(instanceNode);
	if (!status) return status;

	// Check for the expected instantiation node type
	if (!IsEquivalent(instanceNode->name, DAE_INSTANCE_RIGID_CONSTRAINT_ELEMENT))
	{
		return status.Fail(FS("Unknown element for instantiation of entity: ") + TO_FSTRING(GetEntity()->GetDaeId()), instanceNode->line);
	}
	if (parent == NULL || parent->GetEntity() == NULL)
	{
		return status.Fail(FS("No Physics Model parent for rigid constraint instantiation"), instanceNode->line);
	}

	FCDPhysicsModel* model = (FCDPhysicsModel*) parent->GetEntity();
	string physicsRigidConstraintSid = ReadNodeProperty(instanceNode, DAE_CONSTRAINT_ATTRIBUTE);
	FCDPhysicsRigidConstraint* rigidConstraint = model->FindRigidConstraintFromSid(physicsRigidConstraintSid);
	if(!rigidConstraint)
	{
		return status.Warning(FS("Couldn't find rigid constraint for instantiation"), instanceNode->line);
	}
	SetEntity(rigidConstraint);
	SetDirtyFlag();
	return status;
}


// Write out the instantiation information to the XML node tree
xmlNode* FCDPhysicsRigidConstraintInstance::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* instanceNode = AddChild(parentNode, DAE_INSTANCE_RIGID_CONSTRAINT_ELEMENT);

	if (GetEntity() != NULL && GetEntity()->GetObjectType() == FCDPhysicsRigidConstraint::GetClassType())
	{
		FCDPhysicsRigidConstraint* constraint = (FCDPhysicsRigidConstraint*) GetEntity();
		AddAttribute(instanceNode, DAE_CONSTRAINT_ATTRIBUTE, constraint->GetSubId());
	}

	return instanceNode;
}
