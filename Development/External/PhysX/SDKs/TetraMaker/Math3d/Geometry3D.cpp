#include "Geometry3D.h"
#include "tribox.h"

// -----------------------------------------------------------------------------------
m3Real Geometry3D::rayClosestPoint(const m3Vector &orig, const m3Vector &dir, const m3Vector &p)
// -----------------------------------------------------------------------------------
{
	m3Real d2 = dir.dot(dir);
	if (d2 == 0.0f) 
		return 0.0f;
	return dir.dot(p-orig)/d2;
}

// -----------------------------------------------------------------------------------
m3Real Geometry3D::segmentPointDist(
					const m3Vector &p1, const m3Vector &p2, const m3Vector &p)
// -----------------------------------------------------------------------------------
{
	m3Vector d = p2-p1;
	m3Real d2 = d.dot(d);
	if (d2 == 0.0f) 
		return p1.distance(p);
	m3Real t = d.dot(p-p1)/d2;
	if (t < 0.0f) 
		return p1.distance(p);
	else if (t > 1.0f)
		return p2.distance(p);
	m3Vector q = p1 + d*t;
	return q.distance(p);
}

// -----------------------------------------------------------------------------------
m3Real Geometry3D::trianglePointDist(
					const m3Vector &p1, const m3Vector &p2, const m3Vector &p3, 
					const m3Vector &p)
// -----------------------------------------------------------------------------------
{
	m3Vector d1 = p2-p1;
	m3Vector d2 = p3-p1;
	m3Vector pp1 = p-p1;
	m3Real a = d1.dot(d1);
	m3Real b = d2.dot(d1);
	m3Real c = pp1.dot(d1);
	m3Real d = b;
	m3Real e = d2.dot(d2);
	m3Real f = pp1.dot(d2);
	m3Real det = a*e - b*d;
	if (det != 0.0f) {
		m3Real s = (c*e-b*f) / det;
		m3Real t = (a*f-c*d) / det;
		if (s > 0.0f && t > 0.0f && (s+t) < 1.0f) {
			m3Vector q = p1 + d1*s + d2*t;
			return q.distance(p);
		}
	}
	m3Real dist = segmentPointDist(p1, p2, p);
	dist = m3Min(dist, segmentPointDist(p2, p3, p));
	dist = m3Min(dist, segmentPointDist(p3, p1, p));
	return dist;
}

// -----------------------------------------------------------------------------------
void Geometry3D::closestPointOnTriangle(
					const m3Vector &p1, const m3Vector &p2, const m3Vector &p3, const m3Vector &p,
					m3Vector &closest)
// -----------------------------------------------------------------------------------
{
	m3Vector e0 = p2-p1;
	m3Vector e1 = p3-p1;
	m3Vector tmp = p1-p;

	m3Real a = e0.dot(e0);
	m3Real b = e0.dot(e1);
	m3Real c = e1.dot(e1);
	m3Real d = e0.dot(tmp);
	m3Real e = e1.dot(tmp);
	m3Vector coords, clampedCoords;
	coords.x = b*e - c*d;    // s * det
	coords.y = b*d - a*e;    // t * det
	coords.z = a*c - b*b;    // det

	clampedCoords.set(0.0f, 0.0f, 0.0f);
	if (coords.x <= 0.0f) {
		if (c != 0.0f)
			clampedCoords.y = -e/c;
	}
	else if (coords.y <= 0.0f) {
		if (a != 0.0f)           
			clampedCoords.x = -d/a;
	}
	else if (coords.x + coords.y > coords.z) {
		m3Real denominator = a + c - b - b;
		m3Real numerator   = c + e - b - d;
		if (denominator != 0.0f) {
			clampedCoords.x = numerator / denominator;
			clampedCoords.y = 1.0f - clampedCoords.x;
		}
	}
	else {    // all inside
		if (coords.z != 0.0f) {
			clampedCoords.x = coords.x / coords.z;
			clampedCoords.y = coords.y / coords.z;
		}
	}
	clampedCoords.x = m3Max(clampedCoords.x, 0.0f);
	clampedCoords.y = m3Max(clampedCoords.y, 0.0f);
	clampedCoords.x = m3Min(clampedCoords.x, 1.0f);
	clampedCoords.y = m3Min(clampedCoords.y, 1.0f);

	closest = p1 + e0 * clampedCoords.x + e1 * clampedCoords.y;
}

// -----------------------------------------------------------------------------------
m3Real Geometry3D::boxPointDist(const m3Affine &pose, const m3Vector &extent, const m3Vector &p)
// -----------------------------------------------------------------------------------
{
	m3Vector p0 = !pose * p;
	m3Real dx1 = -extent.x - p0.x;
	m3Real dx2 = p0.x - extent.x;
	m3Real dy1 = -extent.y - p0.y;
	m3Real dy2 = p0.y - extent.y;
	m3Real dz1 = -extent.z - p0.z;
	m3Real dz2 = p0.z - extent.z;

	m3Real dist2 = 0.0f;
	if (dx1 > 0.0f) dist2 += dx1*dx1;
	if (dx2 > 0.0f) dist2 += dx2*dx2;
	if (dy1 > 0.0f) dist2 += dy1*dy1;
	if (dy2 > 0.0f) dist2 += dy2*dy2;
	if (dz1 > 0.0f) dist2 += dz1*dz1;
	if (dz2 > 0.0f) dist2 += dz2*dz2;
	return sqrt(dist2);
}

// -----------------------------------------------------------------------------------
bool Geometry3D::cellBoxIntersection(const m3Vector &cellCenter, m3Real cellExtent, 
					const m3Affine &boxPose, const m3Vector &boxExtent)
// -----------------------------------------------------------------------------------
{
	m3Vector cExt(cellExtent, cellExtent, cellExtent);
	m3Vector kD = boxPose.pos - cellCenter;
    const m3Real fCutoff = 1.0f - 1e-6f;
    bool  bExistsParallelPair = false;

	const m3Matrix &B = boxPose.mat;
	m3Matrix absB;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			absB(i,j) = fabs(B(i,j));
			if (absB(i,j) > fCutoff ) bExistsParallelPair = true;
		}
	}

    if (fabs(kD.x) > cExt.x + boxExtent.dot(absB.getRow(0)))
        return false;

	if (fabs(kD.y) > cExt.y + boxExtent.dot(absB.getRow(1)))
        return false;

	if (fabs(kD.z) > cExt.z + boxExtent.dot(absB.getRow(2)))
        return false;

	if (fabs(B.getColumn(0).dot(kD)) > cExt.dot(absB.getColumn(0)) + boxExtent.x)
        return false;

	if (fabs(B.getColumn(1).dot(kD)) > cExt.dot(absB.getColumn(1)) + boxExtent.y)
        return false;

	if (fabs(B.getColumn(2).dot(kD)) > cExt.dot(absB.getColumn(2)) + boxExtent.z)
        return false;

    if ( bExistsParallelPair )
        return true;

	if (fabs(kD.z*B(1,0) - kD.y*B(2,0)) > 
		cExt.y*absB(2,0) + cExt.z*absB(1,0) + boxExtent.y*absB(0,2) + boxExtent.z*absB(0,1))
		return false;

	if (fabs(kD.z*B(1,1) - kD.y*B(2,1)) > 
		cExt.y*absB(2,1) + cExt.z*absB(1,1) + boxExtent.x*absB(0,2) + boxExtent.z*absB(0,0))
		return false;

	if (fabs(kD.z*B(1,2) - kD.y*B(2,2)) > 
		cExt.y*absB(2,2) + cExt.z*absB(1,2) + boxExtent.x*absB(0,1) + boxExtent.y*absB(0,0))
		return false;

	if (fabs(kD.x*B(2,0) - kD.z*B(0,0)) > 
		cExt.x*absB(2,0) + cExt.z*absB(0,0) + boxExtent.y*absB(1,2) + boxExtent.z*absB(1,1))
		return false;

	if (fabs(kD.x*B(2,1) - kD.z*B(0,1)) > 
		cExt.x*absB(2,1) + cExt.z*absB(0,1) + boxExtent.x*absB(1,2) + boxExtent.z*absB(1,0))
		return false;

	if (fabs(kD.x*B(2,2) - kD.z*B(0,2)) > 
		cExt.x*absB(2,2) + cExt.z*absB(0,2) + boxExtent.x*absB(1,1) + boxExtent.y*absB(1,0))
		return false;

	if (fabs(kD.y*B(0,0) - kD.x*B(1,0)) > 
		cExt.x*absB(1,0) + cExt.y*absB(0,0) + boxExtent.y*absB(2,2) + boxExtent.z*absB(2,1))
		return false;

	if (fabs(kD.y*B(0,1) - kD.x*B(1,1)) > 
		cExt.x*absB(1,1) + cExt.y*absB(0,1) + boxExtent.x*absB(2,2) + boxExtent.z*absB(2,0))
		return false;

	if (fabs(kD.y*B(0,2) - kD.x*B(1,2)) > 
		cExt.x*absB(1,2) + cExt.y*absB(0,2) + boxExtent.x*absB(2,1) + boxExtent.y*absB(2,0))
		return false;

    return true;
}

// -----------------------------------------------------------------------------------
void Geometry3D::boxCorners(const m3Affine &pose, const m3Vector &extent, m3Vector p[8])
// -----------------------------------------------------------------------------------
{
	m3Vector a0 = pose.mat.getColumn(0)*extent.x;
	m3Vector a1 = pose.mat.getColumn(1)*extent.y;
	m3Vector a2 = pose.mat.getColumn(2)*extent.z;
	p[0] = pose.pos - a0 - a1 - a2;
	a0 *= 2.0f; a1 *= 2.0f; a2 *= 2.0f;
	p[1] = p[0] + a0;
	p[2] = p[0] + a0 + a1; 
	p[3] = p[0] + a1;
	p[4] = p[0] + a2; p[5] = p[1] + a2; p[6] = p[2] + a2; p[7] = p[3] + a2;
}


// -----------------------------------------------------------------------------------
bool Geometry3D::triangleAABBIntersection(const m3Vector &a, const m3Vector &b, const m3Vector &c, 
										  const m3Bounds &bounds)
// -----------------------------------------------------------------------------------
{
	float boxcenter[3];
	boxcenter[0] = 0.5f * (bounds.min.x + bounds.max.x);
	boxcenter[1] = 0.5f * (bounds.min.y + bounds.max.y);
	boxcenter[2] = 0.5f * (bounds.min.z + bounds.max.z);
	float boxhalfsize[3];
	boxhalfsize[0] = 0.5f * (bounds.max.x - bounds.min.x);
	boxhalfsize[1] = 0.5f * (bounds.max.y - bounds.min.y);
	boxhalfsize[2] = 0.5f * (bounds.max.z - bounds.min.z);
	float triverts[3][3];
	triverts[0][0] = a.x; triverts[0][1] = a.y; triverts[0][2] = a.z;
	triverts[1][0] = b.x; triverts[1][1] = b.y; triverts[1][2] = b.z;
	triverts[2][0] = c.x; triverts[2][1] = c.y; triverts[2][2] = c.z;
	int res = triBoxOverlap(boxcenter, boxhalfsize, triverts);
	return (res != 0);
}

// -----------------------------------------------------------------------------------
bool Geometry3D::rayTriangleIntersection(const m3Vector &orig, const m3Vector &dir,
	 									 const m3Vector &a, const m3Vector &b, const m3Vector &c,
	 									 m3Real &t, m3Real &u, m3Real &v)
// -----------------------------------------------------------------------------------
{
	m3Vector edge1, edge2, tvec, pvec, qvec;
	m3Real det,inv_det;

	edge1 = b - a;
	edge2 = c - a;
	pvec.cross(dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = edge1.dot(pvec);

	if (det == 0.0f)
		return false;
	inv_det = 1.0f / det;

	/* calculate distance from vert0 to ray origin */
	tvec = orig - a;

	/* calculate U parameter and test bounds */
	u = tvec.dot(pvec) * inv_det;
	if (u < 0.0f || u > 1.0f)
	return false;

	/* prepare to test V parameter */
	qvec.cross(tvec, edge1);

	/* calculate V parameter and test bounds */
	v = dir.dot(qvec) * inv_det;
	if (v < 0.0f || u + v > 1.0f)
	return false;

	/* calculate t, ray intersects triangle */
	t = edge2.dot(qvec) * inv_det;

	return true;
}

// -----------------------------------------------------------------------------------
bool Geometry3D::rayPlaneIntersection(const m3Vector &orig, const m3Vector &dir, const m3Plane &plane,
										m3Real &t)
// -----------------------------------------------------------------------------------
{
	m3Real dirn = dir.dot(plane.normal);
	if (dirn == 0.0f) return false;
	t = (plane.d - orig.dot(plane.normal)) / dirn;
	return true;
}

// -----------------------------------------------------------------------------------
void Geometry3D::circumSphere(const m3Vector &p0, const m3Vector &p1, 
								  const m3Vector &p2, const m3Vector &p3, 
								  m3Vector &center, m3Real &radiusSquared)
// -----------------------------------------------------------------------------------
{
	m3Vector b = p1-p0;
	m3Vector c = p2-p0;
	m3Vector d = p3-p0;
	m3Real det = b.x*(c.y*d.z - c.z*d.y) - b.y*(c.x*d.z - c.z*d.x) + b.z*(c.x*d.y-c.y*d.x);
	if (det == 0.0f) {
		center = p0; radiusSquared = 0.0f; return; // singular case
	}
	det *= 2.0f;
	m3Vector v = c.cross(d)*b.dot(b) + d.cross(b)*c.dot(c) +  b.cross(c)*d.dot(d);
	v /= det;
	radiusSquared = v.magnitudeSquared();
	center = p0 + v;
}

// -----------------------------------------------------------------------------------
void Geometry3D::barycentricCoords(const m3Vector &p0, const m3Vector &p1,const m3Vector &p2, const m3Vector &p3,
								   const m3Vector &p, m3Vector &barycentricCoords)
// -----------------------------------------------------------------------------------
{
	m3Vector q  = p-p3;
	m3Vector q0 = p0-p3;
	m3Vector q1 = p1-p3;
	m3Vector q2 = p2-p3;
	m3Matrix m(q0,q1,q2);
	m3Real det = m.determinant();
	m.setColumn(0, q);
	barycentricCoords.x = m.determinant();
	m.setColumn(0, q0); m.setColumn(1,q);
	barycentricCoords.y = m.determinant();
	m.setColumn(1, q1); m.setColumn(2,q);
	barycentricCoords.z = m.determinant();
	if (det != 0.0f)
		barycentricCoords /= det;
}

// -----------------------------------------------------------------------------------
bool Geometry3D::pointInTetra(const m3Vector &p0, const m3Vector &p1,const m3Vector &p2, const m3Vector &p3,
								   const m3Vector &p)
// -----------------------------------------------------------------------------------
{
	m3Vector q  = p-p0;
	m3Vector q0 = p1-p0;
	m3Vector q1 = p2-p0;
	m3Vector q2 = p3-p0;
	m3Matrix m(q0,q1,q2);
	m3Real det = m.determinant();
	m3Real x,y,z;
	m.setColumn(0, q);
	x = m.determinant();
	m.setColumn(0, q0); m.setColumn(1,q);
	y = m.determinant();
	m.setColumn(1, q1); m.setColumn(2,q);
	z = m.determinant();
	if (det < 0.0f) { x = -x; y = -y; z = -z; det = -det; }
	if (x < 0.0f || y < 0.0f || z < 0.0f) return false;
	return (x + y + z < det);
}

// -------------------------------------------------------------------------
bool smallerQuadraticSolution(m3Real a, m3Real b, m3Real c, m3Real tMin, m3Real tMax, m3Real &t)
{
	if (m3Abs(a) < 1.0e-6f) {
		if (b == 0.0f) {
			if (c != 0.0f) return false;
			else t = 0.0f;
		}
		else 
			t = -c/b;
	}
	else {
		b *= 0.5f;
		m3Real d = b*b - a*c;
		if (d < 0.0f) return false;
		d = m3Sqrt(d);
		if (a < 0.0f) { a = -a; b = -b; }	// make a positive
		a = 1.0f / a;
		t = (-b-d)*a;	// this is the smaller solution -- only if a is positive!!
		if (t > tMax) return false;
		if (t < tMin) 
			t = (-b+d)*a;
	}
	return (tMin <= t && t <= tMax);
}

// -------------------------------------------------------------------------
bool Geometry3D::collisionMovingPointMovingTriangle(const m3Vector &p0, const m3Vector &p1,
				const m3Vector &a0, const m3Vector &a1,
				const m3Vector &b0, const m3Vector &b1,
				const m3Vector &c0, const m3Vector &c1,				
				m3Real &collisionTime)
{
	// at intersection: n dot (p - triangle.a) = 0
	// simplified version: linear interpolation of normal to avoid cubic equation
	m3Vector n0,nr;
	n0.cross(b0 - a0, c0 - a0);
	nr.cross(b1 - a1, c1 - a1); nr -= n0;
	m3Vector p  = p0 - a0;
	m3Vector pr = p1 - a1 - p;
	m3Real a = nr.dot(pr);
	m3Real b = nr.dot(p) + n0.dot(pr);
	m3Real c = n0.dot(p);

	m3Real t;
	if (!smallerQuadraticSolution(a,b,c, 0.0f, 1.0f, t))
		return false;

	// is the intersection point inside the triangle at time t?
	m3Real s = 1.0f - t;
	m3Vector pt = p0*s + p1*t;
	m3Vector at = a0*s + a1*t;
	m3Vector bt = b0*s + b1*t;
	m3Vector ct = c0*s + c1*t;
	m3Vector nt; nt.cross(bt-at,ct-at);
	m3Vector cr, da,db,dc;
	da = at-pt; db = bt-pt; dc = ct-pt;
	cr.cross(da,db); if (cr.dot(nt) < 0.0f) return false;
	cr.cross(db,dc); if (cr.dot(nt) < 0.0f) return false;
	cr.cross(dc,da); if (cr.dot(nt) < 0.0f) return false;
	collisionTime = t;
	return true;
}

// -------------------------------------------------------------------------
bool Geometry3D::collisionMovingSphereMovingTriangle(
			const m3Vector &p0, const m3Vector &p1, m3Real radius,
			const m3Vector &a0, const m3Vector &a1,
			const m3Vector &b0, const m3Vector &b1,
			const m3Vector &c0, const m3Vector &c1,				
			m3Real &collisionTime)
{
	// at intersection: n dot (p - triangle.a) = +/- radius, if n is normalized
	// simplified version: linear interpolation of normal to avoid cubic equation
	m3Vector n0,n1,nr;
	n0.cross(b0 - a0, c0 - a0); n0.normalize();
	n1.cross(b1 - a1, c1 - a1); n1.normalize();
	nr = n1 - n0;
	m3Vector p  = p0 - a0;
	m3Vector pr = p1 - a1 - p;
	m3Real a = nr.dot(pr);
	m3Real b = nr.dot(p) + n0.dot(pr);
	m3Real c = n0.dot(p);

	m3Real tNeg, tPos,t;

	bool hitNeg = smallerQuadraticSolution(a,b,c-radius, 0.0f, 1.0f, tNeg);
	bool hitPos = smallerQuadraticSolution(a,b,c+radius, 0.0f, 1.0f, tPos);

	if (hitNeg && hitPos) t = m3Min(tNeg, tPos);
	else if (hitNeg) t = tNeg;
	else if (hitPos) t = tPos;
	else return false;

	// is the intersection point inside the triangle at time t?
	m3Real s = 1.0f - t;
	m3Vector pt = p0*s + p1*t;
	m3Vector at = a0*s + a1*t;
	m3Vector bt = b0*s + b1*t;
	m3Vector ct = c0*s + c1*t;
	m3Vector nt; nt.cross(bt-at,ct-at);
	m3Vector cr, da,db,dc;
	da = at-pt; db = bt-pt; dc = ct-pt;
	cr.cross(da,db); if (cr.dot(nt) < 0.0f) return false;
	cr.cross(db,dc); if (cr.dot(nt) < 0.0f) return false;
	cr.cross(dc,da); if (cr.dot(nt) < 0.0f) return false;
	collisionTime = t;
	return true;
}
