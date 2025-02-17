/*=============================================================================
	PropertyUtils.cpp: Editor property window edit and draw code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __PROPERTYUTILS_H__
#define __PROPERTYUTILS_H__

// Forward declarations
class UPropertyInputProxy;
class WxNumericPropertySpinner;
class WxPropertyWindow_Base;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property drawing
//
///////////////////////////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
	UPropertyDrawProxy
-----------------------------------------------------------------------------*/

/**
 * Allows customized drawing for properties.
 */
class UPropertyDrawProxy : public UObject
{
	DECLARE_CLASS(UPropertyDrawProxy,UObject,CLASS_Intrinsic,UnrealEd)

	/**
	 * @return Controls whether or not the parent property can expand.
	 */
	virtual UBOOL LetPropertyExpand() const
	{
		return TRUE;
	}

	/**
	 * @return	Returns the height required by the input proxy.
	 */
	virtual INT GetProxyHeight() const;

	/**
	 * Determines if this input proxy supports the specified UProperty.
	 *
	 * @param	InItem			The property window node.
	 * @param	InArrayIdx		The index in the case of array properties.
	 *
	 * @return					TRUE if the draw proxy supports the specified property.
	 */
	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
	{ return FALSE; }

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

/*-----------------------------------------------------------------------------
	UPropertyDrawNumeric
-----------------------------------------------------------------------------*/

class UPropertyDrawNumeric : public UPropertyDrawProxy
{
	DECLARE_CLASS(UPropertyDrawNumeric,UPropertyDrawProxy,0,UnrealEd)

	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy );
};

/*-----------------------------------------------------------------------------
	UPropertyDrawColor
-----------------------------------------------------------------------------*/

class UPropertyDrawColor : public UPropertyDrawProxy
{
	DECLARE_CLASS(UPropertyDrawColor,UPropertyDrawProxy,0,UnrealEd)

	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy );
};

/*-----------------------------------------------------------------------------
	UPropertyDrawRotation
-----------------------------------------------------------------------------*/

class UPropertyDrawRotation : public UPropertyDrawProxy
{
	DECLARE_CLASS(UPropertyDrawRotation,UPropertyDrawProxy,0,UnrealEd)

	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy );
};

/*-----------------------------------------------------------------------------
	UPropertyDrawRotationHeader
-----------------------------------------------------------------------------*/

class UPropertyDrawRotationHeader : public UPropertyDrawProxy
{
	DECLARE_CLASS(UPropertyDrawRotationHeader,UPropertyDrawProxy,0,UnrealEd)

	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy );
};

/*-----------------------------------------------------------------------------
	UPropertyDrawBool
-----------------------------------------------------------------------------*/

class UPropertyDrawBool : public UPropertyDrawProxy
{
	DECLARE_CLASS(UPropertyDrawBool,UPropertyDrawProxy,0,UnrealEd)

	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy );
	virtual void DrawUnknown( wxDC* InDC, wxRect InRect );
};

/*-----------------------------------------------------------------------------
	UPropertyDrawArrayHeader
-----------------------------------------------------------------------------*/

class UPropertyDrawArrayHeader : public UPropertyDrawProxy
{
	DECLARE_CLASS(UPropertyDrawArrayHeader,UPropertyDrawProxy,0,UnrealEd)

	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void Draw( wxDC* InDC, wxRect InRect, BYTE* InReadAddress, UProperty* InProperty, UPropertyInputProxy* InInputProxy );
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property input
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * An object/addr pair, passed in bulk into UPropertyInputProxy methods.
 */
class FObjectBaseAddress
{
public:
	FObjectBaseAddress()
		:	Object( NULL )
		,	BaseAddress( NULL )
	{}
	FObjectBaseAddress(UObject* InObject, BYTE* InBaseAddress)
		:	Object( InObject )
		,	BaseAddress( InBaseAddress )
	{}

	UObject*	Object;
	BYTE*		BaseAddress;
};

/*-----------------------------------------------------------------------------
WxPropertyItemButton
-----------------------------------------------------------------------------*/
/**
 * Wraps wxBitmapButton and overrides the erase background event to prevent flickering.
 */
class WxPropertyItemButton : public wxBitmapButton
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyItemButton);

	/**
	 * Constructor.
	 */
	WxPropertyItemButton();
	/**
	 * Constructor.
	 */
	WxPropertyItemButton(wxWindow *parent,
		wxWindowID id,
		const wxBitmap& bitmap,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxBU_AUTODRAW,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = wxButtonNameStr);

	/** Overloads the erase background event to prevent flickering */
	virtual void OnEraseBackground(wxEraseEvent &Event);

	DECLARE_EVENT_TABLE();
};

/*-----------------------------------------------------------------------------
	UPropertyInputProxy
-----------------------------------------------------------------------------*/

/**
 * Handles user input for a property value.
 */
class UPropertyInputProxy : public UObject
{
	DECLARE_CLASS(UPropertyInputProxy,UObject,CLASS_Intrinsic,UnrealEd)

	UPropertyInputProxy()
		:	ComboBox( NULL )
	{}

	/** Property button enums. */
	enum ePropButton
	{
		PB_Add,
		PB_Empty,
		PB_Insert,
		PB_Delete,
		PB_Browse,
		PB_Pick,
		PB_Clear,
		PB_Find,
		PB_Use,
		PB_NewObject,
		PB_Duplicate,
	};

protected:
	/** Any little buttons that appear on the right side of a property window item. */
	TArray<WxPropertyItemButton*> Buttons;

	/** Drop-down combo box used for editing certain properties (eg enums). */
	wxComboBox* ComboBox;

private:
	//////////////////////////////////////////////////////////////////////////
	// Splitter

	/**
	 * Adds a button to the right side of the item window.
	 *
	 * @param	InItem			The parent node of the new button.
	 * @param	InButton		Specifies the type of button to add.
	 * @param	InRC			The rect to place the button in.
	 */
	void AddButton( WxPropertyWindow_Base* InItem, ePropButton InButton, const wxRect& InRC );

	/**
	* Creates any buttons that the property needs.  Empties out all buttons if no property is bound
	* or if the property is const.
	*
	* @param	InItem			The parent node of the new button.
	* @param	InRC			The rect to place the button in.
	*/
	void CreateButtons( WxPropertyWindow_Base* InItem, const wxRect& InRC );

	/**
	* Deletes any buttons associated with the property window item.
	*/
	void DeleteButtons();

public:

	/**
	 * @return	Returns the height required by the input proxy.
	 */
	virtual INT GetProxyHeight() const;

	/**
	 * @return Controls whether or not the parent property can expand.
	 */
	virtual UBOOL LetPropertyExpand() const
	{
		return TRUE;
	}

	/**
	 * @return		The number of buttons currently active on this control.
	 */
	INT GetNumButtons() const		{ return Buttons.Num(); }

	/**
	 * Creates any controls that are needed to edit the property.  Derived classes that implement
	 * this method should call up to their parent.
	 *
	 * @param	InItem			The parent node of the new controls.
	 * @param	InBaseClass		The leafest common base class of the objects being edited.
	 * @param	InRC			The rect to place the controls in.
	 * @param	InValue			The current property value.
	 */
	virtual void CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue )
	{
		CreateButtons( InItem, InRC );
	}

	/**
	 * Deletes any controls which were created for editing.  Baseclass behaviour is to delete
	 * all buttons.  Derived classes that implement this method should call up to their parent.
	 */
	virtual void DeleteControls()
	{
		DeleteButtons();
	}

	/**
	 * Determines if this input proxy supports the specified UProperty.
	 *
	 * @param	InItem			The property window node.
	 * @param	InArrayIdx		The index in the case of array properties.
	 *
	 * @return					TRUE if the input proxy supports the property.
	 */
	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const
	{
		check( 0 );
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// Event handlers

	/**
	 * Allows a customized response to the set focus event of the property item.
	 * @param	InItem			The property window node.
	 */
	virtual void OnSetFocus(WxPropertyWindow_Base* InItem) {}

	/**
	* Allows a customized response to the kill focus event of the property item.
	* @param	InItem			The property window node.
	*/
	virtual void OnKillFocus(WxPropertyWindow_Base* InItem) {}

	/**
	 * Allows a customized response to a user double click on the item in the property window.
	 *
	 * @param	InItem			The property window node.
	 * @param	InObjects		A collection of objects and addresses from which to export a value if bForceValue is FALSE.
	 * @param	InValue			If bForceValue is TRUE, specifies the new property value.
	 * @param	bForceValue		If TRUE, use the value specified with InValue.  Otherwise, export from the property at InReadAddress.
	 */
	virtual void DoubleClick(WxPropertyWindow_Base* InItem, const TArray<FObjectBaseAddress>& InObjects, const TCHAR* InValue, UBOOL bForceValue)
	{}

	/**
	 * Allows special actions on left mouse clicks.  Return TRUE to stop normal processing.
	 *
	 * @param	InItem			The property window node.
	 * @param	InX				The cursor X position associated with the click event.
	 * @param	InY				The cursor Y position associated with the click event.
	 */
	virtual UBOOL LeftClick( WxPropertyWindow_Base* InItem, INT InX, INT InY )
	{ return 0; }

	/**
	 * Allows special actions on right mouse clicks.  Return TRUE to stop normal processing.
	 *
	 * @param	InItem			The property window node.
	 * @param	InX				The cursor X position associated with the click event.
	 * @param	InY				The cursor Y position associated with the click event.
	 */
	virtual UBOOL RightClick( WxPropertyWindow_Base* InItem, INT InX, INT InY )
	{ return 0; }

	/**
	 * Sends the current value in the input control to the selected objects.
	 *
	 * @param	InItem		The property window node containing the property which will receive the new value.
	 */
	virtual void SendToObjects( WxPropertyWindow_Base* InItem )
	{}

	/**
	 * Sends a text string to the selected objects.
	 *
	 * @param	InItem		The property window node containing the property which will receive the new value.
	 * @param	InText		The text value to send.
	 */
	void SendTextToObjects( WxPropertyWindow_Base* InItem, const FString& InText );

	/**
	 * Allows the created controls to react to the parent window being resized.  Baseclass
	 * behaviour is to resize any buttons on the control.  Derived classes that implement
	 * this method should to call up to their parent.
	 *
	 * @param	InItem			The parent node that was resized.
	 * @param	InRC			The window rect.
	 */
	virtual void ParentResized( WxPropertyWindow_Base* InItem, const wxRect& InRC );

	/**
	 * Handles button clicks from within the item.
	 *
	 * @param	InItem			The parent node that received the click.
	 * @param	InButton		The type of button that was clicked.
	 */
	virtual void ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton )
	{}

	/**
	 * Updates the control value with the property value.
	 *
	 * @param	InProperty		The property from which to export.
	 * @param	InData			The parent object read address.
	 */
	virtual void RefreshControlValue( UProperty* InProperty, BYTE* InData )
	{}

	/**
	 * Sends a text value to the property item.  Also handles all notifications of this value changing.
	 */
	void ImportText(WxPropertyWindow_Base* InItem,
					UProperty* InProperty,
					const TCHAR* InBuffer,
					BYTE* InData,
					UObject* InParent);

	void ImportText(WxPropertyWindow_Base* InItem,
					const TArray<FObjectBaseAddress>& InObjects,
					const FString& InValue);

	void ImportText(WxPropertyWindow_Base* InItem,
					const TArray<FObjectBaseAddress>& InObjects,
					const TArray<FString>& InValues);

protected:
	/**
	 * Called when the "Use selected" property item button is clicked.
	 */
	void OnUseSelected(WxPropertyWindow_Base* InItem);
};

/*-----------------------------------------------------------------------------
	UPropertyInputArray
-----------------------------------------------------------------------------*/

class UPropertyInputArray : public UPropertyInputProxy
{
	DECLARE_CLASS(UPropertyInputArray,UPropertyInputProxy,0,UnrealEd)

	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton );
};

/*-----------------------------------------------------------------------------
	UPropertyInputArrayItemBase
-----------------------------------------------------------------------------*/

/**
 * Baseclass for input proxies for types that can exist in an array.
 */
class UPropertyInputArrayItemBase : public UPropertyInputProxy
{
	DECLARE_CLASS(UPropertyInputArrayItemBase,UPropertyInputProxy,0,UnrealEd)

	/**
	 * Implements handling of buttons common to array items.
	 */
	virtual void ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton );

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

/*-----------------------------------------------------------------------------
	UPropertyInputBool
-----------------------------------------------------------------------------*/

class UPropertyInputBool : public UPropertyInputArrayItemBase
{
	DECLARE_CLASS(UPropertyInputBool,UPropertyInputArrayItemBase,0,UnrealEd)

	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void DoubleClick(WxPropertyWindow_Base* InItem, const TArray<FObjectBaseAddress>& InObjects, const TCHAR* InValue, UBOOL bForceValue);
	virtual UBOOL LeftClick( WxPropertyWindow_Base* InItem, INT InX, INT InY );
};

/*-----------------------------------------------------------------------------
	UPropertyInputColor
-----------------------------------------------------------------------------*/

class UPropertyInputColor : public UPropertyInputArrayItemBase
{
	DECLARE_CLASS(UPropertyInputColor,UPropertyInputArrayItemBase,0,UnrealEd)

	/**
	 * Generates a text string based from the color passed in, based on the color type of the property passed in.
	 *
	 * @param InItem	Property that we are basing our string type off of.
	 * @param InColor	Color to use to generate the string.
	 * @param OutString Output for generated string.
	 */
	static void GenerateColorString( const WxPropertyWindow_Base* InItem, const FColor &InColor, FString& OutString );

	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton );
};

/*-----------------------------------------------------------------------------
	UPropertyInputArrayItem
-----------------------------------------------------------------------------*/

class UPropertyInputArrayItem : public UPropertyInputArrayItemBase
{
	DECLARE_CLASS(UPropertyInputArrayItem,UPropertyInputArrayItemBase,0,UnrealEd)

	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton );
};

/*-----------------------------------------------------------------------------
	UPropertyInputText
-----------------------------------------------------------------------------*/

class UPropertyInputText : public UPropertyInputArrayItemBase
{
	/**
	 * This class wraps WxTextCtrl and overrides the erase background event to prevent flickering.
	 */
	class WxPropertyItemTextCtrl : public WxTextCtrl
	{
	public:
		DECLARE_DYNAMIC_CLASS(WxPropertyItemTextCtrl);

		/**
		 * Constructor.
		 */
		WxPropertyItemTextCtrl();
		/**
		 * Constructor.
		 */
		WxPropertyItemTextCtrl( wxWindow* parent,
			wxWindowID id,
			const wxString& value = TEXT(""),
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0 );

		/**
		 * Event handler for the focus event. This function maintains proper selection state within the parent property window.
		 *
		 * @param	Event	Information about the event.
		 */
		virtual void OnSetFocus(wxFocusEvent &Event);

		/**
		 * Event handler for the kill focus event. This function brings the cursor back to index 0 within the text control.
		 *
		 * @param	Event	Information about the event.
		 */
		virtual void OnKillFocus(wxFocusEvent& Event);

		/**
		 * Event handler for the left mouse button down event. This handler is needed because when the text control is clicked and it doesn't have focus it cannot
		 * select all of the text in the control due to the fact that the control thinks it is in the middle of a drag/selection operation. This only applies when
		 * all property item buttons are being rendered because that's the only time when all text control's are created and visible.
		 */
		virtual void OnLeftMouseDown(wxMouseEvent &Event);

		/**
		 * Event handler for the select all text event.
		 *
		 * @param	Event	Information about the event.
		 */
		virtual void OnSelectAllText(wxCommandEvent &Event);

		/**
		 * This event handler sets the default cursor when the mouse cursor enters the text box. This prevents the resize cursor from being permanent until you move the mouse outside of the text box.
		 *
		 * @param	Event	Information about the event.
		 */
		virtual void OnMouseEnter(wxMouseEvent &Event);

		DECLARE_EVENT_TABLE();
	};

	DECLARE_CLASS(UPropertyInputText,UPropertyInputArrayItemBase,0,UnrealEd)

	UPropertyInputText()
		:	TextCtrl( NULL )
	{}

	/**
	 * Clears any selection in the text control.
	 */
	void ClearTextSelection();

	WxPropertyItemTextCtrl* TextCtrl;

protected:
	wxPanel* PropertyPanel;
	wxSizer* PropertySizer;
	

public:
	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue );
	virtual void DeleteControls();
	virtual void SendToObjects( WxPropertyWindow_Base* InItem );
	virtual void ParentResized( WxPropertyWindow_Base* InItem, const wxRect& InRC );
	virtual void ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton );
	virtual void RefreshControlValue( UProperty* InProperty, BYTE* InData );

	/**
	* Allows a customized response to the set focus event of the property item.
	* @param	InItem			The property window node.
	*/
	virtual void OnSetFocus(WxPropertyWindow_Base* InItem);

	/**
	* Allows a customized response to the kill focus event of the property item.
	* @param	InItem			The property window node.
	*/
	virtual void OnKillFocus(WxPropertyWindow_Base* InItem);
};


/*-----------------------------------------------------------------------------
	UPropertyInputNumeric
-----------------------------------------------------------------------------*/

class UPropertyInputNumeric : public UPropertyInputText
{
	DECLARE_CLASS(UPropertyInputNumeric,UPropertyInputText,0,UnrealEd)

	FString Equation;

	virtual void CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue );
	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void SendToObjects( WxPropertyWindow_Base* InItem );
	virtual void RefreshControlValue( UProperty* InProperty, BYTE* InData );

	/**
	 * @return Returns the numeric value of the property.
	 */
	virtual FLOAT GetValue();

	/**
	 * Gets the values for each of the objects that a property item is affecting.
	 * @param	InItem		Property item to get objects from.
	 * @param	Values		Array of FLOATs to store values.
	 */
	virtual void GetValues(WxPropertyWindow_Base* InItem, TArray<FLOAT> &Values);

	/**
	 * Updates the numeric value of the property.
	 * @param	Value	New Value for the property.
	 */
	virtual void SetValue(FLOAT NewValue, WxPropertyWindow_Base* InItem);

	/**
	 * Sends the new values to all objects.
	 * @param NewValues		New values for the property.
	 */
	virtual void SetValues(const TArray<FLOAT>& NewValues, WxPropertyWindow_Base* InItem);

protected:
	/** Spinner for numeric properties. */
	WxNumericPropertySpinner* SpinButton;
};

/*-----------------------------------------------------------------------------
	UPropertyInputInteger
-----------------------------------------------------------------------------*/

class UPropertyInputInteger : public UPropertyInputNumeric
{
	DECLARE_CLASS(UPropertyInputInteger,UPropertyInputNumeric,0,UnrealEd)

	virtual void CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue );
	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void SendToObjects( WxPropertyWindow_Base* InItem );

	/**
	* @return Returns the numeric value of the property.
	*/
	virtual FLOAT GetValue();

	/**
	* Updates the numeric value of the property.
	* @param	Value	New Value for the property.
	*/
	virtual void SetValue(FLOAT NewValue, WxPropertyWindow_Base* InItem);

	/**
	 * Sends the new values to all objects.
	 * @param NewValues		New values for the property.
	 */
	virtual void SetValues(const TArray<FLOAT>& NewValues, WxPropertyWindow_Base* InItem);
};


/*-----------------------------------------------------------------------------
	UPropertyInputRotation
-----------------------------------------------------------------------------*/

class UPropertyInputRotation : public UPropertyInputNumeric
{
	DECLARE_CLASS(UPropertyInputRotation,UPropertyInputNumeric,0,UnrealEd)

	virtual void CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue );
	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void SendToObjects( WxPropertyWindow_Base* InItem );

	/**
	 * @return Returns the numeric value of the property.
	 */
	virtual FLOAT GetValue();

	/**
	 * Gets the values for each of the objects that a property item is affecting.
	 * @param	InItem		Property item to get objects from.
	 * @param	Values		Array of FLOATs to store values.
	 */
	virtual void GetValues(WxPropertyWindow_Base* InItem, TArray<FLOAT> &Values);


	/**
	 * Updates the numeric value of the property.
	 * @param	Value	New Value for the property.
	 */
	virtual void SetValue(FLOAT NewValue, WxPropertyWindow_Base* InItem);

	/**
	 * Sends the new values to all objects.
	 * @param NewValues		New values for the property.
	 */
	virtual void SetValues(const TArray<FLOAT>& NewValues, WxPropertyWindow_Base* InItem);
};

/*-----------------------------------------------------------------------------
	UPropertyInputCombo
-----------------------------------------------------------------------------*/

class UPropertyInputCombo : public UPropertyInputArrayItemBase
{
	DECLARE_CLASS(UPropertyInputCombo,UPropertyInputArrayItemBase,0,UnrealEd)

	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void CreateControls( WxPropertyWindow_Base* InItem, UClass* InBaseClass, const wxRect& InRC, const TCHAR* InValue );
	virtual void DeleteControls();
	virtual void SendToObjects( WxPropertyWindow_Base* InItem );
	virtual void ParentResized( WxPropertyWindow_Base* InItem, const wxRect& InRC );

	/**
	 * Indicates that this combo box's values are friendly names for the real values; currently only used for enum drop-downs.
	 */
	UBOOL bUsesAlternateDisplayValues;
};

/*-----------------------------------------------------------------------------
	UPropertyInputEditInline
-----------------------------------------------------------------------------*/

class UPropertyInputEditInline : public UPropertyInputArrayItemBase
{
	DECLARE_CLASS(UPropertyInputEditInline,UPropertyInputArrayItemBase,0,UnrealEd)

	virtual UBOOL Supports( const WxPropertyWindow_Base* InItem, INT InArrayIdx ) const;
	virtual void ButtonClick( WxPropertyWindow_Base* InItem, ePropButton InButton );

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

#endif // __PROPERTYUTILS_H__
