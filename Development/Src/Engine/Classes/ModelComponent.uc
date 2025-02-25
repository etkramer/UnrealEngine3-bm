/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ModelComponent extends PrimitiveComponent
	native
	noexport;


/** These mirror the C++ side properties. I'm making a class here so
    ModelComponent will get the defaultprops from the PrimitiveComponent base class */
var transient native const noexport object Model;
var transient native const noexport int ZoneIndex;
var transient native const noexport int ComponentIndex; // (note that this is a WORD in C++, but alignment will make everything line up okay)
var transient native const noexport array<pointer> Nodes;
var transient native const noexport array<pointer> Edges;
var transient native const noexport array<pointer> Elements;

defaultproperties
{
	LightingChannels=(BSP=TRUE,bInitialized=TRUE)
	CastShadow=TRUE
	bAcceptsLights=TRUE
	bAcceptsStaticDecals=TRUE
	bUsePrecomputedShadows=TRUE
	bUseAsOccluder=TRUE
	bCullModulatedShadowOnBackfaces=TRUE
	bCullModulatedShadowOnEmissive=TRUE
}
