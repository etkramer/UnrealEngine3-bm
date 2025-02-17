/**
 * To run this do the following:  run UnrealScriptTest.Test0005_Commandlet
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class Test0005_Commandlet extends TestCommandletBase;


event int main( string Params )
{
   local int Retval;

   Retval = 1;

   `log( " --= Test0005_Commandlet =--" );


   Retval = Test0005();

   return Retval;
}


/**
 * This is basically calling the function on the object looking for runtime
 * errors
 **/
function int Test0005()
{
   local int Retval;

   local Test0005_InterfaceBase ABase;
   local Test0005_InterfaceDerived ADerived;
   local Test0005_ImplementingClass AClass;

   `log( "Running:  Test0005" );

   AClass = new(self) class'Test0005_ImplementingClass';

   AClass.baseFoo();
   AClass.derivedFoo();


   ABase = Test0005_InterfaceBase(AClass);

   if( ABase.baseFoo() != "baseFoo" ) { return 1; }


   ADerived = Test0005_InterfaceDerived(AClass);

   if( ADerived.baseFoo() != "baseFoo" ) { return 1; }
   if( ADerived.derivedFoo() != "derivedFoo" ) { return 1; }



	// all test have passed
   Retval = 0;
   return Retval;
}


DefaultProperties
{
	LogToConsole=true
}
