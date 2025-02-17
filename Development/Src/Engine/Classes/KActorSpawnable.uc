/**
 * Version of KActor that can be dynamically spawned and destroyed during gameplay
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/
class KActorSpawnable extends KActor
	native(Physics)
	notplaceable;


/** If this is true then the KActor will scale to zero before hiding self **/
var bool bRecycleScaleToZero;
/** Whether or not we are scaling to zero **/
var protected bool bScalingToZero;


simulated function Tick( float DeltaTime )
{
	Super.Tick( DeltaTime );

	// if we are scaling to zero then do so
	if( bScalingToZero )
	{
		SetDrawScale( DrawScale - DeltaTime );
		if( DrawScale < 0.02 )
		{
			bScalingToZero = FALSE;
			SetDrawScale( default.DrawScale );
			RecycleInternal();
		}
	}
}


simulated function Initialize()
{
	bScalingToZero = FALSE;
	SetDrawScale( default.DrawScale );

	ClearTimer('Recycle');
	SetHidden(FALSE);
	StaticMeshComponent.SetHidden(FALSE);
	bStasis = FALSE;
	SetPhysics(PHYS_RigidBody);
	SetCollision(true, false);
}

/** This will reset the KActorSpawnable to its default state either first scaling to zero or by just hiding the object. **/
simulated function Recycle()
{
	if( bRecycleScaleToZero == TRUE )
	{
		bScalingToZero = TRUE;
	}
	else
	{
		RecycleInternal();
	}
}

/** This will reset the KActorSpawnable to its default state.  This is useful for pooling. **/
simulated function RecycleInternal()
{
	SetHidden(TRUE);
	StaticMeshComponent.SetHidden(TRUE);
	SetPhysics(PHYS_None);
	SetCollision(false, false);
	ClearTimer('Recycle');
	bStasis = TRUE;
}



/** Used when the actor is pulled from a cache for use. */
native function ResetComponents();

defaultproperties
{
	bNoDelete=FALSE
}
