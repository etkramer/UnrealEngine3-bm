/*=============================================================================
	UnUIEditorProperties.h: UI editor custom property window declarations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNUIEDITORPROPERTIES_H__
#define __UNUIEDITORPROPERTIES_H__

#include "Properties.h"
#include "PropertyUtils.h"

/**
 * The purpose of this custom property editing control is to provide a method for linking the editability of one property to the value
 * of another property.  Only when the value of the other (known as the "conditional") property matches the required value will this
 * property window item be editable.
 * <p>
 * Currently, only bool properties can be used as the conditional property, and the conditional property must be contained within the same
 * struct as this item's property.
 * <p>
 * This property editing control renders an additional checkbox just to the left of the property name, which is used to control the value
 * of the conditional property.  When this checkbox is disabled, the text for this property editing control is greyed out.
 * <p>
 * It is registered by adding an entry to the CustomPropertyClasses array of the CustomPropertyItemBindings section of the Editor .ini.  Conditionals
 * are specified using property metadata in the format <EditCondition=ConditionalPropertyName>.
 */
class WxCustomPropertyItem_ConditionalItem : public WxPropertyWindow_Item
{
public:
	DECLARE_DYNAMIC_CLASS(WxCustomPropertyItem_ConditionalItem);

	/**
	 * Returns the property used for determining whether this item's Property can be edited.
	 *
	 * @todo ronp - eventually, allow any type of property by specifying required value in metadata
	 */
	UBoolProperty* GetEditConditionProperty() const;

	/**
	 * Finds the property being used to determine whether this item's associated property should be editable/expandable.
	 * If the propery is successfully found, ConditionPropertyAddresses will be filled with the addresses of the conditional properties' values.
	 *
	 * @param	ConditionProperty	receives the value of the property being used to control this item's editability.
	 * @param	ConditionPropertyAddresses	receives the addresses of the actual property values for the conditional property
	 *
	 * @return	TRUE if both the conditional property and property value addresses were successfully determined.
	 */
	UBOOL GetEditConditionPropertyAddress( UBoolProperty*& ConditionProperty, TArray<BYTE*>& ConditionPropertyAddresses );

	/**
	 * Returns TRUE if MouseX and MouseY fall within the bounding region of the checkbox used for displaying the value of this property's edit condition.
	 */
	UBOOL ClickedCheckbox( INT MouseX, INT MouseY ) const;

	/**
	 * Toggles the value of the property being used as the condition for editing this property.
	 *
	 * @return	the new value of the condition (i.e. TRUE if the condition is now TRUE)
	 */
	virtual UBOOL ToggleConditionValue();

	/**
	 * Returns TRUE if the value of the conditional property matches the value required.  Indicates whether editing or otherwise interacting with this item's
	 * associated property should be allowed.
	 */
	virtual UBOOL IsConditionMet();

	/**
	 * Renders the left side of the property window item.
	 *
	 * This version is responsible for rendering the checkbox used for toggling whether this property item window should be enabled.
	 *
	 * @param	RenderDeviceContext		the device context to use for rendering the item name
	 * @param	ClientRect				the bounding region of the property window item
	 */
	virtual void RenderItemName( wxBufferedPaintDC& RenderDeviceContext, const wxRect& ClientRect );

	/**
	 * Expands child items.  Does nothing if the window is already expanded.
	 */
	virtual void Expand();

	/**
	 * Recursively searches through children for a property named PropertyName and expands it.
	 * If it's a UArrayProperty, the propery's ArrayIndex'th item is also expanded.
	 */
	virtual void ExpandItem( const FString& PropertyName, INT ArrayIndex );

	/**
	 * Expands all items rooted at the property window node.
	 */
	virtual void ExpandAllItems();

	/**
	 * Expands all category items belonging to this window.
	 */
	virtual void ExpandCategories();

protected:
	/**
	 * Called when an property window item receives a left-mouse-button press which wasn't handled by the input proxy.  Typical response is to gain focus
	 * and (if the property window item is expandable) to toggle expansion state.
	 *
	 * @param	Event	the mouse click input that generated the event
	 *
	 * @return	TRUE if this property window item should gain focus as a result of this mouse input event.
	 */
	virtual UBOOL ClickedPropertyItem( wxMouseEvent& In );
};

/**
 * This custom property editing control is used for the UIRoot.ScreenPositionRange struct
 * The purpose of this custom editing control is to present the ScreenPositionRange's properties in a friendlier and more readable
 * manner than they would be displayed using the default property editing controls used for editing properties of structs.
 *
 * It is registered by adding an entry to the CustomPropertyClasses array of the CustomPropertyItemBindings section of the Editor .ini.
 */
// class WxCustomPropertyItem_ScreenPositionRange : public WxPropertyWindow_Item
// {
// public:
// 	DECLARE_DYNAMIC_CLASS(WxCustomPropertyItem_ScreenPositionRange);
// 
// 	/**
// 	 * Initialize this property window.  Must be the first function called after creating the window.
// 	 */
// 	virtual void Create(	wxWindow* InParent,
// 							WxPropertyWindow_Base* InParentItem,
// 							WxPropertyWindow* InTopPropertyWindow,
// 							UProperty* InProperty,
// 							INT InPropertyOffset,
// 							INT InArrayIdx,
// 							UBOOL bInSupportsCustomControls=FALSE);
// 
// 	void OnPaint( wxPaintEvent& Event );
// 	void OnInputTextEnter( wxCommandEvent& Event );
// 	void OnInputTextChanged( wxCommandEvent& Event );
// 	void OnInputComboBox( wxCommandEvent& Event );
// 
// 	DECLARE_EVENT_TABLE();
// 
// protected:
// 	virtual void CreateChildItems();
// };

class UUITexture_CustomDrawProxy : public UPropertyDrawProxy
{
	DECLARE_CLASS(UUITexture_CustomDrawProxy,UPropertyDrawProxy,0,UnrealEd);

public:
	/**
	 * Determines if this draw proxy supports the specified UProperty.
	 *
	 * @param	InItem			The property window node.
	 * @param	InArrayIdx		The index in the case of array properties.
	 *
	 * @return					TRUE if the draw proxy supports the specified property.
	 */
	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;

	/**
	 * Draws the value of InProperty inside of InRect.
	 *
	 * @param	InDC			The wx device context.
	 * @param	InRect			The rect to draw the value into.
	 * @param	InReadAddress	The address of the property value.
	 * @param	InProperty		The property itself.
	 * @param	InInputProxy	The input proxy for the property.
	 */
	virtual void Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy );
};

/**
 * Custom input proxy for UIState properties in UIScreenObject and derived classes.  Filters out UIStates which don't support the widget class.
 */
class UUIState_CustomInputProxy : public UPropertyInputEditInline
{
	DECLARE_CLASS(UUIState_CustomInputProxy,UPropertyInputEditInline,0,UnrealEd);

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
	 * Wrapper method for determining whether a class is valid for use by this property item input proxy.
	 *
	 * @param	InItem			the property window item that contains this proxy.
	 * @param	CheckClass		the class to verify
	 * @param	bAllowAbstract	TRUE if abstract classes are allowed
	 *
	 * @return	TRUE if CheckClass is valid to be used by this input proxy
	 */
	virtual UBOOL IsClassAllowed( WxPropertyWindow_Base* InItem,  UClass* CheckClass, UBOOL bAllowAbstract ) const;
};


/**
 * Custom input proxy for UIState properties in UIScreenObject and derived classes.  Filters out UIStates which don't support the widget class.
 */
class UUIStateClass_CustomInputProxy : public UPropertyInputCombo
{
	DECLARE_CLASS(UUIStateClass_CustomInputProxy,UPropertyInputCombo,0,UnrealEd);

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
	 * Wrapper method for determining whether a class is valid for use by this property item input proxy.
	 *
	 * @param	InItem			the property window item that contains this proxy.
	 * @param	CheckClass		the class to verify
	 * @param	bAllowAbstract	TRUE if abstract classes are allowed
	 *
	 * @return	TRUE if CheckClass is valid to be used by this input proxy
	 */
	virtual UBOOL IsClassAllowed( WxPropertyWindow_Base* InItem,  UClass* CheckClass, UBOOL bAllowAbstract ) const;
};

class UUITexture_CustomInputProxy : public UPropertyInputText
{
	DECLARE_CLASS(UUITexture_CustomInputProxy,UPropertyInputText,0,UnrealEd);

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

	virtual void CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue );
	virtual void ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton );
	virtual void SendToObjects( WxPropertyWindow_Base* InItem );
	virtual void RefreshControlValue( UProperty* InProperty, BYTE* InData );
};

class UPlayerInputMask_CustomDrawProxy : public UPropertyDrawProxy
{
	DECLARE_CLASS(UPlayerInputMask_CustomDrawProxy,UPropertyDrawProxy,0,UnrealEd);

public:
	/**
	 * Determines if this draw proxy supports the specified UProperty.
	 *
	 * @param	InItem			The property window node.
	 * @param	InArrayIdx		The index in the case of array properties.
	 *
	 * @return					TRUE if the draw proxy supports the specified property.
	 */
	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;

	/**
	 * Draws the value of InProperty inside of InRect.
	 *
	 * @param	InDC			The wx device context.
	 * @param	InRect			The rect to draw the value into.
	 * @param	InReadAddress	The address of the property value.
	 * @param	InProperty		The property itself.
	 * @param	InInputProxy	The input proxy for the property.
	 */
	virtual void Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy );

	/**
	 * Draws a representation of an unknown value (i.e. when multiple objects have different values in the same field).
	 *
	 * @param	InDC			The wx device context.
	 * @param	InRect			The rect to draw the value into.
	 */
	virtual void DrawUnknown( wxDC* InDC, wxRect InRect );
};

class UPlayerInputMask_CustomInputProxy : public UPropertyInputBool
{
	DECLARE_CLASS(UPlayerInputMask_CustomInputProxy,UPropertyInputBool,0,UnrealEd);

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
	 * Allows special actions on left mouse clicks.  Return TRUE to stop normal processing.
	 *
	 * @param	InItem			The property window node.
	 * @param	InX				The cursor X position associated with the click event.
	 * @param	InY				The cursor Y position associated with the click event.
	 */
	virtual UBOOL LeftClick( WxPropertyWindow_Base* InItem, INT InX, INT InY );

// 	virtual void SendToObjects( WxPropertyWindow_Base* InItem );
// 	virtual void RefreshControlValue( UProperty* InProperty, BYTE* InData );
};

/** Custom panel class that passes certain events back to the parent. */
class WxCustomPropertyPanel : public wxPanel
{
public:
	WxCustomPropertyPanel(WxPropertyWindow_Base* InParent);

private:

	/** Routes some keys back to the parent so it can handle focus events. */
	void OnChar(wxKeyEvent& Event);

	/** Pointer to the parent item for this panel. */
	WxPropertyWindow_Base* Parent;

	DECLARE_EVENT_TABLE()
};


/**
 * Custom input proxy base class.  This class implements the functions needed to use a wx panel as a input proxy.
 */
class UCustomInputProxy_Base : public UPropertyInputProxy
{
	DECLARE_CLASS(UCustomInputProxy_Base,UPropertyInputProxy,0,UnrealEd);

	/**
	 * @return Controls whether or not the parent property can expand.
	 */
	virtual UBOOL LetPropertyExpand() const
	{
		return FALSE;
	}

	/** 
	 * @return Returns the height the input proxy requires to draw all of its controls.
	 */
	virtual INT GetProxyHeight() const;

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
	 * Deletes any controls which were created for editing.  Derived classes
	 * that implement this method should to call up to their parent.
	 */
	virtual void DeleteControls();

	/**
	 * Allows the created controls to react to the parent window being resized.
	 * @param InItem		Property item that owns this proxy.
	 * @param InRC			Drawing region for the controls.
	 */
	void ParentResized( WxPropertyWindow_Base* InItem, const wxRect& InRC );

protected:

	/** Main panel for the property. */
	wxPanel* PropertyPanel;

};


#endif // __UNUIEDITORPROPERTIES_H__

