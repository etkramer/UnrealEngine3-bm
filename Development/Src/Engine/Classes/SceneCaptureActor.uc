/**
 * SceneCaptureActor
 *
 * Base class for actors that want to capture the scene
 * using a scene capture component 
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SceneCaptureActor extends Actor
	native
	abstract;

/** component that renders the scene to a texture */
var() const SceneCaptureComponent SceneCapture;

cpptext
{
	/** 
	* Update any components used by this actor
	*/
	virtual void SyncComponents() {}

	// UObject interface

	/** 
	* Callback for a property change
	* @param PropertyThatChanged - updated property
	*/
	virtual void PostEditChange( UProperty* PropertyThatChanged );

	/** 
	* Called after object load
	*/
	virtual void PostLoad();

	// AActor interface.

	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors();
}

defaultproperties
{
	// editor-only sprite
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	// allow for actor to tick locally on clients
	bNoDelete=true
	RemoteRole=ROLE_SimulatedProxy	
}
