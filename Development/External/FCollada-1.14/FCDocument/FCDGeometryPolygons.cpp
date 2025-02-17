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
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometryPolygons.h"
#include "FCDocument/FCDGeometrySource.h"
#include "FUtils/FUDaeParser.h"
#include "FUtils/FUDaeWriter.h"
#include "FUtils/FUStringConversion.h"
using namespace FUDaeParser;
using namespace FUDaeWriter;

//
// FCDGeometryPolygons
//

ImplementObjectType(FCDGeometryPolygons);

FCDGeometryPolygons::FCDGeometryPolygons(FCDocument* document, FCDGeometryMesh* _parent)
:	FCDObject(document)
,	parent(_parent)
,	faceVertexCount(0), faceVertexOffset(0)
{
}

FCDGeometryPolygons::~FCDGeometryPolygons()
{
	holeFaces.clear();
	parent = NULL;
}


// Creates a new face.
void FCDGeometryPolygons::AddFace(uint32 degree)
{
	faceVertexCounts.push_back(degree);

	// Inserts empty indices
	for (FCDGeometryPolygonsInputTrackList::iterator it = idxOwners.begin(); it != idxOwners.end(); ++it)
	{
		FCDGeometryPolygonsInput* input = (*it);
		input->indices.resize(input->indices.size() + degree, 0);
	}

	parent->Recalculate();
	SetDirtyFlag();
}

// Removes a face
void FCDGeometryPolygons::RemoveFace(size_t index)
{
	FUAssert(index < GetFaceCount(), return);

	// Remove the associated indices, if they exist.
	size_t offset = GetFaceVertexOffset(index);
	size_t indexCount = GetFaceVertexCount(index);
	for (FCDGeometryPolygonsInputTrackList::iterator it = idxOwners.begin(); it != idxOwners.end(); ++it)
	{
		FCDGeometryPolygonsInput* input = (*it);
		size_t inputIndexCount = input->indices.size();
		if (offset < inputIndexCount)
		{
			UInt32List::iterator end, begin = input->indices.begin() + offset;
			if (offset + indexCount < inputIndexCount) end = begin + indexCount;
			else end = input->indices.end();
			input->indices.erase(begin, end);
		}
	}

	// Remove the face and its holes
	size_t holeBefore = GetHoleCountBefore(index);
	UInt32List::iterator itFV = faceVertexCounts.begin() + index + holeBefore;
	size_t holeCount = GetHoleCount(index);
	faceVertexCounts.erase(itFV, itFV + holeCount + 1); // +1 in order to remove the polygon as well as the holes.

	parent->Recalculate();
	SetDirtyFlag();
}

// Calculates the offset of face-vertex pairs before the given face index within the polygon set.
size_t FCDGeometryPolygons::GetFaceVertexOffset(size_t index) const
{
	size_t offset = 0;

	// We'll need to skip over the holes
	size_t holeCount = GetHoleCountBefore(index);
	if (index + holeCount < faceVertexCounts.size())
	{
		// Sum up the wanted offset
		UInt32List::const_iterator end = faceVertexCounts.begin() + index + holeCount;
		for (UInt32List::const_iterator it = faceVertexCounts.begin(); it != end; ++it)
		{
			offset += (*it);
		}
	}
	return offset;
}

// Calculates the number of holes within the polygon set that appear before the given face index.
size_t FCDGeometryPolygons::GetHoleCountBefore(size_t index) const
{
	size_t holeCount = 0;
	for (UInt32List::const_iterator it = holeFaces.begin(); it != holeFaces.end(); ++it)
	{
		if ((*it) <= index) { ++holeCount; ++index; }
	}
	return holeCount;
}

// Retrieves the number of holes within a given face.
size_t FCDGeometryPolygons::GetHoleCount(size_t index) const
{
	size_t holeCount = 0;
	for (size_t i = index + GetHoleCountBefore(index) + 1; i < faceVertexCounts.size(); ++i)
	{
		bool isHoled = holeFaces.find(i) != holeFaces.end();
		if (!isHoled) break;
		else ++holeCount;
	}
	return holeCount;
}

// The number of face-vertex pairs for a given face.
size_t FCDGeometryPolygons::GetFaceVertexCount(size_t index) const
{
	size_t count = 0;
	if (index < GetFaceCount())
	{
		size_t holeCount = GetHoleCount(index);
		UInt32List::const_iterator it = faceVertexCounts.begin() + index + GetHoleCountBefore(index);
		UInt32List::const_iterator end = it + holeCount + 1; // +1 in order to sum the face-vertex pairs of the polygon as its holes.
		for (; it != end; ++it) count += (*it);
	}
	return count;
}

FCDGeometryPolygonsInput* FCDGeometryPolygons::AddInput(FCDGeometrySource* source, uint32 offset)
{
	// Check if the offset is new
	bool ownsIdx = true;
	for (FCDGeometryPolygonsInputContainer::iterator it = inputs.begin(); it != inputs.end(); ++it)
	{
		if ((*it)->idx == offset)
		{
			ownsIdx = false;
			break;
		}
	}

	FCDGeometryPolygonsInput* input = inputs.Add(this);
	input->SetSource(source);
	input->idx = offset;
	input->ownsIdx = ownsIdx;
	if (ownsIdx) idxOwners.push_back(input);
	SetDirtyFlag();
	return input;
}

void FCDGeometryPolygons::ReleaseInput(FCDGeometryPolygonsInput* input)
{
	FCDGeometryPolygonsInputContainer::iterator itP = inputs.find(input);
	if (itP != inputs.end())
	{
		// Before releasing the polygon set input, verify that shared indices are not lost
		if (input->ownsIdx)
		{
			for (FCDGeometryPolygonsInputContainer::iterator it = inputs.begin(); it != inputs.end(); ++it)
			{
				if ((*it)->idx == input->idx && !(*it)->ownsIdx)
				{
					(*it)->indices = input->indices;
					(*it)->ownsIdx = true;
					idxOwners.push_back(*it);
					break;
				}
			}

			FCDGeometryPolygonsInputContainer::iterator itO = idxOwners.find(input);
			if (itO != idxOwners.end()) idxOwners.erase(itO);
			input->indices.clear();
			input->ownsIdx = false;
		}

		// Release the polygon set input
		inputs.erase(itP);
		SetDirtyFlag();
	}
}

FCDGeometryPolygonsInput* FCDGeometryPolygons::FindInput(FUDaeGeometryInput::Semantic semantic)
{ return const_cast<FCDGeometryPolygonsInput*>(const_cast<const FCDGeometryPolygons*>(this)->FindInput(semantic)); }
const FCDGeometryPolygonsInput* FCDGeometryPolygons::FindInput(FUDaeGeometryInput::Semantic semantic) const
{
	for (FCDGeometryPolygonsInputContainer::const_iterator it = inputs.begin(); it != inputs.end(); ++it)
	{
		if ((*it)->GetSemantic() == semantic) return (*it);
	}
	return NULL;
}

FCDGeometryPolygonsInput* FCDGeometryPolygons::FindInput(const FCDGeometrySource* source)
{ return const_cast<FCDGeometryPolygonsInput*>(const_cast<const FCDGeometryPolygons*>(this)->FindInput(source)); }
const FCDGeometryPolygonsInput* FCDGeometryPolygons::FindInput(const FCDGeometrySource* source) const
{
	for (FCDGeometryPolygonsInputContainer::const_iterator it = inputs.begin(); it != inputs.end(); ++it)
	{
		if ((*it)->GetSource() == source) return (*it);
	}
	return NULL;
}

FCDGeometryPolygonsInput* FCDGeometryPolygons::FindInput(const string& sourceId)
{
	const char* s = sourceId.c_str();
	if (*s == '#') ++s;
	for (FCDGeometryPolygonsInputContainer::iterator it = inputs.begin(); it != inputs.end(); ++it)
	{
		if ((*it)->GetSource()->GetDaeId() == s) return (*it);
	}
	return NULL;
}

void FCDGeometryPolygons::FindInputs(FUDaeGeometryInput::Semantic semantic, FCDGeometryPolygonsInputConstList& _inputs) const
{
	for (FCDGeometryPolygonsInputContainer::const_iterator it = inputs.begin(); it != inputs.end(); ++it)
	{
		if ((*it)->GetSemantic() == semantic) _inputs.push_back(*it);
	}
}

UInt32List* FCDGeometryPolygons::FindIndicesForIdx(uint32 idx)
{ return const_cast<UInt32List*>(const_cast<const FCDGeometryPolygons*>(this)->FindIndicesForIdx(idx)); }
const UInt32List* FCDGeometryPolygons::FindIndicesForIdx(uint32 idx) const
{
	for (FCDGeometryPolygonsInputContainer::const_iterator cit = idxOwners.begin(); cit != idxOwners.end(); ++cit)
	{
		if ((*cit)->idx == idx) return &(*cit)->indices;
	}
	return NULL;
}

UInt32List* FCDGeometryPolygons::FindIndices(FCDGeometrySource* source)
{ return const_cast<UInt32List*>(const_cast<const FCDGeometryPolygons*>(this)->FindIndices(source)); }
const UInt32List* FCDGeometryPolygons::FindIndices(const FCDGeometrySource* source) const
{
	const FCDGeometryPolygonsInput* input = FindInput(source);
	return (input != NULL) ? FindIndices(input) : NULL;
}

UInt32List* FCDGeometryPolygons::FindIndices(FCDGeometryPolygonsInput* input)
{ return const_cast<UInt32List*>(const_cast<const FCDGeometryPolygons*>(this)->FindIndices(input)); }
const UInt32List* FCDGeometryPolygons::FindIndices(const FCDGeometryPolygonsInput* input) const
{
	FCDGeometryPolygonsInputContainer::const_iterator itP = inputs.find(input);
	return itP != inputs.end() ? FindIndicesForIdx(input->idx) : NULL;
}

// Recalculates the face-vertex count within the polygons
void FCDGeometryPolygons::Recalculate()
{
	faceVertexCount = 0;
	for (UInt32List::iterator itC = faceVertexCounts.begin(); itC != faceVertexCounts.end(); ++itC)
	{
		faceVertexCount += (*itC);
	}
	SetDirtyFlag();
}

FUStatus FCDGeometryPolygons::LoadFromXML(xmlNode* baseNode)
{
	FUStatus status;

	// Retrieve the expected face count from the base node's 'count' attribute
	size_t expectedFaceCount = ReadNodeCount(baseNode);

	// Check the node's name to know whether to expect a <vcount> element
	size_t expectedVertexCount; bool isPolygons = false, isTriangles = false, isPolylist = false;
	if (IsEquivalent(baseNode->name, DAE_POLYGONS_ELEMENT)) { expectedVertexCount = 4; isPolygons = true; }
	else if (IsEquivalent(baseNode->name, DAE_TRIANGLES_ELEMENT)) { expectedVertexCount = 3 * expectedFaceCount; isTriangles = true; }
	else if (IsEquivalent(baseNode->name, DAE_POLYLIST_ELEMENT)) { expectedVertexCount = 0; isPolylist = true; }
	else
	{
		return status.Fail(FS("Unknown polygons element in geometry: ") + TO_FSTRING(parent->GetDaeId()), baseNode->line);
	}

	// Retrieve the material symbol used by these polygons
	materialSemantic = TO_FSTRING(ReadNodeProperty(baseNode, DAE_MATERIAL_ATTRIBUTE));
	if (materialSemantic.empty())
	{
		status.Warning(FS("Unknown or missing polygonal material symbol in geometry: ") + TO_FSTRING(parent->GetDaeId()), baseNode->line);
	}

	// Read in the per-face, per-vertex inputs
	uint32 idxCount = 1;
	xmlNode* itNode = NULL;
	bool hasVertexInput = false;
	for (itNode = baseNode->children; itNode != NULL; itNode = itNode->next)
	{
		if (itNode->type != XML_ELEMENT_NODE) continue;
		if (IsEquivalent(itNode->name, DAE_INPUT_ELEMENT))
		{
			string sourceId = ReadNodeSource(itNode);
			if (sourceId[0] == '#') sourceId.erase(0, 1);

			// Parse input idx/offset
			string idx = ReadNodeProperty(itNode, DAE_OFFSET_ATTRIBUTE);
			if (idx.empty()) idx = ReadNodeProperty(itNode, DAE_IDX_ATTRIBUTE); // COLLADA 1.3 Backward-compatibility
			uint32 offset = (!idx.empty()) ? FUStringConversion::ToUInt32(idx) : idxCount;
			idxCount = max(offset + 1, idxCount);

			// Parse input set
			string setString = ReadNodeProperty(itNode, DAE_SET_ATTRIBUTE);
			uint32 set = setString.empty() ? -1 : FUStringConversion::ToInt32(setString);

			// Parse input semantic
			FUDaeGeometryInput::Semantic semantic = FUDaeGeometryInput::FromString(ReadNodeSemantic(itNode));
			if (semantic == FUDaeGeometryInput::UNKNOWN) continue; // Unknown input type
			else if (semantic == FUDaeGeometryInput::VERTEX)
			{
				// There should never be more than one 'VERTEX' input.
				if (hasVertexInput)
				{
					status.Warning(FS("There should never be more than one 'VERTEX' input in a mesh: skipping extra 'VERTEX' inputs."), itNode->line);
					continue;
				}
				hasVertexInput = true;

				// Add an input for all the vertex sources in the parent.
				FCDGeometrySourceList& vertexSources = parent->GetVertexSources();
				size_t vertexSourceCount = vertexSources.size();
				for (uint32 i = 0; i < vertexSourceCount; ++i)
				{
					FCDGeometryPolygonsInput* vertexInput = AddInput(vertexSources[i], offset);
					vertexInput->set = set;
				}
			}
			else
			{
				// Retrieve the source for this input
				FCDGeometrySource* source = parent->FindSourceById(sourceId);
				if (source != NULL)
				{
					// The source may have a dangling type: the input sets contains that information in the COLLADA document.
					// So: enforce the source type of the input to the data source.
					source->SetSourceType(semantic); 
					FCDGeometryPolygonsInput* input = AddInput(source, offset);
					input->set = set;
				}
				else
				{
					status.Warning(FS("Unknown polygons set input with id: '") + TO_FSTRING(sourceId) + FS("' in geometry: ") + TO_FSTRING(parent->GetDaeId()), itNode->line);
				}
			}
		}
		else if (IsEquivalent(itNode->name, DAE_POLYGON_ELEMENT)
			|| IsEquivalent(itNode->name, DAE_VERTEXCOUNT_ELEMENT)
			|| IsEquivalent(itNode->name, DAE_POLYGONHOLED_ELEMENT))
		{
			break;
		}

		// COLLADA 1.3 backward compatibility: <param> is a valid node, but unused
		else if (IsEquivalent(itNode->name, DAE_PARAMETER_ELEMENT)) {} 
		else
		{
			status.Warning(FS("Unknown polygon child element in geometry: ") + TO_FSTRING(parent->GetDaeId()), itNode->line);
		}
	}
	if (itNode == NULL)
	{
		return status.Fail(FS("No polygon <p>/<vcount> element found in geometry: ") + TO_FSTRING(parent->GetDaeId()), baseNode->line);
	}
	if (!hasVertexInput)
	{
		// Verify that we did find a VERTEX polygon set input.
		return status.Fail(FS("Cannot find 'VERTEX' polygons' input within geometry: ") + TO_FSTRING(parent->GetDaeId()), baseNode->line);
	}

	// Look for the <vcount> element and parse it in
	xmlNode* vCountNode = FindChildByType(baseNode, DAE_VERTEXCOUNT_ELEMENT);
	const char* vCountDataString = ReadNodeContentDirect(vCountNode);
	if (vCountDataString != NULL) FUStringConversion::ToUInt32List(vCountDataString, faceVertexCounts);
	bool hasVertexCounts = !faceVertexCounts.empty();
	if (isPolylist && !hasVertexCounts)
	{
		return status.Fail(FS("No or empty <vcount> element found in geometry: ") + TO_FSTRING(parent->GetDaeId()), baseNode->line);
	}
	else if (!isPolylist && hasVertexCounts)
	{
		return status.Fail(FS("<vcount> is only expected with the <polylist> element in geometry: ") + TO_FSTRING(parent->GetDaeId()), baseNode->line);
	}
	else if (isPolylist)
	{
		// Count the total number of face-vertices expected, to pre-buffer the index lists
		expectedVertexCount = 0;
		for (UInt32List::iterator itC = faceVertexCounts.begin(); itC != faceVertexCounts.end(); ++itC)
		{
			expectedVertexCount += *itC;
		}
	}

	// Pre-allocate the buffers with enough memory
	string holeBuffer;
	UInt32List allIndices;
	faceVertexCount = 0;
	allIndices.clear();
	allIndices.reserve(expectedVertexCount * idxCount);
	for (FCDGeometryPolygonsInputContainer::iterator it = idxOwners.begin(); it != idxOwners.end(); ++it)
	{
		(*it)->indices.reserve(expectedVertexCount);
	}

	// Process the tessellation
	for (; itNode != NULL; itNode = itNode->next)
	{
		uint32 localFaceVertexCount;
		const char* content = NULL;
		xmlNode* holeNode = NULL;
		bool failed = false;
		if (!InitTessellation(itNode, &localFaceVertexCount, allIndices, content, holeNode, idxCount, &failed)) continue;

		if (failed)
		{
			return status.Fail(FS("Unknown element found in <ph> element for geometry: ") + TO_FSTRING(parent->GetDaeId()), itNode->line);
		}

		if (isTriangles) for (uint32 i = 0; i < localFaceVertexCount / 3; ++i) faceVertexCounts.push_back(3);
		else if (isPolygons) faceVertexCounts.push_back(localFaceVertexCount);
		faceVertexCount += localFaceVertexCount;

		// Append any hole indices found
		for (; holeNode != NULL; holeNode = holeNode->next)
		{
			if (holeNode->type != XML_ELEMENT_NODE) continue;

			// Read in the hole indices and push them on top of the other indices
			UInt32List holeIndices; holeIndices.reserve(expectedVertexCount * idxCount);
			content = ReadNodeContentDirect(holeNode);
			FUStringConversion::ToUInt32List(content, holeIndices);
			allIndices.insert(allIndices.end(), holeIndices.begin(), holeIndices.end());

			// Create the hole face and record its index
			size_t holeVertexCount = holeIndices.size() / idxCount;
			holeFaces.push_back((uint32) faceVertexCounts.size());
			faceVertexCounts.push_back((uint32) holeVertexCount);
			faceVertexCount += holeVertexCount;
		}

		// Create a new entry for the vertex buffer
		for (size_t offset = 0; offset < allIndices.size(); offset += idxCount)
		{
			for (FCDGeometryPolygonsInputContainer::iterator it = idxOwners.begin(); it != idxOwners.end(); ++it)
			{
				(*it)->indices.push_back(allIndices[offset + (*it)->idx]);
			}
		}
	}

	// Check the actual face count
	if (expectedFaceCount != faceVertexCounts.size() - holeFaces.size())
	{
		return status.Fail(FS("Face count for polygons node doesn't match actual number of faces found in <p> element(s) in geometry: ") + TO_FSTRING(parent->GetDaeId()), baseNode->line);
	}

	SetDirtyFlag();
	return status;
}

bool FCDGeometryPolygons::InitTessellation(xmlNode* itNode, 
		uint32* localFaceVertexCount, UInt32List& allIndices, 
		const char* content, xmlNode*& holeNode, uint32 idxCount, 
		bool* failed)
{
	if (itNode->type != XML_ELEMENT_NODE) return false;
	if (!IsEquivalent(itNode->name, DAE_POLYGON_ELEMENT) 
		&& !IsEquivalent(itNode->name, DAE_POLYGONHOLED_ELEMENT)) return false;

	// Retrieve the indices
	content = NULL;
	holeNode = NULL;
	if (!IsEquivalent(itNode->name, DAE_POLYGONHOLED_ELEMENT)) 
	{
		content = ReadNodeContentDirect(itNode);
	} 
	else 
	{
		// Holed face found
		for (xmlNode* child = itNode->children; child != NULL; child = child->next)
		{
			if (child->type != XML_ELEMENT_NODE) continue;
			if (IsEquivalent(child->name, DAE_POLYGON_ELEMENT)) 
			{
				content = ReadNodeContentDirect(child);
			}
			else if (IsEquivalent(child->name, DAE_HOLE_ELEMENT)) 
			{ 
				holeNode = child; break; 
			}
			else 
			{
				*failed = true;
				return true;
			}
		}
	}

	// Parse the indices
	allIndices.clear();
	FUStringConversion::ToUInt32List(content, allIndices);
	*localFaceVertexCount = (uint32) allIndices.size() / idxCount;
	SetDirtyFlag();
	return true;
}

bool FCDGeometryPolygons::IsTriangles() const
{
	UInt32List::const_iterator itC;
	for (itC = faceVertexCounts.begin(); itC != faceVertexCounts.end() && (*itC) == 3; ++itC) {}
	return (itC == faceVertexCounts.end());
}

// Write out the polygons structure to the COLLADA XML tree
xmlNode* FCDGeometryPolygons::WriteToXML(xmlNode* parentNode) const
{
	// Are there holes? Then, export a <polygons> element.
	// Are there only non-triangles within the list? Then, export a <polylist> element.
	// Otherwise, you only have triangles: export a <triangles> element.
	bool hasHoles = !holeFaces.empty(), hasNPolys = true;
	if( !hasHoles ) hasNPolys = !IsTriangles();

	// Create the base node for these polygons
	const char* polygonNodeType;
	if (hasHoles) polygonNodeType = DAE_POLYGONS_ELEMENT;
	else if (hasNPolys) polygonNodeType = DAE_POLYLIST_ELEMENT;
	else polygonNodeType = DAE_TRIANGLES_ELEMENT;
	xmlNode* polygonsNode = AddChild(parentNode, polygonNodeType);

	// Add the inputs
	// Find which input owner belongs to the <vertices> element. Replace the semantic and the source id accordingly.
	// Make sure to add that 'vertex' input only once.
	FUSStringBuilder verticesNodeId(parent->GetDaeId()); verticesNodeId.append("-vertices");
	const FCDGeometrySourceList& vertexSources = parent->GetVertexSources();
	bool isVertexInputFound = false;
	for (FCDGeometryPolygonsInputContainer::const_iterator itI = inputs.begin(); itI != inputs.end(); ++itI)
	{
		FCDGeometryPolygonsInput* input = *itI;
		FCDGeometrySource* source = input->GetSource();
		if (source != NULL)
		{
			if (vertexSources.find(source) == vertexSources.end())
			{
				const char* semantic = FUDaeGeometryInput::ToString(input->GetSemantic());
				FUDaeWriter::AddInput(polygonsNode, source->GetDaeId(), semantic, input->idx, input->set);
			}
			else if (!isVertexInputFound)
			{
				FUDaeWriter::AddInput(polygonsNode, verticesNodeId.ToCharPtr(), DAE_VERTEX_INPUT, input->idx);
				isVertexInputFound = true;
			}
		}
	}

	FUSStringBuilder builder;
	builder.reserve(1024);

	// For the poly-list case, export the list of vertex counts
	if (!hasHoles && hasNPolys)
	{
		FUStringConversion::ToString(builder, faceVertexCounts);
		xmlNode* vcountNode = AddChild(polygonsNode, DAE_VERTEXCOUNT_ELEMENT);
		AddContentUnprocessed(vcountNode, builder.ToCharPtr());
		builder.clear();
	}

	// For the non-holes cases, open only one <p> element for all the data indices
	xmlNode* pNode = NULL,* phNode = NULL;
	if (!hasHoles) pNode = AddChild(polygonsNode, DAE_POLYGON_ELEMENT);

	// Export the data indices (tessellation information)
	size_t faceCount = GetFaceCount();
	uint32 faceVertexOffset = 0;
	size_t holeOffset = 0;
	for (size_t faceIndex = 0; faceIndex < faceCount; ++faceIndex)
	{
		// For the holes cases, verify whether this face or the next one(s) are holes. We may need to open a new <ph>/<p> element
		size_t holeCount = 0;
		if (hasHoles)
		{
			holeCount = GetHoleCount(faceIndex);

			if (holeCount == 0)
			{
				// Just open a <p> element: this is the most common case
				pNode = AddChild(polygonsNode, DAE_POLYGON_ELEMENT);
			}
			else
			{
				// Open up a new <ph> element and its <p> element
				phNode = AddChild(polygonsNode, DAE_POLYGONHOLED_ELEMENT);
				pNode = AddChild(phNode, DAE_POLYGON_ELEMENT);
			}
		}

		for (size_t holeIndex = 0; holeIndex < holeCount + 1; ++holeIndex)
		{
			// Write out the tessellation information for all the vertices of this face
			uint32 faceVertexCount = faceVertexCounts[faceIndex + holeOffset + holeIndex];
			for (uint32 faceVertexIndex = faceVertexOffset; faceVertexIndex < faceVertexOffset + faceVertexCount; ++faceVertexIndex)
			{
				for (FCDGeometryPolygonsInputTrackList::const_iterator itI = idxOwners.begin(); itI != idxOwners.end(); ++itI)
				{
					UInt32List& indices = (*itI)->indices;
					builder.append(indices[faceVertexIndex]);
					builder.append(' ');
				}
			}

			// For the holes cases: write out the indices for every polygon element
			if (hasHoles)
			{
				if (!builder.empty()) builder.pop_back(); // take out the last space
				AddContentUnprocessed(pNode, builder.ToCharPtr());
				builder.clear();

				if (holeIndex < holeCount)
				{
					// Open up a <h> element
					pNode = AddChild(phNode, DAE_HOLE_ELEMENT);
				}
			}

			faceVertexOffset += faceVertexCount;
		}
		holeOffset += holeCount;
	}

	// For the non-holes cases: write out the indices at the very end, for the single <p> element
	if (!hasHoles)
	{
		if (!builder.empty()) builder.pop_back(); // take out the last space
		AddContentUnprocessed(pNode, builder.ToCharPtr());
	}

	// Write out the material semantic and the number of polygons
	if (!materialSemantic.empty())
	{
		AddAttribute(polygonsNode, DAE_MATERIAL_ATTRIBUTE, materialSemantic);
	}
	AddAttribute(polygonsNode, DAE_COUNT_ATTRIBUTE, GetFaceCount());

	return polygonsNode;
}

// Clone this list of polygons
FCDGeometryPolygons* FCDGeometryPolygons::Clone(FCDGeometryMesh* cloneParent, const FCDGeometrySourceCloneMap& cloneMap)
{
	FCDGeometryPolygons* clone = new FCDGeometryPolygons(GetDocument(), cloneParent);
	clone->materialSemantic = materialSemantic;
	clone->faceVertexCounts = faceVertexCounts;
	clone->faceOffset = faceOffset;
	clone->faceVertexCount = faceVertexCount;
	clone->faceVertexOffset = faceVertexOffset;
	clone->holeOffset = holeOffset;
	clone->holeFaces = holeFaces;
	
	// Clone the geometry inputs
	uint32 inputCount = (uint32) inputs.size();
	clone->inputs.reserve(inputCount);
	for (uint32 i = 0; i < inputCount; ++i)
	{
		FCDGeometryPolygonsInput* input = clone->inputs.Add(clone);
		FCDGeometrySourceCloneMap::const_iterator it = cloneMap.find(inputs[i]->GetSource());
		if (it == cloneMap.end())
		{
			// Attempt to match by ID instead.
			const string& id = inputs[i]->GetSource()->GetDaeId();
			input->SetSource(cloneParent->FindSourceById(id));
		}
		else
		{
			input->SetSource((*it).second);
		}
		input->idx = inputs[i]->idx;
		input->indices = inputs[i]->indices;
		input->ownsIdx = inputs[i]->ownsIdx;
		input->set = inputs[i]->set;

		// Regenerate the idxOwners list with the new inputs
		if (input->ownsIdx) clone->idxOwners.push_back(input);
	}
	return clone;
}

//
// FCDGeometryPolygonsInput
//

ImplementObjectType(FCDGeometryPolygonsInput);

FCDGeometryPolygonsInput::FCDGeometryPolygonsInput(FCDGeometryPolygons* _parent)
:	parent(_parent), source(NULL)
,	idx(0),	set(-1), ownsIdx(false)
{
}

FCDGeometryPolygonsInput::~FCDGeometryPolygonsInput()
{
	if (source != NULL)
	{
		UntrackObject(source);
		source = NULL;
	}
}

FUDaeGeometryInput::Semantic FCDGeometryPolygonsInput::GetSemantic() const
{
	FUAssert(source != NULL, return FUDaeGeometryInput::UNKNOWN);
	return source->GetSourceType();
}

// Sets the referenced source
void FCDGeometryPolygonsInput::SetSource(FCDGeometrySource* _source)
{
	// Untrack the old source and track the new source
	if (source != NULL) UntrackObject(source);
	source = _source;
	if (source != NULL) TrackObject(source);
}

// Callback when the tracked source is released.
void FCDGeometryPolygonsInput::OnObjectReleased(FUObject* object)
{
	if (source == object)
	{
		source = NULL;
		parent->ReleaseInput(this); // This function deletes this object.
	}
}
