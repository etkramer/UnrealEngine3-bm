/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class CombatZoneRenderingComponent extends PrimitiveComponent
	native(AI);

cpptext
{
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void UpdateBounds();
	virtual UBOOL ShouldRecreateProxyOnUpdateTransform() const;
};

defaultproperties
{
	HiddenGame=TRUE
}
