/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** Override for damage because we have a bunch of parameters specific to this channel **/
class AIReactChan_Damage extends AIReactChannel;

/** GoW global macros */

/** Damage Specific Variables **/
var Pawn DamageInstigator;
var bool bDirectDamage;
var class<GearDamageType> damageType;
var TraceHitInfo HitInfo;
var vector LastInstigatorLoc;
var int DamageAmt;


