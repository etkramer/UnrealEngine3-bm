class ActorFactoryAI extends ActorFactory
    native
    config(Editor)
    editinlinenew
    collapsecategories;

cpptext
{
	virtual AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );

	virtual AActor* GetDefaultActor();
};

var() class<AIController> ControllerClass;
var() class<Pawn> PawnClass;
var() string PawnName;
var() bool bGiveDefaultInventory;
var() array< class<Inventory> > InventoryList;
var() int TeamIndex;

defaultproperties
{
    ControllerClass=Class'AIController'
    TeamIndex=255
    bPlaceable=false
}
