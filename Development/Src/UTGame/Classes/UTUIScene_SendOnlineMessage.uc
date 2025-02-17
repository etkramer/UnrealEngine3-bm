/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Friend request scene for UT3, allows the player to send a friend request.
 */
class UTUIScene_SendOnlineMessage extends UTUIFrontEnd_CustomScreen;

/** References to the editboxes on the screen. */
var transient UIEditBox	MessageEditBox;

/** Reference to a label that shows the target player's name. */
var transient UILabel PlayerNameLabel;

/** Person to send the message to. */
var transient string TargetPlayerName;
var transient UniqueNetId TargetPlayerNetId;

event PostInitialize()
{
	local UIObject ButtonBarSafeRegion;

	Super.PostInitialize();

	// Gather widget references
	PlayerNameLabel = UILabel(FindChild('lblPlayerName', true));

	MessageEditBox = UIEditBox(FindChild('txtMessage',true));
	MessageEditBox.NotifyActiveStateChanged=OnNotifyActiveStateChanged;
	MessageEditBox.OnSubmitText=OnSubmitText;

	MessageEditBox.MaxCharacters = GS_MESSAGE_MAXLENGTH;
	MessageEditBox.SetDataStoreBinding("");
	MessageEditBox.SetValue("");

	if ( IsConsole() )
	{
		// this must be done to prevent the
		ButtonBarSafeRegion = FindChild('pnlSafeRegionLong',true);
		ButtonBarSafeRegion.SetPrivateBehavior(PRIVATE_NotFocusable, true);
	}
}


/** Sets up the scene's button bar. */
function SetupButtonBar()
{
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Cancel>", OnButtonBar_Cancel);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.SendMessage>", OnButtonBar_SendMessage);

	if(IsConsole(CONSOLE_PS3))
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Keyboard>", OnButtonBar_ShowKeyboard);
	}
}

/** Sets who the message should be sent to. */
function SetTargetPlayer(UniqueNetId InTargetPlayerNetId, string InTargetPlayerName)
{
	TargetPlayerName = InTargetPlayerName;
	TargetPlayerNetId = InTargetPlayerNetId;

	PlayerNameLabel.SetDataStoreBinding(InTargetPlayerName);
}

/** Callback for when the user wants to back out of this screen. */
function OnCancel()
{
	CloseScene(self);
}

/** Sends the actual friend request. */
function OnSendMessage()
{
	local OnlinePlayerInterface PlayerInt;
	local string Message;
	local string FormattedMsg;
	local UTUIScene_MessageBox MessageBoxRef;

	Message = MessageEditBox.GetValue();

	if(Len(Message) > 0)
	{
		PlayerInt = GetPlayerInterface();
		if(PlayerInt != None)
		{
			if(PlayerInt.SendMessageToFriend(GetPlayerOwner().ControllerId, TargetPlayerNetId, Message))
			{
				CloseScene(self);
			}
			else
			{
				FormattedMsg = Repl(Localize("Errors", "FailedToSendMessage_Message", "UTGameUI"), "\`PlayerName\`", TargetPlayerName);
				MessageBoxRef = DisplayMessageBox(FormattedMsg,"<Strings:UTGameUI.Errors.FailedToSendMessage_Title>");
				MessageBoxRef.OnClosed = OnMessageResultBoxClosed;
			}
		}
	}
	else
	{
		DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidMessage_Message>","<Strings:UTGameUI.Errors.InvalidMessage_Title>");
	}
}

/** Called when the user has closed the request sent box. */
function OnMessageResultBoxClosed()
{
	CloseScene(self);
}

/** Shows the online keyboard. */
function OnShowKeyboard()
{
	if(MessageEditBox.IsFocused())
	{
		ShowKeyboard(MessageEditBox, Localize("KeyboardPrompts", "FriendRequestMessage_Title", "UTGameUI"),
			Localize("KeyboardPrompts", "FriendRequestMessage_Message", "UTGameUI"), false, true, MessageEditBox.GetValue(), GS_MESSAGE_MAXLENGTH);
	}
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_Cancel(UIScreenObject InButton, int InPlayerIndex)
{
	OnCancel();

	return true;
}

function bool OnButtonBar_SendMessage(UIScreenObject InButton, int InPlayerIndex)
{
	OnSendMessage();

	return true;
}

function bool OnButtonBar_ShowKeyboard(UIScreenObject InButton, int InPlayerIndex)
{
	OnShowKeyboard();

	return true;
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
	OnSendMessage();

	return false;
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
		else if(EventParms.InputKeyName=='XboxTypeS_X')
		{
			OnShowKeyboard();
			bResult=true;
		}
	}

	return bResult;
}


defaultproperties
{
	DescriptionMap.Add((WidgetTag="txtMessage",DataStoreMarkup="<Strings:UTgameUI.KeyboardPrompts.SendMessage_Message>"));
}
