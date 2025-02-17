/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDGeometry.h
	This file contains the FCDGeometry class.
	The FCDGeometry class holds the information for one mesh or spline
	entity, within the COLLADA geometry library.
*/
#ifndef _FCD_GEOMETRY_H_
#define _FCD_GEOMETRY_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_

class FCDocument;
class FCDGeometryMesh;
class FCDGeometrySpline;
class FCDGeometryPSurface;

/**
	@defgroup FCDGeometry COLLADA Document Geometry Entity
	@addtogroup FCDocument
*/

/**
	A COLLADA geometry entity.
	There are three types of COLLADA geometry entities: meshes, NURBS surfaces and splines.

	Meshes are collections of polygons where the vertices always have a position,
	usually have a normal to define smooth or hard edges and may be colored or textured.

	Parametrized surfaces (PSurface) are a list of parametrized patches defined as a sequence of 
	weighted control points that may be associated with two knot vectors
	and possibly trimming curves to define a 3D surface.

	Splines are a sequence of control points used to generate a smooth curve.

	@ingroup FCDGeometry
*/

class FCOLLADA_EXPORT FCDGeometry : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);

	// Contains only one of the following, in order of importance.
	FUObjectRef<FCDGeometryMesh> mesh;
	FUObjectRef<FCDGeometryPSurface> pSurface;
	FUObjectRef<FCDGeometrySpline> spline;

public:
	/** Contructor: do not use directly.
		Create new geometries using the FCDLibrary::AddEntity function.
		@param document The COLLADA document which owns the new geometry entity. */
	FCDGeometry(FCDocument* document);

	/** Destructor. */
	virtual ~FCDGeometry();

	/** Retrieves the entity class type.
		This function is a part of the FCDEntity interface.
		@return The entity class type: GEOMETRY. */
	virtual Type GetType() const { return GEOMETRY; }

	/** Retrieves whether the type of this geometry is a mesh.
		@return Whether this geometry is a mesh. */
	bool IsMesh() const { return mesh != NULL; }

	/** Retrieves the mesh information structure for this geometry.
		Verify that this geometry is a mesh using the IsMesh function prior to calling this function.
		@return The mesh information structure. This pointer will be NULL when the geometry is a spline or is undefined. */
	FCDGeometryMesh* GetMesh() { return mesh; }
	const FCDGeometryMesh* GetMesh() const { return mesh; } /**< See above. */

	/** Sets the type of this geometry to mesh and creates an empty mesh structure.
		This function will release any mesh or spline structure that the geometry may already contain
		@return The mesh information structure. This pointer will always be valid. */
	FCDGeometryMesh* CreateMesh();

	/** Retrieves whether the type of this geometry is a spline.
		@return Whether this geometry is a spline. */
	bool IsSpline() const { return spline != NULL; }

	/** Retrieves the spline information structure for this geometry.
		Verify that this geometry is a spline using the IsSpline function prior to calling this function.
		@return The spline information structure. This pointer will be NULL when the geometry is a mesh or is undefined. */
	FCDGeometrySpline* GetSpline() { return spline; }
	const FCDGeometrySpline* GetSpline() const { return spline; } /**< See above. */

	/** Sets the type of this geometry to spline and creates an empty spline structure.
		This function will release any mesh or spline structure that the geometry may already contain.
		@return The spline information structure. This pointer will always be valid. */
	FCDGeometrySpline* CreateSpline();

	/** Retrieves whether the type of this geometry is a parametrized surface.
		@return Whether this geometry is a parametrized surface. */
	bool IsPSurface() const { return pSurface != NULL; }

	/** Retrieves the parametrized surface information structure for this geometry.
		Verify that this geometry is a parametrized surface using the IsNURBSSurfaceSet function prior to calling this function.
		@return The parametrized surface information structure. This pointer will be NULL when the geometry is a mesh, a spline or is undefined. */
	FCDGeometryPSurface* GetPSurface() { return pSurface; }
	const FCDGeometryPSurface* GetPSurface() const { return pSurface; } /**< See above. */

	/** Sets the type of this geometry to parametrized surface and creates an empty NURBS surface structure.
		This function will release any mesh, spline or parametrized surface structure that the geometry may already contain.
		@return The NURBS surface information structure. This pointer will always be valid. */
	FCDGeometryPSurface* CreatePSurface();

	/** @deprecated [INTERNAL] Clones the geometry entity.
		Works only on mesh geometry. Creates a full copy of the geometry, with the vertices overwritten
		by the given data: this is used when importing COLLADA 1.3 skin controllers.
		You will need to release the cloned entity.
		@param newPositions The list of vertex position that will
			overwrite the current mesh vertex positions. This list may be empty.
		@param newPositionsStride The stride, in bytes, of the newPositions list.
			For an empty newPositions list, this value is discarded.
		@param newNormals The list of vertex normals that will overwrite
			the current mesh vertex normals. This list may be empty.
		@param newNormalsStride The stride, in bytes, of the newNormals list.
			For an empty newNormals list, this value is discarded.
		@return The cloned geometry entity. This pointer will be NULL,
			if the geometry is not a mesh. You will need to release this pointer. */
	FCDGeometry* Clone(FloatList& newPositions, uint32 newPositionsStride, FloatList& newNormals, uint32 newNormalsStride);

	/** [INTERNAL] Reads in the \<geometry\> element from a given COLLADA XML tree node.
		@param node The COLLADA XML tree node.
		@return The status of the import. If the status is not successful,
			it may be dangerous to extract information from the geometry.*/
	virtual FUStatus LoadFromXML(xmlNode* node);

	/** [INTERNAL] Writes out the \<geometry\> element to the given COLLADA XML tree node.
		@param parentNode The COLLADA XML parent node in which to insert the geometry information.
		@return The created \<geometry\> element XML tree node. */
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
};

#endif // _FCD_GEOMETRY_H_
