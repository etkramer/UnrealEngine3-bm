//------------------------------------------------------------------------------
// Controls access to Max bone animation curves.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMaxAnimController_H__
#define FxMaxAnimController_H__

#include "FxMaxMain.h"
#include "FxPlatform.h"
#include "FxArray.h"
#include "FxString.h"

// Controls access to a single Max bone animation curve.
class FxMaxAnimCurve
{
public:
	// Constructor.
	FxMaxAnimCurve( const OC3Ent::Face::FxString& inBoneName );
	// Destructor.
	~FxMaxAnimCurve();

	// The bone name.	
	OC3Ent::Face::FxString boneName;

	// The node associated with the bone in the Max scene.
	INode* pNode;

	// The position controller.
	Control* pPositionController;
	// The scale controller.
	Control* pScaleController;
	// The rotation controller.
	Control* pRotationController;

	// FxTrue if the rotation controller was previously a TCB controller.
	OC3Ent::Face::FxBool wasTCB;

	// The position keys.
	IKeyControl* pPositionKeys;
	// The scale keys.
	IKeyControl* pScaleKeys;
	// The rotation keys.
	IKeyControl* pRotationKeys;
};

// Controls access to Max bone animation curves.
class FxMaxAnimController
{
public:
	// Constructor.
	FxMaxAnimController();
	// Destructor.
	~FxMaxAnimController();

	// Sets the Max interface pointer.
	void SetMaxInterfacePointer( Interface* pMaxInterface );

	// Initializes the controller with animation curves for the specified
	// bones.
	OC3Ent::Face::FxBool Initialize( const OC3Ent::Face::FxArray<OC3Ent::Face::FxString>& bones );

	// Finds the animation curve associated with the specified bone name and
	// returns a pointer to it or NULL if it does not exist.
	FxMaxAnimCurve* FindAnimCurve( const OC3Ent::Face::FxString& bone );

	// Removes all keys from all bone animation curves.
	void Clean( void );

	// Prepares the bone animation curve rotation controllers so that any TCB
	// controllers are switched to Linear controllers before importing an
	// animation.  This is an optimization for importing animations in Max.
	void PrepareRotationControllers( void );
	// Restores the bone animation curve rotation controllers so that any former
	// TCB controllers and restored back to TCB controllers.  This is an
	// optimization for importing animations in Max.
	void RestoreRotationControllers( void );

protected:
	// The Max interface pointer.
	Interface* _pMaxInterface;
	// The list of bone animation curves.
	OC3Ent::Face::FxArray<FxMaxAnimCurve*> _animCurves;

	// Clears the list of bone animation curves.
	void _clear( void );
};

#endif
