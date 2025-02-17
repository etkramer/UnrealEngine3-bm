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


// ------------------------------------------------------------------------------------
struct RendererTexRef {
	char     filename[256];
	GLuint   texID;
	int      width, height;
};

//-----------------------------------------------------------------------
Renderer::Renderer()
{
	viewReset();
}


//-----------------------------------------------------------------------
Renderer::~Renderer()
{
	freeTextures();
}

//-----------------------------------------------------------------------
void Renderer::clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
}

//-----------------------------------------------------------------------
void Renderer::flush()
{
	glFlush();
}

//-----------------------------------------------------------------------
void Renderer::clearDepth()
//-----------------------------------------------------------------------
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

//-----------------------------------------------------------------------
void Renderer::clearColor()
//-----------------------------------------------------------------------
{
    glClear(GL_COLOR_BUFFER_BIT);
}

//-----------------------------------------------------------------------
void Renderer::swapBuffers()
//-----------------------------------------------------------------------
{
	glFlush();
	SwapBuffers(openGLhdc); 
}

//-----------------------------------------------------------------------
void Renderer::maskColor(bool red, bool green, bool blue)
//-----------------------------------------------------------------------
{
	GLboolean r = red ? GL_TRUE : GL_FALSE;
	GLboolean g = green ? GL_TRUE : GL_FALSE;
	GLboolean b = blue ? GL_TRUE : GL_FALSE;
	glColorMask(r,g,b, GL_TRUE);
}

//-----------------------------------------------------------------------
void Renderer::lighting(bool on)
//-----------------------------------------------------------------------
{
	if (on)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
}

//-----------------------------------------------------------------------
void Renderer::backColor(float *color)
//-----------------------------------------------------------------------
{
	glClearColor(color[0], color[1], color[2], 1.0f);
}

//-----------------------------------------------------------------------
void Renderer::wireframeMode(bool on)
//-----------------------------------------------------------------------
{
	if (on)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

//-----------------------------------------------------------------------
void Renderer::init(HWND win)
//-----------------------------------------------------------------------
{
		// connect OpenGL output to window 
	openGLhdc = GetDC(win);
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0,0,0,0,0,0,
		0,0,
		0,0,0,0,0,
		32,         // depth bits
		8,          // stencil bits
		0,
		PFD_MAIN_PLANE,
		0,
		0,0,
	};

	int pixelFormat = ChoosePixelFormat(openGLhdc, &pfd);
	SetPixelFormat(openGLhdc, pixelFormat, &pfd);
	openGLhrc = wglCreateContext(openGLhdc);
	if(openGLhrc == NULL)
		MessageBox(win, "Error initializing OpenGL panel", "OpenGL problem", MB_OK);
	if(wglMakeCurrent(openGLhdc, openGLhrc) == false)
		MessageBox(win, "Error initializing OpenGL panel", "OpenGL problem", MB_OK);

	// setup OpenGL

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LINE_SMOOTH);

	glClearColor(0.52f, 0.60f, 0.71f, 1.0f);  // point shop
	glShadeModel(GL_SMOOTH);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_COLOR_MATERIAL);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, getFvColor(colorWhite));
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny_high);

	glLightfv(GL_LIGHT0, GL_AMBIENT, getFvColor(colorDarkGrey));
	glLightfv(GL_LIGHT0, GL_DIFFUSE, getFvColor(colorLightGrey));
	glLightfv(GL_LIGHT0, GL_SPECULAR, getFvColor(colorWhite));
	GLfloat light_direction[]    = {1.0, 1.0, 1.0, 0.0};
	glLightfv(GL_LIGHT0, GL_POSITION, light_direction);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, getFvColor(colorDarkGrey));

	glLineWidth(1.0);
}


//-----------------------------------------------------------------------
void Renderer::resize(int width, int height)
//-----------------------------------------------------------------------
{
	if (height == 0) height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, (GLfloat)(width)/(GLfloat)(height), NEAR_CLIP_PLANE, FAR_CLIP_PLANE);
}


//--------------------------------------------------------
void Renderer::viewReset()
//--------------------------------------------------------
{
	mMouseX = mMouseY = 0;
	mXrot = mYrot = 0.0;
	mXtrans = mYtrans = mZtrans = 0.0;
	mScale = 1.0f;
}


//--------------------------------------------------------
void Renderer::viewClick(int x, int y)
//--------------------------------------------------------
{
	mMouseX = x;
	mMouseY = y;
}


//--------------------------------------------------------
void Renderer::viewDragRotate(int x, int y)
//--------------------------------------------------------
{
	float tmp = (M_PI / 10);
	mYrot += tmp*(x - mMouseX);
	mXrot += tmp*(y - mMouseY);
	if (mXrot < (float)-89.9999) mXrot = (float)-89.9999;
	if (mXrot > (float)89.9999)  mXrot = (float)89.9999;
	mMouseX = x;
	mMouseY = y;
}

//--------------------------------------------------------
void Renderer::viewDragTranslate(int x, int y)
//--------------------------------------------------------
{
	int dx = x - mMouseX;
	int dy = y - mMouseY;
	float fi, x1,y1,z1, x2,y2,z2;
	x1 = (float)dx/5.0f;
	y1 = (float)-dy/5.0f;
	z1 = 0.0f;

	fi = mXrot*M_PI/180.0f;
	x2 = x1;
	y2 = cos(fi)*y1 - sin(fi)*z1;
	z2 = sin(fi)*y1 + cos(fi)*z1;

	fi = mYrot*M_PI/180.0f;
	x1 = cos(fi)*x2 - sin(fi)*z2;
	y1 = y2;
	z1 = sin(fi)*x2 + cos(fi)*z2;

	mXtrans += x1;
	mYtrans += y1;
	mZtrans += z1;
	mMouseX = x;
	mMouseY = y;
}

//--------------------------------------------------------
void Renderer::viewDragScale(int x, int y) {
//--------------------------------------------------------
	float tmp = 1 + (y-mMouseY)*0.01f;
	mScale *= tmp;
	mXtrans *= tmp;
	mYtrans *= tmp;
	mZtrans *= tmp;
	mMouseX = x;
	mMouseY = y;
}


//-------------------------------------------------------------------------------------------------------
void Renderer::viewLookAt(float world_scale, float center_x, float center_y, float center_z, bool left_eye)
//-------------------------------------------------------------------------------------------------------
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 220,   0, 0, 0,   0, 1, 0);

	float s = mScale * world_scale;

	if (!left_eye)
	glRotatef(-3.0, 0.0, 1.0, 0.0);

	// rotate
	glRotatef(mXrot, 1.0, 0.0, 0.0);
	glRotatef(mYrot, 0.0, 1.0, 0.0);

	// translate - change what's at the center of the screen
	glTranslatef(mXtrans, mYtrans, mZtrans);

	// scale
	glScalef(s, s, s);

	// translate - center the world's bounding box
	glTranslatef(-center_x, -center_y, -center_z);
}

// -----------------------------------------------------------------
void Renderer::project(m3Vector &v, int &xi, int &yi, float &depth)
{
	// OpenGL view transformation

	GLint viewPort[4];
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];
	glGetIntegerv(GL_VIEWPORT, viewPort);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

	GLdouble winX, winY, winZ;
	gluProject((GLdouble) v.x, (GLdouble) v.y, (GLdouble) v.z,
		modelMatrix, projMatrix, viewPort, &winX, &winY, &winZ);

	xi = (int)winX; yi = viewPort[3] - (int)winY - 1; depth = (float)winZ;
}


// ---------------------------------------------------------------------
void Renderer::unProject(int xi, int yi, float depth, m3Vector &v)
{
	// OpenGL view transformation

	GLint viewPort[4];
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];
	glGetIntegerv(GL_VIEWPORT, viewPort);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

	yi = viewPort[3] - yi - 1;
	GLdouble wx, wy, wz;
	gluUnProject((GLdouble) xi, (GLdouble) yi, (GLdouble) depth,
	modelMatrix, projMatrix, viewPort, &wx, &wy, &wz);
	v.set((float)wx, (float)wy, (float)wz);
}


// ---------------------------------------------------------------------
float Renderer::depthAt(int xi, int yi)
{
	// depth is taken from depth buffer
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);
	yi = viewPort[3] - yi - 1;
	GLfloat depth;
	glReadPixels(xi, yi, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
	return depth;
}

// ---------------------------------------------------------------------
void Renderer::pushModelTransformation(const m3Affine &affine)
{
    glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	GLfloat m[16];
	m[0] = affine.mat.r00; m[4] = affine.mat.r01; m[ 8] = affine.mat.r02; m[12] = affine.pos.x;
	m[1] = affine.mat.r10; m[5] = affine.mat.r11; m[ 9] = affine.mat.r12; m[13] = affine.pos.y;
	m[2] = affine.mat.r20; m[6] = affine.mat.r21; m[10] = affine.mat.r22; m[14] = affine.pos.z;
	m[3] = 0.0f;           m[7] = 0.0f;           m[11] = 0.0f;           m[15] = 1.0f;
	glMultMatrixf(m);
}

// ---------------------------------------------------------------------
void Renderer::popModelTransformation()
{
    glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

// ------------------ texture handling --------------------------------------


//---------------------------------------------------------------------------
int Renderer::findTexture(char *filename)
{
	int n = (int)textureRefs.size();
	int i = 0;
	while (i < n && strcmpi(textureRefs[i]->filename, filename))
		i++;
	if (i >= n) return -1;
	return i;
}


//---------------------------------------------------------------------------
int Renderer::addJpgTexture(char *filename)
{
	System::Drawing::Bitmap *image;
	try {
		 image = new System::Drawing::Bitmap(filename);
	}
	catch(...) {
		return -1;
	}

	// ------ read out RGB --------------------------------

	int w = image->Width;
	int h = image->Height;
	int lineLen = 3*w;
	GLubyte *buffer = new GLubyte[w*h*3];
	GLubyte *writePtr = &buffer[(h-1)*lineLen];

	for (int j = 0; j < h; j++) {
//		GLubyte *writePtr = &buffer[(h-1-j)*lineLen];
		GLubyte *writePtr = &buffer[j*lineLen];
		for (int i = 0; i < w; i++) {
			System::Drawing::Color color = image->GetPixel(i,j);
			*writePtr++ = color.R;
			*writePtr++ = color.G;
			*writePtr++ = color.B;
		}
	}
	image->Dispose();

	// ------ generate texture --------------------------------

	RendererTexRef *ref = new RendererTexRef();
	glGenTextures(1, &ref->texID);

	glBindTexture(GL_TEXTURE_2D, ref->texID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
	w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	delete[] buffer;

	strcpy(ref->filename, filename);
	textureRefs.push_back(ref);

	return (int)textureRefs.size()-1;
}



//---------------------------------------------------------------------------
void Renderer::freeTextures()
{
	for (int i = 0; i < (int)textureRefs.size(); i++) {
		glDeleteTextures(1, &textureRefs[i]->texID);
		delete textureRefs[i];
	}
	textureRefs.clear();
}


//---------------------------------------------------------------------------
void Renderer::activateTexture(int texNr)
{
	if (texNr >= 0 && texNr < (int)textureRefs.size()) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureRefs[texNr]->texID);
	}
}


//---------------------------------------------------------------------------
void Renderer::deactivateTexture()
{
    glDisable(GL_TEXTURE_2D);
}


// Drawing procedures:

//---------------------------------------------------------------------------------------
void Renderer::drawPoint(const m3Vector &center, RendererColor color)
{
	glBegin(GL_POINTS);
		setColor(color);
		glVertex3f(center.x, center.y, center.z);
	glEnd();
}

//---------------------------------------------------------------------------------------
void Renderer::drawLine(const m3Vector &p0, const m3Vector &p1, RendererColor color)
{
	glBegin(GL_LINES);
		setColor(color);
		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(p1.x, p1.y, p1.z);
	glEnd();
}

//---------------------------------------------------------------------------------------
void Renderer::drawTriangles(int numTriangles, const int *indices,
			const m3Vector *vertices, const m3Vector *normals, const TexCoord *texCoords)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);

	if (normals) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, normals);
	}

	if (texCoords) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
	}

	glDrawElements(GL_TRIANGLES, 3*numTriangles, GL_UNSIGNED_INT, indices);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

//---------------------------------------------------------------------------------------
void Renderer::drawTetrahedron(const m3Vector &p0, const m3Vector &p1, const m3Vector &p2, const m3Vector &p3,
			m3Real scale, RendererColor color)
{
	setColor(color);
	m3Vector normal, center, v[4];
	center = p0;  v[0] = p0;
	center += p1; v[1] = p1;
	center += p2; v[2] = p2;
	center += p3; v[3] = p3;
	center *= 0.25f;
	v[0] = center + (v[0] - center)*scale;
	v[1] = center + (v[1] - center)*scale;
	v[2] = center + (v[2] - center)*scale;
	v[3] = center + (v[3] - center)*scale;
	int sides[4][3] = {{2,1,0}, {0,1,3}, {1,2,3}, {2,0,3}};

	glBegin(GL_TRIANGLES);
	for (int side = 0; side < 4; side ++) {
		m3Vector &v0 = v[sides[side][0]];
		m3Vector &v1 = v[sides[side][1]];
		m3Vector &v2 = v[sides[side][2]];
		normal.cross(v1-v0, v2-v0);
		normal.normalize();
		glNormal3f(normal.x, normal.y, normal.z);
		glVertex3f(v0.x, v0.y, v0.z);
		glVertex3f(v1.x, v1.y, v1.z);
		glVertex3f(v2.x, v2.y, v2.z);
	}
	glEnd();
}

//---------------------------------------------------------------------------------------
void Renderer::drawBounds(const m3Bounds &bounds, RendererColor color)
{
	setColor(color);
	m3Vector v[8];
	v[0].set(bounds.min.x, bounds.min.y, bounds.min.z);
	v[1].set(bounds.max.x, bounds.min.y, bounds.min.z);
	v[2].set(bounds.max.x, bounds.max.y, bounds.min.z);
	v[3].set(bounds.min.x, bounds.max.y, bounds.min.z);
	v[4].set(bounds.min.x, bounds.min.y, bounds.max.z);
	v[5].set(bounds.max.x, bounds.min.y, bounds.max.z);
	v[6].set(bounds.max.x, bounds.max.y, bounds.max.z);
	v[7].set(bounds.min.x, bounds.max.y, bounds.max.z);
	int edges[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,0},{0,4},{1,5},{2,6},{3,7}};

	glBegin(GL_LINES);
	for (int edge = 0; edge < 12; edge ++) {
		m3Vector &v0 = v[edges[edge][0]];
		m3Vector &v1 = v[edges[edge][1]];
		glVertex3f(v0.x, v0.y, v0.z);
		glVertex3f(v1.x, v1.y, v1.z);
	}
	glEnd();
}

//---------------------------------------------------------------------------------------
void Renderer::drawBox(const m3Bounds &bounds, RendererColor color)
{
	setColor(color);
	m3Vector v[8];
	v[0].set(bounds.min.x, bounds.min.y, bounds.min.z);
	v[1].set(bounds.max.x, bounds.min.y, bounds.min.z);
	v[2].set(bounds.max.x, bounds.max.y, bounds.min.z);
	v[3].set(bounds.min.x, bounds.max.y, bounds.min.z);
	v[4].set(bounds.min.x, bounds.min.y, bounds.max.z);
	v[5].set(bounds.max.x, bounds.min.y, bounds.max.z);
	v[6].set(bounds.max.x, bounds.max.y, bounds.max.z);
	v[7].set(bounds.min.x, bounds.max.y, bounds.max.z);
	int sides[6][4] = {{0,3,2,1}, {4,5,6,7}, {0,1,5,4}, {1,2,6,5}, {2,3,7,6}, {3,0,4,7}};
	m3Real normals[6][3] = {{0,0,-1},{0,0,1},{0,-1,0},{1,0,0},{0,1,0},{-1,0,0}};

	glBegin(GL_QUADS);
	for (int side = 0; side < 6; side ++) {
		m3Vector &v0 = v[sides[side][0]];
		m3Vector &v1 = v[sides[side][1]];
		m3Vector &v2 = v[sides[side][2]];
		m3Vector &v3 = v[sides[side][3]];
		glNormal3fv(normals[side]);
		glVertex3f(v0.x, v0.y, v0.z);
		glVertex3f(v1.x, v1.y, v1.z);
		glVertex3f(v2.x, v2.y, v2.z);
		glVertex3f(v3.x, v3.y, v3.z);
	}
	glEnd();
}

//---------------------------------------------------------------------------------------
void Renderer::drawArrow(const m3Vector &orig, const m3Vector &dir, RendererColor color)
{
	setColor(color);
	m3Matrix frame; frame.frame(dir);
	m3Real l = 0.25f * dir.magnitude();
	m3Vector v0 = orig + dir;
	m3Vector v1(-l,-l,0.0f); v1 = v0 + frame * v1;
	m3Vector v2(-l, l,0.0f); v2 = v0 + frame * v2;
	m3Vector v3(-l,0.0f, l); v3 = v0 + frame * v3;
	m3Vector v4(-l,0.0f,-l); v4 = v0 + frame * v4;
	glBegin(GL_LINES);
		glVertex3f(orig.x, orig.y, orig.z); glVertex3f(v0.x, v0.y, v0.z);
		glVertex3f(v0.x, v0.y, v0.z); glVertex3f(v1.x, v1.y, v1.z);
		glVertex3f(v0.x, v0.y, v0.z); glVertex3f(v2.x, v2.y, v2.z);
		glVertex3f(v0.x, v0.y, v0.z); glVertex3f(v3.x, v3.y, v3.z);
		glVertex3f(v0.x, v0.y, v0.z); glVertex3f(v4.x, v4.y, v4.z);
	glEnd();
}

//---------------------------------------------------------------------------------------
void Renderer::drawWireTriangle(const m3Vector &p0, const m3Vector &p1, const m3Vector &p2, RendererColor color)
{
	if (color != noColor) setColor(color);
	glBegin(GL_LINES);
		glVertex3f(p0.x, p0.y, p0.z); glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p1.x, p1.y, p1.z); glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(p2.x, p2.y, p2.z); glVertex3f(p0.x, p0.y, p0.z);
	glEnd();
}

//---------------------------------------------------------------------------------------
void Renderer::subdivide(const m3Vector &p0, const m3Vector &p1, const m3Vector &p2, int level) {

	if (level == 0) { drawWireTriangle(p0, p1, p2); return; }
	m3Vector p01 = p0 + p1; p01.normalize();
	m3Vector p12 = p1 + p2; p12.normalize();
	m3Vector p20 = p2 + p0; p20.normalize();
	subdivide(p0, p01, p20, level-1);
	subdivide(p1, p12, p01, level-1);
	subdivide(p2, p20, p12, level-1);
	subdivide(p01, p12, p20, level-1);
}

#define X 0.525731112119133606f
#define Z 0.850650808352039932f

//---------------------------------------------------------------------------------------
void Renderer::drawWireSphere(const m3Vector &center, m3Real radius, RendererColor color,  int level)
{
	static m3Real coords[12][3] = {
		{-X, 0.0f, Z}, {X, 0.0f, Z}, {-X, 0.0f, -Z}, {X, 0.0f, -Z},
		{0.0f, Z, X}, {0.0f, Z, -X}, {0.0f, -Z, X}, {0.0f, -Z, -X},
		{Z, X, 0.0f}, {-Z, X, 0.0f}, {Z, -X, 0.0f}, {-Z, -X, 0.0f}
	};

	static int indices[20][3] = {
		{0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
		{8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
		{7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
		{6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
	};

	setColor(color);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(center.x, center.y, center.z);
	glScalef(radius, radius, radius);
	m3Vector p0,p1,p2;
	for (int i = 0; i < 20; i++) {
		int i0 = indices[i][0];
		int i1 = indices[i][1];
		int i2 = indices[i][2];
		p0.set(coords[i0][0], coords[i0][1], coords[i0][2]);
		p1.set(coords[i1][0], coords[i1][1], coords[i1][2]);
		p2.set(coords[i2][0], coords[i2][1], coords[i2][2]);
		subdivide(p0,p1,p2, level);
	}
	glPopMatrix();
}

//---------------------------------------------------------------------------------------
void Renderer::setColor(RendererColor color)
{
	if (color == noColor) return;
	const GLfloat s = 1.0f / 256.0f;
	glColor3f(
		(GLfloat)((color >> 16) & 0xFF)*s,
		(GLfloat)((color >>  8) & 0xFF)*s,
		(GLfloat)((color >>  0) & 0xFF)*s
	);
}

//---------------------------------------------------------------------------------------
void Renderer::setColor3r(m3Real color[3])
{
	glColor3f(color[0], color[1], color[2]);
}

//---------------------------------------------------------------------------------------
void Renderer::setColor4r(m3Real color[4])
{
	glColor4f(color[0], color[1], color[2], color[3]);
}

//---------------------------------------------------------------------------------------
float* Renderer::getFvColor(RendererColor color)
{
	const GLfloat s = 1.0f / 256.0f;
	mFvColor[0] = (GLfloat)((color >> 16) & 0xFF)*s;
	mFvColor[1] = (GLfloat)((color >>  8) & 0xFF)*s;
	mFvColor[2] = (GLfloat)((color >>  0) & 0xFF)*s;
	mFvColor[3] = 1.0f;
	return mFvColor;
}

