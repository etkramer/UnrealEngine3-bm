/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDEntity.h"
#include "FCDocument/FCDSceneNode.h"
#include "FCDocument/FCDPhysicsMaterial.h"
#include "FCDocument/FCDPhysicsModel.h"
#include "FCDocument/FCDPhysicsModelInstance.h"
#include "FCDocument/FCDPhysicsRigidBody.h"
#include "FCDocument/FCDPhysicsRigidBodyInstance.h"
#include "FCDocument/FCDPhysicsParameter.h"
#include "FCDocument/FCDPhysicsParameter.hpp"
#include "FCDocument/FCDExternalReference.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDPhysicsRigidBodyInstance);

FCDPhysicsRigidBodyInstance::FCDPhysicsRigidBodyInstance(FCDocument* document, FCDPhysicsModelInstance* _parent)
:	FCDEntityInstance(document, NULL, FCDEntity::PHYSICS_RIGID_BODY)
,	parent(_parent)
,	physicsMaterial(NULL), ownsPhysicsMaterial(false)
{
	instanceMaterialRef = NULL;
}

FCDPhysicsRigidBodyInstance::~FCDPhysicsRigidBodyInstance()
{
	parent = NULL;
	SAFE_DELETE(instanceMaterialRef);

	if(ownsPhysicsMaterial)
	{
		SAFE_DELETE(physicsMaterial);
	}
	else
	{
		physicsMaterial = NULL;
	}
}

void FCDPhysicsRigidBodyInstance::AddParameter(FCDPhysicsParameterGeneric* parameter)
{
	parameters.push_back(parameter);
	SetDirtyFlag();
}

void FCDPhysicsRigidBodyInstance::SetPhysicsMaterial(FCDPhysicsMaterial* material)
{
	if (ownsPhysicsMaterial) SAFE_DELETE(physicsMaterial);
	physicsMaterial = material;
	ownsPhysicsMaterial = false;
	SetDirtyFlag();
}

FCDPhysicsMaterial* FCDPhysicsRigidBodyInstance::AddOwnPhysicsMaterial()
{
	if (ownsPhysicsMaterial) SAFE_DELETE(physicsMaterial);
	physicsMaterial = new FCDPhysicsMaterial(GetDocument());
	ownsPhysicsMaterial = true;
	SetDirtyFlag();
	return physicsMaterial;
}

// Load the geometry instance from the COLLADA document
FUStatus FCDPhysicsRigidBodyInstance::LoadFromXML(xmlNode* instanceNode)
{
	FUStatus status = FCDEntityInstance::LoadFromXML(instanceNode);
	if (!status) return status;

	// Check for the expected instantiation node type
	if (!IsEquivalent(instanceNode->name, DAE_INSTANCE_RIGID_BODY_ELEMENT))
	{
		return status.Fail(FS("Unknown element for instantiation of entity: ") + TO_FSTRING(GetEntity()->GetDaeId()), instanceNode->line);
	}
	if (parent == NULL)
	{
		return status.Fail(FS("No Physics Model parent for rigid body instantiation"), instanceNode->line);
	}

	// Find the target scene node/rigid body
	string targetNodeId = ReadNodeProperty(instanceNode, DAE_TARGET_ATTRIBUTE);
	targetNode = GetDocument()->FindSceneNode(SkipPound(targetNodeId));
	if(!targetNode)
	{
		return status.Fail(FS("Couldn't find target node for instantiation of rigid body"), instanceNode->line);
	}

	// Find the instantiated rigid body
	string physicsRigidBodySid = ReadNodeProperty(instanceNode, DAE_BODY_ATTRIBUTE);
	if (parent->GetEntity() != NULL &&  parent->GetEntity()->GetType() == FCDEntity::PHYSICS_MODEL)
	{
		FCDPhysicsModel* model = (FCDPhysicsModel*) parent->GetEntity();
		FCDPhysicsRigidBody* body = model->FindRigidBodyFromSid(physicsRigidBodySid);
		if(body == NULL)
		{
			return status.Fail(FS("Couldn't find rigid body for instantiation"), instanceNode->line);
		}
		SetEntity(body);
	}

	//Read in the same children as rigid_body + velocity and angular_velocity
	xmlNode* techniqueNode = FindChildByType(instanceNode, DAE_TECHNIQUE_COMMON_ELEMENT);
	xmlNode* param = FindChildByType(techniqueNode, DAE_DYNAMIC_ELEMENT);
	if(param)
	{
		AddParameter(DAE_DYNAMIC_ELEMENT, FUStringConversion::ToBoolean(ReadNodeContentDirect(param)));
	}

	xmlNode* massFrame;
	massFrame = FindChildByType(techniqueNode, DAE_MASS_FRAME_ELEMENT);
	if(massFrame)
	{
        param = FindChildByType(massFrame, DAE_TRANSLATE_ELEMENT);
		if(param)
		{
			AddParameter(DAE_TRANSLATE_ELEMENT, FUStringConversion::ToPoint(ReadNodeContentDirect(param)));
		}
		xmlNodeList rotates;
		FindChildrenByType(massFrame, DAE_ROTATE_ELEMENT, rotates);
		for(xmlNodeList::iterator it = rotates.begin(); it != rotates.end(); it++)
		{
			FCDPhysicsParameter<FMVector4>* p = new FCDPhysicsParameter<FMVector4>(GetDocument(), DAE_ROTATE_ELEMENT);
			p->SetValue(FUStringConversion::ToVector4(ReadNodeContentDirect(*it)));
			AddParameter(p);
		}
	}
	param = FindChildByType(techniqueNode, DAE_INERTIA_ELEMENT);
	if(param) 
	{
		AddParameter(DAE_INERTIA_ELEMENT, FUStringConversion::ToPoint(ReadNodeContentDirect(param)));
	}

	param = FindChildByType(techniqueNode, DAE_MASS_ELEMENT);
	if(param)
	{
		AddParameter(DAE_MASS_ELEMENT, FUStringConversion::ToFloat(ReadNodeContentDirect(param)));
	}

	param = FindChildByType(techniqueNode, DAE_PHYSICS_MATERIAL_ELEMENT);
	if(param) 
	{
		FCDPhysicsMaterial* material = AddOwnPhysicsMaterial();
		material->LoadFromXML(param);
	}
	else
	{
		param = FindChildByType(techniqueNode, DAE_INSTANCE_PHYSICS_MATERIAL_ELEMENT);
		if (param != NULL)
		{
			instanceMaterialRef = new FCDEntityInstance(GetDocument(), NULL, FCDEntity::PHYSICS_MATERIAL);
			instanceMaterialRef->LoadFromXML(param);
			FCDPhysicsMaterial* material = (FCDPhysicsMaterial*) instanceMaterialRef->GetEntity();
			if(!material)
			{
				return status.Fail(FS("Couldn't find material for instantiation of rigid body"), instanceNode->line);
			}
			SetPhysicsMaterial(material);
		}
	}
	
	param = FindChildByType(techniqueNode, DAE_VELOCITY_ELEMENT);
	if(param)
	{
		AddParameter(DAE_VELOCITY_ELEMENT, FUStringConversion::ToPoint(ReadNodeContentDirect(param)));
	}
	param = FindChildByType(techniqueNode, DAE_ANGULAR_VELOCITY_ELEMENT);
	if(param)
	{
		AddParameter(DAE_ANGULAR_VELOCITY_ELEMENT, FUStringConversion::ToPoint(ReadNodeContentDirect(param)));
	}

	SetDirtyFlag();
	return status;
}


FCDPhysicsRigidBody* FCDPhysicsRigidBodyInstance::FlattenRigidBody()
{
	FCDPhysicsRigidBody* rigidBody = (FCDPhysicsRigidBody*) GetEntity(); 
	FCDPhysicsRigidBody* clone = NULL;
	if (rigidBody != NULL)
	{
		clone = rigidBody->Clone();
		clone->Flatten();

		for (FCDPhysicsParameterContainer::iterator itP = parameters.begin(); itP != parameters.end(); ++itP)
		{
			FCDPhysicsParameterGeneric* param = clone->FindParameterByReference((*itP)->GetReference());
			if(param)
			{
				(*itP)->Overwrite(param);
			}
			else
			{
				clone->CopyParameter(*itP);
			}
		}

		if(physicsMaterial)
			clone->SetPhysicsMaterial(physicsMaterial);
	}

	return clone;
}

// Write out the instantiation information to the XML node tree
xmlNode* FCDPhysicsRigidBodyInstance::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* instanceNode = FCDEntityInstance::WriteToXML(parentNode);

	AddAttribute(instanceNode, DAE_TARGET_ATTRIBUTE, string("#") + targetNode->GetDaeId());
	AddAttribute(instanceNode, DAE_BODY_ATTRIBUTE, GetEntity()->GetDaeId());

	//inconsistency in the spec
	RemoveAttribute(instanceNode, DAE_URL_ATTRIBUTE);
	
	xmlNode* techniqueNode = AddChild(instanceNode, DAE_TECHNIQUE_COMMON_ELEMENT);
	for(FCDPhysicsParameterContainer::const_iterator it = parameters.begin(); it!= parameters.end(); it++)
	{
		(*it)->WriteToXML(techniqueNode);
	}
	//the translate and rotate elements need to be in a <mass_frame> tag. Go figure.
	xmlNodeList transforms;
	FindChildrenByType(techniqueNode, DAE_TRANSLATE_ELEMENT, transforms);
	FindChildrenByType(techniqueNode, DAE_ROTATE_ELEMENT, transforms);
	if (!transforms.empty())
	{
		xmlNode* massFrameNode = AddChild(techniqueNode, DAE_MASS_FRAME_ELEMENT);
		for(xmlNodeList::iterator it = transforms.begin(); it != transforms.end(); it++)
		{
			ReParentNode(*it, massFrameNode);
		}
	}

	if(ownsPhysicsMaterial && physicsMaterial)
	{
		xmlNode* materialNode = AddChild(techniqueNode, DAE_PHYSICS_MATERIAL_ELEMENT);
		physicsMaterial->WriteToXML(materialNode);
	}
	else if(instanceMaterialRef)
	{
		instanceMaterialRef->WriteToXML(techniqueNode);
//		xmlNode* materialNode = AddChild(techniqueNode, DAE_INSTANCE_PHYSICS_MATERIAL_ELEMENT);
//		AddAttribute(materialNode, DAE_SID_ATTRIBUTE, instanceMaterialRef->GetSid());
//		AddAttribute(materialNode, DAE_NAME_ATTRIBUTE, instanceMaterialRef->GetName());
//		AddAttribute(materialNode, DAE_URL_ATTRIBUTE, instanceMaterialRef->GetUri().suffix);
	}

	return instanceNode;
}
