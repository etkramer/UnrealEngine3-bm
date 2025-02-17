#ifndef GEOMETRY_3D_H
#define GEOMETRY_3D_H

#include "math3d.h"

// ------------------------------------------------------------------------------------
class Geometry3D {
// ------------------------------------------------------------------------------------
public:

static m3Real Geometry3D::rayClosestPoint(
			const m3Vector &orig, const m3Vector &dir, const m3Vector &p);

static m3Real segmentPointDist(
			const m3Vector &p1, const m3Vector &p2, const m3Vector &p);

static m3Real trianglePointDist(
			const m3Vector &p1, const m3Vector &p2, const m3Vector &p3, 
			const m3Vector &p);

static void Geometry3D::closestPointOnTriangle(
					const m3Vector &p1, const m3Vector &p2, const m3Vector &p3, const m3Vector &p,
					m3Vector &closest);

static m3Real boxPointDist(const m3Affine &pose, const m3Vector &extent, const m3Vector &p);

static bool cellBoxIntersection(const m3Vector &cellCenter, m3Real cellExtent, 
			const m3Affine &boxPose, const m3Vector &boxExtent);

static void boxCorners(const m3Affine &pose, const m3Vector &extent, m3Vector p[8]);

static bool triangleAABBIntersection(const m3Vector &a, const m3Vector &b, const m3Vector &c, 
									 const m3Bounds &bounds);

static bool rayTriangleIntersection(const m3Vector &orig, const m3Vector &dir,
										const m3Vector &a, const m3Vector &b, const m3Vector &c,
										m3Real &t, m3Real &u, m3Real &v);

static bool rayPlaneIntersection(const m3Vector &orig, const m3Vector &dir, const m3Plane &plane,
										m3Real &t);

static void circumSphere(const m3Vector &p0, const m3Vector &p1, 
								  const m3Vector &p2, const m3Vector &p3, 
								  m3Vector &center, m3Real &radiusSquared);

static void barycentricCoords(const m3Vector &p0, const m3Vector &p1,const m3Vector &p2, const m3Vector &p3,
								   const m3Vector &p, m3Vector &barycentricCoords);

static bool pointInTetra(const m3Vector &p0, const m3Vector &p1,const m3Vector &p2, const m3Vector &p3,
								   const m3Vector &p);


static bool collisionMovingPointMovingTriangle(const m3Vector &p0, const m3Vector &p1,
				const m3Vector &a0, const m3Vector &a1,
				const m3Vector &b0, const m3Vector &b1,
				const m3Vector &c0, const m3Vector &c1,				
				m3Real &collisionTime);

static bool collisionMovingSphereMovingTriangle(const m3Vector &p0, const m3Vector &p1, m3Real radius,
				const m3Vector &a0, const m3Vector &a1,
				const m3Vector &b0, const m3Vector &b1,
				const m3Vector &c0, const m3Vector &c1,				
				m3Real &collisionTime);

private:
};

#endif
