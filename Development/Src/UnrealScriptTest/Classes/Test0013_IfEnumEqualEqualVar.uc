/**
 * if( ENUM == var )
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class Test0013_IfEnumEqualEqualVar extends Object;


enum Test0013_SomeEnumFoo
{
	Test0013SEF_ValOne,
	Test0013SEF_ValTwo
};


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
    local Test0013_SomeEnumFoo AnEnum;

    AnEnum = Test0013SEF_ValOne;

    if( Test0013_SomeEnumFoo.Test0013SEF_ValOne == AnEnum )
    {
       `log( "" );
    }

      `log( "AnEnum: " $ AnEnum );
}
