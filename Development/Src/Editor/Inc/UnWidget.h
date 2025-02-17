 /*=============================================================================
	UnWidget: Editor widgets for control like 3DS Max
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

class FWidget
{
public:
	enum EWidgetMode
	{
		WM_None			= -1,
		WM_Translate,
		WM_Rotate,
		WM_Scale,
		WM_ScaleNonUniform,
		WM_Max,
	};

	FWidget();

	void Render( const FSceneView* View,FPrimitiveDrawInterface* PDI );

	/**
	 * Draws an arrow head line for a specific axis.
	 * @param	bCubeHead		[opt] If TRUE, render a cube at the axis tips.  If FALSE (the default), render a cone.
	 */
	void Render_Axis( const FSceneView* View, FPrimitiveDrawInterface* PDI, EAxis InAxis, FMatrix& InMatrix, UMaterialInterface* InMaterial, FColor* InColor, FVector2D& OutAxisEnd, FLOAT InScale, UBOOL bDrawWidget, UBOOL bCubeHead=FALSE );

	/**
	 * Draws the translation widget.
	 */
	void Render_Translate( const FSceneView* View, FPrimitiveDrawInterface* PDI, const FVector& InLocation, UBOOL bDrawWidget );

	/**
	 * Draws the rotation widget.
	 */
	void Render_Rotate( const FSceneView* View, FPrimitiveDrawInterface* PDI, const FVector& InLocation, UBOOL bDrawWidget );

	/**
	 * Draws the scaling widget.
	 */
	void Render_Scale( const FSceneView* View, FPrimitiveDrawInterface* PDI, const FVector& InLocation, UBOOL bDrawWidget );

	/**
	 * Draws the non-uniform scaling widget.
	 */
	void Render_ScaleNonUniform( const FSceneView* View, FPrimitiveDrawInterface* PDI, const FVector& InLocation, UBOOL bDrawWidget );

	/**
	 * Converts mouse movement on the screen to widget axis movement/rotation.
	 */
	void ConvertMouseMovementToAxisMovement( FEditorLevelViewportClient* InViewportClient, const FVector& InLocation, const FVector& InDiff, FVector& InDrag, FRotator& InRotation, FVector& InScale );

	/**
	 * Sets the axis currently being moused over.  Typically called by FMouseDeltaTracker or FEditorLevelViewportClient.
	 *
	 * @param	InCurrentAxis	The new axis value.
	 */
	void SetCurrentAxis(EAxis InCurrentAxis)
	{
		CurrentAxis = InCurrentAxis;
	}

	/**
	 * @return	The axis currently being moused over.
	 */
	EAxis GetCurrentAxis() const
	{
		return CurrentAxis;
	}

private:
	/** The axis currently being moused over */
	EAxis CurrentAxis;

	/** Locations of the various points on the widget */
	FVector2D Origin, XAxisEnd, YAxisEnd, ZAxisEnd;

	enum
	{
		AXIS_ARROW_SEGMENTS = 6
	};

	/** Materials and colors to be used when drawing the items for each axis */
	UMaterial *AxisMaterialX, *AxisMaterialY, *AxisMaterialZ, *CurrentMaterial;
	FColor AxisColorX, AxisColorY, AxisColorZ, CurrentColor;

	/**
	 * An extra matrix to apply to the widget before drawing it (allows for local/custom coordinate systems).
	 */
	FMatrix CustomCoordSystem;
};

/**
 * Widget hit proxy.
 */
struct HWidgetAxis : public HHitProxy
{
	DECLARE_HIT_PROXY(HWidgetAxis,HHitProxy);

	EAxis Axis;

	HWidgetAxis(EAxis InAxis):
		HHitProxy(HPP_UI),
		Axis(InAxis) {}

	virtual EMouseCursor GetMouseCursor()
	{
		return MC_SizeAll;
	}
};
