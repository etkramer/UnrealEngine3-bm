//------------------------------------------------------------------------------
// The template command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxTemplateCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"
#include "FxActorTemplate.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreFile.h"
#include "FxArchive.h"
#include "FxColourMapping.h"
//@todo This is included to get access to FxMsg().  We might want to remove this
//      later on if we actually implement the plug-in architecture.
#include "FxConsole.h"
//@todo Ditto.
#include "FxUtilityFunctions.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxTemplateCommand, 0, FxCommand);

FxTemplateCommand::FxTemplateCommand()
{
}

FxTemplateCommand::~FxTemplateCommand()
{
}

FxCommandSyntax FxTemplateCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-e", "-export", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-s", "-sync", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-f", "-file", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-fg", "-facegraph", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-m", "-mapping", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-ng", "-nodegroups", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-c", "-cameras", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-w", "-workspaces", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-cm", "-colourmap", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-cm2", "-colormap", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-a", "-all", CAT_Flag));
	return newSyntax;
}

FxCommandError FxTemplateCommand::Execute( const FxCommandArgumentList& argList )
{
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	if( pActor )
	{
		FxString filename;
		if( argList.GetArgument("-file", filename) )
		{
			if( argList.GetArgument("-export") )
			{
				if( ExportTemplate(filename) )
				{
					return CE_Success;
				}
			}
			else if( argList.GetArgument("-sync") )
			{
				FxBool syncFaceGraph  = FxFalse;
				FxBool syncMapping    = FxFalse;
				FxBool syncNodeGroups = FxFalse;
				FxBool syncCameras    = FxFalse;
				FxBool syncWorkspaces = FxFalse;
				FxBool syncColourMap  = FxFalse;
				if( argList.GetArgument("-all") )
				{
					syncFaceGraph  = FxTrue;
					syncMapping    = FxTrue;
					syncNodeGroups = FxTrue;
					syncCameras    = FxTrue;
					syncWorkspaces = FxTrue;
					syncColourMap  = FxTrue;
				}
				else
				{
					if( argList.GetArgument("-facegraph") )
					{
						syncFaceGraph = FxTrue;
					}
					if( argList.GetArgument("-mapping") )
					{
						syncMapping = FxTrue;
					}
					if( argList.GetArgument("-nodegroups") )
					{
						syncNodeGroups = FxTrue;
					}
					if( argList.GetArgument("-cameras") )
					{
						syncCameras = FxTrue;
					}
					if( argList.GetArgument("-workspaces") )
					{
						syncWorkspaces = FxTrue;
					}
					if( argList.GetArgument("-colourmap") ||
						argList.GetArgument("-colormap") )
					{
						syncColourMap = FxTrue;
					}
				}
				// For manually issued template -sync commands, if no flags were found assume that
				// -all was intended.
				if( !syncFaceGraph && !syncMapping && !syncNodeGroups && !syncCameras && !syncWorkspaces && !syncColourMap )
				{
					syncFaceGraph  = FxTrue;
					syncMapping    = FxTrue;
					syncNodeGroups = FxTrue;
					syncCameras    = FxTrue;
					syncWorkspaces = FxTrue;
					syncColourMap  = FxTrue;
				}
				if( SyncTemplate(filename, syncFaceGraph, syncMapping, syncNodeGroups, syncCameras, syncWorkspaces, syncColourMap) )
				{
					return CE_Success;
				}
			}
		}
		else
		{
			FxVM::DisplayError("-file must be specified!");
		}
	}
	else
	{
		FxVM::DisplayError("no actor is loaded!");
	}
	return CE_Failure;
}

FxBool FxTemplateCommand::ExportTemplate( const FxString& filename )
{
#ifdef NO_SAVE_VERSION
	filename; // get rid of the warnings;
	FxVM::DisplayError("Can not export template. FaceFX Studio is a no-save build.");
#else
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	if( pActor )
	{
		// Create the actor template.
		FxActorTemplate actorTemplate;
		// Grab the Face Graph for the template.
		FxFaceGraph templateFaceGraph = pActor->GetDecompiledFaceGraph();
		// Strip the bones from the bone poses in the template Face Graph.
		FxFaceGraph::Iterator bonePoseNodeIter    = templateFaceGraph.Begin(FxBonePoseNode::StaticClass());
		FxFaceGraph::Iterator bonePoseNodeEndIter = templateFaceGraph.End(FxBonePoseNode::StaticClass());
		for( ; bonePoseNodeIter != bonePoseNodeEndIter; ++bonePoseNodeIter )
		{
			FxBonePoseNode* pBonePoseNode = FxCast<FxBonePoseNode>(bonePoseNodeIter.GetNode());
			if( pBonePoseNode )
			{
				pBonePoseNode->RemoveAllBones();
			}
		}
		actorTemplate.SetFaceGraph(templateFaceGraph);
		
		// Grab the phoneme mapping for the template.
		FxPhonemeMap templatePhonemeMap;
		FxSessionProxy::GetPhonemeMap(templatePhonemeMap);
		actorTemplate.SetPhonemeMap(templatePhonemeMap);
		
		// Grab the node groups for the template.
		FxArray<FxFaceGraphNodeGroup> templateNodeGroups;
		for( FxSize i = 0; i < FxFaceGraphNodeGroupManager::GetNumGroups(); ++i )
		{
			FxFaceGraphNodeGroup* pNodeGroup = FxFaceGraphNodeGroupManager::GetGroup(i);
			if( pNodeGroup )
			{
				templateNodeGroups.PushBack(*pNodeGroup);
			}
		}
		actorTemplate.SetNodeGroups(templateNodeGroups);

		// Grab the cameras for the template.
		FxArray<FxViewportCamera> templateCameras;
		for( FxSize i = 0; i < FxCameraManager::GetNumCameras(); ++i )
		{
			FxViewportCamera* pCamera = FxCameraManager::GetCamera(i);
			if( pCamera )
			{
				templateCameras.PushBack(*pCamera);
			}
		}
		actorTemplate.SetCameras(templateCameras);

		// Grab the workspaces for the template.
		FxArray<FxWorkspace> templateWorkspaces;
		for( FxSize i = 0; i < FxWorkspaceManager::Instance()->GetNumWorkspaces(); ++i )
		{
			templateWorkspaces.PushBack(FxWorkspaceManager::Instance()->GetWorkspace(i));
		}
		actorTemplate.SetWorkspaces(templateWorkspaces);

		// Grab the colour map
		FxArray<FxColourMapEntry> colourMapping = FxColourMap::ExportColourMap();
		actorTemplate.SetColourMap(colourMapping);

		FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
		directoryCreater.Open();
		FxArchive templateArchive(FxArchiveStoreFile::Create(filename.GetData()), FxArchive::AM_Save);
		templateArchive.Open();
		directoryCreater << actorTemplate;
		templateArchive.SetInternalDataState(directoryCreater.GetInternalData());
		templateArchive << actorTemplate;
		return FxTrue;
	}
#endif
	return FxFalse;
}

FxBool FxTemplateCommand::
SyncTemplate( const FxString& filename, FxBool syncFaceGraph, FxBool syncMapping, 
			  FxBool syncNodeGroups, FxBool syncCameras, FxBool syncWorkspaces,
			  FxBool syncColourMap )
{
	FxArchive templateArchive(FxArchiveStoreFile::Create(filename.GetData()), FxArchive::AM_LinearLoad);
	templateArchive.Open();
	if( templateArchive.IsValid() )
	{
		FxActorTemplate actorTemplate;
		templateArchive << actorTemplate;

		FxBool addedEmptyBonePoseNodes = FxFalse;
		if( syncFaceGraph )
		{
			// Merge the face graph.  First, add the nodes from the template to
			// the merged face graph.
			FxFaceGraph mergedGraph;
			FxFaceGraph::Iterator templateNodeIter = actorTemplate.GetFaceGraph().Begin(FxFaceGraphNode::StaticClass());
			FxFaceGraph::Iterator templateNodeEnd  = actorTemplate.GetFaceGraph().End(FxFaceGraphNode::StaticClass());
			for( ; templateNodeIter != templateNodeEnd; ++templateNodeIter )
			{
				// If the node in the template exists in the actor's face graph, use
				// the node from the actor's face graph.  Otherwise, use the version
				// from the template.
				FxFaceGraphNode* pNode = NULL;
				if( templateNodeIter.GetNode() && 
					FxSessionProxy::GetFaceGraphNode(templateNodeIter.GetNode()->GetNameAsString(), &pNode) )
				{
					FxFaceGraphNode* templateNode = templateNodeIter.GetNode();
					FxFaceGraphNode* newNode = pNode->Clone();
					// The node's min, max and input op come from the template.
					newNode->SetMin(templateNode->GetMin());
					newNode->SetMax(templateNode->GetMax());
					newNode->SetNodeOperation(templateNode->GetNodeOperation());

					// Add any user properties contained in the template that the 
					// node doesn't already have.  Synch all the other user 
					// properties.
					for( FxSize i = 0; i < templateNode->GetNumUserProperties(); ++i )
					{
						if( newNode->FindUserProperty(templateNode->GetUserProperty(i).GetName()) == FxInvalidIndex )
						{
							newNode->AddUserProperty(templateNode->GetUserProperty(i));
						}
						else
						{
							newNode->ReplaceUserProperty(templateNode->GetUserProperty(i));
						}
					}

					mergedGraph.AddNode(newNode);
				}
				else
				{
					FxFaceGraphNode* pNode = templateNodeIter.GetNode();
					if( pNode )
					{
						if( mergedGraph.AddNode(pNode->Clone()) && pNode->IsA(FxBonePoseNode::StaticClass()) )
						{
							addedEmptyBonePoseNodes = FxTrue;
							FxBonePoseNode* pBonePoseNode = FxCast<FxBonePoseNode>(mergedGraph.FindNode(pNode->GetName()));
							FxAssert(pBonePoseNode);
							pBonePoseNode->RemoveAllBones();
							FxWarn("<b>Warning:</b> Added an empty bone pose node to the face graph (<b>%s</b>) from the template.", pNode->GetNameAsCstr());
						}
					}
				}
			}
			// Next, add the nodes from the actor's face graph that aren't in the
			// template to the merged face graph.
			FxFaceGraph* actorFaceGraph = NULL;
			if( FxSessionProxy::GetFaceGraph(actorFaceGraph) && actorFaceGraph )
			{
				FxFaceGraph::Iterator actorNodeIter = actorFaceGraph->Begin(FxFaceGraphNode::StaticClass());
				FxFaceGraph::Iterator actorNodeEnd  = actorFaceGraph->End(FxFaceGraphNode::StaticClass());
				for( ; actorNodeIter != actorNodeEnd; ++actorNodeIter )
				{
					if( actorNodeIter.GetNode() && !actorTemplate.GetFaceGraph().FindNode(actorNodeIter.GetNode()->GetName()) )
					{
						mergedGraph.AddNode(actorNodeIter.GetNode()->Clone());
					}
				}
			}
			// Finally, all the links come from the template face graph.
			for( templateNodeIter = actorTemplate.GetFaceGraph().Begin(FxFaceGraphNode::StaticClass()); 
				templateNodeIter != templateNodeEnd; ++ templateNodeIter )
			{
				FxFaceGraphNode* pNode = templateNodeIter.GetNode();
				if( pNode )
				{
					for( FxSize i = 0; i < pNode->GetNumInputLinks(); ++i )
					{
						const FxFaceGraphNodeLink& nodeLink = pNode->GetInputLink(i);
						mergedGraph.Link(pNode->GetName(), nodeLink.GetNodeName(), 
							nodeLink.GetLinkFnName(), nodeLink.GetLinkFnParams());
					}
				}
			}
			FxSessionProxy::SetFaceGraph(mergedGraph);
		}
		if( syncMapping )
		{
			// Update the mapping.
			FxSessionProxy::SetPhonemeMap(actorTemplate.GetPhonemeMap());
		}
		if( syncNodeGroups )
		{
			// Load the node groups.
			const FxArray<FxFaceGraphNodeGroup> templateNodeGroups = actorTemplate.GetNodeGroups();
			for( FxSize i = 0; i < templateNodeGroups.Length(); ++i )
			{
				// Check if the node group exists in the manager.  If it does,
				// update it.
				FxBool foundNodeGroup = FxFalse;
				for( FxSize j = 0; j < FxFaceGraphNodeGroupManager::GetNumGroups(); ++j ) 
				{
					FxFaceGraphNodeGroup* pNodeGroup = FxFaceGraphNodeGroupManager::GetGroup(j);
					if( pNodeGroup )
					{
						if( pNodeGroup->GetName() == templateNodeGroups[i].GetName() )
						{
							FxFaceGraphNodeGroupManager::RemoveGroup(pNodeGroup->GetName());
							FxFaceGraphNodeGroupManager::AddGroup(templateNodeGroups[i].GetName());
							pNodeGroup = FxFaceGraphNodeGroupManager::GetGroup(templateNodeGroups[i].GetName());
							if( pNodeGroup )
							{
								for( FxSize k = 0; k < templateNodeGroups[i].GetNumNodes(); ++k )
								{
									pNodeGroup->AddNode(templateNodeGroups[i].GetNode(k));
								}
							}
							foundNodeGroup = FxTrue;
							break;
						}
					}
				}
				
				// If the node group didn't already exist in the manager, add it.
				if( !foundNodeGroup )
				{
					FxFaceGraphNodeGroupManager::AddGroup(templateNodeGroups[i].GetName());
					FxFaceGraphNodeGroup* pNodeGroup = FxFaceGraphNodeGroupManager::GetGroup(templateNodeGroups[i].GetName());
					if( pNodeGroup )
					{
						for( FxSize k = 0; k < templateNodeGroups[i].GetNumNodes(); ++k )
						{
							pNodeGroup->AddNode(templateNodeGroups[i].GetNode(k));
						}
					}
				}
			}
			FxSessionProxy::ActorDataChanged(ADCF_NodeGroupsChanged);
		}
		if( syncCameras )
		{
			// Load the cameras.
			for( FxSize i = 0; i < actorTemplate.GetCameras().Length(); ++i )
			{
				// Check if the camera exists in the manager.  If it does, 
				// update it.
				FxBool foundCamera = FxFalse;
				for( FxSize j = 0; j < FxCameraManager::GetNumCameras(); ++j )
				{
					FxViewportCamera* pCamera = FxCameraManager::GetCamera(j);
					if( pCamera )
					{
						if( pCamera->GetName() == actorTemplate.GetCameras()[i].GetName() )
						{
							FxCameraManager::RemoveCamera(pCamera->GetName());
							FxCameraManager::AddCamera(actorTemplate.GetCameras()[i]);
							foundCamera = FxTrue;
							break;
						}
					}
				}

				// If the camera didn't already exist in the manager, add it.
				if( !foundCamera )
				{
					FxCameraManager::AddCamera(actorTemplate.GetCameras()[i]);
				}
			}

			FxSessionProxy::ActorDataChanged(ADCF_CamerasChanged);
		}
		if( syncWorkspaces )
		{
			// Load the workspaces.
			for( FxSize i = 0; i < actorTemplate.GetWorkspaces().Length(); ++i )
			{
				// Check if the workspace exists in the manager.  If it does, 
				// update that workspace.
				FxBool foundWorkspace = FxFalse;
				for( FxSize j = 0; j < FxWorkspaceManager::Instance()->GetNumWorkspaces(); ++j )
				{
					if( FxWorkspaceManager::Instance()->GetWorkspace(j).GetName() == 
						actorTemplate.GetWorkspaces()[i].GetName() )
					{
						FxWorkspaceManager::Instance()->Modify(
							actorTemplate.GetWorkspaces()[i].GetName(),
							actorTemplate.GetWorkspaces()[i]);
						foundWorkspace = FxTrue;
						break;
					}
				}

				// If the workspace didn't already exist in the manager, add it.
				if( !foundWorkspace )
				{
					FxWorkspaceManager::Instance()->Add(actorTemplate.GetWorkspaces()[i]);
				}
			}

			FxSessionProxy::ActorDataChanged(ADCF_WorkspacesChanged);
		}
		if( syncColourMap )
		{
			const FxArray<FxColourMapEntry> colourMap = actorTemplate.GetColourMap();

			// Add all the colours to the colour map.
			for( FxSize i = 0; i < colourMap.Length(); ++i )
			{
				FxColourMap::Set(colourMap[i].name, colourMap[i].colour);
			}

			FxSessionProxy::ActorDataChanged(
				static_cast<FxActorDataChangedFlag>(ADCF_FaceGraphChanged|ADCF_CurvesChanged)
											);
		}

		// If the Face Graph was modified, lay it out again to make it
		// look nice.
		if( syncFaceGraph )
		{
			FxVM::ExecuteCommand("graph -layout");
		}

		if( addedEmptyBonePoseNodes )
		{
			FxMessageBox(_("Synching to the template resulted in empty bone pose nodes being added to the face graph.  As a result, you may notice that nodes do not animate or sliders do not appear to work until you have properly exported a reference pose and bone pose(s) from the FaceFX plug-ins.  Please check the console and log file for more details."), _("Warning"));
			FxWarn("<b>Warning:</b> Synching to the template resulted in empty bone pose nodes being added to the face graph.");
		}

		FxActor* pActor = NULL;
		FxSessionProxy::GetActor(&pActor);
		if( pActor )
		{
			if( 0 == pActor->GetMasterBoneList().GetNumRefBones() )
			{
				FxMessageBox(_("The actor does not have a valid reference pose.  As a result, you may notice that nodes do not animate or sliders do not appear to work until you have properly exported a reference pose and bone pose(s) from the FaceFX plug-ins."), _("Warning"));
				FxWarn("<b>Warning:</b> The actor does not have a valid reference pose.");
			}
		}

		FxSessionProxy::SetIsForcedDirty(FxTrue);
		return FxTrue;
	}

	return FxFalse;
}

} // namespace Face

} // namespace OC3Ent
