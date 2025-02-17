/*=============================================================================
	WinDrv.cpp: Unreal Windows viewport and platform driver.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "WinDrv.h"

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also 
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsWinDrv( INT& Lookup )
{
	UWindowsClient::StaticClass();
	UXnaForceFeedbackManager::StaticClass();
}

/**
 * Auto generates names.
 */
void AutoRegisterNamesWinDrv()
{
}
