#include "SceneManager.h"
#include "glRenderer.h"

static SceneManager *instance = 0;

#define SHIFT_KEY 16

SceneManager* SceneManager::getInstance() {

	if (instance == 0) {
		instance = new SceneManager();
	}
	return instance;
}

void SceneManager::destroyInstance() {

	if (instance != 0) {
		delete instance;
	}
	instance = 0;
}

//---------------------------------------------------------------------
SceneManager::SceneManager()
{
	mMouseDepth = 0.0f;

	mViewWidth = 0;
	mViewHeight = 0;
	mViewScale = 1.0f;
	mViewCenter.zero();
	mSubdivision = 40;
	mSimplificatorDirty = false;

	mOriginalVisible = true;
	mOriginalWireframe = false;
	mSurfaceVisible = false;
	mSurfaceWireframe = false;
	mVolumeVisible = false;
	mVolumeWireframe = false;
	mXClipping = 0.0f;
	mIsoSingle = false;
	mCloseSurface = false;
	mSubdivideSurface = false;

	mMousePressed = false;
	mMousePos.zero();
	mSelectedElement = -1;
	mMouseMode = smmMove;
	strcpy(mMouseMessage, "");

	for (int i = 0; i < 256; i++)
		mKeyDown[i] = false;
	mAnyKey = false;
}

//---------------------------------------------------------------------
SceneManager::~SceneManager()
{
	reset();
}

//---------------------------------------------------------------------
void SceneManager::update()
{
}

//---------------------------------------------------------------------
void SceneManager::reset()
{
	mOriginalMesh.clear();
	mSurfaceMesh.clear();
	mTetraMesh.clear();
	update();
}

//-----------------------------------------------------------------------
void SceneManager::rescaleView(const m3Bounds &bounds)
{
	bounds.getCenter(mViewCenter);
	m3Vector d = bounds.max - bounds.min;
	float s = d.magnitude();
	if (s == 0.0f)
		mViewScale = 1.0f;
	else
		mViewScale = 0.4f * mViewHeight / s;
}

//-----------------------------------------------------------------------
bool SceneManager::loadMesh(char *filename, char *errorMessage, int len)
{
	reset();
	if (!mOriginalMesh.loadFromObjFile(filename)) {
		sprintf(errorMessage, "%s read error", filename);
		return false;
	}

	m3Bounds bounds;
	mOriginalMesh.getBounds(bounds);
	rescaleView(bounds);

	mOriginalVisible = true;
	mSurfaceVisible = false;
	mVolumeVisible = false;
	return true;
}

//-----------------------------------------------------------------------
bool SceneManager::saveMesh(char *filename, char *errorMessage, int len)
{
	if (mOriginalMesh.getNumVertices() == 0) {
		sprintf(errorMessage, "Load a mesh first");
		return false;
	}
	
	if (!mOriginalMesh.saveToObjFile(filename)) {
		sprintf(errorMessage, "%s save error", filename);
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------
bool SceneManager::mergeMesh(char *errorMessage, int len)
{
	if (mOriginalMesh.getNumVertices() == 0) {
		sprintf(errorMessage, "Load a mesh first");
		return false;
	}
	
	mOriginalMesh.mergeVertices();

	return true;
}

//-----------------------------------------------------------------------
bool SceneManager::flipMesh(char *errorMessage, int len)
{
	if (mOriginalMesh.getNumVertices() == 0) {
		sprintf(errorMessage, "Load a mesh first");
		return false;
	}
	mOriginalMesh.flipTriangleOrientation();

	return true;
}

//-----------------------------------------------------------------------
bool SceneManager::saveSurface(char *filename, char *errorMessage, int len)
{
	if (mSurfaceMesh.getNumVertices() == 0) {
		sprintf(errorMessage, "Generate a surface mesh first");
		return false;
	}
	
	if (!mSurfaceMesh.saveToObjFile(filename)) {
		sprintf(errorMessage, "%s save error", filename);
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------
bool SceneManager::loadTetraMesh(char *filename, char *errorMessage, int len)
{
	reset();
	if (!mTetraMesh.load(filename)) {
		sprintf(errorMessage, "%s read error", filename);
		return false;
	}

	m3Bounds bounds;
	mTetraMesh.getBounds(bounds);
	rescaleView(bounds);

	mOriginalVisible = false;
	mSurfaceVisible  = false;
	mVolumeVisible   = true;

	return true;
}


//-----------------------------------------------------------------------
bool SceneManager::saveTetraMesh(char *filename, char *errorMessage, int len)
{
	if (mTetraMesh.getNumVertices() == 0) {
		sprintf(errorMessage, "Generate volumetric mesh first");
		return false;
	}
	
	if (!mTetraMesh.save(filename, &mOriginalMesh)) {
		sprintf(errorMessage, "%s save error", filename);
		return false;
	}
	return true;
}

//---------------------------------------------------------------------
void SceneManager::setSubdivision(int subdivision)
{
	mSubdivision = subdivision;
	mSimplificatorDirty = true;
}

//---------------------------------------------------------------------
void SceneManager::setIsoSingle(bool single)
{
	mIsoSingle = single;
}


//---------------------------------------------------------------------
void SceneManager::setOriginalVisible(bool on)
{
	mOriginalVisible = on;
	draw();
}

//---------------------------------------------------------------------
void SceneManager::setOriginalWireframe(bool on)
{
	mOriginalWireframe = on;
	draw();
}

//---------------------------------------------------------------------
void SceneManager::setSurfaceVisible(bool on)
{
	mSurfaceVisible = on;
	draw();
}

//---------------------------------------------------------------------
void SceneManager::setSurfaceWireframe(bool on)
{
	mSurfaceWireframe = on;
	draw();
}

//---------------------------------------------------------------------
void SceneManager::setVolumeVisible(bool on)
{
	mVolumeVisible = on;
	draw();
}

//---------------------------------------------------------------------
void SceneManager::setVolumeWireframe(bool on)
{
	mVolumeWireframe = on;
	draw();
}

//---------------------------------------------------------------------
void SceneManager::setXClipping(float clipping)
{
	mXClipping = clipping;
	draw();
}

//---------------------------------------------------------------------
int SceneManager::getPercentage() {
	return mTetraMesh.getPercentage();
}

//---------------------------------------------------------------------
void SceneManager::cancelOperation() {
	mTetraMesh.cancelOperation();
}

//---------------------------------------------------------------------
bool SceneManager::copySurface()
{
	if (mOriginalMesh.getNumVertices() == 0)
		return false;

	mTetraMesh.clear();

	Subdivider subdivider;
	mOriginalMesh.initSubdivider(subdivider, mSubdivision);
	subdivider.mergeVertices();
	if (mCloseSurface)
		subdivider.closeMesh();
	if (mSubdivideSurface)
		subdivider.subdivide();
	mSurfaceMesh.initFrom(subdivider);

	// init simplificator
	mSurfaceMesh.initSimplificator(mSimplificator);
	mSimplificatorDirty = false;
	mSurfaceMesh.initFrom(mSimplificator);
	mOriginalVisible = false;
	mSurfaceVisible = true;
	mSurfaceWireframe = true;
	return true;
}

//---------------------------------------------------------------------
bool SceneManager::isoSurface()
{
	if (mOriginalMesh.getNumVertices() == 0)
		return false;

	mTetraMesh.clear();
	mIsoMesh.createSurface(mOriginalMesh, mSimplificator, mSubdivision, mIsoSingle);
	mSimplificatorDirty = false;
	mSurfaceMesh.initFrom(mSimplificator);
	mOriginalVisible = false;
	mSurfaceVisible = true;
	mSurfaceWireframe = true;
	return true;
}

//---------------------------------------------------------------------
bool SceneManager::hullSurface()
{
	if (mOriginalMesh.getNumVertices() == 0)
		return false;

	mTetraMesh.clear();

	Subdivider subdivider;
	mHullMesh.createSurface(mOriginalMesh, subdivider, mSubdivision);

	subdivider.mergeVertices();
	if (mSubdivideSurface)
		subdivider.subdivide();
	mSurfaceMesh.initFrom(subdivider);

	// init simplificator
	mSurfaceMesh.initSimplificator(mSimplificator);
	mSimplificatorDirty = false;
	mSurfaceMesh.initFrom(mSimplificator);
	mOriginalVisible = false;
	mSurfaceVisible = true;
	mSurfaceWireframe = true;
	return true;
}

//---------------------------------------------------------------------
void SceneManager::simplifySurface(float factor)
{
	if (mSimplificatorDirty) {
		mSurfaceMesh.initSimplificator(mSimplificator);
		mSimplificatorDirty = false;
	}

	int numSteps = (int)(factor * mSurfaceMesh.getNumVertices());
	for (int i = 0; i < numSteps; i++)
		mSimplificator.simplificationStep(mSubdivision);

	mSurfaceMesh.initFrom(mSimplificator);
	draw();
}

//---------------------------------------------------------------------
bool SceneManager::generateVolume()
{
	if (mOriginalMesh.getNumVertices() == 0)
		return false;

	bool autoSurface = mSurfaceMesh.getNumVertices() == 0;

	if (autoSurface)
		autoGenerateSurface();

	mTetraMesh.createFrom(mSurfaceMesh, mSubdivision);

	if (autoSurface)
		mSurfaceMesh.clear();

	if (mTetraMesh.getNumTetrahedra() > 0) {
		mOriginalVisible = false;
		mSurfaceVisible = false;
		mVolumeVisible = true;
	}
	return true;
}

//---------------------------------------------------------------------
bool SceneManager::stuffVolume()
{
	if (mOriginalMesh.getNumVertices() == 0)
		return false;

	mTetraMesh.createUsingTetraStuffer(mOriginalMesh, mSubdivision);

	if (mTetraMesh.getNumTetrahedra() > 0) {
		mOriginalVisible = false;
		mSurfaceVisible = false;
		mVolumeVisible = true;
	}
	return true;
}

//---------------------------------------------------------------------
void SceneManager::autoGenerateSurface()
{
	const int maxSteps = 100000;

//	int autoSubdivision = 90;
	int autoSubdivision = mSubdivision + 10;
	if (autoSubdivision > 90) autoSubdivision = 90;

	mTetraMesh.clear();
	mIsoMesh.createSurface(mOriginalMesh, mSimplificator, autoSubdivision, true);
	mSimplificatorDirty = false;
	mSurfaceMesh.initFrom(mSimplificator);

	int step = 0;
	while (step < maxSteps && mSimplificator.simplificationStep(mSubdivision)) 
		step++;

	mSurfaceMesh.initFrom(mSimplificator);
}

//---------------------------------------------------------------------
void SceneManager::draw()
{
	Renderer *renderer = Renderer::getInstance();

	renderer->clear();
	renderer->viewLookAt(mViewScale, mViewCenter.x, mViewCenter.y, mViewCenter.z, true);

	if (mOriginalVisible) {
		mOriginalMesh.draw(Renderer::colorMediumGrey, mOriginalWireframe);
	}

	if (mSurfaceVisible) {
		mSurfaceMesh.draw(Renderer::colorYellow, mSurfaceWireframe);
	}

	if (mVolumeVisible) {
		mTetraMesh.draw(Renderer::colorLightBlue, mVolumeWireframe, mXClipping);
	}

	renderer->swapBuffers();
}

//---------------------------------------------------------------------
void SceneManager::keyDown(unsigned char key)
{
	mKeyDown[key] = true;
	mAnyKey = true;

	m3Affine a;
	a.pos.zero();

	switch (key) {
		case 'X' : { a.mat.rotX(m3HalfPi); mOriginalMesh.transform(a); draw(); break; }
		case 'Y' : { a.mat.rotY(m3HalfPi); mOriginalMesh.transform(a); draw(); break; }
		case 'Z' : { a.mat.rotZ(m3HalfPi); mOriginalMesh.transform(a); draw(); break; }
	}
}

//---------------------------------------------------------------------
void SceneManager::keyUp(unsigned char key)
{
	mKeyDown[key] = false;
	mAnyKey = false;
}


//---------------------------------------------------------------------
void SceneManager::mouseDown(int x, int y, SceneManagerMouseButton b)
{
	mMousePressed = true;
	strcpy(mMouseMessage, "");
    Renderer *renderer = Renderer::getInstance();
	if (mAnyKey) {
		mMouseDepth = renderer->depthAt(x,y);
		m3Vector pos;
		renderer->unProject(x,y, mMouseDepth, pos);
		mMousePos = pos;
		m3Vector normal;
		renderer->unProject(x,y, mMouseDepth+1.0f, normal);
		normal -= pos;
		normal.normalize();

		handleMouseDown(pos, normal);
		draw();
	}
	else 
		renderer->viewClick(x,y);
}


//---------------------------------------------------------------------
void SceneManager::mouseMove(int x, int y, SceneManagerMouseButton b)
{
	if (!mMousePressed) return;
	Renderer *renderer = Renderer::getInstance();
	if (mAnyKey) {
		m3Vector pos;
		renderer->unProject(x,y, mMouseDepth, pos);
		mMousePos = pos;
	}
	else {
		switch (b) {
			case smbLeft : renderer->viewDragRotate(x,y); break;
			case smbMiddle : renderer->viewDragTranslate(x,y); break;
			case smbRight : renderer->viewDragScale(x,y); break;
		}
		draw();
	}
}


//---------------------------------------------------------------------
void SceneManager::mouseUp(int x, int y, SceneManagerMouseButton b)
{
	mMousePressed = false;
	m3Vector pos;
	Renderer *renderer = Renderer::getInstance();
	renderer->unProject(x,y, mMouseDepth, pos);
}

//---------------------------------------------------------------------
void SceneManager::handleMouseDown(const m3Vector &pos, const m3Vector &normal)
{
	m3Real distance;

	if (mKeyDown['D']) {		// delete
		int tetraNr = mTetraMesh.closestTetrahedron(pos, distance);
		if (tetraNr < 0) return;
		mTetraMesh.deleteTetrahedron(tetraNr);
	}
	else if (mKeyDown['A']) {	// add
		int tetraNr = mTetraMesh.closestTetrahedron(pos, distance);
		if (tetraNr < 0) return;
		mTetraMesh.addTetrahedron(tetraNr, pos);
	}
	else if (mKeyDown['V']) {	// vertex info
		int vertNr = mTetraMesh.closestVertex(pos, distance);
		if (vertNr < 0) return;
		sprintf(mMouseMessage, "vertex number %i", vertNr);
		mKeyDown['V'] = false;
		mAnyKey = false;
	}
	else if (mKeyDown['T']) {	// tetra info
		int tetraNr = mTetraMesh.closestTetrahedron(pos, distance);
		if (tetraNr < 0) return;
		const MeshTetrahedron &t = mTetraMesh.getTetrahedron(tetraNr);
		sprintf(mMouseMessage, "tetrahedron number %i (%i,%i,%i,%i)", tetraNr, 
			t.vertexNr[0], t.vertexNr[1], t.vertexNr[2], t.vertexNr[3]);
		mKeyDown['T'] = false;
		mAnyKey = false;
	}
}

//---------------------------------------------------------------------
void SceneManager::undo()
{
	mTetraMesh.undo();
	draw();
}

