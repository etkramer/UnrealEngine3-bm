/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ActorFactory extends Object
	native
	collapsecategories
	hidecategories(Object)
	editinlinenew
	config(Editor)
	abstract;

cpptext
{
	/** Called to actual create an actor at the supplied location/rotation, using the properties in the ActorFactory */
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );

	virtual void PostEditChange( UProperty* PropertyThatChanged );


	/**
	 * If the ActorFactory thinks it could create an Actor with the current settings. 
	 * Used to determine if we should add to context menu for example.
	 *
	 * @param	OutErrorMsg		Receivews localized error string name if returning FALSE.
	 */
	virtual UBOOL CanCreateActor(FString& OutErrorMsg) { return TRUE; }

	/** Fill in parameters automatically, possibly using the specified selection set. */
	virtual void AutoFillFields(class USelection* Selection) {}

	/** Name to put on context menu. */
	virtual FString GetMenuName() { return MenuName; }

	virtual AActor* GetDefaultActor();
	
    protected:
        /**
		 * This will check whether there is enough space to spawn an character.
		 * Additionally it will check the ActorFactoryData to for any overrides 
		 * ( e.g. bCheckSpawnCollision )
		 *
		 * @return if there is enough space to spawn character at this location
		 **/
		UBOOL IsEnoughRoomToSpawnPawn( const FVector* const Location, const class USeqAct_ActorFactory* const ActorFactoryData ) const;

}

/** class to spawn during gameplay; only used if NewActorClass is left at the default */
var class<Actor> GameplayActorClass;

/** Name used as basis for 'New Actor' menu. */
var string			MenuName;

/** Indicates how far up the menu item should be. The higher the number, the higher up the list.*/
var config int		MenuPriority;

/** Actor subclass this ActorFactory creates. */
var	class<Actor>	NewActorClass;

/** Whether to appear on menu (or this Factory only used through scripts etc.) */
var bool			bPlaceable;

/** If this is associated with a specific game, don't display
	If this is empty string, display for all games */
var string			SpecificGameName;


defaultproperties
{
	MenuName="Add Actor"
	NewActorClass=class'Engine.Actor'
	bPlaceable=true
}
