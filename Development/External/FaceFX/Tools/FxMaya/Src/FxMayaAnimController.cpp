//------------------------------------------------------------------------------
// Controls access to Maya bone animation curves.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaAnimController.h"

#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>

using namespace OC3Ent;
using namespace Face;

// Constructor.
FxMayaAnimCurve::
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
				 const MObject& scaleZNode )
				 : boneName(inBoneName)
				 , boneObject(inBoneObject)
				 , isTranslateXValid(translateXValid)
				 , translateX(translateXNode)
				 , isTranslateYValid(translateYValid)
				 , translateY(translateYNode)
				 , isTranslateZValid(translateZValid)
				 , translateZ(translateZNode)
				 , isRotateXValid(rotateXValid)
				 , rotateX(rotateXNode)
				 , isRotateYValid(rotateYValid)
				 , rotateY(rotateYNode)
				 , isRotateZValid(rotateZValid)
				 , rotateZ(rotateZNode)
				 , isScaleXValid(scaleXValid)
				 , scaleX(scaleXNode)
				 , isScaleYValid(scaleYValid)
				 , scaleY(scaleYNode)
				 , isScaleZValid(scaleZValid)
				 , scaleZ(scaleZNode)
{
}

// Destructor.
FxMayaAnimCurve::~FxMayaAnimCurve()
{
}

// Constructor.
FxMayaAnimController::FxMayaAnimController()
{
}

// Destructor.
FxMayaAnimController::~FxMayaAnimController()
{
	_clear();
}

// Initializes the controller with animation curves for the specified
// bones.
MStatus 
FxMayaAnimController::Initialize( const OC3Ent::Face::FxArray<OC3Ent::Face::FxString>& bones )
{
	// Clear any previous entries.
	_clear();
	
	// Set up the attribute names to attach to.
	MString attrTranslateX("translateX");
	MString attrTranslateY("translateY");
	MString attrTranslateZ("translateZ");

	MString attrRotateX("rotateX");
	MString attrRotateY("rotateY");
	MString attrRotateZ("rotateZ");

	MString attrScaleX("scaleX");
	MString attrScaleY("scaleY");
	MString attrScaleZ("scaleZ");

	// Hook into each attribute for each bone.
	MStatus status;
	FxBool allMatching = FxTrue;
	for( FxSize i = 0; i < bones.Length(); ++i )
	{
		FxBool foundMatch = FxFalse;

		MItDag dagIterator(MItDag::kDepthFirst, MFn::kInvalid, &status);
		if( !status )
		{
			status.perror("MItDag constructor");
			return status;
		}

		for( ; !dagIterator.isDone(); dagIterator.next() ) 
		{

			MDagPath dagPath;
			status = dagIterator.getPath(dagPath);
			if( !status ) 
			{
				status.perror("MItDag::getPath");
				continue;
			}

			MFnDagNode dagNode(dagPath, &status);
			if( !status )
			{
				status.perror("MFnDagNode constructor");
				continue;
			}

			if( FxString(dagNode.name().asChar()) == bones[i] )
			{
				foundMatch = FxTrue;
				// Get the attributes by name.
				const MObject attrTX = dagNode.attribute(attrTranslateX, &status);
				const MObject attrTY = dagNode.attribute(attrTranslateY, &status);
				const MObject attrTZ = dagNode.attribute(attrTranslateZ, &status);

				const MObject attrRX = dagNode.attribute(attrRotateX, &status);
				const MObject attrRY = dagNode.attribute(attrRotateY, &status);
				const MObject attrRZ = dagNode.attribute(attrRotateZ, &status);

				const MObject attrSX = dagNode.attribute(attrScaleX, &status);
				const MObject attrSY = dagNode.attribute(attrScaleY, &status);
				const MObject attrSZ = dagNode.attribute(attrScaleZ, &status);

				// Set up the bone animation curve.

				OC3Ent::Face::FxBool isTranslateXValid = FxFalse;
				MObject translateX;
				_getAnimCurveObject(dagPath, dagNode, attrTX, translateX, isTranslateXValid);
				OC3Ent::Face::FxBool isTranslateYValid = FxFalse;
				MObject translateY;
				_getAnimCurveObject(dagPath, dagNode, attrTY, translateY, isTranslateYValid);
				OC3Ent::Face::FxBool isTranslateZValid = FxFalse;
				MObject translateZ;
				_getAnimCurveObject(dagPath, dagNode, attrTZ, translateZ, isTranslateZValid);
				OC3Ent::Face::FxBool isRotateXValid = FxFalse;
				MObject rotateX;
				_getAnimCurveObject(dagPath, dagNode, attrRX, rotateX, isRotateXValid);
				OC3Ent::Face::FxBool isRotateYValid = FxFalse;
				MObject rotateY;
				_getAnimCurveObject(dagPath, dagNode, attrRY, rotateY, isRotateYValid);
				OC3Ent::Face::FxBool isRotateZValid = FxFalse;
				MObject rotateZ;
				_getAnimCurveObject(dagPath, dagNode, attrRZ, rotateZ, isRotateZValid);
				OC3Ent::Face::FxBool isScaleXValid = FxFalse;
				MObject scaleX;
				_getAnimCurveObject(dagPath, dagNode, attrSX, scaleX, isScaleXValid);
				OC3Ent::Face::FxBool isScaleYValid = FxFalse;
				MObject scaleY;
				_getAnimCurveObject(dagPath, dagNode, attrSY, scaleY, isScaleYValid);
				OC3Ent::Face::FxBool isScaleZValid = FxFalse;
				MObject scaleZ;
				_getAnimCurveObject(dagPath, dagNode, attrSZ, scaleZ, isScaleZValid);

				_animCurves.PushBack(new FxMayaAnimCurve(bones[i], dagNode.object(),
					isTranslateXValid, translateX, isTranslateYValid, translateY, isTranslateZValid, translateZ,
					isRotateXValid, rotateX, isRotateYValid, rotateY, isRotateZValid, rotateZ,
					isScaleXValid, scaleX, isScaleYValid, scaleY, isScaleZValid, scaleZ));
			}
		}
		if( !foundMatch )
		{
			allMatching = FxFalse;
		}
	}
	if( !allMatching )
	{
		return MS::kFailure;
	}
	return MS::kSuccess;
}

// Finds the animation curve associated with the specified bone name and
// returns a pointer to it or NULL if it does not exist.
FxMayaAnimCurve* 
FxMayaAnimController::FindAnimCurve( const OC3Ent::Face::FxString& bone )
{
	for( FxSize i = 0; i < _animCurves.Length(); ++i )
	{
		if( _animCurves[i] )
		{
			if( _animCurves[i]->boneName == bone )
			{
				return _animCurves[i];
			}
		}
	}
	return NULL;
}

// Returns an MObject representing the animation curve node connected to 
// the specified plug of the specified node.  If the MObject does not exist
// it is created within this function.  This function should only be called 
// immediately prior to setting keys on all the bones.  The process of creating
// animation curves without setting keys can implode the face bringing all 
// bones to the origin with zero scale.  This is what forced us to import
// the reference pose at frame zero.
void
FxMayaAnimController::_getAnimCurveObject( const MDagPath& dagPath, 
										   const MFnDagNode& dagNode, 
										   const MObject& attribute,
										   MObject& animCurveNode, 
										   OC3Ent::Face::FxBool& isValid )
{
	// Get the attribute's plug.
	MPlug plug(dagNode.object(), attribute);
	// Find all plugs that input to the attribute's plug.
	MPlugArray plugs;
    plug.connectedTo(plugs, FxTrue, FxFalse);
	for( FxSize i = 0; i < plugs.length(); ++i )
	{
		// If it is an animation curve, return the object associated with it.
		if( plugs[i].node().hasFn(MFn::kAnimCurve) )
		{
			isValid = FxTrue;
			animCurveNode = plugs[i].node();
			return;
		}
	}
	// An animation curve object connected to the specified attributed could 
	// not be found so create one and search for it again.
	MFnAnimCurve animCurve;
	animCurve.create(dagPath.transform(), attribute);

	// To avoid recursion (and the possibility of an infinite loop)
	// We search for the curve again.
	MPlug plug2(dagNode.object(), attribute);
	MPlugArray plugs2;
    plug2.connectedTo(plugs2, FxTrue, FxFalse);
	for( FxSize i = 0; i < plugs2.length(); ++i )
	{
		if( plugs2[i].node().hasFn(MFn::kAnimCurve) )
		{
			isValid = FxTrue;
			animCurveNode = plugs2[i].node();
			return;
		}
	}
	// It still could not be found meaning that it could not be created, so
	// it is therefore not valid.
	isValid = FxFalse;
}

// Clears the list of bone animation curves.
void FxMayaAnimController::_clear( void )
{
	for( FxSize i = 0; i < _animCurves.Length(); ++i )
	{
		delete _animCurves[i];
		_animCurves[i] = NULL;
	}
	_animCurves.Clear();
}
