/**
 * Commandlet for verifying that boolean out parameters are handled correctly.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class Test0025_Commandlet extends TestCommandletBase;

var	bool	FirstBool;
var bool	SecondBool;
var	bool	ThirdBool;

event int main( string Params )
{
	local bool bFirstLocalBool, bSecondLocalBool, bThirdLocalBool;
	bFirstLocalBool = false;
	bSecondLocalBool = false;
	bThirdLocalBool = false;

	`log("Performing member var test....");
	RunBoolTest( true, FirstBool );
	`log("After first assignment:" @ `showvar(FirstBool) @ `showvar(SecondBool) @ `showvar(ThirdBool));

	RunBooltest( false, SecondBool );
	`log("After second assignment:" @ `showvar(FirstBool) @ `showvar(SecondBool) @ `showvar(ThirdBool));

	RunBoolTest( true, ThirdBool );
	`log("After third assignment:" @ `showvar(FirstBool) @ `showvar(SecondBool) @ `showvar(ThirdBool));

	`log("");
	`log("Performing local var test....");
	RunBoolTest( true, bFirstLocalBool );
	`log("After first assignment:" @ `showvar(bFirstLocalBool) @ `showvar(bSecondLocalBool) @ `showvar(bThirdLocalBool));

	RunBooltest( false, bSecondLocalBool );
	`log("After second assignment:" @ `showvar(bFirstLocalBool) @ `showvar(bSecondLocalBool) @ `showvar(bThirdLocalBool));

	RunBoolTest( true, bThirdLocalBool );
	`log("After third assignment:" @ `showvar(bFirstLocalBool) @ `showvar(bSecondLocalBool) @ `showvar(bThirdLocalBool));

	return 0;
}

// compiler disallows out booleans
function RunBoolTest( bool bValueToApply, /*out*/ bool BoolVar )
{
	BoolVar = bValueToApply;
}

DefaultProperties
{

}
