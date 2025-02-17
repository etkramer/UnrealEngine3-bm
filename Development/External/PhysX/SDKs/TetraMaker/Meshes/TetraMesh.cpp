#include "TetraMesh.h"
#include "TetraStuffer.h"
#include "Geometry3D.h"

#define EPSILON 1e-5f

const int MeshTetrahedron::sideIndices[4][3] = {{2,1,0},{0,1,3},{1,2,3},{2,0,3}};

// ----------------------------------------------------------------------
struct TetraEdge
{
	void init(int v0, int v1, int tetra) {
		tetraNr = tetra;
		if (v0 > v1) { vNr0 = v0; vNr1 = v1; }
		else { vNr0 = v1; vNr1 = v0; }
	}
	bool operator <(TetraEdge &e) const {
		if (vNr0 < e.vNr0) return true;
		if (vNr0 > e.vNr0) return false;
		return (vNr1 < e.vNr1);
	}
	bool operator ==(TetraEdge &e) const {
		return vNr0 == e.vNr0 && vNr1 == e.vNr1;
	}
	int vNr0, vNr1;
	int tetraNr;
};

// ----------------------------------------------------------------------
struct TetraEdgeList
{
	void add(TetraEdge &edge) { mEdges.push_back(edge); }
	void insert(int pos, TetraEdge &edge);
	int  numEdges() { return mEdges.size(); }
	void sort() { if (mEdges.size() > 0) sort(0, mEdges.size()-1); }
	void sort(int l, int r);
	int  findEdge(int v0, int v1);
	int  findEdgeTetra(int v0, int v1, int tetraNr);
	TetraEdge& operator[](int i) { return mEdges[i]; }

	std::vector<TetraEdge> mEdges;
};

void TetraEdgeList::insert(int pos, TetraEdge &edge)
{
	// slow O(n)
	assert(pos >= 0 && pos < (int)mEdges.size());
	mEdges.push_back(edge);
	for (int i = mEdges.size()-1; i > pos; i--)
		mEdges[i] = mEdges[i-1];
	mEdges[pos] = edge;
}

int TetraEdgeList::findEdge(int v0, int v1)
{
	TetraEdge edge; edge.init(v0, v1, -1);
	int l = 0;
	int r = mEdges.size()-1;
	while (l <= r) {
		int m = (l + r) / 2;
		if (mEdges[m] < edge) l = m+1;
		else if (edge < mEdges[m]) r = m-1;
		else {
			while (m > 0 && mEdges[m-1] == edge) m--;
			return m;
		}
	}
	return -1;
}

int  TetraEdgeList::findEdgeTetra(int v0, int v1, int tetraNr)
{
	int pos = findEdge(v0,v1);
	if (pos < 0) return -1;
	TetraEdge edge; edge.init(v0,v1,tetraNr);
	while (pos < (int)mEdges.size() && mEdges[pos] == edge) {
		if (mEdges[pos].tetraNr == tetraNr)
			return pos;
		pos++;
	}
	return -1;
}

void TetraEdgeList::sort(int l, int r)
{
	int i,j, mi;
	TetraEdge k, m;

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
	if (l < j) sort(l, j);
	if (i < r) sort(i, r);
}

// ----------------------------------------------------------------------
TetraMesh::TetraMesh()
{
	mTetraStuffer = new TetraStuffer(mVertices, mTetrahedra);
	clear();
}

// ----------------------------------------------------------------------
TetraMesh::~TetraMesh()
{
	delete mTetraStuffer;
}

// ----------------------------------------------------------------------
void TetraMesh::clear()
{
	mVertices.clear();
	mTetrahedra.clear();
	mVertexLinks.clear();
	mHash.reset();
	mFirstFarVertex = -1;
	mLastFarVertex  = -1;
	mUndos.clear();
	mPercentage = 0;
	mCancel = false;
}

// ----------------------------------------------------------------------
void TetraMesh::draw(RendererColor baseColor, bool wireframe, m3Real xClipping)
{
	Renderer *renderer = Renderer::getInstance();
	int i;

	renderer->wireframeMode(false);

	m3Bounds bounds, vBounds;
	getBounds(bounds);
	m3Real d = 0.001f * bounds.min.distance(bounds.max);
	m3Vector b(d,d,d);
	for (i = 0; i < (int)mVertices.size(); i++) {
		vBounds.min = mVertices[i] - b;
		vBounds.max = mVertices[i] + b;
		renderer->drawBox(vBounds, 0xFF0000);
	}

	if (wireframe)
		renderer->wireframeMode(true);

	m3Real xLimit = bounds.min.x + xClipping * (bounds.max.x - bounds.min.x);
	m3Vector p0,p1,p2,p3,center;

	for (i = 0; i < (int)mTetrahedra.size(); i++) {
		MeshTetrahedron &tetra = mTetrahedra[i];
		p0 = mVertices[tetra.vertexNr[0]]; center = p0;
		p1 = mVertices[tetra.vertexNr[1]]; center += p1;
		p2 = mVertices[tetra.vertexNr[2]]; center += p2;
		p3 = mVertices[tetra.vertexNr[3]]; center += p3;
		center *= 0.25f;
		if (center.x > xLimit)
			renderer->drawTetrahedron(p0,p1,p2,p3, 0.9f, baseColor);
	}

	if (wireframe)
		renderer->wireframeMode(false);
}

// ----------------------------------------------------------------------
int TetraMesh::getPercentage() const
{
	return mPercentage;
}

// ----------------------------------------------------------------------
void TetraMesh::setPercentage(int percentage)
{
	mPercentage = percentage;
}

// ----------------------------------------------------------------------
void TetraMesh::cancelOperation()
{
	mCancel = true;
}

// ----------------------------------------------------------------------
const MeshTetrahedron& TetraMesh::getTetrahedron(int i) const
{
	assert(0 <= i && i <= (int)mTetrahedra.size());
	return mTetrahedra[i];
}

// ----------------------------------------------------------------------
const m3Vector& TetraMesh::getVertex(int i) const
{
	assert(0 <= i && i <= (int)mVertices.size());
	return mVertices[i];
}


// ----------------------------------------------------------------------
m3Real TetraMesh::getTetraVolume(const MeshTetrahedron &tetra) const
{
	m3Vector v0 = mVertices[tetra.vertexNr[0]];
	m3Vector v1 = mVertices[tetra.vertexNr[1]] - v0;
	m3Vector v2 = mVertices[tetra.vertexNr[2]] - v0;
	m3Vector v3 = mVertices[tetra.vertexNr[3]] - v0;
	return v3.dot(v1.cross(v2)) / 6.0f;
}

// ----------------------------------------------------------------------
m3Real TetraMesh::getTetraVolume(int v0, int v1, int v2, int v3) const
{
	MeshTetrahedron t;
	t.set(v0,v1,v2,v3);
	return getTetraVolume(t);
}

// ----------------------------------------------------------------------
m3Real TetraMesh::getTetraQuality(const MeshTetrahedron &tetra) const
{
	m3Real e = getTetraLongestEdge(tetra);
	if (e == 0.0f) return 0.0f;
	// for regular tetrahedron vol * 6 * sqrt(2) = s^3 -> quality = 1.0
	return m3Abs(getTetraVolume(tetra)) * 6.0f * m3Sqrt2 / (e*e*e);
}

// ----------------------------------------------------------------------
bool TetraMesh::pointInTetra(const MeshTetrahedron &tetra, const m3Vector &p) const
{
	return Geometry3D::pointInTetra(
		mVertices[tetra.vertexNr[0]], mVertices[tetra.vertexNr[1]], 
		mVertices[tetra.vertexNr[2]], mVertices[tetra.vertexNr[3]], p);
}

// ----------------------------------------------------------------------
m3Real TetraMesh::getTetraLongestEdge(const MeshTetrahedron &tetra) const
{
	const m3Vector &v0 = mVertices[tetra.vertexNr[0]];
	const m3Vector &v1 = mVertices[tetra.vertexNr[1]];
	const m3Vector &v2 = mVertices[tetra.vertexNr[2]];
	const m3Vector &v3 = mVertices[tetra.vertexNr[3]];
	m3Real max = v0.distanceSquared(v1);
	max = m3Max(max, v0.distanceSquared(v2)); 
	max = m3Max(max, v0.distanceSquared(v3)); 
	max = m3Max(max, v1.distanceSquared(v2)); 
	max = m3Max(max, v1.distanceSquared(v3)); 
	max = m3Max(max, v2.distanceSquared(v3)); 
	return sqrtf(max);
}

// ----------------------------------------------------------------------
int TetraMesh::closestVertex(const m3Vector &pos, m3Real &distance) const
{
	if (mVertices.size() == 0) return -1;
	m3Vector d;
	m3Real minD2 = -1.0f;
	int closest = -1;
	for (int i = 0; i < (int)mVertices.size(); i++) {
		d = mVertices[i] - pos;
		m3Real d2 = d.magnitudeSquared();
		if (i == 0 || d2 < minD2) {
			minD2 = d2; closest = i;
		}
	}
	distance = m3Sqrt(minD2);
	return closest;
}

// ----------------------------------------------------------------------
int TetraMesh::closestTetrahedron(const m3Vector &pos, m3Real &distance) const
{
	if (mVertices.size() == 0) return -1;
	m3Vector closest;
	m3Real minD2 = -1.0f;
	int closestTetra = -1;
	int i,j;

	for (i = 0; i < (int)mTetrahedra.size(); i++) {
		const MeshTetrahedron &tetra = mTetrahedra[i];

		for (j = 0; j < 4; j++) {
			const int *sj = MeshTetrahedron::sideIndices[j];
			const m3Vector &p0 = mVertices[tetra.vertexNr[sj[0]]];
			const m3Vector &p1 = mVertices[tetra.vertexNr[sj[1]]];
			const m3Vector &p2 = mVertices[tetra.vertexNr[sj[2]]];

			Geometry3D::closestPointOnTriangle(p0,p1,p2, pos, closest);
			m3Real d2 = closest.distanceSquared(pos);
			if (minD2 < 0.0f || d2 < minD2) {
				minD2 = d2; closestTetra = i;
			}
		}
	}
	distance = m3Sqrt(minD2);
	return closestTetra;
}

// ----------------------------------------------------------------------
void quickSort(std::vector<m3Real> &realVector, int l, int r)
{
	int i,j, mi;
	m3Real k, m;

	i = l; j = r; mi = (l + r)/2;
	m = realVector[mi];
	while (i <= j) {
		while(realVector[i] < m) i++;
		while(m < realVector[j]) j--;
		if (i <= j) {
			k = realVector[i]; realVector[i] = realVector[j]; realVector[j] = k;
			i++; j--;
		}
	}
	if (l < j) quickSort(realVector, l, j);
	if (i < r) quickSort(realVector, i, r);
}

// ----------------------------------------------------------------------
void TetraMesh::generateAdditionalVertices(const Mesh &mesh, bool innerOnly, int subdivision)
{
	int i;
	m3Bounds bounds, rayBounds, triBounds;
	m3Vector v0,v1,v2;
	m3Vector dir(1.0f, 0.0f, 0.0f);
	m3Real t,u,v;

	mesh.getBounds(bounds);
	m3Real diagonal = bounds.min.distance(bounds.max);
	bounds.fatten(EPSILON * diagonal);
	m3Real h = diagonal / subdivision * 2.0f;
	m3Real invH = 1.0f / h;

	int firstVertex = mVertices.size();

	if (innerOnly) {
		std::vector<m3Real> cuts;

		m3Vector origin = bounds.min;
		m3Vector p, newV;
		int numY = (int)((bounds.max.y - origin.y) * invH) + 1;
		int numZ = (int)((bounds.max.z - origin.z) * invH) + 1;

		int xi,yi,zi;
		p.x = origin.x - EPSILON;
		for (yi = 0; yi < numY; yi++) {
			p.y = origin.y + yi * h;
			for (zi = 0; zi < numZ; zi++) {
				if (mCancel) return;
				p.z = origin.z + zi * h;
				rayBounds.min = p;
				rayBounds.max = p;
				rayBounds.max.x = bounds.max.x + EPSILON;
				mHash.queryUnique(rayBounds, mItemIndices);
				if (mItemIndices.size() == 0) continue;
				cuts.clear();
				for (i = 0; i < (int)mItemIndices.size(); i++) {
					const MeshTriangle &mt = mesh.getTriangle(mItemIndices[i]);
					mesh.getTriangleBounds(mItemIndices[i], triBounds);
					if (p.y < triBounds.min.y || p.y > triBounds.max.y) continue;
					if (p.z < triBounds.min.z || p.z > triBounds.max.z) continue;
					v0 = mesh.getVertex(mt.vertexNr[0]);
					v1 = mesh.getVertex(mt.vertexNr[1]);
					v2 = mesh.getVertex(mt.vertexNr[2]);
					if (!Geometry3D::rayTriangleIntersection(p, dir, v0,v1,v2, t, u,v))
						continue;
					cuts.push_back(t);
				}
				if (cuts.size() == 0) continue;
				quickSort(cuts, 0, cuts.size()-1);
				for (i = 0; i < (int)cuts.size()-1; i += 2) {
					int x0 = (int)(cuts[i] * invH) + 1;
					int x1 = (int)(cuts[i+1] * invH);
					for (xi = x0; xi <= x1; xi++) {
						newV = p;
						newV.x = origin.x + xi * h;
						newV.disturb(1.0e-4f);
						mVertices.push_back(newV);
					}
				}
			}
		}
	}
	else {
		int numX = (int)((bounds.max.x - bounds.min.x) * invH) + 1;
		int numY = (int)((bounds.max.y - bounds.min.y) * invH) + 1;
		int numZ = (int)((bounds.max.z - bounds.min.z) * invH) + 1;
		int xi,yi,zi;
		m3Vector newV;
		for (xi = 0; xi < numX; xi++) {
			newV.x = bounds.min.x + xi * h;
			for (yi = 0; yi < numY; yi++) {
				newV.y = bounds.min.y + yi * h;
				for (zi = 0; zi < numZ; zi++) {
					newV.z = bounds.min.z + zi * h;
					newV.disturb(1.0e-4f);
					mVertices.push_back(newV);
				}
			}
		}
	}
	relaxAdditionalVertices(mesh, firstVertex, h);
}

// ----------------------------------------------------------------------
void TetraMesh::relaxAdditionalVertices(const Mesh &mesh, int firstVertex, float minDist)
{
	const int numIterations = 5;
	int iters, i,j;

	MeshHash vertexHash;
	m3Real h = minDist;
	m3Real h2 = h*h;
	vertexHash.setGridSpacing(h);
	m3Vector box(h,h,h);
	m3Vector closest,dist;
	m3Bounds vertBounds;

	for (iters = 0; iters < numIterations; iters++) {
		if (mCancel) return;
		vertexHash.reset();
		for (i = firstVertex; i < (int)mVertices.size(); i++)
			vertexHash.add(mVertices[i], i);

		// push away from the surface
		for (i = firstVertex; i < (int)mVertices.size(); i++) {
			m3Vector &pos = mVertices[i];
			vertBounds.min = pos-box;
			vertBounds.max = pos+box;
			mHash.queryUnique(vertBounds, mItemIndices);
			for (j = 0; j < (int)mItemIndices.size(); j++) {
				const MeshTriangle &mt = mesh.getTriangle(mItemIndices[j]);
				const m3Vector &p0 = mesh.getVertex(mt.vertexNr[0]);
				const m3Vector &p1 = mesh.getVertex(mt.vertexNr[1]);
				const m3Vector &p2 = mesh.getVertex(mt.vertexNr[2]);
				Geometry3D::closestPointOnTriangle(p0,p1,p2, pos, closest);
				dist = pos - closest;
				if (dist.magnitudeSquared() > h2)
					continue;
				dist.normalize();
				pos = closest + dist * h;
			}

			// push away from other points
			vertBounds.min = pos-box;
			vertBounds.max = pos+box;
			vertexHash.queryUnique(vertBounds, mItemIndices);
			for (j = 0; j < (int)mItemIndices.size(); j++) {
				if (mItemIndices[j] == i) continue;
				closest = mVertices[mItemIndices[j]];
				dist = pos - closest;
				if (dist.magnitudeSquared() > h2)
					continue;
				dist.normalize();
				pos = closest + dist * h;
			}
		}
	}
}

// ----------------------------------------------------------------------
void TetraMesh::delaunayTetrahedralization(const m3Bounds &bounds)
{
	int i,j,k;

	// start with huge tetrahedron
	mTetrahedra.clear();
	m3Real a = 3.0f * bounds.min.distance(bounds.max);
	m3Real x  = 0.5f * a;
	m3Real y0 = x / m3Sqrt(3.0f);
	m3Real y1 = x * m3Sqrt(3.0f) - y0;
	m3Real z0 = 0.25f * m3Sqrt(6.0f) * a;
	m3Real z1 = a * m3Sqrt(6.0f) / 3.0f - z0;

	m3Vector center; bounds.getCenter(center);

	mFirstFarVertex = mVertices.size();
	m3Vector p0(-x,-y0,-z1); mVertices.push_back(center + p0);
	m3Vector p1( x,-y0,-z1); mVertices.push_back(center + p1);
	m3Vector p2( 0, y1,-z1); mVertices.push_back(center + p2);
	m3Vector p3( 0,  0, z0); mVertices.push_back(center + p3);
	mLastFarVertex = mVertices.size()-1;

	MeshTetrahedron tetra;
	tetra.set(mFirstFarVertex, mFirstFarVertex+1, mFirstFarVertex+2, mFirstFarVertex+3);
	Geometry3D::circumSphere(center+p0, center+p1, center+p2, center+p3, tetra.center, tetra.radiusSquared);
	mTetrahedra.push_back(tetra);

	// build Delaunay triangulation iteratively
	std::vector<MeshTetraFace> faces;

	int lastVertex = (int)mVertices.size()-4;
	for (i = 0; i < lastVertex; i++) {
		if (mCancel) return;
		m3Vector &v = mVertices[i];

		faces.clear();
		// find tetrahedra whose circumsphere contains the new vertex
		// delete them and collect the surface of their union in faces
		j = 0;
		while (j < (int)mTetrahedra.size()) {
			MeshTetrahedron &mt = mTetrahedra[j];
			if (mt.center.distanceSquared(v) >= mt.radiusSquared) {
				j++; continue;
			}
			MeshTetraFace f0; f0.set(mt.vertexNr[2], mt.vertexNr[1], mt.vertexNr[0]);
			MeshTetraFace f1; f1.set(mt.vertexNr[0], mt.vertexNr[1], mt.vertexNr[3]);
			MeshTetraFace f2; f2.set(mt.vertexNr[1], mt.vertexNr[2], mt.vertexNr[3]);
			MeshTetraFace f3; f3.set(mt.vertexNr[2], mt.vertexNr[0], mt.vertexNr[3]);
			bool found0 = false;
			bool found1 = false;
			bool found2 = false;
			bool found3 = false;
			k = 0;
			while (k < (int)faces.size()) {
				MeshTetraFace &f = faces[k];
				bool found = false;
				if (f == f0) { found0 = true; found = true; }
				if (f == f1) { found1 = true; found = true; }
				if (f == f2) { found2 = true; found = true; }
				if (f == f3) { found3 = true; found = true; }
				if (found) {
					faces[k] = faces[faces.size()-1];
					faces.pop_back();
				}
				else k++;
			}
			if (!found0) faces.push_back(f0);
			if (!found1) faces.push_back(f1);
			if (!found2) faces.push_back(f2);
			if (!found3) faces.push_back(f3);
			mTetrahedra[j] = mTetrahedra[mTetrahedra.size()-1];
			mTetrahedra.pop_back();
		}
		// create new tetrahedra from all faces to the new vertex
		for (j = 0; j < (int)faces.size(); j++) {
			MeshTetraFace &f = faces[j];
			tetra.set(f.vertexNr[2], f.vertexNr[1], f.vertexNr[0], i);
			Geometry3D::circumSphere(
				mVertices[tetra.vertexNr[0]],
				mVertices[tetra.vertexNr[1]],
				mVertices[tetra.vertexNr[2]],
				mVertices[tetra.vertexNr[3]],
				tetra.center, tetra.radiusSquared);
			mTetrahedra.push_back(tetra);
		}
	}
}

// ----------------------------------------------------------------------
void TetraMesh::splitTetrahedra(const Mesh &mesh)
{
	int i,j,k;
	m3Vector v0,v1;
	m3Vector p0,p1,p2, n0,n1;
	m3Bounds triBounds;

	TetraEdgeList edges;
	TetraEdge edge;
	for (i = 0; i < (int)mTetrahedra.size(); i++) {
		MeshTetrahedron &t = mTetrahedra[i];
		edge.init(t.vertexNr[0], t.vertexNr[1], i); edges.add(edge);
		edge.init(t.vertexNr[1], t.vertexNr[2], i); edges.add(edge);
		edge.init(t.vertexNr[2], t.vertexNr[0], i); edges.add(edge);
		edge.init(t.vertexNr[0], t.vertexNr[3], i); edges.add(edge);
		edge.init(t.vertexNr[1], t.vertexNr[3], i); edges.add(edge);
		edge.init(t.vertexNr[2], t.vertexNr[3], i); edges.add(edge);
	}
	edges.sort();

	i = 0;
	m3Bounds edgeBounds;
	int lastOriginalVertex = mVertices.size()-1;

	while (i < edges.numEdges()) {
		if (mCancel) return;
		TetraEdge edge = edges[i];
		int vNr0 = edge.vNr0;
		int vNr1 = edge.vNr1;
		j = i+1;
		while (j < edges.numEdges() && edges[j] == edge) 
			j++;

		if (isFarVertex(vNr0) || isFarVertex(vNr1) || 
				vNr0 > lastOriginalVertex || vNr1 > lastOriginalVertex) {
			i = j;
			continue;
		}
		v0 = mVertices[vNr0];
		v1 = mVertices[vNr1];
		edgeBounds.setEmpty();
		edgeBounds.include(v0);
		edgeBounds.include(v1);
		mHash.queryUnique(edgeBounds, mItemIndices);
		if (mItemIndices.size() == 0) {
			i = j;
			continue;
		}
		bool cut = false;
		for (k = 0; k < (int)mItemIndices.size(); k++) {
			const MeshTriangle &triangle = mesh.getTriangle(mItemIndices[k]);
			if (triangle.containsVertexNr(vNr0) || triangle.containsVertexNr(vNr1))
				continue;
			mesh.getTriangleBounds(mItemIndices[k], triBounds);
			if (!triBounds.intersects(edgeBounds))
				continue;
			float t,u,v;
			p0 = mesh.getVertex(triangle.vertexNr[0]);
			p1 = mesh.getVertex(triangle.vertexNr[1]);
			p2 = mesh.getVertex(triangle.vertexNr[2]);
			n0.cross(p1-p0, p2-p0); n0.normalize();
			n1 = v1-v0; 
			m3Real len = n1.normalize();
			if (len == 0.0f) 
				continue;
			if (m3Abs(n0.dot(n1)) < 0.1f)
				continue;
			if (!Geometry3D::rayTriangleIntersection(v0, n1, p0,p1,p2, t,u,v))
				continue;
			t /= len;
			if (t < 0.1f || t > 0.9f) continue;
			mVertices.push_back(v0 * (1.0f - t) + v1 * t);
			cut = true;
			break;
		}
		if (!cut) {
			i = j;
			continue;
		}
		int vNewNr = mVertices.size()-1;
		int firstNewEdge = edges.numEdges() - 1;
		while (i < j) {	
			int tetraNr = edges[i].tetraNr;
			int vNr2, vNr3;
			mTetrahedra[tetraNr].getOppositeVertices(vNr0,vNr1, vNr2,vNr3);

			// split tetrahedron
			MeshTetrahedron newTetra = mTetrahedra[tetraNr];
			mTetrahedra[tetraNr].replaceVertex(vNr1, vNewNr);
			newTetra.replaceVertex(vNr0, vNewNr);
			mTetrahedra.push_back(newTetra);
			int newTetraNr = mTetrahedra.size()-1;

			// update edge list
			edge.init(vNr2,vNr3, newTetraNr);
			int pos = edges.findEdge(vNr2, vNr3); assert(pos >= 0);

			edges.insert(pos, edge);
			firstNewEdge++;
			if (pos < i) { i++; j++; }

			pos = edges.findEdgeTetra(vNr1,vNr2, tetraNr); assert(pos >= 0);
			edges[pos].tetraNr = newTetraNr;
			pos = edges.findEdgeTetra(vNr1,vNr3, tetraNr); assert(pos >= 0);
			edges[pos].tetraNr = newTetraNr;
			edge.init(vNewNr, vNr2, tetraNr); edges.add(edge);
			edge.init(vNewNr, vNr3, tetraNr); edges.add(edge);
			edge.init(vNewNr, vNr2, newTetraNr); edges.add(edge);
			edge.init(vNewNr, vNr3, newTetraNr); edges.add(edge);
			edge.init(vNewNr, vNr0, tetraNr); edges.add(edge);
			edge.init(vNewNr, vNr1, newTetraNr); edges.add(edge);
			i++;
		}
		edges.sort(firstNewEdge, edges.numEdges()-1);
	}
}

// ----------------------------------------------------------------------
void TetraMesh::swapTetrahedra(const Mesh &mesh, int subdivision)
{
	m3Bounds bounds;
	mesh.getBounds(bounds);
	m3Real diagonal = bounds.min.distance(bounds.max);
	m3Real threshold = 0.05f * diagonal / subdivision;

	int i,k;
	m3Vector v0,v1;
	m3Vector p0,p1,p2, n, vCut;
	m3Bounds triBounds;

	TetraEdgeList edges;
	TetraEdge edge;

	for (i = 0; i < (int)mTetrahedra.size(); i++) {
		MeshTetrahedron &t = mTetrahedra[i];
		edge.init(t.vertexNr[0], t.vertexNr[1], i); edges.add(edge);
		edge.init(t.vertexNr[1], t.vertexNr[2], i); edges.add(edge);
		edge.init(t.vertexNr[2], t.vertexNr[0], i); edges.add(edge);
		edge.init(t.vertexNr[0], t.vertexNr[3], i); edges.add(edge);
		edge.init(t.vertexNr[1], t.vertexNr[3], i); edges.add(edge);
		edge.init(t.vertexNr[2], t.vertexNr[3], i); edges.add(edge);
	}
	edges.sort();

	i = 0;
	m3Bounds edgeBounds;

	while (i < edges.numEdges()) {
		if (mCancel) return;
		TetraEdge edge = edges[i];
		int vNr0 = edge.vNr0;
		int vNr1 = edge.vNr1;	

		i++;
		while (i < edges.numEdges() && edges[i] == edge) 
			i++;

		if (isFarVertex(vNr0) || isFarVertex(vNr1))
			continue;

		v0 = mVertices[vNr0];
		v1 = mVertices[vNr1];
		edgeBounds.setEmpty();
		edgeBounds.include(v0);
		edgeBounds.include(v1);
		mHash.queryUnique(edgeBounds, mItemIndices);
		if (mItemIndices.size() == 0) 
			continue;

		bool cut = false;
		for (k = 0; k < (int)mItemIndices.size(); k++) {
			const MeshTriangle &triangle = mesh.getTriangle(mItemIndices[k]);
			if (triangle.containsVertexNr(vNr0) || triangle.containsVertexNr(vNr1))
				continue;
			mesh.getTriangleBounds(mItemIndices[k], triBounds);
			if (!triBounds.intersects(edgeBounds))
				continue;
			float t,u,v;
			p0 = mesh.getVertex(triangle.vertexNr[0]);
			p1 = mesh.getVertex(triangle.vertexNr[1]);
			p2 = mesh.getVertex(triangle.vertexNr[2]);

			n.cross(p1-p0, p2-p0); n.normalize();

			if (m3Abs(n.dot(v0-p0)) < threshold)
				continue;
			if (m3Abs(n.dot(v1-p0)) < threshold)
				continue;

			if (!Geometry3D::rayTriangleIntersection(v0, v1-v0, p0,p1,p2, t,u,v))
				continue;
			cut = true;
			break;
		}
		if (cut) {
			swapEdge(vNr0, vNr1);
		}
	}
}

// ----------------------------------------------------------------------
bool TetraMesh::swapEdge(int v0, int v1)
{
	int i;

	std::vector<int> borderEdges;
	std::vector<MeshTetrahedron> newTetras;

	for (i = 0; i < (int)mTetrahedra.size(); i++) {
		MeshTetrahedron &t = mTetrahedra[i];
		t.flags &= ~TETRAHEDRON_DELETE;
		if (!t.containsVertex(v0) || !t.containsVertex(v1))
			continue;
		t.flags |= TETRAHEDRON_DELETE;
		int v2,v3;
		t.getOppositeVertices(v0,v1, v2,v3);
		if (getTetraVolume(v0,v1,v2,v3) >= 0.0f) {
			borderEdges.push_back(v2);
			borderEdges.push_back(v3);
		}
		else {
			borderEdges.push_back(v3);
			borderEdges.push_back(v2);
		}
	}

	if (borderEdges.size() < 6) return false;

	// start with first edge
	std::vector<int> borderVertices;
	std::vector<m3Real> borderQuality;
	borderVertices.push_back(borderEdges[borderEdges.size()-2]);
	borderVertices.push_back(borderEdges[borderEdges.size()-1]);
	borderEdges.pop_back();
	borderEdges.pop_back();

	// construct border
	int vEnd = borderVertices[1];
	while (borderEdges.size() > 0) {
		int num = borderEdges.size();
		for (i = 0; i < num-1; i += 2) {
			if (borderEdges[i] == vEnd) { vEnd = borderEdges[i+1]; break; }
		}
		// not connected
		if (i >= num) return false;
		borderVertices.push_back(vEnd);
		borderEdges[i] = borderEdges[num-2];
		borderEdges[i+1] = borderEdges[num-1];
		borderEdges.pop_back();
		borderEdges.pop_back();
	}

	// not circular
	if (borderVertices[0] != borderVertices[borderVertices.size()-1])
		return false;
	borderVertices.pop_back();

	if (borderVertices.size() < 3) return false;

	// generate tetrahedra
	MeshTetrahedron tetra0, tetra1;
	borderQuality.resize(borderVertices.size());
	while (borderVertices.size() > 3) {
		int num = borderVertices.size();
		int i0,i1,i2;
		for (i0 = 0; i0 < num; i0++) {
			i1 = (i0+1) % num;
			i2 = (i1+1) % num;
			tetra0.set(borderVertices[i0], borderVertices[i1], borderVertices[i2], v1);
			borderQuality[i1] = getTetraQuality(tetra0);
		}
		m3Real maxQ = 0.0f;
		int maxI0 = -1;
		for (i0 = 0; i0 < num; i0++) {
			i1 = (i0+1) % num;
			i2 = (i1+1) % num;
			tetra0.set(borderVertices[i0], borderVertices[i1], borderVertices[i2],v1);
			tetra1.set(borderVertices[i2], borderVertices[i1], borderVertices[i0],v0);
			if (getTetraVolume(tetra0) < 0.0f || getTetraVolume(tetra1) < 0.0f) 
				continue;
			bool ear = true;
			for (i = 0; i < num; i++) {
				if (i == i0 || i == i1 || i == i2) continue;
				m3Vector &pos = mVertices[borderVertices[i]];
				if (pointInTetra(tetra0, pos) || pointInTetra(tetra1, pos))
					ear = false;
			}
			if (!ear)
				continue;
			m3Real q = (1.0f - borderQuality[i0]) + borderQuality[i1] + (1.0f - borderQuality[i2]);
			if (maxI0 < 0 || q > maxQ) {
				maxQ = q; maxI0 = i0;
			}
		}
		if (maxI0 < 0) return false;
		i0 = maxI0;
		i1 = (i0+1) % num;
		i2 = (i1+1) % num;
		tetra0.set(borderVertices[i0], borderVertices[i1], borderVertices[i2],v1);
		tetra1.set(borderVertices[i2], borderVertices[i1], borderVertices[i0],v0);

		// add tetras, remove vertex;
		newTetras.push_back(tetra0);
		newTetras.push_back(tetra1);
		for (i = i1; i < num-1; i++) 
			borderVertices[i] = borderVertices[i+1];
		borderVertices.pop_back();
	}
	tetra0.set(borderVertices[0], borderVertices[1], borderVertices[2], v1);
	tetra1.set(borderVertices[2], borderVertices[1], borderVertices[0], v0);

	if (getTetraVolume(tetra0) < 0.0f || getTetraVolume(tetra1) < 0.0f) 
		return false;

	newTetras.push_back(tetra0);
	newTetras.push_back(tetra1);

	// add new tetras;
	for (i = 0; i < (int)newTetras.size(); i++)
		mTetrahedra.push_back(newTetras[i]);

	// delete old tetrahedra
	i = 0;
	while (i < (int)mTetrahedra.size()) {
		if (mTetrahedra[i].flags & TETRAHEDRON_DELETE) {
			mTetrahedra[i] = mTetrahedra[mTetrahedra.size()-1];
			mTetrahedra.pop_back();
		}
		else i++;
	}

	return true;
}

// ----------------------------------------------------------------------
void TetraMesh::removeOuterTetrahedra(const Mesh &mesh)
{
	int i,j,k;
	m3Vector center, dir;
	m3Vector v0,v1,v2;
	m3Real t,u,v;
	m3Bounds bounds, rayBounds, triBounds;
	std::vector<int> mItemIndices;

	mesh.getBounds(bounds);

	i = 0;
	while (i < (int)mTetrahedra.size()) {
		if (mCancel) return; 

		MeshTetrahedron &mt = mTetrahedra[i];
		bool remove = false;
		if (isFarVertex(mt.vertexNr[0]) || isFarVertex(mt.vertexNr[1]) ||
				isFarVertex(mt.vertexNr[2]) || isFarVertex(mt.vertexNr[3]))
			remove = true;
		else {
			center =  mVertices[mt.vertexNr[0]];
			center += mVertices[mt.vertexNr[1]];
			center += mVertices[mt.vertexNr[2]];
			center += mVertices[mt.vertexNr[3]];
			center *= 0.25f;
			int numInside = 0;	// test 6 standard rays, majority vote
			for (j = 0; j < 3; j++) {
				rayBounds.min = center;
				rayBounds.max = center;
				switch (j) {
					case 0 : 
						rayBounds.max.x = bounds.max.x + EPSILON; 
						rayBounds.min.x = bounds.min.x - EPSILON;
						dir.set(1.0f, 0.0f, 0.0f);
						break;
					case 1 : 
						rayBounds.max.y = bounds.max.y + EPSILON; 
						rayBounds.min.y = bounds.min.y - EPSILON;
						dir.set(0.0f, 1.0f, 0.0f);
						break;
					case 2 : 
						rayBounds.max.z = bounds.max.z + EPSILON; 
						rayBounds.min.z = bounds.min.z - EPSILON;
						dir.set(0.0f, 0.0f, 1.0f);
						break;
				}
				mHash.queryUnique(rayBounds, mItemIndices);
				bool insideNeg = false;
				bool insidePos = false;
				for (k = 0; k < (int)mItemIndices.size(); k++) {
					const MeshTriangle &triangle = mesh.getTriangle(mItemIndices[k]);
					triBounds.setEmpty();
					v0 = mesh.getVertex(triangle.vertexNr[0]); triBounds.include(v0);
					v1 = mesh.getVertex(triangle.vertexNr[1]); triBounds.include(v1);
					v2 = mesh.getVertex(triangle.vertexNr[2]); triBounds.include(v2);
					if (!rayBounds.intersects(triBounds))
						continue;
					if (!Geometry3D::rayTriangleIntersection(center, dir, v0,v1,v2, t,u,v))
						continue;					
					if (t >= 0.0f)
						insidePos = !insidePos;
					else
						insideNeg = !insideNeg;
				}
				if (insideNeg) numInside++;
				if (insidePos) numInside++;
			}
			if (numInside < 3) remove = true;

			// remove degenerated tetrahedra (slivers)
			m3Real quality = getTetraQuality(mt);
			if (quality < 0.001f) remove = true;
		}
		if (remove) {
			mTetrahedra[i] = mTetrahedra[mTetrahedra.size()-1];
			mTetrahedra.pop_back();
		}
		else i++;
	}
}

// ----------------------------------------------------------------------
void TetraMesh::compressVertices()
{
	// remove vertices that are not referenced by any tetrahedra
	int i,j;
	std::vector<int> oldToNew;
	std::vector<m3Vector> newVertices;

	oldToNew.resize(mVertices.size());
	for (i = 0; i < (int)oldToNew.size(); i++)
		oldToNew[i] = -1;

	for (i = 0; i < (int)mTetrahedra.size(); i++) {
		MeshTetrahedron &t = mTetrahedra[i];
		for (j = 0; j < 4; j++) {
			int vNr = t.vertexNr[j];
			if (oldToNew[vNr] < 0) {
				oldToNew[vNr] = newVertices.size();
				newVertices.push_back(mVertices[vNr]);
			}
			t.vertexNr[j] = oldToNew[vNr];
		}
		if (mCancel) return;
	}

	mVertices.clear();
	for (i = 0; i < (int)newVertices.size(); i++)
		mVertices.push_back(newVertices[i]);
}

// ----------------------------------------------------------------------
void TetraMesh::createFrom(const Mesh &mesh, int subdivision)
{
	int i;
	m3Vector pos;
	m3Bounds bounds, triBounds;

	clear();

	// hash mesh triangles for faster access
	mesh.getBounds(bounds);
	m3Real diagonal = bounds.min.distance(bounds.max);
	mHash.setGridSpacing(0.1f * diagonal);
	for (i = 0; i < mesh.getNumTriangles(); i++) {
		mesh.getTriangleBounds(i, triBounds);
		mHash.add(triBounds, i);
	}

	// generate surface vertices	// same indices as mesh!
	for (i = 0; i < mesh.getNumVertices(); i++) {
		pos = mesh.getVertex(i);
		pos.disturb(1.0e-4f);
		mVertices.push_back(pos);
	}

	setPercentage(0);
	generateAdditionalVertices(mesh, true, subdivision);
	if (mCancel) { clear(); return; }
	setPercentage(20);
	delaunayTetrahedralization(bounds);
	if (mCancel) { clear(); return; }
	setPercentage(60);
//	splitTetrahedra(mesh);
	swapTetrahedra(mesh, subdivision);
	if (mCancel) { clear(); return; }
	setPercentage(70);
	swapTetrahedra(mesh, subdivision);
	if (mCancel) { clear(); return; }
	setPercentage(80);
	removeOuterTetrahedra(mesh);
	if (mCancel) { clear(); return; }
	setPercentage(90);
	compressVertices();
	if (mCancel) { clear(); return; }
	setPercentage(100);
}

// ----------------------------------------------------------------------
void TetraMesh::createUsingTetraStuffer(const Mesh &mesh, int subdivision)
{
	mTetraStuffer->createFrom(mesh, subdivision);
}

// ----------------------------------------------------------------------
void TetraMesh::getBounds(m3Bounds &bounds) const
{
	bounds.setEmpty();
	for (int i = 0; i < (int)mVertices.size(); i++) 
		bounds.include(mVertices[i]);
}

// ----------------------------------------------------------------------
void TetraMesh::deleteTetrahedron(int tetraNr)
{
	if (tetraNr >= (int)mTetrahedra.size()) return;
	TetraUndo undo;
	undo.tetra = mTetrahedra[tetraNr];
	undo.type = tuDelete;
	mUndos.push_back(undo);

	mTetrahedra[tetraNr] = mTetrahedra[mTetrahedra.size()-1];
	mTetrahedra.pop_back();
}

// ----------------------------------------------------------------------
bool TetraMesh::addTetrahedron(int tetraNr, const m3Vector &pos)
{
	if (tetraNr < 0 || tetraNr >= (int)mTetrahedra.size()) return false;
	int i;

	MeshTetrahedron &tetra = mTetrahedra[tetraNr];
	m3Vector closest;
	m3Real d, dmin, d2, minD2 = -1.0f;

	// find closest side
	int closestSide = -1;
	for (i = 0; i < 4; i++) {
		const int *si = MeshTetrahedron::sideIndices[i];
		const m3Vector &p0 = mVertices[tetra.vertexNr[si[0]]];
		const m3Vector &p1 = mVertices[tetra.vertexNr[si[1]]];
		const m3Vector &p2 = mVertices[tetra.vertexNr[si[2]]];
		Geometry3D::closestPointOnTriangle(p0,p1,p2, pos, closest);
		d2 = closest.distanceSquared(pos);
		if (minD2 < 0.0f || d2 < minD2) {
			closestSide = i; minD2 = d2;
		}
	}
	int sideV0 = tetra.vertexNr[MeshTetrahedron::sideIndices[closestSide][0]];
	int sideV1 = tetra.vertexNr[MeshTetrahedron::sideIndices[closestSide][1]];
	int sideV2 = tetra.vertexNr[MeshTetrahedron::sideIndices[closestSide][2]];

	// find closest edge
	int side = 0;
	dmin = Geometry3D::segmentPointDist(mVertices[sideV0], mVertices[sideV1], pos);
	d = Geometry3D::segmentPointDist(mVertices[sideV1], mVertices[sideV2], pos);
	if (d < dmin) { dmin = d; side = 1; }
	d = Geometry3D::segmentPointDist(mVertices[sideV2], mVertices[sideV0], pos);
	if (d < dmin) { side = 2; }

	if (addTetrahedron(tetraNr, sideV0, sideV1, sideV2, side)) return true;
	side = (side+1)%3;
	if (addTetrahedron(tetraNr, sideV0, sideV1, sideV2, side)) return true;
	side = (side+1)%3;
	if (addTetrahedron(tetraNr, sideV0, sideV1, sideV2, side)) return true;
	return false;
}

// ----------------------------------------------------------------------
bool TetraMesh::addTetrahedron(int tetraNr, int sideV0, int sideV1, int sideV2, int edge)
{
	if (tetraNr < 0 || tetraNr >= (int)mTetrahedra.size()) return false;
	int i,j,k;

	MeshTetrahedron &tetra = mTetrahedra[tetraNr];
	int edgeV0 = sideV0;
	int edgeV1 = sideV1;
	if (edge == 1) { edgeV0 = sideV1; edgeV1 = sideV2; }
	if (edge == 2) { edgeV0 = sideV2; edgeV1 = sideV0; }

	// find adjacent tetrahedra
	// slow O(n), OK for a single user operation

	std::vector<int> adjVertices;

	for (i = 0; i < (int)mTetrahedra.size(); i++) {
		MeshTetrahedron &t = mTetrahedra[i];
		if (!t.containsVertex(edgeV0) || !t.containsVertex(edgeV1))
			continue;
		int oppV[2];
		t.getOppositeVertices(edgeV0, edgeV1, oppV[0], oppV[1]);
		for (j = 0; j < 2; j++) {
			adjVertices.push_back(oppV[j]);
			k = 0;
			while (k < (int)adjVertices.size()-1) {	// remove pairs
				if (adjVertices[k] == oppV[j]) {
					adjVertices.pop_back();
					adjVertices[k] = adjVertices[adjVertices.size()-1];
					adjVertices.pop_back();
					break;
				}
				k++;
			}
		}
	}
	if (adjVertices.size() != 2) return false;

	int vNew = adjVertices[0];
	if (tetra.containsVertex(vNew))
		vNew = adjVertices[1];

	MeshTetrahedron tetraNew;
	tetraNew.set(sideV0,sideV1,sideV2,vNew);
	if (getTetraVolume(tetraNew) < 0.0f)
		return false;

	TetraUndo undo;
	undo.tetra = tetraNew;
	undo.type = tuAdd;
	mUndos.push_back(undo);

	mTetrahedra.push_back(tetraNew);

	return true;
}

// ----------------------------------------------------------------------
void TetraMesh::undo()
{
	if (mUndos.size() == 0) return;

	TetraUndo undo;
	undo = mUndos[mUndos.size()-1];
	mUndos.pop_back();

	if (undo.type == tuDelete) {
		mTetrahedra.push_back(undo.tetra);
	}
	else if (undo.type == tuAdd) {
		for (int i = 0; i < (int)mTetrahedra.size(); i++) {
			if (mTetrahedra[i] == undo.tetra) {
				mTetrahedra[i] = mTetrahedra[mTetrahedra.size()-1];
				mTetrahedra.pop_back();
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------
void TetraMesh::computeVertexLinks(const Mesh *originalMesh)
{
	if (!originalMesh) return;
	if (originalMesh->getNumVertices() == 0) return;

	m3Bounds bounds, tetraBounds;
	getBounds(bounds);
	m3Vector pos;
	int i;

	// hash tetrahedra for faster search
	mHash.setGridSpacing(bounds.min.distance(bounds.max) * 0.1f);
	for (i = 0; i < (int)mTetrahedra.size(); i++) {
		MeshTetrahedron &mt = mTetrahedra[i];
		tetraBounds.setEmpty();
		tetraBounds.include(mVertices[mt.vertexNr[0]]);
		tetraBounds.include(mVertices[mt.vertexNr[1]]);
		tetraBounds.include(mVertices[mt.vertexNr[2]]);
		tetraBounds.include(mVertices[mt.vertexNr[3]]);
		mHash.add(tetraBounds, i);
	}

	mVertexLinks.clear();
	MeshVertexLink link;

	for (i = 0; i < (int)originalMesh->getNumVertices(); i++) {
		pos = originalMesh->getVertex(i);
		
		mHash.queryUnique(pos, mItemIndices);
		m3Real minDist = 0.0f;
		m3Vector b;
		int num = (int)mItemIndices.size();
		if (num == 0) num = mTetrahedra.size();

		for (int i = 0; i < num; i++) {
			int j = i;
			if (mItemIndices.size() > 0) j = mItemIndices[i];
			MeshTetrahedron &mt = mTetrahedra[j];
			m3Vector &p0 = mVertices[mt.vertexNr[0]];
			m3Vector &p1 = mVertices[mt.vertexNr[1]];
			m3Vector &p2 = mVertices[mt.vertexNr[2]];
			m3Vector &p3 = mVertices[mt.vertexNr[3]];
			Geometry3D::barycentricCoords(p0, p1, p2, p3, pos, b);

			// is the vertex inside the tetrahedron? If yes we take it
			if (b.x >= 0.0f && b.y >= 0.0f && b.z >= 0.0f && (b.x + b.y + b.z) <= 1.0f) 
			{
				link.barycentricCoords = b;
				link.tetraNr = j;
				break;
			}
			// otherwise, if we are not in any tetrahedron we take the closest one
			m3Real dist = 0.0f;
			if (b.x + b.y + b.z > 1.0f) dist = b.x + b.y + b.z - 1.0f;
			if (b.x < 0.0f) dist = m3Max(dist, -b.x);
			if (b.y < 0.0f) dist = m3Max(dist, -b.y);
			if (b.z < 0.0f) dist = m3Max(dist, -b.z);

			if (i == 0 || dist < minDist) {
				minDist = dist;
				link.barycentricCoords = b;
				link.tetraNr = j;
			}
		}

		mVertexLinks.push_back(link);
	}
}

// ----------------------------------------------------------------------
bool TetraMesh::save(char *filename, const Mesh *originalMesh)
{
	FILE *f = fopen(filename, "w");
	if (!f) return false;

	compressVertices();
	computeVertexLinks(originalMesh);

	int i;
	fprintf(f, "# Tetrahedral mesh generated using TetraMaker (c) AGEIA\n");

	fprintf(f, "\n");
	fprintf(f, "# %i vertices\n", mVertices.size());
	for (i = 0; i < (int)mVertices.size(); i++) {
		const m3Vector &p = mVertices[i];
		fprintf(f, "v %f %f %f\n", p.x, p.y, p.z);
	}

	fprintf(f, "\n");
	fprintf(f, "# %i tetrahedra\n", mTetrahedra.size());
	for (i = 0; i < (int)mTetrahedra.size(); i++) {
		const MeshTetrahedron &t = mTetrahedra[i];
		fprintf(f, "t %i %i %i %i\n", t.vertexNr[0], t.vertexNr[1], t.vertexNr[2], t.vertexNr[3]);
	}

	if (mVertexLinks.size() > 0) {
		fprintf(f,"\n");
		fprintf(f,"# links from the vertices of the original mesh to the containing tetrahedron\n");
		fprintf(f,"# including barycentric coordinates w.r.t. the containing tetrahedron\n");
		fprintf(f,"# original mesh: %s\n", originalMesh->getName());
		fprintf(f,"# %i vertices in the original mesh\n", originalMesh->getNumVertices());
		fprintf(f,"\n");
		for (i = 0; i < (int)mVertexLinks.size(); i++) {
			m3Vector &b = mVertexLinks[i].barycentricCoords;
			int tetraNr = mVertexLinks[i].tetraNr;
			fprintf(f, "l %i %f %f %f\n", tetraNr, b.x, b.y, b.z);
		}
	}

	fclose(f);
	return true;
}

// ----------------------------------------------------------------------
bool TetraMesh::load(char *filename)
{
	FILE *f = fopen(filename, "r");
	if (!f) return false;

	clear();

	MeshTetrahedron tetra;
	tetra.init();
	int i0,i1,i2,i3;
	m3Vector v;
	MeshString s;
	MeshVertexLink link;

	while (!feof(f)) {
		if (fgets(s, MESH_STRING_LEN, f) == NULL) break;

		if (strncmp(s, "v ", 2) == 0) {	// vertex
			sscanf(s, "v %f %f %f", &v.x, &v.y, &v.z);
			mVertices.push_back(v);
		}
		else if (strncmp(s, "t ", 2) == 0) {	// tetra
			sscanf(s, "t %i %i %i %i", &i0,&i1,&i2,&i3);
			tetra.set(i0,i1,i2,i3);
			mTetrahedra.push_back(tetra);
		}
		else if (strncmp(s, "l ",2) == 0) {		// vertex link
			sscanf(s, "l %i %f %f %f", &link.tetraNr, 
				&link.barycentricCoords.x, &link.barycentricCoords.y, &link.barycentricCoords.z);
			mVertexLinks.push_back(link);
		}
	}
	return true;
}
