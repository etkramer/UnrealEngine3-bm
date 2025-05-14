class ActorFactory extends Object
    abstract
    native
    config(Editor)
    editinlinenew
    collapsecategories;

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

var class<Actor> GameplayActorClass;
var string MenuName;
var config int MenuPriority;
var class<Actor> NewActorClass;
var bool bPlaceable;
var bool UseActorSelection;
var string SpecificGameName;

defaultproperties
{
    MenuName="Add Actor"
    NewActorClass=Class'Actor'
    bPlaceable=true
}
