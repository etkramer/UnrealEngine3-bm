//------------------------------------------------------------------------------
// The graph command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxGraphCommand_H__
#define FxGraphCommand_H__

#include "FxCommand.h"
#include "FxFaceGraph.h"

namespace OC3Ent
{

namespace Face
{

// The graph command.
class FxGraphCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxGraphCommand, FxCommand);
public:
	// Constructor.
	FxGraphCommand();
	// Destructor.
	virtual ~FxGraphCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
    // FxTrue if the command is in "-addnode" mode.
	FxBool _addNode;
	// FxTrue if the command is in "-removenode" mode.
	FxBool _removeNode;
	// FxTrue if the command is in "-editnode" mode.
	FxBool _editNode;
	// FxTrue if the node was selected.
	FxBool _selected;
	// The previous selection.
	FxArray<FxString> _prevSelection;
	// FxTrue if the command is in "-link" mode.
	FxBool _link;
	// FxTrue if the command is in "-unlink" mode.
	FxBool _unlink;
	// FxTrue if the command is in "-editlink" mode.
	FxBool _editLink;
	// The graph layout flag.
	FxBool _layout;
	// The node name.
	FxString _node;
	// The node type.
	FxString _type;
	// The node minimum.
	FxReal _nodeMin;
	// The previous node minimum.
	FxReal _prevNodeMin;
	// The node maximum.
	FxReal _nodeMax;
	// The previous node maximum.
	FxReal _prevNodeMax;
	// The node input operation.
	FxString _inputOp;
	// The previous node input operation.
	FxString _prevInputOp;
	// The previous node position.
	FxInt32 _prevNodeX;
	FxInt32 _prevNodeY;
	// A temporary structure to hold information about a link.
	class FxCachedNodeLink
	{
	public:
		FxString NodeName;
		FxString LinkFnName;
		FxLinkFnParameters LinkFnParams;
	};
	// The node inputs.
	FxArray<FxCachedNodeLink> _nodeInputs;
	// The node outputs.
	FxArray<FxCachedNodeLink> _nodeOutputs;
	// The temporary node.
	FxFaceGraphNode* _pTempNode;
	// The from node for the link.
	FxString _from;
	// The to node for the link.
	FxString _to;
	// The link function.
	FxString _linkFn;
	// The previous link function.
	FxString _prevLinkFn;
	// The link parameters.
	FxArray<FxString> _linkFnParams;
	// The actual link parameters.
	FxLinkFnParameters _params;
	// The previous actual link parameters.
	FxLinkFnParameters _prevParams;
	// The type of curve interpolation used for custom link functions.
	FxString _interpType;
	// The node user properties.
	FxArray<FxString> _userProperties;
	// The previous node user properties.
	FxArray<FxFaceGraphNodeUserProperty> _prevUserProperties;
	// The no refresh flag.
	FxBool _noRefresh;
	
	// Save the node properties.
	FxBool _saveNodeProperties( FxFaceGraphNode* pNode );
	// Set the node properties.
	FxBool _setNodeProperties( FxFaceGraphNode* pNode );
	// Set the previous node properties.
	FxBool _setPrevNodeProperties( FxFaceGraphNode* pNode );
	// Get the link function parameters.
	FxBool _getLinkFnParams( const FxLinkFn* pLinkFn );
};

} // namespace Face

} // namespace OC3Ent

#endif
