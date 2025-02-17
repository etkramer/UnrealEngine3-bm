/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

/**
 * WHAT:
 *  Portraits: 2 images and 2 text labels at the bottom;
 *             show Epic team's portraits and quotes.
 *
 *  Credits: 2 labels for showing the actual credits
 *
 * TIMING and ANIMATION:
 *
 *	Portraits fade in and out.
 *  Credits consist of 40-some labels. Every 0.65 seconds a new label is read from
 *  the datastore and sent floating up the screen.
 *
 *
 * KNOWN ISSUES and WORKAROUNDS:
 *  - Any lines that begin with formatting (e.g. <Styles:ChromFont>QUALITY ASSURANCE<Styles:/>)
 *    must be preceded by an empty line in the int.
 *
 *  - ChromFont strings tend to wrap for no reason; this will cause text to overrun other text
 *    while the credits are displaying. Find the offending string and hard wrap it (i.e. separate
 *    it into two lines the way the wrapping system did). This will fix the overrun issue.
 */
class GearUISceneFE_Credits extends GearUISceneFrontEnd_Base
	Config(UI);

const PortraitSize = 128;
const LinesPerScreen = 35;
const LineSpawnDelay = 0.65;

struct CreditData
{
	var int	Id;
	var string	PathToImg;
	var int	U;
	var int	V;
};

/** Config variable stores all the portrait images */
var config array<CreditData> PortraitsData;

/**
 * PortaitsInfo describes the pulsing of the portraits and corresponding text.
 */
struct PortraitsInfo
{
	/** When true we need to start a new pulse and have the opportunity to update our ui state (since everything should be faded out)*/
	var transient bool bIsPulseComplete;

	/** The panel containing both portrait images and quote text labels*/
	var transient UIPanel Panel;

	/** The left text label for displaying quotes */
	var transient UILabel TextLeft;

	/** The left image for displaying portraits */
	var transient UIImage ImgLeft;

	/** The right text label for displaying quotes */
	var transient UILabel TextRight;

	/** The right image for displaying portraits */
	var transient UIImage ImgRight;

	/** The quotes associated with the portraits */
	var transient array<string> Quotes;

	/** How many portaits have we loaded so far */
	var transient int LastPortraitLoaded;
};

/** Instance of Portaits Info; see directly above */
var transient PortraitsInfo Portraits;

/** The left side of the credits text */
var transient array<UILabel> CreditLabels;

/** Are we on an even or odd credit screen */
var transient int CurrentCreditPane;

/** The text being set to a credit label; see TickCredits */
var transient string CreditTextCache;

/** The last line which we pulled in from the less-than-perfect .int file */
var transient int CurrentLineNumber;

/** Should we trigger the next label to float up the screen? */
var transient bool bTriggerNextLabel;




// - - Animations - -

/** Animation for credits fading in */
var transient UIAnimationSeq FadeIn;

/** Animation for credits fading out */
var transient UIAnimationSeq FadeOut;

/** Pulsing animation for Portraits and credits(non-bInEpicMode) */
var transient UIAnimationSeq PulseFade;

/** Fade in, but start at opacity 0 */
var transient UIAnimationSeq ZeroOpacity;

/** Scroll the credits onto the screen from the bottom */
var transient UIAnimationSeq ScrollIn;

/** Scroll the credits off the screen towards the top */
var transient UIAnimationSeq ScrollOut;


/** Current line that we are reading from the file and will send up the screen */
var transient int CurCreditLabelIdx;


// - - Timer - -
/** When TimerTimeLeft > 0.0 we decrement it by DeltaTime. Once the timer hits 0 we call OnTimerTriggered*/
var transient float StartLineTimer;


// - - Termination - -

/** Defaults to -1. A value >= 0 means: when this label is done animating stop the credits. */
var transient int bDoneLabelIdx;

/** We should exit the scene; ensures that the last lines scroll into view */
var transient bool bEndCredits;


/**
 * Called after this screen object's children have been initialized
 * Overloaded to set the deactivated callback
 */
event PostInitialize()
{
	local float TotalCreditsTime;
	local int NumQuotePairs;

	// Initialize references to widgets
	InitializeWidgetReferences();

	// Invalid index because it will get incremented right away.
	Portraits.bIsPulseComplete = false;
	//bStartNewCreditScreen = true;
	Portraits.LastPortraitLoaded = 0;

	BuildQuoteList( Portraits.Quotes );

	// Time the portraits to fit the credits length
	TotalCreditsTime = CountNumCreditLines() * LineSpawnDelay + 20; // 20 is how long it takes for the last line to go from start to finish.
	NumQuotePairs = (Portraits.Quotes.Length + 1) / 2; // Round the number of pairs UP
	PulseFade.Tracks[0].KeyFrames[2].RemainingTime = TotalCreditsTime / NumQuotePairs - 1; // -1 because 0.5 to fade in and 0.5 to fade out

	Super.PostInitialize();
}

/** Initialize references to widgets */
function InitializeWidgetReferences()
{
	local int LabelCount;
	local UILabel TmpLabel;

	//lblCredits_0 = UILabel(FindChild('lblTextLeft', true));
	//lblCredits_1 = UILabel(FindChild('lblTextRight', true));

	Portraits.Panel = UIPanel(FindChild('PortraitPanel', true));
	Portraits.Panel.SetVisibility(false);

	Portraits.TextLeft = UILabel(FindChild('lblPortrait_1', true));
	Portraits.ImgLeft = UIImage(FindChild('imgPortrait_1', true));

	Portraits.TextRight = UILabel(FindChild('lblPortrait_2', true));
	Portraits.ImgRight = UIImage(FindChild('imgPortrait_2', true));


	// Get references to our pool of labels that we can use to show the credits scroll
	TmpLabel = UILabel(FindChild('lblCredit_00', true));
	LabelCount = 1;
	while (TmpLabel != None)
	{
		TmpLabel.SetVisibility(true);
		TmpLabel.Opacity = 0.0;
		TmpLabel.SetValue("");
		CreditLabels.AddItem(TmpLabel);
		//`Log("Added label " $ TmpLabel $ " count is " $ CreditLabels.Length);

		if (LabelCount < 10)
		{
			TmpLabel = UILabel(FindChild( Name("lblCredit_0" $ LabelCount), true));
		}
		else
		{
			TmpLabel = UILabel(FindChild( Name("lblCredit_" $ LabelCount) , true));
		}
	
		LabelCount++;
	}

}

/**
 * Count the total number of lines in the credits
 *
 * @return The number of lines in the credits.
 */
function int CountNumCreditLines()
{
	local int CreditLineCount;
	local string Dumpster; // Used to throw out the return value

	while( NextLine(Dumpster) )
	{
		CreditLineCount++;
	}

	CurrentLineNumber = 0;

	return CreditLineCount;
}

/**
 * Tests a string to see if looks like something that the Data Store returns when it can't find an item.
 *
 * @param	TestMe	The string's first 5 character against ?int?
 *
 * @return	true if the binding is OK
 */
function bool IsDataFound(string TestMe)
{
	return Left(TestMe, 1) != "?";
}

/**
 * Get the next line from the Credits.INT datastore.
 *
 * @param OutCurrentLine 
 * @returns true if we successfully read a line
 */
function bool NextLine( out string OutCurrentLine )
{
	local string LineName;
	
	if (CurrentLineNumber < 10)
	{
		LineName = "line_0000" $ CurrentLineNumber;
	}
	else if (CurrentLineNumber < 100)
	{
		LineName = "line_000" $ CurrentLineNumber;
	}
	else if (CurrentLineNumber < 1000)
	{
		LineName = "line_00" $ CurrentLineNumber;
	}
	else if (CurrentLineNumber < 10000)
	{
		LineName = "line_0" $ CurrentLineNumber;
	}
	else
	{
		`Warn("Warning, you've exceeded the maximum number of lines supported by credits.");
	}

	CurrentLineNumber++;

	OutCurrentLine = Localize( "credits", LineName, "Credits" );

	//`Log( "Credits: Looking for " $ LineName $ ". Found " $ OutCurrentLine $ "." );
	return IsDataFound(OutCurrentLine);
}


/**
 * Queries the datastore and returns a string representing credits for the specified screen and side of the screen.
 *
 * @param	ScreenId	Which screen, e.g. 0, 1, 2, 3, etc...
 * @param	Quotes		The quotes
 */
function BuildQuoteList( out array<string> Quotes )
{
	local int Idx;
	local string IndexStr;
	local string Quote;

	Idx=0;
	Quotes.Length=0;

	quote = Localize("Quotes", "Q000", "Credits");

	while( IsDataFound(quote) )
	{
		Quotes.AddItem( quote );
		Idx++;
		if (Idx < 10)
		{
			IndexStr = "Q00" $ string(Idx);
		}
		else if (Idx < 100)
		{
			IndexStr = "Q0" $ string(Idx);
		}
		else
		{
			IndexStr = "Q" $ string(Idx);
		}

		quote = Localize("Quotes", IndexStr, "Credits");
		//`Log("Read Quote: " $ quote);
	}

	//`Log("\n- - - - - - - Credits: Found " $ Quotes.Length $ " quotes");
}

/**
 * Handler for the completion of this scene's opening animation...
 *
 * @warning - if you override this in a child class, keep in mind that this function will not be called if the scene has no opening animation.
 */
function OnOpenAnimationComplete( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	Super.OnOpenAnimationComplete(Sender, AnimName, TrackTypeMask);
	if ( Sender == Self && TrackTypeMask == 0 && AnimName == SceneAnimation_Open )
	{
		// just start receiving calls to tick.
		OnGearUISceneTick = CreditsSceneTick;

		// Make sure Credits and Portraits are invisible
		AnimatePortraits(ZeroOpacity);
	}
}

/**
 * Notification that one or more tracks in an animation sequence have completed.
 *
 * @param	Sender				the widget that completed animating.
 * @param	AnimName			the name of the animation sequence that completed.
 * @param	TypeMask			a bitmask indicating which animation tracks completed.  It is generated by left shifting 1 by the
 *								values of the EUIAnimType enum.
 *								A value of 0 indicates that all tracks in the animation sequence have finished.
 */
event UIAnimationEnded( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	Super.UIAnimationEnded( Sender, AnimName, TrackTypeMask );

	if ( TrackTypeMask == 0)
	{
		if (Sender == Portraits.Panel && (AnimName == 'FadeOut' || AnimName == 'PulseFade' || AnimName == 'ZeroOpacity' ))
		{
			Portraits.bIsPulseComplete = true;
		}

		if (bDoneLabelIdx >= 0)
		{
			bEndCredits = ( Sender == CreditLabels[bDoneLabelIdx] );
		}
	}
	else if ( ( TrackTypeMask & (0x1 << EAT_Opacity) ) != 0 )
	{
		if ( INDEX_NONE != CreditLabels.Find(Sender) )
		{
			bTriggerNextLabel = TRUE;
		}
	}

}


/**
 * Update the data binding for the portraits/quotes based on the CurrentSet and the portrait CurrentInSet.
 */
function UpdatePortraitDataBindings()
{

	local int FoundIndex;
	local CreditData PortraitData;
	local UILabel QuoteText;
	local UIImage QuotePortrait;
	local TextureCoordinates TexCoord;
	local bool bIsLeftPortrait;

	bIsLeftPortrait = (Portraits.LastPortraitLoaded%2==0) ? TRUE : FALSE;

	QuoteText = (bIsLeftPortrait) ? (Portraits.TextLeft) : (Portraits.TextRight);
	QuotePortrait = (bIsLeftPortrait) ? (Portraits.ImgLeft) : (Portraits.ImgRight);

	FoundIndex = PortraitsData.Find('Id', Portraits.LastPortraitLoaded);
	if (FoundIndex < 0)
	{
		`warn("Could not find portrait " $ Portraits.LastPortraitLoaded $ ".");
		FoundIndex = 0;
		// We don't have a right portrait
		QuoteText.SetVisibility(false);
		QuotePortrait.SetVisibility(false);
	}
	else
	{
		QuoteText.SetVisibility(true);
		QuotePortrait.SetVisibility(true);
	}
	PortraitData = PortraitsData[FoundIndex];

	TexCoord.U = PortraitData.U;
	TexCoord.V = PortraitData.V;
	TexCoord.UL = PortraitSize;
	TexCoord.VL = PortraitSize;

	//`log("PortraitData: " $ PortraitData.Id $ " " $ PortraitData.PathToImg $ " (" $ string(PortraitData.U) $ ", " $ string(PortraitData.V) $ ")\n");

	QuoteText.SetValue(Portraits.Quotes[Portraits.LastPortraitLoaded]);
	QuotePortrait.SetDataStoreBinding(PortraitData.PathToImg);
	QuotePortrait.ImageComponent.SetCoordinates(TexCoord);

	Portraits.LastPortraitLoaded++;
}

/**
 * Play the animation for both portraits/quotes
 * @param	Animation	The UIAnimationSeq to play.
 */
function AnimatePortraits(UIAnimationSeq Animation)
{
	Portraits.Panel.PlayUIAnimation( '', Animation, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
}

/** The Tick method for portraits */
function TickPortraits()
{
	if (Portraits.bIsPulseComplete)
	{
		Portraits.bIsPulseComplete = false;
		
		UpdatePortraitDataBindings();
		UpdatePortraitDataBindings();

		
		Portraits.Panel.SetVisibility(true);
		AnimatePortraits(PulseFade);
	}
}

/** The Tick method for credits scrolling */
function TickCredits(float DeltaTime)
{
	local string TempString;

	// Kick off a new line of credits periodically
	//StartLineTimer -= DeltaTime * AnimationDebugMultiplier;
	if ( bTriggerNextLabel )
	{
		bTriggerNextLabel = FALSE;
		StartLineTimer = LineSpawnDelay;

		if ( !NextLine(TempString) )
		{
			// The last lable that we sent off to float up the screen is what we wait for to end the scene
			bDoneLabelIdx = (CurCreditLabelIdx==0) ? CreditLabels.Length-1 : CurCreditLabelIdx-1;
		}

		if ( bDoneLabelIdx < 0 )
		{
			CreditLabels[CurCreditLabelIdx].SetDataStoreBinding( TempString );
			//`Log( "Cur line: " @ CurCreditLabelIdx @  TempString $ "|||||||  " $ CreditLabels[CurCreditLabelIdx].GetDataStoreBinding() );
			CreditLabels[CurCreditLabelIdx].Opacity = 1.0;
			CreditLabels[CurCreditLabelIdx].PlayUIAnimation( '', ScrollIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
			CurCreditLabelIdx = (CurCreditLabelIdx + 1) % CreditLabels.Length;
		}
	}
}


/**
 * Handler for the OnGearUISceneTick delegate.
 *
 * @param DeltaTime	time since last tick.
 */
function CreditsSceneTick(float DeltaTime)
{
	local int Idx;
	if ( GetGearPlayerOwner().bIsExternalUIOpen )
	{
		// Pause all animations
		Portraits.Panel.PauseAnimations( true );
		for (Idx=0; Idx<CreditLabels.Length; Idx++)
		{
			CreditLabels[Idx].PauseAnimations( true );
		}
	}
	else
	{
		// Unpause all animtaions
		Portraits.Panel.PauseAnimations( false );
		for (Idx=0; Idx<CreditLabels.Length; Idx++)
		{
			CreditLabels[Idx].PauseAnimations( false );
		}

		// - - Cycle portraits
		TickPortraits();
		TickCredits(DeltaTime);

		if (bEndCredits)
		{
			End();
		}
	}

}


/**
 * The credits have finished playing.
 * Quit the scene.
 */
function End()
{
	CloseScene();
}

DefaultProperties
{
	bDoneLabelIdx=-1
	bTriggerNextLabel=TRUE

	/** Animation tracks */
	Begin Object Class=UIAnimationSeq Name=FadeIn_Template
		SeqName=FadeIn
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.5,Data=(DestAsFloat=1.0))))
	End Object
	FadeIn = FadeIn_Template

	Begin Object Class=UIAnimationSeq Name=FadeOut_Template
		SeqName=FadeOut
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=2.5,Data=(DestAsFloat=1.0)),(RemainingTime=0.5,Data=(DestAsFloat=0.0))))
	End Object
	FadeOut = FadeOut_Template

	Begin Object Class=UIAnimationSeq Name=PulseFade_Template
		SeqName=PulseFade
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.5,Data=(DestAsFloat=1.0)),(RemainingTime=10.0,Data=(DestAsFloat=1.0)),(RemainingTime=0.5,Data=(DestAsFloat=0.0))))
	End Object
	PulseFade = PulseFade_Template

	Begin Object Class=UIAnimationSeq Name=ZeroOpacity_Template
		SeqName=ZeroOpacity
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0))))
	End Object
	ZeroOpacity = ZeroOpacity_Template

	Begin Object Class=UIAnimationSeq Name=ScrollIn_Template
		SeqName=ScrollIn
		Tracks(0)=(TrackType=EAT_Top,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=580.0)),(RemainingTime=20.0,Data=(DestAsFloat=0.0))))
		Tracks(1)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.65,Data=(DestAsFloat=1.0))))
	End Object
	ScrollIn = ScrollIn_Template
}
