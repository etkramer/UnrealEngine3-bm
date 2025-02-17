/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
/// UnrealScript "hello world" sample Commandlet.
///
/// Usage:
///     ucc.exe HelloWorld
//=============================================================================
class HelloWorldCommandlet
	extends Commandlet;

var int intparm;
var string strparm;

event int Main( string Parms )
{
	`log( "Hello, world!" );
	`log( "Command line parameters=" $ Parms, Parms!="" );
	`log( "You specified intparm=" $ intparm, intparm!=0 );
	`log( "You specified strparm=" $ strparm, strparm!="" );
	return 0;
}

defaultproperties
{
}
