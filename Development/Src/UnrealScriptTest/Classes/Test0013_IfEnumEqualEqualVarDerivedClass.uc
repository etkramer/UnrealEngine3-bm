/**
 * if( ENUM == var )
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class Test0013_IfEnumEqualEqualVarDerivedClass extends Test0013_IfEnumEqualEqualVar
   dependson( Test0013_IfEnumEqualEqualVar );



function EnumEqualEqualVar()
{
`if(`notdefined(FINAL_RELEASE))
    local Test0013_SomeEnumFoo AnEnum;

    AnEnum = Test0013SEF_ValOne;

 //   if( Test0013SEF_ValOne == AnEnum )  // does not compile
 //   {
 //      `log( "" );
 //   }

      `log( "AnEnum: " $ AnEnum );
`endif
}



function EnumTypeEnumEqualEqualVar()
{
`if(`notdefined(FINAL_RELEASE))
    local Test0013_SomeEnumFoo AnEnum;

    AnEnum = Test0013SEF_ValOne;

    //if( Test0013_SomeEnumFoo.Test0013SEF_ValOne == AnEnum ) // does not compile
    //{
    //   `log( "" );
    //}

      `log( "AnEnum: " $ AnEnum );
`endif
}
