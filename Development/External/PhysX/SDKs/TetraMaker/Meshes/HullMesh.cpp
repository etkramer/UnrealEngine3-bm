#include "HullMesh.h"
#include "Mesh.h"
#include "Subdivider.h"
#include "Geometry3D.h"
#include "MeshHash.h"
#include <algorithm>

// for initial icosahedron
#define X 0.525731112119133606f
#define Z 0.850650808352039932f

// ----------------------------------------------------------------------
void HullMesh::clear()
{
	mVertices.clear();
	mTriangles.clear();
}


// ----------------------------------------------------------------------
void HullMesh::subdivideVertices(m3Vector &p0, m3Vector &p1, m3Vector &p2, int depth)
{
	if (depth == 0) { 
		mVertices.push_back(HullVertex(p0));
		mVertices.push_back(HullVertex(p1));
		mVertices.push_back(HullVertex(p2));
	}
	else {
		m3Vector p01 = p0 + p1; p01.normalize();
		m3Vector p12 = p1 + p2; p12.normalize();
		m3Vector p20 = p2 + p0; p20.normalize();
		subdivideVertices(p0, p01, p20, depth-1);
		subdivideVertices(p1, p12, p01, depth-1);
		subdivideVertices(p2, p20, p12, depth-1);
		subdivideVertices(p01, p12, p20, depth-1);
	}
}

// ----------------------------------------------------------------------
int  HullMesh::indexOfVertex(const m3Vector &pos) const
{
	// binary search in sorted list
	HullVertex v(pos);
	int l = 0;
	int r = mVertices.size()-1;
	if (r < 0) return -1;
	int m = 0;
	while (l <= r) {
        m = (l+r)/2;
        if (mVertices[m] < v) l = m+1;
        else if (v < mVertices[m]) r = m-1;
        else break;
	}
	if (!(mVertices[m] == v)) return -1;
	while (m >= 0 && mVertices[m] == v) m--;
	m++;
	while (m < (int)mVertices.size() && mVertices[m] == v) {
		if (mVertices[m].pos == pos) return m;
		m++;
	}
	return -1;
}

// ----------------------------------------------------------------------
void HullMesh::subdivideTriangles(m3Vector &p0, m3Vector &p1, m3Vector &p2, int depth)
{
	if (depth == 0) { 
		int i0 = indexOfVertex(p0); assert(i0 >= 0);
		int i1 = indexOfVertex(p1); assert(i1 >= 0);
		int i2 = indexOfVertex(p2); assert(i2 >= 0);
		mTriangles.push_back(i0);
		mTriangles.push_back(i1);
		mTriangles.push_back(i2);
	}
	else {
		m3Vector p01 = p0 + p1; p01.normalize();
		m3Vector p12 = p1 + p2; p12.normalize();
		m3Vector p20 = p2 + p0; p20.normalize();
		subdivideTriangles(p0, p01, p20, depth-1);
		subdivideTriangles(p1, p12, p01, depth-1);
		subdivideTriangles(p2, p20, p12, depth-1);
		subdivideTriangles(p01, p12, p20, depth-1);
	}
}

// ----------------------------------------------------------------------
void HullMesh::createSphere(int depth)
{
	clear();

	// start with an icosahedron
	static m3Real coords[12][3] = {
		{-X, 0.0f, Z}, {X, 0.0f, Z}, {-X, 0.0f, -Z}, {X, 0.0f, -Z},
		{0.0f, Z, X}, {0.0f, Z, -X}, {0.0f, -Z, X}, {0.0f, -Z, -X},
		{Z, X, 0.0f}, {-Z, X, 0.0f}, {Z, -X, 0.0f}, {-Z, -X, 0.0f}
	};

	static int indices[20][3] = {
		{1,4,0}, {4,9,0}, {4,5,9}, {8,5,4}, {1,8,4},
		{1,10,8}, {10,3,8}, {8,3,5}, {3,2,5}, {3,7,2},
		{3,10,7}, {10,6,7}, {6,11,7}, {6,0,11}, {6,1,0},
		{10,1,6}, {11,0,9}, {2,11,9}, {5,2,9}, {11,2,7}
	};

	int i,j;
	m3Vector p0,p1,p2;

	// create vertices
	for (i = 0; i < 20; i++) {
		int i0 = indices[i][0];
		int i1 = indices[i][1];
		int i2 = indices[i][2];
		p0.set(coords[i0][0], coords[i0][1], coords[i0][2]);
		p1.set(coords[i1][0], coords[i1][1], coords[i1][2]);
		p2.set(coords[i2][0], coords[i2][1], coords[i2][2]);
		subdivideVertices(p0,p1,p2, depth);
	}

	// compress vertices
	sort(mVertices.begin(), mVertices.end());

	std::vector<HullVertex> compressedVertices;
	for (i = 0; i < (int)mVertices.size(); i++) {
		HullVertex &v = mVertices[i];
		j = i+1;
		bool found = false;
		while (j < (int)mVertices.size() && mVertices[j].pos.x == v.pos.x) {
			if (mVertices[j].pos == v.pos) { found = true; break; }
			j++;
		}
		if (!found) compressedVertices.push_back(v);
	}
	mVertices.clear();
	for (i = 0; i < (int)compressedVertices.size(); i++)
		mVertices.push_back(compressedVertices[i]);

	// generate triangles
	for (i = 0; i < 20; i++) {
		int i0 = indices[i][0];
		int i1 = indices[i][1];
		int i2 = indices[i][2];
		p0.set(coords[i0][0], coords[i0][1], coords[i0][2]);
		p1.set(coords[i1][0], coords[i1][1], coords[i1][2]);
		p2.set(coords[i2][0], coords[i2][1], coords[i2][2]);
		subdivideTriangles(p0,p1,p2, depth);
	}
}

// ----------------------------------------------------------------------
bool HullMesh::pointInsideSweptTriangle(const HullVertex &h0, const HullVertex &h1, const HullVertex &h2, 
										const m3Vector &pos)
{
	m3Bounds b;
	b.setEmpty();
	m3Vector p0 = h0.pos; b.include(p0);
	m3Vector p1 = h1.pos; b.include(p1);
	m3Vector p2 = h2.pos; b.include(p2);
	m3Vector q0 = h0.newPos; b.include(q0);
	m3Vector q1 = h1.newPos; b.include(q1);
	m3Vector q2 = h2.newPos; b.include(q2);
	m3Vector center;
	b.getCenter(center);
	// safety
	m3Real s = 1.001f;
	m3Real d = b.min.distance(b.max) * (s - 1.0f);
	b.fatten(d);
	// early out
	if (!b.contain(pos)) return false;

	p0 = (p0-center)*s + center;
	p1 = (p1-center)*s + center;
	p2 = (p2-center)*s + center;

	q0 = (q0-center)*s + center;
	q1 = (q1-center)*s + center;
	q2 = (q2-center)*s + center;

	if (Geometry3D::pointInTetra(p0, p1, p2, q2, pos)) return true;
	if (Geometry3D::pointInTetra(p0, p1, q2, q1, pos)) return true;
	if (Geometry3D::pointInTetra(p0, q1, q2, q0, pos)) return true;
	return false;
}

// ----------------------------------------------------------------------
bool HullMesh::collisionHandling(HullVertex &h0, HullVertex &h1, HullVertex &h2, const m3Vector &pos)
{
	if (!pointInsideSweptTriangle(h0, h1, h2, pos)) return false;

	m3Real collisionTime;
	if (!Geometry3D::collisionMovingPointMovingTriangle(pos, pos, 
		h0.pos, h0.newPos, h1.pos, h1.newPos, h2.pos, h2.newPos, collisionTime))
		return false;

	collisionTime *= 0.80f;
	h0.newPos = h0.pos + (h0.newPos - h0.pos) * collisionTime;
	h1.newPos = h1.pos + (h1.newPos - h1.pos) * collisionTime;
	h2.newPos = h2.pos + (h2.newPos - h2.pos) * collisionTime;

	return true;
}

// ----------------------------------------------------------------------
void HullMesh::relax(const Mesh &mesh, int maxIterations)
{
	int i,j;
	MeshHash hash;
	m3Bounds bounds;
	m3Vector d,n;
	mesh.getBounds(bounds);
	m3Real h = bounds.min.distance(bounds.max) * 0.05f;
	hash.setGridSpacing(h);

	// hash mesh vertices for faster access
	const std::vector<m3Vector> meshVertices = mesh.getVertices();
	for (i = 0; i < (int)meshVertices.size(); i++) 
		hash.add(meshVertices[i], i);

	// simulate a membrane with zero edge rest lengths
	int v0,v1,v2;
	bool surfaceHit = false;
	for (int iters = 0; iters < maxIterations; iters++) {
		for (i = 0; i < (int) mVertices.size(); i++) 
			mVertices[i].newPos = mVertices[i].pos;

		// elastic edges

		for (i = 0; i < (int) mTriangles.size(); i += 3) {
			v0 = mTriangles[i];
			v1 = mTriangles[i+1];
			v2 = mTriangles[i+2];
			HullVertex &h0 = mVertices[v0];
			HullVertex &h1 = mVertices[v1];
			HullVertex &h2 = mVertices[v2];

			m3Real s = 0.25f;
			d = (h1.newPos - h0.newPos) * s; h0.newPos += d; h1.newPos -= d;
			d = (h2.newPos - h1.newPos) * s; h1.newPos += d; h2.newPos -= d;
			d = (h0.newPos - h2.newPos) * s; h2.newPos += d; h0.newPos -= d;
		}

		// pressure force
		if (!surfaceHit) {
			for (i = 0; i < (int) mVertices.size(); i++) 
				mVertices[i].normal.zero();

			for (i = 0; i < (int) mTriangles.size(); i += 3) {
				v0 = mTriangles[i];
				v1 = mTriangles[i+1];
				v2 = mTriangles[i+2];
				HullVertex &h0 = mVertices[v0];
				HullVertex &h1 = mVertices[v1];
				HullVertex &h2 = mVertices[v2];

				n.cross(h1.newPos - h0.newPos, h2.newPos - h0.newPos);
				n.normalize();
				h0.normal += n;
				h1.normal += n;
				h2.normal += n;
			}

			for (i = 0; i < (int) mVertices.size(); i++) {
				HullVertex &hv = mVertices[i];
				hv.normal.normalize();
				hv.newPos -= hv.normal * h;
			}
		}

		// collision handling
		m3Bounds triBounds;
		std::vector<int> itemIndices;

		for (i = 0; i < (int) mTriangles.size(); i += 3) {
			v0 = mTriangles[i];
			v1 = mTriangles[i+1];
			v2 = mTriangles[i+2];
			HullVertex &h0 = mVertices[v0];
			HullVertex &h1 = mVertices[v1];
			HullVertex &h2 = mVertices[v2];
			triBounds.setEmpty();
			triBounds.include(h0.pos);
			triBounds.include(h1.pos);
			triBounds.include(h2.pos);
			triBounds.include(h0.newPos);
			triBounds.include(h1.newPos);
			triBounds.include(h2.newPos);
			hash.queryUnique(triBounds, itemIndices);
			bool moveOK = true;
			for (j = 0; j < (int)itemIndices.size(); j++) {
				if (collisionHandling(h0,h1,h2, meshVertices[itemIndices[j]]))
					surfaceHit = true;
			}
		}

		for (i = 0; i < (int)mVertices.size(); i++) 
			mVertices[i].pos = mVertices[i].newPos;
	}
}

// ----------------------------------------------------------------------
void HullMesh::createSurface(const Mesh &mesh, Subdivider &subdivider, int subdivision)
{
	int depth = 1;
	int s = subdivision;
	while (s > 10) {
		s /= 2;
		depth++;
	}
	createSphere(depth);

	// transform sphere to enclose mesh

	m3Bounds bounds;
	m3Vector center;
	mesh.getBounds(bounds);
	bounds.getCenter(center);
	m3Real r = bounds.min.distance(bounds.max) * 0.51f;
	for (int i = 0; i < (int)mVertices.size(); i++) {
		m3Vector &pos = mVertices[i].pos;
		pos *= r;
		pos += center;
	}
	relax(mesh, 2 * subdivision);

	initSubdivider(subdivider);
}

// ----------------------------------------------------------------------
void HullMesh::initSubdivider(Subdivider &subdivider)

{
	subdivider.clear();
	int i;

	for (i = 0; i < (int)mVertices.size(); i++)
		subdivider.registerVertex(mVertices[i].pos);

	int *v = &mTriangles[0];
	for (i = 0; i < (int)mTriangles.size()/3; i++) {
		int v0 = *v++;
		int v1 = *v++;
		int v2 = *v++;
		subdivider.registerTriangle(v0, v1, v2);
	}
}
