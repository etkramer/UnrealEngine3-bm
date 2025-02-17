#ifndef QUADRIC_SIMPLIFICATOR_H
#define QUADRIC_SIMPLIFICATOR_H

#include "math3d.h"
#include <vector>
class Mesh;

// ---------------------------------------------------------------------------------------
class Quadric {
public:
	void zero() {
		a00 = 0.0f; a01 = 0.0f; a02 = 0.0f; a03 = 0.0f;
		a11 = 0.0f; a12 = 0.0f; a13 = 0.0f;
		a22 = 0.0f; a23 = 0.0f;
		a33 = 0.0f;
	}

	// generate quadric from plane
	void setFromPlane(const m3Vector &v0, const m3Vector &v1, const m3Vector &v2) {
		m3Vector n; n.cross(v1-v0, v2-v0); n.normalize();
		float d = -n.dot(v0);
		a00 = n.x*n.x; a01 = n.x*n.y; a02 = n.x*n.z; a03 = n.x*d;
		a11 = n.y*n.y; a12 = n.y*n.z; a13 = n.y*d;
		a22 = n.z*n.z; a23 = n.z*d;
		a33 = d*d;
	}

	Quadric operator +(const Quadric & q) const
	{
		Quadric sum;
		sum.a00 = a00 + q.a00; sum.a01 = a01 + q.a01; sum.a02 = a02 + q.a02; sum.a03 = a03 + q.a03;
		sum.a11 = a11 + q.a11; sum.a12 = a12 + q.a12; sum.a13 = a13 + q.a13;
		sum.a22 = a22 + q.a22; sum.a23 = a23 + q.a23;
		sum.a33 = a33 + q.a33;
		return sum;
	}

	void operator +=(const Quadric &q)
	{
		a00 += q.a00; a01 += q.a01; a02 += q.a02; a03 += q.a03;
		a11 += q.a11; a12 += q.a12; a13 += q.a13;
		a22 += q.a22; a23 += q.a23;
		a33 += q.a33;
	}

	float outerProduct(const m3Vector &v)
	{
		return a00*v.x*v.x + 2.0f*a01*v.x*v.y + 2.0f*a02*v.x*v.z + 2.0f*a03*v.x +
			   a11*v.y*v.y + 2.0f*a12*v.y*v.z + 2.0f*a13*v.y +
			   a22*v.z*v.z + 2.0f*a23*v.z + a33;
	}
private:
	float a00, a01, a02, a03;
	float      a11, a12, a13;
	float           a22, a23;
	float                a33;

};

// ---------------------------------------------------------------------------------------
struct QuadricVertex {
public:
	QuadricVertex(const m3Vector &newPos) {
		pos = newPos;
		q.zero();
		deleted = false;
		border = false;
	}
	void removeEdge(int edgeNr);
	void addTriangle(int triangleNr);
	m3Vector pos;
	Quadric  q;
	bool deleted;
	bool border;
	std::vector<int> edges;
	std::vector<int> triangles;
};

// ---------------------------------------------------------------------------------------
class QuadricEdge {
public:
	void init(int v0, int v1) {
		if (v0 < v1) { vertexNr[0] = v0; vertexNr[1] = v1; }
		else { vertexNr[0] = v1; vertexNr[1] = v0; }
		cost = -1.0f;
		ratio = -1.0f;
		heapPos = -1;
		border = false;
	}
	bool operator < (QuadricEdge &e) const {
		if (vertexNr[0] < e.vertexNr[0]) return true;
		if (vertexNr[0] > e.vertexNr[0]) return false;
		return vertexNr[1] < e.vertexNr[1];		
	}
	bool operator == (QuadricEdge &e) const {
		return vertexNr[0] == e.vertexNr[0] && vertexNr[1] == e.vertexNr[1];
	}
	int otherVertex(int vNr) const {
		if (vertexNr[0] == vNr) return vertexNr[1];
		else return vertexNr[0];
	}
	void replaceVertex(int vOld, int vNew) {
		if (vertexNr[0] == vOld) vertexNr[0] = vNew;
		else if (vertexNr[1] == vOld) vertexNr[1] = vNew;
		else assert(0);
		if (vertexNr[0] > vertexNr[1]) {
			int v = vertexNr[0]; vertexNr[0] = vertexNr[1]; vertexNr[1] = v;
		}
	}
	int vertexNr[2];
	m3Real cost;
	m3Real ratio;
	int heapPos;
	bool border;
};

// ---------------------------------------------------------------------------------------
class QuadricTriangle {
public:
	void init(int v0, int v1, int v2) {
		vertexNr[0] = v0; vertexNr[1] = v1; vertexNr[2] = v2;
		deleted = false;
	}
	bool containsVertex(int vNr) const {
		return vertexNr[0] == vNr || vertexNr[1] == vNr || vertexNr[2] == vNr;
	}
	void replaceVertex(int vOld, int vNew) {
		if (vertexNr[0] == vOld) vertexNr[0] = vNew;
		else if (vertexNr[1] == vOld) vertexNr[1] = vNew;
		else if (vertexNr[2] == vOld) vertexNr[2] = vNew;
		else assert(0);
	}
	bool operator == (QuadricTriangle &t) const {
		return t.containsVertex(vertexNr[0]) && 
			t.containsVertex(vertexNr[1]) &&
			t.containsVertex(vertexNr[2]);
	}
	int vertexNr[3];
	bool deleted;
};

// ---------------------------------------------------------------------------------------
struct QuadricVertexRef {
	void init(const m3Vector &p, int vNr) {
		pos = p; vertexNr = vNr;
	}
	bool operator < (const QuadricVertexRef &vr) {
		return pos.x < vr.pos.x;
	}
	m3Vector pos;
	int vertexNr;
};

// ---------------------------------------------------------------------------------------
class QuadricSimplificator {
public:
	~QuadricSimplificator();
	void clear();

	// registration
	void registerVertex(const m3Vector &pos);
	void registerTriangle(int v0, int v1, int v2);
	void endRegistration(bool mergeCloseVertices = false);

	// manipulation
	bool simplificationStep(int subdivision = 0);

	// accessors
	int  getNumVertices() const { return mVertices.size(); }
	bool isDeletedVertex(int vertexNr) const {
		assert(vertexNr < (int)mVertices.size());
		return mVertices[vertexNr]->deleted;
	}
	m3Vector getVertexPosition(int vertexNr) const {
		assert(vertexNr < (int)mVertices.size());
		return mVertices[vertexNr]->pos;
	}
	int  getTriangleNr(int v0, int v1, int v2) const;
	int  getNumTriangles() const { return mTriangles.size(); }
	bool getTriangle(int i, int &v0, int &v1, int &v2) const;
	int  getNumActiveVertices() const;

private:
	void computeCost(QuadricEdge &edge);
	bool legalCollapse(QuadricEdge &edge, float maxLength);
	void collapseEdge(QuadricEdge &edge);
	void quickSortEdges(int l, int r);
	void quickSortVertexRefs(int l, int r);
	void mergeVertices();

	void heapUpdate(int i);
	void heapSift(int i);
	void heapRemove(int i);
	void testHeap();

	m3Bounds mBounds;

	std::vector<QuadricVertex *>  mVertices;
	std::vector<QuadricEdge>      mEdges;
	std::vector<QuadricTriangle>  mTriangles;
	std::vector<QuadricEdge *>    mHeap;
	std::vector<QuadricVertexRef> mVertexRefs;
};


#endif
