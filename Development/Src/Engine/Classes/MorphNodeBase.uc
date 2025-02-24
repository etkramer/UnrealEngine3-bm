/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class MorphNodeBase extends AnimObject
	native(Anim)
	hidecategories(Object)
	abstract;
 
cpptext
{
	/** Add to array all the active morphs below this one, including their weight. */
	virtual void GetActiveMorphs(TArray<FActiveMorph>& OutMorphs) {}
	
	/** Do any initialisation necessary for this MorphNode. */
	virtual void InitMorphNode(USkeletalMeshComponent* InSkelComp);

	/** Add all nodes at or below this one to the output array. */
	virtual void GetNodes(TArray<UMorphNodeBase*>& OutNodes);

	// EDITOR	
	
	/** 
	 * Draws this morph node in the AnimTreeEditor.
	 *
	 * @param	Canvas			The canvas to use.
	 * @param	bSelected		TRUE if this node is selected.
	 * @param	bCurves			If TRUE, render links as splines; if FALSE, render links as straight lines.
	 */
	virtual void DrawMorphNode(FCanvas* Canvas, UBOOL bSelected, UBOOL bCurves) {}

	/** Get location of a connection of a particular type. */
	virtual FIntPoint GetConnectionLocation(INT ConnType, INT ConnIndex);	
	
	/** Return current position of slider for this node in the AnimTreeEditor. Return value should be within 0.0 to 1.0 range. */
	virtual FLOAT GetSliderPosition() { return 0.f; }

	/** Called when slider is moved in the AnimTreeEditor. NewSliderValue is in range 0.0 to 1.0. */
	virtual void HandleSliderMove(FLOAT NewSliderValue) {}	

	/** Render on 3d viewport when node is selected. */
	virtual void Render(const FSceneView* View, FPrimitiveDrawInterface* PDI) {}
	/** Draw on 3d viewport canvas when node is selected */
	virtual void Draw(FViewport* Viewport, FCanvas* Canvas, const FSceneView* View) {}
}

/** User-defined name of morph node, used for identifying a particular by node for example. */
var()	name					NodeName;

/**	If true, draw a slider for this node in the AnimSetViewer. */
var		bool					bDrawSlider;