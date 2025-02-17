/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometryPolygons.h"
#include "FCDocument/FCDGeometryPolygonsTools.h"
#include "FCDocument/FCDGeometrySource.h"

namespace FCDGeometryPolygonsTools
{
	// Triangulates a mesh.
	void Triangulate(FCDGeometryMesh* mesh)
	{
		if (mesh == NULL) return;

		size_t polygonsCount = mesh->GetPolygonsCount();
		for (size_t i = 0; i < polygonsCount; ++i)
		{
			Triangulate(mesh->GetPolygons(i), false);
		}

		// Recalculate the mesh/polygons statistics
		mesh->Recalculate();
	}

	// Triangulates a polygons set.
	void Triangulate(FCDGeometryPolygons* polygons, bool recalculate)
	{
		if (polygons == NULL) return;

		// Pre-allocate and ready the end index/count buffers
		UInt32List oldFaceVertexCounts = polygons->GetFaceVertexCounts();
		UInt32List holeFaces = polygons->GetHoleFaces();
		polygons->GetFaceVertexCounts().clear();
		vector<UInt32List*> dataIndices;
		vector<UInt32List> oldDataIndices;
		for (FCDGeometryPolygonsInputContainer::iterator itI = polygons->GetInputs().begin(); itI != polygons->GetInputs().end(); ++itI)
		{
			if ((*itI)->indices.empty()) continue;
			UInt32List* indices = &(*itI)->indices;
			oldDataIndices.push_back(*indices);
			dataIndices.push_back(indices);
			indices->clear();
		}
		size_t dataIndicesCount = oldDataIndices.size();

		// Rebuild the index/count buffers through simple fan-triangulation.
		// Drop holes and polygons with less than three vertices. 
		size_t oldOffset = 0, oldFaceCount = oldFaceVertexCounts.size();
		for (size_t oldFaceIndex = 0; oldFaceIndex < oldFaceCount; ++oldFaceIndex)
		{
			size_t oldFaceVertexCount = oldFaceVertexCounts[oldFaceIndex];
			bool isHole = holeFaces.find((uint32) oldFaceIndex) != holeFaces.end();
			if (!isHole && oldFaceVertexCount >= 3)
			{
				// Fan-triangulation: works well on convex polygons.
				size_t triangleCount = oldFaceVertexCount - 2;
				for (size_t triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
				{
					for (size_t j = 0; j < dataIndicesCount; ++j)
					{
						UInt32List& oldData = oldDataIndices[j];
						UInt32List* newData = dataIndices[j];
						newData->push_back(oldData[oldOffset]);
						newData->push_back(oldData[oldOffset + triangleIndex + 1]);
						newData->push_back(oldData[oldOffset + triangleIndex + 2]);
					}
					polygons->GetFaceVertexCounts().push_back(3);
				}
			}
			oldOffset += oldFaceVertexCount;
		}

		holeFaces.clear();

		if (recalculate) polygons->Recalculate();
	}

	static uint32 CompressSortedVector(FMVector3& toInsert, FloatList& insertedList, UInt32List& compressIndexReferences)
	{
		// Look for this vector within the already inserted list.
		size_t start = 0, end = compressIndexReferences.size(), mid;
		for (mid = (start + end) / 2; start < end; mid = (start + end) / 2)
		{
			uint32 index = compressIndexReferences[mid];
			if (toInsert.x == insertedList[3 * index]) break;
			else if (toInsert.x < insertedList[3 * index]) end = mid;
			else start = mid + 1;
		}

		// Look for the tolerable range within the binary-sorted dimension.
		size_t rangeStart, rangeEnd;
		for (rangeStart = mid; rangeStart > 0; --rangeStart)
		{
			uint32 index = compressIndexReferences[rangeStart - 1];
			if (!IsEquivalent(insertedList[3 * index], toInsert.x)) break;
		}
		for (rangeEnd = min(mid + 1, compressIndexReferences.size()); rangeEnd < compressIndexReferences.size(); ++rangeEnd)
		{
			uint32 index = compressIndexReferences[rangeEnd];
			if (!IsEquivalent(insertedList[3 * index], toInsert.x)) break;
		}
		FUAssert(rangeStart >= 0 && (rangeStart < rangeEnd || (rangeStart == rangeEnd && rangeEnd == compressIndexReferences.size())), return 0);

		// Look for an equivalent vector within the tolerable range
		for (size_t g = rangeStart; g < rangeEnd; ++g)
		{
			uint32 index = compressIndexReferences[g];
			if (IsEquivalent(toInsert, *(const FMVector3*) &insertedList[3 * index])) return index;
		}

		// Insert this new vector in the list and add the index reference at the correct position.
		uint32 compressIndex = (uint32) (insertedList.size() / 3);
		compressIndexReferences.insert(compressIndexReferences.begin() + mid, compressIndex);
		insertedList.push_back(toInsert.x);
		insertedList.push_back(toInsert.y);
		insertedList.push_back(toInsert.z);
		return compressIndex;
	}

	// Generates the texture tangents and binormals for a given source of texture coordinates.
	void GenerateTextureTangentBasis(FCDGeometryMesh* mesh, FCDGeometrySource* texcoordSource, bool generateBinormals)
	{
		if (texcoordSource == NULL || mesh == NULL) return;

		FCDGeometrySource* tangentSource = NULL;
		FCDGeometrySource* binormalSource = NULL;
		FloatList* tangentData = NULL;
		FloatList* binormalData = NULL;
		UInt32List tangentCompressionIndices;
		UInt32List binormalCompressionIndices;

		// This operation is done on the tessellation.
		size_t polygonsCount = mesh->GetPolygonsCount();
		for (size_t i = 0; i < polygonsCount; ++i)
		{
			FCDGeometryPolygons* polygons = mesh->GetPolygons(i);

			// Verify that this polygons set uses the given texture coordinate source.
			FCDGeometryPolygonsInput* texcoordInput = polygons->FindInput(texcoordSource);
			if (texcoordInput == NULL) continue;

			// Retrieve the data and index buffer of positions/normals/texcoords for this polygons set.
			FCDGeometryPolygonsInput* positionInput = polygons->FindInput(FUDaeGeometryInput::POSITION);
			FCDGeometryPolygonsInput* normalsInput = polygons->FindInput(FUDaeGeometryInput::NORMAL);
			if (positionInput == NULL || normalsInput == NULL) continue;
			FCDGeometrySource* positionSource = positionInput->GetSource();
			FCDGeometrySource* normalsSource = normalsInput->GetSource();
			FCDGeometrySource* texcoordSource = texcoordInput->GetSource();
			if (positionSource == NULL || normalsSource == NULL || texcoordSource == NULL) continue;
			uint32 positionStride = positionSource->GetSourceStride();
			uint32 normalsStride = normalsSource->GetSourceStride();
			uint32 texcoordStride = texcoordSource->GetSourceStride();
			if (positionStride < 3 || normalsStride < 3 || texcoordStride < 2) continue;
			UInt32List* positionIndices = polygons->FindIndices(positionInput);
			UInt32List* normalsIndices = polygons->FindIndices(normalsInput);
			UInt32List* texcoordIndices = polygons->FindIndices(texcoordInput);
			if (positionIndices == NULL || normalsIndices == NULL || texcoordIndices == NULL) continue;
			if (positionIndices->empty() || normalsIndices->empty() || texcoordIndices->empty()) continue;
			FloatList& positionData = positionSource->GetSourceData();
			FloatList& normalsData = normalsSource->GetSourceData();
			FloatList& texcoordData = texcoordSource->GetSourceData();
			if (positionData.empty() || normalsData.empty() || texcoordData.empty()) continue;

			// Create the texture tangents/binormals sources
			if (tangentSource == NULL)
			{
				tangentSource = mesh->AddSource(FUDaeGeometryInput::TEXTANGENT);
				tangentSource->SetDaeId(texcoordSource->GetDaeId() + "-tangents");
				tangentSource->SetSourceStride(3);
				tangentSource->GetSourceData().reserve(texcoordSource->GetSourceData().size());
				tangentData = &tangentSource->GetSourceData();
				if (generateBinormals)
				{
					binormalSource = mesh->AddSource(FUDaeGeometryInput::TEXBINORMAL);
					binormalSource->SetDaeId(texcoordSource->GetDaeId() + "-binormals");
					binormalSource->SetSourceStride(3);
					binormalSource->GetSourceData().reserve(tangentSource->GetSourceData().size());
					binormalData = &binormalSource->GetSourceData();
				}
			}

			// Calculate the next available offset
			uint32 inputOffset = 0;
			for (FCDGeometryPolygonsInputContainer::iterator itI = polygons->GetInputs().begin(); itI != polygons->GetInputs().end(); ++itI)
			{
				inputOffset = max(inputOffset, (*itI)->idx);
			}

			// Create the polygons set input for both the tangents and binormals
			FCDGeometryPolygonsInput* tangentInput = polygons->AddInput(tangentSource, inputOffset + 1);
			tangentInput->set = texcoordInput->set;
			UInt32List* tangentIndices = polygons->FindIndices(tangentInput);
			UInt32List* binormalIndices = NULL;
			if (binormalSource != NULL)
			{
				FCDGeometryPolygonsInput* binormalInput = polygons->AddInput(binormalSource, inputOffset + 2);
				binormalInput->set = tangentInput->set;
				binormalIndices = polygons->FindIndices(binormalInput);
			}

			// Iterate of the faces of the polygons set. This includes holes.
			size_t faceCount = polygons->GetFaceVertexCounts().size();
			size_t faceVertexOffset = 0;
			for (size_t faceIndex = 0; faceIndex < faceCount; ++faceIndex)
			{
				size_t faceVertexCount = polygons->GetFaceVertexCounts()[faceIndex];
				for(size_t vertexIndex = 0; vertexIndex < faceVertexCount; ++vertexIndex)
				{
					// For each face-vertex pair, retrieve the current/previous/next vertex position/normal/texcoord.
					size_t previousVertexIndex = (vertexIndex > 0) ? vertexIndex - 1 : faceVertexCount - 1;
					size_t nextVertexIndex = (vertexIndex < faceVertexCount - 1) ? vertexIndex + 1 : 0;
					FMVector3& previousPosition = *(FMVector3*)&positionData[positionIndices->at(faceVertexOffset + previousVertexIndex) * positionStride];
					FMVector2& previousTexcoord = *(FMVector2*)&texcoordData[texcoordIndices->at(faceVertexOffset + previousVertexIndex) * texcoordStride];
					FMVector3& currentPosition = *(FMVector3*)&positionData[positionIndices->at(faceVertexOffset + vertexIndex) * positionStride];
					FMVector2& currentTexcoord = *(FMVector2*)&texcoordData[texcoordIndices->at(faceVertexOffset + vertexIndex) * texcoordStride];
					FMVector3& nextPosition = *(FMVector3*)&positionData[positionIndices->at(faceVertexOffset + nextVertexIndex) * positionStride];
					FMVector2& nextTexcoord = *(FMVector2*)&texcoordData[texcoordIndices->at(faceVertexOffset + nextVertexIndex) * texcoordStride];
					FMVector3& normal = *(FMVector3*)&normalsData[normalsIndices->at(faceVertexOffset + vertexIndex) * normalsStride];

					// The formulae to calculate the tangent-space basis vectors is taken from Maya 7.0 API documentation:
					// "Appendix A: Tangent and binormal vectors".

					// Prepare the edge vectors.
					FMVector3 previousEdge(0.0f, currentTexcoord.x - previousTexcoord.x, currentTexcoord.y - previousTexcoord.y);
					FMVector3 nextEdge(0.0f, nextTexcoord.x - currentTexcoord.x, nextTexcoord.y - currentTexcoord.y);
					FMVector3 tangent;

					// Calculate the X-coordinate of the tangent vector.
					previousEdge.x = currentPosition.x - previousPosition.x;
					nextEdge.x = nextPosition.x - currentPosition.x;
					FMVector3 crossEdge = previousEdge ^ nextEdge;
					if (IsEquivalent(crossEdge.x, 0.0f)) crossEdge.x = 1.0f; // degenerate
					tangent.x = -crossEdge.y / crossEdge.x;

					// Calculate the Y-coordinate of the tangent vector.
					previousEdge.x = currentPosition.y - previousPosition.y;
					nextEdge.x = nextPosition.y - currentPosition.y;
					crossEdge = previousEdge ^ nextEdge;
					if (IsEquivalent(crossEdge.x, 0.0f)) crossEdge.x = 1.0f; // degenerate
					tangent.y = -crossEdge.y / crossEdge.x;

					// Calculate the Z-coordinate of the tangent vector.
					previousEdge.x = currentPosition.z - previousPosition.z;
					nextEdge.x = nextPosition.z - currentPosition.z;
					crossEdge = previousEdge ^ nextEdge;
					if (IsEquivalent(crossEdge.x, 0.0f)) crossEdge.x = 1.0f; // degenerate
					tangent.z = -crossEdge.y / crossEdge.x;

					// Take the normal vector at this face-vertex pair, out of the calculated tangent vector
					tangent = tangent - normal * (tangent * normal);
					tangent.NormalizeIt();

					uint32 compressedIndex = CompressSortedVector(tangent, *tangentData, tangentCompressionIndices);
					tangentIndices->push_back(compressedIndex);

					if (binormalIndices != NULL)
					{
						// Calculate and store the binormal.
						FMVector3 binormal = (normal ^ tangent).Normalize();
						compressedIndex = CompressSortedVector(binormal, *binormalData, binormalCompressionIndices);
						binormalIndices->push_back(compressedIndex);
					}
				}
				faceVertexOffset += faceVertexCount;
			}
		}
	}
}