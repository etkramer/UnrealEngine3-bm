#include "TetraStuffer.h"
#include "Geometry3D.h"

// Taken from paper "Labelle, Shewchuck: Isosurface Stuffing: 
// Fast Tetrahedral meshes with Good Dihedral Angles

// The algorithm works on a 3D BCC grid (regular cubical grid 
// combined with a regular cubical grid shifted by half the edge length
// 2D example:

//   A----|----B----|--
//   |    |    |    |
//   |    |    |    |
//  ------C---------D--
//   |    |    |    |
//   |    |    |    |
//   E----|----F----|----G
//   |    |    |    |
//   |    |    |    |
//  ------H---------J--
//   |    |    |    |

// the grid is stored by cells of only one of the grids
// Cell C-D-H-J owns corner vertex H plus center vertex F
// it also owns the corner-corner edges H-J and H-C,
// the center-center edges F-G and F-B
// and the center-corner edges F-H, F-J, F-D and F-C (not shown).
// This way all vertices and edges are assign uniquely.

// ----------------------------------------------------------------------
void TetraStuffer::createFrom(const Mesh &mesh, int subdivision)
{
	mIsoValue   = 0.5f;
	mAlphaLong  = 0.22f;
	mAlphaShort = 0.30f;

	createGrid(mesh, subdivision);

	setupGridUsingDensities(mesh);

//	setupGridUsingGeometry(mesh);	
	// this version does not use a density field
	// it computes the cuts directly using the mesh.
	// Pro:	 The boundary of the tet mesh is close to the tri mesh
	// Cons: Only fine grids can resolve detailed meshes

	snapVertices();
	triangulateGrid();
}

// ----------------------------------------------------------------------
void TetraStuffer::createGrid(const Mesh &mesh, int subdivision)
{
	m3Bounds bounds;
	mesh.getBounds(bounds);
	mCellSize = bounds.min.distance(bounds.max) / subdivision;
	if (mCellSize == 0.0f) mCellSize = 1.0f;
	m3Real invH = 1.0f / mCellSize;
	mThickness = mCellSize * 1.5f;
	bounds.fatten(2.0f * mThickness);

	// safetly band of cells -> no boundary check necessary
	mOrigin = bounds.min - m3Vector(mCellSize, mCellSize, mCellSize);
	mNumX = (int)((bounds.max.x - bounds.min.x) * invH) + 3;
	mNumY = (int)((bounds.max.y - bounds.min.y) * invH) + 3;
	mNumZ = (int)((bounds.max.z - bounds.min.z) * invH) + 3;
	int num = mNumX * mNumY * mNumZ;
	mGrid.resize(num);

	for (int xi = 0; xi < mNumX; xi++) {
		for (int yi = 0; yi < mNumY; yi++) {
			for (int zi = 0; zi < mNumZ; zi++) {
				cellAt(xi,yi,zi).init(xi,yi,zi, mOrigin, mCellSize);
			}
		}
	}
}

// ----------------------------------------------------------------------
void TetraStuffer::setupGridUsingDensities(const Mesh &mesh)
{
	// fill in triangles
	m3Vector p0, p1, p2;
	for (int i = 0; i < mesh.getNumTriangles(); i++) {
		const MeshTriangle &t = mesh.getTriangle(i);
		p0 = mesh.getVertex(t.vertexNr[0]);
		p1 = mesh.getVertex(t.vertexNr[1]);
		p2 = mesh.getVertex(t.vertexNr[2]);
		addTriangleField(p0,p1,p2);
	}
	computeCutPoints();
}

// ----------------------------------------------------------------------
void TetraStuffer::addTriangleField(const m3Vector &p0, const m3Vector &p1, const m3Vector &p2)
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

	int x0 = (int)((bounds.min.x - mOrigin.x) * invH); if (x0 < 1) x0 = 1;
	int x1 = (int)((bounds.max.x - mOrigin.x) * invH); if (x1 >= mNumX-1) x1 = mNumX - 2;
	int y0 = (int)((bounds.min.y - mOrigin.y) * invH); if (y0 < 1) y0 = 1;
	int y1 = (int)((bounds.max.y - mOrigin.y) * invH); if (y1 >= mNumY-1) y1 = mNumY - 2;
	int z0 = (int)((bounds.min.z - mOrigin.z) * invH); if (z0 < 1) z0 = 1;
	int z1 = (int)((bounds.max.z - mOrigin.z) * invH); if (z1 >= mNumZ-1) z1 = mNumZ - 2;

	int xi,yi,zi;

	for (xi = x0; xi <= x1; xi++) {
		for (yi = y0; yi <= y1; yi++) {
			for (zi = z0; zi <= z1; zi++) {
				StufferCell &cell = cellAt(xi,yi,zi);
				m3Real dist, density;
				dist = Geometry3D::trianglePointDist(p0, p1, p2, cell.cornerPos);
				if (dist <= mThickness) {
					density = 1.0f - dist * invT - mIsoValue;
					cell.cornerDensity = m3Max(cell.cornerDensity, density);
				}
				dist = Geometry3D::trianglePointDist(p0, p1, p2, cell.centerPos);
				if (dist <= mThickness) {
					density = 1.0f - dist * invT - mIsoValue;
					cell.centerDensity = m3Max(cell.centerDensity, density);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------
int TetraStuffer::getCornerEdges(int xi, int yi, int zi, StufferEdge edges[])
{
	int numEdges = 0; // maximal 14 adjacent edges

	StufferCell &c = cellAt(xi,yi,zi);
	edges[numEdges++].init(&c.cornerCornerCutVertNr[0], &c.cornerCornerCutPos[0], true, xi+1,yi,zi, true);
	edges[numEdges++].init(&c.cornerCornerCutVertNr[1], &c.cornerCornerCutPos[1], true, xi,yi+1,zi, true);
	edges[numEdges++].init(&c.cornerCornerCutVertNr[2], &c.cornerCornerCutPos[2], true, xi,yi,zi+1, true);

	edges[numEdges++].init(&c.centerCornerCutVertNr[0], &c.centerCornerCutPos[0], true, xi,yi,zi, false); 

	StufferCell &cx = cellAt(xi-1,yi,zi);
	edges[numEdges++].init(&cx.cornerCornerCutVertNr[0], &cx.cornerCornerCutPos[0], false, xi-1,yi,zi, true); 
	edges[numEdges++].init(&cx.centerCornerCutVertNr[1], &cx.centerCornerCutPos[1], false, xi-1,yi,zi, false); 

	StufferCell &cy = cellAt(xi,yi-1,zi);
	edges[numEdges++].init(&cy.cornerCornerCutVertNr[1], &cy.cornerCornerCutPos[1], false, xi,yi-1,zi, true); 
	edges[numEdges++].init(&cy.centerCornerCutVertNr[3], &cy.centerCornerCutPos[3], false, xi,yi-1,zi, false); 

	StufferCell &cz = cellAt(xi,yi,zi-1);
	edges[numEdges++].init(&cz.cornerCornerCutVertNr[2], &cz.cornerCornerCutPos[2], false, xi,yi,zi-1, true); 
	edges[numEdges++].init(&cz.centerCornerCutVertNr[4], &cz.centerCornerCutPos[4], false, xi,yi,zi-1, false); 

	StufferCell &cxy = cellAt(xi-1,yi-1,zi);
	edges[numEdges++].init(&cxy.centerCornerCutVertNr[2], &cxy.centerCornerCutPos[2], false, xi-1,yi-1,zi, false); 

	StufferCell &cxz = cellAt(xi-1,yi,zi-1);
	edges[numEdges++].init(&cxz.centerCornerCutVertNr[5], &cxz.centerCornerCutPos[5], false, xi-1,yi,zi-1, false); 

	StufferCell &cyz = cellAt(xi,yi-1,zi-1);
	edges[numEdges++].init(&cyz.centerCornerCutVertNr[7], &cyz.centerCornerCutPos[7], false, xi,yi-1,zi-1, false); 

	StufferCell &cxyz = cellAt(xi-1,yi-1,zi-1);
	edges[numEdges++].init(&cxyz.centerCornerCutVertNr[6], &cxyz.centerCornerCutPos[6], false, xi-1,yi-1,zi-1, false); 

	return numEdges;
}

// ----------------------------------------------------------------------
int TetraStuffer::getCenterEdges(int xi, int yi, int zi, StufferEdge edges[])
{
	int numEdges = 0; // maximal 14 adjacent edges

	StufferCell &c = cellAt(xi,yi,zi);

	edges[numEdges++].init(&c.centerCornerCutVertNr[0], &c.centerCornerCutPos[0], true, xi,yi,zi, true); 
	edges[numEdges++].init(&c.centerCornerCutVertNr[1], &c.centerCornerCutPos[1], true, xi+1,yi,zi, true); 
	edges[numEdges++].init(&c.centerCornerCutVertNr[2], &c.centerCornerCutPos[2], true, xi+1,yi+1,zi, true); 
	edges[numEdges++].init(&c.centerCornerCutVertNr[3], &c.centerCornerCutPos[3], true, xi,yi+1,zi, true); 

	edges[numEdges++].init(&c.centerCornerCutVertNr[4], &c.centerCornerCutPos[4], true, xi,yi,zi+1, true); 
	edges[numEdges++].init(&c.centerCornerCutVertNr[5], &c.centerCornerCutPos[5], true, xi+1,yi,zi+1, true); 
	edges[numEdges++].init(&c.centerCornerCutVertNr[6], &c.centerCornerCutPos[6], true, xi+1,yi+1,zi+1, true); 
	edges[numEdges++].init(&c.centerCornerCutVertNr[7], &c.centerCornerCutPos[7], true, xi,yi+1,zi+1, true); 

	edges[numEdges++].init(&c.centerCenterCutVertNr[0], &c.centerCenterCutPos[0], true, xi+1,yi,zi, false); 
	edges[numEdges++].init(&c.centerCenterCutVertNr[1], &c.centerCenterCutPos[1], true, xi,yi+1,zi, false); 
	edges[numEdges++].init(&c.centerCenterCutVertNr[2], &c.centerCenterCutPos[2], true, xi,yi,zi+1, false); 

	StufferCell &cx = cellAt(xi-1,yi,zi);
	edges[numEdges++].init(&cx.centerCenterCutVertNr[0], &cx.centerCenterCutPos[0], false, xi-1,yi,zi, false); 

	StufferCell &cy = cellAt(xi,yi-1,zi);
	edges[numEdges++].init(&cy.centerCenterCutVertNr[1], &cy.centerCenterCutPos[1], false, xi,yi-1,zi, false); 

	StufferCell &cz = cellAt(xi,yi,zi-1);
	edges[numEdges++].init(&cz.centerCenterCutVertNr[2], &cz.centerCenterCutPos[2], false, xi,yi,zi-1, false); 
	return numEdges;
}

// ----------------------------------------------------------------------
bool TetraStuffer::interpolate(m3Real d0, m3Real d1, const m3Vector &pos0, const m3Vector &pos1, m3Vector &pos)
{
	if ((d0 < 0.0f && d1 < 0.0f) || (d0 > 0.0f && d1 > 0.0f))
		return false;

	m3Real s = d0 / (d0-d1);
	pos = pos0 * (1.0f - s) + pos1 * s;
	return true;
}

// ----------------------------------------------------------------------
void TetraStuffer::computeCutPoints()
{
	m3Vector cut;
	StufferEdge edges[14];
	int i,xi,yi,zi, numEdges;

	for (xi = 1; xi < mNumX-1; xi++) {
		for (yi = 1; yi < mNumY-1; yi++) {
			for (zi = 1; zi < mNumZ-1; zi++) {
				StufferCell &c = cellAt(xi,yi,zi);

				numEdges = getCornerEdges(xi,yi,zi, edges);
				for (i = 0; i < numEdges; i++) {
					StufferEdge &e = edges[i];
					if (!e.cutOwner) continue;
					StufferCell &ca = cellAt(e.adjX, e.adjY, e.adjZ);
					if (e.adjCorner) {
						if (interpolate(c.cornerDensity, ca.cornerDensity, c.cornerPos, ca.cornerPos, cut)) {
							*e.cutVertNr = 0;
							*e.cutPos = cut;
						}
					}
					else {
						if (interpolate(c.cornerDensity, ca.centerDensity, c.cornerPos, ca.centerPos, cut)) {
							*e.cutVertNr = 0;
							*e.cutPos = cut;
						}
					}
				}

				numEdges = getCenterEdges(xi,yi,zi, edges);
				for (i = 0; i < numEdges; i++) {
					StufferEdge &e = edges[i];
					if (!e.cutOwner) continue;
					StufferCell &ca = cellAt(e.adjX, e.adjY, e.adjZ);
					if (e.adjCorner) {
						if (interpolate(c.centerDensity, ca.cornerDensity, c.centerPos, ca.cornerPos, cut)) {
							*e.cutVertNr = 0;
							*e.cutPos = cut;
						}
					}
					else {
						if (interpolate(c.centerDensity, ca.centerDensity, c.centerPos, ca.centerPos, cut)) {
							*e.cutVertNr = 0;
							*e.cutPos = cut;
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------
void TetraStuffer::computeCuts(const Mesh &mesh, float x0, float y0, float z0, float x1, float y1, float z1, 
				int &vertNr, m3Vector &cutPos)
{
	// uses mItemIndices initialized by setupGridUsingGeometry ... 
	m3Vector orig(x0,y0,z0);
	m3Vector dir(x1,y1,z1); dir -= orig;
	float t,u,v;

	for (int i = 0; i < (int)mItemIndices.size(); i++) {
		const MeshTriangle &triangle = mesh.getTriangle(mItemIndices[i]);
		const m3Vector &p0 = mesh.getVertex(triangle.vertexNr[0]);
		const m3Vector &p1 = mesh.getVertex(triangle.vertexNr[1]);
		const m3Vector &p2 = mesh.getVertex(triangle.vertexNr[2]); 
		if (!Geometry3D::rayTriangleIntersection(orig, dir, p0,p1,p2, t,u,v))
			continue;
		if (t < 0.0f || t > 1.0f) continue;
		vertNr = 0;		// mark as used  (-1 is unused)
		cutPos = orig + dir * t;
		break;
	}
}

// ----------------------------------------------------------------------
void TetraStuffer::setupGridUsingGeometry(const Mesh &mesh)
{
	mHash.reset();
	mHash.setGridSpacing(mCellSize);

	int i;
	// hash triangles for fast access
	m3Bounds bounds;
	for (i = 0; i < mesh.getNumTriangles(); i++) {
		mesh.getTriangleBounds(i, bounds);
		mHash.add(bounds, i);
	}

	// compute grid edge - triangle cuts
	int xi,yi,zi, numEdges;

	for (xi = 1; xi < mNumX-1; xi++) {
		for (yi = 1; yi < mNumY-1; yi++) {
			for (zi = 1; zi < mNumZ-1; zi++) {
				StufferCell &c = cellAt(xi,yi,zi);
				c.centerDensity = 0.0f;
				c.cornerDensity = 0.0f;

				// get all triangles potentially cutting this cell's edges
				bounds.min = c.cornerPos;
				float h = mCellSize;
				float h2 = 0.5f * h;
				bounds.max = c.cornerPos + m3Vector(1.5f * h, 1.5f * h, 1.5f * h);
				bounds.fatten(0.01f * mCellSize);	// numerical safety
				mHash.query(bounds, mItemIndices);

				float x = c.cornerPos.x, y = c.cornerPos.y, z = c.cornerPos.z;
				float mx = x + h2, my = y + h2, mz = z + h2;

				computeCuts(mesh, x,y,z, x+h,y,z, c.cornerCornerCutVertNr[0], c.cornerCornerCutPos[0]);
				computeCuts(mesh, x,y,z, x,y+h,z, c.cornerCornerCutVertNr[1], c.cornerCornerCutPos[1]);
				computeCuts(mesh, x,y,z, x,y,z+h, c.cornerCornerCutVertNr[2], c.cornerCornerCutPos[2]);

				computeCuts(mesh, mx,my,mz, mx+h,my,mz, c.centerCenterCutVertNr[0], c.centerCenterCutPos[0]);
				computeCuts(mesh, mx,my,mz, mx,my+h,mz, c.centerCenterCutVertNr[1], c.centerCenterCutPos[1]);
				computeCuts(mesh, mx,my,mz, mx,my,mz+h, c.centerCenterCutVertNr[2], c.centerCenterCutPos[2]);

				computeCuts(mesh, mx,my,mz, x,  y,  z,   c.centerCornerCutVertNr[0], c.centerCornerCutPos[0]);
				computeCuts(mesh, mx,my,mz, x+h,y,  z,   c.centerCornerCutVertNr[1], c.centerCornerCutPos[1]);
				computeCuts(mesh, mx,my,mz, x+h,y+h,z,   c.centerCornerCutVertNr[2], c.centerCornerCutPos[2]);
				computeCuts(mesh, mx,my,mz, x,  y+h,z,   c.centerCornerCutVertNr[3], c.centerCornerCutPos[3]);
				computeCuts(mesh, mx,my,mz, x,  y,  z+h, c.centerCornerCutVertNr[4], c.centerCornerCutPos[4]);
				computeCuts(mesh, mx,my,mz, x+h,y,  z+h, c.centerCornerCutVertNr[5], c.centerCornerCutPos[5]);
				computeCuts(mesh, mx,my,mz, x+h,y+h,z+h, c.centerCornerCutVertNr[6], c.centerCornerCutPos[6]);
				computeCuts(mesh, mx,my,mz, x,  y+h,z+h, c.centerCornerCutVertNr[7], c.centerCornerCutPos[7]);
			}
		}
	}

	// determine inside / outside of grid nodes

	// compute along all axis, majority wins

	for (yi = 1; yi < mNumY-1; yi++) {
		for (zi = 1; zi < mNumZ-1; zi++) {
			float d0 = -1.0f;	// start outside
			float d1 = -1.0f;
			for (xi = 1; xi < mNumX-1; xi++) {
				StufferCell &c = cellAt(xi,yi,zi);
				c.cornerDensity += d0;
				c.centerDensity += d1;
				if (c.cornerCornerCutVertNr[0] >= 0) d0 = -d0;	
				if (c.centerCenterCutVertNr[0] >= 0) d1 = -d1;	
			}
		}
	}
	for (xi = 1; xi < mNumX-1; xi++) {
		for (zi = 1; zi < mNumZ-1; zi++) {
			float d0 = -1.0f;	// start outside
			float d1 = -1.0f;
			for (yi = 1; yi < mNumY-1; yi++) {
				StufferCell &c = cellAt(xi,yi,zi);
				c.cornerDensity += d0;
				c.centerDensity += d1;
				if (c.cornerCornerCutVertNr[1] >= 0) d0 = -d0;	
				if (c.centerCenterCutVertNr[1] >= 0) d1 = -d1;	
			}
		}
	}
	for (xi = 1; xi < mNumX-1; xi++) {
		for (yi = 1; yi < mNumY-1; yi++) {
			float d0 = -1.0f;	// start outside
			float d1 = -1.0f;
			for (zi = 1; zi < mNumZ-1; zi++) {
				StufferCell &c = cellAt(xi,yi,zi);
				c.cornerDensity += d0;
				c.centerDensity += d1;
				if (c.cornerCornerCutVertNr[2] >= 0) d0 = -d0;	
				if (c.centerCenterCutVertNr[2] >= 0) d1 = -d1;	
			}
		}
	}

	// remove cuts on edges with adjacent nodes of the same sign

	StufferEdge edges[14];

	for (xi = 1; xi < mNumX-1; xi++) {
		for (yi = 1; yi < mNumY-1; yi++) {
			for (zi = 1; zi < mNumZ-1; zi++) {
				StufferCell &c = cellAt(xi,yi,zi);

				numEdges = getCornerEdges(xi,yi,zi, edges);
				for (i = 0; i < numEdges; i++) {
					StufferEdge &e = edges[i];
					if (!e.cutOwner) continue;
					StufferCell &ca = cellAt(e.adjX, e.adjY, e.adjZ);
					if (e.adjCorner) {
						if (c.cornerDensity * ca.cornerDensity > 0.0f)
							*e.cutVertNr = -1;
					}
					else {
						if (c.cornerDensity * ca.centerDensity > 0.0f)
							*e.cutVertNr = -1;
					}
				}

				numEdges = getCenterEdges(xi,yi,zi, edges);
				for (i = 0; i < numEdges; i++) {
					StufferEdge &e = edges[i];
					if (!e.cutOwner) continue;
					StufferCell &ca = cellAt(e.adjX, e.adjY, e.adjZ);
					if (e.adjCorner) {
						if (c.centerDensity * ca.cornerDensity > 0.0f)
							*e.cutVertNr = -1;
					}
					else {
						if (c.centerDensity * ca.centerDensity > 0.0f)
							*e.cutVertNr = -1;
					}
				}
			}
		}
	}
			
}

// ----------------------------------------------------------------------
bool TetraStuffer::snapVertex(int xi, int yi, int zi, bool corner, bool testOnly)
{
	StufferEdge edges[14];
	int i, numEdges;

	StufferCell &cell = cellAt(xi,yi,zi);
	m3Vector pos;

	if (corner) {
		numEdges = getCornerEdges(xi,yi,zi, edges);
		pos = cell.cornerPos;
		if (cell.cornerDensity == 0.0f) return false;
	}
	else {
		numEdges = getCenterEdges(xi,yi,zi, edges);
		pos = cell.centerPos;
		if (cell.centerDensity == 0.0f) return false;
	}

	m3Real minLong2  = mCellSize * mAlphaLong; 
	m3Real minShort2 = mCellSize * 0.5f * m3Sqrt3 * mAlphaShort;
	minLong2  = minLong2 * minLong2;
	minShort2 = minShort2 * minShort2;

	m3Real minD2 = 0.0f;
	m3Vector minPos;
	int min = -1;
	
	for (i = 0; i < numEdges; i++) {
		StufferEdge &e = edges[i];
		if (*e.cutVertNr < 0) continue;
		m3Real d2 = pos.distanceSquared(*e.cutPos);
		if (corner != e.adjCorner && d2 > minShort2) continue;	// diagonal (short) edge
		if (corner == e.adjCorner && d2 > minLong2) continue;	// parallel (long) edge
		if (min < 0 || d2 < minD2) {
			min = i; minD2 = d2; minPos = *e.cutPos;
		}
	}

	if (min < 0) return false;	// no violations (cuts too close to vertex)
	if (testOnly) return true;	// report violation, don't fix it

	// snap grid vertex to fix violation
	if (corner) {
		cell.cornerPos = minPos;
		cell.cornerDensity = 0.0f;
	}
	else {
		cell.centerPos = minPos;
		cell.centerDensity = 0.0f;
	}

	for (i = 0; i < numEdges; i++) {
		*edges[i].cutVertNr = -1;
	}

	return true;
}

// ----------------------------------------------------------------------
void TetraStuffer::snapVertices()
{
	int xi,yi,zi;

/*
	Not implemented yet. According to the paper this should improve the tet quality

	// while some negative lattice point q- is violated by a cut point on an edge 
	// adjoining an unvioalted positive lattice point, 
	// warp q- to a violating cut point on such an edge.

	// while some opsitive lattice point q+ is violated 
	// warp q+ to a violating cut point
*/

	for (xi = 1; xi < mNumX-1; xi++) {
		for (yi = 1; yi < mNumY-1; yi++) {
			for (zi = 1; zi < mNumZ-1; zi++) {
				StufferCell &c = cellAt(xi,yi,zi);
				snapVertex(xi,yi,zi, true);
				snapVertex(xi,yi,zi, false);
			}
		}
	}
}

// ----------------------------------------------------------------------
float TetraStuffer::tetraVolume(const MeshTetrahedron &t)
{
	assert(0 <= t.vertexNr[0] && t.vertexNr[0] < (int)mVertices.size());
	assert(0 <= t.vertexNr[1] && t.vertexNr[1] < (int)mVertices.size());
	assert(0 <= t.vertexNr[2] && t.vertexNr[2] < (int)mVertices.size());
	assert(0 <= t.vertexNr[3] && t.vertexNr[3] < (int)mVertices.size());
	m3Vector d1 = mVertices[t.vertexNr[1]] - mVertices[t.vertexNr[0]];
	m3Vector d2 = mVertices[t.vertexNr[2]] - mVertices[t.vertexNr[0]];
	m3Vector d3 = mVertices[t.vertexNr[3]] - mVertices[t.vertexNr[0]];
	return d1.cross(d2).dot(d3);
}

// ----------------------------------------------------------------------
void TetraStuffer::addTetra(int p, float d, const StufferTriangle &triangle)
{
	if (d < 0.0f || triangle.d0 < 0.0f || triangle.d1 < 0.0f || triangle.d2 < 0.0f) return;
	MeshTetrahedron tetra;
	tetra.set(p, triangle.p0, triangle.p1, triangle.p2);
	if (tetraVolume(tetra) < 0.0f) {
		int v = tetra.vertexNr[0]; tetra.vertexNr[0] = tetra.vertexNr[1]; tetra.vertexNr[1] = v;
	}
	mTetrahedra.push_back(tetra);
}

// ----------------------------------------------------------------------
void TetraStuffer::addTriangle(int p0, int p1, int p2, float d0, float d1, float d2)
{
	mTriangles.push_back(StufferTriangle(p0,p1,p2, d0,d1,d2));
}

// ----------------------------------------------------------------------
void TetraStuffer::addTriangles(int p0, int p1, int p2, float d0, float d1, float d2,
								int c01, int c12, int c20)
{
	// long edge first

	int numCuts = 0;
	if (c01 >= 0) numCuts++;
	if (c12 >= 0) numCuts++;
	if (c20 >= 0) numCuts++;

	assert(numCuts < 3);	// 3 cuts on a triangle are not possible

	if (numCuts == 0) {
		mTriangles.push_back(StufferTriangle(p0,p1,p2, d0,d1,d2));
	}
	if (numCuts == 1) {
		if (c01 >= 0) {
			addTriangle(c01,p2,p0, 0.0f,d2,d0);
			addTriangle(c01,p1,p2, 0.0f,d1,d2);
		}
		if (c12 >= 0) {
			addTriangle(c12,p0,p1, 0.0f,d0,d1);
			addTriangle(c12,p2,p0, 0.0f,d2,d0);
		}
		if (c20 >= 0) {
			addTriangle(c20,p1,p2, 0.0f,d1,d2);
			addTriangle(c20,p0,p1, 0.0f,d0,d1);
		}
	}
	if (numCuts == 2) {
		if (c12 < 0) {		// split cut on long edge
			addTriangle(p0, c01,c20, d0,0.0f,0.0f);
			addTriangle(c01,p2, c20, 0.0f,d2,0.0f);
			addTriangle(c01,p1, p2,  0.0f,d1,d2);
		}
		if (c20 < 0) {		// split cut on long edge
			addTriangle(p0, c01,p2,  d0,0.0f,d2 );
			addTriangle(c01,c12,p2,  0.0f,0.0f,d2);
			addTriangle(c01,p1, c12, 0.0f,d1,0.0f);
		}
		if (c01 < 0) {		// ambiguous case
			addTriangle(c20,c12,p2, 0.0f,0.0f,d2);
			if (c20 > c12) {
				addTriangle(p0, c12,c20, d0,0.0f,0.0f);
				addTriangle(p0, p1, c12, d0,d1,0.0f);
			}
			else {
				addTriangle(p0, p1, c20, d0,d1,0.0f);
				addTriangle(p1, c12,c20, d1,0.0f,0.0f);
			}
		}
	}
}

// ----------------------------------------------------------------------
void TetraStuffer::handleTetra(int p0, float d0, int p1, float d1, int p2, float d2, int p3, float d3,
							   int c01, int c12, int c20, int c03, int c13, int c23)
{
	// early out
	if (d0 < 0.0f && d1 < 0.0f && d2 < 0.0f && d3 < 0.0f) 
		return;

	// long edges are 0-3 and 1-2

	// give negative vertices a unique temporary index
	if (p0 < 0) p0 = mVertices.size();
	if (p1 < 0) p1 = mVertices.size()+1;
	if (p2 < 0) p2 = mVertices.size()+2;
	if (p3 < 0) p3 = mVertices.size()+3;

	// add side triangles
	mTriangles.clear();
	addTriangles(p2,p1,p0, d2,d1,d0, c12,c01,c20);
	addTriangles(p3,p0,p1, d3,d0,d1, c03,c01,c13);
	addTriangles(p1,p2,p3, d1,d2,d3, c12,c23,c13);
	addTriangles(p0,p3,p2, d0,d3,d2, c03,c23,c20);

	// copy info to arrays
	int p[10];
	p[0] = p0; p[1] = p1; p[2] = p2; p[3] = p3;
	p[4] = c01; p[5] = c12; p[6] = c20; p[7] = c03; p[8] = c13; p[9] = c23;
	int numPoints = 10;

	int i,j;

	// remove non - existent points
	i = 0;
	while (i < numPoints) {
		if (p[i] < 0) {
			numPoints--;
			p[i] = p[numPoints];
		}
		else i++;
	}

	// create sub-tetras
	StufferVertexChain chain;
	StufferTriangle newTri, newTri2;

	while (numPoints > 3) {
		int min = -1, minAdj = 0;
		// search for a corner to chop off
		// first check only non-zero corners
		for (i = 0; i < numPoints; i++) {
			int numAdjTriangles = 0;
			bool zero = true;
			for (j = 0; j < (int)mTriangles.size(); j++) {
				if (mTriangles[j].contains(p[i])) {
					numAdjTriangles++;
					if (mTriangles[j].getDensityOf(p[i]) != 0.0f) zero = false;
				}
			}
			if (zero) continue;
			if (min < 0 || numAdjTriangles < minAdj) {
				minAdj = numAdjTriangles; min = i;
			}
		}
		// now check all corners
		for (i = 0; i < numPoints; i++) {
			int numAdjTriangles = 0;
			for (j = 0; j < (int)mTriangles.size(); j++) {
				if (mTriangles[j].contains(p[i])) numAdjTriangles++;
			}
			if (min < 0 || numAdjTriangles < minAdj) {
				minAdj = numAdjTriangles; min = i;
			}
		}

		// remove all adjacent triangles and create closing triangle(s)
		chain.clear();
		float d = 0.0f;
		i = 0;
		while (i < (int)mTriangles.size()) {
			StufferTriangle &t = mTriangles[i];
			if (t.contains(p[min])) {
				chain.addTriangle(t, p[min]);
				d = t.getDensityOf(p[min]);
				t = mTriangles[mTriangles.size()-1];
				mTriangles.pop_back();
			}
			else i++;
		}

		if (minAdj == 3) {
			chain.getTriangle(newTri);
			mTriangles.push_back(newTri);
			addTetra(p[min],d,newTri);
		}
		else if (minAdj == 4) {
			chain.getTriangles(c12, c03, newTri, newTri2);
			mTriangles.push_back(newTri);
			mTriangles.push_back(newTri2);
			addTetra(p[min],d, newTri);
			addTetra(p[min],d, newTri2);
		}
		p[min] = p[numPoints-1];
		numPoints--;
	}
}

// ----------------------------------------------------------------------
void TetraStuffer::triangulateGrid()
{
	mVertices.clear();
	mTetrahedra.clear();

	StufferEdge edges[14];
	int xi,yi,zi,i, numEdges;

	// create unique vertex numbers

	for (xi = 1; xi < mNumX-1; xi++) {
		for (yi = 1; yi < mNumY-1; yi++) {
			for (zi = 1; zi < mNumZ-1; zi++) {
				StufferCell &c = cellAt(xi,yi,zi);
				// corner
				if (c.cornerDensity >= 0.0f) {
					c.cornerVertexNr = mVertices.size();
					mVertices.push_back(c.cornerPos);
				}
				else c.cornerVertexNr = -1;
				numEdges = getCornerEdges(xi,yi,zi, edges);
				for (i = 0; i < numEdges; i++) {
					StufferEdge &e = edges[i];
					if (e.cutOwner && *e.cutVertNr >= 0) {
						*e.cutVertNr = mVertices.size();
						mVertices.push_back(*e.cutPos);
					}
				}

				// center
				if (c.centerDensity >= 0.0f) {
					c.centerVertexNr = mVertices.size();
					mVertices.push_back(c.centerPos);
				}
				else c.centerVertexNr = -1;
				numEdges = getCenterEdges(xi,yi,zi, edges);
				for (i = 0; i < numEdges; i++) {
					StufferEdge &e = edges[i];
					if (e.cutOwner && *e.cutVertNr >= 0) {
						*e.cutVertNr = mVertices.size();
						mVertices.push_back(*e.cutPos);
					}
				}
			}
		}
	}

	// enumerate tetrahedra

	for (xi = 1; xi < mNumX-1; xi++) {
		for (yi = 1; yi < mNumY-1; yi++) {
			for (zi = 1; zi < mNumZ-1; zi++) {
				StufferCell &c = cellAt(xi,yi,zi);
				int    p[4],p4,p5, e[4],f[4],g[4],h;
				m3Real d[4],d4,d5;
				p4 = cellAt(xi,yi,zi).centerVertexNr; d4 = cellAt(xi,yi,zi).centerDensity;

				// x direction
				p5  = cellAt(xi+1,yi,zi).centerVertexNr; d5 = cellAt(xi+1,yi,zi).centerDensity;

				p[0] = cellAt(xi+1,yi,  zi  ).cornerVertexNr; d[0] = cellAt(xi+1,yi,  zi  ).cornerDensity;
				p[1] = cellAt(xi+1,yi+1,zi  ).cornerVertexNr; d[1] = cellAt(xi+1,yi+1,zi  ).cornerDensity;
				p[2] = cellAt(xi+1,yi+1,zi+1).cornerVertexNr; d[2] = cellAt(xi+1,yi+1,zi+1).cornerDensity;
				p[3] = cellAt(xi+1,yi,  zi+1).cornerVertexNr; d[3] = cellAt(xi+1,yi,  zi+1).cornerDensity;

				e[0] = cellAt(xi+1,yi,  zi  ).cornerCornerCutVertNr[1];
				e[1] = cellAt(xi+1,yi+1,zi  ).cornerCornerCutVertNr[2];
				e[2] = cellAt(xi+1,yi,  zi+1).cornerCornerCutVertNr[1];
				e[3] = cellAt(xi+1,yi,  zi  ).cornerCornerCutVertNr[2];

				f[0] = cellAt(xi,  yi,  zi  ).centerCornerCutVertNr[1];
				f[1] = cellAt(xi,  yi,  zi  ).centerCornerCutVertNr[2];
				f[2] = cellAt(xi,  yi,  zi  ).centerCornerCutVertNr[6];
				f[3] = cellAt(xi,  yi,  zi  ).centerCornerCutVertNr[5];

				g[0] = cellAt(xi+1,yi,  zi  ).centerCornerCutVertNr[0];
				g[1] = cellAt(xi+1,yi,  zi  ).centerCornerCutVertNr[3];
				g[2] = cellAt(xi+1,yi,  zi  ).centerCornerCutVertNr[7];
				g[3] = cellAt(xi+1,yi,  zi  ).centerCornerCutVertNr[4];

				h    = cellAt(xi,  yi,  zi  ).centerCenterCutVertNr[0];

				for (i = 0; i < 4; i++) {
					int j = (i+1) % 4;
					handleTetra(p[i],d[i], p4,d4, p5,d5, p[j],d[j], f[i],h,g[i],e[i],f[j],g[j]);
				}

				// y direction
				p5  = cellAt(xi,yi+1,zi).centerVertexNr; d5 = cellAt(xi,yi+1,zi).centerDensity;

				p[0] = cellAt(xi+1,yi+1,zi  ).cornerVertexNr; d[0] = cellAt(xi+1,yi+1,zi  ).cornerDensity;
				p[1] = cellAt(xi  ,yi+1,zi  ).cornerVertexNr; d[1] = cellAt(xi,  yi+1,zi  ).cornerDensity;
				p[2] = cellAt(xi  ,yi+1,zi+1).cornerVertexNr; d[2] = cellAt(xi,  yi+1,zi+1).cornerDensity;
				p[3] = cellAt(xi+1,yi+1,zi+1).cornerVertexNr; d[3] = cellAt(xi+1,yi+1,zi+1).cornerDensity;

				e[0] = cellAt(xi,  yi+1,zi  ).cornerCornerCutVertNr[0];
				e[1] = cellAt(xi,  yi+1,zi  ).cornerCornerCutVertNr[2];
				e[2] = cellAt(xi,  yi+1,zi+1).cornerCornerCutVertNr[0];
				e[3] = cellAt(xi+1,yi+1,zi  ).cornerCornerCutVertNr[2];

				f[0] = cellAt(xi,  yi,  zi  ).centerCornerCutVertNr[2];
				f[1] = cellAt(xi,  yi,  zi  ).centerCornerCutVertNr[3];
				f[2] = cellAt(xi,  yi,  zi  ).centerCornerCutVertNr[7];
				f[3] = cellAt(xi,  yi,  zi  ).centerCornerCutVertNr[6];

				g[0] = cellAt(xi,  yi+1,zi  ).centerCornerCutVertNr[1];
				g[1] = cellAt(xi,  yi+1,zi  ).centerCornerCutVertNr[0];
				g[2] = cellAt(xi,  yi+1,zi  ).centerCornerCutVertNr[4];
				g[3] = cellAt(xi,  yi+1,zi  ).centerCornerCutVertNr[5];

				h    = cellAt(xi,  yi,  zi  ).centerCenterCutVertNr[1];

				for (i = 0; i < 4; i++) {
					int j = (i+1) % 4;
					handleTetra(p[i],d[i], p4,d4, p5,d5, p[j],d[j], f[i],h,g[i],e[i],f[j],g[j]);
				}

				// z direction
				p5  = cellAt(xi,  yi,zi+1).centerVertexNr; d5 = cellAt(xi,yi,zi+1).centerDensity;

				p[0] = cellAt(xi,  yi,  zi+1).cornerVertexNr; d[0] = cellAt(xi,  yi,  zi+1).cornerDensity;
				p[1] = cellAt(xi+1,yi,  zi+1).cornerVertexNr; d[1] = cellAt(xi+1,yi,  zi+1).cornerDensity;
				p[2] = cellAt(xi+1,yi+1,zi+1).cornerVertexNr; d[2] = cellAt(xi+1,yi+1,zi+1).cornerDensity;
				p[3] = cellAt(xi,  yi+1,zi+1).cornerVertexNr; d[3] = cellAt(xi,  yi+1,zi+1).cornerDensity;

				e[0] = cellAt(xi,  yi,  zi+1).cornerCornerCutVertNr[0];
				e[1] = cellAt(xi+1,yi,  zi+1).cornerCornerCutVertNr[1];
				e[2] = cellAt(xi,  yi+1,zi+1).cornerCornerCutVertNr[0];
				e[3] = cellAt(xi,  yi,  zi+1).cornerCornerCutVertNr[1];

				f[0] = cellAt(xi,  yi,  zi  ).centerCornerCutVertNr[4];
				f[1] = cellAt(xi,  yi,  zi  ).centerCornerCutVertNr[5];
				f[2] = cellAt(xi,  yi,  zi  ).centerCornerCutVertNr[6];
				f[3] = cellAt(xi,  yi,  zi  ).centerCornerCutVertNr[7];

				g[0] = cellAt(xi,  yi,  zi+1).centerCornerCutVertNr[0];
				g[1] = cellAt(xi,  yi,  zi+1).centerCornerCutVertNr[1];
				g[2] = cellAt(xi,  yi,  zi+1).centerCornerCutVertNr[2];
				g[3] = cellAt(xi,  yi,  zi+1).centerCornerCutVertNr[3];

				h    = cellAt(xi,  yi,  zi  ).centerCenterCutVertNr[2];

				for (i = 0; i < 4; i++) {
					int j = (i+1) % 4;
					handleTetra(p[i],d[i], p4,d4, p5,d5, p[j],d[j], f[i],h,g[i],e[i],f[j],g[j]);
				}
			}
		}
	}
}


