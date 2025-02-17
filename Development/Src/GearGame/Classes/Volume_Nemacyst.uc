class Volume_Nemacyst extends Volume
	native
	placeable;

var array<Volume_Nemacyst> Neighbors;

/** returns whether this volume, or any of its neighbors contain the passed point */
final native function bool FormationContainsPoint(out vector Point);

/** internal use only, caches a list of touching nemacyst volumes */
final private native function FindNeighbors();

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	FindNeighbors();
}


defaultproperties
{
	bRouteBeginPlayEvenIfStatic=true
}