/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FCD_PHYSICS_ANALYTICAL_GEOM_H_
#define _FCD_PHYSICS_ANALYTICAL_GEOM_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

class FCDocument;
class FCDPhysicsShape;
class FCDPhysicsParameterGeneric;
typedef vector<FCDPhysicsShape*> FCDPhysicsShapeList;
typedef vector<FCDPhysicsParameterGeneric*> FCDPhysicsParameterList;

class FCOLLADA_EXPORT FCDPhysicsAnalyticalGeometry : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);

public:
	enum GeomType { BOX, PLANE, SPHERE, CYLINDER, CAPSULE, TAPERED_CYLINDER, TAPERED_CAPSULE };

	FCDPhysicsAnalyticalGeometry(FCDocument* document);
	virtual ~FCDPhysicsAnalyticalGeometry();

	virtual Type GetType() const {return PHYSICS_ANALYTICAL_GEOMETRY;}

	// Returns the entity type
	virtual GeomType GetGeomType() const = 0;

	// Create a copy of this analyticalGeometry
	virtual FCDPhysicsAnalyticalGeometry* Clone() = 0;

	// Read in the <physics_analyticalGeometry> node of the COLLADA document
	virtual FUStatus LoadFromXML(xmlNode* node);

	// Write out the <physics_analyticalGeometry> node to the COLLADA XML tree
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const = 0;
};

class FCOLLADA_EXPORT FCDPASBox : public FCDPhysicsAnalyticalGeometry
{
private:
	DeclareObjectType(FCDPhysicsAnalyticalGeometry);

public:
	FCDPASBox(FCDocument* document);
	virtual ~FCDPASBox() {}

	virtual FUStatus LoadFromXML(xmlNode* node);
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
	virtual GeomType GetGeomType() const {return BOX;}
	virtual FCDPhysicsAnalyticalGeometry* Clone();

public:
	FMVector3 halfExtents;
};

class FCOLLADA_EXPORT FCDPASPlane : public FCDPhysicsAnalyticalGeometry
{
private:
	DeclareObjectType(FCDPhysicsAnalyticalGeometry);

public:
	FCDPASPlane(FCDocument* document);
	virtual ~FCDPASPlane() {}

	virtual FUStatus LoadFromXML(xmlNode* node);
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
	virtual GeomType GetGeomType() const {return PLANE;}
	virtual FCDPhysicsAnalyticalGeometry* Clone();

public:
	FMVector3 normal;
	float d;
};

class FCOLLADA_EXPORT FCDPASSphere : public FCDPhysicsAnalyticalGeometry
{
private:
	DeclareObjectType(FCDPhysicsAnalyticalGeometry);

public:
	FCDPASSphere(FCDocument* document);
	virtual ~FCDPASSphere() {}

	virtual FUStatus LoadFromXML(xmlNode* node);
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
	virtual GeomType GetGeomType() const {return SPHERE;}
	virtual FCDPhysicsAnalyticalGeometry* Clone();

public:
	float radius;
};

class FCOLLADA_EXPORT FCDPASCylinder : public FCDPhysicsAnalyticalGeometry
{
private:
	DeclareObjectType(FCDPhysicsAnalyticalGeometry);

public:
	FCDPASCylinder(FCDocument* document);
	virtual ~FCDPASCylinder() {}

	virtual FUStatus LoadFromXML(xmlNode* node);
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
	virtual GeomType GetGeomType() const {return CYLINDER;}
	virtual FCDPhysicsAnalyticalGeometry* Clone();

public:
	float height;
	float radius;
};

class FCOLLADA_EXPORT FCDPASCapsule : public FCDPhysicsAnalyticalGeometry
{
private:
	DeclareObjectType(FCDPhysicsAnalyticalGeometry);

public:
	FCDPASCapsule(FCDocument* document);
	virtual ~FCDPASCapsule() {}

	virtual FUStatus LoadFromXML(xmlNode* node);
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
	virtual GeomType GetGeomType() const {return CAPSULE;}
	virtual FCDPhysicsAnalyticalGeometry* Clone();

public:
	float height;
	float radius;
};

class FCOLLADA_EXPORT FCDPASTaperedCapsule : public FCDPASCapsule
{
private:
	DeclareObjectType(FCDPASCapsule);

public:
	FCDPASTaperedCapsule(FCDocument* document);
	virtual ~FCDPASTaperedCapsule() {}

	virtual FUStatus LoadFromXML(xmlNode* node);
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
	virtual GeomType GetGeomType() const {return TAPERED_CAPSULE;}
	virtual FCDPhysicsAnalyticalGeometry* Clone();

public:
	//inherits all other attributes from Capsule
	float radius2;
};

class FCOLLADA_EXPORT FCDPASTaperedCylinder : public FCDPASCylinder
{
private:
	DeclareObjectType(FCDPASCylinder);

public:
	FCDPASTaperedCylinder(FCDocument* document);
	virtual ~FCDPASTaperedCylinder() {}

	virtual FUStatus LoadFromXML(xmlNode* node);
	virtual xmlNode* WriteToXML(xmlNode* parentNode) const;
	virtual GeomType GetGeomType() const {return TAPERED_CYLINDER;}
	virtual FCDPhysicsAnalyticalGeometry* Clone();

public:
	//inherits all other attributes from Cylinder
	float radius2;
};


#endif // _FCD_PHYSICS_ANALYTICAL_GEOMETRY_H_
