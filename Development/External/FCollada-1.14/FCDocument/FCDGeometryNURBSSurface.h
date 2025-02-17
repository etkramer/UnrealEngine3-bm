/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/**
	@file FCDGeometryNURBSSurface.h
	This file contains the FCDGeometryNURBSSurface class.
	The FCDGeometryNURBSSurface class hold the information for one COLLADA NURBS surface.
*/

#ifndef _FCD_GEOMETRY_NURBS_SURFACE_H_
#define _FCD_GEOMETRY_NURBS_SURFACE_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_

typedef vector<FCDNURBSSpline*>				FCDTrimCurveGroup;
typedef vector<FCDTrimCurveGroup>			FCDTrimCurveGroupList;

class FCDGeometryPSurface;

/**
	Describes a NURBS surface and its trim curves.

	Trim curves are NURBS curves on the surface parametric domain in 2D.
*/
class FCOLLADA_EXPORT FCDGeometryNURBSSurface : public FCDObject
{
private:
	DeclareObjectType(FCDGeometryNURBSSurface);

	FUDaeNURBSSurface::Type type;

	FCDCVs					cvs;
	FloatList				weights;
	FloatList				uKnots, vKnots;
	uint32					p,q; // the u and v degrees, respectively

	bool					closedInU, closedInV;

	FCDTrimCurveGroupList	trimGroups;
	string					name;

	FCDGeometryNURBSSurface* textureSurface;
	fstring					materialSemantic;

	FCDGeometryPSurface*	parentSet;

	// Max-specific
	bool					flipNormals;

public:
	/** Constructor. Do not use directly, use the FCDGeometryPSurface::AddNURBSSurface method instead. */
	FCDGeometryNURBSSurface(FCDocument* document, FCDGeometryPSurface* parentSet);

	/** Destructor. */
	virtual ~FCDGeometryNURBSSurface();

	/** Get the type of this NURBS surface
		@return The surface type.*/
	FUDaeNURBSSurface::Type GetType(){ return type; }

	/** Get the parent FCDGeometryPSurface of this NURBS surface.
		@return A pointer to the parent set.*/
	FCDGeometryPSurface* GetParentSet() { return parentSet; }
	const FCDGeometryPSurface* GetParentSet() const { return parentSet; }/**< See above. */

	/** Gets the name of the surface.
		@return The surface name.*/
	string& GetName(){ return name; }
	const string& GetName() const { return name; }/**< See above.*/

	/** Sets the name of the surface.
		@param _name The new name.*/
	void SetName( string& _name ){ name = _name; }

	/**	Gain access to the CV list.
		@return The control vertex vector. */
	FCDCVs& GetCVs() { return cvs; }
	const FCDCVs& GetCVs() const { return cvs; } /**< See above.*/

	/**	Get the number of control vertex in this surface.
		@return The number of control vertex in this surface. */
	uint32 GetCVCount() const { return (uint32)cvs.size(); }

	/** Gain access to the weight list.
		@return The weights vector.*/
	FloatList& GetWeights() { return weights; }
	const FloatList& GetWeights() const { return weights; } /**< See above.*/

	/** Get the number of weights in the weight vector.
		@return The number of weight elements.*/
	uint32 GetWeightCount() const { return (uint32)weights.size(); }

	/** Gain access to the knots in the U dimension.
		@return The U knot vector. */
	FloatList& GetUKnots(){ return uKnots; }
	const FloatList& GetUKnots() const { return uKnots; } /**< See above.*/

	/** Get the knot count in the U dimension.
		@return The number of knots in the  vector. */
	uint32 GetUKnotCount() const { return (uint32)uKnots.size(); }

	/** Gain access to the knots in the V dimension.
		@return The V knot vector. */
	FloatList& GetVKnots(){ return vKnots; }
	const FloatList& GetVKnots() const { return vKnots; } /**< See above.*/

	/** Get the knot count in the V dimension.
		@return The number of knots in the vector. */
	uint32 GetVKnotCount() const { return (uint32)vKnots.size(); }

	/** Get the knot domain of the surface. Warning: This method relies on the fact that the
		knot vectors must be of length: N + D + 1, where N is the number of CVs and D the degree
		(either in u or v). If this relation is not satisfied then there may be an out of bounds
		error triggered.
		@param uStart (in/out) The start value in U.
		@param uEnd (in/out) The end value in U.
		@param vStart (in/out) The start value in V.
		@param vEnd (in/out) The end value in V.*/
	void GetKnotDomain( float& uStart, float& uEnd, float& vStart, float& vEnd );

	/** Get the surface U degree.
		@return The U degree. */
	uint32 GetUDegree() const { return p; }

	/** Set the surface U degree.
		@param The desired degree. */
	void SetUDegree( uint32 _p ) { p = _p; }

	/** Get the surface V degree.
		@return The V degree. */
	uint32 GetVDegree() const { return q; }

	/** Set the surface V degree.
		@param _q The desired degree. */
	void SetVDegree( uint32 _q ) { q = _q; }

	/** Know if the surface is closed in U
		@return True if the surface is closed, false otherwise.*/
	bool GetClosedInU() const { return closedInU; }

	/** Set the surface closed in U
		@param closed The desired closed state.*/
	void SetClosedInU(bool closed){ closedInU = closed; }

	/** Know if the surface is closed in V
		@return True if the surface is closed, false otherwise.*/
	bool GetClosedInV() const { return closedInV; }

	/** Set the surface closed in V
		@param closed The desired closed state.*/
	void SetClosedInV(bool closed){ closedInV = closed; }

	/** Get the Max-specific flipNormals attribute.
		@return True the normals are flipped.*/
	bool GetFlipNormals() const { return flipNormals; }

	/** Set the Max-specific flipNormals attribute.
		@param True if we should flip the normals.*/
	void SetFlipNormals( bool flip ) { flipNormals = flip; }

	/** Gain access to the trim curve group list.
		@return The trim curve group list. */
	FCDTrimCurveGroupList& GetTrimCurveGroups() { return trimGroups; }
	const FCDTrimCurveGroupList& GetTrimCurveGroups() const { return trimGroups; }/**< See above.*/

	/** Get the number of trim curves affecting this surface.
		@return The number of curves in the trim curve list. */
	uint32 GetTrimCurveGroupCount() const { return (uint32)trimGroups.size(); }

	/** Adds a trim group to the surface.
		@return The added trim group index. */
	uint32 AddTrimGroup();

	/** Adds an empty trim curve to the specified trim group.
		@param groupIndex The desired group index in which to add the new trim curve.
		@return The newly created curve, NULL if the group index is invalid. */
	FCDNURBSSpline* AddTrimCurve(uint32 groupIndex);

	/** Adds the given NURBS spline to the specified trim group.
		@param groupIndex The desired group index.
		@param toAdd The spline to append to the group list.
		@return toAdd on success, NULL on failure.*/
	FCDNURBSSpline* AddTrimCurve( uint32 groupIndex, FCDNURBSSpline* toAdd );

	/** Get the texture surface attached to this *vertex* surface (use GetType() to validate this).
		@param doCreate If set to true and the current texture surface is null, it will create a new one and assign it.
		@return The assigned texture surface. */
	FCDGeometryNURBSSurface* GetTextureSurface(bool doCreate);

	/** Get a constant pointer to the texture surface attached to this surface.
		@return The assigned texture surface, may be null. */
	const FCDGeometryNURBSSurface* GetTextureSurface() const { return textureSurface; }

	/** Use this method to know if this surface has some texture coordinates attached.
		@return true if the texture coordinates surface is not null, false otherwise.*/
	bool HasTextureCoordinates() const { return (textureSurface != NULL); }

	/** Sets the material semantic used with this NURBS surface's texture surface.
		@param ms The material semantic.*/
	void SetMaterialSemantic( const fstring& ms );

	/** Get the material semantic used with this NURBS surface's texture surface.
		@return The material semantic.*/
	fstring GetMaterialSemantic(){ return materialSemantic; }
	const fstring GetMaterialSemantic() const { return materialSemantic; }/**< See above.*/

	/** Use this method to validate the data within this NURBS surface. If it returns True,
		then the surface is valid and can be used safely.
		@param status This pointer will hold the error messages. Pass in NULL for no feedback. Default: NULL.
		@return False if there's any error within the data of this surface.*/
	bool IsValid( FUStatus* status = NULL );

	/** [INTERNAL] Reads in the \<nurbs\> element from a given COLLADA XML tree node.
		@param nurbsNode The COLLADA XML tree node.
		@return The status of the import. If the status is not successful,
			it may be dangerous to extract information from the NURBS surface.*/
	FUStatus LoadFromXML(xmlNode* nurbsNode);

	/** [INTERNAL] Writes out the \<nurbs\> element to the given COLLADA XML tree node.
		@param parentNode The COLLADA XML parent node in which to insert the NURBS surface information.
		@return The created \<nurbs\> element XML tree node. */
	xmlNode* WriteToXML(xmlNode* parentNode) const;

private:
	void SetType( FUDaeNURBSSurface::Type _type ){ type = _type; }
	void RemoveEmptyTrimGroups();

};

/** A dynamically-sized array of NURBS surfaces. @ingroup FCDGeometry */
typedef vector< FCDGeometryNURBSSurface* > FCDNURBSSurfaceList;

class FCOLLADA_EXPORT FCDGeometryPSurface : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	FCDGeometry* parent;
	FCDNURBSSurfaceList nurbsSurfaceList;

public:
	/** Constructor: do not use directly. Use the FCDGeometry::CreatePSurface function instead.
		@param document The COLLADA document that owns the new parametrized surface.
		@param parent The geometry entity that contains the new parametrized surface. */
	FCDGeometryPSurface(FCDocument* document, FCDGeometry* parent);

	/** Destructor. */
	virtual ~FCDGeometryPSurface();

	/** Retrieve the parent of this geometric parametrized surface: the geometry entity.
		@return The geometry entity that this parametrized surface belongs to. */
	FCDGeometry* GetParent() { return parent; }
	const FCDGeometry* GetParent() const { return parent; } /**< See above. */

	/** Gain access to the NURBS Surface list of the parametrized surface.
		@return A reference to the NURBS Surface list.*/
	FCDNURBSSurfaceList& GetNURBSSurfaceList() { return nurbsSurfaceList; }
	const FCDNURBSSurfaceList& GetNURBSSurfaceList() const { return nurbsSurfaceList; }/**< See above. */

	/** Adds a new NURBS surface to this parametrized surface.
		@return The newly created surface. */
	FCDGeometryNURBSSurface* AddNURBSSurface();

	/** Get the number of NURBS surfaces in the parametrized surface.
		@return The NURBS surface count.*/
	uint32 GetNURBSSurfaceCount() const { return (uint32)nurbsSurfaceList.size(); }

	/** Get the index in this parametrized surface for the given FCDGeometryNURBSSurface key.
		@param key The surface to look for.
		@param index (out) The index of the surface within the parametrized surface.
		@return True if the surface has been found. */
	bool FindNURBSSurface( const FCDGeometryNURBSSurface* key, uint32& index ) const;

	/** [INTERNAL] Reads in the \<nurbs\> elements from a given COLLADA XML tree node.
		@param geometryNode The COLLADA XML tree node of the parent geometry.
		@return The status of the import. If the status is not successful,
			it may be dangerous to extract information from the parametrized surface.*/
	FUStatus LoadFromXML(xmlNode* geometryNode);

	/** [INTERNAL] Writes out the \<nurbs\> elements to the given COLLADA XML tree node.
		@param parentNode The COLLADA XML parent node in which to insert the parametrized surface information.
		@return NULL since it doesn't create a node for the set but a list of sibblings. */
	xmlNode* WriteToXML(xmlNode* parentNode) const;
};

#endif // _FCD_GEOMETRY_NURBS_SURFACE_H_
