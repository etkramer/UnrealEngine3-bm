#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <vector>
#include "Math3D\math3D.h"
#include "SceneManager.h"
#include "Mesh.h"
#include "TetraMesh.h"
#include "IsoMesh.h"
#include "HullMesh.h"

enum SceneManagerMouseButton { smbNone, smbLeft, smbMiddle, smbRight };
enum SceneManagerMouseMode { smmMove, smmScale, smmRotate };

typedef char SceneManagerString[256];

// ------------------------------------------------------------------------------------
class SceneManager {
// ------------------------------------------------------------------------------------
public:
	SceneManager();
    ~SceneManager();

    // singleton pattern
	static SceneManager* getInstance();
	static void destroyInstance();

	void resize(int width, int height) { mViewWidth = width; mViewHeight = height; }
	void reset();
	void draw();

	bool loadMesh(char *filename, char *errorMessage, int len);
	bool saveMesh(char *filename, char *errorMessage, int len);
	bool mergeMesh(char *errorMessage, int len);
	bool flipMesh(char *errorMessage, int len);

	bool saveSurface(char *filename, char *errorMessage, int len);

	bool loadTetraMesh(char *filename, char *errorMessage, int len);
	bool saveTetraMesh(char *filename, char *errorMessage, int len);

	void keyDown(unsigned char key);
	void keyUp(unsigned char key);
	void mouseDown(int x, int y, SceneManagerMouseButton b);
	void mouseMove(int x, int y, SceneManagerMouseButton b);
	void mouseUp(int x, int y, SceneManagerMouseButton b);

	// interface
	bool viewOriginalVisible() const { return mOriginalVisible; }
	void setOriginalVisible(bool on);
	bool viewOriginalWireframe() const { return mOriginalWireframe; }
	void setOriginalWireframe(bool on);

	bool viewSurfaceVisible() const { return mSurfaceVisible; }
	void setSurfaceVisible(bool on);
	bool viewSurfaceWireframe() const { return mSurfaceWireframe; }
	void setSurfaceWireframe(bool on);

	bool viewVolumeVisible() const { return mVolumeVisible; }
	void setVolumeVisible(bool on);
	bool viewVolumeWireframe() const { return mVolumeWireframe; }
	void setVolumeWireframe(bool on);

	void setSubdivision(int subdivision);
	int  getSubdivision() const { return mSubdivision; }

	void setIsoSingle(bool single);
	bool getIsoSingle() const { return mIsoSingle; }

	void setCloseSurface(bool close) { mCloseSurface = close; }
	bool getCloseSurface() const { return mCloseSurface; }

	void setSubdivideSurface(bool subdivide) { mSubdivideSurface = subdivide; }
	bool getSubdivideSurface() const { return mSubdivideSurface; }

	void setXClipping(float clipping);

	bool copySurface();
	bool isoSurface();
	bool hullSurface();
	void simplifySurface(float factor = 0.1f);

	bool generateVolume();
	bool stuffVolume();

	void setMouseMode(SceneManagerMouseMode mode) {
		mMouseMode = mode;
	}

	const char* getMouseMessage() { return mMouseMessage; }
	int  getPercentage();
	void cancelOperation();

	int  getNumOriginalTriangles() const { return mOriginalMesh.getNumTriangles(); }
	int  getNumSurfaceTriangles() const { return mSurfaceMesh.getNumTriangles(); }
	int  getNumSurfaceVertices() const { return mSurfaceMesh.getNumVertices(); }
	int  getNumTetrahedra() const { return mTetraMesh.getNumTetrahedra(); }

	void undo();

private:
	void update();
	void rescaleView(const m3Bounds &bounds);
	void handleMouseDown(const m3Vector &pos, const m3Vector &normal);
	void autoGenerateSurface();

	bool selectElement(const m3Vector &pos);
	int  mSelectedElement;

	// interface
	bool mOriginalVisible;
	bool mOriginalWireframe;
	bool mSurfaceVisible;
	bool mSurfaceWireframe;
	bool mVolumeVisible;
	bool mVolumeWireframe;

	int  mSubdivision;
	bool mIsoSingle;
	bool mCloseSurface;
	bool mSubdivideSurface;

	// scene
	Mesh	   mOriginalMesh;
	Mesh	   mSurfaceMesh;
	QuadricSimplificator mSimplificator;
	bool       mSimplificatorDirty;
	IsoMesh    mIsoMesh;
	HullMesh   mHullMesh;
	TetraMesh  mTetraMesh;

	// view
	int      mViewWidth;
	int      mViewHeight;
	m3Real   mViewScale;
	m3Vector mViewCenter;
	float    mXClipping;

	// keys
	unsigned char mKeyDown[256];
	bool     mAnyKey;

	// mouse states
	m3Real   mMouseDepth;
	bool     mMousePressed;
	m3Vector mMousePos;
	char     mMouseMessage[256];
	SceneManagerMouseMode mMouseMode;
};

#endif
