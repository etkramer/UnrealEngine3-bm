#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <windows.h>

// Demonstrates how to use the NxTetra DLL in your own application.
#include "../NxTetra/NxTetra.h"
#include "wavefront.h"

using namespace NX_TETRA;

NxTetraInterface * getNxTetraInterface(void)
{
	NxTetraInterface *ret = 0;

#ifdef _DEBUG
  const wchar_t *dmodule = L"NxTetraDEBUG.dll";
#else
  const wchar_t *dmodule = L"NxTetra.dll";
#endif
  HMODULE module = LoadLibrary( dmodule );
  if ( module )
  {
    if ( module )
    {
      void *proc = GetProcAddress(module,"getTetraInterface");
      if ( proc )
      {
        typedef NxTetraInterface * (__cdecl * NX_GetToolkit)();
     		ret = ((NX_GetToolkit)proc)();
      }
    }
  }

  return ret;
}


int main(int argc,const char **arglist)
{

  // Usage: TetraConsole <fname.obj>
  //

	NxTetraInterface *nti = getNxTetraInterface();
	if ( nti == 0 )
	{
		printf("Failed to load the NxTetra plug-in.\r\n");
	}
	else
	{
		if ( argc < 2 )
		{
  		printf("Usage: TetraConsole <fname.obj>\r\n");
		}
		else
		{
  		WavefrontObj wo;
  		unsigned int tcount = wo.loadObj(arglist[1]);
  		if ( tcount )
  		{
  			printf("Encountered %d triangles in '%s'\r\n", tcount, arglist[1] );

				printf("First tesing mesh simplification.\r\n");

        TriMesh input;

        input.mVcount   = wo.mVertexCount;
        input.mVertices = wo.mVertices;
        input.mTcount   = wo.mTriCount;
        input.mIndices  = (unsigned int *)wo.mIndices;

        TriMesh output;

        unsigned int tc = nti->simplifySurface(0.5f,input,output);

				unsigned int tc = nti->createIsoSurface(


  		}
  		else
  		{
  			printf("Failed to load Wavefront file '%s'\r\n", arglist[1] );
  		}
		}
	}


	return 0;
}

