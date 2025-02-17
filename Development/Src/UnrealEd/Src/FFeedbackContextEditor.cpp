/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UnrealEd.h"
#include "EngineSequenceClasses.h"
#include "FFeedbackContextEditor.h"
#include "DlgMapCheck.h"

FFeedbackContextEditor::FFeedbackContextEditor()
	: bIsPerformingMapCheck(FALSE),
	  SlowTaskCount(0),
	  RealTimeSlowTaskStarted( 0.0 )
{
	DlgProgress = NULL;

	StatusMessage.StatusText = TEXT( "" );
	StatusMessage.ProgressNumerator = 0;
	StatusMessage.ProgressDenominator = 0;
}

void FFeedbackContextEditor::Serialize( const TCHAR* V, EName Event )
{
	if( !GLog->IsRedirectingTo( this ) )
	{
		GLog->Serialize( V, Event );
	}

	if ( Event == NAME_Error )
	{
		appMsgf( AMT_OK, TEXT("%s"), V );
	}
}

/**
 * Tells the editor that a slow task is beginning
 * 
 * @param Task					The description of the task beginning.
 * @param ShowProgressDialog	Whether to show the progress dialog.
 */
void FFeedbackContextEditor::BeginSlowTask( const TCHAR* Task, UBOOL ShowProgressDialog )
{
	if( GApp && GApp->EditorFrame )
	{
		GIsSlowTask = ++SlowTaskCount>0;

		// Show a wait cursor for long running tasks
		if (SlowTaskCount == 1)
		{
			// NOTE: Any slow tasks that start after the first slow task is running won't display
			//   status text unless 'StatusUpdatef' is called
			StatusMessage.StatusText = Task;

			::wxBeginBusyCursor();

			// Keep track of when we started performing slow tasks
			RealTimeSlowTaskStarted = appSeconds();
		}

		if( ShowProgressDialog && !bShowProgressDlg )
		{
			bShowProgressDlg = TRUE;

			// Create the dialog 
			if( DlgProgress == NULL )
			{
				DlgProgress = new WxDlgProgress( GApp->EditorFrame );
			}

			DlgProgress->ShowDialog();
			DlgProgress->SetStatusText( *StatusMessage.StatusText );
		}
	}
}

/**
 * Tells the editor that the slow task is done.
 */
void FFeedbackContextEditor::EndSlowTask()
{
	if( GApp && GApp->EditorFrame )
	{
		check(SlowTaskCount>0);
		GIsSlowTask = --SlowTaskCount>0;
		// Restore the cursor now that the long running task is done
		if (SlowTaskCount == 0)
		{
			::wxEndBusyCursor();

			if( bShowProgressDlg )
			{
				// We're finished, so make sure we show the progress bar filled 100%, right before we close
				DlgProgress->SetProgressPercent( 1, 1 );
				appSleep( 0.15f );

				// Hide the window
				DlgProgress->Show( FALSE );
			}
			bShowProgressDlg = FALSE;


			// Reset cached message
			StatusMessage.StatusText = TEXT( "" );
			StatusMessage.ProgressNumerator = 0;
			StatusMessage.ProgressDenominator = 0;

			
			// OK, we'll also bump the auto save timer by the 'real time' seconds that we were performing
			// busy work.  This is so that we don't end up always doing an auto save after a long process
			// such as a level rebuild.
			const DOUBLE CurRealTime = appSeconds();
			const FLOAT TimeSpentInSlowTasks = Max( 0.0f, ( FLOAT )( CurRealTime - RealTimeSlowTaskStarted ) );
			GEditor->BumpAutosaveTimer( TimeSpentInSlowTasks );
		}
	}
}

void FFeedbackContextEditor::SetContext( FContextSupplier* InSupplier )
{
}

VARARG_BODY( UBOOL VARARGS, FFeedbackContextEditor::StatusUpdatef, const TCHAR*, VARARG_EXTRA(INT Numerator) VARARG_EXTRA(INT Denominator) )
{
	TCHAR TempStr[4096];
	GET_VARARGS( TempStr, ARRAY_COUNT(TempStr), ARRAY_COUNT(TempStr)-1, Fmt, Fmt );

	// Cache the new status message and progress
	StatusMessage.StatusText = TempStr;
	if( Numerator >= 0 ) // Ignore if set to -1
	{
		StatusMessage.ProgressNumerator = Numerator;
	}
	if( Denominator >= 0 ) // Ignore if set to -1
	{
		StatusMessage.ProgressDenominator = Denominator;
	}

	if( GIsSlowTask )
	{
		GApp->StatusUpdateProgress(
			*StatusMessage.StatusText,
			StatusMessage.ProgressNumerator,
			StatusMessage.ProgressDenominator );

		if( bShowProgressDlg )
		{
			DlgProgress->SetStatusText( *StatusMessage.StatusText );
			DlgProgress->SetProgressPercent( StatusMessage.ProgressNumerator, StatusMessage.ProgressDenominator );
		}
	}


	// Also update the splash screen text (in case we're in the middle of starting up)
	appSetSplashText( SplashTextType::StartupProgress, *StatusMessage.StatusText );


	return 1;
}


/**
 * Updates the progress amount without changing the status message text
 *
 * @param Numerator		New progress numerator
 * @param Denominator	New progress denominator
 */
void FFeedbackContextEditor::UpdateProgress( INT Numerator, INT Denominator )
{
	// Cache the new progress
	if( Numerator >= 0 ) // Ignore if set to -1
	{
		StatusMessage.ProgressNumerator = Numerator;
	}
	if( Denominator >= 0 ) // Ignore if set to -1
	{
		StatusMessage.ProgressDenominator = Denominator;
	}

	if( GIsSlowTask )
	{
		GApp->StatusUpdateProgress(
			NULL,
			StatusMessage.ProgressNumerator,
			StatusMessage.ProgressDenominator );

		if( bShowProgressDlg )
		{
			DlgProgress->SetProgressPercent( StatusMessage.ProgressNumerator, StatusMessage.ProgressDenominator );
		}
	}
}



/** Pushes the current status message/progress onto the stack so it can be restored later */
void FFeedbackContextEditor::PushStatus()
{
	// Push the current message onto the stack.  This doesn't change the current message though.  You should
	// call StatusUpdatef after calling PushStatus to update the message.
	StatusMessageStack.AddItem( StatusMessage );
}



/** Restores the previously pushed status message/progress */
void FFeedbackContextEditor::PopStatus()
{
	if( StatusMessageStack.Num() > 0 )
	{
		// Pop from stack
		StatusMessageStackItem PoppedStatusMessage = StatusMessageStack( StatusMessageStack.Num() - 1 );
		StatusMessageStack.Remove( StatusMessageStack.Num() - 1 );

		// Update the message text
		StatusMessage.StatusText = PoppedStatusMessage.StatusText;

		// Only overwrite progress if the item on the stack actually had some progress set
		if( PoppedStatusMessage.ProgressDenominator > 0 )
		{
			StatusMessage.ProgressNumerator = PoppedStatusMessage.ProgressNumerator;
			StatusMessage.ProgressDenominator = PoppedStatusMessage.ProgressDenominator;
		}

		// Update the GUI!
		if( GIsSlowTask )
		{
			GApp->StatusUpdateProgress(
				*StatusMessage.StatusText,
				StatusMessage.ProgressNumerator,
				StatusMessage.ProgressDenominator );

			if( bShowProgressDlg )
			{
				DlgProgress->SetStatusText( *StatusMessage.StatusText );
				DlgProgress->SetProgressPercent( StatusMessage.ProgressNumerator, StatusMessage.ProgressDenominator );
				DlgProgress->Update();
			}
		}
	}
}



/**
 * @return	TRUE if a map check is currently active.
 */
UBOOL FFeedbackContextEditor::MapCheck_IsActive() const
{
	return bIsPerformingMapCheck;
}

void FFeedbackContextEditor::MapCheck_Show()
{
	GApp->DlgMapCheck->Show( true );
}

/**
 * Same as MapCheck_Show, except it won't display the map check dialog if there are no errors in it.
 */
void FFeedbackContextEditor::MapCheck_ShowConditionally()
{
	GApp->DlgMapCheck->ShowConditionally();
}

/**
 * Hides the map check dialog.
 */
void FFeedbackContextEditor::MapCheck_Hide()
{
	GApp->DlgMapCheck->Show( false );
}

/**
 * Clears out all errors/warnings.
 */
void FFeedbackContextEditor::MapCheck_Clear()
{
	GApp->DlgMapCheck->ClearMessageList();
}

/**
 * Called around bulk MapCheck_Add calls.
 */
void FFeedbackContextEditor::MapCheck_BeginBulkAdd()
{
	GApp->DlgMapCheck->FreezeMessageList();
	bIsPerformingMapCheck = TRUE;
}

/**
 * Called around bulk MapCheck_Add calls.
 */
void FFeedbackContextEditor::MapCheck_EndBulkAdd()
{
	bIsPerformingMapCheck = FALSE;
	GApp->DlgMapCheck->ThawMessageList();
}

/**
 * Adds a message to the map check dialog, to be displayed when the dialog is shown.
 *
 * @param	InType					The	type of message.
 * @param	InActor					Actor associated with the message; can be NULL.
 * @param	InMessage				The message to display.
 * @param	InRecommendedAction		[opt] The recommended course of action to take; default is MCACTION_NONE.
 * @param	InUDNPage				[opt] UDN Page to visit if the user needs more info on the warning.  This will send the user to https://udn.epicgames.com/Three/MapErrors#InUDNPage. 
 */
void FFeedbackContextEditor::MapCheck_Add(MapCheckType InType, UObject* InActor, const TCHAR* InMessage, MapCheckAction InRecommendedAction, const TCHAR* InUDNPage)
{
	AActor* Actor	= Cast<AActor>( InActor );
	WxDlgMapCheck::AddItem( InType, Actor, InMessage, InRecommendedAction, InUDNPage );
}
