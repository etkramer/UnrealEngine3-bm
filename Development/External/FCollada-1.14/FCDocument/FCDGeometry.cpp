/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDAnimated.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometrySpline.h"
#include "FCDocument/FCDGeometryNURBSSurface.h"
#include "FUtils/FUStringConversion.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

ImplementObjectType(FCDGeometry);

FCDGeometry::FCDGeometry(FCDocument* document)
:	FCDEntity(document, "Geometry")
{
}

FCDGeometry::~FCDGeometry()
{
}

// Sets the type of this geometry to mesh and creates an empty mesh structure.
FCDGeometryMesh* FCDGeometry::CreateMesh()
{
	spline = NULL;
	pSurface = NULL;
	mesh = new FCDGeometryMesh(GetDocument(), this);
	SetDirtyFlag();
	return mesh;
}

// Sets the type of this geometry to spline and creates an empty spline structure.
FCDGeometrySpline* FCDGeometry::CreateSpline()
{
	mesh = NULL;
	pSurface = NULL;
	spline = new FCDGeometrySpline(GetDocument(), this);
	SetDirtyFlag();
	return spline;
}

// Sets the type of this geometry to spline and creates an empty spline structure.
FCDGeometryPSurface* FCDGeometry::CreatePSurface()
{
	mesh = NULL;
	spline = NULL;
	pSurface = new FCDGeometryPSurface(GetDocument(), this);
	SetDirtyFlag();
	return pSurface;
}

// Create a copy of this geometry, with the vertices overwritten
FCDGeometry* FCDGeometry::Clone(FloatList& newPositions, uint32 newPositionsStride, FloatList& newNormals, uint32 newNormalsStride)
{
	// Clone only mesh geometries. This function is necessary for COLLADA 1.3 backward compatibility and should not be used
	// in some other way (yet)
	if (!IsMesh()) return NULL;
	
	FCDGeometry* clone = new FCDGeometry(GetDocument());
	clone->mesh = mesh->Clone(newPositions, newPositionsStride, newNormals, newNormalsStride);
	return clone;
}

// Load from a XML node the given geometry
FUStatus FCDGeometry::LoadFromXML(xmlNode* geometryNode)
{
	mesh = NULL;
	spline = NULL;
	pSurface = NULL;

	FUStatus status = FCDEntity::LoadFromXML(geometryNode);
	if (!status) return status;
	if (!IsEquivalent(geometryNode->name, DAE_GEOMETRY_ELEMENT))
	{
		return status.Warning(FS("Geometry library contains unknown element."), geometryNode->line);
	}

	// Read in the first valid child element found
	for (xmlNode* child = geometryNode->children; child != NULL; child = child->next)
	{
		if (child->type != XML_ELEMENT_NODE) continue;

		if (IsEquivalent(child->name, DAE_MESH_ELEMENT))
		{
			// Create a new mesh
			FCDGeometryMesh* m = CreateMesh();
			m->SetConvex(false);
			status.AppendStatus(m->LoadFromXML(child));
			break;
		}
		else if( IsEquivalent(child->name, DAE_CONVEX_MESH_ELEMENT))
		{
			// Create a new convex mesh
			FCDGeometryMesh* m = CreateMesh();
			m->SetConvex(true);
			status.AppendStatus(m->LoadFromXML(child));
			break;
		}
		else if (IsEquivalent(child->name, DAE_SPLINE_ELEMENT))
		{
			// Create a new spline
			FCDGeometrySpline* s = CreateSpline();
			status.AppendStatus(s->LoadFromXML(child));
			break;
		}
		else if (IsEquivalent(child->name, DAE_PARAMETERIZED_SURFACE_ELEMENT) )
		{
			// Create a new NURBS surface set
			FCDGeometryPSurface* set = CreatePSurface();
			status.AppendStatus(set->LoadFromXML(child));
			break;
		}
		else
		{
			return status.Fail(FS("Unknown child in <geometry> with id: ") + TO_FSTRING(GetDaeId()), child->line);
		}
	}

	if (mesh == NULL && spline == NULL && pSurface == NULL )
	{
		status.Warning(FS("No mesh, spline or NURBS surfaces found within geometry: ") + TO_FSTRING(GetDaeId()), geometryNode->line);
	}

	SetDirtyFlag();
	return status;
}

// Write out the <geometry> node
xmlNode* FCDGeometry::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* geometryNode = WriteToEntityXML(parentNode, DAE_GEOMETRY_ELEMENT);

	if (mesh != NULL) mesh->WriteToXML(geometryNode);
	else if (spline != NULL) spline->WriteToXML(geometryNode);
	else if( pSurface != NULL ) pSurface->WriteToXML(geometryNode);

	FCDEntity::WriteToExtraXML(geometryNode);
	return geometryNode;
}
