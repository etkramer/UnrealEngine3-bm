#ifndef M3_BOUNDS_H
#define M3_BOUNDS_H

//---------------------------------------------------------------------------

#include "math3d.h"

//---------------------------------------------------------------------------
class m3Bounds
//---------------------------------------------------------------------------
{	
public:
	inline m3Bounds() { setEmpty(); }
	inline m3Bounds(const m3Vector &min0, const m3Vector &max0) { min = min0; max = max0; }

	inline void set(const m3Vector &min0, const m3Vector &max0) { min = min0; max = max0; }
	inline void setEmpty() {
		set(m3Vector(m3RealMax, m3RealMax, m3RealMax),
			m3Vector(m3RealMin, m3RealMin, m3RealMin));
	}
	inline void setInfinite() {
		set(m3Vector(m3RealMin, m3RealMin, m3RealMin),
			m3Vector(m3RealMax, m3RealMax, m3RealMax));
	}
	inline bool isEmpty() const { 
		if (min.x > max.x) return true;
		if (min.y > max.y) return true;
		if (min.z > max.z) return true;
		return false;
	}

	bool operator == (const m3Bounds &b) const {
		return (min == b.min) && (max == b.max);
	}
	void combine(const m3Bounds &b) {
		min.minimum(b.min);
		max.maximum(b.max);
	}
	void operator += (const m3Bounds &b) {
		combine(b);
	}
	m3Bounds operator + (const m3Bounds &b) const {
		m3Bounds r = *this;
		r.combine(b);
		return r;
	}
	bool intersects(const m3Bounds &b) const {
		if ((b.min.x > max.x) || (min.x > b.max.x)) return false;
		if ((b.min.y > max.y) || (min.y > b.max.y)) return false;
		if ((b.min.z > max.z) || (min.z > b.max.z)) return false;
		return true;
	}
	void intersect(const m3Bounds &b) {
		min.maximum(b.min);
		max.minimum(b.max);
	}
	void include(const m3Vector &v) {
		max.maximum(v);
		min.minimum(v);
	}
	bool contain(const m3Vector &v) const {
		return 
			min.x <= v.x && v.x <= max.x && 
			min.y <= v.y && v.y <= max.y && 
			min.z <= v.z && v.z <= max.z;
	}
	void operator += (const m3Vector &v) {
		include(v);
	}
	void getCenter(m3Vector &v) const {
		v = min + max; v *= 0.5f;
	}
	void clamp(m3Vector &pos) const {
		if (isEmpty()) return;
		pos.maximum(min);
		pos.minimum(max);
	}
	void clamp(m3Vector &pos, m3Real offset) const {
		if (isEmpty()) return;
		if (pos.x < min.x + offset) pos.x = min.x + offset;
		if (pos.x > max.x - offset) pos.x = max.x - offset;
		if (pos.y < min.y + offset) pos.y = min.y + offset;
		if (pos.y > max.y - offset) pos.y = max.y - offset;
		if (pos.z < min.z + offset) pos.z = min.z + offset;
		if (pos.z > max.z - offset) pos.z = max.z - offset;
	}
	void fatten(m3Real d) {
		if (isEmpty()) return;
		min.x -= d; min.y -= d; min.z -= d;
		max.x += d; max.y += d; max.z += d;
	}
	void transform(const m3Affine &a) {
		m3Real dx = 0.5f * (max.x - min.x);
		m3Real dy = 0.5f * (max.y - min.y);
		m3Real dz = 0.5f * (max.z - min.z);
		m3Vector d;
		d.x = m3Abs(a.mat.r00 * dx) + m3Abs(a.mat.r01 * dy) + m3Abs(a.mat.r02 * dz);
		d.y = m3Abs(a.mat.r10 * dx) + m3Abs(a.mat.r11 * dy) + m3Abs(a.mat.r12 * dz);
		d.z = m3Abs(a.mat.r20 * dx) + m3Abs(a.mat.r21 * dy) + m3Abs(a.mat.r22 * dz);
		m3Vector center;
		getCenter(center);
		center += a.pos;
		min = center - d;
		max = center + d;
	}
//--------------------------------------------
	m3Vector min, max;
};


#endif
