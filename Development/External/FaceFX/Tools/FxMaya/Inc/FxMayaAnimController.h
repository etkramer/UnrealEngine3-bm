//------------------------------------------------------------------------------
// Controls access to Maya bone animation curves.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMayaAnimController_H__
#define FxMayaAnimController_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxString.h"

#include "FxMayaMain.h"

#include <maya/MString.h>
#include <maya/MObject.h>
#include <maya/MMatrix.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>

// Controls access to a single Maya bone animation curve.
class FxMayaAnimCurve
{
public:
	// Constructor.
	FxMayaAnimCurve( const OC3Ent::Face::FxString& inBoneName,
				     const MObject& inBoneObject,
					 OC3Ent::Face::FxBool translateXValid,
					 const MObject& translateXNode,
					 OC3Ent::Face::FxBool translateYValid,
		             const MObject& translateYNode,
					 OC3Ent::Face::FxBool translateZValid,
					 const MObject& translateZNode,
					 OC3Ent::Face::FxBool rotateXValid,
					 const MObject& rotateXNode,
					 OC3Ent::Face::FxBool rotateYValid,
					 const MObject& rotateYNode,
					 OC3Ent::Face::FxBool rotateZValid,
					 const MObject& rotateZNode,
					 OC3Ent::Face::FxBool scaleXValid,
					 const MObject& scaleXNode,
					 OC3Ent::Face::FxBool scaleYValid,
					 const MObject& scaleYNode,
					 OC3Ent::Face::FxBool scaleZValid,
					 const MObject& scaleZNode );
	// Destructor.
	~FxMayaAnimCurve();

	// The bone name.	
	OC3Ent::Face::FxString boneName;
	// The object corresponding to the bone.
	MObject boneObject;

	// Animation curve for the x-axis translation component.
	OC3Ent::Face::FxBool isTranslateXValid;
	MFnAnimCurve translateX;
	// Animation curve for the y-axis translation component.
	OC3Ent::Face::FxBool isTranslateYValid;
	MFnAnimCurve translateY;
	// Animation curve for the z-axis translation component.
	OC3Ent::Face::FxBool isTranslateZValid;
	MFnAnimCurve translateZ;
	// Animation curve for the x-axis rotation component.
	OC3Ent::Face::FxBool isRotateXValid;
	MFnAnimCurve rotateX;
	// Animation curve for the y-axis rotation component.
	OC3Ent::Face::FxBool isRotateYValid;
	MFnAnimCurve rotateY;
	// Animation curve for the z-axis rotation component.
	OC3Ent::Face::FxBool isRotateZValid;
	MFnAnimCurve rotateZ;
	// Animation curve for the x-axis scale component.
	OC3Ent::Face::FxBool isScaleXValid;
	MFnAnimCurve scaleX;
	// Animation curve for the y-axis scale component.
	OC3Ent::Face::FxBool isScaleYValid;
	MFnAnimCurve scaleY;
	// Animation curve for the z-axis scale component.
	OC3Ent::Face::FxBool isScaleZValid;
	MFnAnimCurve scaleZ;
};

// Controls access to Maya bone animation curves.
class FxMayaAnimController
{
public:
	// Constructor.
	FxMayaAnimController();
	// Destructor.
	~FxMayaAnimController();

	// Initializes the controller with animation curves for the specified
	// bones.
	MStatus Initialize( const OC3Ent::Face::FxArray<OC3Ent::Face::FxString>& bones );

	// Finds the animation curve associated with the specified bone name and
	// returns a pointer to it or NULL if it does not exist.
	FxMayaAnimCurve* FindAnimCurve( const OC3Ent::Face::FxString& bone );

	// Removes all keys from all bone animation curves.
	void Clean( void );

protected:
	// The list of bone animation curves.
	OC3Ent::Face::FxArray<FxMayaAnimCurve*> _animCurves;

	// Sets the animCurveNode MObject to represent the animation curve 
	// node connected to the specified plug of the specified node.  If the MObject does not exist
	// it is created within this function.  If that fails, isValid is set to FxFalse.
	void _getAnimCurveObject( const MDagPath& dagPath, const MFnDagNode& dagNode, 
		                      const MObject& attribute, MObject& animCurveNode, 
							  OC3Ent::Face::FxBool& isValid );

	// Clears the list of bone animation curves.
	void _clear( void );
};

#endif
