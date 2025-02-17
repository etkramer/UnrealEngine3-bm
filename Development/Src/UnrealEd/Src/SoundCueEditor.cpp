/*=============================================================================
	SoundCueEditor.cpp: SoundCue editing
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "UnLinkedObjEditor.h"
#include "EngineSoundClasses.h"
#include "UnLinkedObjDrawUtils.h"
#include "SoundCueEditor.h"
#include "Properties.h"

IMPLEMENT_CLASS(USoundNodeHelper)

void WxSoundCueEditor::OpenNewObjectMenu()
{
	WxMBSoundCueEdNewNode menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

void WxSoundCueEditor::OpenObjectOptionsMenu()
{
	WxMBSoundCueEdNodeOptions menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

void WxSoundCueEditor::OpenConnectorOptionsMenu()
{
	WxMBSoundCueEdConnectorOptions menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

void WxSoundCueEditor::DrawObjects(FViewport* Viewport, FCanvas* Canvas)
{
	WxLinkedObjEd::DrawObjects( Viewport, Canvas );
	SoundCue->DrawCue(Canvas, SelectedNodes);
}

void WxSoundCueEditor::UpdatePropertyWindow()
{
	if( SelectedNodes.Num() )
	{
		PropertyWindow->SetObjectArray( SelectedNodes, TRUE, TRUE, FALSE );
	}
	else
	{
		PropertyWindow->SetObject( SoundCue, TRUE, TRUE, FALSE );
	}

	PropertyWindow->ExpandAllItems();
}

void WxSoundCueEditor::EmptySelection()
{
	SelectedNodes.Empty();
}

void WxSoundCueEditor::AddToSelection( UObject* Obj )
{
	check( Obj->IsA( USoundNode::StaticClass() ) );

	if( SelectedNodes.ContainsItem( ( USoundNode *)Obj ) )
	{
		return;
	}

	SelectedNodes.AddItem( (USoundNode*)Obj );
}

UBOOL WxSoundCueEditor::IsInSelection( UObject* Obj ) const
{
	check( Obj->IsA( USoundNode::StaticClass() ) );

	return SelectedNodes.ContainsItem( ( USoundNode * )Obj );
}

INT WxSoundCueEditor::GetNumSelected() const
{
	return SelectedNodes.Num();
}

void WxSoundCueEditor::SetSelectedConnector( FLinkedObjectConnector& Connector )
{
	check( Connector.ConnObj->IsA( USoundCue::StaticClass() ) || Connector.ConnObj->IsA( USoundNode::StaticClass() ) );

	ConnObj = Connector.ConnObj;
	ConnType = Connector.ConnType;
	ConnIndex = Connector.ConnIndex;
}

FIntPoint WxSoundCueEditor::GetSelectedConnLocation( FCanvas* Canvas )
{
	// If no ConnNode, return origin. This works in the case of connecting a node to the 'root'.
	if( !ConnObj )
	{
		return FIntPoint( 0, 0 );
	}

	// Special case of connection from 'root' connector.
	if( ConnObj == SoundCue )
	{
		return FIntPoint( 0, 0 );
	}

	USoundNode* ConnNode = CastChecked<USoundNode>( ConnObj );

	FSoundNodeEditorData* ConnNodeEdData = SoundCue->EditorData.Find( ConnNode );
	check( ConnNodeEdData );

	return ConnNode->GetConnectionLocation( Canvas, ConnType, ConnIndex, *ConnNodeEdData );
}

void WxSoundCueEditor::MakeConnectionToConnector( FLinkedObjectConnector& Connector )
{
	// Avoid connections to yourself.
	if( Connector.ConnObj == ConnObj )
	{
		return;
	}

	// Handle special case of connecting a node to the 'root'.
	if( Connector.ConnObj == SoundCue )
	{
		if( ConnType == LOC_OUTPUT )
		{
			check( Connector.ConnType == LOC_INPUT );
			check( Connector.ConnIndex == 0 );

			USoundNode* ConnNode = CastChecked<USoundNode>( ConnObj );
			SoundCue->FirstNode = ConnNode;

			SoundCue->PostEditChange( NULL );
		}
		return;
	}
	else if( ConnObj == SoundCue )
	{
		if( Connector.ConnType == LOC_OUTPUT )
		{
			check( ConnType == LOC_INPUT );
			check( ConnIndex == 0 );

			USoundNode* EndConnNode = CastChecked<USoundNode>( Connector.ConnObj );
			SoundCue->FirstNode = EndConnNode;

			SoundCue->PostEditChange( NULL );
		}
		return;
	}

	// Normal case - connecting an input of one node to the output of another.
	// JTODO: Loop detection!
	if( ConnType == LOC_INPUT && Connector.ConnType == LOC_OUTPUT )
	{
		check( Connector.ConnIndex == 0 );

		USoundNode* ConnNode = CastChecked<USoundNode>( ConnObj );
		USoundNode* EndConnNode = CastChecked<USoundNode>( Connector.ConnObj );
		ConnectNodes( ConnNode, ConnIndex, EndConnNode );
	}
	else if( ConnType == LOC_OUTPUT && Connector.ConnType == LOC_INPUT )
	{
		check( ConnIndex == 0 );

		USoundNode* ConnNode = CastChecked<USoundNode>( ConnObj );
		USoundNode* EndConnNode = CastChecked<USoundNode>( Connector.ConnObj );
		ConnectNodes( EndConnNode, Connector.ConnIndex, ConnNode );
	}
}

void WxSoundCueEditor::MakeConnectionToObject( UObject* EndObj )
{
}

/**
 * Called when the user releases the mouse over a link connector and is holding the ALT key.
 * Commonly used as a shortcut to breaking connections.
 *
 * @param	Connector	The connector that was ALT+clicked upon.
 */
void WxSoundCueEditor::AltClickConnector( FLinkedObjectConnector& Connector )
{
	SetSelectedConnector(Connector);

	wxCommandEvent DummyEvent;
	DummyEvent.SetClientData(&Connector); //this works because OnContextBreakLink() is called immediately
	OnContextBreakLink( DummyEvent );
}

void WxSoundCueEditor::MoveSelectedObjects( INT DeltaX, INT DeltaY )
{
	for( INT i = 0; i < SelectedNodes.Num(); i++ )
	{
		USoundNode* Node = SelectedNodes( i );
		FSoundNodeEditorData* EdData = SoundCue->EditorData.Find( Node );
		check(EdData);
		
		EdData->NodePosX += DeltaX;
		EdData->NodePosY += DeltaY;
	}
}

void WxSoundCueEditor::EdHandleKeyInput( FViewport* Viewport, FName Key, EInputEvent Event )
{
	if( Event == IE_Pressed )
	{
		if( Key == KEY_Delete )
		{
			DeleteSelectedNodes();
		}
	}
}

void WxSoundCueEditor::ConnectNodes( USoundNode* ParentNode, INT ChildIndex, USoundNode* ChildNode )
{
	check( ChildIndex >= 0 && ChildIndex < ParentNode->ChildNodes.Num() );

	ParentNode->ChildNodes( ChildIndex ) = ChildNode;

	ParentNode->PostEditChange( NULL );

	UpdatePropertyWindow();
	RefreshViewport();
}

void WxSoundCueEditor::DeleteSelectedNodes()
{
	for( INT i = 0; i < SelectedNodes.Num(); i++ )
	{
		USoundNode* DelNode = SelectedNodes( i );

		// We look through all nodes to see if there is a node that still references the one we want to delete.
		// If so, break the link.
		for( TMap<USoundNode *, FSoundNodeEditorData>::TIterator It( SoundCue->EditorData ); It; ++It )
		{
			USoundNode* ChkNode = It.Key();

			if( ChkNode == NULL )
			{
				continue;
			}

			for( INT ChildIdx = 0; ChildIdx < ChkNode->ChildNodes.Num(); ChildIdx++ )
			{
				if( ChkNode->ChildNodes( ChildIdx ) == DelNode )
				{
					ChkNode->ChildNodes( ChildIdx ) = NULL;
					ChkNode->PostEditChange( NULL );
				}
			}
		}

		// Also check the 'first node' pointer
		if( SoundCue->FirstNode == DelNode )
		{
			SoundCue->FirstNode = NULL;
		}

		// Remove this node from the SoundCue's map of all SoundNodes
		check( SoundCue->EditorData.Find( DelNode ) );
		SoundCue->EditorData.Remove( DelNode );
	}

	SelectedNodes.Empty();

	UpdatePropertyWindow();
	RefreshViewport();
}

/*-----------------------------------------------------------------------------
	WxSoundCueEditor
-----------------------------------------------------------------------------*/

UBOOL				WxSoundCueEditor::bSoundNodeClassesInitialized = false;
TArray<UClass *>	WxSoundCueEditor::SoundNodeClasses;

BEGIN_EVENT_TABLE( WxSoundCueEditor, wxFrame )
	EVT_MENU_RANGE( IDM_SOUNDCUE_NEW_NODE_START, IDM_SOUNDCUE_NEW_NODE_END, WxSoundCueEditor::OnContextNewSoundNode )
	EVT_MENU( IDM_SOUNDCUE_NEW_WAVE, WxSoundCueEditor::OnContextNewWave )
	EVT_MENU( IDM_SOUNDCUE_ADD_INPUT, WxSoundCueEditor::OnContextAddInput )
	EVT_MENU( IDM_SOUNDCUE_DELETE_INPUT, WxSoundCueEditor::OnContextDeleteInput )
	EVT_MENU( IDM_SOUNDCUE_DELETE_NODE, WxSoundCueEditor::OnContextDeleteNode )
	EVT_MENU( IDM_SOUNDCUE_PLAY_NODE, WxSoundCueEditor::OnContextPlaySoundNode )
	EVT_MENU( IDM_SOUNDCUE_PLAY_CUE, WxSoundCueEditor::OnContextPlaySoundCue )
	EVT_MENU( IDM_SOUNDCUE_STOP_PLAYING, WxSoundCueEditor::OnContextStopPlaying )
	EVT_MENU( IDM_SOUNDCUE_BREAK_LINK, WxSoundCueEditor::OnContextBreakLink )
	EVT_SIZE( WxSoundCueEditor::OnSize )
END_EVENT_TABLE()

IMPLEMENT_COMPARE_POINTER( UClass, SoundCueEditor, { return appStricmp( *A->GetName(), *B->GetName() ); } )

// Static functions that fills in array of all available USoundNode classes and sorts them alphabetically.
void WxSoundCueEditor::InitSoundNodeClasses()
{
	if( bSoundNodeClassesInitialized )
	{
		return;
	}

	// Construct list of non-abstract gameplay sequence object classes.
	for( TObjectIterator<UClass> It; It; ++It )
	{
		if( It->IsChildOf( USoundNode::StaticClass() ) 
			&& !( It->ClassFlags & CLASS_Abstract ) 
			&& !It->IsChildOf( USoundNodeWave::StaticClass() ) 
			&& !It->IsChildOf( USoundNodeAmbient::StaticClass() ) )
		{
			SoundNodeClasses.AddItem(*It);
		}
	}

	Sort<USE_COMPARE_POINTER( UClass, SoundCueEditor )>( &SoundNodeClasses( 0 ), SoundNodeClasses.Num() );

	bSoundNodeClassesInitialized = true;
}

WxSoundCueEditor::WxSoundCueEditor( wxWindow* InParent, wxWindowID InID, USoundCue* InSoundCue )
	: WxLinkedObjEd( InParent, InID, TEXT( "SoundCueEditor" ) )
{
	SoundCue = InSoundCue;

	// Set the sound cue editor window title to include the sound cue being edited.
	SetTitle( *FString::Printf( LocalizeSecure(LocalizeUnrealEd( "SoundCueEditorCaption_F" ), *InSoundCue->GetPathName()) ) );
}

WxSoundCueEditor::~WxSoundCueEditor()
{
	SaveProperties();
}

void WxSoundCueEditor::InitEditor()
{
	CreateControls( FALSE );

	LoadProperties();

	InitSoundNodeClasses();

	// Shift origin so origin is roughly in the middle. Would be nice to use Viewport size, but doesn't seem initialised here...
	LinkedObjVC->Origin2D.X = 150;
	LinkedObjVC->Origin2D.Y = 300;

	BackgroundTexture = LoadObject<UTexture2D>( NULL, TEXT( "EditorMaterials.SoundCueBackground" ), NULL, LOAD_None, NULL );

	ConnObj = NULL;
}

/**
 * Creates the controls for this window
 */
void WxSoundCueEditor::CreateControls( UBOOL bTreeControl )
{
	WxLinkedObjEd::CreateControls( bTreeControl );

	if( PropertyWindow != NULL )
	{
		SetDockingWindowTitle( PropertyWindow, *FString::Printf( LocalizeSecure(LocalizeUnrealEd( "PropertiesCaption_F" ), *SoundCue->GetPathName()) ) );
	}

	wxMenuBar* MenuBar = new wxMenuBar();
	AppendWindowMenu( MenuBar );
	SetMenuBar( MenuBar );

	ToolBar = new WxSoundCueEdToolBar( this, -1 );
	SetToolBar( ToolBar );
}

/**
 * Used to serialize any UObjects contained that need to be to kept around.
 *
 * @param Ar The archive to serialize with
 */
void WxSoundCueEditor::Serialize( FArchive& Ar )
{
	WxLinkedObjEd::Serialize( Ar );
	if( !Ar.IsLoading() && !Ar.IsSaving() )
	{
		Ar << SoundCue << SelectedNodes << ConnObj << SoundNodeClasses;
	}
}

void WxSoundCueEditor::OnContextNewSoundNode( wxCommandEvent& In )
{
	INT NewNodeIndex = In.GetId() - IDM_SOUNDCUE_NEW_NODE_START;	
	check( NewNodeIndex >= 0 && NewNodeIndex < SoundNodeClasses.Num() );

	UClass* NewNodeClass = SoundNodeClasses( NewNodeIndex );
	check( NewNodeClass->IsChildOf( USoundNode::StaticClass() ) );

	USoundNode* NewNode = ConstructObject<USoundNode>( NewNodeClass, SoundCue, NAME_None );

    // If this node allows >0 children but by default has zero - create a connector for starters
	if( ( NewNode->GetMaxChildNodes() > 0 || NewNode->GetMaxChildNodes() == -1 ) && NewNode->ChildNodes.Num() == 0 )
	{
		NewNode->CreateStartingConnectors();
	}

	// Create new editor data struct and add to map in SoundCue.
	FSoundNodeEditorData NewEdData;
	appMemset( &NewEdData, 0, sizeof( FSoundNodeEditorData ) );

	NewEdData.NodePosX = ( LinkedObjVC->NewX - LinkedObjVC->Origin2D.X ) / LinkedObjVC->Zoom2D;
	NewEdData.NodePosY = ( LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y ) / LinkedObjVC->Zoom2D;

	SoundCue->EditorData.Set( NewNode, NewEdData );

	RefreshViewport();
}

void WxSoundCueEditor::OnContextNewWave( wxCommandEvent& In )
{
	USoundNodeWave* NewWave = GEditor->GetSelectedObjects()->GetTop<USoundNodeWave>();
	if( !NewWave )
	{
		return;
	}

	// We can only have a particular SoundNodeWave in the SoundCue once.
	for( TMap<USoundNode*, FSoundNodeEditorData>::TIterator It( SoundCue->EditorData ); It; ++It )
	{
		USoundNode* ChkNode = It.Key();
		if( NewWave == ChkNode )
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd( "Error_OnlyAddSoundNodeWaveOnce" ) );
			return;
		}
	}

	// Create new editor data struct and add to map in SoundCue.
	FSoundNodeEditorData NewEdData;
	appMemset( &NewEdData, 0, sizeof( FSoundNodeEditorData ) );

	NewEdData.NodePosX = ( LinkedObjVC->NewX - LinkedObjVC->Origin2D.X ) / LinkedObjVC->Zoom2D;
	NewEdData.NodePosY = ( LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y ) / LinkedObjVC->Zoom2D;

	SoundCue->EditorData.Set( NewWave, NewEdData );

	RefreshViewport();
}

void WxSoundCueEditor::OnContextAddInput( wxCommandEvent& In )
{
	INT NumSelected = SelectedNodes.Num();
	if( NumSelected != 1 )
	{
		return;
	}

	USoundNode* Node = SelectedNodes( 0 );

	if( Node->GetMaxChildNodes() == -1 || ( Node->ChildNodes.Num() < Node->GetMaxChildNodes() ) )
	{
		Node->InsertChildNode( Node->ChildNodes.Num() );
	}

	Node->PostEditChange( NULL );

	UpdatePropertyWindow();
	RefreshViewport();
}

void WxSoundCueEditor::OnContextDeleteInput( wxCommandEvent& In )
{
	check( ConnType == LOC_INPUT );

	// Can't delete root input!
	if( ConnObj == SoundCue )
	{
		return;
	}

	USoundNode* ConnNode = CastChecked<USoundNode>( ConnObj );
	check( ConnIndex >= 0 && ConnIndex < ConnNode->ChildNodes.Num() );

	ConnNode->RemoveChildNode( ConnIndex );

	ConnNode->PostEditChange( NULL );

	UpdatePropertyWindow();
	RefreshViewport();
}

void WxSoundCueEditor::OnContextDeleteNode( wxCommandEvent& In )
{
	DeleteSelectedNodes();
}

void WxSoundCueEditor::OnContextPlaySoundNode( wxCommandEvent& In )
{
	if( SelectedNodes.Num() != 1 )
	{
		return;
	}

	UAudioComponent* AudioComponent = GEditor->GetPreviewAudioComponent( NULL, SelectedNodes( 0 ) );
	if( AudioComponent )
	{
		AudioComponent->Stop();

		if( AudioComponent->SoundCue->ValidateData() )
		{
			GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
		}

		AudioComponent->bUseOwnerLocation = FALSE;
		AudioComponent->bAutoDestroy = FALSE;
		AudioComponent->Location = FVector( 0.0f, 0.0f, 0.0f );
		AudioComponent->bIsUISound = TRUE;
		AudioComponent->bAllowSpatialization = FALSE;
		AudioComponent->bNoReverb = TRUE;

		AudioComponent->Play();	
	}
}

void WxSoundCueEditor::OnContextPlaySoundCue( wxCommandEvent& In )
{
	UAudioComponent* AudioComponent = GEditor->GetPreviewAudioComponent( SoundCue, NULL );
	if( AudioComponent )
	{
		AudioComponent->Stop();

		if( AudioComponent->SoundCue->ValidateData() )
		{
			GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
		}

		AudioComponent->bUseOwnerLocation = FALSE;
		AudioComponent->bAutoDestroy = FALSE;
		AudioComponent->Location = FVector( 0.0f, 0.0f, 0.0f );
		AudioComponent->bIsUISound = TRUE;
		AudioComponent->bAllowSpatialization = FALSE;
		AudioComponent->bNoReverb = TRUE;

		AudioComponent->Play();	
	}
}

void WxSoundCueEditor::OnContextStopPlaying( wxCommandEvent& In )
{
	UAudioComponent* AudioComponent = GEditor->GetPreviewAudioComponent( NULL, NULL );
	if( AudioComponent )
	{
		AudioComponent->Stop();
	}
}

void WxSoundCueEditor::OnContextBreakLink( wxCommandEvent& In )
{
	if( ConnObj == SoundCue )
	{
		SoundCue->FirstNode = NULL;
		SoundCue->PostEditChange( NULL );
	}
	else
	{
		USoundNode* ConnNode = CastChecked<USoundNode>( ConnObj );

		if(ConnType == LOC_INPUT)
		{
			check( ConnIndex >= 0 && ConnIndex < ConnNode->ChildNodes.Num() );
			ConnNode->ChildNodes( ConnIndex ) = NULL;

			ConnNode->PostEditChange( NULL );
		}
		else if(ConnType == LOC_OUTPUT)
		{
			if(ConnNode == SoundCue->FirstNode)
			{
				SoundCue->FirstNode = NULL;
				SoundCue->PostEditChange(NULL);
			}
			else
			{
				TArray<USoundNode*> NodeCollection;
				SoundCue->EditorData.GenerateKeyArray(NodeCollection);

				UBOOL bFound = FALSE;
				for(int NodeIndex = 0; NodeIndex < NodeCollection.Num() && !bFound; ++NodeIndex)
				{
					USoundNode *CurrentNode = NodeCollection(NodeIndex);
					for(int ChildIndex = 0; ChildIndex < CurrentNode->ChildNodes.Num(); ++ChildIndex)
					{
						if(CurrentNode->ChildNodes(ChildIndex) == ConnNode)
						{
							bFound = TRUE;
							CurrentNode->ChildNodes(ChildIndex) = NULL;
							CurrentNode->PostEditChange(NULL);
							break;
						}
					}
				}
			}
		}
	}

	UpdatePropertyWindow();
	RefreshViewport();
}

void WxSoundCueEditor::OnSize( wxSizeEvent& In )
{
	RefreshViewport();
	const wxRect rc = GetClientRect();
	::MoveWindow( ( HWND )LinkedObjVC->Viewport->GetWindow(), 0, 0, rc.GetWidth(), rc.GetHeight(), 1 );
	In.Skip();
}

/*-----------------------------------------------------------------------------
	WxSoundCueEdToolBar.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxSoundCueEdToolBar, wxToolBar )
END_EVENT_TABLE()

WxSoundCueEdToolBar::WxSoundCueEdToolBar( wxWindow* InParent, wxWindowID InID )
: wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_3DBUTTONS )
{
	PlayCueB.Load( TEXT( "SCE_PlayCue" ) );
	PlayNodeB.Load( TEXT( "SCE_PlayNode" ) );
	StopB.Load( TEXT( "SCE_Stop" ) );
	SetToolBitmapSize( wxSize( 18, 18 ) );

	AddTool( IDM_SOUNDCUE_STOP_PLAYING, StopB, *LocalizeUnrealEd( "Stop" ) );
	AddTool( IDM_SOUNDCUE_PLAY_NODE, PlayNodeB, *LocalizeUnrealEd( "PlaySelectedNode" ) );
	AddTool( IDM_SOUNDCUE_PLAY_CUE, PlayCueB, *LocalizeUnrealEd( "PlaySoundCue" ) );

	Realize();
}

WxSoundCueEdToolBar::~WxSoundCueEdToolBar()
{
}

/*-----------------------------------------------------------------------------
	WxMBSoundCueEdNewNode.
-----------------------------------------------------------------------------*/

WxMBSoundCueEdNewNode::WxMBSoundCueEdNewNode( WxSoundCueEditor* CueEditor )
{
	USoundNodeWave* SelectedWave = GEditor->GetSelectedObjects()->GetTop<USoundNodeWave>();
	if( SelectedWave )
	{
		Append( IDM_SOUNDCUE_NEW_WAVE, *FString::Printf( LocalizeSecure(LocalizeUnrealEd( "SoundNodeWave_F" ), *SelectedWave->GetName()) ), TEXT( "" ) );
		AppendSeparator();
	}

	for( INT i = 0; i < CueEditor->SoundNodeClasses.Num(); i++)
	{
		Append( IDM_SOUNDCUE_NEW_NODE_START + i, *CueEditor->SoundNodeClasses(i)->GetDescription(), TEXT( "" ) );
	}
}

WxMBSoundCueEdNewNode::~WxMBSoundCueEdNewNode()
{
}

/*-----------------------------------------------------------------------------
	WxMBSoundCueEdNodeOptions.
-----------------------------------------------------------------------------*/

WxMBSoundCueEdNodeOptions::WxMBSoundCueEdNodeOptions( WxSoundCueEditor* CueEditor )
{
	INT NumSelected = CueEditor->SelectedNodes.Num();

	if( NumSelected == 1 )
	{
		// See if we adding another input would exceed max child nodes.
		USoundNode* Node = CueEditor->SelectedNodes( 0 );
		if( Node->GetMaxChildNodes() == -1 || ( Node->ChildNodes.Num() < Node->GetMaxChildNodes() ) )
		{
			Append( IDM_SOUNDCUE_ADD_INPUT, *LocalizeUnrealEd( "AddInput" ), TEXT( "" ) );
		}
	}

	Append( IDM_SOUNDCUE_DELETE_NODE, *LocalizeUnrealEd( "DeleteSelectedNodes" ), TEXT( "" ) );

	if( NumSelected == 1 )
	{
		Append( IDM_SOUNDCUE_PLAY_NODE, *LocalizeUnrealEd( "PlaySelectedNode" ), TEXT( "" ) );
	}
}

WxMBSoundCueEdNodeOptions::~WxMBSoundCueEdNodeOptions()
{
}

/*-----------------------------------------------------------------------------
	WxMBSoundCueEdConnectorOptions.
-----------------------------------------------------------------------------*/

WxMBSoundCueEdConnectorOptions::WxMBSoundCueEdConnectorOptions( WxSoundCueEditor* CueEditor )
{
	// Only display the 'Break Link' option if there is a link to break!
	UBOOL bHasConnection = false;
	if( CueEditor->ConnType == LOC_INPUT )
	{
		if( CueEditor->ConnObj == CueEditor->SoundCue )
		{
			if( CueEditor->SoundCue->FirstNode )
			{
				bHasConnection = true;
			}
		}
		else
		{
			USoundNode* ConnNode = CastChecked<USoundNode>( CueEditor->ConnObj );

			if( ConnNode->ChildNodes( CueEditor->ConnIndex ) )
			{
				bHasConnection = true;
			}
		}
	}
	else if(CueEditor->ConnType == LOC_OUTPUT && Cast<USoundNode>(CueEditor->ConnObj))
	{
		bHasConnection = true;
	}

	if( bHasConnection )
	{
		Append( IDM_SOUNDCUE_BREAK_LINK, *LocalizeUnrealEd( "BreakLink" ), TEXT( "" ) );
	}

	// If on an input that can be deleted, show option
	if( CueEditor->ConnType == LOC_INPUT && CueEditor->ConnObj != CueEditor->SoundCue )
	{
		Append( IDM_SOUNDCUE_DELETE_INPUT, *LocalizeUnrealEd( "DeleteInput" ), TEXT( "" ) );
	}
}

WxMBSoundCueEdConnectorOptions::~WxMBSoundCueEdConnectorOptions()
{
}

