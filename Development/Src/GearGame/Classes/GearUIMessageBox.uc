/**
 * Gears2 Message Box scene class.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUIMessageBox extends UIMessageBoxBase;

var		transient	private		float						TimeTillUnpause;
var		transient				UITickableObjectProxy		TickProxy;

/**
 * Creates a tick proxy to allow the scene to check the status of the loading screen each tick.  Once the loading movie is no longer playing,
 * the game will be paused.
 */
function PauseOnLoadingMovieComplete()
{
	bPauseGameWhileActive = false;
	if ( TickProxy == None )
	{
		TickProxy = new class'UITickableObjectProxy';
	}

	`assert(TickProxy != None);
	TickProxy.OnScriptTick = OnProxyTick_WaitForLoadingMovie;
	RegisterTickableObject(TickProxy);
}

/**
 * Delegate for allowing others to subscribe to this object's tick.
 */
function OnProxyTick_WaitForLoadingMovie( UITickableObjectProxy Sender, float DeltaTime )
{
	local LocalPlayer LP;
	local GearPC OwnerPC;
	local string MovieName;

	LP = GetPlayerOwner();
	if ( LP != None )
	{
		OwnerPC = GearPC(LP.Actor);
		if ( OwnerPC != None )
		{
			OwnerPC.GetCurrentMovie(MovieName);
			if ( !(MovieName ~= class'GearUIInteraction'.const.LOADING_MOVIE) )
			{
				if ( TickProxy != None )
				{
					// same amount of time that GearPC.WarmupPause uses - need this so that we don't have a black screen
					TimeTillUnpause = 2.0f;
					TickProxy = None;

					// unregister and unhook the tick proxy.
					bPauseGameWhileActive = true;
				}
				else
				{
					TimeTillUnpause -= DeltaTime;
					if ( TimeTillUnpause < 0 )
					{
						UnregisterTickableObject(TickProxy);
						Sender.OnScriptTick = None;

						OwnerPC.PauseGame(OwnerPC.CanUnpauseControllerConnected);
					}
				}
			}
		}
	}
}

DefaultProperties
{
	ScenePostProcessGroup=UIPostProcess_Dynamic
	CurrentBackgroundSettings=(bEnableBloom=true,bEnableDOF=true,bEnableMotionBlur=false,bEnableSceneEffect=false,bAllowAmbientOcclusion=false,DOF_BlurKernelSize=16,Bloom_Scale=0,Scene_Desaturation=0,Scene_Shadows=(X=0,Y=0,Z=0),Scene_HighLights=(X=2,Y=2,Z=2))
	CurrentForegroundSettings=(bEnableBloom=true,bEnableDOF=true,bEnableMotionBlur=false,bEnableSceneEffect=false,bAllowAmbientOcclusion=false,DOF_BlurKernelSize=16)

	SceneAnimation_Open=ActivateSceneSeq
	SceneAnimation_Close=DeactivateSceneSeq
//	SceneAnimation_LoseFocus=ForegroundPP_FadeInSeq
//	SceneAnimation_RegainingFocus=ForegroundPP_FadeOutSeq
//	SceneAnimation_RegainedFocus=ReactivateSceneSeq

	bDisplayCursor=false
	bPerformAutomaticLayout=FALSE
	TitleWidgetName="lblTitle"
	MessageWidgetName="lblErrorMessage"
	QuestionWidgetName="lblQuestion"
	QuestionWidgetImageName="imgQuestionBG"
	ChoicesWidgetName="btnbarErrorMessage"
	ButtonBarButtonBGStyleName="imgBlank"
	ButtonBarButtonTextStyleName="cmb_BodyText"
}
