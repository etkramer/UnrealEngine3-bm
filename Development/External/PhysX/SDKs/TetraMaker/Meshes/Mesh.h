#ifndef MESH_H
#define MESH_H

#include "glRenderer.h"

#include "QuadricSimplificator.h"
#include "Subdivider.h"
#include "math3d.h"
#include <vector>

//------------------------------------------------------------------------------------
#define MESH_STRING_LEN 256
typedef char MeshString[MESH_STRING_LEN];

//------------------------------------------------------------------------------------
struct MeshMaterial
{
	void init();
	MeshString  name;
	int         texNr;

	MeshString  texFilename;
	m3Real ambient[3];
	m3Real diffuse[3];
	m3Real specular[3];
	m3Real alpha;
	m3Real shininess;
};

//------------------------------------------------------------------------------------
struct MeshTriangle
{
public:
	void init() {
		vertexNr[0] = -1; vertexNr[1] = -1; vertexNr[2] = -1;
		normalNr[0] = -1; normalNr[1] = -1; normalNr[2] = -1;
		texCoordNr[0] = -1; texCoordNr[1] = -1; texCoordNr[2] = -1;
		materialNr = -1;
	}
	void set(int v0, int v1, int v2, int mat) {
		vertexNr[0] = v0; vertexNr[1] = v1; vertexNr[2] = v2;
		normalNr[0] = v0; normalNr[1] = v1; normalNr[2] = v2;
		texCoordNr[0] = v0; texCoordNr[1] = v1; texCoordNr[2] = v2;
		materialNr = mat;
	}
	bool containsVertexNr(int vNr) const {
		return vNr == vertexNr[0] || vNr == vertexNr[1] || vNr == vertexNr[2];
	}

	// representation
	int vertexNr[3];
	int normalNr[3];
	int texCoordNr[3];
	int materialNr;
};

//------------------------------------------------------------------------------------
class Mesh {
public:
	Mesh() { clear(); }

	bool loadFromObjFile(char *filename);
	bool saveToObjFile(char *filename) const;
	void initFrom(const QuadricSimplificator &simplificator);
	void initSimplificator(QuadricSimplificator &simplificator) const;

	void initFrom(const Subdivider &subdivier);
	void initSubdivider(Subdivider &subdivier, int subdivision) const;

	void draw(RendererColor baseColor, bool wireframe = false);

	void clear();

	int   getNumTriangles() const { return (int)mTriangles.size(); }
	const MeshTriangle& getTriangle(int i) const { return mTriangles[i]; }
	void  getTriangleBounds(int i, m3Bounds &bounds) const;

	int   getNumVertices() const { return (int)mVertices.size(); }
	const m3Vector& getVertex(int i) const { return mVertices[i]; }
	const std::vector<m3Vector> &getVertices() const { return mVertices; }

	void setDiagonal(m3Real diagonal);
	void setCenter(const m3Vector &center);
	void transform(const m3Affine &a);

	void getBounds(m3Bounds &bounds) const { bounds = mBounds; }
	bool rayCast(const m3Vector &orig, const m3Vector &dir, m3Real &t);

	void *getUserData() { return mUserData; }
	void setUserData(void *userData) { mUserData = userData; }
	const char* getName() const { return mName; }

	unsigned int addVertex(float x,float y,float z);
	unsigned int addTriangle(unsigned int a,unsigned int b,unsigned int c);
	unsigned int addNormal(float x,float y,float z);
	void updateNormals();
	void updateBounds();
	void mergeVertices();
	void flipTriangleOrientation();

private:
	bool importMtlFile(char *mtllib);
	void extractPath(char *filename);

	static const int maxVerticesPerFace = 20;

	std::vector<MeshTriangle> mTriangles;
	std::vector<m3Vector>     mVertices;
	std::vector<m3Vector>     mNormals;
	std::vector<TexCoord>     mTexCoords;
	std::vector<MeshMaterial> mMaterials;

	std::vector<int> mDrawIndices;

	std::vector<int> mOldToNew;	// simplificator -> mesh

	MeshString mName;
	MeshString mPath;
	m3Bounds mBounds;
	bool mHasTextureCoords;
	bool mHasNormals;

	void *mUserData;
};


#endif
