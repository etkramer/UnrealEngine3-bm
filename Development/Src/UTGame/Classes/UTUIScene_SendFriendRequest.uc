/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Friend request scene for UT3, allows the player to send a friend request.
 */
class UTUIScene_SendFriendRequest extends UTUIFrontEnd_CustomScreen;

/** References to the editboxes on the screen. */
var transient UIEditBox	UserNameEditBox;
var transient UIEditBox	MessageEditBox;

/** Person to send the friend invite to. */
var transient string TargetPlayerName;
var transient UniqueNetId TargetPlayerNetId;

event PostInitialize()
{
	local UIObject ButtonBarSafeRegion;

	Super.PostInitialize();

	UserNameEditBox = UIEditBox(FindChild('txtUserName',true));
	UserNameEditBox.NotifyActiveStateChanged=OnNotifyActiveStateChanged;
	UserNameEditBox.OnSubmitText=OnSubmitText;

	MessageEditBox = UIEditBox(FindChild('txtMessage',true));
	MessageEditBox.NotifyActiveStateChanged=OnNotifyActiveStateChanged;
	MessageEditBox.OnSubmitText=OnSubmitText;

	UserNameEditBox.MaxCharacters = GS_USERNAME_MAXLENGTH;
	UserNameEditBox.SetDataStoreBinding("");
	UserNameEditBox.SetValue("");

	MessageEditBox.MaxCharacters = GS_MESSAGE_MAXLENGTH;
	MessageEditBox.SetDataStoreBinding(Localize("PlayerCard","DefaultFriendRequestMessage", "UTGameUI"));
	MessageEditBox.SetValue(Localize("PlayerCard","DefaultFriendRequestMessage", "UTGameUI"));

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
	local int i;

	// HACK - In this menu, set the button bar button's color to the right color to account for the skins

	for (i=0;i<6;i++)
	{
		ButtonBar.Buttons[i].StringRenderComponent.SetColor( MakeLinearColor(0.0,0.0,0.0,1.0) );
	}

	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Cancel>", OnButtonBar_Cancel);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.SendFriendRequest>", OnButtonBar_SendFriendRequest);

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

	UserNameEditBox.SetDataStoreBinding(InTargetPlayerName);
	UserNameEditBox.SetValue(InTargetPlayerName);
}

/** Callback for when the user wants to back out of this screen. */
function OnCancel()
{
	CloseScene(self);
}

/** Sends the actual friend request. */
function OnSendFriendRequest()
{
	local OnlinePlayerInterface PlayerInt;
	local string Message;
	local string LocalPlayerName;


	Message = MessageEditBox.GetValue();
	TargetPlayerName = TrimWhitespace(UserNameEditBox.GetValue());

	if(Len(TargetPlayerName) > 0)
	{
		if( GetDataStoreStringValue("<OnlinePlayerData:PlayerNickName>", LocalPlayerName, self, GetPlayerOwner()) && (Caps(LocalPlayerName) != Caps(TargetPlayerName)) )
		{
			PlayerInt = GetPlayerInterface();
			if(PlayerInt != None)
			{
				PlayerInt.AddAddFriendByNameCompleteDelegate(GetPlayerOwner().ControllerId,OnAddFriendByNameComplete);
				if(PlayerInt.AddFriendByName(GetPlayerOwner().ControllerId, TargetPlayerName, Message)==false)
				{
					OnAddFriendByNameComplete(false);
				}
			}
		}
		else
		{
			DisplayMessageBox("<Strings:UTGameUI.Errors.CannotSendFriendRequestToYourself_Message>","<Strings:UTGameUI.Errors.CannotSendFriendRequestToYourself_Title>");
		}
	}
	else
	{
		DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidFriendName_Message>","<Strings:UTGameUI.Errors.InvalidFriendName_Title>");
	}
}

/**
 * Called when a friend invite arrives for a local player
 *
 * @param bWasSuccessful true if successfully added, false if not found or failed
 */
function OnAddFriendByNameComplete(bool bWasSuccessful)
{
	local string FormattedMsg;
	local UTUIScene_MessageBox MessageBoxRef;

	if(bWasSuccessful)
	{
		FormattedMsg = Repl(Localize("MessageBox", "FriendRequestSent_Message", "UTGameUI"), "\`PlayerName\`", TargetPlayerName);
		MessageBoxRef = DisplayMessageBox(FormattedMsg,"<Strings:UTGameUI.MessageBox.FriendRequestSent_Title>");
		MessageBoxRef.OnClosed = OnRequestSentMessageClosed;
	}
	else
	{
		FormattedMsg = Repl(Localize("MessageBox", "FriendRequestFailed_Message", "UTGameUI"), "\`PlayerName\`", TargetPlayerName);
		MessageBoxRef = DisplayMessageBox(FormattedMsg,"<Strings:UTGameUI.MessageBox.FriendRequestFailed_Title>");
		MessageBoxRef.OnClosed = OnRequestSentMessageClosed;
	}

	GetPlayerInterface().ClearAddFriendByNameCompleteDelegate(GetPlayerOwner().ControllerId,OnAddFriendByNameComplete);
}

/** Called when the user has closed the request sent box. */
function OnRequestSentMessageClosed()
{
	CloseScene(self);
}

/** Shows the online keyboard. */
function OnShowKeyboard()
{
	if(UserNameEditBox.IsFocused())
	{
		ShowKeyboard(UserNameEditBox, Localize("KeyboardPrompts", "FriendRequestName_Title", "UTGameUI"),
			Localize("KeyboardPrompts", "FriendRequestName_Message", "UTGameUI"), false, true, UserNameEditBox.GetValue(), GS_USERNAME_MAXLENGTH);

	}
	else if(MessageEditBox.IsFocused())
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

function bool OnButtonBar_SendFriendRequest(UIScreenObject InButton, int InPlayerIndex)
{
	OnSendFriendRequest();

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
	OnSendFriendRequest();

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
	DescriptionMap.Add((WidgetTag="txtUserName",DataStoreMarkup="<Strings:UTgameUI.KeyboardPrompts.FriendRequestName_Message>"));
	DescriptionMap.Add((WidgetTag="txtMessage",DataStoreMarkup="<Strings:UTgameUI.KeyboardPrompts.FriendRequestMessage_Message>"));
}
