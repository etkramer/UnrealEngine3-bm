/**
 *
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class Test0014_FullyQualifiedEnumFunctionDerivedClass extends Test0014_FullyQualifiedEnumFunction
   dependson( Test0014_FullyQualifiedEnumFunction );



function FullyQualifiedEnum( Test14_SomeEnumFoo Val )
{
}


function Caller()
{
   FullyQualifiedEnum( Test14SEF_ValOne );
   FullyQualifiedEnum( Test14_SomeEnumFoo.Test14SEF_ValOne );
}


