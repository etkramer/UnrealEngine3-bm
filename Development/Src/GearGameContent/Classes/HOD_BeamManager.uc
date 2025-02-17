/**
 * Hammer of Dawn "Satellite".
 * One instance of this gets spawned, handles shooting beams down on fools' heads.  Manages
 * all beams, etc.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class HOD_BeamManager extends HOD_BeamManagerBase
	notplaceable;

defaultproperties
{
	BeamClass=class'HOD_Beam'
}


