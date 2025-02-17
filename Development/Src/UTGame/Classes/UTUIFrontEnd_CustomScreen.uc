/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*
* Custom Scene for UT3, contains a description label and an automated way of providing descriptions for widgets.
*/
class UTUIFrontEnd_CustomScreen extends UTUIFrontEnd
	native(UIFrontEnd);

var transient UILabel	DescriptionLabel;

/** Target editbox for the onscreen keyboard, if any. */
var transient UIEditBox KeyboardTargetEditBox;

/** Whether or not the keyboard being displayed is a password keyboard. */
var transient bool bIsPasswordKeyboard;

/** Mapping of widget tag to field description. */
struct native DescriptionMapping
{
	var name WidgetTag;
	var string DataStoreMarkup;
};

var transient array<DescriptionMapping> DescriptionMap;

/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize( )
{
	Super.PostInitialize();

	DescriptionLabel = UILabel(FindChild('lblDescription', true));
}


/** Callback for when the object's active state changes. */
function OnNotifyActiveStateChanged( UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, optional UIState PreviouslyActiveState )
{
	local int MappingIdx;

	if(NewlyActiveState.Class == class'UIState_Focused'.default.Class)
	{
		// Loop through all description mappings and try to set a description based on the currently focused widget.
		for(MappingIdx=0; MappingIdx<DescriptionMap.length; MappingIdx++)
		{
			if(DescriptionMap[MappingIdx].WidgetTag==UIObject(Sender).WidgetTag)
			{
				DescriptionLabel.SetDataStoreBinding(DescriptionMap[MappingIdx].DataStoreMarkup);
				break;
			}
		}
	}
}

/** Shows the on screen keyboard. */
function ShowKeyboard(UIEditBox InTargetEditBox, string Title, optional string Message, optional bool bPassword, optional bool bShouldValidate, optional string DefaultText="", optional int MaxLength)
{
	local OnlinePlayerInterface PlayerInt;

	KeyboardTargetEditBox = InTargetEditBox;

	bIsPasswordKeyboard=bPassword;
	PlayerInt = GetPlayerInterface();
	PlayerInt.AddKeyboardInputDoneDelegate(OnKeyboardInputComplete);
	GetUTInteraction().BlockUIInput(true);	// block input
	if(PlayerInt.ShowKeyboardUI(GetPlayerIndex(), Title, Message, bPassword, bShouldValidate, DefaultText, MaxLength)==false)
	{
		OnKeyboardInputComplete(false);
	}
}

/**
 * Delegate used when the keyboard input request has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnKeyboardInputComplete(bool bWasSuccessful)
{
	local OnlinePlayerInterface PlayerInt;
	local byte bWasCancelled;
	local string KeyboardResults;

	PlayerInt = GetPlayerInterface();

	if(PlayerInt != None)
	{
		GetUTInteraction().BlockUIInput(false);	// unblock input
		PlayerInt.ClearKeyboardInputDoneDelegate(OnKeyboardInputComplete);

		if(bWasSuccessful)
		{
			KeyboardResults = PlayerInt.GetKeyboardInputResults(bWasCancelled);

			if(bool(bWasCancelled)==false)
			{
				if(!bIsPasswordKeyboard)
				{
					KeyboardResults=TrimWhitespace(KeyboardResults);
				}

				KeyboardTargetEditBox.SetValue(KeyboardResults);
			}
		}
	}
}

/**
 * Strips any characters which are deemed invalid for profile names from the specified username.
 *
 * @param	Username	the username to strip invalid characters from.
 *
 * @return	the username with all invalid characters stripped.
 */
static function string StripInvalidUsernameCharacters( string Username )
{
	Username = TrimWhitespace(Username);

	// question marks break URL parsing, so strip those as well
	Username = Repl(Username, "?", "");

	//@todo - at this time, there is a bug on gamespy's end that prevents profiles which contain apostrophes from
	// logging in online successfully, so we'll need to strip apostrophes from the profile name until gamespy
	// fixes this bug on their end.
	Username = Repl(Username, "'", "");

	return Username;
}
