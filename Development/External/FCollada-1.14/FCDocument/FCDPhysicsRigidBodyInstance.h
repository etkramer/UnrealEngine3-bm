/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FCD_PHYSICS_RIGID_BODY_ENTITY_H_
#define _FCD_PHYSICS_RIGID_BODY_ENTITY_H_

#ifndef _FCD_ENTITY_INSTANCE_H_
#include "FCDocument/FCDEntityInstance.h"
#endif // _FCD_ENTITY_INSTANCE_H_
#ifndef _FCD_PHYSICS_PARAMETER_H_
#include "FCDocument/FCDPhysicsParameter.h"
#endif // _FCD_PHYSICS_PARAMETER_H_

class FCDocument;
class FCDSceneNode;
class FCDPhysicsRigidBody;
class FCDPhysicsMaterial;
class FCDPhysicsModelInstance;
class FCDPhysicsParameterGeneric;
class FCDExternalReference;

typedef FUObjectContainer<FCDPhysicsParameterGeneric> FCDPhysicsParameterContainer;

class FCOLLADA_EXPORT FCDPhysicsRigidBodyInstance : public FCDEntityInstance
{
private:
	DeclareObjectType(FCDEntityInstance);
	FCDPhysicsModelInstance* parent;
	FCDPhysicsParameterContainer parameters;

	FUObjectPtr<FCDPhysicsMaterial> physicsMaterial;
	FUObjectPtr<FCDSceneNode> targetNode;
	bool ownsPhysicsMaterial;
	FCDEntityInstance* instanceMaterialRef;


public:
	FCDPhysicsRigidBodyInstance(FCDocument* document, FCDPhysicsModelInstance* _parent);
	virtual ~FCDPhysicsRigidBodyInstance();

	FCDPhysicsParameterContainer& GetParameters() { return parameters; }
	void AddParameter(FCDPhysicsParameterGeneric* parameter);
	template <class _T> void AddParameter(const char* name, _T value) { FCDPhysicsParameter<_T>* p = new FCDPhysicsParameter<_T>(GetDocument(), name); p->SetValue(value); AddParameter(p); }

	FCDPhysicsMaterial* GetPhysicsMaterial() { return physicsMaterial; }
	const FCDPhysicsMaterial* GetPhysicsMaterial() const { return physicsMaterial; }
	void SetPhysicsMaterial(FCDPhysicsMaterial* material);
	FCDPhysicsMaterial* AddOwnPhysicsMaterial();

	inline FCDPhysicsRigidBody* GetRigidBody() { return (FCDPhysicsRigidBody*) GetEntity(); }
	inline void SetRigidBody(FCDPhysicsRigidBody* rigidBody) { SetEntity((FCDEntity*) rigidBody); }
	FCDPhysicsRigidBody* FlattenRigidBody();

	FCDSceneNode* GetTargetNode() { return targetNode; }
	const FCDSceneNode* GetTargetNode() const { return targetNode; }
	void SetTargetNode(FCDSceneNode* target) { targetNode = target;	SetDirtyFlag(); }

	// FCDEntity override for RTTI-like
	virtual Type GetType() const { return PHYSICS_RIGID_BODY; }

	// Load the geometry instance from the COLLADA document
	virtual FUStatus LoadFromXML(xmlNode* instanceNode);

	// Write out the instantiation information to the XML node tree
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
};

#endif // _FCD_PHYSICS_RIGID_BODY_ENTITY_H_
