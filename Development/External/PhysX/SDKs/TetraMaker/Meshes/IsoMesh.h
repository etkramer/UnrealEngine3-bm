#ifndef ISO_MESH_H
#define ISO_MESH_H

#include "glRenderer.h"
#include "QuadricSimplificator.h"
#include "math3d.h"
#include <vector>

class QuadricSimplificator;
class Mesh;

//------------------------------------------------------------------------------------
struct IsoCell {
	void init() {
		density = 0.0f;
		vertNrX = -1;
		vertNrY = -1;
		vertNrZ = -1;
	}
	m3Real density;
	int vertNrX;
	int vertNrY;
	int vertNrZ;
};

//------------------------------------------------------------------------------------
struct IsoTriangle{
	void init() {
		vertexNr[0] = -1;
		vertexNr[1] = -1;
		vertexNr[2] = -1;
		adjTriangles[0] = -1;
		adjTriangles[1] = -1;
		adjTriangles[2] = -1;
		groupNr = -1;
	}
	void set(int v0, int v1, int v2) {
		init();
		vertexNr[0] = v0;
		vertexNr[1] = v1;
		vertexNr[2] = v2;
	}
	void addNeighbor(int triangleNr) {
		if (adjTriangles[0] == -1) adjTriangles[0] = triangleNr;
		else if (adjTriangles[1] == -1) adjTriangles[1] = triangleNr;
		else if (adjTriangles[2] == -1) adjTriangles[2] = triangleNr;
	}
	int vertexNr[3];
	int adjTriangles[3];
	int groupNr;
};

// ----------------------------------------------------------------------
struct IsoEdge {
	void set(int newV0, int newV1, int newTriangle) {
		if (newV0 < newV1) { v0 = newV0; v1 = newV1; }
		else { v0 = newV1; v1 = newV0; }
		triangleNr = newTriangle;
	}
	bool operator < (const IsoEdge &e) const {
		if (v0 < e.v0) return true;
		if (v0 > e.v0) return false;
		return (v1 < e.v1);
	}
	bool operator == (const IsoEdge &e) const {
		return v0 == e.v0 && v1 == e.v1;
	}
	int v0, v1;
	int triangleNr;
};

//------------------------------------------------------------------------------------
class IsoMesh {
public:
	IsoMesh() { clear(); }
	void clear();

	void createSurface(const Mesh &mesh, QuadricSimplificator &simplificator, int subdivision, bool singleSurface);
	void draw(RendererColor baseColor, bool wireframe = false);

private:
	inline IsoCell& cellAt(int xi, int yi, int zi) {
		return mGrid[((xi * mNumY) + yi) * mNumZ + zi];
	}
	void addTriangle(const m3Vector &p0, const m3Vector &p1, const m3Vector &p2);
	bool interpolate(m3Real d0, m3Real d1, const m3Vector &pos0, const m3Vector &pos1, m3Vector &pos);

	void addDensities(const Mesh &mesh);
	void generateMesh();
	void removeLayers();
	void initSimplificator(QuadricSimplificator &simplificator);
	void sortEdges(int l, int r);
	void floodFill(int triangleNr, int groupNr, int &numTriangles);

	m3Vector mOrigin;
	m3Real   mCellSize;
	int      mNumX, mNumY, mNumZ;
	m3Real   mIsoValue;
	m3Real   mThickness;

	std::vector<IsoCell> mGrid;

	std::vector<IsoTriangle> mTriangles;
	std::vector<m3Vector> mVertices;
	std::vector<IsoEdge> mEdges;
};


#endif
