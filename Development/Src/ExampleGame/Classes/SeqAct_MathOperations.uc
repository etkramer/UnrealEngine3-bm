/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_MathOperations extends SequenceAction;

enum Operation
{
     OPER_Add,
     OPER_Subtract,
     OPER_Multiply,
     OPER_Divide
};

var() Float A;
var() float B;
var float Result;
var() Operation MathOperation;

event Activated()
{
        switch(MathOperation)
        {
                case OPER_Add:
                     Result=A+B;
                     break;
                case OPER_Subtract:
                     Result=A-B;
                     break;
                case OPER_Multiply:
                     Result=A*B;
                     break;
                case OPER_Divide:
                     Result=A/B; 
                     break;
                default:
                     Result=0;
                     break;
        }
        
        Super.Activated();
        PopulateLinkedVariableValues(); 
        
        OutputLinks[0].bHasImpulse=True;
}

defaultproperties
{
	ObjName="Operations"
	ObjCategory="MasteringUnreal"

	OutputLinks(0)=(LinkDesc="Out")

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Float',LinkDesc="A",PropertyName=A)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Float',LinkDesc="B",PropertyName=B)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Float',LinkDesc="Result",bWriteable=True,PropertyName=Result)
}
