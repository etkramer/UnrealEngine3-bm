#include "XeCOMClasses.h"

#ifdef XECOM_EXPORT
#define XECOM_INTERFACE __declspec(dllexport)
#else
#define XECOM_INTERFACE __declspec(dllimport)
#endif

extern "C" XECOM_INTERFACE HRESULT CreateXenonConsole(IFXenonConsole **OutConsole);