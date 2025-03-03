/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class CoverMeshComponent extends StaticMeshComponent
	native(AI);

cpptext
{
	void UpdateBounds();
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual UBOOL ShouldRecreateProxyOnUpdateTransform() const;
	virtual void UpdateMeshes() {};
};

struct native CoverMeshes
{
	var StaticMesh Base;
	var StaticMesh LeanLeft, LeanRight;
	var StaticMesh Climb, Mantle;
	var StaticMesh SlipLeft, SlipRight;
	var StaticMesh SwatLeft, SwatRight;
	var StaticMesh PopUp;
	var StaticMesh PlayerOnly;
};
var editoronly array<CoverMeshes> Meshes;

/** Base offset applied to all meshes */
var vector LocationOffset;

var editoronly StaticMesh AutoAdjustOn, AutoAdjustOff;
var editoronly StaticMesh Disabled;

defaultproperties
{
    LocationOffset=(X=0.0000000,Y=0.0000000,Z=-60.0000000)
    HiddenGame=true
    bAcceptsStaticDecals=false
    bAcceptsDynamicDecals=false
    CastShadow=false
    bAcceptsLights=false
    CollideActors=false
    BlockActors=false
    BlockZeroExtent=false
    BlockNonZeroExtent=false
    BlockRigidBody=false
    AlwaysLoadOnClient=false
    AlwaysLoadOnServer=false
}
