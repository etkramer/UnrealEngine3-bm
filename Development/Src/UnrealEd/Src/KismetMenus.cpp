/*=============================================================================
	KismetMenus.cpp: Menus/toolbars/dialogs for Kismet
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "UnLinkedObjEditor.h"
#include "Kismet.h"
#include "EngineSequenceClasses.h"
#include "UnLinkedObjDrawUtils.h"


/*-----------------------------------------------------------------------------
	WxKismetToolBar.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxKismetToolBar, wxToolBar )
END_EVENT_TABLE()

WxKismetToolBar::WxKismetToolBar( wxWindow* InParent, wxWindowID InID )
	: wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_3DBUTTONS )
{
	// create the return to parent sequence button
	ParentSequenceB.Load(TEXT("KIS_Parent"));
	RenameSequenceB.Load(TEXT("KIS_Rename"));
	HideB.Load(TEXT("KIS_HideConnectors"));
	ShowB.Load(TEXT("KIS_ShowConnectors"));
	CurvesB.Load(TEXT("KIS_DrawCurves"));
	SearchB.Load( TEXT("KIS_Search") );
	ZoomToFitB.Load( TEXT("KIS_ZoomToFit") );
	UpdateB.Load(TEXT("KIS_Update"));
	OpenB.Load(TEXT("Kismet"));

	SetToolBitmapSize( wxSize( 18, 18 ) );

	AddSeparator();
	AddTool(IDM_KISMET_PARENTSEQ, ParentSequenceB, *LocalizeUnrealEd("OpenParentSequence"));
	AddTool(IDM_KISMET_RENAMESEQ, RenameSequenceB, *LocalizeUnrealEd("RenameSequence"));
	AddSeparator();
	AddTool(IDM_KISMET_BUTTON_ZOOMTOFIT, ZoomToFitB, *LocalizeUnrealEd("ZoomToFit"));
	AddSeparator();
	AddTool(IDM_KISMET_BUTTON_HIDE_CONNECTORS, HideB, *LocalizeUnrealEd("HideUnusedConnectors"));
	AddTool(IDM_KISMET_BUTTON_SHOW_CONNECTORS, ShowB, *LocalizeUnrealEd("ShowAllConnectors"));
	AddSeparator();
	AddCheckTool(IDM_KISMET_BUTTON_SHOW_CURVES, *LocalizeUnrealEd("ToggleCurvedConnections"), CurvesB, wxNullBitmap, *LocalizeUnrealEd("ToggleCurvedConnections"));
	ToggleTool(IDM_KISMET_BUTTON_SHOW_CURVES, true);
	AddSeparator();
	AddCheckTool(IDM_KISMET_OPENSEARCH, *LocalizeUnrealEd("SearchTool"), SearchB, wxNullBitmap, *LocalizeUnrealEd("SearchTool") );
	AddCheckTool(IDM_KISMET_OPENUPDATE, *LocalizeUnrealEd("UpdateList"), UpdateB, wxNullBitmap, *LocalizeUnrealEd("UpdateList") );
	AddSeparator();
	AddTool(IDM_KISMET_OPENNEWWINDOW, OpenB, *LocalizeUnrealEd("OpenNewKismetWindow"));

	Realize();
}

WxKismetToolBar::~WxKismetToolBar()
{
}

/*-----------------------------------------------------------------------------
	WxKismetStatusBar.
-----------------------------------------------------------------------------*/

WxKismetStatusBar::WxKismetStatusBar( wxWindow* InParent, wxWindowID InID )
	: wxStatusBar( InParent, InID )
{
	INT Widths[2] = {-1, 150};

	SetFieldsCount(2, Widths);

	SetStatusText( *LocalizeUnrealEd("ObjectNone"), 0 );
}

WxKismetStatusBar::~WxKismetStatusBar()
{

}

void WxKismetStatusBar::SetMouseOverObject(USequenceObject* SeqObj)
{
	FString ObjInfo(*LocalizeUnrealEd("ObjectNone"));
	if(SeqObj)
	{
#if _DEBUG
		FString DebugObjectName = FString::Printf(TEXT("%s (%s)"), *SeqObj->ObjName, *SeqObj->GetFullName());
		ObjInfo = FString::Printf( LocalizeSecure(LocalizeUnrealEd("Object_F"), *DebugObjectName) );
#else
		ObjInfo = FString::Printf( LocalizeSecure(LocalizeUnrealEd("Object_F"), *SeqObj->ObjName) );
#endif
	}

	SetStatusText( *ObjInfo, 0 );
}

/*-----------------------------------------------------------------------------
	WxMBKismetNewObject.
-----------------------------------------------------------------------------*/

/**
 * Looks for a category in the op's name, returns TRUE if one is found.
 */
static UBOOL GetSequenceObjectCategory(USequenceObject *Op, FString &CategoryName, FString &OpName)
{
	OpName = Op->ObjName;
	if (Op->ObjCategory.Len() > 0)
	{
		CategoryName = Op->ObjCategory;
		return TRUE;
	}
	return FALSE;
}

WxMBKismetNewObject::WxMBKismetNewObject(WxKismet* SeqEditor)
{
	// create the top level menus
	ActionMenu = new wxMenu();
	Append( IDM_KISMET_NEW_ACTION, *LocalizeUnrealEd("NewAction"), ActionMenu );
	ConditionMenu = new wxMenu();
	VariableMenu = new wxMenu();
	EventMenu = new wxMenu();
	// set up map of sequence object classes to their respective menus
	TMap<UClass*,wxMenu*> MenuMap;
	MenuMap.Set(USequenceAction::StaticClass(),ActionMenu);
	MenuMap.Set(USequenceCondition::StaticClass(),ConditionMenu);
	MenuMap.Set(USequenceVariable::StaticClass(),VariableMenu);
	MenuMap.Set(USequenceEvent::StaticClass(),EventMenu);
	// list of categories for all the submenus
	TMap<FString,wxMenu*> ActionSubmenus;
	TMap<FString,wxMenu*> ConditionSubmenus;
	TMap<FString,wxMenu*> VariableSubmenus;
	TMap<FString,wxMenu*> EventSubmenus;
	// map all the submenus to their parent menu
	TMap<wxMenu*,TMap<FString,wxMenu*>*> SubmenuMap;
	SubmenuMap.Set(ActionMenu,&ActionSubmenus);
	SubmenuMap.Set(ConditionMenu,&ConditionSubmenus);
	SubmenuMap.Set(VariableMenu,&VariableSubmenus);
	SubmenuMap.Set(EventMenu,&EventSubmenus);
	// iterate through all known sequence object classes
	INT CommentIndex = INDEX_NONE;
	INT WrappedCommentIndex = INDEX_NONE;
	FString CategoryName, OpName;
	for(INT i=0; i<SeqEditor->SeqObjClasses.Num(); i++)
	{
		UClass *SeqClass = SeqEditor->SeqObjClasses(i);
		if (SeqClass != NULL)
		{
			// special cases
			if( SeqEditor->SeqObjClasses(i)->IsChildOf(USeqAct_Interp::StaticClass()) )
			{
				Append(IDM_NEW_SEQUENCE_OBJECT_START+i, *FString::Printf(TEXT("New %s"), *((USequenceObject*)SeqClass->GetDefaultObject())->ObjName), TEXT(""));
			}
			else if( SeqEditor->SeqObjClasses(i) == USequenceFrame::StaticClass() )
			{
				CommentIndex = i;
			}
			else if( SeqEditor->SeqObjClasses(i) == USequenceFrameWrapped::StaticClass() )
			{
				WrappedCommentIndex = i;
			}
			else
			{
				// grab the default object for reference
				USequenceObject* SeqObjDefault = (USequenceObject*)SeqClass->GetDefaultObject();
				// Skip item if it should be removed from this project
				if( SeqObjDefault->ObjRemoveInProject.ContainsItem( appGetGameName() ) )
				{
					continue;
				}				

				// first look up the menu type
				wxMenu* Menu = NULL;
				for (TMap<UClass*,wxMenu*>::TIterator It(MenuMap); It && Menu == NULL; ++It)
				{
					UClass *TestClass = It.Key();
					if (SeqClass->IsChildOf(TestClass))
					{
						Menu = It.Value();
					}
				}
				if (Menu != NULL)
				{
					// check for a category
					if (GetSequenceObjectCategory(SeqObjDefault,CategoryName,OpName))
					{
						// look up the submenu map
						TMap<FString,wxMenu*>* Submenus = SubmenuMap.FindRef(Menu);
						if (Submenus != NULL)
						{
							wxMenu* Submenu = Submenus->FindRef(CategoryName);
							if (Submenu == NULL)
							{
								// create a new submenu for the category
								Submenu = new wxMenu();
								// add it to the map
								Submenus->Set(*CategoryName,Submenu);
								// and add it to the parent menu
								Menu->Append(IDM_KISMET_NEW_ACTION_CATEGORY_START+(Submenus->Num()),*CategoryName, Submenu);
							}
							// add the object to the submenu
							Submenu->Append(IDM_NEW_SEQUENCE_OBJECT_START+i,*OpName,TEXT(""));
						}
					}
					else
					{
						// otherwise add it to the parent menu
						Menu->Append(IDM_NEW_SEQUENCE_OBJECT_START+i,*OpName,TEXT(""));
					}
				}
			}
		}
	}

	Append( IDM_KISMET_NEW_CONDITION, *LocalizeUnrealEd("NewCondition"), ConditionMenu );
	Append( IDM_KISMET_NEW_VARIABLE, *LocalizeUnrealEd("NewVariable"), VariableMenu );
	Append( IDM_KISMET_NEW_EVENT, *LocalizeUnrealEd("NewEvent"), EventMenu );

	if(CommentIndex != INDEX_NONE)
	{
		AppendSeparator();
		Append( IDM_NEW_SEQUENCE_OBJECT_START+CommentIndex, *LocalizeUnrealEd("NewComment"), TEXT("") );
		if(WrappedCommentIndex != INDEX_NONE)
		{
			Append( IDM_NEW_SEQUENCE_OBJECT_START+WrappedCommentIndex, *LocalizeUnrealEd("NewWrappedComment"), TEXT("") );
		}
	}

	// Update SeqEditor->NewEventClasses and SeqEditor->NewObjActors.
	SeqEditor->BuildSelectedActorLists();

	// Add 'New Event Using' item.
	if(SeqEditor->NewEventClasses.Num() > 0)
	{
		// Don't add items, unless all actors belong to the sequence's level.
		ULevel* SequenceLevel		= SeqEditor->Sequence->GetLevel();
		UBOOL bCrossLevelReferences	= FALSE;
		for( INT ActorIndex = 0 ; ActorIndex < SeqEditor->NewObjActors.Num() ; ++ActorIndex )
		{
			AActor* Actor			= SeqEditor->NewObjActors(ActorIndex);
			ULevel* ActorLevel		= Actor->GetLevel();
			if ( ActorLevel != SequenceLevel )
			{
				bCrossLevelReferences = TRUE;
				break;
			}
		}
		if ( !bCrossLevelReferences )
		{
			AppendSeparator();
			ContextEventMenu = new wxMenu();

			FString NewEventString;
			FString NewObjVarString;

			FString FirstObjName = *SeqEditor->NewObjActors(0)->GetName();
			if( SeqEditor->NewObjActors.Num() > 1 )
			{
				NewEventString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("NewEventsUsing_F"), *FirstObjName) );
				NewObjVarString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("NewObjectVarsUsing_F"), *FirstObjName) );
			}
			else
			{
				NewEventString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("NewEventUsing_F"), *FirstObjName) );
				NewObjVarString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("NewObjectVarUsing_F"), *FirstObjName) );
			}

			Append( IDM_KISMET_NEW_VARIABLE_OBJ_CONTEXT, *NewObjVarString, TEXT("") );
			SeqEditor->bAttachVarsToConnector = false;

			Append( IDM_KISMET_NEW_EVENT_CONTEXT, *NewEventString, ContextEventMenu );

			for(INT i=0; i<SeqEditor->NewEventClasses.Num(); i++)
			{
				USequenceEvent* SeqEvtDefault = (USequenceEvent*)SeqEditor->NewEventClasses(i)->GetDefaultObject();
				ContextEventMenu->Append( IDM_NEW_SEQUENCE_EVENT_START+i, *SeqEvtDefault->ObjName, TEXT("") );
			}
		}
	}
	
	AppendSeparator();

	// Append the option to make a new sequence
	FString NewSeqString = FString::Printf( TEXT("%s: %d Objs"), *LocalizeUnrealEd("CreateNewSequence"), SeqEditor->SelectedSeqObjs.Num() );
	Append(IDM_KISMET_CREATE_SEQUENCE,*NewSeqString,TEXT(""));

	// Append option for pasting clipboard at current mouse location
	Append(IDM_KISMET_PASTE_HERE, *LocalizeUnrealEd("PasteHere"), TEXT(""));

	// append option to import a new sequence
	USequence *SelectedSeq = Cast<USequence>(GEditor->GetSelectedObjects()->GetTop(USequence::StaticClass()));
	if (SelectedSeq != NULL)
	{
		// Disallow importing into cooked level packages.
		const UPackage* SeqPackage = SelectedSeq->GetOutermost();
		if ( !(SeqPackage->PackageFlags & PKG_Cooked) )
		{
			Append(IDM_KISMET_IMPORT_SEQUENCE,*FString::Printf(LocalizeSecure(LocalizeUnrealEd("ImportSequence_F"),*SelectedSeq->GetName())),TEXT(""));
		}
	}

}

WxMBKismetNewObject::~WxMBKismetNewObject()
{
}

/*-----------------------------------------------------------------------------
	WxMBKismetObjectOptions.
-----------------------------------------------------------------------------*/

WxMBKismetObjectOptions::WxMBKismetObjectOptions(WxKismet* SeqEditor)
{
	// look for actions that need to be updated
	USequenceOp *UpdateOp = NULL;
	for (INT idx = 0; idx < SeqEditor->SelectedSeqObjs.Num() && UpdateOp == NULL; idx++)
	{
		USequenceOp *Op = Cast<USequenceOp>(SeqEditor->SelectedSeqObjs(idx));
		if (Op != NULL && !Op->IsA(USequence::StaticClass()) && Op->eventGetObjClassVersion() != Op->ObjInstanceVersion)
		{
			UpdateOp = Op;
		}
	}
	if (UpdateOp != NULL)
	{
		Append(IDM_KISMET_UPDATE_ACTION, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("UpdateLatest_F"),*UpdateOp->ObjName)), TEXT(""));
		AppendSeparator();
	}

	NewVariableMenu = NULL;

	// If one Op is selected, and there are classes of optional variable connectors defined, create menu for them.
	if(SeqEditor->SelectedSeqObjs.Num() == 1)
	{
		UBOOL bSeparate = FALSE;
		USequenceOp* SeqOp = Cast<USequenceOp>( SeqEditor->SelectedSeqObjs(0) );
	
		USeqAct_Interp* Interp = Cast<USeqAct_Interp>( SeqOp );
		if(Interp)
		{
			Append( IDM_KISMET_OPEN_INTERPEDIT, *LocalizeUnrealEd("OpenMatinee"), TEXT("") );
			bSeparate = TRUE;
		}

		// If the operation is a switch class, give options for adding a switch node
		USeqCond_SwitchBase* Switch = Cast<USeqCond_SwitchBase>(SeqOp);
		if( Switch )
		{
			Append( IDM_KISMET_SWITCH_ADD, *LocalizeUnrealEd("AddSwitchConnector"), TEXT("") );
			bSeparate = TRUE;
		}

		USeqAct_ActivateRemoteEvent *Act = Cast<USeqAct_ActivateRemoteEvent>(SeqOp);
		if (Act != NULL)
		{
			Append(IDM_KISMET_SEARCH_REMOTEEVENTS, *LocalizeUnrealEd("SearchRemoteEvents"), TEXT(""));
			bSeparate = TRUE;
		}


		if( bSeparate )
		{
			AppendSeparator();
		}
	}

	UBOOL bNeedsSeparator = FALSE;

	// search for the first selected event
	USequenceEvent *selectedEvt = NULL;
	USeqVar_Object *selectedObjVar = NULL;
	SeqEditor->SelectedSeqObjs.FindItemByClass<USequenceEvent>(&selectedEvt);
	SeqEditor->SelectedSeqObjs.FindItemByClass<USeqVar_Object>(&selectedObjVar);
	AActor *selectedActor = GEditor->GetSelectedActors()->GetTop<AActor>();
	AActor *AssignedActor = NULL;
	INT AssignedActorCnt = 0;
	if(selectedEvt != NULL)
	{
		// add option to link selected actor to all selected events
		if (selectedActor != NULL)
		{
			// make sure the selected actor supports the event selected
			UBOOL bFoundMatch = 0;
			for (INT idx = 0; idx < selectedActor->SupportedEvents.Num() && !bFoundMatch; idx++)
			{
				if (selectedActor->SupportedEvents(idx)->IsChildOf(selectedEvt->GetClass()) ||
					selectedEvt->GetClass()->IsChildOf(selectedActor->SupportedEvents(idx)))
				{
					bFoundMatch = 1;
				}
			}
			if (bFoundMatch)
			{
				Append(IDM_KISMET_LINK_ACTOR_TO_EVT, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("AssignToEvents_F"),*selectedActor->GetName())), TEXT(""));
				bNeedsSeparator = TRUE;
			}
		}
		if (selectedEvt->Originator != NULL)
		{
			AssignedActor = selectedEvt->Originator;
		}
	}
	if (selectedObjVar != NULL &&
		selectedActor != NULL)
	{
		bNeedsSeparator = TRUE;
		Append(IDM_KISMET_LINK_ACTOR_TO_OBJ, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("AssignToObjectVars_F"),*selectedActor->GetName())), TEXT(""));

		// If working with an object list
		USeqVar_ObjectList* ObjList = Cast<USeqVar_ObjectList>(selectedObjVar);
		if( ObjList )
		{
			TArray<AActor*> SelectedActors;
			GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);
			
			// Give option to insert selected actor into the list
			Append( IDM_KISMET_INSERT_ACTOR_TO_OBJLIST, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("InsertToObjectListVars_F"), SelectedActors.Num() > 1 ? TEXT("Selected Actors") : *selectedActor->GetName())), TEXT(""));
			Append( IDM_KISMET_REMOVE_ACTOR_FROM_OBJLIST, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("RemoveFromObjectListVars_F"), SelectedActors.Num() > 1 ? TEXT("Selected Actors") : *selectedActor->GetName())), TEXT(""));
		}
	}
	
	if (AssignedActor == NULL &&
		selectedObjVar != NULL &&
		Cast<AActor>(selectedObjVar->ObjValue) != NULL)
	{
		AssignedActor = Cast<AActor>(selectedObjVar->ObjValue);
	}

	if (AssignedActor != NULL)
	{
		Append(IDM_KISMET_SELECT_ASSIGNED, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("SelectAssigned_F"),*AssignedActor->GetName())), TEXT(""));
		bNeedsSeparator = TRUE;
	}

	// if we have an object var selected,
	if (selectedObjVar != NULL)
	{
		// add an option to clear it's contents
		Append(IDM_KISMET_CLEAR_VARIABLE, *LocalizeUnrealEd("ClearObjectVars"), TEXT(""));
		bNeedsSeparator = TRUE;
	}

	// Add options for finding definitions/uses of named variables.
	if( SeqEditor->SelectedSeqObjs.Num() == 1 )
	{
		USequenceVariable* SeqVar = Cast<USequenceVariable>( SeqEditor->SelectedSeqObjs(0) );
		if(SeqVar && SeqVar->VarName != NAME_None)
		{
			FString FindUsesString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("FindUses_F"), *(SeqVar->VarName.ToString())) );
			Append( IDM_KISMET_FIND_NAMEDVAR_USES, *FindUsesString, TEXT("") );
			bNeedsSeparator = TRUE;
		}

		USeqVar_Named* NamedVar = Cast<USeqVar_Named>( SeqEditor->SelectedSeqObjs(0) );
		if(NamedVar && NamedVar->FindVarName != NAME_None)
		{
			FString FindDefsString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("FindDefinition_F"), *(NamedVar->FindVarName.ToString())) );
			Append( IDM_KISMET_FIND_NAMEDVAR, *FindDefsString, TEXT("") );
			bNeedsSeparator = TRUE;
		}

		USequenceOp *Op = Cast<USequenceOp>(SeqEditor->SelectedSeqObjs(0));
		if (Op != NULL)
		{
			// build the list of variable classes
			//@fixme - this is retarded!!!!
			TArray<UClass*> VariableClasses;
			for (TObjectIterator<UClass> It; It; ++It)
			{
				if (It->IsChildOf(USequenceVariable::StaticClass()))
                {
					VariableClasses.AddItem(*It);
				}
			}
			// get a list of all properties for this class
			TArray<UProperty*> OpProperties;
			SeqEditor->OpExposeVarLinkMap.Empty();
			UClass *SearchClass = Op->GetClass();
			while (SearchClass != NULL &&
				   SearchClass->IsChildOf(USequenceOp::StaticClass()))
			{
				for (TFieldIterator<UProperty,CASTCLASS_None,FALSE> It(SearchClass); It; ++It)
				{
					if (It->PropertyFlags & CPF_Edit)
					{
						OpProperties.AddItem(*It);
					}
				}
				SearchClass = SearchClass->GetSuperClass();
			}
			// check each property to see if there is a variable link for it
			for (INT Idx = 0; Idx < OpProperties.Num(); Idx++)
			{
				for (INT VarIdx = 0; VarIdx < Op->VariableLinks.Num(); VarIdx++)
				{
					if (Op->VariableLinks(VarIdx).PropertyName == OpProperties(Idx)->GetFName())
					{
						// already has a link exposed, remove from the list
						OpProperties.Remove(Idx--,1);
						break;
					}
				}
			}
			// look for a list of variable links that are just hidden
			TArray<INT> HiddenVariableLinks;
			for (INT Idx = 0; Idx < Op->VariableLinks.Num(); Idx++)
			{
				if (Op->VariableLinks(Idx).bHidden)
				{
					HiddenVariableLinks.AddItem(Idx);
				}
			}
			UBOOL bAppendedMenu = FALSE;
			// if we still have properties left to expose
			if (OpProperties.Num() > 0 || HiddenVariableLinks.Num() > 0)
			{
				bAppendedMenu = TRUE;
				if ( bNeedsSeparator )
				{
					bNeedsSeparator = FALSE;
					AppendSeparator();
				}
				// create the submenu
				wxMenu *ExposeMenu = new wxMenu();
				INT MapId = 0;
				for (INT Idx = 0; Idx < OpProperties.Num(); Idx++)
				{
					for (INT VarIdx = 0; VarIdx < VariableClasses.Num(); VarIdx++)
					{
						USequenceVariable *DefaultVar = (USequenceVariable*)(VariableClasses(VarIdx)->GetDefaultObject());
						if (DefaultVar != NULL &&
							DefaultVar->SupportsProperty(OpProperties(Idx)))
						{
							// if dealing w/ object property and object variable, check validity against the supported classes array
							UObjectProperty *ObjProp = OpProperties(Idx)->IsA(UArrayProperty::StaticClass()) ? Cast<UObjectProperty>(((UArrayProperty*)OpProperties(Idx))->Inner) : Cast<UObjectProperty>(OpProperties(Idx));
							USeqVar_Object *ObjVar = Cast<USeqVar_Object>(DefaultVar);
							if (ObjProp != NULL && ObjVar != NULL)
							{
								UBOOL bIsValidChildClass = FALSE;
								UClass *PropClass = ObjProp->PropertyClass;
								for (INT ClassIdx = 0; ClassIdx < ObjVar->SupportedClasses.Num(); ClassIdx++)
								{
									if (PropClass->IsChildOf(ObjVar->SupportedClasses(ClassIdx)))
									{
										bIsValidChildClass = TRUE;
										break;
									}
								}
								if (!bIsValidChildClass)
								{
									// not a valid child so skip adding an entry to expose
									continue;
								}

							}
							// add a property expose entry
							SeqEditor->OpExposeVarLinkMap.Set(MapId,FExposeVarLinkInfo(OpProperties(Idx),VariableClasses(VarIdx)));
							// and add it to the menu
							ExposeMenu->Append(IDM_KISMET_EXPOSE_VARIABLE_START+MapId,*FString::Printf(TEXT("%s - %s"),*OpProperties(Idx)->GetName(),*DefaultVar->ObjName),TEXT(""));
							// increment the id
							MapId++;
						}
					}
				}
				// add any hidden variable links
				for (INT Idx = 0; Idx < HiddenVariableLinks.Num(); Idx++)
				{
					INT VarLinkIdx = HiddenVariableLinks(Idx);
					USequenceVariable *DefaultVar = (USequenceVariable*)(Op->VariableLinks(VarLinkIdx).ExpectedType->GetDefaultObject());
					SeqEditor->OpExposeVarLinkMap.Set(MapId,FExposeVarLinkInfo(VarLinkIdx));
					// and add it to the menu
					ExposeMenu->Append(IDM_KISMET_EXPOSE_VARIABLE_START+MapId,*FString::Printf(TEXT("%s - %s *"),*Op->VariableLinks(VarLinkIdx).LinkDesc,*DefaultVar->ObjName),TEXT(""));
					// increment the id
					MapId++;
				}
				Append(-1,*LocalizeUnrealEd("ExposeVariable"),ExposeMenu);
			}

			// check for output links to expose
			TArray<INT> OutputIndices;
			for (INT Idx = 0; Idx < Op->OutputLinks.Num(); Idx++)
			{
				if (Op->OutputLinks(Idx).bHidden)
				{
					OutputIndices.AddItem(Idx);
				}
			}
			if (OutputIndices.Num() > 0)
			{
				bAppendedMenu = TRUE;
				if ( bNeedsSeparator )
				{
					bNeedsSeparator = FALSE;
					AppendSeparator();
				}
				wxMenu *ExposeMenu = new wxMenu();
				for (INT Idx = 0; Idx < OutputIndices.Num(); Idx++)
				{
					ExposeMenu->Append(IDM_KISMET_EXPOSE_OUTPUT_START+OutputIndices(Idx),*(Op->OutputLinks(OutputIndices(Idx)).LinkDesc));
				}
				Append(-1,*LocalizeUnrealEd("ExposeOutput"),ExposeMenu);
			}
			if (bAppendedMenu)
			{
				AppendSeparator();
			}
		}

	}

	Append(IDM_KISMET_BREAK_ALL_OP_LINKS,*LocalizeUnrealEd("BreakAllOpLinks"),TEXT(""));

	AppendSeparator();

	// add options to hide unused connectors or show all connectors
	Append(IDM_KISMET_HIDE_CONNECTORS,*LocalizeUnrealEd("HideUnusedConnectors"),TEXT(""));
	Append(IDM_KISMET_SHOW_CONNECTORS,*LocalizeUnrealEd("ShowAllConnectors"),TEXT(""));

	AppendSeparator();

    // add option to create a new sequence
	Append(IDM_KISMET_CREATE_SEQUENCE,*LocalizeUnrealEd("CreateNewSequence"),TEXT(""));
	if ( ContainsObjectOfClass(SeqEditor->SelectedSeqObjs, USequence::StaticClass()) )
	{
		// and the option to rename
		Append(IDM_KISMET_RENAMESEQ,*LocalizeUnrealEd("RenameSelectedSequence"),TEXT(""));
		// if we have a sequence selected, add the option to export
		Append(IDM_KISMET_EXPORT_SEQUENCE,*LocalizeUnrealEd("ExportSequenceToPackage"),TEXT(""));
	}

	// Add 'apply style' menu
	USequenceFrame *SelectedComment = NULL;
	SeqEditor->SelectedSeqObjs.FindItemByClass<USequenceFrame>(&SelectedComment);
	if(SelectedComment)
	{
		// Make menu for all styles
		wxMenu* StyleMenu = new wxMenu();
	
		// Make a menu entry for each comment preset.
		UKismetBindings* Bindings = (UKismetBindings*)(UKismetBindings::StaticClass()->GetDefaultObject());
		for(INT i=0; i<Bindings->CommentPresets.Num(); i++)
		{
			FKismetCommentPreset& Preset = Bindings->CommentPresets(i);
			StyleMenu->Append(IDM_KISMET_APPLY_COMMENT_STYLE_START+i, *Preset.PresetName.ToString(), TEXT(""));
		}

		// Add style menu to context menu
		AppendSeparator();
		Append(IDM_KISMET_APPLY_COMMENT_STYLE_MENU, *LocalizeUnrealEd("ApplyCommentStyle"), StyleMenu);
	
		// Add menu options for moving a comment forwards/backwards in the stack.
		// Only really well defined if one object is selected.
		if(SeqEditor->SelectedSeqObjs.Num() == 1)
		{
			Append(IDM_KISMET_COMMENT_TO_FRONT, *LocalizeUnrealEd("CommentToFront"), TEXT(""));
			Append(IDM_KISMET_COMMENT_TO_BACK, *LocalizeUnrealEd("CommentToBack"), TEXT(""));
		}
	}
}

WxMBKismetObjectOptions::~WxMBKismetObjectOptions()
{

}

/*-----------------------------------------------------------------------------
	WxMBKismetConnectorOptions.
-----------------------------------------------------------------------------*/

WxMBKismetConnectorOptions::WxMBKismetConnectorOptions(WxKismet* SeqEditor)
{
	USequenceOp* ConnSeqOp = SeqEditor->ConnSeqOp;
	INT ConnIndex = SeqEditor->ConnIndex;
	INT ConnType = SeqEditor->ConnType;
	INT ConnCount = 0;

	// Add menu option for breaking links. Only input and variable links actually store pointers.
	if( ConnType == LOC_OUTPUT )
	{
		Append( IDM_KISMET_BREAK_LINK_ALL, *LocalizeUnrealEd("BreakAllLinks"), TEXT("") );
		BreakLinkMenu = new wxMenu();
		Append(IDM_KISMET_BREAK_LINK_MENU,*LocalizeUnrealEd("BreakLinkTo"),BreakLinkMenu);
		// add individual links
		for (INT idx = 0; idx < ConnSeqOp->OutputLinks(ConnIndex).Links.Num(); idx++)
		{
			FSeqOpOutputInputLink &link = ConnSeqOp->OutputLinks(ConnIndex).Links(idx);
			if (link.LinkedOp != NULL)
			{
				// add a submenu for this link
				FString ObjName = link.LinkedOp->ObjName;
				FString OpName  =  link.LinkedOp->GetName();
				// If object has a comment use that instead
				if( link.LinkedOp->ObjComment.Len() )
				{
					ObjName = *link.LinkedOp->ObjComment;
				}
				// If input link has a description, use it
				if( link.LinkedOp->InputLinks(link.InputLinkIdx).LinkDesc.Len() )
				{
					OpName = *link.LinkedOp->InputLinks(link.InputLinkIdx).LinkDesc;
				}

				BreakLinkMenu->Append(IDM_KISMET_BREAK_LINK_START+idx,*FString::Printf(TEXT("%s (%s)"), *ObjName, *OpName),TEXT(""));

				ConnCount++;
			}
		}

		// If the operation is a switch class, give options for removing a switch node
		USeqCond_SwitchBase* Switch = Cast<USeqCond_SwitchBase>(ConnSeqOp);
		if( Switch )
		{
			Append( IDM_KISMET_SWITCH_REMOVE, *LocalizeUnrealEd("RemoveSwitchConnector"), TEXT("") );
		}

		// Connection copy/paste
		AppendSeparator();
		Append( IDM_KISMET_COPY_CONNECTIONS, *LocalizeUnrealEd("CopyConnections"), TEXT("") );
		if( ConnCount )
		{
			Append( IDM_KISMET_CUT_CONNECTIONS,  *LocalizeUnrealEd("CutConnections"),  TEXT("") );
		}
		if( SeqEditor->CopyConnInfo.Num() )
		{
			Append( IDM_KISMET_PASTE_CONNECTIONS, *LocalizeUnrealEd("PasteConnections"), TEXT("") );
		}
		AppendSeparator();

		FString Toggle = *LocalizeUnrealEd("ToggleLink");
		if( ConnSeqOp->OutputLinks(ConnIndex).bDisabled )
		{
			Toggle += *FString::Printf( TEXT("  (%s)"), *LocalizeUnrealEd("Enable") );
		}
		else
		{
			Toggle += *FString::Printf( TEXT("  (%s)"), *LocalizeUnrealEd("Disable") );
		}
		Append( IDM_KISMET_TOGGLE_DISABLE_LINK, *Toggle, TEXT("") );

		Toggle = *LocalizeUnrealEd("ToggleLinkPIE");
		if( ConnSeqOp->OutputLinks(ConnIndex).bDisabledPIE )
		{
			Toggle += *FString::Printf( TEXT("  (%s)"), *LocalizeUnrealEd("Enable") );
		}
		else
		{
			Toggle += *FString::Printf( TEXT("  (%s)"), *LocalizeUnrealEd("Disable") );
		}
		Append( IDM_KISMET_TOGGLE_DISABLE_LINK_PIE, *Toggle, TEXT("") );

		AppendSeparator();
		Append( IDM_KISMET_REMOVE_OUTPUT, *LocalizeUnrealEd("HideOutput"), TEXT("") );
		Append( IDM_KISMET_SET_OUTPUT_DELAY, *LocalizeUnrealEd("SetActivateDelay"), TEXT("") );
	}
	else if (ConnType == LOC_INPUT)
	{
		// figure out if there is a link to this connection
		TArray<USequenceOp*> outputOps;
		TArray<INT> outputIndices;
		UBOOL bAddedMenu = FALSE;
		if (WxKismet::FindOutputLinkTo(SeqEditor->Sequence,ConnSeqOp,ConnIndex,outputOps,outputIndices))
		{
			// add the break all option if there are multiple links
			Append(IDM_KISMET_BREAK_LINK_ALL,*LocalizeUnrealEd("BreakAllLinks"),TEXT(""));
			// add the submenu for breaking individual links
			BreakLinkMenu = new wxMenu();
			Append(IDM_KISMET_BREAK_LINK_MENU,*LocalizeUnrealEd("BreakLinkTo"),BreakLinkMenu);
			// and add the individual link options
			for (INT idx = 0; idx < outputOps.Num(); idx++)
			{
				// add a submenu for this link
				FString ObjName = *outputOps(idx)->ObjName;
				FString OpName  =  outputOps(idx)->GetName();
				// If object has a comment use that instead
				if( outputOps(idx)->ObjComment.Len() )
				{
					ObjName = *outputOps(idx)->ObjComment;
				}
				// If object link has a description, use it
				if( outputOps(idx)->OutputLinks(outputIndices(idx)).LinkDesc.Len() )
				{
					OpName = *outputOps(idx)->OutputLinks(outputIndices(idx)).LinkDesc;
				}

				BreakLinkMenu->Append(IDM_KISMET_BREAK_LINK_START+idx,*FString::Printf(TEXT("%s (%s)"), *ObjName, *OpName),TEXT(""));

				ConnCount++;
			}

			AppendSeparator();
			FString Toggle = *LocalizeUnrealEd("ToggleLink");
			if( ConnSeqOp->InputLinks(ConnIndex).bDisabled )
			{
				Toggle += *FString::Printf( TEXT("  (%s)"), *LocalizeUnrealEd("Enable") );
			}
			else
			{
				Toggle += *FString::Printf( TEXT("  (%s)"), *LocalizeUnrealEd("Disable") );
			}
			// add the toggle disable flag option
			Append( IDM_KISMET_TOGGLE_DISABLE_LINK, *Toggle, TEXT("") );

			Toggle = *LocalizeUnrealEd("ToggleLinkPIE");
			if( ConnSeqOp->InputLinks(ConnIndex).bDisabledPIE )
			{
				Toggle += *FString::Printf( TEXT("  (%s)"), *LocalizeUnrealEd("Enable") );
			}
			else
			{
				Toggle += *FString::Printf( TEXT("  (%s)"), *LocalizeUnrealEd("Disable") );
			}
			// add the toggle disable flag option
			Append( IDM_KISMET_TOGGLE_DISABLE_LINK_PIE, *Toggle, TEXT("") );

			Append( IDM_KISMET_SET_INPUT_DELAY, *LocalizeUnrealEd("SetActivateDelay"), TEXT("") );

			bAddedMenu = TRUE;
		}
		
		// Connection copy/paste
		if( bAddedMenu )
		{
			AppendSeparator();
		}
		Append( IDM_KISMET_COPY_CONNECTIONS, *LocalizeUnrealEd("CopyConnections"), TEXT("") );
		if( ConnCount )
		{
			Append( IDM_KISMET_CUT_CONNECTIONS,  *LocalizeUnrealEd("CutConnections"),  TEXT("") );
		}
		if( SeqEditor->CopyConnInfo.Num() )
		{
			Append( IDM_KISMET_PASTE_CONNECTIONS, *LocalizeUnrealEd("PasteConnections"), TEXT("") );
		}
	}
	else if( ConnType == LOC_VARIABLE )
	{
		if( ConnSeqOp->VariableLinks(ConnIndex).LinkedVariables.Num() > 0 )
		{
			Append( IDM_KISMET_BREAK_LINK_ALL, *LocalizeUnrealEd("BreakAllLinks"), TEXT("") );
			BreakLinkMenu = new wxMenu();
			Append(IDM_KISMET_BREAK_LINK_MENU,*LocalizeUnrealEd("BreakLinkTo"),BreakLinkMenu);
			// add the individual variable links
			for (INT idx = 0; idx < ConnSeqOp->VariableLinks(ConnIndex).LinkedVariables.Num(); idx++)
			{
				USequenceVariable *var = ConnSeqOp->VariableLinks(ConnIndex).LinkedVariables(idx);

				// add a submenu for this link
				FString ObjName =  var->GetName();
				FString OpName  = var->GetValueStr();
				// If variable has a name, use it instead of string value
				if( var->VarName != NAME_None )
				{
					ObjName = var->VarName.ToString();
				}
				// If object has a comment use that instead
				else if( var->ObjComment.Len() )
				{
					ObjName = *var->ObjComment;
				}
				BreakLinkMenu->Append(IDM_KISMET_BREAK_LINK_START+idx,*FString::Printf(TEXT("%s (%s)"), *ObjName, *OpName),TEXT(""));

				ConnCount++;
			}
		}
		FSeqVarLink &varLink = ConnSeqOp->VariableLinks(ConnIndex);
		// Do nothing if ExpectedType is NULL!
		if(varLink.ExpectedType != NULL)
		{
			Append(IDM_KISMET_CREATE_LINKED_VARIABLE, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("CreateNewVar_F"),*((USequenceVariable*)varLink.ExpectedType->GetDefaultObject())->ObjName)), TEXT(""));

			// If this expects an object variable - offer option of creating a SeqVar_Object for each selected Actor and connecting it.
			if( varLink.SupportsVariableType(USeqVar_Object::StaticClass()) )
			{
				SeqEditor->BuildSelectedActorLists();

				if(SeqEditor->NewObjActors.Num() > 0)
				{
					FString NewObjVarString;
					FString FirstObjName = SeqEditor->NewObjActors(0)->GetName();
					if( SeqEditor->NewObjActors.Num() > 1 )
					{
						NewObjVarString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("NewObjectVarsUsing_F"), *FirstObjName) );
					}
					else
					{
						NewObjVarString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("NewObjectVarUsing_F"), *FirstObjName) );
					}

					Append( IDM_KISMET_NEW_VARIABLE_OBJ_CONTEXT, *NewObjVarString, TEXT("") );
					SeqEditor->bAttachVarsToConnector = TRUE;
				}
			}

			Append(IDM_KISMET_REMOVE_VARIABLE,TEXT("Remove variable connector"),TEXT(""));
		}

		AppendSeparator();
		Append( IDM_KISMET_COPY_CONNECTIONS, *LocalizeUnrealEd("CopyConnections"), TEXT("") );
		if( ConnCount )
		{
			Append( IDM_KISMET_CUT_CONNECTIONS,  *LocalizeUnrealEd("CutConnections"),  TEXT("") );
		}
		if( SeqEditor->CopyConnInfo.Num() &&
			SeqEditor->CopyConnType == LOC_VARIABLE )
		{
			Append( IDM_KISMET_PASTE_CONNECTIONS, *LocalizeUnrealEd("PasteConnections"), TEXT("") );
		}
	}
	else if (ConnType == LOC_EVENT)
	{
		if( ConnSeqOp->EventLinks(ConnIndex).LinkedEvents.Num() > 0 )
		{
			Append(IDM_KISMET_BREAK_LINK_ALL, *LocalizeUnrealEd("BreakEventLinks"),TEXT(""));
			ConnCount = ConnSeqOp->EventLinks(ConnIndex).LinkedEvents.Num();
		}

		AppendSeparator();
		Append( IDM_KISMET_COPY_CONNECTIONS, *LocalizeUnrealEd("CopyConnections"), TEXT("") );
		if( ConnCount )
		{
			Append( IDM_KISMET_CUT_CONNECTIONS,  *LocalizeUnrealEd("CutConnections"),  TEXT("") );
		}
		if( SeqEditor->CopyConnInfo.Num() && 
			SeqEditor->CopyConnType == LOC_EVENT )
		{
			Append( IDM_KISMET_PASTE_CONNECTIONS, *LocalizeUnrealEd("PasteConnections"), TEXT("") );
		}
	}
}

WxMBKismetConnectorOptions::~WxMBKismetConnectorOptions()
{

}

/*-----------------------------------------------------------------------------
	WxKismetSearch.
-----------------------------------------------------------------------------*/

static const INT KismetSearch_ControlBorder = 5;

BEGIN_EVENT_TABLE( WxKismetSearch, wxFrame )
	EVT_CLOSE( WxKismetSearch::OnClose )
END_EVENT_TABLE()

WxKismetSearch::WxKismetSearch(WxKismet* InKismet, wxWindowID InID)
	:	wxFrame( InKismet, InID, *LocalizeUnrealEd("KismetSearch"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR )
	,	Kismet( InKismet )
{
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));

	wxBoxSizer* TopSizerV = new wxBoxSizer( wxVERTICAL );
	SetSizer(TopSizerV);
	SetAutoLayout(true);

	// Result list box
    wxStaticText* ResultCaption = new wxStaticText( this, -1, *LocalizeUnrealEd("Results"), wxDefaultPosition, wxDefaultSize, 0 );
    TopSizerV->Add(ResultCaption, 0, wxALIGN_LEFT|wxALL, KismetSearch_ControlBorder);

	ResultList = new wxListBox( this, IDM_KISMET_SEARCHRESULT, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
	TopSizerV->Add( ResultList, 1, wxGROW | wxALL, KismetSearch_ControlBorder );

	wxFlexGridSizer* GridSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	GridSizer->AddGrowableCol(1);
	TopSizerV->Add( GridSizer, 0, wxGROW | wxALL, KismetSearch_ControlBorder );

	// Name/Comment search entry
	wxStaticText* NameCaption = new wxStaticText( this, -1, *LocalizeUnrealEd("Search") );
	GridSizer->Add(NameCaption, 0, wxRIGHT | wxALL, KismetSearch_ControlBorder);

	NameEntry = new wxTextCtrl(this, IDM_KISMET_SEARCHENTRY, TEXT(""), wxDefaultPosition, wxDefaultSize, 0 );
	GridSizer->Add(NameEntry, 1, wxGROW | wxALL, KismetSearch_ControlBorder);
	
	// Object search entry
	wxStaticText* SearchTypeCaption = new wxStaticText( this, -1, *LocalizeUnrealEd("SearchType") );
	GridSizer->Add(SearchTypeCaption, 0, wxRIGHT | wxALL, KismetSearch_ControlBorder);

	SearchTypeCombo = new WxComboBox(this, -1, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	GridSizer->Add(SearchTypeCombo, 1, wxGROW | wxALL, KismetSearch_ControlBorder);

	SearchTypeCombo->Append( *LocalizeUnrealEd("CommentsNames") );			// KST_NameComment
	SearchTypeCombo->Append( *LocalizeUnrealEd("ReferencedObjectName") );	// KST_ObjName
	SearchTypeCombo->Append( *LocalizeUnrealEd("NamedVariable") );			// KST_VarName
	SearchTypeCombo->Append( *LocalizeUnrealEd("NamedVariableUse") );		// KST_VarNameUsed
	SearchTypeCombo->Append( *LocalizeUnrealEd("RemoteEvents") );			// KST_RemoteEvents
	SearchTypeCombo->Append( *LocalizeUnrealEd("ReferencesRemoteEvent") );	// KST_ReferencesRemoteEvent

	wxStaticText* SearchScopeCaption = new wxStaticText( this, -1, *LocalizeUnrealEd("SearchScope") );
	GridSizer->Add(SearchScopeCaption, 2, wxRIGHT | wxALL, KismetSearch_ControlBorder);

	SearchScopeCombo = new WxComboBox(this, -1, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	GridSizer->Add(SearchScopeCombo, 3, wxGROW | wxALL, KismetSearch_ControlBorder);

	SearchScopeCombo->Append(*LocalizeUnrealEd("CurrentLevel"));
	SearchScopeCombo->Append(*LocalizeUnrealEd("CurrentSequence"));
	SearchScopeCombo->Append(*LocalizeUnrealEd("AllLevels"));

	// Search button
	SearchButton = new wxButton( this, IDM_KISMET_DOSEARCH, *LocalizeUnrealEd("Search"), wxDefaultPosition, wxDefaultSize, 0 );
	TopSizerV->Add(SearchButton, 0, wxCENTER | wxALL, KismetSearch_ControlBorder);

	FWindowUtil::LoadPosSize( TEXT("KismetSearch"), this, 100, 100, 500, 300 );
	LoadSearchSettings();

	NameEntry->SelectAll();
	NameEntry->SetFocus();
}

/**
 * Loads prior search settings from the editor user settings ini.
 */
void WxKismetSearch::LoadSearchSettings()
{
	EKismetSearchType SearchType	= KST_NameComment;
	EKismetSearchScope SearchScope	= KSS_CurrentLevel;
	FString SearchString;

	GConfig->GetInt( TEXT("KismetSearch"), TEXT("SearchType"), (INT&)SearchType, GEditorUserSettingsIni );
	GConfig->GetInt( TEXT("KismetSearch"), TEXT("SearchScope"), (INT&)SearchScope, GEditorUserSettingsIni );
	GConfig->GetString( TEXT("KismetSearch"), TEXT("SearchString"), SearchString, GEditorUserSettingsIni );

	Clamp( SearchType, (EKismetSearchType)0, MAX_KST_TYPES );
	Clamp( SearchScope, (EKismetSearchScope)0, MAX_KSS_TYPES );

	SetSearchType( SearchType );
	SetSearchScope( SearchScope );
	if ( SearchString.Len() > 0 )
	{
		SetSearchString( *SearchString );
	}
}

/**
 * Saves prior search settings to the editor user settings ini.
 */
void WxKismetSearch::SaveSearchSettings()
{
	const INT SearchType		= GetSearchType();
	const INT SearchScope		= GetSearchScope();
	const FString SearchString	= GetSearchString();

	GConfig->SetInt( TEXT("KismetSearch"), TEXT("SearchType"), SearchType, GEditorUserSettingsIni );
	GConfig->SetInt( TEXT("KismetSearch"), TEXT("SearchScope"), SearchScope, GEditorUserSettingsIni );
	GConfig->SetString( TEXT("KismetSearch"), TEXT("SearchString"), *SearchString, GEditorUserSettingsIni );
}

/**
 * Clears the results list, clearing references to the previous results.
 */
void WxKismetSearch::ClearResultsList()
{
	ResultList->Clear();
}

/**
 * Appends a results to the results list.
 *
 * @param	ResultString	The string to display in the results list.
 * @param	SequenceObj		The sequence object associated with the search result.
 */
void WxKismetSearch::AppendResult(const TCHAR* ResultString, USequenceObject* SequenceObject)
{
	ResultList->Append( ResultString, SequenceObject );
}

UBOOL WxKismetSearch::SetResultListSelection(INT Index)
{
	UBOOL bResult = FALSE;
	if ( Index >= 0 && Index <= static_cast<INT>(ResultList->GetCount()) )
	{
		ResultList->SetSelection( Index, TRUE );
		bResult = TRUE;
	}
	return bResult;
}

/**
 * @return		The selected search result sequence, or NULL if none selected.
 */
USequenceObject* WxKismetSearch::GetSelectedResult() const
{
	USequenceObject* SeqObj	= NULL;
	const INT SelIndex		= ResultList->GetSelection();
	if ( SelIndex != -1 )
	{
		SeqObj = (USequenceObject*)ResultList->GetClientData(SelIndex);
		check( SeqObj );
	}

	return SeqObj;
}

/**
 * @return		The number of results in the results list.
 */
INT WxKismetSearch::GetNumResults() const
{
	return ResultList->GetCount();
}

/**
 * @return		The current search string.
 */
FString WxKismetSearch::GetSearchString() const
{
	return (const TCHAR*)NameEntry->GetValue();
}

/**
 * Sets the search string field.
 *
 * @param	SearchString	The new search string.
 */
void WxKismetSearch::SetSearchString(const TCHAR* SearchString)
{
	NameEntry->SetValue( SearchString );
}

/**
* @return		The selected search type setting.
*/
INT WxKismetSearch::GetSearchType() const
{
	return SearchTypeCombo->GetSelection();
}

/**
 * Sets the search type.
 */
void WxKismetSearch::SetSearchType(EKismetSearchType SearchType)
{
	SearchTypeCombo->SetSelection( SearchType );
}

/**
 * Sets the search scope.
 */
void WxKismetSearch::SetSearchScope(EKismetSearchScope SearchScope)
{
	SearchScopeCombo->SetSelection( SearchScope );
}

/**
 * @return		The selected search scope setting.
 */
INT WxKismetSearch::GetSearchScope() const
{
	return SearchScopeCombo->GetSelection();
}

void WxKismetSearch::OnClose(wxCloseEvent& In)
{
	FWindowUtil::SavePosSize( TEXT("KismetSearch"), this );	
	SaveSearchSettings();

	check(Kismet->SearchWindow == this);
	Kismet->SearchWindow = NULL;

	Kismet->ToolBar->ToggleTool(IDM_KISMET_OPENSEARCH, false);

	this->Destroy();
}

/*-----------------------------------------------------------------------------
	WxKismetUpdate.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxKismetUpdate, wxFrame )
	EVT_CLOSE( WxKismetUpdate::OnClose )
	EVT_LISTBOX( IDM_KISMET_UPDATERESULT, WxKismetUpdate::OnUpdateListChanged )
	EVT_BUTTON( IDM_KISMET_DOUPDATE, WxKismetUpdate::OnContextUpdate )
	EVT_BUTTON( IDM_KISMET_DOUPDATEALL, WxKismetUpdate::OnContextUpdateAll )
END_EVENT_TABLE()

WxKismetUpdate::WxKismetUpdate(WxKismet* InKismet, wxWindowID InID)
	: wxFrame( InKismet, InID, *LocalizeUnrealEd("KismetUpdate"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR )
{
	Kismet = InKismet;

	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));

	wxBoxSizer* TopSizerV = new wxBoxSizer( wxVERTICAL );
	this->SetSizer(TopSizerV);
	this->SetAutoLayout(true);

	// Result list box
    wxStaticText* ResultCaption = new wxStaticText( this, -1, *LocalizeUnrealEd("UpdateList"), wxDefaultPosition, wxDefaultSize, 0 );
    TopSizerV->Add(ResultCaption, 0, wxALIGN_LEFT|wxALL, KismetSearch_ControlBorder);

	UpdateList = new wxListBox( this, IDM_KISMET_UPDATERESULT, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
	TopSizerV->Add( UpdateList, 1, wxGROW | wxALL, KismetSearch_ControlBorder );


	wxFlexGridSizer* GridSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	GridSizer->AddGrowableCol(1);
	TopSizerV->Add( GridSizer, 0, wxGROW | wxALL, KismetSearch_ControlBorder );

	wxBoxSizer *ButtonSizerV = new wxBoxSizer( wxHORIZONTAL );
	TopSizerV->Add( ButtonSizerV, 0, wxCENTER | wxGROW | wxALL, KismetSearch_ControlBorder );

	// Search button
	UpdateButton = new wxButton( this, IDM_KISMET_DOUPDATE, *LocalizeUnrealEd("Update"), wxDefaultPosition, wxDefaultSize, 0 );
	ButtonSizerV->Add(UpdateButton, 0, wxALIGN_LEFT | wxALL, KismetSearch_ControlBorder);

	// Search button
	UpdateAllButton = new wxButton( this, IDM_KISMET_DOUPDATEALL, *LocalizeUnrealEd("UpdateAll"), wxDefaultPosition, wxDefaultSize, 0 );
	ButtonSizerV->Add(UpdateAllButton, 0, wxALIGN_RIGHT | wxALL, KismetSearch_ControlBorder);

	FWindowUtil::LoadPosSize( TEXT("KismetUpdate"), this, 100, 100, 500, 300 );

	BuildUpdateList();
}

void WxKismetUpdate::OnContextUpdate(wxCommandEvent &In)
{
	if (Kismet != NULL && UpdateList != NULL)
	{
		INT SelIdx = UpdateList->GetSelection();
		if (SelIdx != -1)
		{
			// Get the selected SequenceObject pointer.
			USequenceObject* SeqObj = (USequenceObject*)UpdateList->GetClientData(SelIdx);

			GEditor->BeginTransaction(*FString::Printf(LocalizeSecure(LocalizeUnrealEd(TEXT("KismetUndo_Update_f")), *SeqObj->GetSeqObjFullName())));
			SeqObj->UpdateObject();
			GEditor->EndTransaction();
		}
	}
	BuildUpdateList();
}

void WxKismetUpdate::OnContextUpdateAll(wxCommandEvent &In)
{
	if (Kismet != NULL && UpdateList != NULL)
	{
		// this indicates whether the undo transaction should be cancelled or not
		UBOOL bUpdatedObjects = FALSE;

		// begin a transaction
		INT CancelIndex = GEditor->BeginTransaction(*LocalizeUnrealEd(TEXT("KismetUndo_UpdateMultiple")));

		UpdateList->Clear();
		TArray<USequence*> Seqs;
		Seqs.AddItem(Kismet->GetRootSequence());
		for (INT SeqIdx = 0; SeqIdx < Seqs.Num(); SeqIdx++)
		{
			for (INT Idx = 0; Idx < Seqs(SeqIdx)->SequenceObjects.Num(); Idx++)
			{
				USequenceObject *Obj = Seqs(SeqIdx)->SequenceObjects(Idx);
				if (Obj != NULL)
				{
					if (Obj->IsA(USequence::StaticClass()))
					{
						Seqs.AddItem((USequence*)Obj);
					}
					else if (Obj->eventGetObjClassVersion() != Obj->ObjInstanceVersion)
					{
						bUpdatedObjects = TRUE;
						Obj->UpdateObject();
					}
				}
			}
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
	BuildUpdateList();
}

void WxKismetUpdate::OnUpdateListChanged(wxCommandEvent &In)
{
	if (Kismet != NULL)
	{
		INT SelIdx = UpdateList->GetSelection();
		if (SelIdx != -1)
		{
			// Get the selected SequenceObject pointer.
			USequenceObject* SeqObj = (USequenceObject*)UpdateList->GetClientData(SelIdx);
			Kismet->CenterViewOnSeqObj(SeqObj);
		}
	}
}

void WxKismetUpdate::BuildUpdateList()
{
	if (Kismet != NULL && UpdateList != NULL)
	{
		UpdateList->Clear();
		TArray<USequence*> Seqs;

		// This should never be called while GWorld is a PIE world.
		check( !GIsPlayInEditorWorld );
		for ( INT LevelIndex = 0 ; LevelIndex < GWorld->Levels.Num() ; ++LevelIndex )
		{
			ULevel* Level = GWorld->Levels(LevelIndex);
			if (Level->GameSequences.Num() > 0)
			{
				USequence* RootSeq = Level->GameSequences(0);
				check(RootSeq);
				Seqs.AddItem( RootSeq );
			}
		}

		for (INT SeqIdx = 0; SeqIdx < Seqs.Num(); SeqIdx++)
		{
			for (INT Idx = 0; Idx < Seqs(SeqIdx)->SequenceObjects.Num(); Idx++)
			{
				USequenceObject *Obj = Seqs(SeqIdx)->SequenceObjects(Idx);
				if (Obj != NULL)
				{
					if (Obj->IsA(USequence::StaticClass()))
					{
						Seqs.AddItem((USequence*)Obj);
					}
					else
					if (Obj->eventGetObjClassVersion() != Obj->ObjInstanceVersion)
					{
						UpdateList->Append(*Obj->GetSeqObjFullName(),Obj);
					}
				}
			}
		}
	}
}

WxKismetUpdate::~WxKismetUpdate()
{
}

void WxKismetUpdate::OnClose(wxCloseEvent& In)
{
	FWindowUtil::SavePosSize( TEXT("KismetUpdate"), this );	

	this->Destroy();
}

/* ==========================================================================================================
	WxSequenceTreeCtrl
========================================================================================================== */
IMPLEMENT_DYNAMIC_CLASS(WxSequenceTreeCtrl,WxTreeCtrl)

FArchive& operator<<(FArchive &Ar, wxTreeItemId &Id)
{
	return Ar;
}

/** FSerializableObject interface */
void WxSequenceTreeCtrl::Serialize( FArchive& Ar )
{
	Ar << TreeMap;
}

/** Constructor */
WxSequenceTreeCtrl::WxSequenceTreeCtrl()
: KismetEditor(NULL)
{
}

/** Destructor */
WxSequenceTreeCtrl::~WxSequenceTreeCtrl()
{
}

/**
 * Initialize this control.  Must be the first function called after creation.
 *
 * @param	InParent	the window that opened this dialog
 * @param	InID		the ID to use for this dialog
 * @param	InEditor	pointer to the editor window that contains this control
 * @param   InStyle		Style of this tree control.
 */
void WxSequenceTreeCtrl::Create( wxWindow* InParent, wxWindowID InID, WxKismet* InEditor, LONG InStyle)
{
	KismetEditor = InEditor;
	const UBOOL bSuccess = WxTreeCtrl::Create(InParent,InID,NULL, InStyle);
	check( bSuccess );
}

/** Utility for returning total number of sequence objects within a sequence (excluding objects in subsequences) */
static INT GetNumSequenceObjectsWithin(USequence* Seq)
{
	TArray<USequenceObject*> Objs;
	Seq->FindSeqObjectsByClass(USequenceObject::StaticClass(), Objs, FALSE);
	return Objs.Num();
}

/**
 * Adds the root sequence of the specified level to the sequence tree, if a sequence exists for the specified level.
 *
 * @param	Level		The level whose root sequence should be added.  Can be NULL.
 * @param	RootId		The ID of the tree's root node.
 */
void WxSequenceTreeCtrl::AddLevelToTree(ULevel* Level, wxTreeItemId RootId)
{
	// Only add the level if it contains any sequences.
	if ( Level && Level->GameSequences.Num() > 0 )
	{
		USequence* RootSeq = Level->GameSequences(0);
		check(RootSeq); // Should not be able to open Kismet without the level having a Sequence.

		const FString SeqTitle = FString::Printf(TEXT("%s - %s [%d]"),*RootSeq->GetOutermost()->GetName(),*RootSeq->ObjName, GetNumSequenceObjectsWithin(RootSeq));
		const wxTreeItemId RootSeqId = AppendItem( RootId, *SeqTitle, 0, 0, new WxTreeObjectWrapper(RootSeq) );
		TreeMap.Set( RootSeq, RootSeqId );

		AddChildren(RootSeq, RootSeqId);
	}
}

/**
 * Repopulates the tree control with the sequences of all levels in the world.
 */
void WxSequenceTreeCtrl::RefreshTree()
{
	DeleteAllItems();
	TreeMap.Empty();

	//@fixme localization
	wxTreeItemId RootId = AddRoot( TEXT("Sequences"), 0, 0, NULL );

	// This should never be called while GWorld is a PIE world.
	check( !GIsPlayInEditorWorld );

	// Add the persistent level
	AddLevelToTree( GWorld->PersistentLevel, RootId );

	// Add any streaming levels.
	AWorldInfo*	WorldInfo = GWorld->GetWorldInfo();
	for( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
	{
		ULevelStreaming* CurStreamingLevel = WorldInfo->StreamingLevels( LevelIndex );
		if( CurStreamingLevel )
		{
			AddLevelToTree( CurStreamingLevel->LoadedLevel, RootId );
		}
	}

	Expand(RootId);
}

/**
 * Deletes all tree items from the list and clears the Sequence-ItemId map.
 */
void WxSequenceTreeCtrl::ClearTree()
{
	TreeMap.Empty();
	DeleteAllItems();
}

/**
 * Recursively adds all subsequences of the specified sequence to the tree control.
 */
void WxSequenceTreeCtrl::AddChildren( USequence* RootSeq, wxTreeItemId ParentId )
{
	check(RootSeq);

	// Find all sequences. The function will always return parents before children in the array.
	TArray<USequenceObject*> SeqObjs;
	RootSeq->FindSeqObjectsByClass( USequence::StaticClass(), SeqObjs, FALSE );

	// Iterate over sequences, adding them to the tree.
	for(INT i=0; i<SeqObjs.Num(); i++)
	{
		//debugf( TEXT("Item: %d %s"), i, SeqObjs(i)->GetName() );
		USequence* Seq = CastChecked<USequence>( SeqObjs(i) );

		// Only the root sequence should have an Outer that is not another Sequence (in the root case its the level).
		USequence* ParentSeq = CastChecked<USequence>( Seq->GetOuter() );
		wxTreeItemId* SeqParentId = TreeMap.Find(ParentSeq);

		// make sure that this sequence's Outer has the same ID that was passed in.
		check( *SeqParentId == ParentId );

		const FString SeqTitle = FString::Printf(TEXT("%s - [%d]"),*Seq->GetName(), GetNumSequenceObjectsWithin(Seq));
		wxTreeItemId NodeId = AppendItem( ParentId, *SeqTitle, 0, 0, new WxTreeObjectWrapper(Seq) );
		TreeMap.Set( Seq, NodeId );

		// add the child sequences for this sequence (this is new, so this may not be desired behavior)
		AddChildren( Seq, NodeId );
		SortChildren( ParentId );
	}
}

/**
 * De/selects the tree item corresponding to the specified object
 *
 * @param	SeqObj		the sequence object to select in the tree control
 * @param	bSelect		whether to select or deselect the sequence
 *
 * @return	True if the specified sequence object was found
 */
UBOOL WxSequenceTreeCtrl::SelectObject( USequenceObject* SeqObj, UBOOL bSelect/*=TRUE*/ )
{
	wxTreeItemId* TreeItemId = TreeMap.Find( SeqObj );
	if ( TreeItemId != NULL )
	{
		SelectItem( *TreeItemId, bSelect == TRUE );
		if ( bSelect )
		{
			EnsureVisible( *TreeItemId );
		}

		return TRUE;
	}

	return FALSE;
}
