#include "Mesh.h"
#include "Geometry3D.h"
#include "QuadricSimplificator.h"
#include "algorithm"

//-----------------------------------------------------------------------------
void MeshMaterial::init()
{
	strcpy(name, "");
	texNr = -1;
	strcpy(texFilename, "");
	ambient[0] = 0.0f; ambient[1] = 0.0f; ambient[2] = 0.0f;
	diffuse[0] = 0.0f; diffuse[1] = 0.0f; diffuse[2] = 0.0f;
	specular[0] = 0.0f; specular[1] = 0.0f; specular[2] = 0.0f;
	alpha = 0.0f;
	shininess = 0.0f;
};

// ----------------------------------------------------------------------
void Mesh::clear()
{
	mTriangles.clear();
	mNormals.clear();
	mTexCoords.clear();
	mVertices.clear();
	mMaterials.clear();
	mOldToNew.clear();
	mBounds.setEmpty();
	mHasTextureCoords = false;
	mHasNormals = false;

	strcpy(mPath, "");
	strcpy(mName, "");
}

// ----------------------------------------------------------------------
void Mesh::draw(RendererColor baseColor, bool wireframe)
{
	if (mVertices.size() == 0)
		return;
	Renderer *renderer = Renderer::getInstance();
	renderer->setColor(baseColor);

	if (wireframe)
		renderer->wireframeMode(true);

	if (!mHasTextureCoords)
		renderer->deactivateTexture();

	int i;
	int materialNr = -1;
	TexCoord *texPointer = NULL;
	bool textured = false;
	MeshString fname;

	mDrawIndices.clear();

	if (!mHasTextureCoords) {
		for (i = 0; i < (int)mTriangles.size(); i++) {
			MeshTriangle &t = mTriangles[i];
			mDrawIndices.push_back(t.vertexNr[0]);
			mDrawIndices.push_back(t.vertexNr[1]);
			mDrawIndices.push_back(t.vertexNr[2]);
		}
		renderer->drawTriangles(mTriangles.size(), &mDrawIndices[0], &mVertices[0], &mNormals[0], NULL);
		if (wireframe)
			renderer->wireframeMode(false);
		return;
	}

	for (i = 0; i < (int)mMaterials.size(); i++) {
		MeshMaterial &m = mMaterials[i];
		if (strlen(m.texFilename) == 0)
			m.texNr = -1;
		else {
			sprintf(fname, "%s\\%s", mPath, m.texFilename);
			m.texNr = renderer->findTexture(fname);
			if (m.texNr < 0)
				m.texNr = renderer->addJpgTexture(fname);
		}
	}

	for (i = 0; i < (int)mTriangles.size(); i++) {
		MeshTriangle &t = mTriangles[i];
		if (t.materialNr != materialNr) {
			if ((int)mDrawIndices.size() > 0) {
				renderer->drawTriangles(mDrawIndices.size()/3, &mDrawIndices[0], 
					&mVertices[0], &mNormals[0], textured ? &mTexCoords[0] : NULL);
			}

			mDrawIndices.clear();
			textured = false;

			materialNr = t.materialNr;
			if (materialNr >= 0) {
				int texNr = mMaterials[materialNr].texNr;
				if (texNr >= 0) {
					renderer->activateTexture(texNr);
					renderer->setColor(baseColor);
					textured = true;
				}
				else {
					renderer->deactivateTexture();
					renderer->setColor3r(mMaterials[materialNr].ambient);
				}
			}
		}

		mDrawIndices.push_back(t.vertexNr[0]);
		mDrawIndices.push_back(t.vertexNr[1]);
		mDrawIndices.push_back(t.vertexNr[2]);
	}
	if ((int)mDrawIndices.size() > 0)
		renderer->drawTriangles(mDrawIndices.size()/3, &mDrawIndices[0],
			&mVertices[0], &mNormals[0], textured ? &mTexCoords[0] : NULL);

	if (wireframe)
		renderer->wireframeMode(false);

	renderer->deactivateTexture();
}

// ----------------------------------------------------------------------
void Mesh::setDiagonal(m3Real diagonal)
{
	m3Real d = mBounds.min.distance(mBounds.max);
	if (d == 0.0f) return;
	m3Real s = diagonal / d;
	for (int i = 0; i < (int)mVertices.size(); i++) 
		mVertices[i] *= s;
	updateBounds();
}

// ----------------------------------------------------------------------
void Mesh::setCenter(const m3Vector &center)
{
	m3Vector d = center - (mBounds.min + mBounds.max) * 0.5f;
	for (int i = 0; i < (int)mVertices.size(); i++)
		mVertices[i] += d;
	updateBounds();
}

// ----------------------------------------------------------------------
void Mesh::transform(const m3Affine &a)
{
	for (int i = 0; i < (int)mVertices.size(); i++) {
		m3Vector &v = mVertices[i];
		v = a * v;
	}
	updateBounds();
	updateNormals();
}

// ----------------------------------------------------------------------
void Mesh::getTriangleBounds(int i, m3Bounds &bounds) const
{
	const MeshTriangle &mt = mTriangles[i];
	bounds.setEmpty();
	bounds.include(mVertices[mt.vertexNr[0]]);
	bounds.include(mVertices[mt.vertexNr[1]]);
	bounds.include(mVertices[mt.vertexNr[2]]);
}

// ----------------------------------------------------------------------
void Mesh::updateNormals()
{
	mNormals.resize(mVertices.size());
	int i;
	for (i = 0; i < (int)mNormals.size(); i++) 
		mNormals[i].zero();
	m3Vector n;
	for (i = 0; i < (int)mTriangles.size(); i++) {
		MeshTriangle &mt = mTriangles[i];
		mt.normalNr[0] = mt.vertexNr[0];
		mt.normalNr[1] = mt.vertexNr[1];
		mt.normalNr[2] = mt.vertexNr[2];
		n.cross(mVertices[mt.vertexNr[1]] - mVertices[mt.vertexNr[0]], 
			mVertices[mt.vertexNr[2]] - mVertices[mt.vertexNr[0]]);
		mNormals[mt.normalNr[0]] += n;
		mNormals[mt.normalNr[1]] += n;
		mNormals[mt.normalNr[2]] += n;
	}
	for (i = 0; i < (int)mNormals.size(); i++) 
		mNormals[i].normalize();
}

// ----------------------------------------------------------------------
void Mesh::updateBounds()
{
	mBounds.setEmpty();
	for (int i = 0; i < (int)mVertices.size(); i++)
		mBounds.include(mVertices[i]);
}

// ----------------------------------------------------------------------
bool Mesh::rayCast(const m3Vector &orig, const m3Vector &dir, m3Real &t)
{
	t = -1.0f;
	for (int i = 0; i < (int)mTriangles.size(); i++) {
		MeshTriangle &mt = mTriangles[i];
		m3Real ti, u,v;
		if (!Geometry3D::rayTriangleIntersection(orig, dir, 
			mVertices[mt.vertexNr[0]],
			mVertices[mt.vertexNr[1]],
			mVertices[mt.vertexNr[2]], 
			ti, u, v))
			continue;
		if (ti < 0.0f) continue;
		if (t < 0.0f || ti < t) t = ti;
	}
	return t >= 0.0f;
}

// --------------------------------------------------------------------------
void Mesh::extractPath(char *filename)
{
	strcpy(mPath, filename);
	int i = strlen(mPath)-1;
	while (i >= 0 && mPath[i] != '\\' && mPath[i] != ':') i--;
	if (i >= 0) mPath[i] = '\0';
	else strcpy(mPath, ".");

	int j = 0;
	i++;
	while (i < (int)strlen(filename))
		mName[j++] = filename[i++];
	mName[j] = '\0';
}

//-----------------------------------------------------------------------------
bool Mesh::importMtlFile(char *mtllib)
{
	MeshString fname;
	sprintf(fname, "%s\\%s", mPath, mtllib);
	FILE *f = fopen(fname, "r");
	if (!f) return false;
	mMaterials.clear();
	MeshString s;
	MeshMaterial mat;
	mat.init();
	while (!feof(f)) {
		fgets(s, MESH_STRING_LEN, f);
		if (_strnicmp(s, "newmtl", 6) == 0) {
			if (strlen(mat.name) > 0)
				mMaterials.push_back(mat);
			mat.init();
			sscanf(&s[7], "%s", mat.name);
		}
		else if (_strnicmp(s, "Ka", 2) == 0) {
			sscanf(&s[3], "%f %f %f", &mat.ambient[0], &mat.ambient[1], &mat.ambient[2]);
		}
		else if (_strnicmp(s, "Kd", 2) == 0) {
			sscanf(&s[3], "%f %f %f", &mat.diffuse[0], &mat.diffuse[1], &mat.diffuse[2]);
		}
		else if (_strnicmp(s, "Ks", 2) == 0) {
			sscanf(&s[3], "%f %f %f", &mat.specular[0], &mat.specular[1], &mat.specular[2]);
		}
		else if (_strnicmp(s, "Ns", 2) == 0) {
			sscanf(&s[3], "%f", &mat.shininess);
		}
		else if (_strnicmp(s, "Tr", 2) == 0) {
			sscanf(&s[3], "%f", &mat.alpha);
		}
		else if (_strnicmp(s, "d ", 2) == 0) {
			sscanf(&s[2], "%f", &mat.alpha);
		}
		else if (_strnicmp(s, "map_K ", 5) == 0) {
			sscanf(&s[7], "%s", mat.texFilename);
		}
	}
	if (strlen(mat.name) > 0)
		mMaterials.push_back(mat);
	fclose(f);
	return true;
}


//-----------------------------------------------------------------------------
void parseRef(char *s, int nr[])
{
	int i,j,k;
	for (k = 0; k < 3; k++)
		nr[k] = 0;	// zero is not used -> we use it to indicate non existence

	int len = strlen(s);
	char is[256]; i = 0;

	for (k = 0; k < 3; k++) {
		j = 0;
		while (i < len && s[i] != '/') {
			is[j] = s[i]; i++; j++;
		}
		i++;
		if (j > 0) {
			is[j] = 0;
			sscanf(is, "%i", &nr[k]);
		}
	}
}

// ----------------------------------------------------------------------
bool Mesh::loadFromObjFile(char *filename)
{
	FILE *f = fopen(filename, "r");
	if (!f) return false;
	clear();
	MeshString s, subs[maxVerticesPerFace];
	MeshString mtllib, matName;

	mHasTextureCoords = false;
	mHasNormals = false;

	strcpy(mtllib, "");
	int materialNr = -1;
	int i,j;
	m3Vector v;
	MeshTriangle t;
	TexCoord tc;

	std::vector<m3Vector> centermVertices;
	std::vector<TexCoord> centermTexCoords;
	std::vector<m3Vector> centerNormals;

	extractPath(filename);

	while (!feof(f)) {
		if (fgets(s, MESH_STRING_LEN, f) == NULL) break;

		if (strncmp(s, "mtllib", 6) == 0) {  // material library
			sscanf(&s[7], "%s", mtllib);
			importMtlFile(mtllib);
		}
		else if (strncmp(s, "usemtl", 6) == 0) {  // use material
			sscanf(&s[7], "%s", matName);
			materialNr = 0;
			int numMaterials = (int)mMaterials.size();
			while (materialNr < numMaterials &&
				   _stricmp(mMaterials[materialNr].name, matName) != 0)
				materialNr++;
			if (materialNr >= numMaterials) 
				materialNr = -1;
		}
		else if (strncmp(s, "v ", 2) == 0) {	// vertex
			sscanf(s, "v %f %f %f", &v.x, &v.y, &v.z);
			mVertices.push_back(v);
		}
		else if (strncmp(s, "vn ", 3) == 0) {	// normal
			sscanf(s, "vn %f %f %f", &v.x, &v.y, &v.z);
			mNormals.push_back(v);
		}
		else if (strncmp(s, "vt ", 3) == 0) {	// texture coords
			sscanf(s, "vt %f %f", &tc.u, &tc.v);
			mTexCoords.push_back(tc);
		}
		else if (strncmp(s, "f ", 2) == 0) {	// face
			int nr;
			nr = sscanf(s, "f %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
			subs[0], subs[1], subs[2], subs[3], subs[4],
			subs[5], subs[6], subs[7], subs[8], subs[9],
			subs[10], subs[11], subs[12],subs[13], subs[14]);
			int vertNr[maxVerticesPerFace], texNr[maxVerticesPerFace];
			int normalNr[maxVerticesPerFace];
			for (i = 0; i < nr; i++) {
				int refs[3];
				parseRef(subs[i], refs);
				vertNr[i]   = refs[0]-1;
				texNr[i]    = refs[1]-1;
				normalNr[i] = refs[2]-1;
				// if indices are negative they are relative to the current state
				if (refs[0] < 0) vertNr[i]   = mVertices.size()  + refs[0];
				if (refs[1] < 0) texNr[i]    = mTexCoords.size() + refs[1];
				if (refs[2] < 0) normalNr[i] = mNormals.size()   + refs[2];
			}
			if (nr <= 4) {	// simple non-singular triangle or quad
				if (vertNr[0] != vertNr[1] && vertNr[1] != vertNr[2] && vertNr[2] != vertNr[0]) {
					t.init();
					t.vertexNr[0] = vertNr[0];
					t.vertexNr[1] = vertNr[1];
					t.vertexNr[2] = vertNr[2];
					t.normalNr[0] = normalNr[0];
					t.normalNr[1] = normalNr[1];
					t.normalNr[2] = normalNr[2];
					t.texCoordNr[0] = texNr[0];
					t.texCoordNr[1] = texNr[1];
					t.texCoordNr[2] = texNr[2];
					t.materialNr = materialNr;
					mTriangles.push_back(t);
				}
				if (nr == 4) {	// non-singular quad -> generate a second triangle
					if (vertNr[2] != vertNr[3] && vertNr[3] != vertNr[0] && vertNr[0] != vertNr[2]) {
						t.init();
						t.vertexNr[0] = vertNr[2];
						t.vertexNr[1] = vertNr[3];
						t.vertexNr[2] = vertNr[0];
						t.normalNr[0] = normalNr[2];
						t.normalNr[1] = normalNr[3];
						t.normalNr[2] = normalNr[0];
						t.texCoordNr[0] = texNr[0];
						t.texCoordNr[1] = texNr[1];
						t.texCoordNr[2] = texNr[2];
						t.materialNr = materialNr;
						mTriangles.push_back(t);
					}
				}
			}
			else {	// polygonal face

				// compute center properties
				m3Vector centerPos(0.0f, 0.0f, 0.0f);
				TexCoord centerTex; centerTex.zero();
				for (i = 0; i < nr; i++) {
					centerPos += mVertices[vertNr[i]];
					if (texNr[i] >= 0) centerTex += mTexCoords[texNr[i]];
				}
				centerPos /= (float)nr;
				centerTex /= (float)nr;
				m3Vector d1 = centerPos - mVertices[vertNr[0]];
				m3Vector d2 = centerPos - mVertices[vertNr[1]];
				m3Vector centerNormal = d1.cross(d2); centerNormal.normalize();

				// add center vertex
				centermVertices.push_back(centerPos);
				centermTexCoords.push_back(centerTex);
				centerNormals.push_back(centerNormal);

				// add surrounding elements
				for (i = 0; i < nr; i++) {
					j = i+1; if (j >= nr) j = 0;
					t.init();
					t.vertexNr[0] = mVertices.size() + centermVertices.size()-1;
					t.vertexNr[1] = vertNr[i];
					t.vertexNr[2] = vertNr[j];

					t.normalNr[0] = mNormals.size() + centerNormals.size()-1;
					t.normalNr[1] = normalNr[i];
					t.normalNr[2] = normalNr[j];

					t.texCoordNr[0] = mTexCoords.size() + centermTexCoords.size()-1;
					t.texCoordNr[1] = texNr[i];
					t.texCoordNr[2] = texNr[j];
					t.materialNr = materialNr;
					mTriangles.push_back(t);
				}
			}
		}
	}
	fclose(f);

	// new center mVertices are inserted here.
	// If they were inserted when generated, the vertex numbering would be corrupted
	for (i = 0; i < (int)centermVertices.size(); i++)
		mVertices.push_back(centermVertices[i]);

	if (mNormals.size() > 0) {
		for (i = 0; i < (int)centerNormals.size(); i++)
			mNormals.push_back(centerNormals[i]);
	}

	if (mTexCoords.size() > 0) {
		for (i = 0; i < (int)centermTexCoords.size(); i++)
			mTexCoords.push_back(centermTexCoords[i]);
	}

	if (mTexCoords.size() > 0) mHasTextureCoords = true;
	if (mNormals.size() > 0) 
		mHasNormals = true;
	else
		updateNormals();

	updateBounds();
	return true;
}

// ----------------------------------------------------------------------
bool Mesh::saveToObjFile(char *filename) const
{
	// extract name and path
	MeshString name, path;
	strcpy(path, filename);
	int i = strlen(path)-1;
	while (i >= 0 && path[i] != '\\' && path[i] != ':') i--;
	if (i >= 0) path[i] = '\0';
	else strcpy(path, ".");
	strcpy(name, &filename[i+1]);
	i = strlen(name);
	while (i >= 0 && name[i] != '.') i--;
	if (name[i] == '.') name[i] = '\0';

	MeshString objName, mtlName;
	sprintf(objName, "%s\\%s.obj", path, name);
	sprintf(mtlName, "%s\\%s.mtl", path, name);
	FILE *f;

	// export material file
	if (mMaterials.size() > 0) {
		f = fopen(mtlName, "w");
		if (!f) return false;
		for (i = 0; i < (int)mMaterials.size(); i++) {
			const MeshMaterial &mat = mMaterials[i];
			fprintf(f, "\n"); 
			fprintf(f, "newmtl %s\n", mat.name); 
			fprintf(f, "Ka %f %f %f\n", mat.ambient[0], mat.ambient[1], mat.ambient[2]); 
			fprintf(f, "Kd %f %f %f\n", mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]); 
			fprintf(f, "Ks %f %f %f\n", mat.specular[0], mat.specular[1], mat.specular[2]); 
			fprintf(f, "Ns %f\n", mat.shininess); 
			fprintf(f, "Tr %f\n", mat.alpha); 
			if (strlen(mat.texFilename) > 0)
				fprintf(f, "map_Kd %s\n", mat.texFilename);
		}
		fclose(f);
	}

	// export obj file
	f = fopen(objName, "w");
	if (!f) return false;

	fprintf(f, "# Wavefront OBJ\n");
	fprintf(f, "\n");
	fprintf(f, "mtllib %s.mtl\n", name);
	fprintf(f, "\n");

	fprintf(f, "\n");
	fprintf(f, "# %i vertices:\n", mVertices.size());
	for (i = 0; i < (int)mVertices.size(); i++) {
		const m3Vector &v = mVertices[i];
		fprintf(f,"v %f %f %f\n", v.x, v.y, v.z);
	}

	fprintf(f, "\n");
	fprintf(f, "# %i texture coordinates:\n", mTexCoords.size());
	for (i = 0; i < (int)mTexCoords.size(); i++) {
		const TexCoord &tc = mTexCoords[i];
		fprintf(f,"vt %f %f\n", tc.u, tc.v);
	}

	if (mHasNormals) {	// make sure we don't save our own ones
		fprintf(f, "\n");
		fprintf(f, "# %i normals:\n", mNormals.size());
		for (i = 0; i < (int)mNormals.size(); i++) {
			const m3Vector &v = mNormals[i];
			fprintf(f,"vn %f %f %f\n", v.x, v.y, v.z);
		}
	}

	int matNr = -1;
	int groupNr = 0;
	fprintf(f, "\n");
	fprintf(f, "# %i triangles:\n", mTriangles.size());
	for (i = 0; i < (int)mTriangles.size(); i++) {
		const MeshTriangle &t = mTriangles[i];
		if (t.materialNr != matNr) {
			matNr = t.materialNr;
			if (matNr >= 0) {
				fprintf(f, "\n");
				fprintf(f, "g group%i\n", groupNr++);
				fprintf(f, "usemtl %s\n", mMaterials[matNr].name);
			}
		}
		fprintf(f, "f");
		for (int j = 0; j < 3; j++) {
			fprintf(f, " ");
			fprintf(f, "%i/", t.vertexNr[j]+1);
			if (t.texCoordNr[j] >= 0) fprintf(f, "%i", t.texCoordNr[j]+1);
			fprintf(f, "/");
			if (mHasNormals && t.normalNr[j] >= 0) fprintf(f, "%i", t.normalNr[j]+1);
		}
		fprintf(f, "\n");
	}
	fclose(f);

	return true;
}

// ----------------------------------------------------------------------
void Mesh::initFrom(const QuadricSimplificator &simplificator)
{
	clear();
	int i;
	int numVertices = simplificator.getNumVertices();

	// get the vertices
	for (i = 0; i < numVertices; i++) {
		if (!simplificator.isDeletedVertex(i)) {
			mOldToNew.push_back(mVertices.size());
			m3Vector pos = simplificator.getVertexPosition(i);
			mVertices.push_back(pos);
		}
		else mOldToNew.push_back(-1);
	}

	// get the triangles
	MeshTriangle t;
	int numTriangles = simplificator.getNumTriangles();
	int v0,v1,v2;
	for (i = 0; i < numTriangles; i++) {
		if (!simplificator.getTriangle(i, v0,v1,v2))
			continue;
		assert(0 <= v0 && v0 < numVertices);
		assert(0 <= v1 && v1 < numVertices);
		assert(0 <= v2 && v2 < numVertices);
		assert(mOldToNew[v0] >= 0);
		assert(mOldToNew[v1] >= 0);
		assert(mOldToNew[v2] >= 0);

		t.set(mOldToNew[v0],mOldToNew[v1],mOldToNew[v2], -1);
		mTriangles.push_back(t);
	}
	updateNormals();
	updateBounds();
	mHasTextureCoords = false;
	mHasNormals = true;
}

// ----------------------------------------------------------------------
void Mesh::initSimplificator(QuadricSimplificator &simplificator) const
{
	simplificator.clear();

	int i;
	for (i = 0; i < (int)mVertices.size(); i++)
		simplificator.registerVertex(mVertices[i]);
	for (i = 0; i < (int)mTriangles.size(); i++) {
		const MeshTriangle &mt = mTriangles[i];
		simplificator.registerTriangle(mt.vertexNr[0], mt.vertexNr[1], mt.vertexNr[2]);
	}
	simplificator.endRegistration(true);
}

// ----------------------------------------------------------------------
void Mesh::initFrom(const Subdivider &subdivider)
{
	clear();
	int i;
	int numVertices = subdivider.getNumVertices();

	// get the vertices
	for (i = 0; i < numVertices; i++) 
		mVertices.push_back(subdivider.getVertexPosition(i));

	// get the triangles
	MeshTriangle t;
	int numTriangles = subdivider.getNumTriangles();
	int v0,v1,v2;
	for (i = 0; i < numTriangles; i++) {
		subdivider.getTriangle(i, v0,v1,v2);
		t.set(v0,v1,v2,-1);
		mTriangles.push_back(t);
	}
	updateNormals();
	updateBounds();
	mHasTextureCoords = false;
	mHasNormals = true;
}

// ----------------------------------------------------------------------
void Mesh::initSubdivider(Subdivider &subdivider, int subdivision) const
{
	subdivider.clear();

	int i;
	for (i = 0; i < (int)mVertices.size(); i++)
		subdivider.registerVertex(mVertices[i]);
	for (i = 0; i < (int)mTriangles.size(); i++) {
		const MeshTriangle &mt = mTriangles[i];
		subdivider.registerTriangle(mt.vertexNr[0], mt.vertexNr[1], mt.vertexNr[2]);
	}
	subdivider.endRegistration(subdivision);
}


unsigned int Mesh::addVertex(float x,float y,float z)
{
  m3Vector d;
  d.x = x;
  d.y = y;
  d.z = z;
  mVertices.push_back(d);
	return mVertices.size();
}

unsigned int Mesh::addTriangle(unsigned int a,unsigned int b,unsigned int c)
{
	MeshTriangle t;
	t.set(a,b,c,-1);
	mTriangles.push_back(t);
	return mTriangles.size();
}

unsigned int Mesh::addNormal(float x,float y,float z)
{
  m3Vector d;
  d.x = x;
  d.y = y;
  d.z = z;
  mNormals.push_back(d);
	mHasNormals = true;
	return mNormals.size();
}

// ---------------------------------------------------------------------------------------
struct VertexRef {
	void init(const m3Vector &p, int vNr) {
		pos = p; vertexNr = vNr;
	}
	bool operator < (const VertexRef &vr) {
		return pos.x < vr.pos.x;
	}
	m3Vector pos;
	int vertexNr;
};

#define MERGE_THRESHOLD 1.0e-6f

// ----------------------------------------------------------------------
void Mesh::mergeVertices()
{
	updateBounds();
	m3Real d = mBounds.min.distance(mBounds.max) * MERGE_THRESHOLD;
	m3Real d2 = d*d;
	int i,j;
	std::vector<VertexRef> vertexRefs;

	VertexRef vr;
	for (i = 0; i < (int)mVertices.size(); i++) {
		vr.init(mVertices[i], i);
		vertexRefs.push_back(vr);
	}
	std::sort(vertexRefs.begin(), vertexRefs.end());

	std::vector<int> oldToNew;
	for (i = 0; i < (int)mVertices.size(); i++) 
		oldToNew.push_back(-1);

	for (i = 0; i < (int)vertexRefs.size(); i++) {
		int iNr = vertexRefs[i].vertexNr;
		m3Vector pos = vertexRefs[i].pos;
		if (oldToNew[iNr] >= 0) continue;
		oldToNew[iNr] = iNr;
		j = i+1;
		while (j < (int)vertexRefs.size() && fabs(vertexRefs[j].pos.x - pos.x) < MERGE_THRESHOLD) 
		{
			if (vertexRefs[j].pos.distanceSquared(pos) < d2) {
				int jNr = vertexRefs[j].vertexNr;
				oldToNew[jNr] = iNr;
			}
			j++;
		}
	}
	for (i = 0; i < (int)mTriangles.size(); i++) {
		MeshTriangle &t = mTriangles[i];
		for (j = 0; j < 3; j++) {
			t.vertexNr[j] = oldToNew[t.vertexNr[j]];
		}
	}

	updateNormals();
	mHasNormals = true;
}

// ----------------------------------------------------------------------
void Mesh::flipTriangleOrientation()
{
	int i, nr;
	for (i = 0; i < (int)mTriangles.size(); i++) {
		MeshTriangle &t = mTriangles[i];
		nr = t.vertexNr[0]; t.vertexNr[0] = t.vertexNr[1]; t.vertexNr[1] = nr;
		nr = t.normalNr[0]; t.normalNr[0] = t.normalNr[1]; t.normalNr[1] = nr;
		nr = t.texCoordNr[0]; t.texCoordNr[0] = t.texCoordNr[1]; t.texCoordNr[1] = nr;

	}

	if (mHasNormals)
		updateNormals();
}
