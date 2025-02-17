#include "QuadricSimplificator.h"
#include <assert.h>

#define MERGE_THRESHOLD 1.0e-6f

#include "glRenderer.h"

// -------------------------------------------------------------------------
void QuadricVertex::removeEdge(int edgeNr)
{
	int i = 0;
	while (i < (int)edges.size()) {
		if (edges[i] == edgeNr) {
			edges[i] = edges[edges.size()-1];
			edges.pop_back();
		}
		else i++;
	}
}

// -------------------------------------------------------------------------
void QuadricVertex::addTriangle(int triangleNr)
{
	for (int i = 0; i < (int)triangles.size(); i++) 
		if (triangles[i] == triangleNr) return;
	triangles.push_back(triangleNr);
}

// -------------------------------------------------------------------------
QuadricSimplificator::~QuadricSimplificator()
{
	clear();
}

// -------------------------------------------------------------------------
void QuadricSimplificator::clear()
{
	for (int i = 0; i < (int)mVertices.size(); i++)
		delete mVertices[i];
	mVertices.clear();
	mEdges.clear();
	mTriangles.clear();
	mHeap.clear();
	mVertexRefs.clear();
	mBounds.setEmpty();
}

// -------------------------------------------------------------------------
void QuadricSimplificator::registerVertex(const m3Vector &pos) {
	mVertices.push_back(new QuadricVertex(pos));
	mBounds.include(pos);
}


// -------------------------------------------------------------------------
void QuadricSimplificator::computeCost(QuadricEdge &edge)
{
	const int numSteps = 10;

	QuadricVertex *qv0 = mVertices[edge.vertexNr[0]];
	QuadricVertex *qv1 = mVertices[edge.vertexNr[1]];

	edge.ratio = -1.0f;
	edge.cost  = -1.0f;

	Quadric q; q = qv0->q + qv1->q;
	m3Vector pos;

	bool v0fix = false;
	bool v1fix = false;

	for (int i = 0; i <= numSteps; i++) {
		if (v0fix && i > 0) continue;
		if (v1fix && i < numSteps) continue;
		m3Real r = 1.0f/numSteps*i;
		pos = qv0->pos * (1.0f - r) + qv1->pos * r;
		float c = q.outerProduct(pos);
		if (edge.cost < 0.0f || c < edge.cost) {
			edge.cost = c; edge.ratio = r;
		}
	}
}

// -------------------------------------------------------------------------
void QuadricSimplificator::registerTriangle(int v0, int v1, int v2)
{
	int numVertices = (int)mVertices.size();
	assert(v0 < numVertices);
	assert(v1 < numVertices);
	assert(v2 < numVertices);
	QuadricTriangle t;
	t.init(v0, v1, v2);
	int tNr = mTriangles.size();
	mVertices[t.vertexNr[0]]->triangles.push_back(tNr);
	mVertices[t.vertexNr[1]]->triangles.push_back(tNr);
	mVertices[t.vertexNr[2]]->triangles.push_back(tNr);
	mTriangles.push_back(t);
}

// ----------------------------------------------------------------------
void QuadricSimplificator::quickSortEdges(int l, int r)
{
	int i,j, mi;
	QuadricEdge k, m;

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
	if (l < j) quickSortEdges(l, j);
	if (i < r) quickSortEdges(i, r);
}

// ----------------------------------------------------------------------
void QuadricSimplificator::quickSortVertexRefs(int l, int r)
{
	int i,j, mi;
	QuadricVertexRef k, m;

	i = l; j = r; mi = (l + r)/2;
	m = mVertexRefs[mi];
	while (i <= j) {
		while(mVertexRefs[i] < m) i++;
		while(m < mVertexRefs[j]) j--;
		if (i <= j) {
			k = mVertexRefs[i]; mVertexRefs[i] = mVertexRefs[j]; mVertexRefs[j] = k;
			i++; j--;
		}
	}
	if (l < j) quickSortVertexRefs(l, j);
	if (i < r) quickSortVertexRefs(i, r);
}

// -------------------------------------------------------------------------
void QuadricSimplificator::mergeVertices()
{
	m3Real d = mBounds.min.distance(mBounds.max) * MERGE_THRESHOLD;
	m3Real d2 = d*d;
	int i,j,k;
	mVertexRefs.clear();
	QuadricVertexRef vr;
	for (i = 0; i < (int)mVertices.size(); i++) {
		QuadricVertex *v = mVertices[i];
		vr.init(v->pos, i);
		mVertexRefs.push_back(vr);
	}
	quickSortVertexRefs(0, mVertexRefs.size()-1);
	for (i = 0; i < (int)mVertexRefs.size()-1; i++) {
		int iNr = mVertexRefs[i].vertexNr;
		QuadricVertex *vi = mVertices[iNr];
		if (vi->deleted) continue;
		m3Vector pos = mVertexRefs[i].pos;
		j = i+1;
		while (j < (int)mVertexRefs.size() && fabs(mVertexRefs[j].pos.x - pos.x) < MERGE_THRESHOLD) 
		{
			if (mVertexRefs[j].pos.distanceSquared(pos) < d2) {
				int jNr = mVertexRefs[j].vertexNr;
				QuadricVertex *vj = mVertices[jNr];
				for (k = 0; k < (int)vj->triangles.size(); k++) {
					mTriangles[vj->triangles[k]].replaceVertex(jNr, iNr);
					vi->addTriangle(vj->triangles[k]);
				}
				vj->deleted = true;
			}
			j++;
		}
	}
}

// -------------------------------------------------------------------------
void QuadricSimplificator::endRegistration(bool mergeCloseVertices)
{
	if (mergeCloseVertices)
		mergeVertices();

	int i;
	mEdges.clear();
	for (i = 0; i < (int)mTriangles.size(); i++) {
		QuadricTriangle &t = mTriangles[i];
		QuadricVertex *qv0 = mVertices[t.vertexNr[0]];
		QuadricVertex *qv1 = mVertices[t.vertexNr[1]];
		QuadricVertex *qv2 = mVertices[t.vertexNr[2]];
		Quadric q; q.setFromPlane(qv0->pos, qv1->pos, qv2->pos);
		qv0->q += q; qv1->q += q; qv2->q += q;
		QuadricEdge e;
		e.init(t.vertexNr[0], t.vertexNr[1]); mEdges.push_back(e);
		e.init(t.vertexNr[1], t.vertexNr[2]); mEdges.push_back(e);
		e.init(t.vertexNr[2], t.vertexNr[0]); mEdges.push_back(e);
	}

	// remove duplicated edges
	quickSortEdges(0, mEdges.size()-1);
	std::vector<QuadricEdge> edges;
	i = 0;
	while (i < (int)mEdges.size()) {
		QuadricEdge &e = mEdges[i];
		edges.push_back(e);
		i++;
		bool border = true;
		while (i < (int)mEdges.size() && mEdges[i] == e) {
			i++;
			border = false;
		}
		if (border) {
			edges[edges.size()-1].border = true;
			mVertices[e.vertexNr[0]]->border = true;
			mVertices[e.vertexNr[1]]->border = true;
		}
	}
	mEdges.clear();
	for (i = 0; i < (int)edges.size(); i++) {
		mEdges.push_back(edges[i]);
		QuadricEdge &e = mEdges[i];
		computeCost(e);
		mVertices[e.vertexNr[0]]->edges.push_back(i);
		mVertices[e.vertexNr[1]]->edges.push_back(i);
	}
	// make heap
	int num = (int)mEdges.size();
	mHeap.clear();
	mHeap.push_back(NULL);	// dummy, root must be at position 1!
	for (i = 0; i < num; i++) {
		mHeap.push_back(&mEdges[i]);
		mEdges[i].heapPos = i+1;
	}
	for (i = mHeap.size() >> 1; i >= 1; i--) 
		heapSift(i);
}

// -------------------------------------------------------------------------
bool QuadricSimplificator::getTriangle(int i, int &v0, int &v1, int &v2) const
{
	if (i >= (int)mTriangles.size()) return false;
	const QuadricTriangle &t = mTriangles[i];
	if (t.deleted) return false;
	v0 = t.vertexNr[0];
	v1 = t.vertexNr[1];
	v2 = t.vertexNr[2];
	return true;
}

// -------------------------------------------------------------------------
int QuadricSimplificator::getTriangleNr(int v0, int v1, int v2) const
{
	int num = mVertices.size();
	if (v0 >= num || v1 >= num || v2 >= num)
		return -1;
	QuadricVertex *qv0 = mVertices[v0];
	if (qv0->deleted) return -1;

	for (int i = 0; i < (int)qv0->triangles.size(); i++) {
		const QuadricTriangle &t = mTriangles[qv0->triangles[i]];
		if (!t.containsVertex(v1) || !t.containsVertex(v2))
			continue;
		return qv0->triangles[i];
	}
	return -1;
}

// -------------------------------------------------------------------------
int QuadricSimplificator::getNumActiveVertices() const
{
	int num = 0;
	for (int i = 0; i < (int)mVertices.size(); i++) {
		if (!mVertices[i]->deleted) num++;
	}
	return num;
}


// -------------------------------------------------------------------------
void QuadricSimplificator::heapSift(int i)
{
	int num = (int)mHeap.size();
	assert(1 <= i && i < num);
	QuadricEdge *e = mHeap[i];
	while ((i << 1) < num) {
		int j = i << 1;
		if (j+1 < num && mHeap[j+1]->cost < mHeap[j]->cost)
			j++;
		if (mHeap[j]->cost < e->cost) {
			mHeap[i] = mHeap[j]; mHeap[i]->heapPos = i; i = j; 
		}
		else break;
	}
	mHeap[i] = e; mHeap[i]->heapPos = i;
}

// -------------------------------------------------------------------------
void QuadricSimplificator::heapUpdate(int i)
{
	int num = (int)mHeap.size();
	assert(1 <= i && i < num);
	QuadricEdge *e = mHeap[i];
	while (i > 1) {
		int j = i >> 1;
		if (mHeap[j]->cost > e->cost) {
			mHeap[i] = mHeap[j]; mHeap[i]->heapPos = i; i = j;
		}
		else break;
	}
	while ((i << 1) < num) {
		int j = i << 1;
		if (j+1 < num && mHeap[j+1]->cost < mHeap[j]->cost)
			j++;
		if (mHeap[j]->cost < e->cost) {
			mHeap[i] = mHeap[j]; mHeap[i]->heapPos = i; i = j;
		}
		else break;
	}
	mHeap[i] = e; mHeap[i]->heapPos = i;
}

// -------------------------------------------------------------------------
void QuadricSimplificator::heapRemove(int i)
{
	int num = (int)mHeap.size();
	assert(1 <= i && i < num);
	mHeap[i]->heapPos = -1;
	mHeap[i] = mHeap[num-1];
	mHeap.pop_back();
	if (i < num-1) {
		mHeap[i]->heapPos = i;
		heapSift(i);
	}
}

// -------------------------------------------------------------------------
bool QuadricSimplificator::legalCollapse(QuadricEdge &edge, float maxLength)
{
	int vNr0 = edge.vertexNr[0];
	int vNr1 = edge.vertexNr[1];
	QuadricVertex *qv0 = mVertices[vNr0];
	QuadricVertex *qv1 = mVertices[vNr1];

	// here we make sure that the border does not shrink
	int numBorder = 0;
	if (qv0->border) numBorder++;
	if (qv1->border) numBorder++;
	if (numBorder == 1) 
		return false;

	// this is a test to make sure edges don't get too long
	if (maxLength != 0.0f) {
		if (qv0->pos.distanceSquared(qv1->pos) > maxLength * maxLength)
			return false;
	}

	// not legal if there exists v != v0,v1 with (v0,v) and (v1,v) edges
	// but (v,v0,v1) is not a triangle

	int i,j;
	for (i = 0; i < (int)qv0->edges.size(); i++) {
		int v = mEdges[qv0->edges[i]].otherVertex(vNr0);
		bool edgeFound = false;
		for (j = 0; j < (int)qv1->edges.size(); j++) {
			if (mEdges[qv1->edges[j]].otherVertex(vNr1) == v) { edgeFound = true; break; }
		}
		if (!edgeFound) continue;
		bool triFound = false;
		for (j = 0; j < (int)qv0->triangles.size(); j++) {
			QuadricTriangle &t = mTriangles[qv0->triangles[j]];
			if (t.containsVertex(vNr0) && t.containsVertex(vNr1) && t.containsVertex(v)) {
				triFound = true; break;
			}
		}
		if (!triFound) return false;

	}
	return true;
}

// -------------------------------------------------------------------------
void QuadricSimplificator::collapseEdge(QuadricEdge &edge)
{
	int vNr0 = edge.vertexNr[0];
	int vNr1 = edge.vertexNr[1];
	QuadricVertex *qv0 = mVertices[vNr0];
	QuadricVertex *qv1 = mVertices[vNr1];
	// contract edge to the vertex0
	qv0->pos = qv0->pos * (1.0f - edge.ratio) + qv1->pos * edge.ratio;
	qv0->q += qv1->q;
	// merge the edges
	int i,j;
	for (i = 0; i < (int)qv1->edges.size(); i++) {
		QuadricEdge &ei = mEdges[qv1->edges[i]];
		int vi = ei.otherVertex(vNr1);
		if (vi == vNr0) continue;
		// test whether we already have this neighnor
		bool found = false;
		for (j = 0; j < (int)qv0->edges.size(); j++) {
			QuadricEdge &ej = mEdges[qv0->edges[j]];
			if (ej.otherVertex(vNr0) == vi) {
				found = true; break;
			}
		}
		if (found) {
			mVertices[vi]->removeEdge(qv1->edges[i]);
			if (ei.heapPos >= 0)
				heapRemove(ei.heapPos);
		}
		else {
			ei.replaceVertex(vNr1, vNr0);
			qv0->edges.push_back(qv1->edges[i]);
		}
	}
	// remove common edge and update adjacent eges
	i = 0;
	while (i < (int)qv0->edges.size()) {
		QuadricEdge &ei = mEdges[qv0->edges[i]];
		if (ei.otherVertex(vNr0) == vNr1) {
			qv0->edges[i] = qv0->edges[qv0->edges.size()-1];
			qv0->edges.pop_back();
		}
		else {
			computeCost(ei);
			if (ei.heapPos >= 0)
				heapUpdate(ei.heapPos);
			i++;
		}
	}
	// delete collapsed triangles
	i = 0;
	while (i < (int)qv0->triangles.size()) {
		QuadricTriangle &t = mTriangles[qv0->triangles[i]];
		if (t.containsVertex(vNr1)) {
			t.deleted = true;
			qv0->triangles[i] = qv0->triangles[qv0->triangles.size()-1];
			qv0->triangles.pop_back();
		}
		else i++;
	}
	// update triangles
	for (i = 0; i < (int)qv1->triangles.size(); i++) {
		QuadricTriangle &t = mTriangles[qv1->triangles[i]];
		if (t.deleted) continue;
		t.replaceVertex(vNr1, vNr0);
		qv0->triangles.push_back(qv1->triangles[i]);
	}
	qv1->deleted = true;
}

// -------------------------------------------------------------------------
bool QuadricSimplificator::simplificationStep(int subdivision)
{
	m3Real maxLength = 0.0f;
	if (subdivision > 0)
		maxLength = mBounds.min.distance(mBounds.max) / subdivision;

	bool edgeFound = false;
	QuadricEdge *e = NULL;
	while (mHeap.size() > 1) {
		e = mHeap[1];
		heapRemove(1);
		if (legalCollapse(*e, maxLength)) {
			edgeFound = true; break; 
		}
	}
	if (!edgeFound) 
		return false;

	collapseEdge(*e);
	return true;
}

// -------------------------------------------------------------------------
void QuadricSimplificator::testHeap()
{
	int num = (int)mHeap.size();
	for (int i = 1; i < num; i++) {
		assert(mHeap[i]->heapPos == i);
		if ((i << 1) < num) {
			int j = i << 1;
			if (j+1 < num && mHeap[j+1]->cost < mHeap[j]->cost)
				j++;
			assert(mHeap[i]->cost <= mHeap[j]->cost);
		}
	}
}
