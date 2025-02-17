//helper class t

#ifndef _FXXSIHELPER_H_
#define _FXXSIHELPER_H_

#ifndef XSIPLUGINCALLBACK
// Use this define to properly export C++ API callbacks
// in your dll, for example XSILoadPlugin()
// Note for windows developers: Using this define means that you do not need to use a .def file 

// XSI C++ API uses C-linkage.  Disable the warning
// that reminds us that CStatus is a C++ object
#pragma warning( disable : 4190 ) 
#define XSIPLUGINCALLBACK extern "C" __declspec(dllexport)
#endif


void XSISetCurrentFrame(double in_dFrame);
double XSIGetCurrentFrame();
void XSISetSceneStartFrame(double in_dFrame);
void XSISetSceneEndFrame(double in_dFrame);



#endif //_FXXSIHELPER_H_
