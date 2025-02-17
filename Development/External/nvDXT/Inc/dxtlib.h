/****************************************************************************************
	
    Copyright (C) NVIDIA Corporation 2003

    TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
    *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
    OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
    BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
    WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
    BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
    ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
    BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*****************************************************************************************/
#pragma once

#include "nvdxt_options.h"
#include "nvErrorCodes.h"
// for .dds file reading and writing

// see TestDXT.cpp for examples

typedef NV_ERROR_CODE (*DXTReadCallback)(void *buffer, size_t count, void * userData);


struct MIPMapData
{
    size_t mipLevel; 
    size_t width;
    size_t height;

    int faceNumber; // current face number for this image

    int numFaces;   // total number of faces 
        // depth for volume textures
        // 6 for cube maps


};

typedef NV_ERROR_CODE (*DXTWriteCallback) (const void *buffer,
                                     size_t count, 
                                     const MIPMapData * mipMapData, // if nz, this is MIP data
                                     void * userData);




// DXT Error Codes




/*

   Compresses an image with a user supplied callback with the data for each MIP level created

*/

typedef enum nvPixelOrder
{
    nvRGBA,
    nvBGRA,
    nvRGB,
    nvBGR,
    nvGREY,  // one plance copied to RGB

};
class nvDDS
{

public:
    static NV_ERROR_CODE nvDXTcompress(const nvImageContainer & imageContainer,
        nvCompressionOptions * options,
        DXTWriteCallback fileWriteRoutine,  // call to .dds write routine
        const RECT * rect = 0);

    static NV_ERROR_CODE nvDXTcompress(const RGBAImage & srcImage,
        nvCompressionOptions * options,
        DXTWriteCallback fileWriteRoutine,  // call to .dds write routine
        const RECT * rect = 0);

    static NV_ERROR_CODE nvDXTcompress(const unsigned char * srcImage,
        size_t width,
        size_t height,
        size_t byte_pitch,
        nvPixelOrder pixelOrder,

        nvCompressionOptions * options,
        DXTWriteCallback fileWriteRoutine,  // call to .dds write routine
        const RECT * rect = 0);


    // image with MIP maps
    static NV_ERROR_CODE nvDXTcompress(const RGBAMipMappedImage & srcMIPImage,
        nvCompressionOptions * options,
        DXTWriteCallback fileWriteRoutine,  // call to .dds write routine
        const RECT * rect = 0);

    static NV_ERROR_CODE nvDXTcompress(const RGBAMipMappedCubeMap & srcMIPCubeMap,
        nvCompressionOptions * options,
        DXTWriteCallback fileWriteRoutine,  // call to .dds write routine
        const RECT * rect = 0);


    static NV_ERROR_CODE nvDXTcompress(const RGBAMipMappedVolumeMap & srcMIPVolumeMap,
        nvCompressionOptions * options,
        DXTWriteCallback fileWriteRoutine,  // call to .dds write routine
        const RECT * rect = 0);




    static NV_ERROR_CODE nvDXTcompress(const fpImage & srcImage,
        nvCompressionOptions * options,
        DXTWriteCallback fileWriteRoutine,
        const RECT * rect = 0);


    static NV_ERROR_CODE nvDXTcompress(const fpMipMappedImage & srcMIPImage,
        nvCompressionOptions * options,
        DXTWriteCallback fileWriteRoutine,
        const RECT * rect = 0);


    static NV_ERROR_CODE nvDXTcompress(const fpMipMappedCubeMap & srcMIPCubeMap,
        nvCompressionOptions * options,
        DXTWriteCallback fileWriteRoutine,  
        const RECT * rect = 0);



    static NV_ERROR_CODE nvDXTcompress(const fpMipMappedVolumeMap & srcMIPVolumeMap,
        nvCompressionOptions * options,
        DXTWriteCallback fileWriteRoutine,  
        const RECT * rect = 0);



    // array of images for volume map or cube map
    static NV_ERROR_CODE nvDXTcompress(const fpImageArray & srcImageArray,
        nvCompressionOptions * options,
        DXTWriteCallback fileWriteRoutine,  
        const RECT * rect = 0);




    // readMIPMapCount, number of MIP maps to load. 0 is all


    static NV_ERROR_CODE  nvDXTdecompress(
        nvImageContainer & imageData,
        nvPixelFormat pf,        
        int readMIPMapCount,

        DXTReadCallback fileReadRoutine,
        void * userData);


};


/*
#ifndef EXCLUDE_LIBS

#if _DEBUG

 #if _MSC_VER >=1300

  #if _MSC_VER >=1400
   #ifdef _DLL
    #pragma message("Note: including lib: nvDXTlibMTDLLd.vc8.lib") 
    #pragma comment(lib, "nvDXTlibMTDLLd.vc8.lib")
   #else // DLL
    #pragma message("Note: including lib: nvDXTlibMTd.vc8.lib") 
    #pragma comment(lib, "nvDXTlibMTd.vc8.lib")
   #endif //_DLL
  #else // 1400
   #ifdef _MT
    #ifdef _DLL
     #pragma message("Note: including lib: nvDXTlibMTDLLd.vc7.lib") 
     #pragma comment(lib, "nvDXTlibMTDLLd.vc7.lib")
    #else // DLL
     #pragma message("Note: including lib: nvDXTlibMTd.vc7.lib") 
     #pragma comment(lib, "nvDXTlibMTd.vc7.lib")
    #endif //_DLL
   #else // MT
     #pragma message("Note: including lib: nvDXTlibd.vc7.lib") 
     #pragma comment(lib, "nvDXTlibd.vc7.lib")
   #endif // _MT
  #endif // 1400
 #else // 1300
  #ifdef _MT
   #ifdef _DLL                         
    #pragma message("Note: including lib: nvDXTlibMTDLL6.lib") 
    #pragma comment(lib, "nvDXTlibMTDLL6.lib")
   #else // _DLL
    #pragma message("Note: including lib: nvDXTlibMT6.lib") 
    #pragma comment(lib, "nvDXTlibMT6.lib")
   #endif //_DLL
  #else // _MT
   #pragma message("Note: including lib: nvDXTlib6.lib") 
   #pragma comment(lib, "nvDXTlib6.lib")
  #endif // _MT
 
 #endif // 1300

#else // _DEBUG

 #if _MSC_VER >=1300

  #if _MSC_VER >=1400
   #ifdef _DLL
    #pragma message("Note: including lib: nvDXTlibMTDLL.vc8.lib") 
    #pragma comment(lib, "nvDXTlibMTDLL.vc8.lib")
   #else // DLL
    #pragma message("Note: including lib: nvDXTlibMT.vc8.lib") 
    #pragma comment(lib, "nvDXTlibMT.vc8.lib")
   #endif //_DLL
  #else // 1400
   #ifdef _MT
    #ifdef _DLL
     #pragma message("Note: including lib: nvDXTlibMTDLL.vc7.lib") 
     #pragma comment(lib, "nvDXTlibMTDLL.vc7.lib")
    #else // DLL
     #pragma message("Note: including lib: nvDXTlibMT.vc7.lib") 
     #pragma comment(lib, "nvDXTlibMT.vc7.lib")
    #endif //_DLL
   #else // MT
     #pragma message("Note: including lib: nvDXTlib.vc7.lib") 
     #pragma comment(lib, "nvDXTlib.vc7.lib")
   #endif // _MT
  #endif // 1400
 #else // 1300
  #ifdef _MT
   #ifdef _DLL                         
    #pragma message("Note: including lib: nvDXTlibMTDLL6.lib") 
    #pragma comment(lib, "nvDXTlibMTDLL6.lib")
   #else // _DLL
    #pragma message("Note: including lib: nvDXTlibMT6.lib") 
    #pragma comment(lib, "nvDXTlibMT6.lib")
   #endif //_DLL
  #else // _MT
   #pragma message("Note: including lib: nvDXTlib6.lib") 
   #pragma comment(lib, "nvDXTlib6.lib")
  #endif // _MT
 
 #endif // 1300

#endif // _DEBUG

#endif
*/












