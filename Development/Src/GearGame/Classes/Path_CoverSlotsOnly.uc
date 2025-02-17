/** this path constraint only allows movement along coverslots */
class Path_CoverSlotsOnly extends PathConstraint
	native;

cpptext
{
	virtual UBOOL EvaluatePath(UReachSpec* Spec, APawn* Pawn, INT& out_PathCost, INT& out_HeuristicCost)
	{
		return (Cast<ACoverSlotMarker>(*Spec->End) != NULL);
	}
}

static final function CoverSlotsOnly(Pawn P)
{
	P.AddPathConstraint(P.CreatePathConstraint(default.class));
}

DefaultProperties
{
	CacheIdx=7
}

