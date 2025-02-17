/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	Based on the FS Import classes:
	Copyright (C) 2005-2006 Feeling Software Inc
	Copyright (C) 2005-2006 Autodesk Media Entertainment
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDPhysicsModel.h"
#include "FCDocument/FCDPhysicsModelInstance.h"
#include "FCDocument/FCDPhysicsScene.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
#include "FUtils/FUFileManager.h"
#include "FUtils/FUStringConversion.h"
#include "FCDocument/FCDExtra.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDPhysicsScene);

FCDPhysicsScene::FCDPhysicsScene(FCDocument* document)
:	FCDEntity(document, "PhysicsSceneNode")
{
	//FIXME: no default values are specified in the 1.4 spec!
	gravity.x = 0.f; gravity.y = -9.8f; gravity.z = 0.f;
	timestep = 1.f;
}

FCDPhysicsScene::~FCDPhysicsScene()
{
}

FCDPhysicsModelInstance* FCDPhysicsScene::AddInstance(FCDPhysicsModel* model)
{
	FCDPhysicsModelInstance* instance = instances.Add(GetDocument());
	instance->SetEntity(model);
	SetDirtyFlag();
	return instance;
}

// Parse a <scene> or a <node> node from a COLLADA document
FUStatus FCDPhysicsScene::LoadFromXML(xmlNode* sceneNode)
{
	FUStatus status = FCDEntity::LoadFromXML(sceneNode);
	if (!status) return status;

	if(IsEquivalent(sceneNode->name, DAE_PHYSICS_SCENE_ELEMENT))
	{
		for (xmlNode* child = sceneNode->children; child != NULL; child = child->next)
		{
			if (child->type != XML_ELEMENT_NODE) continue;

			// Look for instantiation elements
			if (IsEquivalent(child->name, DAE_INSTANCE_PHYSICS_MODEL_ELEMENT)) 
			{
				FCDPhysicsModelInstance* instance = AddInstance(NULL);
				status.AppendStatus(instance->LoadFromXML(child));
				continue; 
			}
			else if(IsEquivalent(child->name, DAE_TECHNIQUE_COMMON_ELEMENT))
			{
				xmlNode* gravityNode = FindChildByType(child, DAE_GRAVITY_ATTRIBUTE);
				if(gravityNode)
				{
					const char* gravityVal = ReadNodeContentDirect(gravityNode);
					gravity.x = FUStringConversion::ToFloat(&gravityVal);
					gravity.y = FUStringConversion::ToFloat(&gravityVal);
					gravity.z = FUStringConversion::ToFloat(&gravityVal);
				}
				xmlNode* timestepNode = FindChildByType(child, DAE_TIME_STEP_ATTRIBUTE);
				if(timestepNode)
				{
					timestep = FUStringConversion::ToFloat(ReadNodeContentDirect(timestepNode));
				}
			}
			else if (IsEquivalent(child->name, DAE_EXTRA_ELEMENT))
			{
				// The extra information is loaded by the FCDEntity class.
			}
		}
	}

	SetDirtyFlag();
	return status;
}

// Write out a <physics_scene> element to a COLLADA XML document
xmlNode* FCDPhysicsScene::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* physicsSceneNode = WriteToEntityXML(parentNode, DAE_PHYSICS_SCENE_ELEMENT);
	if (physicsSceneNode == NULL) return physicsSceneNode;
	WriteToNodeXML(physicsSceneNode);

	return physicsSceneNode;
}

// Write out a <physics_scene>  element to a COLLADA XML document
void FCDPhysicsScene::WriteToNodeXML(xmlNode* node) const
{
	// Write out the instantiation
	for (FCDPhysicsModelInstanceList::const_iterator itI = instances.begin(); itI != instances.end(); ++itI)
	{
		FCDEntityInstance* instance = (*itI);
		instance->WriteToXML(node);
	}

	// Add COMMON technique.
	xmlNode* techniqueNode = AddChild(node, DAE_TECHNIQUE_COMMON_ELEMENT);
	AddChild(techniqueNode, DAE_GRAVITY_ATTRIBUTE, TO_STRING(gravity));
	AddChild(techniqueNode, DAE_TIME_STEP_ATTRIBUTE, timestep);

	// Write out the extra information
	FCDEntity::WriteToExtraXML(node);
}
