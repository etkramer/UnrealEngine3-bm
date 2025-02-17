#ifndef TETRA_MESH_H
#define TETRA_MESH_H

#include "Mesh.h"
#include "MeshHash.h"
#include "math3d.h"
#include <vector>

struct MeshTetrahedron;
struct MeshTetraFace;
struct MeshVertexLink;
struct TetraUndo;
class  TetraStuffer;

//------------------------------------------------------------------------------------
enum TetrahedronFlags {
	TETRAHEDRON_DELETE = 1
};

//------------------------------------------------------------------------------------
class TetraMesh {
public:
	TetraMesh();
	~TetraMesh();
	void clear();
	void createFrom(const Mesh &mesh, int subdivision);
	void createUsingTetraStuffer(const Mesh &mesh, int subdivision);

	bool save(char *filename, const Mesh *originalMesh = NULL);
	bool load(char *filename);

	void draw(RendererColor baseColor, bool wireframe, m3Real xClipping);
	void getBounds(m3Bounds &bounds) const;

	bool rayCast(const m3Vector &orig, const m3Vector &dir, m3Real &t);
	int  closestVertex(const m3Vector &pos, m3Real &distance) const;
	int  closestTetrahedron(const m3Vector &pos, m3Real &distance) const;

	void deleteTetrahedron(int tetraNr);
	bool addTetrahedron(int tetraNr, const m3Vector &pos);
	void undo();

	int  getNumVertices() const { return mVertices.size(); }
	int  getNumTetrahedra() const { return mTetrahedra.size(); }
	const MeshTetrahedron& getTetrahedron(int i) const;
	const m3Vector& getVertex(int i) const;
	int  getPercentage() const;
	void cancelOperation();

private:
	void   setPercentage(int percentage);
	m3Real getTetraVolume(const MeshTetrahedron &tetra) const;
	m3Real getTetraVolume(int v0, int v1, int v2, int v3) const;
	m3Real getTetraQuality(const MeshTetrahedron &tetra) const;
	m3Real getTetraLongestEdge(const MeshTetrahedron &tetra) const;
	bool   pointInTetra(const MeshTetrahedron &tetra, const m3Vector &p) const;

	void generateAdditionalVertices(const Mesh &mesh, bool innerOnly, int subdivision);
	void relaxAdditionalVertices(const Mesh &mesh, int firstVertex, float minDist);
	void delaunayTetrahedralization(const m3Bounds &bounds);
	void splitTetrahedra(const Mesh &mesh);
	void removeOuterTetrahedra(const Mesh &mesh);
	void compressVertices();
	void computeVertexLinks(const Mesh *originalMesh);
	bool addTetrahedron(int tetraNr, int sideV0, int sideV1, int sideV2, int edge);
	void swapTetrahedra(const Mesh &mesh, int subdivision);
	bool swapEdge(int v0, int v1);

	bool isFarVertex(int vertNr) { return mFirstFarVertex <= vertNr && vertNr <= mLastFarVertex; }

	// representation
	std::vector<m3Vector>        mVertices;
	std::vector<MeshTetrahedron> mTetrahedra;
	std::vector<MeshVertexLink>  mVertexLinks;

	// auxiliaries
	MeshHash mHash;
	TetraStuffer *mTetraStuffer;
	std::vector<int> mItemIndices;
	std::vector<TetraUndo> mUndos;
	int mFirstFarVertex;
	int mLastFarVertex;
	int mPercentage;
	bool mCancel;
};

//------------------------------------------------------------------------------------
struct MeshTetrahedron
{
	void init() {
		vertexNr[0] = -1; vertexNr[1] = -1; vertexNr[2] = -1; vertexNr[3] = -1;
		center.zero();
		radiusSquared = 0.0f;
		flags = 0;
	}
	void set(int v0, int v1, int v2, int v3) {
		flags = 0;
		// first vertex is smallest, second vertex is second smallest
		// orientation is kept constant
		int s1,s2,s3;
		if (v0 < v1 && v0 < v2 && v0 < v3) { vertexNr[0] = v0; s1 = v1; s2 = v2; s3 = v3; }
		else if (v1 < v2 && v1 < v3)       { vertexNr[0] = v1; s1 = v2; s2 = v0; s3 = v3; }
		else if (v2 < v3)                  { vertexNr[0] = v2; s1 = v3; s2 = v0; s3 = v1; }
		else                               { vertexNr[0] = v3; s1 = v0; s2 = v2; s3 = v1; }
		if (s1 < s2 && s1 < s3) { vertexNr[1] = s1; vertexNr[2] = s2; vertexNr[3] = s3; }
		else if (s2 < s3)       { vertexNr[1] = s2; vertexNr[2] = s3; vertexNr[3] = s1; }
		else                    { vertexNr[1] = s3; vertexNr[2] = s1; vertexNr[3] = s2; }
	}
	bool sameVertices(MeshTetrahedron &t) {
		if (vertexNr[0] != t.vertexNr[0]) return false;
		if (vertexNr[1] != t.vertexNr[1]) return false;
		if (vertexNr[2] == t.vertexNr[2] && vertexNr[3] == t.vertexNr[3]) return true;
		if (vertexNr[2] == t.vertexNr[3] && vertexNr[3] == t.vertexNr[2]) return true;
		return false;
	}
	bool operator==(MeshTetrahedron &t) {
		if (vertexNr[0] != t.vertexNr[0]) return false;
		if (vertexNr[1] != t.vertexNr[1]) return false;
		if (vertexNr[2] != t.vertexNr[2]) return false;
		if (vertexNr[3] != t.vertexNr[3]) return false;
		return true;
	}
	bool containsVertex(int nr) {
		return (vertexNr[0] == nr || vertexNr[1] == nr || vertexNr[2] == nr || vertexNr[3] == nr);
	}
	void replaceVertex(int nr, int newNr) {
		if (vertexNr[0] == nr) { vertexNr[0] = newNr; return; }
		if (vertexNr[1] == nr) { vertexNr[1] = newNr; return; }
		if (vertexNr[2] == nr) { vertexNr[2] = newNr; return; }
		if (vertexNr[3] == nr) { vertexNr[3] = newNr; return; }
	}
	void getOppositeVertices(int vNr0, int vNr1, int &vNr2, int &vNr3)
	{
		int v[4], p = 0;
		if (vertexNr[0] != vNr0 && vertexNr[0] != vNr1) v[p++] = vertexNr[0];
		if (vertexNr[1] != vNr0 && vertexNr[1] != vNr1) v[p++] = vertexNr[1];
		if (vertexNr[2] != vNr0 && vertexNr[2] != vNr1) v[p++] = vertexNr[2];
		if (vertexNr[3] != vNr0 && vertexNr[3] != vNr1) v[p++] = vertexNr[3];
		vNr2 = v[0]; vNr3 = v[1];
	}
	// representation
	int vertexNr[4];
	int flags;

	// for Delaunay
	m3Vector center;
	m3Real   radiusSquared;

	// static
	static const int sideIndices[4][3];
};

//------------------------------------------------------------------------------------
struct MeshTetraFace
{
	void set(int v0, int v1, int v2) {
		if (v0 < v1 && v0 < v2) {
			vertexNr[0] = v0; vertexNr[1] = v1; vertexNr[2] = v2;
		}
		else if (v1 < v2) {
			vertexNr[0] = v1; vertexNr[1] = v2; vertexNr[2] = v0;
		}
		else {
			vertexNr[0] = v2; vertexNr[1] = v0; vertexNr[2] = v1;
		}
	}
	bool operator==(MeshTetraFace &f) const {
		if (vertexNr[0] != f.vertexNr[0]) return false;
		if (vertexNr[1] == f.vertexNr[1] && vertexNr[2] == f.vertexNr[2]) return true;
		if (vertexNr[1] == f.vertexNr[2] && vertexNr[2] == f.vertexNr[1]) return true;
		return false;
	}
	bool operator <(MeshTetraFace &f) const {
		if (vertexNr[0] < f.vertexNr[0]) return true;
		if (vertexNr[0] > f.vertexNr[0]) return false;
		if (vertexNr[1] < f.vertexNr[1]) return true;
		if (vertexNr[1] > f.vertexNr[1]) return false;
		return vertexNr[2] < f.vertexNr[2];
	}
	int vertexNr[3];
};

//------------------------------------------------------------------------------------
struct MeshVertexLink
{
	int tetraNr;
	m3Vector barycentricCoords;
};

//------------------------------------------------------------------------------------
enum TetraUndoType { tuAdd, tuDelete };

struct TetraUndo
{
	TetraUndoType type;
	MeshTetrahedron tetra;
};

#endif
