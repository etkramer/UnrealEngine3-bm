/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/**
	@file FCDGeometrySpline.h
	This file contains the FCDGeometrySpline class.
	The FCDGeometrySpline class hold the information for one COLLADA geometric spline.
*/
#ifndef _FCD_GEOMETRY_SPLINE_H_
#define _FCD_GEOMETRY_SPLINE_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_

class FCDocument;
class FCDGeometry;

/** A dynamically-sized array of geometric spline control points. Each control point is simply one 4D position. The fourth dimension is ONLY used in NURBS segments (the control point's weight). @ingroup FCDGeometry */
typedef FMVector3List FCDCVs;
/** A dynamically-sized array of weight values. Each weigth value represents a knot of a control point. @ingroup FCDGeometry */
typedef FloatList FCDKnots;

/** Represents a generic spline.
	A FCSpline contains a list of control vertices and a closed attribute which defaults to false.*/
class FCDSpline : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	FUDaeSplineForm::Form form;
	string name;

protected:
	FCDCVs cvs;

public:
	FCDSpline(FCDocument* document);
	virtual ~FCDSpline();

	/** Retrieves the type of the spline. This is the only method of the FCDSpline interface.
		@return FUDaeSplineType of the spline.*/
	virtual FUDaeSplineType::Type GetSplineType() const = 0;

	/** Gets the name of the spline.
		@return The spline name.*/
	string& GetName(){ return name; }
	const string& GetName() const { return name; }/**< See above.*/

	/** Sets the name of the spline.
		@param _name The new name.*/
	void SetName( string& _name ){ name = _name; }

	/** Retrieves if the spline is closed or not.
		@return The closed boolean value.*/
	bool IsClosed() const { return (form == FUDaeSplineForm::CLOSED); }

	/** Sets if the spline is closed or not.
		@param closed The closed attribute value.*/
	void SetClosed( bool closed ){ form = (closed) ? FUDaeSplineForm::CLOSED : FUDaeSplineForm::OPEN; }

	/** Retrieves the number of CVs in the spline.
		@return Count of control vertices.*/
	size_t GetCVCount(){ return cvs.size(); }

	/** Retrieves a pointer to the control vertex specified by the given index.
		@param index The index, must be higher or equal to 0 and lower than GetCVCount().
		@return The CV reference.*/
	FMVector3* GetCV( size_t index ){ FUAssert(index < GetCVCount(), return NULL); return &(cvs.at(index)); }

	/** Retrieve a reference to the CVs list
		@return The reference to the CV vector.*/
	FCDCVs& GetCVs() { return cvs; }
	const FCDCVs& GetCVs() const { return cvs; } /**< See above. */

	/** Determines if the spline is valid. Pure virtual.
		@return True is the spline is valid, false otherwise.*/
	virtual bool IsValid(FUStatus* status = NULL) const = 0;
};

/** A dynamically-sized array of FCSpline.*/
typedef vector<FCDSpline*> FCDSplineList;

class FCDNURBSSpline;

/** A dynamically-sized array of FCDGeometrySpline. @ingroup FCDGeometry */
typedef vector< FCDNURBSSpline * > FCDNURBSSplineList;

/** Represents a Bezier spline.*/
class FCDBezierSpline : public FCDSpline
{
private:
	DeclareObjectType(FCDSpline);

public:
	FCDBezierSpline(FCDocument* document);
	virtual ~FCDBezierSpline();

	/** FCDSpline method implementation.
		@return The BEZIER spline type.*/
	virtual FUDaeSplineType::Type GetSplineType() const { return FUDaeSplineType::BEZIER; }

	/** Adds a CV to a Bezier spline
		@param cv 3D position of the CV.*/
	bool AddCV( const FMVector3& cv ){ cvs.push_back( cv ); return true; }

	/** Creates one NURB per Bezier segment and appends it to the provided NURB list.
		@param toFill The NURB list to fill.*/
	void ToNURBs( FCDNURBSSplineList &toFill ) const;

	/** Determines if the spline is valid.
		@return True is the spline is valid, false otherwise.*/
	virtual bool IsValid(FUStatus* status = NULL) const;

	/** [INTERNAL] Reads in the \<spline\> element from a given COLLADA XML tree node.
		@param splineNode The COLLADA XML tree node.
		@return The status of the import. If the status is not successful,
			it may be dangerous to extract information from the spline.*/
	FUStatus LoadFromXML(xmlNode* splineNode);

	/** [INTERNAL] Writes out the \<spline\> element to the given COLLADA XML tree node.
		The sources in this spline will have an ID built this way : "parentId ID splineId".
		@param parentId The parent's ID string.
		@param parentNode The COLLADA XML parent node in which to insert the spline information.
		@param splineId The spline's ID string.
		@return The created \<nurbs\> element XML tree node. */
	xmlNode* WriteToXML(xmlNode* parentNode, const string& parentId, const string& splineId) const;
};

/** Represents a NURB spline.*/
class FCDNURBSSpline : public FCDSpline
{
private:
	DeclareObjectType(FCDSpline);

	FloatList weights;
	FCDKnots knots;
	uint32 degree;

public:
	FCDNURBSSpline(FCDocument* document);
	virtual ~FCDNURBSSpline();

	/** FCDSpline method implementation.
		@return The NURBS spline type.*/
	virtual FUDaeSplineType::Type GetSplineType() const { return FUDaeSplineType::NURBS; }

	/** Get the degree for this NURBS.
		@return The degree.*/
	uint32 GetDegree() const { return degree; }

	/** Set the degree for this NURBS.
		@param deg The wanted degree.*/
	void SetDegree( uint32 deg ){ degree = deg; }

	/** Add a control vertex as a 3D position and a weight attribute specific to this CV.
		@param cv The 3D position.
		@param weight The weight attribute.*/
	bool AddCV( const FMVector3& cv, float weight );

	/** Retrieves a reference to the weight specified by the index.
		@param index The index.
		@return The address of the weight value, NULL if index is invalid.*/
	float* GetWeight( size_t index ){ FUAssert( index < GetCVCount(), return NULL); return &(weights.at(index)); }

	/** Retrieves the knot count in this NURB.
		@return The knot count.*/
	size_t GetKnotCount(){ return knots.size(); }

	/** Add a knot to this NURB.
		@param knot The knot value.*/
	void AddKnot( float knot ){ knots.push_back( knot ); }

	/** Retrieves a reference to the knot specified by the index.
		@param index The index.
		@return The address of the knot value, NULL if index is invalid.*/
	float* GetKnot( size_t index ){ FUAssert( index < GetKnotCount(), return NULL); return &(knots.at(index));}

	/** Retrieve a const reference to the weight list.
		@return The weights' const reference.*/
	FloatList& GetWeights() { return weights; }
	const FloatList& GetWeights() const { return weights; } /**< See above. */

	/** Retrive a const reference to the knot list.
		@return The knots' const reference.*/
	FCDKnots& GetKnots() { return knots; }
	const FCDKnots& GetKnots() const { return knots; } /**< See above. */

	/** Determines if the spline is valid.
		@return True is the spline is valid, false otherwise.*/
	virtual bool IsValid(FUStatus* status = NULL) const;

	/** [INTERNAL] Reads in the \<spline\> element from a given COLLADA XML tree node.
		@param splineNode The COLLADA XML tree node.
		@return The status of the import. If the status is not successful,
			it may be dangerous to extract information from the spline.*/
	FUStatus LoadFromXML(xmlNode* splineNode);

	/** [INTERNAL] Writes out the \<spline\> element to the given COLLADA XML tree node.
		The sources in this spline will have an ID built this way : "parentId ID splineId".
		@param parentId The parent's ID string.
		@param parentNode The COLLADA XML parent node in which to insert the spline information.
		@param splineId The spline's ID string.
		@return The created \<nurbs\> element XML tree node. */
	xmlNode* WriteToXML(xmlNode* parentNode, const string& parentId, const string& splineId) const;
};

/**
	A COLLADA geometric spline.

	A COLLADA spline contains an array of FCDSpline of the same type.

	@todo: Insert the mathematical formula to calculate the spline position.

	@ingroup FCDGeometry
*/
class FCOLLADA_EXPORT FCDGeometrySpline : public FCDObject
{
private:
	DeclareObjectType(FCDObject);
	FCDGeometry* parent;

	FUDaeSplineType::Type type;
	FCDSplineList splines;

public:
	/** Constructor: do not use directly. Use the FCDGeometry::CreateMesh function instead.
		@param document The COLLADA document that owns the new spline.
		@param parent The geometry entity that contains the new spline. */
	FCDGeometrySpline(FCDocument* document, FCDGeometry* parent, FUDaeSplineType::Type type = FUDaeSplineType::UNKNOWN );

	/** Destructor. */
	virtual ~FCDGeometrySpline();

	/** Retrieve the parent of this geometric spline: the geometry entity.
		@return The geometry entity that this spline belongs to. */
	FCDGeometry* GetParent() { return parent; }
	const FCDGeometry* GetParent() const { return parent; } /**< See above. */

	/** Retrieve the type of this geometry spline.
		@return The type.*/
	FUDaeSplineType::Type GetType(){ return type; }

	/** Set the spline type for this geometry spline.
		@param The type.
		@retrun False if there are already splines of a different type in the spline array, True otherwise.*/
	bool SetType( FUDaeSplineType::Type _type );

	/** Retrieve the number of splines in this geometry spline.
		@return The spline count.*/
	size_t GetSplineCount() const { return splines.size(); }

	/** Retrieve the total amount of control vertices in the spline array.
		@return The total CV count.*/
	size_t GetTotalCVCount();

	/** Retrieve a pointer to the spline specified by the given index.
		@param The index, higher or equal to 0 and lower than GetSplineCount().
		@return The FCDSpline pointer, or NULL if index is invalid.*/
	FCDSpline* GetSpline(size_t index){ FUAssert( index < GetSplineCount(), return NULL); return splines[index]; }
	const FCDSpline* GetSpline(size_t index) const { FUAssert( index < GetSplineCount(), return NULL); return splines[index]; }/**< see above */

	/** Add a spline to this geometry spline.
		@param spline The spline reference.
		@return False if spline is NULL or if spline's type isn't equivalent to this geometry spline type.*/
	bool AddSpline( FCDSpline *spline );

	/** Remove the given spline in the spline array.
		@param spline The spline to remove.
		@return The removed spline, NULL if not found.*/
	FCDSpline* RemoveSpline( FCDSpline* spline );

	/** Remove the spline specified by the given index.
		@param index The index, lower than GetSplineCount()
		@return The removed spline, NULL if index is invalid.*/
	FCDSpline* RemoveSpline( size_t index );

	/** Convert the Bezier splines in this geometry to a list of NURBS splines.
		@param toFill The list of NURBS to fill. An empty list if the type of this geometry is not BEZIER.*/
	void ConvertBezierToNURBS(FCDNURBSSplineList &toFill );

	/** [INTERNAL] Reads in the \<spline\> element from a given COLLADA XML tree node.
		@param splineNode The COLLADA XML tree node.
		@return The status of the import. If the status is not successful,
			it may be dangerous to extract information from the spline.*/
	FUStatus LoadFromXML(xmlNode* splineNode);

	/** [INTERNAL] Writes out the \<spline\> element to the given COLLADA XML tree node.
		@param parentNode The COLLADA XML parent node in which to insert the spline information.
		@return The created \<spline\> element XML tree node. */
	xmlNode* WriteToXML(xmlNode* parentNode) const;
};

#endif // _FCD_GEOMETRY_SPLINE_H_
