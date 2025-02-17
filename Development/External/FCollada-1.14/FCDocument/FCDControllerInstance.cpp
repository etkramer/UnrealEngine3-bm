/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDController.h"
#include "FCDocument/FCDControllerInstance.h"
#include "FCDocument/FCDSceneNode.h"
#include "FCDocument/FCDSkinController.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeWriter;

//
// FCDControllerInstance
//

ImplementObjectType(FCDControllerInstance);

FCDControllerInstance::FCDControllerInstance(FCDocument* document, FCDSceneNode* parent, FCDEntity::Type entityType)
:	FCDGeometryInstance(document, parent, entityType)
{
}

FCDControllerInstance::~FCDControllerInstance()
{
}

// Retrieves a list of all the root joints for the controller.
void FCDControllerInstance::GetSkeletonRoots(FCDSceneNodeList& parentJoints) const
{
	if (GetEntity() == NULL || GetEntity()->GetType() != FCDEntity::CONTROLLER) return;

	const FCDController* controller = (const FCDController*) GetEntity();
	if (!controller->HasSkinController()) return;

	const FCDSkinController* skin = (const FCDSkinController*) controller->GetSkinController();
	const FCDJointList& joints = skin->GetJoints();
	for (FCDJointList::const_iterator itJ = joints.begin(); itJ != joints.end(); ++itJ)
	{
		const FCDSceneNode* joint = (*itJ).joint;
		if (joint == NULL) continue;

		bool addToList = true;
		size_t parentCount = joint->GetParentCount();
		for (size_t p = 0; p < parentCount; ++p)
		{
			const FCDSceneNode* parentJoint = joint->GetParent(p);
			if (skin->FindJoint(parentJoint) != NULL)
			{
				addToList = false;
				break;
			}
		}

		if (addToList)
		{
			parentJoints.push_back(const_cast<FCDSceneNode*>(joint));
		}
	}
}

// Writes out the controller instance to the given COLLADA XML tree node.
xmlNode* FCDControllerInstance::WriteToXML(xmlNode* parentNode) const
{
	// Export the geometry instantiation information.
	xmlNode* instanceNode = FCDGeometryInstance::WriteToXML(parentNode);
	xmlNode* insertBeforeNode = (instanceNode != NULL) ? instanceNode->children : NULL;

	// Retrieve the parent joints and export the <skeleton> elements.
	FCDSceneNodeList parentJoints;
	GetSkeletonRoots(parentJoints);
	for (FCDSceneNodeList::iterator itS = parentJoints.begin(); itS != parentJoints.end(); ++itS)
	{
		globalSBuilder.set('#'); globalSBuilder.append((*itS)->GetDaeId());
		xmlNode* skeletonNode = InsertChild(instanceNode, insertBeforeNode, DAE_SKELETON_ELEMENT);
		AddContent(skeletonNode, globalSBuilder);
	}

	return instanceNode;
}
