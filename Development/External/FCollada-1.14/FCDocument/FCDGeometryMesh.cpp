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
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometryPolygons.h"
#include "FCDocument/FCDGeometryPolygonsSpecial.h"
#include "FCDocument/FCDGeometrySource.h"
#include "FUtils/FUStringConversion.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

// 
// FCDGeometryMesh
//

ImplementObjectType(FCDGeometryMesh);

FCDGeometryMesh::FCDGeometryMesh(FCDocument* document, FCDGeometry* _parent)
:	FCDObject(document), parent(_parent)
,	faceVertexCount(0), faceCount(0), holeCount(0), convexify(false)
,	isConvex(true)
{
}

FCDGeometryMesh::~FCDGeometryMesh()
{
	polygons.clear();
	sources.clear();
	faceVertexCount = faceCount = holeCount = 0;
	parent = NULL;
}

// Retrieve the parent's id
const string& FCDGeometryMesh::GetDaeId() const
{
	return parent->GetDaeId();
}

void FCDGeometryMesh::SetConvexHullOf(FCDGeometry* _geom)
{
	convexHullOf = _geom->GetDaeId();
	SetDirtyFlag();
}

// Search for a data source in the geometry node
FCDGeometrySource* FCDGeometryMesh::FindSourceById(const string& id)
{ return const_cast<FCDGeometrySource*>(const_cast<const FCDGeometryMesh*>(this)->FindSourceById(id)); }
const FCDGeometrySource* FCDGeometryMesh::FindSourceById(const string& id) const
{
	const char* localId = id.c_str();
	if (localId[0] == '#') ++localId;
	for (FCDGeometrySourceContainer::const_iterator it = sources.begin(); it != sources.end(); ++it)
	{
		if ((*it)->GetSourceId() == localId) return (*it);
	}
	return NULL;
}

// Retrieve the source for the given type
FCDGeometrySource* FCDGeometryMesh::FindSourceByType(FUDaeGeometryInput::Semantic type)
{ return const_cast<FCDGeometrySource*>(const_cast<const FCDGeometryMesh*>(this)->FindSourceByType(type)); }
const FCDGeometrySource* FCDGeometryMesh::FindSourceByType(FUDaeGeometryInput::Semantic type) const 
{
	for (FCDGeometrySourceContainer::const_iterator itS = sources.begin(); itS != sources.end(); ++itS)
	{
		if ((*itS)->GetSourceType() == type) return (*itS);
	}
	return NULL;
}

FCDGeometrySource* FCDGeometryMesh::FindSourceByName(const fstring& name)
{ return const_cast<FCDGeometrySource*>(const_cast<const FCDGeometryMesh*>(this)->FindSourceByName(name)); }
const FCDGeometrySource* FCDGeometryMesh::FindSourceByName(const fstring& name) const 
{
	for (FCDGeometrySourceList::const_iterator itS = sources.begin(); itS != sources.end(); ++itS)
	{
		if ((*itS)->GetName() == name) return (*itS);
	}
	return NULL;
}

// Creates a new polygon group.
FCDGeometryPolygons* FCDGeometryMesh::AddPolygons()
{
	FCDGeometryPolygons* polys = new FCDGeometryPolygons(GetDocument(), this);
	polygons.push_back(polys);

	// Add to this new polygons all the per-vertex sources.
	for (FCDGeometrySourceTrackList::iterator itS = vertexSources.begin(); itS != vertexSources.end(); ++itS)
	{
		polys->AddInput(*itS, 0);
	}

	SetDirtyFlag();
	return polys;
}

bool FCDGeometryMesh::IsTriangles() const
{
	bool isTriangles = true;
	for( size_t i = 0; i < polygons.size() && isTriangles; i++ )
	{
		isTriangles &= polygons[i]->IsTriangles();
	}
	return isTriangles;
}

// Retrieves the polygon sets that use a given material semantic
void FCDGeometryMesh::FindPolygonsByMaterial(const fstring& semantic, FCDGeometryPolygonsList& sets)
{
	for (FCDGeometryPolygonsContainer::iterator itP = polygons.begin(); itP != polygons.end(); ++itP)
	{
		if ((*itP)->GetMaterialSemantic() == semantic) sets.push_back(*itP);
	}
}

// Creates a new per-vertex data source
FCDGeometrySource* FCDGeometryMesh::AddVertexSource(FUDaeGeometryInput::Semantic type)
{
	FCDGeometrySource* vertexSource = AddSource(type);
	vertexSources.push_back(vertexSource);

	// Add this new per-vertex data source to all the existing polygon groups, at offset 0.
	for (FCDGeometryPolygonsContainer::iterator itP = polygons.begin(); itP != polygons.end(); ++itP)
	{
		(*itP)->AddInput(vertexSource, 0);
	}

	SetDirtyFlag();
	return vertexSource;
}

// Sets a source as per-vertex data.
void FCDGeometryMesh::AddVertexSource(FCDGeometrySource* source)
{
	FUAssert(source != NULL, return);
	FUAssert(!vertexSources.contains(source), return);

	// Add the source to the list of per-vertex sources.
	vertexSources.push_back(source);

	// Remove any polygon set input that uses the source.
	for (FCDGeometryPolygonsContainer::iterator itP = polygons.begin(); itP != polygons.end(); ++itP)
	{
		FCDGeometryPolygonsInput* input = (*itP)->FindInput(source);
		if (input != NULL)
		{
			(*itP)->ReleaseInput(input);
		}
		(*itP)->AddInput(source, 0);
	}

	SetDirtyFlag();
}

// Creates a new data source
FCDGeometrySource* FCDGeometryMesh::AddSource(FUDaeGeometryInput::Semantic type)
{
	FCDGeometrySource* source = new FCDGeometrySource(GetDocument(), type);
	sources.push_back(source);
	SetDirtyFlag();
	return source;
}

// Creates a new data source
void FCDGeometryMesh::ReleaseSource(FCDGeometrySource* source)
{
	FUAssert(source != NULL, return);
	FUAssert(sources.contains(source), return);

	// Remove any polygon set input that uses the source
	for (FCDGeometryPolygonsContainer::iterator itP = polygons.begin(); itP != polygons.end(); ++itP)
	{
		FCDGeometryPolygonsInput* input = (*itP)->FindInput(source);
		if (input != NULL)
		{
			(*itP)->ReleaseInput(input);
		}
	}

	// Remove the source from our lists
	vertexSources.erase(source);
	sources.release(source);
	SetDirtyFlag();
}

// Recalculates all the hole/vertex/face-vertex counts and offsets within the mesh and its polygons
void FCDGeometryMesh::Recalculate()
{
	faceCount = holeCount = faceVertexCount = 0;
	for (FCDGeometryPolygonsContainer::iterator itP = polygons.begin(); itP != polygons.end(); ++itP)
	{
		FCDGeometryPolygons* polygons = *itP;
		polygons->Recalculate();

		polygons->SetFaceOffset(faceCount);
		polygons->SetHoleOffset(holeCount);
		polygons->SetFaceVertexOffset(faceVertexCount);
		faceCount += polygons->GetFaceCount();
		holeCount += polygons->GetHoleCount();
		faceVertexCount += polygons->GetFaceVertexCount();
	}
	SetDirtyFlag();
}

// Create a copy of this geometry, with the vertices overwritten
FCDGeometryMesh* FCDGeometryMesh::Clone(FloatList& newPositions, uint32 newPositionsStride, FloatList& newNormals, uint32 newNormalsStride)
{
	// Create the clone and fill it with the known information
	FCDGeometryMesh* clone = new FCDGeometryMesh(GetDocument(), NULL);
	CopyMeshToMesh(clone, newPositions, newPositionsStride, newNormals, newNormalsStride);
	clone->convexHullOf = convexHullOf;
	clone->isConvex = isConvex;
	clone->convexify = convexify;
	return clone;
}

void FCDGeometryMesh::CopyMeshToMesh(FCDGeometryMesh* destMesh, const FloatList& newPositions, uint32 newPositionsStride, FloatList& newNormals, uint32 newNormalsStride)
{
	if(!destMesh) return;

	destMesh->faceCount = faceCount;

	// Clone the source data
	FCDGeometrySourceCloneMap cloneMap;
	size_t sourceCount = sources.size();
	destMesh->sources.reserve(sourceCount);
	for (FCDGeometrySourceContainer::const_iterator itS = sources.begin(); itS != sources.end(); ++itS)
	{
		destMesh->sources.push_back((*itS)->Clone());
		if (vertexSources.find(*itS) != vertexSources.end())
		{
			destMesh->vertexSources.push_back(destMesh->sources.back());
		}
		cloneMap.insert(FCDGeometrySourceCloneMap::value_type(*itS, destMesh->sources.back()));
	}

	// Clone the polygons data
	// Gather up the position and normal data sources
	FCDGeometrySourceList positionSources, normalSources;
	size_t polygonsCount = polygons.size();
	destMesh->polygons.reserve(polygonsCount);
	for (size_t i = 0; i < polygonsCount; ++i)
	{
		destMesh->polygons.push_back(polygons[i]->Clone(destMesh, cloneMap));

		// Retrieve the position data source
		FCDGeometryPolygonsInput* positionInput = destMesh->polygons[i]->FindInput(FUDaeGeometryInput::POSITION);
		if (positionInput != NULL)
		{
			FCDGeometrySource* dataSource = positionInput->GetSource();
			FUAssert(dataSource != NULL, continue);
			if (positionSources.find(dataSource) == positionSources.end())
			{
				positionSources.push_back(dataSource);
			}
		}

		// Retrieve the normal data source
		FCDGeometryPolygonsInput* normalInput = destMesh->polygons[i]->FindInput(FUDaeGeometryInput::NORMAL);
		if (normalInput != NULL)
		{
			FCDGeometrySource* dataSource = normalInput->GetSource();
			FUAssert(dataSource != NULL, continue);
			if (normalSources.find(dataSource) == normalSources.end())
			{
				normalSources.push_back(dataSource);
			}
		}
	}

	// Override the position and normal data sources with the given data (from the controller's bind shape)
#define OVERWRITE_SOURCES(cloneSources, newSourceData, newSourceStride) { \
		size_t dataSourceCount = min(cloneSources.size(), newSourceData.size()), offset = 0; \
		for (size_t i = 0; i < dataSourceCount; ++i) { \
			FCDGeometrySource* dataSource = cloneSources[i]; \
			size_t dataCount = dataSource->GetSourceData().size() / dataSource->GetSourceStride(); \
			if (offset + dataCount > newSourceData.size() / newSourceStride) dataCount = newSourceData.size() / newSourceStride - offset; \
			if (dataCount == 0) break; \
			/* Insert the relevant data in this source */ \
			dataSource->SetSourceData(newSourceData, newSourceStride, offset  * newSourceStride, dataCount * newSourceStride); \
			offset += dataCount; \
		} } 

	OVERWRITE_SOURCES(positionSources, newPositions, newPositionsStride);
	OVERWRITE_SOURCES(normalSources, newNormals, newNormalsStride);
#undef OVERWRITE_SOURCES

}

// Link a convex mesh to its source geometry
FUStatus FCDGeometryMesh::Link()
{
	FUStatus status;
	if(!isConvex || convexHullOf.empty())
		return status;

	FCDGeometry* concaveGeom = GetDocument()->FindGeometry(convexHullOf);
	if(concaveGeom)
	{
		FCDGeometryMesh* origMesh = concaveGeom->GetMesh();
		if(origMesh)
		{
			//copy the mesh of the other geometry
			FloatList empty;
			origMesh->CopyMeshToMesh(this, empty, 0, empty, 0);
			//convert to convex hull (for now, raise a flag and let the host application
			//take care of that).
			SetConvexify(true);
		}
		return status;
	}
	else
	{
		return status.Fail(FS("Unknown geometry for creation of convex hull of: ") + TO_FSTRING(parent->GetDaeId()));
	}
}

// Read in the <mesh> node of the COLLADA document
FUStatus FCDGeometryMesh::LoadFromXML(xmlNode* meshNode)
{
	FUStatus status;

	if(isConvex) // <convex_mesh> element
		convexHullOf = ReadNodeProperty(meshNode, DAE_CONVEX_HULL_OF_ATTRIBUTE);

	if(isConvex && !convexHullOf.empty())
	{
		return status;
	}

	// Read in the data sources
	xmlNodeList sourceDataNodes;
	FindChildrenByType(meshNode, DAE_SOURCE_ELEMENT, sourceDataNodes);
	for (xmlNodeList::iterator it = sourceDataNodes.begin(); it != sourceDataNodes.end(); ++it)
	{
		FCDGeometrySource* source = AddSource();
		status.AppendStatus(source->LoadFromXML(*it));
	}

	// Retrieve the <vertices> node
	xmlNode* verticesNode = FindChildByType(meshNode, DAE_VERTICES_ELEMENT);
	if (verticesNode == NULL)
	{
		return status.Warning(FS("No <vertices> element in mesh: ") + TO_FSTRING(parent->GetDaeId()), meshNode->line);
	}

	// Read in the per-vertex inputs
	bool hasPositions = false;

	xmlNodeList vertexInputNodes;
	FindChildrenByType(verticesNode, DAE_INPUT_ELEMENT, vertexInputNodes);
	for (xmlNodeList::iterator it = vertexInputNodes.begin(); it < vertexInputNodes.end(); ++it)
	{
		xmlNode* vertexInputNode = *it;
		string inputSemantic = ReadNodeSemantic(vertexInputNode);
		FUDaeGeometryInput::Semantic semantic = FUDaeGeometryInput::FromString(inputSemantic);
		if (semantic != FUDaeGeometryInput::VERTEX)
		{
			string sourceId = ReadNodeSource(vertexInputNode);
			FCDGeometrySource* source = FindSourceById(sourceId);
			if (source == NULL)
			{
				return status.Fail(FS("Mesh has source with an unknown id: ") + TO_FSTRING(parent->GetDaeId()), vertexInputNode->line);
			}
			source->SetSourceType(semantic);
			if (semantic == FUDaeGeometryInput::POSITION) hasPositions = true;
			vertexSources.push_back(source);
		}
	}
	if (!hasPositions)
	{
		status.Warning(FS("No vertex position input node in mesh: ") + TO_FSTRING(parent->GetDaeId()), verticesNode->line);
	}
	if (vertexSources.empty())
	{
		status.Warning(FS("Empty <vertices> element in geometry: ") + TO_FSTRING(parent->GetDaeId()), verticesNode->line);
	}

	// Create our rendering object and read in the tessellation
	bool primitivesFound = false;
	xmlNodeList polygonsNodes;
	FindChildrenByType(meshNode, DAE_POLYGONS_ELEMENT, polygonsNodes);
	FindChildrenByType(meshNode, DAE_TRIANGLES_ELEMENT, polygonsNodes);
	FindChildrenByType(meshNode, DAE_POLYLIST_ELEMENT, polygonsNodes);
	if (!polygonsNodes.empty())
	{
		primitivesFound = true;
		for (xmlNodeList::iterator it = polygonsNodes.begin(); it != polygonsNodes.end(); ++it)
		{
			// Create the geometry polygons object
			xmlNode* polygonsNode = *it;
			FCDGeometryPolygons* polygon = new FCDGeometryPolygons(GetDocument(), this);
			polygons.push_back(polygon);
			status = polygon->LoadFromXML(polygonsNode);
			if (!status) return status;
		}
	}

	polygonsNodes.clear();
	FindChildrenByType(meshNode, DAE_TRIFANS_ELEMENT, polygonsNodes);
	if (!polygonsNodes.empty())
	{
		primitivesFound = true;
		for (xmlNodeList::iterator it = polygonsNodes.begin(); it != polygonsNodes.end(); ++it)
		{
			// Create the geometry polygons object
			xmlNode* polygonsNode = *it;
			FCDGeometryPolygonsSpecial* trifan = new FCDGeometryPolygonsSpecial(GetDocument(), this, FCDGeometryPolygons::TRIANGLE_FANS);
			polygons.push_back(trifan);
			status = trifan->LoadFromXML(polygonsNode);
			if (!status) return status;
		}
	}
	polygonsNodes.clear();
	FindChildrenByType(meshNode, DAE_TRISTRIPS_ELEMENT, polygonsNodes);
	if (!polygonsNodes.empty())
	{
		primitivesFound = true;
		for (xmlNodeList::iterator it = polygonsNodes.begin(); it != polygonsNodes.end(); ++it)
		{
			// Create the geometry polygons object
			xmlNode* polygonsNode = *it;
			FCDGeometryPolygonsSpecial* tristrip = new FCDGeometryPolygonsSpecial(GetDocument(), this, FCDGeometryPolygons::TRIANGLE_STRIPS);
			polygons.push_back(tristrip);
			status = tristrip->LoadFromXML(polygonsNode);
			if (!status) return status;
		}
	}
	polygonsNodes.clear();
	FindChildrenByType(meshNode, DAE_LINES_ELEMENT, polygonsNodes);
	if (!polygonsNodes.empty())
	{
		primitivesFound = true;
		for (xmlNodeList::iterator it = polygonsNodes.begin(); it != polygonsNodes.end(); ++it)
		{
			// Create the geometry polygons object
			xmlNode* polygonsNode = *it;
			FCDGeometryPolygonsSpecial* lines = new FCDGeometryPolygonsSpecial(GetDocument(), this, FCDGeometryPolygons::LINES);
			polygons.push_back(lines);
			status = lines->LoadFromXML(polygonsNode);
			if (!status) return status;
		}
	}
	polygonsNodes.clear();
	FindChildrenByType(meshNode, DAE_LINESTRIPS_ELEMENT, polygonsNodes);
	if (!polygonsNodes.empty())
	{
		primitivesFound = true;
		for (xmlNodeList::iterator it = polygonsNodes.begin(); it != polygonsNodes.end(); ++it)
		{
			// Create the geometry polygons object
			xmlNode* polygonsNode = *it;
			FCDGeometryPolygonsSpecial* linestrips = new FCDGeometryPolygonsSpecial(GetDocument(), this, FCDGeometryPolygons::LINE_STRIPS);
			polygons.push_back(linestrips);
			status = linestrips->LoadFromXML(polygonsNode);
			if (!status) return status;
		}
	}


	if(!primitivesFound)
		return status.Warning(FS("No tessellation found for mesh: ") + TO_FSTRING(parent->GetDaeId()), meshNode->line);

	// Calculate the important statistics/offsets/counts
	Recalculate();

	// Apply the length factor on the vertex positions
	float lengthFactor = GetDocument()->GetLengthUnitConversion();
	FCDGeometrySource* positionSource = FindSourceByType(FUDaeGeometryInput::POSITION);
	if (positionSource == NULL)
	{
		return status.Fail(FS("Cannot process the vertex POSITION source for mesh: ") + TO_FSTRING(parent->GetDaeId()), meshNode->line);
	}
	FloatList& positionData = positionSource->GetSourceData();
	for (FloatList::iterator it = positionData.begin(); it != positionData.end(); ++it) (*it) *= lengthFactor;

	SetDirtyFlag();
	return status;
}

// Write out the <mesh> node to the COLLADA XML tree
xmlNode* FCDGeometryMesh::WriteToXML(xmlNode* parentNode) const
{
	xmlNode* meshNode = NULL;

	if(isConvex && !convexHullOf.empty())
	{
		meshNode = AddChild(parentNode, DAE_CONVEX_MESH_ELEMENT);
		FUSStringBuilder convexHullOfName(convexHullOf);
		AddAttribute(meshNode, DAE_CONVEX_HULL_OF_ATTRIBUTE, convexHullOfName);
	}
	else
	{
		meshNode = AddChild(parentNode, DAE_MESH_ELEMENT);

		// Write out the sources
		for (FCDGeometrySourceContainer::const_iterator itS = sources.begin(); itS != sources.end(); ++itS)
		{
			(*itS)->WriteToXML(meshNode);
		}

		// Write out the <vertices> element
		xmlNode* verticesNode = AddChild(meshNode, DAE_VERTICES_ELEMENT);
		for (FCDGeometrySourceTrackList::const_iterator itS = vertexSources.begin(); itS != vertexSources.end(); ++itS)
		{
			const char* semantic = FUDaeGeometryInput::ToString((*itS)->GetSourceType());
			AddInput(verticesNode, (*itS)->GetSourceId(), semantic);
		}
		FUSStringBuilder verticesNodeId(GetDaeId()); verticesNodeId.append("-vertices");
		AddAttribute(verticesNode, DAE_ID_ATTRIBUTE, verticesNodeId);

		// Write out the polygons
		for (FCDGeometryPolygonsContainer::const_iterator itP = polygons.begin(); itP != polygons.end(); ++itP)
		{
			(*itP)->WriteToXML(meshNode);
		}
	}
	return meshNode;
}
