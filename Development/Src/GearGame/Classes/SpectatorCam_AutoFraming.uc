/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SpectatorCam_AutoFraming extends Object
	native(Camera)
	config(Camera);

/** Ref to the camera object that owns this mode object. */
var transient GearSpectatorCamera SpectatorCam;

var protected transient float LastFOV;
/** InterpSpeed for the camera's FOV. */
var() const protected float FOVInterpSpeed;
/** So we can use a faster interpolation speed when the camera is moving. */
var() const protected float FOVInterpSpeed_MovingCamera;


var protected transient rotator LastDeltaRot;
/** InterpSpeed for the camera orientation. */
var() const protected float RotInterpSpeed;
/** We use a faster interpolation speed when the camera is moving. */
var() const protected float RotInterpSpeed_MovingCamera;


var protected transient bool	bLastLookatIsValid;
var protected transient vector	LastLookat;
/** InterpSpeed for the camera's lookat, travelling through world space.. */
var() const protected float		LookatInterpSpeed;
/** So we can use a faster interpolation speed when the camera is moving. */
var() const protected float		LookatInterpSpeed_MovingCamera;


// test vars
var() protected bool bDebugSkipFOVAdj;
var() protected bool bDebugShowWatchedPawns;

// @fixme, enforce these
var() protected const vector2d WorldPitchRange;
var() protected const vector2d RelativeYawRange;
var() protected const vector2d AcceptableFOVRange;

/** When setting FOV, defines how much far from the screen edge to try and keep pawns.  e.g. 0.1 is a 10% "safe zone". */
var() protected const float BorderBufferPercentage_Horizontal;
var() protected const float BorderBufferPercentage_Vertical;

var() protected const float ZoomFOVAdjustmentMag;
var() protected const float ZoomBufferAdjustmentMag;

var() protected const float DirSelectionWeightMultiplier_Good;
var() protected const float DirSelectionWeightMultiplier_Bad;


/** Last known CamActor loc, used to determine if it's moving. */
var protected transient vector LastCamActorLoc;

enum EAutoFramingPawnTracePoint
{
	PawnTrace_Head,
	PawnTrace_Above,
	PawnTrace_Left,
	PawnTrace_Right,
};

/** Internal struct. */
struct native AutoframingWatchedPawn
{
	var GearPawn					GP;
	var vector						LookatLoc;
	var float						LastRelevantTime;
	var EAutoFramingPawnTracePoint	LastTracePoint;
	var float						NormalizedWeight;		// 0..1
	var bool						bNoRelevanceLag;
};

var transient array<AutoframingWatchedPawn>		RelevantPawnList;

/** How long a pawn will stay fully "relevant" after it disappears from sight. */
var() protected const float						RelevanceLagTime;
/** How long it takes a pawn to "fade" from relevance. */
var() protected const float						RelevanceFadeTime;


/** Distance from the pawn to test for non-head vis traces. */
var() protected const float						PawnTraceRadius;
/** What portion of velocity vector is used to bias the lookat for velocity.  Here for tweaking. */
var() protected const float						VelBiasFactor;

var protected transient bool					bWasInterpolating;

cpptext
{
protected:
	FAutoframingWatchedPawn* FindRelevantPawnEntry(AGearPawn const* GP);
	FLOAT FindOptimalFOV(TArray<FAutoframingWatchedPawn> const& VisiblePawnList, FVector const& BaseLoc, FRotator const& BaseRot) const;
	FLOAT CalcHFovFromVFov(FLOAT VFovRad) const;
	FLOAT CalcVFovFromHFov(FLOAT HFovRad) const;
	void UpdateRelevantPawnList(FVector& OutCentroid, FVector const& BaseLoc, FRotator const& BaseRot, UBOOL bInterpolating);
public:
};

/** Sets Loc/Rot/FOV of camera.  Does all the work, basically. */
simulated native function UpdateCamera(CameraActor CamActor, float DeltaTime, out TViewTarget OutVT);

function OnBecomeActive()
{
	bLastLookatIsValid = FALSE;
}


defaultproperties
{
	RelevanceLagTime=2.f
	RelevanceFadeTime=2.f
	PawnTraceRadius=64.f

	bDebugSkipFOVAdj=FALSE
	bDebugShowWatchedPawns=FALSE

	FOVInterpSpeed=0.5f
	FOVInterpSpeed_MovingCamera=6.f

	RotInterpSpeed=1.f
	RotInterpSpeed_MovingCamera=1000.f

	LookatInterpSpeed=6.f
	LookatInterpSpeed_MovingCamera=6.f

	AcceptableFOVRange=(X=40.f,Y=100.f)

	BorderBufferPercentage_Horizontal=0.2f
	BorderBufferPercentage_Vertical=0.2f

	VelBiasFactor=0.3f

	ZoomFOVAdjustmentMag=0.75f
	ZoomBufferAdjustmentMag=0.8f
	DirSelectionWeightMultiplier_Good=2.f
	DirSelectionWeightMultiplier_Bad=0.5f
}