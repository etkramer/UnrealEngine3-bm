/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_SetStaticMesh extends SequenceAction;

/** New mesh to use for the target actor */
var()	StaticMesh	NewStaticMesh;
/** if True then the mesh will be treated as if it is movable */
var()	bool		bIsAllowedToMove;
/** if True then any decals attached to the previous mesh will be reattached to the new mesh */
var()	bool		bAllowDecalsToReattach;

defaultproperties
{
	ObjName="Set StaticMesh"
	ObjCategory="Actor"
	bIsAllowedToMove=true
	bAllowDecalsToReattach=false
}
