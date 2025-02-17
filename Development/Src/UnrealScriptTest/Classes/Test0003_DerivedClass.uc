/**
 * Testing extending a base class and the derived class implementing an
 * interface
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class Test0003_DerivedClass extends Test0003_BaseClass
   implements( Test0003_Interface );


function Test0003_FunctionA()
{
  `log( "" );
}

function bool Test0003_FunctionB()
{
   return TRUE;
}

function string Test0003_FunctionC()
{
   return "Test0003_FunctionC()";
}

function int Test0003_FunctionD()
{
   return 1;
}

function float Test0003_FunctionE()
{
   return 1.0f;
}

function byte Test0003_FunctionF()
{
   return 1;
}
