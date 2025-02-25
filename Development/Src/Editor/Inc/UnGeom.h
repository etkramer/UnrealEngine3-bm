/*=============================================================================
	UnGeom.h: Support for the geometry editing mode in UnrealEd.

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

class FGeomObject;
class FPoly;

/**
 * Base class for all classes that support storing and editing of geometry.
 */
class FGeomBase
{
public:
	FGeomBase();

	/** Does nothing if not in geometry mode.*/
	void Select( UBOOL InSelect = 1 );

	UBOOL IsSelected() const		{ return SelectionIndex > INDEX_NONE; }
	UBOOL IsFullySelected() const	{ return IsSelected() && SelStrength == 1.f; }
	INT GetSelectionIndex() const	{ return SelectionIndex; }

	/**
	 * Allows manual setting of the selection index.  This is dangerous and should
	 * only be called by the FEdModeGeometry::PostUndo() function.
	 */
	void ForceSelectionIndex( INT InSelectionIndex )	{ SelectionIndex = InSelectionIndex; }

	/** Returns a location that represents the middle of the object */
	virtual FVector GetMidPoint() = 0;

	/** Returns a valid position for the widget to be drawn at for this object */
	virtual FVector GetWidgetLocation()					{ return FVector(0,0,0); }

	//@{
	FGeomObject* GetParentObject();
	const FGeomObject* GetParentObject() const;
	//@}

	/** Returns true if this geometry objects is a vertex. */
	virtual UBOOL IsVertex() const						{ return 0; }

	//@{
	const FVector& GetNormal() const					{ return Normal; }
	const FVector& GetMid() const						{ return Mid; }

	INT GetParentObjectIndex() const					{ return ParentObjectIndex; }

	FLOAT GetSelStrength() const						{ return SelStrength; }
	//@}

	//@{
	void SetNormal(const FVector& InNormal)				{ Normal = InNormal; }
	void SetMid(const FVector& InMid)					{ Mid = InMid; }

	void SetParentObjectIndex(INT InParentObjectIndex)	{ ParentObjectIndex = InParentObjectIndex; }

	void SetSelStrength(FLOAT InSelStrength)			{ SelStrength = InSelStrength; }
	//@}

protected:
	/** Allows polys, edges and vertices to point to the FGeomObject that owns them. */
	INT ParentObjectIndex;

	/** The normal vector for this object. */
	FVector Normal;			

	/** The mid point for this object. */
	FVector Mid;			

	/** Strength of the selection of this object - used for soft selection. */
	FLOAT SelStrength;		

private:
	/** If > INDEX_NONE, this object is selected */
	INT SelectionIndex;		
};

///////////////////////////////////////////////////////////////////////////////

/**
 * An index pair denoting a polygon and vertex within the parent objects ABrush.
 */
struct FPolyVertexIndex
{
	FPolyVertexIndex()
	{
		PolyIndex = VertexIndex = INDEX_NONE;
	}
	FPolyVertexIndex( INT InPolyIndex, INT InVertexIndex )
	{
		PolyIndex = InPolyIndex;
		VertexIndex = InVertexIndex;
	}

	INT PolyIndex, VertexIndex;

	UBOOL operator==( const FPolyVertexIndex& In ) const
	{
		if( In.PolyIndex != PolyIndex
				|| In.VertexIndex != VertexIndex )
		{
			return 0;
		}

		return 1;
	}
};

///////////////////////////////////////////////////////////////////////////////

/**
 * A 3D position.
 */
class FGeomVertex : public FGeomBase, public FVector
{
public:
	FGeomVertex();

	virtual FVector GetWidgetLocation();

	virtual FVector GetMidPoint();

	virtual UBOOL IsVertex() const		{ return 1; }

	/** The list of vertices that this vertex represents. */
	TArray<FPolyVertexIndex> ActualVertexIndices;
	FVector* GetActualVertex( FPolyVertexIndex& InPVI );

	/**
	 * Indices into the parent poly pool. A vertex can belong to more 
	 * than one poly and this keeps track of that relationship.
	 */
	TArray<INT> ParentPolyIndices;

	/** Assignment simply copies off the vertex position. */
	FGeomVertex& operator=( const FVector& In )
	{
		X = In.X;
		Y = In.Y;
		Z = In.Z;
		return *this;
	}
};

///////////////////////////////////////////////////////////////////////////////

/**
 * The space between 2 adjacent vertices.
 */
class FGeomEdge : public FGeomBase
{
public:
	FGeomEdge();
	
	virtual FVector GetWidgetLocation();

	virtual FVector GetMidPoint();

	/** Returns true if InEdge matches this edge, independant of winding. */
	UBOOL IsSameEdge( const FGeomEdge& InEdge ) const;

	/**
	 * Indices into the parent poly pool. An edge can belong to more 
	 * than one poly and this keeps track of that relationship.
	 */
	TArray<INT> ParentPolyIndices;

	/** Indices into the parent vertex pool. */
	INT VertexIndices[2];
};

///////////////////////////////////////////////////////////////////////////////

/**
 * An individual polygon.
 */
class FGeomPoly : public FGeomBase
{
public:
	FGeomPoly();

	virtual FVector GetWidgetLocation();

	virtual FVector GetMidPoint();

	/**
	 * The polygon this poly represents.  This is an index into the polygon array inside 
	 * the ABrush that is selected in this objects parent FGeomObject.
	 */
	INT ActualPolyIndex;
	FPoly* GetActualPoly();

	/** Array of indices into the parent objects edge pool. */
	TArray<int> EdgeIndices;

	UBOOL operator==( const FGeomPoly& In ) const
	{
		if( In.ActualPolyIndex != ActualPolyIndex
				|| In.ParentObjectIndex != ParentObjectIndex
				|| In.EdgeIndices.Num() != EdgeIndices.Num() )
		{
			return 0;
		}

		for( INT x = 0 ; x < EdgeIndices.Num() ; ++x )
		{
			if( EdgeIndices(x) != In.EdgeIndices(x) )
			{
				return 0;
			}
		}

		return 1;
	}
};

///////////////////////////////////////////////////////////////////////////////

/**
 * A group of polygons forming an individual object.
 */
class FGeomObject : public FGeomBase, public FSerializableObject
{
public:
	FGeomObject();
	
	virtual FVector GetMidPoint();

	/** Index to the ABrush actor this object represents. */
	ABrush* ActualBrush;
	ABrush* GetActualBrush()				{ return ActualBrush; }
	const ABrush* GetActualBrush() const	{ return ActualBrush; }

	/**
	 * Used for tracking the order of selections within this object.  This 
	 * is required for certain modifiers to work correctly.
	 *
	 * This list is generated on the fly whenever the array is accessed but
	 * is marked as dirty (see bSelectionOrderDirty).
	 *
	 * NOTE: do not serialize
	 */
	TArray<FGeomBase*> SelectionOrder;

	/** @name Dirty seleciton order
	 * When the selection order is dirty, the selection order array needs to be compiled before
	 * being accessed.  This should NOT be serialized.
	 */
	//@{
	/** Dirties the selection order. */
	void DirtySelectionOrder()			{ bSelectionOrderDirty = 1; }

	/** If 1, the selection order array needs to be compiled before being accessed.
	 *
	 * NOTE: do not serialize
	 */
	UBOOL bSelectionOrderDirty;
	//@}

	void CompileSelectionOrder();

	/** Master lists.  All lower data types refer to the contents of these pools through indices. */
	TArray<FGeomVertex> VertexPool;
	TArray<FGeomEdge> EdgePool;
	TArray<FGeomPoly> PolyPool;

	/**
	 * Compiles a list of unique edges.  This runs through the edge pool
	 * and only adds edges into the pool that aren't already there (the
	 * difference being that this routine counts edges that share the same
	 * vertices, but are wound backwards to each other, as being equal).
	 *
	 * @param	InEdges		The edge array to fill up.
	 */
	void CompileUniqueEdgeArray( TArray<FGeomEdge>* InEdges );

	/** Tells the object to recompute all of it's internal data. */
	void ComputeData();

	/** Erases all current data for this object. */
	void ClearData();

	virtual FVector GetWidgetLocation();
	INT AddVertexToPool( INT InObjectIndex, INT InParentPolyIndex, INT InPolyIndex, INT InVertexIndex );
	INT AddEdgeToPool( FGeomPoly* InPoly, INT InParentPolyIndex, INT InVectorIdxA, INT InVectorIdxB );
	void GetFromSource();
	void SendToSource();
	UBOOL FinalizeSourceData();
	INT GetObjectIndex();

	void SelectNone();
	void SoftSelect();
	INT GetNewSelectionIndex();

	/**
	 * Allows manual setting of the last selection index.  This is dangerous and should
	 * only be called by the FEdModeGeometry::PostUndo() function.
	 */
	void ForceLastSelectionIndex( INT InLastSelectionIndex ) { LastSelectionIndex = InLastSelectionIndex; }

	virtual void Serialize(FArchive& Ar);

private:
	INT LastSelectionIndex;
};

/*-----------------------------------------------------------------------------
	Hit proxies for geometry editing.
-----------------------------------------------------------------------------*/

/**
 * Hit proxy for polygons. 
 */
struct HGeomPolyProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HGeomPolyProxy,HHitProxy);

	FGeomObject*	GeomObject;
	INT				PolyIndex;

	HGeomPolyProxy(FGeomObject* InGeomObject, INT InPolyIndex):
		HHitProxy(HPP_UI),
		GeomObject(InGeomObject),
		PolyIndex(InPolyIndex)
	{}

	virtual EMouseCursor GetMouseCursor()
	{
		return MC_Cross;
	}
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Hit proxy for edges. 
 */
struct HGeomEdgeProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HGeomEdgeProxy,HHitProxy);

	FGeomObject*	GeomObject;
	INT				EdgeIndex;

	HGeomEdgeProxy(FGeomObject* InGeomObject, INT InEdgeIndex):
		HHitProxy(HPP_UI),
		GeomObject(InGeomObject),
		EdgeIndex(InEdgeIndex)
	{}

	virtual EMouseCursor GetMouseCursor()
	{
		return MC_Cross;
	}
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Hit proxy for vertices.
 */
struct HGeomVertexProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HGeomVertexProxy,HHitProxy);

	FGeomObject*	GeomObject;
	INT				VertexIndex;

	HGeomVertexProxy(FGeomObject* InGeomObject, INT InVertexIndex):
		HHitProxy(HPP_UI),
		GeomObject(InGeomObject),
		VertexIndex(InVertexIndex)
	{}

	virtual EMouseCursor GetMouseCursor()
	{
		return MC_Cross;
	}
};

/*-----------------------------------------------------------------------------
	WxGeomModifiers
-----------------------------------------------------------------------------*/

/**
 * The floating modifier window that appears during geometry mode.  This window
 * contains buttons and controls for using the various geometry modifiers.
 */
class WxGeomModifiers : public wxFrame
{
public:
	DECLARE_DYNAMIC_CLASS(WxGeomModifiers);

	WxGeomModifiers()	{}
	WxGeomModifiers( wxWindow* parent, wxWindowID id );
	virtual ~WxGeomModifiers();

	wxStaticBox* ModifierBox;
	TArray<wxRadioButton*> RadioButtons;
	TArray<wxButton*> PushButtons;
	class WxPropertyWindow* PropertyWindow;
	wxButton* ActionButton;

	void OnClose( wxCloseEvent& In );
	void OnSize( wxSizeEvent& In );
	void OnModifierSelected( wxCommandEvent& In );
	void OnModifierClicked( wxCommandEvent& In );

	/**
	 * Enable/Disable modifier buttons based on the modifier supporting the current selection type.
	 */
	void OnUpdateUI( wxUpdateUIEvent& In );

	/**
	 * Takes any input the user has made to the Keyboard section
	 * of the current modifier and causes the modifier activate.
	 */
	void OnActionClicked( wxCommandEvent& In );

	/**
	 * Initializes the controls inside the window based on the
	 * current tool in geometry mode.
	 */
	void InitFromTool();

	/**
	 * Figures out proper locations for all child controls.
	 */
	void PositionChildControls();
	void FinalizePropertyWindowValues();

	DECLARE_EVENT_TABLE();
};

