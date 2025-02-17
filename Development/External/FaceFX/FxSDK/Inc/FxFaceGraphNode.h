//------------------------------------------------------------------------------
// This abstract class implements a node in an FxFaceGraph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFaceGraphNode_H__
#define FxFaceGraphNode_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxNamedObject.h"
#include "FxArray.h"
#include "FxFaceGraphNodeLink.h"
#include "FxFaceGraphShared.h"

namespace OC3Ent
{

namespace Face
{

/// The base class for a node in an FxFaceGraph.
/// \ingroup faceGraph
class FxFaceGraphNode : public FxNamedObject, public FxDataContainer
{
	/// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE(FxFaceGraphNode, FxNamedObject)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxFaceGraphNode)
public:
	/// Face Graphs are our friends.
	friend class FxFaceGraph;

	/// Constructor.
	FxFaceGraphNode();
	/// Destructor.
	virtual ~FxFaceGraphNode() = 0;

	/// Clones the %Face Graph node.  All data, including inherited data, must
	/// be cloned.
	virtual FxFaceGraphNode* Clone( void ) = 0;
	/// Copies the data from this object into the other object.
	virtual void CopyData( FxFaceGraphNode* pOther );

	/// Returns the minimum value this node is allowed to assume.
	FX_INLINE FxReal GetMin( void ) const { return _min; }
	/// Sets the minimum value this node is allowed to assume.
	void SetMin( FxReal iMin );

	/// Returns the maximum value this node is allowed to assume.
	FX_INLINE FxReal GetMax( void ) const { return _max; }
	/// Sets the maximum value this node is allowed to assume.
	void SetMax( FxReal iMax );

	/// Returns the operation performed on inputs.
	FX_INLINE FxInputOp GetNodeOperation( void ) const { return _inputOperation; }
	/// Sets the operation performed on inputs.
	void SetNodeOperation( FxInputOp iNodeOperation );

	/// Adds an input link to this %Face Graph node.
	/// \param newInputLink The link to add to the node.
	/// \return \p FxTrue if the link was added, \p FxFalse otherwise.
	virtual FxBool AddInputLink( const FxFaceGraphNodeLink& newInputLink );
	/// Modifies the input link at index, setting it equal to \a inputLink.
	void ModifyInputLink( FxSize index, const FxFaceGraphNodeLink& inputLink );
	/// Removes the input link at the specified index.
	void RemoveInputLink( FxSize index );
	/// Returns the number of inputs to this %Face Graph node.
	FX_INLINE FxSize GetNumInputLinks( void ) const { return _inputs.Length(); }
	/// Returns a const ref to the input link at the specified index.
	FX_INLINE const FxFaceGraphNodeLink& GetInputLink( FxSize index ) const { return _inputs[index]; }
	
	/// Returns the number of outputs from this node.
	/// \return The number of outputs from this node.
	/// \note For internal use only.
	FX_INLINE FxSize GetNumOutputs( void ) const { return _outputs.Length(); }
	/// Returns the output at index.
	/// \param index The index of the desired output.
	/// \return The output at \a index.
	/// \note For internal use only.
	FX_INLINE FxFaceGraphNode* GetOutput( FxSize index ) { return _outputs[index]; }
	/// Adds an output to the node.
	/// \param pOutput A pointer to the output node.
	/// \note For internal use only.
	void AddOutput( FxFaceGraphNode* pOutput );
	/// Checks if this node type is "placeable" in the tools.
	/// \return FxTrue if the node is "placeable", FxFalse otherwise.
	/// \note For internal use only.
	FxBool IsPlaceable( void ) const;

	/// Adds a user property to the node.  If a user property with the same
	/// name already exists, the user property is not added.  This should only 
	/// be called in the constructor of the new node type to add the same 
	/// properties to all nodes of that type.
	/// \param userProperty The property to add.
	void AddUserProperty( const FxFaceGraphNodeUserProperty& userProperty );
	/// Returns the number of user properties associated with the node.
	FxSize GetNumUserProperties( void ) const;
	/// Returns the index of the user property named 'userPropertyName' or 
	/// FxInvalidIndex if it does not exist.
	FxSize FindUserProperty( const FxName& userPropertyName ) const;
	/// Returns the user property at index.  For performance reasons, if access
	/// to the user property is needed on a regular basis at run-time it is 
	/// best to define or cache the index for each property rather than calling
	/// FindUserProperty() before GetUserProperty().
	const FxFaceGraphNodeUserProperty& GetUserProperty( FxSize index ) const;
	/// Replaces any user property with the same name as the supplied user 
	/// property with the values in the supplied user property.  If the
	/// user property does not exist, nothing happens.
	/// \param userProperty The property with new data.
	void ReplaceUserProperty( const FxFaceGraphNodeUserProperty& userProperty );
	
	/// Serializes an FxFaceGraphNode to an archive.
	virtual void Serialize( FxArchive& arc );

protected:
	/// A flag for whether or not this node has been visited during the current
	/// _hasCycles() operation.
	/// \note This is mutable so it can be changed in _hasCycles().
	mutable FxBool _hasBeenVisited;
	/// The minimum value this node is allowed to assume.
	FxReal _min;
	/// The maximum value this node is allowed to assume.
	FxReal _max;

	/// Defines what to do with the inputs.
	FxInputOp _inputOperation;
	/// The inputs to this node.
	FxArray<FxFaceGraphNodeLink> _inputs;

	/// The outputs from this node.
	/// \note Outputs are strictly for internal bookkeeping usage.
	FxArray<FxFaceGraphNode*> _outputs;

	/// FxTrue if this node type is "placeable" by the user in editing tools.
	/// \note This is for internal usage only.
	FxBool _isPlaceable;

	/// The user properties associated with this node.
	FxArray<FxFaceGraphNodeUserProperty> _userProperties;
	
	/// Returns FxTrue if any input of the node has cycles.
	FxBool _hasCycles( void ) const;
};

} // namespace Face

} // namespace OC3Ent

#endif
