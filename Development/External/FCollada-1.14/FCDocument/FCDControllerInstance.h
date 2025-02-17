/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDControllerInstance.h
	This file contains the FCDControllerInstance class.
*/

#ifndef _FCD_CONTROLLER_INSTANCE_H_
#define _FCD_CONTROLLER_INSTANCE_H_

#ifndef _FCD_GEOMETRY_ENTITY_H_
#include "FCDocument/FCDGeometryInstance.h"
#endif // _FCD_GEOMETRY_ENTITY_H_

class FCDSceneNode;
typedef vector<FCDSceneNode*> FCDSceneNodeList; /**< A dynamically-sized array of visual scene nodes. */

/**
	A COLLADA controller instance.

	When a COLLADA controller is instantiated, all its target(s)
	are instantiated in order to use them for the rendering or
	the logic. As such, all the information necessary to
	instantiate a geometry is also necessary to instantiate a
	controller.

	Each COLLADA skin controller should instantiate its own skeleton,
	for this reason, the skeleton root(s) are defined at instantiation.

	In FCollada, we do not buffer the skeleton information. Instead, it is
	calculate when needed for the export or if requested by the user.
*/
class FCDControllerInstance : public FCDGeometryInstance
{
private:
    DeclareObjectType(FCDGeometryInstance);

public:
	/** Constructor: do not use directly.
		Instead, use the FCDSceneNode::AddInstance function.
		@param document The COLLADA document that owns the controller instance.
		@param parent The parent visual scene node.
		@param entity The controller to instantiate. */
	FCDControllerInstance(FCDocument* document, FCDSceneNode* parent, FCDEntity::Type entityType = FCDEntity::CONTROLLER);

	/** Destructor. */
	virtual ~FCDControllerInstance();

	/** Retrieves a list of all the root joints for the controller.
		As noted above: this information is calculated every time.
		It is not loaded from the COLLADA file.
		@param parentJoints The list of visual scene node to fill in
			with the parent joints. This list is not cleared. */
	void GetSkeletonRoots(FCDSceneNodeList& parentJoints) const;

	/** [INTERNAL] Writes out the controller instance to the given COLLADA XML tree node.
		@param parentNode The COLLADA XML parent node in which to insert the node.
		@return The created XML tree node. */
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
};

#endif // _FCD_CONTROLLER_INSTANCE_H_