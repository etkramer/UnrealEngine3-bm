/** triggered when the co-op player (i.e. Dom) leaves the game and is retaken by the AI
 * this is useful for those situations in which Dom needs to do something to progress the game,
 * so the LD can trigger the proper AI actions when the human leaves
 */
class SeqEvent_CoopPlayerLeft extends SequenceEvent;

defaultproperties
{
	ObjCategory="Gear"
	ObjName="Co-op Player Left"
	bPlayerOnly=false
}
