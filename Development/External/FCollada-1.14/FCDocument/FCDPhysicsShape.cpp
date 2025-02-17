/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDPhysicsShape.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDTransform.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometryInstance.h"
#include "FCDocument/FCDPhysicsRigidBody.h"
#include "FCDocument/FCDPhysicsMaterial.h"
#include "FCDocument/FCDPhysicsAnalyticalGeometry.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDPhysicsShape);

FCDPhysicsShape::FCDPhysicsShape(FCDocument* document) : FCDObject(document)
{
	hollow = true;
	physicsMaterial = NULL;
	ownsPhysicsMaterial = false;
	ownsGeometryInstance = false;
	geometry = NULL;
	analGeom = NULL;
	mass = NULL;
	density = NULL;
	instanceMaterialRef = NULL;
}

FCDPhysicsShape::~FCDPhysicsShape()
{
	SetPhysicsMaterial(NULL);
	SAFE_DELETE(mass);
	SAFE_DELETE(density);
	SAFE_DELETE(instanceMaterialRef);

	if(ownsGeometryInstance)
	{
		SAFE_DELETE(geometry);
	}
	else
	{
		geometry = NULL;
	}
}

FCDTransform* FCDPhysicsShape::AddTransform(FCDTransform::Type type, size_t index)
{
	FCDTransform* transform = FCDTFactory::CreateTransform(GetDocument(), NULL, type);
	if (transform != NULL)
	{
		if (index > transforms.size()) transforms.push_back(transform);
		else transforms.insert(transforms.begin() + index, transform);
	}
	SetDirtyFlag();
	return transform;
}

FCDPhysicsMaterial* FCDPhysicsShape::AddOwnPhysicsMaterial()
{
	if (ownsPhysicsMaterial) SAFE_DELETE(physicsMaterial);

	physicsMaterial = new FCDPhysicsMaterial(GetDocument());
	ownsPhysicsMaterial = true;
	SetDirtyFlag();
	return physicsMaterial;
}

void FCDPhysicsShape::SetPhysicsMaterial(FCDPhysicsMaterial* _physicsMaterial)
{
	if (ownsPhysicsMaterial) SAFE_DELETE(physicsMaterial);
	physicsMaterial = _physicsMaterial;
	SetDirtyFlag();
}

FCDGeometryInstance* FCDPhysicsShape::CreateGeometryInstance(FCDGeometry* geom, bool createConvexMesh)
{
	analGeom = NULL;
	if (ownsGeometryInstance) SAFE_DELETE(geometry);
	
	geometry = new FCDGeometryInstance(GetDocument(), NULL, FCDEntity::GEOMETRY);

	if(createConvexMesh)
	{
		FCDGeometry* convexHullGeom = GetDocument()->GetGeometryLibrary()->AddEntity();
		string convexId = geom->GetDaeId()+"-convex";
		convexHullGeom->SetDaeId(convexId);
		convexHullGeom->SetName(FUStringConversion::ToFString(convexId));
		FCDGeometryMesh* convexHullGeomMesh = convexHullGeom->CreateMesh();
		convexHullGeomMesh->SetConvexHullOf(geom);
		convexHullGeomMesh->SetConvex(true);
		geometry->SetEntity(convexHullGeom);
	}
	else
	{
		geometry->SetEntity(geom);
	}

	SetDirtyFlag();
	return geometry;
}

FCDPhysicsAnalyticalGeometry* FCDPhysicsShape::CreateAnalyticalGeometry(FCDPhysicsAnalyticalGeometry::GeomType type)
{
	if (ownsGeometryInstance) SAFE_DELETE(geometry);

	switch (type)
	{
	case FCDPhysicsAnalyticalGeometry::BOX: analGeom = new FCDPASBox(GetDocument()); break;
	case FCDPhysicsAnalyticalGeometry::PLANE: analGeom = new FCDPASPlane(GetDocument()); break;
	case FCDPhysicsAnalyticalGeometry::SPHERE: analGeom = new FCDPASSphere(GetDocument()); break;
	case FCDPhysicsAnalyticalGeometry::CYLINDER: analGeom = new FCDPASCylinder(GetDocument()); break;
	case FCDPhysicsAnalyticalGeometry::CAPSULE: analGeom = new FCDPASCapsule(GetDocument()); break;
	case FCDPhysicsAnalyticalGeometry::TAPERED_CAPSULE: analGeom = new FCDPASTaperedCapsule(GetDocument()); break;
	case FCDPhysicsAnalyticalGeometry::TAPERED_CYLINDER: analGeom = new FCDPASTaperedCylinder(GetDocument()); break;
	default: analGeom = NULL; break;
	}

	SetDirtyFlag();
	return analGeom;
}

// Create a copy of this shape
// Note: geometries are just shallow-copied
FCDPhysicsShape* FCDPhysicsShape::Clone()
{
	FCDPhysicsShape* clone = new FCDPhysicsShape(GetDocument());

	if(mass) clone->SetMass(*mass);
	if(density) clone->SetDensity(*density);
	clone->SetHollow(hollow);

	clone->instanceMaterialRef = (instanceMaterialRef != NULL) ? instanceMaterialRef->Clone() : NULL;
	clone->analGeom = (analGeom != NULL) ? analGeom->Clone() : NULL;
	clone->geometry = (geometry != NULL) ? geometry : NULL;
	clone->ownsGeometryInstance = false;

	if (physicsMaterial != NULL)
	{
		clone->SetPhysicsMaterial(physicsMaterial);
	}

	for(size_t i = 0; i < transforms.size(); ++i)
	{
		clone->transforms.push_back(transforms[i]->Clone(NULL));
	}

	return clone;
}


// Load from a XML node the given physicsShape
FUStatus FCDPhysicsShape::LoadFromXML(xmlNode* physicsShapeNode)
{
	FUStatus status;
	if (!IsEquivalent(physicsShapeNode->name, DAE_SHAPE_ELEMENT))
	{
		return status.Warning(FS("PhysicsShape library contains unknown element."), physicsShapeNode->line);
	}

	// Read in the first valid child element found
	for (xmlNode* child = physicsShapeNode->children; child != NULL; child = child->next)
	{
		if (child->type != XML_ELEMENT_NODE) continue;

		if(IsEquivalent(child->name, DAE_HOLLOW_ELEMENT))
		{
			hollow = FUStringConversion::ToBoolean(ReadNodeContentDirect(child));
		}
		else if(IsEquivalent(child->name, DAE_MASS_ELEMENT))
		{
			SAFE_DELETE(mass);
			mass = new float;
			*mass = FUStringConversion::ToFloat(ReadNodeContentDirect(child));
		}
		else if(IsEquivalent(child->name, DAE_DENSITY_ELEMENT))
		{
			SAFE_DELETE(density);
			density = new float;
			*density = FUStringConversion::ToFloat(ReadNodeContentDirect(child));
		}
		else if(IsEquivalent(child->name, DAE_PHYSICS_MATERIAL_ELEMENT))
		{
			instanceMaterialRef = new FCDEntityInstance(GetDocument(), NULL, FCDEntity::PHYSICS_MATERIAL);
			instanceMaterialRef->LoadFromXML(child);

			if(!HasNodeProperty(child, DAE_URL_ATTRIBUTE))
			{
				//inline definition of physics_material
				FCDPhysicsMaterial* material = AddOwnPhysicsMaterial();
				material->LoadFromXML(child);
				instanceMaterialRef->SetEntity(material);
			}
		}
		else if(IsEquivalent(child->name, DAE_INSTANCE_GEOMETRY_ELEMENT))
		{
			FUUri url = ReadNodeUrl(child);
			if (url.prefix.empty())
			{
				FCDGeometry* entity = GetDocument()->FindGeometry(url.suffix);
				if (entity != NULL)
				{
					analGeom = NULL;
					geometry = new FCDGeometryInstance(GetDocument(), NULL);
					geometry->SetEntity((FCDEntity*)entity);
					status.AppendStatus(geometry->LoadFromXML(child));
					continue; 
				}
			}
			status.Warning(FS("Unable to retrieve FCDGeometry instance for scene node. "), child->line);
		}

#define PARSE_ANALYTICAL_SHAPE(type, nodeName) \
		else if (IsEquivalent(child->name, nodeName)) { \
			FCDPhysicsAnalyticalGeometry* analytical = CreateAnalyticalGeometry(FCDPhysicsAnalyticalGeometry::type); \
			status = analytical->LoadFromXML(child); \
			if (!status) { \
				status.Warning(fstring(FC("Invalid <") FC(nodeName) FC("> shape in node. ")), child->line); break; \
			} }

		PARSE_ANALYTICAL_SHAPE(BOX, DAE_BOX_ELEMENT)
		PARSE_ANALYTICAL_SHAPE(PLANE, DAE_PLANE_ELEMENT)
		PARSE_ANALYTICAL_SHAPE(SPHERE, DAE_SPHERE_ELEMENT)
		PARSE_ANALYTICAL_SHAPE(CYLINDER, DAE_CYLINDER_ELEMENT)
		PARSE_ANALYTICAL_SHAPE(CAPSULE, DAE_CAPSULE_ELEMENT)
		PARSE_ANALYTICAL_SHAPE(TAPERED_CAPSULE, DAE_TAPERED_CAPSULE_ELEMENT)
		PARSE_ANALYTICAL_SHAPE(TAPERED_CYLINDER, DAE_TAPERED_CYLINDER_ELEMENT)
#undef PARSE_ANALYTICAL_SHAPE


		// Parse the physics shape transforms <rotate>, <translate> are supported.
		else if(IsEquivalent(child->name, DAE_ASSET_ELEMENT)) {}
		else if(IsEquivalent(child->name, DAE_EXTRA_ELEMENT)) {}
		else
		{
			FCDTransform* transform = FCDTFactory::CreateTransform(GetDocument(), NULL, child);
			if (transform != NULL && (transform->GetType() != FCDTransform::TRANSLATION
				&& transform->GetType() != FCDTransform::ROTATION ))
			{
				SAFE_DELETE(transform);
			}
			else if(transform != NULL)
			{
				transforms.push_back(transform);
				status.AppendStatus(transform->LoadFromXML(child));
			}
		}
	}

	SetDirtyFlag();
	return status;
}

// Write out the <physicsShape> node
xmlNode* FCDPhysicsShape::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* physicsShapeNode = AddChild(parentNode, DAE_SHAPE_ELEMENT);

	AddChild(physicsShapeNode, DAE_HOLLOW_ELEMENT, hollow?"true":"false");
	if(mass)
		AddChild(physicsShapeNode, DAE_MASS_ELEMENT, FUStringConversion::ToString(*mass));
	if(density)
		AddChild(physicsShapeNode, DAE_DENSITY_ELEMENT, FUStringConversion::ToString(*density));

	if(ownsPhysicsMaterial && physicsMaterial)
	{
		xmlNode* materialNode = AddChild(physicsShapeNode, DAE_PHYSICS_MATERIAL_ELEMENT);
		physicsMaterial->WriteToXML(materialNode);
	}
	else if(instanceMaterialRef)
	{
		instanceMaterialRef->WriteToXML(physicsShapeNode);
	}
	
	if(geometry)
		geometry->WriteToXML(physicsShapeNode);
	if(analGeom)
		analGeom->WriteToXML(physicsShapeNode);

	for(FCDTransformContainer::const_iterator it = transforms.begin(); it != transforms.end(); ++it)
	{
		(*it)->WriteToXML(physicsShapeNode);
	}

	return physicsShapeNode;
}


float FCDPhysicsShape::GetMass()
{
	if(mass) 
		return *mass;
		
	return 0.f;
}

void FCDPhysicsShape::SetMass(float _mass)
{
	SAFE_DELETE(mass);
	mass = new float;
	*mass = _mass;
	SetDirtyFlag();
}

float FCDPhysicsShape::GetDensity() 
{
	if(density) 
		return *density; 

	return 0.f;
}

void FCDPhysicsShape::SetDensity(float _density) 
{
	SAFE_DELETE(density);
	density = new float;
	*density = _density;
	SetDirtyFlag();
}