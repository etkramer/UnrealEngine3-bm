//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================

#include "UnrealEd.h"
#include "RemoteControlGame.h"
#include "BusyCursor.h"

#include "RemoteControlGear.h"
#include "RemoteControlGearFlagsPage.h"

#include "Engine.h"
#include "EngineAnimClasses.h"
#include "EngineAIClasses.h"
#include "EngineDecalClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineSequenceClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineUIPrivateClasses.h"
#include "EngineUISequenceClasses.h"
#include "GameFrameworkClasses.h"
#include "GameFrameworkAnimClasses.h"
#include "GearGameClasses.h"
#include "GearGamePCClasses.h"
#include "GearGamePawnClasses.h"


#pragma warning(disable : 4800) // C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)


WxRemoteControlGearFlagsPage::WxRemoteControlGearFlagsPage( FRemoteControlGame* InGame, wxNotebook* InNotebook )
: WxRemoteControlPage( InGame )
{
	LoadGearRemoteControlXRCs();

	verify( wxXmlResource::Get()->LoadPanel( this, InNotebook, TEXT("ID_WARFARE_FLAGS_PAGE")) );


	BindControl( ShowAccuracyCheckBox, XRCID("ID_SHOW_ACCURACY") );
	ADDEVENTHANDLER( XRCID("ID_SHOW_ACCURACY"), wxEVT_COMMAND_CHECKBOX_CLICKED, &WxRemoteControlGearFlagsPage::ShowAccuracy_Activated );

	BindControl( ShowCoverCheckBox, XRCID("ID_SHOW_COVER") );
	ADDEVENTHANDLER( XRCID("ID_SHOW_COVER"), wxEVT_COMMAND_CHECKBOX_CLICKED, &WxRemoteControlGearFlagsPage::ShowCover_Activated );

	BindControl( ShowDebugAICheckBox, XRCID("ID_SHOW_DEBUGAI") );
	ADDEVENTHANDLER( XRCID("ID_SHOW_DEBUGAI"), wxEVT_COMMAND_CHECKBOX_CLICKED, &WxRemoteControlGearFlagsPage::DebugAI_Activated );


	// @TODO
	// ai logging



	// Size the window.
	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
}


const TCHAR* WxRemoteControlGearFlagsPage::GetPageTitle() const
{
	return TEXT( "GearFlags" );
}


void WxRemoteControlGearFlagsPage::RefreshPage(UBOOL bForce)
{
	DebugAI_UpdateWidget();
	ShowAccuracy_UpdateWidget();
	ShowCover_UpdateWidget();
}



void WxRemoteControlGearFlagsPage::DebugAI_Activated( wxCommandEvent& In )
{
	UBOOL bCurrValue = In.IsChecked();

	DebugAI_Worker( TRUE, bCurrValue );

	DebugAI_UpdateWidget();
}


void WxRemoteControlGearFlagsPage::DebugAI_UpdateWidget()
{
	UBOOL bCurrValue = DebugAI_Worker( FALSE, FALSE );
	ShowDebugAICheckBox->SetValue( bCurrValue );
}


UBOOL WxRemoteControlGearFlagsPage::DebugAI_Worker( UBOOL bShouldSet, UBOOL bValueToSet )
{
	UBOOL Retval = FALSE;

	ULocalPlayer* LocalPlayer = GetGame()->GetLocalPlayer();
	if( LocalPlayer != NULL )
	{
		if( ( LocalPlayer->Actor != NULL )
			)
		{
			AGearPC* PC = Cast<AGearPC>(LocalPlayer->Actor);

			if( bShouldSet == TRUE )
			{
				GetGame()->ExecConsoleCommand( TEXT("DebugAI") );
			}

			if ( PC )
				Retval = PC->bDebugAI;
		}
	}

	return Retval;
}






void WxRemoteControlGearFlagsPage::ShowAccuracy_Activated( wxCommandEvent& In )
{
	UBOOL bIsChecked = In.IsChecked();

	ShowAccuracy_Worker( TRUE, bIsChecked );

	ShowAccuracy_UpdateWidget();
}


void WxRemoteControlGearFlagsPage::ShowAccuracy_UpdateWidget()
{
	bool bCurrValue = ShowAccuracy_Worker( FALSE, FALSE );
	ShowAccuracyCheckBox->SetValue( bCurrValue );
}


UBOOL WxRemoteControlGearFlagsPage::ShowAccuracy_Worker( UBOOL bShouldSet, UBOOL bValueToSet )
{
	UBOOL Retval = FALSE;

	ULocalPlayer* LocalPlayer = GetGame()->GetLocalPlayer();
	if( LocalPlayer != NULL )
	{
		if( ( LocalPlayer->Actor != NULL )
			&& ( LocalPlayer->Actor->Pawn != NULL )
			)
		{
			AGearPawn* WP = Cast<AGearPawn>(LocalPlayer->Actor->Pawn);
			if (WP == NULL)
			{
				// could be in a vehicle, check that
				AVehicle const* const VP = Cast<AVehicle>(LocalPlayer->Actor->Pawn);
				if (VP != NULL)
				{
					WP = Cast<AGearPawn>(VP->Driver);
				}
			}
			if (WP)
			{
				if( bShouldSet == TRUE )
				{
					WP->bWeaponDebug_Accuracy = bValueToSet;
				}

				Retval = WP->bWeaponDebug_Accuracy;
			}
		}
	}

	return Retval;
}





void WxRemoteControlGearFlagsPage::ShowCover_Activated( wxCommandEvent& In )
{
	UBOOL bIsChecked = In.IsChecked();

	ShowCover_Worker( TRUE, bIsChecked );

	ShowCover_UpdateWidget();
}


void WxRemoteControlGearFlagsPage::ShowCover_UpdateWidget()
{
	UBOOL bCurrValue = ShowCover_Worker( FALSE, FALSE );
	ShowCoverCheckBox->SetValue( bCurrValue );
}


UBOOL WxRemoteControlGearFlagsPage::ShowCover_Worker( UBOOL bShouldSet, UBOOL bValueToSet )
{
	UBOOL Retval = FALSE;

	ULocalPlayer* LocalPlayer = GetGame()->GetLocalPlayer();
	if( LocalPlayer != NULL )
	{
		if( ( LocalPlayer->Actor != NULL )
			&& ( LocalPlayer->Actor->Pawn != NULL )
			)
		{
			if( bShouldSet == TRUE )
			{
				GetGame()->ExecConsoleCommand( TEXT("DebugCover") );
			}

			Retval = Cast<AGearPC>(LocalPlayer->Actor)->bDebugCover;
		}
	}

	return Retval;
}





