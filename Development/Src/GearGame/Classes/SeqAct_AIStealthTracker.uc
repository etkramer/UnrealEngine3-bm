/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_AIStealthTracker extends SequenceAction
	native(Sequence);

cpptext
{
	virtual void	OnCreated();
	virtual void	PostLoad();
	virtual void	Activated();
	virtual UBOOL	UpdateOp(FLOAT DeltaTime);
	virtual void	UpdateDynamicLinks();
	virtual void	OnReceivedImpulse( class USequenceOp* ActivatorOp, INT InputLinkIndex );

	void WriteOutputVars( AActor* Spotter, AActor* Spotted );
}

enum ELayerTrackerType
{
	LTT_ByTime,
	LTT_ByObviousBehavior,
};

enum ELayerUpdateResult
{
	LUR_None,
	LUR_StartTracking,
	LUR_StopTracking,
	LUR_Alarm,
};

struct native StealthTargetInfo
{
	/** Target being tracked */
	var	Actor	Target;
	/** Internal timer updated when layer is recognizing player */
	var	float	Timer;
};

struct native StealthTrackerInfo
{
	/** Tracker looking for targets */
	var Actor	Tracker;
	/** Array of info for each enemy being tracked */
	var array<StealthTargetInfo>	TargetInfo;
};

struct native StealthLayerInfo
{
	/** Description of layer */
	var() String	LayerDesc;
	/** Radius of this layer */
	var() float		Radius;
	/** Time limit to reach before this layer fires output */
	var() float		TimeLimit;
	/** Do line trace from tracker to target to try to update? */
	var() bool		bDoVisibilityCheck;
	/** Do FOV checks for trackers */
	var() bool		bDoFOVCheck;
	var() float		FOV;
	/** This layer will trigger "track" for AI targets */
	var() bool		bAIWillTriggerTrack;
	/** This layer will trigger "alarm" for AI targets */
	var() bool		bAIWillTriggerAlarm;
	

	/** Delegate used to update this layer */
	var() ELayerTrackerType	UpdateType;

	/** Array of info for each tracker looking for targets */
	var array<StealthTrackerInfo>	TrackerInfo;

	structdefaultproperties
	{
		LayerDesc="CHANGE ME"
		Radius=128.f
		TimeLimit=5.f
		bDoVisibilityCheck=TRUE
		bDoFOVCheck=FALSE
		FOV=0.7f
		UpdateType=LTT_ByTime
		bAIWillTriggerTrack=TRUE
		bAIWillTriggerAlarm=TRUE
	}
};

/** List of layers to track - assumes smallest to largest radius ordering */
var() array<StealthLayerInfo>	Layers;
/** List of objects to track targets from */
var() array<Object> Trackers;

/** Deactivate action when layer triggers */
var() bool	bDeactivateOnTrigger;

/** List of layer descriptions - used to update outputs */
var array<string> LayerDescs;

/** "Disable" input link has received an impulse */
var		   bool	bAbortTracking;
/** Draw debug rings + colors for each layer/tracker */
var(Debug) bool bDrawDebugLayers;

/**
 *	Delegate for updating a layer
 *	when DeltaTime > 0 returns TRUE if layer time limit has been reached, FALSE otherwise
 *	when DeltaTime < 0 returns TRUE if layer time limit has dropped back to zero, FALSE otherwise
 */
delegate ELayerUpdateResult OnUpdateStealthLayer( int LayerIdx, float DeltaTime, Actor Tracker, Actor Target );

/**
 *	Delegate for testing whether to update given layer
 *	returns TRUE if target is trackable by given tracker, false otherwise
 */
delegate bool OnCheckStealthLayer( int LayerIdx, Actor Tracker, Actor Target );

/**
 *	Setups up Update timers based on the Layer index
 */
event SetUpdateDelegate( int LayerIdx )
{
	switch( Layers[LayerIdx].UpdateType )
	{
		case LTT_ByObviousBehavior:
			OnUpdateStealthLayer = UpdateLayerDelegate_ByTime;
			OnCheckStealthLayer  = CheckLayerDelegate_ByBehavior;
			break;
		case LTT_ByTime:
		default:
			OnUpdateStealthLayer = UpdateLayerDelegate_ByTime;
			OnCheckStealthLayer  = CheckLayerDelegate_ByRange;
			break;
	}
}

native function bool CheckLayerDelegate_ByRange( int LayerIdx, Actor Tracker, Actor Target );
native function bool CheckLayerDelegate_ByBehavior( int LayerIdx, Actor Tracker, Actor Target );
native function ELayerUpdateResult UpdateLayerDelegate_ByTime( int LayerIdx, float DeltaTime, Actor Tracker, Actor Target );
native function bool GetTrackerTargetPairIndices( int LayerIdx, Actor Tracker, Actor Target, out int out_TrackerIdx, out int out_TargetIdx, bool bCreateEntry );

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 3;
}

defaultproperties
{
	ObjName="AI: Stealth Tracker"
	ObjCategory="AI"

	bLatentExecution=TRUE
	bAutoActivateOutputLinks=FALSE
	bDeactivateOnTrigger=TRUE

	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Targets",PropertyName=Targets)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Trackers",PropertyName=Trackers)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Spotted",bWriteable=true)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Object',LinkDesc="Spotter",bWriteable=true)

	Layers(0)=(LayerDesc="Too Close",Radius=128.f,TimeLimit=1.f,UpdateType=LTT_ByTime)
	Layers(1)=(LayerDesc="Suspicion",Radius=256.f,TimeLimit=3.f,UpdateType=LTT_ByTime)
	Layers(2)=(LayerDesc="Behavior",Radius=768.f,TimeLimit=8.f,UpdateType=LTT_ByObviousBehavior)

	OnUpdateStealthLayer=UpdateLayerDelegate_ByTime
}


