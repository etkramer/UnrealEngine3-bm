#include "Subdivider.h"
#include <assert.h>
#include <algorithm>

#define MERGE_THRESHOLD 1.0e-6f

// -------------------------------------------------------------------------
struct SubdividerEdge {
	void init(int newV0, int newV1, int newTriangleNr) {
		if (newV0 > newV1) { v0 = newV0; v1 = newV1; }
		else { v0 = newV1; v1 = newV0; }
		triangleNr = newTriangleNr;
	}
	bool operator<(const SubdividerEdge &e) const {
		if (v0 < e.v0) return true;
		if (v0 > e.v0) return false;
		return (v1 < e.v1);
	}
	bool operator==(const SubdividerEdge &e) const {
		return v0 == e.v0 && v1 == e.v1;
	}
	int v0,v1;
	int triangleNr;
};

// ---------------------------------------------------------------------------------------
struct SubdividerVertexRef {
	void init(const m3Vector &p, int vNr) {
		pos = p; vertexNr = vNr;
	}
	bool operator < (const SubdividerVertexRef &vr) {
		return pos.x < vr.pos.x;
	}
	m3Vector pos;
	int vertexNr;
};

// -------------------------------------------------------------------------
void SubdividerVertex::addTriangle(int triangleNr)
{
	for (int i = 0; i < (int)triangles.size(); i++) 
		if (triangles[i] == triangleNr) return;
	triangles.push_back(triangleNr);
}

// -------------------------------------------------------------------------
void SubdividerVertex::removeTriangle(int triangleNr)
{
	int i = 0;
	while (i < (int)triangles.size() && triangles[i] != triangleNr) i++;
	if (i >= (int)triangles.size()) return;
	triangles[i] = triangles[triangles.size()-1];
	triangles.pop_back();
}

// -------------------------------------------------------------------------
Subdivider::~Subdivider()
{
	clear();
}

// -------------------------------------------------------------------------
void Subdivider::clear()
{
	for (int i = 0; i < (int)mVertices.size(); i++)
		delete mVertices[i];
	mVertices.clear();
	mTriangles.clear();
	mBounds.setEmpty();
	mMaxLength = 1.0f;
}

// -------------------------------------------------------------------------
void Subdivider::registerVertex(const m3Vector &pos) {
	mVertices.push_back(new SubdividerVertex(pos));
	mBounds.include(pos);
}

// -------------------------------------------------------------------------
void Subdivider::registerTriangle(int v0, int v1, int v2)
{
	int numVertices = (int)mVertices.size();
	assert(v0 < numVertices);
	assert(v1 < numVertices);
	assert(v2 < numVertices);
	SubdividerTriangle t;
	t.init(v0, v1, v2);
	int tNr = mTriangles.size();
	mVertices[t.vertexNr[0]]->triangles.push_back(tNr);
	mVertices[t.vertexNr[1]]->triangles.push_back(tNr);
	mVertices[t.vertexNr[2]]->triangles.push_back(tNr);
	mTriangles.push_back(t);
}

// -------------------------------------------------------------------------
void Subdivider::endRegistration(int subdivision)
{
	if (subdivision < 1) subdivision = 1;
	mMaxLength = mBounds.min.distance(mBounds.max) / subdivision;
}

// -------------------------------------------------------------------------
void Subdivider::getTriangle(int i, int &v0, int &v1, int &v2) const
{
	assert(i < (int)mTriangles.size());
	const SubdividerTriangle &t = mTriangles[i];
	v0 = t.vertexNr[0];
	v1 = t.vertexNr[1];
	v2 = t.vertexNr[2];
}

// -------------------------------------------------------------------------
int Subdivider::getTriangleNr(int v0, int v1, int v2) const
{
	int num = mVertices.size();
	if (v0 >= num || v1 >= num || v2 >= num)
		return -1;
	SubdividerVertex *qv0 = mVertices[v0];

	for (int i = 0; i < (int)qv0->triangles.size(); i++) {
		const SubdividerTriangle &t = mTriangles[qv0->triangles[i]];
		if (!t.containsVertex(v1) || !t.containsVertex(v2))
			continue;
		return qv0->triangles[i];
	}
	return -1;
}

// -------------------------------------------------------------------------
void Subdivider::mergeVertices()
{
	m3Real d = mBounds.min.distance(mBounds.max) * MERGE_THRESHOLD;
	m3Real d2 = d*d;
	int i,j,k;
	std::vector<SubdividerVertexRef> refs;
	refs.clear();
	SubdividerVertexRef vr;
	for (i = 0; i < (int)mVertices.size(); i++) {
		SubdividerVertex *v = mVertices[i];
		v->marked = false;
		vr.init(v->pos, i);
		refs.push_back(vr);
	}
	sort(refs.begin(), refs.end());
	for (i = 0; i < (int)refs.size()-1; i++) {
		int iNr = refs[i].vertexNr;
		SubdividerVertex *vi = mVertices[iNr];
		if (vi->marked) continue;
		m3Vector pos = refs[i].pos;
		j = i+1;
		while (j < (int)refs.size() && fabs(refs[j].pos.x - pos.x) < MERGE_THRESHOLD) 
		{
			if (refs[j].pos.distanceSquared(pos) < d2) {
				int jNr = refs[j].vertexNr;
				SubdividerVertex *vj = mVertices[jNr];
				for (k = 0; k < (int)vj->triangles.size(); k++) {
					mTriangles[vj->triangles[k]].replaceVertex(jNr, iNr);
					vi->addTriangle(vj->triangles[k]);
				}
				vj->marked = true;
			}
			j++;
		}
	}

	std::vector<int> oldToNew;
	oldToNew.resize(mVertices.size());
	std::vector<SubdividerVertex*> newVertices;

	for (i = 0; i < (int)mVertices.size(); i++) {
		if (mVertices[i]->marked) {
			delete mVertices[i];
			oldToNew[i] = -1;
		}
		else {
			oldToNew[i] = newVertices.size();
			newVertices.push_back(mVertices[i]);
		}
	}
	mVertices = newVertices;
	for (i = 0; i < (int)mTriangles.size(); i++) {
		SubdividerTriangle &t = mTriangles[i];
		t.vertexNr[0] = oldToNew[t.vertexNr[0]];
		t.vertexNr[1] = oldToNew[t.vertexNr[1]];
		t.vertexNr[2] = oldToNew[t.vertexNr[2]];
	}
}

// -------------------------------------------------------------------------
void Subdivider::closeMesh()
{
	int i,j;
	std::vector<SubdividerEdge> edges, borderEdges;
	SubdividerEdge edge;

	for (i = 0; i < (int)mTriangles.size(); i++) {
		SubdividerTriangle &t = mTriangles[i];
		edge.init(t.vertexNr[0], t.vertexNr[1], i); edges.push_back(edge);
		edge.init(t.vertexNr[1], t.vertexNr[2], i); edges.push_back(edge);
		edge.init(t.vertexNr[2], t.vertexNr[0], i); edges.push_back(edge);
	}
	sort(edges.begin(), edges.end());

	i = 0;
	while (i < (int)edges.size()) {
		SubdividerEdge &ei = edges[i];
		j = i+1;
		while (j < (int)edges.size() && edges[j] == ei) 
			j++;
		if (j == i+1) 
			borderEdges.push_back(ei);
		i = j;
	}

	// find border circles
	std::vector<int> borderVertices;
	while (borderEdges.size() > 0) {
		edge = borderEdges[borderEdges.size()-1];
		borderEdges.pop_back();

		borderVertices.clear();
		borderVertices.push_back(edge.v1);
		borderVertices.push_back(edge.v0);
		int nextV, currentV = edge.v0;
		do {
			nextV = -1;
			for (i = 0; i < (int)borderEdges.size(); i++) {
				SubdividerEdge &ei = borderEdges[i];
				if (ei.v0 == currentV) { nextV = ei.v1; break; }
				else if (ei.v1 == currentV) { nextV = ei.v0; break; }
			}
			if (nextV < 0) break;	// chain ended
			borderEdges[i] = borderEdges[borderEdges.size()-1];
			borderEdges.pop_back();
			borderVertices.push_back(nextV);
			currentV = nextV;
		} while(nextV >= 0);
		if (borderVertices[0] == borderVertices[borderVertices.size()-1])
			borderVertices.pop_back();

		closeHole(borderVertices);
	}
}

// -------------------------------------------------------------------------
float Subdivider::qualityOfTriangle(int v0, int v1, int v2) const
{
	m3Vector &p0 = mVertices[v0]->pos;
	m3Vector &p1 = mVertices[v1]->pos;
	m3Vector &p2 = mVertices[v2]->pos;

	m3Real a = p0.distance(p1);
	m3Real b = p1.distance(p2);
	m3Real c = p2.distance(p0);
	if (a > b && a > c) return b+c-a;
	else if (b > c) return a+c-b;
	else return a+b-c;
}

// -------------------------------------------------------------------------
void Subdivider::closeHole(std::vector<int> &borderVertices)
{
	if (borderVertices.size() < 3) return;

	// compute global rotational direction
	int i, i0,i1,i2,i3,i4;
	int	num = (int)borderVertices.size();
	int triangleNr;

	m3Vector ni, n;
	n.zero();
	for (i0 = 0; i0 < num; i0++) {
		i1 = (i0+1)%num;
		i2 = (i1+1)%num;
		m3Vector &p0 = mVertices[borderVertices[i0]]->pos;
		m3Vector &p1 = mVertices[borderVertices[i1]]->pos;
		m3Vector &p2 = mVertices[borderVertices[i2]]->pos;
		ni.cross(p0-p1, p2-p1);
		ni.normalize();
		n += ni;
	}
	n.normalize();

	// fill hole
	SubdividerTriangle t;
	while (borderVertices.size() > 3) {
		num = (int)borderVertices.size();
		float maxQ = -1.0f;
		int bestI = -1;
		for (i0 = 0; i0 < num; i0++) {
			i1 = (i0+1)%num;
			i2 = (i1+1)%num;
			i3 = (i2+1)%num;
			i4 = (i3+1)%num;
			int b0 = borderVertices[i0];
			int b1 = borderVertices[i1];
			int b2 = borderVertices[i2];
			int b3 = borderVertices[i3];
			int b4 = borderVertices[i4];
			// safety, should not be necessary
			if (i0 == 0) { t.init(b1,b2,b3); bestI = i2; }
			// check whether triangle is an ear
			ni.cross(mVertices[b1]->pos - mVertices[b2]->pos, mVertices[b3]->pos - mVertices[b2]->pos);
			if (ni.dot(n) < 0.0f) continue;
			float quality = 
				qualityOfTriangle(b1,b2,b3) 
				-qualityOfTriangle(b0,b1,b2) 
				-qualityOfTriangle(b2,b3,b4);
			if (maxQ < 0.0f || quality > maxQ) {
				maxQ = quality;
				t.init(b1,b2,b3);
				bestI = i2;
			}
		}
		// remove ear vertex from temporary border
		for (i = bestI; i < (int)borderVertices.size()-1; i++)
			borderVertices[i] = borderVertices[i+1];
		borderVertices.pop_back();

		// do we have the triangle already?
		if (getTriangleNr(t.vertexNr[0], t.vertexNr[1], t.vertexNr[2]) >= 0)
			continue;

		// do we have to subdivide the trianlge?
		m3Vector &p0 = mVertices[t.vertexNr[0]]->pos;
		m3Vector &p2 = mVertices[t.vertexNr[2]]->pos;
		m3Vector dir = p2-p0;
		m3Real d = dir.normalize();
		triangleNr = mTriangles.size();
		mTriangles.push_back(t);
		mVertices[t.vertexNr[0]]->addTriangle(triangleNr);
		mVertices[t.vertexNr[1]]->addTriangle(triangleNr);
		mVertices[t.vertexNr[2]]->addTriangle(triangleNr);
	}
	t.init(borderVertices[0], borderVertices[1], borderVertices[2]);
	if (getTriangleNr(t.vertexNr[0], t.vertexNr[1], t.vertexNr[2]) < 0) {
		triangleNr = mTriangles.size();
		mTriangles.push_back(t);
		mVertices[t.vertexNr[0]]->addTriangle(triangleNr);
		mVertices[t.vertexNr[1]]->addTriangle(triangleNr);
		mVertices[t.vertexNr[2]]->addTriangle(triangleNr);
	}
}

// -------------------------------------------------------------------------
int binarySearch(const std::vector<SubdividerEdge> &edges, int v0, int v1, int triangleNr)
{
	SubdividerEdge edge; edge.init(v0,v1,-1);
	int l = 0;
	int r = edges.size()-1;
	if (r < 0) return -1;
	int m = 0;
	while (l <= r) {
        m = (l+r)/2;
        if (edges[m] < edge) l = m+1;
        else if (edge < edges[m]) r = m-1;
        else break;
	}
	if (!(edges[m] == edge)) return -1;
	while (m >= 0 && edges[m] == edge) m--;
	m++;
	while (m < (int)edges.size() && edges[m] == edge && edges[m].triangleNr != triangleNr)
		m++;
	if (edges[m] == edge && edges[m].triangleNr == triangleNr)
		return m;
	return -1;
}

// -------------------------------------------------------------------------
void sortEdges(std::vector<SubdividerEdge> &edges, int l, int r)
{
	int i,j, mi;
	SubdividerEdge k, m;

	i = l; j = r; mi = (l + r)/2;
	m = edges[mi];
	while (i <= j) {
		while(edges[i] < m) i++;
		while(m < edges[j]) j--;
		if (i <= j) {
			k = edges[i]; edges[i] = edges[j]; edges[j] = k;
			i++; j--;
		}
	}
	if (l < j) sortEdges(edges, l, j);
	if (i < r) sortEdges(edges, i, r);
}

// -------------------------------------------------------------------------
void Subdivider::subdivide()
{
	m3Real threshold = 2.0f * mMaxLength;
	int i,j,k;
	m3Vector newPos;
	std::vector<SubdividerEdge> edges;
	SubdividerEdge edge;
	SubdividerTriangle tNew;

	for (i = 0; i < (int)mTriangles.size(); i++) {
		SubdividerTriangle &t = mTriangles[i];
		edge.init(t.vertexNr[0], t.vertexNr[1], i); edges.push_back(edge);
		edge.init(t.vertexNr[1], t.vertexNr[2], i); edges.push_back(edge);
		edge.init(t.vertexNr[2], t.vertexNr[0], i); edges.push_back(edge);
	}
	sortEdges(edges, 0, edges.size()-1);

	i = 0;
	while (i < (int)edges.size()) {
		SubdividerEdge &ei = edges[i];
		int newI = i+1;
		while (newI < (int)edges.size() && edges[newI] == ei) 
			newI++;
		m3Vector &p0 = mVertices[ei.v0]->pos;
		m3Vector &p1 = mVertices[ei.v1]->pos;
		m3Real d = p0.distance(p1);
		if (d < threshold) {
			i = newI;
			continue;
		}
		int newVertex = mVertices.size();
		newPos = (p0 + p1) * 0.5f;
		newPos.disturb(1.0e-4f);
		mVertices.push_back(new SubdividerVertex(newPos));
		int newEdgeNr = edges.size();
		for (j = i; j < newI; j++) {
			SubdividerEdge ej = edges[j];
			SubdividerTriangle tj = mTriangles[ej.triangleNr];
			int v2 = 0;
			if (tj.vertexNr[1] != ej.v0 && tj.vertexNr[1] != ej.v1) v2 = 1;
			else if (tj.vertexNr[2] != ej.v0 && tj.vertexNr[2] != ej.v1) v2 = 2;
			int v0 = (v2+1)%3;
			int v1 = (v0+1)%3;

			// generate new triangle
			int newTriangle = mTriangles.size();
			tNew.init(tj.vertexNr[v0], newVertex, tj.vertexNr[v2]);
			mTriangles.push_back(tNew);
			mVertices[tNew.vertexNr[0]]->addTriangle(newTriangle);
			mVertices[tNew.vertexNr[1]]->addTriangle(newTriangle);
			mVertices[tNew.vertexNr[2]]->addTriangle(newTriangle);

			// modify existing triangle
			mVertices[tj.vertexNr[v0]]->removeTriangle(ej.triangleNr);
			tj.vertexNr[v0] = newVertex;
			mTriangles[ej.triangleNr].vertexNr[v0] = newVertex;
			mVertices[newVertex]->addTriangle(ej.triangleNr);

			// update edges
			k = binarySearch(edges, tNew.vertexNr[2], tNew.vertexNr[0], ej.triangleNr);
			assert(k >= 0);
			edges[k].triangleNr = newTriangle;
			edge.init(tj.vertexNr[v2], tj.vertexNr[v0], ej.triangleNr); edges.push_back(edge);
			edge.init(tj.vertexNr[v0], tj.vertexNr[v1], ej.triangleNr); edges.push_back(edge);
			edge.init(tNew.vertexNr[0], tNew.vertexNr[1], newTriangle); edges.push_back(edge);
			edge.init(tNew.vertexNr[1], tNew.vertexNr[2], newTriangle); edges.push_back(edge);
		}
		i = newI;
		sortEdges(edges, newEdgeNr, edges.size()-1);
	}
}


