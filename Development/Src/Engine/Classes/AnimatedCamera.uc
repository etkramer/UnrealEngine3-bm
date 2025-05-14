class AnimatedCamera extends Camera
    native
    notplaceable;

const MAX_ACTIVE_CAMERA_ANIMS = 8;

var protected CameraAnimInst AnimInstPool[8];
var protected array<CameraAnimInst> ActiveAnims;
var protected array<CameraAnimInst> FreeAnims;
var protected transient DynamicCameraActor AnimCameraActor;
var protected transient DynamicCameraActor AccumulatorCameraActor;

cpptext
{
protected:
	UCameraAnimInst* AllocCameraAnimInst();
	void ReleaseCameraAnimInst(UCameraAnimInst* Inst);
	UCameraAnimInst* FindExistingCameraAnimInst(UCameraAnim const* Anim);
	
	static void AddScaledInterpProperties(class ACameraActor const* SrcCam, class ACameraActor* DestCam, FLOAT Scale);
	static void ResetInterpProperties(class ACameraActor *CamActor);
	void ApplyInterpPropertiesFromCameraActor(class ACameraActor const* CamActor, FTPOV& OutPOV);

	void ResetTempCameraActor(class ACameraActor* CamActor) const;
public:

	virtual void ModifyPostProcessSettings(FPostProcessSettings& PPSettings) const;
};

// Export UAnimatedCamera::execApplyCameraModifiers(FFrame&, void* const)
native function ApplyCameraModifiers(float DeltaTime, out TPOV OutPOV);

function PostBeginPlay()
{
    local int Idx;

    super(Actor).PostBeginPlay();
    Idx = 0;
    J0x0D:

    // End:0x50 [Loop If]
    if(Idx < 8)
    {
        AnimInstPool[Idx] = new (self) Class'CameraAnimInst';
        FreeAnims[Idx] = AnimInstPool[Idx];
        ++Idx;
        // [Loop Continue]
        goto J0x0D;
    }
    AnimCameraActor = Spawn(Class'DynamicCameraActor', self,,,,, true);
    AccumulatorCameraActor = Spawn(Class'DynamicCameraActor', self,,,,, true);
    //return;    
}

event Destroyed()
{
    AnimCameraActor.Destroy();
    AccumulatorCameraActor.Destroy();
    //return;    
}

// Export UAnimatedCamera::execApplyCameraModifiersNative(FFrame&, void* const)
private native final simulated function ApplyCameraModifiersNative(float DeltaTime, out TPOV OutPOV);

// Export UAnimatedCamera::execPlayCameraAnim(FFrame&, void* const)
native simulated function CameraAnimInst PlayCameraAnim(CameraAnim Anim, optional float Rate = 1.0000000, optional float Scale = 1.0000000, optional float BlendInTime, optional float BlendOutTime, optional bool bLoop, optional bool bRandomStartTime, optional float Duration, optional bool bSingleInstance);

// Export UAnimatedCamera::execStopAllCameraAnims(FFrame&, void* const)
native simulated function StopAllCameraAnims(optional bool bImmediate);

// Export UAnimatedCamera::execStopAllCameraAnimsByType(FFrame&, void* const)
native simulated function StopAllCameraAnimsByType(CameraAnim Anim, optional bool bImmediate);

// Export UAnimatedCamera::execStopCameraAnim(FFrame&, void* const)
native simulated function StopCameraAnim(CameraAnimInst AnimInst, optional bool bImmediate);
