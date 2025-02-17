/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDPhysicsRigidBody.h"
#include "FCDocument/FCDPhysicsMaterial.h"
#include "FCDocument/FCDPhysicsParameter.h"
#include "FCDocument/FCDPhysicsParameter.hpp"
#include "FCDocument/FCDPhysicsShape.h"
#include "FCDocument/FCDTransform.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDAnimated.h"
#include "FUtils/FUStringConversion.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDPhysicsRigidBody);

FCDPhysicsRigidBody::FCDPhysicsRigidBody(FCDocument* document) : FCDEntity(document, "RigidBody")
{
	physicsMaterial = NULL;
	ownsPhysicsMaterial = false;
}

FCDPhysicsRigidBody::~FCDPhysicsRigidBody()
{
	if(ownsPhysicsMaterial)
	{
		SAFE_DELETE(physicsMaterial);
	}
	else
	{
		physicsMaterial = NULL;
	}
}

// Create a copy of this physicsRigidBody
FCDPhysicsRigidBody* FCDPhysicsRigidBody::Clone()
{
	FCDPhysicsRigidBody* clone = new FCDPhysicsRigidBody(GetDocument());
	FCDEntity::Clone(clone);

	clone->parameters = parameters;
	clone->physicsShape = physicsShape;
	clone->SetPhysicsMaterial(physicsMaterial);

	return clone;
}

void FCDPhysicsRigidBody::CopyParameter(FCDPhysicsParameterGeneric* parameter)
{
	FCDPhysicsParameterGeneric* p = parameter->Clone();
	parameters.push_back(p);
}

void FCDPhysicsRigidBody::AddParameter(FCDPhysicsParameterGeneric* parameter)
{
	parameters.push_back(parameter);
	SetDirtyFlag();
}

FCDPhysicsShape* FCDPhysicsRigidBody::AddPhysicsShape()
{
	FCDPhysicsShape* shape = physicsShape.Add(GetDocument());
	SetDirtyFlag();
	return shape;
}

void FCDPhysicsRigidBody::SetPhysicsMaterial(FCDPhysicsMaterial* _physicsMaterial)
{
	if(physicsMaterial && ownsPhysicsMaterial)
	{
		SAFE_DELETE(physicsMaterial);
	}

	physicsMaterial = _physicsMaterial;
	ownsPhysicsMaterial = false;
	SetDirtyFlag();
}

FCDPhysicsMaterial* FCDPhysicsRigidBody::AddOwnPhysicsMaterial()
{
	if (physicsMaterial && ownsPhysicsMaterial)
		SAFE_DELETE(physicsMaterial);

	physicsMaterial = new FCDPhysicsMaterial(GetDocument());
	ownsPhysicsMaterial = true;
	SetDirtyFlag();
	return physicsMaterial;
}

// Load from a XML node the given physicsRigidBody
//FIXME: Default values not assigned if child elements not found
FUStatus FCDPhysicsRigidBody::LoadFromXML(xmlNode* physicsRigidBodyNode)
{
	FUStatus status = FCDEntity::LoadFromXML(physicsRigidBodyNode);
	if (!status) return status;
	if (!IsEquivalent(physicsRigidBodyNode->name, DAE_RIGID_BODY_ELEMENT)) 
	{
		return status.Warning(FS("PhysicsRigidBody library contains unknown element."), physicsRigidBodyNode->line);
	}

	SetSubId(FUDaeParser::ReadNodeSid(physicsRigidBodyNode));

	xmlNode* techniqueNode = FindChildByType(physicsRigidBodyNode, DAE_TECHNIQUE_COMMON_ELEMENT);
	if (techniqueNode != NULL)
	{
		xmlNode* param = FindChildByType(techniqueNode, DAE_DYNAMIC_ELEMENT);
		if(param)
		{
			FCDPhysicsParameter<bool>* p = new FCDPhysicsParameter<bool>(GetDocument(), DAE_DYNAMIC_ELEMENT);
			p->SetValue(FUStringConversion::ToBoolean(ReadNodeContentDirect(param)));
			FCDAnimatedFloat::Create(GetDocument(), param, 
					(float*)p->GetValue());
			AddParameter(p);
		}

		xmlNode* massFrame;
		massFrame = FindChildByType(techniqueNode, DAE_MASS_FRAME_ELEMENT);
		if(massFrame)
		{
			param = FindChildByType(massFrame, DAE_TRANSLATE_ELEMENT);
			if(param)
			{
				FCDPhysicsParameter<FMVector3>* p = new FCDPhysicsParameter<FMVector3>(GetDocument(), DAE_TRANSLATE_ELEMENT);
				p->SetValue(FUStringConversion::ToPoint(ReadNodeContentDirect(param)));
				FCDAnimatedPoint3::Create(GetDocument(), param, p->GetValue());
				AddParameter(p);
			}
			param = FindChildByType(massFrame, DAE_ROTATE_ELEMENT);
			if(param)
			{
				FCDPhysicsParameter<FMVector3>* p = new FCDPhysicsParameter<FMVector3>(GetDocument(), DAE_ROTATE_ELEMENT);
				p->SetValue(FUStringConversion::ToPoint(ReadNodeContentDirect(param)));
				AddParameter(p);
			}
		}
		param = FindChildByType(techniqueNode, DAE_INERTIA_ELEMENT);
		if(param) 
		{
			FCDPhysicsParameter<FMVector3>* p = new FCDPhysicsParameter<FMVector3>(GetDocument(), DAE_INERTIA_ELEMENT);
			p->SetValue(FUStringConversion::ToPoint(ReadNodeContentDirect(param)));
			FCDAnimatedPoint3::Create(GetDocument(), param, p->GetValue());
			AddParameter(p);
		}

		param = FindChildByType(techniqueNode, DAE_MASS_ELEMENT);
		if(param)
		{
			FCDPhysicsParameter<float>* p = new FCDPhysicsParameter<float>(GetDocument(), DAE_MASS_ELEMENT);
			p->SetValue(FUStringConversion::ToFloat(ReadNodeContentDirect(param)));
			FCDAnimatedFloat::Create(GetDocument(), param, p->GetValue());
			AddParameter(p);
		}

		xmlNodeList shapeNodes;
		FindChildrenByType(techniqueNode, DAE_SHAPE_ELEMENT, shapeNodes);
		for(xmlNodeList::iterator itS = shapeNodes.begin(); itS != shapeNodes.end(); ++itS)
		{
			FCDPhysicsShape* shape = AddPhysicsShape();
			status.AppendStatus(shape->LoadFromXML(*itS));
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
				FUUri url = ReadNodeUrl(param);
				if (url.prefix.empty())
				{
					FCDPhysicsMaterial* material = GetDocument()->FindPhysicsMaterial(url.suffix);
					SetPhysicsMaterial(material);
					if(!material)
					{
						status.Warning(FS("Error: Instantiated physics material in rigid body was not found."), techniqueNode->line);
					}
				}
			}
			else
			{
				status.Warning(FS("No physics material defined in rigid body."), techniqueNode->line);
			}
		}
	}

	return status;
}

// Flatten the rigid body: remove all the modifier parameters from the parameter list, permanently modifying their base parameter
void FCDPhysicsRigidBody::Flatten()
{
	vector<FCDPhysicsParameterGeneric*> toDelete;
	for (FCDPhysicsParameterList::iterator itP = parameters.begin(); itP != parameters.end(); ++itP)
	{
		FCDPhysicsParameterList generators;
		if ((*itP)->IsModifier())
		{
			// Overwrite the generators
			FCDPhysicsParameterGeneric* generator = FindParameterByReference((*itP)->GetReference());
			if(generator)
			{
				(*itP)->Overwrite(generator);
				toDelete.push_back(*itP);
			}
			else
			{
				(*itP)->SetGenerator(true);
			}

		}
	}

	while(!toDelete.empty())
	{
		FCDPhysicsParameterGeneric* d = toDelete.front();
		toDelete.erase(toDelete.begin());
		FCDPhysicsParameterList::iterator it = parameters.find(d);
		if (it != parameters.end()) parameters.erase(it);
		SAFE_DELETE(d);
	}

	SetDirtyFlag();
}


FCDPhysicsParameterGeneric* FCDPhysicsRigidBody::FindParameterByReference(const string& reference)
{
	for (FCDPhysicsParameterList::iterator it = parameters.begin(); it != parameters.end(); ++it)
	{
		if ((*it)->GetReference() == reference) return (*it);
	}
	return NULL;
}


// Write out the <physicsRigidBody> node
xmlNode* FCDPhysicsRigidBody::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* physicsRigidBodyNode = WriteToEntityXML(parentNode, DAE_RIGID_BODY_ELEMENT, false);
	const_cast<FCDPhysicsRigidBody*>(this)->SetSubId(AddNodeSid(physicsRigidBodyNode, GetSubId().c_str()));
	
#define ANIMATE_ELEMENT(type, name, paramNode) \
	FCDPhysicsParameter<type>* pdynamic = (FCDPhysicsParameter<type>*)(*it); \
	const FCDAnimated* animated = GetDocument()->FindAnimatedValue( \
			(float*) pdynamic->GetValue()); \
	if (animated) \
	{ \
		GetDocument()->WriteAnimatedValueToXML(animated, paramNode, \
			!GetSubId().empty() ? GetSubId().c_str() : name); \
	}

	xmlNode* baseNode = AddChild(physicsRigidBodyNode, DAE_TECHNIQUE_COMMON_ELEMENT);
	for(FCDPhysicsParameterList::const_iterator it = parameters.begin(); it != parameters.end(); ++it)
	{
		xmlNode* paramNode = (*it)->WriteToXML(baseNode);
		const string& reference = (*it)->GetReference();
		if (reference == DAE_DYNAMIC_ELEMENT) 
		{
			ANIMATE_ELEMENT(bool, "active", paramNode);
		}
		else if (reference == DAE_MASS_ELEMENT)
		{
			ANIMATE_ELEMENT(float, "mass", paramNode);
		}
		else if (reference == DAE_DENSITY_ELEMENT)
		{
			ANIMATE_ELEMENT(float, "density", paramNode);
		}
		else if (reference == DAE_TRANSLATE_ELEMENT)
		{
			ANIMATE_ELEMENT(FMVector3, "centerOfMass", paramNode);
		}
		else if (reference == DAE_INERTIA_ELEMENT)
		{
			ANIMATE_ELEMENT(FMVector3, "inertiaTensor", paramNode);
		}
	}
#undef ANIMATE_ELEMENT

	if (physicsMaterial != NULL)
	{
		if (ownsPhysicsMaterial)
		{
			physicsMaterial->WriteToXML(baseNode);
		}
		else
		{
			xmlNode* instanceNode = AddChild(baseNode, DAE_INSTANCE_PHYSICS_MATERIAL_ELEMENT);
			AddAttribute(instanceNode, DAE_URL_ATTRIBUTE, string("#") + physicsMaterial->GetDaeId());
		}
	}

	for(FCDPhysicsShapeContainer::const_iterator it = physicsShape.begin(); it != physicsShape.end(); ++it)
	{
		(*it)->WriteToXML(baseNode);
	}

	FCDEntity::WriteToExtraXML(physicsRigidBodyNode);
	return physicsRigidBodyNode;
}
