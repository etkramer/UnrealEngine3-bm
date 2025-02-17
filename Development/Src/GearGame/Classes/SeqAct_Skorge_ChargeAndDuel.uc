class SeqAct_Skorge_ChargeAndDuel extends SeqAct_Latent
	native(Sequence);

cpptext
{
	UBOOL UpdateOp(FLOAT DeltaTime);
	void DeActivated();
};

/** Position to start charge attack from */
var() Actor	StartPosition;
/** Location to teleport when done with attack */
var() Actor	Destination;

/** 
 *	Final result of duel to determine which output to fire 
 *	0 = Player win
 *	1 = Skorge win
 *	2 = Draw
 */
var Byte DuelResult;

/** Force the result of the duel to be a draw */
var() bool bForceDraw;

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 3;
}

defaultproperties
{
	ObjName="Skorge: Charge and Duel"
	ObjCategory="Boss"

	OutputLinks(0)=(LinkDesc="Player Win")
	OutputLinks(1)=(LinkDesc="Skorge Win")
	OutputLinks(2)=(LinkDesc="Draw")
	OutputLinks(3)=(LinkDesc="Dueling Marcus")
	OutputLinks(4)=(LinkDesc="Dueling Dom")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Skorge",PropertyName=Targets)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Start",PropertyName=StartPosition)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Dest",PropertyName=Destination)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Bool',LinkDesc="Force Draw",PropertyName=bForceDraw)
}