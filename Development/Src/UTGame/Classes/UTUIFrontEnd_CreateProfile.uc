/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Profile creation screen for UT3.
 */
class UTUIFrontEnd_CreateProfile extends UTUIFrontEnd_CustomScreen;

var transient UIEditBox	UserNameEditBox;
var transient UIEditBox	PasswordEditBox;
var transient UIEditBox	PasswordConfirmEditBox;
var transient UIEditBox	EMailEditBox;
var transient UIEditBox	CDKeyEditBox[4];
var transient UTUIScene_MessageBox MessageBoxReference;
var transient OnlineAccountInterface AccountInterface;
var transient EOnlineAccountCreateStatus CreateErrorStatus;

/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize( )
{
	local int EditboxIdx;

	Super.PostInitialize();

	// Get widget references and setup delegates
	UserNameEditBox = UIEditBox(FindChild('txtUserName',true));
	UserNameEditBox.NotifyActiveStateChanged = OnNotifyActiveStateChanged;
	UserNameEditBox.OnSubmitText=OnSubmitText;
	UserNameEditBox.SetDataStoreBinding("");
	UserNameEditBox.SetValue("");
	UserNameEditBox.MaxCharacters = GS_USERNAME_MAXLENGTH;
	UserNameEditBox.StringRenderComponent.bIgnoreMarkup = true;

	PasswordEditBox = UIEditBox(FindChild('txtPassword',true));
	PasswordEditBox.NotifyActiveStateChanged = OnNotifyActiveStateChanged;
	PasswordEditBox.OnSubmitText=OnSubmitText;
	PasswordEditBox.SetDataStoreBinding("");
	PasswordEditBox.SetValue("");
	PasswordEditBox.MaxCharacters = GS_PASSWORD_MAXLENGTH;
	PasswordEditbox.StringRenderComponent.bIgnoreMarkup = true;

	PasswordConfirmEditBox = UIEditBox(FindChild('txtPasswordConfirm',true));
	PasswordConfirmEditBox.NotifyActiveStateChanged = OnNotifyActiveStateChanged;
	PasswordConfirmEditBox.OnSubmitText=OnSubmitText;
	PasswordConfirmEditBox.SetDataStoreBinding("");
	PasswordConfirmEditBox.SetValue("");
	PasswordConfirmEditBox.MaxCharacters = GS_PASSWORD_MAXLENGTH;
	PasswordConfirmEditBox.StringRenderComponent.bIgnoreMarkup = true;

	EMailEditBox = UIEditBox(FindChild('txtEMail',true));
	EMailEditBox.NotifyActiveStateChanged = OnNotifyActiveStateChanged;
	EMailEditBox.OnSubmitText=OnSubmitText;
	EMailEditBox.SetDataStoreBinding("");
	EMailEditBox.SetValue("");
	EMailEditBox.MaxCharacters = GS_EMAIL_MAXLENGTH;
	EMailEditBox.StringRenderComponent.bIgnoreMarkup = true;


	for(EditboxIdx=0; EditboxIdx<4; EditboxIdx++)
	{
		CDKeyEditbox[EditboxIdx] = UIEditBox(FindChild(name("txtKey"$(EditboxIdx+1)),true));
		CDKeyEditbox[EditboxIdx].NotifyActiveStateChanged = OnNotifyActiveStateChanged;
		CDKeyEditbox[EditboxIdx].OnSubmitText=OnSubmitText;
		CDKeyEditbox[EditboxIdx].SetDataStoreBinding("");
		CDKeyEditbox[EditboxIdx].SetValue("");
		CDKeyEditbox[EditboxIdx].MaxCharacters = GS_CDKEY_PART_MAXLENGTH;
		CDKeyEditbox[EditboxIdx].StringRenderComponent.bIgnoreMarkup = true;
	}


	// Store reference to the account interface.
	AccountInterface=GetAccountInterface();
}

/** Sets up the scene's button bar. */
function SetupButtonBar()
{
	ButtonBar.Clear();

	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Cancel>", OnButtonBar_Cancel);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.CreateProfile>", OnButtonBar_CreateProfile);

	if( IsConsole(CONSOLE_PS3) )
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Keyboard>", OnButtonBar_ShowKeyboard);
	}
}

/** Callback for when the user wants to back out of this screen. */
function OnCancel()
{
	CloseScene(self);
}

/** Tries to create the user's account. */
function OnCreateProfile()
{
	local string UserName;
	local string Password;
	local string PasswordConfirm;
	local string EMailAddress;
	local string CDKey;

	// @todo: Retrieve CD Key
	CDKey = "";
	UserName = UserNameEditBox.GetValue();
	Password = PasswordEditBox.GetValue();
	PasswordConfirm = PasswordConfirmEditBox.GetValue();
	EMailAddress = EMailEditBox.GetValue();

	// Verify contents of user name box
	if(ValidateUserName(UserName))
	{
		if(Len(Password) > 0)
		{
			if(Password==PasswordConfirm)
			{
				if(Len(EMailAddress) > 0)
				{
					`Log("UTUIFrontEnd_CreateProfile::OnCreateProfile() - Creating profile for player with name '"$UserName$"'");

					if(AccountInterface != None)
					{
						UserName = TrimWhitespace(UserName);
						MessageBoxReference = GetMessageBoxScene();
						MessageBoxReference.DisplayModalBox("<Strings:UTGameUI.Generic.CreatingProfile>","");

						AccountInterface.AddCreateOnlineAccountCompletedDelegate(OnCreateOnlineAccountCompleted);
						if(AccountInterface.CreateOnlineAccount(StripInvalidUsernameCharacters(UserName), Password, EMailAddress, CDKey)==false)
						{
							OnCreateOnlineAccountCompleted(OACS_UnknownError);
						}
					}
					else
					{
						`Log("UTUIFrontEnd_CreateProfile::OnCreateProfile() - Unable to find account interface!");
					}
				}
				else
				{
					DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidEMail_Message>","<Strings:UTGameUI.Errors.InvalidEMail_Title>");
				}
			}
			else
			{
				DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidPasswordConfirm_Message>","<Strings:UTGameUI.Errors.InvalidPasswordConfirm_Title>");
			}
		}
		else
		{
			DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidPassword_Message>","<Strings:UTGameUI.Errors.InvalidPassword_Title>");
		}
	}
}

/**
 * Validates the user name and displays an error message if the name is invalid.
 *
 * @param UserName	Name to validate
 *
 * @return 	TRUE if the name is valid, FALSE otherwise.
 */
function bool ValidateUserName(string UserName)
{
	local bool bResult;
	local string FirstChar;

	bResult = false;

	if(Len(UserName) >= 3 && Len(UserName) <= GS_USERNAME_MAXLENGTH)
	{
		FirstChar = Mid(UserName, 0, 1);

			// Check for invalid characters
		if(   FirstChar != " "
		   && FirstChar != "+" 
		   && FirstChar != "#" 
		   && FirstChar != "@"  
		   && FirstChar != "0" 
		   && FirstChar != "1" 
		   && FirstChar != "2" 
		   && FirstChar != "3"
		   && FirstChar != "4"
		   && FirstChar != "5"
		   && FirstChar != "6"
		   && FirstChar != "7"
		   && FirstChar != "8"
		   && FirstChar != "9")
		{
			bResult=true;
		}
		else
		{
			DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidUserNameFirstChar_Message>","<Strings:UTGameUI.Errors.InvalidUserName_Title>");
		}
	}
	else
	{
		DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidUserNameCreateProfile_Message>","<Strings:UTGameUI.Errors.InvalidUserName_Title>");
	}

	return bResult;
}

/**
 * Delegate used in notifying the UI/game that the account creation completed
 *
 * @param ErrorStatus whether the account created successfully or not
 */
function OnCreateOnlineAccountCompleted(EOnlineAccountCreateStatus ErrorStatus)
{
	// Hide modal box and clear delegates
	MessageBoxReference.OnClosed = OnCreateOnlineAccount_Closed;
	MessageBoxReference.Close();

	CreateErrorStatus = ErrorStatus;
}

/** Callback for when the creating account message finishes hiding. */
function OnCreateOnlineAccount_Closed()
{
	MessageBoxReference.OnClosed = None;
	MessageBoxReference = None;

	AccountInterface.ClearCreateOnlineAccountCompletedDelegate(OnCreateOnlineAccountCompleted);

	switch(CreateErrorStatus)
	{
	case OACS_CreateSuccessful:
		// Display success messagebox
		MessageBoxReference = DisplayMessageBox("<Strings:UTGameUI.MessageBox.ProfileCreated_Message>","<Strings:UTGameUI.MessageBox.ProfileCreated_Title>");
		MessageBoxReference.OnClosed = OnProfileCreatedMessage_Closed;
		break;

//@todo ut3merge
//	case OACS_ServiceUnavailable:
//		DisplayMessageBox("<Strings:UTGameUI.Errors.ServiceUnavailable_Message>","<Strings:UTGameUI.Errors.ServiceUnavailable_Title>");
//		break;

	case OACS_InvalidUserName:
	case OACS_InvalidUniqueUserName:
		DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidUserName_Message>","<Strings:UTGameUI.Errors.InvalidUserName_Title>");
		break;

	case OACS_InvalidPassword:
		DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidPasswordForEmail_Message>","<Strings:UTGameUI.Errors.InvalidPasswordForEmail_Title>");
		break;

	case OACS_UniqueUserNameInUse:
		DisplayMessageBox("<Strings:UTGameUI.Errors.NameInUse_Message>","<Strings:UTGameUI.Errors.NameInUse_Title>");
		break;

	case OACS_UnknownError:
	default:
		// Display default failure message
		DisplayMessageBox("<Strings:UTGameUI.Errors.ProfileCreationFailed_Message>","<Strings:UTGameUI.Errors.ProfileCreationFailed_Title>");
		break;
	}
}

/** Callback for when the profile successfully created message box has closed. */
function OnProfileCreatedMessage_Closed()
{
	local UTUIFrontEnd_LoginScreen LoginScreen;

	// Log the user in
	LoginScreen = UTUIFrontEnd_LoginScreen(GetSceneClient().FindSceneByTag('LoginScreen'));
	LoginScreen.LocalLoginCheckBox.SetValue(false);
	LoginScreen.SavePasswordCheckBox.SetValue(true);
	LoginScreen.UserNameEditBox.SetValue(UserNameEditBox.GetValue());
	LoginScreen.UserNameEditBox.SetDatastoreBinding(UserNameEditBox.GetValue());
	LoginScreen.PasswordEditBox.SetValue(PasswordEditBox.GetValue());
	LoginScreen.PasswordEditBox.SetDatastoreBinding(PasswordEditBox.GetValue());
	LoginScreen.bLoginOnShow = true;

	MessageBoxReference.OnClosed = None;
	MessageBoxReference = None;
	CloseScene(self);
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
			ShowKeyboard(UserNameEditBox, Localize("Profiles", "UserName", "UTGameUI"),
				Localize("Profiles", "UserName_Description", "UTGameUI"), false, true, UserNameEditBox.GetValue(), GS_USERNAME_MAXLENGTH);
		}
		else if(PasswordEditBox.IsFocused())
		{
			ShowKeyboard(PasswordEditBox, Localize("Profiles", "Password", "UTGameUI"),
				Localize("Profiles", "Password_Description", "UTGameUI"), true, true, PasswordEditBox.GetValue(), GS_PASSWORD_MAXLENGTH);
		}
		else if(PasswordConfirmEditBox.IsFocused())
		{
			ShowKeyboard(PasswordConfirmEditBox, Localize("Profiles", "ConfirmPassword", "UTGameUI"),
				Localize("Profiles", "ConfirmPassword_Description", "UTGameUI"), true, true, PasswordConfirmEditBox.GetValue(), GS_PASSWORD_MAXLENGTH);
		}
		else if(EMailEditBox.IsFocused())
		{
			ShowKeyboard(EMailEditBox, Localize("Profiles", "EMailAddress", "UTGameUI"),
				Localize("Profiles", "EMailAddress_ShortDescription", "UTGameUI"), false, true, EMailEditBox.GetValue(), GS_EMAIL_MAXLENGTH);
		}
	}
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_Cancel(UIScreenObject InButton, int InPlayerIndex)
{
	OnCancel();

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
	OnCreateProfile();

	return false;
}


defaultproperties
{
	DescriptionMap.Add((WidgetTag="txtUserName",DataStoreMarkup="<Strings:UTgameUI.Profiles.UserName_Description>"));
	DescriptionMap.Add((WidgetTag="txtPassword",DataStoreMarkup="<Strings:UTgameUI.Profiles.Password_Description>"));
	DescriptionMap.Add((WidgetTag="txtPasswordConfirm",DataStoreMarkup="<Strings:UTgameUI.Profiles.ConfirmPassword_Description>"));
	DescriptionMap.Add((WidgetTag="txtEMail",DataStoreMarkup="<Strings:UTgameUI.Profiles.EMailAddress_Description>"));
	DescriptionMap.Add((WidgetTag="txtKey1",DataStoreMarkup="<Strings:UTgameUI.Profiles.CDKey_Description>"));
	DescriptionMap.Add((WidgetTag="txtKey2",DataStoreMarkup="<Strings:UTgameUI.Profiles.CDKey_Description>"));
	DescriptionMap.Add((WidgetTag="txtKey3",DataStoreMarkup="<Strings:UTgameUI.Profiles.CDKey_Description>"));
	DescriptionMap.Add((WidgetTag="txtKey4",DataStoreMarkup="<Strings:UTgameUI.Profiles.CDKey_Description>"));
}
