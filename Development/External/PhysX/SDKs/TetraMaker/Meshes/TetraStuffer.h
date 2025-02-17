#ifndef TETRA_STUFFER_H
#define TETRA_STUFFER_H

#include "TetraMesh.h"

//------------------------------------------------------------------------------------
struct StufferCell {
	void init(int x, int y, int z, const m3Vector &origin, m3Real cellSize) {
		m3Real h = cellSize, h2 = 0.5f*cellSize;
		cornerDensity = -FLT_MAX;
		centerDensity = -FLT_MAX;
		cornerPos.set(origin.x + x*h, origin.y + y*h, origin.z + z*h);
		centerPos.set(cornerPos.x + h2, cornerPos.y + h2, cornerPos.z + h2);
		int i;
		for (i = 0; i < 3; i++) cornerCornerCutVertNr[i] = -1;
		for (i = 0; i < 3; i++) centerCenterCutVertNr[i] = -1;
		for (i = 0; i < 8; i++) centerCornerCutVertNr[i] = -1;
		cornerVertexNr = -1;
		centerVertexNr = -1;
	}
	m3Real cornerDensity;
	m3Real centerDensity;
	m3Vector cornerPos;
	m3Vector centerPos;
	int cornerVertexNr;
	int centerVertexNr;

	int cornerCornerCutVertNr[3];
	int centerCenterCutVertNr[3];
	int centerCornerCutVertNr[8];
	m3Vector cornerCornerCutPos[3];
	m3Vector centerCenterCutPos[3];
	m3Vector centerCornerCutPos[8];
};

//------------------------------------------------------------------------------------
struct StufferEdge {
	void init(int *cutVertNr, m3Vector *cutPos, bool cutOwner, int adjX, int adjY, int adjZ, bool adjCorner) {
		this->cutVertNr = cutVertNr;
		this->cutPos = cutPos;
		this->cutOwner = cutOwner;
		this->adjX = adjX;
		this->adjY = adjY;
		this->adjZ = adjZ;
		this->adjCorner = adjCorner;
	}
	int *cutVertNr;
	m3Vector *cutPos;
	bool cutOwner;
	int  adjX, adjY, adjZ;
	bool adjCorner;
};

//------------------------------------------------------------------------------------
struct StufferTriangle {
	StufferTriangle() {}
	StufferTriangle(int p0, int p1, int p2, float d0, float d1, float d2) { init(p0,p1,p2, d0,d1,d2); }
	void init(int p0, int p1, int p2, float d0, float d1, float d2) { 
		this->p0 = p0; this->p1 = p1; this->p2 = p2;
		this->d0 = d0; this->d1 = d1; this->d2 = d2;
	}
	bool contains(int p) {
		return p == p0 || p == p1 || p == p2;
	}
	float getDensityOf(int p) {
		if (p == p0) return d0;
		else if (p == p1) return d1;
		else if (p == p2) return d2;
		else assert(0);
	}
	int p0, p1, p2;	
	float d0, d1, d2;
};

//------------------------------------------------------------------------------------
struct StufferVertexChain {
	StufferVertexChain() { clear(); } 
	void clear() { p[0] = -1; p[1] = -1; p[2] = -1; p[3] = -1; }
	void addTriangle(const StufferTriangle &t, int tip) {
		int p0,p1;
		float d0,d1;
		if (tip == t.p0) { p0 = t.p1; d0 = t.d1; p1 = t.p2; d1 = t.d2; }
		if (tip == t.p1) { p0 = t.p0; d0 = t.d0; p1 = t.p2; d1 = t.d2; }
		if (tip == t.p2) { p0 = t.p0; d0 = t.d0; p1 = t.p1; d1 = t.d1; }
		if (p[1] < 0) {
			p[1] = p0; d[1] = d0;
			p[2] = p1; d[2] = d1;
		}
		else {
			if      (p0 == p[1]) { p[0] = p1; d[0] = d1; }
			else if (p1 == p[1]) { p[0] = p0; d[0] = d0; }
			else if (p0 == p[2]) { p[3] = p1; d[3] = d1; }
			else if (p1 == p[2]) { p[3] = p0; d[3] = d0; }
		}
	}
	void getTriangle(StufferTriangle &t) {
		if (p[0] < 0) t.init(p[1],p[2],p[3],d[1],d[2],d[3]);
		else t.init(p[0],p[1],p[2],d[0],d[1],d[2]);
	}
	void getTriangles(int cut0, int cut1, StufferTriangle &t0, StufferTriangle &t1) {
		if (p[0] == cut0 || p[0] == cut1) {
			t0.init(p[0],p[1],p[2],d[0],d[1],d[2]);
			t1.init(p[0],p[2],p[3],d[0],d[2],d[3]);
		}
		else {
			t0.init(p[1],p[2],p[3],d[1],d[2],d[3]);
			t1.init(p[1],p[3],p[0],d[1],d[3],d[0]);
		}
	}
	int p[4];
	float d[4];
};

//------------------------------------------------------------------------------------
class TetraStuffer {
public:
	TetraStuffer(std::vector<m3Vector> &vertices, std::vector<MeshTetrahedron> &tetrahedra) :
		mVertices(vertices), mTetrahedra(tetrahedra) 
		{}
	void createFrom(const Mesh &mesh, int subdivision);

private:
	inline bool validCoords(int xi, int yi, int zi) {
		return (xi >= 0 && xi < mNumX && yi >= 0 && yi < mNumY && zi >= 0 && zi < mNumZ);
	}
	inline StufferCell& cellAt(int xi, int yi, int zi) {
		assert(validCoords(xi,yi,zi));
		return mGrid[((xi * mNumY) + yi) * mNumZ + zi];
	}

	void createGrid(const Mesh &mesh, int subdivision);
	void setupGridUsingDensities(const Mesh &mesh);
	void setupGridUsingGeometry(const Mesh &mesh);
	void addTriangleField(const m3Vector &p0, const m3Vector &p1, const m3Vector &p2);
	bool interpolate(m3Real d0, m3Real d1, const m3Vector &pos0, const m3Vector &pos1, m3Vector &pos);
	void computeCutPoints();

	int  getCornerEdges(int xi, int yi, int zi, StufferEdge cuts[]);
	int  getCenterEdges(int xi, int yi, int zi, StufferEdge cuts[]);

	void computeCuts(const Mesh &mesh, float x0, float y0, float z0, float x1, float y1, float z1, 
					int &vertNr, m3Vector &cutPos);

	bool snapVertex(int xi, int yi, int zi, bool corner, bool testOnly = false);
	void snapVertices();

	void handleTetra(int p0, float d0, int p1, float d1, int p2, float d2, int p3, float d3,
					   int c01, int c12, int c20, int c03, int c13, int c23);
	void addTetra(int p, float d, const StufferTriangle &triangle);
	void addTriangle(int p0, int p1, int p2, float d0, float d1, float d2);
	void addTriangles(int p0, int p1, int p2, float d0, float d1, float d2,
						int c01, int c12, int c20);
	float tetraVolume(const MeshTetrahedron &t);
	void triangulateGrid();

	// representation
	std::vector<StufferCell>     mGrid;
	std::vector<m3Vector>        &mVertices;
	std::vector<MeshTetrahedron> &mTetrahedra;

	// auxiliaries
	std::vector<StufferTriangle> mTriangles;
	m3Real mIsoValue;
	m3Real mCellSize;
	m3Real mAlphaLong;
	m3Real mAlphaShort;

	m3Real mThickness;
	m3Vector mOrigin;
	int mNumX, mNumY, mNumZ;

	MeshHash mHash;
	std::vector<int> mItemIndices;
};

#endif
