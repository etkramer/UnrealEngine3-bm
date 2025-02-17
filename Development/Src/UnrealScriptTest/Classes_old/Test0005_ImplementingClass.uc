/**
 * Test interfaces extending a base interface and having a class implement
 * both of the base and derived's methods 
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Test0005_ImplementingClass extends Object
   implements( Test0005_InterfaceDerived );


function string baseFoo()
{
  return "baseFoo";
}

function string derivedFoo()
{
  return "derivedFoo";
}