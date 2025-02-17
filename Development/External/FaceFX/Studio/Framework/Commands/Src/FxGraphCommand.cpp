//------------------------------------------------------------------------------
// The graph command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxGraphCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"
#include "FxClass.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxGraphCommand, 0, FxCommand);

FxGraphCommand::FxGraphCommand()
{
	_isUndoable  = FxTrue;
	_addNode     = FxFalse;
	_removeNode  = FxFalse;
	_editNode    = FxFalse;
	_selected    = FxFalse;
	_link        = FxFalse;
	_unlink		 = FxFalse;
	_editLink    = FxFalse;
	_layout      = FxFalse;
	_nodeMin	 = FxInvalidValue;
	_prevNodeMin = FxInvalidValue;
	_nodeMax	 = FxInvalidValue;
	_prevNodeMax = FxInvalidValue;
	_prevNodeX   = FxInvalidIndex;
	_prevNodeY   = FxInvalidIndex;
	_noRefresh	 = FxFalse;
	_pTempNode   = NULL;
}

FxGraphCommand::~FxGraphCommand()
{
	if( _pTempNode )
	{
		delete _pTempNode;
		_pTempNode = NULL;
	}
}

FxCommandSyntax FxGraphCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-lo", "-layout", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-an", "-addnode", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-nt", "-nodetype", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-n", "-name", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-nx", "-nodex", CAT_Int32));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-ny", "-nodey", CAT_Int32));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-rn", "-removenode", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-en", "-editnode", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-min", "-minimum", CAT_Real));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-max", "-maximum", CAT_Real));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-io", "-inputop", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-l", "-link", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-u", "-unlink", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-el", "-editlink", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-f", "-from", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-t", "-to", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-lf", "-linkfn", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-lp", "-linkfnparams", CAT_StringArray));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-it", "-interp", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-up", "-userprops", CAT_StringArray));
	//@todo We don't use -norefresh anymore, so it can be removed...
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-nr", "-norefresh", CAT_Flag));
	return newSyntax;
}

FxCommandError FxGraphCommand::Execute( const FxCommandArgumentList& argList )
{
	// If -layout is the only argument, the command is not undoable.
	if( argList.GetArgument("-layout") && 1 == argList.GetNumArguments() )
	{
		_isUndoable = FxFalse;
		if( FxSessionProxy::LayoutFaceGraph() )
		{
			return CE_Success;
		}
		return CE_Failure;
	}
	else
	{
		// Check for node commands.
		if( argList.GetArgument("-addnode") )
		{
			_addNode = FxTrue;
			if( !argList.GetArgument("-nodetype", _type) )
			{
				FxVM::DisplayError("-nodetype was not specified!");
				return CE_Failure;
			}
			argList.GetArgument("-nodex", _prevNodeX);
			argList.GetArgument("-nodey", _prevNodeY);
		}
		if( argList.GetArgument("-removenode") )
		{
			_removeNode = FxTrue;
		}
		if( argList.GetArgument("-editnode") )
		{
			_editNode = FxTrue;
		}
		
		if( _addNode || _removeNode || _editNode )
		{
			// The node name is required for all node operations.
			if( !argList.GetArgument("-name", _node) )
			{
				FxVM::DisplayError("-name was not specified!");
				return CE_Failure;
			}
			// Is the node selected?
			_selected = FxSessionProxy::IsFaceGraphNodeSelected(_node);
			// Each property is optional.
			argList.GetArgument("-minimum", _nodeMin);
			argList.GetArgument("-maximum", _nodeMax);
			argList.GetArgument("-inputop", _inputOp);
			argList.GetArgument("-userprops", _userProperties);
			_noRefresh = argList.GetArgument("-norefresh");
			if( _editNode )
			{
				// At least one property must be specified for -editnode.
				if( _nodeMin == FxInvalidValue && _nodeMax == FxInvalidValue &&
					0 == _inputOp.Length() && 0 == _userProperties.Length() )
				{
					FxVM::DisplayError("no properties for -editnode!");
					return CE_Failure;
				}

				if( 0 != _userProperties.Length() && _userProperties.Length() % 2 )
				{
					FxVM::DisplayError("invalid user properties format for -editnode!");
					return CE_Failure;
				}
			}
		}

		// Check for link commands.
		if( argList.GetArgument("-link") )
		{
			_link = FxTrue;
			if( !argList.GetArgument("-linkfn", _linkFn) )
			{
				FxVM::DisplayError("-linkfn was not specified!");
				return CE_Failure;
			}
			argList.GetArgument("-interp", _interpType);
		}
		if( argList.GetArgument("-unlink") )
		{
			_unlink = FxTrue;
		}
		if( argList.GetArgument("-editlink") )
		{
			_editLink = FxTrue;
			argList.GetArgument("-interp", _interpType);
		}

		if( _link || _unlink || _editLink )
		{
			// The nodes from and to are required for all link operations.
			if( !argList.GetArgument("-from", _from) )
			{
				FxVM::DisplayError("-from was not specified!");
				return CE_Failure;
			}
			if( !argList.GetArgument("-to", _to) )
			{
				FxVM::DisplayError("-to was not specified!");
				return CE_Failure;
			}
			if( _link || _editLink )
			{
				// The link type is only required for -link and -editlink.
				if( !argList.GetArgument("-linkfn", _linkFn) )
				{
					FxVM::DisplayError("-linkfn was not specified!");
					return CE_Failure;
				}
				// Parameters are optional.
				argList.GetArgument("-linkfnparams", _linkFnParams);
			}
		}

		// The command also requests that a graph layout be performed.
		if( argList.GetArgument("-layout") )
		{
			_layout = FxTrue;
		}

		return Redo();
	}
}

FxCommandError FxGraphCommand::Undo( void )
{
	// All link operations get priority when being undone.  Node operations
	// are secondary.  Linking is only undoable if it was not part of an add
	// node operation (because the add node undo will bring the link back).
	if( _link && !_addNode )
	{
		// Get the from and to nodes.
		FxFaceGraphNode* pFromNode = NULL;
		FxFaceGraphNode* pToNode   = NULL;
		if( !FxSessionProxy::GetFaceGraphNode(_from, &pFromNode) )
		{
			FxVM::DisplayError("from node not found!");
			return CE_Failure;
		}
		if( !FxSessionProxy::GetFaceGraphNode(_to, &pToNode) )
		{
			FxVM::DisplayError("to node not found!");
			return CE_Failure;
		}
		// Remove the link.
		if( !FxSessionProxy::RemoveFaceGraphNodeLink(pFromNode, pToNode) )
		{
			FxVM::DisplayError("could not remove link!");
			return CE_Failure;
		}
		if( !FxSessionProxy::FaceGraphChanged() )
		{
			FxVM::DisplayError("session error!");
			return CE_Failure;
		}
	}
	else if( _unlink )
	{
		// Get the from and to nodes.
		FxFaceGraphNode* pFromNode = NULL;
		FxFaceGraphNode* pToNode   = NULL;
		if( !FxSessionProxy::GetFaceGraphNode(_from, &pFromNode) )
		{
			FxVM::DisplayError("from node not found!");
			return CE_Failure;
		}
		if( !FxSessionProxy::GetFaceGraphNode(_to, &pToNode) )
		{
			FxVM::DisplayError("to node not found!");
			return CE_Failure;
		}
		// Get the link function.
		const FxLinkFn* pLinkFn = FxLinkFn::FindLinkFunction(_prevLinkFn.GetData());
		if( !pLinkFn )
		{
			FxVM::DisplayError("link function not found!");
			return CE_Failure;
		}
		// Set up the link.
		if( !FxSessionProxy::AddFaceGraphNodeLink(pFromNode, pToNode, pLinkFn, _prevParams) )
		{
			FxVM::DisplayError("could not add link!");
			return CE_Failure;
		}
		if( !FxSessionProxy::FaceGraphChanged() )
		{
			FxVM::DisplayError("session error!");
			return CE_Failure;
		}
	}
	else if( _editLink )
	{
		// Get the from and to nodes.
		FxFaceGraphNode* pFromNode = NULL;
		FxFaceGraphNode* pToNode   = NULL;
		if( !FxSessionProxy::GetFaceGraphNode(_from, &pFromNode) )
		{
			FxVM::DisplayError("from node not found!");
			return CE_Failure;
		}
		if( !FxSessionProxy::GetFaceGraphNode(_to, &pToNode) )
		{
			FxVM::DisplayError("to node not found!");
			return CE_Failure;
		}
	
		// Get the link function.
		const FxLinkFn* pLinkFn = FxLinkFn::FindLinkFunction(_prevLinkFn.GetData());
		if( !pLinkFn )
		{
			FxVM::DisplayError("link function not found!");
			return CE_Failure;
		}
		
		if( !FxSessionProxy::EditFaceGraphNodeLink(pFromNode, pToNode, pLinkFn, _prevParams) )
		{
			FxVM::DisplayError("could not edit link!");
			return CE_Failure;
		}
		if( !FxSessionProxy::FaceGraphChanged() )
		{
			FxVM::DisplayError("session error!");
			return CE_Failure;
		}
	}
	
	// Next undo the node operations.
	if( _addNode )
	{
		FxFaceGraphNode* pNode = NULL;
		if( !FxSessionProxy::GetFaceGraphNode(_node, &pNode) )
		{
			FxVM::DisplayError("could not get node!");
			return CE_Failure;
		}
		else
		{
			if( pNode )
			{
				// Remember the node position.
				FxSessionProxy::GetFaceGraphNodePos(_node, _prevNodeX, _prevNodeY);
				// Remove the node.
				if( !FxSessionProxy::RemoveFaceGraphNode(pNode) )
				{
					FxVM::DisplayError("could not delete node!");
					return CE_Failure;
				}
				if( !FxSessionProxy::FaceGraphChanged() )
				{
					FxVM::DisplayError("session error!");
					return CE_Failure;
				}
			}
		}
		return CE_Success;
	}
	else if( _removeNode )
	{
		if( _pTempNode )
		{
			// Add the node to the face graph.
			if( !FxSessionProxy::AddFaceGraphNode(_pTempNode) )
			{
				FxVM::DisplayError("duplicate node, invalid session, or NULL node!");
				return CE_Failure;
			}
			// Reset the node position.
			FxSessionProxy::SetFaceGraphNodePos(_node, _prevNodeX, _prevNodeY);
			// Reset the node inputs and outputs.
			for( FxSize i = 0; i < _nodeInputs.Length(); ++i )
			{
				FxFaceGraphNode* pInputNode = NULL;
				if( !FxSessionProxy::GetFaceGraphNode(_nodeInputs[i].NodeName, &pInputNode) )
				{
					FxVM::DisplayError("could not get input node!");
					return CE_Failure;
				}
				if( pInputNode )
				{
					FxFaceGraphNodeLink inputLink;
					inputLink.SetNode(pInputNode);
					inputLink.SetLinkFnName(_nodeInputs[i].LinkFnName.GetData());
					inputLink.SetLinkFnParams(_nodeInputs[i].LinkFnParams);
					_pTempNode->AddInputLink(inputLink);
					pInputNode->AddOutput(_pTempNode);
				}
			}
			for( FxSize i = 0; i < _nodeOutputs.Length(); ++i )
			{
				FxFaceGraphNode* pOutputNode = NULL;
				if( !FxSessionProxy::GetFaceGraphNode(_nodeOutputs[i].NodeName, &pOutputNode) )
				{
					FxVM::DisplayError("could not get output node!");
					return CE_Failure;
				}
				if( pOutputNode )
				{
					FxFaceGraphNodeLink outputLink;
					outputLink.SetNode(_pTempNode);
					outputLink.SetLinkFnName(_nodeOutputs[i].LinkFnName.GetData());
					outputLink.SetLinkFnParams(_nodeOutputs[i].LinkFnParams);
					pOutputNode->AddInputLink(outputLink);
					_pTempNode->AddOutput(pOutputNode);
				}
			}
			if( !FxSessionProxy::FaceGraphChanged() )
			{
				FxVM::DisplayError("session error!");
				return CE_Failure;
			}
			// If it was selected, add it back to the current selection.
			if( _selected )
			{
				FxSessionProxy::AddFaceGraphNodeToSelection(_node);
			}
			_pTempNode = NULL;
		}
		else
		{
			FxVM::DisplayError("could not undo node removal!");
			return CE_Failure;
		}
	}
	else if( _editNode )
	{
		FxFaceGraphNode* pNode = NULL;
		if( !FxSessionProxy::GetFaceGraphNode(_node, &pNode) )
		{
			FxVM::DisplayError("could not get node!");
			return CE_Failure;
		}
		else
		{
			if( pNode )
			{
				// Set the previous node properties.
				if( !_setPrevNodeProperties(pNode) )
				{
					return CE_Failure;
				}
				else
				{
					if( !FxSessionProxy::FaceGraphChanged() )
					{
						FxVM::DisplayError("session error!");
						return CE_Failure;
					}
				}
			}
		}
	}
	return CE_Success;
}

FxCommandError FxGraphCommand::Redo( void )
{
	// All node operations get priority when first executed.  Link operations
	// are secondary.
	if( _addNode )
	{
		// Make sure the class type represents a valid FaceFx class.
		const FxClass* pClass = FxClass::FindClassDesc(_type.GetData());
		if( pClass )
		{
			// Construct an object of the class type and make sure that it
			// represents a valid face graph node class.
			FxObject* pObject = pClass->ConstructObject();
			FxFaceGraphNode* pNode = FxCast<FxFaceGraphNode>(pObject);
			if( pNode )
			{
				// Make sure the node is placeable in the face graph.
				if( pNode->IsPlaceable() )
				{
					pNode->SetName(_node.GetData());
					// Set any node properties.
					if( !_setNodeProperties(pNode) )
					{
						FxVM::DisplayError("could not set node properties!");
						return CE_Failure;
					}
					// Add the node to the face graph.
					if( !FxSessionProxy::AddFaceGraphNode(pNode) )
					{
						FxVM::DisplayError("duplicate node, invalid session, or NULL node!");
						return CE_Failure;
					}
					if( _prevNodeX != FxInvalidIndex && _prevNodeY != FxInvalidIndex )
					{
						// Reset the node position.
						FxSessionProxy::SetFaceGraphNodePos(_node, _prevNodeX, _prevNodeY);
					}
					if( !FxSessionProxy::FaceGraphChanged() )
					{
						FxVM::DisplayError("session error!");
						return CE_Failure;
					}
				}
				else
				{
					FxVM::DisplayError("node type is not placeable!");
					return CE_Failure;
				}
			}
			else
			{
				if( pObject )
				{
					delete pObject;
					pObject = NULL;
				}
				FxVM::DisplayError("invalid node type!");
				return CE_Failure;
			}
		}
		else
		{
			FxVM::DisplayError("unknown node type!");
			return CE_Failure;
		}
	}
	else if( _removeNode )
	{
		FxFaceGraphNode* pNode = NULL;
		if( !FxSessionProxy::GetFaceGraphNode(_node, &pNode) )
		{
			FxVM::DisplayError("could not get node!");
			return CE_Failure;
		}
		else
		{
			if( pNode )
			{
				// If it is selected, remove it from the current selection.
				if( _selected )
				{
					FxSessionProxy::RemoveFaceGraphNodeFromSelection(_node);
				}
				_pTempNode = pNode->Clone();
				// Remember the node inputs.
				_nodeInputs.Clear();
				for( FxSize i = 0; i < pNode->GetNumInputLinks(); ++i )
				{
					const FxFaceGraphNodeLink& inputLink = pNode->GetInputLink(i);
					FxCachedNodeLink cachedInputLink;
					cachedInputLink.NodeName     = inputLink.GetNodeName().GetAsString();
					cachedInputLink.LinkFnName   = inputLink.GetLinkFnName().GetAsString();
					cachedInputLink.LinkFnParams = inputLink.GetLinkFnParams();
					_nodeInputs.PushBack(cachedInputLink);
				}
				// Remember the node outputs.
				_nodeOutputs.Clear();
				for( FxSize i = 0; i < pNode->GetNumOutputs(); ++i )
				{
					FxFaceGraphNode* pOutputNode = pNode->GetOutput(i);
					if( pOutputNode )
					{
						for( FxSize j = 0; j < pOutputNode->GetNumInputLinks(); ++j )
						{
							const FxFaceGraphNodeLink& outputLink = pOutputNode->GetInputLink(j);
							if( outputLink.GetNodeName() == pNode->GetName() )
							{
								FxCachedNodeLink cachedOutputLink;
								cachedOutputLink.NodeName     = pOutputNode->GetNameAsString();
								cachedOutputLink.LinkFnName   = outputLink.GetLinkFnName().GetAsString();
								cachedOutputLink.LinkFnParams = outputLink.GetLinkFnParams();
								_nodeOutputs.PushBack(cachedOutputLink);
							}
						}
					}
				}
				// Remember the node position.
				FxSessionProxy::GetFaceGraphNodePos(_node, _prevNodeX, _prevNodeY);
				// Remove the node.
				if( !FxSessionProxy::RemoveFaceGraphNode(pNode) )
				{
					FxVM::DisplayError("could not delete node!");
					return CE_Failure;
				}
				if( !FxSessionProxy::FaceGraphChanged() )
				{
					FxVM::DisplayError("session error!");
					return CE_Failure;
				}
			}
			else
			{
				FxVM::DisplayError("node does not exist!");
				return CE_Failure;
			}
		}
	}
	else if( _editNode )
	{
		FxFaceGraphNode* pNode = NULL;
		if( !FxSessionProxy::GetFaceGraphNode(_node, &pNode) )
		{
			FxVM::DisplayError("could not get node!");
			return CE_Failure;
		}
		else
		{
			if( pNode )
			{
				// Save any node properties.
				if( !_saveNodeProperties(pNode) )
				{
					FxVM::DisplayError("could not save node properties!");
					return CE_Failure;
				}
				// Set any node properties.
				if( !_setNodeProperties(pNode) )
				{
					FxVM::DisplayError("could not set node properties!");
					return CE_Failure;
				}
				else
				{
					if( !_noRefresh )
					{
						if( !FxSessionProxy::FaceGraphChanged() )
						{
							FxVM::DisplayError("session error!");
							return CE_Failure;
						}
					}
					else
					{
						FxVM::DisplayWarning("Not refreshing");
					}
				}
			}
			else
			{
				FxVM::DisplayError("node does not exist!");
				return CE_Failure;
			}
		}
	}

	// Link operations.
	if( _link )
	{
		// Get the from and to nodes.
		FxFaceGraphNode* pFromNode = NULL;
		FxFaceGraphNode* pToNode   = NULL;
		if( !FxSessionProxy::GetFaceGraphNode(_from, &pFromNode) )
		{
			FxVM::DisplayError("from node not found!");
			return CE_Failure;
		}
		if( !FxSessionProxy::GetFaceGraphNode(_to, &pToNode) )
		{
			FxVM::DisplayError("to node not found!");
			return CE_Failure;
		}
		// Get the link function.
		const FxLinkFn* pLinkFn = FxLinkFn::FindLinkFunction(_linkFn.GetData());
		if( !pLinkFn )
		{
			FxVM::DisplayError("link function not found!");
			return CE_Failure;
		}

		// Get the link function parameters.
		if( !_getLinkFnParams(pLinkFn) )
		{
			FxVM::DisplayError("invalid link fn params!");
			// If a node was added in this command, make sure to remove it
			// if the link fails.
			if( _addNode )
			{
				FxFaceGraphNode* pNode = NULL;
				if( !FxSessionProxy::GetFaceGraphNode(_node, &pNode) )
				{
					FxVM::DisplayError("could not get node!");
					return CE_Failure;
				}
				// Remove the node.
				if( !FxSessionProxy::RemoveFaceGraphNode(pNode) )
				{
					FxVM::DisplayError("could not delete node!");
					return CE_Failure;
				}
				if( !FxSessionProxy::FaceGraphChanged() )
				{
					FxVM::DisplayError("session error!");
					return CE_Failure;
				}
			}
			return CE_Failure;
		}

		// Set up the link.
		if( !FxSessionProxy::AddFaceGraphNodeLink(pFromNode, pToNode, pLinkFn, _params) )
		{
			FxVM::DisplayError("could not add link!");
			return CE_Failure;
		}
		if( !FxSessionProxy::FaceGraphChanged() )
		{
			FxVM::DisplayError("session error!");
			return CE_Failure;
		}
	}
	else if( _unlink )
	{
		// Get the from and to nodes.
		FxFaceGraphNode* pFromNode = NULL;
		FxFaceGraphNode* pToNode   = NULL;
		if( !FxSessionProxy::GetFaceGraphNode(_from, &pFromNode) )
		{
			FxVM::DisplayError("from node not found!");
			return CE_Failure;
		}
		if( !FxSessionProxy::GetFaceGraphNode(_to, &pToNode) )
		{
			FxVM::DisplayError("to node not found!");
			return CE_Failure;
		}
		// Save the previous link function.
		if( !FxSessionProxy::GetFaceGraphNodeLink(pFromNode, pToNode, _prevLinkFn, _prevParams) )
		{
			FxVM::DisplayError("could not get link!");
			return CE_Failure;
		}
		// Remove the link.
		if( !FxSessionProxy::RemoveFaceGraphNodeLink(pFromNode, pToNode) )
		{
			FxVM::DisplayError("could not remove link!");
			return CE_Failure;
		}
		if( !FxSessionProxy::FaceGraphChanged() )
		{
			FxVM::DisplayError("session error!");
			return CE_Failure;
		}
	}
	else if( _editLink )
	{
		// Get the from and to nodes.
		FxFaceGraphNode* pFromNode = NULL;
		FxFaceGraphNode* pToNode   = NULL;
		if( !FxSessionProxy::GetFaceGraphNode(_from, &pFromNode) )
		{
			FxVM::DisplayError("from node not found!");
			return CE_Failure;
		}
		if( !FxSessionProxy::GetFaceGraphNode(_to, &pToNode) )
		{
			FxVM::DisplayError("to node not found!");
			return CE_Failure;
		}
		// Save the previous link function.
		if( !FxSessionProxy::GetFaceGraphNodeLink(pFromNode, pToNode, _prevLinkFn, _prevParams) )
		{
			FxVM::DisplayError("could not get link!");
			return CE_Failure;
		}

		// Get the link function.
		const FxLinkFn* pLinkFn = FxLinkFn::FindLinkFunction(_linkFn.GetData());
		if( !pLinkFn )
		{
			FxVM::DisplayError("link function not found!");
			return CE_Failure;
		}
		// Get the link function parameters.
		if( !_getLinkFnParams(pLinkFn) )
		{
			FxVM::DisplayError("invalid get link fn params!");
			return CE_Failure;
		}

		if( !FxSessionProxy::EditFaceGraphNodeLink(pFromNode, pToNode, pLinkFn, _params) )
		{
			FxVM::DisplayError("could not edit link!");
			return CE_Failure;
		}
		if( !FxSessionProxy::FaceGraphChanged() )
		{
			FxVM::DisplayError("session error!");
			return CE_Failure;
		}
	}

	// Layout operations.
	if( _layout )
	{
		if( !FxSessionProxy::LayoutFaceGraph() )
		{
			return CE_Failure;
		}
		// Layout isn't undoable.
		_layout = FxFalse;
	}
	return CE_Success;
}

FxBool FxGraphCommand::_saveNodeProperties( FxFaceGraphNode* pNode )
{
	// Save the node properties.
	if( pNode )
	{
		_prevNodeMin = pNode->GetMin();
		_prevNodeMax = pNode->GetMax();
		if( pNode->GetNodeOperation() == IO_Sum )
		{
			_prevInputOp = "sum";
		}
		else if( pNode->GetNodeOperation() == IO_Multiply )
		{
			_prevInputOp = "mul";
		}
		else if( pNode->GetNodeOperation() == IO_Max )
		{
			_prevInputOp = "max";
		}
		else if( pNode->GetNodeOperation() == IO_Min )
		{
			_prevInputOp = "min";
		}
		else
		{
			FxVM::DisplayError("invalid input op!");
			return FxFalse;
		}
		_prevUserProperties.Clear();
		for( FxSize i = 0; i < pNode->GetNumUserProperties(); ++i )
		{
			_prevUserProperties.PushBack(pNode->GetUserProperty(i));
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxGraphCommand::_setNodeProperties( FxFaceGraphNode* pNode )
{
	// Set any node properties.
	if( pNode )
	{
		if( _nodeMin != FxInvalidValue )
		{
			pNode->SetMin(_nodeMin);
		}
		if( _nodeMax != FxInvalidValue )
		{
			pNode->SetMax(_nodeMax);
		}
		if( _inputOp.Length() > 0 )
		{
			if( _inputOp == "Sum Inputs" || _inputOp.ToLower() == "sum" )
			{
				pNode->SetNodeOperation(IO_Sum);
			}
			else if( _inputOp == "Multiply Inputs" || _inputOp.ToLower() == "mul" )
			{
				pNode->SetNodeOperation(IO_Multiply);
			}
			else if( _inputOp == "Select Max Input" || _inputOp.ToLower() == "max" )
			{
				pNode->SetNodeOperation(IO_Max);
			}
			else if( _inputOp == "Select Min Input" || _inputOp.ToLower() == "min" )
			{
				pNode->SetNodeOperation(IO_Min);
			}
			else
			{
				FxVM::DisplayError("invalid input op!");
				return FxFalse;
			}
		}
		// Warn the user if the node is in a potentially invalid state.
		if( _nodeMin != FxInvalidValue && pNode->GetMin() > pNode->GetMax() )
		{
			FxString warning("Node \"");
			warning = FxString::Concat(warning, pNode->GetNameAsString());
			warning = FxString::Concat(warning, "\" has a minimum value that is greater than it's maximum value.");
			FxVM::DisplayWarning(warning);
		}
		else if( pNode->GetMax() < pNode->GetMin() )
		{
			FxString warning("Node \"");
			warning = FxString::Concat(warning, pNode->GetNameAsString());
			warning = FxString::Concat(warning, "\" has a maximum value that is less than it's minimum value.");
			FxVM::DisplayWarning(warning);
		}
		for( FxSize i = 0; i < _userProperties.Length(); i += 2 )
		{
			FxSize userPropertyIndex = pNode->FindUserProperty(_userProperties[i].GetData());
			const FxFaceGraphNodeUserProperty& userProperty = pNode->GetUserProperty(userPropertyIndex);

			if( UPT_Integer == userProperty.GetPropertyType() )
			{
				FxInt32 integerProperty = FxAtoi(_userProperties[i+1].GetData());
				FxFaceGraphNodeUserProperty newIntegerUserProperty = userProperty;
				newIntegerUserProperty.SetIntegerProperty(integerProperty);
				pNode->ReplaceUserProperty(newIntegerUserProperty);
			}
			else if( UPT_Bool == userProperty.GetPropertyType() )
			{
				FxFaceGraphNodeUserProperty newBoolUserProperty = userProperty;
				if( _userProperties[i+1] == "False" || _userProperties[i+1] == "false" ||
					_userProperties[i+1] == "FALSE" )
				{
					newBoolUserProperty.SetBoolProperty(FxFalse);
				}
				else
				{
					newBoolUserProperty.SetBoolProperty(FxTrue);
				}
				pNode->ReplaceUserProperty(newBoolUserProperty);
			}
			else if( UPT_Real == userProperty.GetPropertyType() )
			{
				FxReal realProperty = FxAtof(_userProperties[i+1].GetData());
				FxFaceGraphNodeUserProperty newRealUserProperty = userProperty;
				newRealUserProperty.SetRealProperty(realProperty);
				pNode->ReplaceUserProperty(newRealUserProperty);
			}
			else if( UPT_String == userProperty.GetPropertyType() )
			{
				FxFaceGraphNodeUserProperty newStringUserProperty = userProperty;
				newStringUserProperty.SetStringProperty(_userProperties[i+1]);
				pNode->ReplaceUserProperty(newStringUserProperty);
			}
			else if( UPT_Choice == userProperty.GetPropertyType() )
			{
				FxFaceGraphNodeUserProperty newChoiceUserProperty = userProperty;
				newChoiceUserProperty.SetChoiceProperty(_userProperties[i+1]);
				pNode->ReplaceUserProperty(newChoiceUserProperty);
			}
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxGraphCommand::_setPrevNodeProperties( FxFaceGraphNode* pNode )
{
	// Set the previous node properties.
	if( pNode )
	{
		pNode->SetMin(_prevNodeMin);
		pNode->SetMax(_prevNodeMax);
		if( _prevInputOp == "sum" )
		{
			pNode->SetNodeOperation(IO_Sum);
		}
		else if( _prevInputOp == "mul" )
		{
			pNode->SetNodeOperation(IO_Multiply);
		}
		else if( _prevInputOp == "max" )
		{
			pNode->SetNodeOperation(IO_Max);
		}
		else if( _prevInputOp == "min" )
		{
			pNode->SetNodeOperation(IO_Min);
		}
		else
		{
			FxVM::DisplayError("invalid input op!");
			return FxFalse;
		}
		// Warn the user if the node is in a potentially invalid state.
		if( _nodeMin != FxInvalidValue && pNode->GetMin() > pNode->GetMax() )
		{
			FxString warning("Node \"");
			warning = FxString::Concat(warning, pNode->GetNameAsString());
			warning = FxString::Concat(warning, "\" has a minimum value that is greater than it's maximum value.");
			FxVM::DisplayWarning(warning);
		}
		else if( pNode->GetMax() < pNode->GetMin() )
		{
			FxString warning("Node \"");
			warning = FxString::Concat(warning, pNode->GetNameAsString());
			warning = FxString::Concat(warning, "\" has a maximum value that is less than it's minimum value.");
			FxVM::DisplayWarning(warning);
		}
		for( FxSize i = 0; i < _prevUserProperties.Length(); ++i )
		{
			pNode->ReplaceUserProperty(_prevUserProperties[i]);
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxGraphCommand::_getLinkFnParams( const FxLinkFn* pLinkFn )
{
	// Parse the link function parameters.
	// Parameters come in as "p0=v|p1=v".
	if( pLinkFn )
	{
		// Set up the default values for each parameter.
		_params.parameters.Clear();
		for( FxSize i = 0; i < pLinkFn->GetNumParams(); ++i )
		{
			_params.parameters.PushBack(pLinkFn->GetParamDefaultValue(i));
		}
		
		for( FxSize i = 0; i < _linkFnParams.Length(); ++i )
		{
			// Remember the parsed parameter's index, name, and value.
			FxSize paramIndex = FxInvalidIndex;
			FxString parameterName;
			FxString parameterValue;
			FxBool foundParameterName = FxFalse;
			for( FxSize j = 0; j < _linkFnParams[i].Length(); ++j )
			{
				// The = character separates the parameter name from the
				// parameter value.  If spaces are present, they are assumed
				// to be part of the parameter name or value.
				if( _linkFnParams[i][j] != '=' )
				{
					if( !foundParameterName )
					{
						// Read the parameter name.
						parameterName = FxString::Concat(parameterName, _linkFnParams[i][j]);
					}
					else
					{
						// Read the parameter value.
						parameterValue = FxString::Concat(parameterValue, _linkFnParams[i][j]);
					}
				}
				else
				{
					// Verify this is an actual parameter name the link function expects.
					for( FxSize k = 0; k < pLinkFn->GetNumParams(); ++k )
					{
						if( pLinkFn->GetParamName(k) == parameterName )
						{
							foundParameterName = FxTrue;
							paramIndex = k;
							break;
						}
					}
					if( !foundParameterName )
					{
						return FxFalse;
					}
				}
			}
			_params.parameters[paramIndex] = FxAtof(parameterValue.GetData());
		}
		return FxTrue;
	}
	return FxFalse;
}

} // namespace Face

} // namespace OC3Ent
