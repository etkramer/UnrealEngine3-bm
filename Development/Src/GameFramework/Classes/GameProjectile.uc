/**
 * GameProjectile
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GameProjectile extends Projectile
	config(Game)
	native
	abstract
	notplaceable;




defaultproperties
{
	bCanBeAdheredTo=FALSE
	bCanBeFrictionedTo=FALSE
}
