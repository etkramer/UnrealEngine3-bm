/**
 * This is used by the scene management to isolate lights and primitives.  For lighting and actor or component
 * use a DynamicLightEnvironmentComponent.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class LightEnvironmentComponent extends ActorComponent
	collapsecategories
	native;

/** Whether the light environment is used or treated the same as a LightEnvironment=NULL reference. */
var() protected{protected} const bool bEnabled;

/** Whether the light environment should override GSystemSettings.bUseCompositeDynamicLights, and never composite dynamic lights into the light environment. */
var() bool bForceNonCompositeDynamicLights;

/** Array of primitive components which are using this light environment and currently attached. */
var transient protected {protected} array<PrimitiveComponent> AffectedComponents;

cpptext
{
	/**
	 * Signals to the light environment that a light has changed, so the environment may need to be updated.
	 * @param Light - The light that changed.
	 */
	virtual void UpdateLight(const ULightComponent* Light) {}

	// Methods that update AffectedComponents
	void AddAffectedComponent(UPrimitiveComponent* NewComponent);
	void RemoveAffectedComponent(UPrimitiveComponent* OldComponent);
}

/**
 * Changes the value of bEnabled.
 * @param bNewEnabled - The value to assign to bEnabled.
 */
native final function SetEnabled(bool bNewEnabled);

/** Returns whether the light environment is enabled */
native final function bool IsEnabled() const;

defaultproperties
{
	bEnabled=True
}
