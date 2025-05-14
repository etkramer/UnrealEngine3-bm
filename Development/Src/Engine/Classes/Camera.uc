class Camera extends Actor
    native
    notplaceable;

enum EViewTargetBlendFunction
{
    VTBlend_Linear,                 // 0
    VTBlend_Cubic,                  // 1
    VTBlend_EaseIn,                 // 2
    VTBlend_EaseOut,                // 3
    VTBlend_EaseInOut,              // 4
    VTBlend_MAX                     // 5
};

struct native TCameraCache
{
    var float TimeStamp;
    var TPOV POV;

    structdefaultproperties
    {
        TimeStamp=0.0000000
        POV=(Location=(X=0.0000000,Y=0.0000000,Z=0.0000000),Rotation=(Pitch=0,Yaw=0,Roll=0),FOV=90.0000000)
    }
};

struct native TViewTarget
{
    var() Actor Target;
    var() Controller Controller;
    var() TPOV POV;
    var() float AspectRatio;
    var() PlayerReplicationInfo PRI;

    structdefaultproperties
    {
        Target=none
        Controller=none
        POV=(Location=(X=0.0000000,Y=0.0000000,Z=0.0000000),Rotation=(Pitch=0,Yaw=0,Roll=0),FOV=90.0000000)
        AspectRatio=0.0000000
        PRI=none
    }
};

struct native ViewTargetTransitionParams
{
    var() float BlendTime;
    var() Camera.EViewTargetBlendFunction BlendFunction;
    var() float BlendExp;
    var() bool bResetCameraBehindPlayer;
    var() bool bKeepBatmanOnScreen;
    var() bool bDisableCamerCollisionDuringBlend;

    structdefaultproperties
    {
        BlendTime=0.0000000
        BlendFunction=VTBlend_Cubic
        BlendExp=2.0000000
        bResetCameraBehindPlayer=true
        bKeepBatmanOnScreen=false
        bDisableCamerCollisionDuringBlend=false
    }

	structcpptext
	{
		FViewTargetTransitionParams()
		{}
		FViewTargetTransitionParams(EEventParm)
		: BlendTime(0.f), BlendFunction(VTBlend_Cubic), BlendExp(2.f), bResetCameraBehindPlayer(TRUE), bKeepBatmanOnScreen(FALSE), bDisableCamerCollisionDuringBlend(FALSE)
		{}
	}
};

var PlayerController PCOwner;
var name CameraStyle;
var float DefaultFOV;
var bool bLockedFOV;
var bool bConstrainAspectRatio;
var bool bEnableFading;
var bool bEnableAudioFading;
var bool bCamOverridePostProcess;
var bool bEnableColorScaling;
var bool bEnableColorScaleInterp;
var float LockedFOV;
var float ConstrainedAspectRatio;
var float DefaultAspectRatio;
var Color FadeColor;
var float FadeAmount;
var PostProcessSettings CamPostProcessSettings;
var Vector ColorScale;
var Vector DesiredColorScale;
var Vector OriginalColorScale;
var float ColorScaleInterpDuration;
var float ColorScaleInterpStartTime;
var float SoundFadeAmount;
var float FarCullDistance;
var TCameraCache CameraCache;
var TViewTarget ViewTarget;
var TViewTarget PendingViewTarget;
var float BlendTimeToGo;
var ViewTargetTransitionParams BlendParams;
var array<CameraModifier> ModifierList;
var float FreeCamDistance;
var Vector FreeCamOffset;
var Vector2D FadeAlpha;
var float FadeTime;
var float FadeTimeRemaining;

cpptext
{
	void	AssignViewTarget(AActor* NewTarget, FTViewTarget& VT, struct FViewTargetTransitionParams TransitionParams=FViewTargetTransitionParams(EC_EventParm));
	AActor* GetViewTarget();
	virtual UBOOL	PlayerControlled();
	virtual void	ModifyPostProcessSettings(FPostProcessSettings& PPSettings) const {};
}

function NextVisionMode()
{
    //return;    
}

function NextViewMode()
{
    //return;    
}

// Export UCamera::execApplyCameraModifiers(FFrame&, void* const)
native function ApplyCameraModifiers(float DeltaTime, out TPOV OutPOV);

function InitializeFor(PlayerController PC)
{
    CameraCache.POV.FOV = DefaultFOV;
    PCOwner = PC;
    SetViewTarget(PC.Pawn);
    SetDesiredColorScale(WorldInfo.DefaultColorScale, 5.0000000);
    UpdateCamera(0.0000000);
    //return;    
}

function float GetFOVAngle()
{
    // End:0x0F
    if(bLockedFOV)
    {
        return LockedFOV;
    }
    return CameraCache.POV.FOV;
    //return ReturnValue;    
}

function SetFOV(float NewFOV)
{
    // End:0x27
    if((NewFOV < float(1)) || NewFOV > float(170))
    {
        bLockedFOV = false;
        return;
    }
    bLockedFOV = true;
    LockedFOV = NewFOV;
    //return;    
}

final function GetCameraViewPoint(out Vector OutCamLoc, out Rotator OutCamRot)
{
    OutCamLoc = CameraCache.POV.Location;
    OutCamRot = CameraCache.POV.Rotation;
    //return;    
}

simulated function SetDesiredColorScale(Vector NewColorScale, float InterpTime)
{
    // End:0x55
    if(!bEnableColorScaling)
    {
        bEnableColorScaling = true;
        ColorScale.X = 1.0000000;
        ColorScale.Y = 1.0000000;
        ColorScale.Z = 1.0000000;
    }
    // End:0xA2
    if(NewColorScale != ColorScale)
    {
        OriginalColorScale = ColorScale;
        DesiredColorScale = NewColorScale;
        ColorScaleInterpStartTime = WorldInfo.TimeSeconds;
        ColorScaleInterpDuration = InterpTime;
        bEnableColorScaleInterp = true;
    }
    //return;    
}

simulated event UpdateCamera(float DeltaTime)
{
    local TPOV NewPOV;
    local float DurationPct, BlendPct;

    // End:0x5F
    if(bEnableColorScaleInterp)
    {
        BlendPct = FClamp(TimeSince(ColorScaleInterpStartTime) / ColorScaleInterpDuration, 0.0000000, 1.0000000);
        ColorScale = VLerp(OriginalColorScale, DesiredColorScale, BlendPct);
        // End:0x5F
        if(BlendPct == 1.0000000)
        {
            bEnableColorScaleInterp = false;
        }
    }
    bConstrainAspectRatio = false;
    bCamOverridePostProcess = false;
    CheckViewTarget(ViewTarget);
    UpdateViewTarget(ViewTarget, DeltaTime);
    NewPOV = ViewTarget.POV;
    ConstrainedAspectRatio = ViewTarget.AspectRatio;
    // End:0x2E4
    if(PendingViewTarget.Target != none)
    {
        BlendTimeToGo -= DeltaTime;
        CheckViewTarget(PendingViewTarget);
        UpdateViewTarget(PendingViewTarget, DeltaTime);
        // End:0x27E
        if(BlendTimeToGo > float(0))
        {
            DurationPct = 1.0000000 - (BlendTimeToGo / BlendParams.BlendTime);
            switch(BlendParams.BlendFunction)
            {
                // End:0x166
                case 0:
                    BlendPct = Lerp(0.0000000, 1.0000000, DurationPct);
                    // End:0x22F
                    break;
                // End:0x193
                case 1:
                    BlendPct = FCubicInterp(0.0000000, 0.0000000, 1.0000000, 0.0000000, DurationPct);
                    // End:0x22F
                    break;
                // End:0x1C6
                case 2:
                    BlendPct = FInterpEaseIn(0.0000000, 1.0000000, DurationPct, BlendParams.BlendExp);
                    // End:0x22F
                    break;
                // End:0x1F9
                case 3:
                    BlendPct = FInterpEaseOut(0.0000000, 1.0000000, DurationPct, BlendParams.BlendExp);
                    // End:0x22F
                    break;
                // End:0x22C
                case 4:
                    BlendPct = FInterpEaseInOut(0.0000000, 1.0000000, DurationPct, BlendParams.BlendExp);
                    // End:0x22F
                    break;
                // End:0xFFFF
                default:
                    break;
            }
            NewPOV = BlendViewTargets(ViewTarget, PendingViewTarget, BlendPct);
            ConstrainedAspectRatio = Lerp(ViewTarget.AspectRatio, PendingViewTarget.AspectRatio, BlendPct);            
        }
        else
        {
            ViewTarget = PendingViewTarget;
            PendingViewTarget.Target = none;
            PendingViewTarget.Controller = none;
            BlendTimeToGo = 0.0000000;
            NewPOV = PendingViewTarget.POV;
            ConstrainedAspectRatio = PendingViewTarget.AspectRatio;
        }
    }
    FillCameraCache(NewPOV);
    // End:0x384
    if(bEnableFading && FadeTimeRemaining > 0.0000000)
    {
        FadeTimeRemaining = FMax(FadeTimeRemaining - DeltaTime, 0.0000000);
        // End:0x384
        if(FadeTime > 0.0000000)
        {
            FadeAmount = FadeAlpha.X + ((1.0000000 - (FadeTimeRemaining / FadeTime)) * (FadeAlpha.Y - FadeAlpha.X));
        }
    }
    //return;    
}

function TPOV BlendViewTargets(const out TViewTarget A, const out TViewTarget B, float Alpha)
{
    local TPOV POV;

    POV.Location = VLerp(A.POV.Location, B.POV.Location, Alpha);
    POV.FOV = Lerp(A.POV.FOV, B.POV.FOV, Alpha);
    POV.Rotation = RLerp(A.POV.Rotation, B.POV.Rotation, Alpha, true);
    return POV;
    //return ReturnValue;    
}

function FillCameraCache(const out TPOV NewPOV)
{
    CameraCache.TimeStamp = WorldInfo.TimeSeconds;
    CameraCache.POV = NewPOV;
    //return;    
}

// Export UCamera::execCheckViewTarget(FFrame&, void* const)
native function CheckViewTarget(out TViewTarget VT);

function UpdateViewTarget(out TViewTarget OutVT, float DeltaTime)
{
    local Vector Loc, pos, HitLocation, HitNormal;
    local Rotator Rot;
    local Actor HitActor;
    local CameraActor CamActor;
    local bool bDoNotApplyModifiers;
    local TPOV OrigPOV;

    OrigPOV = OutVT.POV;
    OutVT.POV.FOV = DefaultFOV;
    CamActor = CameraActor(OutVT.Target);
    // End:0xF7
    if(CamActor != none)
    {
        CamActor.GetCameraView(DeltaTime, OutVT.POV);
        bConstrainAspectRatio = bConstrainAspectRatio || CamActor.bConstrainAspectRatio;
        OutVT.AspectRatio = CamActor.AspectRatio;
        bCamOverridePostProcess = CamActor.bCamOverridePostProcess;
        CamPostProcessSettings = CamActor.CamOverridePostProcess;        
    }
    else
    {
        // End:0x362
        if((Pawn(OutVT.Target) == none) || !Pawn(OutVT.Target).CalcCamera(DeltaTime, OutVT.POV.Location, OutVT.POV.Rotation, OutVT.POV.FOV))
        {
            bDoNotApplyModifiers = true;
            switch(CameraStyle)
            {
                // End:0x1C8
                case 'Fixed':
                    OutVT.POV = OrigPOV;
                    // End:0x362
                    break;
                // End:0x1D4
                case 'ThirdPerson':
                // End:0x2FB
                case 'FreeCam':
                    Loc = OutVT.Target.Location;
                    Rot = OutVT.Target.Rotation;
                    // End:0x248
                    if(CameraStyle == 'FreeCam')
                    {
                        Rot = PCOwner.Rotation;
                    }
                    Loc += (FreeCamOffset >> Rot);
                    pos = Loc - (Vector(Rot) * FreeCamDistance);
                    HitActor = Trace(HitLocation, HitNormal, pos, Loc, false, vect(12.0000000, 12.0000000, 12.0000000));
                    OutVT.POV.Location = ((HitActor == none) ? pos : HitLocation);
                    OutVT.POV.Rotation = Rot;
                    // End:0x362
                    break;
                // End:0x307
                case 'FirstPerson':
                // End:0xFFFF
                default:
                    OutVT.Target.GetActorEyesViewPoint(OutVT.POV.Location, OutVT.POV.Rotation);
                    // End:0x362
                    break;
                    break;
            }
        }
    }
    // End:0x38C
    if(!bDoNotApplyModifiers)
    {
        ApplyCameraModifiers(DeltaTime, OutVT.POV);
    }
    //return;    
}

// Export UCamera::execSetViewTarget(FFrame&, void* const)
native function SetViewTarget(Actor NewViewTarget, optional ViewTargetTransitionParams TransitionParams);

function ProcessViewRotation(float DeltaTime, out Rotator OutViewRotation, out Rotator OutDeltaRot)
{
    local int ModifierIdx;

    ModifierIdx = 0;
    J0x07:

    // End:0x71 [Loop If]
    if(ModifierIdx < ModifierList.Length)
    {
        // End:0x67
        if(ModifierList[ModifierIdx] != none)
        {
            // End:0x67
            if(ModifierList[ModifierIdx].ProcessViewRotation(ViewTarget.Target, DeltaTime, OutViewRotation, OutDeltaRot))
            {
                // [Explicit Break]
                goto J0x71;
            }
        }
        ModifierIdx++;
        // [Loop Continue]
        goto J0x07;
    }
    J0x71:

    //return;    
}

function bool AllowPawnRotation()
{
    return true;
    //return ReturnValue;    
}

simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
    local Vector EyesLoc;
    local Rotator EyesRot;
    local Canvas Canvas;

    Canvas = HUD.Canvas;
    Canvas.SetDrawColor(255, 255, 255);
    Canvas.DrawText((("	Camera Style:" $ string(CameraStyle)) @ "main ViewTarget:") $ string(ViewTarget.Target));
    out_YPos += out_YL;
    Canvas.SetPos(4.0000000, out_YPos);
    Canvas.DrawText((((("   CamLoc:" $ string(CameraCache.POV.Location)) @ "CamRot:") $ string(CameraCache.POV.Rotation)) @ "FOV:") $ string(CameraCache.POV.FOV));
    out_YPos += out_YL;
    Canvas.SetPos(4.0000000, out_YPos);
    Canvas.DrawText("   AspectRatio:" $ string(ConstrainedAspectRatio));
    out_YPos += out_YL;
    Canvas.SetPos(4.0000000, out_YPos);
    // End:0x251
    if(ViewTarget.Target != none)
    {
        ViewTarget.Target.GetActorEyesViewPoint(EyesLoc, EyesRot);
        Canvas.DrawText((("   EyesLoc:" $ string(EyesLoc)) @ "EyesRot:") $ string(EyesRot));
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
    }
    //return;    
}

simulated function bool EffectedByReverb()
{
    return true;
    //return ReturnValue;    
}

defaultproperties
{
    DefaultFOV=90.0000000
    DefaultAspectRatio=1.3333300
    CamPostProcessSettings=(bEnableBloom=true,bEnableDOF=false,bEnableMotionBlur=true,bEnableSceneEffect=true,bAllowAmbientOcclusion=true,bAllowAtmospheric=false,Atmospheric_ForegroundColour=(R=63,G=118,B=97,A=255),Atmospheric_ForegroundStrength=0.0100000,Atmospheric_BackgroundColour=(R=5,G=14,B=11,A=255),Atmospheric_BackgroundStrength=0.1000000,Atmospheric_ForegroundMaxDistance=0.0000000,Atmospheric_ForegroundWidth=0.1000000,Atmospheric_BackgroundMaxDistance=0.2000000,Atmospheric_BackgroundWidth=0.2000000,Bloom_Scale=1.0000000,Bloom_InterpolationDuration=1.0000000,DOF_FalloffExponent=4.0000000,DOF_BlurKernelSize=16.0000000,DOF_MaxNearBlurAmount=1.0000000,DOF_MaxFarBlurAmount=1.0000000,DOF_ModulateBlurColor=(R=255,G=255,B=255,A=255),DOF_FocusType=FOCUS_Distance,DOF_FocusInnerRadius=2000.0000000,DOF_FocusDistance=0.0000000,DOF_FocusPosition=(X=0.0000000,Y=0.0000000,Z=0.0000000),DOF_InterpolationDuration=1.0000000,MotionBlur_MaxVelocity=1.0000000,MotionBlur_Amount=0.5000000,MotionBlur_FullMotionBlur=true,MotionBlur_CameraRotationThreshold=45.0000000,MotionBlur_CameraTranslationThreshold=10000.0000000,MotionBlur_InterpolationDuration=1.0000000,Scene_Desaturation=0.0000000,Scene_HighLights=(X=1.0000000,Y=1.0000000,Z=1.0000000),Scene_MidTones=(X=1.0000000,Y=1.0000000,Z=1.0000000),Scene_Shadows=(X=0.0000000,Y=0.0000000,Z=0.0000000),Scene_InterpolationDuration=1.0000000)
    CameraCache=(TimeStamp=0.0000000,POV=(Location=(X=0.0000000,Y=0.0000000,Z=0.0000000),Rotation=(Pitch=0,Yaw=0,Roll=0),FOV=90.0000000))
    ViewTarget=(Target=none,Controller=none,POV=(Location=(X=0.0000000,Y=0.0000000,Z=0.0000000),Rotation=(Pitch=0,Yaw=0,Roll=0),FOV=90.0000000),AspectRatio=0.0000000,PRI=none)
    PendingViewTarget=(Target=none,Controller=none,POV=(Location=(X=0.0000000,Y=0.0000000,Z=0.0000000),Rotation=(Pitch=0,Yaw=0,Roll=0),FOV=90.0000000),AspectRatio=0.0000000,PRI=none)
    BlendParams=(BlendTime=0.0000000,BlendFunction=VTBlend_Cubic,BlendExp=2.0000000,bResetCameraBehindPlayer=true,bKeepBatmanOnScreen=false,bDisableCamerCollisionDuringBlend=false)
    FreeCamDistance=256.0000000
    bHidden=true
}
