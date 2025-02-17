/**
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class SeqEvt_GearTouch extends SeqEvent_Touch
	native(Sequence);

enum EGearPawnRoadieRunFilter
{
	RRF_None,
	RRF_IgnoreRoadieRunningGearPawns,
	RRF_IgnoreNotRoadieRunningGearPawns
};

enum EGearCoverFilterType
{
	GCF_None,
	GCF_IgnoreInCoverHunkeredDown,
	GCF_IgnoreAllInCover
};

var(Gears) EGearPawnRoadieRunFilter RoadieRunFilterType;
var(Gears) EGearCoverFilterType CoverStatusFilterType;

cpptext
{
protected:
	virtual void DoTouchActivation(AActor *InOriginator, AActor *InInstigator);
	virtual void DoUnTouchActivation(AActor *InOriginator, AActor *InInstigator, INT TouchIdx);
}

function CheckRoadieRunTouchActivate(Actor InOriginator,Actor InInstigator, bool bNewRoadieRuning)
{
	if(RoadieRunFilterType == RRF_IgnoreNotRoadieRunningGearPawns)
	{
		if( bNewRoadieRuning)
		{
			CheckTouchActivate(InOriginator,InInstigator);
		}
		else
		{
			CheckUnTouchActivate(InOriginator,InInstigator);
		}
		
	}
	else if(RoadieRunFilterType == RRF_IgnoreRoadieRunningGearPawns)
	{
		if( !bNewRoadieRuning)
		{
			CheckTouchActivate(InOriginator,InInstigator);
		}
		else
		{
			CheckUnTouchActivate(InOriginator,InInstigator);
		}

	}
}

function CheckCoverStatusTouchActivate(Actor InOriginator, Actor InInstigator, bool bInCover, bool bExposed)
{
	//`log(GetFuncName()@bInCover@bExposed@CoverStatusFilterType);
	if(CoverStatusFilterType == GCF_IgnoreInCoverHunkeredDown)
	{
		if(!bInCover || bExposed)
		{
			CheckTouchActivate(InOriginator,InInstigator);
		}
		else
		{
			CheckUnTouchActivate(InOriginator,InInstigator);
		}
	}
	else if(CoverStatusFilterType == GCF_IgnoreAllInCover)
	{
		if(!bInCover)
		{
			CheckTouchActivate(InOriginator,InInstigator);
		}
		else
		{
			CheckUnTouchActivate(InOriginator,InInstigator);
		}
	}
}
defaultproperties
{
	ObjName="Gear Touch"
	ObjCategory="Gear"
	CoverStatusFilterType=GCF_None
	RoadieRunFilterType=RRF_None
}
