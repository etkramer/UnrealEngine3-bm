//------------------------------------------------------------------------------
// Controls access to XSI bone animation curves.
//------------------------------------------------------------------------------

#ifndef _FXXSIANIMCONTROLLER_H__
#define _FXXSIANIMCONTROLLER_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxString.h"

#include <xsi_ref.h>
#include <xsi_fcurve.h>


// Controls access to a single Maya bone animation curve.
class FxXSIAnimCurve
{
public:
	// Constructor.
	FxXSIAnimCurve( const OC3Ent::Face::FxString& inBoneName,
		const XSI::CRef& inBoneObject,
		const XSI::CRef& translateXNode,
		const XSI::CRef& translateYNode,
		const XSI::CRef& translateZNode,
		const XSI::CRef& rotateXNode,
		const XSI::CRef& rotateYNode,
		const XSI::CRef& rotateZNode,
		const XSI::CRef& scaleXNode,
		const XSI::CRef& scaleYNode,
		const XSI::CRef& scaleZNode );
	// Destructor.
	~FxXSIAnimCurve();

	// The bone name.	
	OC3Ent::Face::FxString m_boneName;
	// The object corresponding to the bone.
	XSI::CRef m_boneObject;

	// Animation curve for the x-axis translation component.
	XSI::FCurve m_translateX;
	// An`imation curve for the y-axis translation component.
	XSI::FCurve m_translateY;
	// Animation curve for the z-axis translation component.
	XSI::FCurve m_translateZ;
	// Animation curve for the x-axis rotation component.
	XSI::FCurve m_rotateX;
	// Animation curve for the y-axis rotation component.
	XSI::FCurve m_rotateY;
	// Animation curve for the z-axis rotation component.
	XSI::FCurve m_rotateZ;
	// Animation curve for the x-axis scale component.
	XSI::FCurve m_scaleX;
	// Animation curve for the y-axis scale component.
	XSI::FCurve m_scaleY;
	// Animation curve for the z-axis scale component.
	XSI::FCurve m_scaleZ;
};

// Controls access to XSI bone animation curves.
class FxXSIAnimController
{
public:
	// Constructor.
	FxXSIAnimController();
	// Destructor.
	~FxXSIAnimController();

	// Initializes the controller with animation curves for the specified
	// bones.
	bool Initialize( const OC3Ent::Face::FxArray<OC3Ent::Face::FxString>& bones );

	// Finds the animation curve associated with the specified bone name and
	// returns a pointer to it or NULL if it does not exist.
	FxXSIAnimCurve* FindAnimCurve( const OC3Ent::Face::FxString& bone );

	// Removes all keys from all bone animation curves.
	void Clean( void );

protected:
	// The list of bone animation curves.
	OC3Ent::Face::FxArray<FxXSIAnimCurve*> m_animCurves;

	// Clears the list of bone animation curves.
	void _clear( void );
};

#endif //_FXXSIANIMCONTROLLER_H__
