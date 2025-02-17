#ifndef M3_PLANE_H
#define M3_PLANE_H

//---------------------------------------------------------------------------

#include "math3d.h"

//---------------------------------------------------------------------------
class m3Plane
//---------------------------------------------------------------------------
{	
public:
	m3Plane() {}
	m3Plane(const m3Vector &normal, m3Real d) : normal(normal), d(d) { this->normal.normalize(); }
	m3Plane(const m3Vector &a, const m3Vector &b, const m3Vector &c) { 
		set(a,b,c); 
	}
	void set(const m3Vector &a, const m3Vector &b, const m3Vector &c) {
		normal.cross(b-a, c-a);
		normal.normalize();
		d = a.dot(normal);
	}
	m3Real distance(const m3Vector &pos) const {
		return normal.dot(pos) - d;
	}
	m3Vector project(const m3Vector &pos) const {
		return pos - normal*distance(pos);
	}


// ------------------------------
	m3Vector normal;
	m3Real d;
};


#endif
