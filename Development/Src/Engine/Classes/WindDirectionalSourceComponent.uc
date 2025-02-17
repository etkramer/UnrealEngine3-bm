/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class WindDirectionalSourceComponent extends ActorComponent
	native
	collapsecategories
	hidecategories(Object)
	editinlinenew;

var native private	transient noimport const pointer SceneProxy{FWindSourceSceneProxy};

var() float	Strength;
var() float Phase;
var() float Frequency;
var() float Speed;

cpptext
{
protected:
	// UActorComponent interface.
	virtual void Attach();
	virtual void Detach( UBOOL bWillReattach = FALSE );
public:
	
	/**
	 * Creates a proxy to represent the wind source to the scene manager in the rendering thread.
	 * @return The proxy object.
	 */
	 virtual class FWindSourceSceneProxy* CreateSceneProxy() const;
}

defaultproperties
{
	Strength=1.0
	Frequency=1.0
	Speed=1024.0
}
