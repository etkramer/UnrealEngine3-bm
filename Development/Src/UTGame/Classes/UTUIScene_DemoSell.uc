/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUIScene_DemoSell extends UTUIScene
	native(UI);

var transient UIButton SellButton;
var transient UIButton ExitButton;

native function Sell();

event PostInitialize()
{
	Super.PostInitialize();

	SellButton = UIButton( FindChild('SellButton',true));
	SellButton.OnClicked = OnSell;

	ExitButton = UIButton(FindChild('ExitButton',true));
	ExitButton.OnClicked = OnExit;
}

function bool OnSell(UIScreenObject EventObject, int PlayerIndex)
{
	Sell();
	OnExit(EventObject, PlayerIndex);
	return true;
}

function bool OnExit(UIScreenObject EventObject, int PlayerIndex)
{
	GetUTPlayerOwner().ConsoleCommand("Quit");
	return true;
}


defaultproperties
{
}
