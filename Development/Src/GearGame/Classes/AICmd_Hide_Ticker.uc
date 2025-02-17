/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Hide_Ticker extends AICmd_Hide
	within GearAI_Ticker;


/** GoW global macros */


function Pushed()
{	
	//`AILog(GetFuncName()@self);
	GearPawn_LocustTickerBase(MyGearPawn).PlayHideSound();
	Super.Pushed();
}

function Popped()
{	
	GearPawn_LocustTickerBase(MyGearPawn).bHiding=false;
	Super.Popped();
}