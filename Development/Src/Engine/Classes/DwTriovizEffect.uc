// BM1
class DwTriovizEffect extends PostProcessEffect
    native;

enum EDwTriovizProfile
{
    e_DwTriovizProfile_MainMenu,    // 0
    e_DwTriovizProfile_MainGame,    // 1
    e_DwTriovizProfile_FullScreenVideo,// 2
    e_DwTriovizProfile_CharacterViewer,// 3
    e_DwTriovizProfile_MAX          // 4
};

enum ETriovizTargetShape
{
    ETRIOVIZTargetShape_Rectangle,  // 0
    ETRIOVIZTargetShape_Triangle,   // 1
    ETRIOVIZTargetShape_TriangleMirror,// 2
    ETRIOVIZTargetShape_MAX         // 3
};

enum ETrioviz
{
    TRIOVIZ_NONE,                   // 0
    TRIOVIZ_ZMAP,                   // 1
    TRIOVIZ_DMAP,                   // 2
    TRIOVIZ_DMAP_DILATED,           // 3
    TRIOVIZ_DISPLACEMENT,           // 4
    TRIOVIZ_DISPLACEMENT_RGB_MAP,   // 5
    TRIOVIZ_DISPLACEMENT_RGB_MAP_LEFT,// 6
    TRIOVIZ_DISPLACEMENT_RGB_MAP_RIGHT,// 7
    TRIOVIZ_DOF,                    // 8
    TRIOVIZ_DOF_LEFT,               // 9
    TRIOVIZ_DOF_RIGHT,              // 10
    TRIOVIZ_SCENE_DOF,              // 11
    TRIOVIZ_SCENE_DOF_BLUR,         // 12
    TRIOVIZ_ANTIGHOSTMASK,          // 13
    TRIOVIZ_MAX                     // 14
};

enum ETriovizEditType
{
    ETriovizEditType_MinMax_None,   // 0
    ETriovizEditType_MinMax_TargetShape,// 1
    ETriovizEditType_MinMax_TargetCenter,// 2
    ETriovizEditType_MinMax_TargetScale,// 3
    ETriovizEditType_MinMax_TargetMaxCenter,// 4
    ETriovizEditType_MinMax_TargetMaxScale,// 5
    ETriovizEditType_MinMax_ByHand, // 6
    ETriovizEditType_MinMax_Last,   // 7
    ETriovizEditType_DMap_None,     // 8
    ETriovizEditType_DMap_CurveForeground_P1,// 9
    ETriovizEditType_DMap_CurveForeground_P2,// 10
    ETriovizEditType_DMap_CurveBackground_P1,// 11
    ETriovizEditType_DMap_CurveBackground_P2,// 12
    ETriovizEditType_DMap_ByHand,   // 13
    ETriovizEditType_DMap_Last,     // 14
    ETriovizEditType_MAX            // 15
};

enum ETriovizParm
{
    ETriovizParm_Menu_Trioviz,      // 0
    ETriovizParm_Menu_ColorCorrectionGame,// 1
    ETriovizParm_Menu_ColorCorrectionUI,// 2
    ETriovizParm_Menu_DebugMode,    // 3
    ETriovizParm_Menu_PlayerSelect, // 4
    ETriovizParm_Menu_ProfileSelect,// 5
    ETriovizParm_Debug_Show,        // 6
    ETriovizParm_Debug_Multiplier,  // 7
    ETriovizParm_Debug_SplitDemoMode,// 8
    ETriovizParm_Debug_EditMode,    // 9
    ETriovizParm_MinMax_EditType,   // 10
    ETriovizParm_MinMax_TargetShape,// 11
    ETriovizParm_MinMax_TargetShapeCenter_X,// 12
    ETriovizParm_MinMax_TargetShapeCenter_Y,// 13
    ETriovizParm_MinMax_TargetShapeScale_X,// 14
    ETriovizParm_MinMax_TargetShapeScale_Y,// 15
    ETriovizParm_MinMax_TargetMaxShapeCenter_X,// 16
    ETriovizParm_MinMax_TargetMaxShapeCenter_Y,// 17
    ETriovizParm_MinMax_TargetMaxShapeScale_X,// 18
    ETriovizParm_MinMax_TargetMaxShapeScale_Y,// 19
    ETriovizParm_DMap_EditType,     // 20
    ETriovizParm_DMap_DMapCurveForeground_P1X,// 21
    ETriovizParm_DMap_DMapCurveForeground_P2X,// 22
    ETriovizParm_DMap_DMapCurveBackground_P1X,// 23
    ETriovizParm_DMap_DMapCurveBackground_P2X,// 24
    ETriovizParm_DMap_DMapCurveForeground_P1Y,// 25
    ETriovizParm_DMap_DMapCurveForeground_P2Y,// 26
    ETriovizParm_DMap_DMapCurveBackground_P1Y,// 27
    ETriovizParm_DMap_DMapCurveBackground_P2Y,// 28
    ETriovizParm_MinMax_AdaptedSpeed,// 29
    ETriovizParm_ZMap_UseParametersFromCinematicDOF,// 30
    ETriovizParm_ZMap_UseAutomaticDisparityLevel,// 31
    ETriovizParm_ZMap_ZMapGamma,    // 32
    ETriovizParm_ZMap_ZMapMin,      // 33
    ETriovizParm_ZMap_DisparityLevel,// 34
    ETriovizParm_ZMap_ZMapMax,      // 35
    ETriovizParm_DMap_DMapScaleBackground,// 36
    ETriovizParm_DMap_DMapCompose,  // 37
    ETriovizParm_DMap_DMapComposeWeight_X,// 38
    ETriovizParm_DMap_DMapComposeWeight_Y,// 39
    ETriovizParm_DMap_DMapComposeWeight_Z,// 40
    ETriovizParm_DMap_DMapComposeWeight_W,// 41
    ETriovizParm_DMap_DMapGammaForeground,// 42
    ETriovizParm_DMap_DMapGammaBackground,// 43
    ETriovizParm_Disparity_PercentFactorForeground,// 44
    ETriovizParm_Disparity_PercentFactorBackground,// 45
    ETriovizParm_AntiGhost_AntiGhostContrast,// 46
    ETriovizParm_AntiGhost_AntiGhostScale,// 47
    ETriovizParm_DOF_UseTriovizDof, // 48
    ETriovizParm_DOF_FocusInnerRadius,// 49
    ETriovizParm_DOF_FalloffExponent,// 50
    ETriovizParm_DOF_MaxNearBlurAmount,// 51
    ETriovizParm_DOF_MaxFarBlurAmount,// 52
    ETriovizParm_Dilation_DilationKernelSize,// 53
    ETriovizParm_DepthControl_UseDownsampledDepthBuffer,// 54
    ETriovizParm_Menu_Exit,         // 55
    ETriovizParm_MAX                // 56
};

struct native STriovizParmsRange_BYTE
{
    var int MinValue;
    var int MaxValue;

    structdefaultproperties
    {
        MinValue=0
        MaxValue=1
    }
};

struct native STriovizParmsRange_FLOAT
{
    var() float MinValue;
    var() float MaxValue;
    var() float StepValue;

    structdefaultproperties
    {
        MinValue=0.0000000
        MaxValue=1.0000000
        StepValue=0.1000000
    }
};

struct native STriovizParms_MinMax
{
    var(MinMax) float AdaptedSpeed;
    var(MinMax) DwTriovizEffect.ETriovizTargetShape TargetShape;
    var(MinMax) Vector4 TargetShapeCenterAndScale;
    var(MinMax) Vector4 TargetMaxZShapeCenterAndScale;
    var DwTriovizEffect.ETriovizEditType EditType;

    structdefaultproperties
    {
        AdaptedSpeed=0.2500000
        TargetShape=ETRIOVIZTargetShape_Rectangle
        TargetShapeCenterAndScale=(X=0.5000000,Y=0.5000000,Z=0.1500000,W=0.2000000)
        TargetMaxZShapeCenterAndScale=(X=0.5000000,Y=0.5000000,Z=0.4200000,W=0.3000000)
        EditType=ETriovizEditType_MinMax_None
    }
};

struct native STriovizParms_ZMap
{
    var float ZMapMin;
    var float ZMapMax;
    var(ZMap) float ZMapGamma;
    var(ZMap) float DisparityLevel;
    var(ZMap) bool UseAutomaticDisparityLevel;
    var(ZMap) bool UseParametersFromCinematicDOF;

    structdefaultproperties
    {
        ZMapMin=50.0000000
        ZMapMax=8000.0000000
        ZMapGamma=1.0000000
        DisparityLevel=3500.0000000
        UseAutomaticDisparityLevel=true
        UseParametersFromCinematicDOF=false
    }
};

struct native STriovizParms_DMap
{
    var(DMap) float DMapGammaForeground;
    var(DMap) float DMapGammaBackground;
    var(DMap) float DMapScaleBackground;
    var(DMap) bool DMapCompose;
    var(DMap) Vector4 DMapComposeWeight;
    var(DMap) Vector4 DMapCurveY;
    var(DMap) Texture2D DMapComposeTexture;
    var DwTriovizEffect.ETriovizEditType EditType;

    structdefaultproperties
    {
        DMapGammaForeground=2.0000000
        DMapGammaBackground=0.5000000
        DMapScaleBackground=1.0000000
        DMapCompose=false
        DMapComposeWeight=(X=1.0000000,Y=1.0000000,Z=1.0000000,W=1.0000000)
        DMapCurveY=(X=1.0000000,Y=1.0000000,Z=1.0000000,W=1.0000000)
        DMapComposeTexture=none
        EditType=ETriovizEditType_DMap_None
    }
};

struct native STriovizParms_Disparity
{
    var(Disparity) float PercentFactorForeground;
    var(Disparity) float PercentFactorBackground;

    structdefaultproperties
    {
        PercentFactorForeground=0.0031250
        PercentFactorBackground=0.0031250
    }
};

struct native STriovizParms_AntiGhost
{
    var(AntiGhost) float AntiGhostContrast;
    var(AntiGhost) float AntiGhostScale;

    structdefaultproperties
    {
        AntiGhostContrast=3.0000000
        AntiGhostScale=1.0000000
    }
};

struct native STriovizParms_DOF
{
    var(DOF) bool UseTriovizDof;
    var(DOF) float FocusInnerRadius;
    var(DOF) float FalloffExponent;
    var(DOF) float MaxNearBlurAmount;
    var(DOF) float MaxFarBlurAmount;

    structdefaultproperties
    {
        UseTriovizDof=true
        FocusInnerRadius=1.0000000
        FalloffExponent=4.0000000
        MaxNearBlurAmount=0.3500000
        MaxFarBlurAmount=0.2000000
    }
};

struct native STriovizParms_Dilation
{
    var(Dilation) float DilationKernelSize;

    structdefaultproperties
    {
        DilationKernelSize=0.0031250
    }
};

struct native transient STriovizParms_DepthControl
{
    var init bool UseDownsampledDepthBuffer;

    structdefaultproperties
    {
        UseDownsampledDepthBuffer=false
    }
};

struct native transient STriovizParms_Debug
{
    var(Debug) init DwTriovizEffect.ETrioviz Show;
    var(Debug) init float Multiplier;
    var(Debug) init bool EditMode;
    var init bool SplitDemoMode;

    structdefaultproperties
    {
        Show=TRIOVIZ_NONE
        Multiplier=1.5000000
        EditMode=false
        SplitDemoMode=false
    }
};

struct native STriovizParms
{
    var(MinMax) STriovizParms_MinMax MinMax;
    var(ZMap) STriovizParms_ZMap ZMap;
    var(DMap) STriovizParms_DMap DMap;
    var(Disparity) STriovizParms_Disparity Disparity;
    var(AntiGhost) STriovizParms_AntiGhost AntiGhost;
    var(DOF) STriovizParms_DOF DOF;
    var(Dilation) STriovizParms_Dilation Dilation;
    var STriovizParms_DepthControl DepthControl;
    var(Debug) transient STriovizParms_Debug Debug;
    var transient bool IsCinematicMode;
    var transient bool UseTriovizPostProcess;
    var transient bool ApplyColorCorrectionInTriovizUber;
    var bool UseTriovizColorCorrectionUI;
    var bool UseTriovizColorCorrectionGame;

    structdefaultproperties
    {
        MinMax=(AdaptedSpeed=0.2500000,TargetShape=ETRIOVIZTargetShape_Rectangle,TargetShapeCenterAndScale=(X=0.5000000,Y=0.5000000,Z=0.1500000,W=0.2000000),TargetMaxZShapeCenterAndScale=(X=0.5000000,Y=0.5000000,Z=0.4200000,W=0.3000000),EditType=ETriovizEditType_MinMax_None)
        ZMap=(ZMapMin=50.0000000,ZMapMax=8000.0000000,ZMapGamma=1.0000000,DisparityLevel=3500.0000000,UseAutomaticDisparityLevel=true,UseParametersFromCinematicDOF=false)
        DMap=(DMapGammaForeground=2.0000000,DMapGammaBackground=0.5000000,DMapScaleBackground=1.0000000,DMapCompose=false,DMapComposeWeight=(X=1.0000000,Y=1.0000000,Z=1.0000000,W=1.0000000),DMapCurveY=(X=1.0000000,Y=1.0000000,Z=1.0000000,W=1.0000000),DMapComposeTexture=none,EditType=ETriovizEditType_DMap_None)
        Disparity=(PercentFactorForeground=0.0031250,PercentFactorBackground=0.0031250)
        AntiGhost=(AntiGhostContrast=3.0000000,AntiGhostScale=1.0000000)
        DOF=(UseTriovizDof=true,FocusInnerRadius=1.0000000,FalloffExponent=4.0000000,MaxNearBlurAmount=0.3500000,MaxFarBlurAmount=0.2000000)
        Dilation=(DilationKernelSize=0.0031250)
        DepthControl=(UseDownsampledDepthBuffer=false)
        Debug=(Show=TRIOVIZ_NONE,Multiplier=1.5000000,EditMode=false,SplitDemoMode=false)
        IsCinematicMode=false
        UseTriovizPostProcess=false
        ApplyColorCorrectionInTriovizUber=false
        UseTriovizColorCorrectionUI=false
        UseTriovizColorCorrectionGame=false
    }
};

var() bool bEnableInEditor;

defaultproperties
{
    bEnableInEditor=true
}