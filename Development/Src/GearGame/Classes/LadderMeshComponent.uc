/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class LadderMeshComponent extends StaticMeshComponent
	native(AI);

cpptext
{
	void UpdateBounds();
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual UBOOL ShouldRecreateProxyOnUpdateTransform() const;
	virtual void UpdateMeshes() {};
};

struct native LadderMeshes
{
	var StaticMesh Base;
};
var editoronly array<LadderMeshes> Meshes;

/** Base offset applied to all meshes */
var vector LocationOffset;

defaultproperties
{
	HiddenGame=TRUE
	AlwaysLoadOnServer=FALSE
	AlwaysLoadOnClient=FALSE
	CollideActors=FALSE
	BlockActors=FALSE
	BlockZeroExtent=FALSE
	BlockNonZeroExtent=FALSE
	BlockRigidBody=FALSE
	bAcceptsStaticDecals=FALSE
	bAcceptsDynamicDecals=FALSE
	bAcceptsLights=FALSE
	CastShadow=FALSE

	StaticMesh=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy__BASE_TALL'
	LocationOffset=(X=0,Y=0,Z=-24)
	
	Meshes(0)=(Base=StaticMesh'NodeBuddies.3D_Icons.NodeBuddy__BASE_TALL')
}
