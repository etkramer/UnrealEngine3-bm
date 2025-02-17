/*=============================================================================
	UnUIStyleEditor.cpp: UI style editor window and control implementations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


// Includes
#include "UnrealEd.h"
#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UnUIStyleEditor.h"
#include "DlgUISkinEditor.h"
#include "ScopedTransaction.h"
#include "PropertyWindowManager.h"
#include "GenericBrowser.h"

/**
* Syncs the generic browser to the object specified by the given property.
*/
static void SyncToGenericBrowser(const FString& Name, UClass* Class)
{
	// Create a list of objects to sync the generic browser to.
	TArray<UObject*> Objects;
	UObject* Object = UObject::StaticFindObject( Class, ANY_PACKAGE, *Name, FALSE );

	if(Object)
	{
		Objects.AddItem(Object);
	}

	// Sync up the generic browser.
	WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
	if ( GenericBrowser )
	{
		// Make sure the window is visible.  The window needs to be visible *before*
		// the browser is sync'd to objects so that redraws actually happen!
		GUnrealEd->GetBrowserManager()->ShowWindow( GenericBrowser->GetDockID(), TRUE );

		// Sync.
		GenericBrowser->SyncToObjects( Objects );
	}
}

/* ==========================================================================================================
	WxTextPreviewPanel
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxTextPreviewPanel,wxPanel)
BEGIN_EVENT_TABLE(WxTextPreviewPanel,wxPanel)
	EVT_TEXT( ID_UI_EDITSTYLE_PREVIEW_TEXT, WxTextPreviewPanel::OnTextChanged )
	EVT_SIZE( WxTextPreviewPanel::OnSize )
END_EVENT_TABLE()

/**
 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
 * 
 * @param	InParent		the window that contains this control
 * @param	InID			the window ID to use for this control
 * @param	CurrentStyle	a copy of the style that is being edited
 * @param	CurrentState	the state that is being edited
 */
void WxTextPreviewPanel::Create( wxWindow* InParent, wxWindowID InID, UUIStyle* CurrentStyle, UUIState* CurrentState )
{
	SetupStyleData(CurrentStyle,CurrentState);

	PreviewString = ConstructObject<UUIPreviewString>(UUIPreviewString::StaticClass(), UUIScreenObject::StaticClass()->GetDefaultObject());
	PreviewString->SetCurrentMenuState(CurrentState);

	PreviewString->SetStringTextStyle(StyleData);
	PreviewString->SetValue(TEXT("The quick brown fox jumped over the lazy dog"),TRUE);

	verify(wxPanel::Create(InParent,InID,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL|wxCLIP_CHILDREN));
	CreateControls();
}

/**
 * Creates the controls and sizers for this panel.
 */
void WxTextPreviewPanel::CreateControls()
{
	// create the vert sizer
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer* PreviewBox = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUnrealEd("Preview"));
		{
			// create the text control
			txt_Preview = new WxTextCtrl( this, ID_UI_EDITSTYLE_PREVIEW_TEXT, *PreviewString->GetValue(TRUE) );
			PreviewBox->Add(txt_Preview, 0, wxGROW|wxALL, 5);

			// Create the preview viewport.
			pnl_Preview = new WxViewportHolder(this,ID_UI_EDITSTYLE_PREVIEW_PANEL,FALSE,wxDefaultPosition,wxDefaultSize,wxSUNKEN_BORDER|wxTAB_TRAVERSAL);
			{
				pnl_Preview->bAutoDestroy = TRUE;
				FViewport* Viewport = GEngine->Client->CreateWindowChildViewport(this,(HWND)pnl_Preview->GetHandle());
				Viewport->CaptureJoystickInput(FALSE);

				pnl_Preview->SetViewport(Viewport);
				pnl_Preview->SetSize(wxSize(-1,112));
			}
			PreviewBox->Add(pnl_Preview, 1, wxGROW|wxALL, 5);
		}
		BaseVSizer->Add(PreviewBox, 1, wxGROW|wxALL, 2);
	}
	SetSizer(BaseVSizer);
}

/**
 * 	Sets the value of StyleData appropriately based on weather we are dealing with an text style or a combo style
 */
void WxTextPreviewPanel::SetupStyleData( UUIStyle* CurrentStyle, UUIState* CurrentState )
{
	// if the style is the combo style then extract the image style from the combo 
	if(CurrentStyle->StyleDataClass == UUIStyle_Combo::StaticClass())
	{
		UUIStyle_Combo* ComboData = Cast<UUIStyle_Combo>(CurrentStyle->GetStyleForState(CurrentState));
		StyleData = ComboData->TextStyle;
	}
	else if(CurrentStyle->StyleDataClass == UUIStyle_Text::StaticClass()) 
	{
		// otherwise create a style data reference to the passed text style
		StyleData.SetSourceStyle(CurrentStyle);
		StyleData.SetSourceState(CurrentState);
	}	
}

/**
 *	Returns the current text data evaluated from the referenced StyleData
 */
UUIStyle_Text* WxTextPreviewPanel::GetCurrentTextData()
{
	UUIStyle_Text* ImageData = NULL;

	// If we are editing a combo style then data reference will contain a custom data so return custom data
	ImageData = Cast<UUIStyle_Text>(StyleData.GetCustomStyleData());

	if(ImageData == NULL)
	{
		// Otherwise this should return image data from the referenced image style
		ImageData = Cast<UUIStyle_Text>(StyleData.GetStyleData());
	}

	return ImageData;
}

/** Called when the user types text into the preview text control */
void WxTextPreviewPanel::OnTextChanged( wxCommandEvent& Event )
{
	if ( PreviewString != NULL && txt_Preview != NULL )
	{
		PreviewString->SetValue( txt_Preview->GetValue().c_str(), TRUE );
		RefreshViewport();
	}
}

void WxTextPreviewPanel::OnSize( wxSizeEvent& Event )
{
	if ( PreviewString != NULL && pnl_Preview != NULL && pnl_Preview->Viewport != NULL )
	{
		FRenderParameters Parameters(HorzPadding, VertPadding, pnl_Preview->Viewport->GetSizeX() - HorzPadding * 2, pnl_Preview->Viewport->GetSizeY() - VertPadding * 2, NULL, pnl_Preview->Viewport->GetSizeY());
		Parameters.TextAlignment[UIORIENT_Horizontal] = PreviewString->StringStyleData.TextAlignment[UIORIENT_Horizontal];
		Parameters.TextAlignment[UIORIENT_Vertical] = PreviewString->StringStyleData.TextAlignment[UIORIENT_Vertical];
		Parameters.Scaling = PreviewString->StringStyleData.TextScale;
		Parameters.SpacingAdjust = PreviewString->StringStyleData.TextSpacingAdjust;

		PreviewString->PreviewViewportSize = FVector2D(pnl_Preview->Viewport->GetSizeX(), pnl_Preview->Viewport->GetSizeY());
		PreviewString->ApplyFormatting(Parameters,TRUE);
	}

	Event.Skip();
}

/**
 * Forces the viewport to be redrawn
 */
void WxTextPreviewPanel::RefreshViewport()
{
	if ( pnl_Preview != NULL && pnl_Preview->Viewport != NULL )
	{
		PreviewString->PreviewViewportSize = FVector2D(pnl_Preview->Viewport->GetSizeX(), pnl_Preview->Viewport->GetSizeY());

		// assigning a new string value will reapply style changes to the nodes in the string
		PreviewString->RefreshNodeStyles();

		// this recalculates the extents for each of the nodes contained by the string
		FRenderParameters Parameters(HorzPadding, VertPadding, pnl_Preview->Viewport->GetSizeX() - HorzPadding * 2, pnl_Preview->Viewport->GetSizeY() - VertPadding * 2, NULL, pnl_Preview->Viewport->GetSizeY());
		Parameters.Scaling = PreviewString->StringStyleData.TextScale;
		Parameters.SpacingAdjust = PreviewString->StringStyleData.TextSpacingAdjust;
		Parameters.TextAlignment[UIORIENT_Horizontal] = PreviewString->StringStyleData.TextAlignment[UIORIENT_Horizontal];
		Parameters.TextAlignment[UIORIENT_Vertical] = PreviewString->StringStyleData.TextAlignment[UIORIENT_Vertical];

		PreviewString->ApplyFormatting(Parameters,TRUE);

		// this causes Draw to be called on the viewport
		pnl_Preview->Viewport->Invalidate();
	}
}

/**
 * Changes the style used by this preview panel.
 */
void WxTextPreviewPanel::SetPreviewStyle( UUIStyle* CurrentStyle, UUIState* CurrentState )
{
	SetupStyleData(CurrentStyle,CurrentState);
	
	UUIStyle_Text* TextStyleData = GetCurrentTextData();
	if ( TextStyleData != NULL )
	{
		PreviewString->SetCurrentMenuState(CurrentState);
		PreviewString->SetStringTextStyle(StyleData);
		RefreshViewport();
	}
}

/**
 * Draws the preview text in the viewport using the specified font
 *
 * @param Viewport the viewport being drawn into
 * @param RI the render interface to draw with
 */
void WxTextPreviewPanel::Draw(FViewport* Viewport, FCanvas* Canvas)
{
	// Erase with our background color
	Clear(Canvas,BackgroundColor);

	// And draw the text using the PreviewString
	const FLOAT LabelX = HorzPadding, LabelY = VertPadding, LabelX2 = Viewport->GetSizeX(), LabelY2 = Viewport->GetSizeY();
	const FLOAT LabelWidth = Viewport->GetSizeX() - HorzPadding * 2, LabelHeight = Viewport->GetSizeY() - VertPadding * 2;
	
	FRenderParameters Parameters(LabelX, LabelY, LabelWidth, LabelHeight, NULL, Viewport->GetSizeY());
	Parameters.TextAlignment[UIORIENT_Horizontal] = PreviewString->StringStyleData.TextAlignment[UIORIENT_Horizontal];
	Parameters.TextAlignment[UIORIENT_Vertical] = PreviewString->StringStyleData.TextAlignment[UIORIENT_Vertical];
	Parameters.Scaling = PreviewString->StringStyleData.TextScale;
	Parameters.SpacingAdjust = PreviewString->StringStyleData.TextSpacingAdjust;

	// here's where we calculate the bounding region that should be visible to our internal UIString.  Since each node of the UIString
	// may have its own alignment value, we must apply the overall alignment (as dictated by the label's style) to the bounding region
	// that we pass to the UIString's render function.  The UIString is aligned within the label's bounding region, then the UIString further
	// divides the available region into smaller chunks which are used to apply any per-node alignment.
	switch( Parameters.TextAlignment[UIORIENT_Horizontal] )
	{
	case UIALIGN_Left:
		// the entire label's bounding region is available to the string
		Parameters.DrawX = LabelX;
		Parameters.DrawXL = LabelWidth;
		break;

	case UIALIGN_Center:
		// reduce the bounding region to the width of the longest line of the UIString (the value of StringExtent.X)
		Parameters.DrawX = LabelX + (LabelWidth * 0.5f - PreviewString->StringExtent.X * 0.5f);
		Parameters.DrawXL = PreviewString->StringExtent.X;
		break;

	case UIALIGN_Right:
		// reduce the bounding region to the width of the longest line of the UIString (the value of StringExtent.X)
		Parameters.DrawX = LabelX2 - PreviewString->StringExtent.X;
		Parameters.DrawXL = LabelX2 - Parameters.DrawX;
		break;
	}

	switch( Parameters.TextAlignment[UIORIENT_Vertical] )
	{
	case UIALIGN_Left:
		// the entire label's bounding region is available to the string
		Parameters.DrawY = LabelY;
		Parameters.DrawYL = LabelHeight;
		break;

	case UIALIGN_Center:
		// reduce the bounding region to the combined height of all lines in the UIString (the value of StringExtent.Y)
		Parameters.DrawY = LabelY + (LabelHeight * 0.5f - PreviewString->StringExtent.Y * 0.5f);
		Parameters.DrawYL = PreviewString->StringExtent.Y;
		break;

	case UIALIGN_Right:
		// reduce the bounding region to the combined height of all lines in the UIString (the value of StringExtent.Y)
		Parameters.DrawY = LabelY2 - PreviewString->StringExtent.Y;
		Parameters.DrawYL = LabelY2 - Parameters.DrawY;
		break;
	}

	PreviewString->Render_String(Canvas, Parameters);
}

//FSerializableObject interface
void WxTextPreviewPanel::Serialize( FArchive& Ar )
{
	Ar << PreviewString;
}

/** Destructor */
WxTextPreviewPanel::~WxTextPreviewPanel()
{
}

/* ==========================================================================================================
	WxImagePreviewPanelBase
========================================================================================================== */

IMPLEMENT_ABSTRACT_CLASS(WxImagePreviewPanelBase,wxPanel)
BEGIN_EVENT_TABLE(WxImagePreviewPanelBase,wxPanel)
EVT_BUTTON(ID_PROP_PB_BROWSE,WxImagePreviewPanelBase::OnClick_Browse)
END_EVENT_TABLE()

/** Constructor */
WxImagePreviewPanelBase::WxImagePreviewPanelBase()
: txt_Preview(NULL), pnl_Preview(NULL), btn_Preview(NULL), btn_Assign(NULL)
, PreviewImage(NULL), HorzPadding(0), VertPadding(0), BackgroundColor(0)
{
}

/**
 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
 * 
 * @param	InParent				the window that contains this control
 * @param	InID					the window ID to use for this control
 * @param	InStyle					a copy of the style currently being edited
 * @param	CurrentState			the state that is being edited
 * @param	OwnerImageGroup			image property group which owns this panel
 */
void WxImagePreviewPanelBase::Create( wxWindow* InParent, wxWindowID InID, UUIStyle* CurrentStyle,
									 UUIState* CurrentState, class WxStyleImagePropertiesGroup* OwnerImageGroup /*=NULL*/)
{
	this->OwnerImageGroup = OwnerImageGroup; 

	SetupStyleData(CurrentStyle, CurrentState);

	verify(wxPanel::Create(InParent,InID,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL|wxCLIP_CHILDREN));

	CreateControls();

	PreviewImage = ConstructObject<UUITexture>(UUITexture::StaticClass());

	UUIStyle_Image* CurrentImageData = GetCurrentImageData();
	check(CurrentImageData);

	RenderCoordinates = CurrentImageData->Coordinates;
	PreviewImage->SetImageStyle(CurrentImageData);
}

/**
 *	Sets the value of StyleData appropriately based on wheather we are dealing with an image style or a combo style
 */
void WxImagePreviewPanelBase::SetupStyleData( UUIStyle* CurrentStyle, UUIState* CurrentState )
{
	// if the style is the combo style then extract the image style from the combo 
	if(CurrentStyle->StyleDataClass == UUIStyle_Combo::StaticClass())
	{
		UUIStyle_Combo* ComboData = Cast<UUIStyle_Combo>(CurrentStyle->GetStyleForState(CurrentState));
		StyleData = ComboData->ImageStyle;
	}
	else if(CurrentStyle->StyleDataClass == UUIStyle_Image::StaticClass())  // otherwise create a style data reference to the passed image style
	{
		StyleData.SetSourceStyle(CurrentStyle);
		StyleData.SetSourceState(CurrentState);
	}	
}

/**
 *	Returns the current image data evaluated from the referenced StyleData
 */
UUIStyle_Image* WxImagePreviewPanelBase::GetCurrentImageData()
{
	UUIStyle_Image* ImageData = NULL;

	// If we are editing a combo style then data reference will contain a custom data so return custom data
	ImageData = Cast<UUIStyle_Image>(StyleData.GetCustomStyleData());

	if(ImageData == NULL)
	{
		// Otherwise this should return image data from the referenced image style
		ImageData = Cast<UUIStyle_Image>(StyleData.GetStyleData());
	}

	return ImageData;
}

/**
 * Disables editing any properties through the controls
 */
void WxImagePreviewPanelBase::DisableControls()
{
	txt_Preview->Disable();
	if(pnl_Preview)
	{
		pnl_Preview->Disable();
	}
	btn_Preview->Disable();
	btn_Assign->Disable();
}

/** 
 * Displays the <multiple values> tag in the text label for the texture
 */
void WxImagePreviewPanelBase::DisplayMultipleValues()
{
	SetPreviewImage(NULL);

	if(txt_Preview)
	{
		txt_Preview->DisplayMultipleValues();
	}
}

/**
 * Sets the current texture to None.
 */
void WxImagePreviewPanelBase::SetImageToNone()
{
	// Set the selected texture to none.
	FName Name(NAME_None);
	txt_Preview->SetValue( *Name.ToString() );
	SetPreviewImage(NULL);

	// Notify the owner property group about texture change
	if(OwnerImageGroup)
	{
		OwnerImageGroup->ApplyDefaultTexture(NULL);
	}
}

/**
 *	Changes the style data used for displaying this image.
 */
void WxImagePreviewPanelBase::SetPreviewStyle( UUIStyle* CurrentStyle, UUIState* CurrentState )
{
	SetupStyleData( CurrentStyle, CurrentState );

	UUIStyle_Image* ImageStyleData = GetCurrentImageData();
	if ( ImageStyleData != NULL )
	{
		RenderCoordinates = ImageStyleData->Coordinates;
		PreviewImage->SetImageStyle(ImageStyleData);
		SetPreviewImage(ImageStyleData->DefaultImage);
	}
}

/**
 * Sets the preview image to the surface specified
 */
void WxImagePreviewPanelBase::SetPreviewImage( USurface* NewPreviewImage )
{
	PreviewImage->ImageTexture = NewPreviewImage;
	if ( PreviewImage != NULL && PreviewImage->GetSurface() == NewPreviewImage )
	{
		txt_Preview->SetValue( *NewPreviewImage->GetFullName() );
		RefreshViewport();
	}
}

/**
 * Gets the currently selected preview image
 */
USurface* WxImagePreviewPanelBase::GetPreviewImage() const
{
	return PreviewImage != NULL
		? PreviewImage->GetSurface()
		: NULL;
}


/**
 * Forces the viewport to be redrawn
 */
void WxImagePreviewPanelBase::RefreshViewport()
{
	if ( pnl_Preview != NULL && pnl_Preview->Viewport != NULL )
	{
		pnl_Preview->Viewport->Invalidate();
	}
}

/**
 * Draws the preview text in the viewport using the specified font
 *
 * @param Viewport the viewport being drawn into
 * @param RI the render interface to draw with
 */
void WxImagePreviewPanelBase::Draw(FViewport* Viewport, FCanvas* Canvas)
{
	// Erase with our background color
	//@todo - allow the user to choose the background color, or perhaps even some arbitrary bitmap/jpg
	Clear(Canvas,BackgroundColor);

	if ( PreviewImage != NULL )
	{
		USurface* Surface = PreviewImage->GetSurface();
		if ( Surface != NULL )
		{
			FRenderParameters Params( HorzPadding, VertPadding, Viewport->GetSizeX() - HorzPadding * 2, Viewport->GetSizeY() - VertPadding * 2, NULL, Viewport->GetSizeY() );

			Params.DrawCoords = RenderCoordinates;

			// change the style's state to match the current preview
			PreviewImage->Render_Texture(Canvas, Params);
		}
	}
}

/**
 * Event handler for when the sync to generic browser button has been clicked for locating the current preview image.
 *
 * @param	Event	Information about the event.
 */
void WxImagePreviewPanelBase::OnClick_Browse( wxCommandEvent& Event )
{
	if(PreviewImage)
	{
		SyncToGenericBrowser(PreviewImage->ImageTexture->GetPathName(), PreviewImage->ImageTexture->GetClass());
	}
}

// FSerializableObject interface
void WxImagePreviewPanelBase::Serialize( FArchive& Ar )
{
	Ar << PreviewImage;
}

/* ==========================================================================================================
	WxImagePreviewTextCtrl
========================================================================================================== */	
BEGIN_EVENT_TABLE(WxImagePreviewTextCtrl, WxTextCtrl)
	EVT_CHAR(WxImagePreviewTextCtrl::OnChar)
END_EVENT_TABLE()

WxImagePreviewTextCtrl::WxImagePreviewTextCtrl(WxDefaultImagePreviewPanel* InParent) : 
WxUITextCtrl(InParent, wxID_ANY, TEXT(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY),
Parent(InParent)
{

}

void WxImagePreviewTextCtrl::OnChar(wxKeyEvent &Event)
{
	switch(Event.GetKeyCode())
	{
	case WXK_BACK: case WXK_DELETE:
		{
			Parent->SetImageToNone();
		}
		break;
	default:
		break;
	}
}


/* ==========================================================================================================
	WxDefaultImagePreviewPanel
========================================================================================================== */	
IMPLEMENT_DYNAMIC_CLASS(WxDefaultImagePreviewPanel,WxImagePreviewPanelBase)
BEGIN_EVENT_TABLE(WxDefaultImagePreviewPanel,WxImagePreviewPanelBase)
	EVT_BUTTON(ID_PROP_PB_USE,WxDefaultImagePreviewPanel::OnClick_UseSelected)
END_EVENT_TABLE()

/**
 * Creates the controls and sizers for this panel.
 */
void WxDefaultImagePreviewPanel::CreateControls()
{
	HorzPadding = VertPadding = 0;
	wxBoxSizer* BaseHSizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(BaseHSizer);

	wxBoxSizer* TextVSizer = new wxBoxSizer(wxVERTICAL);
	BaseHSizer->Add(TextVSizer, 1, wxALIGN_BOTTOM|wxLEFT|wxRIGHT, 5);

	lbl_Preview = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesImage_Label_DefaultTexture")) );
	TextVSizer->Add(lbl_Preview, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

	wxBoxSizer* TextHSizer = new wxBoxSizer(wxHORIZONTAL);
	TextVSizer->Add(TextHSizer, 0, wxGROW|wxALL, 0);

	txt_Preview = new WxImagePreviewTextCtrl(this);
	TextHSizer->Add(txt_Preview, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

	btn_Assign = new wxBitmapButton( this, ID_PROP_PB_USE, GPropertyWindowManager->Prop_UseCurrentBrowserSelectionB, wxDefaultPosition, wxSize(24,24), wxBU_AUTODRAW|wxBU_EXACTFIT );
	TextHSizer->Add(btn_Assign, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

	btn_Preview = new wxBitmapButton( this, ID_PROP_PB_BROWSE, GPropertyWindowManager->Prop_ShowGenericBrowserB, wxDefaultPosition, wxSize(24,24), wxBU_AUTODRAW|wxBU_EXACTFIT );
	TextHSizer->Add(btn_Preview, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

	pnl_Preview = new WxViewportHolder( this, wxID_ANY, FALSE, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
	pnl_Preview->bAutoDestroy = TRUE;
	FViewport* Viewport = GEngine->Client->CreateWindowChildViewport(this,(HWND)pnl_Preview->GetHandle());
	Viewport->CaptureJoystickInput(FALSE);

	pnl_Preview->SetViewport(Viewport);
	pnl_Preview->SetSize(wxSize(48, 48));

	BaseHSizer->Add(pnl_Preview, 0, wxALIGN_BOTTOM|wxRIGHT|wxTOP, 5);
}


/**
* Changes the style data used by this preview panel.  DefaultImagePreview Panel will only use default image style settings and not actual values
*/
void WxDefaultImagePreviewPanel::SetPreviewStyle( UUIStyle* CurrentStyle, UUIState* CurrentState )
{
	SetupStyleData( CurrentStyle, CurrentState );

	UUIStyle_Image* ImageStyleData = GetCurrentImageData();
	if ( ImageStyleData != NULL )
	{
		// Use default image style style settings
		PreviewImage->SetImageStyle(Cast<UUIStyle_Image>(UUIStyle_Image::StaticClass()->GetDefaultObject()));

		// But use the underlying style's texture
		SetPreviewImage(ImageStyleData->DefaultImage);
	}
}

void WxDefaultImagePreviewPanel::OnClick_UseSelected( wxCommandEvent& Event )
{
	USurface* SelectedImage = GEditor->GetSelectedObjects()->GetTop<USurface>();
	if ( SelectedImage != NULL )
	{
		txt_Preview->SetValue( *SelectedImage->GetPathName() );
		SetPreviewImage(SelectedImage);

		// Notify the owner property group about texture change
		if(OwnerImageGroup)
		{
			OwnerImageGroup->ApplyDefaultTexture(SelectedImage);
		}
	}
}

/* ==========================================================================================================
	WxImagePreviewPanel
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxImagePreviewPanel,WxImagePreviewPanelBase)
BEGIN_EVENT_TABLE(WxImagePreviewPanel,WxImagePreviewPanelBase)
	EVT_BUTTON(ID_PROP_PB_USE,WxImagePreviewPanel::OnClick_UseSelected)
END_EVENT_TABLE()

/**
 * Creates the controls and sizers for this panel.
 */
void WxImagePreviewPanel::CreateControls()
{
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer* PreviewSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUnrealEd("Preview"));
		{
			// Textbox
			wxBoxSizer* TextHSizer = new wxBoxSizer(wxHORIZONTAL);
			{
				txt_Preview = new WxUITextCtrl( this, wxID_ANY, TEXT(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
				TextHSizer->Add(txt_Preview, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

				btn_Assign = new wxBitmapButton( this, ID_PROP_PB_USE, GPropertyWindowManager->Prop_UseCurrentBrowserSelectionB, wxDefaultPosition, wxSize(24,24), wxBU_AUTODRAW|wxBU_EXACTFIT );
				TextHSizer->Add(btn_Assign, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

				btn_Preview = new wxBitmapButton( this, ID_PROP_PB_BROWSE, GPropertyWindowManager->Prop_ShowGenericBrowserB, wxDefaultPosition, wxSize(24,24), wxBU_AUTODRAW|wxBU_EXACTFIT );
				TextHSizer->Add(btn_Preview, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
			}
			PreviewSizer->Add(TextHSizer, 0, wxGROW|wxALL, 0);


			// Preview Viewport
			pnl_Preview = new WxViewportHolder( this, wxID_ANY, FALSE, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
			{
				pnl_Preview->bAutoDestroy = TRUE;
				FViewport* Viewport = GEngine->Client->CreateWindowChildViewport(this,(HWND)pnl_Preview->GetHandle());
				Viewport->CaptureJoystickInput(FALSE);

				pnl_Preview->SetViewport(Viewport);
				pnl_Preview->SetSize(wxSize(-1, 48));
			}
			PreviewSizer->Add(pnl_Preview, 1, wxGROW|wxALL, 5);
		}
		BaseVSizer->Add(PreviewSizer, 1, wxEXPAND | wxALL, 2);
	}
	SetSizer(BaseVSizer);

	
}

void WxImagePreviewPanel::OnClick_UseSelected( wxCommandEvent& Event )
{
	USurface* SelectedSurface = GEditor->GetSelectedObjects()->GetTop<USurface>();
	if ( SelectedSurface != NULL )
	{
		txt_Preview->SetValue( *SelectedSurface->GetPathName() );
		SetPreviewImage(SelectedSurface);
	}
}



/* ==========================================================================================================
	WxColorAdjustmentPanel
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxColorAdjustmentPanel,wxPanel)
BEGIN_EVENT_TABLE(WxColorAdjustmentPanel,wxPanel)
	EVT_BUTTON(ID_UI_EDITSTYLE_COLOR,WxColorAdjustmentPanel::OnClick_ChangeColor)
	EVT_COMMAND_SCROLL(ID_UI_EDITSTYLE_ALPHA, WxColorAdjustmentPanel::OnScroll_ChangeAlpha)
END_EVENT_TABLE()

/** Default constructor for use in two-stage dynamic window creation */
WxColorAdjustmentPanel::WxColorAdjustmentPanel(): btn_Color(NULL), lbl_AlphaSlider(NULL), sldr_AlphaSlider(NULL), OwnerPropertyGroup(NULL)
{
}

/**
 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
 * 
 * @param	InParent			the window that contains this control
 * @param	InID				the window ID to use for this control
 * @param	InitialData			the data that will be used to first initialize controls, cannot be NULL
 * @param	OwnerPropertyGroup  the style property group which contains this panel
 */
void WxColorAdjustmentPanel::Create( wxWindow* InParent, wxWindowID InID, class UUIStyle_Data* InitialData, class WxStylePropertiesGroup* OwnerPropertyGroup )
{
	check(InitialData);
	check(OwnerPropertyGroup);

	verify(wxPanel::Create(InParent,InID,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL));

	this->OwnerPropertyGroup = OwnerPropertyGroup;

	// Initialize the StyleDataArray with the data for the initial state
	StyleDataArray.AddItem(InitialData);

    CreateControls();
}

/**
 * Creates the controls and sizers for this panel.
 */
void WxColorAdjustmentPanel::CreateControls()
{
	wxBoxSizer* MainHSizer = new wxBoxSizer(wxHORIZONTAL);	
	SetSizer(MainHSizer);
	{
		// Vertical Sizer for the color Button and a label
		wxBoxSizer* ColorVSizer = new wxBoxSizer(wxVERTICAL);
		MainHSizer->Add(ColorVSizer, 0, wxALIGN_TOP|wxALL, 0);
		{
			// add the "Color:" label
			wxStaticText* lbl_StyleColor = new wxStaticText(this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesCommon_Label_StyleColor")));
			ColorVSizer->Add(lbl_StyleColor, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 3);

			// add a spacer
			ColorVSizer->Add(3, 4, 0, wxALIGN_CENTER_HORIZONTAL, 0);

			// add the button that displays the selected color
			btn_Color = new WxUIButton( this, ID_UI_EDITSTYLE_COLOR, TEXT(""), wxDefaultPosition, wxSize(140,-1), 0 );
			btn_Color->SetHelpText(*LocalizeUI(TEXT("StylePropertiesCommon_HelpText_StyleColor")));
			btn_Color->SetToolTip(*LocalizeUI(TEXT("StylePropertiesCommon_ToolTip_StyleColor")));
			ColorVSizer->Add(btn_Color, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM|wxFIXED_MINSIZE, 3);
		}


		// Vertical Sizer for the color Button and a label
		wxBoxSizer* AlphaVSizer = new wxBoxSizer(wxVERTICAL);
		MainHSizer->Add(AlphaVSizer, 0, wxGROW|wxALL, 0);
		{
			// add "Alpha:" label
			wxStaticText* lbl_AlphaValue = new wxStaticText(this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesCommon_Label_AlphaValue")));
			AlphaVSizer->Add(lbl_AlphaValue, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 3);

			// add the slider for selecting alpha value
			sldr_AlphaSlider = new wxSlider( this, ID_UI_EDITSTYLE_ALPHA, 100, 0, 300, wxDefaultPosition, wxSize(140,-1), wxSL_HORIZONTAL|wxSL_LABELS|wxSL_TOP|wxRAISED_BORDER );
			AlphaVSizer->Add(sldr_AlphaSlider, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP|wxFIXED_MINSIZE, 3);
		}
	}
}

/**
 * Initialize the values of the controls contained by this panel with the values from the current style.
 */
void WxColorAdjustmentPanel::InitializeControlValues()
{
	check(StyleDataArray.Num() > 0);

	FLinearColor CurrentColor = StyleDataArray(0)->StyleColor;

	// Set color button's background color to the current state's color
	if(CompareColorProperty())
	{
		// Clear the multiple values label
		btn_Color->ClearMultipleValues();

		// Convert from FLinearColor to FColor to obtain color values ranging from 0-255
		FColor ConvertedColor = CurrentColor;
		btn_Color->SetBackgroundColour( wxColour(ConvertedColor.R,ConvertedColor.G,ConvertedColor.B) );
	}
	else
	{
		btn_Color->DisplayMultipleValues();
	}

	// Compute the value of alpha scaled between 0-100
	INT Alpha = 100 * CurrentColor.A;
	check(Alpha >= 0);

	// Initialize the value of the alpha slider
	sldr_AlphaSlider->SetValue(Alpha);
}

/**
 *	Notifies owner style editor that property has been modified
 */
void WxColorAdjustmentPanel::NotifyPropertyChanged()
{
	if(OwnerPropertyGroup)
	{
		OwnerPropertyGroup->NotifyStyleModified(TRUE);
	}	
}

/**
 * Called when the user selects a new menu state or states from the list. Updates internal
 * array of data that will be modified through this panel. Reinitializes all
 * control values with the style data from the first item in the passed array 
 *
 * @param	SelectedStatesData		Array of data from the currently selected states
 */
void WxColorAdjustmentPanel::NotifySelectedStatesChanged( const TArray< UUIStyle_Data* > & SelectedStatesData )
{	
	// Update current StyleDataArray to hold currently selected data by making a shallow copy of the passed array
	StyleDataArray = SelectedStatesData;

	// If no styles are selected, then controls will retain their old values
	if(SelectedStatesData.Num() > 0)
	{
		InitializeControlValues();
	}	
}

/**
 * returns TRUE is color values for all the selected states are the same	
 */
UBOOL WxColorAdjustmentPanel::CompareColorProperty() const
{
	UBOOL Result = TRUE;

	if(StyleDataArray.Num() > 0)
	{
		FLinearColor FirstColor = StyleDataArray(0)->StyleColor;
		FLinearColor CurrentColor; 

		// Compare each state color data against the first state's data 
		for(TIndexedContainerConstIterator< TArray<UUIStyle_Data*> > It(StyleDataArray); It; ++It)
		{
			CurrentColor = (*It)->StyleColor;
			if( CurrentColor.R !=  FirstColor.R || CurrentColor.G !=  FirstColor.G || CurrentColor.B !=  FirstColor.B)
			{
				Result = FALSE;
				break;
			}
		}
	}
	return Result;
}

/**
 * Pops up a color picker for choosing the style color, then notifies all groups that the color has been changed.
 * Called when the user presses the "choose color" button.
 */
void WxColorAdjustmentPanel::OnClick_ChangeColor( wxCommandEvent& Event )
{
	if(StyleDataArray.Num() > 0)
	{
		// Get the color currently displayed in the style editor which is the color from the first item in the style data array
		UUIStyle_Data* StyleData = StyleDataArray(0);
		check(StyleData);

		// Prompt the user for a new color
		FColor NewColor = SelectColor(StyleData->StyleColor);

		// Change the button's background color to the selected color
		btn_Color->ClearMultipleValues();
		btn_Color->SetBackgroundColour( wxColour(NewColor.R,NewColor.G,NewColor.B));
			
		UpdateColorProperty(NewColor);
		NotifyPropertyChanged();
	}
}

/**
 * Common routine for letting the user select a color choice.  Copied from WxFontPropertiesDlg::WxFontPreviewDlg
 *
 * @param Color the current value of the color that is being changed
 *
 * @return The newly selected color or the original if no change
 */
FColor WxColorAdjustmentPanel::SelectColor(FColor Color)
{
	wxColourData Data;
	Data.SetChooseFull(TRUE);

	wxColour CurrentColor = wxColour(Color.R,Color.G,Color.B);
	Data.SetColour(CurrentColor);

	// Set all 16 custom colors to our color
	for (INT Index = 0; Index < 16; Index++)
	{
		Data.SetCustomColour(Index,CurrentColor);
	}
	// Create the dialog with our custom color set
	wxColourDialog Dialog(this,&Data);
	if (Dialog.ShowModal() == wxID_OK)
	{
		// Get the data that was changed
		wxColourData OutData = Dialog.GetColourData();
		// Now get the resultant color
		wxColour OutColor = OutData.GetColour();
		// Convert to our data type
		Color.R = OutColor.Red();
		Color.G = OutColor.Green();
		Color.B = OutColor.Blue();
	}
	return Color;
}

/**
 * Updates all currently selected States's data for with the RGB values of the new color
 *
 * @param	Color	source color for the RGB values
 */
void WxColorAdjustmentPanel::UpdateColorProperty( const FLinearColor & Color )
{
	// Iterate over all currently selected state data updating each
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Data* StateData = StyleDataArray(StateIndex);
		StateData->StyleColor.R = Color.R;
		StateData->StyleColor.G = Color.G;
		StateData->StyleColor.B = Color.B;
		StateData->SetDirtiness(TRUE);
	}
}

/**
 *	Handler for the alpha slider scroll event, sets the currently selected states color alpha value to the value of the slider
 */
void WxColorAdjustmentPanel::OnScroll_ChangeAlpha( wxScrollEvent& Event )
{
	// Obtain the current alpha value from the slider
	INT Alpha = sldr_AlphaSlider->GetValue();

	// Convert the slider percentage value into proper 0.0-1.0f fraction
	check(Alpha >= 0);
	FLOAT ConvertedAlpha = Alpha/100.0f;

	UpdateAlphaProperty(ConvertedAlpha);
	NotifyPropertyChanged();
}

/**
 * Updates all currently selected States's data with the new alpha value
 *
 * @param	Alpha	alpha value between 0.0 - 1.0
 */
void WxColorAdjustmentPanel::UpdateAlphaProperty( FLOAT Alpha )
{
	check( Alpha >= 0.0f );

	// Iterate over all currently selected state data updating each
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Data* StateData = StyleDataArray(StateIndex);
		StateData->StyleColor.A = Alpha;
		StateData->SetDirtiness(TRUE);
	}
}



/* ==========================================================================================================
	WxImageAdjustmentPanel
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxImageAdjustmentPanel,wxPanel)
BEGIN_EVENT_TABLE(WxImageAdjustmentPanel,wxPanel)
	EVT_CHOICE(ID_UI_EDITSTYLE_COMBO_ALIGNMENT,WxImageAdjustmentPanel::OnSelectionChanged)
	EVT_CHOICE(ID_UI_EDITSTYLE_COMBO_ADJUSTMENT,WxImageAdjustmentPanel::OnSelectionChanged)
	EVT_SPINCTRL(ID_UI_EDITSTYLE_SPIN_GUTTER_H,WxImageAdjustmentPanel::OnGutterTextChanged)
	EVT_SPINCTRL(ID_UI_EDITSTYLE_SPIN_GUTTER_V,WxImageAdjustmentPanel::OnGutterTextChanged)
	EVT_SPINCTRL(ID_UI_EDITSTYLE_SPIN_ATLASCOORDS_ORIGIN,WxImageAdjustmentPanel::OnCoordinatesTextChanged)
	EVT_SPINCTRL(ID_UI_EDITSTYLE_SPIN_ATLASCOORDS_EXTENT,WxImageAdjustmentPanel::OnCoordinatesTextChanged)
	EVT_SPINCTRL(ID_UI_EDITSTYLE_IMAGEPADDING_H,WxImageAdjustmentPanel::OnPaddingChanged)
	EVT_SPINCTRL(ID_UI_EDITSTYLE_IMAGEPADDING_V,WxImageAdjustmentPanel::OnPaddingChanged)
	EVT_TEXT_ENTER(ID_UI_EDITSTYLE_SPIN_GUTTER_H,WxImageAdjustmentPanel::OnGutterTextChanged)
	EVT_TEXT_ENTER(ID_UI_EDITSTYLE_SPIN_GUTTER_V,WxImageAdjustmentPanel::OnGutterTextChanged)
	EVT_TEXT_ENTER(ID_UI_EDITSTYLE_SPIN_ATLASCOORDS_ORIGIN,WxImageAdjustmentPanel::OnCoordinatesTextChanged)
	EVT_TEXT_ENTER(ID_UI_EDITSTYLE_SPIN_ATLASCOORDS_EXTENT,WxImageAdjustmentPanel::OnCoordinatesTextChanged)
	EVT_TEXT_ENTER(ID_UI_EDITSTYLE_IMAGEPADDING_H,WxImageAdjustmentPanel::OnPaddingChanged)
	EVT_TEXT_ENTER(ID_UI_EDITSTYLE_IMAGEPADDING_V,WxImageAdjustmentPanel::OnPaddingChanged)
END_EVENT_TABLE()
/** Default constructor for use in two-stage dynamic window creation */
WxImageAdjustmentPanel::WxImageAdjustmentPanel()
: lbl_Alignment(NULL), cmb_Alignment(NULL)
, lbl_Adjustment(NULL), cmb_Adjustment(NULL)
, lbl_Padding(NULL), spin_Padding(NULL)
, Orientation(UIORIENT_MAX)
{
	for ( INT i = 0; i < UIORIENT_MAX; i++ )
	{
		lbl_Gutter[i] = NULL;
		spin_Gutter[i] = NULL;
	}

	for ( INT i = 0; i < 2; i++ )
	{
		lbl_TextureCoordinates[i] = NULL;
		spin_TextureCoordinates[i] = NULL;
	}
}

/**
 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
 * 
 * @param	InParent					the window that contains this control
 * @param	InID						the window ID to use for this control
 * @param	InOrientation				the orientation to use for this panel.  this controls the text that is placed in the various labels
 * @param   OwnerImagePropertiesGroup	image properties group which will be notified about the property change
 * @param	CurrentStyle				a copy of the style being edited
 * @param	CurrentStyle				the style being edited
 */
void WxImageAdjustmentPanel::Create( wxWindow* InParent, wxWindowID InID, EUIOrientation InOrientation, class WxStyleImagePropertiesGroup* OwnerImagePropertiesGroup, UUIStyle* CurrentStyle, UUIState* CurrentState )
{
	check(CurrentStyle);
	check(CurrentState);
	check(OwnerImagePropertiesGroup);

	this->EditingStyle = CurrentStyle;
	this->CurrentState = CurrentState;
	this->OwnerImagePropertiesGroup = OwnerImagePropertiesGroup;
	Orientation = InOrientation;

	UUIStyle_Image* ImageData = OwnerImagePropertiesGroup->GetImageDataForState( CurrentStyle, CurrentState );
	check(ImageData);

	StyleDataArray.AddItem(ImageData);	

	verify(wxPanel::Create(InParent,InID,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL));

	CreateControls();
}

/**
* Disables editing of any properties inside this group
*/
void WxImageAdjustmentPanel::DisableControls()
{
	cmb_Alignment->Disable();
	cmb_Adjustment->Disable();
	spin_Padding->Disable();

	for ( INT i = 0; i < UIORIENT_MAX; i++ )
	{
		spin_Gutter[i]->Disable();
	}

	for ( INT i = 0; i < 2; i++ )
	{
		spin_TextureCoordinates[i]->Disable();
	}
}
/**
 * Creates the controls and sizers for this panel.
 */
void WxImageAdjustmentPanel::CreateControls()
{
	wxStaticBox* StaticBox = new wxStaticBox(this, wxID_ANY, *LocalizeUI(*FString::Printf(TEXT("ImageAlignmentCaption[%d]"), Orientation)));
	wxStaticBoxSizer* StaticVSizer = new wxStaticBoxSizer(StaticBox, wxVERTICAL);
	SetSizer(StaticVSizer);

	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	StaticVSizer->Add(BaseVSizer, 1, wxGROW|wxALL, 0);

	FString tmpString = *LocalizeUI( *FString::Printf(TEXT("StylePropertiesText_Alignment[%d]"), Orientation) );
	TArray<FString> tmpOptions;

	wxBoxSizer* AlignmentVSizer = new wxBoxSizer(wxVERTICAL);
	BaseVSizer->Add(AlignmentVSizer, 1, wxGROW|wxALL, 0);
	{
		tmpString.ParseIntoArray(&tmpOptions,TEXT(","),TRUE);

		// alignment
		lbl_Alignment = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("ImageAlignment_Label_Alignment")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
		AlignmentVSizer->Add(lbl_Alignment, 0, wxGROW|wxTOP|wxADJUST_MINSIZE, 5);

		wxArrayString AlignmentOptions;
		for ( INT i = 0; i < tmpOptions.Num(); i++ )
		{
			AlignmentOptions.Add(*tmpOptions(i));
		}
		cmb_Alignment = new WxUIChoice( this, ID_UI_EDITSTYLE_COMBO_ALIGNMENT, wxDefaultPosition, wxDefaultSize, AlignmentOptions );
		cmb_Alignment->SetSelection(0);
		AlignmentVSizer->Add(cmb_Alignment, 0, wxGROW|wxBOTTOM, 5);

		wxBoxSizer* AdjustmentVSizer = new wxBoxSizer(wxVERTICAL);
		BaseVSizer->Add(AdjustmentVSizer, 1, wxGROW|wxALL, 0);

		lbl_Adjustment = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("ImageAlignment_Label_Adjustment")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
		AdjustmentVSizer->Add(lbl_Adjustment, 0, wxGROW|wxTOP|wxADJUST_MINSIZE, 5);

		// build the list of MaterialAdjustmentTypes using the metadata
		wxArrayString cmb_AdjustmentStrings;
		static UEnum* ImageAdjustmentEnum = FindObjectChecked<UEnum>(ANY_PACKAGE, TEXT("Engine.UIRoot:EMaterialAdjustmentType"), TRUE);
		for ( INT EnumIndex = 0; EnumIndex < ImageAdjustmentEnum->NumEnums(); EnumIndex++ )
		{
			cmb_AdjustmentStrings.Add(*ImageAdjustmentEnum->GetMetaData(TEXT("DisplayName"), EnumIndex));
		}
		cmb_Adjustment = new WxUIChoice( this, ID_UI_EDITSTYLE_COMBO_ADJUSTMENT, wxDefaultPosition, wxDefaultSize, cmb_AdjustmentStrings );
		cmb_Adjustment->SetSelection(1);
		AdjustmentVSizer->Add(cmb_Adjustment, 0, wxGROW|wxBOTTOM, 5);
	}

	// padding
	wxBoxSizer* PaddingVSizer = new wxBoxSizer(wxVERTICAL);
	BaseVSizer->Add(PaddingVSizer, 1, wxGROW|wxALL, 0);
	{
		lbl_Padding = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesCommon_Label_Padding")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
		PaddingVSizer->Add( lbl_Padding, 0, wxGROW|wxTOP|wxADJUST_MINSIZE, 5);

		spin_Padding = new WxUISpinCtrl(this, ID_UI_EDITSTYLE_IMAGEPADDING_H + Orientation, 0.0f, wxDefaultPosition, wxDefaultSize, -1000.0f, 1000.0f);
		spin_Padding->SetToolTip( Orientation == UIORIENT_Horizontal ? *LocalizeUI(TEXT("StylePropertiesCommon_ToolTip_PaddingH")) : *LocalizeUI(TEXT("StylePropertiesCommon_ToolTip_PaddingV")));
		PaddingVSizer->Add(spin_Padding, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);
	}

	{
		// gutter
		wxFlexGridSizer* GutterSizer = new wxFlexGridSizer(2, 2, 0, 0);
		GutterSizer->AddGrowableCol(0, 1);
		GutterSizer->AddGrowableCol(1, 1);
		BaseVSizer->Add(GutterSizer, 0, wxGROW|wxALL, 0);

		tmpString = LocalizeUI( *FString::Printf(TEXT("ImageAlignment_Label_Gutter[%d]"), Orientation) );
		tmpOptions.Empty();
		tmpString.ParseIntoArray(&tmpOptions, TEXT(","), TRUE);
		check(tmpOptions.Num() == UIORIENT_MAX);

		lbl_Gutter[UIORIENT_Horizontal] = new wxStaticText( this, wxID_STATIC, *tmpOptions(UIORIENT_Horizontal), wxDefaultPosition, wxDefaultSize, 0 );
		GutterSizer->Add(lbl_Gutter[UIORIENT_Horizontal], 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxTOP|wxADJUST_MINSIZE, 5);

		lbl_Gutter[UIORIENT_Vertical] = new wxStaticText( this, wxID_STATIC, *tmpOptions(UIORIENT_Vertical), wxDefaultPosition, wxDefaultSize, 0 );
		GutterSizer->Add(lbl_Gutter[UIORIENT_Vertical], 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxTOP|wxADJUST_MINSIZE, 5);

		spin_Gutter[UIORIENT_Horizontal] = new WxUISpinCtrl( this, ID_UI_EDITSTYLE_SPIN_GUTTER_H, 0, wxDefaultPosition, wxDefaultSize, 0, 1024 );
		GutterSizer->Add(spin_Gutter[UIORIENT_Horizontal], 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);

		spin_Gutter[UIORIENT_Vertical] = new WxUISpinCtrl( this, ID_UI_EDITSTYLE_SPIN_GUTTER_V, 0, wxDefaultPosition, wxDefaultSize, 0, 1024 );
		GutterSizer->Add(spin_Gutter[UIORIENT_Vertical], 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT, 5);
	}

	{
		// coordinates
		wxFlexGridSizer* CoordinatesSizer = new wxFlexGridSizer(2, 2, 0, 0);
		CoordinatesSizer->AddGrowableCol(0, 1);
		CoordinatesSizer->AddGrowableCol(1, 1);
		BaseVSizer->Add(CoordinatesSizer, 0, wxGROW|wxALL, 0);

		// cannot use a for loop for this, since the order in which the controls are added to the grid sizer is important.
		lbl_TextureCoordinates[0] = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("ImageCoordinates_Label[0]")), wxDefaultPosition, wxDefaultSize, 0 );
		CoordinatesSizer->Add(lbl_TextureCoordinates[0], 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

		lbl_TextureCoordinates[1] = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("ImageCoordinates_Label[1]")), wxDefaultPosition, wxDefaultSize, 0 );
		CoordinatesSizer->Add(lbl_TextureCoordinates[1], 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

		spin_TextureCoordinates[0] = new WxUISpinCtrl( this, ID_UI_EDITSTYLE_SPIN_ATLASCOORDS_ORIGIN, 0, wxDefaultPosition, wxDefaultSize, -8192, 8192 );
		CoordinatesSizer->Add(spin_TextureCoordinates[0], 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);

		spin_TextureCoordinates[1] = new WxUISpinCtrl( this, ID_UI_EDITSTYLE_SPIN_ATLASCOORDS_EXTENT, 0, wxDefaultPosition, wxDefaultSize, -8192, 8192 );
		CoordinatesSizer->Add(spin_TextureCoordinates[1], 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT, 5);
	}
}

void WxImageAdjustmentPanel::InitializeControlValues()
{
	InitializeAlignmentCombo();
	InitializeAdjustmentCombo();
	InitializePaddingSpinner();
	InitializeSpinGutters();
	InitializeSpinTextureCoords();
}

/** Initializes the alignment combo */
void WxImageAdjustmentPanel::InitializeAlignmentCombo()
{
	if(CompareAlignmentProperty())
	{
		cmb_Alignment->ClearMultipleValues();
		UUIStyle_Image* CurrentData = OwnerImagePropertiesGroup->GetCurrentStyleData();
		SetAlignment((EUIAlignment)CurrentData->AdjustmentType[Orientation].Alignment);
	}
	else
	{
		cmb_Alignment->DisplayMultipleValues();
	}
}


/** Initializes the adjustment combo */
void WxImageAdjustmentPanel::InitializeAdjustmentCombo()
{
	if(CompareAdjustmentProperty())
	{
		cmb_Adjustment->ClearMultipleValues();
		UUIStyle_Image* CurrentData = OwnerImagePropertiesGroup->GetCurrentStyleData();
		SetAdjustment((EMaterialAdjustmentType)CurrentData->AdjustmentType[Orientation].AdjustmentType);
	}
	else
	{
		cmb_Adjustment->DisplayMultipleValues();
	}
}

/** populates the value of the padding spinner with the values from the currently selected states */
void WxImageAdjustmentPanel::InitializePaddingSpinner()
{
	if ( ComparePaddingProperty() )
	{
		spin_Padding->ClearMultipleValues();

		UUIStyle_Image* CurrentData = OwnerImagePropertiesGroup->GetCurrentStyleData();
		spin_Padding->SetValue( CurrentData->StylePadding[Orientation] );
	}
	else
	{
		spin_Padding->DisplayMultipleValues();
	}
}

/** initializes the spin gutters */
void WxImageAdjustmentPanel::InitializeSpinGutters()
{
	for(INT GutterOrientation = 0; GutterOrientation < UIORIENT_MAX; ++GutterOrientation )
	{
		if(CompareGutterProperty((EUIOrientation)GutterOrientation))
		{
			spin_Gutter[GutterOrientation]->ClearMultipleValues();

			UUIStyle_Image* CurrentData = OwnerImagePropertiesGroup->GetCurrentStyleData();
			check(CurrentData);

//@fixme ronp
// 			FLOAT GutterValue = CurrentData->AdjustmentType[this->Orientation].ProtectedRegion.GetValue(this);
// 			if ( spin_Gutter[GutterOrientation] != NULL )
// 			{
// 				spin_Gutter[GutterOrientation]->SetValue(appTrunc(GutterValue));
// 			}
		}
		else
		{
			spin_Gutter[GutterOrientation]->DisplayMultipleValues();
		}
	}	
}

/** Initialize spin controls for setting texture coordinates */
void WxImageAdjustmentPanel::InitializeSpinTextureCoords()
{
	// Set each spin control value individually and check if a special multiple values tag needs to be displayed

	UUIStyle_Image* FirstState = OwnerImagePropertiesGroup->GetCurrentStyleData();
	if ( FirstState != NULL )
	{
		if(CompareTextureCoordProperty((EUIOrientation)Orientation, FALSE) && spin_TextureCoordinates[0] != NULL)
		{
			spin_TextureCoordinates[0]->ClearMultipleValues();
			FLOAT CoordValue = Orientation == UIORIENT_Horizontal ? FirstState->Coordinates.U : FirstState->Coordinates.V;
			spin_TextureCoordinates[0]->SetValue(appTrunc(CoordValue));
		}
		else
		{
			check(spin_TextureCoordinates[0]);
			spin_TextureCoordinates[0]->DisplayMultipleValues();
		}

		if(CompareTextureCoordProperty((EUIOrientation)Orientation, TRUE) && spin_TextureCoordinates[1] != NULL)
		{
			spin_TextureCoordinates[1]->ClearMultipleValues();
			FLOAT CoordValue = Orientation == UIORIENT_Horizontal ? FirstState->Coordinates.UL : FirstState->Coordinates.VL;
			spin_TextureCoordinates[1]->SetValue(appTrunc(CoordValue));
		}
		else
		{
			check(spin_TextureCoordinates[1]);
			spin_TextureCoordinates[1]->DisplayMultipleValues();
		}
	}
}

/** Returns TRUE if all items in the StyleData array have the same alignment value for the corresponding orientation */
UBOOL WxImageAdjustmentPanel::CompareAlignmentProperty() const
{
	check(Orientation < UIORIENT_MAX);
	UBOOL bResult = TRUE;

	if ( StyleDataArray.Num() > 0 )
	{
		const EUIAlignment CompareAlignment = static_cast<EUIAlignment>(StyleDataArray(0)->AdjustmentType[Orientation].Alignment);
		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			EUIAlignment CurrentAlignment = static_cast<EUIAlignment>(StyleDataArray(StyleIndex)->AdjustmentType[Orientation].Alignment);
			if ( CurrentAlignment != CompareAlignment )
			{
				bResult = FALSE;
				break;
			}
		}
	}

	return bResult;
}

/**
 * @return	TRUE if the padding property has the same value for all elements of the StyleData array.
 */
UBOOL WxImageAdjustmentPanel::ComparePaddingProperty() const
{
	check(Orientation < UIORIENT_MAX);
	UBOOL bResult = TRUE;

	if ( StyleDataArray.Num() > 0 )
	{
		const FLOAT ComparePaddingValue = StyleDataArray(0)->StylePadding[Orientation];
		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			FLOAT CurrentPadding = StyleDataArray(StyleIndex)->StylePadding[Orientation];
			if ( CurrentPadding != ComparePaddingValue )
			{
				bResult = FALSE;
				break;
			}
		}		
	}
	return bResult;
}


/** Returns TRUE if all items in the StyleData array have the same material adjustment value for the corresponding orientation */
UBOOL WxImageAdjustmentPanel::CompareAdjustmentProperty() const
{
	check(Orientation < UIORIENT_MAX);
	UBOOL bResult = TRUE;

	if ( StyleDataArray.Num() > 0 )
	{
		const EMaterialAdjustmentType CompareAdjustment = static_cast<EMaterialAdjustmentType>(StyleDataArray(0)->AdjustmentType[Orientation].AdjustmentType);
		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			EMaterialAdjustmentType CurrentAdjustment = static_cast<EMaterialAdjustmentType>(StyleDataArray(StyleIndex)->AdjustmentType[Orientation].AdjustmentType);
			if ( CurrentAdjustment != CompareAdjustment )
			{
				bResult = FALSE;
				break;
			}
		}
	}

	return bResult;
}

/**
 *	Returns TRUE if all items in the StyleData array have the same gutter property for the adjustment panel's orientation
 *
 *	@param	GutterOrientation	specifies which gutter value for the adjustment panel's orientation to evaluate. 
 *  It will specify either top or bottom gutter if this panel's orientation is UIORIENT_Vertical, or left or right if this
 *  panel's orientation is UIORIENT_Horizontal  
 */
UBOOL WxImageAdjustmentPanel::CompareGutterProperty(EUIOrientation GutterOrientation) const
{
	check(Orientation < UIORIENT_MAX);
	UBOOL bResult = TRUE;
	
/*
	if ( StyleDataArray.Num() > 0 )
	{
		const FUIScreenValue_Extent& CompareRegion = StyleDataArray(0)->AdjustmentType[Orientation].ProtectedRegion;
		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			const FUIScreenValue_Extent& CurrentRegion = StyleDataArray(StyleIndex)->AdjustmentType[Orientation].ProtectedRegion;
			if ( CurrentRegion.Value != CompareRegion.Value || CurrentRegion.ScaleType != CompareRegion.ScaleType || CurrentRegion.Orientation != CompareRegion.Orientation )
			{
				bResult = FALSE;
				break;
			}
		}
	}
*/

	return bResult;
}


/**
 *	Returns TRUE if all items in the StyleData array have the same texture coordinate properties for the specified orientation and either size or position.
 *  
 *  @param	Orientation		either UIORIENT_Horizontal or UIORIENT_Vertical
 *  @param  bCompareSize	if set to FALSE the coordinates position value will be compared, if TRUE the coords size value will be compared 
 */
UBOOL WxImageAdjustmentPanel::CompareTextureCoordProperty(EUIOrientation Orientation, UBOOL bCompareSize ) const
{
	check(Orientation < UIORIENT_MAX);
	UBOOL bResult = TRUE;

	if ( StyleDataArray.Num() > 0 )
	{
		const FTextureCoordinates& CompareCoords = StyleDataArray(0)->Coordinates;
		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			const FTextureCoordinates& CurrentCoords = StyleDataArray(StyleIndex)->Coordinates;
			if ( Orientation == UIORIENT_Horizontal )
			{
				if ( bCompareSize )
				{
					if ( CurrentCoords.UL != CompareCoords.UL )
					{
						bResult = FALSE;
						break;
					}
				}
				else
				{
					if ( CurrentCoords.U != CompareCoords.U )
					{
						bResult = FALSE;
						break;
					}
				}
			}
			else
			{
				if ( bCompareSize )
				{
					if ( CurrentCoords.VL != CompareCoords.VL )
					{
						bResult = FALSE;
						break;
					}
				}
				else
				{
					if ( CurrentCoords.V != CompareCoords.V )
					{
						bResult = FALSE;
						break;
					}
				}
			}
		}
	}
	return bResult;
}

/**
 *	Notifies owner style editor that property has been modified
 */
void WxImageAdjustmentPanel::NotifyPropertyChanged()
{
	if(OwnerImagePropertiesGroup)
	{
		OwnerImagePropertiesGroup->NotifyStyleModified(TRUE);
	}	
}

/**
 * Called when the user selects a new menu state or states from the list.  Reinitializes 
 * all control values with the style data from the first selected state and updates internal
 * array of data that will be modified through this panel.
 *
 * @param	SelectedStates	Array of states that are currently selected
 */
void WxImageAdjustmentPanel::NotifySelectedStatesChanged(TArray<UUIState*> & SelectedStates)
{	
	// Update current StyleDataArray to hold currently selected data
	StyleDataArray.Empty();
	UUIStyle_Image* StateData;
	for( TArray<UUIState*>::TIterator It(SelectedStates); It; ++It)
	{
		StateData = OwnerImagePropertiesGroup->GetImageDataForState(EditingStyle, *It);
		
		if(StateData)
		{
			StyleDataArray.AddItem(StateData);
		}
	}

	// If No styles are selected, then controls will retain their old values
	if(SelectedStates.Num() >= 1)
	{
		InitializeControlValues();
	}	
}

void WxImageAdjustmentPanel::OnSelectionChanged( wxCommandEvent& Event )
{
	switch ( Event.GetId() )
	{
	case ID_UI_EDITSTYLE_COMBO_ADJUSTMENT:
		// guard against selection of <multiple values> item
		if(Event.GetString() != cmb_Adjustment->MultipleValuesString)
		{
			UpdateAdjustmentProperty( GetAdjustmentType() );
			NotifyPropertyChanged();
		}
		break;

	case ID_UI_EDITSTYLE_COMBO_ALIGNMENT:
		// guard against selection of <multiple values> item
		if(Event.GetString() != cmb_Alignment->MultipleValuesString)
		{
			UpdateAlignmentProperty( GetAlignment() );
			NotifyPropertyChanged();
		}
		break;
	}

	// allow the dialog window to process the event so that it can update the preview window
	Event.Skip();
}

/**
 * Called when the user modifies the text value of a padding spin control by clicking on spin buttons
 */
void WxImageAdjustmentPanel::OnPaddingChanged( wxSpinEvent& Event )
{
	EUIOrientation ModifiedOrientation = static_cast<EUIOrientation>(Event.GetId() - ID_UI_EDITSTYLE_IMAGEPADDING_H);
	if ( ModifiedOrientation < UIORIENT_MAX && ModifiedOrientation == Orientation )
	{
		if ( Event.GetString() != spin_Padding->MultipleValuesString )
		{
			UpdatePaddingProperty(spin_Padding->GetValue() );
			NotifyPropertyChanged();
		}
	}
}

/**
 * Called when the user modifies the text value of a padding spin control by editing the text field
 */
void WxImageAdjustmentPanel::OnPaddingChanged( wxCommandEvent& Event )
{
	EUIOrientation ModifiedOrientation = static_cast<EUIOrientation>(Event.GetId() - ID_UI_EDITSTYLE_IMAGEPADDING_H);
	if ( ModifiedOrientation < UIORIENT_MAX && ModifiedOrientation == Orientation )
	{
		if ( Event.GetString() != spin_Padding->MultipleValuesString && Event.GetString().IsNumber() )
		{
			UpdatePaddingProperty( spin_Padding->GetValue() );
			NotifyPropertyChanged();
		}
	}
}

/**
 * Called when the user modifies the text value of a gutter spin control by clicking on spin buttons
 */
void WxImageAdjustmentPanel::OnGutterTextChanged( wxSpinEvent& Event )
{
	INT idx = Abs(Event.GetId() - ID_UI_EDITSTYLE_SPIN_GUTTER_H);
	check(idx == 0 || idx == 1);

	if(Event.GetString() != spin_Gutter[idx]->MultipleValuesString)
	{
		UpdateGutterProperty( idx, spin_Gutter[idx]->GetValue() );
		NotifyPropertyChanged();
	}
}

/**
 * Called when the user modifies the text value of a gutter spin control by editing the text field
 */
void WxImageAdjustmentPanel::OnGutterTextChanged( wxCommandEvent& Event )
{
	INT idx = Abs(Event.GetId() - ID_UI_EDITSTYLE_SPIN_GUTTER_H);
	check(idx == 0 || idx == 1);
	
	if(Event.GetString() != spin_Gutter[idx]->MultipleValuesString && Event.GetString().IsNumber())
	{
		UpdateGutterProperty( idx, spin_Gutter[idx]->GetValue() );
		NotifyPropertyChanged();
	}
}

/**
 * Called when the user modifies the text value of a texture coordinate spin control by clicking on spin buttons
 */
void WxImageAdjustmentPanel::OnCoordinatesTextChanged( wxSpinEvent& Event )
{
	UBOOL bExtentValue = (Event.GetId() == ID_UI_EDITSTYLE_SPIN_ATLASCOORDS_EXTENT);

	// Check against selection of an invalid <multiple values> tag
	if(Event.GetString() != spin_TextureCoordinates[bExtentValue]->MultipleValuesString )
	{
		UpdateCoordinatesProperty(!bExtentValue, spin_TextureCoordinates[bExtentValue]->GetValue() );
		NotifyPropertyChanged();
	}
}

/**
 * Called when the user modifies the text value of a texture coordinate spin control by editing the text field
 */
void WxImageAdjustmentPanel::OnCoordinatesTextChanged( wxCommandEvent& Event )
{
	UBOOL bExtentValue = (Event.GetId() == ID_UI_EDITSTYLE_SPIN_ATLASCOORDS_EXTENT);

	// Check against selection of an invalid <multiple values> tag
	if(Event.GetString() != spin_TextureCoordinates[bExtentValue]->MultipleValuesString && Event.GetString().IsNumber())
	{
		UpdateCoordinatesProperty(!bExtentValue, spin_TextureCoordinates[bExtentValue]->GetValue() );
		NotifyPropertyChanged();
	}
}

/**
 * Updates all currently selected States's Image data for this Orientation with the new Adjustment
 *
 * @param	Adjustment	New adjustment for this image orientation
 */
void WxImageAdjustmentPanel::UpdateAdjustmentProperty(EMaterialAdjustmentType Adjustment)
{
	// Iterate over all currently selected state data updating each
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Image* StyleData = StyleDataArray(StateIndex);
		const UBOOL bValueDiffers = StyleData->AdjustmentType[Orientation].AdjustmentType != Adjustment;

		StyleData->AdjustmentType[Orientation].AdjustmentType = Adjustment;
		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}
}

/**
 * Updates all currently selected States's Image data for this Orientation with the new Alignment
 *
 * @param	Alignment	New alignment for this image orientation
 */
void WxImageAdjustmentPanel::UpdateAlignmentProperty(EUIAlignment Alignment)
{
	// Iterate over all currently selected state data updating each
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Image* StyleData = StyleDataArray(StateIndex);
		const UBOOL bValueDiffers = StyleData->AdjustmentType[Orientation].Alignment != Alignment;

		StyleData->AdjustmentType[Orientation].Alignment = Alignment;
		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}
}

/**
 * Updates the Padding value for all currently selected states with the value from the padding spinner
 */
void WxImageAdjustmentPanel::UpdatePaddingProperty( FLOAT NewPaddingValue )
{
	checkSlow(Orientation<UIORIENT_MAX);

	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Image* StyleData = StyleDataArray(StateIndex);
		const UBOOL bValueDiffers = StyleData->StylePadding[Orientation] != NewPaddingValue;

		StyleData->StylePadding[Orientation] = NewPaddingValue;
		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}

	if ( OwnerImagePropertiesGroup->pnl_Preview != NULL )
	{
		if ( Orientation == UIORIENT_Horizontal )
		{
			OwnerImagePropertiesGroup->pnl_Preview->HorzPadding = NewPaddingValue;
		}
		else
		{
			OwnerImagePropertiesGroup->pnl_Preview->VertPadding = NewPaddingValue;
		}
	}
}

/**
 * Updates all currently selected States's Image data for this Orientation with the new Gutter value
 *
 * @param	GutterOrientation		Orientation of the gutter
 * @param	GutterValue				New float value of the gutter for this orientation
 */
void WxImageAdjustmentPanel::UpdateGutterProperty(INT GutterOrientation, FLOAT GutterValue )
{
	// Iterate over all currently selected state data updating each
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Image* StyleData = StyleDataArray(StateIndex);
//@fixme ronp
//		const UBOOL bValueDiffers = StyleData->AdjustmentType[Orientation].ProtectedRegio != GutterValue;

//		StyleData->AdjustmentType[Orientation].ProtectedRegion->[GutterOrientation] = GutterValue;
// 		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}
}

/**
 * Updates the texture coordinates for all currently selected states with the new value.
 *
 * @param	bOriginValue	TRUE to modify the U or V value, FALSE to modify the UL or VL value (depending on orientation)
 * @param	NewValue		the new value to apply
 */
void WxImageAdjustmentPanel::UpdateCoordinatesProperty( UBOOL bOriginValue, FLOAT NewValue )
{
	// Iterate over all currently selected state data updating each
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Image* ImageStyleData = StyleDataArray(StateIndex);

		UBOOL bValueDiffers = FALSE;
		if ( bOriginValue )
		{
			if ( Orientation == UIORIENT_Horizontal )
			{
				bValueDiffers = ImageStyleData->Coordinates.U != NewValue;
				ImageStyleData->Coordinates.U = NewValue;
			}
			else
			{
				bValueDiffers = ImageStyleData->Coordinates.V != NewValue;
				ImageStyleData->Coordinates.V = NewValue;
			}
		}
		else
		{
			if ( Orientation == UIORIENT_Horizontal )
			{
				bValueDiffers = ImageStyleData->Coordinates.UL != NewValue;
				ImageStyleData->Coordinates.UL = NewValue;
			}
			else
			{
				bValueDiffers = ImageStyleData->Coordinates.VL != NewValue;
				ImageStyleData->Coordinates.VL = NewValue;
			}
		}

		ImageStyleData->SetDirtiness(ImageStyleData->IsDirty() || bValueDiffers);
	}
}

/* ==========================================================================================================
	WxStylePropertiesGroup
========================================================================================================== */

IMPLEMENT_ABSTRACT_CLASS(WxStylePropertiesGroup,wxPanel)
BEGIN_EVENT_TABLE(WxStylePropertiesGroup,wxPanel)
END_EVENT_TABLE()

/** Default constructor for use in two-stage dynamic window creation */
WxStylePropertiesGroup::WxStylePropertiesGroup()
: EditingStyle(NULL), CurrentState(NULL)
{
	SceneManager = GUnrealEd->GetBrowserManager()->UISceneManager;
}

/**
 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
 * 
 * @param	InParent			the window that contains this control
 * @param	InID				the window ID to use for this control
 * @param	CurrentStyle		the UIStyle that is being edited
 * @param	InitialState		the state that should be initially active for the current style
 */
void WxStylePropertiesGroup::Create( wxWindow* InParent, wxWindowID InID, class UUIStyle* CurrentStyle, class UUIState* InitialState )
{
	check(CurrentStyle);
	check(InitialState);
	check(SceneManager);

	EditingStyle = CurrentStyle;
	CurrentState = InitialState;
	
	// call the base initializer
	verify(wxPanel::Create(InParent,InID, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL));

	CreateControls();

	InitializeControlValues();
}

/**
 * Called to notify this property group that the style has been changed.  Refreshes the preview window with the style's current values
 *
 * @param	bPropagateStyleChanges	when TRUE the Style editor will propagate changes to the styles which derive 
 *                                  from the editing style and refresh the widgets whose style has been changed.
 *                                  flag set to FALSE will cause reinitialization of controls of this property group
 */
void WxStylePropertiesGroup::NotifyStyleModified( UBOOL bPropagateStyleChanges )
{
	if(bPropagateStyleChanges)
	{
		NotifyPropertyChanged();
		RefreshPreview();
	}
	else
	{
		InitializeControlValues();
	}	
}

/**
 *	Notifies owner style editor that property has been modified
 */
void WxStylePropertiesGroup::NotifyPropertyChanged()
{
	// Only the style editor instance which edits the EditingStyle will take action 
	GCallbackEvent->Send( CALLBACK_UIEditor_PropertyModified, EditingStyle );
}

/* ==========================================================================================================
	WxStyleCommonPropertiesGroup
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxStyleCommonPropertiesGroup,WxStylePropertiesGroup)

BEGIN_EVENT_TABLE(WxStyleCommonPropertiesGroup,WxStylePropertiesGroup)
	EVT_MENU(IDM_UI_EDITSTYLE_DIFFPROPERTIES, WxStyleCommonPropertiesGroup::OnContext_DiffProperties)
	EVT_CHECKLISTBOX(ID_UI_EDITSTYLE_STATELIST, WxStyleCommonPropertiesGroup::OnToggleMenuState)
	EVT_COMBOBOX(ID_UI_EDITSTYLE_GROUP_TAG, WxStyleCommonPropertiesGroup::OnCombobox_SelectGroupName)
	EVT_TEXT_ENTER(ID_UI_EDITSTYLE_GROUP_TAG,WxStyleCommonPropertiesGroup::OnCombobox_SelectGroupName)
END_EVENT_TABLE()

WxStyleCommonPropertiesGroup::WxStyleCommonPropertiesGroup()
{
}

/**
 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
 * 
 * @param	InParent			the window that contains this control
 * @param	InID				the window ID to use for this control
 * @param	CurrentStyle		the UIStyle that is being edited
 * @param	InitialState		the state that should be initially active for the current style
 */
void WxStyleCommonPropertiesGroup::Create( wxWindow* InParent, wxWindowID InID, UUIStyle* CurrentStyle, class UUIState* InitialState )
{
	EditingStyle = CurrentStyle;
	check(EditingStyle);

	// set the initial value for the style tag text control...since it uses a text validator, the value for the
	// style tag text control must be stored in a member of this control
	StyleTagString = *CurrentStyle->StyleTag.ToString();

	WxStylePropertiesGroup::Create(InParent,InID, CurrentStyle,InitialState);
}

/**
 * Creates the controls and sizers for this panel.
 */
void WxStyleCommonPropertiesGroup::CreateControls()
{
	wxStaticBox* StaticBox = new wxStaticBox(this, wxID_ANY, *LocalizeUI(TEXT("StylePropertiesCommon_Caption")));
	wxStaticBoxSizer* StaticBoxSizer = new wxStaticBoxSizer(StaticBox, wxHORIZONTAL);
	SetSizer(StaticBoxSizer);
	{
		wxBoxSizer* CommonVSizer = new wxBoxSizer(wxVERTICAL);
		StaticBoxSizer->Add(CommonVSizer, 2, wxALIGN_CENTER_VERTICAL|wxALL, 0);
		{
			// add the horz sizer for the style id row
			wxBoxSizer* StyleIDHSizer = new wxBoxSizer(wxHORIZONTAL);
			CommonVSizer->Add(StyleIDHSizer, 0, wxGROW|wxALL, 0);
			{
				// add the "Style ID:" label
				lbl_StyleID = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesCommon_Label_StyleID")) );
				StyleIDHSizer->Add(lbl_StyleID, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

				// add the label for displaying the style's StyleID
				txt_StyleID = new wxStaticText( this, wxID_STATIC, TEXT("") );
				txt_StyleID->SetToolTip(*LocalizeUI(TEXT("StylePropertiesCommon_ToolTip_StyleID")));
				StyleIDHSizer->Add(txt_StyleID, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);
			}

			// add the vert sizer for the friendly name row
			wxBoxSizer* StyleNameVSizer = new wxBoxSizer(wxVERTICAL);
			CommonVSizer->Add(StyleNameVSizer, 0, wxGROW|wxALL, 0);
			{
				// add the "Friendly Name:" label
				lbl_FriendlyName = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewStyle_Label_StyleName")) );
				StyleNameVSizer->Add(lbl_FriendlyName, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

				// add the text control for entering the friendly name
				txt_FriendlyName = new wxTextCtrl( this, ID_UI_EDITSTYLE_NAME, TEXT("") );
				StyleNameVSizer->Add(txt_FriendlyName, 1, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);
			}
			// add the horz sizer for the tag/color row
			wxBoxSizer* StyleTagHSizer = new wxBoxSizer(wxHORIZONTAL);
			CommonVSizer->Add(StyleTagHSizer, 0, wxGROW|wxALL, 0);
			{
				// add the vert sizer for the style tag control
				wxBoxSizer* StyleTagVSizer = new wxBoxSizer(wxVERTICAL);
				StyleTagHSizer->Add(StyleTagVSizer, 3, wxALIGN_CENTER_VERTICAL|wxALL, 0);
				{
					// add the "Unique Tag:" label
					lbl_StyleTag = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewStyle_Label_StyleTag")) );
					StyleTagVSizer->Add(lbl_StyleTag, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

					// add the text control for entering the style tag 
					txt_StyleTag = new wxTextCtrl( this, ID_UI_EDITSTYLE_TAG, TEXT("") );
					txt_StyleTag->SetToolTip(*LocalizeUI(TEXT("DlgNewStyle_Tooltip_StyleTag")));
					StyleTagVSizer->Add(txt_StyleTag, 1, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);
				}
				{
					// add the "Group Name:" label
					wxStaticText *label = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("DlgNewStyle_Label_StyleGroupName")) );
					StyleTagVSizer->Add(label, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

					// add the text control for entering the style tag 
					cmb_StyleGroupTag = new wxComboBox( this, ID_UI_EDITSTYLE_GROUP_TAG, TEXT("Auto"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxTE_PROCESS_ENTER );
					cmb_StyleGroupTag->SetToolTip(*LocalizeUI(TEXT("DlgNewStyle_Label_StyleGroupName_ToolTip")));
					StyleTagVSizer->Add(cmb_StyleGroupTag, 1, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);
				}
			}
		}
		wxBoxSizer* MenuStateListVSizer = new wxBoxSizer(wxVERTICAL);
		StaticBoxSizer->Add(MenuStateListVSizer, 1, wxGROW|wxALL, 0);
		{
			wxArrayString MenuStateNames;

			TArray<FUIStateResourceInfo> StateResources;
			SceneManager->GetSupportedUIStates(StateResources);
			for ( INT StateIndex = 0; StateIndex < StateResources.Num(); StateIndex++ )
			{
				FUIStateResourceInfo& ResourceInfo = StateResources(StateIndex);
				MenuStateNames.Add( *ResourceInfo.FriendlyName );
			}

			chklst_MenuStates = new wxCheckListBox( this, ID_UI_EDITSTYLE_STATELIST, wxDefaultPosition, wxDefaultSize, MenuStateNames, wxLB_EXTENDED );
			chklst_MenuStates->SetHelpText(TEXT("Select a menu state to edit the style data for that state.  Only style data for states which are checked will be saved into the style."));
			chklst_MenuStates->SetToolTip(TEXT("Menu States"));
			MenuStateListVSizer->Add(chklst_MenuStates, 1, wxGROW|wxALL, 5);

			chklst_MenuStates->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(WxStyleCommonPropertiesGroup::OnChecklistMenuStatesRightDown), NULL, this);
		}
	}
	// Set validators
	txt_StyleTag->SetValidator( WxNameTextValidator(&StyleTagString,VALIDATE_Name) );
}

/**
 * Initialize the values of the controls contained by this panel with the values from the current style.
 */
void WxStyleCommonPropertiesGroup::InitializeControlValues()
{
	Freeze();

	// set the value of Style Tag
	txt_StyleTag->SetValue(StyleTagString);

	// disable modifying the tag if the style is one of the designated default ones, or if this style is based on a style from a different skin
	if(EditingStyle->IsDefaultStyle())
	{
		txt_StyleTag->Disable();
	}

	UUIStyle* StyleArchetype = EditingStyle->GetArchetype<UUIStyle>();
	if ( !StyleArchetype->HasAnyFlags(RF_ClassDefaultObject) )
	{
		// if the style that is being edited is based on a style from another skin, don't allow the style to be renamed
		if ( StyleArchetype->GetOuterUUISkin() != EditingStyle->GetOuterUUISkin() )
		{
			txt_StyleTag->Disable();
		}
	}

	// set the value for StyleID
	txt_StyleID->SetLabel( *EditingStyle->StyleID.String() );

	// set the value for the style's friendly name
	txt_FriendlyName->SetValue( *EditingStyle->StyleName );

	// no need to set the value for the style's tag, as that will be handled by the text validator assigned to the style tag text control
	UUIStyle_Data* DefaultStateData = EditingStyle->GetStyleForStateByClass(UUIState_Enabled::StaticClass());

	INT SelectedStateIndex = 0;
	TArray<FUIStateResourceInfo> StateResources;
	SceneManager->GetSupportedUIStates(StateResources);
	for ( INT StateIndex = 0; StateIndex < StateResources.Num(); StateIndex++ )
	{
		FUIStateResourceInfo& ResourceInfo = StateResources(StateIndex);
		UUIState* MenuState = Cast<UUIState>(ResourceInfo.UIResource);

		UUIStyle_Data* StateData = EditingStyle->StateDataMap.FindRef(MenuState);
		if ( StateData != NULL )
		{
			chklst_MenuStates->Check(StateIndex,StateData->bEnabled == TRUE);
		}
		else
		{
			// if the style doesn't already contain a style data object for this menu state, create one now

			// use the style data associated with this state from this style's archetype as the archetype
			// for the new style data object
			UUIStyle_Data* StateDataTemplate = EditingStyle->GetStyleForState(MenuState);
			if ( StateDataTemplate == NULL )
			{
				StateDataTemplate = DefaultStateData;
			}

			StateData = EditingStyle->AddNewState(MenuState, StateDataTemplate);
			StateData->bEnabled = FALSE;
		}

		if ( ResourceInfo.UIResource == CurrentState )
		{
			SelectedStateIndex = StateIndex;
		}
	}

	chklst_MenuStates->SetSelection(SelectedStateIndex);

	cmb_StyleGroupTag->Freeze();
	cmb_StyleGroupTag->Clear();
	TArray<FString> AvailableStyleGroups;
	EditingStyle->GetOuterUUISkin()->GetStyleGroups(AvailableStyleGroups);
	for ( INT StyleGroupIndex = 0; StyleGroupIndex < AvailableStyleGroups.Num(); StyleGroupIndex++ )
	{
		cmb_StyleGroupTag->AppendString(*AvailableStyleGroups(StyleGroupIndex));
	}

	if ( AvailableStyleGroups.Num() > 0 )
	{
		cmb_StyleGroupTag->SetValue(*EditingStyle->StyleGroupName);
	}

	cmb_StyleGroupTag->Thaw();

	//@todo - do other stuff

	Thaw();
}

/**
 * Called to notify this property group that the user has selected a new UIStates from the list of 
 * states available for this style.
 *
 * @param	SelectedStates	Array of states that are going to be edited
 */
void WxStyleCommonPropertiesGroup::NotifySelectedStatesChanged( TArray<UUIState*> & SelectedStates )
{
	WxStylePropertiesGroup::NotifySelectedStatesChanged(SelectedStates);
	NotifyStyleModified(FALSE);
}

/**
 * Disables editing of any properties inside this group
 */
void WxStyleCommonPropertiesGroup::DisableControls()
{
	txt_FriendlyName->Disable();
	txt_StyleTag->Disable();
	chklst_MenuStates->Disable();
}

/**
 * Applies any changes made in this property group to the style being edited.
 */
void WxStyleCommonPropertiesGroup::SaveChanges()
{
	// apply the style tag
	EditingStyle->StyleTag = StyleTagString.c_str();

	//@todo - save the friendly name to .int file
	EditingStyle->StyleName = txt_FriendlyName->GetValue().c_str();

	const FString SelectedStyleGroupName = cmb_StyleGroupTag->GetValue().c_str();
	if ( EditingStyle->StyleGroupName != SelectedStyleGroupName )
	{
		FScopedTransaction Transaction(*LocalizeUI(TEXT("TransChangeStyleGroup")));

		//saving group name
		EditingStyle->StyleGroupName = SelectedStyleGroupName;
	}

	// the user may have added a new style group name by typing a new value into the style groups editbox
	// so attempt to add all style group names now.

	const INT StyleGroupCount = cmb_StyleGroupTag->GetCount();
	UUISkin* OwnerSkin = EditingStyle->GetOuterUUISkin();
	{
		FScopedTransaction Transaction(*LocalizeUI(TEXT("TransAddStyleGroup")));

		UBOOL bShouldCancel=TRUE;
		for ( INT GroupIndex = 0; GroupIndex < StyleGroupCount; GroupIndex++ )
		{
			const FString GroupName = cmb_StyleGroupTag->GetString(GroupIndex).c_str();
			if ( OwnerSkin->AddStyleGroupName(GroupName) )
			{
				bShouldCancel = FALSE;
			}
		}

		if ( OwnerSkin->AddStyleGroupName(SelectedStyleGroupName) )
		{
			bShouldCancel = FALSE;
		}

		if ( bShouldCancel )
		{
			Transaction.Cancel();
		}
		else
		{
			WxDlgUISkinEditor* EditSkinDialog = wxDynamicCast(GetParent()->GetParent(), WxDlgUISkinEditor);
			if ( EditSkinDialog != NULL )
			{
				EditSkinDialog->RebuildStyleGroups();
			}
		}
	}
}

/**
 * Makes sure that state data that has been modified by the user will not be lost due to revert of the unchecked states.
 * If such data is found, a message box is displayed asking user weather to proceed with the revert process.
 *
 * @return	TRUE if deserialization of any unchecked states can be proceeded, or FALSE if it should not. 
 */
UBOOL WxStyleCommonPropertiesGroup::VerifyUncheckedStateChanges()
{
	check(SceneManager);
	check(EditingStyle);
	check(chklst_MenuStates);

	// Iterate over all states' data checking if it has been modified and if it is going to be reverted 
	UBOOL Ret = TRUE;
	UBOOL DispWarning = FALSE;

	TArray<FUIStateResourceInfo> StateResources;
	SceneManager->GetSupportedUIStates(StateResources);
	for ( INT StateIndex = 0; StateIndex < StateResources.Num(); StateIndex++ )
	{
		FUIStateResourceInfo& ResourceInfo = StateResources(StateIndex);
		UUIStyle_Data* StateData = EditingStyle->StateDataMap.FindRef(Cast<UUIState>(ResourceInfo.UIResource));
		check(StateData);

		if ( !chklst_MenuStates->IsChecked(StateIndex) && StateData->IsDirty() )
		{
			DispWarning = TRUE;
		}
	}

	if(DispWarning)
	{
		// Display warning to the user asking if he wants to  deserialize the modified data
		wxMessageDialog DeserializationWarning(this, *LocalizeUI(TEXT("DlgEditStyle_Warning_ModifiedStateRevert")),
			*LocalizeUI(TEXT("DlgEditStyle_Warning_Title")), wxOK|wxCANCEL|wxICON_EXCLAMATION );

		if( wxID_CANCEL == DeserializationWarning.ShowModal())
		{
			Ret = FALSE;
		}
	}

	return Ret;
}


/**
 * Reverts data for states that are unchecked to the original values stored in the archetype
 */
void WxStyleCommonPropertiesGroup::DeserializeUncheckedStates()
{
	check(SceneManager);
	check(EditingStyle);
	check(chklst_MenuStates);

	// we only want to save changes to style data for states that were checked, so for each state data item that is
	// unchecked, deserialize it to restore the values from its archetype, so that none of its values are saved to disk
	TArray<FUIStateResourceInfo> StateResources;
	SceneManager->GetSupportedUIStates(StateResources);
	for ( INT StateIndex = 0; StateIndex < StateResources.Num(); StateIndex++ )
	{
		FUIStateResourceInfo& ResourceInfo = StateResources(StateIndex);
		UUIState* CurrentState = Cast<UUIState>(ResourceInfo.UIResource);
		UUIStyle_Data* StateData = EditingStyle->StateDataMap.FindRef(CurrentState);
		check(StateData);

		if ( !chklst_MenuStates->IsChecked(StateIndex) )
		{
			//@fixme - note that if this state data's archetype is another state data object later in the list, we
			// will end up initializing the style to whatever values were set for the other state data object during this
			// style editing session - the correct thing to do would be to check for this case and sort the list by dependency
			StateData->InitializeProperties();

			// allow data to perform necessary initialization
			StateData->Created(CurrentState);
			StateData->bEnabled = FALSE;
		}
		else
		{
			StateData->bEnabled = TRUE;
		}
	}
}

/**
 * Clears the dirty flag from all the style data changed by this editor 
 */
void WxStyleCommonPropertiesGroup::ClearDirtyFlags()
{
	check(EditingStyle);

	for(TMap<UUIState*,UUIStyle_Data*>::TIterator It(EditingStyle->StateDataMap); It; ++It)
	{
		UUIStyle_Data* StateData = It.Value();
		check(StateData);

		// Set the dirty flag to FALSE
		StateData->SetDirtiness(FALSE);
	}
}

/**
 * Serializes the style reference
 */
void WxStyleCommonPropertiesGroup::Serialize( FArchive& Ar )
{
	Ar << EditingStyle;
}

/**
 * Obtains a list of currently selected states
 *
 * @param	SelectedStates	Array that will be populated with states currently selected in the editor
 */
void WxStyleCommonPropertiesGroup::GetSelectedStates( TArray<UUIState*>& SelectedStates )
{
	check(chklst_MenuStates);	
	SelectedStates.Empty();

	// Obtain the currently selected Indexes
	wxArrayInt SelectedIndices; 
	chklst_MenuStates->GetSelections(SelectedIndices);

	// Transform Indexes into actual States
	UUIState* CurrentState;
	TArray<FUIStateResourceInfo> StateResources;
	SceneManager->GetSupportedUIStates(StateResources);

	for( UINT i = 0; i < SelectedIndices.GetCount(); ++i )
	{
		checkSlow(StateResources.IsValidIndex(SelectedIndices.Item(i)));
		CurrentState = CastChecked<UUIState>(StateResources(SelectedIndices.Item(i)).UIResource);

		SelectedStates.Push(CurrentState);		
	}
}

/** Called when the user checks or unchecks a menu state */
void WxStyleCommonPropertiesGroup::OnToggleMenuState( wxCommandEvent& Event )
{
	TArray<FUIStateResourceInfo> StateResources;
	SceneManager->GetSupportedUIStates(StateResources);

	INT SelectionIndex = Event.GetInt();
	if ( StateResources.IsValidIndex(SelectionIndex) )
	{
		FUIStateResourceInfo& ResourceInfo = StateResources(SelectionIndex);

		UUIState* CurrentState = CastChecked<UUIState>(ResourceInfo.UIResource);
		UUIStyle_Data* StateData = EditingStyle->StateDataMap.FindRef(CurrentState);
		check(StateData);

		StateData->bEnabled = chklst_MenuStates->IsChecked(SelectionIndex);
	}

	Event.Skip();
}

/** Handler for mouse button right click int the states listbox */ 
void WxStyleCommonPropertiesGroup::OnChecklistMenuStatesRightDown(wxMouseEvent& event)
{
	// Display a context menu 
	wxMenu* ContextMenu = new wxMenu;

	ContextMenu->Append(IDM_UI_EDITSTYLE_DIFFPROPERTIES, *LocalizeUI(TEXT("StylePropertiesCommon_MenuDiffProperties")),
		wxString(TEXT("")), FALSE);

	// Enable diffing of properties only if two states are selected
	TArray<UUIState*> SelectedStates;
	GetSelectedStates(SelectedStates);

	if (SelectedStates.Num() == 2)
	{
		ContextMenu->Enable(IDM_UI_EDITSTYLE_DIFFPROPERTIES, TRUE);
	} 
	else
	{
		ContextMenu->Enable(IDM_UI_EDITSTYLE_DIFFPROPERTIES, FALSE);
	}

	PopupMenu( ContextMenu );

	delete ContextMenu;
	// SetArchetypeSubMenu will be deleted automatically by the ContextMenu destructor
}

/** Event handler for comparing properties of two states */
void WxStyleCommonPropertiesGroup::OnContext_DiffProperties( wxCommandEvent& Event )
{
	// At this point there should be only two states selected
	TArray<UUIState*> SelectedStates;
	GetSelectedStates(SelectedStates);
	check(SelectedStates.Num() == 2);

	WxDlgDiffStateProperties* dlg = new WxDlgDiffStateProperties;
	dlg->Create(EditingStyle, SelectedStates(0), SelectedStates(1), this);
	dlg->Show();
}

/** Event handler for selecting group name from combobox */
void WxStyleCommonPropertiesGroup::OnCombobox_SelectGroupName( wxCommandEvent& Event )
{
	FString SelectedGroupName = cmb_StyleGroupTag->GetValue().c_str();

	// trim the whitespace
	SelectedGroupName.Trim(); SelectedGroupName.TrimTrailing();

	cmb_StyleGroupTag->SetValue( *SelectedGroupName );
	if ( SelectedGroupName.Len() > 0 )
	{
		if ( EditingStyle->GetOuterUUISkin()->FindStyleGroupIndex(SelectedGroupName) == INDEX_NONE )
		{
			wxMessageDialog DeserializationWarning(this, *FString::Printf( LocalizeSecure(LocalizeUI(TEXT("DlgNewStyle_Label_StyleGroupNameNoExist")), *SelectedGroupName)),
				TEXT("Message"), wxOK|wxCANCEL|wxICON_EXCLAMATION );

			if ( DeserializationWarning.ShowModal() == wxID_OK )
			{
				cmb_StyleGroupTag->AppendString( *SelectedGroupName );
			}
			else
			{
				// restore the previous value
				cmb_StyleGroupTag->SetValue( *EditingStyle->StyleGroupName );
			}
		}
	}
}

/* ==========================================================================================================
	WxStyleTextPropertiesGroup
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxStyleTextPropertiesGroup,WxStylePropertiesGroup)

BEGIN_EVENT_TABLE(WxStyleTextPropertiesGroup,WxStylePropertiesGroup)
	EVT_CHOICE( ID_UI_EDITSTYLE_COMBO_FONT, WxStyleTextPropertiesGroup::OnFontChanged )
	EVT_CHOICE( ID_UI_EDITSTYLE_COMBO_ALIGNMENT_H, WxStyleTextPropertiesGroup::OnAlignmentChanged )
	EVT_CHOICE( ID_UI_EDITSTYLE_COMBO_ALIGNMENT_V, WxStyleTextPropertiesGroup::OnAlignmentChanged )
	EVT_CHOICE( ID_UI_EDITSTYLE_COMBO_TEXTCLIPMODE, WxStyleTextPropertiesGroup::OnClipModeChanged )
	EVT_CHOICE( ID_UI_EDITSTYLE_COMBO_TEXTCLIPALIGNMENT, WxStyleTextPropertiesGroup::OnClipAlignmentChanged )
	EVT_CHECKLISTBOX( ID_UI_EDITSTYLE_CHECKLIST_ATTRIBUTES, WxStyleTextPropertiesGroup::OnAttributesChanged )
	EVT_SPINCTRL( ID_UI_EDITSTYLE_TEXTSCALE_X, WxStyleTextPropertiesGroup::OnScaleChanged )
	EVT_SPINCTRL( ID_UI_EDITSTYLE_TEXTSCALE_Y, WxStyleTextPropertiesGroup::OnScaleChanged )
	EVT_CHOICE( ID_UI_EDITSTYLE_AUTOSCALE_MODE, WxStyleTextPropertiesGroup::OnAutoScaleModeChanged )
	EVT_SPINCTRL( ID_UI_EDITSTYLE_AUTOSCALE_MIN_VALUE, WxStyleTextPropertiesGroup::OnMinAutoScaleChanged )
	EVT_SPINCTRL( ID_UI_EDITSTYLE_SPACINGADJUST_X, WxStyleTextPropertiesGroup::OnSpacingAdjustChanged )
	EVT_SPINCTRL( ID_UI_EDITSTYLE_SPACINGADJUST_Y, WxStyleTextPropertiesGroup::OnSpacingAdjustChanged )
	EVT_SPINCTRL( ID_UI_EDITSTYLE_TEXTPADDING_V, WxStyleTextPropertiesGroup::OnPaddingChanged )
	EVT_SPINCTRL( ID_UI_EDITSTYLE_TEXTPADDING_H, WxStyleTextPropertiesGroup::OnPaddingChanged )
	EVT_TEXT_ENTER(ID_UI_EDITSTYLE_TEXTPADDING_V, WxStyleTextPropertiesGroup::OnPaddingChanged)
	EVT_TEXT_ENTER(ID_UI_EDITSTYLE_TEXTPADDING_H, WxStyleTextPropertiesGroup::OnPaddingChanged)
END_EVENT_TABLE()

/**
 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
 * 
 * @param	InParent			the window that contains this control
 * @param	InID				the window ID to use for this control
 * @param	CurrentStyle		the UIStyle that is being edited
 * @param	InitialState		the state that should be initially active for the current style
 */
void WxStyleTextPropertiesGroup::Create( wxWindow* InParent, wxWindowID InID, class UUIStyle* CurrentStyle, class UUIState* InitialState )
{
	check(CurrentStyle);

	UUIStyle_Text* InitialStyleData = GetTextDataForState(CurrentStyle, InitialState);

	check(InitialStyleData);
	StyleDataArray.AddItem(InitialStyleData);
	WxStylePropertiesGroup::Create(InParent,InID,CurrentStyle,InitialState);
}

/**
 * Creates the controls and sizers for this panel.
 */
void WxStyleTextPropertiesGroup::CreateControls()
{
	check(StyleDataArray.Num() > 0);

	wxStaticBox* StaticBox = new wxStaticBox(this, wxID_ANY, *LocalizeUI(TEXT("StylePropertiesText_Caption")));
	wxStaticBoxSizer* StaticVSizer = new wxStaticBoxSizer(StaticBox, wxVERTICAL);
	SetSizer(StaticVSizer);

	// add the primary horizontal sizer
	wxBoxSizer* OptionsHSizer = new wxBoxSizer(wxHORIZONTAL);
	StaticVSizer->Add(OptionsHSizer, 0, wxGROW|wxALL, 5);

	// add the vertical sizer to contain the font and alignment combos
	wxBoxSizer* LeftVSizer = new wxBoxSizer(wxVERTICAL);
	OptionsHSizer->Add(LeftVSizer, 1, wxGROW|wxALL, 0);
	{
		// add the font sizer
		wxBoxSizer* FontVSizer = new wxBoxSizer(wxVERTICAL);
		LeftVSizer->Add(FontVSizer, 0, wxGROW|wxALL, 0);
		{
			// add the font label
			wxStaticText* lbl_Font = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_Font")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
			FontVSizer->Add(lbl_Font, 0, wxGROW|wxALL|wxADJUST_MINSIZE, 5);

			// add the font combo box
			cmb_Font = new WxUIChoice( this, ID_UI_EDITSTYLE_COMBO_FONT, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
			FontVSizer->Add(cmb_Font, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);
		}

		// add a spacer
		LeftVSizer->Add(3, 20, 0, wxALIGN_CENTER_HORIZONTAL, 0);

		// add the grid sizer that will contain the clipping mode and alignment controls
		wxFlexGridSizer* ClippingGridSizer = new wxFlexGridSizer(2, 2, 0, 0);
		ClippingGridSizer->AddGrowableCol(0);
		ClippingGridSizer->AddGrowableCol(1);
		LeftVSizer->Add(ClippingGridSizer, 0, wxGROW|wxALL, 0);
		{
			wxStaticText* lbl_ClipMode = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_ClipMode")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
			ClippingGridSizer->Add(lbl_ClipMode, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

			wxStaticText* lbl_ClipAlignment = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_ClipAlignment")), wxDefaultPosition, wxDefaultSize, 0 );
			ClippingGridSizer->Add(lbl_ClipAlignment, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

			wxArrayString cmb_ClipModeStrings;
			// find the enum and use its metadata to get the names for the enum values
			static UEnum* ClipModeEnum=FindObjectChecked<UEnum>(UUIRoot::StaticClass(), TEXT("ETextClipMode"), TRUE);
			for ( INT NameIndex = 0; NameIndex < ClipModeEnum->NumEnums() - 1; NameIndex++ )
			{
				if ( ClipModeEnum->HasMetaData(TEXT("DisplayName"), NameIndex) )
				{
					cmb_ClipModeStrings.Add(*ClipModeEnum->GetMetaData(TEXT("DisplayName"), NameIndex));
				}
				else
				{
					cmb_ClipModeStrings.Add(*ClipModeEnum->GetEnum(NameIndex).ToString());
				}
			}

			cmb_ClipMode = new WxUIChoice( this, ID_UI_EDITSTYLE_COMBO_TEXTCLIPMODE, wxDefaultPosition, wxDefaultSize, cmb_ClipModeStrings, 0 );
			cmb_ClipMode->SetSelection(0);
			cmb_ClipMode->SetToolTip(*LocalizeUI(TEXT("StylePropertiesText_ToolTip_ClipMode")));
			ClippingGridSizer->Add(cmb_ClipMode, 1, wxGROW|wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 5);


			wxArrayString cmb_ClipAlignmentStrings;
			{
				// find the enum and use its metadata to get the names for the enum values
				static UEnum* ClipAlignmentEnum = FindObjectChecked<UEnum>(UUIRoot::StaticClass(), TEXT("EUIAlignment"), TRUE);
				for ( INT Idx = 0; Idx < UIALIGN_Default; Idx++ )
				{
					if ( ClipAlignmentEnum->HasMetaData(TEXT("DisplayName"), Idx) )
					{
						cmb_ClipAlignmentStrings.Add(*ClipAlignmentEnum->GetMetaData(TEXT("DisplayName"), Idx));
					}
					else
					{
						cmb_ClipAlignmentStrings.Add(*ClipAlignmentEnum->GetEnum(Idx).ToString());
					}
				}
			}

			cmb_ClipAlignment = new WxUIChoice( this, ID_UI_EDITSTYLE_COMBO_TEXTCLIPALIGNMENT, wxDefaultPosition, wxDefaultSize, cmb_ClipAlignmentStrings, 0 );
			cmb_ClipAlignment->SetSelection(0);
			cmb_ClipAlignment->SetToolTip(*LocalizeUI(TEXT("StylePropertiesText_ToolTip_ClipAlignment")));
			ClippingGridSizer->Add(cmb_ClipAlignment, 1, wxGROW|wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 5);
		}

		// add the grid sizer that will contain the autoscale mode controls
		wxFlexGridSizer* AutoScaleGridSizer = new wxFlexGridSizer(2, 2, 0, 0);
		AutoScaleGridSizer->AddGrowableCol(0);
		LeftVSizer->Add(AutoScaleGridSizer, 0, wxGROW|wxALL, 0);
		{
			wxStaticText* lbl_AutoScaling = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_AutoScaleMode")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
			AutoScaleGridSizer->Add(lbl_AutoScaling, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

			wxStaticText* lbl_MinScale = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_MinAutoScale")), wxDefaultPosition, wxDefaultSize, 0 );
			AutoScaleGridSizer->Add(lbl_MinScale, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

			wxArrayString cmb_AutoScaleModeStrings;
			cmb_AutoScaleModeStrings.Add(*LocalizeUI(TEXT("StylePropertiesText_AutoScaleMode_Options[0]")));
			cmb_AutoScaleModeStrings.Add(*LocalizeUI(TEXT("StylePropertiesText_AutoScaleMode_Options[1]")));
			cmb_AutoScaleModeStrings.Add(*LocalizeUI(TEXT("StylePropertiesText_AutoScaleMode_Options[2]")));
			cmb_AutoScaleModeStrings.Add(*LocalizeUI(TEXT("StylePropertiesText_AutoScaleMode_Options[3]")));

			cmb_AutoScaleMode = new WxUIChoice( this, ID_UI_EDITSTYLE_AUTOSCALE_MODE, wxDefaultPosition, wxDefaultSize, cmb_AutoScaleModeStrings, 0 );
			cmb_AutoScaleMode->SetSelection(0);
			cmb_AutoScaleMode->SetToolTip(*LocalizeUI(TEXT("StylePropertiesText_ToolTip_AutoScaleMode")));
			AutoScaleGridSizer->Add(cmb_AutoScaleMode, 1, wxGROW|wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 5);

			spin_AutoScaleMinValue = new WxUISpinCtrl( this, ID_UI_EDITSTYLE_AUTOSCALE_MIN_VALUE, 0.6f, wxDefaultPosition, wxDefaultSize, 0.f, 10.f );
			spin_AutoScaleMinValue->SetToolTip(*LocalizeUI(TEXT("StylePropertiesText_ToolTip_MinAutoScale")));
			AutoScaleGridSizer->Add(spin_AutoScaleMinValue, 1, wxGROW|wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 5);
		}

		// add the vert sizer that will contain the Spacing Adjust label and combo box controls
		wxBoxSizer* SpacingAdjustVSizer = new wxBoxSizer(wxVERTICAL);
		LeftVSizer->Add(SpacingAdjustVSizer, 0, wxGROW|wxALL, 0);
		{
			wxStaticText* lbl_SpacingAdjust = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_SpacingAdjust")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
			SpacingAdjustVSizer->Add(lbl_SpacingAdjust, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

			// add the horizontal sizer that will contain the alignment combo boxes
			wxBoxSizer* SpacingAdjustHSizer = new wxBoxSizer(wxHORIZONTAL);
			SpacingAdjustVSizer->Add(SpacingAdjustHSizer, 0, wxGROW|wxALL, 0);
			{
				// add the label for the horizontal character spacing spinner
				wxStaticText* lbl_SpacingAdjustX = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_SpacingAdjustX")) );
				SpacingAdjustHSizer->Add(lbl_SpacingAdjustX, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

				spin_SpacingAdjust[UIORIENT_Horizontal] = new WxUISpinCtrl(this, ID_UI_EDITSTYLE_SPACINGADJUST_X, 1.0f, wxDefaultPosition, wxDefaultSize, -10.0f, 10.0f);
				spin_SpacingAdjust[UIORIENT_Horizontal]->SetToolTip(*LocalizeUI(TEXT("StylePropertiesText_ToolTip_SpacingAdjustX")));
				SpacingAdjustHSizer->Add(spin_SpacingAdjust[UIORIENT_Horizontal], 1, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);

				// add the label for the vertical character spacing spinner
				wxStaticText* lbl_SpacingAdjustCtrlY = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_SpacingAdjustY")), wxDefaultPosition, wxDefaultSize  );
				SpacingAdjustHSizer->Add(lbl_SpacingAdjustCtrlY, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

				spin_SpacingAdjust[UIORIENT_Vertical] = new WxUISpinCtrl(this, ID_UI_EDITSTYLE_SPACINGADJUST_Y, 1.0f, wxDefaultPosition, wxDefaultSize, -10.0f, 10.0f);
				spin_SpacingAdjust[UIORIENT_Vertical]->SetToolTip(*LocalizeUI(TEXT("StylePropertiesText_ToolTip_SpacingAdjustY")));
				SpacingAdjustHSizer->Add(spin_SpacingAdjust[UIORIENT_Vertical], 1, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);
			}
		}
	}

	// now do the right side
	wxBoxSizer* RightVSizer = new wxBoxSizer(wxVERTICAL);
	OptionsHSizer->Add(RightVSizer, 1, wxGROW|wxALL, 0);
	{
		// add the color adjustment panel
		pnl_Color = new WxColorAdjustmentPanel();
		pnl_Color->Create(this, wxID_ANY, StyleDataArray(0), this);
		RightVSizer->Add(pnl_Color, 0, wxGROW|wxLEFT, 5);

		// add the vert sizer that will contain the alignment label and combo horz sizer
		wxBoxSizer* AlignmentVSizer = new wxBoxSizer(wxVERTICAL);
		RightVSizer->Add(AlignmentVSizer, 0, wxGROW|wxALL, 0);
		{
			wxStaticText* lbl_Alignment = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_Alignment")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
			AlignmentVSizer->Add(lbl_Alignment, 0, wxGROW|wxALL|wxADJUST_MINSIZE, 5);

			// add the horizontal sizer that will contain the alignment combo boxes
			wxBoxSizer* AlignmentComboHSizer = new wxBoxSizer(wxHORIZONTAL);
			AlignmentVSizer->Add(AlignmentComboHSizer, 0, wxGROW|wxALL, 0);
			{
				// add the label for the horizontal alignment combo
				wxStaticText* lbl_HorzAlignment = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_HorzAbbreviation")) );
				AlignmentComboHSizer->Add(lbl_HorzAlignment, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM|wxADJUST_MINSIZE, 5);

				cmb_Alignment[UIORIENT_Horizontal] = new WxUIChoice( this, ID_UI_EDITSTYLE_COMBO_ALIGNMENT_H, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
				cmb_Alignment[UIORIENT_Horizontal]->SetToolTip(*LocalizeUI(TEXT("StylePropertiesText_ToolTip_Alignment[0]")));
				AlignmentComboHSizer->Add(cmb_Alignment[UIORIENT_Horizontal], 1, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);

				wxStaticText* lbl_VertAlignment = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_VertAbbreviation")), wxDefaultPosition, wxDefaultSize, 0 );
				AlignmentComboHSizer->Add(lbl_VertAlignment, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM|wxADJUST_MINSIZE, 5);

				cmb_Alignment[UIORIENT_Vertical] = new WxUIChoice( this, ID_UI_EDITSTYLE_COMBO_ALIGNMENT_V, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
				cmb_Alignment[UIORIENT_Vertical]->SetToolTip(*LocalizeUI(TEXT("StylePropertiesText_ToolTip_Alignment[1]")));
				AlignmentComboHSizer->Add(cmb_Alignment[UIORIENT_Vertical], 1, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);
			}
		}

		// add the vert sizer that will contain the Scale label and combo box controls
		wxBoxSizer* ScaleVSizer = new wxBoxSizer(wxVERTICAL);
		RightVSizer->Add(ScaleVSizer, 0, wxGROW|wxALL, 0);
		{
			wxStaticText* lbl_Scale = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_Scale")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
			ScaleVSizer->Add(lbl_Scale, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

			// add the horizontal sizer that will contain the alignment combo boxes
			wxBoxSizer* ScaleHSizer = new wxBoxSizer(wxHORIZONTAL);
			ScaleVSizer->Add(ScaleHSizer, 0, wxGROW|wxALL, 0);
			{
				// add the label for the horizontal alignment combo
				wxStaticText* lbl_ScaleX = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_ScaleX")) );
				ScaleHSizer->Add(lbl_ScaleX, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

				spin_TextScale[UIORIENT_Horizontal] = new WxUISpinCtrl(this, ID_UI_EDITSTYLE_TEXTSCALE_X, 1.0f, wxDefaultPosition, wxDefaultSize, -10.0f, 10.0f);
				spin_TextScale[UIORIENT_Horizontal]->SetToolTip(*LocalizeUI(TEXT("StylePropertiesText_ToolTip_ScaleX")));
				ScaleHSizer->Add(spin_TextScale[UIORIENT_Horizontal], 1, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);

				wxStaticText* lbl_scaleCtrlY = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_ScaleY")), wxDefaultPosition, wxDefaultSize  );
				ScaleHSizer->Add(lbl_scaleCtrlY, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

				spin_TextScale[UIORIENT_Vertical] = new WxUISpinCtrl(this, ID_UI_EDITSTYLE_TEXTSCALE_Y, 1.0f, wxDefaultPosition, wxDefaultSize, -10.0f, 10.0f);
				spin_TextScale[UIORIENT_Vertical]->SetToolTip(*LocalizeUI(TEXT("StylePropertiesText_ToolTip_ScaleY")));
				ScaleHSizer->Add(spin_TextScale[UIORIENT_Vertical], 1, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);
			}
		}

		// add the vert sizer that will contain the padding label and spin controls
		wxBoxSizer* PaddingVSizer = new wxBoxSizer(wxVERTICAL);
		RightVSizer->Add(PaddingVSizer, 0, wxGROW|wxALL, 0);
		{
			// add the label that will contain the "Padding:"
			wxStaticText* lbl_Padding = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesCommon_Label_Padding")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
			PaddingVSizer->Add(lbl_Padding, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

			// add the horizontal sizer that will contain the padding spin controls
			wxBoxSizer* PaddingHSizer = new wxBoxSizer(wxHORIZONTAL);
			PaddingVSizer->Add(PaddingHSizer, 0, wxGROW|wxALL, 0);
			{
				// add the label for the horizontal padding spinner
				wxStaticText* lbl_HorzPadding = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesCommon_Label_PaddingH")) );
				PaddingHSizer->Add(lbl_HorzPadding, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

				spin_Padding[UIORIENT_Horizontal] = new WxUISpinCtrl(this, ID_UI_EDITSTYLE_TEXTPADDING_H, 0.0f, wxDefaultPosition, wxDefaultSize, -1000.0f, 1000.0f);
				spin_Padding[UIORIENT_Horizontal]->SetToolTip(*LocalizeUI(TEXT("StylePropertiesCommon_ToolTip_PaddingH")));
				PaddingHSizer->Add(spin_Padding[UIORIENT_Horizontal], 1, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);

				// add the label for the vertical padding spinner
				wxStaticText* lbl_VertPadding = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesCommon_Label_PaddingV")) );
				PaddingHSizer->Add(lbl_VertPadding, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

				spin_Padding[UIORIENT_Vertical] = new WxUISpinCtrl(this, ID_UI_EDITSTYLE_TEXTPADDING_V, 0.0f, wxDefaultPosition, wxDefaultSize, -1000.0f, 1000.0f);
				spin_Padding[UIORIENT_Vertical]->SetToolTip(*LocalizeUI(TEXT("StylePropertiesCommon_ToolTip_PaddingV")));
				PaddingHSizer->Add(spin_Padding[UIORIENT_Vertical], 1, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);
			}
		}

// 		// create the controls for editing the style attributes
// 		// create the attributes label
// 		wxStaticText* lbl_Attributes = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesText_Label_Attributes")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
// 		RightVSizer->Add(lbl_Attributes, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);
// 
// 		// create a list of checkboxes, where each checkbox corresponds to a member of the UITextAttributes struct
// 		wxString chklst_AttributesStrings[] = {
// 			*LocalizeUnrealEd(TEXT("Bold")),
// 			*LocalizeUnrealEd(TEXT("Italic")),
// 			*LocalizeUnrealEd(TEXT("Underline")),
// 			*LocalizeUnrealEd(TEXT("Shadow")),
// 			*LocalizeUnrealEd(TEXT("Strikethrough"))
// 		};
// 
// 		chklst_Attributes = new wxCheckListBox( this, ID_UI_EDITSTYLE_CHECKLIST_ATTRIBUTES, wxDefaultPosition, wxSize(-1, 80), 5, chklst_AttributesStrings, wxLB_EXTENDED|wxLB_NEEDED_SB|wxLB_SORT );
// 		RightVSizer->Add(chklst_Attributes, 1, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);
	}

	pnl_Preview = new WxTextPreviewPanel;
	pnl_Preview->Create(this, wxID_ANY, EditingStyle, CurrentState);
	StaticVSizer->Add(pnl_Preview, 1, wxGROW|wxALL, 5);
}

/**
 * Initialize the values of the controls contained by this panel with the values from the current style.
 */
void WxStyleTextPropertiesGroup::InitializeControlValues()
{
	// prevent flickering while updating the values of the controls
	Freeze();

	// Initialize the color controls
	pnl_Color->InitializeControlValues();

	InitializeFontCombo();

	InitializeAlignmentCombos();

	InitializeStringClipping();

	InitializeTextScale();

	InitializePadding();

	InitializeAutoScaling();

	InitializeSpacingAdjust();

	//@todo - disable until this is implemented
	if ( chklst_Attributes != NULL )
	{
		chklst_Attributes->Enable(false);
	}

	RefreshPreview();

	Thaw();
	Refresh();
}

/**
 *	Initializes the font combo box
 */
void WxStyleTextPropertiesGroup::InitializeFontCombo()
{
	// clear the existing values from the font combo box
	cmb_Font->Clear();

	// iterate through all fonts, adding them to the combo
	UFont* InitialFont = NULL;
	const UUIStyle_Text* const InitialStateData = GetCurrentStyleData();
	check(InitialStateData);	

	for ( TObjectIterator<UFont> It; It; ++It )
	{
		UFont* Font = *It;

		if (Font == InitialStateData->StyleFont )
		{
			InitialFont = Font;
		}

		cmb_Font->Append(*Font->GetName(), Font);
	}

	if ( InitialFont == NULL )
	{
		// the font referenced by this style no longer exists, or isn't loaded....set the font to the default small font
		InitialFont = GEngine->SmallFont;
	}

	// set the value of the font combo to the font from the style or multiple values if user is editing multiple states and font data is not the same
	check(InitialFont);
	if ( CompareFontProperty() )
	{
		cmb_Font->SetStringSelection(*InitialFont->GetName());
	}
	else
	{
		cmb_Font->DisplayMultipleValues();
	}	
}

/**
 *	Initializes the alignment combo boxes
 */
void WxStyleTextPropertiesGroup::InitializeAlignmentCombos()
{
	const UUIStyle_Text* const InitialStateData = GetCurrentStyleData();
	check(InitialStateData);

	FString AlignmentString[UIORIENT_MAX];
	TArray<FString>	AlignmentOptions[UIORIENT_MAX];

	for ( INT OrientationIndex = 0; OrientationIndex < UIORIENT_MAX; OrientationIndex++ )
	{
		// clear the existing values of the alignment combos
		cmb_Alignment[OrientationIndex]->Clear();

		// load the alignment options from loc file
		AlignmentString[OrientationIndex] = LocalizeUI(*FString::Printf(TEXT("StylePropertiesText_Alignment[%d]"), OrientationIndex));
		AlignmentString[OrientationIndex].ParseIntoArray(&AlignmentOptions[OrientationIndex], TEXT(","),TRUE);

		// fill the alignment combo with the value from the loc file
		for ( INT AlignmentIndex = 0; AlignmentIndex < AlignmentOptions[OrientationIndex].Num(); AlignmentIndex++ )
		{
			cmb_Alignment[OrientationIndex]->Append(*AlignmentOptions[OrientationIndex](AlignmentIndex));
		}

		if ( CompareTextAlignmentProperty(static_cast<EUIOrientation>(OrientationIndex)) )
		{
			// set the selected alignment for this orientation
			cmb_Alignment[OrientationIndex]->SetSelection(InitialStateData->Alignment[OrientationIndex]);
		}
		else
		{
			cmb_Alignment[OrientationIndex]->DisplayMultipleValues();
		}		
	}
}


/**
 * Initializes the clipping combo boxes.
 */
void WxStyleTextPropertiesGroup::InitializeStringClipping()
{
	const UUIStyle_Text* const InitialStateData = GetCurrentStyleData();
	check(InitialStateData);

	cmb_ClipMode->Freeze();
	{
		// clip mode
		if ( CompareClipModeProperty() )
		{
			cmb_ClipMode->ClearMultipleValues();
			cmb_ClipMode->SetSelection(InitialStateData->ClipMode);
		}
		else
		{
			cmb_ClipMode->DisplayMultipleValues();
		}
	}
	cmb_ClipMode->Thaw();

	cmb_ClipAlignment->Freeze();
	{
		// clip alignment
		if ( CompareClipAlignmentProperty() )
		{
			cmb_ClipAlignment->ClearMultipleValues();
			cmb_ClipAlignment->SetSelection(InitialStateData->ClipAlignment);
		}
		else
		{
			cmb_ClipAlignment->DisplayMultipleValues();
		}
	}
	cmb_ClipAlignment->Thaw();
}

/**
 *	Initializes the scale edit boxes
 */
void WxStyleTextPropertiesGroup::InitializeTextScale()
{
	const UUIStyle_Text* const InitialStateData = GetCurrentStyleData();
	check(InitialStateData);

	for ( INT OrientationIdx= 0; OrientationIdx < UIORIENT_MAX; OrientationIdx++ )
	{
		EUIOrientation Orientation = static_cast<EUIOrientation>(OrientationIdx);
		if ( CompareTextScaleProperty(Orientation) )
		{
			spin_TextScale[Orientation]->ClearMultipleValues();
			spin_TextScale[Orientation]->SetValue( InitialStateData->Scale[Orientation] );
		}
		else
		{
			spin_TextScale[Orientation]->DisplayMultipleValues();
		}
	}
}

/**
 *	Initializes the padding spin controls
 */
void WxStyleTextPropertiesGroup::InitializePadding()
{
	const UUIStyle_Text* const InitialStateData = GetCurrentStyleData();
	check(InitialStateData);

	for ( INT OrientationIdx= 0; OrientationIdx < UIORIENT_MAX; OrientationIdx++ )
	{
		EUIOrientation Orientation = static_cast<EUIOrientation>(OrientationIdx);
		if ( ComparePaddingProperty(Orientation) )
		{
			spin_Padding[Orientation]->ClearMultipleValues();
			spin_Padding[Orientation]->SetValue( InitialStateData->StylePadding[Orientation] );

			if ( pnl_Preview )
			{
				if ( Orientation == UIORIENT_Horizontal )
				{
					pnl_Preview->HorzPadding = InitialStateData->StylePadding[Orientation];
				}
				else
				{
					pnl_Preview->VertPadding = InitialStateData->StylePadding[Orientation];
				}
			}
		}
		else
		{
			spin_Padding[Orientation]->DisplayMultipleValues();
			if ( pnl_Preview )
			{
				if ( Orientation == UIORIENT_Horizontal )
				{
					pnl_Preview->HorzPadding = 0;
				}
				else
				{
					pnl_Preview->VertPadding = 0;
				}
			}
		}
	}
}

/**
 * Initializes the values of the autoscale mode and min autoscale controls.
 */
void WxStyleTextPropertiesGroup::InitializeAutoScaling()
{
	const UUIStyle_Text* const InitialStateData = GetCurrentStyleData();
	check(InitialStateData);

	if ( CompareAutoScaleModeProperty() )
	{
		cmb_AutoScaleMode->ClearMultipleValues();
		cmb_AutoScaleMode->SetSelection(InitialStateData->AutoScaling.AutoScaleMode);
	}
	else
	{
		cmb_AutoScaleMode->DisplayMultipleValues();
	}

	if ( CompareMinAutoScaleValues() )
	{
		spin_AutoScaleMinValue->ClearMultipleValues();
		spin_AutoScaleMinValue->SetValue(InitialStateData->AutoScaling.MinScale);
	}
	else
	{
		spin_AutoScaleMinValue->DisplayMultipleValues();
	}
}

/**
 *	Initializes the spacing adjust edit boxes
 */
void WxStyleTextPropertiesGroup::InitializeSpacingAdjust()
{
	const UUIStyle_Text* const InitialStateData = GetCurrentStyleData();
	check(InitialStateData);

	for ( INT OrientationIdx= 0; OrientationIdx < UIORIENT_MAX; OrientationIdx++ )
	{
		EUIOrientation Orientation = static_cast<EUIOrientation>(OrientationIdx);
		if ( CompareSpacingAdjustProperty(Orientation) )
		{
			spin_SpacingAdjust[Orientation]->ClearMultipleValues();
			spin_SpacingAdjust[Orientation]->SetValue( InitialStateData->SpacingAdjust[Orientation] );
		}
		else
		{
			spin_SpacingAdjust[Orientation]->DisplayMultipleValues();
		}
	}
}

/**
 * Called to notify this property group that the user has selected a new UIStates to be edited
 *
 * @param	SelectedStates	Array of states for which style data is going to be edited
 */
void WxStyleTextPropertiesGroup::NotifySelectedStatesChanged( TArray<UUIState*> & SelectedStates )
{
	// save the configured values for the current states
	SaveChanges();

	WxStylePropertiesGroup::NotifySelectedStatesChanged(SelectedStates);

	// Update current StyleDataArray with selected states' data
	StyleDataArray.Empty();
	for ( INT StateIndex = 0; StateIndex < SelectedStates.Num(); StateIndex++ )
	{
		UUIStyle_Text* StyleData = GetTextDataForState(EditingStyle, SelectedStates(StateIndex));
		if ( StyleData != NULL )
		{
			StyleDataArray.AddItem(StyleData);
		}
	}

	// The color panel needs to be notified about the state change, pass in data from our StyleDataArray which holds the currently selected states data
	TArray<UUIStyle_Data*> BaseData;
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		// convert each item to the base class pointer
		BaseData.AddItem(StyleDataArray(StateIndex));
	}

	pnl_Color->NotifySelectedStatesChanged(BaseData);
	if(SelectedStates.Num() >= 1)
	{
		InitializeControlValues();	
	}
}

/**
 * Disables editing of any properties inside this group
 */
void WxStyleTextPropertiesGroup::DisableControls()
{
	pnl_Color->Disable();
	cmb_Font->Disable();

	for ( INT i = 0; i < UIORIENT_MAX; i++ )
	{
		cmb_Alignment[i]->Disable();
		spin_TextScale[i]->Disable();
		spin_SpacingAdjust[i]->Disable();
		spin_Padding[i]->Disable();
	}

	if ( chklst_Attributes != NULL )
	{
		chklst_Attributes->Disable();
	}
	cmb_ClipMode->Disable();
	cmb_ClipAlignment->Disable();
	cmb_AutoScaleMode->Disable();
	spin_AutoScaleMinValue->Disable();

	if(pnl_Preview)
	{
		pnl_Preview->Disable();
	}
}

/**
 *  Removes any preview controls from the display, used to obtain the minimal set of controls needed to display all the properties
 */
void WxStyleTextPropertiesGroup::RemovePreviewControls()
{
	if(pnl_Preview)
	{
		pnl_Preview->Destroy();
		pnl_Preview=NULL;
	}
}

/**
 *	Refreshes any preview windows in the property group
 */
void WxStyleTextPropertiesGroup::RefreshPreview()
{
	if ( pnl_Preview != NULL )
	{
		pnl_Preview->SetPreviewStyle(EditingStyle, CurrentState);
		pnl_Preview->RefreshViewport();
	}
}

/**
 * Returns the style data for the currently active state
 */
UUIStyle_Text* WxStyleTextPropertiesGroup::GetCurrentStyleData()
{
	UUIStyle_Text* Result = NULL;
	if(StyleDataArray.Num() > 0)
	{
		Result = Cast<UUIStyle_Text>(StyleDataArray(0));
	}

	return Result;
}


/**
 * Returns text style data associated with the specified state of the style we are currently editing, if the style is a combo style then 
 * custom data from its TextStyle reference will be returned
 *
 * @param	Style	style from which the data will be extracted
 * @param	State	state for which the data will be returned
 */
UUIStyle_Text* WxStyleTextPropertiesGroup::GetTextDataForState( const UUIStyle* Style, UUIState* State )
{
	UUIStyle_Text* Result = NULL;

	if(Style->StyleDataClass == UUIStyle_Combo::StaticClass())
	{
		UUIStyle_Combo* ComboData = CastChecked<UUIStyle_Combo>(Style->GetStyleForState(State));
		check(ComboData);

		Result = CastChecked<UUIStyle_Text>(ComboData->TextStyle.GetCustomStyleData());
	}
	else if(Style->StyleDataClass == UUIStyle_Text::StaticClass())
	{
		Result = CastChecked<UUIStyle_Text>(Style->GetStyleForState(State));
	}

	return Result;
}

/** Called when the user changes the font */
void WxStyleTextPropertiesGroup::OnFontChanged( wxCommandEvent& Event )
{
	// guard against selection of <multiple values> item
	if(Event.GetString() != cmb_Font->MultipleValuesString)
	{
		UpdateFontProperty((UFont*)cmb_Font->GetClientData(cmb_Font->GetSelection()));	
		NotifyStyleModified(TRUE);
	}
}

/** Called when the user changes the alignment */
void WxStyleTextPropertiesGroup::OnAlignmentChanged( wxCommandEvent& Event )
{
	EUIOrientation Index = Event.GetId() == ID_UI_EDITSTYLE_COMBO_ALIGNMENT_H
		? UIORIENT_Horizontal
		: UIORIENT_Vertical;

	// guard against selection of <multiple values> item
	if ( Event.GetString() != cmb_Alignment[Index]->MultipleValuesString)
	{
		UpdateAlignmentProperty( Index, (EUIAlignment)cmb_Alignment[Index]->GetSelection() );
		NotifyStyleModified(TRUE);
	}
}

/** Called when the user changes the clipping mode */
void WxStyleTextPropertiesGroup::OnClipModeChanged( wxCommandEvent& Event )
{
	// guard against selection of <multiple values> item
	if ( Event.GetString() != cmb_ClipMode->MultipleValuesString )
	{
		UpdateClipModeProperty(static_cast<ETextClipMode>(cmb_ClipMode->GetSelection()) );
		NotifyStyleModified(TRUE);
	}
}

/** Called when the user changes the clip alignment */
void WxStyleTextPropertiesGroup::OnClipAlignmentChanged( wxCommandEvent& Event )
{
	// guard against selection of <multiple values> item
	if ( Event.GetString() != cmb_ClipAlignment->MultipleValuesString )
	{
		EUIAlignment NewClipAlignment = static_cast<EUIAlignment>(cmb_ClipAlignment->GetSelection());
		UpdateClipAlignmentProperty(NewClipAlignment);
		NotifyStyleModified(TRUE);
	}
}

/** Called when the user changes any of the font attributes */
void WxStyleTextPropertiesGroup::OnAttributesChanged( wxCommandEvent& Event )
{
	//@todo - support for applying attributes
	UpdateAttributesProperty();
	NotifyStyleModified(TRUE);
}

/** Called when the user changes any of the text scale values */
void WxStyleTextPropertiesGroup::OnScaleChanged( wxSpinEvent& Event )
{
	EUIOrientation Orientation = Event.GetId() == ID_UI_EDITSTYLE_TEXTSCALE_X
		? UIORIENT_Horizontal
		: UIORIENT_Vertical;

	if ( Event.GetString() != spin_TextScale[Orientation]->MultipleValuesString )
	{
		UpdateScaleProperty(Orientation, spin_TextScale[Orientation]->GetValue());
		NotifyStyleModified(TRUE);
	}
}

/** Called when the user changes the value of one of the padding spin controls */
void WxStyleTextPropertiesGroup::OnPaddingChanged( wxSpinEvent& Event )
{
	EUIOrientation Orientation = static_cast<EUIOrientation>(Event.GetId() - ID_UI_EDITSTYLE_TEXTPADDING_H);
	if ( Orientation < UIORIENT_MAX )
	{
		if ( Event.GetString() != spin_Padding[Orientation]->MultipleValuesString )
		{
			UpdatePaddingProperty(Orientation, spin_Padding[Orientation]->GetValue());
			NotifyStyleModified(TRUE);
		}
	}
}

/** Called when the user modifies the text value of a padding spin control by editing the text field */
void WxStyleTextPropertiesGroup::OnPaddingChanged( wxCommandEvent& Event )
{
	EUIOrientation Orientation = static_cast<EUIOrientation>(Event.GetId() - ID_UI_EDITSTYLE_TEXTPADDING_H);
	if ( Orientation < UIORIENT_MAX )
	{
		if ( Event.GetString() != spin_Padding[Orientation]->MultipleValuesString )
		{
			UpdatePaddingProperty(Orientation, spin_Padding[Orientation]->GetValue());
			NotifyStyleModified(TRUE);
		}
	}
}

/** called when the user changes the auto-scale mode */
void WxStyleTextPropertiesGroup::OnAutoScaleModeChanged( wxCommandEvent& Event )
{
	if ( Event.GetString() != cmb_AutoScaleMode->MultipleValuesString )
	{
		ETextAutoScaleMode NewAutoScaleMode = static_cast<ETextAutoScaleMode>(cmb_AutoScaleMode->GetSelection());
		UpdateAutoScaleModeProperty(NewAutoScaleMode);
		NotifyStyleModified(TRUE);
	}
}

/** called when the user changes minimum auto-scale value */
void WxStyleTextPropertiesGroup::OnMinAutoScaleChanged( wxSpinEvent& Event )
{
	if ( Event.GetString() != spin_AutoScaleMinValue->MultipleValuesString )
	{
		const FLOAT NewMinAutoScaleFactor = spin_AutoScaleMinValue->GetValue();
		UpdateMinAutoScaleProperty(NewMinAutoScaleFactor);
		NotifyStyleModified(TRUE);
	}
}

/** Called when the user changes any of the text spacing adjust values */
void WxStyleTextPropertiesGroup::OnSpacingAdjustChanged( wxSpinEvent& Event )
{
	EUIOrientation Orientation = Event.GetId() == ID_UI_EDITSTYLE_SPACINGADJUST_X
		? UIORIENT_Horizontal
		: UIORIENT_Vertical;

	if ( Event.GetString() != spin_SpacingAdjust[Orientation]->MultipleValuesString )
	{
		UpdateSpacingAdjustProperty(Orientation, spin_SpacingAdjust[Orientation]->GetValue());
		NotifyStyleModified(TRUE);
	}
}

/**
 * Updates all currently selected States's data with the new font
 * 
 * @param	Font	New font to be assigned to currently selected states data
 */
void WxStyleTextPropertiesGroup::UpdateFontProperty( UFont* Font )
{
	check(Font);
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Text* StyleData = StyleDataArray(StateIndex);
		const UBOOL bValueDiffers = StyleData->StyleFont != Font;

		StyleData->StyleFont = Font;
		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}
}

/**
* Updates all currently selected States's data with the new alignment
*
* @param	OrientationIndex	Which orientation property will receive new Alignment
* @param	Alignment			New Alignment assigned to selected orientation
*/
void WxStyleTextPropertiesGroup::UpdateAlignmentProperty(EUIOrientation OrientationIndex, EUIAlignment Alignment)
{
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Text* StyleData = StyleDataArray(StateIndex);
		const UBOOL bValueDiffers = StyleData->Alignment[OrientationIndex] != Alignment;

		StyleData->Alignment[OrientationIndex] = Alignment;
		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}
}


/**
 * Updates all currently selected States's data with the new clip mode
 * 
 * @param	NewClipMode		the new clip mode to apply to the selected states
 */
void WxStyleTextPropertiesGroup::UpdateClipModeProperty( ETextClipMode NewClipMode )
{
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Text* StyleData = StyleDataArray(StateIndex);
		const UBOOL bValueDiffers = StyleData->ClipMode != NewClipMode;

		StyleData->ClipMode = NewClipMode;
		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}
}

/**
 * Updates all currently selected States's data with the new clip alignment
 * 
 * @param	NewAlignment	the new clip alignment to apply to the selected states
 */
void WxStyleTextPropertiesGroup::UpdateClipAlignmentProperty( EUIAlignment NewAlignment )
{
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Text* StyleData = StyleDataArray(StateIndex);
		const UBOOL bValueDiffers = StyleData->ClipAlignment != NewAlignment;

		StyleData->ClipAlignment = NewAlignment;
		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}
}

/**
 * Updates all currently selected States's data with the new attributes* 
 */
void WxStyleTextPropertiesGroup::UpdateAttributesProperty()
{
	//@todo - apply attributes to the currently selected states of a style
}

/**
 * Changes the scale for the specified orientation in all selected states to the specified value.
 *
 * @param	Orientation		indicates which axis's scale value should be modified
 * @param	NewScale		the new scale to be applied
 */
void WxStyleTextPropertiesGroup::UpdateScaleProperty( EUIOrientation Orientation, FLOAT NewScale )
{
	checkSlow(Orientation<UIORIENT_MAX);

	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Text* StyleData = StyleDataArray(StateIndex);
		const UBOOL bValueDiffers = StyleData->Scale[Orientation] != NewScale;

		StyleData->Scale[Orientation] = NewScale;
		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}
}

/**
 * Changes the padding value for the specified orientation in all selected states.
 *
 * @param	Orientation		indicates which axis's scale value should be modified
 * @param	NewPaddingValue	the new padding value to apply.
 */
void WxStyleTextPropertiesGroup::UpdatePaddingProperty( EUIOrientation Orientation, FLOAT NewPaddingValue )
{
	checkSlow(Orientation<UIORIENT_MAX);

	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Text* StyleData = StyleDataArray(StateIndex);
		const UBOOL bValueDiffers = StyleData->StylePadding[Orientation] != NewPaddingValue;

		StyleData->StylePadding[Orientation] = NewPaddingValue;
		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}

	if ( pnl_Preview )
	{
		if ( Orientation == UIORIENT_Horizontal )
		{
			pnl_Preview->HorzPadding = NewPaddingValue;
		}
		else
		{
			pnl_Preview->VertPadding = NewPaddingValue;
		}
	}
}

/**
 * Changes the AutoScaleMode for all selected states to the specified value.
 *
 * @param	NewAutoScaleMode	the new autoscale mode to apply.
 */
void WxStyleTextPropertiesGroup::UpdateAutoScaleModeProperty( ETextAutoScaleMode NewAutoScaleMode )
{
	checkSlow(NewAutoScaleMode<UIAUTOSCALE_MAX);

	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Text* StyleData = StyleDataArray(StateIndex);
		const UBOOL bValueDiffers = StyleData->AutoScaling.AutoScaleMode != NewAutoScaleMode;

		StyleData->AutoScaling.AutoScaleMode = NewAutoScaleMode;
		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}

	spin_AutoScaleMinValue->Enable(NewAutoScaleMode != UIAUTOSCALE_None);
}

/**
 * Updates all currently selected States's data with the new scale
 *
 * @param	NewMinScale		the new minimum scale to apply
 */
void WxStyleTextPropertiesGroup::UpdateMinAutoScaleProperty( FLOAT NewMinScale )
{
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Text* StyleData = StyleDataArray(StateIndex);
		const UBOOL bValueDiffers = StyleData->AutoScaling.MinScale != NewMinScale;

		StyleData->AutoScaling.MinScale = NewMinScale;
		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}
}

/**
 * Changes the spacing adjust for the specified orientation in all selected states to the specified value.
 *
 * @param	Orientation		indicates which axis's scale value should be modified
 * @param	NewSpacingAdjust		the new spacing adjust to be applied
 */
void WxStyleTextPropertiesGroup::UpdateSpacingAdjustProperty( EUIOrientation Orientation, FLOAT NewSpacingAdjust )
{
	checkSlow(Orientation<UIORIENT_MAX);

	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Text* StyleData = StyleDataArray(StateIndex);
		const UBOOL bValueDiffers = StyleData->SpacingAdjust[Orientation] != NewSpacingAdjust;

		StyleData->SpacingAdjust[Orientation] = NewSpacingAdjust;
		StyleData->SetDirtiness(StyleData->IsDirty() || bValueDiffers);
	}
}

/**
 * Returns TRUE if all the items in the StyleData array have the same font property value	
 */
UBOOL WxStyleTextPropertiesGroup::CompareFontProperty() const
{
	UBOOL bResult = TRUE;

	if(StyleDataArray.Num() > 0)
	{
		const UFont* FirstFont = StyleDataArray(0)->StyleFont;
		const UFont* CurrentFont = NULL;
		check(FirstFont);

		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			CurrentFont = StyleDataArray(StyleIndex)->StyleFont;
			check(CurrentFont);
			if(CurrentFont != FirstFont)
			{
				bResult = FALSE;
				break;
			}
		}		
	}
	return bResult;
}

/**
 * Returns TRUE if all the items in the StyleData array have the same alignment values for the given orientation
 *
 * @param Orientation	orientation for which the alignment will be extracted, value must be less than UIORIENT_MAX
 */
UBOOL WxStyleTextPropertiesGroup::CompareTextAlignmentProperty( EUIOrientation Orientation ) const
{
	check(Orientation < UIORIENT_MAX);
	UBOOL bResult = TRUE;

	if ( StyleDataArray.Num() > 0 )
	{
		const EUIAlignment FirstAlignment = static_cast<EUIAlignment>(StyleDataArray(0)->Alignment[Orientation]);
		EUIAlignment CurrentAlignment;
        
		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			CurrentAlignment = static_cast<EUIAlignment>(StyleDataArray(StyleIndex)->Alignment[Orientation]);
			
			if(CurrentAlignment != FirstAlignment)
			{
				bResult = FALSE;
				break;
			}
		}		
	}
	return bResult;
}


/**
 * @return	TRUE if all the items in the StyleData array have the same clipping mode
 */
UBOOL WxStyleTextPropertiesGroup::CompareClipModeProperty() const
{
	UBOOL bResult = TRUE;

	if ( StyleDataArray.Num() > 0 )
	{
		const ETextClipMode FirstClipMode = static_cast<const ETextClipMode>(StyleDataArray(0)->ClipMode);
		ETextClipMode CurrentClipMode;

		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			CurrentClipMode = static_cast<ETextClipMode>(StyleDataArray(StyleIndex)->ClipMode);
			if ( CurrentClipMode != FirstClipMode )
			{
				bResult = FALSE;
				break;
			}
		}		
	}

	return bResult;
}

/**
 * @return	TRUE if all the items in the StyleData array have the same clipping mode
 */
UBOOL WxStyleTextPropertiesGroup::CompareClipAlignmentProperty() const
{
	UBOOL bResult = TRUE;

	if ( StyleDataArray.Num() > 0 )
	{
		const EUIAlignment FirstAlignment = static_cast<const EUIAlignment>(StyleDataArray(0)->ClipAlignment);
		EUIAlignment CurrentAlignment;

		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			CurrentAlignment = static_cast<EUIAlignment>(StyleDataArray(StyleIndex)->ClipAlignment);
			if ( CurrentAlignment != FirstAlignment )
			{
				bResult = FALSE;
				break;
			}
		}		
	}
	return bResult;
}

/**
 * Returns TRUE if the Scale property for the specified orientation is the same for all items in the StyleData array.
 *
 * @param Orientation	orientation for which the alignment will be extracted, value must be less than UIORIENT_MAX
 */
UBOOL WxStyleTextPropertiesGroup::CompareTextScaleProperty( EUIOrientation Orientation ) const
{
	check(Orientation < UIORIENT_MAX);
	UBOOL bResult = TRUE;

	if ( StyleDataArray.Num() > 0 )
	{
		const FLOAT FirstScale = StyleDataArray(0)->Scale[Orientation];
		FLOAT CurrentScale;

		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			CurrentScale = StyleDataArray(StyleIndex)->Scale[Orientation];
			if(CurrentScale != FirstScale)
			{
				bResult = FALSE;
				break;
			}
		}		
	}
	return bResult;
}

/**
 * @return	TRUE if the padding property has the same value for all elements of the StyleData array.
 */
UBOOL WxStyleTextPropertiesGroup::ComparePaddingProperty( EUIOrientation Orientation ) const
{
	check(Orientation < UIORIENT_MAX);
	UBOOL bResult = TRUE;

	if ( StyleDataArray.Num() > 0 )
	{
		const FLOAT ComparePaddingValue = StyleDataArray(0)->StylePadding[Orientation];
		FLOAT CurrentPadding=0;

		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			CurrentPadding = StyleDataArray(StyleIndex)->StylePadding[Orientation];
			if ( CurrentPadding != ComparePaddingValue )
			{
				bResult = FALSE;
				break;
			}
		}		
	}
	return bResult;
}

/**
 * @return	TRUE if the AutoScaleMode property for all items in the StyleData array have the same value.
 */
UBOOL WxStyleTextPropertiesGroup::CompareAutoScaleModeProperty() const
{
	UBOOL bResult = TRUE;

	if ( StyleDataArray.Num() > 0 )
	{
		ETextAutoScaleMode InitialAutoScaleMode = static_cast<ETextAutoScaleMode>(StyleDataArray(0)->AutoScaling.AutoScaleMode);
		ETextAutoScaleMode CurrentAutoScaleMode;

		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			CurrentAutoScaleMode = static_cast<ETextAutoScaleMode>(StyleDataArray(StyleIndex)->AutoScaling.AutoScaleMode);
			if ( CurrentAutoScaleMode != InitialAutoScaleMode )
			{
				bResult = FALSE;
				break;
			}
		}		
	}

	return bResult;
}

/**
 * @return	TRUE if the AutoScaling.MinScale property for all items in the StyleData array have the same value.
 */
UBOOL WxStyleTextPropertiesGroup::CompareMinAutoScaleValues() const
{
	UBOOL bResult = TRUE;

	if ( StyleDataArray.Num() > 0 )
	{
		const FLOAT FirstMinScale = StyleDataArray(0)->AutoScaling.MinScale;
		FLOAT CurrentMinScale;

		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			CurrentMinScale = StyleDataArray(StyleIndex)->AutoScaling.MinScale;
			if ( CurrentMinScale != FirstMinScale )
			{
				bResult = FALSE;
				break;
			}
		}		
	}

	return bResult;
}

/**
 * Returns TRUE if the SpacingAdjust property for the specified orientation is the same for all items in the StyleData array.
 *
 * @param Orientation	orientation for which the alignment will be extracted, value must be less than UIORIENT_MAX
 */
UBOOL WxStyleTextPropertiesGroup::CompareSpacingAdjustProperty( EUIOrientation Orientation ) const
{
	check(Orientation < UIORIENT_MAX);
	UBOOL bResult = TRUE;

	if ( StyleDataArray.Num() > 0 )
	{
		const FLOAT FirstSpacingAdjust = StyleDataArray(0)->SpacingAdjust[Orientation];
		FLOAT CurrentSpacingAdjust;

		for ( INT StyleIndex = 1; StyleIndex < StyleDataArray.Num(); StyleIndex++ )
		{
			CurrentSpacingAdjust = StyleDataArray(StyleIndex)->SpacingAdjust[Orientation];
			if(CurrentSpacingAdjust != FirstSpacingAdjust)
			{
				bResult = FALSE;
				break;
			}
		}		
	}
	return bResult;
}

// FSerializableObject interface
void WxStyleTextPropertiesGroup::Serialize( FArchive& Ar )
{
	Ar << StyleDataArray;
}

/* ==========================================================================================================
	WxStyleImagePropertiesGroup
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxStyleImagePropertiesGroup,WxStylePropertiesGroup)

BEGIN_EVENT_TABLE(WxStyleImagePropertiesGroup,WxStylePropertiesGroup)
END_EVENT_TABLE()

/** Destructor */
WxStyleImagePropertiesGroup::~WxStyleImagePropertiesGroup()
{
}

/**
 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
 * 
 * @param	InParent			the window that contains this control
 * @param	InID				the window ID to use for this control
 * @param	CurrentStyle		the UIStyle that is being edited
 * @param	InitialState		the state that should be initially active for the current style
 */
void WxStyleImagePropertiesGroup::Create( wxWindow* InParent, wxWindowID InID, class UUIStyle* CurrentStyle, class UUIState* InitialState )
{
	UUIStyle_Image* InitialStyleData = GetImageDataForState(CurrentStyle, InitialState);

	check(InitialStyleData);
	StyleDataArray.AddItem(InitialStyleData);
	WxStylePropertiesGroup::Create(InParent,InID,CurrentStyle,InitialState);
}

/**
 * Creates the controls and sizers for this panel.
 */
void WxStyleImagePropertiesGroup::CreateControls()
{
	check(StyleDataArray.Num() >0);

	wxStaticBox* StaticBox = new wxStaticBox(this, wxID_ANY, *LocalizeUI(TEXT("StylePropertiesImage_Caption")));
	wxStaticBoxSizer* StaticVSizer = new wxStaticBoxSizer(StaticBox, wxVERTICAL);
	SetSizer(StaticVSizer);

	// add the color adjustment panel
	pnl_Color = new WxColorAdjustmentPanel();
	pnl_Color->Create(this, wxID_ANY, StyleDataArray(0), this);
	StaticVSizer->Add(pnl_Color, 0, wxGROW|wxLEFT, 5);

	// add the texture selection controls
	pnl_DefaultTexture = new WxDefaultImagePreviewPanel;
	pnl_DefaultTexture->Create(this, ID_UI_EDITSTYLE_PNL_DEFAULTIMAGE, EditingStyle, CurrentState, this);
	StaticVSizer->Add(pnl_DefaultTexture, 0, wxGROW|wxALL, 0);

	wxBoxSizer* AdjustmentHSizer = new wxBoxSizer(wxHORIZONTAL);
	StaticVSizer->Add(AdjustmentHSizer, 0, wxGROW|wxALL, 0);

	for ( INT i = 0; i < UIORIENT_MAX; i++ )
	{
		pnl_Adjustment[i] = new WxImageAdjustmentPanel;
		pnl_Adjustment[i]->Create(this, ID_UI_EDITSTYLE_PNL_ADJUSTMENT_H + i, (EUIOrientation)i, this, EditingStyle, CurrentState);
		AdjustmentHSizer->Add(pnl_Adjustment[i], 1, wxGROW|wxALL, 5);
	}

	pnl_Preview = new WxImagePreviewPanel;
	pnl_Preview->Create(this, ID_UI_EDITSTYLE_PREVIEW_PANEL, EditingStyle, CurrentState, this);
	StaticVSizer->Add(pnl_Preview, 1, wxGROW|wxALL, 5);
}

/**
 * Initialize the values of the controls contained by this panel with the values from the current style.
 */
void WxStyleImagePropertiesGroup::InitializeControlValues()
{
	Freeze();

	// Initialize the color controls
	pnl_Color->InitializeControlValues();

	// setup the preview images
	InitializeDefaultTexture();

	RefreshPreview();

	for ( INT i = 0; i < UIORIENT_MAX; i++ )
	{
		pnl_Adjustment[i]->InitializeControlValues();
	}

	Thaw();
	Refresh();
}

/** Initialize the default texture control */
void WxStyleImagePropertiesGroup::InitializeDefaultTexture()
{
	// Display <multiple values> in the texture text label if selected states have different texture data
	if(CompareTextureData())
	{
		pnl_DefaultTexture->SetPreviewStyle( EditingStyle, CurrentState );
	}
	else
	{
		pnl_DefaultTexture->DisplayMultipleValues();
	}
}

/**
 * Called to notify this property group that the user has selected a new UIStates to be edited
 *
 * @param	SelectedStates	Array of states for which style data will be edited
 */
void WxStyleImagePropertiesGroup::NotifySelectedStatesChanged(TArray<UUIState*> & SelectedStates)
{
	// save the configured values for the current states
	SaveChanges();

	WxStylePropertiesGroup::NotifySelectedStatesChanged(SelectedStates);
	// Update current StyleDataArray
	StyleDataArray.Empty();
	for( TArray<UUIState*>::TIterator It(SelectedStates); It; ++It)
	{
		UUIStyle_Image* StyleData = GetImageDataForState(EditingStyle, (*It));
		if(StyleData)
		{
			StyleDataArray.AddItem(StyleData);
		}
	}

	for ( INT i = 0; i < UIORIENT_MAX; i++ )
	{
		pnl_Adjustment[i]->NotifySelectedStatesChanged(SelectedStates);
	}

	// The color panel needs to be notified about the state change, pass in data from our StyleDataArray which holds the currently selected states data
	TArray<UUIStyle_Data*> BaseData;
	for(TArray<UUIStyle_Image*>::TIterator It(StyleDataArray); It; ++It)
	{
		// convert each item to the base class pointer
		BaseData.AddItem( (UUIStyle_Data*)(*It));
	}
	pnl_Color->NotifySelectedStatesChanged(BaseData);

	if(SelectedStates.Num() >= 1)
	{
		InitializeControlValues();	
	}
}

/**
 * Disables editing of any properties inside this group
 */
void WxStyleImagePropertiesGroup::DisableControls()
{
	pnl_Color->Disable();

	pnl_DefaultTexture->DisableControls();

	for ( INT i = 0; i < UIORIENT_MAX; i++ )
	{
		pnl_Adjustment[i]->DisableControls();
	}

	if(pnl_Preview)
	{
		pnl_Preview->Disable();
	}
}

/**
 *	returns TRUE if all the items in the styledata array have the same texture property value
 */
UBOOL WxStyleImagePropertiesGroup::CompareTextureData() const
{
	UBOOL Result = TRUE;

	if( StyleDataArray.Num() > 0 )
	{
		const USurface* FirstTexture = StyleDataArray(0)->DefaultImage;
		const USurface* CurrentTexture = NULL;

		for ( INT DataIndex = 0; DataIndex < StyleDataArray.Num(); DataIndex++ )
		{
			CurrentTexture = StyleDataArray(DataIndex)->DefaultImage;
			if(CurrentTexture != FirstTexture)
			{
				Result = FALSE;
				break;
			}
		}		
	}
	return Result;
}

/**
 *  Removes any preview controls from the display, used to obtain the minimal set of controls needed to display all the properties
 */
void WxStyleImagePropertiesGroup::RemovePreviewControls()
{
	if(pnl_Preview)
	{
		pnl_Preview->Destroy();
		pnl_Preview=NULL;
	}
}

/**
 *	Refreshes any preview windows in the property group
 */
void WxStyleImagePropertiesGroup::RefreshPreview()
{
	if ( pnl_Preview != NULL )
	{
		pnl_Preview->SetPreviewStyle(EditingStyle, CurrentState);
		pnl_Preview->RefreshViewport();
	}
}

/**
 * Returns the style data for the currently active state
 */
UUIStyle_Image* WxStyleImagePropertiesGroup::GetCurrentStyleData()
{
	UUIStyle_Image* Result = NULL;
	if(StyleDataArray.Num() > 0)
	{
		Result = Cast<UUIStyle_Image>(StyleDataArray(0));
	}

	return Result;
}

/**
 * Returns text style data associated with the specified state of the style we are currently editing, if the style is a combo style then 
 * custom data from its TextStyle reference will be returned
 *
 * @param	Style	style from which the data will be extracted
 * @param	State	state for which the data will be returned
 */
UUIStyle_Image* WxStyleImagePropertiesGroup::GetImageDataForState( const UUIStyle* Style, UUIState* State )
{
	UUIStyle_Image* Result = NULL;

	if(Style->StyleDataClass == UUIStyle_Combo::StaticClass())
	{
		UUIStyle_Combo* ComboData = CastChecked<UUIStyle_Combo>(Style->GetStyleForState(State));
		check(ComboData);

		Result = CastChecked<UUIStyle_Image>(ComboData->ImageStyle.GetCustomStyleData());
	}
	else if(Style->StyleDataClass == UUIStyle_Image::StaticClass())
	{
		Result = CastChecked<UUIStyle_Image>(Style->GetStyleForState(State));
	}

	return Result;
}

/**
* Applies the specified texture to current style's selected states 
*
* @param	DefaultTexture	Texture to be assigned to each Image style
*/
void WxStyleImagePropertiesGroup::ApplyDefaultTexture(USurface* DefaultTexture)
{
	// Do this only if user has modified the displayed texture
	if ( DefaultTexture != GetCurrentStyleData()->DefaultImage )
	{		
		// Iterate over data for currently selected states and update their textures
		for ( INT i = 0; i < StyleDataArray.Num(); i++ )
		{
			UUIStyle_Image* ImageStyle = StyleDataArray(i);
			ImageStyle->DefaultImage = DefaultTexture;
			ImageStyle->SetDirtiness(TRUE);
		}

		NotifyStyleModified(TRUE);		
	}
}

// FSerializableObject interface
void WxStyleImagePropertiesGroup::Serialize( FArchive& Ar )
{
	Ar << StyleDataArray;
}

/* ==========================================================================================================
	WxStyleComboPropertiesGroup
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxStyleComboPropertiesGroup,WxStylePropertiesGroup)

BEGIN_EVENT_TABLE(WxStyleComboPropertiesGroup,WxStylePropertiesGroup)
	EVT_CHOICE(ID_UI_EDITSTYLE_COMBO_TEXTSTYLE, WxStyleComboPropertiesGroup::OnSelectedStyle)
	EVT_CHOICE(ID_UI_EDITSTYLE_COMBO_IMAGESTYLE, WxStyleComboPropertiesGroup::OnSelectedStyle)
	EVT_CHOICE(ID_UI_EDITSTYLE_COMBO_TEXTSTATE, WxStyleComboPropertiesGroup::OnSelectedStyle)
	EVT_CHOICE(ID_UI_EDITSTYLE_COMBO_IMAGESTATE, WxStyleComboPropertiesGroup::OnSelectedStyle)
	EVT_RADIOBUTTON(ID_UI_EDITSTYLE_COMBO_TEXTEXISTING, WxStyleComboPropertiesGroup::OnRadioButtonPressed)
	EVT_RADIOBUTTON(ID_UI_EDITSTYLE_COMBO_TEXTCUSTOM, WxStyleComboPropertiesGroup::OnRadioButtonPressed)
	EVT_RADIOBUTTON(ID_UI_EDITSTYLE_COMBO_IMAGEEXISTING, WxStyleComboPropertiesGroup::OnRadioButtonPressed)
	EVT_RADIOBUTTON(ID_UI_EDITSTYLE_COMBO_IMAGECUSTOM, WxStyleComboPropertiesGroup::OnRadioButtonPressed)
END_EVENT_TABLE()

/** Destructor */
WxStyleComboPropertiesGroup::~WxStyleComboPropertiesGroup()
{
}

/**
 * Initializes this window when using two-stage dynamic window creation.  Must be the first function called after creation.
 * 
 * @param	InParent			the window that contains this control
 * @param	InID				the window ID to use for this control
 * @param	CurrentStyle		the UIStyle that is being edited
 * @param	InitialState		the state that should be initially active for the current style
 */
void WxStyleComboPropertiesGroup::Create( wxWindow* InParent, wxWindowID InID, class UUIStyle* CurrentStyle, class UUIState* InitialState )
{	
    // Populate all combo styles in the current skin with new custom style data objects if they don't contain them yet
	// This code must be enabled in order for old skins to be compatibile with new custom combo style data
	// if you remove this code then you have to at least invoke CreateCustomStyleData on the currently edited style
	
	CreateCustomStyleData(CurrentStyle);

	UUIStyle_Combo* InitialStyleData = CastChecked<UUIStyle_Combo>(CurrentStyle->GetStyleForState(InitialState));
	check(InitialStyleData);
	StyleDataArray.AddItem(InitialStyleData);
	WxStylePropertiesGroup::Create(InParent,InID,CurrentStyle,InitialState);	
}

/**
 * Populates the ComboStyle's NULL custom style data references with new default objects
 */
void WxStyleComboPropertiesGroup::CreateCustomStyleData(UUIStyle* ComboStyle)
{
	check(ComboStyle);

	// make sure that our parent style contains custom style data objects, if not create them,
	// this code is present for compatibility with old skin files which don't contain custom style data 
	UUIStyle* ArchetypeStyle = Cast<UUIStyle>(ComboStyle->GetArchetype());
	check(ArchetypeStyle);

	if( !ArchetypeStyle->HasAnyFlags(RF_ClassDefaultObject) )
	{
		UBOOL bArchetypeContainsCustomData = TRUE;
		for( TMap<UUIState*, UUIStyle_Data*>::TIterator It(ArchetypeStyle->StateDataMap); It; ++It)
		{
			UUIStyle_Combo* ParentData = Cast<UUIStyle_Combo>(It.Value());
			if(ParentData->ImageStyle.GetCustomStyleData() == NULL || ParentData->TextStyle.GetCustomStyleData() == NULL)
			{
				bArchetypeContainsCustomData = FALSE;
				break;
			}
		}

		if(!bArchetypeContainsCustomData)
		{
			CreateCustomStyleData(ArchetypeStyle);
		}
	}

	BOOL bMarkDirty = FALSE;
	
	// Iterate over this style's data map and make sure it contains data for all styles
	TArray<FUIStateResourceInfo> StateResources;
	SceneManager->GetSupportedUIStates(StateResources);
	for ( INT StateIndex = 0; StateIndex < StateResources.Num(); StateIndex++ )
	{
		FUIStateResourceInfo& ResourceInfo = StateResources(StateIndex);
		UUIState* MenuState = Cast<UUIState>(ResourceInfo.UIResource);

		UUIStyle_Combo* ComboData = Cast<UUIStyle_Combo>(ComboStyle->StateDataMap.FindRef(MenuState));
		check(ComboData);

		// NOTE: We no longer care whether or not a state is enabled (via bEnabled) before propagating to the style
		{
			// Obtain archetype style data to use its objects as templates for our custom data objects
			UUIStyle_Combo* ArchetypeComboData = Cast<UUIStyle_Combo>(ComboStyle->GetArchetypeStyleForState(MenuState));

			// create a custom text style data if one does not exist 
			if( !ComboData->TextStyle.IsCustomStyleDataValid() )
			{
				UUIStyle_Text* TextTemplate = NULL;
				if(ArchetypeComboData)
				{
					TextTemplate = Cast<UUIStyle_Text>(ArchetypeComboData->TextStyle.GetCustomStyleData());
				}
				else
				{
					TextTemplate = Cast<UUIStyle_Text>(UUIStyle_Text::StaticClass()->GetDefaultObject());
				}
				check(TextTemplate);

				UUIStyle_Data* NewTextData = ConstructObject<UUIStyle_Data>
				(
					UUIStyle_Text::StaticClass(),						// class
					ComboData,						    				// outer must be the combo data or this object will not be deep serialized
					NAME_None,	    									// name
					RF_Public|RF_Transactional,							// flags
					TextTemplate										// template
				);

				check(NewTextData);	
				NewTextData->Modify(TRUE);	
				NewTextData->bEnabled = FALSE;   // initially make the style use the existing data
				ComboData->Modify(TRUE);
				ComboData->TextStyle.SetOwnerStyle(ComboStyle);
				ComboData->TextStyle.SetCustomStyleData(NewTextData); 
				
			}

			// create a default custom image style data if one does not exist 
			if( !ComboData->ImageStyle.IsCustomStyleDataValid() )
			{
				UUIStyle_Image* ImageTemplate = NULL;
				
				if(ArchetypeComboData)
				{
					ImageTemplate = Cast<UUIStyle_Image>(ArchetypeComboData->ImageStyle.GetCustomStyleData());
				}
				else
				{
					ImageTemplate = Cast<UUIStyle_Image>(UUIStyle_Image::StaticClass()->GetDefaultObject());
				}
				check(ImageTemplate);

				UUIStyle_Data* NewImageData = ConstructObject<UUIStyle_Data>
				(
					UUIStyle_Image::StaticClass(),						// class
					ComboData,						    				// outer must be the combo data or this object will not be deep serialized
					NAME_None,	    									// name
					RF_Public|RF_Transactional,							// flags
					ImageTemplate										// template
				);
		
				check(NewImageData);	
				NewImageData->Modify(TRUE);	
				NewImageData->bEnabled = FALSE;    // initially make the style use the existing data	
				ComboData->Modify(TRUE);	
				ComboData->ImageStyle.SetOwnerStyle(ComboStyle);
				ComboData->ImageStyle.SetCustomStyleData(NewImageData); 			
				
			}
		}
	}
}

/**
 * Creates the controls and sizers for this panel.
 */
void WxStyleComboPropertiesGroup::CreateControls()
{
	UUIStyle_Combo* InitialStyleData = GetCurrentStyleData();
	check(InitialStyleData);

	wxStaticBox* StaticBox = new wxStaticBox(this, wxID_ANY, *LocalizeUI(TEXT("StylePropertiesCombo_Caption")));
	wxStaticBoxSizer* StaticVSizer = new wxStaticBoxSizer(StaticBox, wxVERTICAL);
	SetSizer(StaticVSizer);

	// create the vert sizer that will contain the text style controls
	wxBoxSizer* TextVSizer = new wxBoxSizer(wxVERTICAL);
	StaticVSizer->Add(TextVSizer, 0, wxGROW|wxALL, 5);
	CreateTextControls(TextVSizer);

	// create the vert sizer that will contain the image style controls
	wxBoxSizer* ImageVSizer = new wxBoxSizer(wxVERTICAL);
	StaticVSizer->Add(ImageVSizer, 0, wxGROW|wxALL, 5);
	CreateImageControls(ImageVSizer);	
}

/**
 * Creates the controls associated with text properties
 *
 *	@param TextSizer	sizer to which the newly created controls will be inserted
 */
void WxStyleComboPropertiesGroup::CreateTextControls( wxBoxSizer* const TextSizer )
{
	check( TextSizer );

	// create the horizontal sizer that will contain the label and radio buttons
	wxBoxSizer* TextStyleSelectionHSizer = new wxBoxSizer(wxHORIZONTAL);
	TextSizer->Add(TextStyleSelectionHSizer, 0, wxGROW|wxALL, 0);

	// create the text style combo label
	lbl_TextStyle = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesCombo_Label_TextStyle")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	TextStyleSelectionHSizer->Add(lbl_TextStyle, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

	wxBoxSizer* RadioGroupHSizer = new wxBoxSizer(wxHORIZONTAL);

	btn_TextExisting = new wxRadioButton();
	btn_TextExisting->Create( this, ID_UI_EDITSTYLE_COMBO_TEXTEXISTING, *LocalizeUI(TEXT("StylePropertiesCombo_RadioButton_Existing")), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	btn_TextCustom = new wxRadioButton();
	btn_TextCustom->Create( this, ID_UI_EDITSTYLE_COMBO_TEXTCUSTOM, *LocalizeUI(TEXT("StylePropertiesCombo_RadioButton_Custom")), wxDefaultPosition, wxDefaultSize);

	RadioGroupHSizer->Add(btn_TextExisting, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxLEFT, 10);
	RadioGroupHSizer->Add(btn_TextCustom, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxRIGHT, 10);
	TextStyleSelectionHSizer->Add(RadioGroupHSizer, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	// create the horizontal sizer that will contain the comboboxes for choosing the source style and state
	wxStaticBox* ExisingStyleStaticBox = new wxStaticBox();
	ExisingStyleStaticBox->Create(this, wxID_ANY, wxString(TEXT("Existing Text Style")));

	wxStaticBoxSizer* TextStyleHSizer = new wxStaticBoxSizer(ExisingStyleStaticBox, wxHORIZONTAL);
	TextSizer->Add(TextStyleHSizer, 0, wxGROW|wxALL, 5);

	// create the combo for choosing the source style
	cmb_TextStyle = new WxUIChoice( this, ID_UI_EDITSTYLE_COMBO_TEXTSTYLE, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmb_TextStyle->SetToolTip(*LocalizeUI(TEXT("StylePropertiesCombo_ToolTip_TextStyle")));
	TextStyleHSizer->Add(cmb_TextStyle, 2, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

	// create the combo for choosing the source state from the chosen style
	cmb_TextState = new WxUIChoice( this, ID_UI_EDITSTYLE_COMBO_TEXTSTATE, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmb_TextState->SetToolTip(*LocalizeUI(TEXT("StylePropertiesCombo_ToolTip_TextState")));
	TextStyleHSizer->Add(cmb_TextState, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

	// create a text properties group panel for editing new custom text style data
	pnl_CustomTextProperties = new WxStyleTextPropertiesGroup(); 
	pnl_CustomTextProperties->Create(this, wxID_ANY, EditingStyle, CurrentState);

	// remove preview controls for the sake of screen real estate
	pnl_CustomTextProperties->RemovePreviewControls();
	TextSizer->Add(pnl_CustomTextProperties, 1, wxGROW|wxALL, 5);
}

/**
 * Creates the controls associated with image properties
 *
 *	@param ImageSizer	sizer to which the newly created controls will be inserted
 */
void WxStyleComboPropertiesGroup::CreateImageControls( wxBoxSizer* const ImageSizer )
{
	check( ImageSizer );

	// create the horizontal sizer that will contain the label and radio buttons
	wxBoxSizer* ImageStyleSelectionHSizer = new wxBoxSizer(wxHORIZONTAL);
	ImageSizer->Add(ImageStyleSelectionHSizer, 0, wxGROW|wxALL, 0);

	// create the image style combo label
	lbl_ImageStyle = new wxStaticText( this, wxID_STATIC, *LocalizeUI(TEXT("StylePropertiesCombo_Label_ImageStyle")), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	ImageStyleSelectionHSizer->Add(lbl_ImageStyle, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

	// create two radio buttons for selecting between existing or custom style
	wxBoxSizer* RadioGroupHSizer = new wxBoxSizer(wxHORIZONTAL);

	btn_ImageExisting = new wxRadioButton();
	btn_ImageExisting->Create( this, ID_UI_EDITSTYLE_COMBO_IMAGEEXISTING, *LocalizeUI(TEXT("StylePropertiesCombo_RadioButton_Existing")), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	btn_ImageCustom = new wxRadioButton();
	btn_ImageCustom->Create( this, ID_UI_EDITSTYLE_COMBO_IMAGECUSTOM, *LocalizeUI(TEXT("StylePropertiesCombo_RadioButton_Custom")), wxDefaultPosition, wxDefaultSize);

	RadioGroupHSizer->Add(btn_ImageExisting, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	RadioGroupHSizer->Add(btn_ImageCustom, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	ImageStyleSelectionHSizer->Add(RadioGroupHSizer, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxRIGHT, 10);


	// create the horizontal sizer that will contain the comboboxes for choosing the source style and state
	wxStaticBox* ExisingStyleStaticBox = new wxStaticBox();
	ExisingStyleStaticBox->Create(this, wxID_ANY, wxString(TEXT("Existing Image Style")));

	wxStaticBoxSizer* ImageStyleHSizer = new wxStaticBoxSizer(ExisingStyleStaticBox, wxHORIZONTAL);
	ImageSizer->Add(ImageStyleHSizer, 0, wxGROW|wxALL, 5);

	// create the image style combo
	cmb_ImageStyle = new WxUIChoice( this, ID_UI_EDITSTYLE_COMBO_IMAGESTYLE, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmb_ImageStyle->SetToolTip(*LocalizeUI(TEXT("StylePropertiesCombo_ToolTip_ImageStyle")));
	ImageStyleHSizer->Add(cmb_ImageStyle, 2, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

	// create the combo for choosing the source state from the chosen style
	cmb_ImageState = new WxUIChoice( this, ID_UI_EDITSTYLE_COMBO_IMAGESTATE, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	cmb_ImageState->SetToolTip(*LocalizeUI(TEXT("StylePropertiesCombo_ToolTip_ImageState")));
	ImageStyleHSizer->Add(cmb_ImageState, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

	// create a image properties group panel for editing new custom image style data
	pnl_CustomImageProperties = new WxStyleImagePropertiesGroup(); 
	pnl_CustomImageProperties->Create(this, wxID_ANY, EditingStyle, CurrentState);

	// remove preview controls for the sake of screen real estate
	pnl_CustomImageProperties->RemovePreviewControls();
	ImageSizer->Add(pnl_CustomImageProperties, 1, wxGROW|wxALL, 5);
}

/**
 * Initialize the values of the controls contained by this panel with the values from the current style.
 */
void WxStyleComboPropertiesGroup::InitializeControlValues()
{
	UUISkin* CurrentSkin = EditingStyle->GetOuterUUISkin();
	check(CurrentSkin);

	// Initialize controls to this current style data
	UUIStyle_Combo* StyleData = GetCurrentStyleData();
	check(StyleData);

	Freeze();

	cmb_TextStyle->Clear();
	cmb_TextState->Clear();
	cmb_ImageStyle->Clear();
	cmb_ImageState->Clear();

	INT TextStyleIndex=0, ImageStyleIndex=0;

	TArray<UUIStyle*> SkinStyles;
	CurrentSkin->GetAvailableStyles(SkinStyles, TRUE);

	// fill the style combos with the available text and image styles
	for ( INT i = 0; i < SkinStyles.Num(); i++ )
	{
		UUIStyle* tmpStyle = SkinStyles(i);
		if ( tmpStyle->StyleDataClass->IsChildOf(UUIStyle_Text::StaticClass()) )
		{
			// add this style to the text combo
			INT idx = cmb_TextStyle->Append(*tmpStyle->GetStyleName(), tmpStyle);

			// if this text style is the current text source for this combo style, save the index; we'll use it later
			if ( tmpStyle == StyleData->TextStyle.GetSourceStyle() )
			{
				TextStyleIndex = idx;
			}
		}
		else if ( tmpStyle->StyleDataClass->IsChildOf(UUIStyle_Image::StaticClass()) )
		{
			// add this style to the image combo
			INT idx = cmb_ImageStyle->Append(*tmpStyle->GetStyleName(), tmpStyle);
	
			// if this image style is the current image source for this combo style, save the index; we'll use it later
			if ( tmpStyle == StyleData->ImageStyle.GetSourceStyle() )
			{
				ImageStyleIndex = idx;
			}
		}
	}

	if(CompareTextStyleProperty())
	{
		cmb_TextStyle->SetSelection( TextStyleIndex );
	}
	else
	{
		cmb_TextStyle->DisplayMultipleValues();
	}

	if(CompareImageStyleProperty())
	{
		cmb_ImageStyle->SetSelection( ImageStyleIndex );
	}
	else
	{
		cmb_ImageStyle->DisplayMultipleValues();
	}

	RefreshAvailableTextStates();
	RefreshAvailableImageStates();

	ApplyTextStyle();
	ApplyImageStyle();

	InitializeCustomTextRadioButtons();
	InitializeCustomImageRadioButtons();
	
	Thaw();
	Refresh();
}


/**
 *	Initializes values of custom style data selection radio buttons
 */
void WxStyleComboPropertiesGroup::InitializeCustomTextRadioButtons()
{
	// Check if custom style data is enabled for all currently selected states or disabled for all currently selected states
	if(CompareCustomTextEnabledProperty())
	{
		// Initialize controls to this current style data
		UUIStyle_Combo* StyleData = GetCurrentStyleData();
		check(StyleData);

		// Disable either the existing style combo boxes or custom style data panels based on current data CutomData value
		if(StyleData->TextStyle.IsCustomStyleDataEnabled() == TRUE)
		{
			cmb_TextStyle->Disable();
			cmb_TextState->Disable();
			pnl_CustomTextProperties->Enable();
			btn_TextCustom->SetValue(true);
		}
		else
		{
			cmb_TextStyle->Enable();
			cmb_TextState->Enable();
			pnl_CustomTextProperties->Disable();
			btn_TextExisting->SetValue(true);
		}
	}
	else
	{
		// If custom style data is not enabled for all states then uncheck both radio buttons and disable both panels
		cmb_TextStyle->Disable();
		cmb_TextState->Disable();
		pnl_CustomTextProperties->Disable();
		btn_TextCustom->SetValue(false);
		btn_TextExisting->SetValue(false);
	}
}

/**
 *	Initializes values of custom style data selection radio buttons
 */
void WxStyleComboPropertiesGroup::InitializeCustomImageRadioButtons()
{
	// Check if custom style data is enabled for all currently selected states or disabled for all currently selected states
	if(CompareCustomImageEnabledProperty())
	{
		// Initialize controls to this current style data
		UUIStyle_Combo* StyleData = GetCurrentStyleData();
		check(StyleData);

		// Disable either the existing style combo boxes or custom style data panels based on current data CutomData value
		if(StyleData->ImageStyle.IsCustomStyleDataEnabled() == TRUE)
		{
			cmb_ImageStyle->Disable();
			cmb_ImageState->Disable();
			pnl_CustomImageProperties->Enable();
			btn_ImageCustom->SetValue(true);
		}
		else
		{
			cmb_ImageStyle->Enable();
			cmb_ImageState->Enable();
			pnl_CustomImageProperties->Disable();
			btn_ImageExisting->SetValue(true);
		}
	}
	else
	{
		// If custom style data is not enabled for all states then uncheck both radio buttons and disable both panels
		cmb_ImageStyle->Disable();
		cmb_ImageState->Disable();
		pnl_CustomImageProperties->Disable();
		btn_ImageCustom->SetValue(false);
		btn_ImageExisting->SetValue(false);
	}
}

/**
 * Called to notify this property group that the style has been changed.  Refreshes the preview window with the style's current values
 *
 * @param	bPropagateStyleChanges	when TRUE the Style editor will propagate changes to the styles which derived 
 *                                  from the editing style and refresh the widgets whose style has been changed.
 *                                  flag set to FALSE will cause reinitialization of controls of this property group
 */
void WxStyleComboPropertiesGroup::NotifyStyleModified( UBOOL bPropagateStyleChanges /*=TRUE*/ )
{
	// Only need to pass the call to custom text and image property groups if we are not propagating changes
	if(bPropagateStyleChanges == FALSE)
	{
		pnl_CustomTextProperties->NotifyStyleModified(bPropagateStyleChanges);
		pnl_CustomImageProperties->NotifyStyleModified(bPropagateStyleChanges);
	}

	if(pnl_TextStyle)
	{
		pnl_TextStyle->RefreshViewport();
	}

	if(pnl_ImageStyle)
	{
		pnl_ImageStyle->RefreshViewport();
	}	
	
	WxStylePropertiesGroup::NotifyStyleModified(bPropagateStyleChanges);
}

/**
 * Clears and refills the source state combo box will the states available in the currently selected source text style.
 */
void WxStyleComboPropertiesGroup::RefreshAvailableTextStates()
{
	cmb_TextState->Freeze();

	// save the currently selected state
	UUIState* SelectedState = GetTextState();
	INT SelectedIndex = 0;

	// clear existing items from the text source state combo
	cmb_TextState->Clear();
	UUIStyle* SelectedStyle = GetTextStyle();

	if ( SelectedStyle != NULL )
	{
		TArray<FUIStateResourceInfo> StateResources;
		SceneManager->GetSupportedUIStates(StateResources);

		// iterate through the list of available UI menu state classes
		for ( INT StateIndex = 0; StateIndex < StateResources.Num(); StateIndex++ )
		{
			FUIStateResourceInfo& ResourceInfo = StateResources(StateIndex);

			// if the selected style supports this menu state, add it to the source state combo
			UUIStyle_Data** StyleData = SelectedStyle->StateDataMap.Find((UUIState*)ResourceInfo.UIResource);
			if ( StyleData != NULL )
			{
				INT idx = cmb_TextState->Append( *ResourceInfo.FriendlyName, ResourceInfo.UIResource );
				if ( ResourceInfo.UIResource == SelectedState )
				{
					SelectedIndex = idx;
				}
			}
		}
	}

	if(SelectedState != NULL && CompareTextStateProperty())
	{
		cmb_TextState->SetSelection(SelectedIndex);
	}
	else
	{
		cmb_TextState->DisplayMultipleValues();
	}

	cmb_TextState->Thaw();
}

/**
 * Clears and refills the source state combo box with the states available in the currently selected source image style.
 */
void WxStyleComboPropertiesGroup::RefreshAvailableImageStates()
{
	cmb_ImageState->Freeze();

	// save the currently selected state
	UUIState* SelectedState = GetImageState();
	INT SelectedIndex = 0;

	// clear existing items from the text source state combo
	cmb_ImageState->Clear();
	UUIStyle* SelectedStyle = GetImageStyle();

	if ( SelectedStyle != NULL )
	{
		TArray<FUIStateResourceInfo> StateResources;
		SceneManager->GetSupportedUIStates(StateResources);

		// iterate through the list of available UI menu state classes
		for ( INT StateIndex = 0; StateIndex < StateResources.Num(); StateIndex++ )
		{
			FUIStateResourceInfo& ResourceInfo = StateResources(StateIndex);

			// if the selected style supports this menu state, add it to the source state combo
			UUIStyle_Data** StyleData = SelectedStyle->StateDataMap.Find((UUIState*)ResourceInfo.UIResource);
			if ( StyleData != NULL )
			{
				INT idx = cmb_ImageState->Append( *ResourceInfo.FriendlyName, ResourceInfo.UIResource );
				if ( ResourceInfo.UIResource == SelectedState )
				{
					SelectedIndex = idx;
				}
			}
		}
	}
	
	if(SelectedState != NULL && CompareImageStateProperty())
	{
		cmb_ImageState->SetSelection(SelectedIndex);
	}
	else
	{		
		cmb_ImageState->DisplayMultipleValues();
	}

	cmb_ImageState->Thaw();
}

/**
* Returns the style data for the currently active state
*/
UUIStyle_Combo* WxStyleComboPropertiesGroup::GetCurrentStyleData() const
{
	return Cast<UUIStyle_Combo>(EditingStyle->GetStyleForState(CurrentState));
}

/**
 * Applies the values from the specified text style
 */
void WxStyleComboPropertiesGroup::ApplyTextStyle()
{
	UUIStyle_Combo* StyleData = GetCurrentStyleData();
	check(StyleData);
	if(pnl_TextStyle)
	{
		pnl_TextStyle->SetPreviewStyle(StyleData->TextStyle.GetSourceStyle(), StyleData->TextStyle.GetSourceState());
	}
}

/**
 * Applies the values from the specified image style
 */
void WxStyleComboPropertiesGroup::ApplyImageStyle()
{
	UUIStyle_Combo* StyleData = GetCurrentStyleData();
	check(StyleData);
	if(pnl_ImageStyle)
	{
		pnl_ImageStyle->SetPreviewStyle(StyleData->ImageStyle.GetSourceStyle(), StyleData->ImageStyle.GetSourceState());
	}
}


/**
 * Called to notify this property group that the user has selected a new UIStates to be edited.
 *
 * @param	SelectedStates	Array of states for which style data will be edited.
 */
void WxStyleComboPropertiesGroup::NotifySelectedStatesChanged( TArray<UUIState*> & SelectedStates )
{
	WxStylePropertiesGroup::NotifySelectedStatesChanged(SelectedStates);
	
	// Update current StyleDataArray with selected states' data
	UUIStyle_Combo* StyleData;
	StyleDataArray.Empty();

	for( TArray<UUIState*>::TIterator It(SelectedStates); It; ++It)
	{
		StyleData = CastChecked<UUIStyle_Combo>(EditingStyle->GetStyleForState(*It));
		if(StyleData)
		{
			StyleDataArray.AddItem(StyleData);
		}
	}

	pnl_CustomTextProperties->NotifySelectedStatesChanged( SelectedStates );
	pnl_CustomImageProperties->NotifySelectedStatesChanged( SelectedStates );


	if(SelectedStates.Num() >= 1)
	{
		InitializeControlValues();	
	}
}

/**
* Disables editing of any properties inside this group
*/
void WxStyleComboPropertiesGroup::DisableControls()
{
	cmb_TextStyle->Disable();
	cmb_TextState->Disable();
	btn_TextExisting->Disable();
	btn_TextCustom->Disable();
	pnl_CustomTextProperties->DisableControls();

	cmb_ImageStyle->Disable();
	cmb_ImageState->Disable();
	btn_ImageExisting->Disable();
	btn_ImageCustom->Disable();
	pnl_CustomImageProperties->DisableControls();

	if(pnl_TextStyle)
	{
		pnl_TextStyle->Disable();
	}

	if(pnl_ImageStyle)
	{
		pnl_ImageStyle->Disable();
	}
}

/**
 *  Removes any preview controls from the display, used to obtain the minimal set of controls needed to display all the properties
 */
void WxStyleComboPropertiesGroup::RemovePreviewControls()
{
	if(pnl_TextStyle)
	{
		pnl_TextStyle->Destroy();
		pnl_TextStyle=NULL;
	}

	if(pnl_ImageStyle)
	{
		pnl_ImageStyle->Destroy();
		pnl_ImageStyle=NULL;
	}
}

/**
 *	returns TRUE if all items in the styledata array have the same value of source text style property
 */
UBOOL WxStyleComboPropertiesGroup::CompareTextStyleProperty() const
{
	UBOOL Result = TRUE;

	if(this->StyleDataArray.Num() > 0)
	{
		const UUIStyle* FirstStyle = StyleDataArray(0)->TextStyle.GetSourceStyle();
		const UUIStyle* CurrentStyle = NULL;

		if( FirstStyle != NULL )
		{
			for(TIndexedContainerConstIterator< TArray<UUIStyle_Combo*> > It(this->StyleDataArray); It;++It)
			{
				CurrentStyle = (*It)->TextStyle.GetSourceStyle();
				check(CurrentStyle);
				if(CurrentStyle != FirstStyle)
				{
					Result = FALSE;
					break;
				}
			}
		}
	}
	return Result;
}


/**
 *	returns TRUE if all items in the StyleData array have the same value of source text state property
 */
UBOOL WxStyleComboPropertiesGroup::CompareTextStateProperty() const
{
	UBOOL Result = TRUE;

	if(this->StyleDataArray.Num() > 0)
	{
		const UUIState* FirstState = StyleDataArray(0)->TextStyle.GetSourceState();
		const UUIState* CurrentState = NULL;
		check(FirstState);

		for(TIndexedContainerConstIterator< TArray<UUIStyle_Combo*> > It(this->StyleDataArray); It; ++It)
		{
			CurrentState = (*It)->TextStyle.GetSourceState();
			check(CurrentState);
			if(CurrentState != FirstState)
			{
				Result = FALSE;
				break;
			}
		}		
	}
	return Result;
}

/**
 *	returns TRUE if all items in the styledata array have the same value of source image style property
 */
UBOOL WxStyleComboPropertiesGroup::CompareImageStyleProperty() const
{
	UBOOL Result = TRUE;

	if(this->StyleDataArray.Num() > 0)
	{
		const UUIStyle* FirstStyle = StyleDataArray(0)->ImageStyle.GetSourceStyle();
		const UUIStyle* CurrentStyle = NULL;

		if( FirstStyle != NULL )
		{
			for(TIndexedContainerConstIterator< TArray<UUIStyle_Combo*> > It(this->StyleDataArray); It; ++It)
			{
				CurrentStyle = (*It)->ImageStyle.GetSourceStyle();
				check(CurrentStyle);
				if(CurrentStyle != FirstStyle)
				{
					Result = FALSE;
					break;
				}
			}		
		}
	}
	return Result;
}

/**
 *	returns TRUE if all items in the StyleData array have the same value of source image state property
 */
UBOOL WxStyleComboPropertiesGroup::CompareImageStateProperty() const
{
	UBOOL Result = TRUE;

	if(this->StyleDataArray.Num() > 0)
	{
		const UUIState* FirstState = StyleDataArray(0)->ImageStyle.GetSourceState();
		const UUIState* CurrentState = NULL;
		check(FirstState);

		for(TIndexedContainerConstIterator< TArray<UUIStyle_Combo*> > It(this->StyleDataArray); It; ++It)
		{
			CurrentState = (*It)->ImageStyle.GetSourceState();
			check(CurrentState);
			if(CurrentState != FirstState)
			{
				Result = FALSE;
				break;
			}
		}		
	}
	return Result;
}


/**
 *	returns TRUE if all items in the StyleData array have the text custom style data enabled or if all have it disabled,
 *  if the bEnabled flag toggles across states then this function will return FALSE
 */
UBOOL WxStyleComboPropertiesGroup::CompareCustomTextEnabledProperty() const
{
	UBOOL Result = TRUE;

	if(this->StyleDataArray.Num() > 0)
	{
		UBOOL FirstData = StyleDataArray(0)->TextStyle.IsCustomStyleDataEnabled();
		UBOOL CurrentData;

		for(TIndexedContainerConstIterator< TArray<UUIStyle_Combo*> > It(this->StyleDataArray); It; ++It)
		{
			CurrentData = (*It)->TextStyle.IsCustomStyleDataEnabled();
			if(CurrentData != FirstData)
			{
				Result = FALSE;
				break;
			}
		}		
	}
	return Result;
}

/**
 *	returns TRUE if all items in the StyleData array have the image custom style data enabled or if all have it disabled,
 *  if the bEnabled flag toggles across states then this function will return FALSE
 */
UBOOL WxStyleComboPropertiesGroup::CompareCustomImageEnabledProperty() const
{
	UBOOL Result = TRUE;

	if(this->StyleDataArray.Num() > 0)
	{
		UBOOL FirstData = StyleDataArray(0)->ImageStyle.IsCustomStyleDataEnabled();
		UBOOL CurrentData;

		for(TIndexedContainerConstIterator< TArray<UUIStyle_Combo*> > It(this->StyleDataArray); It; ++It)
		{
			CurrentData = (*It)->ImageStyle.IsCustomStyleDataEnabled();
			if(CurrentData != FirstData)
			{
				Result = FALSE;
				break;
			}
		}		
	}
	return Result;
}

// FSerializableObject interface
void WxStyleComboPropertiesGroup::Serialize( FArchive& Ar )
{
	Ar << StyleDataArray;
}

/** Called when the user selects a new style in one of the combo boxes */
void WxStyleComboPropertiesGroup::OnSelectedStyle( wxCommandEvent& Event )
{
	switch ( Event.GetId() )
	{
	case ID_UI_EDITSTYLE_COMBO_TEXTSTYLE:
		// guard against selection of <multiple values> item
		if(Event.GetString() != cmb_TextStyle->MultipleValuesString)
		{
			UpdateSourceTextStyle( GetTextStyle() );
			RefreshAvailableTextStates();
			ApplyTextStyle();
			NotifyStyleModified(TRUE);
		}
		break;

	case ID_UI_EDITSTYLE_COMBO_TEXTSTATE:
		// guard against selection of <multiple values> item
		if(Event.GetString() != cmb_TextState->MultipleValuesString)
		{
			UpdateSourceTextState( GetTextState() );
			ApplyTextStyle();
			NotifyStyleModified(TRUE);
		}
		break;


	case ID_UI_EDITSTYLE_COMBO_IMAGESTYLE:
		// guard against selection of <multiple values> item
		if(Event.GetString() != cmb_ImageStyle->MultipleValuesString)
		{
			UpdateSourceImageStyle( GetImageStyle() );
			RefreshAvailableImageStates();
			ApplyImageStyle();
			NotifyStyleModified(TRUE);
		}
		break;

	case ID_UI_EDITSTYLE_COMBO_IMAGESTATE:
		// guard against selection of <multiple values> item
		if(Event.GetString() != cmb_ImageState->MultipleValuesString)
		{
			UpdateSourceImageState( GetImageState() );
			ApplyImageStyle();
			NotifyStyleModified(TRUE);
		}
		break;
	}
}

/**
 * Called when the user changes the style radio buttons selection
 */
void WxStyleComboPropertiesGroup::OnRadioButtonPressed( wxCommandEvent& Event )
{
	switch ( Event.GetId() )
	{
	case ID_UI_EDITSTYLE_COMBO_TEXTEXISTING:
		EnableCustomTextStyleData(FALSE);
		InitializeCustomTextRadioButtons();
		NotifyStyleModified(TRUE);
		break;

	case ID_UI_EDITSTYLE_COMBO_TEXTCUSTOM:
		EnableCustomTextStyleData(TRUE);
		InitializeCustomTextRadioButtons();
		NotifyStyleModified(TRUE);
		break;

	case ID_UI_EDITSTYLE_COMBO_IMAGEEXISTING:
		EnableCustomImageStyleData(FALSE);
		InitializeCustomImageRadioButtons();
		NotifyStyleModified(TRUE);
		break;
		
	case ID_UI_EDITSTYLE_COMBO_IMAGECUSTOM:
		EnableCustomImageStyleData(TRUE);
		InitializeCustomImageRadioButtons();
		NotifyStyleModified(TRUE);
		break;	
	}
}

/**
* Updates all currently selected States's Combo data with new source text style
*
* @param	SourceStyle		New source style for the text attribute
*/
void WxStyleComboPropertiesGroup::UpdateSourceTextStyle(UUIStyle* SourceStyle)
{
	check(SourceStyle);

	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Combo* ComboData = StyleDataArray(StateIndex);
		ComboData->TextStyle.SafeSetStyle(SourceStyle);
		ComboData->SetDirtiness(TRUE);
	}
}

/**
* Updates all currently selected States's Combo data with new source text state
*
* @param	SourceState		New source state for the text attribute
*/
void WxStyleComboPropertiesGroup::UpdateSourceTextState(UUIState* SourceState)
{
	check(SourceState);

	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Combo* ComboData = StyleDataArray(StateIndex);
		ComboData->TextStyle.SafeSetState(SourceState);
		ComboData->SetDirtiness(TRUE);
	}
}

/**
* Updates all currently selected States's Combo data with new source image style
*
* @param	SourceStyle		New source style for the image attribute
*/
void WxStyleComboPropertiesGroup::UpdateSourceImageStyle(UUIStyle* SourceStyle)
{
	check(SourceStyle);

	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Combo* ComboData = StyleDataArray(StateIndex);
		ComboData->ImageStyle.SafeSetStyle(SourceStyle);
		ComboData->SetDirtiness(TRUE);
	}
}

/**
* Updates all currently selected States's Combo data with new source image state
*
* @param	SourceState		New source state for the image attribute
*/
void WxStyleComboPropertiesGroup::UpdateSourceImageState(UUIState* SourceState)
{
	check(SourceState);

	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Combo* ComboData = StyleDataArray(StateIndex);
		ComboData->ImageStyle.SafeSetState(SourceState);
		ComboData->SetDirtiness(TRUE);
	}
}

/**
 * Enables or disables the use of the custom style data in TextStyle FStyleDataReference objects for currently selected states
 */
void WxStyleComboPropertiesGroup::EnableCustomTextStyleData(UBOOL Flag)
{
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Combo* ComboData = StyleDataArray(StateIndex);
		ComboData->TextStyle.EnableCustomStyleData(Flag);
		ComboData->SetDirtiness(TRUE);
	}
}

/**
 * Enables or disables the use of the custom style data in TextStyle FStyleDataReference objects for currently selected states
 */
void WxStyleComboPropertiesGroup::EnableCustomImageStyleData(UBOOL Flag)
{
	for ( INT StateIndex = 0; StateIndex < StyleDataArray.Num(); StateIndex++ )
	{
		UUIStyle_Combo* ComboData = StyleDataArray(StateIndex);
		ComboData->ImageStyle.EnableCustomStyleData(Flag);
		ComboData->SetDirtiness(TRUE);
	}
}


UUIStyle* WxStyleComboPropertiesGroup::GetTextStyle() const
{
	UUIStyle* Result = NULL;
	UUIStyle_Combo* StyleData = GetCurrentStyleData();
	check(StyleData);

	if ( cmb_TextStyle != NULL && cmb_TextStyle->GetCount() > 0 )
	{
		Result = (UUIStyle*)cmb_TextStyle->GetClientData(cmb_TextStyle->GetSelection());
	}
	else if ( StyleData != NULL )
	{
		Result = StyleData->TextStyle.GetSourceStyle();
	}

	return Result;
}

UUIStyle* WxStyleComboPropertiesGroup::GetImageStyle() const
{
	UUIStyle* Result = NULL;
	UUIStyle_Combo* StyleData = GetCurrentStyleData();
	check(StyleData);

	if ( cmb_ImageStyle != NULL && cmb_ImageStyle->GetCount() > 0 )
	{
		Result = (UUIStyle*)cmb_ImageStyle->GetClientData(cmb_ImageStyle->GetSelection());
	}
	else if ( StyleData != NULL )
	{
		Result = StyleData->ImageStyle.GetSourceStyle();
	}

	return Result;
}

UUIState* WxStyleComboPropertiesGroup::GetTextState() const
{
	UUIState* Result = NULL;
	UUIStyle_Combo* StyleData = GetCurrentStyleData();
	check(StyleData);

	if ( cmb_TextState != NULL && cmb_TextState->GetCount() > 0 )
	{
		Result = (UUIState*)cmb_TextState->GetClientData(cmb_TextState->GetSelection());
	}
	else if ( StyleData != NULL )
	{
		Result = StyleData->TextStyle.GetSourceState();
	}

	return Result;
}

UUIState* WxStyleComboPropertiesGroup::GetImageState() const
{
	UUIState* Result = NULL;
	UUIStyle_Combo* StyleData = GetCurrentStyleData();
	check(StyleData);

	if ( cmb_ImageState != NULL && cmb_ImageState->GetCount() > 0 )
	{
		Result = (UUIState*)cmb_ImageState->GetClientData(cmb_ImageState->GetSelection());
	}
	else if ( StyleData != NULL )
	{
		Result = StyleData->ImageStyle.GetSourceState();
	}

	return Result;
}

/* ==========================================================================================================
	WxDlgEditUIStyle
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxDlgEditUIStyle,wxDialog)

BEGIN_EVENT_TABLE(WxDlgEditUIStyle,wxDialog)
	EVT_BUTTON(wxID_OK, WxDlgEditUIStyle::OnClick_CloseDialog)
	EVT_BUTTON(wxID_CANCEL, WxDlgEditUIStyle::OnClick_CloseDialog)
	EVT_CLOSE(WxDlgEditUIStyle::OnCloseWindow)
	EVT_CHECKLISTBOX(ID_UI_EDITSTYLE_STATELIST, WxDlgEditUIStyle::OnMenuStateToggled)
	EVT_LISTBOX(ID_UI_EDITSTYLE_STATELIST, WxDlgEditUIStyle::OnMenuStateChanged)
END_EVENT_TABLE()

/**
 * Constructor
 */
WxDlgEditUIStyle::WxDlgEditUIStyle() : Style(NULL), pnl_Common(NULL), pnl_Custom(NULL), btn_OK(NULL), btn_Cancel(NULL), PropagatorArchive(NULL)
{

}

/**
 * Destructor
 * Saves the window position and size to the ini
 */
WxDlgEditUIStyle::~WxDlgEditUIStyle()
{
	delete PropagatorArchive;
	UnregisterCallbacks();

	FWindowUtil::SavePosSize(TEXT("EditStyleDialog"), this);
}

/**
 * Initialize this dialog.  Must be the first function called after creating the dialog.
 *
 * @param	EditStyle	the style being edited
 * @param	InParent	the window that opened this dialog
 * @param	InID		the ID to use for this dialog
 */
void WxDlgEditUIStyle::Create( UUIStyle* EditStyle, wxWindow* InParent, wxWindowID InID/*=ID_UI_EDITSTYLE_DLG*/, long AdditionalButtonIds/*=0*/ )
{

	Style = EditStyle;

	// Serialize current style property values into an archive
	OriginalStyleArchive.ActivateWriter();
	OriginalStyleArchive.SerializeObject(Style);
	OriginalStyleArchive.SerializeObject(Style->GetOutermost());
	
	// Prepare to propagate style changes to derived styles
 	PreEditStyleProperty();

	verify(wxDialog::Create(
		InParent, InID, 
		*FString::Printf(LocalizeSecure(LocalizeUI(TEXT("DlgEditStyle_Title_f")), *(EditStyle->StyleName + TEXT(" (") + EditStyle->GetOuter()->GetName() + TEXT(")")))),
		wxDefaultPosition, wxDefaultSize, 
		wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxCLIP_CHILDREN
		));

	SetExtraStyle(GetExtraStyle()|wxWS_EX_TRANSIENT|wxWS_EX_BLOCK_EVENTS|wxWS_EX_VALIDATE_RECURSIVELY);

	CreateControls(AdditionalButtonIds);
	RegisterCallbacks();

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();

	wxRect DefaultPos = GetRect();
	FWindowUtil::LoadPosSize( TEXT("EditStyleDialog"), this, DefaultPos.GetLeft(), DefaultPos.GetTop(), 400, DefaultPos.GetHeight() );

	// This preview update must be called otherwise text preview window will be blank initially
	RefreshPanels(FALSE);
}

/**
 * Creates the controls and sizers for this dialog.
 */
void WxDlgEditUIStyle::CreateControls( long AdditionalButtons/*=0*/ )
{
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(BaseVSizer);

	UUIState* CurrentState = UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>();

	// add the box with the common properties
	pnl_Common = new WxStyleCommonPropertiesGroup();
	pnl_Common->Create(this, wxID_ANY, Style, CurrentState);
	BaseVSizer->Add(pnl_Common, 0, wxGROW|wxALL, 5);

	// create the custom panel for editing this style type
	check(Style->StyleDataClass);
	check(Style->StyleDataClass->GetDefaultObject<UUIStyle_Data>()->UIEditorControlClass.Len() > 0);
	pnl_Custom = wxDynamicCast(wxCreateDynamicObject(*Style->StyleDataClass->GetDefaultObject<UUIStyle_Data>()->UIEditorControlClass),WxStylePropertiesGroup);
	pnl_Custom->Create(this, wxID_ANY, Style, CurrentState);
	BaseVSizer->Add(pnl_Custom, 1, wxGROW|wxALL, 5);

	// add the horz sizer for the ok/cancel buttons
	wxBoxSizer* ButtonHSizer = new wxBoxSizer(wxHORIZONTAL);
	BaseVSizer->Add(ButtonHSizer, 0, wxALIGN_RIGHT|wxALL, 2);

	// create the OK button
	btn_OK = new wxButton( this, wxID_OK, *LocalizeUnrealEd(TEXT("&OK")) );
	ButtonHSizer->Add(btn_OK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	// create the cancel button
	btn_Cancel = new wxButton( this, wxID_CANCEL, *LocalizeUnrealEd(TEXT("&Cancel")) );
	ButtonHSizer->Add(btn_Cancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	if ( AdditionalButtons != 0 )
	{
		if ( (AdditionalButtons&ID_CANCEL_ALL) != 0 )
		{
			// create the "Cancel All" button
			wxButton* btn_CancelAll = new wxButton( this, ID_CANCEL_ALL, *LocalizeUnrealEd(TEXT("&CancelAll")) );
			ButtonHSizer->Add(btn_CancelAll, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		}
	}
}

void WxDlgEditUIStyle::RegisterCallbacks()
{
	// Register callbacks to listen for events
	GCallbackEvent->Register(CALLBACK_UIEditor_StyleModified, this);
	GCallbackEvent->Register(CALLBACK_UIEditor_PropertyModified, this);
}

void WxDlgEditUIStyle::UnregisterCallbacks()
{
	// Unregister callbacks
	GCallbackEvent->UnregisterAll(this);
}

/**
 * Notifies all control groups to update any preview windows.  Called when something which would affect the appearance
 * of a style has been changed by the user.
 *
 * @param	bReinitializeControls	if TRUE, the property group controls will be reinitialized as well.
 */
void WxDlgEditUIStyle::RefreshPanels(UBOOL bReinitializeControls /*=FALSE*/)
{
	if(bReinitializeControls)
	{
		pnl_Common->NotifyStyleModified(FALSE);
		pnl_Custom->NotifyStyleModified(FALSE);
	}
	else
	{
		pnl_Common->RefreshPreview();
		pnl_Custom->RefreshPreview();
	}
}

/**
 * Applies changes made in the dialog to the UIStyle being edited. 
 */
void WxDlgEditUIStyle::SaveChanges()
{
	pnl_Common->SaveChanges();

	// Update the style tag only if the new tag is unique
	UUISkin* Skin = Cast<UUISkin>(Style->GetOuter());
	verify(Skin);
// 	if( Skin->IsUniqueTag(Style->StyleTag) )
// 	{
// 		Style->StyleTag = WorkingStyle->StyleTag;
// 	}	

	// Propagate style changes to derived styles
	PostEditStyleProperty();

	// initialize the propagator for the next change to style properties
	PreEditStyleProperty();
}

/**
 * Function saves changes done on the working style back to the original style, and notifies
 * all objects about the style change
 */
void WxDlgEditUIStyle::SaveChangesAndNotify()
{
	// Since we are working with a copy of the original style we have to save the changes back
	// to the original style
	SaveChanges();

	SendStyleModifiedEvent();
}

/**
* Function reverts the data in the working style for the states which are unchecked to its archetype value. 
* If there is data which has been modified by the user which will be reverted, he will be prompted to
* confirm or cancel the revert process.
*
* @return	TRUE if revert process of unchecked states succeeded, or FALSE if it was stopped by the user. 
*/
UBOOL WxDlgEditUIStyle::DeserializeUncheckedStates()
{
	if( pnl_Common->VerifyUncheckedStateChanges() )
	{
		pnl_Common->DeserializeUncheckedStates();
		return TRUE;
	}
	else
	{
		return FALSE;
	}		
}


/**
 * Function saves the current state of the edited style and all of its derived styles to the undo buffer
 */
void WxDlgEditUIStyle::CreateUndoTransaction()
{
	check(Style);
	UUISkin* Skin = CastChecked<UUISkin>(Style->GetOuter());

	FReloadObjectArc CurrentStyleArchive;	// temporary archive which stores current style data

	// Save current style values into a temporary archive
	CurrentStyleArchive.ActivateWriter();
	CurrentStyleArchive.SerializeObject(Style);
	CurrentStyleArchive.SerializeObject(Style->GetOutermost());
	
	RestoreOriginalStyleValues();

	// Begin the scoped transaction session
	const FScopedTransaction Transaction( *LocalizeUI(TEXT("TransEditStyle")) );
	
	// Gather current style and all its derived styles
	TArray<UUIStyle*> AffectedStyles;
	Skin->GetDerivedStyles(Style,AffectedStyles,UUISkin::DERIVETYPE_All);
	AffectedStyles.AddItem(Style);

	// Iterate over all gathered styles and save their data to the undo buffer, so that a single undo action
	// restores all style data back to its original state
	for( INT StyleIndex = 0; StyleIndex < AffectedStyles.Num(); StyleIndex++ )
	{
		UUIStyle* StyleToModify = AffectedStyles(StyleIndex);
		StyleToModify->Modify();

		// Save each style's state data
		for(TMap<UUIState*,UUIStyle_Data*>::TIterator StateIterator(StyleToModify->StateDataMap); StateIterator; ++StateIterator)
		{
			UUIStyle_Data* StateData = StateIterator.Value();
			check(StateData);

			StateData->Modify();
		}
	}	

	// Copy back the modified values
	CurrentStyleArchive.ActivateReader();
	CurrentStyleArchive.SerializeObject(Style);
	CurrentStyleArchive.SerializeObject(Style->GetOutermost());

	// Propagate style changes to derived styles
	PostEditStyleProperty();

	// initialize the propagator for the next change to style properties
	PreEditStyleProperty();
}

/** Function deletes the current archive and serializes derived styles into a newly create PropagatorArchive */
void WxDlgEditUIStyle::PreEditStyleProperty()
{	
	// Save current values into the archive to prepare it for another property change
	if(PropagatorArchive == NULL)
	{
		PropagatorArchive = new FReloadObjectArc();
		check(PropagatorArchive);
	}

	SerializeDerivedStyles(Style, PropagatorArchive, TRUE);
}

/** Function serializes derived styles from existing PropagatorArchive */
void WxDlgEditUIStyle::PostEditStyleProperty()
{
	// Propagate style changes to the derived styles by serializing objects from the archive
	check(PropagatorArchive);
	SerializeDerivedStyles(Style, PropagatorArchive, FALSE);

	delete PropagatorArchive;
	PropagatorArchive = NULL;
}

/** Function reverts the current style values and all of the derived style values to the previously saved originals */
void WxDlgEditUIStyle::RestoreOriginalStyleValues()
{
	OriginalStyleArchive.ActivateReader();
	OriginalStyleArchive.SerializeObject(Style);
	OriginalStyleArchive.SerializeObject(Style->GetOutermost());

	// Propagate style changes to derived styles
	// read the original values for this style from the archive 
	PostEditStyleProperty();

	// initialize the propagator for the next change to style properties
	PreEditStyleProperty();
}

/**
 * Function stores or reloads current state of passed style and all styles derived from a reload archive 
 *
 * @param	CurrentStyle	Data for this style and all styles derived from it will be saved/reloaded
 * @param	Archive			Specifies which archive object to use in the store/reload process
 * @param	WriteFlag		If TRUE, process will save the state, if FALSE a reload will be performed
 */
void WxDlgEditUIStyle::SerializeDerivedStyles( UUIStyle* const CurrentStyle, FReloadObjectArc* Archive, UBOOL WriteFlag )
{
	check(CurrentStyle);
	check(Archive);

	if(WriteFlag)
	{
		Archive->ActivateWriter();
	}
	else
	{
		Archive->ActivateReader();
	}
	
	// Serialize the CurrentStyle and derived styles
	UUISkin* Skin = CastChecked<UUISkin>(CurrentStyle->GetOuter());

	// Get the list of all styles derived from this parent style
	TArray<UUIStyle*> DerivedStyles;

	// even though we're going to recursively serialize all styles based on this one, for now we'll only retrieve the
	// styles that have CurrentStyle as their ObjectArchetype, so that re-initialization of the style objects happens
	// in the correct order.
	Skin->GetDerivedStyles(CurrentStyle, DerivedStyles, UUISkin::DERIVETYPE_DirectOnly);

	for ( INT StyleIndex = 0; StyleIndex < DerivedStyles.Num(); StyleIndex++ )
	{
		UUIStyle* DerivedStyle = DerivedStyles(StyleIndex);

		// serialize the style to the archive
		Archive->SerializeObject(DerivedStyle);

		// serialize this style's derived styles
		SerializeDerivedStyles(DerivedStyle, Archive, WriteFlag);
	}
}


/**
 * Delegate a StyleModifiedEvent from this StyleEditor instance.  Notifies all objects about 
 * the change in the style this style editor is referencing.  This should be called whenever
 * the style has been modified through this editor.  If there are any derived styles from this style
 * same event will be generated for them as well, as they could change as a result of parent style change
 */
void WxDlgEditUIStyle::SendStyleModifiedEvent()
{
	// This editor also responds to the following event, but there is no need to notify ourselves
	// about the style change, temporarily unregister ourselves as listener for this event
	GCallbackEvent->Unregister( CALLBACK_UIEditor_StyleModified, this );

	// Gather all the derived styles from this style
	UUISkin* Skin = Cast<UUISkin>(Style->GetOuter());
	check(Skin);

	TArray<UUIStyle*> DerivedStyles;
	Skin->GetDerivedStyles(Style, DerivedStyles, UUISkin::DERIVETYPE_All, UUISkin::SEARCH_SameSkinOnly );
	DerivedStyles.AddItem(Style);

	// Send a StyleModified event for each item
	for(TArray<UUIStyle*>::TIterator It(DerivedStyles); It; ++It)
	{
		GCallbackEvent->Send( CALLBACK_UIEditor_StyleModified, (*It) );
	}	

	// Now we can register ourselves back as event listener
	GCallbackEvent->Register( CALLBACK_UIEditor_StyleModified, this );	
}

void WxDlgEditUIStyle::Send( ECallbackEventType InType, UObject* InObject )
{
	switch ( InType )
	{	
		case CALLBACK_UIEditor_StyleModified:
			UpdateReferencedStyle(Cast<UUIStyle>(InObject));
			break;
		case CALLBACK_UIEditor_PropertyModified:
			if(Cast<UUIStyle>(InObject) == Style )
			{
				SaveChangesAndNotify();
			}
			break;
	}
}

/**
 *	Handler for external style modification event
 *	Called when a style has been modified by another instance of style editor.
 *	If the modified style is the one we are currently referencing, then we must update our data
 */
void WxDlgEditUIStyle::UpdateReferencedStyle(UUIStyle * UpdatedStyle)
{
	// We should respond to this event only if we reference the modified style directly,
	// or indirectly, i.e. Combo Style
	if(UpdatedStyle && 	(UpdatedStyle == Style || Style->ReferencesStyle(UpdatedStyle)) )  
	{
		// The style referenced by this window has been modified externally, so we will 
		// throw out existing StyleValues and reset all the values currently displayed
		// to ones contained in the UpdatedStyle
		
        // Update the archived original values 
		// Serialize current style property values into an archive
		OriginalStyleArchive.ActivateWriter();
		OriginalStyleArchive.SerializeObject(Style);
		OriginalStyleArchive.SerializeObject(Style->GetOutermost());

		RefreshPanels(TRUE);
	}
}


/**
 * Called when the user clicks the checkbox next to a menu state.  Notifies the Combo style properties panel (if applicable).
 */
void WxDlgEditUIStyle::OnMenuStateToggled( wxCommandEvent& Event )
{
	check(pnl_Custom);

	WxStyleComboPropertiesGroup* ComboPanel = wxDynamicCast(pnl_Custom,WxStyleComboPropertiesGroup);
	if ( ComboPanel != NULL )
	{
		// have the combo panel create custom combo style data objects for any newly enabled state which didn't have them already.
		ComboPanel->CreateCustomStyleData(Style);
	}
}

/**
 * Called when the user selects a different menu state from the list of UIStyles available for this state.  Notifies all
 * panels that the "CurrentState" has been modified.
 */
void WxDlgEditUIStyle::OnMenuStateChanged( wxCommandEvent& Event )
{
	check(pnl_Common);
	// Obtain the currently selected states
	TArray<UUIState*> SelectedStates; 	
	pnl_Common->GetSelectedStates(SelectedStates);
	
	pnl_Common->NotifySelectedStatesChanged(SelectedStates);
	pnl_Custom->NotifySelectedStatesChanged(SelectedStates);
}

/**
 * Called when the user clicks the OK/cancel button.
 */
void WxDlgEditUIStyle::OnClick_CloseDialog( wxCommandEvent& Event )
{
	SetReturnCode(Event.GetId());
	if ( !IsModal() )
	{

		if(Event.GetId() == wxID_CANCEL )
		{
			RestoreOriginalStyleValues();
			SendStyleModifiedEvent();	
			Close();
		}
		if ( Event.GetId() == wxID_OK && Validate() && TransferDataFromWindow() )
		{
			if( DeserializeUncheckedStates())
			{
				SaveChanges(); 
				CreateUndoTransaction();
				SendStyleModifiedEvent();

				// Now that all widgets should be updated with the new style we can clear any dirty flags
				pnl_Common->ClearDirtyFlags();

				Close();
			}
			else
			{
				/* user canceled the revert of unchecked states, don't close the dialog */
			}
		}
	}
	else
	{
		Event.Skip();
	}
}

/**
 * Called when the dialog is closed
 */
void WxDlgEditUIStyle::OnCloseWindow( wxCloseEvent& Event )
{
	if ( !IsModal() )
	{
		this->Destroy();
	}
	else
	{
		Event.Skip();
	}

}


/* ==========================================================================================================
WxDlgDiffStateProperties
========================================================================================================== */

IMPLEMENT_DYNAMIC_CLASS(WxDlgDiffStateProperties,wxDialog)

BEGIN_EVENT_TABLE(WxDlgDiffStateProperties,wxDialog)
	EVT_RADIOBUTTON(ID_UI_DIFFSTATES_LAYOUTHORIZONTAL, WxDlgDiffStateProperties::OnLayoutChanged)
	EVT_RADIOBUTTON(ID_UI_DIFFSTATES_LAYOUTVERTICAL, WxDlgDiffStateProperties::OnLayoutChanged)
END_EVENT_TABLE()

/**
* Constructor
*/
WxDlgDiffStateProperties::WxDlgDiffStateProperties():  Style(NULL), FirstState(NULL), SecondState(NULL), FirstStatePanel(NULL),
SecondStatePanel(NULL), DividerLine(NULL)
{
	CurrentLayout = wxHORIZONTAL;
}

/**
* Destructor
* Saves the window position and size to the ini
*/
WxDlgDiffStateProperties::~WxDlgDiffStateProperties()
{
	FWindowUtil::SavePosSize(TEXT("DiffStatesDialog"), this);
}

/**
* Initialize this dialog.  Must be the first function called after creating the dialog.
*
* @param	ViewStyle			the style which states are to be compared
* @param	FirstDiffState		first state to be compared
* @param	SecondDiffState		second state to be compared
* @param	InParent			the window that opened this dialog
* @param	InID				the ID to use for this dialog
*/
void WxDlgDiffStateProperties::Create( UUIStyle* ViewStyle, UUIState* FirstDiffState, UUIState* SecondDiffState,
			wxWindow* InParent, wxWindowID InID /*=ID_UI_DIFFSTATES_DLG*/)
{
	check(ViewStyle && FirstDiffState && SecondDiffState);

	Style = ViewStyle;

	FirstState = FirstDiffState;
	SecondState = SecondDiffState;

	verify(wxDialog::Create(
		InParent, InID, 
		*FString::Printf(LocalizeSecure(LocalizeUI(TEXT("DlgDiffStates_Title_f")), *Style->StyleName)),
		wxDefaultPosition, wxDefaultSize, 
		wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxCLIP_CHILDREN
		));

	SetExtraStyle(GetExtraStyle()|wxWS_EX_TRANSIENT|wxWS_EX_BLOCK_EVENTS|wxWS_EX_VALIDATE_RECURSIVELY);

	// Create sizer for all children of this window
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(BaseVSizer);

	CreateDiffPanels();
	SetupPanelLayout(CurrentLayout);
	CreateButtonControls();

	// Set up minimal size of the window
	FWindowUtil::LoadPosSize( TEXT("DiffStatesDialog"), this);
	wxRect DefaultPos = GetRect();
	GetSizer()->Fit(this);
	this->SetMinSize(wxSize(DefaultPos.GetWidth(),this->GetRect().GetHeight()));
	Centre();
}

/**
 *	Creates panels for holding the controls for individual properties and adds the state controls to these panels
 */
void WxDlgDiffStateProperties::CreateDiffPanels()
{	
	// Create Panels for holding individual property controls
	FirstStatePanel = new wxPanel();
	SecondStatePanel = new wxPanel();
	FirstStatePanel->Create(this, wxID_ANY);
	SecondStatePanel->Create(this, wxID_ANY);

	CreateStateControls(FirstStatePanel, FirstState);
	CreateStateControls(SecondStatePanel, SecondState);	
}

/**
 * Adds the state panels and a divider between them to the window display
 *
 * @param	LayoutOrientation		specifies the layout of state panels to be either horizontal or vertical
 */
void WxDlgDiffStateProperties::SetupPanelLayout(wxOrientation LayoutOrientation)
{	
	wxBoxSizer* BaseLayoutSizer = new wxBoxSizer(LayoutOrientation);
	GetSizer()->Prepend(BaseLayoutSizer, 0, wxGROW|wxALL, 5);

	BaseLayoutSizer->Add(FirstStatePanel, 1, wxGROW|wxALL, 5);

	if(DividerLine)
	{
		DividerLine->Destroy();
		DividerLine = NULL;
	}
	wxOrientation DividerOrientation = (LayoutOrientation == wxVERTICAL) ? wxLI_HORIZONTAL : wxLI_VERTICAL;
	DividerLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, DividerOrientation );
	BaseLayoutSizer->Add(DividerLine, 0, wxGROW|wxALL, 1);

	BaseLayoutSizer->Add(SecondStatePanel, 1, wxGROW|wxALL, 5);
}

/**
 *	Adds the button controls to the window
 */
void WxDlgDiffStateProperties::CreateButtonControls()
{
	// add the horizontal sizer for the ok button and layout buttons
	wxBoxSizer* ButtonHSizer = new wxBoxSizer(wxHORIZONTAL);
	GetSizer()->Add(ButtonHSizer, 0,wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	// create radio buttons to specify horizontal or vertical layout of two state panels
	wxStaticBox* RadioGroup = new wxStaticBox();
	RadioGroup->Create(this, wxID_ANY, *LocalizeUI(TEXT("DlgDiffStates_LayoutGroupCaption")));

	wxStaticBoxSizer* RadioGroupHSizer = new wxStaticBoxSizer(RadioGroup, wxHORIZONTAL);

	wxRadioButton* ButtonVertical = new wxRadioButton();
	ButtonVertical->Create( this, ID_UI_DIFFSTATES_LAYOUTHORIZONTAL, *LocalizeUI(TEXT("DlgDiffStates_Layout_Horizontal")), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	wxRadioButton* ButtonHorizontal = new wxRadioButton();
	ButtonHorizontal->Create( this, ID_UI_DIFFSTATES_LAYOUTVERTICAL, *LocalizeUI(TEXT("DlgDiffStates_Layout_Vertical")), wxDefaultPosition, wxDefaultSize);

	RadioGroupHSizer->Add(ButtonHorizontal, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	RadioGroupHSizer->Add(ButtonVertical, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	ButtonHSizer->Add(RadioGroupHSizer, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxRIGHT, 10);

	// create the OK button
	wxButton* btn_OK = new wxButton( this, wxID_OK, *LocalizeUnrealEd(TEXT("&OK")) );
	ButtonHSizer->Add(btn_OK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

/**
 * Creates a sizer with the controls specific to this state
 */
void WxDlgDiffStateProperties::CreateStateControls( wxWindow* ParentWindow, UUIState* StateToAdd)
{
	check(StateToAdd);
	check(ParentWindow);

	wxBoxSizer* PanelVSizer = new wxBoxSizer(wxVERTICAL);

	// add the panel with the common properties
	WxStylePropertiesGroup* CommonPropertiesPanel = new WxStyleCommonPropertiesGroup();
	CommonPropertiesPanel->Create(ParentWindow, wxID_ANY, Style, StateToAdd);
	CommonPropertiesPanel->DisableControls();
	CommonPropertiesPanel->RemovePreviewControls();
	PanelVSizer->Add(CommonPropertiesPanel, 0, wxGROW|wxALL, 5);

	// add custom panel displaying properties for this style type
	check(Style->StyleDataClass);
	check(Style->StyleDataClass->GetDefaultObject<UUIStyle_Data>()->UIEditorControlClass.Len() > 0);
	WxStylePropertiesGroup* CustomPropertiesPanel = wxDynamicCast(wxCreateDynamicObject(*Style->StyleDataClass->GetDefaultObject<UUIStyle_Data>()->UIEditorControlClass),WxStylePropertiesGroup);
	CustomPropertiesPanel->Create(ParentWindow, wxID_ANY, Style, StateToAdd);
	CustomPropertiesPanel->DisableControls();
	CustomPropertiesPanel->RemovePreviewControls();
	PanelVSizer->Add(CustomPropertiesPanel, 0, wxGROW|wxALL, 5);

	ParentWindow->SetSizer(PanelVSizer);
}

/** 
 *	Event handler for switching layout modes
 */
void WxDlgDiffStateProperties::OnLayoutChanged( wxCommandEvent& Event)
{
	if( Event.GetId() == ID_UI_DIFFSTATES_LAYOUTVERTICAL )
	{
		CurrentLayout = wxVERTICAL;		
	}
	else if(Event.GetId() == ID_UI_DIFFSTATES_LAYOUTHORIZONTAL )
	{
		CurrentLayout = wxHORIZONTAL;		
	}
	
	// Remove the previous sizer
	GetSizer()->Remove(0);

	SetupPanelLayout(CurrentLayout);
	GetSizer()->Layout();
	GetSizer()->Fit(this);
	
	Event.Skip();
}

