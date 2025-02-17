#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#define NOMINMAX
#include <vector>
#include <windows.h>
#include <GL\gl.h>
#include <GL\glu.h>
#include "Math3d\math3d.h"

typedef unsigned int RendererColor;
struct RendererTexRef;

// ------------------------------------------------------------------------------------
struct TexCoord {
	TexCoord() {}
	TexCoord(m3Real u, m3Real v) { this->u = u; this->v = v; }
	void zero() { u = 0.0f; v = 0.0f; }
	TexCoord operator + (const TexCoord &tc) const {
		TexCoord r; r.u = u + tc.u; r.v = v + tc.v;
		return tc;
	}
	void operator += (const TexCoord &tc) {
		u += tc.u; v += tc.v;
	}
	void operator *= (m3Real r) {
		u *= r; v *= r;
	}
	void operator /= (m3Real r) {
		u /= r; v /= r;
	}
	m3Real u,v;
};

// ------------------------------------------------------------------------------------
class Renderer {
private:
	Renderer();
	~Renderer();

public:
	// singleton pattern
	static Renderer* getInstance();
	static void destroyInstance();

	static const RendererColor noColor = 0xFFFFFFFF;

	// general
	void init(HWND win);
	void resize(int width, int height);

	void flush();
	void clear();
	void clearDepth();
	void clearColor();
	void swapBuffers();
	void maskColor(bool red, bool green, bool blue);
	void lighting(bool on);
	void backColor(float *color);
	void wireframeMode(bool on);

	// mouse interface
	void viewReset();
	void viewLookAt(float world_scale, float center_x, float center_y, float center_z, bool left_eye);
	void viewClick(int x, int y);
	void viewDragRotate(int x, int y);
	void viewDragTranslate(int x, int y);
	void viewDragScale(int x, int y);

	// view transformation
	void  project(m3Vector &v, int &xi, int &yi, float &depth);
	void  unProject(int xi, int yi, float depth, m3Vector &v);
	float depthAt(int xi, int yi);
	void  pushModelTransformation(const m3Affine &affine);
	void  popModelTransformation();

	// textures
	int  findTexture(char *filename);
	int  addJpgTexture(char *filename);
	void freeTextures();
	void activateTexture(int texNr);
	void deactivateTexture();

	// drawing procedures
	void setColor(RendererColor color);
	void setColor3r(m3Real color[3]);
	void setColor4r(m3Real color[4]);

	void drawPoint(const m3Vector &center, RendererColor color = noColor);
	void drawLine(const m3Vector &p0, const m3Vector &p1, RendererColor color = noColor);
	void drawTriangles(int numTriangles, const int *indices,
			const m3Vector *vertices, const m3Vector *normals, const TexCoord *texCoords);
	void drawTetrahedron(const m3Vector &p0, const m3Vector &p1,  const m3Vector &p2,  const m3Vector &p3,
			m3Real scale, RendererColor color = noColor);
	void drawBounds(const m3Bounds &bounds, RendererColor color = noColor);
	void drawBox(const m3Bounds &bounds, RendererColor color);
	void drawArrow(const m3Vector &orig, const m3Vector &dir, RendererColor color = noColor);
	void drawWireTriangle(const m3Vector &p0, const m3Vector &p1, const m3Vector &p2, RendererColor color = noColor);
	void drawWireSphere(const m3Vector &center, m3Real radius, RendererColor color = noColor, int level = 0);

	static const int colorWhite      = 0xFFFFFF;
	static const int colorBrown      = 0x502B27;
	static const int colorPeach      = 0xB85248;
	static const int colorRed        = 0xFF0000;
	static const int colorGreen      = 0x00FF00;
	static const int colorBlue       = 0x0000FF;
	static const int colorLightBlue  = 0x8080FF;
	static const int colorBlack      = 0x000000;
	static const int colorLightGrey  = 0xB2B2B2;
	static const int colorMediumGrey = 0x808080;
	static const int colorDarkGrey   = 0x333333;
	static const int colorYellow     = 0xFFFF00;
	static const int colorMagenta    = 0xFF00FF;
	static const int colorCyan       = 0x00FFFF;

private:
	HDC    openGLhdc;	// used for connection OpenGL - windows
	HGLRC  openGLhrc;

    int   mMouseX, mMouseY;				// mouse positions
	float mXrot, mYrot;					// rotations
	float mXtrans, mYtrans, mZtrans;	// translation
	float mScale;						// scale

	GLfloat* getFvColor(RendererColor color);
	GLfloat mFvColor[4];

	void subdivide(const m3Vector &p0, const m3Vector &p1, const m3Vector &p2, int level);
	std::vector<RendererTexRef *> textureRefs;
};

#endif
