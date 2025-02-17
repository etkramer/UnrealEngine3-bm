/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Profile login screen for UT3.
 */
class UTUIFrontEnd_LoginScreen extends UTUIFrontEnd_CustomScreen
	Config(Game)
	native(UIFrontEnd);

var string	CreateProfileScene;
var transient UIEditBox	UserNameEditBox;
var transient UIEditBox	PasswordEditBox;

/** Reference to the local login checkbox. */
var transient UICheckbox LocalLoginCheckBox;

/** Reference to the save password checkbox. */
var transient UICheckbox SavePasswordCheckBox;

/** Reference to the autologin checkbox. */
var transient UICheckbox AutoLoginCheckBox;

var transient UTUIScene_MessageBox MessageBoxReference;
var transient EOnlineServerConnectionStatus LoginErrorCode;

/** if TRUE, logic procedure will begin automatically as soon as the menu completes its opening animation */
var transient bool bLoginOnShow;

/** Whether or not to autofill in the password field for the user. */
var config bool bSavePassword;

/** Whether or not to autologin for the user. */
var config bool bAutoLogin;

/** Array of player names that have logged in previously on this machine. */
var config array<string> PlayerNames;

/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize( )
{
	Super.PostInitialize();

	UserNameEditBox = UIEditBox(FindChild('txtUserName',true));
	UserNameEditBox.NotifyActiveStateChanged=OnNotifyActiveStateChanged;
	UserNameEditBox.OnSubmitText=OnSubmitText;
	UserNameEditBox.StringRenderComponent.bIgnoreMarkup = true;
	UserNameEditBox.MaxCharacters = GS_USERNAME_MAXLENGTH;
	UserNameEditBox.SetDataStoreBinding("");

	PasswordEditBox = UIEditBox(FindChild('txtPassword',true));
	PasswordEditBox.NotifyActiveStateChanged=OnNotifyActiveStateChanged;
	PasswordEditBox.OnSubmitText=OnSubmitText;
	PasswordEditbox.StringRenderComponent.bIgnoreMarkup = true;
	PasswordEditBox.MaxCharacters = GS_PASSWORD_MAXLENGTH;
	PasswordEditBox.SetDataStoreBinding("");
	PasswordEditBox.SetValue("");

	// If we have any player names stored, set the default editbox text to the first name in the array.
	if(PlayerNames.length > 0)
	{
		UserNameEditBox.SetDataStoreBinding(PlayerNames[0]);
		PasswordEditBox.SetFocus(None);
	}

	SavePasswordCheckBox = UICheckbox(FindChild('chkSavePassword',true));
	SavePasswordCheckBox.SetValue(default.bSavePassword);
	SavePasswordCheckBox.OnValueChanged=OnSavePasswordCheckBox_ValueChanged;

	AutoLoginCheckBox = UICheckbox(FindChild('chkAutoLogin',true));
	AutoLoginCheckBox.SetValue(default.bAutoLogin);
	AutoLoginCheckBox.OnValueChanged=OnAutoLoginCheckBox_ValueChanged;

	LocalLoginCheckBox = UICheckbox(FindChild('chkLocal',true));
	//LocalLoginCheckBox.OnValueChanged=OnLocalLoginCheckBox_ValueChanged;
	//LocalLoginCheckBox.SetVisibility(false);

	// Handle saved passwords and auto login
	CheckLoginProperties();
}

/** Checks to see if we should autopopuplate the password box or autologin. */
native function CheckLoginProperties();

/** Sets up the scene's button bar. */
function SetupButtonBar()
{
	ButtonBar.Clear();

	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.PlayOffline>", OnButtonBar_PlayOffline);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Login>", OnButtonBar_Login);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.CreateProfileLoginScreen>", OnButtonBar_CreateProfile);

	if(IsConsole(CONSOLE_PS3))
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Keyboard>", OnButtonBar_ShowKeyboard);
	}
}

/** Callback for when the show animation has completed for this scene. */
function OnMainRegion_Show_UIAnimEnd( UIScreenObject AnimTarget, name AnimName, int TrackTypeMask )
{
	Super.OnMainRegion_Show_UIAnimEnd(AnimTarget, AnimName, TrackTypeMask);

	`log(`location @ `showobj(AnimTarget) @ `showvar(AnimName) @ `showvar(TrackTypeMask));
	if ( bLoginOnShow && TrackTypeMask == 0 )
	{
		bLoginOnShow=false;
		OnLogin();
	}
}

/**
 * Callback for when the savepassword checkbox changes value.
 *
 * @param Sender		Object that sent the event
 * @param PlayerIndex	Index of the player that performed the action.
 */
function OnSavePasswordCheckBox_ValueChanged(UIObject Sender, int PlayerIndex)
{
	if(SavePasswordCheckBox.IsChecked()==false)
	{
		AutoLoginCheckBox.SetValue(false);
	}
}


/**
 * Callback for when the autologin checkbox changes value.
 *
 * @param Sender		Object that sent the event
 * @param PlayerIndex	Index of the player that performed the action.
 */
function OnAutoLoginCheckBox_ValueChanged(UIObject Sender, int PlayerIndex)
{
	if(AutoLoginCheckBox.IsChecked())
	{
		SavePasswordCheckBox.SetValue(true);
	}
}

/**
 * Callback for when the local login checkbox changes value.
 *
 * @param Sender		Object that sent the event
 * @param PlayerIndex	Index of the player that performed the action.
 */
function OnLocalLoginCheckBox_ValueChanged(UIObject Sender, int PlayerIndex)
{
	PasswordEditBox.SetValue("");
	PasswordEditBox.SetEnabled(!LocalLoginCheckBox.IsChecked());
	SavePasswordCheckBox.SetEnabled(!LocalLoginCheckBox.IsChecked());
}

/** Callback for when the user wants to back out of this screen. */
function OnCancel()
{
	CloseScene(self);
}

/** Play offline callback, logs the user in offline. */
function OnPlayOffline()
{
	OnLogin(true);
}

/** Tries to login the user */
function OnLogin(optional bool bLocalLogin=false)
{
	local string UserName;
	local string Password;
	local OnlinePlayerInterface PlayerInt;

	/* - Removed, have a play offline button now.
	local bool bLocalLogin;
	bLocalLogin = LocalLoginCheckBox.IsChecked();
	*/

	UserName = TrimWhitespace(UserNameEditBox.GetValue());
	Password = PasswordEditBox.GetValue();

	// Set the value of save password and auto login
	bSavePassword = SavePasswordCheckBox.IsChecked();
	bAutoLogin = AutoLoginCheckBox.IsChecked();
	if (bSavePassword	!= default.bSavePassword
	||	bAutoLogin		!= default.bAutoLogin )
	{
		default.bSavePassword = bSavePassword;
		default.bAutoLogin = bAutoLogin;
		SaveConfig();
	}

	// Verify contents of user name box
	if(Len(UserName) > 0 || bLocalLogin)
	{
		if(Len(Password) > 0 || bLocalLogin)
		{
			// Try logging in
			`Log("UTUIFrontEnd_LoginScreen::OnLogin() - Attempting to login with name '"$UserName$"' bLocalOnly:"@bLocalLogin);

			PlayerInt = GetPlayerInterface();

			if(PlayerInt!=none)
			{
				// Set flag
				SetDataStoreStringValue("<Registry:bChangingLoginStatus>", "1");

				// Display a modal messagebox
				MessageBoxReference = GetMessageBoxScene();
				MessageBoxReference.DisplayModalBox("<Strings:UTGameUI.Generic.LoggingIn>","");

				PlayerInt.AddLoginChangeDelegate(OnLoginChange);
				PlayerInt.AddLoginFailedDelegate(GetBestControllerId(),OnLoginFailed);
				PlayerInt.AddLoginCancelledDelegate(OnLoginCancelled);
				if(PlayerInt.Login(GetBestControllerId(), UserName, Password, bLocalLogin)==false)
				{
					`Log("UTUIFrontEnd_LoginScreen::OnLogin() - Login call failed.");
					//OnLoginCancelled();
				}
			}
			else
			{
				`Log("UTUIFrontEnd_LoginScreen::OnLogin() - Failed to find player interface.");
			}
		}
		else
		{
			DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidPassword_Message>","<Strings:UTGameUI.Errors.InvalidPassword_Title>");
		}
	}
	else
	{
		DisplayMessageBox("<Strings:UTGameUI.Errors." $ (bLocalLogin ? "InvalidUserName_LocalLogin" : "InvalidUserName_Message") $ ">", "<Strings:UTGameUI.Errors.InvalidUserName_Title>");
	}
}

/** Clears login delegates. */
function ClearLoginDelegates()
{
	local OnlinePlayerInterface PlayerInt;
	PlayerInt = GetPlayerInterface();

	PlayerInt.ClearLoginChangeDelegate(OnLoginChange);
	PlayerInt.ClearLoginFailedDelegate(GetPlayerOwner().ControllerId,OnLoginFailed);
	PlayerInt.ClearLoginCancelledDelegate(OnLoginCancelled);

	// Clear flag
	SetDataStoreStringValue("<Registry:bChangingLoginStatus>", "0");
}

/**
 * Delegate used in login notifications
 */
function OnLoginChange()
{
	local int NameIdx;
	local int FindIdx;
	local string UserName;

	// Close the modal messagebox
	MessageBoxReference.OnClosed = OnLoginChange_Closed;
	MessageBoxReference.Close();

	// Store the player names in a config array so we can show a list of recent logins.
	UserName = StripInvalidUsernameCharacters(UserNameEditBox.GetValue());
	SavePassword(PasswordEditBox.GetValue());

	// See if the name is already in the list, if so we will push it to the top.
	PlayerNames = class'UTUIFrontEnd_LoginScreen'.default.PlayerNames;
	FindIdx=INDEX_NONE;
	for(NameIdx=0; NameIdx<PlayerNames.length; NameIdx++)
	{
		if(PlayerNames[NameIdx]==UserName)
		{
			FindIdx=NameIdx;
			break;
		}
	}

	// If the name doesnt exist, increase the array length, but cap it at some number and shift all existing names down by 1.
	if(FindIdx==INDEX_NONE)
	{
		if(PlayerNames.length < 10)
		{
			PlayerNames.length = PlayerNames.length + 1;
		}

		FindIdx=PlayerNames.length-1;
	}

	for(NameIdx=FindIdx; NameIdx>=1; NameIdx--)
	{
		PlayerNames[NameIdx]=PlayerNames[NameIdx-1];
	}

	PlayerNames[0] = UserName;
	default.PlayerNames=PlayerNames;
	SaveConfig();
}

/** Callback for when the login message closes for login change. */
function OnLoginChange_Closed()
{
	MessageBoxReference = none;

	ClearLoginDelegates();

	// Logged in successfully so close the login scene.
	CloseScene(self);
}


/**
 * Delegate used to notify when a login request was cancelled by the user
 */
function OnLoginCancelled()
{
	// Close the modal messagebox
	MessageBoxReference.OnClosed = OnLoginCancelled_Closed;
	MessageBoxReference.Close();
}

/** Callback for when the login cancelled messagebox has closed. */
function OnLoginCancelled_Closed()
{
	MessageBoxReference = none;
	ClearLoginDelegates();
}

/**
* Delegate used in notifying the UI/game that the manual login failed
*
* @param LocalUserNum the controller number of the associated user
* @param ErrorCode the async error code that occurred
*/
function OnLoginFailed(byte LocalUserNum,EOnlineServerConnectionStatus ErrorCode)
{
	// Close the modal messagebox
	LoginErrorCode=ErrorCode;
	MessageBoxReference.OnClosed=OnLoginFailed_Closed;
	MessageBoxReference.Close();
}

/** Callback for when the login message closes. */
function OnLoginFailed_Closed()
{
	MessageBoxReference=None;

	// Display an error message
	switch(LoginErrorCode)
	{
	case OSCS_ConnectionDropped:
		DisplayMessageBox("<Strings:UTGameUI.Errors.ConnectionLost_Message>","<Strings:UTGameUI.Errors.ConnectionLost_Title>");
		break;
	case OSCS_NoNetworkConnection:
		DisplayMessageBox("<Strings:UTGameUI.Errors.LinkDisconnected_Message>","<Strings:UTGameUI.Errors.Error_Title>");
		break;
	case OSCS_ServiceUnavailable:
		DisplayMessageBox("<Strings:UTGameUI.Errors.ServiceUnavailable_Message>","<Strings:UTGameUI.Errors.ServiceUnavailable_Title>");
		break;
	case OSCS_UpdateRequired:
		DisplayMessageBox("<Strings:UTGameUI.Errors.UpdateRequired_Message>","<Strings:UTGameUI.Errors.UpdateRequired_Title>");
		break;
	case OSCS_ServersTooBusy:
		DisplayMessageBox("<Strings:UTGameUI.Errors.ServersTooBusy_Message>","<Strings:UTGameUI.Errors.ServersTooBusy_Title>");
		break;
	case OSCS_DuplicateLoginDetected:
		DisplayMessageBox("<Strings:UTGameUI.Errors.DuplicateLogin_Message>","<Strings:UTGameUI.Errors.DuplicateLogin_Title>");
		break;
	case OSCS_InvalidUser:
		DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidUserLogin_Message>","<Strings:UTGameUI.Errors.InvalidUserLogin_Title>");
		break;
	default:
		break;
	}

	ClearLoginDelegates();
}

/** Displays the account creation scene */
function OnCreateProfile()
{
	OpenSceneByName(CreateProfileScene);
}

/** Shows the online keyboard. */
function OnShowKeyboard()
{
	local OnlinePlayerInterface PlayerInt;

	PlayerInt = GetPlayerInterface();

	if(PlayerInt != None)
	{
		if(UserNameEditBox.IsFocused())
		{
			ShowKeyboard(UserNameEditBox, Localize("KeyboardPrompts", "LoginScreen_Name_Title", "UTGameUI"),
				Localize("KeyboardPrompts", "LoginScreen_Name_Message", "UTGameUI"), false, true, UserNameEditBox.GetValue(), GS_USERNAME_MAXLENGTH);

		}
		else if(PasswordEditBox.IsFocused())
		{
			ShowKeyboard(PasswordEditBox, Localize("KeyboardPrompts", "LoginScreen_Password_Title", "UTGameUI"),
				Localize("KeyboardPrompts", "LoginScreen_Password_Message", "UTGameUI"), true, true, PasswordEditBox.GetValue(), GS_PASSWORD_MAXLENGTH);
		}
	}
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_Cancel(UIScreenObject InButton, int InPlayerIndex)
{
	OnCancel();

	return true;
}

function bool OnButtonBar_PlayOffline(UIScreenObject InButton, int InPlayerIndex)
{
	OnPlayOffline();

	return true;
}


function bool OnButtonBar_Login(UIScreenObject InButton, int InPlayerIndex)
{
	OnLogin();

	return true;
}

function bool OnButtonBar_CreateProfile(UIScreenObject InButton, int InPlayerIndex)
{
	OnCreateProfile();

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
	OnLogin();

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
			OnPlayOffline();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_Y')
		{
			OnCreateProfile();
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
	CreateProfileScene="UI_Scenes_FrontEnd.Scenes.CreateProfile"
	DescriptionMap.Add((WidgetTag="txtUserName",DataStoreMarkup="<Strings:UTgameUI.Profiles.LoginUserName_Description>"));
	DescriptionMap.Add((WidgetTag="txtPassword",DataStoreMarkup="<Strings:UTgameUI.Profiles.LoginPassword_Description>"));
	DescriptionMap.Add((WidgetTag="chkSavePassword",DataStoreMarkup="<Strings:UTgameUI.Profiles.SavePassword_Description>"));
	DescriptionMap.Add((WidgetTag="chkAutoLogin",DataStoreMarkup="<Strings:UTgameUI.Profiles.AutoLogin_Description>"));

	bSaveSceneValuesOnClose=false
}
