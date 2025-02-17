#ifndef SUBDIVIDER_H
#define SUBDIVIDER_H

#include "math3d.h"
#include <vector>
class Mesh;

// ---------------------------------------------------------------------------------------
struct SubdividerVertex {
	SubdividerVertex(const m3Vector &newPos) {
		pos = newPos;
		marked = false;
	}
	void addTriangle(int triangleNr);
	void removeTriangle(int triangleNr);

	bool marked;
	m3Vector pos;
	std::vector<int> triangles;
};

// ---------------------------------------------------------------------------------------
struct SubdividerTriangle {
	void init(int v0, int v1, int v2) {
		vertexNr[0] = v0; vertexNr[1] = v1; vertexNr[2] = v2;
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
	bool operator == (SubdividerTriangle &t) const {
		return t.containsVertex(vertexNr[0]) && 
			t.containsVertex(vertexNr[1]) &&
			t.containsVertex(vertexNr[2]);
	}
	int vertexNr[3];
};

// ---------------------------------------------------------------------------------------
class Subdivider {
public:
	~Subdivider();
	void clear();

	// registration
	void registerVertex(const m3Vector &pos);
	void registerTriangle(int v0, int v1, int v2);
	void endRegistration(int subdivision);

	// manipulation
	void mergeVertices();
	void closeMesh();
	void subdivide();

	// accessors
	int  getNumVertices() const { return mVertices.size(); }
	const m3Vector& getVertexPosition(int vertexNr) const {
		assert(vertexNr < (int)mVertices.size());
		return mVertices[vertexNr]->pos;
	}
	int  getNumTriangles() const { return mTriangles.size(); }
	void getTriangle(int i, int &v0, int &v1, int &v2) const;

private:
	int  getTriangleNr(int v0, int v1, int v2) const;
	void closeHole(std::vector<int> &borderVertices);
	float qualityOfTriangle(int v0, int v1, int v2) const;

	m3Real mMaxLength;
	m3Bounds mBounds;
	std::vector<SubdividerVertex *>  mVertices;
	std::vector<SubdividerTriangle>  mTriangles;
};


#endif
