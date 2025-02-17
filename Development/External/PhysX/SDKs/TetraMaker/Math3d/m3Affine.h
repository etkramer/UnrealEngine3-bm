#ifndef M3_AFFINE_H
#define M3_AFFINE_H

//---------------------------------------------------------------------------

#include "math3d.h"

//---------------------------------------------------------------------------
class m3Affine
//---------------------------------------------------------------------------
{	
public:
	m3Affine() { id(); }
	m3Affine(const m3Vector &pos0, const m3Matrix &mat0) { pos = pos0; mat = mat0; }
	void set(const m3Vector &pos0, const m3Matrix &mat0) { pos = pos0; mat = mat0; }

	void id() { pos.zero(); mat.id(); }

	void orthoNormalize() {
		mat.gramSchmidt();
	}

	m3Vector operator *(const m3Vector &v) const {
		return mat * v + pos;
	}
	m3Affine operator *(const m3Affine &a) const {
		m3Affine res(mat * a.pos + pos, mat * a.mat);
		return res;
	}
	m3Affine operator ~() const {			// general inverse
		m3Matrix inv = mat; inv.invert();
		m3Affine res(-(inv*pos), inv);
		return res;
	}
	m3Affine operator !() const {			// fast inverse valid for orthogonal mat
		m3Matrix inv = mat; inv.transpose();
		m3Affine res(-(inv*pos), inv);
		return res;
	}

	// ------------------------------------------------
	m3Vector pos;
	m3Matrix mat;
};

#endif
