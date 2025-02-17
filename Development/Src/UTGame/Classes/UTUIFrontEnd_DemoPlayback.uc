/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Demo Playback scene for UT3.
 */
class UTUIFrontEnd_DemoPlayback extends UTUIFrontEnd;

/** Reference to widget that has a list of demo files. */
var transient UIList DemoFilesList;

/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize( )
{
	Super.PostInitialize();

	DemoFilesList = UIList(FindChild('lstDemoFiles', true));
	DemoFilesList.OnSubmitSelection = OnDemoFilesList_SubmitSelection;
}

/** Sets up the scene's button bar. */
function SetupButtonBar()
{
	ButtonBar.Clear();
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.PlayDemo>", OnButtonBar_PlayDemo);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.DeleteDemo>", OnButtonBar_DelDemo);
}


function DoDelete(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	local string DemoFileName;
	local int CurrentSelection;

	if (SelectedOption == 0)
	{
		CurrentSelection = DemoFilesList.GetCurrentItem();

		if(CurrentSelection != INDEX_NONE)
		{
			if(class'UTUIMenuList'.static.GetCellFieldString(DemoFilesList, 'FileName', CurrentSelection, DemoFileName))
			{
				DeleteDemo(DemoFilename);
			}
		}
	}
}

/** Plays the currently selected demo. */
function OnPlayDemo()
{
	local string DemoFileName;
	local int CurrentSelection;

	CurrentSelection = DemoFilesList.GetCurrentItem();

	if(CurrentSelection != INDEX_NONE)
	{
		if(class'UTUIMenuList'.static.GetCellFieldString(DemoFilesList, 'FileName', CurrentSelection, DemoFileName))
		{
			ConsoleCommand("demoplay "$DemoFileName);
		}
	}
}

/** Callback for when the user wants to back out of this screen. */
function OnBack()
{
	CloseScene(self);
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_Back(UIScreenObject InButton, int InPlayerIndex)
{
	OnBack();

	return true;
}

function bool OnButtonBar_DelDemo(UIScreenObject InButton, int InPlayerIndex)
{

	local UTUIScene_MessageBox MB;


	MB = GetMessageBoxScene();
	if (MB!=none)
	{
		MB.DisplayAcceptCancelBox("<Strings:UTGameUI.Community.DemoDelWarning>","<Strings:UTGameUI.Campaign.Confirmation", DoDelete);
	}

	return true;
}


function bool OnButtonBar_PlayDemo(UIScreenObject InButton, int InPlayerIndex)
{
	OnPlayDemo();

	return true;
}



/** Demo File List - Submit Selection. */
function OnDemoFilesList_SubmitSelection( UIList Sender, int PlayerIndex )
{
	OnPlayDemo();
}


/**
 * Provides a hook for unrealscript to respond to input using actual input key names (i.e. Left, Tab, etc.)
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called BEFORE kismet is given a chance to process the input.
 *
 * @param	EventParms	information about the input event.
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool HandleInputKey( const out InputEventParameters EventParms )
{
	local bool bResult;

	bResult=false;

	if(EventParms.EventType==IE_Released)
	{
		if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
		{
			OnBack();
			bResult=true;
		}
	}

	return bResult;
}


defaultproperties
{

}
