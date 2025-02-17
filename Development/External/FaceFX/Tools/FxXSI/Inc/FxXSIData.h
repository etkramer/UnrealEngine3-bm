//------------------------------------------------------------------------------
// The static global data maintained by the FaceFx XSI plug-in.
//------------------------------------------------------------------------------

#ifndef _FXXSIDATA_H__
#define _FXXSIDATA_H__

#include "FxToolXSI.h"

namespace OC3Ent
{

	namespace Face
	{

		// Static global data for the XSI plug-in.
		class FxXSIData
		{
		public:
			// The XSI plug-in interface.
			static FxToolXSI xsiInterface;
		};

	} // namespace Face

} // namespace OC3Ent

#endif //_FXXSIDATA_H__

