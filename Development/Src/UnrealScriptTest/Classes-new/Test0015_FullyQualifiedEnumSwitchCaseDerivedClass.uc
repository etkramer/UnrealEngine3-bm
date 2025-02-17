/**
 *
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class Test0015_FullyQualifiedEnumSwitchCaseDerivedClass extends Test0015_FullyQualifiedEnumSwitchCase
    dependson( Test0015_FullyQualifiedEnumSwitchCase );


function CaseStatementWithFullQualifiedEnum()
{
   local Test15_SomeEnumFoo AnEnum;

   AnEnum = Test15SEF_ValOne;

   switch( AnEnum )
   {
	case Test15SEF_ValOne:  // Error, Bad or missing expression in 'Case'
        break;


//	case SomeEnum.Test15SEF_ValTwo:  // Error, Bad or missing expression in 'Case'
        break;
   };


}
