/**
 * Testing extending a base class implementing an interface and having
 * a derived class overriding a subset of the functions
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class Test0004_BaseClass extends Object
   implements( Test0004_Interface );


function Test0004_FunctionA()
{
  `log( "" );
}

function bool Test0004_FunctionB()
{
   return TRUE;
}

function string Test0004_FunctionC()
{
   return "Test0004_FunctionC()";
}

function int Test0004_FunctionD()
{
   return 1;
}

function float Test0004_FunctionE()
{
   return 1.0f;
}

function byte Test0004_FunctionF()
{
   return 1;
}
