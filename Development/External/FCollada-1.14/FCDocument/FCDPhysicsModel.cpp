/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDPhysicsModel.h"
#include "FCDocument/FCDPhysicsModelInstance.h"
#include "FCDocument/FCDPhysicsRigidBody.h"
#include "FCDocument/FCDPhysicsRigidConstraint.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDAnimated.h"
#include "FUtils/FUStringConversion.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDPhysicsModel);

FCDPhysicsModel::FCDPhysicsModel(FCDocument* document)
:	FCDEntity(document, "PhysicsModel")
{
}

FCDPhysicsModel::~FCDPhysicsModel()
{
}

FCDPhysicsModelInstance* FCDPhysicsModel::AddPhysicsModelInstance(FCDPhysicsModel* model)
{
	FCDPhysicsModelInstance* instance = instances.Add(GetDocument());	
	instance->SetEntity(model);
	SetDirtyFlag(); 
	return instance;
}

FCDPhysicsRigidBody* FCDPhysicsModel::AddRigidBody()
{
	FCDPhysicsRigidBody* rigidBody = rigidBodies.Add(GetDocument());
	SetDirtyFlag(); 
	return rigidBody;
}

FCDPhysicsRigidConstraint* FCDPhysicsModel::AddRigidConstraint()
{
	FCDPhysicsRigidConstraint* constraint = rigidConstraints.Add(GetDocument(), this);
	SetDirtyFlag(); 
	return constraint;
}

// Create a copy of this physicsModel, with the vertices overwritten
FCDPhysicsModel* FCDPhysicsModel::Clone()
{
	FCDPhysicsModel* clone = new FCDPhysicsModel(GetDocument());
	clone->rigidBodies = rigidBodies;
	clone->instances = instances;
	clone->rigidConstraints = rigidConstraints;
	return clone;
}

FCDPhysicsRigidBody* FCDPhysicsModel::FindRigidBodyFromSid(const string& sid)
{ return const_cast<FCDPhysicsRigidBody*>(const_cast<const FCDPhysicsModel*>(this)->FindRigidBodyFromSid(sid)); }
const FCDPhysicsRigidBody* FCDPhysicsModel::FindRigidBodyFromSid(const string& sid) const
{
	for(FCDPhysicsRigidBodyContainer::const_iterator it = rigidBodies.begin(); it!= rigidBodies.end(); ++it)
	{
		if((*it)->GetSubId()==sid) return (*it);
	}
	return NULL;
}

FCDPhysicsRigidConstraint* FCDPhysicsModel::FindRigidConstraintFromSid(const string& sid)
{ return const_cast<FCDPhysicsRigidConstraint*>(const_cast<const FCDPhysicsModel*>(this)->FindRigidConstraintFromSid(sid)); }
const FCDPhysicsRigidConstraint* FCDPhysicsModel::FindRigidConstraintFromSid(const string& sid) const
{
	for(FCDPhysicsRigidConstraintContainer::const_iterator it = rigidConstraints.begin(); it!= rigidConstraints.end(); ++it)
	{
		if((*it)->GetSubId()==sid) return (*it);
	}
	return NULL;
}


// Load from a XML node the given physicsModel
FUStatus FCDPhysicsModel::LoadFromXML(xmlNode* physicsModelNode)
{
	FUStatus status = FCDEntity::LoadFromXML(physicsModelNode);
	if (!status) return status;
	if (!IsEquivalent(physicsModelNode->name, DAE_PHYSICS_MODEL_ELEMENT))
	{
		return status.Warning(FS("PhysicsModel library contains unknown element."), physicsModelNode->line);
	}

	// Read in the first valid child element found
	for (xmlNode* child = physicsModelNode->children; child != NULL; child = child->next)
	{
		if (child->type != XML_ELEMENT_NODE) continue;

		if(IsEquivalent(child->name, DAE_RIGID_BODY_ELEMENT))
		{
			FCDPhysicsRigidBody* rigidBody = AddRigidBody();
			status.AppendStatus(rigidBody->LoadFromXML(child));

		}
		else if(IsEquivalent(child->name, DAE_RIGID_CONSTRAINT_ELEMENT))
		{
			FCDPhysicsRigidConstraint* rigidConstraint = AddRigidConstraint();
			status.AppendStatus(rigidConstraint->LoadFromXML(child));
		}
		else if(IsEquivalent(child->name, DAE_INSTANCE_PHYSICS_MODEL_ELEMENT))
		{
			//FIXME: the instantiated physicsModel might not have been parsed yet
			FUUri url = ReadNodeUrl(child);
			if (url.prefix.empty()) 
			{ 
				FCDEntity* entity = GetDocument()->FindPhysicsModel(url.suffix);
				if (entity != NULL) 
				{
					FCDPhysicsModelInstance* instance = AddPhysicsModelInstance((FCDPhysicsModel*) entity);
					status.AppendStatus(instance->LoadFromXML(child));
				}
				else
				{
					status.Warning(FS("Unable to retrieve instance for scene node: ") + TO_FSTRING(GetDaeId()), child->line);
				}
			}
		}
		else if(IsEquivalent(child->name, DAE_ASSET_ELEMENT))
		{
		}
		else if(IsEquivalent(child->name, DAE_EXTRA_ELEMENT))
		{
		}
	}

	SetDirtyFlag();
	return status;
}

// Write out the <physicsModel> node
xmlNode* FCDPhysicsModel::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* physicsModelNode = WriteToEntityXML(parentNode, DAE_PHYSICS_MODEL_ELEMENT);
	for(FCDPhysicsModelInstanceContainer::const_iterator it = instances.begin(); it != instances.end(); ++it)
	{
		(*it)->WriteToXML(physicsModelNode);
	}
	for(FCDPhysicsRigidBodyContainer::const_iterator it = rigidBodies.begin(); it != rigidBodies.end(); ++it)
	{
		(*it)->WriteToXML(physicsModelNode);
	}
	for(FCDPhysicsRigidConstraintContainer::const_iterator it = rigidConstraints.begin(); it != rigidConstraints.end(); ++it)
	{
		(*it)->WriteToXML(physicsModelNode);
	}

	//TODO: Add asset and extra

	FCDEntity::WriteToExtraXML(physicsModelNode);
	return physicsModelNode;
}
