// NxTetra.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "NxTetra.h"
#include "Mesh.h"
#include "TetraMesh.h"
#include "IsoMesh.h"
#include "HullMesh.h"
#include "QuadricSimplificator.h"

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the NXTETRA_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// NXTETRA_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

// JD: Made static lib for epic...
#define NXTETRA_API

//#ifdef NXTETRA_EXPORTS
//#define NXTETRA_API __declspec(dllexport)
//#else
//#define NXTETRA_API __declspec(dllimport)
//#endif

// JD: Made static lib for epic...
//BOOL APIENTRY DllMain( HANDLE hModule,
//                       DWORD  ul_reason_for_call,
//                       LPVOID lpReserved
//					 )
//{
//    switch (ul_reason_for_call)
//	{
//		case DLL_PROCESS_ATTACH:
//		case DLL_THREAD_ATTACH:
//		case DLL_THREAD_DETACH:
//		case DLL_PROCESS_DETACH:
//			break;
//    }
//    return TRUE;
//}

class TetraInterface : public NxTetraInterface
{
public:
  TetraInterface(void)
  {
		mSubdivision = 30;
  }

  ~TetraInterface(void)
  {
  }


	unsigned int createIsoSurface(const NxTetraMesh &inputMesh,NxTetraMesh &output,bool isoSingle)
	{
		unsigned int ret = 0;

    releaseTetraMesh(output);
    Mesh m;
		getMesh(m,inputMesh);

		QuadricSimplificator simp;

		IsoMesh isoMesh;
		isoMesh.createSurface(m,simp,mSubdivision,isoSingle);
		m.initFrom(simp);
		getMesh(output,m);

		ret = output.mTcount;


    return ret;
	}

	unsigned int simplifySurface(float factor,const NxTetraMesh &inputMesh,NxTetraMesh &outputMesh)
	{
		unsigned int ret = 0;

    Mesh m;
		getMesh(m,inputMesh);

		QuadricSimplificator simp;
		m.initSimplificator(simp);

		factor = 1.0f - factor;

		int numSteps = (int)(factor*m.getNumVertices());
		if ( numSteps )
		{
		  for (int i=0; i<numSteps; i++)
  			simp.simplificationStep(mSubdivision);
		  m.initFrom(simp);
      getMesh(outputMesh,m);
		}
		else
		{
			getMesh(outputMesh,m);
		}

		ret = outputMesh.mTcount;

    return ret;
	}

	void getMesh(NxTetraMesh &mesh,const Mesh &im)
	{
    releaseTetraMesh(mesh);
		mesh.mVcount = (unsigned int) im.getNumVertices();
		mesh.mTcount = (unsigned int) im.getNumTriangles();
		mesh.mVertices = new float[mesh.mVcount*3];
		mesh.mIndices  = new unsigned int[mesh.mTcount*3];
		float *dest = (float *)mesh.mVertices;
		for (unsigned int i=0; i<mesh.mVcount; i++)
		{
			const m3Vector &v = im.getVertex(i);
			dest[0] = v.x;
			dest[1] = v.y;
			dest[2] = v.z;
			dest+=3;
		}
		unsigned int *idx = mesh.mIndices;
		for (unsigned int i=0; i<mesh.mTcount; i++)
		{
			const MeshTriangle &t = im.getTriangle(i);
			idx[0] = t.vertexNr[0];
			idx[1] = t.vertexNr[1];
			idx[2] = t.vertexNr[2];
			idx+=3;
		}

	}

	void getMesh(Mesh &m,const NxTetraMesh &imesh)
	{
		const float *p = imesh.mVertices;
		for (unsigned int i=0; i<imesh.mVcount; i++)
		{
			m.addVertex( p[0], p[1], p[2] );
			m.addNormal(0,1,0);
			p+=3;
		}
		const unsigned int *idx = imesh.mIndices;
		for (unsigned int i=0; i<imesh.mTcount; i++)
		{
			m.addTriangle( idx[0], idx[1], idx[2] );
			idx+=3;
		}
		m.updateNormals();
		m.updateBounds();
	}


  unsigned int createTetraMesh(const NxTetraMesh &inputMesh,NxTetraMesh &outputMesh)
  {
  	unsigned int ret = 0;

    releaseTetraMesh(outputMesh);

    Mesh m;
		getMesh(m,inputMesh);

    TetraMesh tm;
    tm.createFrom( m, mSubdivision );

    outputMesh.mTcount = tm.getNumTetrahedra();
    if ( outputMesh.mTcount )
    {
			outputMesh.mIsTetra = true;

      unsigned int vcount = outputMesh.mVcount   = tm.getNumVertices();
      outputMesh.mVertices = new float[outputMesh.mVcount*3];
      outputMesh.mIndices  = new unsigned int[outputMesh.mTcount*4];

      float *dest = outputMesh.mVertices;
      for (unsigned int i=0; i<vcount; i++)
      {
        const m3Vector &v = tm.getVertex(i);
        dest[0] = v.x;
        dest[1] = v.y;
        dest[2] = v.z;
        dest+=3;
      }
      unsigned int *idx = outputMesh.mIndices;
      for (unsigned int i=0; i<outputMesh.mTcount; i++)
      {
        const MeshTetrahedron& t = tm.getTetrahedron(i);

        unsigned int i1 = t.vertexNr[0];
        unsigned int i2 = t.vertexNr[1];
        unsigned int i3 = t.vertexNr[2];
        unsigned int i4 = t.vertexNr[3];

				assert( i1 >= 0 && i1 < vcount );
				assert( i2 >= 0 && i2 < vcount );
				assert( i3 >= 0 && i3 < vcount );
				assert( i4 >= 0 && i3 < vcount );

				idx[0] = i1;
				idx[1] = i2;
				idx[2] = i3;
				idx[3] = i4;

        idx+=4;
      }

    }


    return outputMesh.mTcount;
  }

  bool         releaseTetraMesh(NxTetraMesh &mesh)
  {
  	bool ret = true;

		delete mesh.mIndices;
		delete mesh.mVertices;

		mesh.mIndices = 0;
		mesh.mVertices = 0;
		mesh.mVcount = 0;
		mesh.mTcount = 0;

  	return ret;
  }

  void         setSubdivisionLevel(unsigned int subdivisionLevel)
  {
  	mSubdivision = subdivisionLevel;
  }

  bool createTetraMesh(NxTetraMesh &mesh,         // mesh to populate
                       unsigned int vcount,       // the number of vertices.
                       const float *vertices,     // the vertex positions.
                       unsigned int tcount,       // the number of triangles or tetrahdra
                       const unsigned int *indices,      // the indices 3 per tri or 4 per tetrahdron
                       bool isTetra)         // flag to indicate triangles or tetrahedron.
  {
    bool ret = false;

    releaseTetraMesh(mesh);

    if ( vcount && tcount && vertices && indices )
    {

      mesh.mVcount = vcount;
      mesh.mVertices = new float[vcount*3];
      memcpy(mesh.mVertices, vertices, sizeof(float)*3*vcount);

      mesh.mTcount   = tcount;
			mesh.mIsTetra  = isTetra;

      if ( isTetra )
      {
        mesh.mIndices = new unsigned int[tcount*4];
        memcpy(mesh.mIndices,indices,sizeof(unsigned int)*tcount*4);
      }
      else
      {
        mesh.mIndices = new unsigned int[tcount*3];
        memcpy(mesh.mIndices,indices,sizeof(unsigned int)*tcount*3);
      }

      ret = true;
    }

		return ret;
  }



private:
	unsigned int mSubdivision;
};


TetraInterface gTetraInterface;

extern "C"
{

NXTETRA_API NxTetraInterface * getTetraInterface(void)
{
	NxTetraInterface *ret =0;

	ret = &gTetraInterface;

  return ret;
}

}
