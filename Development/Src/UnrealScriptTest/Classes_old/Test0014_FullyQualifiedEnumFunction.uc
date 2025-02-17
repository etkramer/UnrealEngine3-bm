/**
 *
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class Test0014_FullyQualifiedEnumFunction extends Object;


enum Test14_SomeEnumFoo
{
  Test14SEF_ValOne,
  Test14SEF_ValTwo
};



function FullyQualifiedEnum( Test14_SomeEnumFoo Val )
{
}


function Caller()
{
   FullyQualifiedEnum( Test14SEF_ValOne );
   FullyQualifiedEnum( Test14_SomeEnumFoo.Test14SEF_ValOne );
}


