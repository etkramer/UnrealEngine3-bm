/*=============================================================================
	UnUIContainers.cpp: Implementations for complex UI widget classes
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


/* ==========================================================================================================
	Includes
========================================================================================================== */
#include "EnginePrivate.h"
#include "CanvasScene.h"

#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"

#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "ScopedObjectStateChange.h"

#include "UnUIKeys.h"
#define TEMP_SPLITSCREEN_INDEX UCONST_TEMP_SPLITSCREEN_INDEX


/* ==========================================================================================================
	Definitions
========================================================================================================== */
IMPLEMENT_CLASS(UUIContainer);
	IMPLEMENT_CLASS(UUICalloutButtonPanel);
	IMPLEMENT_CLASS(UUIPanel);
	IMPLEMENT_CLASS(UUIFrameBox);
	IMPLEMENT_CLASS(UUISafeRegionPanel);
	IMPLEMENT_CLASS(UUIScrollFrame);
	IMPLEMENT_CLASS(UUITabPage);

IMPLEMENT_CLASS(UUITabControl);


/* ==========================================================================================================
	Implementations
========================================================================================================== */
/* ==========================================================================================================
	UUICalloutButtonPanel
========================================================================================================== */
/**
 * Retrieves the list of callout button input aliases which are available for use in the specified button panel.
 *
 * @param	AvailableAliases	receives the list of callout button aliases which aren't in use by this button panel.
 * @param	PlayerOwner			specifies the player that should be used for looking up the ButtonCallouts datastore; relevant
 *								when one player is using a gamepad and another is using a keyboard, for example.
 */
void UUICalloutButtonPanel::GetAvailableCalloutButtonAliases( TArray<FName>& AvailableAliases, ULocalPlayer* PlayerOwner/*=NULL*/ )
{
	UUICalloutButton* ButtonArc = ButtonTemplate;
	if ( ButtonArc == NULL )
	{
		ButtonArc = UUICalloutButton::StaticClass()->GetDefaultObject<UUICalloutButton>();
	}

	AvailableAliases.Empty();
	UUIDataStore_InputAlias* ButtonCalloutDS = ButtonArc->GetCalloutDataStore(PlayerOwner);
	if ( ButtonCalloutDS != NULL )
	{
		TArray<FUIDataProviderField> AliasFields;
		ButtonCalloutDS->GetSupportedDataFields(AliasFields);
		for ( INT FieldIdx = 0; FieldIdx < AliasFields.Num(); FieldIdx++ )
		{
			FUIDataProviderField& Alias = AliasFields(FieldIdx);
			if ( !eventContainsButton(Alias.FieldTag) )
			{
				AvailableAliases.AddUniqueItem(Alias.FieldTag);
			}
		}
	}
}

/**
 * Returns a reference to the input proxy for this button's scene.
 *
 * @param	bCreateIfNecessary	specify TRUE to have an input proxy created for you if it doesn't exist.  Future calls
 *								to this function would then return that proxy.
 *
 * @return	a reference to the button bar input proxy used by this button's owner scene.
 */
UUIEvent_CalloutButtonInputProxy* UUICalloutButtonPanel::GetCalloutInputProxy( UBOOL bCreateIfNecessary/*=FALSE*/ )
{
	UUIEvent_CalloutButtonInputProxy* Result = NULL;

	UUIScene* Scene = GetScene();
	if ( Scene != NULL )
	{
		// this will return either the owning scene or (in the case of a UIPrefab) the UIPrefab itself
		UUIScreenObject* InputProxyOwner = Scene->GetEditorDefaultParentWidget();
		if ( InputProxyOwner != NULL )
		{
			// find the scene's enabled state sequence - that's where the proxy object will go; normally we'd put this
			// into the focused state, but in this case, we want the support "pass-through" input handling, for split-screen
			UUISequence* ProxyOwnerSequence = NULL;
			for ( INT StateIdx = 0; StateIdx < InputProxyOwner->StateStack.Num(); StateIdx++ )
			{
				UUIState_Enabled* SceneEnabledState = Cast<UUIState_Enabled>(InputProxyOwner->StateStack(StateIdx));
				if ( SceneEnabledState != NULL )
				{
					// if this scene didn't have any default events to instance and didn't have any sequence objects in the sequence
					// then the state won't have a sequence at all - create one now so we are able to register the input events correctly
					if ( GIsGame && SceneEnabledState->StateSequence == NULL && bCreateIfNecessary )
					{
						SceneEnabledState->Created();
						check(SceneEnabledState->StateSequence);

						SceneEnabledState->StateSequence->InitializeSequence();
					}

					ProxyOwnerSequence = SceneEnabledState->StateSequence;
					break;
				}
			}

			if ( ProxyOwnerSequence != NULL )
			{
				UUIEvent_CalloutButtonInputProxy* InputProxy = NULL;

				// found the right state - now search for the input proxy
				for ( INT SeqIdx = 0; SeqIdx < ProxyOwnerSequence->SequenceObjects.Num(); SeqIdx++ )
				{
					UUIEvent_CalloutButtonInputProxy* NextInputProxy = Cast<UUIEvent_CalloutButtonInputProxy>(ProxyOwnerSequence->SequenceObjects(SeqIdx));
					if ( NextInputProxy != NULL && NextInputProxy->ButtonPanel == this )
					{
						InputProxy = NextInputProxy;
						break;
					}
				}
				
				// not found - create one if that was requested
				if ( InputProxy == NULL && bCreateIfNecessary )
				{
					UUIEvent_CalloutButtonInputProxy* ProxyArchetype = GetArchetype<UUICalloutButtonPanel>()->GetCalloutInputProxy(FALSE);
					EObjectFlags ProxyFlags = RF_Transactional|GetMaskedFlags(RF_Public|RF_Transient|RF_ArchetypeObject);

					InputProxy = ConstructObject<UUIEvent_CalloutButtonInputProxy>(
						UUIEvent_CalloutButtonInputProxy::StaticClass(),
						ProxyOwnerSequence, NAME_None, ProxyFlags, ProxyArchetype
						);

					InputProxy->Modify();
					InputProxy->ButtonPanel = this;
					InputProxy->OnCreated();

					ProxyOwnerSequence->AddSequenceObject(InputProxy);
				}

				Result = InputProxy;
			}
		}
	}

	return Result;
}

/**
 * Create a new callout button and give it the specified input alias tag.
 *
 * @param	ButtonInputAlias	the input alias to assign to the button (i.e. Accept, Cancel, Conditional1, etc.); this
 *								tag will be used to generate the button's data store markup.
 *
 * @return	an instance of a new UICalloutButton which has ButtonInputAlias as its InputAliasTag.
 */
UUICalloutButton* UUICalloutButtonPanel::CreateCalloutButton( FName ButtonInputAlias, FName ButtonName/*=NAME_None*/, UBOOL bInsertChild/*=TRUE*/ )
{
	UUICalloutButton* Result = NULL;
	if ( ButtonInputAlias != NAME_None )
	{
		UUICalloutButton* ButtonArch = ButtonTemplate;
		if ( ButtonArch == NULL )
		{
			ButtonArch = UUICalloutButton::StaticClass()->GetDefaultObject<UUICalloutButton>();
		}

		// if in the editor, create the widget in the transient package with a transient name so that undoing then adding another widget with the same name can work

		Result = Cast<UUICalloutButton>(CreateWidget(this, ButtonArch->GetClass(), ButtonArch, ButtonName));
		if ( Result != NULL )
		{
			Result->eventSetInputAlias(ButtonInputAlias);
			if ( bInsertChild )
			{
				Modify();
				if ( eventInsertButton(Result) != INDEX_NONE )
				{
					// the button couldn't apply its datastore binding from within the call to SetInputAlias (because it wasn't yet initialized)
					// so refresh the button now
					Result->RefreshSubscriberValue();
				}
				else
				{
					// indicate failure by returning NULL
					Result = NULL;
				}
			}
		}
	}
	else
	{
		debugf(NAME_Warning, TEXT("%s::CreateCalloutButton: You must specify a valid ButtonInputAlias to create a new callout button."), *GetPathName());
	}

	return Result;
}

/**
 * Finds the most appropriate position to insert the specified button, based on its InputAliasTag.  Only relevant if this
 * button's InputAliasTag is contained in the CalloutButtonAliases array.
 *
 * @param	ButtonToInsert	the button that will be inserted into the panel
 * @param	bSearchChildren	specify TRUE to search for the best insertion index into the Children array, rather than the
 *			CalloutButtonAliases array.
 *
 * @return	index [into the Children/CalloutButton array] for the position to insert the button, or INDEX_NONE if the button's
 *			InputAliasTag is not contained in the CalloutButtonAliases array (meaning that it's a dynamic button)
 */
INT UUICalloutButtonPanel::FindBestInsertionIndex( UUICalloutButton* ButtonToInsert, UBOOL bSearchChildrenArray/*=FALSE*/ )
{
	INT Result = INDEX_NONE;

	TArray<UUIObject*>& TargetArray = bSearchChildrenArray ? Children : (TArray<UUIObject*>&)CalloutButtons;
	if ( ButtonToInsert != NULL && ButtonToInsert->InputAliasTag != NAME_None && !TargetArray.ContainsItem(ButtonToInsert) )
	{
		INT AliasArrayIndex = CalloutButtonAliases.FindItemIndex(ButtonToInsert->InputAliasTag);
		if ( AliasArrayIndex != INDEX_NONE )
		{
			if ( AliasArrayIndex + 1 < CalloutButtonAliases.Num() )
			{
				// the button that should be after ButtonToInsert is NOT the last button in the array
				// find the position of that button and insert the button there.
				FName SearchTag = CalloutButtonAliases(AliasArrayIndex + 1);
				for ( INT Idx = 0; Idx < TargetArray.Num(); Idx++ )
				{
					UUICalloutButton* ButtonElem = Cast<UUICalloutButton>(TargetArray(Idx));
					if ( ButtonElem != NULL && ButtonElem->InputAliasTag == SearchTag )
					{
						Result = Idx;
						break;
					}
				}
			}
			else
			{
				// find the last location of the last UICalloutButton and insert the new button just after that.
				for ( Result = TargetArray.Num(); Result > 0; Result-- )
				{
					UUIObject* Obj = TargetArray(Result - 1);
					if ( Obj->IsA(UUICalloutButton::StaticClass()) )
					{
						break;
					}
				}
			}
		}
	}

	return Result;
}

/**
 * Request that the docking relationships between the panel's buttons be updated at the beginning of the next scene update.
 * Calling this method will trigger a scene update as well.
 *
 * @param	bImmediately	specify TRUE to have the docking updated immediately rather than waiting until the next scene
 *							update.  A scene update will then only be triggered if any docking relationships changed.
 */
void UUICalloutButtonPanel::RequestButtonDockingUpdate( UBOOL bImmediately/*=FALSE*/ )
{
	if ( bImmediately )
	{
		SetupDockingRelationships();
	}
	else
	{
		bRefreshButtonDocking = bEnableSceneUpdateNotifications = TRUE;
		RequestSceneUpdate(TRUE, TRUE);
	}
}

/**
 * Set up the docking links between the callout buttons.
 */
void UUICalloutButtonPanel::SetupDockingRelationships()
{
	if ( !bGeneratingInitialButtons && ButtonLayout < CBLT_MAX )
	{
		INT StartIndex = CalloutButtons.Num() - 1, LastIndex = -1, Delta = -1;
		EUIWidgetFace TargetFace = static_cast<EUIWidgetFace>(UIFACE_Left + ButtonBarOrientation);
		EUIWidgetFace SourceFace = static_cast<EUIWidgetFace>(UIFACE_Left + ButtonBarOrientation);

		EUIWidgetFace AdjacentFaces[2] =
		{
			ButtonBarOrientation == UIORIENT_Horizontal ? UIFACE_Top : UIFACE_Left,
			ButtonBarOrientation == UIORIENT_Horizontal ? UIFACE_Bottom : UIFACE_Right
		};

		// the first button will dock to the panel itself
		UUIObject* CurrentDockTarget = this;
		if ( ButtonLayout == CBLT_DockRight )
		{
			TargetFace = SourceFace = static_cast<EUIWidgetFace>(UIFACE_Right + ButtonBarOrientation);
			StartIndex = 0;
			LastIndex = CalloutButtons.Num();
			Delta = 1;
		}
		else if ( ButtonLayout == CBLT_Centered || ButtonLayout == CBLT_Justified )
		{
			// the first button will not be docked to the panel at all
			TargetFace = UIFACE_MAX;
			CurrentDockTarget = NULL;
		}

		for ( INT ButtonIndex = StartIndex; ButtonIndex != LastIndex; ButtonIndex += Delta )
		{
			UUICalloutButton* Button = CalloutButtons(ButtonIndex);
			checkSlow(Button);

			if ( Button->IsVisible() )
			{
				if ( ButtonLayout != CBLT_None )
				{
					// (the following comments assume the ButtonOrientation is UIORIENT_Horizontal and the ButtonLayout is CBLT_DockLeft)
					// dock the top and bottom faces of the button to this panel
					Button->SetDockTarget(AdjacentFaces[0], this, AdjacentFaces[0]);
					Button->SetDockTarget(AdjacentFaces[1], this, AdjacentFaces[1]);
				}

				Button->InvalidateAllPositions(FALSE);
				Button->RefreshPosition();

				if ( Button->StringRenderComponent != NULL )
				{
					const FLOAT HorzPadding = ButtonPadding[UIORIENT_Horizontal].GetValue(this, UIEXTENTEVAL_Pixels);
					const FLOAT VertPadding = ButtonPadding[UIORIENT_Vertical].GetValue(this, UIEXTENTEVAL_Pixels);

					Button->StringRenderComponent->Modify();
					Button->StringRenderComponent->eventSetAutoSizePadding(UIORIENT_Horizontal, HorzPadding, HorzPadding, UIEXTENTEVAL_Pixels, UIEXTENTEVAL_Pixels);
					Button->StringRenderComponent->eventSetAutoSizePadding(UIORIENT_Vertical, VertPadding, VertPadding, UIEXTENTEVAL_Pixels, UIEXTENTEVAL_Pixels);
				}

				// dock the left face of the button to the right face of the previous button (or the left face of this panel
				// if this is the first button).
				Button->SetDockTarget( SourceFace, CurrentDockTarget, TargetFace );

				// clear any docking for the opposite face
				EUIWidgetFace OppositeSourceFace = GetOppositeFace(SourceFace);
				Button->SetDockTarget( OppositeSourceFace, NULL, UIFACE_MAX);

				if ( ButtonLayout != CBLT_Justified )
				{
					// the next button in the list will dock to this button
					CurrentDockTarget = Button;
					// the next button in the list will dock to the face opposite the source (left) face - the right face
					TargetFace = OppositeSourceFace;
				}
			}
		}
	}

	bRefreshButtonDocking = FALSE;
}

/* === UUIObject interface === */
/**
 * Called immediately before the scene perform an update.  Recalculates the docking relationships between this panel's
 * buttons.
 */
void UUICalloutButtonPanel::PreSceneUpdate()
{
	Super::PreSceneUpdate();

	if ( bRefreshButtonDocking )
	{
		SetupDockingRelationships();
	}

	if ( ButtonLayout != CBLT_Centered && ButtonLayout != CBLT_Justified )
	{
		// no longer need these updates
		bEnableSceneUpdateNotifications = FALSE;
	}
}

/**
 * Called immediately after the scene perform an update.  Positions buttons when the layout mode is centered.
 */
void UUICalloutButtonPanel::PostSceneUpdate()
{
	FVector2D ViewportOrigin(EC_EventParm);
	GetViewportOrigin(ViewportOrigin);
	const UBOOL bIncludeViewportOrigin = FALSE;
	const FLOAT OffsetValue = bIncludeViewportOrigin ? 0.f : ViewportOrigin.X;

	Super::PostSceneUpdate();

	const FLOAT PanelSize = GetBounds(ButtonBarOrientation, EVALPOS_PixelViewport);
	FLOAT TotalButtonSize = 0.f;

	TArray<UUICalloutButton*> VisibleButtons;
	for ( INT ButtonIdx = 0; ButtonIdx < CalloutButtons.Num(); ButtonIdx++ )
	{
		if ( CalloutButtons(ButtonIdx) != NULL && CalloutButtons(ButtonIdx)->IsVisible() )
		{
			VisibleButtons.AddItem(CalloutButtons(ButtonIdx));
		}
	}

	TArray<FLOAT> ButtonSizes;
	ButtonSizes.Empty(VisibleButtons.Num());
	ButtonSizes.AddZeroed(VisibleButtons.Num());

	const EUIWidgetFace NearFace = static_cast<EUIWidgetFace>(UIFACE_Left + ButtonBarOrientation);
	const EUIWidgetFace FarFace = static_cast<EUIWidgetFace>(NearFace + 2);
	
	const FLOAT HorzPadding = ButtonPadding[UIORIENT_Horizontal].GetValue(this, UIEXTENTEVAL_Pixels);
	const FLOAT VertPadding = ButtonPadding[UIORIENT_Vertical].GetValue(this, UIEXTENTEVAL_Pixels);
	for ( INT ButtonIdx = VisibleButtons.Num() - 1; ButtonIdx >= 0; ButtonIdx-- )
	{
		UUICalloutButton* Button = VisibleButtons(ButtonIdx);
		if ( Button != NULL )
		{
			const FLOAT ButtonSize = Button->GetBounds(ButtonBarOrientation, EVALPOS_PixelViewport);
			ButtonSizes(ButtonIdx) = ButtonSize;
			TotalButtonSize += ButtonSize;
		}
	}

	if ( ButtonLayout == CBLT_Centered )
	{
		const FLOAT FirstButtonPosition = GetPosition(UIFACE_Left + ButtonBarOrientation, EVALPOS_PixelViewport, bIncludeViewportOrigin) + (PanelSize * 0.5f - TotalButtonSize * 0.5f);
		FLOAT CurrentButtonPos = FirstButtonPosition;
		for ( INT ButtonIdx = VisibleButtons.Num() - 1; ButtonIdx >= 0; ButtonIdx-- )
		{
			UUICalloutButton* Button = VisibleButtons(ButtonIdx);
			if ( Button != NULL )
			{
				Button->UUIScreenObject::SetPosition(CurrentButtonPos, NearFace, EVALPOS_PixelViewport, bIncludeViewportOrigin, FALSE);
				Button->RenderBounds[NearFace] = OffsetValue + CurrentButtonPos;
				Button->Position.ValidatePosition(NearFace);
				Button->DockTargets.MarkResolved(NearFace);

				CurrentButtonPos += ButtonSizes(ButtonIdx);
				Button->UUIScreenObject::SetPosition(CurrentButtonPos, FarFace, EVALPOS_PixelViewport, bIncludeViewportOrigin, FALSE);
				Button->RenderBounds[FarFace] = OffsetValue + CurrentButtonPos;
				Button->Position.ValidatePosition(FarFace);
				Button->DockTargets.MarkResolved(FarFace);
			}
		}
	}
	else if ( ButtonLayout == CBLT_Justified && VisibleButtons.Num() > 0 )
	{
		// the size of the region where each button will be centered
		const FLOAT ButtonRegionSize = PanelSize / (FLOAT)VisibleButtons.Num();
		const FLOAT StartingPosition = GetPosition(UIFACE_Left + ButtonBarOrientation, EVALPOS_PixelViewport, bIncludeViewportOrigin);

		for ( INT ButtonIdx = VisibleButtons.Num() - 1; ButtonIdx >= 0; ButtonIdx-- )
		{
			UUICalloutButton* Button = VisibleButtons(ButtonIdx);
			if ( Button != NULL )
			{
				const FLOAT RegionStartingPosition = StartingPosition + (ButtonRegionSize * (VisibleButtons.Num() - ButtonIdx - 1));
				FLOAT CurrentButtonPos = RegionStartingPosition + (ButtonRegionSize * 0.5f - ButtonSizes(ButtonIdx) * 0.5f);

				Button->UUIScreenObject::SetPosition(CurrentButtonPos, NearFace, EVALPOS_PixelViewport, bIncludeViewportOrigin, FALSE);
				Button->RenderBounds[NearFace] = OffsetValue + CurrentButtonPos;
				Button->Position.ValidatePosition(NearFace);
				Button->DockTargets.MarkResolved(NearFace);

				CurrentButtonPos += ButtonSizes(ButtonIdx);
				Button->UUIScreenObject::SetPosition(CurrentButtonPos + ButtonSizes(ButtonIdx), FarFace, EVALPOS_PixelViewport, bIncludeViewportOrigin, FALSE);
				Button->RenderBounds[FarFace] = OffsetValue + CurrentButtonPos;
				Button->Position.ValidatePosition(FarFace);
				Button->DockTargets.MarkResolved(FarFace);
			}
		}
	}

	// no longer need these updates
	bEnableSceneUpdateNotifications = FALSE;
}

/**
 * Called when the scene receives a notification that the viewport has been resized.  Propagated down to all children.
 *
 * @param	OldViewportSize		the previous size of the viewport
 * @param	NewViewportSize		the new size of the viewport
 */
void UUICalloutButtonPanel::NotifyResolutionChanged( const FVector2D& OldViewportSize, const FVector2D& NewViewportSize )
{
	Super::NotifyResolutionChanged(OldViewportSize, NewViewportSize);

	RequestButtonDockingUpdate();
}

/* === UObject interface === */
/**
 * Called when a property value has been changed in the editor.
 */
void UUICalloutButtonPanel::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if (PropertyName == TEXT("ButtonLayout") 
			||	PropertyName == TEXT("bRefreshButtonDocking")
			||	PropertyName == TEXT("ButtonBarOrientation")
			||	PropertyName == TEXT("ButtonPadding") )
			{
				RequestButtonDockingUpdate();
			}
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Presave function. Gets called once before an object gets serialized for saving. This function is necessary
 * for save time computation as Serialize gets called three times per object from within UObject::SavePackage.
 *
 * @warning: Objects created from within PreSave will NOT have PreSave called on them!!!
 *
 * This version syncs the CalloutButtonAliases array with the tags of the buttons currently in the CalloutButtons
 * array, then calls SaveConfig() to publish these tags to the .ini.
 */
void UUICalloutButtonPanel::PreSave()
{
	Super::PreSave();

	UBOOL bRequiresSaveConfig = FALSE;

	TArray<FName> NewAliases;
	NewAliases.Empty(CalloutButtons.Num());

	for ( INT ButtonIdx = 0, AliasIdx=0; ButtonIdx < CalloutButtons.Num(); ButtonIdx++ )
	{
		UUICalloutButton* CalloutButton = CalloutButtons(ButtonIdx);
		if ( CalloutButton != NULL && CalloutButton->InputAliasTag != NAME_None )
		{
			NewAliases.AddItem(CalloutButton->InputAliasTag);
		}
	}

	if ( NewAliases != CalloutButtonAliases )
	{
		CalloutButtonAliases = NewAliases;
		SaveConfig();
	}
}
	
/**
 * Serializer - this version serializes the lookup map during transactions.
 */
void UUICalloutButtonPanel::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);

	if ( Ar.IsTransacting() )
	{
		Ar << ButtonInputKeyMappings;
	}
}

/* ==========================================================================================================
	UUIContainer
========================================================================================================== */
/**
 * Adds the specified face to the owning scene's DockingStack for the owning widget.  Takes wrap behavior and
 * autosizing into account, ensuring that all widget faces are added to the scene's docking stack in the appropriate
 * order.
 *
 * @param	DockingStack	the docking stack to add this docking node to.  Generally the scene's DockingStack.
 * @param	Face			the face that should be added
 *
 * @return	TRUE if a docking node was added to the scene's DockingStack for the specified face, or if a docking node already
 *			existed in the stack for the specified face of this widget.
 */
UBOOL UUIContainer::AddDockingNode( TLookupMap<FUIDockingNode>& DockingStack, EUIWidgetFace Face )
{
	if ( AutoAlignment != NULL )
	{
		AutoAlignment->AddDockingNode(DockingStack, Face);
	}

	return Super::AddDockingNode(DockingStack, Face);
}

/**
 * Evalutes the Position value for the specified face into an actual pixel value.  Should only be
 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
 *
 * @param	Face	the face that should be resolved
 */
void UUIContainer::ResolveFacePosition( EUIWidgetFace Face )
{
	Super::ResolveFacePosition(Face);

	if ( AutoAlignment != NULL )
	{
		AutoAlignment->ResolveFacePosition(Face);
	}
}

/**
 * Called when a property value has been changed in the editor.
 */
void UUIContainer::PostEditChange( UProperty* PropertyThatChanged )
{
	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Called when a property value has been changed in the editor.
 */
void UUIContainer::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PostEditChange(PropertyThatChanged);
}


/* ==========================================================================================================
	UIPanel
========================================================================================================== */
/* === UIPanel interface === */
/**
 * Changes the background image for this button, creating the wrapper UITexture if necessary.
 *
 * @param	NewImage		the new surface to use for this UIImage
 * @param	NewCoordinates	the optional coordinates for use with texture atlasing
 */
void UUIPanel::SetBackgroundImage( USurface* NewImage )
{
	if ( BackgroundImageComponent != NULL )
	{
		BackgroundImageComponent->SetImage(NewImage);
	}
}

/* === UIObject interface === */
/**
 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
 *
 * This version adds the BackgroundImageComponent (if non-NULL) to the StyleSubscribers array.
 */
void UUIPanel::InitializeStyleSubscribers()
{
	Super::InitializeStyleSubscribers();

	VALIDATE_COMPONENT(BackgroundImageComponent);
	AddStyleSubscriber(BackgroundImageComponent);
}

/* === UUIScreenObject interface === */

/**
 * Routes rendering calls to children of this screen object.
 *
 * This version sets a clip mask on the canvas while the children are being rendered.
 *
 * @param	Canvas	the canvas to use for rendering
 * @param	UIPostProcessGroup	Group determines current pp pass that is being rendered
 */
void UUIPanel::Render_Children( FCanvas* Canvas, EUIPostProcessGroup UIPostProcessGroup )
{
	if ( bEnforceClipping )
	{
		Canvas->PushMaskRegion( 
			RenderBounds[UIFACE_Left],
			RenderBounds[UIFACE_Top],
			RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left],
			RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top]
		);
	}

	Super::Render_Children(Canvas,UIPostProcessGroup);

	if ( bEnforceClipping )
	{
		Canvas->PopMaskRegion();
	}
}

/**
 * Render this widget.
 *
 * @param	Canvas	the canvas to use for rendering this widget
 */
void UUIPanel::Render_Widget( FCanvas* Canvas )
{
	if ( BackgroundImageComponent != NULL )
	{
		FRenderParameters Parameters(
			RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top],
			RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left],
			RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top],
			NULL, GetViewportHeight()
			);

		BackgroundImageComponent->RenderComponent(Canvas, Parameters);
	}
}

/* === UObject interface === */
/**
 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
 */
void UUIPanel::PreEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PreEditChange(PropertyThatChanged);

	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("BackgroundImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty && BackgroundImageComponent != NULL )
				{
					// the user either cleared the value of the BackgroundImageComponent (which should never happen since
					// we use the 'noclear' keyword on the property declaration), or is assigning a new value to the BackgroundImageComponent.
					// Unsubscribe the current component from our list of style resolvers.
					RemoveStyleSubscriber(BackgroundImageComponent);
				}
			}
		}
	}
}

/**
 * Called when a property value has been changed in the editor.
 */
void UUIPanel::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("BackgroundImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty )
				{
					if ( BackgroundImageComponent != NULL )
					{
						UUIComp_DrawImage* ComponentTemplate = GetArchetype<UUIPanel>()->BackgroundImageComponent;
						if ( ComponentTemplate != NULL )
						{
							BackgroundImageComponent->StyleResolverTag = ComponentTemplate->StyleResolverTag;
						}
						else
						{
							BackgroundImageComponent->StyleResolverTag = TEXT("Panel Background Style");
						}

						// user created a new background image component - add it to the list of style subscribers
						AddStyleSubscriber(BackgroundImageComponent);

						// now initialize the component's image
						BackgroundImageComponent->SetImage(BackgroundImageComponent->GetImage());
					}
				}
				else if ( BackgroundImageComponent != NULL )
				{
					// a property of the ImageComponent was changed
					if ( ModifiedProperty->GetFName() == TEXT("ImageRef") && BackgroundImageComponent->GetImage() != NULL )
					{
#if 0
						USurface* CurrentValue = BackgroundImageComponent->GetImage();

						// changed the value of the image texture/material
						// clear the data store binding
						//@fixme ronp - do we always need to clear the data store binding?
 						SetDataStoreBinding(TEXT(""));

						// clearing the data store binding value may have cleared the value of the image component's texture,
						// so restore the value now
						SetImage(CurrentValue);
#endif
					}
				}
			}
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Called after this object has been completely de-serialized.
 */
void UUIPanel::PostLoad()
{
	Super::PostLoad();
}


/* ==========================================================================================================
	UIFrameBox
========================================================================================================== */
/* === UIFrameBox interface === */
/**
 * Changes the background image for one of the image components.
 *
 * @param	ImageToSet		The image component we are going to set the image for.
 * @param	NewImage		the new surface to use for this UIImage
 */
void UUIFrameBox::SetBackgroundImage( EFrameBoxImage ImageToSet, USurface* NewImage )
{
	if ( BackgroundImageComponent[ImageToSet] != NULL )
	{
		BackgroundImageComponent[ImageToSet]->SetImage(NewImage);
	}
}

/* === UIObject interface === */
/**
 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
 *
 * This version adds the BackgroundImageComponent (if non-NULL) to the StyleSubscribers array.
 */
void UUIFrameBox::InitializeStyleSubscribers()
{
	Super::InitializeStyleSubscribers();

	for(INT ImageIdx=0; ImageIdx<FBI_MAX; ImageIdx++)
	{
		VALIDATE_COMPONENT(BackgroundImageComponent[ImageIdx]);
		AddStyleSubscriber(BackgroundImageComponent[ImageIdx]);
	}
}

/* === UUIScreenObject interface === */
/**
 * Render this widget.
 *
 * @param	Canvas	the canvas to use for rendering this widget
 */
void UUIFrameBox::Render_Widget( FCanvas* Canvas )
{
	if ( BackgroundImageComponent != NULL )
	{
		FRenderParameters ImageParams;
		FRenderParameters Parameters(
			RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top],
			RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left],
			RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top],
			NULL, GetViewportHeight()
			);

		// Top Left
		ImageParams = Parameters;
		ImageParams.DrawXL = BackgroundCornerSizes.TopLeft[0];
		ImageParams.DrawYL = BackgroundCornerSizes.TopLeft[1];
		BackgroundImageComponent[FBI_TopLeft]->RenderComponent(Canvas, ImageParams);

		// Top
		ImageParams = Parameters;
		ImageParams.DrawX += BackgroundCornerSizes.TopLeft[0];
		ImageParams.DrawXL -= (BackgroundCornerSizes.TopLeft[0] + BackgroundCornerSizes.TopRight[0]);
		ImageParams.DrawYL = BackgroundCornerSizes.TopHeight;
		BackgroundImageComponent[FBI_Top]->RenderComponent(Canvas, ImageParams);

		// Top Right
		ImageParams = Parameters;
		ImageParams.DrawX += Parameters.DrawXL - BackgroundCornerSizes.TopRight[0];
		ImageParams.DrawXL = BackgroundCornerSizes.TopRight[0];
		ImageParams.DrawYL = BackgroundCornerSizes.TopRight[1];
		BackgroundImageComponent[FBI_TopRight]->RenderComponent(Canvas, ImageParams);

		// Center Left
		ImageParams = Parameters;
		ImageParams.DrawY += BackgroundCornerSizes.TopLeft[1];
		ImageParams.DrawXL = BackgroundCornerSizes.CenterLeftWidth;
		ImageParams.DrawYL -= (BackgroundCornerSizes.TopLeft[1] + BackgroundCornerSizes.BottomLeft[1]);
		BackgroundImageComponent[FBI_CenterLeft]->RenderComponent(Canvas, ImageParams);

		// Center
		ImageParams = Parameters;
		ImageParams.DrawX += BackgroundCornerSizes.CenterLeftWidth;
		ImageParams.DrawY += BackgroundCornerSizes.TopHeight;
		ImageParams.DrawXL -= (BackgroundCornerSizes.CenterLeftWidth + BackgroundCornerSizes.CenterRightWidth);
		ImageParams.DrawYL -= (BackgroundCornerSizes.TopHeight + BackgroundCornerSizes.BottomHeight);
		BackgroundImageComponent[FBI_Center]->RenderComponent(Canvas, ImageParams);

		// Center Right
		ImageParams = Parameters;
		ImageParams.DrawX += Parameters.DrawXL - BackgroundCornerSizes.CenterRightWidth;
		ImageParams.DrawY += BackgroundCornerSizes.TopRight[1];
		ImageParams.DrawXL = BackgroundCornerSizes.CenterRightWidth;
		ImageParams.DrawYL -= (BackgroundCornerSizes.TopRight[1] + BackgroundCornerSizes.BottomRight[1]);
		BackgroundImageComponent[FBI_CenterRight]->RenderComponent(Canvas, ImageParams);

		// Bottom Left
		ImageParams = Parameters;
		ImageParams.DrawY += Parameters.DrawYL - BackgroundCornerSizes.BottomLeft[1];
		ImageParams.DrawXL = BackgroundCornerSizes.BottomLeft[0];
		ImageParams.DrawYL = BackgroundCornerSizes.BottomLeft[1];
		BackgroundImageComponent[FBI_BottomLeft]->RenderComponent(Canvas, ImageParams);

		// Bottom
		ImageParams = Parameters;
		ImageParams.DrawX += BackgroundCornerSizes.BottomLeft[0];
		ImageParams.DrawY += Parameters.DrawYL - BackgroundCornerSizes.BottomHeight;
		ImageParams.DrawXL -= (BackgroundCornerSizes.BottomLeft[0] + BackgroundCornerSizes.BottomRight[0]);
		ImageParams.DrawYL = BackgroundCornerSizes.BottomHeight;
		BackgroundImageComponent[FBI_Bottom]->RenderComponent(Canvas, ImageParams);

		// Bottom Right
		ImageParams = Parameters;
		ImageParams.DrawX += Parameters.DrawXL - BackgroundCornerSizes.BottomRight[0];
		ImageParams.DrawY += Parameters.DrawYL - BackgroundCornerSizes.BottomRight[1];
		ImageParams.DrawXL = BackgroundCornerSizes.BottomRight[0];
		ImageParams.DrawYL = BackgroundCornerSizes.BottomRight[1];
		BackgroundImageComponent[FBI_BottomRight]->RenderComponent(Canvas, ImageParams);
	}
}

/* === UObject interface === */
/**
 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
 */
void UUIFrameBox::PreEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PreEditChange(PropertyThatChanged);

	/*
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("BackgroundImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty && BackgroundImageComponent != NULL )
				{
					// the user either cleared the value of the BackgroundImageComponent (which should never happen since
					// we use the 'noclear' keyword on the property declaration), or is assigning a new value to the BackgroundImageComponent.
					// Unsubscribe the current component from our list of style resolvers.
					RemoveStyleSubscriber(BackgroundImageComponent);
				}
			}
		}
	}
	*/
}

/**
 * Called when a property value has been changed in the editor.
 */
void UUIFrameBox::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	/*
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("BackgroundImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty )
				{
					if ( BackgroundImageComponent != NULL )
					{
						UUIComp_DrawImage* ComponentTemplate = GetArchetype<UUIPanel>()->BackgroundImageComponent;
						if ( ComponentTemplate != NULL )
						{
							BackgroundImageComponent->StyleResolverTag = ComponentTemplate->StyleResolverTag;
						}
						else
						{
							BackgroundImageComponent->StyleResolverTag = TEXT("Panel Background Style");
						}

						// user created a new background image component - add it to the list of style subscribers
						AddStyleSubscriber(BackgroundImageComponent);

						// now initialize the component's image
						BackgroundImageComponent->SetImage(BackgroundImageComponent->GetImage());
					}
				}
				else if ( BackgroundImageComponent != NULL )
				{
					// a property of the ImageComponent was changed
					if ( ModifiedProperty->GetFName() == TEXT("ImageRef") && BackgroundImageComponent->GetImage() != NULL )
					{
#if 0
						USurface* CurrentValue = BackgroundImageComponent->GetImage();

						// changed the value of the image texture/material
						// clear the data store binding
						//@fixme ronp - do we always need to clear the data store binding?
 						SetDataStoreBinding(TEXT(""));

						// clearing the data store binding value may have cleared the value of the image component's texture,
						// so restore the value now
						SetImage(CurrentValue);
#endif
					}
				}
			}
		}
	}
	*/

	Super::PostEditChange(PropertyThatChanged);
}

/* ==========================================================================================================
	UUISafeRegionPanel
========================================================================================================== */
/**
 * Called at the beginning of the first scene update and propagated to all widgets in the scene.  Provides classes with
 * an opportunity to initialize anything that couldn't be setup earlier due to lack of a viewport.
 *
 * This version sets the scene's PrimarySafeRegionPanel value to this panel if bPrimarySafeRegion is TRUE
 */
void UUISafeRegionPanel::PreInitialSceneUpdate()
{
	if ( bPrimarySafeRegion )
	{
		UUIScene* SceneOwner = GetScene();
		if ( SceneOwner != NULL && SceneOwner->PrimarySafeRegionPanel == NULL )
		{
			SceneOwner->PrimarySafeRegionPanel = this;
		}
	}

	Super::PreInitialSceneUpdate();
}

/**
 * Initializes the panel and sets its position to match the safe region.
 * This is ugly.
 */
void UUISafeRegionPanel::ResolveFacePosition(EUIWidgetFace Face)
{
	// if this is the first face of this panel that is being resolved, set the position of the panel according to the current viewport size
	if ( GetNumResolvedFaces() == 0 )
	{
		AlignPanel();
	}

	Super::ResolveFacePosition(Face);
}

void UUISafeRegionPanel::AlignPanel()
{
	FVector2D CompleteViewportSize(EC_EventParm);
	FVector2D ViewportSize(EC_EventParm), ViewportOrigin(EC_EventParm);

	// Calculate the Preview Platform.
	if( OwnerScene && OwnerScene->GetViewportSize(ViewportSize) )
	{
		UBOOL bApplySafeZone[UIFACE_MAX] = { TRUE, TRUE, TRUE, TRUE };
		if ( UUIInteraction::GetPlayerCount() > 1 )
		{
			OwnerScene->GetViewportOrigin(ViewportOrigin);
			OwnerScene->SceneClient->GetViewportSize(NULL, CompleteViewportSize);

			if ( Abs(ViewportOrigin.X) > DELTA )
			{
				bApplySafeZone[UIFACE_Left] = FALSE;
			}

			if ( Abs(ViewportOrigin.Y) > DELTA )
			{
				bApplySafeZone[UIFACE_Top] = FALSE;
			}

			if ( !ARE_FLOATS_EQUAL(CompleteViewportSize.X,ViewportSize.X) )
			{
				// split horizontally - only one side of the viewport needs a safezone
				bApplySafeZone[UIFACE_Right] = !bApplySafeZone[UIFACE_Left];
			}

			if ( !ARE_FLOATS_EQUAL(CompleteViewportSize.Y, ViewportSize.Y) )
			{
				// split vertically - only one side of the viewport needs a safezone
				bApplySafeZone[UIFACE_Bottom] = !bApplySafeZone[UIFACE_Top];
			}
		}
		else
		{
			CompleteViewportSize = ViewportSize;
		}

		// Calculate the current aspect ratio and safe zone percentage
		const BYTE EvalType = EVALPOS_PixelViewport;
		const FLOAT CurrentAspectRatio = CompleteViewportSize.Y != 0 ? CompleteViewportSize.X / CompleteViewportSize.Y : UCONST_ASPECTRATIO_Normal;

		const FLOAT SafeZonePercentage = bUseFullRegionIn4x3 && (bForce4x3AspectRatio || Abs(CurrentAspectRatio - UCONST_ASPECTRATIO_Normal) < DELTA)
			? RegionPercentages[ESRT_FullRegion]
			: RegionPercentages[RegionType];

		const FLOAT DeadZonePercentage = (1.f - SafeZonePercentage) * 0.5f;

		if ( bForce4x3AspectRatio )
		{
			// if we're not already at the correct aspect ratio
			if ( Abs(CurrentAspectRatio - UCONST_ASPECTRATIO_Normal) > DELTA )
			{
				// the screen is wider than a normal TV screen - reduce the width of this panel such that the ratio between width and height is 4:3
				if ( CurrentAspectRatio > UCONST_ASPECTRATIO_Normal )
				{
					// apply the safe zone border
					const FLOAT SafeZoneHeight = CompleteViewportSize.Y * SafeZonePercentage;
					const FLOAT SafeZoneWidth = SafeZoneHeight * UCONST_ASPECTRATIO_Normal;

					// and center the panel as well
					const FVector2D DeadZoneSize(
						CompleteViewportSize.X * 0.5f - SafeZoneWidth * 0.5f,
						CompleteViewportSize.Y * 0.5f - SafeZoneHeight * 0.5f
						);

					const FLOAT PosX = bApplySafeZone[UIFACE_Left] ? DeadZoneSize.X : 0.f;
					const FLOAT PosY = bApplySafeZone[UIFACE_Top] ? DeadZoneSize.Y : 0.f;
					const FLOAT PosX2 = bApplySafeZone[UIFACE_Right] ? ViewportSize.X - DeadZoneSize.X : SafeZoneWidth * 0.5f;
					const FLOAT PosY2 = bApplySafeZone[UIFACE_Bottom] ? ViewportSize.Y - DeadZoneSize.Y : SafeZoneHeight * 0.5f;
					SetPosition(PosX, PosY, PosX2, PosY2, EVALPOS_PixelViewport);
				}
				// otherwise, the screen is MORE square than a regular TV screen - adjust the height of this panel such that the ratio is back to 4:3
				else
				{

					// apply the safe zone border
					const FLOAT SafeZoneWidth = CompleteViewportSize.X * SafeZonePercentage;
					const FLOAT SafeZoneHeight = SafeZoneWidth / UCONST_ASPECTRATIO_Normal;

					// and center the panel as well
					const FVector2D DeadZoneSize(
						CompleteViewportSize.X * 0.5f - SafeZoneWidth * 0.5f,
						CompleteViewportSize.Y * 0.5f - SafeZoneHeight * 0.5f
						);

					// reposition the panel using absolute pixel coordinates
					const FLOAT PosX = bApplySafeZone[UIFACE_Left] ? DeadZoneSize.X : 0.f;
					const FLOAT PosY = bApplySafeZone[UIFACE_Top] ? DeadZoneSize.Y : 0.f;
					const FLOAT PosX2 = bApplySafeZone[UIFACE_Right] ? ViewportSize.X - DeadZoneSize.X : SafeZoneWidth * 0.5f;
					const FLOAT PosY2 = bApplySafeZone[UIFACE_Bottom] ? ViewportSize.Y - DeadZoneSize.Y : SafeZoneHeight * 0.5f;
					SetPosition(PosX, PosY, PosX2, PosY2, EVALPOS_PixelViewport);
				}
			}
			else
			{
				// viewport is already at the correct aspect ratio, so just apply the safe zone border
				const FVector2D DeadZoneSize(CompleteViewportSize.X * DeadZonePercentage, CompleteViewportSize.Y * DeadZonePercentage);
				
				const FLOAT PosX = bApplySafeZone[UIFACE_Left] ? DeadZoneSize.X : 0.f;
				const FLOAT PosY = bApplySafeZone[UIFACE_Top] ? DeadZoneSize.Y : 0.f;
				const FLOAT PosX2 = bApplySafeZone[UIFACE_Right] ? ViewportSize.X - DeadZoneSize.X : ViewportSize.X;
				const FLOAT PosY2 = bApplySafeZone[UIFACE_Bottom] ? ViewportSize.Y - DeadZoneSize.Y : ViewportSize.Y;
				SetPosition(PosX, PosY, PosX2, PosY2, EVALPOS_PixelViewport);
			}
		}
		else
		{
			// not maintaining aspect ratio - just apply the safe zone border
			const FVector2D DeadZoneSize(CompleteViewportSize.X * DeadZonePercentage, CompleteViewportSize.Y * DeadZonePercentage);

			const FLOAT PosX = bApplySafeZone[UIFACE_Left] ? DeadZoneSize.X : 0.f;
			const FLOAT PosY = bApplySafeZone[UIFACE_Top] ? DeadZoneSize.Y : 0.f;
			const FLOAT PosX2 = bApplySafeZone[UIFACE_Right] ? ViewportSize.X - DeadZoneSize.X : ViewportSize.X;
			const FLOAT PosY2 = bApplySafeZone[UIFACE_Bottom] ? ViewportSize.Y - DeadZoneSize.Y : ViewportSize.Y;
			SetPosition(PosX, PosY, PosX2, PosY2, EVALPOS_PixelViewport);
		}
	}
}


/**
 * Called when a property value has been changed in the editor.
 */
void UUISafeRegionPanel::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* OutermostProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( OutermostProperty != NULL )
		{
			FName PropertyName = OutermostProperty->GetFName();
			if (PropertyName == TEXT("RegionType")
			||	PropertyName == TEXT("RegionPercentages")
			||	PropertyName == TEXT("bForce4x3AspectRatio")
			||	PropertyName == TEXT("bUseFullRegionIn4x3"))
			{
				RefreshPosition();
			}
			else if ( PropertyName == TEXT("bPrimarySafeRegion") )
			{
				UUIScene* SceneOwner = GetScene();
				if ( SceneOwner )
				{
					if ( bPrimarySafeRegion )
					{
						TArray<UUIObject*> SceneChildren;
						TArray<UUIObject*> ExclusionSet;
						ExclusionSet.AddItem(this);

						SceneOwner->GetChildren(SceneChildren, TRUE, &ExclusionSet);

						TArray<UUISafeRegionPanel*> OtherSafeRegionPanels;
						if ( ContainsObjectOfClass(SceneChildren, UUISafeRegionPanel::StaticClass(), FALSE, ((TArray<UUIObject*>*)&OtherSafeRegionPanels)) )
						{
							for ( INT PanelIdx = 0; PanelIdx < OtherSafeRegionPanels.Num(); PanelIdx++ )
							{
								UUISafeRegionPanel* OtherPanel = OtherSafeRegionPanels(PanelIdx);
								if ( OtherPanel->bPrimarySafeRegion )
								{
									OtherPanel->bPrimarySafeRegion = FALSE;
								}
							}
						}

						if ( SceneOwner->PrimarySafeRegionPanel != this )
						{
							SceneOwner->PrimarySafeRegionPanel = this;
						}
					}
					else if ( SceneOwner->PrimarySafeRegionPanel == this )
					{
						SceneOwner->PrimarySafeRegionPanel = NULL;
					}
				}
			}
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

/* ==========================================================================================================
	UITabControl
========================================================================================================== */
/* === UITabControl interface === */
/**
 * Enables the bUpdateLayout flag and triggers a scene update to occur during the next frame.
 */
void UUITabControl::RequestLayoutUpdate()
{
	bUpdateLayout = TRUE;
	RequestSceneUpdate(TRUE,TRUE);
}

/**
 * Positions and resizes the tab buttons according the tab control's configuration.
 */
void UUITabControl::ReapplyLayout()
{
	// first, setup all the docking links between the tab control, tab buttons, and tab pages
	SetupDockingRelationships();

	// now resize the tabs according to the value of TabSizeMode
	switch ( TabSizeMode )
	{
	case TAST_Manual:
		// nothing - the size for each button is set manually
		break;

	case TAST_Fill:
		// in order to resize the tab buttons, we must first know the width (or height, if dockface is left or right)
		// of the tab control.  We won't know that until after ResolveFacePosition is called for both faces.  So here
		// we just make sure that autosizing is NOT enabled for the tab buttons.
		// the width (or height if the tabs are vertical) of each tab button is determined
		// by the length of its caption; so just enable auto-sizing
		for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
		{
			UUITabButton* TabButton = Pages(PageIndex)->TabButton;
			if ( TabButton != NULL && TabButton->StringRenderComponent != NULL )
			{
				TabButton->StringRenderComponent->eventEnableAutoSizing(UIORIENT_Horizontal, FALSE);
			}
		}
		break;

	case TAST_Auto:
		{
			const FLOAT HalfHorzPadding = TabButtonPadding[UIORIENT_Horizontal].Value * 0.5f;

			// the width (or height if the tabs are vertical) of each tab button is determined
			// by the length of its caption; so just enable auto-sizing
			for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
			{
				UUITabButton* TabButton = Pages(PageIndex)->TabButton;
				if ( TabButton != NULL && TabButton->StringRenderComponent != NULL )
				{
					TabButton->StringRenderComponent->eventEnableAutoSizing(UIORIENT_Horizontal, TRUE);

					EUIExtentEvalType ScaleType = static_cast<EUIExtentEvalType>(TabButtonPadding[UIORIENT_Horizontal].ScaleType);
					TabButton->StringRenderComponent->eventSetAutoSizePadding(UIORIENT_Horizontal, 
						HalfHorzPadding, HalfHorzPadding, ScaleType, ScaleType);
				}
			}
		}
		break;
	}
}

/**
 * Set up the docking links between the tab control, buttons, and pages, based on the TabDockFace.
 */
void UUITabControl::SetupDockingRelationships()
{
	if ( TabDockFace < UIFACE_MAX )
	{
		FLOAT ActualButtonHeight = TabButtonSize.GetValue(this);
		FLOAT ButtonVerticalPadding = TabButtonPadding[UIORIENT_Vertical].GetValue(this);

		EUIWidgetFace TargetFace=(EUIWidgetFace)TabDockFace;
		EUIWidgetFace SourceFace = GetOppositeFace(TabDockFace);

		EUIWidgetFace AlignmentSourceFace, AlignmentTargetFace;
		if ( TargetFace == UIFACE_Top || TargetFace == UIFACE_Bottom )
		{
			AlignmentSourceFace = AlignmentTargetFace = UIFACE_Left;
			if ( TargetFace == UIFACE_Bottom )
			{
				ActualButtonHeight *= -1;
				ButtonVerticalPadding *= -1;
			}
		}
		else
		{
			AlignmentSourceFace = AlignmentTargetFace = UIFACE_Top;
		}

		// the first button will dock to the tab control itself.
		UUIObject* CurrentAlignmentDockTarget = this;
		for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
		{
			UUITabPage* TabPage = Pages(PageIndex);
			UUITabButton* TabButton = TabPage->TabButton;

			checkSlow(TabPage!=NULL);
			checkSlow(TabButton!=NULL);

			// dock the buttons to the tab control - (the following comments assume we're docked to the
			// top face of the tab control, for sake of example):
			// dock the top face of the button to the top face of the tab control
			TabButton->SetDockParameters(TargetFace, this, TargetFace, 0.f);
			// dock the bottom face of the button to the top face of the tab control using the configured
			// button height as the dock padding
			TabButton->SetDockParameters(SourceFace, this, TargetFace, ActualButtonHeight + ButtonVerticalPadding);
			// dock the left face of the button to the left face of the tab control (if it's the first button),
			// or the right face of the previous button in the list (if not the first button).  The first time
			// through this loop, AlignmentSourceFace and AlignmentTargetFace are the same value, but we'll switch
			// AlignmentTargetFace to the opposite face right after this
			TabButton->SetDockParameters(AlignmentSourceFace, CurrentAlignmentDockTarget, AlignmentTargetFace, 0.f);

			// the next button in the list will dock to this button
			CurrentAlignmentDockTarget = TabButton;

			// the next button in the list will dock to the face opposite the face it's docking (i.e. dock left face to right face)
			AlignmentTargetFace = GetOppositeFace(AlignmentSourceFace);


			// now the page itself - dock the top face of the panel to the bottom face of its TabButton
			TabPage->SetDockParameters(TargetFace, TabButton, SourceFace, 0.f);

			// then dock the remaining faces of the page to the tab control
			for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
			{
				if ( FaceIndex != TargetFace )
				{
					TabPage->SetDockParameters(FaceIndex, this, FaceIndex, 0.f);
				}
			}
		}
	}
}

/**
 * Returns the number of pages in this tab control.
 */
INT UUITabControl::GetPageCount() const
{
	return Pages.Num();
}

/**
 * Returns a reference to the page at the specified index.
 */
UUITabPage* UUITabControl::GetPageAtIndex( INT PageIndex ) const
{
	UUITabPage* Result = NULL;
	if ( Pages.IsValidIndex(PageIndex) )
	{
		Result = Pages(PageIndex);
	}
	return Result;
}

/**
 * Returns a reference to the tab button which is currently in the Targeted state, or NULL if no buttons are in that state.
 */
UUITabButton* UUITabControl::FindTargetedTab( INT PlayerIndex ) const
{
	UUITabButton* Result = NULL;

	for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
	{
		UUITabPage* Page = Pages(PageIndex);
		if ( Page != NULL && Page->TabButton != NULL && Page->TabButton->IsTargeted(PlayerIndex) )
		{
			Result = Page->TabButton;
			break;
		}
	}

	return Result;
}

/**
 * Creates a new UITabPage of the specified class as well as its associated tab button.
 *
 * @param	TabPageClass	the class to use for creating the tab page.
 *
 * @return	a pointer to a new instance of the specified UITabPage class
 */
UUITabPage* UUITabControl::CreateTabPage( UClass* TabPageClass, UUITabPage* PagePrefab/*=0*/ )
{
	UUITabPage* Result = NULL;

	if ( TabPageClass != NULL )
	{
		checkSlow(TabPageClass->IsChildOf(UUITabPage::StaticClass()));

		// first, we need to create the tab button; to do this, we call a static method in the tab page class,
		// which we'll need the TabPageClass's CDO for
		UUITabButton* TabButton = NULL;
		if ( PagePrefab == NULL )
		{
			UUITabPage* TabPageCDO = TabPageClass->GetDefaultObject<UUITabPage>();
			TabButton = TabPageCDO->eventCreateTabButton(this);
		}
		else
		{
			TabButton = PagePrefab->eventCreateTabButton(this);
		}

		if ( TabButton != NULL )
		{
			FScopedObjectStateChange TabButtonNotification(TabButton);

			// now that we have the TabButton, have it create the tab page using the specified class
			UUITabPage* NewTabPage = Cast<UUITabPage>(TabButton->CreateWidget(TabButton, TabPageClass, PagePrefab));
			FScopedObjectStateChange TabPageNotification(NewTabPage);

			// need to link the tab page and tab button together.
			if ( NewTabPage->eventLinkToTabButton(TabButton, this) )
			{
				Result = NewTabPage;
			}
			else
			{
				TabPageNotification.CancelEdit();
				TabButtonNotification.CancelEdit();
			}
		}
	}
	return Result;
}

/* === UIObject interface === */
/**
 * Render this widget.
 *
 * @param	Canvas	the FCanvas to use for rendering this widget
 */
void UUITabControl::Render_Widget( FCanvas* Canvas )
{
	Super::Render_Widget(Canvas);
}

/**
 * Adds docking nodes for all faces of this widget to the specified scene
 *
 * @param	DockingStack	the docking stack to add this widget's docking.  Generally the scene's DockingStack.
 *
 * @return	TRUE if docking nodes were successfully added for all faces of this widget.
 */
UBOOL UUITabControl::AddDockingLink( TLookupMap<FUIDockingNode>& DockingStack )
{
	if ( bUpdateLayout )
	{
		bUpdateLayout = FALSE;
		ReapplyLayout();
	}

	return Super::AddDockingLink(DockingStack);
}

/**
 * Adds the specified face to the DockingStack for the specified widget.
 *
 * This version ensures that the tab buttons faces (and thus, the size of their captions) have already been resolved
 * Only relevant when the TabSizeMode is TAST_Fill, because we must make sure that all buttons are at least wide enough
 * to fit the largest caption of the group.
 *
 * @param	DockingStack	the docking stack to add this docking node to.  Generally the scene's DockingStack.
 * @param	Face			the face that should be added
 *
 * @return	TRUE if a docking node was added to the scene's DockingStack for the specified face, or if a docking node already
 *			existed in the stack for the specified face of this widget.
 */
UBOOL UUITabControl::AddDockingNode( TLookupMap<FUIDockingNode>& DockingStack, EUIWidgetFace Face )
{
	checkSlow(Face<UIFACE_MAX);

	if ( TabSizeMode == TAST_Fill )
	{
		switch( TabDockFace )
		{
		// docked to the top or bottom faces
		case UIFACE_Top:
		case UIFACE_Bottom:
			{
				if ( Face == UIFACE_Right && !DockingStack.HasKey(FUIDockingNode(this,Face)) )
				{
					for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
					{
						UUITabPage* Page = Pages(PageIndex);
						UUIButton* TabButton = Page->TabButton;
						for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
						{
							TabButton->AddDockingNode(DockingStack, (EUIWidgetFace)FaceIndex);
						}
					}
				}
			}
			break;

		case UIFACE_Left:
		case UIFACE_Right:
			if ( Face == UIFACE_Bottom && !DockingStack.HasKey(FUIDockingNode(this,Face)) )
			{
				for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
				{
					UUITabPage* Page = Pages(PageIndex);
					UUIButton* TabButton = Page->TabButton;
					for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
					{
						TabButton->AddDockingNode(DockingStack, (EUIWidgetFace)FaceIndex);
					}
				}
			}
			break;

		default:
			break;
		}
	}

	return Super::AddDockingNode(DockingStack, Face);
}

/**
 * Evalutes the Position value for the specified face into an actual pixel value.  Should only be
 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
 *
 * @param	Face	the face that should be resolved
 */
void UUITabControl::ResolveFacePosition( EUIWidgetFace Face )
{
	Super::ResolveFacePosition(Face);

	if ( Pages.Num() > 0 && TabSizeMode == TAST_Fill )
	{
		EUIWidgetFace FirstFace, SecondFace;
		EUIOrientation Orientation;
		switch( TabDockFace )
		{
		case UIFACE_Top:
		case UIFACE_Bottom:

			FirstFace = UIFACE_Left;
			SecondFace = UIFACE_Right;
			Orientation = UIORIENT_Horizontal;
			break;

		case UIFACE_Left:
		case UIFACE_Right:
			FirstFace = UIFACE_Top;
			SecondFace = UIFACE_Bottom;
			Orientation = UIORIENT_Vertical;
			break;

		default:
			FirstFace = UIFACE_MAX;
			SecondFace = UIFACE_MAX;
			Orientation = UIORIENT_MAX;
			checkf(false, TEXT("Invalid value for TabDockFace (UIFACE_MAX) in '%s'"), *GetWidgetPathName());
			break;
		}

		if ( Face == FirstFace || Face == SecondFace )
		{
			//@todo ronp - do I need to override AddDockingNode to ensure that these faces are added first?
			if ( HasPositionBeenResolved(FirstFace) && HasPositionBeenResolved(SecondFace) )
			{
				// get the size of the area available to render the buttons
				const FLOAT BoundingRegionSize = Position.GetBoundsExtent(this, Orientation, EVALPOS_PixelViewport);

				// determine how wide each button should be to fill the available space
				FLOAT TargetButtonSize = BoundingRegionSize / Pages.Num();

				// this is the smallest size a button can be
				FLOAT MinButtonSize = TargetButtonSize;

				// now check to see if any buttons are larget that TargetButtonWidth.  If so, it indicates that there isn't enough
				// space to fit all buttons evenly
				for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
				{
					UUITabButton* TabButton = Pages(PageIndex)->TabButton;
					if ( TabButton->StringRenderComponent != NULL && TabButton->StringRenderComponent->ValueString != NULL )
					{
						UUIString* CaptionString = TabButton->StringRenderComponent->ValueString;
						if ( CaptionString != NULL )
						{
							MinButtonSize = Max<FLOAT>(MinButtonSize, CaptionString->StringExtent[Orientation]);
						}
					}
				}

				// this will be where each button's right/bottom face will fall, in absolute pixels
				FLOAT CurrentButtonPosition = Position.GetPositionValue(this, FirstFace, EVALPOS_PixelViewport);

				// resize all buttons so that they are that size.
				//@fixme ronp - handle the case where MinButtonSize > TargetButtonWidth
				for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
				{
					UUITabButton* TabButton = Pages(PageIndex)->TabButton;

					// set the position and width of the button to the desired values.  Prevent SetPositionValue from triggering
					// another scene update (which will cause a flicker since the button would be rendered this frame
					// using its previously resolved position, but the next frame will use this new value); this requires
					// us to also update the button's RenderBounds for that face as well
					TabButton->Position.SetPositionValue(TabButton, CurrentButtonPosition, FirstFace, EVALPOS_PixelViewport, FALSE);
					TabButton->Position.SetPositionValue(TabButton, TargetButtonSize, SecondFace, EVALPOS_PixelOwner, FALSE);

					TabButton->RenderBounds[FirstFace] = CurrentButtonPosition;
					TabButton->RenderBounds[SecondFace] = CurrentButtonPosition + TargetButtonSize;

					// advance the button face location by the size of the button
					CurrentButtonPosition += TargetButtonSize;
				}
			}
		}
	}
}

/**
 * Called when a style reference is resolved successfully.  Applies the TabButtonCaptionStyle and TabButtonBackgroundStyle
 * to the tab buttons.
 *
 * @param	ResolvedStyle			the style resolved by the style reference
 * @param	StyleProperty			the name of the style reference property that was resolved.
 * @param	ArrayIndex				the array index of the style reference that was resolved.  should only be >0 for style reference arrays.
 * @param	bInvalidateStyleData	if TRUE, the resolved style is different than the style that was previously resolved by this style reference.
 */
void UUITabControl::OnStyleResolved( UUIStyle* ResolvedStyle, const FStyleReferenceId& StylePropertyId, INT ArrayIndex, UBOOL bInvalidateStyleData )
{
	Super::OnStyleResolved(ResolvedStyle,StylePropertyId,ArrayIndex,bInvalidateStyleData);

	FString StylePropertyName = StylePropertyId.GetStyleReferenceName();
	if ( StylePropertyName == TEXT("TabButtonBackgroundStyle") || StylePropertyName == TEXT("TabButtonCaptionStyle") )
	{
		for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
		{
			UUITabPage* Page = Pages(PageIndex);
			if ( Page != NULL && Page->TabButton != NULL )
			{
				Page->TabButton->SetWidgetStyle(ResolvedStyle, StylePropertyId, ArrayIndex);
			}
		}
	}
}

/* === UUIScreenObject interface === */
/**
 * Perform all initialization for this widget. Called on all widgets when a scene is opened,
 * once the scene has been completely initialized.
 * For widgets added at runtime, called after the widget has been inserted into its parent's
 * list of children.
 *
 * @param	inOwnerScene	the scene to add this widget to.
 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
 *							is being added to the scene's list of children.
 */
void UUITabControl::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner/*=NULL*/ )
{
	// Because we manage the styles for the tab buttons, and the buttons' styles are stored in their components (which are style subscribers),
	// we need to initialize the StyleSubscribers arrays for the buttons prior to calling Super::Initialize() so that the first time
	// OnResolveStyles is called, the buttons will be ready to receive those styles.
	for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
	{
		UUITabPage* Page = Pages(PageIndex);
		if ( Page != NULL && Page->TabButton != NULL )
		{
			Page->TabButton->InitializeStyleSubscribers();
		}
	}

	Super::Initialize(inOwnerScene, inOwner);
}

/**
 * Generates a array of UI Action keys that this widget supports.
 *
 * @param	out_KeyNames	Storage for the list of supported keynames.
 */
void UUITabControl::GetSupportedUIActionKeyNames( TArray<FName>& out_KeyNames )
{
	Super::GetSupportedUIActionKeyNames(out_KeyNames);

	out_KeyNames.AddUniqueItem(UIKEY_NextPage);
	out_KeyNames.AddUniqueItem(UIKEY_PreviousPage);
}

/**
 * Activates the focused state for this widget and sets it to be the focused control of its parent (if applicable)
 *
 * @param	Sender		Control that called SetFocus.  Possible values are:
 *						-	if NULL is specified, it indicates that this is the first step in a focus change.  The widget will
 *							attempt to set focus to its most eligible child widget.  If there are no eligible child widgets, this
 *							widget will enter the focused state and start propagating the focus chain back up through the Owner chain
 *							by calling SetFocus on its Owner widget.
 *						-	if Sender is the widget's owner, it indicates that we are in the middle of a focus change.  Everything else
 *							proceeds the same as if the value for Sender was NULL.
 *						-	if Sender is a child of this widget, it indicates that focus has been successfully changed, and the focus is now being
 *							propagated upwards.  This widget will now enter the focused state and continue propagating the focus chain upwards through
 *							the owner chain.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated the focus change.
 */
UBOOL UUITabControl::SetFocus( UUIScreenObject* Sender, INT PlayerIndex/*=0*/ )
{
	UBOOL bResult = FALSE;

	UUITabButton* TargetedTab = FindTargetedTab(PlayerIndex);
	if ( bAllowPagePreviews && TargetedTab != NULL && IsVisible() && CanAcceptFocus(PlayerIndex) && (Sender == NULL || Sender == GetParent()) )
	{
		if ( TargetedTab == GetLastFocusedControl(FALSE, PlayerIndex) )
		{
			// when returning focus to a tab control that has a targeted tab, we always want the tab page's first control to receive focus,
			// not whichever control was last focused (the default behavior)
			bResult = TargetedTab->FocusFirstControl(this, PlayerIndex);
		}
		else
		{
			GainFocus(NULL, PlayerIndex);

			// if we currently have a targeted tab, we won't propagate focus anywhere; we'll just make ourselves the focused control
			// so that left/right will switch which tab is targeted
			eventActivatePage(TargetedTab->TabPage, PlayerIndex, FALSE);
			bResult = TRUE;
		}
	}
	else
	{
		bResult = Super::SetFocus(Sender, PlayerIndex);
	}

	return bResult;
}

/**
 * Sets focus to the first focus target within this container.
 *
 * @param	Sender	the widget that generated the focus change.  if NULL, this widget generated the focus change.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated the focus change.
 *
 * @return	TRUE if focus was successfully propagated to the first focus target within this container.
 */
UBOOL UUITabControl::FocusFirstControl( UUIScreenObject* Sender, INT PlayerIndex/*=0*/ )
{
	UBOOL bResult = FALSE;

	UUITabButton* TargetedTab = FindTargetedTab(PlayerIndex);
	if ( bAllowPagePreviews && TargetedTab != NULL )
	{
		if ( TargetedTab == GetLastFocusedControl(FALSE, PlayerIndex) )
		{
			// when returning focus to a tab control that has a targeted tab, we always want the tab page's first control to receive focus,
			// not whichever control was last focused (the default behavior)
			TargetedTab->DeactivateStateByClass(UUIState_TargetedTab::StaticClass(), PlayerIndex);
			bResult = TargetedTab->FocusFirstControl(this, PlayerIndex);
		}
		else
		{
			GainFocus(NULL, PlayerIndex);

			// if we currently have a targeted tab, we won't propagate focus anywhere; we'll just make ourselves the focused control
			// so that left/right will switch which tab is targeted
			eventActivatePage(TargetedTab->TabPage, PlayerIndex, FALSE);
			bResult = TRUE;
		}
	}
	else if ( ActivePage != NULL )
	{
		bResult = ActivePage->FocusFirstControl(NULL, PlayerIndex);
	}
	else
	{
		bResult = Super::FocusFirstControl(Sender, PlayerIndex);
	}

	return bResult;
}

/**
 * Sets focus to the last focus target within this container.
 *
 * @param	Sender			the widget that generated the focus change.  if NULL, this widget generated the focus change.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated the focus change.
 *
 * @return	TRUE if focus was successfully propagated to the last focus target within this container.
 */
UBOOL UUITabControl::FocusLastControl( UUIScreenObject* Sender, INT PlayerIndex/*=0*/ )
{
	UBOOL bResult = FALSE;

	UUITabButton* TargetedTab = FindTargetedTab(PlayerIndex);
	if ( bAllowPagePreviews && TargetedTab != NULL )
	{
		if ( TargetedTab == GetLastFocusedControl(FALSE, PlayerIndex) )
		{
			// when returning focus to a tab control that has a targeted tab, we always want the tab page's first control to receive focus,
			// not whichever control was last focused (the default behavior)
			TargetedTab->DeactivateStateByClass(UUIState_TargetedTab::StaticClass(), PlayerIndex);
			bResult = TargetedTab->FocusLastControl(this, PlayerIndex);
		}
		else
		{
			GainFocus(NULL, PlayerIndex);

			// if we currently have a targeted tab, we won't propagate focus anywhere; we'll just make ourselves the focused control
			// so that left/right will switch which tab is targeted
			eventActivatePage(TargetedTab->TabPage, PlayerIndex, FALSE);
			bResult = TRUE;
		}
	}
	else if ( ActivePage != NULL )
	{
		bResult = ActivePage->FocusLastControl(NULL, PlayerIndex);
	}
	else
	{
		bResult = Super::FocusLastControl(Sender, PlayerIndex);
	}

	return bResult;
}

/**
 * Sets focus to the next control in the tab order (relative to Sender) for widget.  If Sender is the last control in
 * the tab order, propagates the call upwards to this widget's parent widget.
 *
 * @param	Sender			the widget to use as the base for determining which control to focus next
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated the focus change.
 *
 * @return	TRUE if we successfully set focus to the next control in tab order.  FALSE if Sender was the last eligible
 *			child of this widget or we couldn't otherwise set focus to another control.
 */
UBOOL UUITabControl::NextControl( UUIScreenObject* Sender, INT PlayerIndex/*=0*/ )
{
	//tracef(TEXT("UUITabControl::NextControl  Sender:%s  Focused:%s"), *Sender->GetName(), *GetFocusedControl(PlayerIndex)->GetName());
	UBOOL bResult = FALSE;

	UUITabButton* TabButtonSender = Cast<UUITabButton>(Sender);

	// if the sender is one of this tab control's tab buttons, it means that the currently focused control
	// is the last control in the currently active page and the focus chain is attempting to set focus to the
	// next page's first control.  We don't allow this because in the tab control, navigation between pages can
	// only happen when ActivatePage is called.  Instead, what we do is make the tab control itself the overall
	// focused control so that the user can use the arrow keys to move between tab buttons.
	if ( TabButtonSender == NULL || TabButtonSender->GetOwner() != this )
	{
		bResult = Super::NextControl(Sender, PlayerIndex);
	}
	else
	{
		if ( !bAllowPagePreviews )
		{
			bResult = Super::NextControl(this, PlayerIndex);
		}
		else
		{
			GainFocus(NULL, PlayerIndex);

			// make the tab button the targeted tab button
			TabButtonSender->ActivateStateByClass(UUIState_TargetedTab::StaticClass(), PlayerIndex);
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Sets focus to the previous control in the tab order (relative to Sender) for widget.  If Sender is the first control in
 * the tab order, propagates the call upwards to this widget's parent widget.
 *
 * @param	Sender			the widget to use as the base for determining which control to focus next
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated the focus change.
 *
 * @return	TRUE if we successfully set focus to the previous control in tab order.  FALSE if Sender was the first eligible
 *			child of this widget or we couldn't otherwise set focus to another control.
 */
UBOOL UUITabControl::PrevControl( UUIScreenObject* Sender, INT PlayerIndex/*=0*/ )
{
	//tracef(TEXT("UUITabControl::PrevControl  Sender:%s  Focused:%s"), *Sender->GetName(), *GetFocusedControl(PlayerIndex)->GetName());
	UBOOL bResult = FALSE;

	UUITabButton* TabButtonSender = Cast<UUITabButton>(Sender);

	// if the sender is one of this tab control's tab buttons, it means that the currently focused control
	// is the first control in the currently active page and the focus chain is attempting to set focus to the
	// previous page's last control.  We don't allow this because in the tab control, navigation between pages can
	// only happen when ActivatePage is called.  Instead, what we do is make the tab control itself the overall
	// focused control so that the user can use the arrow keys to move between tab buttons.
	if ( TabButtonSender == NULL || TabButtonSender->GetOwner() != this )
	{
		bResult = Super::PrevControl(Sender, PlayerIndex);
	}
	else
	{
		if ( !bAllowPagePreviews )
		{
			bResult = Super::PrevControl(this, PlayerIndex);
		}
		else
		{
			GainFocus(NULL, PlayerIndex);

			// make the tab button the targeted tab button
			TabButtonSender->ActivateStateByClass(UUIState_TargetedTab::StaticClass(), PlayerIndex);
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Sets focus to the child widget that is next in the specified direction in the navigation network within this widget.
 *
 * @param	Sender		Control that called NavigateFocus.  Possible values are:
 *						-	if NULL is specified, it indicates that this is the first step in a focus change.  The widget will
 *							attempt to set focus to its most eligible child widget.  If there are no eligible child widgets, this
 *							widget will enter the focused state and start propagating the focus chain back up through the Owner chain
 *							by calling SetFocus on its Owner widget.
 *						-	if Sender is the widget's owner, it indicates that we are in the middle of a focus change.  Everything else
 *							proceeds the same as if the value for Sender was NULL.
 *						-	if Sender is a child of this widget, it indicates that focus has been successfully changed, and the focus is now being
 *							propagated upwards.  This widget will now enter the focused state and continue propagating the focus chain upwards through
 *							the owner chain.
 * @param	Direction 		the direction to navigate focus.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player to set focus for.
 * @param	bFocusChanged	TRUE if the focus was changed
 *
 * @return	TRUE if the navigation event was handled successfully.
 */
UBOOL UUITabControl::NavigateFocus( UUIScreenObject* Sender, BYTE Direction, INT PlayerIndex/*=0*/, BYTE* bFocusChanged/*=NULL*/ )
{
	//tracef(TEXT("UUITabControl::NavigateFocus  Sender:%s  Direction:%s  Focused:%s"), *Sender->GetName(), *GetDockFaceText(Direction), *GetFocusedControl(PlayerIndex)->GetName());
	UBOOL bResult = FALSE;

	UUITabButton* TabButtonSender = Cast<UUITabButton>(Sender);

	// if the sender is one of this tab control's tab buttons, it means that the currently focused control
	// is the first or last control in the currently active page and the focus chain is attempting to set focus to the
	// the nearest sibling of that tab button.  We don't allow this because in the tab control, navigation between pages can
	// only happen when ActivatePage is called.  Instead, what we do is make the tab control itself the overall
	// focused control so that the user can use the arrow keys to move between tab buttons.
	if ( TabButtonSender == NULL || TabButtonSender->GetOwner() != this )
	{
		bResult = Super::NavigateFocus(Sender, Direction, PlayerIndex, bFocusChanged);
	}
	else
	{
		if ( !bAllowPagePreviews )
		{
			bResult = Super::NavigateFocus(this, Direction, PlayerIndex, bFocusChanged);
		}
		else
		{
			GainFocus(NULL, PlayerIndex);

			// make the tab button the targeted tab button
			TabButtonSender->ActivateStateByClass(UUIState_TargetedTab::StaticClass(), PlayerIndex);
			bResult = TRUE;
		}
	}

	return bResult;
}


/**
 * Called when the scene receives a notification that the viewport has been resized.  Propagated down to all children.
 *
 * This version requests a layout update on the tab control.
 *
 * @param	OldViewportSize		the previous size of the viewport
 * @param	NewViewportSize		the new size of the viewport
 */
void UUITabControl::NotifyResolutionChanged( const FVector2D& OldViewportSize, const FVector2D& NewViewportSize )
{
	RequestLayoutUpdate();

	Super::NotifyResolutionChanged(OldViewportSize, NewViewportSize);
}

/**
 * Handles input events for this widget.
 *
 * @param	EventParms		the parameters for the input event
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UUITabControl::ProcessInputKey( const FSubscribedInputEventParameters& EventParms )
{
	UBOOL bResult = FALSE;
	if ( EventParms.InputAliasName == UIKEY_NextPage || EventParms.InputAliasName == UIKEY_PreviousPage )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
			if ( EventParms.InputAliasName == UIKEY_NextPage )
			{
				eventActivateNextPage(EventParms.PlayerIndex);
			}
			else
			{
				eventActivatePreviousPage(EventParms.PlayerIndex);
			}
		}

		bResult = TRUE;
	}

	// Make sure to call the superclass's implementation after trying to consume input ourselves so that
	// we can respond to events defined in the super's class.
	bResult = bResult || Super::ProcessInputKey(EventParms);
	return bResult;
}

/* === UObject interface === */
/**
 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
 */
void UUITabControl::PreEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PreEditChange(PropertyThatChanged);
}

/**
 * Called when a property value from a member struct or array has been changed in the editor.
 */
void UUITabControl::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PostEditChange(PropertyThatChanged);

	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* ModifiedMemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( ModifiedMemberProperty != NULL )
		{
			FName PropertyName = ModifiedMemberProperty->GetFName();
			if ( PropertyName == TEXT("TabDockFace") || PropertyName == TEXT("Pages") )
			{
				RequestLayoutUpdate();
			}
			else if ( PropertyName == TEXT("TabSizeMode") || PropertyName == TEXT("TabButtonSize") || PropertyName == TEXT("TabButtonPadding") )
			{
				RequestLayoutUpdate();

				// if the user changed the way tabs are autosized, we'll need to reapply formatting to the tab's captions
				RequestFormattingUpdate();
			}
		}
	}
}

/* ==========================================================================================================
	UITabButton
========================================================================================================== */
/* === UUIScreenObject interface === */
/**
 * Called when this widget is created.  Copies the style data from the owning tab control into this button's
 * string and image rendering components, then calls InitializeStyleSubscribers.  This is necessary because tab control
 * manages the styles for tab buttons - initialization of the style data is handled by the tab control for existing tab
 * buttons, but for new tab buttons being added to the tab control we need to perform this step ourselves.
 */
void UUITabButton::Created( UUIScreenObject* Creator )
{
	Super::Created(Creator);

	UUITabControl* TabControlCreator = Cast<UUITabControl>(Creator);
	if ( TabControlCreator != NULL )
	{
		// first, initialize the list of style subscribers so that SetWidgetStyle finds the correct resolver
		InitializeStyleSubscribers();

		// next, copy the style data from the tab control into the string and image rendering components.
		if ( BackgroundImageComponent != NULL )
		{
			// the style reference may be NULL if it has never been resolved - this might happen when instancing a
			// UIPrefab which contains a tab control and several existing buttons.
			UUIStyle* BackgroundStyle = TabControlCreator->TabButtonBackgroundStyle.GetResolvedStyle();
			if ( BackgroundStyle != NULL )
			{
				FStyleReferenceId BackgroundStyleId(TEXT("TabButtonBackgroundStyle"), FindFieldWithFlag<UProperty, CASTCLASS_UProperty>(BackgroundImageComponent->GetClass(), TEXT("ImageStyle")));
				SetWidgetStyle(BackgroundStyle, BackgroundStyleId);
			}
		}

		// now the caption rendering component.
		if ( StringRenderComponent != NULL )
		{
			UUIStyle* CaptionStyle = TabControlCreator->TabButtonCaptionStyle.GetResolvedStyle();
			if ( CaptionStyle != NULL )
			{
				FStyleReferenceId CaptionStyleId(TEXT("TabButtonCaptionStyle"), FindFieldWithFlag<UProperty, CASTCLASS_UProperty>(StringRenderComponent->GetClass(), TEXT("StringStyle")));
				SetWidgetStyle(CaptionStyle, CaptionStyleId);
			}
		}
	}
}

/**
 * Determines whether this page can be activated.  Calls the IsActivationAllowed delegate to provide other objects
 * a chance to veto the activation of this button.
 *
 * Child classes which override this method should call Super::CanActivateButton() FIRST and only check additional
 * conditions if the return value is true.
 *
 * @param	PlayerIndex	the index [into the Engine.GamePlayers array] for the player that wishes to activate this page.
 *
 * @return	TRUE if this button is allowed to become the active tab button.
 */
UBOOL UUITabButton::CanActivateButton( INT PlayerIndex )
{
	UBOOL bResult = FALSE;

	if ( GIsGame && IsEnabled(PlayerIndex) && TabPage != NULL )
	{
		if ( DELEGATE_IS_SET(IsActivationAllowed) )
		{
			bResult = delegateIsActivationAllowed(this, PlayerIndex);
		}
		else
		{
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Returns TRUE if this widget has a UIState_TargetedTab object in its StateStack
 *
 * @param	StateIndex	if specified, will be set to the index of the last state in the list of active states that
 *						has the class specified
 */
UBOOL UUITabButton::IsTargeted( INT PlayerIndex/*=0*/, INT* StateIndex/*=NULL*/ ) const
{
	return HasActiveStateOfClass(UUIState_TargetedTab::StaticClass(),PlayerIndex,StateIndex);
}
void UUITabButton::execIsTargeted( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(PlayerIndex,GetBestPlayerIndex());
	P_GET_INT_OPTX_REF(StateIndex,0);
	P_FINISH;
	*(UBOOL*)Result=IsTargeted(PlayerIndex,pStateIndex);
}

/**
 * Handles input events for this button.
 *
 * This version ignores input if the tab button's owner doesn't allow targetting mode.
 *
 * @param	EventParms		the parameters for the input event
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UUITabButton::ProcessInputKey( const struct FSubscribedInputEventParameters& EventParms )
{
	UUITabControl* OwnerTabControl = Cast<UUITabControl>(GetOwner());
	if ( OwnerTabControl != NULL )
	{
		// @todo: Maybe use a different bool here instead of page previewing?
		if(OwnerTabControl->bAllowPagePreviews==FALSE && EventParms.InputAliasName==UIKEY_Clicked)
		{
			return FALSE;
		}
	}

	return Super::ProcessInputKey(EventParms);
}

/* ==========================================================================================================
	UITabPage
========================================================================================================== */
/**
 * Returns the tab control that contains this tab page, or NULL if it's not part of a tab control.
 */
UUITabControl* UUITabPage::GetOwnerTabControl() const
{
	UUITabControl* Result=NULL;

	// TabPage's Owner should be the TabButton.  TabButton's Owner should be the TabControl, so we should be able
	// to find it simply by iterating up the owner chain
	for ( UUIObject* NextOwner = GetOwner(); NextOwner && Result == NULL; NextOwner = NextOwner->GetOwner() )
	{
		Result = Cast<UUITabControl>(NextOwner);
	}
	if ( Result == NULL )
	{
		// but for some reason that didn't work - try the same thing but start with our tabButton (in case the TabButton isn't
		// our Owner for some reason)
		if ( TabButton != NULL && TabButton != GetOwner() )
		{
			for ( UUIObject* NextOwner = TabButton->GetOwner(); NextOwner && Result == NULL; NextOwner = NextOwner->GetOwner() )
			{
				Result = Cast<UUITabControl>(NextOwner);
			}
		}

		if ( Result == NULL && GetOuter() != GetOwner() )
		{
			// OK, last ditch effort.  a widget's Owner is always the Outer as well, so now we iterate up the Outer chain
			for ( UUIObject* NextOwner = Cast<UUIObject>(GetOuter()); NextOwner && Result == NULL; NextOwner = Cast<UUIObject>(NextOwner->GetOuter()) )
			{
				Result = Cast<UUITabControl>(NextOwner);
			}
		}
	}
	return Result;
}

/* === UIScreenObject interface === */
/**
 * Called when this widget is created.
 */
void UUITabPage::Created( UUIScreenObject* Creator )
{
	UUITabButton* TabButtonCreator = Cast<UUITabButton>(Creator);
	if ( TabButtonCreator != NULL )
	{
		//@fixme ronp - verify that we don't have a TabButton at this point!  we shouldn't, since it's transient now
		TabButton = TabButtonCreator;
	}

	Super::Created(Creator);
}

/**
 * Perform all initialization for this widget. Called on all widgets when a scene is opened,
 * once the scene has been completely initialized.
 * For widgets added at runtime, called after the widget has been inserted into its parent's
 * list of children.
 *
 * @param	inOwnerScene	the scene to add this widget to.
 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
 *							is being added to the scene's list of children.
 */
void UUITabPage::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner/*=NULL*/ )
{
	UUITabButton* TabButtonOwner = Cast<UUITabButton>(inOwner);
	if ( TabButtonOwner != NULL )
	{
		//@fixme ronp - verify that we don't have a TabButton at this point!  we shouldn't, since it's transient now
		TabButton = TabButtonOwner;
	}

	// Since UITabPage used to be a child of UIScrollFrame, we need to clear the NotifyPositionChanged delegate for
	// any children that still have that delegate pointing to OnChildRepositioned.  OnChildRepositioned is a method in
	// UIScrollFrame that doesn't exist in UIPanel
	TArray<UUIObject*> TabPageChildren;
	GetChildren(TabPageChildren, TRUE);

	for ( INT ChildIndex = 0; ChildIndex < TabPageChildren.Num(); ChildIndex++ )
	{
		UUIObject* Child = TabPageChildren(ChildIndex);
		if ( OBJ_DELEGATE_IS_SET(Child,NotifyPositionChanged) )
		{
			FScriptDelegate& Delegate = Child->__NotifyPositionChanged__Delegate;
			if ( Delegate.FunctionName == TEXT("OnChildRepositioned") && Delegate.Object == this )
			{
				Delegate.FunctionName = NAME_None;
				Delegate.Object = NULL;
			}
		}
	}

	Super::Initialize(inOwnerScene, inOwner);
}

/**
 * Sets the data store binding for this object to the text specified.
 *
 * @param	MarkupText			a markup string which resolves to data exposed by a data store.  The expected format is:
 *								<DataStoreTag:DataFieldTag>
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 */
void UUITabPage::SetDataStoreBinding( const FString& MarkupText, INT BindingIndex/*=INDEX_NONE*/ )
{
	switch( BindingIndex )
	{
	case UCONST_TOOLTIP_BINDING_INDEX:
		if ( TabButton != NULL )
		{
			TabButton->SetDataStoreBinding(MarkupText, BindingIndex);
		}
		break;

	case UCONST_TABPAGE_DESCRIPTION_DATABINDING_INDEX:
		if ( appStrcmp(*PageDescription.MarkupString,*MarkupText) )
		{
			Modify();
			PageDescription.MarkupString = MarkupText;
		}
		break;

	case UCONST_TABPAGE_CAPTION_DATABINDING_INDEX:
	case INDEX_NONE:
		if ( TabButton != NULL )
		{
			TabButton->SetDataStoreBinding(MarkupText, INDEX_NONE);
		}

		if ( appStrcmp(*ButtonCaption.MarkupString,*MarkupText) )
		{
			Modify();
			ButtonCaption.MarkupString = MarkupText;
		}
	    break;

	default:
		if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
		{
			SetDefaultDataBinding(MarkupText, BindingIndex);
		}
	}

	RefreshSubscriberValue(BindingIndex);
}

/**
 * Retrieves the markup string corresponding to the data store that this object is bound to.
 *
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	a datastore markup string which resolves to the datastore field that this object is bound to, in the format:
 *			<DataStoreTag:DataFieldTag>
 */
FString UUITabPage::GetDataStoreBinding(INT BindingIndex/*=INDEX_NONE*/) const
{
	FString Result;

	switch( BindingIndex )
	{
	case UCONST_TABPAGE_DESCRIPTION_DATABINDING_INDEX:
		Result = PageDescription.MarkupString;
	    break;

	case UCONST_TABPAGE_CAPTION_DATABINDING_INDEX:
	case INDEX_NONE:
		Result = ButtonCaption.MarkupString;
	    break;

	default:
		if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
		{
			Result = GetDefaultDataBinding(BindingIndex);
		}
		break;
	}

	return Result;
}

/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
UBOOL UUITabPage::RefreshSubscriberValue(INT BindingIndex/*=INDEX_NONE*/)
{
	UBOOL bResult = FALSE;

	if ( DELEGATE_IS_SET(OnRefreshSubscriberValue) && delegateOnRefreshSubscriberValue(this, BindingIndex) )
	{
		bResult = TRUE;
	}
	else if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		bResult = ResolveDefaultDataBinding(BindingIndex);
	}
	else if ( TabButton != NULL )
	{
		TabButton->SetDataStoreBinding(ButtonCaption.MarkupString, INDEX_NONE);
		bResult = TRUE;
	}
	
	// nothing to do yet...
	return bResult;
}

/**
 * Handler for the UIDataStore.OnDataStoreValueUpdated delegate.  Used by data stores to indicate that some data provided by the data
 * has changed.  Subscribers should use this function to refresh any data store values being displayed with the updated value.
 * notify subscribers when they should refresh their values from this data store.
 *
 * @param	SourceDataStore		the data store that generated the refresh notification; useful for subscribers with multiple data store
 *								bindings, to tell which data store sent the notification.
 * @param	PropertyTag			the tag associated with the data field that was updated; Subscribers can use this tag to determine whether
 *								there is any need to refresh their data values.
 * @param	SourceProvider		for data stores which contain nested providers, the provider that contains the data which changed.
 * @param	ArrayIndex			for collection fields, indicates which element was changed.  value of INDEX_NONE indicates not an array
 *								or that the entire array was updated.
 */
void UUITabPage::NotifyDataStoreValueUpdated( UUIDataStore* SourceDataStore, UBOOL bValuesInvalidated, FName PropertyTag, UUIDataProvider* SourceProvider, INT ArrayIndex )
{
	//@fixme - for now, just pass through to RefreshSubscriberValue
	RefreshSubscriberValue();
}

/**
 * Retrieves the list of data stores bound by this subscriber.
 *
 * @param	out_BoundDataStores		receives the array of data stores that subscriber is bound to.
 */
void UUITabPage::GetBoundDataStores(TArray<UUIDataStore*>& out_BoundDataStores)
{
	GetDefaultDataStores(out_BoundDataStores);
	if ( ButtonCaption )
	{
		out_BoundDataStores.AddUniqueItem(*ButtonCaption);
	}
	if ( ButtonToolTip )
	{
		out_BoundDataStores.AddUniqueItem(*ButtonToolTip);
	}
	if ( PageDescription )
	{
		out_BoundDataStores.AddUniqueItem(*PageDescription);
	}
}

/**
 * Notifies this subscriber to unbind itself from all bound data stores
 */
void UUITabPage::ClearBoundDataStores()
{
	TMultiMap<FName,FUIDataStoreBinding*> DataBindingMap;
	GetDataBindings(DataBindingMap);

	TArray<FUIDataStoreBinding*> DataBindings;
	DataBindingMap.GenerateValueArray(DataBindings);
	for ( INT BindingIndex = 0; BindingIndex < DataBindings.Num(); BindingIndex++ )
	{
		FUIDataStoreBinding* Binding = DataBindings(BindingIndex);
		Binding->ClearDataBinding();
	}

	TArray<UUIDataStore*> DataStores;
	GetBoundDataStores(DataStores);

	for ( INT DataStoreIndex = 0; DataStoreIndex < DataStores.Num(); DataStoreIndex++ )
	{
		UUIDataStore* DataStore = DataStores(DataStoreIndex);
		DataStore->eventSubscriberDetached(this);
	}
}

/* ==========================================================================================================
	UUIScrollFrame
========================================================================================================== */
/* === UUIScrollFrame interface === */
/**
 * Changes the background image for this slider, creating the wrapper UITexture if necessary.
 *
 * @param	NewBarImage		the new surface to use for the slider's background image
 */
void UUIScrollFrame::SetBackgroundImage( USurface* NewBackgroundImage )
{
	if ( StaticBackgroundImage != NULL )
	{
		StaticBackgroundImage->SetImage(NewBackgroundImage);
	}
}

/**
 * Sets the flag indicating that the client region needs to be recalculated and triggers a scene update.
 *
 * @param	bImmediately	specify TRUE to immediately call CalculateClientRegion instead of triggering a scene update.
 */
void UUIScrollFrame::ReapplyFormatting( UBOOL bImmediately/*=FALSE*/ )
{
	bRecalculateClientRegion = TRUE;
	if ( bImmediately )
	{
		CalculateClientRegion();
		RefreshScrollbars(TRUE);
	}
	else
	{
		RequestSceneUpdate(FALSE, TRUE);
	}
}

/**
 * Determines the size of the region necessary to contain all children of this widget, and resizes this
 * UIScrollClientPanel accordingly.
 *
 * @return	the size of the client region (in pixels) after calculation.
 */
void UUIScrollFrame::CalculateClientRegion( FVector2D* RegionSize/*=NULL*/ )
{
	FLOAT FrameX, FrameX2, FrameY, FrameY2;
	GetPositionExtents(FrameX, FrameX2, FrameY, FrameY2, TRUE);

	const FLOAT ClientRegionOffsetX = HorizontalClientRegion.GetValue(this) * ClientRegionPosition.X;
	const FLOAT ClientRegionOffsetY = VerticalClientRegion.GetValue(this) * ClientRegionPosition.Y;

	FLOAT MinX=FrameX, MaxX=FrameX2, MinY=FrameY, MaxY=FrameY2;
	FLOAT SizeX = 0.f, SizeY = 0.f;

	// we don't want to count the scrollbars.
	TArray<UUIObject*> ExclusionSet;
	ExclusionSet.AddItem(ScrollbarHorizontal);
	ExclusionSet.AddItem(ScrollbarVertical);

	// since children don't necessarily have to be inside the bounds of their parents, we'll need to consider all children inside this container
	TArray<UUIObject*> AllChildren;
	GetChildren(AllChildren, TRUE, &ExclusionSet);

	if ( AllChildren.Num() > 0 )
	{
		for ( INT ChildIndex = 0; ChildIndex < AllChildren.Num(); ChildIndex++ )
		{
			UUIObject* Child = AllChildren(ChildIndex);

			// find the extrema bounds across all children of this 
			FLOAT ChildMinX, ChildMaxX, ChildMinY, ChildMaxY;
			Child->GetPositionExtents(ChildMinX, ChildMaxX, ChildMinY, ChildMaxY, TRUE);

			MinX = Min(MinX, ChildMinX);
			MaxX = Max(MaxX, ChildMaxX);
			MinY = Min(MinY, ChildMinY);
			MaxY = Max(MaxY, ChildMaxY);
		}

		SizeX = MaxX - MinX;
		SizeY = MaxY - MinY;
	}

	// update the size of the client region.
	HorizontalClientRegion.SetValue(this, SizeX);
	VerticalClientRegion.SetValue(this, SizeY);

	// though the client region doesn't move, the value of ClientRegionPosition has probably changed
	// since it is the ratio of the size of the region to the offset from the frame
	ClientRegionPosition.X = SizeX != 0 ? ClientRegionOffsetX / SizeX : 0.f;
	ClientRegionPosition.Y = SizeY != 0 ? ClientRegionOffsetY / SizeY : 0.f;

	// Reset the scrollbars.
	RefreshScrollbars();

	// Resolved...
 	bRecalculateClientRegion = FALSE;
	if ( RegionSize != NULL )
	{
		RegionSize->X = SizeX;
		RegionSize->Y = SizeY;
	}
}

#define VERT_SCROLLBAR_ALWAYS_VISIBLE 1

/**
 * Sets the flag indicating that the scrollbars need to be re-resolved.
 */
void UUIScrollFrame::RefreshScrollbars( UBOOL bImmediately/*=FALSE*/ )
{
	bRefreshScrollbars = TRUE;
	if ( bImmediately && HasResolvedAllFaces() )
	{
		ResolveScrollbars();
	}
}

/**
 * Enables and initializes the scrollbars that need to be visible
 */
void UUIScrollFrame::ResolveScrollbars()
{
	const FLOAT FrameX = GetPosition(UIFACE_Left, EVALPOS_PixelViewport);
	const FLOAT FrameY = GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
	const FLOAT ClientX = GetClientRegionPosition(UIORIENT_Horizontal);
	const FLOAT ClientY = GetClientRegionPosition(UIORIENT_Vertical);
	const FLOAT ClientExtentX = HorizontalClientRegion.GetValue(this);
	const FLOAT ClientExtentY = VerticalClientRegion.GetValue(this);

	const UBOOL bHorzVisible = ClientExtentX > GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport);
	const UBOOL bVertVisible = ClientExtentY > GetBounds(UIORIENT_Vertical, EVALPOS_PixelViewport);

	ScrollbarHorizontal->eventSetVisibility(bHorzVisible);
#if VERT_SCROLLBAR_ALWAYS_VISIBLE
	ScrollbarVertical->SetEnabled(bVertVisible, GetPlayerOwnerIndex());
	ScrollbarHorizontal->EnableCornerPadding(TRUE);
#else
	ScrollbarVertical->eventSetVisibility(bVertVisible);

 	ScrollbarHorizontal->EnableCornerPadding(bVertVisible);
#endif
	ScrollbarVertical->EnableCornerPadding(bHorzVisible);

	const FLOAT HorizontalBarWidth	= bHorzVisible ? ScrollbarHorizontal->GetScrollZoneWidth() : 0.f;
#if VERT_SCROLLBAR_ALWAYS_VISIBLE
	const FLOAT VerticalBarWidth	= ScrollbarVertical->GetScrollZoneWidth();
#else
 	const FLOAT VerticalBarWidth	= bVertVisible ? ScrollbarVertical->GetScrollZoneWidth() : 0.f;
#endif

	const FLOAT FrameExtentX = GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport) - VerticalBarWidth;
	const FLOAT FrameExtentY = GetBounds(UIORIENT_Vertical, EVALPOS_PixelViewport) - HorizontalBarWidth;

	const FLOAT MaxHiddenRegionX = ClientExtentX - FrameExtentX;
	ScrollbarHorizontal->SetMarkerSize( ClientExtentX != 0 ? Max(1.f, FrameExtentX / ClientExtentX) : 1.f );
	ScrollbarHorizontal->SetMarkerPosition( MaxHiddenRegionX != 0 ? (FrameX - ClientX) / MaxHiddenRegionX : 0.f );

	if ( bHorzVisible )
	{
		// now set the NudgeMultiplier such that the number of clicks it takes to go from one side to the other is proportional
		// to the size of the "hidden" region
		const FLOAT VisibilityRatio = MaxHiddenRegionX / FrameExtentX;
		ScrollbarHorizontal->NudgeMultiplier = Clamp(MaxHiddenRegionX * VisibilityRatio, 5.f, 50.f);
	}
	
	const FLOAT MaxHiddenRegionY = ClientExtentY - FrameExtentY;
	ScrollbarVertical->SetMarkerSize( ClientExtentY != 0 ? Min(1.f, FrameExtentY / ClientExtentY) : 1.f );
	ScrollbarVertical->SetMarkerPosition( MaxHiddenRegionY != 0 ? (FrameY - ClientY) / MaxHiddenRegionY : 0.f );
#if !VERT_SCROLLBAR_ALWAYS_VISIBLE
	if ( bVertVisible )
#endif
	{
		// now set the NudgeMultiplier such that the number of clicks it takes to go from one side to the other is proportional
		// to the size of the "hidden" region
		const FLOAT VisibilityRatio = FrameExtentY != 0 ? MaxHiddenRegionY / FrameExtentY : 0.f;

		ScrollbarVertical->NudgeMultiplier = Clamp(MaxHiddenRegionY * VisibilityRatio, 5.f, 50.f);
		ScrollbarVertical->RefreshFormatting(FALSE);
	}

	bRefreshScrollbars = FALSE;
}

/**
 * Scrolls all of the child widgets by the specified amount in the specified direction.
 *
 * @param	Sender			the scrollbar that generated the event.
 * @param	PositionChange	indicates the amount that the scrollbar has travelled.
 * @param	bPositionMaxed	indicates that the scrollbar's marker has reached its farthest available position,
 *                          used to achieve pixel exact scrolling
 */
UBOOL UUIScrollFrame::ScrollRegion( UUIScrollbar* Sender, FLOAT PositionChange, UBOOL bPositionMaxed/*=FALSE*/ )
{
	UBOOL bResult = FALSE;
	
	if ( Sender != NULL )
	{
		if ( Sender == ScrollbarHorizontal )
		{
			if ( !bPositionMaxed )
			{
				const FLOAT PixelChange = PositionChange * ScrollbarHorizontal->GetNudgeValue();
				const FLOAT ScrollRegionExtent = ScrollbarHorizontal->GetScrollZoneExtent();
				const FLOAT PercentChange = ScrollRegionExtent != 0.f ? PixelChange / ScrollRegionExtent : 0.f;

				ClientRegionPosition.X = Clamp(ClientRegionPosition.X + PercentChange, 0.f, 1.f);
			}
			else if ( PositionChange < 0 )
			{
				ClientRegionPosition.X = 0.f;
			}
			else
			{
				const FLOAT FrameExtentX = GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport) - (ScrollbarVertical->IsVisible() ? ScrollbarVertical->GetScrollZoneWidth() : 0.f);
				const FLOAT ClientRegionExtent = HorizontalClientRegion.GetValue(this);
				const FLOAT MaxHiddenPixels = ClientRegionExtent - FrameExtentX;

				ClientRegionPosition.X = ClientRegionExtent != 0 ? MaxHiddenPixels / ClientRegionExtent : 0.f;
			}
			bResult = TRUE;
		}
		else if ( Sender == ScrollbarVertical )
		{
			if ( !bPositionMaxed )
			{
#if 0
				const FLOAT ClientRegionExtent = VerticalClientRegion.GetValue(this);
				const FLOAT PixelChange = PositionChange * ScrollbarVertical->GetNudgeValue();
				const FLOAT CurrentPosition = ClientRegionExtent * ClientRegionPosition.Y;
				const FLOAT NewPosition = CurrentPosition + PixelChange;
				ClientRegionPosition.Y = ClientRegionExtent != 0 ? NewPosition / ClientRegionExtent : 0.f;
#else
				const FLOAT PixelChange = PositionChange * ScrollbarVertical->GetNudgeValue();
				const FLOAT ScrollRegionExtent = ScrollbarVertical->GetScrollZoneExtent();
				const FLOAT PercentChange = ScrollRegionExtent != 0.f ? PixelChange / ScrollRegionExtent : 0.f;

				ClientRegionPosition.Y = Clamp(ClientRegionPosition.Y + PercentChange, 0.f, 1.f);
#endif
			}
			else if ( PositionChange < 0 )
			{
				ClientRegionPosition.Y = 0.f;
			}
			else
			{
				const FLOAT FrameExtentY = GetBounds(UIORIENT_Vertical, EVALPOS_PixelViewport) - (ScrollbarHorizontal->IsVisible() ? ScrollbarHorizontal->GetScrollZoneWidth() : 0.f);
				const FLOAT ClientRegionExtent = VerticalClientRegion.GetValue(this);
				const FLOAT MaxHiddenPixels = ClientRegionExtent - FrameExtentY;

				ClientRegionPosition.Y = ClientRegionExtent != 0 ? MaxHiddenPixels / ClientRegionExtent : 0.f;
			}

			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Changes the position of the client region and synchronizes the scrollbars to the new position.
 *
 * @param	Orientation		specify UIORIENT_Horizontal to set the position of the left side; specify
 *							UIORIENT_Vertical to set the position of the top side.
 * @param	NewPosition		the position to move the client region to, in pixels.
 *
 * @return	TRUE if the client region was moved successfully.
 */
UBOOL UUIScrollFrame::SetClientRegionPosition( /*EUIOrientation*/BYTE Orientation, FLOAT NewPosition )
{
	UBOOL bResult = FALSE;

	if ( Orientation < UIORIENT_MAX )
	{
		FLOAT MinX=0.f, MinY=0.f, MaxX=0.f, MaxY=0.f;
		GetClipRegion(MinX, MinY, MaxX, MaxY);

		const FVector2D VisibleRegionPos(MinX, MinY);
		const FVector2D VisibleRegionSize(MaxX - MinX, MaxY - MinY);

		const FLOAT ClientRegionSize = GetClientRegionSize(Orientation);
		const FLOAT PositionOffset = NewPosition - VisibleRegionPos[Orientation];

		NewPosition = ClientRegionSize != 0 ? Clamp(PositionOffset, VisibleRegionSize[Orientation] - ClientRegionSize, 0.f) / -ClientRegionSize : 0;
		if ( Abs(NewPosition - ClientRegionPosition[Orientation]) > DELTA )
		{
			ClientRegionPosition[Orientation] = NewPosition;
			RefreshScrollbars(TRUE);
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Changes the position of the client region and synchronizes the scrollbars to the new position.
 *
 * @param	NewPosition		the position to move the client region to, in pixels.
 *
 * @return	TRUE if the client region was moved successfully.
 */
UBOOL UUIScrollFrame::SetClientRegionPositionVector( FVector2D NewPosition )
{
	UBOOL bResult = FALSE;

	FLOAT MinX=0.f, MinY=0.f, MaxX=0.f, MaxY=0.f;
	GetClipRegion(MinX, MinY, MaxX, MaxY);

	const FVector2D VisibleRegionSize(MaxX - MinX, MaxY - MinY);
	const FVector2D ClientRegionSize(GetClientRegionSizeVector());
	const FVector2D MinValue(VisibleRegionSize - ClientRegionSize);

	// normalize the new position; if the client region is smaller than the visible region for either orientation, 
	// negate the value for that orientation
	if ( ClientRegionSize.X <= VisibleRegionSize.X )
	{
		NewPosition.X = 0.f;
	}
	if ( ClientRegionSize.Y <= VisibleRegionSize.Y )
	{
		NewPosition.Y = 0.f;
	}

	NewPosition.X = Clamp(NewPosition.X, MinValue.X, 0.f) / -ClientRegionSize.X;
	NewPosition.Y = Clamp(NewPosition.Y, MinValue.Y, 0.f) / -ClientRegionSize.Y;
	if ( !ClientRegionPosition.Equals(NewPosition, DELTA) )
	{
		ClientRegionPosition = NewPosition;
		RefreshScrollbars(TRUE);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Gets the position of either the left or top side of the client region.
 *
 * @param	Orientation		specify UIORIENT_Horizontal to retrieve the position of the left side; specify
 *							UIORIENT_Vertical to retrieve the position of the top side.
 *
 * @return	the position of the client region, in canvas coordinates relative to 0,0.
 */
FLOAT UUIScrollFrame::GetClientRegionPosition( /*EUIOrientation*/BYTE Orientation ) const
{
	FLOAT Result = 0.f;
	if ( Orientation < UIORIENT_MAX )
	{
		if ( Orientation == UIORIENT_Horizontal )
		{
			const FLOAT FrameX = GetPosition(UIFACE_Left, EVALPOS_PixelViewport);
			const FLOAT OffsetX = HorizontalClientRegion.GetValue(this) * ClientRegionPosition.X;
			Result = FrameX - OffsetX;
		}
		else if ( Orientation == UIORIENT_Vertical )
		{
			const FLOAT FrameY = GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
			const FLOAT OffsetY = VerticalClientRegion.GetValue(this) * ClientRegionPosition.Y;
			Result = FrameY - OffsetY;
		}
	}

	return Result;
}

/**
 * Returns the size of a single orientation of the client region, in pixels.
 *
 * @param	Orientation		specify UIORIENT_Horizontal to retrieve the width of the client region or UIORIENT_Vertical
 *							to get the height of the client region.
 *
 * @return	the width or height of the client region, in pixels.
 */
FLOAT UUIScrollFrame::GetClientRegionSize( /*EUIOrientation*/BYTE Orientation ) const
{
	FLOAT Result=0.f;
	if ( Orientation < UIORIENT_MAX )
	{
		if ( Orientation == UIORIENT_Horizontal )
		{
			Result = HorizontalClientRegion.GetValue(this);
		}
		else if ( Orientation == UIORIENT_Vertical )
		{
			Result = VerticalClientRegion.GetValue(this);
		}
	}
	return Result;
}

/**
 * Gets the position of the upper-left corner of the client region.
 *
 * @return	the position of the client region, in canvas coordinates relative to 0,0.
 */
FVector2D UUIScrollFrame::GetClientRegionPositionVector() const
{
	const FLOAT FrameX = GetPosition(UIFACE_Left, EVALPOS_PixelViewport);
	const FLOAT FrameY = GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
	const FLOAT OffsetX = HorizontalClientRegion.GetValue(this) * ClientRegionPosition.X;
	const FLOAT OffsetY = VerticalClientRegion.GetValue(this) * ClientRegionPosition.Y;
	return FVector2D(
		FrameX - OffsetX,
		FrameY - OffsetY
		);
}

/**
 * Returns the size of the client region, in pixels.
 */
FVector2D UUIScrollFrame::GetClientRegionSizeVector() const
{
	return FVector2D(HorizontalClientRegion.GetValue(this), VerticalClientRegion.GetValue(this));
}

/**
 * Returns a vector containing the size of the region (in pixels) available for rendering inside this scrollframe,
 * taking account whether the scrollbars are visible.
 */
void UUIScrollFrame::GetClipRegion( FLOAT& MinX, FLOAT& MinY, FLOAT& MaxX, FLOAT& MaxY ) const
{
	GetPositionExtents(MinX, MaxX, MinY, MaxY, FALSE);
#if VERT_SCROLLBAR_ALWAYS_VISIBLE
	if ( ScrollbarVertical != NULL )
#else
	if ( ScrollbarVertical != NULL && ScrollbarVertical->IsVisible() )
#endif
	{
		MaxX -= ScrollbarVertical->GetScrollZoneWidth();
	}

	if ( ScrollbarHorizontal != NULL && ScrollbarHorizontal->IsVisible() )
	{
		MaxY -= ScrollbarHorizontal->GetScrollZoneWidth();
	}
}

/**
 * Returns the percentage of the client region that is visible within the scrollframe's bounds.
 *
 * @param	Orientation		specifies whether to return the vertical or horizontal percentage.
 *
 * @return	a value from 0.0 to 1.0 representing the percentage of the client region that can be visible at once.
 */
FLOAT UUIScrollFrame::GetVisibleRegionPercentage( /*EUIOrientation*/BYTE Orientation ) const
{
	FLOAT Result=0.f;
	if ( Orientation < UIORIENT_MAX )
	{
		const FLOAT FrameExtentX = GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport);
		const FLOAT FrameExtentY = GetBounds(UIORIENT_Vertical, EVALPOS_PixelViewport);
		const FLOAT ClientExtentX = HorizontalClientRegion.GetValue(this);
		const FLOAT ClientExtentY = VerticalClientRegion.GetValue(this);
		const FLOAT HorizontalBarWidth	= ClientExtentX > FrameExtentX ? ScrollbarHorizontal->GetScrollZoneWidth() : 0.f;
		const FLOAT VerticalBarWidth	= ClientExtentY > FrameExtentY ? ScrollbarVertical->GetScrollZoneWidth() : 0.f;

		if ( Orientation == UIORIENT_Horizontal )
		{
			if ( ClientExtentX != 0 )
			{
				Result = (FrameExtentX - VerticalBarWidth) / ClientExtentX;
			}
		}
		else if ( Orientation == UIORIENT_Vertical )
		{
			if ( ClientExtentY != 0 )
			{
				Result = (FrameExtentY - HorizontalBarWidth) / ClientExtentY;
			}
		}
	}
	return Result;
}

/* === UIObject interface === */
/**
 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
 *
 * This version adds the BackgroundImageComponent (if non-NULL) to the StyleSubscribers array.
 */
void UUIScrollFrame::InitializeStyleSubscribers()
{
	Super::InitializeStyleSubscribers();

	VALIDATE_COMPONENT(StaticBackgroundImage);
	AddStyleSubscriber(StaticBackgroundImage);
}

/**
 * Evalutes the Position value for the specified face into an actual pixel value.  Should only be
 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
 *
 * @param	Face	the face that should be resolved
 */
void UUIScrollFrame::ResolveFacePosition( EUIWidgetFace Face )
{
	if ( bRecalculateClientRegion && GetNumResolvedFaces() == 0 )
	{
		CalculateClientRegion();
	}

	Super::ResolveFacePosition(Face);

	if ( HasResolvedAllFaces() && bRefreshScrollbars )
	{
 		ResolveScrollbars();
	}
}

/* === UUIScreenObject interface === */
/**
 * Initializes the buttons and creates the background image.
 *
 * @param	inOwnerScene	the scene to add this widget to.
 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
 *							is being added to the scene's list of children.
 */
void UUIScrollFrame::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner/*=NULL*/ )
{
	Super::Initialize( inOwnerScene, inOwner );

	ValidateScrollbars();
}

#define FIX_CORRUPTED_SCROLLBARS 0
/**
 * Ensures that this scrollframe has valid scrollbars and that the scrollbars' states are correct (i.e. correct
 * Outer, ObjectArchetype, part of the Children array, etc.)
 */
void UUIScrollFrame::ValidateScrollbars()
{
#if FIX_CORRUPTED_SCROLLBARS
	ScrollbarVertical = ScrollbarHorizontal = NULL;
#endif

	TArray<UUIScrollbar*> ScrollbarChildren;
	if ( ContainsObjectOfClass<UUIObject>(Children, UUIScrollbar::StaticClass(), FALSE, (TArray<UUIObject*>*)&ScrollbarChildren) )
	{
		for ( INT ScrollbarIndex = 0; ScrollbarIndex < ScrollbarChildren.Num(); ScrollbarIndex++ )
		{
			UUIScrollbar* Scrollbar = ScrollbarChildren(ScrollbarIndex);
			if ( Scrollbar->GetOuter() == this )
			{
				if ( Scrollbar->ScrollbarOrientation == UIORIENT_Horizontal )
				{
#if FIX_CORRUPTED_SCROLLBARS
					if ( ScrollbarHorizontal != NULL && Scrollbar != ScrollbarHorizontal )
					{
						RemoveChild(Scrollbar);
						continue;
					}
#endif
					ScrollbarHorizontal = Scrollbar;
				}

				else if ( Scrollbar->ScrollbarOrientation == UIORIENT_Vertical )
				{
#if FIX_CORRUPTED_SCROLLBARS
					if ( ScrollbarVertical != NULL && Scrollbar != ScrollbarVertical )
					{
						RemoveChild(Scrollbar);
						continue;
					}
#endif
					ScrollbarVertical = Scrollbar;
				}
			}
		}
	}
	
	UUIScrollFrame* ScrollFrameArch = GetArchetype<UUIScrollFrame>();
	UBOOL bCreateScrollbarVert = ScrollbarVertical == NULL || ScrollbarVertical->GetOuter() != this;
	if ( !bCreateScrollbarVert && ScrollbarVertical->GetArchetype() != ScrollFrameArch->ScrollbarVertical )
	{
		// remove the old scrollbar
		RemoveChild(ScrollbarVertical);
		// move it into the transient package so that we have no conflicts
		ScrollbarVertical->Rename(NULL, GetTransientPackage(), REN_ForceNoResetLoaders);
		ScrollbarVertical = NULL;
		// create a new one
		bCreateScrollbarVert = TRUE;
	}
	UBOOL bCreateScrollbarHorz = ScrollbarHorizontal == NULL || ScrollbarHorizontal->GetOuter() != this;
	if ( !bCreateScrollbarHorz && ScrollbarHorizontal->GetArchetype() != ScrollFrameArch->ScrollbarHorizontal )
	{
		// remove the old scrollbar
		RemoveChild(ScrollbarHorizontal);
		// move it into the transient package so that we have no conflicts
		ScrollbarHorizontal->Rename(NULL, GetTransientPackage(), REN_ForceNoResetLoaders);
		ScrollbarHorizontal = NULL;
		// create a new one
		bCreateScrollbarHorz = TRUE;
	}

	{
		// create internal controls
		if ( bCreateScrollbarVert )
		{
			ScrollbarVertical = Cast<UUIScrollbar>(CreateWidget(this, ScrollFrameArch->ScrollbarVertical->GetClass(), ScrollFrameArch->ScrollbarVertical));
		}
		InsertChild( ScrollbarVertical );

		if ( bCreateScrollbarHorz )
		{
			ScrollbarHorizontal = Cast<UUIScrollbar>(CreateWidget(this, ScrollFrameArch->ScrollbarHorizontal->GetClass(), ScrollFrameArch->ScrollbarHorizontal));
		}
		InsertChild( ScrollbarHorizontal );
	}

#if VERT_SCROLLBAR_ALWAYS_VISIBLE
	// ensure that the vertical scrollbar is always visible
	if ( ScrollbarVertical != NULL )
	{
		ScrollbarVertical->eventSetVisibility(TRUE);
	}
#endif
}

/**
 * Generates a array of UI Action keys that this widget supports.
 *
 * @param	out_KeyNames	Storage for the list of supported keynames.
 */
void UUIScrollFrame::GetSupportedUIActionKeyNames(TArray<FName> &out_KeyNames )
{
	Super::GetSupportedUIActionKeyNames(out_KeyNames);

	out_KeyNames.AddUniqueItem(UIKEY_ScrollUp);
	out_KeyNames.AddUniqueItem(UIKEY_ScrollDown);
	out_KeyNames.AddUniqueItem(UIKEY_ScrollLeft);
	out_KeyNames.AddUniqueItem(UIKEY_ScrollRight);
	out_KeyNames.AddUniqueItem(UIKEY_ScrollTop);
	out_KeyNames.AddUniqueItem(UIKEY_ScrollBottom);
	out_KeyNames.AddUniqueItem(UIKEY_PageUp);
	out_KeyNames.AddUniqueItem(UIKEY_PageDown);

}

/**
 * Handles input events for this scrollframe widget.
 *
 * @param	EventParms		the parameters for the input event
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UUIScrollFrame::ProcessInputKey( const struct FSubscribedInputEventParameters& EventParms )
{
	UBOOL bResult = FALSE;

	if ( EventParms.InputAliasName == UIKEY_ScrollLeft )
	{
		switch ( EventParms.EventType )
		{
		case IE_Pressed:
		case IE_Repeat:
		case IE_DoubleClick:
			if ( ScrollbarHorizontal != NULL )
			{
				ScrollbarHorizontal->ScrollDecrement(ScrollbarHorizontal, EventParms.PlayerIndex);
			}
			break;
		}

		// swallow this input event so that the user doesn't accidentally switch focus on the PC
		bResult = TRUE;
	}
	else if ( EventParms.InputAliasName == UIKEY_ScrollRight )
	{
		switch ( EventParms.EventType )
		{
		case IE_Pressed:
		case IE_Repeat:
		case IE_DoubleClick:
			if ( ScrollbarHorizontal != NULL )
			{
				ScrollbarHorizontal->ScrollIncrement(ScrollbarHorizontal, EventParms.PlayerIndex);
			}
			break;
		}

		// swallow this input event so that the user doesn't accidentally switch focus on the PC
		bResult = TRUE;
	}
	else if ( EventParms.InputAliasName == UIKEY_ScrollUp )
	{
		switch ( EventParms.EventType )
		{
		case IE_Pressed:
		case IE_Repeat:
		case IE_DoubleClick:
			if ( ScrollbarVertical != NULL )
			{
				ScrollbarVertical->ScrollDecrement(ScrollbarVertical, EventParms.PlayerIndex);
// 				PlayUISound(DecrementIndexCue,EventParms.PlayerIndex);
			}
			break;
		}

		// swallow this input event so that the user doesn't accidentally switch focus on the PC
		bResult = TRUE;
	}
	else if ( EventParms.InputAliasName == UIKEY_ScrollDown )
	{
		switch ( EventParms.EventType )
		{
		case IE_Pressed:
		case IE_Repeat:
		case IE_DoubleClick:
			if ( ScrollbarVertical != NULL )
			{
				ScrollbarVertical->ScrollIncrement(ScrollbarVertical, EventParms.PlayerIndex);
			}
			break;
		}

		// swallow this input event so that the user doesn't accidentally switch focus on the PC
		bResult = TRUE;
	}
	else if ( EventParms.InputAliasName == UIKEY_PageUp )
	{
		switch ( EventParms.EventType )
		{
		case IE_Pressed:
		case IE_Repeat:
		case IE_DoubleClick:
			if ( ScrollbarVertical != NULL )
			{
				const FLOAT MarkerPosition = ScrollbarVertical->GetMarkerPosPercent();
				eventScrollZoneClicked(ScrollbarVertical, MarkerPosition - 1, EventParms.PlayerIndex);

				//@todo - play sound?
			}
			break;
		}

		// swallow IE_Released
		bResult = TRUE;
	}
	else if ( EventParms.InputAliasName == UIKEY_PageDown )
	{
		switch ( EventParms.EventType )
		{
		case IE_Pressed:
		case IE_Repeat:
		case IE_DoubleClick:
			if ( ScrollbarVertical != NULL )
			{
				const FLOAT MarkerPosition = ScrollbarVertical->GetMarkerPosPercent();
				eventScrollZoneClicked(ScrollbarVertical, MarkerPosition + 1, EventParms.PlayerIndex);

				//@todo - play sound?
			}
			break;
		}

		// swallow IE_Released
		bResult = TRUE;
	}
	else if ( EventParms.InputAliasName == UIKEY_ScrollTop )
	{
		switch ( EventParms.EventType )
		{
		case IE_Pressed:
		case IE_Repeat:
		case IE_DoubleClick:
			SetClientRegionPosition(UIORIENT_Vertical, 0);
			break;
		}

		// swallow IE_Released
		bResult = TRUE;
	}
	else if ( EventParms.InputAliasName == UIKEY_ScrollBottom )
	{
		switch ( EventParms.EventType )
		{
		case IE_Pressed:
		case IE_Repeat:
		case IE_DoubleClick:
			if ( ScrollbarVertical != NULL )
			{
				FLOAT MinX=0.f, MinY=0.f, MaxX=0.f, MaxY=0.f;
				GetClipRegion(MinX, MinY, MaxX, MaxY);
				SetClientRegionPosition(UIORIENT_Vertical, MaxY);

				//@todo - play sound?
			}
			break;
		}

		// swallow IE_Released
		bResult = TRUE;
	}

	// Make sure to call the superclass's implementation after trying to consume input ourselves so that
	// we can respond to events defined in the super's class.
	bResult = bResult || Super::ProcessInputKey(EventParms);
	return bResult;
}

/**
 * Render this scroll frame.
 *
 * @param	Canvas	the FCanvas to use for rendering this widget
 */
void UUIScrollFrame::Render_Widget( FCanvas* Canvas )
{
	if ( StaticBackgroundImage != NULL )
	{
		FRenderParameters Parameters(
			RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top],
			RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left],
			RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top],
			NULL, GetViewportHeight()
		);

		StaticBackgroundImage->RenderComponent( Canvas, Parameters );
	}
}
/**
 * Routes rendering calls to children of this screen object.
 *
 * This version sets a clip mask on the canvas while the children are being rendered.
 *
 * @param	Canvas	the canvas to use for rendering
 * @param	UIPostProcessGroup	Group determines current pp pass that is being rendered
 */
void UUIScrollFrame::Render_Children( FCanvas* Canvas, EUIPostProcessGroup UIPostProcessGroup )
{
	UUIScene* OwnerScene = GetScene();
	const UBOOL bRotationSupported = OwnerScene->bSupportsRotation;

	// store the current global alph modulation
	const FLOAT CurrentAlphaModulation = Canvas->AlphaModulate;

	// if we're focused, we'll need to render any focused children last so that they always render on top.
	// the easiest way to do this is to build a list of Children array indexes that we'll render - indexes for
	// focused children go at the end; indexes for non-focused children go just before the index of the first focused child.
	TArray<UUIObject*> RenderList = Children;
	{
		for ( INT PlayerIndex = 0; PlayerIndex < FocusControls.Num(); PlayerIndex++ )
		{
			UUIObject* FocusedPlayerControl = FocusControls(PlayerIndex).GetFocusedControl();
			if ( FocusedPlayerControl != NULL )
			{
				INT Idx = RenderList.FindItemIndex(FocusedPlayerControl);
				if ( Idx != INDEX_NONE )
				{
					RenderList.Remove(Idx);
				}
				RenderList.AddItem(FocusedPlayerControl);
			}
		}

		RenderList.RemoveItem(ScrollbarVertical);
		RenderList.RemoveItem(ScrollbarHorizontal);
	}

	GetClipRegion(FrameBounds[UIFACE_Left], FrameBounds[UIFACE_Top], FrameBounds[UIFACE_Right], FrameBounds[UIFACE_Bottom]);

	// set the clip mask on the canvas
	Canvas->PushMaskRegion(
		FrameBounds[UIFACE_Left],
		FrameBounds[UIFACE_Top],
		Abs(FrameBounds[UIFACE_Right] - FrameBounds[UIFACE_Left]),
		Abs(FrameBounds[UIFACE_Bottom] - FrameBounds[UIFACE_Top])
		);

	const FVector2D ClientRegionPos(
		HorizontalClientRegion.GetValue(this) * ClientRegionPosition.X,
		VerticalClientRegion.GetValue(this) * ClientRegionPosition.Y
		);
	FTranslationMatrix ClientRegionTransform(-FVector(ClientRegionPos,0));
	for ( INT i = 0; i < RenderList.Num(); i++ )
	{
		UUIObject* Child = RenderList(i);

		// apply the widget's rotation
		if ( Child->IsVisible() )
		{
			UBOOL bRenderChild=TRUE;

			UBOOL bPopTranform = FALSE;
			if ( Child != ScrollbarHorizontal && Child != ScrollbarVertical )
			{
				// get the extents for this child widget
				FLOAT MinX=0.f, MaxX=0.f, MinY=0.f, MaxY=0.f;
				Child->GetPositionExtents(MinX, MaxX, MinY, MaxY, TRUE);

				// don't render this child if it's completely outside this panel's visible area
				if (MinX - DELTA <= FrameBounds[UIFACE_Right] + DELTA && MinY - DELTA <= FrameBounds[UIFACE_Bottom] + DELTA
				&&	MaxX + DELTA >= FrameBounds[UIFACE_Left] - DELTA && MaxY + DELTA >= FrameBounds[UIFACE_Top] - DELTA )
				{
					if ( bRotationSupported )
					{
						// apply the widget's transform matrix combined with the scrollregion offset translation
						Canvas->PushRelativeTransform(ClientRegionTransform * Child->GenerateTransformMatrix(FALSE));
					}
					else
					{
						Canvas->PushRelativeTransform(ClientRegionTransform);
					}

					bPopTranform = TRUE;
				}
				else
				{
					// this widget is completely outside the client region - no need to render it at all
					bRenderChild = FALSE;
				}
			}
			else if ( bRotationSupported )
			{
				// apply the widget's transform matrix 
				Canvas->PushRelativeTransform(Child->GenerateTransformMatrix(FALSE));
				bPopTranform = TRUE;
			}

			if ( !bRenderChild )
			{
				continue;
			}

			// use the widget's ZDepth as the sorting key for the canvas
			Canvas->PushDepthSortKey(appCeil(Child->GetZDepth()));

			// add this widget to the scene's render stack
			OwnerScene->RenderStack.Push(Child);

			// now render the child
			Render_Child(Canvas, Child, UIPostProcessGroup);

			// restore the previous sort key
			Canvas->PopDepthSortKey();

			// restore the previous transform
			if ( bPopTranform )
			{
				Canvas->PopTransform();
			}
		}
	}

	// clear the clip mask
	Canvas->PopMaskRegion();

	// render scrollbars
	{
		if ( ScrollbarHorizontal != NULL && ScrollbarHorizontal->IsVisible() )
		{
			if ( bRotationSupported )
			{
				// apply the widget's transform matrix 
				Canvas->PushRelativeTransform(ScrollbarHorizontal->GenerateTransformMatrix(FALSE));			// use the widget's ZDepth as the sorting key for the canvas
			}
			Canvas->PushDepthSortKey(appCeil(ScrollbarHorizontal->GetZDepth()));

			// add this widget to the scene's render stack
			OwnerScene->RenderStack.Push(ScrollbarHorizontal);

			// now render the child
			Render_Child(Canvas, ScrollbarHorizontal, UIPostProcessGroup);

			// restore the previous sort key
			Canvas->PopDepthSortKey();
			if( bRotationSupported )
			{
				Canvas->PopTransform();
			}
		}
		if ( ScrollbarVertical != NULL && ScrollbarVertical->IsVisible() )
		{
			if ( bRotationSupported )
			{
				// apply the widget's transform matrix 
				Canvas->PushRelativeTransform(ScrollbarVertical->GenerateTransformMatrix(FALSE));			// use the widget's ZDepth as the sorting key for the canvas
			}
			// use the widget's ZDepth as the sorting key for the canvas
			Canvas->PushDepthSortKey(appCeil(ScrollbarVertical->GetZDepth()));

			// add this widget to the scene's render stack
			OwnerScene->RenderStack.Push(ScrollbarVertical);

			// now render the child
			Render_Child(Canvas, ScrollbarVertical, UIPostProcessGroup);

			// restore the previous sort key
			Canvas->PopDepthSortKey();
			if( bRotationSupported )
			{
				Canvas->PopTransform();
			}
		}
	}

	// restore the previous global fade value
	Canvas->AlphaModulate = CurrentAlphaModulation;
}

/**
 * Insert a widget as a child of this one.  This version routes the call to the ClientPanel if the widget is not
 * eligible to be a child of this scroll frame.
 *
 * @param	NewChild		the widget to insert
 * @param	InsertIndex		the position to insert the widget.  If not specified, the widget is insert at the end of
 *							the list
 * @param	bRenameExisting	controls what happens if there is another widget in this widget's Children list with the same tag as NewChild.
 *							if TRUE, renames the existing widget giving a unique transient name.
 *							if FALSE, does not add NewChild to the list and returns FALSE.
 *
 * @return	the position that that the child was inserted in, or INDEX_NONE if the widget was not inserted
 */
INT UUIScrollFrame::InsertChild( UUIObject* NewChild, INT InsertIndex/*=INDEX_NONE*/, UBOOL bRenameExisting/*=TRUE*/ )
{
	if ( NewChild == ScrollbarHorizontal || NewChild == ScrollbarVertical )
	{
		// Scrollbars should always be rendered last
		//@todo ronp zdepth support - might need to do additional work (such as ensuring that the scrollbars' ZDepth is always lower than any other
		// children in the scrollframe)
		InsertIndex = INDEX_NONE;
	}
	else
	{
		// all other children must be inserted into the Children array before the scrollbars
		InsertIndex = Max(0, Min(InsertIndex, Min(Children.FindItemIndex(ScrollbarHorizontal), Children.FindItemIndex(ScrollbarVertical))));
	}

	return Super::InsertChild(NewChild, InsertIndex, bRenameExisting);
}

/**
 * Called immediately after a child has been added to this screen object.
 *
 * @param	WidgetOwner		the screen object that the NewChild was added as a child for
 * @param	NewChild		the widget that was added
 */
void UUIScrollFrame::NotifyAddedChild( UUIScreenObject* WidgetOwner, UUIObject* NewChild )
{
	Super::NotifyAddedChild( WidgetOwner, NewChild );

	// ignore the adding of our own children
	if ( NewChild != ScrollbarVertical && NewChild != ScrollbarHorizontal )
	{
		// Recalculate the region extent since adding a widget could have affected it.
		ReapplyFormatting();
	}
}

/**
 * Called immediately after a child has been removed from this screen object.
 *
 * @param	WidgetOwner		the screen object that the widget was removed from.
 * @param	OldChild		the widget that was removed
 * @param	ExclusionSet	used to indicate that multiple widgets are being removed in one batch; useful for preventing references
 *							between the widgets being removed from being severed.
 */
void UUIScrollFrame::NotifyRemovedChild( UUIScreenObject* WidgetOwner, UUIObject* OldChild, TArray<UUIObject*>* ExclusionSet/*=NULL*/ )
{
	Super::NotifyRemovedChild( WidgetOwner, OldChild, ExclusionSet );

	if ( OldChild != ScrollbarVertical && OldChild != ScrollbarHorizontal )
	{
		// Recalculate the region extent since removing a widget could have affected it.
		ReapplyFormatting();
	}
}

/**
 * Called when a property is modified that could potentially affect the widget's position onscreen.
 */
void UUIScrollFrame::RefreshPosition()
{	
	RefreshScrollbars();

	Super::RefreshPosition();
}

/**
 * Called to globally update the formatting of all UIStrings.
 */
void UUIScrollFrame::RefreshFormatting( UBOOL bRequestSceneUpdate/*=TRUE*/ )
{
	Super::RefreshFormatting(bRequestSceneUpdate);

	// Recalculate the region extent since the movement of the frame could have affected it.
	bRefreshScrollbars = TRUE;
}

/**
 * Called when the scene receives a notification that the viewport has been resized.  Propagated down to all children.
 *
 * @param	OldViewportSize		the previous size of the viewport
 * @param	NewViewportSize		the new size of the viewport
 */
void UUIScrollFrame::NotifyResolutionChanged( const FVector2D& OldViewportSize, const FVector2D& NewViewportSize )
{
	RefreshFormatting();
	ReapplyFormatting();
	Super::NotifyResolutionChanged(OldViewportSize, NewViewportSize);
}

/**
 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
 */
void UUIScrollFrame::PreEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PreEditChange(PropertyThatChanged);

	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("StaticBackgroundImage") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty && StaticBackgroundImage != NULL )
				{
					// the user either cleared the value of the BackgroundImageComponent (which should never happen since
					// we use the 'noclear' keyword on the property declaration), or is assigning a new value to the BackgroundImageComponent.
					// Unsubscribe the current component from our list of style resolvers.
					RemoveStyleSubscriber(StaticBackgroundImage);
				}
			}
		}
	}
}

/**
 * Called when a property value has been changed in the editor.
 */
void UUIScrollFrame::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("StaticBackgroundImage") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty )
				{
					if ( StaticBackgroundImage != NULL )
					{
						UUIComp_DrawImage* ComponentTemplate = GetArchetype<UUIScrollFrame>()->StaticBackgroundImage;
						if ( ComponentTemplate != NULL )
						{
							StaticBackgroundImage->StyleResolverTag = ComponentTemplate->StyleResolverTag;
						}
						else
						{
							StaticBackgroundImage->StyleResolverTag = TEXT("Background Image Style");
						}

						// user created a new background image component - add it to the list of style subscribers
						AddStyleSubscriber(StaticBackgroundImage);

						// now initialize the component's image
						StaticBackgroundImage->SetImage(StaticBackgroundImage->GetImage());
					}
				}
				else if ( StaticBackgroundImage != NULL )
				{
					// a property of the ImageComponent was changed
					if ( ModifiedProperty->GetFName() == TEXT("ImageRef") && StaticBackgroundImage->GetImage() != NULL )
					{
#if 0
						USurface* CurrentValue = BackgroundImageComponent->GetImage();

						// changed the value of the image texture/material
						// clear the data store binding
						//@fixme ronp - do we always need to clear the data store binding?
 						SetDataStoreBinding(TEXT(""));

						// clearing the data store binding value may have cleared the value of the image component's texture,
						// so restore the value now
						SetImage(CurrentValue);
#endif
					}
				}
			}
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Called after duplication & serialization and before PostLoad.
 * This version fixes up the scrollbar references for UIScrollFrame archetypes.
 */
void UUIScrollFrame::PostDuplicate()
{
	Super::PostDuplicate();

	if ( IsAPrefabArchetype() )
	{
		// when creating a UIPrefab, Initialize() won't be called prior to the first instance of the UIPrefab being created
		// so we need to fixup the scrollbars before then
		ValidateScrollbars();
	}
}

/**
 * Called after this object has been completely de-serialized.  This version migrates values for the deprecated Background,
 * BackgroundCoordinates, and PrimaryStyle properties over to the BackgroundImageComponent.
 */
void UUIScrollFrame::PostLoad()
{
	Super::PostLoad();
}



// EOF







