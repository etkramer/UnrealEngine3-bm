#include "stdafx.h"
#include <windows.h>
#include "glRenderer.h"
#include <math.h>
#include <stdio.h>

const GLfloat shiny_high[1]        = {100};  // a value between 1 & 128
const GLfloat shiny_low[1]         = {10};

#define NEAR_CLIP_PLANE 0.1
#define FAR_CLIP_PLANE 50000

#define M_PI 3.14159265358979f

static Renderer *instance = 0;

Renderer* Renderer::getInstance()
{
	if (instance == NULL)
		instance = new Renderer();
	return instance;
}

void Renderer::destroyInstance()
{
	if (instance != 0) {
		delete instance;
	}
	instance = 0;
}


Renderer::Renderer()
{
}


//-----------------------------------------------------------------------
Renderer::~Renderer()
{
}

//-----------------------------------------------------------------------
void Renderer::clear()
{
}

//-----------------------------------------------------------------------
void Renderer::flush()
{
}

//-----------------------------------------------------------------------
void Renderer::clearDepth()
//-----------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------
void Renderer::clearColor()
//-----------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------
void Renderer::swapBuffers()
//-----------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------
void Renderer::maskColor(bool red, bool green, bool blue)
//-----------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------
void Renderer::lighting(bool on)
//-----------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------
void Renderer::backColor(float *color)
//-----------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------
void Renderer::wireframeMode(bool on)
//-----------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------
void Renderer::init(HWND win)
//-----------------------------------------------------------------------
{
}


//-----------------------------------------------------------------------
void Renderer::resize(int width, int height)
//-----------------------------------------------------------------------
{
}


//--------------------------------------------------------
void Renderer::viewReset()
//--------------------------------------------------------
{
}


//--------------------------------------------------------
void Renderer::viewClick(int x, int y)
//--------------------------------------------------------
{
}


//--------------------------------------------------------
void Renderer::viewDragRotate(int x, int y)
//--------------------------------------------------------
{
}

//--------------------------------------------------------
void Renderer::viewDragTranslate(int x, int y)
//--------------------------------------------------------
{
}

//--------------------------------------------------------
void Renderer::viewDragScale(int x, int y) {
//--------------------------------------------------------
}


//-------------------------------------------------------------------------------------------------------
void Renderer::viewLookAt(float world_scale, float center_x, float center_y, float center_z, bool left_eye)
//-------------------------------------------------------------------------------------------------------
{
}

// -----------------------------------------------------------------
void Renderer::project(m3Vector &v, int &xi, int &yi, float &depth)
{
}


// ---------------------------------------------------------------------
void Renderer::unProject(int xi, int yi, float depth, m3Vector &v)
{
}


// ---------------------------------------------------------------------
float Renderer::depthAt(int xi, int yi)
{
	return 0;
}

// ---------------------------------------------------------------------
void Renderer::pushModelTransformation(const m3Affine &affine)
{
}

// ---------------------------------------------------------------------
void Renderer::popModelTransformation()
{
}

// ------------------ texture handling --------------------------------------


//---------------------------------------------------------------------------
int Renderer::findTexture(char *filename)
{
	return 0;
}


//---------------------------------------------------------------------------
int Renderer::addJpgTexture(char *filename)
{
	return 0;
}



//---------------------------------------------------------------------------
void Renderer::freeTextures()
{
}


//---------------------------------------------------------------------------
void Renderer::activateTexture(int texNr)
{
}


//---------------------------------------------------------------------------
void Renderer::deactivateTexture()
{
}


// Drawing procedures:

//---------------------------------------------------------------------------------------
void Renderer::drawPoint(const m3Vector &center, RendererColor color)
{
}

//---------------------------------------------------------------------------------------
void Renderer::drawLine(const m3Vector &p0, const m3Vector &p1, RendererColor color)
{
}

//---------------------------------------------------------------------------------------
void Renderer::drawTriangles(int numTriangles, const int *indices,
			const m3Vector *vertices, const m3Vector *normals, const TexCoord *texCoords)
{
}

//---------------------------------------------------------------------------------------
void Renderer::drawTetrahedron(const m3Vector &p0, const m3Vector &p1, const m3Vector &p2, const m3Vector &p3,
			m3Real scale, RendererColor color)
{
}

//---------------------------------------------------------------------------------------
void Renderer::drawBounds(const m3Bounds &bounds, RendererColor color)
{
}

//---------------------------------------------------------------------------------------
void Renderer::drawBox(const m3Bounds &bounds, RendererColor color)
{
}

//---------------------------------------------------------------------------------------
void Renderer::drawArrow(const m3Vector &orig, const m3Vector &dir, RendererColor color)
{
}

//---------------------------------------------------------------------------------------
void Renderer::drawWireTriangle(const m3Vector &p0, const m3Vector &p1, const m3Vector &p2, RendererColor color)
{
}

//---------------------------------------------------------------------------------------
void Renderer::subdivide(const m3Vector &p0, const m3Vector &p1, const m3Vector &p2, int level) {

}

//---------------------------------------------------------------------------------------
void Renderer::drawWireSphere(const m3Vector &center, m3Real radius, RendererColor color,  int level)
{
}

//---------------------------------------------------------------------------------------
void Renderer::setColor(RendererColor color)
{
}

//---------------------------------------------------------------------------------------
void Renderer::setColor3r(m3Real color[3])
{
}

//---------------------------------------------------------------------------------------
void Renderer::setColor4r(m3Real color[4])
{
}

//---------------------------------------------------------------------------------------
float* Renderer::getFvColor(RendererColor color)
{
	return mFvColor;
}

