/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SoundMode extends Object
	native( Sound )
	hidecategories( object );

struct native AudioEQEffect
{
	/** Start time of effect */
	var	native transient	double	RootTime;

	/** High frequency filter cutoff frequency (Hz) */
	var()					float	HFFrequency;
	/** High frequency gain */
	var()					float	HFGain;

	/** Middle frequency filter cutoff frequency (Hz) */
	var()					float	MFCutoffFrequency;
	/** Middle frequency filter bandwidth frequency (Hz) */
	var()					float	MFBandwidthFrequency;
	/** Middle frequency filter gain */
	var()					float	MFGain;

	/** Low frequency filter cutoff frequency (Hz) */
	var()					float	LFFrequency;			
	/** Low frequency filter gain */
	var()					float	LFGain;

	structcpptext
	{
		// Cannot use strcutdefaultproperties here as this class is a member of a native class
		FAudioEQEffect( void ) :
			RootTime( 0.0 ),
			HFFrequency( 12000.0f ),
			HFGain( 1.0f ),
			MFCutoffFrequency( 8000.0f ),
			MFBandwidthFrequency( 1000.0f ),
			MFGain( 1.0f ),
			LFFrequency( 2000.0f ),
			LFGain( 1.0f )
		{
		}

		/** Interpolates between Start and Ed reverb effect settings */
		void Interpolate( FLOAT InterpValue, FAudioEQEffect& Start, FAudioEQEffect& End );
	}
};

/**
 * Elements of data for sound group volume control
 */
struct native SoundClassAdjuster
{
	var()	transient	ESoundClassName	SoundClassName;
	var					name			SoundClass;
	var()				float			VolumeAdjuster;
	var()				float			PitchAdjuster;

	structdefaultproperties
	{
		SoundClassName="Master"
		SoundClass=Master
		VolumeAdjuster=1
		PitchAdjuster=1
	}
};

/** Whether to apply the EQ effect */
var( EQ )			bool							bApplyEQ<ToolTip=Whether to apply an EQ effect.>;
var( EQ )			AudioEQEffect					EQSettings;

/** Array of changes to be applied to groups */
var( SoundClasses )	array<SoundClassAdjuster>		SoundClassEffects;

var()				float							InitialDelay<ToolTip=Initial delay in seconds before the the mode is applied.>;
var()				float							Duration<ToolTip=Duration of mode, negative means it will be applied until another mode is set.>;
var()				float							FadeInTime<ToolTip=Time taken in seconds for the mode to fade in.>;
var()				float							FadeOutTime<ToolTip=Time taken in seconds for the mode to fade out.>;

cpptext
{
	// UObject interface.
	virtual void Serialize( FArchive& Ar );

	/**
	 * Returns a description of this object that can be used as extra information in list views.
	 */
	virtual FString GetDesc( void );

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT InIndex );

	/** 
	 * Populate the enum using the serialised fname
	 */
	void Fixup( void );

	/**
	 * Called when a property value from a member struct or array has been changed in the editor.
	 */
	virtual void PostEditChange( UProperty* PropertyThatChanged );
}

defaultproperties
{
	bApplyEQ=FALSE
	InitialDelay=0.0
	Duration=-1.0
	FadeInTime=0.2
	FadeOutTime=0.2
}
