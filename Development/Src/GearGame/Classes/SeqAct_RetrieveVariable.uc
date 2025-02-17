class SeqAct_RetrieveVariable extends SequenceAction
	native(Sequence);

cpptext
{
	virtual void Activated();
}

/** Name of the variable to use for retrieval */
var() Name VariableName;

var Object ObjectData;
var bool BoolData;
var int IntData;
var string StringData;
var float FloatData;

defaultproperties
{
	ObjName="Retrieve Variable"
	ObjCategory="Misc"

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Object",bWriteable=TRUE,PropertyName=ObjectData)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Bool',LinkDesc="Bool",bWriteable=TRUE,PropertyName=BoolData)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Int',LinkDesc="Int",bWriteable=TRUE,PropertyName=IntData)
	VariableLinks(3)=(ExpectedType=class'SeqVar_String',LinkDesc="String",bWriteable=TRUE,PropertyName=StringData)
	VariableLinks(4)=(ExpectedType=class'SeqVar_Float',LinkDesc="Float",bWriteable=TRUE,PropertyName=FloatData)
}