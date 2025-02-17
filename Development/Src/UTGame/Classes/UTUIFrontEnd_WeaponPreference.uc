/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * UI scene that allows the user to set their weapon preferences.
 */
class UTUIFrontEnd_WeaponPreference extends UTUIFrontEnd
	native(UIFrontEnd);

/** List of weapons. */
var transient UIList WeaponList;

/** Description of the currently selected weapon. */
var transient UILabel DescriptionLabel;

/** Label with the name of the currently selected weapon. */
var transient UILabel DetailsLabel;

/** scrollframe which contains the description label - allows the player to read long descriptions */
var	transient UIScrollFrame DescriptionScroller;

/** Reference to the menu datastore */
var transient UTUIDataStore_MenuItems MenuDataStore;

/** Weapon mesh widget. */
var transient UTUIMeshWidget	WeaponMesh;

/** Weapon classes. */
var transient array< class<UTWeapon> > WeaponClasses;

/** Weapon scale factor to use in 1024x768 */
var() float						BaseWeaponHeight;

/** default weapon priorities when entering the scene. */
var array<float>				OldPriorities;


/** Reference to the shift up and down buttons. */
var transient UIButton ShiftUpButton;
var transient UIButton ShiftDownButton;

/** Shift up/down label callouts. */
var transient UILabel ShiftUpLabel;
var transient UILabel ShiftDownLabel;

delegate MarkDirty();

/** Post initialize callback. */
event PostInitialize()
{
	local int WeaponIdx;
	local string WeaponClassStr;
	local class<UTWeapon> WeaponClassRef;

	Super.PostInitialize();

	// Find widget references
	WeaponList = UIList(FindChild('lstWeapons', true));
	WeaponList.OnValueChanged = OnWeaponList_ValueChanged;
	WeaponList.OnRawInputKey = OnWeaponList_RawInputKey;

	ShiftUpLabel = UILabel(FindChild('lblShiftUp', true));
	ShiftDownLabel = UILabel(FindChild('lblShiftDown', true));
	ShiftUpLabel.SetVisibility(IsConsole(CONSOLE_Any));
	ShiftDownLabel.SetVisibility(IsConsole(CONSOLE_Any));

	DetailsLabel = UILabel(FindChild('Details', true));
	DescriptionLabel = UILabel(FindChild('lblDescription', true));
	WeaponMesh = UTUIMeshWidget(FindChild('meshWeapon', true));
	WeaponMesh.BaseHeight = BaseWeaponHeight;

	// Get reference to the menu datastore
	MenuDataStore = UTUIDataStore_MenuItems(StaticResolveDataStore('UTMenuItems'));

	// Store old weapon priorities.
	OldPriorities.length=MenuDataStore.GetProviderCount('DropDownWeapons');
	WeaponClasses.length=OldPriorities.length;
	for(WeaponIdx=0; WeaponIdx<OldPriorities.length; WeaponIdx++)
	{
		if (MenuDataStore.GetValueFromProviderSet('DropDownWeapons', 'ClassName', WeaponIdx, WeaponClassStr))
		{

			WeaponClassRef = GetWeaponClass(WeaponClassStr);
			WeaponClasses[WeaponIdx] = WeaponClassRef;
			OldPriorities[WeaponIdx] = WeaponClassRef.default.Priority;
		}
	}

	DescriptionScroller = UIScrollFrame(FindChild('DescriptionScrollFrame',true));

	// if we're on a console platform, make the scrollframe not focusable.
	if ( IsConsole() && DescriptionScroller != None )
	{
		DescriptionScroller.SetPrivateBehavior(PRIVATE_NotFocusable, true);
	}


	// Setup shift button callbacks.
	ShiftUpButton = UIButton(FindChild('btnShiftUp', true));
	ShiftUpButton.OnClicked=OnButtonBar_ShiftUp;
	ShiftUpButton.SetVisibility(!IsConsole(CONSOLE_Any));
	ShiftDownButton = UIButton(FindChild('btnShiftDown', true));
	ShiftDownButton.OnClicked=OnButtonBar_ShiftDown;
	ShiftDownButton.SetVisibility(!IsConsole(CONSOLE_Any));

}

/**
 * Called just after the scene is added to the ActiveScenes array, or when this scene has become the active scene as a result
 * of closing another scene.
 *
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
event SceneActivated( bool bInitialActivation )
{
	Super.SceneActivated(bInitialActivation);

	InitializeWeaponList();
}

/** Sets up the button bar for the parent scene. */
function SetupButtonBar()
{
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Accept>", OnButtonBar_Accept);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ResetToDefaults>", OnButtonBar_ResetToDefaults);
}

/** Gets a priority for a weapon given its classname. */
event class<UTWeapon> GetWeaponClass(string ClassName)
{
	local class<UTWeapon> WeaponClassRef;

	WeaponClassRef = class<UTWeapon>(DynamicLoadObject(ClassName, class'Class'));
	return WeaponClassRef;
}

/** Callback for the mutator lists, captures the accept button before the mutators get to it. */
function bool OnWeaponList_RawInputKey( const out InputEventParameters EventParms )
{
	local bool bResult;

	bResult = false;

	if(EventParms.EventType==IE_Released)
	{
		if(EventParms.InputKeyName=='XboxTypeS_LeftTrigger')
		{
			OnResetToDefaults();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_A' || EventParms.InputKeyName=='Enter')
		{
			OnAccept();
			bResult = true;
		}
	}

	return bResult;
}

/** Updates the description and preview image for the currently selected weapon when the user changes the currently selected weapon. */
function OnWeaponList_ValueChanged( UIObject Sender, int PlayerIndex )
{
	local string OutValue;
	local int SelectedItem;
	local SkeletalMesh WeaponMeshRef;

	SelectedItem = WeaponList.GetCurrentItem();

	if(class'UTUIMenuList'.static.GetCellFieldString(WeaponList, 'Description', SelectedItem, OutValue))
	{
		DescriptionLabel.SetDataStoreBinding(OutValue);
	}

	if ( DescriptionScroller != None )
	{
		// reset the position of the scrollframe
		DescriptionScroller.SetClientRegionPosition(UIORIENT_Vertical, DescriptionScroller.GetPosition(UIFACE_Top, EVALPOS_PixelViewport));
	}

	if(class'UTUIMenuList'.static.GetCellFieldString(WeaponList, 'FriendlyName', SelectedItem, OutValue))
	{
		DetailsLabel.SetDataStoreBinding(OutValue);
	}

	/*
	if(class'UTUIMenuList'.static.GetCellFieldString(WeaponList, 'ClassName', SelectedItem, OutValue))
	{
		WeaponClassRef = class<UTWeapon>(FindObject(OutValue, class'Class'));
		if(WeaponClassRef!=None)
		{
			WeaponMesh.SkeletalMeshComp.SetSkeletalMesh(SkeletalMeshComponent(WeaponClassRef.Default.PickupFactoryMesh).SkeletalMesh);
		}
		else
		{
			`Log("UTUIFrontEnd_WeaponPreference::OnWeaponList_ValueChanged() - Unable to find weapon class:"@OutValue);
		}
	}
	*/

	if(class'UTUIMenuList'.static.GetCellFieldString(WeaponList, 'MeshReference', SelectedItem, OutValue))
	{
		WeaponMeshRef = SkeletalMesh(DynamicLoadObject(OutValue, class'SkeletalMesh'));
		if(WeaponMeshRef!=None)
		{
			WeaponMesh.SkeletalMeshComp.SetSkeletalMesh(WeaponMeshRef);
		}
		else
		{
			`Log("UTUIFrontEnd_WeaponPreference::OnWeaponList_ValueChanged() - FindObject failed for:"@OutValue);
		}
	}
}

/** Callback for when the user has accepted their weapon preferences. */
function OnAccept()
{
	local int WeaponIdx;
	local string WeaponClassStr;
	local class<UTWeapon> WeaponClassRef;
	local UTProfileSettings Profile;
	local int ProfileSettingId;
	local float CurrentPriority;

	Profile = GetPlayerProfile();

	for(WeaponIdx=0; WeaponIdx<MenuDataStore.WeaponPriority.length; WeaponIdx++)
	{
		if (MenuDataStore.GetValueFromProviderSet('DropDownWeapons', 'ClassName', MenuDataStore.WeaponPriority[WeaponIdx], WeaponClassStr))
		{
			CurrentPriority = MenuDataStore.WeaponPriority.length-WeaponIdx;
			WeaponClassRef = GetWeaponClass(WeaponClassStr);
			WeaponClassRef.default.Priority = CurrentPriority;
			WeaponClassRef.static.StaticSaveConfig();

			// Store default UT weapons in the profile
			if(Profile.GetProfileSettingId(name(WeaponClassStr$"_Priority"), ProfileSettingId))
			{
				Profile.SetProfileSettingValueFloat(ProfileSettingId, CurrentPriority);
			}
		}
	}

	CloseScene(self);
}


/** Callback for when the user wants to back out of the scene. */
function OnBack()
{
	local int WeaponIdx;
	local string WeaponClassStr;
	local class<UTWeapon> WeaponClassRef;

	// Restore weapon priorities
	for(WeaponIdx=0; WeaponIdx<OldPriorities.length; WeaponIdx++)
	{
		if (MenuDataStore.GetValueFromProviderSet('DropDownWeapons', 'ClassName', WeaponIdx, WeaponClassStr))
		{
			WeaponClassRef = GetWeaponClass(WeaponClassStr);
			WeaponClassRef.default.Priority = OldPriorities[WeaponIdx];
		}
	}

	CloseScene(self);
}

/** Shifts the currently selected weapon up or down in the weapon preference order. */
function OnShiftWeapon(bool bShiftUp)
{
	local int SelectedItem;
	local int SwapItem;
	local int NewIndex;

	SelectedItem = WeaponList.Index;

	if(bShiftUp)
	{
		NewIndex = SelectedItem-1;
	}
	else
	{
		NewIndex = SelectedItem+1;
	}

	if(NewIndex >= 0 && NewIndex < MenuDataStore.WeaponPriority.length)
	{
		SwapItem = MenuDataStore.WeaponPriority[NewIndex];
		MenuDataStore.WeaponPriority[NewIndex] = MenuDataStore.WeaponPriority[SelectedItem];
		MenuDataStore.WeaponPriority[SelectedItem] = SwapItem;

		WeaponList.RefreshSubscriberValue();
		WeaponList.SetIndex(NewIndex);
	}

	MarkDirty();
}

/** Initializes the weapon list. */
function InitializeWeaponList()
{
	local int WeaponIdx;
	local array<float> Priorities;	// Stores the list of priorities for the set of weapons in the weapon priority array.
	local float CurrentPriority;
	local int PriorityIdx;
	local bool bAddedItem;
	local class<UTWeapon> WeaponClassRef;
	local string WeaponClassStr;

	// Initialize the weapon priority array, get the names of all possible classes and add them to the weapon priority array in
	// order of their priority, from highest to lowest.
	MenuDataStore.WeaponPriority.length = 0;
	for(WeaponIdx=0; WeaponIdx<MenuDataStore.GetProviderCount('DropDownWeapons'); WeaponIdx++)
	{
		bAddedItem = false;

		if (MenuDataStore.GetValueFromProviderSet('DropDownWeapons', 'ClassName', WeaponIdx, WeaponClassStr))
		{
			WeaponClassRef = class<UTWeapon>(DynamicLoadObject(WeaponClassStr, class'Class'));
			CurrentPriority = WeaponClassRef.default.Priority;

			for(PriorityIdx=0; PriorityIdx < Priorities.length; PriorityIdx++)
			{
				if(Priorities[PriorityIdx] < CurrentPriority)
				{
					MenuDataStore.WeaponPriority.Insert(PriorityIdx, 1);
					MenuDataStore.WeaponPriority[PriorityIdx]=WeaponIdx;

					Priorities.Insert(PriorityIdx, 1);
					Priorities[PriorityIdx]=CurrentPriority;

					bAddedItem = true;
					break;
				}
			}

			if(bAddedItem==false)
			{
				MenuDataStore.WeaponPriority.AddItem(WeaponIdx);
				Priorities.AddItem(CurrentPriority);
			}
		}
	}

	WeaponIdx = WeaponList.Index;
	WeaponList.RefreshSubscriberValue();
	WeaponList.SetIndex(WeaponIdx);
}

/** Loads the default weapon priorities from the INI file. */
native function LoadINIDefaults();

/** Resets the weapon order to its defaults. */
function OnResetToDefaults()
{
	LoadINIDefaults();
	InitializeWeaponList();

	MarkDirty();
}

/** Button bar callbacks. */
function bool OnButtonBar_ShiftUp(UIScreenObject InButton, int PlayerIndex)
{
	OnShiftWeapon(true);

	return true;
}

function bool OnButtonBar_ShiftDown(UIScreenObject InButton, int PlayerIndex)
{
	OnShiftWeapon(false);

	return true;
}

function bool OnButtonBar_Accept(UIScreenObject InButton, int PlayerIndex)
{
	OnAccept();

	return true;
}

function bool OnButtonBar_Back(UIScreenObject InButton, int PlayerIndex)
{
	OnBack();

	return true;
}

function bool OnButtonBar_ResetToDefaults(UIScreenObject InButton, int PlayerIndex)
{
	OnResetToDefaults();

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

	bResult = false;

	if(EventParms.EventType==IE_Released)
	{
		if(EventParms.InputKeyName=='XboxTypeS_A' || EventParms.InputKeyName=='XboxTypeS_Start' || EventParms.InputKeyName=='Enter')
		{
			OnAccept();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
		{
			OnBack();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_LeftTrigger')
		{
			OnResetToDefaults();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_LeftShoulder')
		{
			OnShiftWeapon(true);
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_RightShoulder')
		{
			OnShiftWeapon(false);
			bResult=true;
		}
	}

	return bResult;
}


defaultproperties
{
	BaseWeaponHeight=256.0
}

