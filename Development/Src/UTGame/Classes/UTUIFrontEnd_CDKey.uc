/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * CD Key entry for UT3.
 */
class UTUIFrontEnd_CDKey extends UTUIFrontEnd_CustomScreen;

var transient UIEditBox	CDKeyEditBox[4];
var transient OnlineAccountInterface AccountInterface;

/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize( )
{
	local int EditboxIdx;

	Super.PostInitialize();

	for(EditboxIdx=0; EditboxIdx<4; EditboxIdx++)
	{
		CDKeyEditbox[EditboxIdx] = UIEditBox(FindChild(name("txtKey"$(EditboxIdx+1)),true));
		CDKeyEditbox[EditboxIdx].NotifyActiveStateChanged = OnNotifyActiveStateChanged;
		CDKeyEditbox[EditboxIdx].OnSubmitText=OnSubmitText;
		CDKeyEditbox[EditboxIdx].SetDataStoreBinding("");
		CDKeyEditbox[EditboxIdx].SetValue("");
		CDKeyEditbox[EditboxIdx].MaxCharacters = GS_CDKEY_PART_MAXLENGTH;
	}


	// Store reference to the account interface.
	AccountInterface=GetAccountInterface();
}

/** Sets up the scene's button bar. */
function SetupButtonBar()
{
	ButtonBar.Clear();

	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ExitGame>", OnButtonBar_Cancel);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Accept>", OnButtonBar_Accept);
}

/** Callback for when the user wants to back out of this screen. */
function OnCancel()
{
	ConsoleCommand("quit");
}

/** Tries to create the user's account. */
function OnAccept()
{
//@todo ut3merge
//	local int EditboxIdx;
//	local string CDKey;
//
//	for(EditboxIdx=0; EditboxIdx<4; EditboxIdx++)
//	{
//		CDKey $= CDKeyEditbox[EditboxIdx].GetValue();
//	}

	// Verify contents of user name box
//@todo ut3merge
//	if(AccountInterface.SaveKey(CDKey))
	if ( true )
	{
		CloseScene(self);
	}
	else
	{
		DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidCDKey_Message>","<Strings:UTGameUI.Errors.InvalidCDKey_Title>");
	}
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_Cancel(UIScreenObject InButton, int InPlayerIndex)
{
	OnCancel();

	return true;
}

function bool OnButtonBar_Accept(UIScreenObject InButton, int InPlayerIndex)
{
	OnAccept();

	return true;
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
			OnCancel();
			bResult=true;
		}
	}

	return bResult;
}

/**
 * Called when the user presses enter (or any other action bound to UIKey_SubmitText) while this editbox has focus.
 *
 * @param	Sender	the editbox that is submitting text
 * @param	PlayerIndex		the index of the player that generated the call to SetValue; used as the PlayerIndex when activating
 *							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 *
 * @return	if TRUE, the editbox will clear the existing value of its textbox.
 */
function bool OnSubmitText( UIEditBox Sender, int PlayerIndex )
{
	OnAccept();

	return false;
}


defaultproperties
{
	DescriptionMap.Add((WidgetTag="txtKey1",DataStoreMarkup="<Strings:UTgameUI.Profiles.CDKey_Description>"));
	DescriptionMap.Add((WidgetTag="txtKey2",DataStoreMarkup="<Strings:UTgameUI.Profiles.CDKey_Description>"));
	DescriptionMap.Add((WidgetTag="txtKey3",DataStoreMarkup="<Strings:UTgameUI.Profiles.CDKey_Description>"));
	DescriptionMap.Add((WidgetTag="txtKey4",DataStoreMarkup="<Strings:UTgameUI.Profiles.CDKey_Description>"));
}
