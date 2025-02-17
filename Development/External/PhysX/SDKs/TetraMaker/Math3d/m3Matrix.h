#ifndef M3_MATRIX_H
#define M3_MATRIX_H
//---------------------------------------------------------------------------

#include "math3d.h"

//---------------------------------------------------------------------------
class m3Matrix {
//---------------------------------------------------------------------------
public:
	m3Matrix() {}
	m3Matrix(const m3Matrix &m) { *this = m; }
	m3Matrix(const m3Vector &col0, const m3Vector &col1, const m3Vector &col2) {
		r00 = col0.x; r01 = col1.x; r02 = col2.x;
		r10 = col0.y; r11 = col1.y; r12 = col2.y;
		r20 = col0.z; r21 = col1.z; r22 = col2.z;
	}
	m3Matrix(m3Real c00, m3Real c01, m3Real c02, m3Real c10, m3Real c11, m3Real c12, m3Real c20, m3Real c21, m3Real c22) {
		r00 = c00; r01 = c01; r02 = c02;
		r10 = c10; r11 = c11; r12 = c12;
		r20 = c20; r21 = c21; r22 = c22;
	}
	m3Real & m3Matrix::operator()(int i, int j) {
		assert(i >= 0 && i <= 2);
		assert(j >= 0 && i <= 2);
		return (&r00)[i*3+j];
	}
	const m3Real & m3Matrix::operator()(int i, int j) const {
		assert(i >= 0 && i <= 2);
		assert(j >= 0 && i <= 2);
		return (&r00)[i*3+j];
	}
	void zero() { 
		r00 = 0.0; r01 = 0.0; r02 = 0.0;
		r10 = 0.0; r11 = 0.0; r12 = 0.0;
		r20 = 0.0; r21 = 0.0; r22 = 0.0;
	}
	void id() {
		r00 = 1.0; r01 = 0.0; r02 = 0.0;
		r10 = 0.0; r11 = 1.0; r12 = 0.0;
		r20 = 0.0; r21 = 0.0; r22 = 1.0;
	}
	void cross(const m3Vector &v) {
		r00 = 0.0; r01 =-v.z; r02 = v.y;
		r10 = v.z; r11 = 0.0; r12 =-v.x;
		r20 =-v.y; r21 = v.x; r22 = 0.0;
	}
	void setColumns(const m3Vector &col0, const m3Vector &col1, const m3Vector &col2) {
		r00 = col0.x; r01 = col1.x; r02 = col2.x;
		r10 = col0.y; r11 = col1.y; r12 = col2.y;
		r20 = col0.z; r21 = col1.z; r22 = col2.z;
	}
	void setColumn(int i, const m3Vector &col) {
		if (i == 0) { r00 = col.x; r10 = col.y; r20 = col.z; }
		if (i == 1) { r01 = col.x; r11 = col.y; r21 = col.z; }
		if (i == 2) { r02 = col.x; r12 = col.y; r22 = col.z; }
	}
	void setRows(const m3Vector &row0, const m3Vector &row1, const m3Vector &row2) {
		r00 = row0.x; r01 = row0.y; r02 = row0.z;
		r10 = row1.x; r11 = row1.y; r12 = row1.z;
		r20 = row2.x; r21 = row2.y; r22 = row2.z;
	}
	void setRow(int i, const m3Vector &row) {
		if (i == 0) { r00 = row.x; r01 = row.y; r02 = row.z; }
		if (i == 1) { r10 = row.x; r11 = row.y; r12 = row.z; }
		if (i == 2) { r20 = row.x; r21 = row.y; r22 = row.z; }
	}
	void getColumns(m3Vector &col0, m3Vector &col1, m3Vector &col2) const {
		col0.x = r00; col1.x = r01; col2.x = r02;
		col0.y = r10; col1.y = r11; col2.y = r12;
		col0.z = r20; col1.z = r21; col2.z = r22;
	}
	void getColumn(int i, m3Vector &col) const {
		if (i == 0) { col.x = r00; col.y = r10; col.z = r20; }
		if (i == 1) { col.x = r01; col.y = r11; col.z = r21; }
		if (i == 2) { col.x = r02; col.y = r12; col.z = r22; }
	}
	m3Vector getColumn(int i) const {
		m3Vector r; getColumn(i, r);
		return r;
	}
	void getRows(m3Vector &row0, m3Vector &row1, m3Vector &row2) const {
		row0.x = r00; row0.y = r01; row0.z = r02;
		row1.x = r10; row1.y = r11; row1.z = r12;
		row2.x = r20; row2.y = r21; row2.z = r22;
	}
	void getRow(int i, m3Vector &row) const {
		if (i == 0) { row.x = r00; row.y = r01; row.z = r02; }
		if (i == 1) { row.x = r10; row.y = r11; row.z = r12; }
		if (i == 2) { row.x = r20; row.y = r21; row.z = r22; }
	}
	m3Vector getRow(int i) const {
		m3Vector r; getRow(i, r);
		return r;
	}
	bool operator == (const m3Matrix &m) const {
		return 
			(r00 == m.r00) && (r01 == m.r01) && (r02 == m.r02) &&
			(r10 == m.r10) && (r11 == m.r11) && (r12 == m.r12) &&
			(r20 == m.r20) && (r21 == m.r21) && (r22 == m.r22);
	}
	m3Matrix operator + (const m3Matrix &m) const {
		m3Matrix res; 
		res.r00 = r00 + m.r00; res.r01 = r01 + m.r01; res.r02 = r02 + m.r02;
		res.r10 = r10 + m.r10; res.r11 = r11 + m.r11; res.r12 = r12 + m.r12;
		res.r20 = r20 + m.r20; res.r21 = r21 + m.r21; res.r22 = r22 + m.r22;
		return res;
	}
	m3Matrix operator - (const m3Matrix &m) const {
		m3Matrix res; 
		res.r00 = r00 - m.r00; res.r01 = r01 - m.r01; res.r02 = r02 - m.r02;
		res.r10 = r10 - m.r10; res.r11 = r11 - m.r11; res.r12 = r12 - m.r12;
		res.r20 = r20 - m.r20; res.r21 = r21 - m.r21; res.r22 = r22 - m.r22;
		return res;
	}
	void operator += (const m3Matrix &m) {
		r00 += m.r00; r01 += m.r01; r02 += m.r02;
		r10 += m.r10; r11 += m.r11; r12 += m.r12;
		r20 += m.r20; r21 += m.r21; r22 += m.r22;
	}
	void operator -= (const m3Matrix &m) {
		r00 -= m.r00; r01 -= m.r01; r02 -= m.r02;
		r10 -= m.r10; r11 -= m.r11; r12 -= m.r12;
		r20 -= m.r20; r21 -= m.r21; r22 -= m.r22;
	}
	m3Matrix operator * (const m3Real f) const {
		m3Matrix res;
		res.r00 = r00*f; res.r01 = r01*f; res.r02 = r02*f;
		res.r10 = r10*f; res.r11 = r11*f; res.r12 = r12*f;
		res.r20 = r20*f; res.r21 = r21*f; res.r22 = r22*f;
		return res;
	}
	m3Matrix operator / (const m3Real f) const {
		m3Matrix res;
		res.r00 = r00*f; res.r01 = r01*f; res.r02 = r02*f;
		res.r10 = r10*f; res.r11 = r11*f; res.r12 = r12*f;
		res.r20 = r20*f; res.r21 = r21*f; res.r22 = r22*f;
		return res;
	}
	void operator *=(m3Real f) { 
		r00 *= f; r01 *= f; r02 *= f;
		r10 *= f; r11 *= f; r12 *= f;
		r20 *= f; r21 *= f; r22 *= f;
	}
	void operator /=(m3Real f) { 
		r00 /= f; r01 /= f; r02 /= f;
		r10 /= f; r11 /= f; r12 /= f;
		r20 /= f; r21 /= f; r22 /= f;
	}
	m3Vector operator * (const m3Vector &v) const {
		return multiply(v);
	}
	m3Vector multiply(const m3Vector &v) const {
		m3Vector res;
		res.x = r00 * v.x + r01 * v.y + r02 * v.z;
		res.y = r10 * v.x + r11 * v.y + r12 * v.z;
		res.z = r20 * v.x + r21 * v.y + r22 * v.z;
		return res;
	}
	m3Vector multiplyTransposed(const m3Vector &v) const {
		m3Vector res;
		res.x = r00 * v.x + r10 * v.y + r20 * v.z;
		res.y = r01 * v.x + r11 * v.y + r21 * v.z;
		res.z = r02 * v.x + r12 * v.y + r22 * v.z;
		return res;
	}
	void multiply(const m3Matrix &left, const m3Matrix &right) {
		m3Matrix res;
		res.r00 = left.r00*right.r00 + left.r01*right.r10 + left.r02*right.r20;
		res.r01 = left.r00*right.r01 + left.r01*right.r11 + left.r02*right.r21;
		res.r02 = left.r00*right.r02 + left.r01*right.r12 + left.r02*right.r22;
		res.r10 = left.r10*right.r00 + left.r11*right.r10 + left.r12*right.r20;
		res.r11 = left.r10*right.r01 + left.r11*right.r11 + left.r12*right.r21;
		res.r12 = left.r10*right.r02 + left.r11*right.r12 + left.r12*right.r22;
		res.r20 = left.r20*right.r00 + left.r21*right.r10 + left.r22*right.r20;
		res.r21 = left.r20*right.r01 + left.r21*right.r11 + left.r22*right.r21;
		res.r22 = left.r20*right.r02 + left.r21*right.r12 + left.r22*right.r22;
		*this = res;
	}
	void operator *=(const m3Matrix m) { 
		multiply(*this, m);
	}
	void multiplyTransposedLeft(const m3Matrix &left, const m3Matrix &right) {
		m3Matrix res;
		res.r00 = left.r00*right.r00 + left.r10*right.r10 + left.r20*right.r20;
		res.r01 = left.r00*right.r01 + left.r10*right.r11 + left.r20*right.r21;
		res.r02 = left.r00*right.r02 + left.r10*right.r12 + left.r20*right.r22;
		res.r10 = left.r01*right.r00 + left.r11*right.r10 + left.r21*right.r20;
		res.r11 = left.r01*right.r01 + left.r11*right.r11 + left.r21*right.r21;
		res.r12 = left.r01*right.r02 + left.r11*right.r12 + left.r21*right.r22;
		res.r20 = left.r02*right.r00 + left.r12*right.r10 + left.r22*right.r20;
		res.r21 = left.r02*right.r01 + left.r12*right.r11 + left.r22*right.r21;
		res.r22 = left.r02*right.r02 + left.r12*right.r12 + left.r22*right.r22;
		*this = res;
	}	
	void multiplyTransposedRight(const m3Matrix &left, const m3Matrix &right) {
		m3Matrix res;
		res.r00 = left.r00*right.r00 + left.r01*right.r01 + left.r02*right.r02;
		res.r01 = left.r00*right.r10 + left.r01*right.r11 + left.r02*right.r12;
		res.r02 = left.r00*right.r20 + left.r01*right.r21 + left.r02*right.r22;
		res.r10 = left.r10*right.r00 + left.r11*right.r01 + left.r12*right.r02;
		res.r11 = left.r10*right.r10 + left.r11*right.r11 + left.r12*right.r12;
		res.r12 = left.r10*right.r20 + left.r11*right.r21 + left.r12*right.r22;
		res.r20 = left.r20*right.r00 + left.r21*right.r01 + left.r22*right.r02;
		res.r21 = left.r20*right.r10 + left.r21*right.r11 + left.r22*right.r12;
		res.r22 = left.r20*right.r20 + left.r21*right.r21 + left.r22*right.r22;
		*this = res;
	}
	m3Matrix operator * (const m3Matrix &m) const {
		m3Matrix res;
		res.multiply(*this, m);
		return res;
	}
	bool invert() {
		m3Real d, b00,b01,b02,b10,b11,b12,b20,b21,b22;

		b00 = r11*r22 - r12*r21; b01 = r02*r21 - r01*r22; b02 = r01*r12 - r02*r11;
		b10 = r12*r20 - r10*r22; b11 = r00*r22 - r02*r20; b12 = r02*r10 - r00*r12;
		b20 = r10*r21 - r11*r20; b21 = r01*r20 - r00*r21; b22 = r00*r11 - r01*r10;

		d = b00*r00	+ b01*r10 + b02*r20;	
		if (d == 0.0)
			return false;
		d = (m3Real)1.0/d;

		r00 = b00*d; r01 = b01*d; r02 = b02*d;
		r10 = b10*d; r11 = b11*d; r12 = b12*d;
		r20 = b20*d; r21 = b21*d; r22 = b22*d;
		return true;
	}
	bool setInverse(m3Matrix &m) {
		m = *this;
		return m.invert();
	}
	void transpose() {
		m3Real r;
		r = r01; r01 = r10; r10 = r;
		r = r02; r02 = r20; r20 = r;
		r = r12; r12 = r21; r21 = r;
	}
	void setTransposed(m3Matrix &m) {
		m = *this;
		m.transpose();
	}
	m3Real magnitudeSquared() { 
		return r00*r00+r01*r01+r02*r02 + r10*r10+r11*r11+r12*r12 + r20*r20+r21*r21+r22*r22; 
	}
	m3Real magnitude() { return sqrt(magnitudeSquared()); }

	void gramSchmidt() {
		m3Vector c0,c1,c2;
		getColumn(0,c0); c0.normalize();
		getColumn(1,c1); c1 -= c0*c1.dot(c0); c1.normalize();
		c2.cross(c0, c1);
		setColumns(c0, c1, c2);
	}

	void rotX(m3Real angle)
	{
		m3Real cosAngle = cos(angle);
		m3Real sinAngle = sin(angle);
		id();
		r11 = r22 = cosAngle;
		r12 = -sinAngle;
		r21 = sinAngle;
	}
	void rotY(m3Real angle)
	{
		m3Real cosAngle = cos(angle);
		m3Real sinAngle = sin(angle);
		id();
		r00 = r22 = cosAngle;
		r02 = sinAngle;
		r20 = -sinAngle;
	}
	void rotZ(m3Real angle)
	{
		m3Real cosAngle = cos(angle);
		m3Real sinAngle = sin(angle);
		id();
		r00 = r11 = cosAngle;
		r01 = -sinAngle;
		r10 = sinAngle;
	}
	void rot(const m3Vector &n, m3Real angle)
	{
		// Rodrigues formula: axis n needs to be normalized!
		m3Real cosa = cos(angle);
		m3Real cosa1 = 1.0f - cosa;
		m3Real sina = sin(angle);
		r00 = cosa + n.x*n.x*cosa1;
		r11 = cosa + n.y*n.y*cosa1;
		r22 = cosa + n.z*n.z*cosa1;
		r01 = r10 = n.x*n.y*cosa1;
		r02 = r20 = n.x*n.z*cosa1;
		r12 = r21 = n.y*n.z*cosa1;
		r01 -= n.z*sina; r10 += n.z*sina;
		r02 += n.y*sina; r20 -= n.y*sina;
		r12 -= n.x*sina; r21 += n.x*sina;
	}
	void getAxisAndAngle(m3Vector &n, m3Real &angle)
	{
	m3Real tr, s, w;
    tr = r00 + r11 + r22;
    if (tr >= 0.0f) {
		w = 0.5f * m3Sqrt(tr + 1.0f);
		n.x = r21 - r12;
		n.y = r02 - r20;
		n.z = r10 - r01;
	}
    else {
		int i = 0; 
		if (r11 > r00)
			i = 1; 
		if(r22 > r11 && r22 > r00)
			i = 2; 
		w = 0.0f;
		switch (i)
			{
			case 0:
				s = m3Sqrt(r00 - r11 - r22 + 1.0f);
				n.x = 0.5f * s;
				s = 0.5f / s;
				n.y = (r01 + r10) * s; 
				n.z = (r20 + r02) * s;
				w   = (r21 - r12) * s;
				break;
			case 1:
				s = m3Sqrt(r11 - r22 - r00 + 1.0f);
				n.y = 0.5f * s;
				s = 0.5f / s;
				n.z = (r12 + r21) * s;
				n.x = (r01 + r10) * s;
				w   = (r02 - r20) * s;
				break;
			case 2:
				s = m3Sqrt(r22 - r00 - r11 + 1.0f);
				n.z = 0.5f * s;
				s = 0.5f / s;
				n.x = (r20 + r02) * s;
				n.y = (r12 + r21) * s;
				w   = (r10 - r01) * s;
			}
		}
		angle = 2.0f * m3Acos(w);
		n.normalize();
	}
	void rot(const m3Vector &r1, const m3Vector &r2)
	{
		// rotates direction r1 into direction r2
		id();
		m3Vector n; n.cross(r1,r2);
		if (n.isZero()) return;
		n.normalize();
		m3Real l = r1.magnitude() * r2.magnitude();
		if (l == 0.0f) return;
		m3Real cosf = r1.dot(r2) / l;
		rot(n, m3Acos(cosf));
	}
	m3Real determinant()
	{
		return r00*(r11*r22-r12*r21)-r01*(r10*r22-r12*r20)+r02*(r10*r21-r11*r20);
	}
	void frame(const m3Vector &axis0) {
		m3Vector a0 = axis0; a0.normalize();
		m3Vector a2 = a0.newDirection();
		m3Vector a1 = a0 ^ a2; a1.normalize();
		a2 = a0 ^ a1; a2.normalize();
		setColumn(0, a0);
		setColumn(1, a1);
		setColumn(2, a2);
	}


// --------------------------------------------------
	m3Real r00,r01,r02;
	m3Real r10,r11,r12;
	m3Real r20,r21,r22;

};


#endif
