/**
 * This class is used for testing and verifying the behavior of component instancing for actors which contain multiple layers of nested components.
 * This component is assigned to the value of a component property in TestComponent_ComplexNesting_Parent.
 * This component contains components of its own.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class CTComplexNestChild extends TestComponentsBase
	native;

var()	CTComplexNestGrandchild UnmodifiedGrandChild_Defined_SingleRef;
var()	CTComplexNestGrandchild UnmodifiedGrandChild_Defined_MultipleRef;
var()	CTComplexNestGrandchild ModifiedGrandChild_Defined_SingleRef;
var()	CTComplexNestGrandchild ModifiedGrandChild_Defined_MultipleRef;

var()	CTComplexNestGrandchild UnmodifiedGrandChild_NotDefined_SingleRef;
var()	CTComplexNestGrandchild UnmodifiedGrandChild_NotDefined_MultipleRef;
var()	CTComplexNestGrandchild ModifiedGrandChild_NotDefined_SingleRef;
var()	CTComplexNestGrandchild ModifiedGrandChild_NotDefined_MultipleRef;

var()	array<CTComplexNestGrandchild> UnmodifiedGrandChild_Defined_Array;
var()	array<CTComplexNestGrandchild> ModifiedGrandChild_Defined_Array;
var()	array<CTComplexNestGrandchild> UnmodifiedGrandChild_Undefined_Array;
var()	array<CTComplexNestGrandchild> ModifiedGrandChild_Undefined_Array;

DefaultProperties
{

}
