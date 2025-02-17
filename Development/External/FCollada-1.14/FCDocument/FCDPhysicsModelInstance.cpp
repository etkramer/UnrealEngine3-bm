/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDController.h"
#include "FCDocument/FCDEntity.h"
#include "FCDocument/FCDForceField.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDPhysicsModel.h"
#include "FCDocument/FCDPhysicsModelInstance.h"
#include "FCDocument/FCDPhysicsRigidBodyInstance.h"
#include "FCDocument/FCDPhysicsRigidConstraintInstance.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDPhysicsModelInstance);

FCDPhysicsModelInstance::FCDPhysicsModelInstance(FCDocument* document)
:	FCDEntityInstance(document, NULL, FCDEntity::PHYSICS_MODEL)
{
}

FCDPhysicsModelInstance::~FCDPhysicsModelInstance()
{
}

FCDPhysicsRigidBodyInstance* FCDPhysicsModelInstance::AddRigidBodyInstance(FCDPhysicsRigidBody* rigidBody)
{
	FCDPhysicsRigidBodyInstance* instance = new FCDPhysicsRigidBodyInstance(GetDocument(), this);
	instance->SetRigidBody(rigidBody);
	instances.push_back(instance);
	SetDirtyFlag();
	return instance;
}

FCDPhysicsRigidConstraintInstance* FCDPhysicsModelInstance::AddRigidConstraintInstance(FCDPhysicsRigidConstraint* rigidConstraint)
{
	FCDPhysicsRigidConstraintInstance* instance = new FCDPhysicsRigidConstraintInstance(GetDocument(), this);
	instance->SetRigidConstraint(rigidConstraint);
	instances.push_back(instance);
	SetDirtyFlag();
	return instance;
}

FCDEntityInstance* FCDPhysicsModelInstance::AddForceFieldInstance(FCDForceField* forceField)
{
	FCDEntityInstance* instance = instances.Add(GetDocument(), (FCDSceneNode*) NULL, FCDEntity::FORCE_FIELD);
	instance->SetEntity(forceField);
	SetDirtyFlag();
	return instance;
}

// Load the geometry instance from the COLLADA document
FUStatus FCDPhysicsModelInstance::LoadFromXML(xmlNode* instanceNode)
{
	FUStatus status = FCDEntityInstance::LoadFromXML(instanceNode);
	if (!status) return status;

	if (GetEntity() == NULL)
	{
		return status.Fail(FS("Trying to instantiate non-valid physics entity."), instanceNode->line);
	}

	// Check for the expected instantiation node type
	if (!IsEquivalent(instanceNode->name, DAE_INSTANCE_PHYSICS_MODEL_ELEMENT))
	{
		return status.Fail(FS("Unknown element for instantiation of entity: ") + TO_FSTRING(GetEntity()->GetDaeId()), instanceNode->line);
	}

	//this is already done in the FCDSceneNode
//	string physicsModelId = ReadNodeProperty(instanceNode, DAE_TARGET_ATTRIBUTE);
//	entity = GetDocument()->FindPhysicsModel(physicsModelId);
//	if(!entity)	return status.Fail(FS("Couldn't find physics model for instantiation"), instanceNode->line);

	xmlNodeList rigidBodyNodes;
	FindChildrenByType(instanceNode, DAE_INSTANCE_RIGID_BODY_ELEMENT, rigidBodyNodes);
	for(xmlNodeList::iterator itB = rigidBodyNodes.begin(); itB != rigidBodyNodes.end(); ++itB)
	{
		FCDPhysicsRigidBodyInstance* instance = AddRigidBodyInstance(NULL);
		status.AppendStatus(instance->LoadFromXML(*itB));
	}

	xmlNodeList rigidConstraintNodes;
	FindChildrenByType(instanceNode, DAE_INSTANCE_RIGID_CONSTRAINT_ELEMENT, rigidConstraintNodes);
	for(xmlNodeList::iterator itC = rigidConstraintNodes.begin(); itC != rigidConstraintNodes.end(); ++itC)
	{
		FCDPhysicsRigidConstraintInstance* instance = AddRigidConstraintInstance(NULL);
		status.AppendStatus(instance->LoadFromXML(*itC));
	}

	//FIXME: create a FCDPhysicsForceFieldInstance class
	xmlNodeList forceFieldNodes;
	FindChildrenByType(instanceNode, DAE_INSTANCE_FORCE_FIELD_ELEMENT, forceFieldNodes);
	for(xmlNodeList::iterator itN = forceFieldNodes.begin(); itN != forceFieldNodes.end(); ++itN)
	{
		FUUri uri = ReadNodeUrl(*itN);
		FCDForceField* forceField = GetDocument()->GetForceFieldLibrary()->FindDaeId(uri.suffix);
		FCDEntityInstance* instance = AddForceFieldInstance(forceField);
		status.AppendStatus(instance->LoadFromXML(*itN));
	}

	SetDirtyFlag();
	return status;
}

// Write out the instantiation information to the XML node tree
xmlNode* FCDPhysicsModelInstance::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* instanceNode = FCDEntityInstance::WriteToXML(parentNode);

	// The sub-instances must be ordered correctly: force fields first, then rigid bodies; rigid constraints are last.
	for(FCDEntityInstanceContainer::const_iterator it = instances.begin(); it != instances.end(); ++it)
	{
		if ((*it)->GetEntityType() == FCDEntity::FORCE_FIELD)
		{
			(*it)->WriteToXML(instanceNode);
		}
	}
	for(FCDEntityInstanceContainer::const_iterator it = instances.begin(); it != instances.end(); ++it)
	{
		if ((*it)->GetEntityType() == FCDEntity::PHYSICS_RIGID_BODY)
		{
			(*it)->WriteToXML(instanceNode);
		}
	}
	for(FCDEntityInstanceContainer::const_iterator it = instances.begin(); it != instances.end(); ++it)
	{
		if ((*it)->GetEntityType() == FCDEntity::PHYSICS_RIGID_CONSTRAINT)
		{
			(*it)->WriteToXML(instanceNode);
		}
	}

	return instanceNode;
}
