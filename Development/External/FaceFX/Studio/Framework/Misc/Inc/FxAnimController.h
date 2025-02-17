//------------------------------------------------------------------------------
// Provides for tying an animation to an FxFaceGraph, and exposes mechanisms 
// to operate on them both as a single unit, ensuring correct operation.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnimController_H__
#define FxAnimController_H__

#include "FxPlatform.h"
#include "FxNamedObject.h"

namespace OC3Ent
{

namespace Face
{

class FxAnim;
class FxAnimCurve;
class FxFaceGraph;

class FxAnimController
{
public:
	/// Constructor.
	FxAnimController();
	/// Destructor.
	~FxAnimController();

	/// Sets the animation pointer.
	void SetAnimPointer( FxAnim* pAnim );
	/// Sets the Face Graph pointer
	void SetFaceGraphPointer( FxFaceGraph* pFaceGraph );

	/// Returns FxTrue if the object is in a usable state, FxFalse otherwise.
	FX_INLINE FxBool OK( void ) const { return _pAnim && _pFaceGraph; }

//------------------------------------------------------------------------------
// Animation Stuff
//------------------------------------------------------------------------------
	/// Gets the extents of the animation.
	void GetAnimationTime( FxReal& startTime, FxReal& endTime );

	void DestroyUserData( void );
	void DestroyCurveUserData( FxSize index );

	// Proofs the user data in the anim controller.
	void ProofUserData( void );
//------------------------------------------------------------------------------
// Curve Stuff
//------------------------------------------------------------------------------
	/// Returns the number of curves in the animation.
	FxSize GetNumCurves( void ) const;

	/// Returns the name of the given curve.
	const FxName& GetCurveName( FxSize index ) const;
	/// Returns the index of the curve with the given name.
	FxSize GetCurveIndex( const FxName& name ) const;
	/// Returns a pointer to the curve at index or NULL.
	FxAnimCurve* GetCurve( FxSize index );

	/// Returns true if a curve is visible.
	FxBool IsCurveVisible( FxSize index ) const;
	FxBool IsCurveVisible( const FxName& name ) const;
	/// Sets a curve's visibility flag.
	void SetCurveVisible( FxSize index, FxBool visible );
	void SetCurveVisible( const FxName& name, FxBool visible );

	/// Returns a curve's colour.
	wxColour GetCurveColour( FxSize index ) const;
	wxColour GetCurveColour( const FxName& name) const;
	/// Sets a curve's colour.
	void SetCurveColour( FxSize index, const wxColour& colour );
	void SetCurveColour( const FxName& name, const wxColour& colour );

	/// Returns FxTrue if the curve is virtual.
	FxBool IsCurveVirtual( FxSize index ) const;
	/// Returns FxTrue if the curve is owned by the analysis.
	FxBool IsCurveOwnedByAnalysis( FxSize index ) const;

//------------------------------------------------------------------------------
// Key Stuff
//------------------------------------------------------------------------------
	/// Returns the number keys in a curve.
	FxSize GetNumKeys( FxSize curveIndex ) const;

	/// Sets a key to selected
	void SetKeySelected( FxSize curveIndex, FxSize keyIndex, FxBool selected = FxTrue );
	/// Returns FxTrue if the key is selected.
	FxBool IsKeySelected( FxSize curveIndex, FxSize keyIndex ) const;
	/// Returns FxTrue if the key is the pivot key.
	FxBool IsKeyPivot( FxSize curveIndex, FxSize keyIndex ) const;
	/// Returns FxTrue if the key is a temporary key.
	FxBool IsKeyTemporary( FxSize curveIndex, FxSize keyIndex ) const;
	/// Returns FxTrue if the key's tangent handle is locked.
	FxBool IsTangentLocked( FxSize curveIndex, FxSize keyIndex ) const;
	/// Sets whether or not a tangent handle is locked.
	void SetTangentLocked( FxSize curveIndex, FxSize keyIndex, FxBool isLocked );
	/// Returns the tangent handle points for a specific key.
	void GetTangentHandles( FxSize curveIndex, FxSize keyIndex,
							wxPoint& incTanHandle, wxPoint& outTanHandle );
	/// Sets the tangent handle points for a specific key.
	void SetTangentHandles( FxSize curveIndex, FxSize keyIndex,
							const wxPoint& incTanHandle, const wxPoint outTanHandle );

	/// Returns the information for a given key.
	void GetKeyInfo( FxSize curveIndex, FxSize keyIndex, 
					 FxReal& time, FxReal& value, 
					 FxReal& slopeIn, FxReal& slopeOut );


	/// Returns the time of a given key
	FxReal GetKeyTime( FxSize curveIndex, FxSize keyIndex );

	/// Finds a key at the specified time.  If no key exists at the specified
	/// time, FxInvalidIndex is returned.
	FxSize FindKeyAtTime( FxSize curveIndex, FxReal time );

	/// Inserts a key with the specified time, value, incoming slope,
	/// outgoing slope, and flags.  Returns the index of the newly inserted
	/// key.
	FxSize InsertKey( FxSize curveIndex, FxReal time, FxReal value, 
		               FxReal slopeIn, FxReal slopeOut, 
					   FxInt32 flags, FxBool autoSlope = FxFalse );

	/// Removes the key at the specified index.
	void RemoveKey( FxSize curveIndex, FxSize keyIndex );

	/// Modifies the specified key's value.
	void ModifyKeyValue( FxSize curveIndex, FxSize keyIndex, FxReal value );

	/// Sets the pivot key.
	void SetPivotKey( FxSize curveIndex, FxSize keyIndex );
	/// Gets the index of the pivot key
	void GetPivotKey( FxSize& curveIndex, FxSize& keyIndex );
	/// Modifies the pivot key.
	void ModifyPivotKey( FxReal time, FxReal value, FxReal slopeIn, FxReal slopeOut, FxBool canRecalc = FxTrue );

	enum HitTestFlags
	{
		HitTestAll = 0x1,
		HitTestSelected = 0x2,
		HitTestPivot = 0x4,
	};

	/// Hit tests a key.
	FxBool HitTest( FxInt32 flags, FxReal time, FxReal value, 
					FxReal timeEpsilon, FxReal valueEpsilon,
					FxSize* outCurveIndex = NULL, FxSize* outKeyIndex = NULL );
	/// Hit tests the tangent handles.
	FxBool HitTestTanHandles( FxInt32 x, FxInt32 y, FxInt32 dx, FxInt32 dy, FxInt32& whichHandle );

//------------------------------------------------------------------------------
// Selection stuff
//------------------------------------------------------------------------------
	/// Adds a selection to the visible curves.
	void AddToSelection( FxReal minTime, FxReal maxTime, 
						 FxReal minValue, FxReal maxValue );
	/// Toggles a selection
	void ToggleSelection( FxReal minTime, FxReal maxTime,
						  FxReal minValue, FxReal maxValue );
	/// Clears the selection.
	void ClearSelection( void );

	/// Transforms the selection by a time and value.
	void TransformSelection( FxReal deltaTime, FxReal deltaValue, FxReal slope = FxInvalidValue );

	/// Deletes the keys from the selection.
	void DeleteSelection( void );

	/// Returns the number of selected keys
	FxSize GetNumSelectedKeys( void );

	/// Gets the current edit curve and key values.
	FxInt32 GetEditCurve( void );
	FxInt32 GetEditKey( void );

	/// Recalculates the pivot key - the "leftmost" selected key.
	void RecalcPivotKey( void );

	/// Adds a key to each of the currently selected curves at the given time 
	/// and value, and sets the selection to the new keys.
	FxBool InsertNewKey( FxReal time, FxReal value, FxReal slopeIn = 0.0f, 
					   FxReal slopeOut = 0.0f );

	/// Sets all selected key's tangents to locked or unlocked.
	void SetSelectionTangentLocked( FxBool locked );

//------------------------------------------------------------------------------
// Evaluation
//------------------------------------------------------------------------------
	/// Evaluates each track in the animation for a given time.
	void Evaluate( FxReal time, FxBool useDummyRegisters = FxFalse );

	/// Gets the value of the curve at a time.
	FxReal GetCurveValue( FxSize curveIndex, FxReal time );
	/// Gets the value of the Face Graph from the last evaluation call.
	FxReal GetValue( FxSize curveIndex, FxBool useDummyRegisters = FxFalse );

	/// Gets the current time in the animation controller.
	FxReal GetTime( void ) const;
	/// Sets the current time in the animation controller, forcing an update
	/// at the new time.
	void SetTime( FxReal time, FxBool shouldDisableRegisters = FxTrue );

private:
	/// Links the animation to the Face Graph.
	FxBool Link( void );
	/// Clears the pivot key flags from the entire animation
	void ClearPivotKey( void );
	/// Recalculates the edit curve and edit key values.
	void RecalcEditKey( void );
	
	FxAnim* _pAnim;
	FxFaceGraph*  _pFaceGraph;

	FxInt32 _editCurve;
	FxInt32 _editKey;

	FxReal  _currentTime;
	FxReal  _lastEvalTime;

	FxArray<FxReal> _nodeUserValues;
};

} // namespace Face

} // namespace OC3Ent

#endif
