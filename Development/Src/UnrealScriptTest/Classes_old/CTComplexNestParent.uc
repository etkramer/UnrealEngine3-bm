/**
 * This class is used for testing and verifying the behavior of component instancing for actors which contain multiple layers of nested components.
 * This component contains a component property, and is assigned to the value of a component property in ComponentTestActor_ComplexNesting.
 * The component assigned to this component's component property has components of its own.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class CTComplexNestParent extends TestComponentsBase
	native;

var()	CTComplexNestChild UnmodifiedChild_Defined_SingleRef;
var()	CTComplexNestChild UnmodifiedChild_Defined_MultipleRef;
var()	CTComplexNestChild ModifiedChild_Defined_SingleRef;
var()	CTComplexNestChild ModifiedChild_Defined_MultipleRef;

var()	CTComplexNestChild UnmodifiedChild_NotDefined_SingleRef;
var()	CTComplexNestChild UnmodifiedChild_NotDefined_MultipleRef;
var()	CTComplexNestChild ModifiedChild_NotDefined_SingleRef;
var()	CTComplexNestChild ModifiedChild_NotDefined_MultipleRef;

var()	array<CTComplexNestChild> UnmodifiedChild_Defined_Array;
var()	array<CTComplexNestChild> ModifiedChild_Defined_Array;
var()	array<CTComplexNestChild> UnmodifiedChild_Undefined_Array;
var()	array<CTComplexNestChild> ModifiedChild_Undefined_Array;

DefaultProperties
{

}
