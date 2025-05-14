class ActorFactoryAmbientSoundSimple extends ActorFactory
    native
    config(Editor)
    editinlinenew
    collapsecategories;

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
	virtual UBOOL CanCreateActor(FString& OutErrorMsg);
	virtual void AutoFillFields(class USelection* Selection);
	virtual FString GetMenuName();
}

var() SoundNodeWave SoundNodeWave;

defaultproperties
{
    MenuName="Add AmbientSoundSimple"
    NewActorClass=Class'AmbientSoundSimple'
}
