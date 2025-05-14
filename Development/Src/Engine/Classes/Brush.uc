class Brush extends Actor
    native
    hidecategories(Object)
	hidecategories(Movement)
	hidecategories(Display);

enum ECsgOper
{
    CSG_Active,                     // 0
    CSG_Add,                        // 1
    CSG_Subtract,                   // 2
    CSG_Intersect,                  // 3
    CSG_Deintersect,                // 4
    CSG_MAX                         // 5
};

struct native export GeomSelection
{
    var int Type;
    var int Index;
    var int SelectionIndex;
    var float SelStrength;

    structdefaultproperties
    {
        Type=0
        Index=0
        SelectionIndex=0
        SelStrength=0.0000000
    }
};

var() Brush.ECsgOper CsgOper;
var() Color BrushColor;
var int PolyFlags;
var() bool bColored;
var bool bSolidWhenSelected;
var const export Model Brush;
var const editconst export editinline BrushComponent BrushComponent;
var editoronly array<GeomSelection> SavedSelections;

cpptext
{
	// UObject interface.
	virtual void PostLoad();

	virtual void PostEditChange(UProperty* PropertyThatChanged);

	virtual UBOOL IsABrush() const {return TRUE;}

	/**
	 * Note that the object has been modified.  If we are currently recording into the 
	 * transaction buffer (undo/redo), save a copy of this object into the buffer and 
	 * marks the package as needing to be saved.
	 *
	 * @param	bAlwaysMarkDirty	if TRUE, marks the package dirty even if we aren't
	 *								currently recording an active undo/redo transaction
	 */
	virtual void Modify(UBOOL bAlwaysMarkDirty = FALSE);

	/**
	 * Serialize function
	 *
	 * @param Ar Archive to serialize with
	 */
	virtual void Serialize(FArchive& Ar);

	/**
	* Return whether this actor is a builder brush or not.
	*
	* @return TRUE if this actor is a builder brush, FALSE otherwise
	*/
	virtual UBOOL IsABuilderBrush() const;

	/**
	* Return whether this actor is the current builder brush or not
	*
	* @return TRUE if htis actor is the current builder brush, FALSE otherwise
	*/
	virtual UBOOL IsCurrentBuilderBrush() const;

	// ABrush interface.
	virtual void CopyPosRotScaleFrom( ABrush* Other );
	virtual void InitPosRotScale();

	void CheckForErrors();

	/**
	* Figures out the best color to use for this brushes wireframe drawing.
	*/

	virtual FColor GetWireColor() const;
}

defaultproperties
{
    // Reference: BrushComponent'Default__Brush.BrushComponent0'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'BrushComponent0'
    begin object name="BrushComponent0" class=Class'BrushComponent'
    end object
    BrushComponent=BrushComponent0
    bStatic=true
    bHidden=true
    bNoDelete=true
    bEdShouldSnap=true
    Components[0]=BrushComponent0
    CollisionComponent=BrushComponent0
}
