/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Credits scene for UT3.
 */
class UTUIFrontEnd_Credits extends UTUIFrontEnd
	native(UIFrontEnd);

struct native CreditsImageSetData
{
	var transient Surface  TexImage;
	var string			   TexImageName;
	var TextureCoordinates TexCoords;
	var string			   LabelMarkup;
};

struct native CreditsImageSet
{
	var array<CreditsImageSetData> ImageData;
};

/** How long to show the scene for. */
var() float	SceneTimeInSec;

/** How much of a delay to have before starting to show gears photos. */
var() float DelayBeforePictures;

/** How much of a delay to have after pictures end. */
var() float DelayAfterPictures;

/** Whether or not the credits have finished player. */
var bool bFinishedPlaying;

/** Record when we started displaying. */
var transient float StartTime;

/** A set of images and labels to cycle between. */
var	transient array<CreditsImageSet>	ImageSets;

/** Which object set to use for fading. */
var transient int CurrentObjectOffset;

/** Which image set we are currently on. */
var transient int CurrentImageSet;

/** Labels that hold quotes for each person. */
var transient UILabel QuoteLabels[6];

/** Photos of each person. */
var transient UIImage PhotoImage[6];

/** Labels to hold the scrolling credits text. */
var transient UILabel TextLabels[3];

/** Which text set we are currently on. */
var transient int CurrentTextSet;

/** Scrolling offset to start from. */
var transient float StartOffset[3];

/** Sets of text used for the scrolling credits. */
var transient array<string>	 TextSets;

cpptext
{
	virtual void Tick( FLOAT DeltaTime );

	/**
	 * Changes the datastore bindings for widgets to the next image set.
	 *
	 * @param bFront Whether we are updating front or back widgets.
	 */
	void UpdateWidgets(UBOOL bFront);

	/**
	 * Updates the position and text for credits widgets.
	 */
	void UpdateCreditsText();

	/**
	 * Callback that happens the first time the scene is rendered, any widget positioning initialization should be done here.
	 *
	 * By default this function recursively calls itself on all of its children.
	 */
	virtual void PreInitialSceneUpdate();
}

/** Sets up the credits scene and retrieves references to widgets. */
native function SetupScene();

/** Make sure we use the frontend skin for credits. */
event Initialized()
{
	local UISkin Skin;

	if ( IsGame() )
	{
		// make sure we're using the right skin
		Skin = UISkin(DynamicLoadObject("UI_Skin_Derived.UTDerivedSkin",class'UISkin'));
		if ( Skin != none )
		{
			SceneClient.ChangeActiveSkin(Skin);
		}
	}

	Super.Initialized();
}


/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize( )
{
	local int ImageIdx, DataIdx;

	Super.PostInitialize();

	// Retrieve references to all of the portrait images.
	for(ImageIdx=0; ImageIdx<ImageSets.length; ImageIdx++)
	{
		for (DataIdx=0; DataIdx<ImageSets[ImageIdx].ImageData.length; DataIdx++)
		{
			ImageSets[ImageIdx].ImageData[DataIdx].TexImage = Surface(DynamicLoadObject(ImageSets[ImageIdx].ImageData[DataIdx].TexImageName, class'Surface'));

			if(ImageSets[ImageIdx].ImageData[DataIdx].TexImage == None)
			{
				`Log("UTUIFrontEnd_Credits - Couldn't find image "$ImageSets[ImageIdx].ImageData[DataIdx].TexImageName);
			}
			else
			{
				`Log("UTUIFrontEnd_Credits - DLO Image: "$ImageSets[ImageIdx].ImageData[DataIdx].TexImageName);
			}
		}
	}

	SetupScene();
}

/** Sets up the scene's button bar. */
function SetupButtonBar()
{
	ButtonBar.Clear();
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
}


/** Finishes the credits, either by closing the scene or going back to the main menu. */
function OnFinishCredits()
{
	local UTPlayerController PC;

	PC = GetUTPlayerOwner();

	if(PC.GetURLMap()!="UTFrontEnd")
	{
		PC.QuitToMainMenu();
	}
	else
	{
		CloseScene(self);
	}
}

/** Callback for when the credits have finished displaying. */
event OnCreditsFinished()
{
	if(bFinishedPlaying==false)
	{
		OnFinishCredits();
		bFinishedPlaying=true;
	}
}

/** Callback for when the user wants to back out of this screen. */
function OnBack()
{
	OnFinishCredits();
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_Back(UIScreenObject InButton, int InPlayerIndex)
{
	OnBack();

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
			OnBack();
			bResult=true;
		}
	}

	return bResult;
}



defaultproperties
{
	SceneTimeInSec = 240;
	DelayBeforePictures = 10;
	DelayAfterPictures = 20;

	// Polge, Morris, Golding
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_02", TexCoords=(U=128,V=137,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG01_01>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_06", TexCoords=(U=256,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG01_02>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_06", TexCoords=(U=0,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG01_03>"))))
	// Oelfke, Mahajan, Markiewicz
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_02", TexCoords=(U=128,V=0,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG01_13>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_01", TexCoords=(U=384,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG01_14>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_09", TexCoords=(U=0,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG01_04>"))))
	// Sweitzer, Wilcox, Davis
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_02", TexCoords=(U=0,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG01_05>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_02", TexCoords=(U=256,V=0,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG01_06>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_01", TexCoords=(U=256,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG04_09>"))))
	// Farris, McLaughlin, James
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_01", TexCoords=(U=256,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG01_07>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_01", TexCoords=(U=384,V=137,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG01_08>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_02", TexCoords=(U=0,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG01_09>"))))
	// O'Flaherty, Jones, Bartlett
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_01", TexCoords=(U=128,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG01_10>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_04", TexCoords=(U=256,V=0,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG01_11>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_02", TexCoords=(U=256,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG01_12>"))))
	// Buck, Caudle, Dossett
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_02", TexCoords=(U=384,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG02_01>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_02", TexCoords=(U=384,V=137,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG02_02>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_03", TexCoords=(U=384,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG02_03>"))))
	// Ellis, Green, Hancy
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_04", TexCoords=(U=0,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG02_04>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_02", TexCoords=(U=384,V=275,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG02_05>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_03", TexCoords=(U=0,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG02_06>"))))
	// Hawkins, Hayes, Herzog
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_02", TexCoords=(U=256,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG02_07>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_03", TexCoords=(U=0,V=137,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG02_08>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_03", TexCoords=(U=384,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG02_09>"))))
	// Hosfelt, Jay, Johnstone
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_03", TexCoords=(U=384,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG02_10>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_03", TexCoords=(U=0,V=275,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG02_11>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_04", TexCoords=(U=0,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG02_12>"))))
	// Johnson, Lanning, Mitchell
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_03", TexCoords=(U=128,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG03_01>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_03", TexCoords=(U=128,V=137,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG03_02>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_04", TexCoords=(U=0,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG03_03>"))))
	// Mountain, Pierce, Smith
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_03", TexCoords=(U=128,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG03_04>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_09", TexCoords=(U=128,V=275,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG03_05>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_04", TexCoords=(U=128,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG03_06>"))))
	// Tucker, Wells, Perna
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_04", TexCoords=(U=128,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG03_07>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_04", TexCoords=(U=128,V=137,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG03_08>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_01", TexCoords=(U=128,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG03_09>"))))
	// Brown, Weing, Brucks
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_04", TexCoords=(U=256,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG03_10>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_05", TexCoords=(U=0,V=0,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG03_11>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_04", TexCoords=(U=384,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG03_12>"))))
	// Cole, Fitzsimmons, Frank
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_04", TexCoords=(U=384,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG04_01>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_05", TexCoords=(U=0,V=137,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG04_02>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_09", TexCoords=(U=128,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG08_03>"))))
	// Mader, Marshall, Rauchberger
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_05", TexCoords=(U=0,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG04_03>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_05", TexCoords=(U=128,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG04_04>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_05", TexCoords=(U=128,V=275,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG04_05>"))))
	// Rogers, Spalinski, Spano
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_05", TexCoords=(U=256,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG04_06>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_05", TexCoords=(U=256,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG04_07>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_03", TexCoords=(U=256,V=137,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG04_08>"))))
	// Spencer, Willard, Larson
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_05", TexCoords=(U=256,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG04_10>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_03", TexCoords=(U=256,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG04_11>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_05", TexCoords=(U=384,V=0,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG04_12>"))))
	// Graf, Vogel, Adams
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_01", TexCoords=(U=384,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG05_01>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_06", TexCoords=(U=128,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG05_02>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_05", TexCoords=(U=384,V=137,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG05_03>"))))
	// Burke, Fricker, Hunt
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_05", TexCoords=(U=384,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG05_04>"),(TexImageName="UI_FrontEnd_Art.Credits_Port_10", TexCoords=(U=0,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG05_05>"),(TexImageName="UI_FrontEnd_Art.Credits_Port_10", TexCoords=(U=0,V=0,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG05_06>"))))
	// Newton, Prestenback, Scheidecker
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits_Port_10", TexCoords=(U=128,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG05_07>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_06", TexCoords=(U=0,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG05_08>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_06", TexCoords=(U=0,V=275,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG05_09>"))))
	// Scott, Sherman, Smedberg
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_02", TexCoords=(U=128,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG05_10>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_06", TexCoords=(U=128,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG05_11>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_06", TexCoords=(U=128,V=137,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG05_12>"))))
	// Sweeney, Wright, Zamani
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_07", TexCoords=(U=0,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG06_01>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_06", TexCoords=(U=256,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG06_13>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_06", TexCoords=(U=256,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG06_02>"))))
	// Capps, Fergusson, Jessen
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_01", TexCoords=(U=0,V=137,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG06_03>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_01", TexCoords=(U=0,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG06_04>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_09", TexCoords=(U=0,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG06_05>"))))
	// Rein, Wilbur, Babcock
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_07", TexCoords=(U=0,V=275,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG06_06>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_07", TexCoords=(U=128,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG06_07>"),(TexImageName="UI_FrontEnd_Art.Credits_Port_10", TexCoords=(U=128,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG06_08>"))))
	// Bigwood, Holcomb, Jones
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_06", TexCoords=(U=384,V=0,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG06_09>"),(TexImageName="UI_FrontEnd_Art.Credits_Port_10", TexCoords=(U=0,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG06_10>"),(TexImageName="UI_FrontEnd_Art.Credits_Port_10", TexCoords=(U=128,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG06_11>"))))
	// Thorne, Schultz, Smith
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_07", TexCoords=(U=0,V=0,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG06_12>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_06", TexCoords=(U=384,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG07_01>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_06", TexCoords=(U=384,V=275,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG07_02>"))))
	// Ent, Dube, Bleszinski
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_09", TexCoords=(U=0,V=0,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG07_03>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_07", TexCoords=(U=128,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG07_04>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_01", TexCoords=(U=0,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG07_05>"))))
	// Thompson, Andrews, Farrow
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_07", TexCoords=(U=128,V=275,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG07_06>"),(TexImageName="UI_FrontEnd_Art.Credits_Port_10", TexCoords=(U=256,V=0,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG07_07>"),(TexImageName="UI_FrontEnd_Art.Credits_Port_10", TexCoords=(U=256,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG07_08>"))))
	// Amidon, Mielke
	ImageSets.Add((ImageData=((TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_08", TexCoords=(U=0,V=0,UL=128,VL=128),LabelMarkup="<Strings:UTGameCredits.Quotes.IMG08_01>"),(TexImageName="UI_FrontEnd_Art.Credits.Credits_Port_09", TexCoords=(U=128,V=137,UL=128,VL=128),LabelMarkup="<strings:UTGameCredits.Quotes.IMG08_02>"))))

	TextSets.Add("<Strings:UTGameCredits.Credits.01>");
}

