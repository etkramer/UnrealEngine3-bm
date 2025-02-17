/**
 * Base class for grouping all unrealscript component testing classes.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class TestComponentsBase extends ActorComponent
	native;

/**
 * the value for this property will not be modified in instances of this component.
 */
var()			int						UnmodifiedInt;

/**
 * the value for this property will be modified in instances of this component.
 */
var()			float					ModifiedFloat;

DefaultProperties
{
	UnmodifiedInt=10
	ModifiedFloat=10.f
}
