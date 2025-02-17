/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Base class of all geometry mode modifiers.
 */
class GeomModifier
	extends Object
	abstract
	hidecategories(Object,GeomModifier)
	native;

/** A human readable name for this modifier (appears on buttons, menus, etc) */
var(GeomModifier) string Description;

/** If true, this modifier should be displayed as a push button instead of a radio button */
var(GeomModifier) bool bPushButton;

/**
 * TRUE if the modifier has been initialized.
 * This is useful for interpreting user input and mouse drags correctly.
 */
var(GeomModifier) bool bInitialized;

cpptext
{
	/**
	 * @return		The modifier's description string.
	 */
	const FString& GetModifierDescription() const;

	/**
	 * @return		TRUE if the key was handled by this editor mode tool.
	 */
	virtual UBOOL InputKey(struct FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event);

	/**
	 * @return		TRUE if the delta was handled by this editor mode tool.
	 */
	virtual UBOOL InputDelta(struct FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale);

	/**
	 * Applies the modifier.  Does nothing if the editor is not in geometry mode.
	 *
	 * @return		TRUE if something happened.
	 */
 	UBOOL Apply();

	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

	/**
	 * Gives the individual modifiers a chance to do something the first time they are activated.
	 */
	virtual void Initialize();
	
	/**
	 * Starts the modification of geometry data.
	 */
	UBOOL StartModify();

	/**
	 * Ends the modification of geometry data.
	 */
	UBOOL EndModify();

	/**
	 * Handles the starting of transactions against the selected ABrushes.
	 */
	void StartTrans();
	
	/**
	 * Handles the stopping of transactions against the selected ABrushes.
	 */
	void EndTrans();

protected:
	/**
	 * Interface for displaying error messages.
	 *
	 * @param	InErrorMsg		The error message to display.
	 */
	void GeomError(const FString& InErrorMsg);
	
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();
}

defaultproperties
{
	Description="None"
	bPushButton=False
	bInitialized=False
}
