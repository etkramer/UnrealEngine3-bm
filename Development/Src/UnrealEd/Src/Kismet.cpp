/*=============================================================================
	Kismet.cpp: Gameplay sequence editing
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

@todo UI integration tasks:
- WxKismet::InitSeqObjClasses (UI kismet should only display UI kismet classes)
- WxKismet::RebuildTreeControl (only grabs sequences from the level)
- WxKismet::FindActorReferences (need a corresponding version for the UI)
- WxKismet::DeleteSelected (cannot allow UISequences to be deleted, because they're owned by a widget)
- WxKismet::FindOutputLinkTo (needs to use interface for retrieving the objects contained by the current sequencecontainer)
- WxKismet::ChangeActiveSequence (need a way to display sequence objects contained within UIStates)
- WxKismet::CenterViewOnSeqObj (see General notes below)
- WxKismet::OpenKismet/OpenSequenceInKismet (assumes that we always want to use the WxKismet class for the editor)
- WxKismet::OnContextSelectAssigned (need this to work for UI as well)
- WxKismet::OnContextNewScriptingObjVarContext (need this to work for UI as well)
- WxKismet::OnOpenNewWindow (calls static function to open a new kismet window, which doesn't use the correct WxKismet class)

General:
- all code that attempts to find the SequenceObject's ParentSequence by either directly accessing ParentSequence or casting the object's outer to Sequence is unsafe for UI sequence objects
=============================================================================*/

#include "UnrealEd.h"
#include "Factories.h"
#include "UnLinkedObjEditor.h"
#include "Kismet.h"
#include "EngineSequenceClasses.h"
#include "EnginePrefabClasses.h"
#include "UnLinkedObjDrawUtils.h"
#include "ScopedTransaction.h"
#include "Properties.h"
#include "LevelBrowser.h"
#include "DlgRename.h"
#include "ScopedObjectStateChange.h"

static INT	MultiEventSpacing = 100;
static INT	PasteOffset = 30;
static INT	FitCommentToSelectedBorder = 15;

/** Maximum region that is valid for sequence objects to exist in. */
static INT	MaxSequenceSize = 10000;

IMPLEMENT_CLASS(UKismetBindings);
IMPLEMENT_CLASS(USequenceObjectHelper);

void WxKismet::Serialize(FArchive& Ar)
{
	WxLinkedObjEd::Serialize(Ar);

	Ar << SeqObjClasses;
	Ar << ConnSeqOp;
	Ar << SelectedSeqObjs;
	Ar << Sequence;
	Ar << OldSequence;
	Ar << CopyConnInfo;
	Ar << OpExposeVarLinkMap;
}

FArchive& operator<<(FArchive &Ar, FCopyConnInfo &Info)
{
	return Ar << Info.SeqObject;
}

FArchive& operator<<(FArchive &Ar, FExposeVarLinkInfo &Info)
{
	Ar << Info.Property << Info.VariableClass << Info.LinkIdx;
	INT Type = (INT)Info.Type;
	Ar << Type;
	if (Ar.IsLoading())
	{
		Info.Type = (FExposeVarLinkInfo::EExposeType)Type;
	}
	return Ar;
}

static UBOOL SeqObjPosIsValid(INT NewPosX, INT NewPosY)
{
	if(NewPosX < -MaxSequenceSize || NewPosX > MaxSequenceSize || NewPosY < -MaxSequenceSize || NewPosY > MaxSequenceSize)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/* epic ===============================================
* static FindOutputLinkTo
*
* Finds all of the ops in the sequence's SequenceObjects
* list that contains an output link to the specified
* op's input.
*
* Returns TRUE to indicate at least one output was
* returned.
*
* =====================================================
*/
UBOOL WxKismet::FindOutputLinkTo(const USequence *sequence, const USequenceOp *targetOp, const INT inputIdx, TArray<USequenceOp*> &outputOps, TArray<INT> &outputIndices)
{
	outputOps.Empty();
	outputIndices.Empty();
	// iterate through all sequence objects in the sequence,
	for (INT objIdx = 0; objIdx < sequence->SequenceObjects.Num(); objIdx++)
	{
		// if this is an op
		USequenceOp *chkOp = Cast<USequenceOp>(sequence->SequenceObjects(objIdx));
		if (chkOp != NULL)
		{
			// iterate through each of the outputs
			for (INT linkIdx = 0; linkIdx < chkOp->OutputLinks.Num(); linkIdx++)
			{
				// iterate through each input linked to this output
				UBOOL bFoundMatch = 0;
				for (INT linkInputIdx = 0; linkInputIdx < chkOp->OutputLinks(linkIdx).Links.Num() && !bFoundMatch; linkInputIdx++)
				{
					// if this matches the op
					if (chkOp->OutputLinks(linkIdx).Links(linkInputIdx).LinkedOp == targetOp &&
						chkOp->OutputLinks(linkIdx).Links(linkInputIdx).InputLinkIdx == inputIdx)
					{
						// add to the list
						outputOps.AddItem(chkOp);
						outputIndices.AddItem(linkIdx);
						bFoundMatch = 1;
					}
				}
			}
		}
	}
	return (outputOps.Num() > 0);
}

void WxKismet::OpenNewObjectMenu()
{
	WxMBKismetNewObject menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

void WxKismet::OpenObjectOptionsMenu()
{
	WxMBKismetObjectOptions menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

void WxKismet::OpenConnectorOptionsMenu()
{
	WxMBKismetConnectorOptions menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

void WxKismet::ClickedLine(FLinkedObjectConnector &Src, FLinkedObjectConnector &Dest)
{
	UBOOL bCtrlDown = (LinkedObjVC->Viewport->KeyState(KEY_LeftControl) || LinkedObjVC->Viewport->KeyState(KEY_RightControl));	
	if (bCtrlDown)
	{
		// disable ctrl-dragging lines due to LD request
		return;
	}
	UBOOL bAltDown = (LinkedObjVC->Viewport->KeyState(KEY_LeftAlt) || LinkedObjVC->Viewport->KeyState(KEY_RightAlt));
	UBOOL bShiftDown = LinkedObjVC->Viewport->KeyState(KEY_LeftShift) || LinkedObjVC->Viewport->KeyState(KEY_RightShift);
	INT HitX = LinkedObjVC->Viewport->GetMouseX();
	INT HitY = LinkedObjVC->Viewport->GetMouseY();
	// drag this line
	if (bCtrlDown || bShiftDown || bAltDown)
	{
		FIntPoint MousePos(HitX,HitY), SrcPos, DestPos;
		USequenceOp *SrcOp = Cast<USequenceOp>(Src.ConnObj),
					*DestOp = Cast<USequenceOp>(Dest.ConnObj);
		if (SrcOp != NULL &&
			DestOp != NULL)
		{
			// figure out which end we should pick up
			SrcPos = SrcOp->GetConnectionLocation(LOC_OUTPUT,Src.ConnIndex);
			DestPos = DestOp->GetConnectionLocation(LOC_INPUT,Dest.ConnIndex);
			if ((SrcPos - MousePos).Size() < (DestPos - MousePos).Size())
			{
				// closer to source
				SetSelectedConnector(Dest);
			}
			else
			{
				// closer to end
				SetSelectedConnector(Src);
			}
			if (!bCtrlDown)
			{
				FScopedTransaction Transaction(TEXT("Remove Link"));	//@todo localize
				FScopedObjectStateChange Notifier(SrcOp);
				UBOOL bModified = FALSE;

				// break the connection to src
				for (INT Idx = 0; Idx < SrcOp->OutputLinks(Src.ConnIndex).Links.Num(); Idx++)
				{
					if (SrcOp->OutputLinks(Src.ConnIndex).Links(Idx).LinkedOp == DestOp &&
						SrcOp->OutputLinks(Src.ConnIndex).Links(Idx).InputLinkIdx == Dest.ConnIndex)
					{
						bModified = TRUE;
						SrcOp->OutputLinks(Src.ConnIndex).Links.Remove(Idx--,1);
						break;
					}
				}

				if ( !bModified )
				{
					Notifier.CancelEdit();
					Transaction.Cancel();
				}
			}
			if (bShiftDown || bCtrlDown)
			{
				// start making the line and update the position
				LinkedObjVC->bMakingLine = TRUE;
				LinkedObjVC->NewX = HitX;
				LinkedObjVC->NewY = HitY;
			}
		}
	}
	else
	{
		// select the two
		EmptySelection();
		AddToSelection(Src.ConnObj);
		AddToSelection(Dest.ConnObj);
	}
}

void WxKismet::DoubleClickedLine(FLinkedObjectConnector &Src, FLinkedObjectConnector &Dest)
{
	// select the two and zoom
	EmptySelection();
	AddToSelection(Src.ConnObj);
	AddToSelection(Dest.ConnObj);
	ZoomSelection();
}

void WxKismet::DoubleClickedConnector(FLinkedObjectConnector& Connector)
{
	//debugf(TEXT("double-clicked connector: %s %d"),*Connector.ConnObj->GetName(),Connector.ConnIndex);
	USequenceOp* Op = Cast<USequenceOp>(Connector.ConnObj);
	if (Op != NULL)
	{
		if (Connector.ConnType == LOC_INPUT)
		{
			TArray<USequenceOp*> LinkedOps;
			TArray<INT> LinkedIndices;
			if (FindOutputLinkTo(Op->ParentSequence,Op,Connector.ConnIndex,LinkedOps,LinkedIndices))
			{
				EmptySelection();
				for (INT Idx = 0; Idx < LinkedOps.Num(); Idx++)
				{
					AddToSelection(LinkedOps(Idx));
				}
				ZoomSelection();
			}
		}
		else
		if (Connector.ConnType == LOC_OUTPUT)
		{
			if (Op->OutputLinks(Connector.ConnIndex).Links.Num() > 0)
			{
				EmptySelection();
				for (INT Idx = 0; Idx < Op->OutputLinks(Connector.ConnIndex).Links.Num(); Idx++)
				{
					AddToSelection(Op->OutputLinks(Connector.ConnIndex).Links(Idx).LinkedOp);
				}
				ZoomSelection();
			}
		}
	}
}

void WxKismet::DoubleClickedObject(UObject* Obj)
{
	if ( Obj->IsA(USequenceVariable::StaticClass()) )
	{
		// if double clicking on an object variable
		if ( Obj->IsA(USeqVar_Object::StaticClass()) )
		{
			USeqVar_Object *objVar = (USeqVar_Object*)Obj;
			if (objVar->ObjValue != NULL &&
				objVar->ObjValue->IsA(AActor::StaticClass()))
			{
				AActor *Actor = (AActor*)(objVar->ObjValue);
				// Select the actor and move the camera to a view of this actor.
				GEditor->SelectNone( FALSE, TRUE );
				GEditor->SelectActor( Actor, TRUE, NULL, TRUE );
				GEditor->MoveViewportCamerasToActor( *Actor, FALSE );
			}
		}
		else
		// if double clicking on a named variable
		if ( Obj->IsA(USeqVar_Named::StaticClass()) )
		{
			// Double clicking on a named variable causes the Kismet view to jump to the named variable.
			USeqVar_Named* NamedVar = static_cast<USeqVar_Named*>( Obj );
			if ( NamedVar->FindVarName != NAME_None )
			{
				DoSearch( *(NamedVar->FindVarName.ToString()), KST_VarName, TRUE );
			}
		}
	}
	else if ( Obj->IsA(USequenceOp::StaticClass()) )
	{
		// if double clicking on an action
		if ( Obj->IsA(USequenceAction::StaticClass()) )
		{
			if( Obj->IsA(USeqAct_Interp::StaticClass()) )
			{
				USeqAct_Interp* InterpAct = (USeqAct_Interp*)(Obj);
				// Release the mouse before we pop up the Matinee window.
				LinkedObjVC->Viewport->LockMouseToWindow(FALSE);
				LinkedObjVC->Viewport->Invalidate();

				// Open matinee to edit the selected Interp action.
				OpenMatinee(InterpAct);
			}
			else
			// If double clicking on an "activate remote event" action.
			if ( Obj->IsA(USeqAct_ActivateRemoteEvent::StaticClass()) )
			{
				// Jump to the remote event activated by this action.
				USeqAct_ActivateRemoteEvent* ActivateAction = static_cast<USeqAct_ActivateRemoteEvent*>( Obj );
				if ( ActivateAction->EventName != NAME_None )
				{
					DoSearch( *(ActivateAction->EventName.ToString()), KST_RemoteEvents, TRUE, KSS_AllLevels );
				}
			}
			else
			{
				ZoomSelection();
			}
		}
		else
		// if double clicking on an event
		if ( Obj->IsA(USequenceEvent::StaticClass()) )
		{
			// if double clicking on a remote event
			if ( Obj->IsA(USeqEvent_RemoteEvent::StaticClass()) )
			{
				// Jump to an "activate remote event" action that references this remote event.
				USeqEvent_RemoteEvent* RemoteEvent = static_cast<USeqEvent_RemoteEvent*>( Obj );
				if ( RemoteEvent->EventName != NAME_None )
				{
					// This will handle opening search window, jumping to first result etc.
					DoSearch( *(RemoteEvent->EventName.ToString()), KST_ReferencesRemoteEvent, TRUE, KSS_AllLevels );
				}
			}
			else
			{
				// select the associated actor and move the camera
				USequenceEvent *evt = (USequenceEvent*)(Obj);
				AActor* EvtOriginator = evt->Originator;
				if (EvtOriginator != NULL)
				{
					GEditor->SelectNone( FALSE, TRUE );
					GEditor->SelectActor( EvtOriginator, TRUE, NULL, TRUE );
					GEditor->MoveViewportCamerasToActor( *EvtOriginator, FALSE );
				}
			}
		}
		else
		// if double clicking on a sequence
		if ( Obj->IsA(USequence::StaticClass()) )
		{
			// make it the active sequence
			ChangeActiveSequence((USequence*)Obj);
			GEditor->ResetTransaction( *LocalizeUnrealEd("OpenSequence") );
		}
	}
	else if ( Obj->IsA(USequenceFrame::StaticClass()) )
	{
		USequenceFrameWrapped *Comment = (USequenceFrameWrapped*)(Obj);
		WxDlgGenericStringWrappedEntry dlg;
		INT Result = dlg.ShowModal( TEXT("EditComment"), TEXT("CommentText"), *Comment->ObjComment );
		if (Result == wxID_OK)
		{
			Comment->ObjComment = dlg.GetEnteredString();
		}
	}

	// Call the double click event of the helper class for this sequence object.
	USequenceObject* SeqObj = Cast<USequenceObject>(Obj);

	if(SeqObj != NULL)
	{
		UClass* Class = LoadObject<UClass>( NULL, *SeqObj->GetEdHelperClassName(), NULL, LOAD_None, NULL );
		if ( Class != NULL )
		{
			USequenceObjectHelper* NodeHelper = CastChecked<USequenceObjectHelper>(Class->GetDefaultObject());

			if( NodeHelper )
			{
				NodeHelper->OnDoubleClick(this, SeqObj);
			}
		}
	}

}

/** Create new . */
USequenceObject* WxKismet::NewShorcutObject()
{
	UClass* NewSeqObjClass = NULL;
	USequenceObject* NewSeqObj = NULL;

	// Use default object (properties set from ..Editor.ini) for bindings.
	UKismetBindings* Bindings = (UKismetBindings*)(UKismetBindings::StaticClass()->GetDefaultObject());
	const UBOOL bCtrlDown = (LinkedObjVC->Viewport->KeyState(KEY_LeftControl) || LinkedObjVC->Viewport->KeyState(KEY_RightControl));
	const UBOOL bShiftDown = (LinkedObjVC->Viewport->KeyState(KEY_LeftShift) || LinkedObjVC->Viewport->KeyState(KEY_RightShift));

	// Iterate over each bind and see if we are holding those keys down.
	for(INT i=0; i<Bindings->Bindings.Num() && !NewSeqObjClass; i++)
	{
		const FKismetKeyBind& Bind = Bindings->Bindings(i);
		const UBOOL bKeyCorrect = LinkedObjVC->Viewport->KeyState( Bind.Key );
		const UBOOL bCtrlCorrect = (bCtrlDown == Bind.bControl);
		const UBOOL bShiftCorrect = (bShiftDown == Bind.bShift);
		if(bKeyCorrect && bCtrlCorrect && bShiftCorrect)
		{
			NewSeqObjClass = FindObject<UClass>(ANY_PACKAGE, *Bind.SeqObjClassName.ToString());
		}
	}

	// If we got a class (and its a valid sequence class for this kismet window), create a new object.
	if ( NewSeqObjClass && NewSeqObjClass->IsChildOf(USequenceObject::StaticClass()) && IsValidSequenceClass(NewSeqObjClass) )
	{
		// Slight hack special case for creating subsequences from selected...
		if( NewSeqObjClass == USequence::StaticClass() )
		{
			// Unlock mouse from viewport so you can move cursor to 'New Sequence Name' diaog.
			LinkedObjVC->Viewport->LockMouseToWindow(FALSE);

			wxCommandEvent DummyEvent;
			OnContextCreateSequence( DummyEvent );
		}
		else
		{
			INT NewPosX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
			INT NewPosY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;

			// Create new object. This will clear current selection and select the new object.
			NewSeqObj = NewSequenceObject(NewSeqObjClass, NewPosX, NewPosY);
		}
	}

	// Return newly created object
	return NewSeqObj;
}

/** 
 *	Executed when user clicks and releases on background without moving mouse much. 
 *	We use the Kismet key bindings to see if we should spawn an object now.
 *	If we return 'TRUE' it will release the current selection set.
 */
UBOOL WxKismet::ClickOnBackground()
{
	USequenceObject* NewSeqObj = NewShorcutObject();
	if(NewSeqObj)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/** Create new sequence object using key bindings, but also automatically hook it up. */
UBOOL WxKismet::ClickOnConnector(UObject* InObj, INT InConnType, INT InConnIndex)
{
	USequenceObject* NewSeqObj = NewShorcutObject();
	if(NewSeqObj)
	{
		// We got a new object - lets see if its an op, and if so connect it in automagically
		USequenceOp* NewSeqOp = Cast<USequenceOp>(NewSeqObj);
		USequenceOp* ClickedOp = Cast<USequenceOp>(InObj);

		// Check new op has at least one input and output
		if(NewSeqOp && ClickedOp && NewSeqOp->OutputLinks.Num() > 0 && NewSeqOp->InputLinks.Num() > 0)
		{

			wxCommandEvent CutEvent, PasteEvent;
			CutEvent.SetId(IDM_KISMET_CUT_CONNECTIONS);


			if(InConnType == LOC_OUTPUT)
			{
				// First we use the connection copy/paste code to copy connections from the clicked output connector to the first output of the new op.
				ConnSeqOp = ClickedOp;
				ConnType = LOC_OUTPUT;
				ConnIndex = InConnIndex;
				OnContextCopyConnections(CutEvent);

				ConnSeqOp = NewSeqOp;
				ConnType = LOC_OUTPUT;
				ConnIndex = 0;
				OnContextPasteConnections(PasteEvent);

				// Make connection from clicked output connector to first input of new op.
				MakeLogicConnection(ClickedOp, InConnIndex, NewSeqOp, 0);
			}
			else if(InConnType == LOC_INPUT)
			{
				// First we use the connection copy/paste code to copy connections from the clicked input connector to the first input of the new op.
				ConnSeqOp = ClickedOp;
				ConnType = LOC_INPUT;
				ConnIndex = InConnIndex;
				OnContextCopyConnections(CutEvent);

				ConnSeqOp = NewSeqOp;
				ConnType = LOC_INPUT;
				ConnIndex = 0;
				OnContextPasteConnections(PasteEvent);

				// Make connection from clicked input connector to first output of new op.
				MakeLogicConnection(NewSeqOp, 0, ClickedOp, InConnIndex);
			}
		}

		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

void WxKismet::DrawObjects(FViewport* Viewport, FCanvas* Canvas)
{
	WxLinkedObjEd::DrawObjects( Viewport, Canvas );

	// Draw valid region.
	DrawTile(Canvas, -10*MaxSequenceSize, -10*MaxSequenceSize, 20*MaxSequenceSize, 9*MaxSequenceSize, 0.f, 0.f, 0.f, 0.f, FLinearColor(0,0,0,0.15f));
	DrawTile(Canvas, -10*MaxSequenceSize, -MaxSequenceSize, 9*MaxSequenceSize, 2*MaxSequenceSize, 0.f, 0.f, 0.f, 0.f, FLinearColor(0,0,0,0.15f));
	DrawTile(Canvas, -10*MaxSequenceSize, MaxSequenceSize, 20*MaxSequenceSize, 9*MaxSequenceSize, 0.f, 0.f, 0.f, 0.f, FLinearColor(0,0,0,0.15f));
	DrawTile(Canvas, MaxSequenceSize, -MaxSequenceSize, 9*MaxSequenceSize, 2*MaxSequenceSize, 0.f, 0.f, 0.f, 0.f, FLinearColor(0,0,0,0.15f));
	DrawLine(Canvas, FIntPoint(MaxSequenceSize, MaxSequenceSize), FIntPoint(MaxSequenceSize, -MaxSequenceSize), FColor(0,0,0));
	DrawLine(Canvas, FIntPoint(MaxSequenceSize, -MaxSequenceSize), FIntPoint(-MaxSequenceSize, -MaxSequenceSize), FColor(0,0,0));
	DrawLine(Canvas, FIntPoint(-MaxSequenceSize, -MaxSequenceSize), FIntPoint(-MaxSequenceSize, MaxSequenceSize), FColor(0,0,0));
	DrawLine(Canvas, FIntPoint(-MaxSequenceSize, MaxSequenceSize), FIntPoint(MaxSequenceSize, MaxSequenceSize), FColor(0,0,0));

	INT MouseOverIndex = INDEX_NONE;
	USequenceObject* MouseOverSeqObj = NULL;
	if(LinkedObjVC->MouseOverObject)
	{
		MouseOverSeqObj = Cast<USequenceObject>(LinkedObjVC->MouseOverObject);

		if (MouseOverSeqObj != NULL)
		{
			// See if its a SequenceOp.
			USequenceOp* MouseOverSeqOp = Cast<USequenceOp>(MouseOverSeqObj);
			if(MouseOverSeqOp)
			{
				// Need to convert from visible index to actual connector index.
				MouseOverIndex = MouseOverSeqOp->VisibleIndexToActualIndex(LinkedObjVC->MouseOverConnType, LinkedObjVC->MouseOverConnIndex);
			}
		}
	}

	if(Sequence)
	{
		Sequence->DrawSequence(Canvas, SelectedSeqObjs, MouseOverSeqObj, LinkedObjVC->MouseOverConnType, MouseOverIndex, appSeconds() - LinkedObjVC->MouseOverTime, bDrawCurves);
	}
}

void WxKismet::UpdatePropertyWindow()
{
	PropertyWindow->SetObjectArray( SelectedSeqObjs, 1,1,0 );
}

void WxKismet::EmptySelection()
{
	DuplicationCount = 0;
	SelectedSeqObjs.Empty();
	if (PropertyWindow != NULL)
	{
		PropertyWindow->SetObject(NULL,FALSE,FALSE,FALSE);
	}
}

void WxKismet::AddToSelection( UObject* Obj )
{
	checkSlow( Obj->IsA(USequenceObject::StaticClass()) );
	if ( Obj->IsA(USequenceObject::StaticClass()) )
	{
		SelectedSeqObjs.AddUniqueItem( (USequenceObject*)Obj );
		((USequenceObject*)Obj)->OnSelected();
	}
}

void WxKismet::RemoveFromSelection( UObject* Obj )
{
	checkSlow( Obj->IsA(USequenceObject::StaticClass()) );
	if (Obj->IsA(USequenceObject::StaticClass()))
	{
		SelectedSeqObjs.RemoveItem( (USequenceObject*)Obj );
	}
}

UBOOL WxKismet::IsInSelection( UObject* Obj ) const
{
	checkSlow( Obj->IsA(USequenceObject::StaticClass()) );
	return (Obj->IsA(USequenceObject::StaticClass()) && SelectedSeqObjs.ContainsItem( (USequenceObject*)Obj ));
}

INT WxKismet::GetNumSelected() const
{
	return SelectedSeqObjs.Num();
}

void WxKismet::SetSelectedConnector( FLinkedObjectConnector& Connector )
{
	checkSlow( Connector.ConnObj->IsA(USequenceOp::StaticClass()) );
	if (Connector.ConnObj->IsA(USequenceOp::StaticClass()))
	{
		ConnSeqOp = (USequenceOp*)Connector.ConnObj;
		ConnType = Connector.ConnType;
		ConnIndex = ConnSeqOp->VisibleIndexToActualIndex(Connector.ConnType, Connector.ConnIndex);
	}
	else
	{
		ConnSeqOp = NULL;
	}
}

FIntPoint WxKismet::GetSelectedConnLocation(FCanvas* Canvas)
{
	if (ConnSeqOp == NULL)
	{
		return FIntPoint(0,0);
	}
	else
	{
		return ConnSeqOp->GetConnectionLocation(ConnType, ConnIndex);
	}
}

INT WxKismet::GetSelectedConnectorType()
{
	return ConnType;
}

UBOOL WxKismet::ShouldHighlightConnector(FLinkedObjectConnector& Connector)
{
	checkSlow( Connector.ConnObj->IsA(USequenceOp::StaticClass()) );
	if (Connector.ConnObj->IsA(USequenceOp::StaticClass()))
	{
		USequenceOp* SeqOp = (USequenceOp*)Connector.ConnObj;
		INT ActualIndex = SeqOp->VisibleIndexToActualIndex(Connector.ConnType, Connector.ConnIndex);

		// Always highlight when not making a line.
		if(!LinkedObjVC->bMakingLine)
		{
			return TRUE;
		}

		// If making line from variable of event, can never link to another type of connector.
		if(ConnType == LOC_VARIABLE || ConnType == LOC_EVENT)
		{
			return FALSE;
		}

		// If making line from input/output, can't link to var/event connector
		if(Connector.ConnType == LOC_VARIABLE || Connector.ConnType == LOC_EVENT)
		{
			return FALSE;
		}

		// Can't link connectors of the same type.
		if(Connector.ConnType == ConnType)
		{
			return FALSE;
		}

		// Can't link to yourself.
		if( Connector.ConnObj == ConnSeqOp && 
			ConnType == Connector.ConnType && 
			ConnIndex == ActualIndex)
		{
			return FALSE;
		}
	}
	return TRUE;
}

FColor WxKismet::GetMakingLinkColor()
{
	if(!LinkedObjVC->bMakingLine)
	{
		// Shouldn't really be here!
		return FColor(0,0,0);
	}

	if( ConnType == LOC_INPUT )
	{
		if( ConnSeqOp->InputLinks(ConnIndex).bDisabled )
		{
			return FColor(255,0,0);
		}
		if( ConnSeqOp->InputLinks(ConnIndex).bDisabledPIE )
		{
			return FColor(255,128,0);
		}
		return FColor(0,0,0);
	}
	else if( ConnType == LOC_OUTPUT )
	{
		if( ConnSeqOp->OutputLinks(ConnIndex).bDisabled )
		{
			return FColor(255,0,0);
		}
		if( ConnSeqOp->OutputLinks(ConnIndex).bDisabledPIE )
		{
			return FColor(255,128,0);
		}
		return FColor(0,0,0);
	}
	else if(ConnType == LOC_EVENT)
	{
		// The constant comes from MouseOverColorScale in UnSequenceDraw.cpp. Bit hacky - sorry!
		return FColor( FLinearColor(0.8f,0,0) );
	}
	else // variable connector
	{
		return FColor( FLinearColor(ConnSeqOp->GetVarConnectorColor(ConnIndex)) * 0.8f );
	}
}

void WxKismet::MakeConnectionToConnector( FLinkedObjectConnector& Connector )
{
	check( Connector.ConnObj->IsA(USequenceOp::StaticClass()) );
	USequenceOp* EndSeqOp = (USequenceOp*)Connector.ConnObj;
	INT ActualEndIndex = EndSeqOp->VisibleIndexToActualIndex(Connector.ConnType, Connector.ConnIndex);

	if(ConnType == LOC_INPUT && Connector.ConnType == LOC_OUTPUT)
		MakeLogicConnection((USequenceOp*)Connector.ConnObj, ActualEndIndex, ConnSeqOp, ConnIndex);
	else if(ConnType == LOC_OUTPUT && Connector.ConnType == LOC_INPUT)
		MakeLogicConnection(ConnSeqOp, ConnIndex, (USequenceOp*)Connector.ConnObj, ActualEndIndex);

	NotifyObjectsChanged();
}

void WxKismet::MakeConnectionToObject( UObject* EndObj )
{
	if (ConnType == LOC_VARIABLE)
	{
		USequenceVariable* SeqVar = Cast<USequenceVariable>(EndObj);
		if (SeqVar != NULL)
		{
			MakeVariableConnection(ConnSeqOp, ConnIndex, SeqVar);
		}
	}
	else
	if (ConnType == LOC_EVENT)
	{
		USequenceEvent *SeqEvt = Cast<USequenceEvent>(EndObj);
		if (SeqEvt != NULL)
		{
			FScopedObjectStateChange Notifier(ConnSeqOp);

			ConnSeqOp->EventLinks(ConnIndex).LinkedEvents.AddUniqueItem(SeqEvt);

			//@todo ronp uiprefabs: how to call OnConnect on any instances that are updated
			SeqEvt->OnConnect(ConnSeqOp,ConnIndex);
		}
	}

	NotifyObjectsChanged();
}

/* epic ===============================================
* ::PositionSelectedObjects
*
* Called once the mouse is released after moving objects,
* used to snap things to the grid.
*
* =====================================================
*/
void WxKismet::PositionSelectedObjects()
{
	for (INT Idx = 0; Idx < SelectedSeqObjs.Num(); Idx++)
	{
		USequenceObject *SeqObj = SelectedSeqObjs(Idx);
		
		FScopedObjectStateChange Notifier(SeqObj);

		SeqObj->SnapPosition(KISMET_GRIDSIZE, MaxSequenceSize);

		if ( !SeqObj->HasAnyFlags(RF_ArchetypeObject) )
		{
			Notifier.CancelEdit();
		}
	}
}

/**
 * Called when the user releases the mouse over a link connector and is holding the ALT key.
 * Commonly used as a shortcut to breaking connections.
 *
 * @param	Connector	The connector that was ALT+clicked upon.
 */
void WxKismet::AltClickConnector(FLinkedObjectConnector& Connector)
{
	wxCommandEvent DummyEvent( 0, IDM_KISMET_BREAK_LINK_ALL );
	OnContextBreakLink( DummyEvent );
}

/* epic ===============================================
* ::MoveSelectedObjects
*
* Updates the position of all objects selected with
* mouse movement.
*
* =====================================================
*/
void WxKismet::MoveSelectedObjects( INT DeltaX, INT DeltaY )
{
	for(INT i=0; i<SelectedSeqObjs.Num(); i++)
	{
		USequenceObject* SeqObj = SelectedSeqObjs(i);

		SeqObj->ObjPosX += DeltaX;
		SeqObj->ObjPosY += DeltaY;
	}
}

void WxKismet::EdHandleKeyInput(FViewport* Viewport, FName Key, EInputEvent Event)
{
	UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);
	UBOOL bShiftDown = Viewport->KeyState(KEY_LeftShift) || Viewport->KeyState(KEY_RightShift);
	UBOOL bAltDown = Viewport->KeyState(KEY_LeftAlt) || Viewport->KeyState(KEY_RightAlt);

	if( Event == IE_Pressed )
	{		
		if( Key == KEY_Delete )
		{
			DeleteSelected();
		}
		else if( (bCtrlDown && Key == KEY_W) || (bShiftDown && Key == KEY_D) )
		{
			// preserve the duplication count despite the selection change
			// (apologies for the crappiness of this code, such a nightmare to fix w/o breaking normal pasting, grr)
			INT OldDuplicationCount = DuplicationCount;
			KismetCopy();
			PasteCount = OldDuplicationCount;
			KismetPaste();
			DuplicationCount = ++OldDuplicationCount;
		}
		else if( Key == KEY_BackSpace )
		{
			CenterViewOnSeqObj(Sequence);
		}
		else if( bCtrlDown && Key == KEY_C )
		{
			KismetCopy();
		}
		else if( bCtrlDown && Key == KEY_V )
		{
			KismetPaste();
		}
		else if( bCtrlDown && Key == KEY_X )
		{
			KismetCopy();
			DeleteSelected(TRUE);
		}
		else if( bCtrlDown && Key == KEY_Z )
		{
			KismetUndo();
		}
		else if( bCtrlDown && Key == KEY_Y )
		{
			KismetRedo();
		}
		else if( Key == KEY_C )
		{
			if(SelectedSeqObjs.Num() > 0)
			{
				NewSequenceObject( USequenceFrame::StaticClass(), 0, 0 );
			}
		}
		else if( Key == KEY_A )
		{
			wxCommandEvent DummyEvent;
			OnButtonZoomToFit( DummyEvent );
			RefreshViewport();
		}
		else if( bCtrlDown && Key == KEY_Tab )
		{
			if(OldSequence)
			{
				ChangeActiveSequence(OldSequence);
			}
		}
		else if (Key == KEY_Equals || Key == KEY_Add)
		{
			if (SelectedSeqObjs.Num() > 0)
			{
				ShowConnectors(SelectedSeqObjs);
			}
			else
			{
				ShowConnectors(Sequence->SequenceObjects);
			}
		}
		else if (Key == KEY_Underscore || Key == KEY_Subtract)
		{
			if (SelectedSeqObjs.Num() > 0)
			{
				HideConnectors(Sequence,SelectedSeqObjs);
			}
			else
			{
				HideConnectors(Sequence,Sequence->SequenceObjects);
			}
		}
		else if ( Key == KEY_PageUp || Key == KEY_PageDown )
		{
			// PageUp is bring to front, PageDown is send to back.
			if ( SelectedSeqObjs.Num() == 1 )
			{
				USequenceObject* SeqObj	= SelectedSeqObjs(0);
				const INT Index			= Sequence->SequenceObjects.FindItemIndex( SeqObj );
				if ( Index != INDEX_NONE )
				{
					const INT TargetIndex = (Key == KEY_PageUp) ? Sequence->SequenceObjects.Num()-1 : 0;
					const FScopedTransaction Transaction( *LocalizeUnrealEd("LinkedObjectModify") );
					Sequence->Modify();
					Sequence->SequenceObjects.SwapItems( Index, TargetIndex );
					RefreshViewport();
				}
			}
		}
	}
	else if ( Event == IE_Released )
	{
		// LMBRelease + hotkey performs certain actions.
		if( Key == KEY_LeftMouseButton )
		{
			if( Viewport->KeyState(KEY_R) )
			{
				// Create a remote event and "Activate Remote Event" action pair.
				WxDlgGenericStringEntry dlg;
				const INT Result = dlg.ShowModal( TEXT("NewRemoteEvent"), TEXT("EventName"), TEXT("NewEventName") );
				if ( Result == wxID_OK )
				{
					const FString EventNameString( dlg.GetEnteredString() );
					if( !EventNameString.Len() )
					{
						appMsgf(AMT_OK,*LocalizeUnrealEd("Error_InvalidInput"));
					}
					else
					{
						const FName EventName( *EventNameString );
						const INT NewPosX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
						const INT NewPosY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;

						const FScopedTransaction Transaction( *LocalizeUnrealEd("NewRemoteEvent") );
						USeqEvent_RemoteEvent* RemoteEvent = static_cast<USeqEvent_RemoteEvent*>( NewSequenceObject( USeqEvent_RemoteEvent::StaticClass(), NewPosX, NewPosY ) );
						RemoteEvent->EventName = EventName;
						RemoteEvent->ObjComment = EventNameString;
						RemoteEvent->PostEditChange( NULL );
						USeqAct_ActivateRemoteEvent* ActivateRemoteEvent = static_cast<USeqAct_ActivateRemoteEvent*>( NewSequenceObject( USeqAct_ActivateRemoteEvent::StaticClass(), NewPosX+150, NewPosY ) );
						ActivateRemoteEvent->EventName = EventName;
						ActivateRemoteEvent->ObjComment = EventNameString;
						ActivateRemoteEvent->PostEditChange( NULL );
						Sequence->UpdateNamedVarStatus();
					}
				}
			}
		}
	}
}

void WxKismet::OnMouseOver(UObject* Obj)
{
	USequenceObject* SeqObj = NULL;
	if(Obj)
	{
		SeqObj = CastChecked<USequenceObject>(Obj);
	}

	StatusBar->SetMouseOverObject( SeqObj );
}

void WxKismet::BeginTransactionOnSelected()
{
	GEditor->BeginTransaction( *LocalizeUnrealEd("LinkedObjectModify") );

	for(INT i=0; i<SelectedSeqObjs.Num(); i++)
	{
		USequenceObject* SeqObj = SelectedSeqObjs(i);
		SeqObj->Modify();
	}
}

void WxKismet::EndTransactionOnSelected()
{
	GEditor->EndTransaction();
}

/*-----------------------------------------------------------------------------
	WxKismet
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxKismet, WxLinkedObjEd )
	EVT_CLOSE( WxKismet::OnClose )
	EVT_SIZE( WxKismet::OnSize )
	EVT_MENU_RANGE(IDM_NEW_SEQUENCE_OBJECT_START, IDM_NEW_SEQUENCE_OBJECT_END, WxKismet::OnContextNewScriptingObject)
	EVT_MENU_RANGE(IDM_NEW_SEQUENCE_EVENT_START, IDM_NEW_SEQUENCE_EVENT_END, WxKismet::OnContextNewScriptingEventContext)
	EVT_MENU(IDM_KISMET_UPDATE_ACTION, WxKismet::OnContextUpdateAction)
	EVT_MENU( IDM_KISMET_NEW_VARIABLE_OBJ_CONTEXT, WxKismet::OnContextNewScriptingObjVarContext)
	EVT_MENU(IDM_KISMET_CREATE_LINKED_VARIABLE, WxKismet::OnContextCreateLinkedVariable)
	EVT_MENU(IDM_KISMET_CLEAR_VARIABLE, WxKismet::OnContextClearVariable)
	EVT_MENU(IDM_KISMET_BREAK_LINK_ALL, WxKismet::OnContextBreakLink)
	EVT_MENU(IDM_KISMET_TOGGLE_DISABLE_LINK, WxKismet::OnContextToggleLink)
	EVT_MENU(IDM_KISMET_TOGGLE_DISABLE_LINK_PIE, WxKismet::OnContextToggleLinkPIE)
	EVT_MENU(IDM_KISMET_COPY_CONNECTIONS, WxKismet::OnContextCopyConnections)
	EVT_MENU(IDM_KISMET_CUT_CONNECTIONS, WxKismet::OnContextCopyConnections)
	EVT_MENU(IDM_KISMET_PASTE_CONNECTIONS, WxKismet::OnContextPasteConnections)
	EVT_MENU_RANGE(IDM_KISMET_BREAK_LINK_START, IDM_KISMET_BREAK_LINK_END, WxKismet::OnContextBreakLink)
	EVT_MENU(IDM_KISMET_RENAMESEQ, WxKismet::OnContextRenameSequence)
	EVT_MENU(IDM_KISMET_DELETE_OBJECT, WxKismet::OnContextDelSeqObject)
	EVT_MENU(IDM_KISMET_SELECT_EVENT_ACTORS, WxKismet::OnContextSelectEventActors)
	EVT_MENU(IDM_KISMET_FIRE_EVENT, WxKismet::OnContextFireEvent)
	EVT_MENU(IDM_KISMET_OPEN_INTERPEDIT, WxKismet::OnContextOpenInterpEdit)
	EVT_MENU(IDM_KISMET_SWITCH_ADD, WxKismet::OnContextSwitchAdd)
	EVT_MENU(IDM_KISMET_SWITCH_REMOVE, WxKismet::OnConextSwitchRemove)
	EVT_MENU(IDM_KISMET_BUTTON_ZOOMTOFIT, WxKismet::OnButtonZoomToFit)
	EVT_MENU(IDM_KISMET_BREAK_ALL_OP_LINKS, WxKismet::OnContextBreakAllOpLinks)
	EVT_MENU(IDM_KISMET_HIDE_CONNECTORS, WxKismet::OnContextHideConnectors)
	EVT_MENU(IDM_KISMET_SHOW_CONNECTORS, WxKismet::OnContextShowConnectors)
	EVT_MENU(IDM_KISMET_FIND_NAMEDVAR_USES, WxKismet::OnContextFindNamedVarUses)
	EVT_MENU(IDM_KISMET_FIND_NAMEDVAR, WxKismet::OnContextFindNamedVarDefs)
	EVT_MENU(IDM_KISMET_CREATE_SEQUENCE, WxKismet::OnContextCreateSequence)
	EVT_MENU(IDM_KISMET_EXPORT_SEQUENCE, WxKismet::OnContextExportSequence)
	EVT_MENU(IDM_KISMET_IMPORT_SEQUENCE, WxKismet::OnContextImportSequence)
	EVT_MENU(IDM_KISMET_PASTE_HERE, WxKismet::OnContextPasteHere)
	EVT_MENU(IDM_KISMET_LINK_ACTOR_TO_EVT, WxKismet::OnContextLinkEvent)
	EVT_MENU(IDM_KISMET_LINK_ACTOR_TO_OBJ, WxKismet::OnContextLinkObject)
	EVT_MENU(IDM_KISMET_SELECT_ASSIGNED, WxKismet::OnContextSelectAssigned)
	EVT_MENU(IDM_KISMET_SEARCH_REMOTEEVENTS, WxKismet::OnContextSearchRemoteEvents)
	EVT_MENU(IDM_KISMET_INSERT_ACTOR_TO_OBJLIST, WxKismet::OnContextInsertIntoObjectList)
	EVT_MENU(IDM_KISMET_REMOVE_ACTOR_FROM_OBJLIST, WxKismet::OnContextInsertIntoObjectList)
	EVT_TOOL(IDM_KISMET_PARENTSEQ, WxKismet::OnButtonParentSequence)
	EVT_TOOL(IDM_KISMET_RENAMESEQ, WxKismet::OnButtonRenameSequence)
	EVT_TOOL(IDM_KISMET_BUTTON_HIDE_CONNECTORS, WxKismet::OnButtonHideConnectors)
	EVT_TOOL(IDM_KISMET_BUTTON_SHOW_CONNECTORS, WxKismet::OnButtonShowConnectors)
	EVT_TOOL(IDM_KISMET_BUTTON_SHOW_CURVES, WxKismet::OnButtonShowCurves)
	EVT_TOOL(IDM_KISMET_OPENSEARCH, WxKismet::OnOpenSearch)
	EVT_TOOL(IDM_KISMET_OPENUPDATE, WxKismet::OnOpenUpdate)
	EVT_TOOL(IDM_KISMET_OPENNEWWINDOW, WxKismet::OnOpenNewWindow)
	EVT_TREE_SEL_CHANGED( IDM_LINKEDOBJED_TREE, WxKismet::OnTreeItemSelected )
	EVT_TREE_ITEM_EXPANDING( IDM_LINKEDOBJED_TREE, WxKismet::OnTreeExpanding )
	EVT_TREE_ITEM_COLLAPSED( IDM_LINKEDOBJED_TREE, WxKismet::OnTreeCollapsed )
	EVT_BUTTON(IDM_KISMET_DOSEARCH, WxKismet::OnDoSearch )
	EVT_TEXT_ENTER(IDM_KISMET_SEARCHENTRY, WxKismet::OnDoSearch )
	EVT_LISTBOX( IDM_KISMET_SEARCHRESULT, WxKismet::OnSearchResultChanged )
	EVT_MENU_RANGE(IDM_KISMET_EXPOSE_VARIABLE_START, IDM_KISMET_EXPOSE_VARIABLE_END, WxKismet::OnContextExposeVariable)
	EVT_MENU(IDM_KISMET_REMOVE_VARIABLE, WxKismet::OnContextRemoveVariable)
	EVT_MENU_RANGE(IDM_KISMET_EXPOSE_OUTPUT_START, IDM_KISMET_EXPOSE_OUTPUT_END, WxKismet::OnContextExposeOutput)
	EVT_MENU(IDM_KISMET_REMOVE_OUTPUT, WxKismet::OnContextRemoveOutput)
	EVT_MENU(IDM_KISMET_SET_OUTPUT_DELAY, WxKismet::OnContextSetOutputDelay)
	EVT_MENU(IDM_KISMET_SET_INPUT_DELAY, WxKismet::OnContextSetInputDelay)
	EVT_MENU_RANGE(IDM_KISMET_APPLY_COMMENT_STYLE_START, IDM_KISMET_APPLY_COMMENT_STYLE_END, WxKismet::OnContextApplyCommentStyle)
	EVT_MENU(IDM_KISMET_COMMENT_TO_FRONT, WxKismet::OnContextCommentToFront)
	EVT_MENU(IDM_KISMET_COMMENT_TO_BACK, WxKismet::OnContextCommentToBack)
END_EVENT_TABLE()

IMPLEMENT_COMPARE_POINTER( UClass, Kismet_SequenceClass,
{
	USequenceObject* ADef = A->GetDefaultObject<USequenceObject>();
	USequenceObject* BDef = B->GetDefaultObject<USequenceObject>();

	return appStricmp( *(ADef->ObjCategory + ADef->ObjName), *(BDef->ObjCategory + BDef->ObjName) );
})

/** Fills in array of all available USequenceObject classes and sorts them alphabetically. */
void WxKismet::InitSeqObjClasses()
{
	SeqObjClasses.Empty();

	// Construct list of non-abstract usable gameplay sequence object classes.
	UUnrealEdEngine* Editor = Cast<UUnrealEdEngine>(GEngine);
	for(TObjectIterator<UClass> It; It; ++It)
	{
		if ( It->IsChildOf(USequenceObject::StaticClass()) && IsValidSequenceClass(*It) && (Editor == NULL || !Editor->HiddenKismetClassNames.ContainsItem(It->GetFName())))
		{
			SeqObjClasses.AddItem(*It);
		}
	}

	Sort<USE_COMPARE_POINTER( UClass,Kismet_SequenceClass)>(&SeqObjClasses(0), SeqObjClasses.Num() );
}

/**
 * Returns whether the specified class can be displayed in the list of ops which are available to be placed.
 *
 * @param	SequenceClass	a child class of USequenceObject
 *
 * @return	TRUE if the specified class should be available in the context menus for adding sequence ops
 */
UBOOL WxKismet::IsValidSequenceClass( UClass* SequenceClass ) const
{
	UBOOL bResult = FALSE;
	if ( SequenceClass != NULL && !SequenceClass->HasAnyClassFlags(CLASS_Abstract|CLASS_Deprecated) )
	{
		USequenceObject* SequenceCDO = SequenceClass->GetDefaultObject<USequenceObject>();
		check(SequenceCDO != NULL);

		bResult = SequenceCDO->eventIsValidLevelSequenceObject();
	}

	return bResult;
}


/** 
 * Opens a kismet window to edit the specified sequence.  If InSequence is NULL, the kismet
 * of the current level will be opened, or created if it does not yet exist.
 *
 * @param	InSequence			The sequence to open.  If NULL, open the current level's root sequence.
 * @param	bCreateNewWindow	If TRUE, open the sequence in a new kismet window.
 * @param	WindowParent		When a window is being created, use this as its parent.
 */
void WxKismet::OpenKismet(USequence* InSequence, UBOOL bCreateNewWindow, wxWindow* WindowParent)
{
	// If no sequence was specified, use the root sequence of the current level.
	if ( InSequence == NULL )
	{
		InSequence = GWorld->GetGameSequence();
		if ( InSequence == NULL )
		{
			// The current level contains no sequence -- create a new one.
			InSequence = ConstructObject<USequence>( USequence::StaticClass(), GWorld->CurrentLevel, TEXT("Main_Sequence"), RF_Transactional );
			GWorld->SetGameSequence( InSequence );
			if( !GWorld->GetGameSequence() )
			{
				return;
			}
		}
	}

	check(InSequence);
	InSequence->UpdateNamedVarStatus();

	if ( bCreateNewWindow || GApp->KismetWindows.Num() == 0 )
	{
		// Create a new window if requested by the user or if none exist.
		WxKismet* SequenceEditor = new WxKismet( WindowParent, -1, InSequence );
		SequenceEditor->InitEditor();
		SequenceEditor->Show();
	}
	else
	{
		// Bring all kismet windows to focus.
		for (INT Idx = 0; Idx < GApp->KismetWindows.Num(); Idx++)
		{
			WxKismet* KismetWindow = GApp->KismetWindows(Idx);
			KismetWindow->Show();
			KismetWindow->SetFocus();
		}
	}
}

/** 
 *	Searches the current levels Kismet sequences to find any references to supplied Actor.
 *	Opens a Kismet window if none are currently open, or re-uses first one. 
 *  Then issues a search and centers each kismet window on the first result.
 *
 *	@param FindActor	Actor to find references to.
 *
 * @fixme ui kismet
 */
void WxKismet::FindActorReferences(AActor* FindActor)
{
	// If no Kismet windows open - open one now.
	if( GApp->KismetWindows.Num() == 0 )
	{
		WxKismet::OpenKismet( NULL, FALSE, GApp->EditorFrame );
	}
	check( GApp->KismetWindows.Num() > 0 );

	ULevel* Level = FindActor->GetLevel();
	USequence* LevelSequence = GWorld->GetGameSequence( Level );

	//search in each kismet window
	for ( INT KismetWindowIndex = 0 ; KismetWindowIndex < GApp->KismetWindows.Num() ; ++KismetWindowIndex )
	{
		WxKismet* KismetWindow = GApp->KismetWindows(KismetWindowIndex);

		// Set the workspace of the kismet window to be the sequence for the actor's level.
		KismetWindow->ChangeActiveSequence( LevelSequence, TRUE );
		
		TArray<USequenceObject*> Results;
		//conduct the search
		EKismetSearchScope SearchScope = KSS_CurrentLevel;
		KismetWindow->DoSilentSearch(Results, *FindActor->GetName(), KST_ObjName, SearchScope );

		//if there are one or more results, center the view on the first result
		if (Results.Num() == 1)
		{
			KismetWindow->CenterViewOnSeqObj( Results(0) );
		}
		else if (Results.Num() > 1)
		{
			// re-run the search, this time using the search window
			// This will handle opening search window, jumping to first result etc.
			KismetWindow->DoSearch( *FindActor->GetName(), KST_ObjName, TRUE, SearchScope );
		}
	}
}

/** 
 *	Open a Kismet window to edit the supplied Sequence.
 *	Opens a Kismet window if none are currently open, or re-uses first one.
 */
void WxKismet::OpenSequenceInKismet(USequence* InSeq,wxWindow* WindowParent/*=GApp->EditorFrame*/)
{
	// If no Kismet windows open - open one now.
	if( GApp->KismetWindows.Num() == 0 )
	{
		WxKismet::OpenKismet(InSeq,FALSE,WindowParent);
	}
	check( GApp->KismetWindows.Num() > 0 );

	WxKismet* UseKismet = GApp->KismetWindows(0);

	UseKismet->ChangeActiveSequence(InSeq);
}

/**
 * Activates the passed in sequence for editing.
 *
 * @param	NewSeq - new sequence to edit
 */
void WxKismet::ChangeActiveSequence(USequence *NewSeq, UBOOL bNotifyTree)
{
	if (NewSeq != NULL)
	{
		if (NewSeq != Sequence)
		{
			// clear all existing refs
			if (LinkedObjVC != NULL)
			{
				LinkedObjVC->MouseOverObject = NULL;
			}
			SelectedSeqObjs.Empty();
			// update any connectors on both sequences, to catch recent changes
			if (Sequence != NULL)
			{
				Sequence->UpdateConnectors();
			}
			NewSeq->UpdateConnectors();

			// remember the last sequence we were on.
			OldSequence = Sequence;

			// and set the new sequence
			Sequence = NewSeq;

			// Slight hack to ensure all Sequences and SequenceObjects are RF_Transactional
			Sequence->SetFlags( RF_Transactional );

			for (INT Idx = 0; Idx < Sequence->SequenceObjects.Num(); Idx++)
			{
				USequenceObject *Obj = Sequence->SequenceObjects(Idx);
				if (Obj != NULL)
				{
					Obj->SetFlags( RF_Transactional );

					// Make sure all sequence objects have sane positions!
					Obj->ObjPosX = Clamp(Obj->ObjPosX, -MaxSequenceSize, MaxSequenceSize);
					Obj->ObjPosY = Clamp(Obj->ObjPosY, -MaxSequenceSize, MaxSequenceSize);

					// verify named variables
					USeqVar_Named *NamedVar = Cast<USeqVar_Named>(Obj);
					if (NamedVar != NULL)
					{
						NamedVar->UpdateStatus();
					}
				}
			}
		}

		const FString SequenceTitle = Sequence->GetSeqObjFullName();
		SetTitle( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("KismetCaption_F"), *SequenceTitle) ) );

		// Set position the last remember 
		LinkedObjVC->Origin2D.X = Sequence->DefaultViewX;
		LinkedObjVC->Origin2D.Y = Sequence->DefaultViewY;
		LinkedObjVC->Zoom2D = Sequence->DefaultViewZoom;

		// When changing sequence, clear selection (don't want to have things selected that aren't in current sequence)
		EmptySelection();

		NewSeq->UpdateNamedVarStatus();

		if(bNotifyTree)
		{
			WxSequenceTreeCtrl* TreeCtrl = wxDynamicCast(TreeControl,WxSequenceTreeCtrl);
			if ( TreeCtrl != NULL )
			{
				// If this is initial call to ChangeActiveSequence, we may not have built the tree at all yet...
				TreeCtrl->RefreshTree();
				TreeCtrl->SelectObject(Sequence);
			}
		}

		// Make this sequence's level current.
		ULevel* LevelToMakeCurrent = NewSeq->GetLevel();
		if ( LevelToMakeCurrent && LevelToMakeCurrent != GWorld->CurrentLevel )
		{
			WxLevelBrowser* LevelBrowser = GUnrealEd->GetBrowser<WxLevelBrowser>( TEXT("LevelBrowser") );
			LevelBrowser->MakeLevelCurrent( LevelToMakeCurrent );
		}

		ClearCopiedConnections();

		RefreshViewport();
	}
}

/** Switch Kismet to the Sequence containing the supplied object and move view to center on it. */
void WxKismet::CenterViewOnSeqObj(USequenceObject* ViewObj)
{
	// Get the parent sequence of this object
	USequence* ParentSeq = ViewObj->ParentSequence;

	// Can't do anything if this object has no parent...
	if(ParentSeq)
	{
		// Check this 
		if(!ParentSeq->SequenceObjects.ContainsItem(ViewObj))
		{
			appMsgf( AMT_OK, TEXT("Warning! Object '%s' Has Incorrect ParentSequence '%s'"), *ViewObj->GetName(), *ParentSeq->GetName() );
		}

		// Change view to that sequence
		ChangeActiveSequence(ParentSeq, TRUE);

		// Then adjust origin to put selected SequenceObject at the center.
		INT SizeX = GraphWindow->GetSize().x;
		INT SizeY = GraphWindow->GetSize().y;

		FLOAT DrawOriginX = (ViewObj->ObjPosX + 20)- ((SizeX*0.5f)/LinkedObjVC->Zoom2D);
		FLOAT DrawOriginY = (ViewObj->ObjPosY + 20)- ((SizeY*0.5f)/LinkedObjVC->Zoom2D);

		LinkedObjVC->Origin2D.X = -(DrawOriginX * LinkedObjVC->Zoom2D);
		LinkedObjVC->Origin2D.Y = -(DrawOriginY * LinkedObjVC->Zoom2D);
		ViewPosChanged();

		// Select object we are centering on.
		EmptySelection();
		AddToSelection(ViewObj);

		UpdatePropertyWindow();

		// Update view
		RefreshViewport();
	}
}

/** Change view to encompass selection. This will not adjust zoom beyone the 0.1 - 1.0 range though. */
void WxKismet::ZoomSelection()
{
	FIntRect SelectedBox = CalcBoundingBoxOfSelected();
	if(SelectedBox.Area() > 0)
	{
		ZoomToBox(SelectedBox);
	}
}

/** Change view to encompass all objects in current sequence. This will not adjust zoom beyone the 0.1 - 1.0 range though. */
void WxKismet::ZoomAll()
{
	FIntRect AllBox = CalcBoundingBoxOfAll();
	if(AllBox.Area() > 0)
	{
		ZoomToBox(AllBox);
	}
}

/** Change view to encompass the supplied box. This will not adjust zoom beyone the 0.1 - 1.0 range though. */
void WxKismet::ZoomToBox(const FIntRect& Box)
{
	// Get center point and radius of supplied box.
	FIntPoint Center, Extent;
	Box.GetCenterAndExtents(Center, Extent);
	Extent += FIntPoint(20, 20); // A little padding around the outside of selection.
		
	// Calculate the zoom factor we need to fit the box onto the screen.
	INT SizeX = GraphWindow->GetSize().x;
	INT SizeY = GraphWindow->GetSize().y;

	FLOAT NewZoomX = ((FLOAT)SizeX)/((FLOAT)(2 * Extent.X));
	FLOAT NewZoomY = ((FLOAT)SizeY)/((FLOAT)(2 * Extent.Y));
	FLOAT NewZoom = ::Min(NewZoomX, NewZoomY);
	LinkedObjVC->Zoom2D = Clamp(NewZoom, 0.1f, 1.0f);

	// Set origin to center on box center.
	FLOAT DrawOriginX = Center.X - ((SizeX*0.5f)/LinkedObjVC->Zoom2D);
	FLOAT DrawOriginY = Center.Y - ((SizeY*0.5f)/LinkedObjVC->Zoom2D);

	LinkedObjVC->Origin2D.X = -(DrawOriginX * LinkedObjVC->Zoom2D);
	LinkedObjVC->Origin2D.Y = -(DrawOriginY * LinkedObjVC->Zoom2D);

	// Notify that view has changed.
	ViewPosChanged();
}

/** Handler for 'zoom to fit' button. Will fit to selection if there is one, or entire sequence otherwise. */
void WxKismet::OnButtonZoomToFit(wxCommandEvent &In)
{
	if(SelectedSeqObjs.Num() == 0)
	{
		ZoomAll();
	}
	else
	{
		ZoomSelection();
	}
	RefreshViewport();
}

/** Close all currently open Kismet windows. Uses the KismetWindows array in GApp.  */
void WxKismet::CloseAllKismetWindows()
{
	// Because the Kismet destructor removes itself from the array, we need to copy the array first.
	// Destructor may not be called on Close - it happens as part of the wxWindows idle process.

	TArray<WxKismet*> KisWinCopy;
	KisWinCopy.Add( GApp->KismetWindows.Num() );
	for(INT i=0; i<GApp->KismetWindows.Num(); i++)
	{
		KisWinCopy(i) = GApp->KismetWindows(i);
	}

	for(INT i=0; i<KisWinCopy.Num(); i++)
	{
		KisWinCopy(i)->Close(TRUE);
	}
}

/** 
 *	Iterate over all Kismet sequences and make sure that none are editing the supplied Sequence (or a subsequence of supplied sequence).
 *	If it finds one, it sets it to the root sequence instead.
 */
void WxKismet::EnsureNoKismetEditingSequence(USequence* InSeq)
{
	// Do nothing if no sequence is passed in.
	if(!InSeq)
	{
		return;
	}

	//@fixme - why use an array here?  it seems like there will only ever be one element in the array...
	TArray<USequenceObject*> RemovedSeqs;
	RemovedSeqs.AddItem(InSeq);
	InSeq->FindSeqObjectsByClass(USequence::StaticClass(), RemovedSeqs, TRUE);

	USequence* RootSeq = InSeq->GetRootSequence();
	for(INT i=0; i<GApp->KismetWindows.Num(); i++)
	{
		WxKismet* KisWindow = GApp->KismetWindows(i);
		if( RemovedSeqs.ContainsItem(KisWindow->Sequence) )
		{
			KisWindow->ChangeActiveSequence(RootSeq, TRUE);
		}
	}
}


/** Kismet constructor. Initialise variables, create toolbar and status bar, set view to supplied sequence and add to global list of Kismet windows. */
WxKismet::WxKismet( wxWindow* InParent, wxWindowID InID, class USequence* InSequence, const TCHAR* Title )
: WxLinkedObjEd( InParent, InID, *LocalizeUnrealEd(Title) ), Sequence(InSequence)
{
}

/** Kismet destructor. Will remove itself from global list of Kismet windows. */
WxKismet::~WxKismet()
{
	SaveProperties();

	GCallbackEvent->UnregisterAll(this);

	// Remove from global list of Kismet windows.
	check( GApp->KismetWindows.ContainsItem(this) );
	GApp->KismetWindows.RemoveItem(this);

	GEditor->ResetTransaction( *LocalizeUnrealEd("QuitKismet") );
}

void WxKismet::InitEditor()
{
	CreateControls(TRUE);

	// Register the CALLBACK_RefreshEditor_Kismet event
	GCallbackEvent->Register(CALLBACK_RefreshEditor_Kismet, this);
	GCallbackEvent->Register(CALLBACK_Undo, this);
	
	LoadProperties();

	// Initialise the tree control.
	RebuildTreeControl();
}

/**
 * Creates the controls for this window
 */
void WxKismet::CreateControls( UBOOL bTreeControl )
{
	WxLinkedObjEd::CreateControls(bTreeControl);

	wxMenuBar* MenuBar = new wxMenuBar();
	AppendWindowMenu(MenuBar);
	SetMenuBar(MenuBar);

	OldSequence = NULL;
	bDrawCurves = TRUE;	
	bAttachVarsToConnector = FALSE;
	SearchWindow = NULL;

	ConnSeqOp = NULL;
	ConnType = 0;
	ConnIndex = 0;

	PasteCount = 0;

	CopyConnType = INDEX_NONE;

	// activate the base sequence
	USequence* InitialActiveSequence = Sequence;
	Sequence = NULL;

	ChangeActiveSequence( InitialActiveSequence );

	// Create toolbar
	ToolBar = new WxKismetToolBar( this, -1 );
	SetToolBar( ToolBar );

	// Create status bar
	StatusBar = new WxKismetStatusBar( this, -1 );
	SetStatusBar( StatusBar );

	// load the background texture
	BackgroundTexture = LoadObject<UTexture2D>(NULL, TEXT("EditorMaterials.KismetBackground"), NULL, LOAD_None, NULL);

	InitSeqObjClasses();

	TreeImages->Add( WxBitmap( "KIS_SeqTreeClosed" ), wxColor( 192,192,192 ) ); // 0
	TreeImages->Add( WxBitmap( "KIS_SeqTreeOpen" ), wxColor( 192,192,192 ) ); // 1

	// Keep a list of all Kismets in case we need to close them etc.
	GApp->KismetWindows.AddItem(this);
}

/**
 * Creates the tree control for this linked object editor.  Only called if TRUE is specified for bTreeControl
 * in the constructor.
 *
 * @param	TreeParent	the window that should be the parent for the tree control
 */
void WxKismet::CreateTreeControl( wxWindow* TreeParent )
{
	TreeImages = new wxImageList( 16, 15 );
	TreeControl = new WxSequenceTreeCtrl;
	((WxSequenceTreeCtrl*)TreeControl)->Create(TreeParent, IDM_LINKEDOBJED_TREE, this);
	TreeControl->AssignImageList(TreeImages);
}

/** Called when the window closes (but may be some time before actually destroyed) */
void WxKismet::OnClose( wxCloseEvent& In )
{
	if(SearchWindow)
	{
		SearchWindow->Close();
	}

	SeqObjClasses.Empty();
	ConnSeqOp = NULL;
	SelectedSeqObjs.Empty();
	Sequence = NULL;
	OldSequence = NULL;
	CopyConnInfo.Empty();
	OpExposeVarLinkMap.Empty();

	WxSequenceTreeCtrl* TreeCtrl = wxDynamicCast(TreeControl,WxSequenceTreeCtrl);
	if ( TreeCtrl != NULL )
	{
		TreeCtrl->ClearTree();
	}
	else
	{
		TreeControl->DeleteAllItems();
	}
	this->Destroy();
}

void WxKismet::OnSize(wxSizeEvent& In)
{
	RefreshViewport();
	In.Skip();
}

/** Handle global 'refresh Kismet' events. */
void WxKismet::Send( ECallbackEventType InType )
{
	switch ( InType )
	{
	case CALLBACK_RefreshEditor_Kismet:
		RebuildTreeControl();
		RefreshViewport();
		break;

	case CALLBACK_Undo:
		RebuildTreeControl();
		RefreshViewport();
		UpdatePropertyWindow();
		Sequence->UpdateNamedVarStatus();					// May have changed named variable status
		GetRootSequence()->UpdateInterpActionConnectors();	// May have changed SeqAct_Interp/InterpData connectedness.
		ClearCopiedConnections();
		NotifyObjectsChanged();
		break;
	}
}

/**
 * Creates a link between the ouptut and input of two sequence
 * operations.  First searches for a previous entry in the
 * output links, and if none is found adds a new link.
 * 
 * @param	OutOp - op creating the output link
 * @param	OutConnIndex - index in OutOp's OutputLinks[] to use
 * @param	InOp - op creating the input link
 * @param	InConnIndex - index in InOp's InputLinks[] to use
 */
void WxKismet::MakeLogicConnection(USequenceOp* OutOp, INT OutConnIndex, USequenceOp* InOp, INT InConnIndex)
{
	// Check connector indices are within range before doing anything else.
	if( OutConnIndex >= OutOp->OutputLinks.Num() ||
		InConnIndex >= InOp->InputLinks.Num() )
	{
		return;
	}

	const FScopedTransaction Transaction( *LocalizeUnrealEd("MakeLogicConnection") );
	//OutOp->Modify();
	FScopedObjectStateChange Notifier(OutOp);

	// grab the output link
	FSeqOpOutputLink &outLink = OutOp->OutputLinks(OutConnIndex);
	// make sure the link doesn't already exist
	INT newIdx = -1;
	UBOOL bFoundEntry = 0;
	for (INT linkIdx = 0; linkIdx < outLink.Links.Num() && !bFoundEntry; linkIdx++)
	{
		// if it's an empty entry save the index
		if (outLink.Links(linkIdx).LinkedOp == NULL)
		{
			newIdx = linkIdx;
		}
		else
		// check for a match
		if (outLink.Links(linkIdx).LinkedOp == InOp &&
			outLink.Links(linkIdx).InputLinkIdx == InConnIndex)
		{
			bFoundEntry = 1;
		}
	}
	// if we didn't find an entry, add one
	if (!bFoundEntry)
	{
		// if we didn't find an empty entry, create a new one
		if (newIdx == -1)
		{
			newIdx = outLink.Links.AddZeroed();
		}
		// add the entry
		outLink.Links(newIdx).LinkedOp = InOp;
		outLink.Links(newIdx).InputLinkIdx = InConnIndex;
		// notify the op that it has connected to something
	
		//@todo ronp uiprefabs: how to call OnConnect on any instances that are updated
		OutOp->OnConnect(InOp,OutConnIndex);
	}
	else
	{
		Notifier.CancelEdit();
	}
}

/** 
 *	Checks to see if a particular variable connector may connect to a particular variable. 
 *	Looks at variable type and allowed type of connector.
 */
static UBOOL CanConnectVarToLink(FSeqVarLink* VarLink, USequenceVariable* Var)
{
	// In cases where variable connector has no type (eg. uninitialised external vars), disallow all connections
	if(VarLink->ExpectedType == NULL)
	{
		return FALSE;
	}		

	// Check we are not trying to attach something new to a variable which is at capacity already.
	if(VarLink->LinkedVariables.Num() == VarLink->MaxVars)
	{
		return FALSE;
	}

	
	// Check to make sure we're not already linked to this exact object variable!  There's no need to stack additional
	// variables links to the same port.
	for( INT RefCount = 0; ; ++RefCount )
	{
		UObject** DesiredObjectRef = Var->GetObjectRef( RefCount );
		if( DesiredObjectRef != NULL )
		{
			// Check for an 'object list' variable type first
			if( VarLink->SupportsVariableType( USeqVar_ObjectList::StaticClass() ) )
			{
				for( INT CurLinkIdx = 0; CurLinkIdx < VarLink->LinkedVariables.Num(); ++CurLinkIdx )
				{
					if( VarLink->LinkedVariables( CurLinkIdx ) != NULL )
					{
						USeqVar_ObjectList* TheList = Cast< USeqVar_ObjectList >( VarLink->LinkedVariables( CurLinkIdx ) );
						if( TheList != NULL )
						{
							for( INT CurObjectIndex = 0; CurObjectIndex < TheList->ObjList.Num(); ++CurObjectIndex )
							{
								UObject** ObjectRef = TheList->GetObjectRef( CurObjectIndex );
								if( ObjectRef != NULL )
								{
									if( ObjectRef == DesiredObjectRef )
									{
										// We're already linked to this object
										return FALSE;
									}
								}
							}
						}
					}
				}
			}

			// OK, now check for single object links
			else if( VarLink->SupportsVariableType( USeqVar_Object::StaticClass(), FALSE ) )
			{
				for( INT CurLinkIndex = 0; CurLinkIndex < VarLink->LinkedVariables.Num(); ++CurLinkIndex )
				{
					USequenceVariable* CurLink = VarLink->LinkedVariables( CurLinkIndex );

					if( CurLink != NULL )
					{
						for( INT RefCount = 0; ; ++RefCount )
						{
							UObject** ObjectRef = CurLink->GetObjectRef( RefCount );
							if( ObjectRef != NULL )
							{
								if( ObjectRef == DesiredObjectRef )
								{
									// We're already linked to this object
									return FALSE;
								}
							}
							else
							{
								// No more objects
								break;
							}
						}
					}
				}
			}
		}
		else
		{
			// No more objects
			break;
		}
	}


	// Usual case - connecting normal variable to connector. Check class is as expected.
	if( VarLink->SupportsVariableType(Var->GetClass(),FALSE) )
	{
		return TRUE;
	}

	// Check cases where variables can change type (Named and External variables)
	USeqVar_External* ExtVar = Cast<USeqVar_External>(Var);
	if(ExtVar && (!ExtVar->ExpectedType || ExtVar->ExpectedType == VarLink->ExpectedType))
	{
		return TRUE;
	}

	USeqVar_Named* NamedVar = Cast<USeqVar_Named>(Var);
	if(NamedVar && (!NamedVar->ExpectedType || NamedVar->ExpectedType == VarLink->ExpectedType))
	{
		return TRUE;
	}

	return FALSE;
}

/** Create a connection between the variable connector at index OpConnIndex on the SequenceOp Op to the variable Var. */
void WxKismet::MakeVariableConnection(USequenceOp* Op, INT OpConnIndex, USequenceVariable* Var)
{
	FSeqVarLink* VarLink = &Op->VariableLinks(OpConnIndex);

	// Check the type is correct before making line.
	if( CanConnectVarToLink(VarLink, Var) )
	{
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("MakeLogicConnection") );
			//Op->Modify();
			FScopedObjectStateChange Notifier(Op);
			VarLink->LinkedVariables.AddItem(Var);

			//@todo ronp uiprefabs: how to call OnConnect on any instances that are updated
			Var->OnConnect(Op,OpConnIndex);
		}

		// If connection is of InterpData type, ensure all SeqAct_Interps are up to date.
		if(VarLink->ExpectedType == UInterpData::StaticClass())
		{
			GetRootSequence()->UpdateInterpActionConnectors();
		}
	}
}

/** 
 *	Iterate over selected SequenceObjects and remove them from the sequence.
 *	This will clear any references to those objects by other objects in the sequence.
 */
void WxKismet::DeleteSelected(UBOOL bSilent)
{
	// Make sure any pending property item changes are applied before we go and potentially delete
	// the object out from underneath the property window.
	PropertyWindow->FlushLastFocused();
	PropertyWindow->ClearLastFocused();

	UBOOL bDeletingSequence = FALSE;
	TArray<USequenceObject*> DeletingSeqs;

	// Loop through all selected objects and unselect anything that isn't deletable.
	// make a copy of the array in case there happen to be multiple copies of the same object
	// in the array for some reason.
	TArray<USequenceObject*> ArrayCopy = SelectedSeqObjs;
	for( INT ObjIdx = 0; ObjIdx < ArrayCopy.Num(); ObjIdx++ )
	{
		USequenceObject* DelObj = ArrayCopy(ObjIdx);
		if ( !DelObj->bDeletable )
		{
			RemoveFromSelection(DelObj);
		}
	}

	for(INT ObjIdx=0; ObjIdx<SelectedSeqObjs.Num(); ObjIdx++)
	{
		USequence* DelSeq = Cast<USequence>( SelectedSeqObjs(ObjIdx) );
		if(DelSeq)
		{
			bDeletingSequence = TRUE;
			DeletingSeqs.AddItem(DelSeq);
			DelSeq->FindSeqObjectsByClass(USequence::StaticClass(), DeletingSeqs, TRUE);
		}
	}

	// pop up a confirm when deleting sequences
	if (!bSilent && bDeletingSequence && !appMsgf(AMT_YesNo,*LocalizeUnrealEd("DeleteSequenceConfirm")))
	{
		// abort! abort!
		return;
	}

	if (LinkedObjVC != NULL && Cast<USequenceObject>(LinkedObjVC->MouseOverObject) != NULL &&
		SelectedSeqObjs.ContainsItem((USequenceObject*)(LinkedObjVC->MouseOverObject)))
	{
		LinkedObjVC->MouseOverObject = NULL;
	}

	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd("DeleteSelected") );
		FScopedObjectStateChange Notifier(Sequence);

		Sequence->RemoveObjects(SelectedSeqObjs);
	}

	// Ensure the SearchWindow (if up) doesn't contain references to deleted SequenceObjects.
	if(SearchWindow)
	{
		SearchWindow->ClearResultsList();
	}

	SelectedSeqObjs.Empty();
	UpdatePropertyWindow();

	// May have deleted a named variable, so update their global status.
	Sequence->UpdateNamedVarStatus();
	GetRootSequence()->UpdateInterpActionConnectors();

	// We see if there are any PrefabInstances referring to any Sequences we are removing.
	// If they do - set the pointer to NULL.
	for(TObjectIterator<APrefabInstance> It; It; ++It)
	{
		APrefabInstance* PrefabInst = *It;
		if( !PrefabInst->bDeleteMe && 
			!PrefabInst->IsPendingKill() &&
			PrefabInst->SequenceInstance &&
			DeletingSeqs.ContainsItem(PrefabInst->SequenceInstance) )
		{
			PrefabInst->SequenceInstance = NULL;
		}
	}

	// Reset connector 'copy' buffer.
	ClearCopiedConnections();

	RebuildTreeControl();
	RefreshViewport();
	NotifyObjectsChanged();
}

/** NOT CURRENTLY SUPPORTED */
void WxKismet::SingleTickSequence()
{
	/* @todo: figure out editor ticking!
	debugf(TEXT("Ticking scripting system"));
	Sequence->UpdateOp();

	LinkedObjVC->Viewport->Invalidate();
	*/
}

/** Take the current view position and store it in the sequence object. */
void WxKismet::ViewPosChanged()
{
	Sequence->DefaultViewX = LinkedObjVC->Origin2D.X;
	Sequence->DefaultViewY = LinkedObjVC->Origin2D.Y;
	Sequence->DefaultViewZoom = LinkedObjVC->Zoom2D;
}

/** 
 *	Handling for draggin on 'special' hit proxies. Should only have 1 object selected when this is being called. 
 *	In this case used for handling resizing handles on Comment objects. 
 *
 *	@param	DeltaX			Horizontal drag distance this frame (in pixels)
 *	@param	DeltaY			Vertical drag distance this frame (in pixels)
 *	@param	SpecialIndex	Used to indicate different classes of action. @see HLinkedObjProxySpecial.
 */
void WxKismet::SpecialDrag( INT DeltaX, INT DeltaY, INT NewX, INT NewY, INT SpecialIndex )
{
	// Can only 'special drag' one object at a time.
	if(SelectedSeqObjs.Num() != 1)
	{
		return;
	}

	USequenceFrame* SeqFrame = Cast<USequenceFrame>(SelectedSeqObjs(0));
	if(SeqFrame)
	{
		if(SpecialIndex == 1)
		{
			// Apply dragging to 
			SeqFrame->SizeX += DeltaX;
			SeqFrame->SizeX = ::Max<INT>(SeqFrame->SizeX, 64);

			SeqFrame->SizeY += DeltaY;
			SeqFrame->SizeY = ::Max<INT>(SeqFrame->SizeY, 64);
		}
	}
}

/**
 * Ensures that only selected objects are marked with the RF_EdSelected flag.
 */
static void RefreshSelectionSets()
{
	// Make a list of all selection sets and refresh RF_EdSelectedFlags.
	TArray<USelection*> SelectionSets;
	for ( TObjectIterator<UObject> It ; It ; ++It )
	{
		UObject* Object = *It;
		Object->ClearFlags( RF_EdSelected );
		if ( Object->IsA(USelection::StaticClass()) )
		{
			SelectionSets.AddItem( static_cast<USelection*>(Object) );
		}
	}

	for ( INT SelectionSetIndex = 0 ; SelectionSetIndex < SelectionSets.Num() ; ++SelectionSetIndex )
	{
		USelection* Selection = SelectionSets(SelectionSetIndex);
		Selection->RefreshObjectFlags();
	}
}

/** Kismet-specific Undo. Uses the global Undo transactor to reverse changes and updates viewport etc. */
void WxKismet::KismetUndo()
{
	if ( GEditor->UndoTransaction() )
	{
		RefreshSelectionSets();
		GCallbackEvent->Send(CALLBACK_Undo);
	}
}

/** Kismet-specific Redo. Uses the global Undo transactor to redo changes and updates viewport etc. */
void WxKismet::KismetRedo()
{
	if ( GEditor->RedoTransaction() )
	{
		RefreshSelectionSets();
		GCallbackEvent->Send(CALLBACK_Undo);
	}
}

/** Export selected sequence objects to text and puts into Windows clipboard. */
void WxKismet::KismetCopy()
{
	USelection* SelectionSet = GEditor->GetSelectedObjects();

	// Add sequence objects selected in Kismet to global selection so the USequenceExporterT3D knows which objects to export.
	SelectionSet->SelectNone( USequenceObject::StaticClass() );
	for(INT i=0; i<SelectedSeqObjs.Num(); i++)
	{
		// Check that we are not trying to copy a Matinee action while its open in the Matinee tool.
		USeqAct_Interp* Interp = Cast<USeqAct_Interp>( SelectedSeqObjs(i) );
		if( Interp && Interp->bIsBeingEdited )
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("MatineeActionAlreadyOpen") );
			return;
		}

		SelectionSet->Select( SelectedSeqObjs(i) );
	}

	// Iterate over all objects making sure they import/export flags are unset. 
	// These are used for ensuring we export each object only once etc.
	for( FObjectIterator It; It; ++It )
	{
		It->ClearFlags( RF_TagImp | RF_TagExp );
	}

	FStringOutputDevice Ar;
	const FExportObjectInnerContext Context;
	UExporter::ExportToOutputDevice( &Context, Sequence, NULL, Ar, TEXT("copy"), 0, PPF_ExportsNotFullyQualified );
	appClipboardCopy( *Ar );

	PasteCount = 0;

	SelectionSet->SelectNone( USequenceObject::StaticClass() );
}

/** 
 *	Take contents of windows clipboard and use USequenceFactory to create new SequenceObjects (possibly new subsequences as well). 
 *	If bAtMousePos is TRUE, it will move the objects so the top-left corner of their bounding box is at the current mouse position (NewX/NewY in LinkedObjVC)
 */
void WxKismet::KismetPaste(UBOOL bAtMousePos)
{
	// Find where we want the top-left corner of selection to be.
	INT DesPosX, DesPosY;
	if(bAtMousePos)
	{
		DesPosX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
		DesPosY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;
	}
	else
	{
		// center pasted objects in the middle of the screen
		INT SizeX = GraphWindow->GetSize().x;
		INT SizeY = GraphWindow->GetSize().y;
		DesPosX = ((((FLOAT)SizeX) * 0.5f) - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
		DesPosY = ((((FLOAT)SizeY) * 0.5f) - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;

		DesPosX += (PasteCount * PasteOffset);
		DesPosY += (PasteCount * PasteOffset);
	}

	// Check this is a valid location - to avoid pasting stuff outside sequence area.
	if(!SeqObjPosIsValid(DesPosX, DesPosY))
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd("NewObjectInvalidLocation"));
		return;
	}


	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd("Paste") );
		FScopedObjectStateChange Notifier(Sequence);
		//Sequence->Modify();

		PasteCount++;

		// Get pasted text.
		FString PasteString = appClipboardPaste();
		const TCHAR* Paste = *PasteString;

		USequenceFactory* Factory = new USequenceFactory;
		Factory->FactoryCreateText( NULL, Sequence, NAME_None, RF_Transactional, NULL, TEXT("paste"), Paste, Paste+appStrlen(Paste), GWarn );

		// Select the newly pasted stuff, and offset a bit.
		TArray<USequenceObject*> NewSelection;

		for ( USelection::TObjectIterator It( GEditor->GetSelectedObjects()->ObjectItor() ) ; It ; ++It )
		{
			UObject* Object = *It;
			USequenceObject* NewSeqObj = Cast<USequenceObject>( Object );
			if ( NewSeqObj )
			{
				NewSelection.AddItem(NewSeqObj);

				NewSeqObj->ParentSequence = Sequence;
			}
		}

		// Update tree view to show new numbers.
		RebuildTreeControl();

		EmptySelection();
		for (INT Idx = 0; Idx < NewSelection.Num(); Idx++)
		{
			AddToSelection(NewSelection(Idx));
			NewSelection(Idx)->OnPasted();
		}

		// Update property window to reflect new selection.
		UpdatePropertyWindow();

		// If we want to paste the copied objects at the current mouse position (NewX/NewY in LinkedObjVC) and we actually pasted something...
		if(SelectedSeqObjs.Num() > 0)
		{
			// By default offset pasted objects by the default value.
			// This is a fallback case for SequenceClasses that have a DrawHeight or DrawWidth of 0.
			INT DeltaX = PasteOffset;
			INT DeltaY = PasteOffset;

			// If we can be more intelligent, then calculate current bounding box of selection.
			// Offset to the desired destination.
			FIntRect SelectedBox = CalcBoundingBoxOfSelected();
			if(SelectedBox.Area() > 0)
			{
				DeltaX = DesPosX - SelectedBox.Min.X;
				DeltaY = DesPosY - SelectedBox.Min.Y;
			}

			// Apply to all selected objects.
			for(INT i=0; i<SelectedSeqObjs.Num(); i++)
			{
				SelectedSeqObjs(i)->ObjPosX += DeltaX;
				SelectedSeqObjs(i)->ObjPosY += DeltaY;
			}
		}
	} // ScopedTransaction

	ClearCopiedConnections();

	// We may have pasted a variable with a VarName, so update status of SeqVar_Named
	Sequence->UpdateNamedVarStatus();
	GetRootSequence()->UpdateInterpActionConnectors();

	RefreshViewport();
	NotifyObjectsChanged();
}

/** Copy all links to the selected ones so we can apply them to a new connector. */
void WxKismet::OnContextCopyConnections( wxCommandEvent &In )
{
	CopyConnInfo.Empty();

	// FIRST ITEM IS ALWAYS THE COPIED CONNECTOR/INDEX
	CopyConnInfo.AddItem( FCopyConnInfo(ConnSeqOp, ConnIndex) );

	// If copying from an output - quite easy - just copy the contents of that connectors Links array.
	if(ConnType == LOC_OUTPUT)
	{
		CopyConnType = LOC_OUTPUT;

		FSeqOpOutputLink& OutLink = ConnSeqOp->OutputLinks(ConnIndex);
		for(INT i=0; i<OutLink.Links.Num(); i++)
		{
			CopyConnInfo.AddItem( FCopyConnInfo(OutLink.Links(i).LinkedOp, OutLink.Links(i).InputLinkIdx) );
		}

		// For CUT, just empty output links
		if( In.GetId() == IDM_KISMET_CUT_CONNECTIONS )
		{
			FScopedObjectStateChange Notifier(ConnSeqOp);
			OutLink.Links.Empty();
		}
	}
	// If copying from an input - must find all places that link to us
	else if(ConnType == LOC_INPUT)
	{
		CopyConnType = LOC_INPUT;

		// iterate through all other objects, looking for output links that point to this op
		for (INT chkIdx = 0; chkIdx < Sequence->SequenceObjects.Num(); chkIdx++)
		{
			USequenceOp *ChkOp = Cast<USequenceOp>(Sequence->SequenceObjects(chkIdx));
			if (ChkOp != NULL)
			{
				FScopedObjectStateChange Notifier(ChkOp);
				UBOOL bModified = FALSE;

				// iterate through this op's output links
				for (INT linkIdx = 0; linkIdx < ChkOp->OutputLinks.Num(); linkIdx++)
				{
					FSeqOpOutputLink& OutLink = ChkOp->OutputLinks(linkIdx);

					// iterate through all the inputs linked to this output
					for (INT inputIdx = 0; inputIdx < OutLink.Links.Num(); inputIdx++)
					{
						if( OutLink.Links(inputIdx).LinkedOp == ConnSeqOp &&
							OutLink.Links(inputIdx).InputLinkIdx == ConnIndex )
						{
							CopyConnInfo.AddItem( FCopyConnInfo(ChkOp, linkIdx) );

							// For CUT, remove each link separately
							if( In.GetId() == IDM_KISMET_CUT_CONNECTIONS )
							{
								bModified = TRUE;
								OutLink.Links.Remove( inputIdx--, 1 );
							}
						}
					}
				}

				Notifier.FinishEdit(!bModified);
			}
		}
	}
	else 
	if( ConnType == LOC_VARIABLE )
	{
		CopyConnType = LOC_VARIABLE;

		FSeqVarLink& VarLink = ConnSeqOp->VariableLinks(ConnIndex);
		for( INT VarIdx = 0; VarIdx < VarLink.LinkedVariables.Num(); VarIdx++ )
		{
			CopyConnInfo.AddItem( FCopyConnInfo(VarLink.LinkedVariables(VarIdx), -1) );
		}

		// For CUT, just empty variable links
		if( In.GetId() == IDM_KISMET_CUT_CONNECTIONS )
		{
			FScopedObjectStateChange Notifier(ConnSeqOp);
			VarLink.LinkedVariables.Empty();
		}
	}
	else 
	if( ConnType == LOC_EVENT )
	{
		CopyConnType = LOC_EVENT;

		FSeqEventLink& EventLink = ConnSeqOp->EventLinks(ConnIndex);
		for( INT EventIdx = 0; EventIdx < EventLink.LinkedEvents.Num(); EventIdx++ )
		{
			CopyConnInfo.AddItem( FCopyConnInfo(EventLink.LinkedEvents(EventIdx), -1) );
		}

		// For CUT, just empty event links
		if( In.GetId() == IDM_KISMET_CUT_CONNECTIONS )
		{
			FScopedObjectStateChange Notifier(ConnSeqOp);
			EventLink.LinkedEvents.Empty();
		}
	}
}

/** Apply the copied connections to the selected connector. */
void WxKismet::OnContextPasteConnections( wxCommandEvent &In )
{
	// Must be something to paste
	if(	(CopyConnInfo.Num() == 0) )
	{
		return;
	}

	// If copying between matching connector types
	if( CopyConnType == ConnType )
	{
		// Make appropriate logic connections (SKIP FIRST ITEM - THAT IS THE COPIED CONNECTOR/INDEX)
		if( CopyConnType == LOC_INPUT )
		{
			for(INT i=1; i<CopyConnInfo.Num(); i++)
			{
				MakeLogicConnection( Cast<USequenceOp>(CopyConnInfo(i).SeqObject), CopyConnInfo(i).ConnIndex, ConnSeqOp, ConnIndex );
			}	
		}
		else 
		if( CopyConnType == LOC_OUTPUT )
		{
			for(INT i=1; i<CopyConnInfo.Num(); i++)
			{
				MakeLogicConnection( ConnSeqOp, ConnIndex, Cast<USequenceOp>(CopyConnInfo(i).SeqObject), CopyConnInfo(i).ConnIndex );
			}	
		}
		else
		if( CopyConnType == LOC_VARIABLE )
		{
			for( INT i = 1; i < CopyConnInfo.Num(); i++ )
			{
				USequenceVariable* Var = Cast<USequenceVariable>(CopyConnInfo(i).SeqObject);
				if( Var )
				{
					MakeVariableConnection( ConnSeqOp, ConnIndex, Var );
				}
			}
		}
		else
		if( CopyConnType == LOC_EVENT )
		{
			for( INT i = 1; i < CopyConnInfo.Num(); i++ )
			{
				USequenceEvent* Event = Cast<USequenceEvent>(CopyConnInfo(i).SeqObject);
				if( Event )
				{
					MakeConnectionToObject( Event );
				}
			}
		}
	}
	else
	// Otherwise, if copying from input to output
	if( CopyConnType == LOC_INPUT &&
		ConnType == LOC_OUTPUT )
	{
		// Make the connection instead of copying all the links (Stored as the first item)
		MakeLogicConnection( ConnSeqOp, ConnIndex, Cast<USequenceOp>(CopyConnInfo(0).SeqObject), CopyConnInfo(0).ConnIndex );
	}
	else 
	if( CopyConnType == LOC_OUTPUT &&
		ConnType == LOC_INPUT )
	{
		// Make the connection instead of copying all the links (Stored as the first item)
		MakeLogicConnection( Cast<USequenceOp>(CopyConnInfo(0).SeqObject), CopyConnInfo(0).ConnIndex, ConnSeqOp, ConnIndex );
	}

	NotifyObjectsChanged();
}

/** Invalidate any copied connections. */
void WxKismet::ClearCopiedConnections()
{
	CopyConnInfo.Empty();
}

void WxKismet::OnContextSelectAssigned( wxCommandEvent &In )
{
	GUnrealEd->SelectNone( FALSE, TRUE );
	for (INT Idx = 0; Idx < SelectedSeqObjs.Num(); Idx++)
	{
		USequenceEvent *Event = Cast<USequenceEvent>(SelectedSeqObjs(Idx));
		AActor *AssignedActor = NULL;
		if (Event != NULL)
		{
			AssignedActor = Event->Originator;
		}
		if (AssignedActor == NULL)
		{
			USeqVar_Object *ObjVar = Cast<USeqVar_Object>(SelectedSeqObjs(Idx));
			if (ObjVar != NULL)
			{
				AssignedActor = Cast<AActor>(ObjVar->ObjValue);
			}
		}
		if (AssignedActor != NULL)
		{
			GUnrealEd->SelectActor(AssignedActor, TRUE, NULL, TRUE);
		}
	}
}

void WxKismet::OnContextSearchRemoteEvents( wxCommandEvent &In )
{
	if (SelectedSeqObjs.Num() > 0)
	{
		USeqAct_ActivateRemoteEvent *Act = Cast<USeqAct_ActivateRemoteEvent>(SelectedSeqObjs(0));
		if (Act != NULL)
		{
			DoSearch(*(Act->EventName.ToString()),KST_RemoteEvents,FALSE,KSS_AllLevels);
		}
	}
}

/** Handle a call to change the style of a comment box. */
void WxKismet::OnContextApplyCommentStyle( wxCommandEvent &In )
{
	FScopedTransaction Transaction( TEXT("Apply Comment Style") );

	// Determine which style we want, and grab it from the bindings object
	UKismetBindings* Bindings = (UKismetBindings*)(UKismetBindings::StaticClass()->GetDefaultObject());
	INT StyleIndex = In.GetId() - IDM_KISMET_APPLY_COMMENT_STYLE_START;
	check(StyleIndex >= 0);
	check(StyleIndex < Bindings->CommentPresets.Num());
	FKismetCommentPreset& Preset = Bindings->CommentPresets(StyleIndex);

	// Go over each selected comment ('frame') and apply the settings from the preset
	for(INT i=0; i<SelectedSeqObjs.Num(); i++)
	{
		USequenceFrame* Comment = Cast<USequenceFrame>(SelectedSeqObjs(i));
		if(Comment)
		{
			FScopedObjectStateChange Notifier(Comment);
			//Comment->Modify();

			Comment->BorderWidth = Preset.BorderWidth;
			Comment->BorderColor = Preset.BorderColor;
			Comment->bFilled = Preset.bFilled;
			Comment->FillColor = Preset.FillColor;
		}
	}
}

/** Move selected comment to front (last in array). */
void WxKismet::OnContextCommentToFront( wxCommandEvent &In )
{
	if(SelectedSeqObjs.Num() > 0)
	{
		USequenceFrame* Comment = Cast<USequenceFrame>(SelectedSeqObjs(0));
		if(Comment)
		{
			FScopedObjectStateChange Notifier(Comment);

			// Remove from array and add at end.
			Sequence->SequenceObjects.RemoveItem(Comment);
			Sequence->SequenceObjects.AddItem(Comment);
		}
	}
}

/** Move selected comment to back (first in array). */
void WxKismet::OnContextCommentToBack( wxCommandEvent &In )
{
	if(SelectedSeqObjs.Num() > 0)
	{
		USequenceFrame* Comment = Cast<USequenceFrame>(SelectedSeqObjs(0));
		if(Comment)
		{
			FScopedObjectStateChange Notifier(Sequence);

			// Remove from array and add at end.
			Sequence->SequenceObjects.RemoveItem(Comment);
			Sequence->SequenceObjects.InsertItem(Comment, 0);
		}
	}
}


/** Get world sequence by default - override if sequence is elsewhere (ie in a Package) */
USequence* WxKismet::GetRootSequence()
{
	return Sequence->GetRootSequence();
}

/** Refresh Kismet tree control showing heirarchy of Sequences. */
void WxKismet::RebuildTreeControl()
{
	WxSequenceTreeCtrl* TreeCtrl = wxDynamicCast(TreeControl,WxSequenceTreeCtrl);
	check(TreeCtrl);

	TreeCtrl->RefreshTree();

	UBOOL bWasSequenceFound = TreeCtrl->SelectObject(Sequence);
	if( !bWasSequenceFound )
	{
		// The current sequence wasn't found in the tree we just rebuilt, so it may have been deleted.
		// We'll change the active sequence to be the root level sequence.
		Sequence = NULL;
		OldSequence = NULL;
		ChangeActiveSequence( GWorld->GetGameSequence() );
	}
}

/** Open the Matinee tool to edit the supplied SeqAct_Interp (Matinee) action. Will check that SeqAct_Interp has an InterpData attached. */
void WxKismet::OpenMatinee(USeqAct_Interp* Interp)
{
	UInterpData* Data = Interp->FindInterpDataFromVariable();
	if(!Data)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_MatineeActionMustHaveData") );
		return;
	}

	// Make sure we can't open the same action twice in Matinee.
	if(Interp->bIsBeingEdited)
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("MatineeActionAlreadyOpen") );
		return;
	}

	// Don't let you open Matinee if a transaction is currently active.
	if( GEditor->IsTransactionActive() )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("TransactionIsActive") );
		return;
	}

	// If already in Matinee mode, exit out before going back in with new Interpolation.
	if( GEditorModeTools().GetCurrentModeID() == EM_InterpEdit )
	{
		GEditorModeTools().SetCurrentMode( EM_Default );
	}

	GEditorModeTools().SetCurrentMode( EM_InterpEdit );

	FEdModeInterpEdit* InterpEditMode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();

	InterpEditMode->InitInterpMode( Interp );

	// Minimize Kismet here?
	//this->Iconize(TRUE);
	//this->Close();
}

/** Notification that the user clicked on an element of the tree control. Will find the corresponing USequence and jump to it. */
void WxKismet::OnTreeItemSelected( wxTreeEvent &In )
{
	wxTreeItemId TreeId				= In.GetItem();
	if ( TreeId.IsOk() )
	{
		WxTreeObjectWrapper* LeafData	= (WxTreeObjectWrapper*)TreeControl->GetItemData(TreeId);
		if( LeafData )
		{
			USequence* ClickedSeq		= LeafData->GetObject<USequence>();
			if(ClickedSeq != Sequence)
			{
				ChangeActiveSequence(ClickedSeq, FALSE);
			}
		}
	}
}

/** Notification that a tree node expanded. Changes icon in tree view. */
void WxKismet::OnTreeExpanding( wxTreeEvent& In )
{
	TreeControl->SetItemImage( In.GetItem(), 1 );
}

/** Notification that a tree node collapsed. Changes icon in tree view. */
void WxKismet::OnTreeCollapsed( wxTreeEvent& In )
{
	TreeControl->SetItemImage( In.GetItem(), 0 );
}

/** 
 *	Create a new USequenceObject in the currently active Sequence of the give class and at the given position.
 *	This function does special handling if creating a USequenceFrame object and we have objects selected - it will create frame around selection.
 */
USequenceObject* WxKismet::NewSequenceObject(UClass* NewSeqObjClass, INT NewPosX, INT NewPosY, UBOOL bTransient)
{
	if(!SeqObjPosIsValid(NewPosX, NewPosY))
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd("NewObjectInvalidLocation"));
		return NULL;
	}

	// figure out which Outer to use
	UObject* DesiredOuter;
	if (bTransient) 
	{
		DesiredOuter = UObject::GetTransientPackage();
	}
	else
	{
		DesiredOuter = Sequence;
	}

	FScopedTransaction Transaction( *LocalizeUnrealEd("NewObject") );
	//Sequence->Modify();
	FScopedObjectStateChange Notifier(DesiredOuter);
	
	// if creating a comment ask for the initial text
	FString CommentText;
	if (NewSeqObjClass == USequenceFrame::StaticClass())
	{
		WxDlgGenericStringEntry dlg;
		INT Result = dlg.ShowModal( TEXT("NewComment"), TEXT("CommentText"), TEXT("Comment") );
		if (Result != wxID_OK)
		{
			// abort!
			Transaction.Cancel();
			Notifier.CancelEdit();
			return NULL;
		}
		else
		{
			CommentText = dlg.GetEnteredString();
		}
	}

	USequenceObject* NewSeqObj = ConstructObject<USequenceObject>( NewSeqObjClass, DesiredOuter, NAME_None,	RF_Transactional|Sequence->GetMaskedFlags(RF_ArchetypeObject|RF_Public) );
	debugf(TEXT("Created new %s in sequence %s"),*NewSeqObj->GetName(),*Sequence->GetName());

	USequenceFrame* SeqFrame = Cast<USequenceFrame>(NewSeqObj);
	if (SeqFrame != NULL)
	{
		SeqFrame->ObjComment = CommentText;
	}
	// If creating a SequenceFrame (comment) object, and we have things selected, create frame around selection.
	if (SeqFrame != NULL && SelectedSeqObjs.Num() > 0)
	{
		FIntRect SelectedRect = CalcBoundingBoxOfSelected();
		SeqFrame->ObjPosX = SelectedRect.Min.X - FitCommentToSelectedBorder;
		SeqFrame->ObjPosY = SelectedRect.Min.Y - FitCommentToSelectedBorder;
		SeqFrame->SnapPosition(KISMET_GRIDSIZE, MaxSequenceSize);
		SeqFrame->SizeX = (SelectedRect.Max.X - SelectedRect.Min.X) + (2 * FitCommentToSelectedBorder);
		SeqFrame->SizeY = (SelectedRect.Max.Y - SelectedRect.Min.Y) + (2 * FitCommentToSelectedBorder);
		SeqFrame->bDrawBox = TRUE;
	}
	else
	{
		NewSeqObj->ObjPosX = NewPosX;
		NewSeqObj->ObjPosY = NewPosY;
		NewSeqObj->SnapPosition(KISMET_GRIDSIZE, MaxSequenceSize);
	}

	if ( Sequence->AddSequenceObject(NewSeqObj) )
	{
		//@todo ronp uiprefabs: how to call OnConnect on any instances that are updated
		NewSeqObj->Modify();
		NewSeqObj->OnCreated();

		RebuildTreeControl();

		EmptySelection();	
		AddToSelection(NewSeqObj);

		RefreshViewport();
		UpdatePropertyWindow();
		NotifyObjectsChanged();
	}
	else
	{
		Notifier.CancelEdit();
		Transaction.Cancel();
		NewSeqObj = NULL;
	}

	return NewSeqObj;
}

/** 
 *	Handler for menu option for creating arbitary new sequence object.
 *	Find class from the SeqObjClasses array and event id.
 */
void WxKismet::OnContextNewScriptingObject( wxCommandEvent& In )
{
	INT NewSeqObjIndex = In.GetId() - IDM_NEW_SEQUENCE_OBJECT_START;
	check( NewSeqObjIndex >= 0 && NewSeqObjIndex < SeqObjClasses.Num() );

	UClass* NewSeqObjClass = SeqObjClasses(NewSeqObjIndex);
	check( NewSeqObjClass->IsChildOf(USequenceObject::StaticClass()) );

	// Calculate position in sequence from mouse position.
	INT NewPosX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
	INT NewPosY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;

	NewSequenceObject(NewSeqObjClass, NewPosX, NewPosY);
}

/** 
 *	Create a new Object Variable and fill it in with the currently selected Actor.
 *	If multiple Actors are selected, will create an Object Variable for each, offsetting them.
 *	If bAttachVarsToConnector is TRUE, it will also connect these new variables to the selected variable connector.
 */
void WxKismet::OnContextNewScriptingObjVarContext(wxCommandEvent& In)
{
	if (NewObjActors.Num() > 0)
	{
		// Make sure all actors belong to the sequence's level.
		ULevel* SequenceLevel = Sequence->GetLevel();
		if ( SequenceLevel )
		{
			for( INT ActorIndex = 0 ; ActorIndex < NewObjActors.Num() ; ++ActorIndex )
			{
				AActor* Actor		= NewObjActors(ActorIndex);
				ULevel* ActorLevel	= Actor->GetLevel();
				if ( ActorLevel != SequenceLevel )
				{
					appMsgf( AMT_OK, *FString::Printf( LocalizeSecure(LocalizeUnrealEd(TEXT("KismetCantHaveCrossLevelReferences_F")),*SequenceLevel->GetOutermost()->GetName(),*Actor->GetPathName()) ) );
					return;
				}
			}
		}
		else
		{
			warnf( NAME_Log, TEXT("Sequence object '%s' has no ULevel in outer chain."), *Sequence->GetPathName() );
		}

		// Create the new sequence objects.
		const FScopedTransaction Transaction( *LocalizeUnrealEd("NewEvent") );
		FScopedObjectStateChange Notifier(Sequence);
		//Sequence->Modify();
		EmptySelection();	

		for(INT i=0; i<NewObjActors.Num(); i++)
		{
			AActor* Actor = NewObjActors(i);
			USeqVar_Object* NewObjVar = ConstructObject<USeqVar_Object>( USeqVar_Object::StaticClass(), Sequence, NAME_None, RF_Transactional|Sequence->GetMaskedFlags(RF_Public|RF_ArchetypeObject));
			debugf(TEXT("Created new %s in sequence %s"),*NewObjVar->GetName(),*Sequence->GetName());

			if(bAttachVarsToConnector && ConnSeqOp && (ConnType == LOC_VARIABLE))
			{
				FIntPoint ConnectorPos = ConnSeqOp->GetConnectionLocation(LOC_VARIABLE, ConnIndex);			
				NewObjVar->ObjPosX = ConnectorPos.X - (LO_MIN_SHAPE_SIZE/2) + (MultiEventSpacing * i);
				NewObjVar->ObjPosY = ConnectorPos.Y + 10;
			}
			else
			{
				NewObjVar->ObjPosX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D + (MultiEventSpacing * i);
				NewObjVar->ObjPosY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;
				NewObjVar->SnapPosition(KISMET_GRIDSIZE, MaxSequenceSize);
			}

			NewObjVar->ObjValue = Actor;

			Sequence->AddSequenceObject(NewObjVar);

			//@todo ronp uiprefabs: how to call OnConnect on any instances that are updated
			NewObjVar->OnCreated();

			AddToSelection(NewObjVar);

			// If desired, and we have a connector selected, attach to it now
			if(bAttachVarsToConnector && ConnSeqOp && (ConnType == LOC_VARIABLE))
			{
				MakeVariableConnection(ConnSeqOp, ConnIndex, NewObjVar);
			}
		}

		RebuildTreeControl();
		RefreshViewport();
		UpdatePropertyWindow();
		NotifyObjectsChanged();
	}
}

/**
 * Creates a new variable at the selected connector and automatically links it.
 */
void WxKismet::OnContextCreateLinkedVariable(wxCommandEvent &In)
{
	if(ConnSeqOp != NULL && ConnType == LOC_VARIABLE)
	{
		UClass* varClass = ConnSeqOp->VariableLinks(ConnIndex).ExpectedType;
		if(varClass)
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("NewVar") );
			FScopedObjectStateChange Notifier(Sequence);
			//Sequence->Modify();

			USequenceVariable *var = ConstructObject<USequenceVariable>( varClass, Sequence, NAME_None, RF_Transactional|Sequence->GetMaskedFlags(RF_ArchetypeObject|RF_Public) );
			debugf(TEXT("Created new %s in sequence %s"),*var->GetName(),*Sequence->GetName());

			FIntPoint ConnectorPos = ConnSeqOp->GetConnectionLocation(LOC_VARIABLE, ConnIndex);			
			var->ObjPosX = ConnectorPos.X - (LO_MIN_SHAPE_SIZE/2);
			var->ObjPosY = ConnectorPos.Y + 10;

			Sequence->AddSequenceObject(var);

			//@todo ronp uiprefabs: how to call OnConnect on any instances that are updated
			var->OnCreated();

			EmptySelection();	
			AddToSelection(var);

			MakeVariableConnection(ConnSeqOp, ConnIndex, var);

			RebuildTreeControl();
			RefreshViewport();
			UpdatePropertyWindow();
			NotifyObjectsChanged();
		}
	}
}

/**
 * Clears the values of all selected object variables.
 */
void WxKismet::OnContextClearVariable(wxCommandEvent &In)
{
	// iterate through for all sequence obj vars
	for (INT Idx = 0; Idx < SelectedSeqObjs.Num(); Idx++)
	{
		USeqVar_Object *objVar = Cast<USeqVar_Object>(SelectedSeqObjs(Idx));
		if (objVar != NULL)
		{
			FScopedObjectStateChange Notifier(objVar);	//@todo ronp uiprefabs: change this so that we only attempt to perform an update once

			// set their values to NULL
			objVar->ObjValue = NULL;
		}
	}
}

/**
 * Creates a new event using the selected actor(s), allowing only events that the
 * actor(s) support.
 */
void WxKismet::OnContextNewScriptingEventContext( wxCommandEvent& In )
{
	if(!SeqObjPosIsValid(LinkedObjVC->NewX, LinkedObjVC->NewY))
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd("NewObjectInvalidLocation"));
		return;
	}

	if (NewObjActors.Num() > 0)
	{
		// Make sure all actors belong to the sequence's level.
		ULevel* SequenceLevel = Sequence->GetLevel();
		if ( SequenceLevel )
		{
			for( INT ActorIndex = 0 ; ActorIndex < NewObjActors.Num() ; ++ActorIndex )
			{
				AActor* Actor		= NewObjActors(ActorIndex);
				ULevel* ActorLevel	= Actor->GetLevel();
				if ( ActorLevel != SequenceLevel )
				{
					appMsgf( AMT_OK, *FString::Printf( LocalizeSecure(LocalizeUnrealEd(TEXT("KismetCantHaveCrossLevelReferences_F")),*SequenceLevel->GetOutermost()->GetName(),*Actor->GetPathName()) ) );
					return;
				}
			}
		}
		else
		{
			warnf( NAME_Log, TEXT("Sequence object '%s' has no ULevel in outer chain."), *Sequence->GetPathName() );
		}

		const FScopedTransaction Transaction( *LocalizeUnrealEd("NewEvent") );
		FScopedObjectStateChange Notifier(Sequence);
		//Sequence->Modify();
		EmptySelection();	

		INT NewSeqEvtIndex = In.GetId() - IDM_NEW_SEQUENCE_EVENT_START;
		check( NewSeqEvtIndex >= 0 && NewSeqEvtIndex < NewEventClasses.Num() );

		UClass* NewSeqEvtClass = NewEventClasses(NewSeqEvtIndex);

		for(INT i=0; i<NewObjActors.Num(); i++)
		{
			AActor* Actor = NewObjActors(i);
			USequenceEvent* NewSeqEvt = ConstructObject<USequenceEvent>( NewSeqEvtClass, Sequence, NAME_None, RF_Transactional|Sequence->GetMaskedFlags(RF_ArchetypeObject|RF_Public));

			NewSeqEvt->ObjPosX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
			NewSeqEvt->ObjPosY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D + (MultiEventSpacing * i);
			NewSeqEvt->SnapPosition(KISMET_GRIDSIZE, MaxSequenceSize);

			NewSeqEvt->ObjName = FString::Printf( TEXT("%s %s"), *Actor->GetName(), *NewSeqEvt->ObjName );
			NewSeqEvt->Originator = Actor;

			Sequence->AddSequenceObject(NewSeqEvt);
			//@todo ronp uiprefabs: how to call OnConnect on any instances that are updated
			NewSeqEvt->OnCreated();

			AddToSelection(NewSeqEvt);
			debugf(TEXT("Created new %s in sequence %s [%s]"),*NewSeqEvt->GetName(),*Sequence->GetName(),*NewSeqEvt->ObjName);
		}

		RebuildTreeControl();
		RefreshViewport();
		UpdatePropertyWindow();
		NotifyObjectsChanged();
	}
}

/**
 * Toggle link disabled flag
 */
void WxKismet::OnContextToggleLink( wxCommandEvent& In )
{
	FScopedTransaction Transaction(TEXT("Toggle Link"));	//@todo localize
	// Toggle the link disable flag
	if( ConnType == LOC_OUTPUT )
	{
		FScopedObjectStateChange Notifier(ConnSeqOp);
		ConnSeqOp->OutputLinks(ConnIndex).bDisabled = !ConnSeqOp->OutputLinks(ConnIndex).bDisabled;
	}
	else if( ConnType == LOC_INPUT )
	{
		FScopedObjectStateChange Notifier(ConnSeqOp);
		ConnSeqOp->InputLinks(ConnIndex).bDisabled  = !ConnSeqOp->InputLinks(ConnIndex).bDisabled;
	}	
}

/**
 * Toggle link PIE disabled flag
 */
void WxKismet::OnContextToggleLinkPIE( wxCommandEvent& In )
{
	FScopedTransaction Transaction(TEXT("Toggle Link"));	//@todo localize
	// Toggle the link disable flag
	if( ConnType == LOC_OUTPUT )
	{
		FScopedObjectStateChange Notifier(ConnSeqOp);
		ConnSeqOp->OutputLinks(ConnIndex).bDisabledPIE = !ConnSeqOp->OutputLinks(ConnIndex).bDisabledPIE;
	}
	else if( ConnType == LOC_INPUT )
	{
		FScopedObjectStateChange Notifier(ConnSeqOp);
		ConnSeqOp->InputLinks(ConnIndex).bDisabledPIE  = !ConnSeqOp->InputLinks(ConnIndex).bDisabledPIE;
	}	
	Sequence->MarkPackageDirty();
}

/**
 * Break link from selected input or variable connector 
 * (output connectors don't store pointer to target)
 */
void WxKismet::OnContextBreakLink( wxCommandEvent& In )
{
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd("BreakLink") );

		if (ConnType == LOC_EVENT)
		{
			FScopedObjectStateChange Notifier(ConnSeqOp);
			//ConnSeqOp->Modify();

			ConnSeqOp->EventLinks(ConnIndex).LinkedEvents.Empty();
		}
		else
		if(ConnType == LOC_VARIABLE)
		{
			FScopedObjectStateChange Notifier(ConnSeqOp);
			//ConnSeqOp->Modify();

			if (In.GetId() == IDM_KISMET_BREAK_LINK_ALL)
			{
				// empty the linked vars array
				ConnSeqOp->VariableLinks(ConnIndex).LinkedVariables.Empty();
			}
			else
			{
				// determine the selected break
				INT breakIdx = In.GetId() - IDM_KISMET_BREAK_LINK_START;
				ConnSeqOp->VariableLinks(ConnIndex).LinkedVariables.Remove(breakIdx,1);
			}
		}
		else
		if (ConnType == LOC_INPUT)
		{
			// find all ops that point to this
			TArray<USequenceOp*> outputOps;
			TArray<INT> outputIndices;
			FindOutputLinkTo(Sequence,ConnSeqOp,ConnIndex,outputOps,outputIndices);
			// if break all...
			if (In.GetId() == IDM_KISMET_BREAK_LINK_ALL)
			{
				// break all the links individually
				for (INT Idx = 0; Idx < outputOps.Num(); Idx++)
				{
					FScopedObjectStateChange OutputOpNotifier(outputOps(Idx));
					//outputOps(Idx)->Modify();

					for (INT linkIdx = 0; linkIdx < outputOps(Idx)->OutputLinks(outputIndices(Idx)).Links.Num(); linkIdx++)
					{
						// if this is the linked index
						if( outputOps(Idx)->OutputLinks(outputIndices(Idx)).Links(linkIdx).LinkedOp == ConnSeqOp &&
							outputOps(Idx)->OutputLinks(outputIndices(Idx)).Links(linkIdx).InputLinkIdx == ConnIndex )
						{
							// remove the entry
							outputOps(Idx)->OutputLinks(outputIndices(Idx)).Links.Remove(linkIdx--,1);
						}
					}
				}
			}
			else
			{
				// determine the selected break
				INT breakIdx = In.GetId() - IDM_KISMET_BREAK_LINK_START;
				if (breakIdx >= 0 &&
					breakIdx < outputOps.Num())
				{
					FScopedObjectStateChange OutputOpNotifier(outputOps(breakIdx));
					//outputOps(Idx)->Modify();

					// find the actual index of the link
					for (INT linkIdx = 0; linkIdx < outputOps(breakIdx)->OutputLinks(outputIndices(breakIdx)).Links.Num(); linkIdx++)
					{
						// if this is the linked index
						if (outputOps(breakIdx)->OutputLinks(outputIndices(breakIdx)).Links(linkIdx).LinkedOp == ConnSeqOp)
						{
							// remove the entry
							outputOps(breakIdx)->OutputLinks(outputIndices(breakIdx)).Links.Remove(linkIdx--,1);
						}
					}
				}
			}
		}
		else
		if (ConnType == LOC_OUTPUT)
		{
			FScopedObjectStateChange Notifier(ConnSeqOp);
			//ConnSeqOp->Modify();

			if (In.GetId() == IDM_KISMET_BREAK_LINK_ALL)
			{
				// empty the links for this output
				ConnSeqOp->OutputLinks(ConnIndex).Links.Empty();
			}
			else
			{
				// determine the selected break
				INT breakIdx = In.GetId() - IDM_KISMET_BREAK_LINK_START;
				if (breakIdx >= 0 &&
					breakIdx < ConnSeqOp->OutputLinks(ConnIndex).Links.Num())
				{
					// remove the link
					ConnSeqOp->OutputLinks(ConnIndex).Links.Remove(breakIdx,1);
				}
			}
		}
	} // ScopedTransaction

	GetRootSequence()->UpdateInterpActionConnectors();

	RefreshViewport();
	NotifyObjectsChanged();
}

void WxKismet::NotifyObjectsChanged()
{
	if (Sequence != NULL)
	{
		Sequence->MarkPackageDirty();
	}
}

/**
 * Update an old action to the latest version.
 */
void WxKismet::OnContextUpdateAction( wxCommandEvent& In )
{
	// this indicates whether the undo transaction should be cancelled or not
	UBOOL bUpdatedObjects = FALSE;

	// begin a transaction
	INT CancelIndex = GEditor->BeginTransaction(*LocalizeUnrealEd(TEXT("KismetUndo_UpdateMultiple")));

	for (INT Idx = 0; Idx < SelectedSeqObjs.Num(); Idx++)
	{
		USequenceOp *Op = Cast<USequenceOp>(SelectedSeqObjs(Idx));
		if (Op != NULL && Op->eventGetObjClassVersion() != Op->ObjInstanceVersion)
		{
			bUpdatedObjects = TRUE;
			Op->UpdateObject();
		}
	}

	if (bUpdatedObjects)
	{
		// refresh property window in case any properties were changed in the update process
		UpdatePropertyWindow();
	}

	// end the transaction if we actually updated some sequence objects
	if ( bUpdatedObjects == TRUE )
	{
		GEditor->EndTransaction();
	}
	else
	{
		// otherwise, cancel this transaction
		GEditor->CancelTransaction(CancelIndex);
	}
}

/** Delete the selected sequence objects. */
void WxKismet::OnContextDelSeqObject( wxCommandEvent& In )
{
	DeleteSelected();

	RebuildTreeControl();
	RefreshViewport();
}

/** For all selected events, select the actors that correspond to them. */
void WxKismet::OnContextSelectEventActors( wxCommandEvent& In )
{
	GUnrealEd->SelectNone( TRUE, TRUE );

	for(INT i=0; i<SelectedSeqObjs.Num(); i++)
	{
		USequenceEvent* SeqEvt = Cast<USequenceEvent>( SelectedSeqObjs(i) );
		if(SeqEvt && SeqEvt->Originator)
		{
			GUnrealEd->SelectActor( SeqEvt->Originator, 1, NULL, 0 );
		}
	}

	GUnrealEd->NoteSelectionChange();
}

/** Force-fire an event in the script. Useful for debugging. NOT CURRENTLY IMPLEMENTED. */
void WxKismet::OnContextFireEvent( wxCommandEvent& In )
{
	/*@todo - re-evaluate activation of events within kismet
	for(INT i=0; i<SelectedSeqObjs.Num(); i++)
	{
		USequenceEvent* SeqEvt = Cast<USequenceEvent>( SelectedSeqObjs(i) );
		if(SeqEvt && SeqEvt->Originator)
			SeqEvt->Originator->ActivateEvent(SeqEvt);
	}
	*/
}

/** Handler for double-clicking on Matinee action. Will open Matinee to edit the selected USeqAct_Interp (Matinee) action. */
void WxKismet::OnContextOpenInterpEdit( wxCommandEvent& In )
{
	check( SelectedSeqObjs.Num() == 1 );

	USeqAct_Interp* Interp = Cast<USeqAct_Interp>( SelectedSeqObjs(0) );
	check(Interp);

	OpenMatinee(Interp);
}

/** Handler for adding a switch connector */
void WxKismet::OnContextSwitchAdd( wxCommandEvent& In )
{
	check( SelectedSeqObjs.Num() == 1 );

	USeqCond_SwitchBase* Switch = Cast<USeqCond_SwitchBase>( SelectedSeqObjs(0) );
	check(Switch);

	FScopedTransaction Transaction(TEXT("Add Case"));	//@todo localize
	FScopedObjectStateChange Notifier(Switch);

	// Insert item into the end of the list (but always keep "Default" switch last)
	INT Num	= Max<INT>( Switch->OutputLinks.Num() - 1, 0 );
	Switch->OutputLinks.InsertZeroed( Num );
	Switch->OutputLinks(Num).LinkDesc = FString(TEXT("None"));
	Switch->eventInsertValueEntry(Num);
}

/** Handler for removing a switch connector */
void WxKismet::OnConextSwitchRemove( wxCommandEvent& In )
{
	USeqCond_SwitchBase* Switch = Cast<USeqCond_SwitchBase>(ConnSeqOp);
	check(Switch);

	// Don't allow removal of the default node
	if( appStricmp( *Switch->OutputLinks(ConnIndex).LinkDesc, TEXT("Default") ) != 0 )
	{
		FScopedTransaction Transaction(TEXT("Remove Case"));	//@todo localize
		FScopedObjectStateChange Notifier(Switch);

		Switch->OutputLinks(ConnIndex).Links.Empty();
		Switch->OutputLinks.Remove( ConnIndex );
		Switch->eventRemoveValueEntry(ConnIndex);
	}
}

/**
 * Iterates through all specified sequence objects and sets
 * bHidden for any connectors that have no links.
 */
void WxKismet::HideConnectors(USequence *Sequence,TArray<USequenceObject*> &objList)
{
	// iterate through all selected sequence objects
	for (INT Idx = 0; Idx < objList.Num(); Idx++)
	{
		USequenceOp *op = Cast<USequenceOp>(objList(Idx));
		// if it is a valid operation
		if (op != NULL)
		{
			// first check all input connectors for active links
			for (INT linkIdx = 0; linkIdx < op->InputLinks.Num(); linkIdx++)
			{
				// if not already hidden,
				if (!op->InputLinks(linkIdx).bHidden)
				{
					// search for all output links pointing at this input link
					TArray<USequenceOp*> outputOps;
					TArray<INT> outputIndices;
					if (!FindOutputLinkTo(Sequence,op,linkIdx,outputOps,outputIndices))
					{
						// no links to this input, set as hidden
						op->InputLinks(linkIdx).bHidden = 1;
					}
				}
			}
			// next check all output connectors...
			for (INT linkIdx = 0; linkIdx < op->OutputLinks.Num(); linkIdx++)
			{
				// if not already hidden,
				if (!op->OutputLinks(linkIdx).bHidden)
				{
					if (op->OutputLinks(linkIdx).Links.Num() == 0)
					{
						// no links out, hidden
						op->OutputLinks(linkIdx).bHidden = 1;
					}
				}
			}
			// variable connectors...
			for (INT linkIdx = 0; linkIdx < op->VariableLinks.Num(); linkIdx++)
			{
				// if not already hidden,
				if (!op->VariableLinks(linkIdx).bHidden)
				{
					if (op->VariableLinks(linkIdx).LinkedVariables.Num() == 0)
					{
						// hidden
						op->VariableLinks(linkIdx).bHidden = 1;
					}
				}
			}
			// and finally events
			for (INT linkIdx = 0; linkIdx < op->EventLinks.Num(); linkIdx++)
			{
				// if not already hidden,
				if (!op->EventLinks(linkIdx).bHidden)
				{
					// check for any currently linked events
					if (op->EventLinks(linkIdx).LinkedEvents.Num() == 0)
					{
						// hide away
						op->EventLinks(linkIdx).bHidden = 1;
					}
				}
			}
		}
	}
}

/** Break all links going to or from the selected Ops. */
void WxKismet::OnContextBreakAllOpLinks( wxCommandEvent& In )
{
	const FScopedTransaction Transaction( *LocalizeUnrealEd("BreakAllOpLinks") );

	// Iterate over all objects.
	for(INT i=0; i<SelectedSeqObjs.Num(); i++)
	{
		// If its an Op..
		USequenceOp* Op = Cast<USequenceOp>(SelectedSeqObjs(i));
		if(Op)
		{
			FScopedObjectStateChange Notifier(Op);
			UBOOL bModified = FALSE;

			// ..first break all output links to other ops, variables and events
			for(INT OutIdx=0; OutIdx<Op->OutputLinks.Num(); OutIdx++)
			{
				FSeqOpOutputLink& OutLink = Op->OutputLinks(OutIdx);

				bModified = OutLink.Links.Num() > 0;
				OutLink.Links.Empty();
			}

			for(INT VarIdx=0; VarIdx<Op->VariableLinks.Num(); VarIdx++)
			{
				FSeqVarLink& VarLink = Op->VariableLinks(VarIdx);

				bModified = VarLink.LinkedVariables.Num() > 0;
				VarLink.LinkedVariables.Empty();
			}

			for(INT EventIdx=0; EventIdx<Op->EventLinks.Num(); EventIdx++)
			{
				FSeqEventLink& EventLink = Op->EventLinks(EventIdx);
				bModified = EventLink.LinkedEvents.Num() > 0;
				EventLink.LinkedEvents.Empty();
			}

			// iterate through all other objects, looking for output links that point to this op
			for (INT chkIdx = 0; chkIdx < Sequence->SequenceObjects.Num(); chkIdx++)
			{
				// if it is a sequence op,
				USequenceOp *ChkOp = Cast<USequenceOp>(Sequence->SequenceObjects(chkIdx));
				if (ChkOp != NULL)
				{
					// iterate through this op's output links
					for (INT linkIdx = 0; linkIdx < ChkOp->OutputLinks.Num(); linkIdx++)
					{
						// iterate through all the inputs linked to this output
						for (INT inputIdx = 0; inputIdx < ChkOp->OutputLinks(linkIdx).Links.Num(); inputIdx++)
						{
							if (ChkOp->OutputLinks(linkIdx).Links(inputIdx).LinkedOp == Op)
							{
								FScopedObjectStateChange LinkNotifier(ChkOp);

								// remove the entry
								ChkOp->OutputLinks(linkIdx).Links.Remove(inputIdx--,1);
							}
						}
					}
				}
			}

			Notifier.FinishEdit(!bModified);
		}

		// If this is a variable - check all ops to remove links to it.
		USequenceVariable* SeqVar = Cast<USequenceVariable>(SelectedSeqObjs(i));
		if(SeqVar)
		{
			for(INT ChkIdx=0; ChkIdx<Sequence->SequenceObjects.Num(); ChkIdx++)
			{
				USequenceOp* ChkOp = Cast<USequenceOp>( Sequence->SequenceObjects(ChkIdx) );
				if(ChkOp)
				{
					for(INT ChkLnkIdx=0; ChkLnkIdx < ChkOp->VariableLinks.Num(); ChkLnkIdx++)
					{
						for (INT varIdx = 0; varIdx < ChkOp->VariableLinks(ChkLnkIdx).LinkedVariables.Num(); varIdx++)
						{
							if (ChkOp->VariableLinks(ChkLnkIdx).LinkedVariables(varIdx) == SeqVar)
							{
								FScopedObjectStateChange VarNotifier(ChkOp);

								// Remove from list
								ChkOp->VariableLinks(ChkLnkIdx).LinkedVariables.Remove(varIdx--,1);
							}
						}
					}
				}
			}
		}
	}

	RefreshViewport();
}

/**
 * Iterates through all selected sequence objects and sets
 * bHidden for any connectors that have no links.
 */
void WxKismet::OnContextHideConnectors( wxCommandEvent& In )
{
	HideConnectors(Sequence,SelectedSeqObjs);
}

/**
 * Iterates through all sequence objects and hides any
 * unused connectors.
 */
void WxKismet::OnButtonHideConnectors(wxCommandEvent& In)
{
	HideConnectors(Sequence,Sequence->SequenceObjects);
}

/**
 * Shows all connectors for each object passed in.
 * 
 * @param	objList - list of all objects to show connectors
 */
void WxKismet::ShowConnectors(TArray<USequenceObject*> &objList)
{
	// iterate through all objects
	for (INT Idx = 0; Idx < objList.Num(); Idx++)
	{
		USequenceOp *op = Cast<USequenceOp>(objList(Idx));
		// if it is a valid sequence op
		if (op != NULL)
		{
			// show all input connectors
			for (INT linkIdx = 0; linkIdx < op->InputLinks.Num(); linkIdx++)
			{
				op->InputLinks(linkIdx).bHidden = 0;
			}
			// output connectors
			for (INT linkIdx = 0; linkIdx < op->OutputLinks.Num(); linkIdx++)
			{
				op->OutputLinks(linkIdx).bHidden = 0;
			}
			// variable connectors
			for (INT linkIdx = 0; linkIdx < op->VariableLinks.Num(); linkIdx++)
			{
				op->VariableLinks(linkIdx).bHidden = 0;
			}
			// event connectors
			for (INT linkIdx = 0; linkIdx < op->EventLinks.Num(); linkIdx++)
			{
				op->EventLinks(linkIdx).bHidden = 0;
			}
		}
	}
}

/**
 * Iterates through all selected objects and unhides any perviously
 * hidden connectors, input/output/variable/event.
 */
void WxKismet::OnContextShowConnectors( wxCommandEvent& In )
{
	ShowConnectors(SelectedSeqObjs);
}

/**
 * Iterates through all sequence objects and shows all hidden connectors.
 */
void WxKismet::OnButtonShowConnectors( wxCommandEvent& In )
{
	ShowConnectors(Sequence->SequenceObjects);
}

/** Toggle drawing logic connections as straight lines or curves. */
void WxKismet::OnButtonShowCurves(wxCommandEvent &In)
{
	bDrawCurves = In.IsChecked();
	RefreshViewport();
}

struct FNewSeqActivated
{
	FName					InternalInputOpName;
	INT						InternalInputIndex;
	TArray<FCopyConnInfo>	ExternalOutputs;
};
TArray<FNewSeqActivated> NewSeqInputs;

struct FNewFinishSeq
{
	FName					InternalOutputOpName;
	INT						InternalOutputIndex;
	TArray<FCopyConnInfo>	ExternalInputs;
};
TArray<FNewFinishSeq> NewSeqOutputs;


/** Add a connection from some output outside the selection set to some input inside the selection set. */
static void AddSeqInput(const FCopyConnInfo& ExternalOutput, const FCopyConnInfo& InternalInput)
{
	check(ExternalOutput.SeqObject->IsA(USequenceOp::StaticClass()));
	check(InternalInput.SeqObject->IsA(USequenceOp::StaticClass()));

	// See if there is already a connection for this (ie going to the internal input).
	for(INT i=0; i<NewSeqInputs.Num(); i++)
	{
		FNewSeqActivated& NewIn = NewSeqInputs(i);
		if( NewIn.InternalInputOpName == InternalInput.SeqObject->GetFName() &&
			NewIn.InternalInputIndex == InternalInput.ConnIndex )
		{
			// If so, add this external output to the list to connect to it, and return.
			NewIn.ExternalOutputs.AddItem(ExternalOutput);
			return;
		}
	}

	// Nothing currently connected to this input - make a new one.
	INT NewIndex = NewSeqInputs.AddZeroed();
	NewSeqInputs(NewIndex).InternalInputOpName = InternalInput.SeqObject->GetFName();
	NewSeqInputs(NewIndex).InternalInputIndex = InternalInput.ConnIndex;
	NewSeqInputs(NewIndex).ExternalOutputs.AddItem(ExternalOutput);
}

/** Add a connection from some output outside the selection set to some input inside the selection set. */
static void AddSeqOutput(const FCopyConnInfo& InternalOutput, const FCopyConnInfo& ExternalInput)
{
	check(InternalOutput.SeqObject->IsA(USequenceOp::StaticClass()));
	check(ExternalInput.SeqObject->IsA(USequenceOp::StaticClass()));

	for(INT i=0; i<NewSeqOutputs.Num(); i++)
	{
		FNewFinishSeq& NewOut = NewSeqOutputs(i);
		if( NewOut.InternalOutputOpName == InternalOutput.SeqObject->GetFName() &&
			NewOut.InternalOutputIndex == InternalOutput.ConnIndex )
		{
			NewOut.ExternalInputs.AddItem(ExternalInput);
			return;
		}
	}

	INT NewIndex = NewSeqOutputs.AddZeroed();
	NewSeqOutputs(NewIndex).InternalOutputOpName = InternalOutput.SeqObject->GetFName();
	NewSeqOutputs(NewIndex).InternalOutputIndex = InternalOutput.ConnIndex;
	NewSeqOutputs(NewIndex).ExternalInputs.AddItem(ExternalInput);
}

/** Utility for finding the index of an input that was created for the Sequence Activated Event in that sequence. */
static INT FindInputIndexForEvent(USequence* Seq, USeqEvent_SequenceActivated* Event)
{
	for(INT i=0; i<Seq->InputLinks.Num(); i++)
	{
		if(Seq->InputLinks(i).LinkedOp == Event)
		{
			return i;
		}
	}

	return INDEX_NONE;
}


/** Utility for finding the index of an output that was created for the Sequence Finished Action in that sequence. */
static INT FindOutputIndexForAction(USequence* Seq, USeqAct_FinishSequence* Action)
{
	for(INT i=0; i<Seq->OutputLinks.Num(); i++)
	{
		if(Seq->OutputLinks(i).LinkedOp == Action)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

/**
 * Iterates through all selected objects, removes them from their current
 * sequence and places them in a new one.
 */
void WxKismet::OnContextCreateSequence( wxCommandEvent& In )
{
	if(!SeqObjPosIsValid(LinkedObjVC->NewX, LinkedObjVC->NewY))
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd("NewObjectInvalidLocation"));
		return;
	}

	// pop up an option to name the new sequence
	WxDlgGenericStringEntry dlg;
	INT Result = dlg.ShowModal( TEXT("NameSequence"), TEXT("SequenceName"), TEXT("Sub_Sequence") );
	if (Result == wxID_OK)
	{
		FString NewSeqName = dlg.GetEnteredString();
		// validate the name
		FString Error;
		if (!FIsValidObjectName(FName(*NewSeqName), Error))
		{
			appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_ObjectNameIsIllegal"), *NewSeqName));
		}
		else
		{
			USequence* ExistingSequence = FindObject<USequence>(Sequence, *NewSeqName);
			if (ExistingSequence)
			{
				appMsgf(AMT_OK,LocalizeSecure(LocalizeUnrealEd("Error_ObjectNameAlreadyExists"), *ExistingSequence->GetFullName()));
			}
			else
			{
				USequence *NewSeq = NULL;
				{
					const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("NewSequence")) );
					FScopedObjectStateChange Notifier(Sequence);
					//Sequence->Modify();

					// create the new sequence
					NewSeq = ConstructObject<USequence>(USequence::StaticClass(), Sequence, *NewSeqName, RF_Transactional|Sequence->GetMaskedFlags(RF_ArchetypeObject|RF_Public));
					check(NewSeq != NULL && "Failed to create new sub sequence");
					NewSeq->ObjName = NewSeqName;
					NewSeq->ObjPosX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
					NewSeq->ObjPosY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;
					NewSeq->DefaultViewX = LinkedObjVC->Origin2D.X;
					NewSeq->DefaultViewY = LinkedObjVC->Origin2D.Y;
					NewSeq->DefaultViewZoom = LinkedObjVC->Zoom2D;
					NewSeq->SnapPosition(KISMET_GRIDSIZE, MaxSequenceSize);

					// add the new sequence to this sequence
					Sequence->AddSequenceObject(NewSeq);

					// Now we have created our new sub-sequence, we basically cut and paste our selected objects into it.

					// Add current selection to the selection set.
					USelection* SelectionSet = GEditor->GetSelectedObjects();
					SelectionSet->SelectNone( USequenceObject::StaticClass() );
					for(INT i=0; i<SelectedSeqObjs.Num(); i++)
					{
						SelectionSet->Select( SelectedSeqObjs(i) );
					}

					// Get the bounding box of the selection.
					FIntRect SelectionBox = CalcBoundingBoxOfSelected();

					NewSeqInputs.Empty();
					NewSeqOutputs.Empty();

					// Look for all ops that are not part of the selection but refer to part of the selection set
					// This results in an input (sequence activated) event
					for(INT ObjIndex=0; ObjIndex<Sequence->SequenceObjects.Num(); ObjIndex++)
					{
						USequenceOp* Op = Cast<USequenceOp>(Sequence->SequenceObjects(ObjIndex));
						if(Op && !SelectedSeqObjs.ContainsItem(Op))
						{
							for(INT OutputIndex=0; OutputIndex<Op->OutputLinks.Num(); OutputIndex++)
							{
								FSeqOpOutputLink& OutputLink = Op->OutputLinks(OutputIndex);
								for(INT LinkIndex=0; LinkIndex<OutputLink.Links.Num(); LinkIndex++)
								{
									USequenceOp* LinkedOp = OutputLink.Links(LinkIndex).LinkedOp;
									INT LinkedOpIndex = OutputLink.Links(LinkIndex).InputLinkIdx;
									if(SelectedSeqObjs.ContainsItem(LinkedOp))
									{
										AddSeqInput(FCopyConnInfo(Op, OutputIndex), FCopyConnInfo(LinkedOp, LinkedOpIndex));
									}
								}
							}
						}
					}

					// Look for all ops that are part of the selection but refer to something out of the the selection
					// This results in an output (sequence finished) action
					for(INT ObjIndex=0; ObjIndex<SelectedSeqObjs.Num(); ObjIndex++)
					{
						USequenceOp* Op = Cast<USequenceOp>(SelectedSeqObjs(ObjIndex));
						if(Op)
						{
							for(INT OutputIndex=0; OutputIndex<Op->OutputLinks.Num(); OutputIndex++)
							{
								FSeqOpOutputLink& OutputLink = Op->OutputLinks(OutputIndex);
								for(INT LinkIndex=0; LinkIndex<OutputLink.Links.Num(); LinkIndex++)
								{
									USequenceOp* LinkedOp = OutputLink.Links(LinkIndex).LinkedOp;
									INT LinkedOpIndex = OutputLink.Links(LinkIndex).InputLinkIdx;
									if(!SelectedSeqObjs.ContainsItem(LinkedOp))
									{
										AddSeqOutput(FCopyConnInfo(Op, OutputIndex), FCopyConnInfo(LinkedOp, LinkedOpIndex));
									}
								}
							}
						}
					}

					// Iterate over all objects making sure they import/export flags are unset. 
					// These are used for ensuring we export each object only once etc.
					for( FObjectIterator It; It; ++It )
					{
						It->ClearFlags( RF_TagImp | RF_TagExp );
					}

					// Export to text.
					FStringOutputDevice TempAr;
					const FExportObjectInnerContext Context;
					UExporter::ExportToOutputDevice( &Context, Sequence, NULL, TempAr, TEXT("copy"), 0, PPF_ExportsNotFullyQualified );

					SelectionSet->SelectNone( USequenceObject::StaticClass() );

					// Import into new sequence.
					const TCHAR* TempStr = *TempAr;
					USequenceFactory* Factory = new USequenceFactory;
					Factory->FactoryCreateText( NULL, NewSeq, NAME_None, RF_Transactional, NULL, TEXT(""), TempStr, TempStr+appStrlen(TempStr), GWarn );

					NewSeq->OnCreated();

					// May have disconnected some Matinee actions from their data...
					NewSeq->UpdateInterpActionConnectors();

					// Now create input/output actions/events in the new sequence
					INT SeqActivatedX = SelectionBox.Min.X - 250;
					INT SeqActivatedY = SelectionBox.Min.Y;
					// Array to keep track of newly created input actions
					TArray<USeqEvent_SequenceActivated*> NewActivated;
					NewActivated.Add(NewSeqInputs.Num());
					for(INT i=0; i<NewSeqInputs.Num(); i++)
					{
						NewActivated(i) = ConstructObject<USeqEvent_SequenceActivated>( USeqEvent_SequenceActivated::StaticClass(), NewSeq, NAME_None,	RF_Transactional);
						NewActivated(i)->ObjPosX = SeqActivatedX;
						NewActivated(i)->ObjPosY = SeqActivatedY;
						NewActivated(i)->InputLabel = FString::Printf(TEXT("AutoIn%d"), i);
						NewSeq->AddSequenceObject(NewActivated(i));
						NewActivated(i)->OnCreated();

						SeqActivatedY += 150;
					}

					INT SeqFinishedX = SelectionBox.Max.X + 100;
					INT SeqFinishedY = SelectionBox.Min.Y;
					// Array to keep track of newly created output actions
					TArray<USeqAct_FinishSequence*> NewFinished;
					NewFinished.Add(NewSeqOutputs.Num());
					for(INT i=0; i<NewSeqOutputs.Num(); i++)
					{
						NewFinished(i) = ConstructObject<USeqAct_FinishSequence>( USeqAct_FinishSequence::StaticClass(), NewSeq, NAME_None,	RF_Transactional);
						NewFinished(i)->ObjPosX = SeqFinishedX;
						NewFinished(i)->ObjPosY = SeqFinishedY;
						NewFinished(i)->OutputLabel= FString::Printf(TEXT("AutoOut%d"), i);
						NewSeq->AddSequenceObject(NewFinished(i));
						NewFinished(i)->OnCreated();

						SeqFinishedY += 55;
					}

					// Now we establish the connections

					// First, inputs (sequence activated events).
					for(INT i=0; i<NewSeqInputs.Num(); i++)
					{
						FNewSeqActivated& NewIn = NewSeqInputs(i);
						USeqEvent_SequenceActivated* Event = NewActivated(i);
						INT NewSeqInputIndex = FindInputIndexForEvent(NewSeq, Event);
						check(NewSeqInputIndex != INDEX_NONE);

						// Make internal connection from event to internal input
						// The op in the new sequence should have the same name as it had before it was copy/pasted in - so look for it
						USequenceOp* InternalOp = FindObject<USequenceOp>(NewSeq, *(NewIn.InternalInputOpName.ToString()));
						check(InternalOp);

						MakeLogicConnection(Event, 0, InternalOp, NewIn.InternalInputIndex);

						for(INT j=0; j<NewIn.ExternalOutputs.Num(); j++)
						{
							FCopyConnInfo& ConnInfo = NewIn.ExternalOutputs(j);
							MakeLogicConnection((USequenceOp*)ConnInfo.SeqObject, ConnInfo.ConnIndex, NewSeq, NewSeqInputIndex);
						}
					}

					// Then, outputs (sequence finished actions).
					for(INT i=0; i<NewSeqOutputs.Num(); i++)
					{
						FNewFinishSeq& NewOut = NewSeqOutputs(i);
						USeqAct_FinishSequence* Action = NewFinished(i);
						INT NewSeqOutputIndex = FindOutputIndexForAction(NewSeq, Action);
						check(NewSeqOutputIndex != INDEX_NONE);

						// Make internal connection from internal output to action
						// The op in the new sequence should have the same name as it had before it was copy/pasted in - so look for it
						USequenceOp* InternalOp = FindObject<USequenceOp>(NewSeq, *(NewOut.InternalOutputOpName.ToString()));
						check(InternalOp);
						MakeLogicConnection(InternalOp, NewOut.InternalOutputIndex, Action, 0);

						for(INT j=0; j<NewOut.ExternalInputs.Num(); j++)
						{
							FCopyConnInfo& ConnInfo = NewOut.ExternalInputs(j);
							MakeLogicConnection(NewSeq, NewSeqOutputIndex, (USequenceOp*)ConnInfo.SeqObject, ConnInfo.ConnIndex);
						}
					}

				} // ScopedTransaction

				// Delete the selected SequenceObjects that we copied into new sequence. 
				// This is its own undo transaction.
				DeleteSelected();

				// Select just the newly created sequence.
				EmptySelection();
				AddToSelection(NewSeq);
				UpdatePropertyWindow();
				NotifyObjectsChanged();

				// Rebuild the tree control to show new sequence.
				RebuildTreeControl();

				ClearCopiedConnections();
			}
		}
	}
}

/**
 * Takes the currently selected sequence and exports it to a package.
 */
void WxKismet::OnContextExportSequence( wxCommandEvent& In )
{
	// iterate through selected objects, looking for any sequences
	for (INT Idx = 0; Idx < SelectedSeqObjs.Num(); Idx++)
	{
		USequence *seq = Cast<USequence>(SelectedSeqObjs(Idx));
		if (seq != NULL)
		{
			// first get the package to export to
			// (TRUE for the constructor means to hide the redirect check box that doesn't apply)
			WxDlgRename renameDlg;
			renameDlg.SetTitle(*FString::Printf(LocalizeSecure(LocalizeUnrealEd("ExportSequence_F"),*seq->GetName())));
			FString newPkg, newGroup, newName = seq->GetName();
			if (renameDlg.ShowModal(newPkg,newGroup,newName) == wxID_OK)
			{
				// search for an existing object of that type
				if (renameDlg.GetNewName().Len() == 0 ||
					renameDlg.GetNewPackage().Len() == 0)
				{
					appMsgf(AMT_OK,*LocalizeUnrealEd("Error_MustSpecifyValidSeqName"));
				}
				else
				{
					UPackage *pkg = GEngine->CreatePackage(NULL,*renameDlg.GetNewPackage());
					pkg->SetFlags(RF_Standalone);
					if (renameDlg.GetNewGroup().Len() != 0)
					{
						pkg = GEngine->CreatePackage(pkg,*renameDlg.GetNewGroup());
						pkg->SetFlags(RF_Standalone);
					}

					// Iterate over all objects making sure they import/export flags are unset. 
					// These are used for ensuring we export each object only once etc.
					for( FObjectIterator It; It; ++It )
					{
						It->ClearFlags( RF_TagImp | RF_TagExp );
					}

					FStringOutputDevice TempAr;
					const FExportObjectInnerContext Context;
					UExporter::ExportToOutputDevice( &Context, seq, NULL, TempAr, TEXT("t3d"), 0, PPF_ExportsNotFullyQualified );

					// replace the Name=KismetSequenceName with Name=PackageSequenceName
					// this is so that the factory will create the sequence on top of any existing sequence
					// in the package properly
					FString OldStr(FString::Printf(TEXT("Name=%s"), *seq->GetName()));
					FString NewStr(FString::Printf(TEXT("Name=%s"), *renameDlg.GetNewName()));
					FString TextToImport = TempAr.Replace(*OldStr, *NewStr);

					const TCHAR* TempStr = *TextToImport;
					USequenceFactory* Factory = new USequenceFactory;
					UObject* NewObj = Factory->FactoryCreateText( NULL, pkg, NAME_None, RF_Transactional, NULL, TEXT(""), TempStr, TempStr+appStrlen(TempStr), GWarn );
					NewObj->SetFlags( RF_Public | RF_Standalone );

					check(NewObj);
					check(NewObj->GetOuter() == pkg);
					check(renameDlg.GetNewName() == NewObj->GetName());

					USequence* NewSeq = CastChecked<USequence>(NewObj);
				
					// Set name of new sequence in package to desired name.
					NewSeq->ObjName = NewSeq->GetName();

					// and clear any unwanted properties
					NewSeq->OnExport();

					// Mark the package we exported this into as dirty
					NewSeq->MarkPackageDirty();
				}
			}
		}
	}
}

/** Handler for importing a USequence from a package. Finds the first currently selected USequence and copies it into the current sequence. */
void WxKismet::OnContextImportSequence( wxCommandEvent &In )
{
	USequence *seq = Cast<USequence>(GEditor->GetSelectedObjects()->GetTop(USequence::StaticClass()));
	// make a copy of the sequence
	if (seq != NULL)
	{
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("ImportSequence") );
			FScopedObjectStateChange Notifier(Sequence);
			//Sequence->Modify();

			// Iterate over all objects making sure they import/export flags are unset. 
			// These are used for ensuring we export each object only once etc.
			for( FObjectIterator It; It; ++It )
			{
				It->ClearFlags( RF_TagImp | RF_TagExp );
			}

			FStringOutputDevice TempAr;
			const FExportObjectInnerContext Context;
			UExporter::ExportToOutputDevice( &Context, seq, NULL, TempAr, TEXT("t3d"), 0, PPF_ExportsNotFullyQualified );

			const TCHAR* TempStr = *TempAr;
			USequenceFactory* Factory = new USequenceFactory;
			UObject* NewObj = Factory->FactoryCreateText( NULL, Sequence, NAME_None, RF_Transactional, NULL, TEXT("paste"), TempStr, TempStr+appStrlen(TempStr), GWarn );
		
			check(NewObj);
			USequence* NewSeq = CastChecked<USequence>(NewObj);

			// and set the object position
			NewSeq->ObjPosX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
			NewSeq->ObjPosY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;
			NewSeq->SnapPosition(KISMET_GRIDSIZE, MaxSequenceSize);

			EmptySelection();
			AddToSelection(NewSeq);
			UpdatePropertyWindow();
			NotifyObjectsChanged();
		} // ScopedTransaction

		// Rebuild the tree to show new sequence.
		RebuildTreeControl();

		// Importing sequence may have changed named variable status, so update now.
		Sequence->UpdateNamedVarStatus();
		GetRootSequence()->UpdateInterpActionConnectors();
	}
}

/** Handler from context menu for pasting current selection of clipboard at mouse location. */
void WxKismet::OnContextPasteHere(wxCommandEvent& In)
{
	KismetPaste(TRUE);
}

/**
 * Sets the currently selected actor as the originator of all selected
 * events that the actor supports.
 */
void WxKismet::OnContextLinkEvent( wxCommandEvent& In )
{
	const FScopedTransaction Transaction( *LocalizeUnrealEd("LinkEvent") );

	USelection* SelectionSet = GEditor->GetSelectedActors();
	for( INT Idx = 0; Idx < SelectedSeqObjs.Num(); Idx++ )
	{
		USequenceEvent *Evt = Cast<USequenceEvent>(SelectedSeqObjs(Idx));
		if( Evt != NULL )
		{
			UBOOL bFoundMatch = FALSE;
			for( USelection::TObjectIterator It( SelectionSet->ObjectItor() ) ; It && !bFoundMatch ; ++It )
			{
				// if this actor supports this event
				AActor *actor = Cast<AActor>(*It);
				if( actor )
				{
					for( INT EvtIdx = 0; EvtIdx < actor->SupportedEvents.Num() && !bFoundMatch; EvtIdx++ )
					{
						if( actor->SupportedEvents(EvtIdx)->IsChildOf( Evt->GetClass() ) ||
							Evt->GetClass()->IsChildOf(actor->SupportedEvents(EvtIdx)) )
						{
							bFoundMatch = TRUE;
							break;
						}
					}

					if( bFoundMatch )
					{
						FScopedObjectStateChange Notifier(Evt);
						//Evt->Modify();

						Evt->Originator = actor;
						// reset the title bar
						Evt->ObjName = FString::Printf( TEXT("%s %s"), *actor->GetName(), *((USequenceObject*)Evt->GetClass()->GetDefaultObject())->ObjName );
					}
				}
			}
		}
	}
}

/**
 * Sets the currently selected actor as the value for all selected
 * object variables.
 */
void WxKismet::OnContextLinkObject( wxCommandEvent& In )
{
	const FScopedTransaction Transaction( *LocalizeUnrealEd("LinkObject") );

	AActor* SelectedActor = GEditor->GetSelectedActors()->GetTop<AActor>();
	for (INT Idx = 0; Idx < SelectedSeqObjs.Num(); Idx++)
	{
		USeqVar_Object *objVar = Cast<USeqVar_Object>(SelectedSeqObjs(Idx));
		if (objVar != NULL)
		{
			FScopedObjectStateChange Notifier(objVar);
			//objVar->Modify();
			objVar->ObjValue = SelectedActor;
		}
	}
}

void WxKismet::OnContextInsertIntoObjectList( wxCommandEvent& In )
{
	TArray<AActor*> SelectedActors;
	GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);

	// ADD OBJECTS TO LIST
	if( In.GetId() == IDM_KISMET_INSERT_ACTOR_TO_OBJLIST )
	{
		// For each selected kismet object
		for( INT ObjIdx = 0; ObjIdx < SelectedSeqObjs.Num(); ObjIdx++ )
		{
			// If the object is an object list
			USeqVar_ObjectList* objList = Cast<USeqVar_ObjectList>(SelectedSeqObjs(ObjIdx));
			if( objList )
			{
				// Go through each selected actor
				for( INT ActorIdx = 0; ActorIdx < SelectedActors.Num(); ActorIdx++ )
				{
					// If actor is valid
					if( SelectedActors(ActorIdx) )
					{
						//@fixme - need transaction tracking and property change propagation
						// Add to the list
						objList->ObjList.AddItem( SelectedActors(ActorIdx) );
					}
				}	
			}
		}
	}
	// REMOVE OBJECTS FROM LIST
	else if( In.GetId() == IDM_KISMET_REMOVE_ACTOR_FROM_OBJLIST )
	{
		// For each selected kismet object
		for( INT ObjIdx = 0; ObjIdx < SelectedSeqObjs.Num(); ObjIdx++ )
		{
			// If the object is an object list
			USeqVar_ObjectList* objList = Cast<USeqVar_ObjectList>(SelectedSeqObjs(ObjIdx));
			if( objList )
			{
				// Go through each selected actor
				for( INT ActorIdx = 0; ActorIdx < SelectedActors.Num(); ActorIdx++ )
				{
					// If actor is valid
					if( SelectedActors(ActorIdx) )
					{
						//@fixme - need transaction tracking and property change propagation
						// Remove from the list and update idx
						ActorIdx -= objList->ObjList.RemoveItem( SelectedActors(ActorIdx) );
					}
				}	
			}
		}
	}
}

/**
 * Pops up a rename dialog box for each currently selected sequence.
 */
void WxKismet::OnContextRenameSequence( wxCommandEvent& In )
{
	UBOOL bSuccess = FALSE;
	{
		FScopedTransaction Transaction( *LocalizeUnrealEd("RenameSequence") );

		TArray<USequence*> SelectedSequences;
		if ( ContainsObjectOfClass(SelectedSeqObjs, USequence::StaticClass(), FALSE, (TArray<USequenceObject*>*)&SelectedSequences) )
		{
			UBOOL bDisplayedPrefabSequenceWarning = FALSE;
			for ( INT SeqIndex = 0; SeqIndex < SelectedSequences.Num(); SeqIndex++ )
			{
				USequence* Seq = SelectedSequences(SeqIndex);
				if ( Seq->IsPrefabSequenceContainer() )
				{
					if ( !bDisplayedPrefabSequenceWarning )
					{
						appMsgf(AMT_OK, *LocalizeUnrealEd(TEXT("PrefabSequenceRenameError")));
						bDisplayedPrefabSequenceWarning = TRUE;
					}

					continue;
				}

				WxDlgGenericStringEntry dlg;
				INT Result = dlg.ShowModal( TEXT("NameSequence"), TEXT("SequenceName"), *Seq->GetName() );
				if (Result == wxID_OK)
				{
					FString NewSeqName = dlg.GetEnteredString();

					FString Error;
					if (!FIsValidObjectName(FName(*NewSeqName), Error))
					{
						appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_ObjectNameIsIllegal"), *NewSeqName));
					}
					else if (!Seq->Rename(*NewSeqName, Seq->GetOuter(), REN_Test))
					{
						appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_ObjectNameAlreadyExists"), *Seq->GetName()));
					}
					else
					{
						FScopedObjectStateChange Notifier(Seq);
						//Sequence->Modify();

						if ( Seq->Rename(*NewSeqName,Seq->GetOuter()) )
						{
							Seq->ObjName = Seq->GetName();
							bSuccess = TRUE;
						}
						else
						{
							Notifier.FinishEdit(TRUE);
						}
					}
				}
			}
		}

		if ( !bSuccess )
		{
			Transaction.Cancel();
		}
	} // ScopedTransaction

	if ( bSuccess )
	{
		RebuildTreeControl();
	}
}

/** Find the uses the selected named variable. */
void WxKismet::OnContextFindNamedVarUses( wxCommandEvent& In )
{
	// Only works if only 1 thing selected
	if(SelectedSeqObjs.Num() == 1)
	{
		// Must be a SequenceVariable
		USequenceVariable* SeqVar = Cast<USequenceVariable>( SelectedSeqObjs(0) );
		if(SeqVar && SeqVar->VarName != NAME_None)
		{
			DoSearch( *(SeqVar->VarName.ToString()), KST_VarNameUses, FALSE, KSS_AllLevels);
		}
	}
}

/** Find the variable(s) definition with the name used by the selected SeqVar_Named. */
void WxKismet::OnContextFindNamedVarDefs( wxCommandEvent& In )
{
	// Only works if only 1 thing selected
	if(SelectedSeqObjs.Num() == 1)
	{
		// Must be a USeqVar_Named
		USeqVar_Named* NamedVar = Cast<USeqVar_Named>( SelectedSeqObjs(0) );
		if(NamedVar && NamedVar->FindVarName != NAME_None)
		{
			DoSearch( *(NamedVar->FindVarName.ToString()), KST_VarName, TRUE );
		}
	}
}

void WxKismet::OnContextExposeVariable( wxCommandEvent& In )
{
	if (SelectedSeqObjs.Num() == 1)
	{
		INT Id = In.GetId() - IDM_KISMET_EXPOSE_VARIABLE_START;
		USequenceOp *Op = Cast<USequenceOp>(SelectedSeqObjs(0));
		// grab the expose info
		const FExposeVarLinkInfo *ExposeInfo = OpExposeVarLinkMap.Find(Id);
		// if we're exposing a property
		if (ExposeInfo->Type == FExposeVarLinkInfo::TYPE_Property)
		{
			if (ExposeInfo->VariableClass != NULL && ExposeInfo->Property != NULL)
			{
				FScopedTransaction Transaction(TEXT("Expose Variable Link"));
				FScopedObjectStateChange Notifier(Op);

				// create a new variable link
				INT VarIdx = Op->VariableLinks.AddZeroed();
				FSeqVarLink &VarLink = Op->VariableLinks(VarIdx);
				VarLink.LinkDesc = ExposeInfo->Property->GetName();
				VarLink.PropertyName = ExposeInfo->Property->GetFName();
				VarLink.ExpectedType = ExposeInfo->VariableClass;
				VarLink.MinVars = 0;
				VarLink.MaxVars = 255;
			}
		}
		else
		if (ExposeInfo->Type == FExposeVarLinkInfo::TYPE_HiddenLink)
		{
			FScopedTransaction Transaction(TEXT("Expose Variable Link"));
			FScopedObjectStateChange Notifier(Op);

			// otherwise just unhide the existing link
			Op->VariableLinks(ExposeInfo->LinkIdx).bHidden = FALSE;
		}
	}
}

void WxKismet::OnContextRemoveVariable( wxCommandEvent& In )
{
	if(ConnSeqOp != NULL && ConnType == LOC_VARIABLE &&
	   ConnIndex >= 0 && ConnIndex < ConnSeqOp->VariableLinks.Num())
	{
		FScopedTransaction Transaction(TEXT("Remove Variable Link"));	//@todo localize
		FScopedObjectStateChange Notifier(ConnSeqOp);

		// if this is linked to a property
		if (ConnSeqOp->VariableLinks(ConnIndex).PropertyName != NAME_None)
		{
			// then we can just remove the link
			ConnSeqOp->VariableLinks.Remove(ConnIndex,1);
		}
		else
		{
			// otherwise hide it
			ConnSeqOp->VariableLinks(ConnIndex).bHidden = TRUE;
		}
	}
}

/**
 * Unhide an output link.
 */
void WxKismet::OnContextExposeOutput( wxCommandEvent& In )
{
	if (SelectedSeqObjs.Num() == 1)
	{
		const INT OutputLinkIdx = In.GetId() - IDM_KISMET_EXPOSE_OUTPUT_START;
		USequenceOp *Op = Cast<USequenceOp>(SelectedSeqObjs(0));
		if (Op != NULL &&
			OutputLinkIdx >= 0 && OutputLinkIdx < Op->OutputLinks.Num())
		{
			Op->OutputLinks(OutputLinkIdx).bHidden = FALSE;
			Op->MarkPackageDirty();
		}
	}
}

/**
 * Hide an output link.
 */
void WxKismet::OnContextRemoveOutput( wxCommandEvent& In )
{
	if (ConnSeqOp != NULL && ConnType == LOC_OUTPUT &&
		ConnIndex >= 0 && ConnIndex < ConnSeqOp->OutputLinks.Num())
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd("HideOutput") );
		FScopedObjectStateChange Notifier(ConnSeqOp);

		//ConnSeqOp->Modify();
		ConnSeqOp->OutputLinks(ConnIndex).bHidden = TRUE;
	}
}

/**
 * Set the activation delay for an output link.
 */
void WxKismet::OnContextSetOutputDelay( wxCommandEvent& In )
{
	if (ConnSeqOp != NULL && ConnType == LOC_OUTPUT &&
		ConnIndex >= 0 && ConnIndex < ConnSeqOp->OutputLinks.Num())
	{
		WxDlgGenericStringEntry dlg;
		const INT Result = dlg.ShowModal( TEXT("SetOutputDelay"), TEXT("OutputDelayText"), TEXT("1.0") );
		if (Result == wxID_OK)
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("SetActivateDelay") );
			FScopedObjectStateChange Notifier(ConnSeqOp);

			//ConnSeqOp->Modify();
			ConnSeqOp->OutputLinks(ConnIndex).ActivateDelay = appAtof(*dlg.GetEnteredString());
		}
	}
}

/**
 * Set the activation delay for an input link.
 */
void WxKismet::OnContextSetInputDelay( wxCommandEvent& In )
{
	if (ConnSeqOp != NULL && ConnType == LOC_INPUT &&
		ConnIndex >= 0 && ConnIndex < ConnSeqOp->InputLinks.Num())
	{
		WxDlgGenericStringEntry dlg;
		const INT Result = dlg.ShowModal( TEXT("SetInputDelay"), TEXT("InputDelayText"), TEXT("1.0") );
		if (Result == wxID_OK)
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("SetActivateDelay") );
			FScopedObjectStateChange Notifier(ConnSeqOp);
			//ConnSeqOp->Modify();
			ConnSeqOp->InputLinks(ConnIndex).ActivateDelay = appAtof(*dlg.GetEnteredString());
		}
	}
}

/**
 * Activates the outer sequence of the current sequence.
 */
void WxKismet::OnButtonParentSequence( wxCommandEvent &In )
{
	CenterViewOnSeqObj(Sequence);
}

/**
 * Pops up a dialog box and renames the current sequence.
 */
void WxKismet::OnButtonRenameSequence( wxCommandEvent &In )
{
	if ( Sequence->IsPrefabSequenceContainer() )
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd(TEXT("PrefabSequenceRenameError")));
		return;
	}

	WxDlgGenericStringEntry dlg;
	const INT Result = dlg.ShowModal( TEXT("NameSequence"), TEXT("SequenceName"), *Sequence->GetName() );

	if (Result == wxID_OK)
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd("RenameSequence") );

		FString NewSeqName = dlg.GetEnteredString();
		NewSeqName = NewSeqName.Replace(TEXT(" "),TEXT("_"));

		if (!Sequence->Rename(*NewSeqName, Sequence->GetOuter(), REN_Test))
		{
			appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_ObjectNameAlreadyExists"), *Sequence->GetName()));
		}
		else
		{
			FScopedObjectStateChange Notifier(Sequence);
			//Sequence->Modify();

			Sequence->Rename(*NewSeqName,Sequence->GetOuter());
			Sequence->ObjName = Sequence->GetName();

			// reactivate with new name
			ChangeActiveSequence(Sequence);
		}
	}

	RebuildTreeControl();
}

/**
 * Toggles the "Search for SequencesObjects" tool.
 */
void WxKismet::OnOpenSearch(wxCommandEvent &In)
{
	if(!SearchWindow)
	{
		SearchWindow = new WxKismetSearch( this, -1 );
		SearchWindow->Show();

		ToolBar->ToggleTool(IDM_KISMET_OPENSEARCH, TRUE);
	}
	else
	{
		SearchWindow->Close();

		ToolBar->ToggleTool(IDM_KISMET_OPENSEARCH, FALSE);
	}
}

void WxKismet::OnOpenUpdate(wxCommandEvent &In)
{
	WxKismetUpdate *UpdateWindow = new WxKismetUpdate(this,-1);
	UpdateWindow->Show();
}

/**
 * Open the current level's root sequence in a new window.
 */
void WxKismet::OnOpenNewWindow( wxCommandEvent &In )
{
	OpenKismet( NULL, TRUE, GApp->EditorFrame );
}

/** 
 * Handler for pressing Search button (or pressing Enter on entry box) on the Kismet Search tool.
 * Searches all sequences and puts results into the ResultList list box.
 */
void WxKismet::OnDoSearch(wxCommandEvent &In)
{
	// Should not be able to call this if the Search dialog is not up!
	check(SearchWindow);

	// Save search settings to the editor user settings ini.
	SearchWindow->SaveSearchSettings();
	const EKismetSearchScope Scope = (EKismetSearchScope)SearchWindow->GetSearchScope();
	TArray<USequenceObject*> Results;
	DoSilentSearch(Results, SearchWindow->GetSearchString(), (EKismetSearchType)SearchWindow->GetSearchType(), Scope);
	UpdateSearchWindowResults(Results);
}

void WxKismet::UpdateSearchWindowResults(TArray<USequenceObject*> &NewResults)
{
	// Update the list box with search results.
	SearchWindow->ClearResultsList();

	for(INT Idx = 0; Idx < NewResults.Num(); Idx++)
	{
		USequenceObject *Obj = NewResults(Idx);
		if (Obj == NULL)
		{
			continue;
		}

		FString ResultName;

		// If there is a comment, put it after the object name in brackets.
		if(Obj->ObjComment.Len() > 0)
		{
			ResultName = FString::Printf( TEXT("%s (%s)"),*Obj->GetSeqObjFullName(),*Obj->ObjComment );
		}
		else
		{
			ResultName = Obj->GetSeqObjFullName();
		}

		// prepend the level name
		if ((EKismetSearchType)SearchWindow->GetSearchType() == KSS_AllLevels)
		{
			ULevel *Level = Cast<ULevel>(Obj->GetRootSequence()->GetOuter());
			SearchWindow->AppendResult( *FString::Printf(TEXT("%s: %s"),*Level->GetOuter()->GetOuter()->GetName(),*ResultName), Obj );
		}
		else
		{
			SearchWindow->AppendResult( *ResultName, Obj );
		}
	}
}

/**
 * Called when user clicks on a search result. Changes active Sequence
 * and moves the view to center on the selected SequenceObject.
 */
void WxKismet::OnSearchResultChanged(wxCommandEvent &In)
{
	SearchWindow->SaveSearchSettings();
	USequenceObject* SeqObj = SearchWindow->GetSelectedResult();
	if ( SeqObj )
	{
		CenterViewOnSeqObj( SeqObj );
	}
}


/** 
*	Run a silent search now. 
*	This will do a search without opening any search windows
*
* @param Results - the array to fill with search results
* @param SearchString - the substring to search for
* @param Type - indicates which fields to search
* @param Scope - the scope of the search, which defaults to the current sequence
*/

void WxKismet::DoSilentSearch(TArray<USequenceObject*> &Results, const FString SearchString, EKismetSearchType Type, EKismetSearchScope Scope)
{
	Results.Empty();

	const INT ScopeIndex = Scope;

	if(SearchString.Len() > 0 || Type == KST_RemoteEvents || Type == KST_ReferencesRemoteEvent)
	{
		const INT TypeIndex = Type;
		const FName SearchName = FName(*SearchString,FNAME_Find,TRUE);

		// Search the entire levels sequence.
		UBOOL bRecursiveSearch = FALSE;
		TArray<USequence*> SearchSeqs;
		// if all levels
		if (ScopeIndex == KSS_AllLevels)
		{
			bRecursiveSearch = TRUE;
			// then add the root sequence for each level
			for (TObjectIterator<ULevel> LevelIt; LevelIt; ++LevelIt)
			{
				// skip any levels from PIE
				if (GEditor->PlayWorld != NULL &&
					GEditor->PlayWorld->Levels.ContainsItem(*LevelIt))
				{
					continue;
				}
				if (LevelIt->GameSequences.Num() > 0)
				{
					SearchSeqs.AddItem(LevelIt->GameSequences(0));
				}
			}
		}
		else
		{
			// otherwise use the current sequence
			SearchSeqs.AddItem(GetRootSequence());
			bRecursiveSearch = (ScopeIndex == KSS_CurrentLevel ? TRUE : FALSE);
		}

		for (INT SeqIdx = 0; SeqIdx < SearchSeqs.Num(); SeqIdx++)
		{
			USequence *Seq = SearchSeqs(SeqIdx);
			if (Seq != NULL)
			{
				// Comment/Name
				if(TypeIndex == KST_NameComment)
				{
					Seq->FindSeqObjectsByName(SearchString, TRUE, Results, bRecursiveSearch);
				}
				// Reference Object
				else if(TypeIndex == KST_ObjName)
				{
					Seq->FindSeqObjectsByObjectName(SearchName, Results, bRecursiveSearch);
				}
				// Named Variable (uses and declarations)
				else if(TypeIndex == KST_VarName || TypeIndex == KST_VarNameUses)
				{
					TArray<USequenceVariable*> ResultVars;

					if(TypeIndex == KST_VarNameUses)
					{
						Seq->FindNamedVariables(SearchName, TRUE, ResultVars, bRecursiveSearch);
					}
					else
					{
						Seq->FindNamedVariables(SearchName, FALSE, ResultVars, bRecursiveSearch);
					}

					// Copy results into Results array.
					for(INT i=0; i<ResultVars.Num(); i++)
					{
						Results.AddItem( ResultVars(i) );
					}
				}
				else if (TypeIndex == KST_RemoteEvents)
				{
					// grab a list of all remote events
					TArray<USequenceObject*> Events;
					Seq->FindSeqObjectsByClass(USeqEvent_RemoteEvent::StaticClass(),Events,bRecursiveSearch);
					// for each event,
					for (INT EvtIdx = 0; EvtIdx < Events.Num(); EvtIdx++)
					{
						USeqEvent_RemoteEvent *Evt = Cast<USeqEvent_RemoteEvent>(Events(EvtIdx));
						// if it has a matching event name,
						if (Evt != NULL && (SearchString.Len() == 0 || Evt->EventName.ToString() == SearchString))
						{
							// add to the list of results
							Results.AddItem(Evt);
						}
					}
				}
				else if (TypeIndex == KST_ReferencesRemoteEvent)
				{
					// grab a list of all remote events
					TArray<USequenceObject*> Actions;
					Seq->FindSeqObjectsByClass(USeqAct_ActivateRemoteEvent::StaticClass(),Actions,bRecursiveSearch);
					// for each action,
					for (INT ActionIndex = 0; ActionIndex < Actions.Num(); ActionIndex++)
					{
						USeqAct_ActivateRemoteEvent *Act = Cast<USeqAct_ActivateRemoteEvent>(Actions(ActionIndex));
						// if it has a matching event name,
						if (Act != NULL && (SearchString.Len() == 0 || Act->EventName.ToString() == SearchString))
						{
							// add to the list of results
							Results.AddItem(Act);
						}
					}
				}
			}
		}
	}
}

/** 
 *	Run a search now. 
 *	This will open the search window if not already open, set the supplied settings and run the search.
 *	It will also jump to the first search result (if there are any) if you wish.
 */
void WxKismet::DoSearch(const TCHAR* InText, EKismetSearchType Type, UBOOL bJumpToFirstResult, EKismetSearchScope Scope)
{
	// clear a previous search if the window is open
	if (SearchWindow != NULL)
	{
		SearchWindow->ClearResultsList();
	}

	TArray<USequenceObject*> Results;
	DoSilentSearch(Results,InText,Type,Scope);

	// only update the search window if not automatically jumping to the result, or multiple results were found
	UBOOL bUpdateSearchWindow = !bJumpToFirstResult || Results.Num() > 1;

	if (bUpdateSearchWindow)
	{
		wxCommandEvent DummyEvent;
		// If search window is not currently open - open it now.
		if(!SearchWindow)
		{
			OnOpenSearch( DummyEvent );
		}
		if (SearchWindow != NULL)
		{
			// Enter the selected text into the search box, select search type, and start the search.
			SearchWindow->SetSearchString( InText );
			SearchWindow->SetSearchType( Type );
			SearchWindow->SetSearchScope( Scope );

			// and fill in the results instead of repeating the search
			UpdateSearchWindowResults(Results);

			// If we had some valid results, select the first one and move to it.
			if( bJumpToFirstResult )
			{
				// Highlight first element (this doesn't generate an Event, so we manually call OnSearchResultChanged afterwards).
				if ( SearchWindow->SetResultListSelection(0) )
				{
					OnSearchResultChanged( DummyEvent );
				}
			}
		}
	}
	else
	{
		if ( bJumpToFirstResult && Results.Num() > 0 )
		{
			CenterViewOnSeqObj(Results(0));
		}

	}
}

/** Update the NewObjActors and NewEventClasses arrays based on currently selected Actors. */
void WxKismet::BuildSelectedActorLists()
{
	// add menu for context sensitive events using the selected actors
	UBOOL bHaveClasses = FALSE;
	NewEventClasses.Empty();
	NewObjActors.Empty();

	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );
		NewObjActors.AddItem( Actor );

		// always refer to the default since the array may be out of date
		Actor = Actor->GetClass()->GetDefaultActor();

		// If first actor - add all the events it supports.
		if(!bHaveClasses)
		{
			NewEventClasses.Append(Actor->SupportedEvents);
			bHaveClasses = TRUE;
		}
		else // If a subsequent actor, remove any events from NewEventClasses which are not in this actors list.
		{
			INT EvtIdx = 0;
			while( EvtIdx < NewEventClasses.Num() )
			{
				UBOOL bFound = FALSE;
				for (INT j = 0; j < Actor->SupportedEvents.Num(); j++)
				{
					if (NewEventClasses(EvtIdx)->IsChildOf(Actor->SupportedEvents(j)))
					{
						bFound = TRUE;
						break;
					}
				}
				if (bFound)
				{
					EvtIdx++;
				}
				else
				{
					NewEventClasses.Remove(EvtIdx, 1);
				}
			}
		}
	}
}
	
/** 
 *	Calculate the bounding box that encompasses the selected SequenceObjects. 
 *	Does not produce sensible result if nothing is selected. 
 */
FIntRect WxKismet::CalcBoundingBoxOfSelected()
{
	FIntRect Result(0, 0, 0, 0);
	UBOOL bResultValid = FALSE;

	for(INT i=0; i<SelectedSeqObjs.Num(); i++)
	{
		FIntRect ObjBox = SelectedSeqObjs(i)->GetSeqObjBoundingBox();

		if(bResultValid)
		{
			// Expand result box to encompass
			Result.Min.X = ::Min(Result.Min.X, ObjBox.Min.X);
			Result.Min.Y = ::Min(Result.Min.Y, ObjBox.Min.Y);

			Result.Max.X = ::Max(Result.Max.X, ObjBox.Max.X);
			Result.Max.Y = ::Max(Result.Max.Y, ObjBox.Max.Y);
		}
		else
		{
			// Use this objects bounding box to initialise result.
			Result = ObjBox;
			bResultValid = TRUE;
		}
	}

	return Result;
}

/** 
 *	Calculate the bounding box that encompasses SequenceObjects in the current Sequence. 
 *	Does not produce sensible result if no objects in sequence. 
 */
FIntRect WxKismet::CalcBoundingBoxOfAll()
{
	FIntRect Result(0, 0, 0, 0);
	UBOOL bResultValid = FALSE;

	for(INT i=0; i<Sequence->SequenceObjects.Num(); i++)
	{
		FIntRect ObjBox = Sequence->SequenceObjects(i)->GetSeqObjBoundingBox();

		if(bResultValid)
		{
			Result.Min.X = ::Min(Result.Min.X, ObjBox.Min.X);
			Result.Min.Y = ::Min(Result.Min.Y, ObjBox.Min.Y);

			Result.Max.X = ::Max(Result.Max.X, ObjBox.Max.X);
			Result.Max.Y = ::Max(Result.Max.Y, ObjBox.Max.Y);
		}
		else
		{
			Result = ObjBox;
			bResultValid = TRUE;
		}
	}

	return Result;
}
