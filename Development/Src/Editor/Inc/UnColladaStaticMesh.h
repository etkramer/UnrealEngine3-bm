/*=============================================================================
	COLLADA static mesh helper.
	Based on Feeling Software's Collada import classes [FCollada].
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

	Class implementation inspired by the code of Richard Stenson, SCEA R&D
=============================================================================*/

#ifndef __UNCOLLADASTATICMESH_H__
#define __UNCOLLADASTATICMESH_H__

// Forward declarations
class FCDGeometry;
class FCDGeometryInstance;
class FCDGeometryMesh;

namespace UnCollada
{

// Forward declarations
class CImporter;
class CExporter;
typedef TArray<UINT> UINTArray;

/**
 * COLLADA static mesh importer.
 */
class CStaticMesh
{
public:
	CStaticMesh(CImporter* BaseImporter);
	CStaticMesh(CExporter* BaseExporter);
	~CStaticMesh();

	/*
	 * Imports the polygonal elements of a given COLLADA mesh into the given UE3 rendering mesh.
	 */
	void ImportStaticMeshPolygons(FStaticMeshRenderData* RenderMesh, const FCDGeometryInstance* ColladaInstance, const FCDGeometryMesh* ColladaMesh);

	/*
	 * Exports the given static rendering mesh into a COLLADA geometry.
	 */
	FCDGeometry* ExportStaticMesh(FStaticMeshRenderData& RenderMesh, const TCHAR* MeshName);

private:
	CImporter* BaseImporter;
	CExporter* BaseExporter;

	/*
	 * Creates the smoothing group masks from the normals of a COLLADA mesh.
	 */
	void CreateSmoothingGroups(const FCDGeometryMesh* ColladaMesh, UINTArray& PolygonSmoothingGroups);
};

} // namespace UnCollada

#endif // __UNCOLLADASTATICMESH_H__
