/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDGeometryPolygonsSpecial.h
	This file defines the FCDGeometryPolygonsTools namespace.
*/

#ifndef _FCD_GEOMETRY_POLYGONS_TOOLS_H_
#define _FCD_GEOMETRY_POLYGONS_TOOLS_H_

class FCDGeometryMesh;
class FCDGeometryPolygons;

/** Holds commonly-used transformation functions for meshes and polygons sets. */
namespace FCDGeometryPolygonsTools
{
	/** Triangulates a mesh.
		@param mesh The mesh to triangulate. */
	void Triangulate(FCDGeometryMesh* mesh);

	/** Triangulates a polygons set.
		@param polygons The polygons set to triangulate.
		@param recalculate Once the triangulation is done,
			should the statistics of the mesh be recalculated?*/
	void Triangulate(FCDGeometryPolygons* polygons, bool recalculate = true);

	/** Generates the texture tangents and binormals for a given source of texture coordinates.
		A source of normals and a source of vertex positions will be expected.
		@param mesh The mesh that contains the texture coordinate source.
		@param source The source of texture coordinates that needs its tangents and binormals generated.
		@param generateBinormals Whether the binormals should also be generated.
			Do note that the binormal is always the cross-product of the tangent and the normal at a vertex-face pair. */
	void GenerateTextureTangentBasis(FCDGeometryMesh* mesh, FCDGeometrySource* texcoordSource, bool generateBinormals = true);
}

#endif // _FCD_GEOMETRY_POLYGONS_TOOLS_H_