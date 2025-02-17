/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class EmergenceHoleRenderingComponent extends PrimitiveComponent
	native;

cpptext
{
	FPrimitiveSceneProxy* CreateSceneProxy();
};

defaultproperties
{
	HiddenGame=TRUE
	AlwaysLoadOnClient=FALSE
	AlwaysLoadOnServer=FALSE
}
