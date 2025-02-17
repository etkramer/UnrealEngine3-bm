#ifndef M3_VECTOR_H
#define M3_VECTOR_H

//---------------------------------------------------------------------------

#include "math3d.h"
#include <assert.h>

//---------------------------------------------------------------------------
class m3Vector
//---------------------------------------------------------------------------
{	
public:
	inline m3Vector() {}
	inline m3Vector(m3Real x0, m3Real y0, m3Real z0) { x = x0; y = y0; z = z0; }

	inline void set(m3Real x0, m3Real y0, m3Real z0) { x = x0; y = y0; z = z0; }
	inline void zero() { x = 0.0; y = 0.0; z = 0.0; }
	inline bool isZero() { return x == 0.0 && y == 0.0 && z == 0.0; }

	m3Real & operator[] (int i) {
		assert(i >= 0 && i <= 2);
		return (&x)[i];
	}
	bool operator == (const m3Vector &v) const {
		return (x == v.x) && (y == v.y) && (z == v.z);
	}
	m3Vector operator + (const m3Vector &v) const {
		m3Vector r; r.x = x + v.x; r.y = y + v.y; r.z = z + v.z;
		return r;
	}
	m3Vector operator - (const m3Vector &v) const {
		m3Vector r; r.x = x - v.x; r.y = y - v.y; r.z = z - v.z;
		return r;
	}
	void operator += (const m3Vector &v) {
		x += v.x; y += v.y; z += v.z;
	}
	void operator -= (const m3Vector &v) {
		x -= v.x; y -= v.y; z -= v.z;
	}
	void operator *= (const m3Vector &v) {
		x *= v.x; y *= v.y; z *= v.z;
	}
	void operator /= (const m3Vector &v) {
		x /= v.x; y /= v.y; z /= v.z;
	}
	m3Vector operator -() const {
		m3Vector r; r.x = -x; r.y = -y; r.z = -z;
		return r;
	}
	m3Vector operator * (const m3Real f) const {
		m3Vector r; r.x = x*f; r.y = y*f; r.z = z*f;
		return r;
	}
	m3Vector operator / (const m3Real f) const {
		m3Vector r; r.x = x/f; r.y = y/f; r.z = z/f;
		return r;
	}
	inline void cross(const m3Vector &v1, const m3Vector &v2) {
		x = v1.y*v2.z-v2.y*v1.z;
		y = v1.z*v2.x-v2.z*v1.x;
		z = v1.x*v2.y-v2.x*v1.y;
	}
	m3Vector cross(const m3Vector &v) const {
		m3Vector r;
		r.x = y*v.z-v.y*z;
		r.y = z*v.x-v.z*x;
		r.z = x*v.y-v.x*y;
		return r;
	}
	m3Vector operator ^ (const m3Vector &v) {
		return cross(v);
	}

	inline m3Real dot(const m3Vector &v) const {
		return x*v.x + y*v.y + z*v.z;
	}
	m3Real operator | (const m3Vector &v) {
		return dot(v);
	}

	inline void minimum(const m3Vector &v) {
		if (v.x < x) x = v.x;
		if (v.y < y) y = v.y;
		if (v.z < z) z = v.z;
	}
	inline void maximum(const m3Vector &v) {
		if (v.x > x) x = v.x;
		if (v.y > y) y = v.y;
		if (v.z > z) z = v.z;
	}

	inline m3Real magnitudeSquared() const { return x*x + y*y + z*z; }
	inline m3Real magnitude() const { return sqrt(x*x + y*y + z*z); }
	inline m3Real distanceSquared(const m3Vector &v) const {
		m3Real dx,dy,dz; dx = v.x - x; dy = v.y - y; dz = v.z - z;
		return dx*dx + dy*dy + dz*dz;
	}
	inline m3Real distance(const m3Vector &v) const {
		m3Real dx,dy,dz; dx = v.x - x; dy = v.y - y; dz = v.z - z;
		return sqrt(dx*dx + dy*dy + dz*dz);
	}
	void operator *=(m3Real f) { x *=f; y *=f; z *= f; }
	void operator /=(m3Real f) { x /=f; y /=f; z /= f; }
	m3Real normalize() {
		m3Real l = magnitude();
		if (l == 0.0f) { x = 1.0f; y = 0.0f; z = 0.0f;  }
		else { m3Real l1 = 1.0f/l; x *= l1; y *= l1; z *= l1; }
		return l;
	}
	m3Vector newDirection() {
		if (fabs(x) < fabs(y) && fabs(x) < fabs(z)) return m3Vector(x,-z,y);
		else if (fabs(y) < fabs(z)) return m3Vector(z,y,-x);
		else return m3Vector(-y,x,z);
	}
	void disturb() {
		x = m3Disturb(x);
		y = m3Disturb(y);
		z = m3Disturb(z);
	}
	void disturb(m3Real fraction)
	{
		m3Real d;
		d = m3Abs(x * fraction); x += m3Random(-d, d);
		d = m3Abs(y * fraction); y += m3Random(-d, d);
		d = m3Abs(z * fraction); z += m3Random(-d, d);
	}


// ------------------------------
	m3Real x,y,z;
};


#endif
