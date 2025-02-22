/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Used to affect post process settings in the game and editor.
 */
class PostProcessVolume extends Volume
	native
	placeable
	dependson(DOFEffect)
	hidecategories(Advanced,Collision,Volume);

struct native PostProcessSettings
{
	/** Whether to use bloom effect.																*/
	var()	bool			bEnableBloom;
	/** Whether to use depth of field effect.														*/
	var()	bool			bEnableDOF;
	/** Whether to use motion blur effect.															*/
	var()	bool			bEnableMotionBlur;
	/** Whether to use the material/ scene effect.													*/
	var()	bool			bEnableSceneEffect;
	/** Whether to allow ambient occlusion.															*/
	var()	bool			bAllowAmbientOcclusion;

	// BM1
	var() bool bAllowAtmospheric;
    var() Color Atmospheric_ForegroundColour;
    var() interp float Atmospheric_ForegroundStrength;
    var() Color Atmospheric_BackgroundColour;
    var() interp float Atmospheric_BackgroundStrength;
    var() interp float Atmospheric_ForegroundMaxDistance;
    var() interp float Atmospheric_ForegroundWidth;
    var() interp float Atmospheric_BackgroundMaxDistance;
    var() interp float Atmospheric_BackgroundWidth;

	/** Scale for the blooming.																		*/
	var()	interp float	Bloom_Scale;
	/** Duration over which to interpolate values to.												*/
	var()	float			Bloom_InterpolationDuration;

	/** Exponent to apply to blur amount after it has been normalized to [0,1].						*/
	var()	interp float	DOF_FalloffExponent;
	/** Affects the size of the Poisson disc kernel.												*/
	var()	interp float	DOF_BlurKernelSize;
	/** [0,1] value for clamping how much blur to apply to items in front of the focus plane.		*/
	var()	interp float	DOF_MaxNearBlurAmount;
	/** [0,1] value for clamping how much blur to apply to items behind the focus plane.			*/
	var()	interp float	DOF_MaxFarBlurAmount;
	/** Blur color for debugging etc.																*/
	var()	color			DOF_ModulateBlurColor;
	/** Controls how the focus point is determined.													*/
	var()	EFocusType		DOF_FocusType;
	/** Inner focus radius.																			*/
	var()	interp float	DOF_FocusInnerRadius;
	/** Used when FOCUS_Distance is enabled.														*/
	var()	interp float	DOF_FocusDistance;
	/** Used when FOCUS_Position is enabled.														*/
	var()	vector			DOF_FocusPosition;
	/** Duration over which to interpolate values to.												*/
	var()	float			DOF_InterpolationDuration;

	/** Maximum blur velocity amount.  This is a clamp on the amount of blur.						*/
	var()	interp float	MotionBlur_MaxVelocity;
	/** This is a scalar on the blur																*/
	var()	interp float	MotionBlur_Amount;
	/** Whether everything (static/dynamic objects) should motion blur or not. If disabled, only moving objects may blur. */
	var()	bool			MotionBlur_FullMotionBlur;
	/** Threshhold for when to turn off motion blur when the camera rotates swiftly during a single frame (in degrees). */
	var()	interp float	MotionBlur_CameraRotationThreshold;
	/** Threshhold for when to turn off motion blur when the camera translates swiftly during a single frame (in world units). */
	var()	interp float	MotionBlur_CameraTranslationThreshold;
	/** Duration over which to interpolate values to.												*/
	var()	float			MotionBlur_InterpolationDuration;

	/** Desaturation amount.																		*/
	var()	interp float	Scene_Desaturation;
	/** Controlling white point.																	*/
	var()	interp vector	Scene_HighLights;
	/** Controlling gamma curve.																	*/
	var()	interp vector	Scene_MidTones;
	/** Controlling black point.																	*/
	var()	interp vector	Scene_Shadows;
	/** Duration over which to interpolate values to.												*/
	var()	float			Scene_InterpolationDuration;

structcpptext
{
	FPostProcessSettings()
	{}

	FPostProcessSettings(INT A)
	{
		bEnableBloom=TRUE;
		bEnableDOF=FALSE;
		bEnableMotionBlur=TRUE;
		bEnableSceneEffect=TRUE;
		bAllowAmbientOcclusion=TRUE;

		Bloom_Scale=1;
		Bloom_InterpolationDuration=1;

		DOF_FalloffExponent=4;
		DOF_BlurKernelSize=16;
		DOF_MaxNearBlurAmount=1;
		DOF_MaxFarBlurAmount=1;
		DOF_ModulateBlurColor=FColor(255,255,255,255);
		DOF_FocusType=FOCUS_Distance;
		DOF_FocusInnerRadius=2000;
		DOF_FocusDistance=0;
		DOF_InterpolationDuration=1;

		MotionBlur_MaxVelocity=1.0f;
		MotionBlur_Amount=0.5f;
		MotionBlur_FullMotionBlur=TRUE;
		MotionBlur_CameraRotationThreshold=90.0f;
		MotionBlur_CameraTranslationThreshold=10000.0f;
		MotionBlur_InterpolationDuration=1;

		Scene_Desaturation=0;
		Scene_HighLights=FVector(1,1,1);
		Scene_MidTones=FVector(1,1,1);
		Scene_Shadows=FVector(0,0,0);
		Scene_InterpolationDuration=1;
	}

	/**
	 * Adds the values of the interplatable parameters from the given settings struct.
	 * Each entry is multiplied by scale before adding.
	 */
	void AddInterpolatable(FPostProcessSettings const& AdditiveSettings, FLOAT Scale=1.f)
	{
		Bloom_Scale += Scale * AdditiveSettings.Bloom_Scale;
		DOF_FalloffExponent += Scale * AdditiveSettings.DOF_FalloffExponent;
		DOF_BlurKernelSize += Scale * AdditiveSettings.DOF_BlurKernelSize;
		DOF_MaxNearBlurAmount += Scale * AdditiveSettings.DOF_MaxNearBlurAmount;
		DOF_MaxFarBlurAmount += Scale * AdditiveSettings.DOF_MaxFarBlurAmount;
		DOF_FocusInnerRadius += Scale * AdditiveSettings.DOF_FocusInnerRadius;
		DOF_FocusDistance += Scale * AdditiveSettings.DOF_FocusDistance;
		MotionBlur_MaxVelocity += Scale * AdditiveSettings.MotionBlur_MaxVelocity;
		MotionBlur_Amount += Scale * AdditiveSettings.MotionBlur_Amount;
		MotionBlur_CameraRotationThreshold += Scale * AdditiveSettings.MotionBlur_CameraRotationThreshold;
		MotionBlur_CameraTranslationThreshold += Scale * AdditiveSettings.MotionBlur_CameraTranslationThreshold;
		Scene_Desaturation += Scale * AdditiveSettings.Scene_Desaturation;
		Scene_HighLights += Scale * AdditiveSettings.Scene_HighLights;
		Scene_MidTones += Scale * AdditiveSettings.Scene_MidTones;
		Scene_Shadows += Scale * AdditiveSettings.Scene_Shadows;
	}
}

	structdefaultproperties
	{
		bEnableBloom=TRUE
		bEnableDOF=FALSE
		bEnableMotionBlur=TRUE
		bEnableSceneEffect=TRUE
		bAllowAmbientOcclusion=TRUE

		Bloom_Scale=1
		Bloom_InterpolationDuration=1

		DOF_FalloffExponent=4
		DOF_BlurKernelSize=16
		DOF_MaxNearBlurAmount=1
		DOF_MaxFarBlurAmount=1
		DOF_ModulateBlurColor=(R=255,G=255,B=255,A=255)
		DOF_FocusType=FOCUS_Distance
		DOF_FocusInnerRadius=2000
		DOF_FocusDistance=0
		DOF_InterpolationDuration=1

		MotionBlur_MaxVelocity=1.0
		MotionBlur_Amount=0.5
		MotionBlur_FullMotionBlur=TRUE
		MotionBlur_CameraRotationThreshold=45.0
		MotionBlur_CameraTranslationThreshold=10000.0
		MotionBlur_InterpolationDuration=1

		Scene_Desaturation=0
		Scene_HighLights=(X=1,Y=1,Z=1)
		Scene_MidTones=(X=1,Y=1,Z=1)
		Scene_Shadows=(X=0,Y=0,Z=0)
		Scene_InterpolationDuration=1
	}

};

/**
 * Priority of this volume. In the case of overlapping volumes the one with the highest priority
 * is chosen. The order is undefined if two or more overlapping volumes have the same priority.
 */
var()							float					Priority;

/**
 * Post process settings to use for this volume.
 */
var()							PostProcessSettings		Settings;

/** Next volume in linked listed, sorted by priority in descending order.							*/
var const noimport transient	PostProcessVolume		NextLowerPriorityVolume;


/** Whether this volume is enabled or not.															*/
var()							bool					bEnabled;

replication
{
	if (bNetDirty)
		bEnabled;
}

/**
 * Kismet support for toggling bDisabled.
 */
simulated function OnToggle(SeqAct_Toggle action)
{
	if (action.InputLinks[0].bHasImpulse)
	{
		// "Turn On" -- mapped to enabling of volume.
		bEnabled = TRUE;
	}
	else if (action.InputLinks[1].bHasImpulse)
	{
		// "Turn Off" -- mapped to disabling of volume.
		bEnabled = FALSE;
	}
	else if (action.InputLinks[2].bHasImpulse)
	{
		// "Toggle"
		bEnabled = !bEnabled;
	}
	ForceNetRelevant();
	SetForcedInitialReplicatedProperty(Property'Engine.PostProcessVolume.bEnabled', (bEnabled == default.bEnabled));
}

cpptext
{
	/**
	 * Routes ClearComponents call to Super and removes volume from linked list in world info.
	 */
	virtual void ClearComponents();
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PostLoad();
protected:
	/**
	 * Routes UpdateComponents call to Super and adds volume to linked list in world info.
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
	bStatic=false
	bStasis=true

	SupportedEvents.Empty
	SupportedEvents(0)=class'SeqEvent_Touch'

	bEnabled=True
}
