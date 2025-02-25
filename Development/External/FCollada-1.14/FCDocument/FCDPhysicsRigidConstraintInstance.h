/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FCD_PHYSICS_RIGID_CONSTRAINT_INSTANCE_H_
#define _FCD_PHYSICS_RIGID_CONSTRAINT_INSTANCE_H_

#ifndef _FCD_ENTITY_INSTANCE_H_
#include "FCDocument/FCDEntityInstance.h"
#endif // _FCD_ENTITY_INSTANCE_H_

class FCDocument;
class FCDEntity;
class FCDSceneNode;
class FCDPhysicsModel;
class FCDPhysicsModelInstance;
class FCDPhysicsRigidConstraint;

class FCOLLADA_EXPORT FCDPhysicsRigidConstraintInstance : public FCDEntityInstance
{
private:
	DeclareObjectType(FCDEntityInstance);
	FCDPhysicsModelInstance* parent;

public:
	FCDPhysicsRigidConstraintInstance(FCDocument* document, FCDPhysicsModelInstance* parent);
	virtual ~FCDPhysicsRigidConstraintInstance();

	// Retrieves the parent physics model.
	FCDPhysicsModelInstance* GetParent() { return parent; }
	const FCDPhysicsModelInstance* GetParent() const { return parent; }

	inline FCDPhysicsRigidConstraint* GetRigidConstraint() { return (FCDPhysicsRigidConstraint*) GetEntity(); }
	inline void SetRigidConstraint(FCDPhysicsRigidConstraint* rigidConstraint) { SetEntity((FCDEntity*) rigidConstraint); }

	// FCDEntity override for RTTI-like
	virtual Type GetType() const { return PHYSICS_RIGID_CONSTRAINT; }

	// Load the geometry instance from the COLLADA document
	virtual FUStatus LoadFromXML(xmlNode* instanceNode);

	// Write out the instantiation information to the XML node tree
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
};

#endif // _FCD_PHYSICS_RIGID_CONSTRAINT_INSTANCE_H_
