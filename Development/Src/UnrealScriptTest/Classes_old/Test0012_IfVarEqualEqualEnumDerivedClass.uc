/**
 * if( var == ENUM )
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class Test0012_IfVarEqualEqualEnumDerivedClass extends Test0012_IfVarEqualEqualEnum
  dependson( Test0012_IfVarEqualEqualEnum );



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
`if(`notdefined(FINAL_RELEASE))
    local Test0012_SomeEnumFoo AnEnum;

    AnEnum = Test0012SEF_ValOne;

    //if( AnEnum == SomeEnumFoo.SEF_ValOne )  // does not compile
    //{
    //    `log( "" );
    //}

   `log( "AnEnum: " $ AnEnum );
`endif
}
