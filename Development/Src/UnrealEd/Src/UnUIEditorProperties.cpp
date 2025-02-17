/*=============================================================================
	UnUIEditorProperties.cpp: UI editor custom property window implementations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineUIPrivateClasses.h"

#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UnUIEditorProperties.h"
#include "GenericBrowser.h"
#include "Properties.h"
#include "PropertyWindowManager.h"	// required for access to GPropertyWindowManager
#include "ScopedTransaction.h"
#include "ScopedPropertyChange.h"	// required for access to FScopedPropertyChange

IMPLEMENT_CLASS(UUITexture_CustomInputProxy);
IMPLEMENT_CLASS(UUITexture_CustomDrawProxy);
IMPLEMENT_CLASS(UPlayerInputMask_CustomDrawProxy);
IMPLEMENT_CLASS(UPlayerInputMask_CustomInputProxy);
IMPLEMENT_CLASS(UUIState_CustomInputProxy);
IMPLEMENT_CLASS(UUIStateClass_CustomInputProxy);

IMPLEMENT_CLASS(UCustomInputProxy_Base);

typedef WxPropertyWindow_Objects::TObjectIterator		TPropObjectIterator;
typedef WxPropertyWindow_Objects::TObjectConstIterator	TPropObjectConstIterator;

/* ==========================================================================================================
	WxCustomPropertyItem_ConditionalItem
========================================================================================================== */
IMPLEMENT_DYNAMIC_CLASS(WxCustomPropertyItem_ConditionalItem,WxPropertyWindow_Item);

/**
 * Returns the property used for determining whether this item's Property can be edited.
 *
 * @todo ronp - eventually, allow any type of property by specifying required value in metadata
 */
UBoolProperty* WxCustomPropertyItem_ConditionalItem::GetEditConditionProperty() const
{
	UBoolProperty* EditConditionProperty = NULL;
	if ( Property != NULL )
	{
		// find the name of the property that should be used to determine whether this property should be editable
		FString ConditionPropertyName = Property->GetMetaData(TEXT("EditCondition"));

		// for now, only support boolean conditions, and only allow use of another property within the same struct as the conditional property
		if ( ConditionPropertyName.Len() > 0 && ConditionPropertyName.InStr(TEXT(".")) == INDEX_NONE )
		{
			UStruct* Scope = Property->GetOwnerStruct();
			EditConditionProperty = FindFieldWithFlag<UBoolProperty,CASTCLASS_UBoolProperty>(Scope, *ConditionPropertyName);
		}
	}
	return EditConditionProperty;
}

/**
 * Finds the property being used to determine whether this item's associated property should be editable/expandable.
 * If the propery is successfully found, ConditionPropertyAddresses will be filled with the addresses of the conditional properties' values.
 *
 * @param	ConditionProperty	receives the value of the property being used to control this item's editability.
 * @param	ConditionPropertyAddresses	receives the addresses of the actual property values for the conditional property
 *
 * @return	TRUE if both the conditional property and property value addresses were successfully determined.
 */
UBOOL WxCustomPropertyItem_ConditionalItem::GetEditConditionPropertyAddress( UBoolProperty*& ConditionProperty, TArray<BYTE*>& ConditionPropertyAddresses )
{
	UBOOL bResult = FALSE;

	UBoolProperty* EditConditionProperty = GetEditConditionProperty();
	if ( EditConditionProperty != NULL )
	{
		check(ParentItem);
		WxPropertyWindow_Objects* ObjectNode = FindObjectItemParent();
		for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
		{
			UObject* Obj = *Itor;

			// Get the address corresponding to the base of this property (i.e. if a struct property, set BaseOffset to the address of value for the whole struct)
			BYTE* BaseOffset = ParentItem->GetContents((BYTE*)Obj);
			check(BaseOffset != NULL);

			// now calculate the address of the property value being used as the condition and add it to the array.
			ConditionPropertyAddresses.AddItem( BaseOffset + EditConditionProperty->Offset );
			bResult = TRUE;
		}
	}

	if ( bResult )
	{
		// set the output variable
		ConditionProperty = EditConditionProperty;
	}

	return bResult;
}

/**
 * Returns TRUE if the value of the conditional property matches the value required.  Indicates whether editing or otherwise interacting with this item's
 * associated property should be allowed.
 */
UBOOL WxCustomPropertyItem_ConditionalItem::IsConditionMet()
{
	UBOOL bResult = FALSE;

	UBoolProperty* ConditionProperty;
	TArray<BYTE*> ConditionValues;

	// get the property we're going to use as the condition for allowing edits, as well as the addresses of the values for that property in all selected objects
	if ( GetEditConditionPropertyAddress(ConditionProperty, ConditionValues) )
	{
		UBOOL bAllConditionsMet = TRUE;
		
		for ( INT ValueIdx = 0; bAllConditionsMet && ValueIdx < ConditionValues.Num(); ValueIdx++ )
		{
			BYTE* ValueAddr = ConditionValues(ValueIdx);
			bAllConditionsMet = (*(BITFIELD*)ValueAddr & ConditionProperty->BitMask) != 0;
		}

		bResult = bAllConditionsMet;
	}

	return bResult;
}

/**
 * Returns TRUE if MouseX and MouseY fall within the bounding region of the checkbox used for displaying the value of this property's edit condition.
 */
UBOOL WxCustomPropertyItem_ConditionalItem::ClickedCheckbox( INT MouseX, INT MouseY ) const
{
	const wxRect rc = GetClientRect();
	if ( MouseX >= (rc.x + IndentX - PROP_Indent) && MouseX <= (rc.x + IndentX) &&
		MouseY >= rc.y && MouseY <= rc.GetBottom() )
	{
		return TRUE;
	}

	return FALSE;
}

/**
 * Toggles the value of the property being used as the condition for editing this property.
 *
 * @return	the new value of the condition (i.e. TRUE if the condition is now TRUE)
 */
UBOOL WxCustomPropertyItem_ConditionalItem::ToggleConditionValue()
{
	UBOOL bNewValue = FALSE;

	UBoolProperty* ConditionProperty;
	TArray<BYTE*> ConditionValues;

	// get the property we're going to use as the condition for allowing edits, as well as the addresses of the values for that property in all selected objects
	if ( GetEditConditionPropertyAddress(ConditionProperty, ConditionValues) )
	{
		FScopedPropertyChange PropertyChangeNotification(this, ConditionProperty, TRUE);

		bNewValue = !IsConditionMet();
		for ( INT ValueIdx = 0; ValueIdx < ConditionValues.Num(); ValueIdx++ )
		{
			BYTE* ValueAddr = ConditionValues(ValueIdx);
			if ( bNewValue )
			{
				*(BITFIELD*)ValueAddr |= ConditionProperty->BitMask;
			}
			else
			{
				*(BITFIELD*)ValueAddr &= ~ConditionProperty->BitMask;
			}
		}
	}

	return bNewValue;
}

/**
 * Renders the left side of the property window item.
 *
 * This version is responsible for rendering the checkbox used for toggling whether this property item window should be enabled.
 *
 * @param	RenderDeviceContext		the device context to use for rendering the item name
 * @param	ClientRect				the bounding region of the property window item
 */
void WxCustomPropertyItem_ConditionalItem::RenderItemName( wxBufferedPaintDC& RenderDeviceContext, const wxRect& ClientRect )
{
	const UBOOL bItemEnabled = IsConditionMet();

	// determine which checkbox image to render
 	const WxMaskedBitmap& bmp = bItemEnabled
		? GPropertyWindowManager->CheckBoxOnB
		: GPropertyWindowManager->CheckBoxOffB;

	// render the checkbox bitmap
 	RenderDeviceContext.DrawBitmap( bmp, ClientRect.x + IndentX - PROP_Indent, ClientRect.y + ((PROP_DefaultItemHeight - bmp.GetHeight()) / 2), 1 );

	// now render the text itself; if the item is disabled, change the font color to light grey
	if ( !bItemEnabled )
	{
		wxColour DisabledColor(64,64,64);
		RenderDeviceContext.SetTextForeground(DisabledColor);
	}

	WxPropertyWindow_Item::RenderItemName(RenderDeviceContext, ClientRect);
}

/**
 * Called when an property window item receives a left-mouse-button press which wasn't handled by the input proxy.  Typical response is to gain focus
 * and (if the property window item is expandable) to toggle expansion state.
 *
 * @param	Event	the mouse click input that generated the event
 *
 * @return	TRUE if this property window item should gain focus as a result of this mouse input event.
 */
UBOOL WxCustomPropertyItem_ConditionalItem::ClickedPropertyItem( wxMouseEvent& Event )
{
	UBOOL bShouldGainFocus = TRUE;

	// if this property is edit-const, it can't be changed
	// or if we couldn't find a valid condition property, also use the base version
	if ( Property == NULL || (Property->PropertyFlags & CPF_EditConst) != 0 || GetEditConditionProperty() == NULL )
	{
		bShouldGainFocus = WxPropertyWindow_Item::ClickedPropertyItem(Event);
	}

	// if they clicked on the checkbox, toggle the edit condition
	else if ( ClickedCheckbox(Event.GetX(), Event.GetY()) )
	{
		FScopedTransaction Transaction(*LocalizeUnrealEd("PropertyWindowEditProperties"));
		FScopedPropertyChange PropertyChangeNotification(this, NULL, TRUE);

		bShouldGainFocus = !bCanBeExpanded;
		if ( ToggleConditionValue() == FALSE )
		{
			bShouldGainFocus = FALSE;

			// if we just disabled the condition which allows us to edit this control
			// collapse the item if this is an expandable item
			if ( bCanBeExpanded )
			{
				Collapse();
			}

			//@todo - for editboxes and things like that - we need a way to force this item to lose focus
// 			else
// 			{
// 				TopPropertyWindow->SetFocus();
// 			}
		}

		if ( !bCanBeExpanded && ParentItem != NULL )
		{
			ParentItem->Refresh();
		}
		else
		{
			Refresh();
		}
	}
	// if the condition for editing this control has been met (i.e. the checkbox is checked), pass the event back to the base version, which will do the right thing
	// based on where the user clicked
	else if ( IsConditionMet() )
	{
		bShouldGainFocus = WxPropertyWindow_Item::ClickedPropertyItem(Event);
	}
	else
	{
		// the condition is false, so this control isn't allowed to do anything - swallow the event.
		bShouldGainFocus = FALSE;
	}

	return bShouldGainFocus;
}

/**
 * Expands child items.  Does nothing if the window is already expanded.
 */
void WxCustomPropertyItem_ConditionalItem::Expand()
{
	if ( IsConditionMet() )
	{
		WxPropertyWindow_Item::Expand();
	}
}

/**
 * Recursively searches through children for a property named PropertyName and expands it.
 * If it's a UArrayProperty, the propery's ArrayIndex'th item is also expanded.
 */
void WxCustomPropertyItem_ConditionalItem::ExpandItem( const FString& PropertyName, INT ArrayIndex )
{
	if ( IsConditionMet() )
	{
		WxPropertyWindow_Item::ExpandItem(PropertyName,ArrayIndex);
	}
}

/**
 * Expands all items rooted at the property window node.
 */
void WxCustomPropertyItem_ConditionalItem::ExpandAllItems()
{
	if ( IsConditionMet() )
	{
		WxPropertyWindow_Item::ExpandAllItems();
	}
}

/**
 * Expands all category items belonging to this window.
 */
void WxCustomPropertyItem_ConditionalItem::ExpandCategories()
{
	if ( IsConditionMet() )
	{
		WxPropertyWindow_Item::ExpandCategories();
	}
}

/* ==========================================================================================================
	UUITexture_CustomDrawProxy
========================================================================================================== */
UBOOL UUITexture_CustomDrawProxy::Supports( const WxPropertyWindow_Base* InItem, INT ArrayIdx ) const
{
	UBOOL bResult = FALSE;

	// this draw proxy is only valid for properties of type UITexture
	if ( InItem->Property != NULL )
	{
		UObjectProperty* ObjectProp = Cast<UObjectProperty>(InItem->Property);
		if ( ObjectProp != NULL )
		{
			if ( ObjectProp->PropertyClass->IsChildOf(UUITexture::StaticClass()) )
			{
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

void UUITexture_CustomDrawProxy::Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy )
{
	// override the default Draw so that we can draw the name of the material/texture assigned to the UIImage's UITextureBase, rather than the name
	// of the UITextureBase (which is unimportant)
	FString CurrentValueString = *LocalizeUnrealEd(TEXT("None"));
	UUITexture* ImageWrapper = *(UUITexture**)InReadAddress;
	if ( ImageWrapper != NULL )
	{
		USurface* CurrentValue = ImageWrapper->GetSurface();
		if ( CurrentValue != NULL )
		{
			CurrentValueString = CurrentValue->GetPathName();
		}
	}

	// . . . and draw it.
	wxCoord W, H;
	InDC->GetTextExtent( *CurrentValueString, &W, &H );
	InDC->DrawText( *CurrentValueString, InRect.x, InRect.y+((InRect.GetHeight() - H) / 2) );
}

/* ==========================================================================================================
	UPlayerInputMask_CustomDrawProxy
========================================================================================================== */
UBOOL UPlayerInputMask_CustomDrawProxy::Supports( const WxPropertyWindow_Base* InItem, INT ArrayIdx ) const
{
	UBOOL bResult = FALSE;

	// this draw proxy is only valid for the UIScreenObject::PlayerInputMask property
	if ( InItem->Property != NULL )
	{
		if ( InItem->Property->GetOwnerClass() == UUIObject::StaticClass() && InItem->Property->GetFName() == TEXT("PlayerInputMask") )
		{
			bResult = TRUE;
		}
	}

	return bResult;
}

#define MASKCHECKBOX_WIDTH 30
void UPlayerInputMask_CustomDrawProxy::Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy )
{
	// get the value of the widget's PlayerInputMask property
	BYTE PlayerInputMask = (*(BYTE*)InReadAddress) >> 4;

	// if PlayerInputMask is 0, it means that a specific input mask has not been set, so only the "Any" checkbox will be checked (which means "use whatever mask the scene is using")
	const UBOOL bAllowAnyGamepad = PlayerInputMask == 0;

	// store the rendering bounding region into temp vars
	int CurX = InRect.x;
	int CheckboxY = InRect.y + ((InRect.GetHeight() - 13) / 2);

	// for each supported gamepad
	for ( INT GamepadIndex = 0; GamepadIndex < UCONST_MAX_SUPPORTED_GAMEPADS; GamepadIndex++ )
	{
		// remember where we started rendering the left edge of this checkbox so we can evenly space them
		int StartX = CurX;

		// determine which checkbox image should be rendered for this gamepad; if the input mask is 255, it means the widget
		// accepts input from any gamepad, so we will only enable the "any" checkbox
		const WxMaskedBitmap& checkBox = (!bAllowAnyGamepad && (PlayerInputMask & (1<<GamepadIndex))) ? GPropertyWindowManager->CheckBoxOnB : GPropertyWindowManager->CheckBoxOffB;
		InDC->DrawBitmap( checkBox, CurX, CheckboxY, true );

		// advance the cursor past the checkbox and add a little spacing between the checkbox and the index label
		CurX += checkBox.GetWidth() + 1;

		// now render the label describing which gamepad this checkbox is associated with.
		FString IndexLabel = appItoa(GamepadIndex + 1);

		wxCoord TextWidth, TextHeight;
		InDC->GetTextExtent( *IndexLabel, &TextWidth, &TextHeight );
		InDC->DrawText( *IndexLabel, CurX, InRect.y + ((InRect.GetHeight() - TextHeight) / 2) );

		// increment the cursor by a fixed amount to make it easy to handle hit detection
		CurX = StartX + MASKCHECKBOX_WIDTH;
	}

	// let's leave a little bit more room between the other options and the "Any" option
	CurX += 2;
	const WxMaskedBitmap& checkBox = bAllowAnyGamepad ? GPropertyWindowManager->CheckBoxOnB : GPropertyWindowManager->CheckBoxOffB;
	InDC->DrawBitmap( checkBox, CurX, CheckboxY, true );
	CurX += checkBox.GetWidth() + 1;

	wxCoord TextWidth, TextHeight;
	InDC->GetTextExtent( TEXT("Any"), &TextWidth, &TextHeight );
	InDC->DrawText( TEXT("Any"), CurX, InRect.y + ((InRect.GetHeight() - TextHeight) / 2) );
}


/**
 * Draws a representation of an unknown value (i.e. when multiple objects have different values in the same field).
 *
 * @param	InDC			The wx device context.
 * @param	InRect			The rect to draw the value into.
 */
void UPlayerInputMask_CustomDrawProxy::DrawUnknown(wxDC* InDC, wxRect InRect )
{
	int CurX = InRect.x;
	int CheckboxY = InRect.y + ((InRect.GetHeight() - 13) / 2);

	for ( INT GamepadIndex = 0; GamepadIndex < UCONST_MAX_SUPPORTED_GAMEPADS; GamepadIndex++ )
	{
		int StartX = CurX;
		FString IndexLabel = appItoa(GamepadIndex + 1);

		const WxMaskedBitmap& checkBox = GPropertyWindowManager->CheckBoxUnknownB;
		InDC->DrawBitmap( checkBox, CurX, CheckboxY, true );
		CurX += checkBox.GetWidth() + 1;

		wxCoord TextWidth, TextHeight;
		InDC->GetTextExtent( *IndexLabel, &TextWidth, &TextHeight );
		InDC->DrawText( *IndexLabel, CurX, InRect.y + ((InRect.GetHeight() - TextHeight) / 2) );
		CurX = StartX + MASKCHECKBOX_WIDTH;
	}

	const WxMaskedBitmap& checkBox = GPropertyWindowManager->CheckBoxUnknownB;
	InDC->DrawBitmap( checkBox, CurX, CheckboxY, true );
	CurX += checkBox.GetWidth() + 1;

	wxCoord TextWidth, TextHeight;
	InDC->GetTextExtent( TEXT("Any"), &TextWidth, &TextHeight );
	InDC->DrawText( TEXT("Any"), CurX, InRect.y + ((InRect.GetHeight() - TextHeight) / 2) );
}

/* ==========================================================================================================
	UUIState_CustomInputProxy
========================================================================================================== */

/**
 * Determines if this input proxy supports the specified UProperty.
 *
 * @param	InItem			The property window node.
 * @param	InArrayIdx		The index in the case of array properties.
 *
 * @return					TRUE if the draw proxy supports the specified property.
 */
UBOOL UUIState_CustomInputProxy::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	UBOOL bResult = FALSE;

	if ( Super::Supports(InItem, InArrayIdx) && InItem->Property != NULL )
	{
		UObjectProperty* ObjectProperty = Cast<UObjectProperty>(InItem->Property);
		if ( ObjectProperty != NULL && ObjectProperty->PropertyClass->IsChildOf(UUIState::StaticClass()) )
		{
			bResult = TRUE;
		}
	}

	return bResult;
}


/**
 * Wrapper method for determining whether a class is valid for use by this property item input proxy.
 *
 * @param	InItem			the property window item that contains this proxy.
 * @param	CheckClass		the class to verify
 * @param	bAllowAbstract	TRUE if abstract classes are allowed
 *
 * @return	TRUE if CheckClass is valid to be used by this input proxy
 */
UBOOL UUIState_CustomInputProxy::IsClassAllowed( WxPropertyWindow_Base* InItem,  UClass* CheckClass, UBOOL bAllowAbstract ) const
{
	UBOOL bResult = Super::IsClassAllowed(InItem, CheckClass, bAllowAbstract);
	if ( bResult )
	{
		UUIState* StateCDO = CheckClass->GetDefaultObject<UUIState>();

		// return FALSE if any of the widgets being edited aren't supported by this class
		WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
		for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
		{
			UUIScreenObject* CurrentObject = CastChecked<UUIScreenObject>(*Itor);
			if ( !StateCDO->eventIsWidgetClassSupported(CurrentObject->GetClass()) )
			{
				bResult = FALSE;
				break;
			}
		}
	}
	return bResult;
}

/* ==========================================================================================================
	UUIStateClass_CustomInputProxy
========================================================================================================== */

/**
 * Determines if this input proxy supports the specified UProperty.
 *
 * @param	InItem			The property window node.
 * @param	InArrayIdx		The index in the case of array properties.
 *
 * @return					TRUE if the draw proxy supports the specified property.
 */
UBOOL UUIStateClass_CustomInputProxy::Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
{
	UBOOL bResult = FALSE;

	if ( Super::Supports(InItem, InArrayIdx) && InItem->Property != NULL )
	{
		UClassProperty* ClassProperty = Cast<UClassProperty>(InItem->Property);
		if ( ClassProperty != NULL && ClassProperty->MetaClass->IsChildOf(UUIState::StaticClass()) )
		{
			bResult = TRUE;
		}
	}

	return bResult;
}


/**
 * Wrapper method for determining whether a class is valid for use by this property item input proxy.
 *
 * @param	InItem			the property window item that contains this proxy.
 * @param	CheckClass		the class to verify
 * @param	bAllowAbstract	TRUE if abstract classes are allowed
 *
 * @return	TRUE if CheckClass is valid to be used by this input proxy
 */
UBOOL UUIStateClass_CustomInputProxy::IsClassAllowed( WxPropertyWindow_Base* InItem,  UClass* CheckClass, UBOOL bAllowAbstract ) const
{
	UBOOL bResult = Super::IsClassAllowed(InItem, CheckClass, bAllowAbstract);
	if ( bResult )
	{
		UUIState* StateCDO = CheckClass->GetDefaultObject<UUIState>();

		// return FALSE if any of the widgets being edited aren't supported by this class
		WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
		for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
		{
			UUIScreenObject* CurrentObject = CastChecked<UUIScreenObject>(*Itor);
			if ( !StateCDO->eventIsWidgetClassSupported(CurrentObject->GetClass()) )
			{
				bResult = FALSE;
				break;
			}
		}
	}
	return bResult;
}

/* ==========================================================================================================
	UUITexture_CustomInputProxy
========================================================================================================== */
UBOOL UUITexture_CustomInputProxy::Supports( const WxPropertyWindow_Base* InItem, INT ArrayIdx ) const
{
	UBOOL bResult = FALSE;

	// this input proxy is only valid for properties of type UITexture
	if ( InItem->Property != NULL )
	{
		UObjectProperty* ObjectProp = Cast<UObjectProperty>(InItem->Property);
		if ( ObjectProp != NULL )
		{
			if ( ObjectProp->PropertyClass->IsChildOf(UUITexture::StaticClass()) )
			{
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

// Allows the created controls to react to the parent window being resized.  Derived classes
// that implement this method should to call up to their parent.
void UUITexture_CustomInputProxy::CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue )
{
	// InValue will be the path name of the UITexture assigned for the value of UIImage.Image, which
	// we don't want.  Get the name of the underlying texture or material assigned to this UIImage and
	// display that instead
	
	// this will be the list of materials/textures assigned to the selected objects
	TArray<USurface*> ImageSurfaces;
	TArray<BYTE*> Addresses;

	if ( InItem->GetReadAddress(InItem, FALSE, Addresses, FALSE) )
	{
		for ( INT i = 0; i < Addresses.Num(); i++ )
		{
			UUITexture* ImageWrapper = *(UUITexture**)Addresses(i);
			if ( ImageWrapper != NULL )
			{
				ImageSurfaces.AddUniqueItem(ImageWrapper->GetSurface());
			}
		}
	}

	FString NewValue;
	if ( ImageSurfaces.Num() == 0 )
	{
		NewValue = LocalizeUnrealEd(TEXT("None"));
	}
	else if ( ImageSurfaces.Num() == 1 )
	{
		if ( ImageSurfaces(0) != NULL )
		{
            NewValue = ImageSurfaces(0)->GetPathName();
		}
		else
		{
			NewValue = LocalizeUnrealEd(TEXT("None"));
		}
	}
	else
	{
		NewValue = LocalizeUnrealEd(TEXT("MultiplePropertyValues"));
	}

	// Create any buttons for the control.
	Super::CreateControls( InItem, InBaseClass, InRC, *NewValue );
}

void UUITexture_CustomInputProxy::ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton )
{
	// we need to override the default behavior of these button clicks to allow the assignment of a
	// texture or material to a UIImage through a UITextureBase to appear transparent to the user.
	switch ( InButton )
	{
	case PB_Browse:
		if ( GUnrealEd )
		{
			WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
			if ( GenericBrowser )
			{
				TArray<UObject*> CurrentImages;

				// Get a list of addresses for objects handled by the property window.
				TArray<BYTE*> Addresses;
				verify( InItem->GetReadAddress( InItem, InItem->bSingleSelectOnly, Addresses, FALSE ) );

				// translate the raw addresses into UITexture pointers and retrieve the underlying USurface that's assigned to each one
				for ( INT AddressIndex = 0; AddressIndex < Addresses.Num(); AddressIndex++ )
				{
					BYTE* PropertyAddress = Addresses(AddressIndex);

					UPropertyValue PropertyValue;
					if ( InItem->Property->GetPropertyValue(PropertyAddress, PropertyValue) )
					{
						UUITexture* TextureObject = Cast<UUITexture>(PropertyValue.ObjectValue);
						if ( TextureObject != NULL && TextureObject->ImageTexture != NULL )
						{
							CurrentImages.AddUniqueItem(TextureObject->ImageTexture);
						}
					}
				}

				// Sync the generic browser to the object(s) specified by the given property.
				if ( CurrentImages.Num() )
				{
					// Make sure the window is visible.  The window needs to be visible *before*
					// the browser is sync'd to objects so that redraws actually happen.
					GUnrealEd->GetBrowserManager()->ShowWindow( GenericBrowser->GetDockID(), TRUE );

					// Sync.
					GenericBrowser->SyncToObjects( CurrentImages );
				}
			}
		}
		break;

	case PB_Clear:
		{
			// Just clear the TextCtrl and counting on SendToObjects() doesn't work, because
			// SendToObjects() won't send the empty string to a multi selection.
			// Intead, we must manually forward the text to each property.
			TextCtrl->SetValue( *LocalizeUnrealEd(TEXT("None")) );

			UPropertyValue PropertyValue;
			PropertyValue.ObjectValue = NULL;

			WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
			for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
			{
				UObject* CurrentObject = *Itor;
				ICustomPropertyItemHandler* IHandler = static_cast<ICustomPropertyItemHandler*>(CurrentObject->GetInterfaceAddress(UCustomPropertyItemHandler::StaticClass()));
				if ( IHandler != NULL )
				{
					if ( !IHandler->IsCustomPropertyValueIdentical(InItem->Property, PropertyValue, InItem->ArrayIndex) )
					{
						FScopedPropertyChange PropertyChangeNotification(InItem);

						// change the value of this property
						IHandler->EditorSetPropertyValue(InItem->Property, PropertyValue, InItem->ArrayIndex);
					}
				}
			}
		}
		break;

	case PB_Use:
		{
			USelection* SelectedSet = GEditor->GetSelectedSet( USurface::StaticClass() );

			// If an object of that type is selected, use it.
			USurface* SelectedObject = SelectedSet->GetTop<USurface>();
			if( SelectedObject )
			{
				UPropertyValue PropertyValue;
				PropertyValue.ObjectValue = SelectedObject;

				TextCtrl->SetValue( *SelectedObject->GetPathName() );

				WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
				for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
				{
					UObject* CurrentObject = *Itor;
					ICustomPropertyItemHandler* IHandler = static_cast<ICustomPropertyItemHandler*>(CurrentObject->GetInterfaceAddress(UCustomPropertyItemHandler::StaticClass()));
					if ( IHandler != NULL )
					{
						if ( !IHandler->IsCustomPropertyValueIdentical(InItem->Property, PropertyValue, InItem->ArrayIndex) )
						{
							FScopedPropertyChange PropertyChangeNotification(InItem);

							// change the value of this property
							IHandler->EditorSetPropertyValue(InItem->Property, PropertyValue, InItem->ArrayIndex);
						}
					}
				}
			}
		}
		break;

	default:
		Super::ButtonClick(InItem, InButton);
		break;
	}
}

void UUITexture_CustomInputProxy::SendToObjects( WxPropertyWindow_Base* InItem )
{
	// the default behavior of this method would call ExportText to get the value of the UITexture assigned for the value
	// of UIImage.Image, which we don't want.  Get the name of the underlying texture or material assigned to this UIImage and
	// display that instead
	if( TextCtrl == NULL )
	{
		return;
	}

	FString Value = TextCtrl->GetValue().c_str();
	USurface* Surface = FindObject<USurface>( ANY_PACKAGE, *Value );

	// If more than one object is selected, an empty field indicates their values for this property differ.
	// Don't send it to the objects value in this case (if we did, they would all get set to None which isn't good).
	WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();

	if( ObjectNode->GetNumObjects() == 1 || Value.Len() )
	{
		UPropertyValue PropertyValue;
		PropertyValue.ObjectValue = Surface;

		// Build up a list of objects to modify.
		for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
		{
			UObject* CurrentObject = *Itor;
			ICustomPropertyItemHandler* IHandler = static_cast<ICustomPropertyItemHandler*>(CurrentObject->GetInterfaceAddress(UCustomPropertyItemHandler::StaticClass()));
			if ( IHandler != NULL )
			{
				if ( !IHandler->IsCustomPropertyValueIdentical(InItem->Property, PropertyValue, InItem->ArrayIndex) )
				{
					FScopedPropertyChange PropertyChangeNotification(InItem, NULL, TRUE);

					// change the value of this property
					IHandler->EditorSetPropertyValue(InItem->Property, PropertyValue, InItem->ArrayIndex);
				}
			}
		}
	}
}

void UUITexture_CustomInputProxy::RefreshControlValue( UProperty* InProperty, BYTE* InData )
{
	// the default behavior of this method would call ExportText to get the value of the UITexture assigned for the value
	// of UIImage.Image, which we don't want.  Get the name of the underlying texture or material assigned to this UIImage and
	// display that instead
	FString CurrentValueString = *LocalizeUnrealEd(TEXT("None"));
	UUITexture* ImageWrapper = *(UUITexture**)InData;
	if ( ImageWrapper != NULL )
	{
		USurface* CurrentValue = ImageWrapper->GetSurface();
		if ( CurrentValue != NULL )
		{
			CurrentValueString = CurrentValue->GetPathName();
		}
	}

	TextCtrl->SetValue( *CurrentValueString );
}

/* ==========================================================================================================
	UPlayerInputMask_CustomInputProxy
========================================================================================================== */
/**
 * Determines if this input proxy supports the specified UProperty.
 *
 * @param	InItem			The property window node.
 * @param	InArrayIdx		The index in the case of array properties.
 *
 * @return					TRUE if the draw proxy supports the specified property.
 */
UBOOL UPlayerInputMask_CustomInputProxy::Supports( const WxPropertyWindow_Base* InItem, INT ArrayIdx ) const
{
	UBOOL bResult = FALSE;

	// this draw proxy is only valid for the UIScreenObject::PlayerInputMask property
	if ( InItem->Property != NULL )
	{
		if ( InItem->Property->GetOwnerClass() == UUIObject::StaticClass() && InItem->Property->GetFName() == TEXT("PlayerInputMask") )
		{
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Allows special actions on left mouse clicks.  Return TRUE to stop normal processing.
 *
 * @param	InItem			The property window node.
 * @param	InX				The cursor X position associated with the click event.
 * @param	InY				The cursor Y position associated with the click event.
 */
UBOOL UPlayerInputMask_CustomInputProxy::LeftClick( WxPropertyWindow_Base* InItem, INT InX, INT InY )
{
	UBOOL bResult = FALSE;
	if ( InItem->GetPropertyWindow() && InItem->GetPropertyWindow()->IsReadOnly() )
	{
		bResult = TRUE;
	}
	else
	{
		// If clicking on top of the checkbox graphic, toggle it's value and return.
		InX -= InItem->GetPropertyWindow()->GetSplitterPos();

		const int MaxWidth = (UCONST_MAX_SUPPORTED_GAMEPADS + 1) * MASKCHECKBOX_WIDTH;
		if( InX > 0 && InX < MaxWidth )
		{
			// get the current value, prior to receiving this click
			FString PreviousValue = InItem->GetPropertyText();
			INT CurrentValue = appAtoi(*PreviousValue) >> 4;

			// figure out which gamepad's checkbox was actually clicked
			INT ClickedIndex = InX / MASKCHECKBOX_WIDTH;
			if ( ClickedIndex <= UCONST_MAX_SUPPORTED_GAMEPADS )
			{
				// if the current value for this gamepad is "off", turn it on and vice versa
				UBOOL bShouldBeEnabled;
				if ( ClickedIndex < UCONST_MAX_SUPPORTED_GAMEPADS )
				{
					bShouldBeEnabled = (CurrentValue&(1<<ClickedIndex)) == 0;
				}
				else
				{
					bShouldBeEnabled = CurrentValue != 0;
				}

				FScopedPropertyChange PropertyChangeNotification(InItem,NULL,TRUE);

				TArray<FObjectBaseAddress> ObjectsToModify;
				WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();
				for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
				{
					UUIObject* Widget = Cast<UUIObject>(*Itor);
					if ( Widget != NULL )
					{
						const UBOOL bApplyToChildren = TRUE;//Cast<UUIObject>(Widget) != NULL;
						if ( bShouldBeEnabled )
						{
							if ( ClickedIndex < UCONST_MAX_SUPPORTED_GAMEPADS )
							{
								Widget->eventEnablePlayerInput(ClickedIndex, bApplyToChildren);
							}
							else
							{
								Widget->SetInputMask(0, bApplyToChildren, TRUE);
							}
						}
						else
						{
							if ( ClickedIndex < UCONST_MAX_SUPPORTED_GAMEPADS )
							{
								Widget->eventDisablePlayerInput(ClickedIndex, bApplyToChildren);
							}
							else
							{
								Widget->SetInputMask(1, bApplyToChildren, TRUE);
							}
						}
					}
				}

				InItem->Refresh();
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/* ==========================================================================================================
	WxCustomPropertyPanel
========================================================================================================== */
BEGIN_EVENT_TABLE(WxCustomPropertyPanel, wxPanel)
	EVT_CHAR(WxCustomPropertyPanel::OnChar)
END_EVENT_TABLE()

WxCustomPropertyPanel::WxCustomPropertyPanel(WxPropertyWindow_Base* InParent) : 
wxPanel(InParent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER),
Parent(InParent)
{
	
}

/** Routes some keys back to the parent so it can handle focus events. */
void WxCustomPropertyPanel::OnChar(wxKeyEvent& Event)
{
	switch(Event.GetKeyCode())
	{
	case WXK_TAB:case WXK_UP:case WXK_DOWN:
		Event.SetEventObject(Parent);
		Parent->AddPendingEvent(Event);
		break;

	default:
		Event.Skip();
	}
}
/* ==========================================================================================================
UCustomInputProxy_Base
========================================================================================================== */
/** 
 * @return Returns the height the input proxy requires to draw all of its controls.
 */
INT UCustomInputProxy_Base::GetProxyHeight() const
{
	INT ProxyHeight;
	if(PropertyPanel != NULL && PropertyPanel->GetSizer() != NULL)
	{
		ProxyHeight = PropertyPanel->GetSizer()->GetMinSize().GetHeight() + PROP_DefaultItemHeight;
	}
	else
	{
		ProxyHeight = Super::GetProxyHeight();
	}

	return ProxyHeight;
}

/** 
 * Allows the created controls to react to the parent window being resized.  Derived classes
 * that implement this method should to call up to their parent.
 *
 * @param InItem		Property item that owns this proxy.
 * @param InBaseClass	UObject class that owns the item.
 * @param InRC			Drawing region for the controls.
 * @param InValue		Starting value for the property.
 */
void UCustomInputProxy_Base::CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue )
{
	PropertyPanel = new WxCustomPropertyPanel(InItem);

	// Create any buttons for the control.
	Super::CreateControls( InItem, InBaseClass, InRC, InValue );
}

/**
 * Deletes any controls which were created for editing.  Derived classes
 * that implement this method should to call up to their parent.
 */
void UCustomInputProxy_Base::DeleteControls()
{
	if( PropertyPanel )
	{
		PropertyPanel->Destroy();
		PropertyPanel = NULL;
	}

	// Delete any buttons on the control.
	Super::DeleteControls();
}

/**
 * Allows the created controls to react to the parent window being resized.
 * @param InItem		Property item that owns this proxy.
 * @param InRC			Drawing region for the controls.
 */
void UCustomInputProxy_Base::ParentResized( WxPropertyWindow_Base* InItem, const wxRect& InRC )
{
	Super::ParentResized( InItem, InRC );

	if( PropertyPanel )
	{
		PropertyPanel->SetSize( InItem->IndentX, InRC.y + PROP_DefaultItemHeight, InRC.GetWidth() + (InRC.x - InItem->IndentX), InRC.GetHeight() - PROP_DefaultItemHeight );
		PropertyPanel->Layout();
	}
}


//@todo: Didn't like the way this was turning out so commented it out for now, but it is a good example on how to create a custom property so I will probably refer to it later. - ASM 3/23/2006
#if 0
IMPLEMENT_CLASS(UCustomInputProxy_UIScreenObjectPosition);

/**
 * Custom input proxy for UI widget position properties.
 */
class UCustomInputProxy_UIScreenObjectPosition : public UCustomInputProxy_Base
{
	DECLARE_CLASS(UCustomInputProxy_UIScreenObjectPosition,UCustomInputProxy_Base,0,UnrealEd);

public:	
	/**
	 * Determines if this input proxy supports the specified UProperty.
	 *
	 * @param	InItem			The property window node.
	 * @param	InArrayIdx		The index in the case of array properties.
	 *
	 * @return					TRUE if the draw proxy supports the specified property.
	 */
	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;

	/** 
	 * Allows the created controls to react to the parent window being resized.  Derived classes
	 * that implement this method should to call up to their parent.
	 *
	 * @param InItem		Property item that owns this proxy.
	 * @param InBaseClass	UObject class that owns the item.
	 * @param InRC			Drawing region for the controls.
	 * @param InValue		Starting value for the property.
	 */
	virtual void CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue );

	/**
	 * Sends the current value in the input control to the selected objects.
	 *
	 * @param	InItem		The property window node containing the property which will receive the new value.
	 */
	virtual void SendToObjects( WxPropertyWindow_Base* InItem );

	/**
	 * Updates the control value with the property value.
	 *
	 * Our version ignores the parameters passed in and uses the data from the property item.
	 *
	 * @param	InProperty		The property from which to export.
	 * @param	InData			The parent object read address.
	 */
	virtual void RefreshControlValue( UProperty* InProperty, BYTE* InData );

private:

	/**
	 * Updates the control value with the property value.
	 */
	void RefreshControlValue();

	/** Property item that we are editing with this proxy. */
	WxPropertyWindow_Base* Item;

	/** Text boxes for each face value. */
	wxTextCtrl* TextValue[UIFACE_MAX];

	/** Combo boxes for each face's scale type. */
	wxComboBox* ComboScale[UIFACE_MAX];
};


	/* ==========================================================================================================
	UCustomInputProxy_UIScreenObjectPosition
	========================================================================================================== */

	UBOOL UCustomInputProxy_UIScreenObjectPosition::Supports( const WxPropertyWindow_Base* InItem, INT ArrayIdx ) const
	{
		UBOOL bResult = FALSE;

		// this input proxy is only valid for properties of type UITexture
		if ( InItem->Property != NULL )
		{
			UStructProperty* StructProp = Cast<UStructProperty>(InItem->Property,CASTCLASS_UStructProperty);
			if ( StructProp != NULL )
			{
				
				if ( StructProp->GetOwnerClass()->IsChildOf(UUIScreenObject::StaticClass()) )
				{
					bResult = TRUE;
				}
			}
		}

		return bResult;
	}

	/** 
	* Allows the created controls to react to the parent window being resized.  Derived classes
	* that implement this method should to call up to their parent.
	*
	* @param InItem		Property item that owns this proxy.
	* @param InBaseClass	UObject class that owns the item.
	* @param InRC			Drawing region for the controls.
	* @param InValue		Starting value for the property.
	*/
	void UCustomInputProxy_UIScreenObjectPosition::CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue )
	{
		Item = InItem;

		if(InItem != NULL)
		{
			// Create the panel for this control.
			Super::CreateControls( InItem, InBaseClass, InRC, InValue );

			wxSizer* MainSizer = new wxBoxSizer(wxVERTICAL);
			{
				// Create a value/scale pair for each row.
				for(INT FaceIdx = 0; FaceIdx < 4; FaceIdx++)
				{
					FString FaceNameTag = FString::Printf(TEXT("UIEditor_FaceText[%i]"), FaceIdx);
					wxSizer* TopSizer = new wxStaticBoxSizer(wxHORIZONTAL, PropertyPanel, *LocalizeUI(*FaceNameTag));
					{
						ComboScale[FaceIdx] = new wxComboBox(PropertyPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
						TopSizer->Add(ComboScale[FaceIdx], 0, wxEXPAND | wxALL, 5);

						TextValue[FaceIdx] = new wxTextCtrl(PropertyPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC) );
						TopSizer->Add(TextValue[FaceIdx], 1, wxEXPAND | wxALL, 5);
					}
					MainSizer->Add(TopSizer, 1, wxEXPAND | wxALL, 5);
				}
			}
			PropertyPanel->SetSizer(MainSizer);
			PropertyPanel->SetAutoLayout(true);

			// Build a list of possible scale type enum values.
			wxArrayString EnumTypes;
			static UEnum* WidgetFaceEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPositionEvalType"), TRUE);
			if ( WidgetFaceEnum != NULL )
			{
				for( INT EnumIdx=0; EnumIdx<WidgetFaceEnum->NumEnums() - 1; EnumIdx++ )
				{
					EnumTypes.Add(*WidgetFaceEnum->GetEnum(EnumIdx));
				}
			}


			for(INT FaceIdx = 0; FaceIdx < UIFACE_MAX; FaceIdx++)
			{
				ComboScale[FaceIdx]->Append(EnumTypes);
			}

			// Set the current value to all of the widgets.
			RefreshControlValue();
		}
	}

	/**
	* Sends the current value in the input control to the selected objects.
	*
	* @param	InItem		The property window node containing the property which will receive the new value.
	*/
	void UCustomInputProxy_UIScreenObjectPosition::SendToObjects( WxPropertyWindow_Base* InItem )
	{
		// We will go through all of the widgets that we have created and send the value of any widgets that have a non-empty value to
		// all of the objects that are selected and attached to this property item.
 		if(InItem != NULL && PropertyPanel != NULL)
		{
			// Get all of the parent UUIScreenObjects so we can set positions properly.
			WxPropertyWindow_Objects* ObjectNode = InItem->FindObjectItemParent();


			// Build up a list of objects to modify.
			UBOOL bObjectsChanged = FALSE;
			TArray<UUIScreenObject*> ObjectsToModify;
			for ( TPropObjectIterator Itor( ObjectNode->ObjectIterator() ) ; Itor ; ++Itor )
			{
				UObject*	Object = *Itor;

				UUIScreenObject* Widget = Cast<UUIScreenObject>(Object);

				if(Object != NULL)
				{
					ObjectsToModify.AddItem( Widget );
				}
			}


			TArray<BYTE*> Addresses;
			if(InItem->GetReadAddress(InItem, Addresses) && Addresses.Num() == ObjectsToModify.Num())
			{
				for(INT FaceIdx = 0; FaceIdx < UIFACE_MAX; FaceIdx++)
				{
					wxString Text;

					// Text Value
					Text = TextValue[FaceIdx]->GetValue();
					if(Text.Length())
					{
						bObjectsChanged = TRUE;
						FLOAT Value = appAtof(Text);

						for(INT AddressIdx = 0; AddressIdx < Addresses.Num(); AddressIdx++)
						{
							UUIScreenObject* Widget = ObjectsToModify(AddressIdx);
							FUIScreenValue_Bounds* Bounds = (FUIScreenValue_Bounds*)Addresses(AddressIdx);
							
							Widget->SetPosition(Value, (EUIWidgetFace)FaceIdx, (EPositionEvalType)Bounds->ScaleType[FaceIdx]);
						}
					}


					// Scale Combo
					Text = ComboScale[FaceIdx]->GetValue();

					if(Text.Length())
					{
						INT Scale = ComboScale[FaceIdx]->GetSelection();

						for(INT AddressIdx = 0; AddressIdx < Addresses.Num(); AddressIdx++)
						{
							UUIScreenObject* Widget = ObjectsToModify(AddressIdx);
							FUIScreenValue_Bounds* Bounds = (FUIScreenValue_Bounds*)Addresses(AddressIdx);
							Widget->SetPosition(Bounds->Value[FaceIdx], (EUIWidgetFace)FaceIdx, (EPositionEvalType)Scale);
						}
					}
				}
			}

			if(bObjectsChanged == TRUE)
			{
				for(INT )
			}
		}
	}

	/**
	* Updates the control value with the property value.
	*
	* Our version ignores the parameters passed in and uses the data from the property item.
	*
	* @param	InProperty		The property from which to export.
	* @param	InData			The parent object read address.
	*/
	void UCustomInputProxy_UIScreenObjectPosition::RefreshControlValue( UProperty* InProperty, BYTE* InData )
	{
		RefreshControlValue();
	}

	/**
	* Updates the control value with the property value.
	*/
	void UCustomInputProxy_UIScreenObjectPosition::RefreshControlValue()
	{
		if(Item != NULL)
		{
			// Read values for each of the widgets and diff them, if we have any items in our struct that have differing
			// values, then we blank out the text for that widget.
			TArray<BYTE*> Addresses;
			if(Item->GetReadAddress(Item, Addresses))
			{
				for(INT FaceIdx = 0; FaceIdx < UIFACE_MAX; FaceIdx++)
				{
					FUIScreenValue_Bounds* Bounds = (FUIScreenValue_Bounds*)Addresses(0);
					UBOOL bSameScale = TRUE;
					UBOOL bSameValue = TRUE;
					FLOAT MatchValue = Bounds->Value[FaceIdx];
					INT MatchScale = Bounds->ScaleType[FaceIdx];


					for(INT AddressIdx = 1; AddressIdx < Addresses.Num(); AddressIdx++)
					{
						Bounds = (FUIScreenValue_Bounds*)Addresses(AddressIdx);

						if(bSameValue && Bounds->Value[FaceIdx] != MatchValue)
						{
							bSameValue = FALSE;
						}

						if(bSameScale && Bounds->ScaleType[FaceIdx] != MatchScale)
						{
							bSameScale = FALSE;
						}
					}

					if(bSameValue == TRUE)
					{
						TextValue[FaceIdx]->SetValue( *FString::Printf(TEXT("%f"), MatchValue) );
					}
					else
					{
						TextValue[FaceIdx]->SetValue( wxEmptyString );
					}

					if(bSameScale == TRUE)
					{
						ComboScale[FaceIdx]->SetSelection( MatchScale );
					}
					else
					{
						ComboScale[FaceIdx]->SetValue( wxEmptyString );
					}

				}
			}
		}
	}
#endif


