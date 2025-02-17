/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Used to affect reverb settings in the game and editor.
 */
class ReverbVolume extends Volume
	native
	placeable
	hidecategories(Advanced,Attachment,Collision,Volume);

/**
 * Indicates a reverb preset to use.
 */
enum ReverbPreset
{
    REVERB_Default,
    REVERB_Bathroom,
    REVERB_StoneRoom,
    REVERB_Auditorium,
    REVERB_ConcertHall,
    REVERB_Cave,
    REVERB_Hallway,
    REVERB_StoneCorridor,
    REVERB_Alley,
    REVERB_Forest,
    REVERB_City,
    REVERB_Mountains,
    REVERB_Quarry,
    REVERB_Plain,
    REVERB_ParkingLot,
    REVERB_SewerPipe,
    REVERB_Underwater,
    REVERB_SmallRoom,
    REVERB_MediumRoom,
    REVERB_LargeRoom,
    REVERB_MediumHall,
    REVERB_LargeHall,
    REVERB_Plate,
};

/** Struct encapsulating settings for reverb effects. */
struct native ReverbSettings
{
	/** The reverb preset to employ. */
	var() ReverbPreset	ReverbType;

	/** Volume level of the reverb affect. */
	var() float			Volume;

	/** Time to fade from the current reverb settings into this setting, in seconds. */
	var() float			FadeTime;

	structdefaultproperties
	{
		ReverbType=REVERB_Default
		Volume=0.5
		FadeTime=2.0
	}
};

/**
 * Priority of this volume. In the case of overlapping volumes the one with the highest priority
 * is chosen. The order is undefined if two or more overlapping volumes have the same priority.
 */
var()							float				Priority;

/** Reverb settings to use for this volume. */
var()							ReverbSettings		Settings;

/** Next volume in linked listed, sorted by priority in descending order. */
var const noimport transient	ReverbVolume		NextLowerPriorityVolume;

cpptext
{
	/**
	 * Removes the reverb volume to world info's list of reverb volumes.
	 */
	virtual void ClearComponents();

	/**
	 * callback for changed property 
	 */
	virtual void PostEditChange( UProperty* PropertyThatChanged );
	
protected:
	/**
	 * Adds the reverb volume to world info's list of reverb volumes.
	 */
	virtual void UpdateComponentsInternal(UBOOL bCollisionUpdate = FALSE);
public:
}

defaultproperties
{
	Begin Object Name=BrushComponent0
		CollideActors=False
		BlockActors=False
		BlockZeroExtent=False
		BlockNonZeroExtent=False
		BlockRigidBody=False
	End Object

	bCollideActors=False
	bBlockActors=False
	bProjTarget=False
	SupportedEvents.Empty
	SupportedEvents(0)=class'SeqEvent_Touch'
}
