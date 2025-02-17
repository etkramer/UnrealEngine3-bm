#include "Mesh.h"
#include "IsoMesh.h"
#include "MarchingCubes.h"
#include "QuadricSimplificator.h"
#include "Geometry3D.h"

// ----------------------------------------------------------------------
void IsoMesh::clear()
{
	mOrigin.zero();
	mCellSize = 1.0f;
	mNumX = 0;
	mNumY = 0;
	mNumZ = 0;
	mIsoValue = 0.5f;
	mGrid.clear();
	mTriangles.clear();
	mVertices.clear();
}

// ----------------------------------------------------------------------
void IsoMesh::createSurface(const Mesh &mesh, QuadricSimplificator &simplificator, int subdivision, bool singleSurface)
{
	clear();

	// init parameters;
	m3Bounds bounds;
	mesh.getBounds(bounds);
	mCellSize = bounds.min.distance(bounds.max) / subdivision;
	if (mCellSize == 0.0f) mCellSize = 1.0f;
	mThickness = mCellSize * 1.5f;
	bounds.fatten(2.0f * mThickness);

	mOrigin = bounds.min;
	m3Real invH = 1.0f / mCellSize;
	mNumX = (int)((bounds.max.x - bounds.min.x) * invH) + 1;
	mNumY = (int)((bounds.max.y - bounds.min.y) * invH) + 1;
	mNumZ = (int)((bounds.max.z - bounds.min.z) * invH) + 1;
	int num = mNumX * mNumY * mNumZ;
	mGrid.resize(num);

	for (int i = 0; i < num; i++)
		mGrid[i].init();

	// create the iso surface
	addDensities(mesh);
	generateMesh();
	if (singleSurface)
		removeLayers();
	initSimplificator(simplificator);
}

// ----------------------------------------------------------------------
void IsoMesh::addDensities(const Mesh &mesh)
{
	// fill in triangles
	m3Vector p0, p1, p2;
	for (int i = 0; i < mesh.getNumTriangles(); i++) {
		const MeshTriangle &t = mesh.getTriangle(i);
		p0 = mesh.getVertex(t.vertexNr[0]);
		p1 = mesh.getVertex(t.vertexNr[1]);
		p2 = mesh.getVertex(t.vertexNr[2]);
		addTriangle(p0,p1,p2);
	}
}

// ----------------------------------------------------------------------
void IsoMesh::addTriangle(const m3Vector &p0, const m3Vector &p1, const m3Vector &p2)
{
	if (mThickness == 0.0f) return;
	m3Bounds bounds;
	bounds.setEmpty();
	bounds.include(p0);
	bounds.include(p1);
	bounds.include(p2);
	bounds.fatten(mThickness);
	m3Real h = mCellSize;
	m3Real invH = 1.0f / h;
	m3Real invT = 1.0f / mThickness;
	m3Vector pos;

	int x0 = (int)((bounds.min.x - mOrigin.x) * invH); if (x0 < 0) x0 = 0;
	int x1 = (int)((bounds.max.x - mOrigin.x) * invH); if (x1 >= mNumX) x1 = mNumX - 1;
	int y0 = (int)((bounds.min.y - mOrigin.y) * invH); if (y0 < 0) y0 = 0;
	int y1 = (int)((bounds.max.y - mOrigin.y) * invH); if (y1 >= mNumY) y1 = mNumY - 1;
	int z0 = (int)((bounds.min.z - mOrigin.z) * invH); if (z0 < 0) z0 = 0;
	int z1 = (int)((bounds.max.z - mOrigin.z) * invH); if (z1 >= mNumZ) z1 = mNumZ - 1;

	int xi,yi,zi;

	for (xi = x0; xi <= x1; xi++) {
		pos.x = mOrigin.x + h * xi;
		for (yi = y0; yi <= y1; yi++) {
			pos.y = mOrigin.y + h * yi;
			for (zi = z0; zi <= z1; zi++) {
				pos.z = mOrigin.z + h * zi;
				m3Real dist = Geometry3D::trianglePointDist(p0, p1, p2, pos);
				if (dist > mThickness) continue;
				IsoCell &cell = cellAt(xi,yi,zi);
				m3Real density = 1.0f - dist * invT;
				if (cell.density == 0.0f) 
					cell.density = density;
				else 
					cell.density = m3Max(cell.density, density);
			}
		}
	}
}

// ----------------------------------------------------------------------
bool IsoMesh::interpolate(m3Real d0, m3Real d1, const m3Vector &pos0, const m3Vector &pos1, m3Vector &pos)
{
	if ((d0 < mIsoValue && d1 < mIsoValue) || (d0 > mIsoValue && d1 > mIsoValue))
		return false;

	m3Real s = (mIsoValue-d0) / (d1-d0);
	m3Clamp(s, 0.01f, 0.99f);	// safety not to produce vertices at the same location
	pos = pos0 * (1.0f - s) + pos1 * s;
	return true;
}

// ----------------------------------------------------------------------
void IsoMesh::draw(RendererColor baseColor, bool wireframe)
{
}

// ----------------------------------------------------------------------
void IsoMesh::generateMesh()
{
	mTriangles.clear();
	mVertices.clear();

	int xi,yi,zi;
	m3Real h = mCellSize;
	m3Vector pos, vPos;

	// generate vertices
	for (xi = 0; xi < mNumX; xi++) {
		pos.x = mOrigin.x + h * xi;
		for (yi = 0; yi < mNumY; yi++) {
			pos.y = mOrigin.y + h * yi;
			for (zi = 0; zi < mNumZ; zi++) {
				pos.z = mOrigin.z + h * zi;
				IsoCell &cell = cellAt(xi,yi,zi);
				m3Real d  = cell.density;
				if (xi < mNumX-1) {
					m3Real dx = cellAt(xi+1,yi,zi).density;
					if (interpolate(d,dx, pos, pos + m3Vector(h,0.0f,0.0f), vPos)) {
						cell.vertNrX = mVertices.size();
						mVertices.push_back(vPos);
					}
				}
				if (yi < mNumY-1) {
					m3Real dy = cellAt(xi,yi+1,zi).density;
					if (interpolate(d,dy, pos, pos + m3Vector(0.0f,h,0.0f), vPos)) {
						cell.vertNrY = mVertices.size();
						mVertices.push_back(vPos);
					}
				}
				if (zi < mNumZ-1) {
					m3Real dz = cellAt(xi,yi,zi+1).density;
					if (interpolate(d,dz, pos, pos + m3Vector(0.0f,0.0f,h), vPos)) {
						cell.vertNrZ = mVertices.size();
						mVertices.push_back(vPos);
					}
				}
			}
		}
	}

	// generate triangles
	int edges[12];
	IsoTriangle triangle;

	for (xi = 0; xi < mNumX-1; xi++) {
		pos.x = mOrigin.x + h * xi;
		for (yi = 0; yi < mNumY-1; yi++) {
			pos.y = mOrigin.y + h * yi;
			for (zi = 0; zi < mNumZ-1; zi++) {
				pos.z = mOrigin.z + h * zi;
				int code = 0;
				if (cellAt(xi,  yi,  zi  ).density > mIsoValue) code |= 1;
				if (cellAt(xi+1,yi,  zi  ).density > mIsoValue) code |= 2;
				if (cellAt(xi+1,yi+1,zi  ).density > mIsoValue) code |= 4;
				if (cellAt(xi,  yi+1,zi  ).density > mIsoValue) code |= 8;
				if (cellAt(xi,  yi,  zi+1).density > mIsoValue) code |= 16;
				if (cellAt(xi+1,yi,  zi+1).density > mIsoValue) code |= 32;
				if (cellAt(xi+1,yi+1,zi+1).density > mIsoValue) code |= 64;
				if (cellAt(xi,  yi+1,zi+1).density > mIsoValue) code |= 128;
				if (code == 0 || code == 255)
					continue;

				edges[ 0] = cellAt(xi,  yi,  zi  ).vertNrX;
				edges[ 1] = cellAt(xi+1,yi,  zi  ).vertNrY;
				edges[ 2] = cellAt(xi,  yi+1,zi  ).vertNrX;
				edges[ 3] = cellAt(xi,  yi,  zi  ).vertNrY;

				edges[ 4] = cellAt(xi,  yi,  zi+1).vertNrX;
				edges[ 5] = cellAt(xi+1,yi,  zi+1).vertNrY;
				edges[ 6] = cellAt(xi,  yi+1,zi+1).vertNrX;
				edges[ 7] = cellAt(xi,  yi,  zi+1).vertNrY;

				edges[ 8] = cellAt(xi,  yi,  zi  ).vertNrZ;
				edges[ 9] = cellAt(xi+1,yi,  zi  ).vertNrZ;
				edges[10] = cellAt(xi+1,yi+1,zi  ).vertNrZ;
				edges[11] = cellAt(xi,  yi+1,zi  ).vertNrZ;

				int *v = triTable[code];
				while (*v >= 0) {
					int v0 = edges[*v++]; assert(v0 >= 0);
					int v1 = edges[*v++]; assert(v1 >= 0);
					int v2 = edges[*v++]; assert(v2 >= 0);
					triangle.set(v0,v1,v2);
					mTriangles.push_back(triangle);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------
void IsoMesh::sortEdges(int l, int r)
{
	int i,j, mi;
	IsoEdge k, m;

	i = l; j = r; mi = (l + r)/2;
	m = mEdges[mi];
	while (i <= j) {
		while(mEdges[i] < m) i++;
		while(m < mEdges[j]) j--;
		if (i <= j) {
			k = mEdges[i]; mEdges[i] = mEdges[j]; mEdges[j] = k;
			i++; j--;
		}
	}
	if (l < j) sortEdges(l, j);
	if (i < r) sortEdges(i, r);
}

// ----------------------------------------------------------------------
void IsoMesh::floodFill(int triangleNr, int groupNr, int &numTriangles)
{
	IsoTriangle &t = mTriangles[triangleNr];
	t.groupNr = groupNr;
	numTriangles++;
	int adj;
	adj = t.adjTriangles[0];
	if (adj >= 0 && mTriangles[adj].groupNr < 0)
		floodFill(adj, groupNr, numTriangles);
	adj = t.adjTriangles[1];
	if (adj >= 0 && mTriangles[adj].groupNr < 0)
		floodFill(adj, groupNr, numTriangles);
	adj = t.adjTriangles[2];
	if (adj >= 0 && mTriangles[adj].groupNr < 0)
		floodFill(adj, groupNr, numTriangles);
}

// ----------------------------------------------------------------------
void IsoMesh::removeLayers()
{
	int i,j;

	// compute adjacencies
	mEdges.clear();
	IsoEdge edge;
	for (i = 0; i < (int)mTriangles.size(); i++) {
		IsoTriangle &t = mTriangles[i];
		edge.set(t.vertexNr[0], t.vertexNr[1], i); mEdges.push_back(edge);
		edge.set(t.vertexNr[1], t.vertexNr[2], i); mEdges.push_back(edge);
		edge.set(t.vertexNr[2], t.vertexNr[0], i); mEdges.push_back(edge);
	}
	sortEdges(0, mEdges.size()-1);

	i = 0;
	while (i < (int)mEdges.size()) {
		j = i+1;
		while (j < (int)mEdges.size() && mEdges[j] == mEdges[i]) j++;
		if (j > i+1) {
			mTriangles[mEdges[i].triangleNr].addNeighbor(mEdges[j-1].triangleNr);
			mTriangles[mEdges[j-1].triangleNr].addNeighbor(mEdges[i].triangleNr);
		}
		i = j;
	}

	// mark regions
	std::vector<int> mNumTriangles;

	for (i = 0; i < (int)mTriangles.size(); i++) {
		if (mTriangles[i].groupNr >= 0) continue;
		int num = 0;
		floodFill(i, mNumTriangles.size(), num);
		mNumTriangles.push_back(num);
	}

	// find largest group
	if (mNumTriangles.size() == 0) return;
	int maxNum = mNumTriangles[0];
	int groupNr = 0;
	for (i = 1; i < (int)mNumTriangles.size(); i++) {
		if (mNumTriangles[i] > maxNum) {
			maxNum = mNumTriangles[i];
			groupNr = i;
		}
	}

	// remove triangles of other groups
	i = 0;
	while (i < (int)mTriangles.size()) {
		if (mTriangles[i].groupNr != groupNr) {
			mTriangles[i] = mTriangles[mTriangles.size()-1];
			mTriangles.pop_back();
		}
		else i++;
	}

	// remove non references vertices
	std::vector<int> oldToNew;
	std::vector<m3Vector> newVertices;

	oldToNew.resize(mVertices.size());
	for (i = 0; i < (int)oldToNew.size(); i++)
		oldToNew[i] = -1;

	for (i = 0; i < (int)mTriangles.size(); i++) {
		IsoTriangle &t = mTriangles[i];
		for (j = 0; j < 3; j++) {
			int vNr = t.vertexNr[j];
			if (oldToNew[vNr] < 0) {
				oldToNew[vNr] = newVertices.size();
				newVertices.push_back(mVertices[vNr]);
			}
			t.vertexNr[j] = oldToNew[vNr];
		}
	}

	mVertices.clear();
	for (i = 0; i < (int)newVertices.size(); i++)
		mVertices.push_back(newVertices[i]);
}

// ----------------------------------------------------------------------
void IsoMesh::initSimplificator(QuadricSimplificator &simplificator)
{
	simplificator.clear();
	int i;

	for (i = 0; i < (int)mVertices.size(); i++)
		simplificator.registerVertex(mVertices[i]);

	for (i = 0; i < (int)mTriangles.size(); i++) {
		IsoTriangle &t = mTriangles[i];
		simplificator.registerTriangle(t.vertexNr[0], t.vertexNr[1], t.vertexNr[2]);
	}
	simplificator.endRegistration();
}
