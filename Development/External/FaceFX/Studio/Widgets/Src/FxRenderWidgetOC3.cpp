//------------------------------------------------------------------------------
// The render widget for OC3's internal engine FxRenderer.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#ifdef USE_FX_RENDERER

#include "FxRenderWidgetOC3.h"
#include "FxStudioApp.h"
#include "FxUtilityFunctions.h"
#include "FxArchive.h"
#include "FxArchiveStoreFile.h"
#include "FxArchiveStoreFileFast.h"
#include "FxMorphTargetNode.h"
#include "FxProgressDialog.h"
#include "FxFaceGraphNodeUserData.h"
#include "FxCameraManager.h"
#include "FxCreateCameraDialog.h"

#ifdef IS_FACEFX_STUDIO
	#include "FxVM.h"
	#include "FxSessionProxy.h"
#endif // IS_FACEFX_STUDIO

#include "wx/dcbuffer.h"

namespace OC3Ent
{

namespace Face
{

struct FxMorphTargetUserData
{
	FxMorphTargetUserData() : pMorphTarget(NULL) {}
	void SetMorphTarget( FxScene* pScene, const FxName& morphTargetName )
	{
		pMorphTarget = NULL;
		if( pScene )
		{
			FxSize numMeshes = pScene->GetNumMeshes();
			for( FxSize i = 0; i < numMeshes; ++i )
			{
				FxMesh* mesh = pScene->GetMesh(i);
				if( mesh )
				{
					FxMorphController* morpher = mesh->GetMorphController();
					if( morpher )
					{
						FxSize numTargets = morpher->GetNumTargets();
						for( FxSize j = 0; j < numTargets; ++j )
						{
							FxMorphTarget& morphTarget = morpher->GetTarget(j);
							if( morphTargetName == morphTarget.GetName() )
							{
								pMorphTarget = &morphTarget;
								return;
							}
						}
					}
				}
			}
		}
	}
	FxMorphTarget* pMorphTarget;
};

//------------------------------------------------------------------------------
// FxRendererContainerWindow.
//------------------------------------------------------------------------------

WX_IMPLEMENT_DYNAMIC_CLASS(FxRendererContainerWindow, wxWindow)

BEGIN_EVENT_TABLE(FxRendererContainerWindow, wxWindow)
	EVT_SIZE(FxRendererContainerWindow::OnSize)
	EVT_HELP_RANGE(wxID_ANY, wxID_HIGHEST, FxRendererContainerWindow::OnHelp)
END_EVENT_TABLE()

FxRendererContainerWindow::FxRendererContainerWindow( wxWindow* parent )
	: Super(parent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE|wxCLIP_CHILDREN)
	, _renderWindow(NULL)
{
}

FxRendererContainerWindow::~FxRendererContainerWindow()
{
}

void FxRendererContainerWindow::SetRenderWindow( FxRenderWindowWxOGL* renderWindow )
{
	_renderWindow = renderWindow;
}

void FxRendererContainerWindow::OnSize( wxSizeEvent& event )
{
	if( _renderWindow )
	{
		_renderWindow->SetClientSize(GetClientSize());
	}
	event.Skip();
}

void FxRendererContainerWindow::OnHelp(wxHelpEvent& FxUnused(event))
{
#ifdef IS_FACEFX_STUDIO
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().LoadFile();
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().DisplaySection(wxT("Preview Tab"));
#endif
}

//------------------------------------------------------------------------------
// FxRenderWidgetOC3OffscreenViewport.
//------------------------------------------------------------------------------

FxRenderWidgetOC3OffscreenViewport::FxRenderWidgetOC3OffscreenViewport()
{
#ifdef WIN32
	_pOffscreenRenderTarget = new FxRendererOGLOffscreenTargetWin32();
#else
	_pOffscreenRenderTarget = NULL;
#endif
}

FxRenderWidgetOC3OffscreenViewport::~FxRenderWidgetOC3OffscreenViewport()
{
	if( _pOffscreenRenderTarget )
	{
		delete _pOffscreenRenderTarget;
		_pOffscreenRenderTarget = NULL;
	}
}

void FxRenderWidgetOC3OffscreenViewport::GetRenderTargetSize( FxInt32& width, FxInt32& height )
{
	width = 0;
	height = 0;
	if( _pOffscreenRenderTarget )
	{
		_pOffscreenRenderTarget->GetRenderTargetSize(width, height);
	}
}

void FxRenderWidgetOC3OffscreenViewport::SetRenderTargetSize( FxInt32 width, FxInt32 height )
{
	if( _pOffscreenRenderTarget )
	{
		_pOffscreenRenderTarget->SetRenderTargetSize(width, height);
	}
}

void FxRenderWidgetOC3OffscreenViewport::Render( const FxName& cameraToUse )
{
	if( _pOffscreenRenderTarget )
	{
#ifdef WIN32
		_pOffscreenRenderTarget->Render(cameraToUse);
#endif
	}
}

const FxByte* FxRenderWidgetOC3OffscreenViewport::GetImage( void ) const
{
	if( _pOffscreenRenderTarget )
	{
#ifdef WIN32	
		return _pOffscreenRenderTarget->GetImage();
#endif
	}
	return NULL;
}

//------------------------------------------------------------------------------
// FxRenderWidgetOC3NotifyHook.
//------------------------------------------------------------------------------

void FxRenderWidgetOC3NotifyHook::NotifyRenderStateChanged( void )
{
#ifdef IS_FACEFX_STUDIO
	// Force all open viewports to be redrawn, thus ensuring a consistent 
	// rendering state across all viewports.
	FxSessionProxy::RefreshControls();
#endif // IS_FACEFX_STUDIO
}

//------------------------------------------------------------------------------
// FxRenderWidgetOC3.
//------------------------------------------------------------------------------

void FxOC3RenderWidgetRemoveUserData( FxCompiledFaceGraph& cg )
{
	FxSize numNodes = cg.nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		if( NT_MorphTarget == cg.nodes[i].nodeType && cg.nodes[i].pUserData )
		{
			FxMorphTargetUserData* pUserData = reinterpret_cast<FxMorphTargetUserData*>(cg.nodes[i].pUserData);
			if( pUserData->pMorphTarget )
			{
				pUserData->pMorphTarget->SetWeight(0.0f);
			}
			delete pUserData;
			cg.nodes[i].pUserData = NULL;
		}
	}
}

void FxOC3RenderWidgetPreDestroyCallbackFunc( FxCompiledFaceGraph& cg )
{
	FxOC3RenderWidgetRemoveUserData(cg);
}

void FxOC3RenderWidgetPreCompileCallbackFunc( FxCompiledFaceGraph& cg, FxFaceGraph& FxUnused(faceGraph) )
{
	FxOC3RenderWidgetRemoveUserData(cg);
}

WX_IMPLEMENT_DYNAMIC_CLASS(FxRenderWidgetOC3, wxWindow)

BEGIN_EVENT_TABLE(FxRenderWidgetOC3, wxWindow)
	EVT_SIZE(FxRenderWidgetOC3::OnSize)
	EVT_PAINT(FxRenderWidgetOC3::OnPaint)
	EVT_SYS_COLOUR_CHANGED(FxRenderWidgetOC3::OnSystemColoursChanged)
	EVT_BUTTON(ControlID_RenderWidgetOC3ToolbarResetViewButton, FxRenderWidgetOC3::OnResetViewButton)
	EVT_BUTTON(ControlID_RenderWidgetOC3ToolbarToggleWireframeRenderingButton, FxRenderWidgetOC3::OnToggleWireframeRenderingButton)
	EVT_BUTTON(ControlID_RenderWidgetOC3ToolbarToggleLightingButton, FxRenderWidgetOC3::OnToggleLightingButton)
	EVT_BUTTON(ControlID_RenderWidgetOC3ToolbarToggleMeshRenderingButton, FxRenderWidgetOC3::OnToggleMeshRenderingButton)
	EVT_BUTTON(ControlID_RenderWidgetOC3ToolbarToggleNormalsRenderingButton, FxRenderWidgetOC3::OnToggleNormalsRenderingButton)
	EVT_BUTTON(ControlID_RenderWidgetOC3ToolbarToggleInfluencesRenderingButton, FxRenderWidgetOC3::OnToggleInfluencesRenderingButton)
	EVT_BUTTON(ControlID_RenderWidgetOC3ToolbarToggleSkeletonRenderingButton, FxRenderWidgetOC3::OnToggleSkeletonRenderingButton)
	EVT_BUTTON(ControlID_RenderWidgetOC3ToolbarCreateCameraButton, FxRenderWidgetOC3::OnCreateCameraButton)
	EVT_BUTTON(ControlID_RenderWidgetOC3ToolbarDeleteCameraButton, FxRenderWidgetOC3::OnDeleteCameraButton)
	EVT_CHOICE(ControlID_RenderWidgetOC3ToolbarCameraChoice, FxRenderWidgetOC3::OnCameraChoiceChange)
	EVT_UPDATE_UI_RANGE(ControlID_RenderWidgetOC3ToolbarResetViewButton, ControlID_RenderWidgetOC3ToolbarCameraChoice, FxRenderWidgetOC3::UpdateToolbarState)
END_EVENT_TABLE()

FxRenderWidgetOC3::FxRenderWidgetOC3( wxWindow* parent, FxWidgetMediator* mediator )
	: Super(parent, mediator)
	, _renderWindow(NULL)
	, _containerWindow(NULL)
	, _toolbarResetViewButton(NULL)
	, _toolbarToggleWireframeRenderingButton(NULL)
	, _toolbarToggleLightingButton(NULL)
	, _toolbarToggleMeshRenderingButton(NULL)
	, _toolbarToggleNormalsRenderingButton(NULL)
	, _toolbarToggleInfluencesRenderingButton(NULL)
	, _toolbarToggleSkeletonRenderingButton(NULL)
	, _toolbarCreateCameraButton(NULL)
	, _toolbarDeleteCameraButton(NULL)
	, _toolbarCameraChoice(NULL)
#ifdef IS_FACEFX_STUDIO
	, rw_showbindpose(FxVM::FindConsoleVariable("rw_showbindpose"))
	, rw_scalefactor(FxVM::FindConsoleVariable("rw_scalefactor"))
#else
	, rw_showbindpose(NULL)
	, rw_scalefactor(NULL)
#endif
	, _readyToDraw(FxFalse)
{
#ifdef IS_FACEFX_STUDIO
	FxAssert(rw_showbindpose);
	FxAssert(rw_scalefactor);
#endif
	_pScene = NULL;

	_refreshSystemColors();

	wxBoxSizer* pSizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(pSizer);
	SetAutoLayout(TRUE);
	pSizer->AddSpacer(5);

	_toolbarResetViewButton = new FxButton(this, ControlID_RenderWidgetOC3ToolbarResetViewButton, _("RV"), wxDefaultPosition, wxSize(20,20));
	_toolbarResetViewButton->MakeToolbar();
	_toolbarResetViewButton->SetToolTip(_("Reset View"));
	pSizer->Add(_toolbarResetViewButton, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);

	_toolbarToggleWireframeRenderingButton = new FxButton(this, ControlID_RenderWidgetOC3ToolbarToggleWireframeRenderingButton, _("TW"), wxDefaultPosition, wxSize(20,20));
	_toolbarToggleWireframeRenderingButton->MakeToolbar();
	_toolbarToggleWireframeRenderingButton->MakeToggle();
	_toolbarToggleWireframeRenderingButton->SetToolTip(_("Toggle Wireframe Rendering"));
	pSizer->Add(_toolbarToggleWireframeRenderingButton, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);

	_toolbarToggleLightingButton = new FxButton(this, ControlID_RenderWidgetOC3ToolbarToggleLightingButton, _("TL"), wxDefaultPosition, wxSize(20,20));
	_toolbarToggleLightingButton->MakeToolbar();
	_toolbarToggleLightingButton->MakeToggle();
	_toolbarToggleLightingButton->SetToolTip(_("Toggle Lighting"));
	pSizer->Add(_toolbarToggleLightingButton, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);

	_toolbarToggleMeshRenderingButton = new FxButton(this, ControlID_RenderWidgetOC3ToolbarToggleMeshRenderingButton, _("TM"), wxDefaultPosition, wxSize(20,20));
	_toolbarToggleMeshRenderingButton->MakeToolbar();
	_toolbarToggleMeshRenderingButton->MakeToggle();
	_toolbarToggleMeshRenderingButton->SetToolTip(_("Toggle Mesh Rendering"));
	pSizer->Add(_toolbarToggleMeshRenderingButton, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);

	_toolbarToggleNormalsRenderingButton = new FxButton(this, ControlID_RenderWidgetOC3ToolbarToggleNormalsRenderingButton, _("TN"), wxDefaultPosition, wxSize(20,20));
	_toolbarToggleNormalsRenderingButton->MakeToolbar();
	_toolbarToggleNormalsRenderingButton->MakeToggle();
	_toolbarToggleNormalsRenderingButton->SetToolTip(_("Toggle Normals Rendering"));
	pSizer->Add(_toolbarToggleNormalsRenderingButton, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);

	_toolbarToggleInfluencesRenderingButton = new FxButton(this, ControlID_RenderWidgetOC3ToolbarToggleInfluencesRenderingButton, _("TI"), wxDefaultPosition, wxSize(20,20));
	_toolbarToggleInfluencesRenderingButton->MakeToolbar();
	_toolbarToggleInfluencesRenderingButton->MakeToggle();
	_toolbarToggleInfluencesRenderingButton->SetToolTip(_("Toggle Influences Rendering"));
	pSizer->Add(_toolbarToggleInfluencesRenderingButton, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);

	_toolbarToggleSkeletonRenderingButton = new FxButton(this, ControlID_RenderWidgetOC3ToolbarToggleSkeletonRenderingButton, _("TS"), wxDefaultPosition, wxSize(20,20));
	_toolbarToggleSkeletonRenderingButton->MakeToolbar();
	_toolbarToggleSkeletonRenderingButton->MakeToggle();
	_toolbarToggleSkeletonRenderingButton->SetToolTip(_("Toggle Skeleton Rendering"));
	pSizer->Add(_toolbarToggleSkeletonRenderingButton, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);

	pSizer->AddStretchSpacer();

	_toolbarCreateCameraButton = new FxButton(this, ControlID_RenderWidgetOC3ToolbarCreateCameraButton, _("CC"), wxDefaultPosition, wxSize(20,20));
	_toolbarCreateCameraButton->MakeToolbar();
	_toolbarCreateCameraButton->SetToolTip(_("Create New Camera"));
	pSizer->Add(_toolbarCreateCameraButton, 0, wxALL|wxALIGN_RIGHT, 3);

	_toolbarDeleteCameraButton = new FxButton(this, ControlID_RenderWidgetOC3ToolbarDeleteCameraButton, _("DC"), wxDefaultPosition, wxSize(20,20));
	_toolbarDeleteCameraButton->MakeToolbar();
	_toolbarDeleteCameraButton->SetToolTip(_("Delete Existing Camera"));
	pSizer->Add(_toolbarDeleteCameraButton, 0, wxALL|wxALIGN_RIGHT, 3);

	_toolbarCameraChoice = new wxChoice(this, ControlID_RenderWidgetOC3ToolbarCameraChoice, wxDefaultPosition, wxSize(100,20));
	pSizer->Add(_toolbarCameraChoice, 0, wxALL|wxALIGN_RIGHT, 3);

	Layout();
		
	_containerWindow = new FxRendererContainerWindow(this);
	_renderWindow = new FxRenderWindowWxOGL(_containerWindow, this, &_notifyHook);
	_containerWindow->SetRenderWindow(_renderWindow);

	_resizeComponents();
	_loadIcons();
}

FxRenderWidgetOC3::~FxRenderWidgetOC3()
{
	// Clean up the render window.
	if( _renderWindow )
	{
		_renderWindow->SetScene(NULL);
		_renderWindow->Cleanup();
	}

#ifdef WIN32
	FxRendererOGLOffscreenTargetWin32::SetScene(NULL);
#endif // WIN32
	
    if( _pScene )
	{
		delete _pScene;
		_pScene = NULL;
	}
}

void FxRenderWidgetOC3::OnAppStartup( FxWidget* FxUnused(sender) )
{
	FxRenderer::Startup();
#ifdef WIN32
	if( FxAreWGLExtensionsPresent() )
	{
		FxMsg("Hardware supports WGL_ARB_pbuffer and WGL_ARB_pixel_format.");
		FxUInt32 screenResWidth  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
		FxUInt32 screenResHeight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
		FxMsg("Allocating hardware pbuffer of size %dx%d", screenResWidth, screenResHeight);
		FxRendererOGLOffscreenTargetWin32::Startup((HWND)GetHandle(), screenResWidth, screenResHeight);
	}
	else
	{
		FxWarn("Hardware does not support WGL_ARB_pbuffer and WGL_ARB_pixel_format.");
	}
#endif // WIN32
	FxCompiledFaceGraph::SetPreDestroyCallback(FxOC3RenderWidgetPreDestroyCallbackFunc);
	FxCompiledFaceGraph::SetCompilationCallbacks(FxOC3RenderWidgetPreCompileCallbackFunc, NULL, NULL, NULL);
}

void FxRenderWidgetOC3::OnAppShutdown( FxWidget* FxUnused(sender) )
{
	_readyToDraw = FxFalse;
	// Need to get the actor from the session proxy in Studio because of the
	// order of widget messages.
#ifdef IS_FACEFX_STUDIO
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
#else
	//@todo compiled This might not be correct in this case.  Double check with
	//      the animation playback and live anim samples.
	FxActor* pActor = _actor;
#endif // IS_FACEFX_STUDIO

	if( pActor )
	{
		FxOC3RenderWidgetRemoveUserData(pActor->GetCompiledFaceGraph());
	}
#ifdef WIN32
	if( FxAreWGLExtensionsPresent() )
	{
		FxRendererOGLOffscreenTargetWin32::Shutdown();
	}
#endif // WIN32
	FxRenderer::Shutdown();
}

void FxRenderWidgetOC3::OnCreateActor( FxWidget* FxUnused(sender) )
{
	_readyToDraw = FxFalse;

#ifdef WIN32
	FxRendererOGLOffscreenTargetWin32::SetScene(NULL);
#endif // WIN32

	if( _renderWindow )
	{
		_renderWindow->SetScene(NULL);
	}

	if( _pScene )
	{
		delete _pScene;
		_pScene = NULL;
	}
}

void FxRenderWidgetOC3::OnLoadActor( FxWidget* FxUnused(sender), const FxString& actorPath )
{
	_readyToDraw = FxFalse;

#ifdef WIN32
	FxRendererOGLOffscreenTargetWin32::SetScene(NULL);
#endif // WIN32

	if( _renderWindow )
	{
		_renderWindow->SetScene(NULL);
	}

	if( _pScene )
	{
		delete _pScene;
		_pScene = NULL;
	}

	// The code below requires at least a three letter file extension.
	if( actorPath.Length() < 3 )
	{
		return;
	}

	FxBool generatedWarnings = FxFalse;

	// Load the scene from a scene archive.
	FxBool bLoadedScene = FxFalse;
	FxString sceneFilename = actorPath;
	sceneFilename[sceneFilename.Length()-3] = 'f';
	sceneFilename[sceneFilename.Length()-2] = 'x';
	sceneFilename[sceneFilename.Length()-1] = 'r';
	_fxrPath = sceneFilename;
	if( FxFileExists(sceneFilename) )
	{
		FxArchiveProgressDisplay::Begin(_("Loading scene..."), FxStudioApp::GetMainWindow());
		if( FxLoadSceneFromFile(_pScene, sceneFilename.GetData(), FxTrue, FxArchiveProgressDisplay::UpdateProgress) )
		{
			_setupDefaultLighting();
			bLoadedScene = FxTrue;
		}
		FxArchiveProgressDisplay::End();
	}
	if( !bLoadedScene )
	{
		FxWarn("Warning (.fxr file does not exist): failed to load %s.", sceneFilename.GetData());
		generatedWarnings = FxTrue;
	}
	if( _renderWindow )
	{
		// Explicitly tell the SetScene function to not render the scene.
		// This avoids a weird 'pop' if the FaceFX reference pose
		// does not match the skeleton's bind pose.  A render is forced
		// through the correct code path at the end of this function.
		_renderWindow->SetScene(_pScene, FxFalse);
	}

#ifdef WIN32
	FxRendererOGLOffscreenTargetWin32::SetScene(_pScene);
#endif // WIN32

	// Link up the bones.
	_linkupBones(generatedWarnings);

	// If there are morph targets in the .fxr file, make sure there are morph
	// target nodes in the actor's Face Graph.
	if( _pScene && _actor )
	{
		// Make a pass over the actor's Face Graph and convert the old-style
		// FxGenericTargetNodes to the new 1.5+ style FxMorphTargetNodes.
		FxBool autoConvertedToMorphTargetNodes = FxFalse;
		FxFaceGraph& faceGraph = _actor->GetDecompiledFaceGraph();
		FxSize numNodesToChangeTypeOf = faceGraph.CountNodes(FxGenericTargetNode::StaticClass());
		if( numNodesToChangeTypeOf > 0 )
		{
			FxArray<FxName> namesOfNodesToChangeTypeOf;
			namesOfNodesToChangeTypeOf.Reserve(numNodesToChangeTypeOf);
			FxFaceGraph::Iterator genericTargetNodeIter    = faceGraph.Begin(FxGenericTargetNode::StaticClass());
			FxFaceGraph::Iterator genericTargetNodeEndIter = faceGraph.End(FxGenericTargetNode::StaticClass());
			for( ; genericTargetNodeIter != genericTargetNodeEndIter; ++genericTargetNodeIter )
			{
				FxGenericTargetNode* pGenericTargetNode = FxCast<FxGenericTargetNode>(genericTargetNodeIter.GetNode());
				if( pGenericTargetNode )
				{
					namesOfNodesToChangeTypeOf.PushBack(pGenericTargetNode->GetName());
				}
			}
			FxMsg("Automatically converting <b>%d</b> old-style morph target node types to the new 1.5+ style type:", numNodesToChangeTypeOf);
			for( FxSize i = 0; i < numNodesToChangeTypeOf; ++i )
			{
				FxMsg("&nbsp;&nbsp;Converting morph target node %s...", namesOfNodesToChangeTypeOf[i].GetAsCstr());
				faceGraph.ChangeNodeType(namesOfNodesToChangeTypeOf[i], FxMorphTargetNode::StaticClass());
				FxFaceGraphNode* pNewNode = faceGraph.FindNode(namesOfNodesToChangeTypeOf[i]);
				if( pNewNode )
				{
					FxFaceGraphNodeUserProperty morphTargetNameUserProperty = pNewNode->GetUserProperty(FX_MORPH_TARGET_NODE_USERPROP_TARGET_NAME_INDEX);
					morphTargetNameUserProperty.SetStringProperty(namesOfNodesToChangeTypeOf[i].GetAsString());
					pNewNode->ReplaceUserProperty(morphTargetNameUserProperty);
				}
			}
			autoConvertedToMorphTargetNodes = FxTrue;
			FxMsg("Done.");
		}

		FxBool addedMorphTargetNodes = FxFalse;
		FxSize numMeshes = _pScene->GetNumMeshes();
		for( FxSize i = 0; i < numMeshes; ++i )
		{
			FxMesh* mesh = _pScene->GetMesh(i);
			if( mesh )
			{
				FxMorphController* morpher = mesh->GetMorphController();
				if( morpher )
				{
					FxSize numMorphTargets = morpher->GetNumTargets();
					for( FxSize j = 0; j < numMorphTargets; ++j )
					{
						FxMorphTarget& morphTarget = morpher->GetTarget(j);

						FxFaceGraphNode* pNode = _actor->GetDecompiledFaceGraph().FindNode(morphTarget.GetName());
						if( !pNode )
						{
							FxMorphTargetNode* pMorphTargetNode = new FxMorphTargetNode();
							pMorphTargetNode->SetName(morphTarget.GetName());
							pMorphTargetNode->SetUserData(new FxNodeUserData());
							FxFaceGraphNodeUserProperty morphTargetNameUserProperty = pMorphTargetNode->GetUserProperty(FX_MORPH_TARGET_NODE_USERPROP_TARGET_NAME_INDEX);
							morphTargetNameUserProperty.SetStringProperty(morphTarget.GetNameAsString());
							pMorphTargetNode->ReplaceUserProperty(morphTargetNameUserProperty);
							_actor->GetDecompiledFaceGraph().AddNode(pMorphTargetNode);
#ifdef IS_FACEFX_STUDIO
							FxSessionProxy::CompileFaceGraph();
#else
							_actor->CompileFaceGraph();
#endif
							if( _mediator )
							{
								_mediator->OnAddFaceGraphNode(this, pMorphTargetNode);
							}
							addedMorphTargetNodes = FxTrue;
							FxMsg("Added morph target node %s to the Face Graph.", morphTarget.GetNameAsCstr());
						}
						else
						{
							if( !pNode->IsKindOf(FxMorphTargetNode::StaticClass()) )
							{
								generatedWarnings = FxTrue;
								FxWarn("Warning:  morph target node %s could not be added to the Face Graph because a non-morph target node with the same name already exists.",
									morphTarget.GetNameAsCstr());
							}
						}
					}
				}
			}
		}

		// Add the managed cameras into the scene.
		OnActorInternalDataChanged(NULL, ADCF_CamerasChanged);

		if( addedMorphTargetNodes )
		{
#ifdef IS_FACEFX_STUDIO
			FxVM::ExecuteCommand("graph -layout");
#endif
			if( _mediator )
			{
				_mediator->OnActorInternalDataChanged(this, ADCF_FaceGraphChanged);
			}
#ifdef IS_FACEFX_STUDIO
			FxSessionProxy::SetIsForcedDirty(FxTrue);
			FxMessageBox(_("Morph target nodes were automatically added to the Face Graph for you.  Please check the console for details."), 
				         _("Added morph target nodes"));
#endif
		}

		if( autoConvertedToMorphTargetNodes )
		{
#ifdef IS_FACEFX_STUDIO
			FxSessionProxy::CompileFaceGraph();
#else
			_actor->CompileFaceGraph();
#endif
			FxMessageBox(_("Morph target nodes were automatically converted to the FaceFX 1.5+ type.  Please check the console for details."), 
						 _("Automatically converted morph target nodes"));
		}
	}

	if( generatedWarnings )
	{
		FxMessageBox(_("Warnings were generated while loading this actor.  Please check the console for details."), 
			         _("Generated warnings"));
	}


	_readyToDraw = FxTrue;

	// Force a render through our render widget's OnPaint function.
	_updateActor();
	Refresh(FALSE);
	Update();
	UpdateWindowUI();
}

void FxRenderWidgetOC3::OnCloseActor( FxWidget* FxUnused(sender) )
{
	_readyToDraw = FxFalse;

#ifdef WIN32
	FxRendererOGLOffscreenTargetWin32::SetScene(NULL);
#endif // WIN32

	if( _renderWindow )
	{
		_renderWindow->SetScene(NULL);
	}
	if( _pScene )
	{
		delete _pScene;
		_pScene = NULL;
	}

	if( _toolbarCameraChoice )
	{
		_toolbarCameraChoice->Freeze();
		_toolbarCameraChoice->Clear();
		_toolbarCameraChoice->Thaw();
	}
	
	UpdateWindowUI();
}

void FxRenderWidgetOC3::OnSaveActor( FxWidget* sender, const FxString& actorPath )
{
	if( _pScene )
	{
		FxString savePath = actorPath;
		savePath[savePath.Length()-3] = 'f';
		savePath[savePath.Length()-2] = 'x';
		savePath[savePath.Length()-1] = 'r';
		// If this is a "Save As" operation, save a new .fxr file so that it doesn't have
		// to be done manually.
		if( _fxrPath != savePath )
		{
			// Issue a warning to the log if the savePath file already exists
			// (it shouldn't) and it is read-only.
			if( FxFileIsReadOnly(savePath) )
			{
				FxWarn("%s is read-only!", savePath.GetData());
			}
			_readyToDraw = FxFalse;
			_fxrPath = savePath;
			// Remove all managed cameras from the scene first.
			for( FxSize i = 0; i < FxCameraManager::GetNumCameras(); ++i )
			{
				FxViewportCamera* pCamera = FxCameraManager::GetCamera(i);
				if( pCamera )
				{
					_pScene->RemoveCamera(pCamera->GetName());
				}
			}
			// Save the scene to a rendering archive.
			FxArchiveProgressDisplay::Begin(_("Saving Scene..."), FxStudioApp::GetMainWindow());
			FxSaveSceneToFile(_pScene, savePath.GetData(), FxArchive::ABO_LittleEndian, FxArchiveProgressDisplay::UpdateProgress);
			FxArchiveProgressDisplay::End();
			
			// Explicitly load the newly saved rendering archive to avoid
			// texture flipping that would be *really* hard to detect and 
			// solve down at the FxRenderer::FxTexture::Serialize() level.
			OnLoadActor(sender, actorPath);

			// Add the managed cameras back into the scene.
			OnActorInternalDataChanged(NULL, ADCF_CamerasChanged);
		}
	}
	if( _actor )
	{
		_readyToDraw = FxTrue;
	}
}

void FxRenderWidgetOC3::OnActorChanged( FxWidget* sender, FxActor* actor )
{
	if( _actor && !actor )
	{
		FxOC3RenderWidgetRemoveUserData(_actor->GetCompiledFaceGraph());
	}
	if( !actor )
	{
		_readyToDraw = FxFalse;
	}
	Super::OnActorChanged(sender, actor);
}

void FxRenderWidgetOC3::OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags )
{
	if( _actor && !_readyToDraw && flags & ADCF_FaceGraphChanged )
	{
		_readyToDraw = FxTrue;
	}
	if( _actor && _toolbarCameraChoice && flags & ADCF_CamerasChanged )
	{
		if( _pScene )
		{
			// Remove all managed cameras from the scene first.
			for( FxSize i = 0; i < FxCameraManager::GetNumCameras(); ++i )
			{
				FxViewportCamera* pCamera = FxCameraManager::GetCamera(i);
				if( pCamera )
				{
					_pScene->RemoveCamera(pCamera->GetName());
				}
			}
			// Add all newly updated managed cameras to the scene.
			for( FxSize i = 0; i < FxCameraManager::GetNumCameras(); ++i )
			{
				FxViewportCamera* pCamera = FxCameraManager::GetCamera(i);
				if( pCamera )
				{
					FxCamera* pNewSceneCamera = new FxCamera();
					pNewSceneCamera->SetName(pCamera->GetName());
					pNewSceneCamera->SetPosition(pCamera->GetBindPos());
					FxMatrix rotationMatrix(pCamera->GetBindRot());
					FxReal yaw, pitch, roll;
					rotationMatrix.ToEulerAnglesXYZ(yaw, pitch, roll);
					pNewSceneCamera->SetOrientation(FxOrientation(yaw, pitch, roll));
					pNewSceneCamera->SetUseFixedAspectRatio(pCamera->UsesFixedAspectRatio());
					pNewSceneCamera->SetFixedAspectRatioWidth(pCamera->GetFixedAspectRatioWidth());
					pNewSceneCamera->SetFixedAspectRatioHeight(pCamera->GetFixedAspectRatioHeight());
					_pScene->AddCamera(pNewSceneCamera);
				}
			}
		}
		// Add the newly updated managed cameras to the camera choice drop down.
		_toolbarCameraChoice->Freeze();
		_toolbarCameraChoice->Clear();
		for( FxSize i = 0; i < FxCameraManager::GetNumCameras(); ++i )
		{
			_toolbarCameraChoice->Append(wxString(FxCameraManager::GetCamera(i)->GetNameAsCstr(), wxConvLibc));
		}
		_toolbarCameraChoice->Thaw();
		if( _toolbarCameraChoice->GetCount() > 0 )
		{
			_toolbarCameraChoice->SetSelection(0);
		}
		// Force an update and render cycle to update any cameras that may be 
		// bound to bones in the skeleton.
		_updateActor();
		Refresh(FALSE);
		Update();
		UpdateWindowUI();
	}
	Super::OnActorInternalDataChanged(sender, flags);
}

void FxRenderWidgetOC3::OnTimeChanged( FxWidget* FxUnused(sender), FxReal FxUnused(newTime) )
{
	_updateActor();
	wxRect workRect;
	if( _containerWindow )
	{
		workRect = _containerWindow->GetRect();
	}
	Refresh(FALSE, &workRect);
	Update();
}

void FxRenderWidgetOC3::OnRefresh( FxWidget* FxUnused(sender) )
{
#ifdef IS_FACEFX_STUDIO
	// Check for rw_scalefactor change.
	FxReal scaleFactor = static_cast<FxReal>(*rw_scalefactor);
	if( !FxRealEquality(FxRenderer::GetUnitScaleFactor(), scaleFactor) )
	{
		FxRenderer::SetUnitScaleFactor(scaleFactor);
		_renderWindow->SetScene(_pScene, FxFalse);
#ifdef WIN32
		FxRendererOGLOffscreenTargetWin32::SetScene(_pScene);
#endif // WIN32
	}
#endif // IS_FACEFX_STUDIO
	_updateActor();
	wxRect workRect;
	if( _containerWindow )
	{
		workRect = _containerWindow->GetRect();
	}
	Refresh(FALSE, &workRect);
	Update();
}

void FxRenderWidgetOC3::
OnQueryRenderWidgetCaps( FxWidget* FxUnused(sender), FxRenderWidgetCaps& renderWidgetCaps )
{
	renderWidgetCaps.renderWidgetName = FxString("Internal OC3 Sample Render Widget");
	renderWidgetCaps.renderWidgetVersion = 1720;
	renderWidgetCaps.supportsOffscreenRenderTargets = FxTrue;
	renderWidgetCaps.supportsMultipleCameras = FxTrue;
	renderWidgetCaps.supportsFixedAspectRatioCameras = FxTrue;
	renderWidgetCaps.supportsBoneLockedCameras = FxTrue;
}

void FxRenderWidgetOC3::OnShowBindPoseUpdated( FxConsoleVariable& FxUnused(cvar) )
{
#ifdef IS_FACEFX_STUDIO
	// This causes all of the Studio controls to be refreshed, but only 
	// happens when the user explicitly sets the rw_showbindpose console
	// variable to true via the command panel.
	FxSessionProxy::RefreshControls();
#endif // IS_FACEFX_STUDIO
}

void FxRenderWidgetOC3::OnScaleFactorUpdated( FxConsoleVariable& FxUnused(cvar) )
{
#ifdef IS_FACEFX_STUDIO
	// This causes all of the Studio controls to be refreshed, but only 
	// happens when the user explicitly sets the rw_scalefactor console
	// variable to true via the command panel.
	FxSessionProxy::RefreshControls();
#endif // IS_FACEFX_STUDIO
}

void FxRenderWidgetOC3::SetScenePtr( FxScene* pScene )
{
	_readyToDraw = FxFalse;
	_pScene = pScene;
	_setupDefaultLighting();
	if( _renderWindow )
	{
		_renderWindow->SetScene(_pScene, FxFalse);
	}
	if( _pScene )
	{
		_readyToDraw = FxTrue;
	}
	// Force a render through our render widget's OnPaint function.
	_updateActor();
	Refresh(FALSE);
	Update();
	UpdateWindowUI();
}

FxScene* FxRenderWidgetOC3::GetScenePtr( void )
{
	return _pScene;
}

void FxRenderWidgetOC3::OnSize( wxSizeEvent& event )
{
	_resizeComponents();
	if( GetAutoLayout() )
	{
		Layout();
	}
	_refreshChildren();
	event.Skip();
}

void FxRenderWidgetOC3::OnPaint( wxPaintEvent& event )
{
	wxBufferedPaintDC dc(this);
	dc.Clear();
	DrawToolbar(&dc);
	if( _renderWindow )
	{
		_renderWindow->Render();
	}
	event.Skip();
}

void FxRenderWidgetOC3::OnSystemColoursChanged( wxSysColourChangedEvent& FxUnused(event) )
{
	_refreshSystemColors();
	Refresh(FALSE);
	Update();
}

void FxRenderWidgetOC3::OnResetViewButton( wxCommandEvent& FxUnused(event) )
{
	if( _renderWindow )
	{
		wxCommandEvent resetViewEvent;
		_renderWindow->OnResetView(resetViewEvent);
	}
}

void FxRenderWidgetOC3::OnToggleWireframeRenderingButton( wxCommandEvent& FxUnused(event) )
{
	if( _renderWindow )
	{
		wxKeyEvent keyEvent(wxEVT_KEY_DOWN);
		keyEvent.m_keyCode = 'W';
		_renderWindow->ProcessEvent(keyEvent);
	}
}

void FxRenderWidgetOC3::OnToggleLightingButton( wxCommandEvent& FxUnused(event) )
{
	if( _renderWindow )
	{
		wxKeyEvent keyEvent(wxEVT_KEY_DOWN);
		keyEvent.m_keyCode = 'L';
		_renderWindow->ProcessEvent(keyEvent);
	}
}

void FxRenderWidgetOC3::OnToggleMeshRenderingButton( wxCommandEvent& FxUnused(event) )
{
	if( _renderWindow )
	{
		wxKeyEvent keyEvent(wxEVT_KEY_DOWN);
		keyEvent.m_keyCode = 'M';
		_renderWindow->ProcessEvent(keyEvent);
	}
}

void FxRenderWidgetOC3::OnToggleNormalsRenderingButton( wxCommandEvent& FxUnused(event) )
{
	if( _renderWindow )
	{
		wxKeyEvent keyEvent(wxEVT_KEY_DOWN);
		keyEvent.m_keyCode = 'N';
		_renderWindow->ProcessEvent(keyEvent);
	}
}

void FxRenderWidgetOC3::OnToggleInfluencesRenderingButton( wxCommandEvent& FxUnused(event) )
{
	if( _renderWindow )
	{
		wxKeyEvent keyEvent(wxEVT_KEY_DOWN);
		keyEvent.m_keyCode = 'I';
		_renderWindow->ProcessEvent(keyEvent);
	}
}

void FxRenderWidgetOC3::OnToggleSkeletonRenderingButton( wxCommandEvent& FxUnused(event) )
{
	if( _renderWindow )
	{
		wxKeyEvent keyEvent(wxEVT_KEY_DOWN);
		keyEvent.m_keyCode = 'S';
		_renderWindow->ProcessEvent(keyEvent);
	}
}

void FxRenderWidgetOC3::OnCreateCameraButton( wxCommandEvent& FxUnused(event) )
{
	if( _toolbarCameraChoice && _pScene && _renderWindow )
	{
		// Get the name of the camera from the user.
		FxCreateCameraDialog* createCameraDialog = new FxCreateCameraDialog();
		FxString newCameraName;
		FxString boundBoneName;
		FxBool usesFixedAspectRatio;
		FxBool isBoundToBone;
		createCameraDialog->Initialize(_actor, &newCameraName, &usesFixedAspectRatio, &isBoundToBone, &boundBoneName);
		createCameraDialog->Create(this);
		if( wxID_OK == createCameraDialog->ShowModal() )
		{
			// Make sure that the actor is updated to the current state.
			_updateActor();

			FxName cameraName(newCameraName.GetData());
			// If the camera already exists, prompt the user to overwrite it.
			FxBool cameraAlreadyExisted = FxFalse;
			FxViewportCamera* pCamera = FxCameraManager::GetCamera(cameraName);
			if( pCamera )
			{
				cameraAlreadyExisted = FxTrue;
				if( wxNO == FxMessageBox(_("A camera with that name already exists.  Would you like to overwrite it?"), _("Duplicate Camera"), wxYES_NO|wxCENTRE, this) )
				{
					return;
				}
			}

			// Add the new camera to the manager.
			if( !pCamera )
			{
				FxViewportCamera newCamera;
				newCamera.SetName(cameraName);
				FxCameraManager::AddCamera(newCamera);
			}

			pCamera = FxCameraManager::GetCamera(cameraName);
			FxCamera* pSceneCamera = _renderWindow->GetCamera();
			if( pCamera && pSceneCamera )
			{
				// Set up the new camera assuming it will not be bound to a bone.
				pCamera->SetIsBoundToBone(FxFalse);
				pCamera->SetBoundBoneName(FxName::NullName);
				pCamera->SetUsesFixedAspectRatio(FxFalse);
				pCamera->SetBindPos(pSceneCamera->GetPosition());
				pCamera->SetBindRot(pSceneCamera->GetOrientation().ToQuat().Normalize());
				pCamera->SetPos(pCamera->GetBindPos());
				pCamera->SetRot(pCamera->GetBindRot());

				if( usesFixedAspectRatio )
				{
					pCamera->SetUsesFixedAspectRatio(FxTrue);
					pCamera->SetFixedAspectRatioWidth(pSceneCamera->GetWidth());
					pCamera->SetFixedAspectRatioHeight(pSceneCamera->GetHeight());
				}

				if( isBoundToBone )
				{
					// Make sure the user actually selected a bone name.
					if( boundBoneName.Length() > 0 )
					{
						if( _pScene->GetNumSkeletons() > 0 )
						{
							FxSkeleton* pSkeleton = _pScene->GetSkeleton(0);
							if( pSkeleton )
							{
								FxJoint* pJoint = pSkeleton->FindJoint(boundBoneName.GetData());
								if( pJoint )
								{
									// "Bind" the camera to the joint.
									pCamera->SetIsBoundToBone(FxTrue);
									pCamera->SetBoundBoneName(pJoint->GetName());
									FxMatrix cameraLocalMatrix;
									cameraLocalMatrix.Translate(pSceneCamera->GetPosition());
									cameraLocalMatrix = cameraLocalMatrix * pJoint->GetWorldMatrix().GetInverse();
									FxVec3 cameraBindPos(cameraLocalMatrix.GetTranslation());
									pCamera->SetBindPos(cameraBindPos);
								}
							}
						}
					}
					else
					{
						FxMessageBox(_("You selected to bind the camera to a bone but failed to specify which bone."), _("Camera Not Bound To Bone"));
					}
				}

				// Add a new camera to the scene if it was not already there.
				if( !cameraAlreadyExisted )
				{
					FxCamera* pNewSceneCamera = new FxCamera(*pSceneCamera);
					pNewSceneCamera->SetName(cameraName);
					pNewSceneCamera->SetUseFixedAspectRatio(pCamera->UsesFixedAspectRatio());
					pNewSceneCamera->SetFixedAspectRatioWidth(pCamera->GetFixedAspectRatioWidth());
					pNewSceneCamera->SetFixedAspectRatioHeight(pCamera->GetFixedAspectRatioHeight());
					_pScene->AddCamera(pNewSceneCamera);
				}
				else
				{
					// The camera already existed in the scene.
					FxCamera* pOldSceneCamera = _pScene->FindCamera(cameraName);
					if( pOldSceneCamera )
					{
						pOldSceneCamera->SetUseFixedAspectRatio(pCamera->UsesFixedAspectRatio());
						pOldSceneCamera->SetFixedAspectRatioWidth(pCamera->GetFixedAspectRatioWidth());
						pOldSceneCamera->SetFixedAspectRatioHeight(pCamera->GetFixedAspectRatioHeight());

						if( !pCamera->IsBoundToBone() )
						{
							// If the camera already existed and is not "bound" to
							// a bone, it is possible that it was "bound" last time,
							// so make sure that the camera in the scene matches
							// the current state of the camera.
							pOldSceneCamera->SetPosition(pCamera->GetPos());
							FxMatrix rotationMatrix(pCamera->GetRot());
							FxReal yaw, pitch, roll;
							rotationMatrix.ToEulerAnglesXYZ(yaw, pitch, roll);
							pOldSceneCamera->SetOrientation(FxOrientation(yaw, pitch, roll));
						}
					}
				}

                // Update any cameras that are "bound" to bones.
				_updateBoundCameras();
			}
		}

		_toolbarCameraChoice->Freeze();
		_toolbarCameraChoice->Clear();
		for( FxSize i = 0; i < FxCameraManager::GetNumCameras(); ++i )
		{
			_toolbarCameraChoice->Append(wxString(FxCameraManager::GetCamera(i)->GetNameAsCstr(), wxConvLibc));
		}
		_toolbarCameraChoice->Thaw();
		FxInt32 index = _toolbarCameraChoice->FindString(wxString(newCameraName.GetData(), wxConvLibc));
		if( index != -1 )
		{
			_toolbarCameraChoice->SetSelection(index);
		}
#ifdef IS_FACEFX_STUDIO
		FxSessionProxy::SetIsForcedDirty(FxTrue);
#endif
		if( _mediator )
		{
			_mediator->OnRefresh(this);
		}
		UpdateWindowUI();
	}
}

void FxRenderWidgetOC3::OnDeleteCameraButton( wxCommandEvent& FxUnused(event) )
{
	if( _toolbarCameraChoice && _pScene && _renderWindow )
	{
		FxInt32 selection = _toolbarCameraChoice->GetSelection();
		if( selection != -1 )
		{
			wxString cameraNameString = _toolbarCameraChoice->GetString(selection);
			FxName cameraName(FxString(cameraNameString.mb_str(wxConvLibc)).GetData());
			FxCameraManager::RemoveCamera(cameraName);
			_pScene->RemoveCamera(cameraName);
			
			_toolbarCameraChoice->Freeze();
			_toolbarCameraChoice->Clear();
			for( FxSize i = 0; i < FxCameraManager::GetNumCameras(); ++i )
			{
				_toolbarCameraChoice->Append(wxString(FxCameraManager::GetCamera(i)->GetNameAsCstr(), wxConvLibc));
			}
			_toolbarCameraChoice->Thaw();
			if( _toolbarCameraChoice->GetCount() > 0 )
			{
				_toolbarCameraChoice->SetSelection(0);
			}
			wxCommandEvent dummySelectionEvent;
			OnCameraChoiceChange(dummySelectionEvent);
#ifdef IS_FACEFX_STUDIO
			FxSessionProxy::SetIsForcedDirty(FxTrue);
#endif
			if( _mediator )
			{
				_mediator->OnRefresh(this);
			}
		}
		UpdateWindowUI();
	}
}

void FxRenderWidgetOC3::OnCameraChoiceChange( wxCommandEvent& FxUnused(event) )
{
	if( _toolbarCameraChoice && _pScene && _renderWindow )
	{
		FxInt32 selection = _toolbarCameraChoice->GetSelection();
		if( selection != -1 )
		{
			// Make sure the actor is updated to the current state.
			_updateActor();

			wxString cameraNameString = _toolbarCameraChoice->GetString(selection);
			FxName cameraName(FxString(cameraNameString.mb_str(wxConvLibc)).GetData());
			FxViewportCamera* pCamera = FxCameraManager::GetCamera(cameraName);
			FxCamera* pSceneCamera = _renderWindow->GetCamera();
			if( pCamera && pSceneCamera )
			{
				pSceneCamera->SetPosition(pCamera->GetPos());
				FxMatrix rotationMatrix(pCamera->GetRot());
				FxReal yaw, pitch, roll;
				rotationMatrix.ToEulerAnglesXYZ(yaw, pitch, roll);
				pSceneCamera->SetOrientation(FxOrientation(yaw, pitch, roll));
				Refresh(FALSE);
				Update();
			}
		}
	}
}

void FxRenderWidgetOC3::UpdateToolbarState( wxUpdateUIEvent& FxUnused(event) )
{
	if( _renderWindow                           &&
		_toolbarResetViewButton                 &&
		_toolbarToggleWireframeRenderingButton  &&
		_toolbarToggleLightingButton            && 
		_toolbarToggleMeshRenderingButton       &&
		_toolbarToggleNormalsRenderingButton    && 
		_toolbarToggleInfluencesRenderingButton &&
		_toolbarToggleSkeletonRenderingButton   &&
		_toolbarCreateCameraButton              &&
		_toolbarDeleteCameraButton              &&
		_toolbarCameraChoice )
	{
		FxScene* pScene = _renderWindow->GetScene();
		if( pScene )
		{
			_toolbarResetViewButton->Enable(FxTrue);
			_toolbarToggleWireframeRenderingButton->Enable(FxTrue);
			_toolbarToggleLightingButton->Enable(FxTrue);
			_toolbarToggleMeshRenderingButton->Enable(FxTrue);
			_toolbarToggleNormalsRenderingButton->Enable(FxTrue); 
			_toolbarToggleInfluencesRenderingButton->Enable(FxTrue);
			_toolbarToggleSkeletonRenderingButton->Enable(FxTrue);
			_toolbarCreateCameraButton->Enable(FxTrue);
			if( FxCameraManager::GetNumCameras() > 0 )
			{
				_toolbarDeleteCameraButton->Enable(FxTrue);
				_toolbarCameraChoice->Enable(FxTrue);
			}
			else
			{
				_toolbarDeleteCameraButton->Enable(FxFalse);
				_toolbarCameraChoice->Enable(FxFalse);
			}
			
            if( pScene->WireframeEnabled() )
			{
				_toolbarToggleWireframeRenderingButton->SetValue(FxTrue);
			}
			else
			{
				_toolbarToggleWireframeRenderingButton->SetValue(FxFalse);
			}
			if( pScene->LightingEnabled() )
			{
				_toolbarToggleLightingButton->SetValue(FxTrue);
			}
			else
			{
				_toolbarToggleLightingButton->SetValue(FxFalse);
			}
			if( pScene->DrawMeshesEnabled() )
			{
				_toolbarToggleMeshRenderingButton->SetValue(FxTrue);
			}
			else
			{
				_toolbarToggleMeshRenderingButton->SetValue(FxFalse);
			}
			if( pScene->DrawNormalsEnabled() )
			{
				_toolbarToggleNormalsRenderingButton->SetValue(FxTrue);
			}
			else
			{
				_toolbarToggleNormalsRenderingButton->SetValue(FxFalse);
			}
			if( pScene->DrawWeightsEnabled() )
			{
				_toolbarToggleInfluencesRenderingButton->SetValue(FxTrue);
			}
			else
			{
				_toolbarToggleInfluencesRenderingButton->SetValue(FxFalse);
			}
			if( pScene->DrawSkeletonsEnabled() )
			{
				_toolbarToggleSkeletonRenderingButton->SetValue(FxTrue);
			}
			else
			{
				_toolbarToggleSkeletonRenderingButton->SetValue(FxFalse);
			}
		}
		else
		{
			_toolbarResetViewButton->Enable(FxFalse);
			_toolbarToggleWireframeRenderingButton->Enable(FxFalse);
			_toolbarToggleLightingButton->Enable(FxFalse);
			_toolbarToggleMeshRenderingButton->Enable(FxFalse);
			_toolbarToggleNormalsRenderingButton->Enable(FxFalse); 
			_toolbarToggleInfluencesRenderingButton->Enable(FxFalse);
			_toolbarToggleSkeletonRenderingButton->Enable(FxFalse);
			_toolbarCreateCameraButton->Enable(FxFalse);
			_toolbarDeleteCameraButton->Enable(FxFalse);
			_toolbarCameraChoice->Enable(FxFalse);
		}
	}
}

void FxRenderWidgetOC3::DrawToolbar( wxDC* pDC )
{
	// Draw the toolbar background.
	pDC->SetBrush(wxBrush(_colourFace));
	pDC->SetPen(wxPen(_colourFace));
	pDC->DrawRectangle(_toolbarRect);

	// Shade the edges.
	pDC->SetPen(wxPen(_colourShadow));
	pDC->DrawLine(_toolbarRect.GetLeft(), _toolbarRect.GetBottom(),
		_toolbarRect.GetRight(), _toolbarRect.GetBottom());

	// Clean up.
	pDC->SetBrush(wxNullBrush);
	pDC->SetPen(wxNullPen);
}

void FxRenderWidgetOC3::_refreshSystemColors( void )
{
	_colourFace = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	_colourShadow = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
}

void FxRenderWidgetOC3::_refreshChildren( void )
{
	wxWindowList& children = GetChildren();
	wxWindowListNode* node = children.GetFirst();
	while( node )
	{
		node->GetData()->Refresh();
		node = node->GetNext();
	}
}

void FxRenderWidgetOC3::_resizeComponents( void )
{
	wxRect clientRect(GetClientRect());
	_toolbarRect = wxRect(clientRect.GetLeft(), clientRect.GetTop(),
		clientRect.GetWidth(), FxToolbarHeight);
	wxRect containerWindowRect(clientRect.GetLeft(), clientRect.GetTop() + FxToolbarHeight,
		clientRect.GetWidth(), clientRect.GetHeight() - FxToolbarHeight);

	if( _containerWindow )
	{
		_containerWindow->SetSize(containerWindowRect);
	}

	Refresh(FALSE);
	Update();
}

void FxRenderWidgetOC3::_linkupBones( FxBool& generatedWarnings )
{
	// Set the bone client indices in the master bone list of the actor.
	if( _pScene && _actor )
	{
		if( _pScene->GetNumSkeletons() > 0 )
		{
			FxSkeleton* skeleton = _pScene->GetSkeleton(0);
			if( skeleton )
			{
				FxSize numRefBones = _actor->GetMasterBoneList().GetNumRefBones();
				for( FxSize i = 0; i < numRefBones; ++i )
				{
					FxBool found = FxFalse;
					FxSize numJoints = skeleton->GetNumJoints();
					for( FxSize j = 0; j < numJoints; ++j )
					{
						FxJoint* joint = skeleton->GetJoint(j);
						if( joint )
						{
							if( joint->GetName() == _actor->GetMasterBoneList().GetRefBone(i).GetName() )
							{
								_actor->GetMasterBoneList().SetRefBoneClientIndex(i, j);
								found = FxTrue;
							}
						}
					}
					if( !found )
					{
						FxWarn("Warning (.fxa -> .fxr mismatch):  refBone %s could not be found in the skeleton.", 
							_actor->GetMasterBoneList().GetRefBone(i).GetNameAsCstr());
							_actor->GetMasterBoneList().SetRefBoneClientIndex(i, FX_INT32_MAX);
						generatedWarnings = FxTrue;
					}
				}
			}
		}
	}
}

void FxRenderWidgetOC3::_updateActor( void )
{
	if( _readyToDraw && _actor && _pScene )
	{
		if( _actor->ShouldClientRelink() )
		{
			FxBool generatedWarnings = FxFalse;
			_linkupBones(generatedWarnings);
			_actor->SetShouldClientRelink(FxFalse);
		}
		// Update any morph target nodes that may be in the Face Graph.
		FxCompiledFaceGraph& cg = _actor->GetCompiledFaceGraph();
		FxSize numNodes = cg.nodes.Length();
		for( FxSize i = 0; i < numNodes; ++i )
		{
			if( NT_MorphTarget == cg.nodes[i].nodeType )
			{
				// If the user data is invalid, create it.
				if( !cg.nodes[i].pUserData )
				{
					cg.nodes[i].pUserData = new FxMorphTargetUserData();
					// Link it up.
					const FxFaceGraphNodeUserProperty& morphTargetNameUserProperty = 
						cg.nodes[i].userProperties[FX_MORPH_TARGET_NODE_USERPROP_TARGET_NAME_INDEX];
					reinterpret_cast<FxMorphTargetUserData*>(cg.nodes[i].pUserData)->SetMorphTarget(
						_pScene, morphTargetNameUserProperty.GetStringProperty().GetData());
				}
				// Update the morph target.
				FxMorphTarget* pMorphTarget = reinterpret_cast<FxMorphTargetUserData*>(cg.nodes[i].pUserData)->pMorphTarget;
				if( pMorphTarget )
				{
					pMorphTarget->SetWeight(cg.nodes[i].finalValue);
				}
			}
		}
		// Continue with any other Face Graph processing and bone blending.

		// Blend in any bones that may be active in the Face Graph.
		FxSkeleton* pSkeleton = NULL;
		if( _pScene->GetNumSkeletons() > 0 )
		{
			pSkeleton = _pScene->GetSkeleton(0);
			if( pSkeleton )
			{
#ifdef IS_FACEFX_STUDIO
				FxBool showBindPose = static_cast<FxBool>(*rw_showbindpose);
#else
				FxBool showBindPose = FxFalse;
#endif
				if( showBindPose )
				{
					pSkeleton->SetBindPose();
				}
				else
				{
					FxVec3 bonePos, boneScale;
					FxQuat boneRot;
					FxReal boneWeight;
					FxMasterBoneList& masterBoneList = _actor->GetMasterBoneList();
					FxSize numRefBones = masterBoneList.GetNumRefBones();
					for( FxSize i = 0; i < numRefBones; ++i )
					{
						masterBoneList.GetBlendedBone(i, cg, bonePos, boneRot, boneScale, boneWeight);
						FxInt32 clientIndex = masterBoneList.GetRefBoneClientIndex(i);
						if( FX_INT32_MAX != clientIndex )
						{
							FxJoint* joint = pSkeleton->GetJoint(clientIndex);
							if( joint )
							{
								// Note that this isn't exactly how you would want to use
								// blendedBone's weight member in your engine.  The FxRenderer
								// sample engine does not support playing back an underlying
								// skeletal animation, so in the case of your engine, you would
								// want to do the blend a little differently so that only
								// blendedBone's weight portion of the final blend comes from
								// FaceFX' blendedBone.
								FxVec3 finalPos   = joint->GetBindPos();
								FxQuat finalRot   = joint->GetBindRot();
								FxVec3 finalScale = joint->GetBindScale();
								finalPos += finalPos.Lerp(bonePos, boneWeight);
								FxQuat rhs = finalRot.Slerp(boneRot, boneWeight);
								finalRot = finalRot * joint->GetBindRot().GetInverse() * rhs;
								// Not strictly required, but can't hurt.
								finalRot.Normalize();
								finalScale += finalScale.Lerp(boneScale, boneWeight);
								joint->SetPos(finalPos);
								joint->SetRot(finalRot);
								joint->SetScale(finalScale);
							}
						}
					}
				}
			}
		}
	}
	if( _readyToDraw && _pScene )
	{
		// Tick the scene to update the skeleton and morph and skin the vertices.
		_pScene->Tick();
		// Update "bound" cameras.
		_updateBoundCameras();
	}
}

void FxRenderWidgetOC3::_updateBoundCameras( void )
{
	if( _readyToDraw && _pScene )
	{
		FxSkeleton* pSkeleton = NULL;
		if( _pScene->GetNumSkeletons() > 0 )
		{
			pSkeleton = _pScene->GetSkeleton(0);
			if( pSkeleton )
			{
				for( FxSize i = 0; i < FxCameraManager::GetNumCameras(); ++i )
				{
					FxViewportCamera* pCamera = FxCameraManager::GetCamera(i);
					FxCamera* pSceneCamera = _pScene->FindCamera(pCamera->GetName());
					if( pCamera && pSceneCamera )
					{
						if( pCamera->IsBoundToBone() )
						{
							const FxName& boneName = pCamera->GetBoundBoneName();
							FxJoint* pJoint = pSkeleton->FindJoint(boneName);
							if( pJoint )
							{
								FxReal yaw, pitch, roll;
								pJoint->GetWorldMatrix().ToEulerAnglesXYZ(yaw, pitch, roll);
								FxMatrix jointRotationMatrix;
								jointRotationMatrix.YawRotate(yaw);
								jointRotationMatrix.PitchRotate(pitch);
								jointRotationMatrix.RollRotate(roll);
								FxMatrix cameraRotationMatrix(pCamera->GetBindRot());
								cameraRotationMatrix = cameraRotationMatrix * jointRotationMatrix.GetInverse();
								cameraRotationMatrix.ToEulerAnglesXYZ(yaw, pitch, roll);
								pSceneCamera->SetOrientation(FxOrientation(yaw, pitch, roll));
								pCamera->SetRot(pSceneCamera->GetOrientation().ToQuat().Normalize());

								FxMatrix cameraWorldMatrix;
								cameraWorldMatrix.Translate(pCamera->GetBindPos());
								cameraWorldMatrix = cameraWorldMatrix * pJoint->GetWorldMatrix();
								pSceneCamera->SetPosition(cameraWorldMatrix.GetTranslation());
								pCamera->SetPos(pSceneCamera->GetPosition());
							}
						}
					}
				}
			}
		}
	}
}

void FxRenderWidgetOC3::_setupDefaultLighting( void )
{
	if( _pScene )
	{
		if( 0 == _pScene->GetNumLights() )
		{
			FxLight* light = new FxLight();
			light->SetDiffuse(FxRGBA(1.0f, 1.0f, 1.0f));
			light->SetPosition(FxVec3(0.0f, 0.0f, 2000.0f));
			_pScene->AddLight(light);

			FxLight* light2 = new FxLight();
			light2->SetDiffuse(FxRGBA(1.0f, 1.0f, 1.0f));
			light2->SetPosition(FxVec3(0.0f, 0.0f, -2000.0f));
			_pScene->AddLight(light2);
		}
	}
}

void FxRenderWidgetOC3::_loadIcons( void )
{
	if( _toolbarCreateCameraButton && _toolbarDeleteCameraButton )
	{
		wxString resetviewPath = FxStudioApp::GetIconPath(wxT("resetview.ico"));
		wxString resetviewDisabledPath = FxStudioApp::GetIconPath(wxT("resetview_disabled.ico"));
		if( FxFileExists(resetviewPath) && FxFileExists(resetviewDisabledPath) )
		{
			wxIcon resetviewIcon = wxIcon(wxIconLocation(resetviewPath));
			resetviewIcon.SetHeight(16);
			resetviewIcon.SetWidth(16);
			wxIcon resetviewDisabledIcon = wxIcon(wxIconLocation(resetviewDisabledPath));
			resetviewDisabledIcon.SetHeight(16);
			resetviewDisabledIcon.SetWidth(16);
			_toolbarResetViewButton->SetEnabledBitmap(resetviewIcon);
			_toolbarResetViewButton->SetDisabledBitmap(resetviewDisabledIcon);
		}

		wxString togglewireframePath = FxStudioApp::GetIconPath(wxT("togglewireframe.ico"));
		wxString togglewireframeDisabledPath = FxStudioApp::GetIconPath(wxT("togglewireframe_disabled.ico"));
		if( FxFileExists(togglewireframePath) && FxFileExists(togglewireframeDisabledPath) )
		{
			wxIcon togglewireframeIcon = wxIcon(wxIconLocation(togglewireframePath));
			togglewireframeIcon.SetHeight(16);
			togglewireframeIcon.SetWidth(16);
			wxIcon togglewireframeDisabledIcon = wxIcon(wxIconLocation(togglewireframeDisabledPath));
			togglewireframeDisabledIcon.SetHeight(16);
			togglewireframeDisabledIcon.SetWidth(16);
			_toolbarToggleWireframeRenderingButton->SetEnabledBitmap(togglewireframeIcon);
			_toolbarToggleWireframeRenderingButton->SetDisabledBitmap(togglewireframeDisabledIcon);
		}

		wxString togglelightingPath = FxStudioApp::GetIconPath(wxT("togglelighting.ico"));
		wxString togglelightingDisabledPath = FxStudioApp::GetIconPath(wxT("togglelighting_disabled.ico"));
		if( FxFileExists(togglelightingPath) && FxFileExists(togglelightingDisabledPath) )
		{
			wxIcon togglelightingIcon = wxIcon(wxIconLocation(togglelightingPath));
			togglelightingIcon.SetHeight(16);
			togglelightingIcon.SetWidth(16);
			wxIcon togglelightingDisabledIcon = wxIcon(wxIconLocation(togglelightingDisabledPath));
			togglelightingDisabledIcon.SetHeight(16);
			togglelightingDisabledIcon.SetWidth(16);
			_toolbarToggleLightingButton->SetEnabledBitmap(togglelightingIcon);
			_toolbarToggleLightingButton->SetDisabledBitmap(togglelightingDisabledIcon);
		}

		wxString togglemeshPath = FxStudioApp::GetIconPath(wxT("togglemesh.ico"));
		wxString togglemeshDisabledPath = FxStudioApp::GetIconPath(wxT("togglemesh_disabled.ico"));
		if( FxFileExists(togglemeshPath) && FxFileExists(togglemeshDisabledPath) )
		{
			wxIcon togglemeshIcon = wxIcon(wxIconLocation(togglemeshPath));
			togglemeshIcon.SetHeight(16);
			togglemeshIcon.SetWidth(16);
			wxIcon togglemeshDisabledIcon = wxIcon(wxIconLocation(togglemeshDisabledPath));
			togglemeshDisabledIcon.SetHeight(16);
			togglemeshDisabledIcon.SetWidth(16);
			_toolbarToggleMeshRenderingButton->SetEnabledBitmap(togglemeshIcon);
			_toolbarToggleMeshRenderingButton->SetDisabledBitmap(togglemeshDisabledIcon);
		}

		wxString togglenormalsPath = FxStudioApp::GetIconPath(wxT("togglenormals.ico"));
		wxString togglenormalsDisabledPath = FxStudioApp::GetIconPath(wxT("togglenormals_disabled.ico"));
		if( FxFileExists(togglenormalsPath) && FxFileExists(togglenormalsDisabledPath) )
		{
			wxIcon togglenormalsIcon = wxIcon(wxIconLocation(togglenormalsPath));
			togglenormalsIcon.SetHeight(16);
			togglenormalsIcon.SetWidth(16);
			wxIcon togglenormalsDisabledIcon = wxIcon(wxIconLocation(togglenormalsDisabledPath));
			togglenormalsDisabledIcon.SetHeight(16);
			togglenormalsDisabledIcon.SetWidth(16);
			_toolbarToggleNormalsRenderingButton->SetEnabledBitmap(togglenormalsIcon);
			_toolbarToggleNormalsRenderingButton->SetDisabledBitmap(togglenormalsDisabledIcon);
		}

		wxString toggleinfluencesPath = FxStudioApp::GetIconPath(wxT("toggleinfluences.ico"));
		wxString toggleinfluencesDisabledPath = FxStudioApp::GetIconPath(wxT("toggleinfluences_disabled.ico"));
		if( FxFileExists(toggleinfluencesPath) && FxFileExists(toggleinfluencesDisabledPath) )
		{
			wxIcon toggleinfluencesIcon = wxIcon(wxIconLocation(toggleinfluencesPath));
			toggleinfluencesIcon.SetHeight(16);
			toggleinfluencesIcon.SetWidth(16);
			wxIcon toggleinfluencesDisabledIcon = wxIcon(wxIconLocation(toggleinfluencesDisabledPath));
			toggleinfluencesDisabledIcon.SetHeight(16);
			toggleinfluencesDisabledIcon.SetWidth(16);
			_toolbarToggleInfluencesRenderingButton->SetEnabledBitmap(toggleinfluencesIcon);
			_toolbarToggleInfluencesRenderingButton->SetDisabledBitmap(toggleinfluencesDisabledIcon);
		}

		wxString toggleskeletonPath = FxStudioApp::GetIconPath(wxT("toggleskeleton.ico"));
		wxString toggleskeletonDisabledPath = FxStudioApp::GetIconPath(wxT("toggleskeleton_disabled.ico"));
		if( FxFileExists(toggleskeletonPath) && FxFileExists(toggleskeletonDisabledPath) )
		{
			wxIcon toggleskeletonIcon = wxIcon(wxIconLocation(toggleskeletonPath));
			toggleskeletonIcon.SetHeight(16);
			toggleskeletonIcon.SetWidth(16);
			wxIcon toggleskeletonDisabledIcon = wxIcon(wxIconLocation(toggleskeletonDisabledPath));
			toggleskeletonDisabledIcon.SetHeight(16);
			toggleskeletonDisabledIcon.SetWidth(16);
			_toolbarToggleSkeletonRenderingButton->SetEnabledBitmap(toggleskeletonIcon);
			_toolbarToggleSkeletonRenderingButton->SetDisabledBitmap(toggleskeletonDisabledIcon);
		}

		wxString createcameraPath = FxStudioApp::GetIconPath(wxT("createcamera.ico"));
		wxString createcameraDisabledPath = FxStudioApp::GetIconPath(wxT("createcamera_disabled.ico"));
		if( FxFileExists(createcameraPath) && FxFileExists(createcameraDisabledPath) )
		{
			wxIcon createcameraIcon = wxIcon(wxIconLocation(createcameraPath));
			createcameraIcon.SetHeight(16);
			createcameraIcon.SetWidth(16);
			wxIcon createcameraDisabledIcon = wxIcon(wxIconLocation(createcameraDisabledPath));
			createcameraDisabledIcon.SetHeight(16);
			createcameraDisabledIcon.SetWidth(16);
			_toolbarCreateCameraButton->SetEnabledBitmap(createcameraIcon);
			_toolbarCreateCameraButton->SetDisabledBitmap(createcameraDisabledIcon);
		}

		wxString deletecameraPath = FxStudioApp::GetIconPath(wxT("deletecamera.ico"));
		wxString deletecameraDisabledPath = FxStudioApp::GetIconPath(wxT("deletecamera_disabled.ico"));
		if( FxFileExists(deletecameraPath) && FxFileExists(deletecameraDisabledPath) )
		{
			wxIcon deletecameraIcon = wxIcon(wxIconLocation(deletecameraPath));
			deletecameraIcon.SetHeight(16);
			deletecameraIcon.SetWidth(16);
			wxIcon deletecameraDisabledIcon = wxIcon(wxIconLocation(deletecameraDisabledPath));
			deletecameraDisabledIcon.SetHeight(16);
			deletecameraDisabledIcon.SetWidth(16);
			_toolbarDeleteCameraButton->SetEnabledBitmap(deletecameraIcon);
			_toolbarDeleteCameraButton->SetDisabledBitmap(deletecameraDisabledIcon);
		}
	}
}

} // namespace Face

} // namespace OC3Ent

#endif // USE_FX_RENDERER
