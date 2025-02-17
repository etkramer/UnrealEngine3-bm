/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ASVSkelComponent extends SkeletalMeshComponent
	transient
	native;

var native const pointer	AnimSetViewerPtr;

/** If TRUE, render a wireframe skeleton of the mesh animated with the raw (uncompressed) animation data. */
var bool		bRenderRawSkeleton;
/** If TRUE, render softbody tetrahedra */
var bool		bShowSoftBodyTetra;

/** Holds onto the bone color that will be used to render the bones of its skeletal mesh */
var Color		BoneColor;

/** If TRUE then the skeletal mesh associated with the component is drawn. */
var bool		bDrawMesh;

cpptext
{
	// UPrimitiveComponent interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
}

defaultproperties
{
	bDrawMesh = true
	bShowSoftBodyTetra = true
	BoneColor = (R=230, G=230, B=255)
}
