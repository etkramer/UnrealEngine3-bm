/**
 * if( var == ENUM )
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class Test0012_IfVarEqualEqualEnum extends Object;


enum Test0012_SomeEnumFoo
{
	Test0012SEF_ValOne,
	Test0012SEF_ValTwo
};



function VarEqualEqualEnum()
{
    local Test0012_SomeEnumFoo AnEnum;

    AnEnum = Test0012SEF_ValOne;

    if( AnEnum == Test0012SEF_ValOne )  // compiles
    {
       `log( "" );
    }

}

function VarEqualEqualEnumTypeEnum()
{
    local Test0012_SomeEnumFoo AnEnum;

    AnEnum = Test0012SEF_ValOne;

    if( AnEnum == Test0012_SomeEnumFoo.Test0012SEF_ValOne )  // compiles
    {
       `log( "" );
    }

}
