#ifndef HULL_MESH_H
#define HULL_MESH_H

#include "math3d.h"
#include <vector>

class Subdivider;
class Mesh;

//------------------------------------------------------------------------------------
struct HullVertex
{
	HullVertex(const m3Vector &p) : pos(p), newPos(p) { normal.zero(); }
	bool operator<(const HullVertex &v) const {
		return pos.x < v.pos.x;
	}
	bool operator==(const HullVertex &v) const {
		return pos.x == v.pos.x;
	}
	m3Vector pos;
	m3Vector newPos;
	m3Vector normal;
};

//------------------------------------------------------------------------------------
class HullMesh {
public:
	HullMesh() { clear(); }
	void clear();

	void createSurface(const Mesh &mesh, Subdivider &subdivider, int subdivision);

private:
	void subdivideVertices(m3Vector &p0, m3Vector &p1, m3Vector &p2, int depth);
	void subdivideTriangles(m3Vector &p0, m3Vector &p1, m3Vector &p2, int depth);
	void createSphere(int depth);
	void mergeVertices();
	void relax(const Mesh &mesh, int maxIterations);
	void initSubdivider(Subdivider &subdivider);
	int  indexOfVertex(const m3Vector &pos) const;
	bool pointInsideSweptTriangle(const HullVertex &h0, const HullVertex &h1, const HullVertex &h2, const m3Vector &pos);
	bool collisionHandling(HullVertex &h0, HullVertex &h1, HullVertex &h2, const m3Vector &pos);

	std::vector<HullVertex> mVertices;
	std::vector<int> mTriangles;
};


#endif
