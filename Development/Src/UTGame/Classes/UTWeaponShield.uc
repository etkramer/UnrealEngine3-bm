/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


/** special actor that only blocks weapons fire */
class UTWeaponShield extends Actor
	native
	abstract;

/** If true, doesn't block projectiles flagged as bNotBlockedByShield */
var bool bIgnoreFlaggedProjectiles;

cpptext
{
	virtual UBOOL IgnoreBlockingBy(const AActor* Other) const;
	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive, AActor* SourceActor, DWORD TraceFlags);
}

defaultproperties
{
	bProjTarget=true
	bCollideActors=true
}
