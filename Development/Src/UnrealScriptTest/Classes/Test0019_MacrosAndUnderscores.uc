/**
 * Tests Macros And Underscores
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/
class Test0019_MacrosAndUnderscores extends Object;  

`define NoUnderscores 1
`define Has_Underscores 1

var int TestA;
var int TestB;

defaultproperties
{
	TestA=`NoUnderscores

	TestB=`Has_Underscores
}