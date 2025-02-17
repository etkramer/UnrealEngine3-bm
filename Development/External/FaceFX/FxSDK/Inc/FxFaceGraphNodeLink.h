//------------------------------------------------------------------------------
// This class defines a link between two nodes in an FxFaceGraph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFaceGraphNodeLink_H__
#define FxFaceGraphNodeLink_H__

#include "FxPlatform.h"
#include "FxObject.h"
#include "FxLinkFn.h"
#include "FxName.h"

namespace OC3Ent
{

namespace Face
{

// Forward declare FxFaceGraphNode.
class FxFaceGraphNode;

/// A link between two nodes in the %Face Graph.
/// \ingroup faceGraph
class FxFaceGraphNodeLink : public FxObject, public FxDataContainer
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxFaceGraphNodeLink, FxObject)
public:
	/// Default constructor.
	FxFaceGraphNodeLink();

	/// Constructor.
	/// \param inputName The name of the node to which to link.
	/// \param inputNode The node to which to link.
	/// \param linkFunctionName The name of the link function to use.
	/// \param linkFunctionType The type of link function to execute on the 
	/// value coming from \a inputNode.
	/// \param linkFunctionParams The parameters of the link function.
	FxFaceGraphNodeLink( const FxName& inputName, FxFaceGraphNode* inputNode, 
					     const FxName& linkFunctionName, 
					     FxLinkFnType linkFunctionType,
					     const FxLinkFnParameters& linkFunctionParams );

	/// Copy constructor.
	FxFaceGraphNodeLink( const FxFaceGraphNodeLink& other );
	/// Assignment operator.
	FxFaceGraphNodeLink& operator=( const FxFaceGraphNodeLink& other );
	/// Destructor.
	virtual ~FxFaceGraphNodeLink();

	/// Returns the name of the linked node.
	FX_INLINE const FxName& GetNodeName( void ) const { return _inputName; }
	
	/// Returns a pointer to the linked node.
	FX_INLINE const FxFaceGraphNode* GetNode( void ) const { return _input; }
	/// Sets the linked node.
	void SetNode( FxFaceGraphNode* inputNode );

	/// Returns the link function algorithm name.
	FX_INLINE FxName GetLinkFnName( void ) const { return _linkFnName; }
	/// Sets the link function algorithm name.
	void SetLinkFnName( const FxName& linkFn );

	/// Returns the link function type.
	FX_INLINE FxLinkFnType GetLinkFnType( void ) const { return _linkFnType; }
	/// Sets the link function pointer.
	void SetLinkFn( const FxLinkFn* linkFn );

	/// Returns the link function parameters.
	FX_INLINE const FxLinkFnParameters& GetLinkFnParams( void ) const { return _linkFnParams; }
	/// Sets the link function parameters.
	void SetLinkFnParams( const FxLinkFnParameters& linkFnParams );

	/// Returns FxTrue if the link is corrective.
	FX_INLINE FxBool IsCorrective( void ) const { return (LFT_Corrective == _linkFnType); }

	/// Returns the correction factor.
	FxReal GetCorrectionFactor( void ) const;
	/// Sets the correction factor.
	void SetCorrectionFactor( FxReal newValue );

	/// Serialization.
	void Serialize( FxArchive& arc );

private:
	/// The name of the node we're linked to.
	FxName _inputName;
	/// The node we're linked to.
	FxFaceGraphNode* _input;
	/// The function name.
	FxName _linkFnName;
	/// The function with which to transform this input's value.
	FxLinkFnType _linkFnType;
	/// The parameters to the link function.
	FxLinkFnParameters _linkFnParams;
};

} // namespace Face

} // namespace OC3Ent

#endif
