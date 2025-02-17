/*=============================================================================
	UnUIStyleEditor.h: UI style editor window and control class declarations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNUISTYLEEDITOR_H__
#define __UNUISTYLEEDITOR_H__


/**
 * This panel allows you to preview effects of a UIStyle_Text on a string.  It contains a textbox for entering
 * text and a viewport which renders the text.
 */
class WxTextPreviewPanel : public wxPanel, public FViewportClient, public FSerializableObject
{
	DECLARE_DYNAMIC_CLASS(WxTextPreviewPanel)
	DECLARE_EVENT_TABLE()

public:
	WxTextPreviewPanel() : txt_Preview(NULL), pnl_Preview(NULL), BackgroundColor(0), HorzPadding(0), VertPadding(0)
	{}

	/** Destructor */
	~WxTextPreviewPanel();

	/**
	 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * 
	 * @param	InParent		the window that contains this control
	 * @param	InID			the window ID to use for this control
	 * @param	CurrentStyle	a copy of the style that is being edited
	 * @param	CurrentState	the state that is being edited
	 */
	void Create( wxWindow* InParent, wxWindowID InID, UUIStyle* CurrentStyle, UUIState* CurrentState );

	/**
	 *	Returns the current text data evaluated from the referenced StyleData
	 */
	UUIStyle_Text* GetCurrentTextData();

	/**
	* Forces the viewport to be redrawn
	*/
	void RefreshViewport();

	/**
	 * Changes the style used by this preview panel.
	 */
	void SetPreviewStyle( UUIStyle* CurrentStyle, UUIState* CurrentState );

	//FSerializableObject interface
	virtual void Serialize( FArchive& Ar );

	// FViewportClient interface
	/**
	 * Draws the preview text in the viewport using the specified font
	 *
	 * @param Viewport the viewport being drawn into
	 * @param RI the render interface to draw with
	 */
	virtual void Draw(FViewport* Viewport, FCanvas* Canvas);

	FColor				BackgroundColor;

	// Controls
	WxTextCtrl*			txt_Preview;
	WxViewportHolder*	pnl_Preview;

	/** the amount of padding to use when rendering the text */
	FLOAT HorzPadding, VertPadding;

protected:
	/**
	 * Creates the controls and sizers for this panel.
	 */
	void CreateControls();

	/**
	 * 	Sets the value of StyleData appropriately based on weather we are dealing with an text style or a combo style
	 */
	void SetupStyleData( UUIStyle* CurrentStyle, UUIState* CurrentState );

private:
	/* =======================================
		Windows event handlers.
	   =====================================*/

	/** Called when the user types text into the preview text control */
	void OnTextChanged( wxCommandEvent& Event );

	/** Called when the size of the panel is modified */
	void OnSize( wxSizeEvent& Event );

	/**
	 * The style/state currently being previewed
	 */
	FStyleDataReference		StyleData;

	/** used to preview the effects of the style on a string */
	UUIPreviewString*		PreviewString;
};

class WxImagePreviewPanelBase : public wxPanel, public FViewportClient, public FSerializableObject
{
	DECLARE_DYNAMIC_CLASS(WxImagePreviewPanelBase)
	DECLARE_EVENT_TABLE()

public:
	WxImagePreviewPanelBase();

	/**
	 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * 
	 * @param	InParent				the window that contains this control
	 * @param	InID					the window ID to use for this control
	 * @param	InStyle					a copy of the style currently being edited
	 * @param	CurrentState			the state that is being edited
	 * @param	OwnerImageGroup			image property group which owns this panel
	 */
	virtual void Create( wxWindow* InParent, wxWindowID InID, UUIStyle* CurrentStyle, UUIState* CurrentState,
						class WxStyleImagePropertiesGroup* OwnerImageGroup = NULL);

	/**
	 *	Returns the current image data evaluated from the referenced StyleData
	 */
	UUIStyle_Image* GetCurrentImageData();

	/**
	 * Gets the currently selected preview image
	 */
	USurface* GetPreviewImage() const;

	/**
	 * Sets the current texture to None.
	 */
	void SetImageToNone();

	/**
	 * Changes the style data used by this preview panel.
	 */
	virtual void SetPreviewStyle( UUIStyle* CurrentStyle, UUIState* CurrentState );

	/**
	 * Sets the preview image to the surface specified
	 */
	void SetPreviewImage( USurface* NewPreviewImage );	

	// FViewportClient interface
	/**
	 * Draws the preview image in the viewport
	 *
	 * @param Viewport the viewport being drawn into
	 * @param RI the render interface to draw with
	 */
	virtual void Draw(FViewport* Viewport, FCanvas* Canvas);

	/**
	 * Disables editing any properties through the controls
	 */
	virtual void DisableControls();

	/** 
	 * Displays the <multiple values> tag in the text label for the texture
	 */
	void DisplayMultipleValues();

	/**
	 * Forces the viewport to be redrawn
	 */
	void RefreshViewport();

	// FSerializableObject interface
	virtual void Serialize( FArchive& Ar );

	WxUITextCtrl*		txt_Preview;
	WxViewportHolder*	pnl_Preview;

	/** clicking this button displays the generic browser and changes its filter to the corresponding type */
	wxBitmapButton*		btn_Preview;

	/** clicking this buttons assigns whichever texture/material is selected in the browser as the texture to preview */
	wxBitmapButton*		btn_Assign;

	/** Size to pad preview images by */
	INT HorzPadding, VertPadding;

	/** the preview texture's UV coordinates */
	FTextureCoordinates RenderCoordinates;

	/** the current background color */
	FColor				BackgroundColor;

protected:
	/**
	 * Creates the controls and sizers for this panel.
	 */
	virtual void CreateControls()=0;

	/**
	 *	Sets the value of StyleData appropriately based on wheather we are dealing with an image style or a combo style
	 */
	void SetupStyleData( UUIStyle* CurrentStyle, UUIState* CurrentState );

	/**
	 * Event handler for when the sync to generic browser button has been clicked for locating the current preview image.
	 *
	 * @param	Event	Information about the event.
	 */
	void OnClick_Browse( wxCommandEvent& Event );

	/** Property group which contains this panel */
	class WxStyleImagePropertiesGroup* OwnerImageGroup;

	/**
	 * The style/state currently being previewed
	 */
	FStyleDataReference	StyleData;

	/** used to preview the effects of the style on an image */
	class UUITexture*	PreviewImage;
};


class WxDefaultImagePreviewPanel : public WxImagePreviewPanelBase
{
	DECLARE_DYNAMIC_CLASS(WxDefaultImagePreviewPanel)
	DECLARE_EVENT_TABLE()

public:
	wxStaticText*		lbl_Preview;

	/**
	 * Changes the style data used by this preview panel.  DefaultImagePreview Panel will only use default image style settings and not actual values
	 */
	virtual void SetPreviewStyle( UUIStyle* CurrentStyle, UUIState* CurrentState );

protected:
	/**
	 * Creates the controls and sizers for this panel.
	 */
	virtual void CreateControls();

private:
	void OnClick_UseSelected( wxCommandEvent& Event );
	void OnClick_Browse( wxCommandEvent& Event );

};

/** Readonly textbox that only responds to special keys used to clear current the texture. */
class WxImagePreviewTextCtrl : public WxUITextCtrl
{
public:
	WxImagePreviewTextCtrl(WxDefaultImagePreviewPanel* InParent);
private:

	/** On Character event that responds to backspace only. */
	void OnChar(wxKeyEvent &event);

	/** Reference to our parent style editor. */
	class WxDefaultImagePreviewPanel* Parent;
	DECLARE_EVENT_TABLE()
};


/**
 * This panel allows you to preview effects of a UIStyle_Image on a texture.  It contains a viewport for viewing the texture,
 * a button for choosing the texture, and a textbox which displays the pathname of the texture.
 */
class WxImagePreviewPanel : public WxImagePreviewPanelBase
{
	DECLARE_DYNAMIC_CLASS(WxImagePreviewPanel)
	DECLARE_EVENT_TABLE()
public:

protected:
	/**
	 * Creates the controls and sizers for this panel.
	 */
	virtual void CreateControls();

private:
	void OnClick_UseSelected( wxCommandEvent& Event );
};


/**
 *	This is a UI custom color adjustment panel class which contains a RGB color picker button and an Alpha value slider
 */
class WxColorAdjustmentPanel: public wxPanel
{
	DECLARE_DYNAMIC_CLASS(WxColorAdjustmentPanel)
	DECLARE_EVENT_TABLE()

public:

	/** Default constructor for use in two-stage dynamic window creation */
	WxColorAdjustmentPanel();

	/**
	 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * 
	 * @param	InParent			the window that contains this control
	 * @param	InID				the window ID to use for this control
	 * @param	InitialData			the data that will be used to first initialize controls, cannot be NULL
	 * @param	OwnerPropertyGroup  the style property group which contains this panel
	 */
	void Create( wxWindow* InParent, wxWindowID InID, class UUIStyle_Data* InitialData, class WxStylePropertiesGroup* OwnerPropertyGroup );

	/**
	 * Called when the user selects a new menu state or states from the list. Updates internal
	 * array of data that will be modified through this panel. Reinitializes all
	 * control values with the style data from the first item in the passed array 
	 *
	 * @param	SelectedStatesData		Array of data from the currently selected states
	 */
	void NotifySelectedStatesChanged( const TArray< UUIStyle_Data* > & SelectedStatesData );

	/**
	 * Initialize the values of the controls contained by this panel with the values from the current style.
	 */
	void InitializeControlValues();

protected:

	/**
	 * Creates the controls and sizers for this panel.
	 */
	virtual void CreateControls();

	/**
	 *	Notifies owner style editor that property has been modified
	 */
	void NotifyPropertyChanged();


	/** Owner style property group panel used for notification of color changes */
	WxStylePropertiesGroup*    OwnerPropertyGroup;

	/** Button used for selection of RGB color */ 
	WxUIButton*				   btn_Color;

	/** Label display next to the alpha slider */
	wxStaticText*			   lbl_AlphaSlider;

	/** Slider used to change the color alpha value */
	wxSlider*				   sldr_AlphaSlider;

	/** the currently selected states data being edited */
	TArray<UUIStyle_Data*>	   StyleDataArray;

	/** Owner image properties group that will receive notification about property change */
	class WxStyleImagePropertiesGroup* OwnerImagePropertiesGroup;

private:

	/**
	 * Pops up a color picker for choosing the style color, then notifies all groups that the color has been changed.
	 * Called when the user presses the "choose color" button.
	 */
	void OnClick_ChangeColor( wxCommandEvent& Event );

	/**
	 *	Handler for the alpha slider scroll event, sets the currently selected states color alpha value to the value of the slider
	 */
	void OnScroll_ChangeAlpha( wxScrollEvent& Event );

	/**
	 * Common routine for letting the user select a color choice.  Copied from WxFontPropertiesDlg::WxFontPreviewDlg
	 *
	 * @param Color the current value of the color that is being changed
	 *
	 * @return The newly selected color or the original if no change
	 */
	FColor SelectColor(FColor Color);

	/**
	 * returns TRUE is color values for all the selected states are the same	
	 */
	UBOOL CompareColorProperty() const;

	/**
	 * Updates all currently selected States's data for with the RGB values of the new color
	 *
	 * @param	Color	source color for the RGB values
	 */
	void UpdateColorProperty( const FLinearColor & Color );	

	/**
	 * Updates all currently selected States's data with the new alpha value
	 *
	 * @param	Alpha	alpha value between 0.0 - 1.0
	 */
	void UpdateAlphaProperty( FLOAT Alpha );
};

/**
 * This class contains controls for editing the ImageAdjustmentData for a single orientation of an image style.  An image style
 * editor panel will generally have one of these for each orientation supported.
 */
class WxImageAdjustmentPanel : public wxPanel
{
	DECLARE_DYNAMIC_CLASS(WxImageAdjustmentPanel)
	DECLARE_EVENT_TABLE()

public:
	/** Default constructor for use in two-stage dynamic window creation */
	WxImageAdjustmentPanel();

	/**
	 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * 
	 * @param	InParent					the window that contains this control
	 * @param	InID						the window ID to use for this control
	 * @param	InOrientation				the orientation to use for this panel.  this controls the text that is placed in the various labels
	 * @param   OwnerImagePropertiesGroup	image properties group which will be notified about the property change
	 * @param	CurrentStyle				a copy of the style being edited
	 * @param	CurrentState				the state that is being edited
	 */
	virtual void Create( wxWindow* InParent, wxWindowID InID, EUIOrientation InOrientation, class WxStyleImagePropertiesGroup* OwnerImagePropertiesGroup, UUIStyle* CurrentStyle, UUIState* CurrentState );

	/**
	 * Disables editing of any properties inside this group
	 */
	virtual void DisableControls();

	/**
	* Called when the user selects a new menu state or states from the list.  Reinitializes 
	* all control values with the style data from the first selected state and updates internal
	* array of data that will be modified through this panel.
	*
	* @param	SelectedStates	Array of states that are currently selected
	*/
	virtual void NotifySelectedStatesChanged(TArray<UUIState*> & SelectedStates);

	/**
	 * Sets the value of the alignment combo to the value specified
	 */
	void SetAlignment( EUIAlignment Alignment )
	{
		if ( cmb_Alignment != NULL )
		{
			cmb_Alignment->SetSelection(Alignment);
		}
	}

	/**
	 * Sets the vaue of the adjustment combo
	 */
	void SetAdjustment( EMaterialAdjustmentType AdjustmentType )
	{
		if ( cmb_Adjustment != NULL )
		{
			cmb_Adjustment->SetSelection(AdjustmentType);
		}
	}

	/**
	 * Sets the value of the padding control
	 */
	void SetPadding( FLOAT NewPaddingValue )
	{
		if ( spin_Padding != NULL )
		{
			spin_Padding->SetValue(NewPaddingValue);
		}
	}

	/**
	 * Sets the value of the gutter controls
	 */
	void SetGutter( FLOAT LeftGutter, FLOAT RightGutter )
	{
		if ( spin_Gutter[UIORIENT_Horizontal] != NULL )
		{
			spin_Gutter[UIORIENT_Horizontal]->SetValue(appTrunc(LeftGutter));
		}
		if ( spin_Gutter[UIORIENT_Vertical] != NULL )
		{
			spin_Gutter[UIORIENT_Vertical]->SetValue(appTrunc(RightGutter));
		}
	}

	/**
	 * Sets the value of the texture coordinate controls
	 */
	void SetTextureCoordinates( FLOAT Origin, FLOAT Extent )
	{
		if ( spin_TextureCoordinates[0] != NULL )
		{
			spin_TextureCoordinates[0]->SetValue(appTrunc(Origin));
		}
		if ( spin_TextureCoordinates[1] != NULL )
		{
			spin_TextureCoordinates[1]->SetValue(appTrunc(Extent));
		}
	}

	// Getters
	EUIAlignment GetAlignment() const
	{
		return cmb_Alignment ? (EUIAlignment)cmb_Alignment->GetSelection() : UIALIGN_MAX;
	}

	EMaterialAdjustmentType GetAdjustmentType() const
	{
		return cmb_Adjustment ? (EMaterialAdjustmentType)cmb_Adjustment->GetSelection() : ADJUST_None;
	}

	FLOAT GetPadding() const
	{
		return spin_Padding ? spin_Padding->GetValue() : 0.f;
	}

	void GetGutterPixels( FLOAT& HorzGutter, FLOAT& VertGutter ) const
	{
		if ( spin_Gutter[UIORIENT_Horizontal] != NULL )
		{
			HorzGutter = spin_Gutter[UIORIENT_Horizontal]->GetValue();
		}

		if ( spin_Gutter[UIORIENT_Vertical] != NULL )
		{
			VertGutter = spin_Gutter[UIORIENT_Vertical]->GetValue();
		}
	}

	void GetTextureCoordinates( FLOAT& Origin, FLOAT& Extent )
	{
		if ( spin_TextureCoordinates[0] != NULL )
		{
			Origin = spin_TextureCoordinates[0]->GetValue();
		}

		if ( spin_TextureCoordinates[1] != NULL )
		{
			Extent = spin_TextureCoordinates[1]->GetValue();
		}
	}

	/** Initializes values of the controls */
	void InitializeControlValues();

	// Controls

	/** combobox for selecting the image alignment */
	wxStaticText*	lbl_Alignment;
	WxUIChoice*		cmb_Alignment;

	/** combo for choosing the image adjustment type */
	wxStaticText*	lbl_Adjustment;
	WxUIChoice*		cmb_Adjustment;

	/** spinner for setting the padding value */
	wxStaticText*	lbl_Padding;
	WxUISpinCtrl*	spin_Padding;

	/** numeric edit controls for specifying the number of pixels for the protected region */
	wxStaticText*	lbl_Gutter[UIORIENT_MAX];
	WxUISpinCtrl*	spin_Gutter[UIORIENT_MAX];

	/** spinner for specifying the texture coordinates to use for the default texture */
	wxStaticText*	lbl_TextureCoordinates[2];
	WxUISpinCtrl*	spin_TextureCoordinates[2];

protected:

	/** Initialized the alignment combo */
	void InitializeAlignmentCombo();

	/** Initialized adjustment combo */
	void InitializeAdjustmentCombo();

	/** populates the value of the padding spinner with the values from the currently selected states */
	void InitializePaddingSpinner();

	/** initializes the spin gutters */
	void InitializeSpinGutters();

	/** Initialize spin controls for setting texture coordinates */
	void InitializeSpinTextureCoords();

	UUIStyle* EditingStyle;

	UUIState* CurrentState;

	/** the currently selected states' data being edited */
	TArray<UUIStyle_Image*>	StyleDataArray;

	/** the orientation that this panel is associated with */
	BYTE			Orientation;

	/** Owner image properties group that will receive notification about property changed */
	class WxStyleImagePropertiesGroup* OwnerImagePropertiesGroup;

	/**
	 * Creates the controls and sizers for this panel.
	 */
	virtual void CreateControls();

	/**
	 *	Notifies owner properties group that property has been modified
	 */
	void NotifyPropertyChanged();

private:
	void OnSelectionChanged( wxCommandEvent& Event );

	void OnGutterTextChanged( wxSpinEvent& Event );
	void OnGutterTextChanged( wxCommandEvent& Event );

	void OnPaddingChanged( wxSpinEvent& Event );
	void OnPaddingChanged( wxCommandEvent& Event );

	void OnCoordinatesTextChanged( wxSpinEvent& Event );
	void OnCoordinatesTextChanged( wxCommandEvent& Event );

	/* ===============================================
		Private helper functions
		=============================================*/

	/**
	 * Updates all currently selected States's Image data for this Orientation with the new Alignment
	 *
	 * @param	Alignment	New alignment for this image orientation
	 */
	void UpdateAlignmentProperty(EUIAlignment Alignment);

	/**
	 * Updates all currently selected States's Image data for this Orientation with the new Adjustment
	 *
	 * @param	Adjustment	New adjustment for this image orientation
	 */
	void UpdateAdjustmentProperty(EMaterialAdjustmentType Adjustment);

	/**
	 * Updates the Padding value for all currently selected states with the value from the padding spinner
	 */
	void UpdatePaddingProperty( FLOAT Padding );

	/**
	 * Updates all currently selected States's Image data for this Orientation with the new Gutter value
	 *
	 * @param	GutterOrientation		Orientation of the gutter
	 * @param	GutterValue				New float value of the gutter for this orientation
	 */
	void UpdateGutterProperty(INT GutterOrientation, FLOAT GutterValue );

	/**
	 * Updates the texture coordinates for all currently selected states with the new value.
	 *
	 * @param	bOriginValue	TRUE to modify the U or V value, FALSE to modify the UL or VL value (depending on orientation)
	 * @param	NewValue		the new value to apply
	 */
	void UpdateCoordinatesProperty( UBOOL bOriginValue, FLOAT NewValue );

	/** Returns TRUE if all items in the StyleData array have the same alignment value for the corresponding orientation */
	UBOOL CompareAlignmentProperty() const;

	/** Returns TRUE if all items in the StyleData array have the same value for StylePadding for the corresponding orientation */
	UBOOL ComparePaddingProperty() const;
	
	/** Returns TRUE if all items in the StyleData array have the same material adjustment value for the corresponding orientation */
	UBOOL CompareAdjustmentProperty() const;

	/**
	 *	Returns TRUE if all items in the StyleData array have the same gutter property for the adjustment panel's orientation
	 *
	 *	@param	GutterOrientation	specifies which gutter value for the adjustment panel's orientation to evaluate. 
	 *  It will specify either top or bottom gutter if this panel's orientation is UIORIENT_Vertical, or left or right if this
	 *  panel's orientation is UIORIENT_Horizontal  
	 */
	UBOOL CompareGutterProperty(EUIOrientation GutterOrientation) const;

	/**
	 *	Returns TRUE if all items in the StyleData array have the same texture coordinate properties for the specified orientation and either size or position.
	 *  
	 *  @param	Orientation		either UIORIENT_Horizontal or UIORIENT_Vertical
	 *  @param  bCompareSize	if set to FALSE the coordinates position value will be compare, if TRUE the coords size value will be compared 
	 */
	UBOOL CompareTextureCoordProperty(EUIOrientation Orientation, UBOOL bCompareSize ) const;
};

/**
 * This is an abstract base class for all custom style property editing panels
 */
class WxStylePropertiesGroup : public wxPanel, public FSerializableObject
{
	DECLARE_DYNAMIC_CLASS(WxStylePropertiesGroup)

	DECLARE_EVENT_TABLE()

public:
	/** Default constructor for use in two-stage dynamic window creation */
	WxStylePropertiesGroup();

	/**
	 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * 
	 * @param	InParent			the window that contains this control
	 * @param	InID				the window ID to use for this control
	 * @param	CurrentStyle		the UIStyle that is being edited
	 * @param	InitialState		the state that should be initially active for the current style
	 */
	virtual void Create( wxWindow* InParent, wxWindowID InID, class UUIStyle* CurrentStyle, class UUIState* InitialState );
	
	/**
	* Called to notify this property group that the user has selected a new UIStates from the list of states available for this style.
	*/
	virtual void NotifySelectedStatesChanged(TArray<UUIState*> & SelectedStates)
	{
		if(SelectedStates.Num())
		{
			CurrentState = SelectedStates(0);
		}
	}

	/**
	 * Called to notify this property group that the style has been changed.  Refreshes the preview window with the style's current values
	 *
	 * @param	bPropagateStyleChanges	when TRUE the Style editor will propagate changes to the styles which derive 
	 *                                  from the editing style and refresh the widgets whose style has been changed.
	 *                                  flag set to FALSE will cause reinitialization of controls of this property group
	 */
	virtual void NotifyStyleModified( UBOOL bPropagateStyleChanges=TRUE );

	/**
	 *	Notifies owner style editor that property has been modified
	 */
	void NotifyPropertyChanged();

	/**
	 * Applies any changes made in this property group to the style being edited.  Called when the user clicks the OK button.
	 */
 	virtual void SaveChanges()=0;

	/**
	 * Disables editing of any properties inside this group
	 */
	virtual void DisableControls()=0;

	/**
	 *	Refreshes any preview windows in the property group
	 */
	virtual void RefreshPreview(){};

	/**
	 *  Removes any preview controls from the display, used to obtain the minimal set of controls needed to display all the properties
	 */
	virtual void RemovePreviewControls()=0;

	/**
	 * A copy of the style being edited.
	 */
	class UUIStyle* EditingStyle;

	/** the currently selected style state for which the properties are displayed*/
	class UUIState*	CurrentState;

protected:
	/** pointer to the scene manager singleton */
	class UUISceneManager*	SceneManager;

	/**
	 * Creates the controls and sizers for this panel.
	 */
	virtual void CreateControls()=0;

	/**
	 * Initialize the values of the controls contained by this panel with the values from the current style.
	 */
	virtual void InitializeControlValues()=0;
};

/**
 * This class contains controls for editing properties common to all style classes, such as style tag, friendly name,
 * and style color
 */
class WxStyleCommonPropertiesGroup : public WxStylePropertiesGroup
{
	DECLARE_DYNAMIC_CLASS(WxStyleCommonPropertiesGroup)

	DECLARE_EVENT_TABLE()

public:
	WxStyleCommonPropertiesGroup();

	/**
	 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * 
	 * @param	InParent			the window that contains this control
	 * @param	InID				the window ID to use for this control
	 * @param	CurrentStyle		the UIStyle that is being edited
	 * @param	InitialState		the state that should be initially active for the current style
	 */
	virtual void Create( wxWindow* InParent, wxWindowID InID, class UUIStyle* CurrentStyle, class UUIState* InitialState );

	/**
	 * Called to notify this property group that the user has selected a new UIStates from the list of 
	 * states available for this style.
	 *
	 * @param	SelectedStates	Array of states that are going to be edited
	 */
	virtual void NotifySelectedStatesChanged( TArray<UUIState*> & SelectedStates );

	/**
	 * Disables editing of any properties inside this group
	 */
	virtual void DisableControls();

	/**
	 *  Removes any preview controls from the display, used to obtain the minimal set of controls needed to display all the properties
	 */
	virtual void RemovePreviewControls(){};

	/**
	 * Applies any changes made in this property group to the style being edited.  Called when the user clicks the OK button.
	 */
	virtual void SaveChanges();

	/**
	 * Makes sure that state data that has been modified by the user will not be lost due to deserialization of the unchecked states.
	 * If such data is found, a message box is displayed asking user weather to proceed with the deserialization proscess.
	 *
	 * @return	TRUE if deserialization of any unchecked states can be proceeded, or FALSE if it should not. 
	 */
	UBOOL VerifyUncheckedStateChanges();

	/**
	 * Reverts data for states that are unchecked to the original values stored in the archetype
	 */
	void DeserializeUncheckedStates();

	/**
	 * Clears the dirty flag from all the style data changed by this editor 
	 */
	void ClearDirtyFlags();

	/**
	 * Obtains a list of currently selected states
	 *
	 * @param	SelectedStates	Array that will be populated with states currently selected in the editor
	 */
	void GetSelectedStates( TArray<UUIState*> & SelectedStates );

	// FSerializableObject interface
	virtual void Serialize( FArchive& Ar );

protected:
	/**
	 * Creates the controls and sizers for this panel.
	 */
	virtual void CreateControls();

	/**
	 * Initialize the values of the controls contained by this panel with the values from the current style.
	 */
	virtual void InitializeControlValues();

private:
	/** the label for the style id display */
	wxStaticText*	lbl_StyleID;

	/** the label that displays the style ID */
	wxStaticText*	txt_StyleID;

	/** the label for the friendly name text control */
	wxStaticText*	lbl_FriendlyName;

	/** text control for editing the style's friendly name */
	wxTextCtrl*		txt_FriendlyName;

	/** label for the style tag text control */
	wxStaticText*	lbl_StyleTag;

	/** text control for editing the style's unique tag */
	wxTextCtrl*		txt_StyleTag;

	/** listbox for choosing which menu states that will be supported by this UIStyle */
	wxCheckListBox*	chklst_MenuStates;

	/** local storage for the style tag - necessary for the wxTextValidator */
	wxString		StyleTagString;

	/** combobox control for the group tag name */
	wxComboBox*		cmb_StyleGroupTag;

	/* =======================================
		Windows event handlers.
	=====================================*/

	/** Called when the user checks or unchecks a menu state */
	void OnToggleMenuState( wxCommandEvent& Event );

	/** Handler for mouse button right click in the states listbox */ 
	void OnChecklistMenuStatesRightDown( wxMouseEvent& event );

	/** Event handler for comparing properties of two states */
	void OnContext_DiffProperties( wxCommandEvent& Event );

	/** Event handler for selecting group name from combobox */
	void OnCombobox_SelectGroupName( wxCommandEvent& Event );
};

/**
 * This class contains controls for editing the properties of UIStyle_Text
 */
class WxStyleTextPropertiesGroup : public WxStylePropertiesGroup
{
	DECLARE_DYNAMIC_CLASS(WxStyleTextPropertiesGroup)

	DECLARE_EVENT_TABLE()
	
public:
	WxStyleTextPropertiesGroup() 
	: pnl_Color(NULL), cmb_Font(NULL), cmb_ClipMode(NULL), cmb_ClipAlignment(NULL)
	, cmb_AutoScaleMode(NULL), spin_AutoScaleMinValue(NULL), chklst_Attributes(NULL), pnl_Preview(NULL) 
	{
		spin_TextScale[UIORIENT_Horizontal] = NULL;
		spin_TextScale[UIORIENT_Vertical] = NULL;
		spin_Padding[UIORIENT_Horizontal] = NULL;
		spin_Padding[UIORIENT_Vertical] = NULL;
		cmb_Alignment[UIORIENT_Horizontal] = NULL;
		cmb_Alignment[UIORIENT_Vertical] = NULL;
		spin_SpacingAdjust[UIORIENT_Horizontal] = NULL;
		spin_SpacingAdjust[UIORIENT_Vertical] = NULL;
	}

	/**
	 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * 
	 * @param	InParent			the window that contains this control
	 * @param	InID				the window ID to use for this control
	 * @param	CurrentStyle		the UIStyle that is being edited
	 * @param	InitialState		the state that should be initially active for the current style
	 */
	virtual void Create( wxWindow* InParent, wxWindowID InID, class UUIStyle* CurrentStyle, class UUIState* InitialState );

	// FSerializableObject interface
	virtual void Serialize( FArchive& Ar );
	
	/**
	 * Called to notify this property group that the user has selected a new UIStates to be edited
	 *
	 * @param	SelectedStates	Array of states for which style data is going to be edited
	 */
	virtual void NotifySelectedStatesChanged( TArray<UUIState*> & SelectedStates );

	/**
	* Disables editing of any properties inside this group
	*/
	virtual void DisableControls();

	/**
	 *  Removes any preview controls from the display, used to obtain the minimal set of controls needed to display all the properties
	 */
	virtual void RemovePreviewControls();

	/**
	 *	Refreshes any preview windows in the property group
	 */
	virtual void RefreshPreview();

	/**
	 * Applies any changes made in this property group to the style being edited.  Called when the user clicks the OK button.
	 */
	virtual void SaveChanges(){}

	/**
	 * Returns the style data for the currently active state
	 */
	UUIStyle_Text* GetCurrentStyleData();

	/**
	 * Returns text style data associated with the specified state of the style we are currently editing, if the style is a combo style then 
	 * custom data from its TextStyle reference will be returned
	 *
	 * @param	Style	style from which the data will be extracted
	 * @param	State	state for which the data will be returned
	 */
	UUIStyle_Text* GetTextDataForState( const UUIStyle* Style, UUIState* State );

protected:
	/**
	 * Creates the controls and sizers for this panel.
	 */
	virtual void CreateControls();

	/**
	 * Initialize the values of the controls contained by this panel with the values from the current style.
	 */
	virtual void InitializeControlValues();

	/**
	 *	Initializes the font combo box
	 */
	void InitializeFontCombo();

	/**
	 *	Initializes the alignment combo boxes
	 */
	void InitializeAlignmentCombos();

	/**
	 * Initializes the clipping combo boxes.
	 */
	void InitializeStringClipping();

	/**
	 *	Initializes the scale edit boxes
	 */
	void InitializeTextScale();

	/**
	 * Initializes the padding spin controls
	 */
	void InitializePadding();

	/**
	 * Initializes the values of the autoscale mode and min autoscale controls.
	 */
	void InitializeAutoScaling();

	/**
	 *	Initializes the spacing adjust edit boxes
	 */
	void InitializeSpacingAdjust();

	/**
	 * Returns TRUE if all the items in the StyleData array have the same font property value	
	 */
	UBOOL CompareFontProperty() const;

	/**
	 * Returns TRUE if all the items in the StyleData array have the same alignment values for the given orientation
	 *
	 * @param Orientation	orientation for which the alignment will be extracted, value must be less than UIORIENT_MAX
	 */
	UBOOL CompareTextAlignmentProperty( EUIOrientation Orientation ) const;

	/**
	 * @return	TRUE if all the items in the StyleData array have the same clipping mode
	 */
	UBOOL CompareClipModeProperty() const;

	/**
	 * @return	TRUE if all the items in the StyleData array have the same clipping mode
	 */
	UBOOL CompareClipAlignmentProperty() const;

	/**
	* Returns TRUE if the Scale property for the specified orientation is the same for all items in the StyleData array.
	*
	* @param Orientation	orientation for which the alignment will be extracted, value must be less than UIORIENT_MAX
	*/
	UBOOL CompareTextScaleProperty( EUIOrientation Orientation ) const;

	/**
	 * @return	TRUE if the AutoScaleMode property for all items in the StyleData array have the same value.
	 */
	UBOOL CompareAutoScaleModeProperty() const;

	/**
	 * @return	TRUE if the padding property has the same value for all elements in the StyleData array.
	 */
	UBOOL ComparePaddingProperty( EUIOrientation Orientation ) const;

	/**
	 * @return	TRUE if the AutoScaling.MinScale property for all items in the StyleData array have the same value.
	 */
	UBOOL CompareMinAutoScaleValues() const;

	/**
	 * Returns TRUE if the SpacingAdjust property for the specified orientation is the same for all items in the StyleData array.
	 *
	 * @param Orientation	orientation for which the alignment will be extracted, value must be less than UIORIENT_MAX
	 */
	UBOOL CompareSpacingAdjustProperty( EUIOrientation Orientation ) const;

private:

	/** Panel with controls for selecting the color */
	WxColorAdjustmentPanel*		pnl_Color;

	/** the combo box displaying the list of available fonts */
	WxUIChoice*					cmb_Font;

	/** combo box for choosing the text alignment for this style */
	WxUIChoice*					cmb_Alignment[UIORIENT_MAX];

	/** combo box for choosing the clip mode for a text style */
	WxUIChoice*					cmb_ClipMode;

	/** combo box for choosing the clip alignment for a text style */
	WxUIChoice*					cmb_ClipAlignment;

	/** spin control for the setting the text scale */
	WxUISpinCtrl*				spin_TextScale[UIORIENT_MAX];

	/** combo box for choosing how the text should be autoscaled */
	WxUIChoice*					cmb_AutoScaleMode;

	/** spinner for choosing the minimum autoscale factor */
	WxUISpinCtrl*				spin_AutoScaleMinValue;

	/** spin control for the setting the text spacing adjust */
	WxUISpinCtrl*				spin_SpacingAdjust[UIORIENT_MAX];

	/** spin control for setting the padding */
	WxUISpinCtrl*				spin_Padding[UIORIENT_MAX];

	/** checkbox list of font attributes for this style */
	wxCheckListBox*				chklst_Attributes;

	/** panel for previewing what the text will look like */
	WxTextPreviewPanel*			pnl_Preview;

	/** the currently selected states' data being edited */
	TArray<UUIStyle_Text*>		StyleDataArray;

	/* =======================================
		Windows event handlers.
	   =====================================*/

	/** Called when the user changes the font */
	void OnFontChanged( wxCommandEvent& Event );

	/** Called when the user changes the alignment */
	void OnAlignmentChanged( wxCommandEvent& Event );

	/** Called when the user changes the clipping mode */
	void OnClipModeChanged( wxCommandEvent& Event );

	/** Called when the user changes the clip alignment */
	void OnClipAlignmentChanged( wxCommandEvent& Event );

	/** Called when the user changes any of the font attributes */
	void OnAttributesChanged( wxCommandEvent& Event );

	/** Called when the user changes any of the text scale values */
	void OnScaleChanged( wxSpinEvent& Event );

	/** called when the user changes the auto-scale mode */
	void OnAutoScaleModeChanged( wxCommandEvent& Event );

	/** called when the user changes minimum auto-scale value */
	void OnMinAutoScaleChanged( wxSpinEvent& Event );

	/** Called when the user changes any of the text spacing adjust values */
	void OnSpacingAdjustChanged( wxSpinEvent& Event );

	/** Called when the user changes the value of one of the padding spin controls */
	void OnPaddingChanged( wxSpinEvent& Event );
	/** Called when the user modifies the text value of a padding spin control by editing the text field */
	void OnPaddingChanged( wxCommandEvent& Event );

	/* =======================================
		Private helper Functions.
		=====================================*/

	/**
	 * Updates all currently selected States's data with the new font
	 * 
	 * @param	Font	New font to be assigned to currently selected states data
	 */
	void UpdateFontProperty(UFont* Font);

	/**
	 * Updates all currently selected States's data with the new alignment
	 *
	 * @param	OrientationIndex	Which orientation property will receive new Alignment
	 * @param	Alignment			New Alignment assigned to selected orientation
	 */
	void UpdateAlignmentProperty(EUIOrientation OrientationIndex, EUIAlignment Alignment);

	/**
	 * Updates all currently selected States's data with the new clip mode
	 * 
	 * @param	NewClipMode		the new clip mode to apply to the selected states
	 */
	void UpdateClipModeProperty( ETextClipMode NewClipMode );

	/**
	 * Updates all currently selected States's data with the new clip alignment
	 * 
	 * @param	NewAlignment	the new clip alignment to apply to the selected states
	 */
	void UpdateClipAlignmentProperty( EUIAlignment NewAlignment );

	/**
	 * Updates all currently selected States's data with the new attributes
	 */
	void UpdateAttributesProperty();

	/**
	 * Changes the scale for the specified orientation in all selected states to the specified value.
	 *
	 * @param	Orientation		indicates which axis's scale value should be modified
	 * @param	NewScale		the new scale to be applied
	 */
	void UpdateScaleProperty( EUIOrientation Orientation, FLOAT NewScale );

	/**
	 * Changes the AutoScaleMode for all selected states to the specified value.
	 *
	 * @param	NewAutoScaleMode	the new autoscale mode to apply.
	 */
	void UpdateAutoScaleModeProperty( ETextAutoScaleMode NewAutoScaleMode );

	/**
	 * Updates all currently selected States's data with the new scale
	 *
	 * @param	NewMinScale		the new minimum scale to apply
	 */
	void UpdateMinAutoScaleProperty( FLOAT NewMinScale );

	/**
	 * Changes the spacing adjust for the specified orientation in all selected states to the specified value.
	 *
	 * @param	Orientation		indicates which axis's scale value should be modified
	 * @param	NewSpacingAdjust		the new spacing adjust to be applied
	 */
	void UpdateSpacingAdjustProperty( EUIOrientation Orientation, FLOAT NewSpacingAdjust );

	/**
	 * Changes the padding value for the specified orientation in all selected states.
	 *
	 * @param	Orientation		indicates which axis's scale value should be modified
	 * @param	NewPaddingValue	the new padding value to apply.
	 */
	void UpdatePaddingProperty( EUIOrientation Orientation, FLOAT NewPaddingValue );
};

/**
 * This class contains controls for editing the properties of UIStyle_Image
 */
class WxStyleImagePropertiesGroup : public WxStylePropertiesGroup
{
	DECLARE_DYNAMIC_CLASS(WxStyleImagePropertiesGroup)

	DECLARE_EVENT_TABLE()
	
public:
	WxStyleImagePropertiesGroup(): pnl_Color(NULL), pnl_DefaultTexture(NULL),pnl_Preview(NULL){}

	/** Destructor */
	virtual ~WxStyleImagePropertiesGroup();

	/**
	 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * 
	 * @param	InParent			the window that contains this control
	 * @param	InID				the window ID to use for this control
	 * @param	CurrentStyle		the UIStyle that is being edited
	 * @param	InitialState		the state that should be initially active for the current style
	 */
	virtual void Create( wxWindow* InParent, wxWindowID InID, class UUIStyle* CurrentStyle, class UUIState* InitialState );

	/**
	 * Called to notify this property group that the user has selected a new UIStates to be edited
	 *
	 * @param	SelectedStates	Array of states for which style data will be edited
	 */
	virtual void NotifySelectedStatesChanged(TArray<UUIState*> & SelectedStates);

	/**
	 * Disables editing of any properties inside this group
	 */
	virtual void DisableControls();

	/**
	 *  Removes any preview controls from the display, used to obtain the minimal set of controls needed to display all the properties
	 */
	virtual void RemovePreviewControls();

	/**
	 *	Refreshes any preview windows in the property group
	 */
	virtual void RefreshPreview();

	/**
	 * Returns the style data for the currently active state
	 */
	UUIStyle_Image* GetCurrentStyleData();

	/**
	 * Returns text style data associated with the specified state of the style we are currently editing, if the style is a combo style then 
	 * custom data from its TextStyle reference will be returned
	 *
	 * @param	Style	style from which the data will be extracted
	 * @param	State	state for which the data will be returned
	 */
	UUIStyle_Image* WxStyleImagePropertiesGroup::GetImageDataForState( const UUIStyle* Style, UUIState* State );

	/**
	 * Applies the specified texture to current style's selected states 
	 *
	 * @param	DefaultTexture	Texture to be assigned to each Image style
	 */
	void ApplyDefaultTexture(USurface* DefaultTexture);

	/**
	 * Applies any changes made in this property group to the style being edited.  Called when the user clicks the OK button.
	 */
	virtual void SaveChanges(){}

	// FSerializableObject interface
	virtual void Serialize( FArchive& Ar );

	/** Panel with controls for selecting the color */
	WxColorAdjustmentPanel*		pnl_Color;

	/** this panel contains controls for selecting the default texture for this image style */
	WxDefaultImagePreviewPanel*		pnl_DefaultTexture;

	/** these panels contain controls for editing the image adjustment attributes */
	WxImageAdjustmentPanel*			pnl_Adjustment[UIORIENT_MAX];

	/** this panel displays the image preview */
	WxImagePreviewPanel*			pnl_Preview;

protected:
	/**
	 * Creates the controls and sizers for this panel.
	 */
	virtual void CreateControls();

	/**
	 * Initialize the values of the controls contained by this panel with the values from the current style.
	 */
	virtual void InitializeControlValues();

	/** Initialize the default texture control */
	void InitializeDefaultTexture();

	/**
	 *	returns TRUE if all the items in the styledata array have the same texture property value
	 */
	UBOOL CompareTextureData() const;

private:

	/** style data being edited */
	TArray<UUIStyle_Image*> StyleDataArray;
};

/**
 * This class contains controls for editing the properties of UIStyle_Combo
 */
class WxStyleComboPropertiesGroup : public WxStylePropertiesGroup
{
	DECLARE_DYNAMIC_CLASS(WxStyleComboPropertiesGroup)

	DECLARE_EVENT_TABLE()
	
public:
	WxStyleComboPropertiesGroup() : lbl_TextStyle(NULL), btn_TextExisting(NULL), btn_TextCustom(NULL), lbl_ImageStyle(NULL), btn_ImageExisting(NULL), btn_ImageCustom(NULL),
		cmb_TextStyle(NULL), cmb_ImageStyle(NULL), pnl_TextStyle(NULL), pnl_ImageStyle(NULL), pnl_CustomTextProperties(NULL), pnl_CustomImageProperties(NULL)
	{}

	/** Destructor */
	virtual ~WxStyleComboPropertiesGroup();

	/**
	 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
	 * 
	 * @param	InParent			the window that contains this control
	 * @param	InID				the window ID to use for this control
	 * @param	CurrentStyle		the UIStyle that is being edited
	 * @param	InitialState		the state that should be initially active for the current style
	 */
	virtual void Create( wxWindow* InParent, wxWindowID InID, class UUIStyle* CurrentStyle, class UUIState* InitialState );

	/**
	 * Called to notify this property group that the style has been changed.  Refreshes the preview window with the style's current values
	 *
	 * @param	bPropagateStyleChanges	when TRUE the Style editor will propagate changes to the styles which derive 
	 *                                  from the editing style and refresh the widgets whose style has been changed.
	 *                                  flag set to FALSE will cause reinitialization of controls of this property group
	 */
	virtual void NotifyStyleModified( UBOOL bPropagateStyleChanges=TRUE );

	/**
	 * Applies any changes made in this property group to the style being edited.  Called when the user clicks the OK button.
	 */
	virtual void SaveChanges(){}

	/**
	 * Called to notify this property group that the user has selected a new UIStates to be edited.
	 *
	 * @param	SelectedStates	Array of states for which style data will be edited.
	 */
	virtual void NotifySelectedStatesChanged( TArray<UUIState*> & SelectedStates );

	/**
	* Disables editing of any properties inside this group
	*/
	virtual void DisableControls();

	/**
	 *  Removes any preview controls from the display, used to obtain the minimal set of controls needed to display all the properties
	 */
	virtual void RemovePreviewControls();

	/**
	 * Clears and refills the source state combo box will the states available in the currently selected source text style.
	 */
	void RefreshAvailableTextStates();

	/**
	 * Clears and refills the source state combo box with the states available in the currently selected source image style.
	 */
	void RefreshAvailableImageStates();

	/**
	 * Applies the values from the specified text style
	 */
	void ApplyTextStyle();

	/**
	 * Applies the values from the specified image style
	 */
	void ApplyImageStyle();

	/**
	* Returns the style data for the currently active state
	*/
	UUIStyle_Combo* GetCurrentStyleData() const;

	/**
	 * Populates the ComboStyle's NULL custom style data references with new default objects
	 */
	void CreateCustomStyleData(UUIStyle* ComboStyle);

	// FSerializableObject interface
	virtual void Serialize( FArchive& Ar );

	// Safely retrieves the values of the corresponding controls
	UUIStyle* GetTextStyle() const;
	UUIStyle* GetImageStyle() const;
	UUIState* GetTextState() const;
	UUIState* GetImageState() const;

	/** combo box for choosing the text style */
	wxStaticText*			lbl_TextStyle;
	wxRadioButton*			btn_TextExisting;
	wxRadioButton*			btn_TextCustom;
	WxUIChoice*				cmb_TextStyle;
	WxUIChoice*				cmb_TextState;
	
	/** panel with controls for editing custom text style data */
	WxStyleTextPropertiesGroup*		pnl_CustomTextProperties;

	/** combo box for choosing the image style */
	wxStaticText*			lbl_ImageStyle;
	wxRadioButton*			btn_ImageExisting;
	wxRadioButton*			btn_ImageCustom;
	WxUIChoice*				cmb_ImageStyle;
	WxUIChoice*				cmb_ImageState;

	/** panel with controls for editing custom text style data */
	WxStyleImagePropertiesGroup*		pnl_CustomImageProperties;

	/** preview panels */
	WxTextPreviewPanel*			pnl_TextStyle;
	WxImagePreviewPanel*		pnl_ImageStyle;

protected:

	/**
	 * Creates the controls and sizers for this panel.
	 */
	virtual void CreateControls();

	/**
	 * Creates the controls associated with text properties
	 *
	 *	@param TextSizer	sizer to which the newly created controls will be inserted
	 */
	void CreateTextControls( wxBoxSizer* const TextSizer );

	/**
	 * Creates the controls associated with image properties
	 *
	 *	@param ImageSizer	sizer to which the newly created controls will be inserted
	 */
	void CreateImageControls( wxBoxSizer* const ImageSizer );

	/**
	 * Initialize the values of the controls contained by this panel with the values from the current style.
	 */
	virtual void InitializeControlValues();

	/**
	 *	Initializes values of custom style data selection radio buttons
	 */
	void InitializeCustomTextRadioButtons();

	/**
	 *	Initializes values of custom style data selection radio buttons
	 */
	void InitializeCustomImageRadioButtons();

	/**
	 *	returns TRUE if all items in the styledata array have the same value of source text style property
	 */
	UBOOL CompareTextStyleProperty() const;

	/**
	 *	returns TRUE if all items in the StyleData array have the same value of source text state property
	 */
	UBOOL CompareTextStateProperty() const;

	/**
	 *	returns TRUE if all items in the styledata array have the same value of source image style property
	 */
	UBOOL CompareImageStyleProperty() const;

	/**
	 *	returns TRUE if all items in the StyleData array have the same value of source image state property
	 */
	UBOOL CompareImageStateProperty() const;

	/**
	 *	returns TRUE if all items in the StyleData array have the text custom style data enabled or if all have it disabled,
	 *  if the bEnabled flag toggles across states then this function will return FALSE
	 */
	UBOOL CompareCustomTextEnabledProperty() const;

	/**
	 *	returns TRUE if all items in the StyleData array have the image custom style data enabled or if all have it disabled,
	 *  if the bEnabled flag toggles across states then this function will return FALSE
	 */
	UBOOL CompareCustomImageEnabledProperty() const;

private:

	/** Style Data for the states currently selected */
	TArray<UUIStyle_Combo*> StyleDataArray;

	/* =======================================
		Windows event handlers.
	   =====================================*/

	/** called when the user selects a new style from one of the combos */
	void OnSelectedStyle( wxCommandEvent& Event );

	/**
	 * Called when the user changes the style radio buttons selection
	 */
	void OnRadioButtonPressed( wxCommandEvent& Event );


	/**
	 * Updates all currently selected States's Combo data with new source text style
	 *
	 * @param	SourceStyle		New source style for the text attribute
	 */
	void UpdateSourceTextStyle(UUIStyle* SourceStyle);
	
	/**
	* Updates all currently selected States's Combo data with new source text state
	*
	* @param	SourceState		New source state for the text attribute
	*/
	void UpdateSourceTextState(UUIState* SourceState);
	
	/**
	* Updates all currently selected States's Combo data with new source image style
	*
	* @param	SourceStyle		New source style for the image attribute
	*/
	void UpdateSourceImageStyle(UUIStyle* SourceStyle);
	
	/**
	* Updates all currently selected States's Combo data with new source image state
	*
	* @param	SourceState		New source state for the image attribute
	*/
	void UpdateSourceImageState(UUIState* SourceState);

	/**
	 * Enables or disables the use of the custom style data in TextStyle FStyleDataReference objects for currently selected states
	 */
	void EnableCustomTextStyleData(UBOOL Flag);

	/**
	 * Enables or disables the use of the custom style data in TextStyle FStyleDataReference objects for currently selected states
	 */
	void EnableCustomImageStyleData(UBOOL Flag);

};


/*
 * This dialog is shown when the user chooses to customize a UI style.  It allows the user to modify the unique tag, friendly name,
 * and any properties specific to the specified style type.  This dialog is designed to NOT be modal, and will take care of publishing
 * changes to the style.  It is safe, however, to call ShowModal on this dialog if you wish - this will prevent the user from being able
 * to select objects using the generic browser.
 */
class WxDlgEditUIStyle : public wxDialog, public FCallbackEventDevice
{
	DECLARE_DYNAMIC_CLASS(WxDlgEditUIStyle)

	DECLARE_EVENT_TABLE()

public:
	/**
	 * Constructor
	 */
    WxDlgEditUIStyle();

	/**
	 * Destructor
	 * Saves the window position and size to the ini
	 */

	~WxDlgEditUIStyle();

	/**
	 * Initialize this dialog.  Must be the first function called after creating the dialog.
	 * @todo - support for specifying the initial state to modify
	 *
	 * @param	EditStyle	the style being edited
	 * @param	InParent	the window that opened this dialog
	 * @param	InID		the ID to use for this dialog
	 */
    void Create( UUIStyle* EditStyle, wxWindow* InParent, wxWindowID InID=ID_UI_EDITSTYLE_DLG, long AdditionalButtonIds=0 );

	/**
	 * Notifies all control groups to update any preview windows.  Called when something which would affect the appearance
	 * of a style has been changed by the user.
	 *
	 * @param	bReinitializeControls	if TRUE, the property group controls will be reinitialized as well.
	 */
	void RefreshPanels(UBOOL bReinitializeControls=FALSE);

	/**
	 * Applies changes made in the dialog to the UIStyle being edited. 
	 */
	virtual void SaveChanges();

	/**
	* Function saves the current state of the edited style and all of its derived styles to the undo buffer
	*/
	void CreateUndoTransaction();

	/**
	* Function saves changes done on the working style back to the original style, and notifies
	* all objects about the style change
	*/
	void SaveChangesAndNotify();

	/**
	 * Delegate a StyleModifiedEvent from this StyleEditor instance.  Notifies all objects about 
	 * the change in the style this style editor is referencing.  This should be called whenever
	 * the style has been modified through this editor.  If there are any derived styles from this style
	 * same event will be generated for them as well, as they could change as a result of parent style change
	 */
	void SendStyleModifiedEvent();
		
	/** FCallbackEventDevice interface */
	virtual void Send( ECallbackEventType InType, UObject* InObject );

protected:
	
    /**
     * Creates the controls and sizers for this dialog.
     */
    void CreateControls( long AdditionalButtonIds=0 );

	/** Registers itself as an event listener */
	void RegisterCallbacks();

	/** Unregister itself from event listening */
	void UnregisterCallbacks();

	/**
	* Function stores or reloads current state of passed style and all styles derived from a reload archive 
	*
	* @param	CurrentStyle	Data for this style and all styles derived from it will be saved/reloaded
	* @param	Archive			Specifies which archive object to use in the store/reload process
	* @param	WriteFlag		If TRUE, process will save the state, if FALSE a reload will be performed
	*/
	void SerializeDerivedStyles( UUIStyle* const CurrentStyle, FReloadObjectArc* Archive, UBOOL WriteFlag);

	/** Function deletes the current archive and serializes derived styles into a newly create PropagatorArchive */
	void PreEditStyleProperty();

	/** Function serializes derived styles from existing PropagatorArchive */
	void PostEditStyleProperty();

	/** Function reverts the current style values and all of the derived style values to the previously saved originals */
	void RestoreOriginalStyleValues();

	/**
	 * Function reverts the data in the working style for the states which are unchecked to its archetype values. 
	 * If there is data which has been modified by the user which will be reverted, he will be prompted to
	 * confirm or cancel the revert process.
	 *
	 * @return	TRUE if revert process of unchecked states succeeded, or FALSE if it was stopped by the user. 
	 */
	UBOOL DeserializeUncheckedStates();
	
	
private:

	/** 
	 * An archive to store the original state of the style objects.
	 * Used to restore original style values when user cancels style changes
	 */
	FReloadObjectArc			OriginalStyleArchive;

	/** An archive used to serialize derived styles, used to propagate style changes to derived styles */
	FReloadObjectArc*			PropagatorArchive;

	/** the style being edited */
	UUIStyle*					Style;

	/** the group containing properties common to all UI styles */
	WxStyleCommonPropertiesGroup*		pnl_Common;

	/** the controls that contains properties specific to the style class being edited */
	WxStylePropertiesGroup*		pnl_Custom;

	/** the OK button */
	wxButton*					btn_OK;

	/** the cancel button */
	wxButton*					btn_Cancel;

	/**
	 * Called when the user clicks the OK/cancel button.
	 */
	void OnClick_CloseDialog( wxCommandEvent& Event );

	/**
	 * Called when the user clicks the checkbox next to a menu state.  Notifies the Combo style properties panel (if applicable).
	 */
	void OnMenuStateToggled( wxCommandEvent& Event );

	/**
	 * Called when the user selects a different style from the list of UIStyles available for this state.  Notifies all
	 * panels that the "CurrentState" has been modified.
	 */
	void OnMenuStateChanged( wxCommandEvent& Event );

	/**
	 * Called when the dialog is closed
	 */
	void OnCloseWindow( wxCloseEvent& Event );

	/**
	 *	Handler for external style modification event
	 *	Called when a style has been modified by another instance of style editor.
	 *	If the modified style is the one we are currently referencing, then we must update our data
	 */
	void UpdateReferencedStyle(UUIStyle * UpdatedStyle);
};


/**
 *	This class is used to visually compare differences in properties between two states of a same style,
    it displays properties for the both styles side by side in a read-only format
 */
class WxDlgDiffStateProperties : public wxDialog
{
	DECLARE_DYNAMIC_CLASS(WxDlgDiffStateProperties)

	DECLARE_EVENT_TABLE()

public:

	/**
	* Constructor
	*/
	WxDlgDiffStateProperties();

	/**
	* Destructor
	* Saves the window position and size to the ini
	*/
	~WxDlgDiffStateProperties();

	/**
	* Initialize this dialog.  Must be the first function called after creating the dialog.
	*
	* @param	ViewStyle			the style which states are to be compared
	* @param	FirstDiffState		first state to be compared
	* @param	SecondDiffState		second state to be compared
	* @param	InParent			the window that opened this dialog
	* @param	InID				the ID to use for this dialog
	*/
	void Create( UUIStyle* ViewStyle, UUIState* FirstDiffState, UUIState* SecondDiffState,
		wxWindow* InParent, wxWindowID InID=ID_UI_DIFFSTATES_DLG );

	/**
	 *	Creates panels for holding the controls for individual properties and adds the state controls to these panels
	 */
	void CreateDiffPanels();

	/**
	 * Adds the state panels and a divider between them to the window display
	 *
	 * @param	LayoutOrientation		specifies the layout of state panels to be either horizontal or vertical
	 */
	void SetupPanelLayout(wxOrientation LayoutOrientation);

	/**
	 *	Adds the button controls to the window
	 */
	void CreateButtonControls();


private:


	UUIStyle* Style;				// Style whose two states we are comparing

	UUIState* FirstState;			// First state of the ViewingStyle to be displayed

	UUIState* SecondState;			// Second state of the ViewingStyle to be displayed

	wxOrientation CurrentLayout;	// Specifies current layout of the state panels

	wxPanel* FirstStatePanel;		// Container panel holding controls for the first state

	wxPanel* SecondStatePanel;		// Container panel holding controls for the second state

	wxStaticLine* DividerLine;		// Divider to be placed between the two panels

	/**
	 * Creates a sizer with the controls specific to this state
	 */
	void CreateStateControls( wxWindow* ParentWindow, UUIState* StateToAdd);

	/** 
	 *	Event handler for switching layout modes
	 */
	void OnLayoutChanged( wxCommandEvent& Event);

};

#endif


