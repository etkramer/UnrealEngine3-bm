/**
 * To run this do the following:  run UnrealScriptTest.TestCommandlet
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class TestCommandlet extends CommandLet;

event int main( string Params )
{
   `log( " --= TestCommandlet =--" );

   `log( "You should run each of the indiv test commandlets to test the various runtime regressions." );

   return 0;
}
