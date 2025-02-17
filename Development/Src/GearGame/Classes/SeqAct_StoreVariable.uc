class SeqAct_StoreVariable extends SequenceAction
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
	ObjName="Store Variable"
	ObjCategory="Misc"

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Object",PropertyName=ObjectData)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Bool',LinkDesc="Bool",PropertyName=BoolData)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Int',LinkDesc="Int",PropertyName=IntData)
	VariableLinks(3)=(ExpectedType=class'SeqVar_String',LinkDesc="String",PropertyName=StringData)
	VariableLinks(4)=(ExpectedType=class'SeqVar_Float',LinkDesc="Float",PropertyName=FloatData)
}