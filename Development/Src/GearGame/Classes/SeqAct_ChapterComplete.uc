class SeqAct_ChapterComplete extends SequenceAction;

var() EChapterPoint CompletedChapter;

event Activated()
{
	local EChapterPoint UnlockedChapter;
	local GearPC PC;

	UnlockedChapter = (CompletedChapter == CHAP_Max - 1) ? CompletedChapter : EChapterPoint(CompletedChapter + 1);
	foreach GetWorldInfo().AllControllers(class'GearPC', PC)
	{
//		GearGame(PC.WorldInfo.Game).StatsObject.LogGameplayEvent('UnlockedChapter',PC,UnlockedChapter);
		PC.ClientUnlockChapter(UnlockedChapter, CompletedChapter, PC.GetCurrentDifficultyLevel());
	}
}

defaultproperties
{
	ObjName="Chapter Complete"
	ObjCategory="Level"
	bCallHandler=false

	VariableLinks.Empty
}
