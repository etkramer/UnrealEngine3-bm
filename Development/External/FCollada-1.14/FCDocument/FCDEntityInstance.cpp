/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDEntity.h"
#include "FCDocument/FCDEntityInstance.h"
#include "FCDocument/FCDExternalReference.h"
#include "FCDocument/FCDExternalReferenceManager.h"
#include "FCDocument/FCDPlaceHolder.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
#include "FUtils/FUFileManager.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDEntityInstance);

FCDEntityInstance::FCDEntityInstance(FCDocument* document, FCDSceneNode* _parent, FCDEntity::Type type)
:	FCDObject(document), entityType(type)
,	entity(NULL), parent(_parent), externalReference(NULL)
,	useExternalReferenceID(false)
{
}

FCDEntityInstance::~FCDEntityInstance()
{
	if (externalReference != NULL)
	{
		UntrackObject(externalReference);
		SAFE_DELETE(externalReference);
	}

	if (entity != NULL)
	{
		UntrackObject(entity);
		entity = NULL;
	}
}

FUStatus FCDEntityInstance::LoadFromXML(xmlNode* instanceNode)
{
	FUStatus status;

	FUUri uri = ReadNodeUrl(instanceNode);
	if (!uri.prefix.empty())
	{
		FCDExternalReference* reference = GetExternalReference();
		reference->SetUri(uri);
	}
	else
	{
		LoadExternalEntity(GetDocument(), uri.suffix);
		if (entity == NULL)
		{
			status.Warning(FS("Unable to find instantiated entity: ") + TO_FSTRING(uri.suffix) + FC("."), instanceNode->line);
		}
	}
	SetDirtyFlag();
	return status;
}

void FCDEntityInstance::LoadExternalEntity(FCDocument* externalDocument, string daeId)
{
	if (externalDocument == NULL || entity != NULL) return;

	FCDEntity* instantiatedEntity = NULL;
	switch (entityType)
	{
	case FCDEntity::ANIMATION: instantiatedEntity = (FCDEntity*) externalDocument->FindAnimation(daeId); break;
	case FCDEntity::CAMERA: instantiatedEntity = (FCDEntity*) externalDocument->FindCamera(daeId); break;
	case FCDEntity::LIGHT: instantiatedEntity = (FCDEntity*) externalDocument->FindLight(daeId); break;
	case FCDEntity::GEOMETRY: instantiatedEntity = (FCDEntity*) externalDocument->FindGeometry(daeId); break;
	case FCDEntity::CONTROLLER: instantiatedEntity = (FCDEntity*) externalDocument->FindController(daeId); break;
	case FCDEntity::MATERIAL: instantiatedEntity = (FCDEntity*) externalDocument->FindMaterial(daeId); break;
	case FCDEntity::EFFECT: instantiatedEntity = (FCDEntity*) externalDocument->FindEffect(daeId); break;
	case FCDEntity::SCENE_NODE: instantiatedEntity = (FCDEntity*) externalDocument->FindSceneNode(daeId); break;
	case FCDEntity::FORCE_FIELD: instantiatedEntity = (FCDEntity*) externalDocument->FindForceField(daeId); break;
	case FCDEntity::PHYSICS_MATERIAL: instantiatedEntity = (FCDEntity*) externalDocument->FindPhysicsMaterial(daeId); break;
	case FCDEntity::PHYSICS_MODEL: instantiatedEntity = (FCDEntity*) externalDocument->FindPhysicsModel(daeId); break;
	default: break;
	}

	if (instantiatedEntity != NULL)
	{
		SetEntity(instantiatedEntity);
	}
}

// Write out the instantiation information to the XML node tree
xmlNode* FCDEntityInstance::WriteToXML(xmlNode* parentNode) const
{
	const char* instanceEntityName = GetInstanceClassType(entityType);
	xmlNode* instanceNode = AddChild(parentNode, instanceEntityName);
	string urlString = "#";
	if (externalReference != NULL)
	{
		FUUri uri = externalReference->GetUri();
		urlString = TO_STRING(uri.prefix) + string("#");
		if( entity == NULL || useExternalReferenceID ) urlString += uri.suffix;
	}

	if (entity != NULL && !useExternalReferenceID)
	{
		urlString += entity->GetDaeId();
	}

	AddAttribute(instanceNode, DAE_URL_ATTRIBUTE, urlString);
	return instanceNode;
}

FCDExternalReference* FCDEntityInstance::GetExternalReference()
{
	if (externalReference == NULL)
	{
		externalReference = GetDocument()->GetExternalReferenceManager()->AddExternalReference(this);
		TrackObject(externalReference);
	}
	return externalReference;
}

// Retrieves the COLLADA name for the instantiation of a given entity type.
const char* FCDEntityInstance::GetInstanceClassType(FCDEntity::Type type)
{
	const char* instanceEntityName;
	switch (type)
	{
	case FCDEntity::ANIMATION: instanceEntityName = DAE_INSTANCE_ANIMATION_ELEMENT; break;
	case FCDEntity::CAMERA: instanceEntityName = DAE_INSTANCE_CAMERA_ELEMENT; break;
	case FCDEntity::CONTROLLER: instanceEntityName = DAE_INSTANCE_CONTROLLER_ELEMENT; break;
	case FCDEntity::EFFECT: instanceEntityName = DAE_INSTANCE_EFFECT_ELEMENT; break;
	case FCDEntity::GEOMETRY: instanceEntityName = DAE_INSTANCE_GEOMETRY_ELEMENT; break;
	case FCDEntity::LIGHT: instanceEntityName = DAE_INSTANCE_LIGHT_ELEMENT; break;
	case FCDEntity::MATERIAL: instanceEntityName = DAE_INSTANCE_MATERIAL_ELEMENT; break;
	case FCDEntity::PHYSICS_MODEL: instanceEntityName = DAE_INSTANCE_PHYSICS_MODEL_ELEMENT; break;
	case FCDEntity::PHYSICS_RIGID_BODY: instanceEntityName = DAE_INSTANCE_RIGID_BODY_ELEMENT; break;
	case FCDEntity::PHYSICS_RIGID_CONSTRAINT: instanceEntityName = DAE_INSTANCE_RIGID_CONSTRAINT_ELEMENT; break;
	case FCDEntity::SCENE_NODE: instanceEntityName = DAE_INSTANCE_NODE_ELEMENT; break;

	case FCDEntity::ANIMATION_CLIP:
	case FCDEntity::ENTITY:
	case FCDEntity::IMAGE:
	case FCDEntity::TEXTURE:
	default: instanceEntityName = DAEERR_UNKNOWN_ELEMENT;
	}
	return instanceEntityName;
}

const FCDEntity* FCDEntityInstance::GetEntity() const
{
	if (entity != NULL) return entity;
	if (externalReference != NULL)
	{
		FCDPlaceHolder* placeHolder = externalReference->GetPlaceHolder();
		if (placeHolder != NULL)
		{
			const_cast<FCDEntityInstance*>(this)->LoadExternalEntity(placeHolder->GetTarget(FCollada::GetDereferenceFlag()), externalReference->GetEntityId());
		}
	}
	return entity;
}

void FCDEntityInstance::SetEntity(FCDEntity* _entity)
{
	// Stop tracking the old entity
	if (entity != NULL)
	{
		UntrackObject(entity);
		entity = NULL;
	}

	if (_entity != NULL && _entity->GetType() == entityType)
	{
		// Track the new entity
		entity = _entity;
		TrackObject(entity);

		// Update the external reference
		if (entity->GetDocument() == GetDocument())
		{
			UntrackObject(externalReference);
			SAFE_DELETE(externalReference);
		}
		else
		{
			if (externalReference == NULL)
			{
				externalReference = GetDocument()->GetExternalReferenceManager()->AddExternalReference(this);
				TrackObject(externalReference);
			}
			externalReference->SetEntityDocument(entity->GetDocument());
			externalReference->SetEntityId(entity->GetDaeId());
		}
	}
	SetDirtyFlag();
}

FCDEntityInstance* FCDEntityInstance::Clone(FCDEntityInstance* clone) const
{
	if (clone == NULL)
	{
		clone = new FCDEntityInstance(const_cast<FCDocument*>(GetDocument()), const_cast<FCDSceneNode*>(parent), entityType);
	}

	clone->SetEntity(entity);
	return clone;
}

// Callback when an object tracked by this tracker is being released.
void FCDEntityInstance::OnObjectReleased(FUObject* object)
{
	if (object == entity)
	{
		entity = NULL;
		if (externalReference == NULL || externalReference->GetPlaceHolder()->IsTargetLoaded())
		{
			Release();
		}
	}
	else if (object == externalReference)
	{
		externalReference = NULL;
		Release();
	}
}