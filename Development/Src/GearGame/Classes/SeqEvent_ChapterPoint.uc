/** activated when the user is starting the game from a chapter point */
class SeqEvent_ChapterPoint extends SequenceEvent
	native
	hidecategories(SequenceEvent);

/** identifier for the chapter */
var() EChapterPoint Chapter;
/** sublevels to force load and/or make visible when this chapterpoint is triggered */
var() array<LevelRecord> SubLevelsToLoad;
/** inventory to start Marcus with - if none specified, use defaults */
var() array< class<Weapon> > MarcusInventory;

event Activated()
{
	local GearPawn_COGMarcus Marcus;
	local int i;

	if (MarcusInventory.length > 0)
	{
		// start the match if necessary to get a Marcus in the game
		if (GetWorldInfo().Game.IsInState('PendingMatch'))
		{
			GetWorldInfo().Game.StartMatch();
		}
		foreach GetWorldInfo().AllPawns(class'GearPawn_COGMarcus', Marcus)
		{
			Marcus.InvManager.DiscardInventory();
			for (i = 0; i < MarcusInventory.length; i++)
			{
				Marcus.CreateInventory(MarcusInventory[i]);
			}
		}
	}
}

defaultproperties
{
	ObjName="Chapter Point"
	bPlayerOnly=false
	VariableLinks.Empty()
}
