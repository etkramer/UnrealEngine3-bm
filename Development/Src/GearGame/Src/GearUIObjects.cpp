/*=============================================================================
	GearUIObjects.cpp: Gears2 UI widget class implementations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/* ==========================================================================================================
	Includes
========================================================================================================== */
#include "GearGame.h"

#include "EngineUISequenceClasses.h"
#include "GearGameUIClasses.h"
#include "GearGameUIPrivateClasses.h"

IMPLEMENT_CLASS(UGearUICollapsingSelectionList);
IMPLEMENT_CLASS(UUIComp_CollapsingListPresenter);


// this define is just used to make sure that all of the code related to masks and tranforms for the selection list's animations is grouped
// together
#define ANIMATION_USES_CANVASMASK	1


/* ==========================================================================================================
	Statics
========================================================================================================== */



/* ==========================================================================================================
	UGearUICollapsingSelectionList
========================================================================================================== */
/* === UGearUICollapsingSelectionList interface === */
/**
 * Returns the height (in pixels) of the caption or value string (whichever is larger).
 */
FLOAT UGearUICollapsingSelectionList::GetMaxLabelHeight() const
{
	return Max<FLOAT>(GetCaptionLabelHeight(), GetValueLabelHeight());
}
/**
 * Returns the height (in pixels) of the caption string.
 */
FLOAT UGearUICollapsingSelectionList::GetCaptionLabelHeight() const
{
	FLOAT Result = 0.f;
	if ( CaptionComponent != NULL )
	{
		Result = CaptionComponent->GetSubregionSize(UIORIENT_Vertical, UIEXTENTEVAL_Pixels);
	}
	return Result;
}
/**
 * Returns the height (in pixels) of the value string.
 */
FLOAT UGearUICollapsingSelectionList::GetValueLabelHeight() const
{
	FLOAT Result = 0.f;
	if ( ValueComponent != NULL )
	{
		Result = ValueComponent->GetSubregionSize(UIORIENT_Vertical, UIEXTENTEVAL_Pixels);
	}
	return Result;
}

/**
 * Expands the list to display the available choices for the bound game setting.
 *
 * @return	TRUE if the list was able to begin expanding.  FALSE if the list is not in the collapsed state or wasn't allowed to expand.
 */
UBOOL UGearUICollapsingSelectionList::Expand()
{
	UBOOL bResult = FALSE;

	if ( !IsExpanded()
	&&	(!DELEGATE_IS_SET(IsExpandAllowed) || delegateIsExpandAllowed(this)) )
	{
		if ( GetItemCount() > 0 )
		{
			ListExpansionState = LES_Expanding;
			eventListExpansionBegin();
		}

		bResult = TRUE;
	}

	return bResult;
}

/**
 * Begins the process of expanding the list to allow its items to be seen.
 */
void UGearUICollapsingSelectionList::ListExpansionBegin()
{
	PlayUISound(ExpandCue, GetBestPlayerIndex());
	eventOnListExpanding();

	RefreshPosition();
}

/**
 * Called when the list has been fully expanded.
 */
void UGearUICollapsingSelectionList::ListExpansionComplete()
{
	// The following code would also need to occur each frame the expand animation is active
	ListExpansionState = LES_Expanded;
	if ( VerticalScrollbar != NULL && bEnableVerticalScrollbar )
	{
		VerticalScrollbar->Opacity = Opacity;
	}

	RefreshPosition();
	if ( VerticalScrollbar != NULL )
	{
		VerticalScrollbar->RefreshMarker();
	}

	if ( DELEGATE_IS_SET(OnListExpanded) )
	{
		delegateOnListExpanded(this);
	}
}

/**
 * Returns the list to the collapsed state so that it only displays the setting caption and current value.
 *
 * @return	TRUE if the list was able to begin collapsing.  FALSE if the list is not in the expanded state or wasn't allowed to collapse.
 */
UBOOL UGearUICollapsingSelectionList::Collapse()
{
	UBOOL bResult = FALSE;

	if ( !IsCollapsed()
	&&	(!DELEGATE_IS_SET(IsCollapseAllowed) || delegateIsCollapseAllowed(this)) )
	{
		// if we're being collapsed but the user didn't submit a selection (i.e. maybe the widget lost focus or whatever)
		// revert to the previous value
		if ( SubmittedIndex != Index || IsExpanding() )
		{
			SetIndex(SubmittedIndex);
			PlayUISound(CollapseCancelCue, GetBestPlayerIndex());
		}
		else
		{
			PlayUISound(CollapseAcceptCue, GetBestPlayerIndex());
		}

		if ( GetItemCount() > 0 )
		{
			if ( VerticalScrollbar != NULL )
			{
				VerticalScrollbar->Opacity = 0.f;
			}

			if ( ValueComponent != NULL )
			{
				FString NewValue = GetElementValue(Index);
				ValueComponent->SetValue(NewValue);
			}

			eventListCollapseBegin();
			ListExpansionState = LES_Collapsing;
		}

		bResult = TRUE;
	}

	return bResult;
}

/**
 * Begins the process of collapsing the list to allow only its current value to be seen.
 */
void UGearUICollapsingSelectionList::ListCollapseBegin()
{
	eventOnListCollapsing();
}

/**
 * Called when the list has been fully collapsed.
 */
void UGearUICollapsingSelectionList::ListCollapseComplete()
{
	ListExpansionState = LES_Collapsed;

	// The following code would also need to occur each frame the collapse animation is active
	RefreshFormatting();
	if ( VerticalScrollbar != NULL )
	{
		VerticalScrollbar->RefreshMarker();
	}

	// this would need to be called each frame we're animating.
	if ( DELEGATE_IS_SET(NotifyPositionChanged) )
	{
		delegateNotifyPositionChanged(this);
	}

	RequestSceneUpdate(FALSE, TRUE, TRUE);
	if ( DELEGATE_IS_SET(OnListCollapsed) )
	{
		delegateOnListCollapsed(this);
	}
}

/**
 * Sets the currently selected item according to the string associated with it.
 *
 * @param	StringToSelect	the string associated with the item that should become the selected item
 *
 * @return	TRUE if the value was changed successfully; FALSE if it wasn't found or was already selected.
 */
UBOOL UGearUICollapsingSelectionList::SetStringValue( const FString& StringToSelect )
{
	const UBOOL bResult = SetIndexValue(FindItemIndex(StringToSelect), FALSE);
	return bResult;
}

/**
 * Sets the list's index to the value specified and activates the appropriate notification events.
 *
 * @param	NewIndex			An index into the Items array that should become the new Index for the list.
 * @param	bClampValue			if TRUE, NewIndex will be clamped to a valid value in the range of 0 -> ItemCount - 1
 * @param	bSkipNotification	if TRUE, no events are generated as a result of updating the list's index.
 *
 * @return	TRUE if the list's Index was successfully changed.
 */
UBOOL UGearUICollapsingSelectionList::SetIndexValue(INT NewIndex,INT PlayerIndex,UBOOL bClampValue,UBOOL bSkipNotification)
{
	const UBOOL bResult = SetIndex(NewIndex, bClampValue, bSkipNotification);

	if ( bResult )
	{
		NotifySubmitSelection(PlayerIndex);
	}

	return bResult;
}

/* === UUIList interface === */
/**
 * Sets the list's index to the value specified and activates the appropriate notification events.
 *
 * @param	NewIndex			An index into the Items array that should become the new Index for the list.
 * @param	bClampValue			if TRUE, NewIndex will be clamped to a valid value in the range of 0 -> ItemCount - 1
 * @param	bSkipNotification	if TRUE, no events are generated as a result of updating the list's index.
 *
 * @return	TRUE if the list's Index was successfully changed.
 */
UBOOL UGearUICollapsingSelectionList::SetIndex( INT NewIndex, UBOOL bClampValue/*=TRUE*/, UBOOL bSkipNotification/*=FALSE*/ )
{
	const INT CurrentItemCount = GetItemCount();
	if ( bClampValue )
	{
		NewIndex = CurrentItemCount > 0
			? Clamp(NewIndex, 0, CurrentItemCount - 1)
			: INDEX_NONE;
	}
	else if ( !Items.IsValidIndex(NewIndex) )
	{
		// the only "valid" invalid index is -1, so if the new index is out of range, change it to INDEX_NONE
		NewIndex = INDEX_NONE;
	}

	if ( NewIndex != INDEX_NONE )
	{
		INT ModifiedIndex = NewIndex;
		while ( !CanSelectElement(ModifiedIndex) )
		{
			if ( ++ModifiedIndex >= CurrentItemCount )
			{
				ModifiedIndex = 0;
			}

			if ( ModifiedIndex == NewIndex )
			{
				// didn't find any selectable items
				break;
			}
		}

		NewIndex = ModifiedIndex;
	}

	UBOOL bResult = Super::SetIndex(NewIndex, bClampValue, bSkipNotification);
	return bResult;
}

/**
 * Called when the list's index has changed.
 *
 * @param	PreviousIndex	the list's Index before it was changed
 * @param	PlayerIndex		the index of the player associated with this index change.
 */
void UGearUICollapsingSelectionList::NotifyIndexChanged( INT PreviousIndex, INT PlayerIndex )
{
	if ( ValueComponent != NULL
	&&	(bIndexChangeUpdatesValue || (IsCollapsed() && !IsPressed(PlayerIndex))) )
	{
		FString NewValue = GetElementValue(Index);
		ValueComponent->SetValue(NewValue);
		ValueComponent->eventEnableAutoSizing(UIORIENT_Vertical, GIsGame || NewValue.Len() > 0);
	}

	Super::NotifyIndexChanged(PreviousIndex, PlayerIndex);
}

/**
 * Called whenever the user chooses an item while this list is focused.  Activates the SubmitSelection kismet event and calls
 * the OnSubmitSelection delegate.
 */
void UGearUICollapsingSelectionList::NotifySubmitSelection( INT PlayerIndex/*=INDEX_NONE*/ )
{
	if ( !DELEGATE_IS_SET(CanSubmitValue) || delegateCanSubmitValue(this, PlayerIndex) )
	{
		SubmittedIndex = Index;
		if (GIsGame && GetMaxVisibleElementCount() > 0 && Items.IsValidIndex(Index) && IsElementEnabled(Index)
		&&	!IsCollapsed() && IsFocused(PlayerIndex) )
		{
			if ( ValueComponent != NULL )
			{
				FString NewValue = GetElementValue(Index);
				ValueComponent->SetValue(NewValue);
				ValueComponent->eventEnableAutoSizing(UIORIENT_Vertical, GIsGame || NewValue.Len() > 0);
			}

			Collapse();
		}

		Super::NotifySubmitSelection(PlayerIndex);
	}
}
	
/**
 * Wrapper for calculating the amount of additional room the list needs at the top to render headers or other things.
 */
FLOAT UGearUICollapsingSelectionList::GetHeaderSize() const
{
	return Super::GetHeaderSize() + GetMaxLabelHeight();
}

/* === UUIObject interface === */
/**
 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
 *
 * This version adds the BackgroundImageComponent (if non-NULL) to the StyleSubscribers array.
 */
void UGearUICollapsingSelectionList::InitializeStyleSubscribers()
{
	Super::InitializeStyleSubscribers();

	VALIDATE_COMPONENT(CaptionComponent);
	VALIDATE_COMPONENT(ValueComponent);
	VALIDATE_COMPONENT(BackgroundImageComponent);
	VALIDATE_COMPONENT(FocusIndicator);

	AddStyleSubscriber(CaptionComponent);
	AddStyleSubscriber(ValueComponent);
	AddStyleSubscriber(BackgroundImageComponent);
	AddStyleSubscriber(FocusIndicator);
}

/**
 * Render this list.
 *
 * @param	Canvas	the canvas to use for rendering this widget
 */
void UGearUICollapsingSelectionList::Render_Widget( FCanvas* Canvas )
{
	if ( !IsCollapsed() )
	{
		const FLOAT CurrentAlpha = Canvas->AlphaModulate;

		UUIComp_CollapsingListPresenter* ListComponent = Cast<UUIComp_CollapsingListPresenter>(CellDataComponent);
		if ( ListComponent != NULL && IsTransitioning() )
		{
			// when an opacity animation is applied
			Canvas->AlphaModulate = ListComponent->AnimationOpacity;
		}

		Super::Render_Widget(Canvas);

		Canvas->AlphaModulate = CurrentAlpha;
	}

	if ( CaptionComponent != NULL )
	{
		CaptionComponent->Render_String(Canvas);
	}
	if ( ValueComponent != NULL )
	{
		ValueComponent->Render_String(Canvas);
	}
	if ( FocusIndicator != NULL )
	{
		FRenderParameters IndicatorParms(GetViewportHeight());

		const FLOAT FocusIndicatorPixelSize = FocusIndicatorSize.GetValue(this);
		IndicatorParms.DrawXL = IndicatorParms.DrawYL = FocusIndicatorPixelSize;
		IndicatorParms.DrawX = RenderBounds[UIFACE_Right] - FocusIndicatorPixelSize;

		const FLOAT CaptionHeight = GetMaxLabelHeight();
		IndicatorParms.DrawY = RenderBounds[UIFACE_Top] + (CaptionHeight * 0.5f - FocusIndicatorPixelSize * 0.5f);

		FocusIndicator->RenderComponent(Canvas, IndicatorParms);
	}

	if ( bDebugShowBounds )
	{
		// render a box which displays the size of the caption/value text
		const FLOAT CaptionWidth = CaptionSize.GetValue(this);
		const FLOAT ValueWidth = ValueSize.GetValue(this);

		// show the caption portion
		FVector2D StartLoc(RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top]);
		FVector2D EndLoc(RenderBounds[UIFACE_Left] + CaptionWidth, RenderBounds[UIFACE_Bottom]);
		if ( CaptionComponent != NULL )
		{
			EndLoc.Y = RenderBounds[UIFACE_Top] + CaptionComponent->GetSubregionSize(UIORIENT_Vertical, UIEXTENTEVAL_Pixels);
		}
		DrawBox2D(Canvas, StartLoc + FVector2D(1,1), EndLoc - FVector2D(1,1), FColor(0,255,64));	// draw an bright green box to show the caption area

		StartLoc.X = RenderBounds[UIFACE_Left] + CaptionWidth;
		EndLoc.X = RenderBounds[UIFACE_Right];
		if ( ValueComponent != NULL )
		{
			EndLoc.Y = RenderBounds[UIFACE_Top] + ValueComponent->GetSubregionSize(UIORIENT_Vertical, UIEXTENTEVAL_Pixels);
		}
		else
		{
			EndLoc.Y = RenderBounds[UIFACE_Bottom];
		}
		DrawBox2D(Canvas, StartLoc + FVector2D(1,1), EndLoc - FVector2D(1,1), FColor(0,128,192));	// draw an dull blue box to show the value area
	}
}

/**
 * Renders the list's background image, if assigned.
 */
void UGearUICollapsingSelectionList::RenderBackgroundImage( FCanvas* Canvas, const FRenderParameters& Parameters )
{
	if ( BackgroundImageComponent != NULL )
	{
		// make a copy so we can adjust the values
		FRenderParameters Parms = Parameters;

		// don't consider the header element spacing to be part of the "list", so don't render the background behind it.
		Parms.DrawY += HeaderElementSpacing.GetValue(this);

		// UITexture::Render_Texture expects the XL/YL to be a width/height value, not a coordinate
		Parms.DrawX = RenderBounds[UIFACE_Left];
		Parms.DrawXL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
		Parms.DrawYL -= Parms.DrawY;

		// now apply the additional padding provided for aligning background images that don't stretch properly
		Parms.DrawX += HorzBackgroundPadding[0].GetValue(this);
		Parms.DrawXL += HorzBackgroundPadding[1].GetValue(this);

		Parms.DrawY += VertBackgroundPadding[0].GetValue(this);
		Parms.DrawYL += VertBackgroundPadding[1].GetValue(this);

		const FLOAT CurrentAlpha = Canvas->AlphaModulate;
		if ( IsTransitioning() )
		{
			Canvas->AlphaModulate = Min(1.f, Canvas->AlphaModulate * 2);
		}
		BackgroundImageComponent->RenderComponent(Canvas, Parms);

		Canvas->AlphaModulate = CurrentAlpha;
		if ( bDebugShowBounds )
		{
			// show the caption portion
			FVector2D StartLoc(Parms.DrawX, Parms.DrawY);
			FVector2D EndLoc(Parms.DrawX + Parms.DrawXL, Parms.DrawY + Parms.DrawYL);
			DrawBox2D(Canvas, StartLoc + FVector2D(1,1), EndLoc - FVector2D(1,1), FColor(255,255,64));	// draw a yellow box to show the background image area
		}
	}
}

/**
 * Called immediately before and after the scene perform an update.  Only called if bEnableSceneUpdateNotifications
 * is set to TRUE on this widget.
 *
 * Default implementation simply calls the script delegates.
 */
void UGearUICollapsingSelectionList::PreSceneUpdate()
{
	Super::PreSceneUpdate();

	const FLOAT CaptionWidth = CaptionSize.GetValue(this);

	const FLOAT AvailableValuePixels = GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport) - CaptionSize.GetValue(this);
	const FLOAT ValueWidth = Min<FLOAT>(AvailableValuePixels, ValueSize.GetValue(this));
	if ( CaptionComponent != NULL && !CaptionComponent->IsAutoSizeEnabled(UIORIENT_Horizontal) )
	{
		CaptionComponent->ClampRegion[UIORIENT_Horizontal].bSubregionEnabled = TRUE;// EnableSubregion(UIORIENT_Horizontal, TRUE);  don't want to trigger another scene update
		CaptionComponent->SetSubregionOffset(UIORIENT_Horizontal, 0.f, UIEXTENTEVAL_Pixels);
		CaptionComponent->SetSubregionSize(UIORIENT_Horizontal, CaptionWidth, UIEXTENTEVAL_Pixels);

		CaptionComponent->ClampRegion[UIORIENT_Vertical].bSubregionEnabled = FALSE;
	}

	//@todo - if the caption is auto-sized, need to make sure we adjust the clamp region of the value string
	if ( ValueComponent != NULL )
	{
		ValueComponent->ClampRegion[UIORIENT_Horizontal].bSubregionEnabled = TRUE;// EnableSubregion(UIORIENT_Horizontal, TRUE);  don't want to trigger another scene update
		ValueComponent->SetSubregionOffset(UIORIENT_Horizontal, CaptionWidth, UIEXTENTEVAL_Pixels);
		ValueComponent->SetSubregionSize(UIORIENT_Horizontal, ValueWidth, UIEXTENTEVAL_Pixels);

		ValueComponent->ClampRegion[UIORIENT_Vertical].bSubregionEnabled = FALSE;
	}
}
void UGearUICollapsingSelectionList::PostSceneUpdate()
{
	if ( CaptionComponent != NULL && ValueComponent != NULL )
	{
		const FLOAT CaptionHeight = CaptionComponent->GetSubregionSize(UIORIENT_Vertical, UIEXTENTEVAL_Pixels);
		const FLOAT ValueHeight = ValueComponent->GetSubregionSize(UIORIENT_Vertical, UIEXTENTEVAL_Pixels);
		const FLOAT MaxHeight = Max<FLOAT>(CaptionHeight, ValueHeight);

		CaptionComponent->ClampRegion[UIORIENT_Vertical].bSubregionEnabled = TRUE;// EnableSubregion(UIORIENT_Vertical, TRUE);  don't want to trigger another scene update
		ValueComponent->ClampRegion[UIORIENT_Vertical].bSubregionEnabled = TRUE;// EnableSubregion(UIORIENT_Vertical, TRUE);  don't want to trigger another scene update

		CaptionComponent->SetSubregionSize(UIORIENT_Vertical, MaxHeight, UIEXTENTEVAL_Pixels);
		ValueComponent->SetSubregionSize(UIORIENT_Vertical, MaxHeight, UIEXTENTEVAL_Pixels);
	}

	Super::PostSceneUpdate();
}

/**
 * Evaluates the Position value for the specified face into an actual pixel value.  Should only be
 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
 *
 * @param	Face	the face that should be resolved
 */
void UGearUICollapsingSelectionList::ResolveFacePosition( EUIWidgetFace Face )
{
	UUIObject::ResolveFacePosition(Face);

	if ( CaptionComponent != NULL )
	{
		CaptionComponent->ResolveFacePosition(Face);
	}

	if ( ValueComponent != NULL )
	{
		ValueComponent->ResolveFacePosition(Face);
	}

	if ( CellDataComponent != NULL )
	{
		CellDataComponent->ResolveFacePosition(Face);
	}
}

/** === IUIDataStorePublisher interface === */
/**
 * Sets the data store binding for this object to the text specified.
 *
 * @param	MarkupText			a markup string which resolves to data exposed by a data store.  The expected format is:
 *								<DataStoreTag:DataFieldTag>
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 */
void UGearUICollapsingSelectionList::SetDataStoreBinding( const FString& MarkupText, INT BindingIndex/*=INDEX_NONE*/ )
{
	if ( BindingIndex == INDEX_NONE || BindingIndex == CaptionDataSource.BindingIndex )
	{
		CaptionDataSource.MarkupString = MarkupText;
		RefreshSubscriberValue(BindingIndex);
	}
	
	if ( BindingIndex == INDEX_NONE || BindingIndex == DataSource.BindingIndex )
	{
		//@note: leaving this here for now in case we decide we need to do additional stuff here...
		Super::SetDataStoreBinding(MarkupText, BindingIndex);
	}
	else
	{
		Super::SetDataStoreBinding(MarkupText, BindingIndex);
	}
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
FString UGearUICollapsingSelectionList::GetDataStoreBinding( INT BindingIndex/*=INDEX_NONE*/ ) const
{
	FString Result;

	if ( BindingIndex == CaptionDataSource.BindingIndex )
	{
		Result = CaptionDataSource.MarkupString;
	}
	else if ( BindingIndex == INDEX_NONE || BindingIndex == DataSource.BindingIndex )
	{
		Result = DataSource.MarkupString;
	}
	else
	{
		Result = Super::GetDataStoreBinding(BindingIndex);
	}

	return Result;
}

/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
UBOOL UGearUICollapsingSelectionList::RefreshSubscriberValue( INT BindingIndex/*=INDEX_NONE*/ )
{
	const UBOOL bListBinding = IsInitialized() && (BindingIndex == INDEX_NONE || BindingIndex == UCONST_COLLAPSESELECTION_LIST_DATABINDING_INDEX);
	if ( bListBinding )
	{
		// don't allow the UIList implementation of RefreshSubscriberValue change the index as it won't know which element was
		// actually selected so will just set the index to its current value.
		eventDisableSetIndex();
	}

	UBOOL bResult = Super::RefreshSubscriberValue(BindingIndex);
	if ( bListBinding )
	{
		// now we can search the list for the string that was pulled from the data source, so re-enable SetIndex
		eventEnableSetIndex();

		FUIProviderFieldValue ResolvedValue(EC_EventParm);
		if ( DataSource.GetBindingValue(ResolvedValue) )
		{
			INT ItemIndex = INDEX_NONE;
			if ( ResolvedValue.StringValue.Len() )
			{
				ItemIndex = FindItemIndex(ResolvedValue.StringValue, 0);
			}
			else if ( ResolvedValue.ArrayValue.Num() > 0 )
			{
				ItemIndex = Items.FindItemIndex(ResolvedValue.ArrayValue(0));
			}

			if ( Items.IsValidIndex(ItemIndex) )
			{
				if ( !SetIndex(ItemIndex, TRUE) )
				{
					FString NewValue = GetElementValue(Index);
					ValueComponent->SetValue(NewValue);
				}
				bResult = TRUE;
			}
		}
	}

	if ( bResult )
	{
		// sync the SubmittedIndex with the Index pulled from the data store.
		SubmittedIndex = Index;
	}
	
	if ( BindingIndex == INDEX_NONE || BindingIndex == CaptionDataSource.BindingIndex )
	{
		if ( CaptionComponent != NULL && IsInitialized() )
		{
			CaptionDataSource.ResolveMarkup(this);
			CaptionComponent->SetValue(CaptionDataSource.MarkupString);
			CaptionComponent->ReapplyFormatting();
			bResult = TRUE;
		}
	}

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
void UGearUICollapsingSelectionList::NotifyDataStoreValueUpdated( UUIDataStore* SourceDataStore, UBOOL bValuesInvalidated, FName PropertyTag, UUIDataProvider* SourceProvider, INT ArrayIndex )
{
	const UBOOL bBoundToDataStore = (SourceDataStore == CaptionDataSource.ResolvedDataStore
								&&	(PropertyTag == NAME_None || PropertyTag == CaptionDataSource.DataStoreField));

	if ( bBoundToDataStore )
	{
		LOG_DATAFIELD_UPDATE(SourceDataStore,bValuesInvalidated,PropertyTag,SourceProvider,ArrayIndex);
		RefreshSubscriberValue(CaptionDataSource.BindingIndex);
	}
	else
	{
		Super::NotifyDataStoreValueUpdated(SourceDataStore, bValuesInvalidated, PropertyTag, SourceProvider, ArrayIndex);

		//@fixme ronp - this could be handled better by returning a pointer to the data property which was updated...or something like that
		// then we call super to see if we should sync our SubmittedIndex
		TArray<UUIDataStore*> BoundDataStores;
		Super::GetBoundDataStores(BoundDataStores);

		if ( BoundDataStores.ContainsItem(SourceDataStore) )
		{
			SubmittedIndex = Index;
		}
	}
}

/**
 * Retrieves the list of data stores bound by this subscriber.
 *
 * @param	out_BoundDataStores		receives the array of data stores that subscriber is bound to.
 */
void UGearUICollapsingSelectionList::GetBoundDataStores(TArray<UUIDataStore*>& out_BoundDataStores)
{
	Super::GetBoundDataStores(out_BoundDataStores);

	if ( CaptionComponent != NULL )
	{
		CaptionComponent->GetResolvedDataStores(out_BoundDataStores);
	}
	if ( CaptionDataSource )
	{
		out_BoundDataStores.AddUniqueItem(*CaptionDataSource);
	}
}

/* === UUIScreenObject interface === */
/**
 * Perform all initialization for this widget. Called on all widgets when a scene is opened,
 * once the scene has been completely initialized.
 * For widgets added at runtime, called after the widget has been inserted into its parent's
 * list of children.
 *
 * Initializes the string components.
 *
 * @param	inOwnerScene	the scene to add this widget to.
 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
 *							is being added to the scene's list of children.
 */
void UGearUICollapsingSelectionList::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner/*=NULL*/ )
{
	if ( CaptionComponent != NULL )
	{
		CaptionComponent->InitializeComponent();
	}

	if ( ValueComponent != NULL )
	{
		ValueComponent->InitializeComponent();
	}

	Super::Initialize(inOwnerScene, inOwner);

	if ( VerticalScrollbar != NULL )
	{
		VerticalScrollbar->Opacity = 0.f;
	}
}

/**
 * Called when a property is modified that could potentially affect the widget's position onscreen.
 */
void UGearUICollapsingSelectionList::RefreshPosition()
{
	Super::RefreshPosition();

	//@note: nothing for now.
}

/**
 * Called to globally update the formatting of all UIStrings.
 */
void UGearUICollapsingSelectionList::RefreshFormatting( UBOOL bRequestSceneUpdate/*=TRUE*/ )
{
	Super::RefreshFormatting(bRequestSceneUpdate);

	if ( CaptionComponent != NULL )
	{
		CaptionComponent->ReapplyFormatting(bRequestSceneUpdate);
	}

	if ( ValueComponent != NULL )
	{
		ValueComponent->ReapplyFormatting(bRequestSceneUpdate);
	}
}

/**
 * Marks the Position for any faces dependent on the specified face, in this widget or its children,
 * as out of sync with the corresponding RenderBounds.
 *
 * @param	Face	the face to modify; value must be one of the EUIWidgetFace values.
 */
void UGearUICollapsingSelectionList::InvalidatePositionDependencies( BYTE Face )
{
	Super::InvalidatePositionDependencies(Face);

	if ( CaptionComponent != NULL )
	{
		CaptionComponent->InvalidatePositionDependencies(Face);
	}

	if ( ValueComponent != NULL )
	{
		ValueComponent->InvalidatePositionDependencies(Face);
	}
}

/**
 * Retrieves the current value for some data currently being interpolated by this widget.
 *
 * @param	AnimationType		the type of animation data to retrieve
 * @param	out_CurrentValue	receives the current data value; animation type determines which of the fields holds the actual data value.
 *
 * @return	TRUE if the widget supports the animation type specified.
 */
UBOOL UGearUICollapsingSelectionList::Anim_GetValue( /*EUIAnimType*/BYTE AnimationType, FUIAnimationRawData& out_CurrentValue ) const
{
	UBOOL bResult = FALSE;

	switch( AnimationType )
	{
		case EAT_PositionOffset:
		{
			UUIComp_CollapsingListPresenter* ListComponent = Cast<UUIComp_CollapsingListPresenter>(CellDataComponent);
			if ( ListComponent != NULL )
			{
				out_CurrentValue.DestAsVector = ListComponent->RenderOffsetPercentage;
				bResult = TRUE;
			}
			break;
		}

		case EAT_Color:
		{
			UUIComp_CollapsingListPresenter* ListComponent = Cast<UUIComp_CollapsingListPresenter>(CellDataComponent);
			if ( ListComponent != NULL )
			{
				out_CurrentValue.DestAsColor.A = ListComponent->AnimationOpacity;
				bResult = TRUE;
			}
			break;
		}
	}

	return bResult || Super::Anim_GetValue(AnimationType, out_CurrentValue);
}

/**
 * Updates the current value for some data currently being interpolated by this widget.
 *
 * @param	AnimationType		the type of animation data to set
 * @param	out_CurrentValue	contains the updated data value; animation type determines which of the fields holds the actual data value.
 *
 * @return	TRUE if the widget supports the animation type specified.
 */
UBOOL UGearUICollapsingSelectionList::Anim_SetValue( /*EUIAnimType*/BYTE AnimationType, const FUIAnimationRawData& NewValue )
{
	UBOOL bResult = FALSE;

	switch( AnimationType )
	{
		case EAT_PositionOffset:
		{
			UUIComp_CollapsingListPresenter* ListComponent = Cast<UUIComp_CollapsingListPresenter>(CellDataComponent);
			if ( ListComponent != NULL )
			{
				ListComponent->RenderOffsetPercentage = NewValue.DestAsVector;
			}
			bResult = TRUE;
			break;
		}

		case EAT_Color:
		{
			UUIComp_CollapsingListPresenter* ListComponent = Cast<UUIComp_CollapsingListPresenter>(CellDataComponent);
			if ( ListComponent != NULL )
			{
				ListComponent->AnimationOpacity = NewValue.DestAsColor.A;
			}
			bResult = TRUE;
			break;
		}
	}

	return bResult || Super::Anim_SetValue(AnimationType, NewValue);
}

/**
 * Deactivates the UIState_Focused menu state and updates the pertinent members of FocusControls.
 *
 * @param	FocusedChild	the child of this widget that is currently "focused" control for this widget.
 *							A value of NULL indicates that there is no focused child.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated the focus change.
 */
UBOOL UGearUICollapsingSelectionList::LoseFocus( UUIObject* FocusedChild, INT PlayerIndex )
{
	UBOOL bResult = Super::LoseFocus(FocusedChild, PlayerIndex);

	// if the list is losing focus, hide it.
	if (bResult
	&&	FocusPropagation.IsValidIndex(PlayerIndex)
	&&	!FocusPropagation(PlayerIndex).bPendingReceiveFocus)
	{
		if ( IsExpanded() || IsExpanding() )
		{
			bResult = Collapse();
		}
		else if ( IsCollapsing() && CollapseAnimation != NULL )
		{
			// make the collapsing animation finish immediately
			StopUIAnimation(CollapseAnimation->SeqName, CollapseAnimation, TRUE);
		}

		UUIScene* SceneOwner = GetScene();
		if ( SceneOwner != NULL )
		{
			UUIObject* FocusHintObject = SceneOwner->eventGetFocusHint(TRUE);
			if ( FocusHintObject != NULL && FocusHintObject->IsAnimating() )
			{
				FocusHintObject->StopUIAnimation(NAME_None, ExpandFocusHintAnimation);
				FocusHintObject->StopUIAnimation(NAME_None, CollapseFocusHintAnimation);
			}
		}
	}

//	tracef(TEXT("ComboBox::LoseFocus  FocusedChild: %s"), FocusedChild != NULL ? *FocusedChild->GetTag().ToString() : TEXT("NULL"));
	return bResult;
}

/**
 * Handles input events for this list.
 *
 * @param	EventParms		the parameters for the input event
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UGearUICollapsingSelectionList::ProcessInputKey( const FSubscribedInputEventParameters& EventParms )
{
	UBOOL bResult = FALSE;

	if ( EventParms.InputAliasName == UIKEY_Clicked )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_DoubleClick )
		{
			const UBOOL bIsDoubleClickPress = EventParms.EventType==IE_DoubleClick;

			// notify unrealscript
			if ( DELEGATE_IS_SET(OnPressed) )
			{
				delegateOnPressed(this, EventParms.PlayerIndex);
			}

			if ( bIsDoubleClickPress && DELEGATE_IS_SET(OnDoubleClick) )
			{
				delegateOnDoubleClick(this, EventParms.PlayerIndex);
			}

			// activate the pressed state
			ActivateStateByClass(UUIState_Pressed::StaticClass(),EventParms.PlayerIndex);
			if ( bIsDoubleClickPress )
			{
				ActivateEventByClass(EventParms.PlayerIndex, UUIEvent_OnDoubleClick::StaticClass(), this);
				if ( !bSingleClickSubmission && IsCursorInputKey(EventParms.InputKeyName) )
				{
					NotifySubmitSelection(EventParms.PlayerIndex);
				}
			}

			bResult = TRUE;
		}
		else if ( EventParms.EventType == IE_Repeat )
		{
			//@todo - should we ignore repeats???
			//@todo - definitely should while animating.
			if ( DELEGATE_IS_SET(OnPressRepeat) )
			{
				delegateOnPressRepeat(this, EventParms.PlayerIndex);
			}

			bResult = TRUE;
		}
		// only do this if we're still the focused widget
		else if ( EventParms.EventType == IE_Released && IsFocused(EventParms.PlayerIndex) )
		{
			// Fire OnPressed Delegate
			if ( DELEGATE_IS_SET(OnPressRelease) )
			{
				delegateOnPressRelease(this, EventParms.PlayerIndex);
			}

			if ( IsPressed(EventParms.PlayerIndex) )
			{
				// deactivate the pressed state
				DeactivateStateByClass(UUIState_Pressed::StaticClass(),EventParms.PlayerIndex);

				FVector2D MousePos(0,0);
				const UBOOL bCursorSelection = IsCursorInputKey(EventParms.InputKeyName);
				if ( !bCursorSelection || !GetCursorPosition(MousePos, GetScene()) || ContainsPoint(MousePos) )
				{
					if ( IsExpanded() )
					{
						if ( !DELEGATE_IS_SET(OnClicked) || !delegateOnClicked(this, EventParms.PlayerIndex) )
						{
							if ( bCursorSelection )
							{
								// if the user used the cursor the select this element, update the list's index
								// this will result in a call to NotifyValueChanged 
								INT ClickedIndex = CalculateIndexFromCursorLocation();
								if ( ClickedIndex != INDEX_NONE )
								{
									SetIndex(ClickedIndex);
								}
							}
							
							// activate the on click event
							ActivateEventByClass(EventParms.PlayerIndex,UUIEvent_OnClick::StaticClass(), this);
						}

						if ( bSingleClickSubmission || !bCursorSelection )
						{
							NotifySubmitSelection(EventParms.PlayerIndex);
						}
					}
					else if ( IsCollapsed() )
					{
						Expand();
					}
					else if ( IsExpanding() )
					{
						Collapse();
					}
					else if ( IsCollapsing() )
					{
						Expand();
					}
				}
			}
			bResult = TRUE;
		}
	}
	else if ( EventParms.InputAliasName == UIKEY_SubmitListSelection )
	{
		const UBOOL bIsPressed = IsPressed(EventParms.PlayerIndex);
		if ( IsFocused(EventParms.PlayerIndex) )
		{
			if ( EventParms.EventType == IE_Pressed )
			{
				// notify unrealscript
				if ( DELEGATE_IS_SET(OnPressed) )
				{
					delegateOnPressed(this, EventParms.PlayerIndex);
				}

				// activate the pressed state
				ActivateStateByClass(UUIState_Pressed::StaticClass(),EventParms.PlayerIndex);
			}
			else if ( EventParms.EventType == IE_Repeat )
			{
				if ( DELEGATE_IS_SET(OnPressRepeat) )
				{
					delegateOnPressRepeat(this, EventParms.PlayerIndex);
				}

				bResult = TRUE;
			}
			else if ( EventParms.EventType == IE_Released )
			{
				// Fire OnPressed Delegate
				if ( DELEGATE_IS_SET(OnPressRelease) )
				{
					delegateOnPressRelease(this, EventParms.PlayerIndex);
				}

				if ( bIsPressed )
				{
					// deactivate the pressed state
					DeactivateStateByClass(UUIState_Pressed::StaticClass(),EventParms.PlayerIndex);
				}
			}

			if ( IsExpanded() || IsExpanding() )
			{
				Super::ProcessInputKey(EventParms);
				bResult = TRUE;
			}
			else if ( IsCollapsed() || IsCollapsing() )
			{
				if ( EventParms.EventType == IE_Released && bIsPressed )
				{
					Expand();
				}
				bResult = TRUE;
			}
			else
			{
				bResult = TRUE;
			}
		}
	}
	else if ( 
			EventParms.InputAliasName == UIKEY_MoveSelectionLeft
		||	EventParms.InputAliasName == UIKEY_MoveSelectionRight
		||	EventParms.InputAliasName == UIKEY_MoveSelectionUp
		||	EventParms.InputAliasName == UIKEY_MoveSelectionDown
		||	EventParms.InputAliasName == UIKEY_PageUp
		||	EventParms.InputAliasName == UIKEY_PageDown
		||	EventParms.InputAliasName == UIKEY_SelectFirstElement
		||	EventParms.InputAliasName == UIKEY_SelectLastElement
		||	EventParms.InputAliasName == UIKEY_SelectAllItems )
	{
		if ( IsExpanded() || IsExpanding() || IsPressed(EventParms.PlayerIndex) )
		{
			bResult = Super::ProcessInputKey(EventParms);
		}
		else
		{
			// we've determined that we don't want to process this input or that we want to use the non-list aliases
			// for handling the input event, but in order for UUIObject::ProcessInputKey to handle this input correctly
			// we'll need to re-translate the input key into an input alias using the UIObject class 
 			FInputEventParameters AlternateParms(
				EventParms.PlayerIndex, EventParms.ControllerId,
				EventParms.InputKeyName, static_cast<EInputEvent>(EventParms.EventType),
				EventParms.bAltPressed, EventParms.bCtrlPressed, EventParms.bShiftPressed
				);

			FName AlternateKey(NAME_None);
			if ( TranslateKey(AlternateParms, AlternateKey, UUIObject::StaticClass()) )
			{
				FSubscribedInputEventParameters AlternateAliasParms(AlternateParms, AlternateKey);
				bResult = Super::ProcessInputKey(AlternateAliasParms);
			}
			else
			{
				bResult = Super::Super::ProcessInputKey(EventParms);
			}
		}
	}
	else
	{
		bResult = Super::Super::ProcessInputKey(EventParms);
	}

	return bResult;
}

/* === UObject interface === */
/**
 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
 */
void UGearUICollapsingSelectionList::PreEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PreEditChange(PropertyThatChanged);

	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
/*			if ( PropertyName == TEXT("LabelBackground") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the StringRenderComponent itself was changed
				if ( MemberProperty == ModifiedProperty && LabelBackground != NULL )
				{
					// the user either cleared the value of the LabelBackground or is assigning a new value to it.
					// Unsubscribe the current component from our list of style resolvers.
					RemoveStyleSubscriber(LabelBackground);
				}
			}
			else */
			if ( PropertyName == TEXT("CaptionComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the StringRenderComponent itself was changed
				if ( MemberProperty == ModifiedProperty && CaptionComponent != NULL )
				{
					// the user either cleared the value of the StringRenderComponent or is assigning a new value to it.
					// Unsubscribe the current component from our list of style resolvers.
					RemoveStyleSubscriber(CaptionComponent);
				}
			}
			else if ( PropertyName == TEXT("ValueComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the StringRenderComponent itself was changed
				if ( MemberProperty == ModifiedProperty && ValueComponent != NULL )
				{
					// the user either cleared the value of the StringRenderComponent or is assigning a new value to it.
					// Unsubscribe the current component from our list of style resolvers.
					RemoveStyleSubscriber(ValueComponent);
				}
			}
		}
	}
}

/**
 * Called when a property value from a member struct or array has been changed in the editor.
 */
void UGearUICollapsingSelectionList::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UBOOL bHandled = FALSE;

		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("CaptionDataSource")	)
			{
				if ( RefreshSubscriberValue(CaptionDataSource.BindingIndex) && CaptionComponent != NULL )
				{
					if (CaptionComponent->IsAutoSizeEnabled(UIORIENT_Horizontal)
					||	CaptionComponent->IsAutoSizeEnabled(UIORIENT_Vertical)
					||	CaptionComponent->GetWrapMode() != CLIP_None )
					{
						RefreshPosition();
					}
				}
			}
			else if ( PropertyName == TEXT("CaptionSize") || PropertyName == TEXT("ValueSize") )
			{
				if ( PropertyName == TEXT("CaptionSize") )
				{
					if ( CaptionComponent != NULL && ValueComponent != NULL )
					{
						const FLOAT CaptionWidth = CaptionSize.GetValue(this);
						const FLOAT MaxValueSize = GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport) - CaptionWidth;
						
						const FLOAT CurrentValueSize = ValueSize.GetValue(this);
						if ( CurrentValueSize > MaxValueSize )
						{
							ValueComponent->SetSubregionOffset(UIORIENT_Horizontal, CaptionWidth, UIEXTENTEVAL_Pixels);
							ValueComponent->SetSubregionSize(UIORIENT_Horizontal, MaxValueSize, UIEXTENTEVAL_Pixels);
						}
					}

					// extra stuff
				}
				else if ( PropertyName == TEXT("ValueSize") )
				{
					if ( CaptionComponent != NULL && ValueComponent != NULL )
					{
						const FLOAT ValueWidth = ValueSize.GetValue(this);
						const FLOAT MaxCaptionSize = GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport) - ValueWidth;
						
						const FLOAT CurrentCaptionSize = CaptionSize.GetValue(this);
						if ( CurrentCaptionSize > MaxCaptionSize )
						{
							CaptionComponent->SetSubregionSize(UIORIENT_Horizontal, MaxCaptionSize, UIEXTENTEVAL_Pixels);
						}
						ValueComponent->SetSubregionOffset(UIORIENT_Horizontal, MaxCaptionSize, UIEXTENTEVAL_Pixels);
					}
				}
			}
			else if ( PropertyName == TEXT("RowAutoSizeMode") )
			{
				// force this list to always use this auto-size mode
				RowAutoSizeMode = CELLAUTOSIZE_AdjustList;
				RefreshPosition();
			}
			else if ( PropertyName == TEXT("ListExpansionState") )
			{
				RefreshPosition();
			}
			else
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the ImageComponent itself was changed
				if ( PropertyName == TEXT("CaptionComponent") )
				{
					if ( MemberProperty == ModifiedProperty )
					{
						if ( CaptionComponent != NULL )
						{
							UUIComp_DrawString* CaptionComponentTemplate = GetArchetype<ThisClass>()->CaptionComponent;
							if ( CaptionComponent != NULL )
							{
								CaptionComponent->StyleResolverTag = CaptionComponentTemplate->StyleResolverTag;
							}

							// user added created a new string render component - add it to the list of style subscribers
							AddStyleSubscriber(CaptionComponent);

							// now initialize the new string component
							TScriptInterface<IUIDataStoreSubscriber> Subscriber(this);
							CaptionComponent->InitializeComponent(&Subscriber);

							// then initialize its style
							CaptionComponent->NotifyResolveStyle(GetActiveSkin(), FALSE, GetCurrentState());

							// finally initialize its text
							RefreshSubscriberValue(CaptionDataSource.BindingIndex);
						}
					}
					else
					{
						// user modified a property of the CaptionComponent - make sure it's not a property which will interfere with CaptionSize/ValueSize
					}
				}
				else if ( PropertyName == TEXT("ValueComponent") )
				{
					if ( MemberProperty == ModifiedProperty )
					{
						if ( ValueComponent != NULL )
						{
							UUIComp_DrawString* ValueComponentTemplate = GetArchetype<ThisClass>()->ValueComponent;
							if ( ValueComponent != NULL )
							{
								ValueComponent->StyleResolverTag = ValueComponentTemplate->StyleResolverTag;
							}

							// user added created a new string render component - add it to the list of style subscribers
							AddStyleSubscriber(ValueComponent);

							// now initialize the new string component
							TScriptInterface<IUIDataStoreSubscriber> Subscriber(this);
							ValueComponent->InitializeComponent(&Subscriber);

							// then initialize its style
							ValueComponent->NotifyResolveStyle(GetActiveSkin(), FALSE, GetCurrentState());

							// finally initialize its text
							RefreshSubscriberValue(CaptionDataSource.BindingIndex);
						}
					}
					else
					{
						// user modified a property of the ValueComponent - make sure it's not a property which will interfere with CaptionSize/ValueSize
					}
				}
			}
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

/* ==========================================================================================================
	UUIComp_GearCaption
========================================================================================================== */
IMPLEMENT_CLASS(UUIComp_GearCaption);

/**
 * Initializes the render parameters that will be used for formatting the string.
 *
 * @param	Face			the face that was being resolved
 * @param	out_Parameters	[out] the formatting parameters to use for formatting the string.
 *
 * @return	TRUE if the formatting data is ready to be applied to the string, taking into account the autosize settings.
 */
UBOOL UUIComp_GearCaption::GetStringFormatParameters( EUIWidgetFace Face, FRenderParameters& out_Parameters ) const
{
	UBOOL bResult = Super::GetStringFormatParameters(Face, out_Parameters);
	return bResult;
}

/**
 * Calculates the position and size of the bounding region available for rendering this component's string, taking
 * into account any configured bounding region clamping.
 *
 * @param	[out] BoundingRegionStart	receives the location of the upper left corner of the bounding region, in
 *										pixels relative to the upper left corner of the screen.
 * @param	[out] BoundingRegionSize	receives the size of the bounding region, in absolute pixels.
 */
void UUIComp_GearCaption::CalculateBoundingRegion( FLOAT* BoundingRegionStart[UIORIENT_MAX], FLOAT* BoundingRegionSize[UIORIENT_MAX] ) const
{
	Super::CalculateBoundingRegion(BoundingRegionStart, BoundingRegionSize);
}

/**
 * Wrapper for getting the docking-state of the owning widget's four faces.  No special logic here, but child classes
 * can use this method to make the formatting code ignore the fact that the widget may be docked (in cases where it is
 * irrelevant)
 *
 * @param	bFaceDocked		[out] an array of bools representing whether the widget is docked on the respective face.
 */
void UUIComp_GearCaption::GetOwnerDockingState( UBOOL* bFaceDocked[UIFACE_MAX] ) const
{
	for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
	{
		*bFaceDocked[FaceIndex] = FALSE;
	}
}

/**
 * Adjusts the owning widget's bounds according to the wrapping mode and autosize behaviors.
 */
void UUIComp_GearCaption::UpdateOwnerBounds( FRenderParameters& Parameters )
{
	Super::UpdateOwnerBounds(Parameters);

//	ClampRegion[UIORIENT_Vertical].bSubregionEnabled = TRUE;
// 	EnableSubregion(UIORIENT_Vertical);
	SetSubregionSize(UIORIENT_Vertical, Parameters.DrawYL, UIEXTENTEVAL_Pixels);
}

/* ==========================================================================================================
	UUIComp_CollapsingListPresenter
========================================================================================================== */
/* === UUIComp_ListPresenter interface === */
/**
 * Renders the elements in this list.
 *
 * @param	RI					the render interface to use for rendering
 */
void UUIComp_CollapsingListPresenter::Render_List( FCanvas* Canvas )
{
	Super::Render_List(Canvas);

#if ANIMATION_USES_CANVASMASK
	UGearUICollapsingSelectionList* OwnerList = GetOuterUGearUICollapsingSelectionList();
	if ( Canvas != NULL && !RenderOffsetPercentage.IsNearlyZero(DELTA) && OwnerList->IsTransitioning() )
	{
		Canvas->PopTransform();
		Canvas->PopMaskRegion();
	}
#endif
}

/**
 * Renders the overlay image for a single list element.  Moved into a separate function to allow child classes to easily override
 * and modify the way that the overlay is rendered.
 *
 * @param	same as Render_ListElement, except that no values are passed back to the caller.
 */
void UUIComp_CollapsingListPresenter::Render_ElementOverlay( FCanvas* Canvas, INT ElementIndex, const FRenderParameters& Parameters, const FVector2D& DefaultCellSize )
{
	UGearUICollapsingSelectionList* Owner = GetOuterUGearUICollapsingSelectionList();

	// the only thing we need to do differently is to adjust the parameters used for rendering the overlay selection so that it takes up
	// the entire width of the list.
	FRenderParameters OverlayParameters = Parameters;

	const FLOAT ListIndentPixels = Owner->ListIndent.GetValue(Owner);
	const FLOAT HintPaddingPixels = SelectionHintPadding.GetValue(Owner);
	OverlayParameters.DrawX -= Max(ListIndentPixels, HintPaddingPixels);
//	OverlayParameters.DrawXL += ListIndentPixels;

	Super::Render_ElementOverlay(Canvas, ElementIndex, OverlayParameters, DefaultCellSize);

#if 0
	// look up the list of element cells associated with the specified list item
	TArray<FUIListElementCell>& Cells = ListItems(ElementIndex).Cells;
	if ( Cells.Num() > 0 )
	{
		EUIListElementState CellState = static_cast<EUIListElementState>(ListItems(ElementIndex).ElementState);

		// Render configured item overlay/background
		if ( ListItemOverlay[CellState] != NULL )
		{

			// calculate the dimensions to use for any outlines that need to be rendered for this element
			switch ( Owner->CellLinkType )
			{
			case LINKED_None:
				OverlayParameters.DrawXL = DefaultCellSize.X;
				OverlayParameters.DrawYL = DefaultCellSize.Y;
				break;

			case LINKED_Rows:
				OverlayParameters.DrawXL = DefaultCellSize.X;
				// UITexture::Render_Texture expects the YL to be a height value, not a coordinate
				OverlayParameters.DrawYL -= OverlayParameters.DrawY;
				break;

			case LINKED_Columns:
				// UITexture::Render_Texture expects the XL to be a width value, not a coordinate
				OverlayParameters.DrawXL -= OverlayParameters.DrawX;
				OverlayParameters.DrawYL = DefaultCellSize.Y;
				break;
			}

			OverlayParameters.DrawCoords = ListItemOverlayCoordinates[CellState];
			ListItemOverlay[CellState]->Render_Texture(Canvas, OverlayParameters);
		}
	}
#endif
}

/* === UUIComp_ListPresenterBase interface === */
/**
 * Determines the appropriate position for the selection hint object based on the size of the list's rows and any padding that must be taken
 * into account.
 *
 * @param	SelectionHintObject		the widget that will display the selection hint (usually a label).
 * @param	ElementIndex			the index of the element to display the selection hint next to.
 */
UBOOL UUIComp_CollapsingListPresenter::SetSelectionHintPosition( UUIObject* SelectionHintObject, INT ElementIndex )
{
	UBOOL bResult = FALSE;

	UGearUICollapsingSelectionList* OwnerList = GetOuterUGearUICollapsingSelectionList();
	UUIScene* OwnerScene = OwnerList->GetScene();
	INT Idx = OwnerList->TopIndex;

	// figure out the amount of padding that should be applied
	const FLOAT HintPadding = SelectionHintPadding.GetValue(OwnerList);
	const FLOAT HeaderSize = OwnerList->GetHeaderSize();

	FLOAT PosY = OwnerList->GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
	if ( OwnerList->IsCollapsed() )
	{
		SelectionHintObject->SetDockParameters(UIFACE_Top, OwnerList, UIFACE_Top, 0.f);
		SelectionHintObject->SetDockParameters(UIFACE_Bottom, OwnerList, UIFACE_Bottom, 0.f);
		SelectionHintObject->SetDockParameters(UIFACE_Right, OwnerList, UIFACE_Left, -2.f);
		bResult = TRUE;
	}
	else if ( OwnerList->bFadingSelectionHintOut )
	{
		SelectionHintObject->SetDockParameters(UIFACE_Top, OwnerList, UIFACE_Top, 0.f);
		SelectionHintObject->SetDockParameters(UIFACE_Right, OwnerList, UIFACE_Left, -2.f);

		// don't dock to the bottom anymore
		SelectionHintObject->SetDockParameters(UIFACE_Bottom, NULL, UIFACE_MAX, 0.f);
		bResult = TRUE;
	}
	else
	{
		PosY += OwnerList->GetHeaderSize();

		SelectionHintObject->SetDockParameters(UIFACE_Right, OwnerList, UIFACE_Left, HintPadding);
		SelectionHintObject->SetDockParameters(UIFACE_Top, NULL, UIFACE_MAX, 0.f);
		SelectionHintObject->SetDockParameters(UIFACE_Bottom, NULL, UIFACE_MAX, 0.f);

		for ( ; Idx < ListItems.Num(); Idx++ )
		{
			if ( Idx == ElementIndex )
			{		
				// found the top position - don't call SetPosition if the scene is resolving positions as that case is handled below
				if ( OwnerScene == NULL || !OwnerScene->bResolvingScenePositions )
				{
					SelectionHintObject->SetPosition(PosY, UIFACE_Top, EVALPOS_PixelViewport);
				}
				bResult = TRUE;
				break;
			}

			PosY += OwnerList->GetRowHeight(Idx, FALSE, FALSE);
		}

		if ( OwnerScene != NULL && OwnerScene->bResolvingScenePositions )
		{
			// since UpdateOwnerBounds potentially adjusts the list's renderbounds directly, we need to sync up to those values here
			SelectionHintObject->Position.SetPositionValue(SelectionHintObject, PosY, UIFACE_Top, EVALPOS_PixelViewport, FALSE);
			SelectionHintObject->RenderBounds[UIFACE_Top] = PosY;
			SelectionHintObject->Position.ValidatePosition(UIFACE_Top);
			SelectionHintObject->DockTargets.MarkResolved(UIFACE_Top);

			FLOAT NextValue = OwnerList->GetPosition(UIFACE_Left, EVALPOS_PixelViewport) + HintPadding;
			SelectionHintObject->Position.SetPositionValue(SelectionHintObject, NextValue, UIFACE_Right, EVALPOS_PixelViewport, FALSE);
			SelectionHintObject->RenderBounds[UIFACE_Right] = NextValue;
			SelectionHintObject->Position.ValidatePosition(UIFACE_Right);
			SelectionHintObject->DockTargets.MarkResolved(UIFACE_Right);


			// SelectionHintObject should be configured to lock its width and height, and if it's a label it should be configured
			// to auto-size itself.  So once we've set the RenderBounds value for the docked faces, calling ResolveFacePosition should be
			// all we need to do.
			SelectionHintObject->RefreshFormatting(FALSE);
			SelectionHintObject->ResolveFacePosition(UIFACE_Left);
			SelectionHintObject->ResolveFacePosition(UIFACE_Bottom);
		}
	}

// 	debugf(TEXT("%s::SetSelectionHintPosition  Collapsed:%i  HintPadding:%f   TopIndex:%i   PosY:%f  Idx:%i  ElementIdx:%i  bResult:%i  bResolvingScenePositions:%i  NumItems:%i"),
// 		*GetPathName(), OwnerList->IsCollapsed(), HintPadding, OwnerList->TopIndex, PosY, Idx, ElementIndex, bResult, OwnerList->GetScene() ? OwnerList->GetScene()->bResolvingScenePositions : -1, ListItems.Num());

	return bResult;
}

/**
 * Initializes the render parameters that will be used for formatting the list elements.
 *
 * @param	Face			the face that was being resolved
 * @param	out_Parameters	[out] the formatting parameters to use when calling ApplyFormatting.
 *
 * @return	TRUE if the formatting data is ready to be applied to the list elements, taking into account the autosize settings.
 */
UBOOL UUIComp_CollapsingListPresenter::GetListRenderParameters( EUIWidgetFace Face, FRenderParameters& out_Parameters )
{
	UBOOL bResult = Super::GetListRenderParameters(Face, out_Parameters);

	if ( bResult )
	{
		UGearUICollapsingSelectionList* OwnerList = GetOuterUGearUICollapsingSelectionList();
		const FLOAT Offset = OwnerList->GetMaxLabelHeight();
		out_Parameters.DrawY += Offset;

		const FLOAT IndentPixels = OwnerList->ListIndent.GetValue(OwnerList);
		out_Parameters.DrawX += IndentPixels;
	}

	return bResult;
}

/**
 * Wrapper for getting the docking-state of the owning widget's four faces.  No special logic here, but child classes
 * can use this method to make the formatting code ignore the fact that the widget may be docked (in cases where it is
 * irrelevant)
 *
 * @param	bFaceDocked		[out] an array of bools representing whether the widget is docked on the respective face.
 */
void UUIComp_CollapsingListPresenter::GetOwnerDockingState( UBOOL* bFaceDocked[UIFACE_MAX] ) const
{
	for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
	{
		*bFaceDocked[FaceIndex] = FALSE;
	}
}

/**
 * Adjusts the owning widget's bounds according to the autosize settings.
 */
void UUIComp_CollapsingListPresenter::UpdateOwnerBounds( FRenderParameters& Parameters )
{
	UGearUICollapsingSelectionList* OwnerList = GetOuterUGearUICollapsingSelectionList();
	const FLOAT LabelHeight = OwnerList->GetMaxLabelHeight();

	// for convenience
	FLOAT* RenderBounds = OwnerList->RenderBounds;
	FUIScreenValue_Bounds& Position = OwnerList->Position;
	FUIDockingSet& DockTargets = OwnerList->DockTargets;

	// Now that we've formatted the list, adjust the bounds of the owning widget if applicable (i.e. if auto-sizing is enabled).
	// The order in which the widget's faces were resolved was determined by the logic in AddDockingNode, and GetListFormatParameters
	// should only return TRUE only after the faces which are NOT going to be adjusted here have all been resolved.

	// So, at this point the only faces that the RenderBounds of the widget should be out-of-date for are those faces which we are going
	// to adjust.  Therefore, we can use RenderBounds directly to determine offsets and such, rather than the [slower] GetPosition methods.
	UBOOL bDocked[UIFACE_MAX] = { FALSE, FALSE, FALSE, FALSE };
	UBOOL* pDocked[UIFACE_MAX] = 
	{
		&bDocked[UIFACE_Left], &bDocked[UIFACE_Top],
		&bDocked[UIFACE_Right], &bDocked[UIFACE_Bottom],
	};

	GetOwnerDockingState(pDocked);

	const UBOOL bLockedHorz = bDocked[UIFACE_Left] && bDocked[UIFACE_Right];
	const UBOOL bLockedVert = bDocked[UIFACE_Top] && bDocked[UIFACE_Bottom];

	const UBOOL bAutoSizeHorz = ShouldAdjustListBounds(UIORIENT_Horizontal);
	const UBOOL bAutoSizeVert = ShouldAdjustListBounds(UIORIENT_Vertical);

	FLOAT TargetHeight = OwnerList->GetHeaderSize();
	if ( OwnerList->IsCollapsed() )
	{
		// fully collapsed

		//@todo ronp - should refactor this into its own method to avoid duplication of code
	}
// 	else if ( !OwnerList->IsExpanded() )
// 	{
// 		// expanding or collapsing
// 
// 		//@todo ronp - here we do the same thing as the else below, but we need to calculate the required height of the list
// 		// based on the expansion/collapse animation position
// 	}
	else
	{
		// fully expanded
		const INT NumRows = GetMaxNumVisibleRows();
		const FLOAT Spacing = OwnerList->CellSpacing.GetValue(OwnerList);
		for ( INT RowIndex = OwnerList->TopIndex; RowIndex - OwnerList->TopIndex < NumRows; RowIndex++ )
		{
			TargetHeight += OwnerList->GetRowHeight(RowIndex);
			if ( RowIndex < NumRows - 1 )
			{
				TargetHeight += Spacing;
			}
		}
	}

	UBOOL bUpdateScrollbar = FALSE;

	// if the widget is docked on both faces, no adjustments can be made
	if ( !bDocked[UIFACE_Top] || !bDocked[UIFACE_Bottom] )
	{
		if ( !bDocked[UIFACE_Bottom] )
		{
			// If the widget is not docked or is docked only on its top face, then the bottom face of the widget needs to be adjusted
			OwnerList->Position.SetPositionValue(OwnerList, TargetHeight, UIFACE_Bottom, EVALPOS_PixelOwner, FALSE);
			OwnerList->RenderBounds[UIFACE_Bottom] = OwnerList->RenderBounds[UIFACE_Top] + TargetHeight;
			OwnerList->Position.ValidatePosition(UIFACE_Bottom);
			OwnerList->DockTargets.MarkResolved(UIFACE_Bottom);
			bUpdateScrollbar = TRUE;
		}
		else
		{
			// otherwise, if the widget is docked only on its bottom face, then the top face needs to be adjusted.
			const FLOAT NewPositionY = OwnerList->RenderBounds[UIFACE_Bottom] - TargetHeight;

			OwnerList->Position.SetPositionValue(OwnerList, NewPositionY, UIFACE_Top, EVALPOS_PixelViewport, FALSE);
			OwnerList->RenderBounds[UIFACE_Top] = NewPositionY;
			OwnerList->Position.ValidatePosition(UIFACE_Top);
			OwnerList->DockTargets.MarkResolved(UIFACE_Top);
			bUpdateScrollbar = TRUE;
		}
	}

	if ( bAutoSizeHorz )
	{
		FLOAT TargetWidth = 0.f;

		const INT NumCols = OwnerList->CellLinkType != LINKED_None ? GetTotalColumnCount() : OwnerList->GetMaxNumVisibleColumns();
		const FLOAT Spacing = OwnerList->CellLinkType != LINKED_Columns ? OwnerList->CellSpacing.GetValue(OwnerList) : 0.f;
		for ( INT CellIndex = 0; CellIndex < NumCols; CellIndex++ )
		{
			TargetWidth += OwnerList->GetColumnWidth(CellIndex);
			if ( CellIndex < NumCols - 1 )
			{
				TargetWidth += Spacing;
			}
		}

		// if the widget is docked on both faces, no adjustments can be made
		if ( !bDocked[UIFACE_Left] || !bDocked[UIFACE_Right] )
		{
			if ( !bDocked[UIFACE_Right] )
			{
				// If the widget is not docked or is docked only on its left face, then the right face of the widget needs to be adjusted
				OwnerList->Position.SetPositionValue(OwnerList, TargetWidth, UIFACE_Right, EVALPOS_PixelOwner, FALSE);
				OwnerList->RenderBounds[UIFACE_Right] = OwnerList->RenderBounds[UIFACE_Left] + TargetWidth;
				OwnerList->Position.ValidatePosition(UIFACE_Right);
				OwnerList->DockTargets.MarkResolved(UIFACE_Right);
			}
			else
			{
				// otherwise, if the widget is docked only on its right face, then the left face needs to be adjusted.
				const FLOAT NewPositionX = OwnerList->RenderBounds[UIFACE_Right] - TargetWidth;
				OwnerList->Position.SetPositionValue(OwnerList, NewPositionX, UIFACE_Left, EVALPOS_PixelViewport, FALSE);
				OwnerList->RenderBounds[UIFACE_Left] = NewPositionX;
				OwnerList->Position.ValidatePosition(UIFACE_Left);
				OwnerList->DockTargets.MarkResolved(UIFACE_Left);
			}
		}
	}

	if ( bUpdateScrollbar && OwnerList->VerticalScrollbar != NULL )
	{
		// if we changed the size of the list, we'll need to make sure the vertical scrollbar has been updated to match the new size
		OwnerList->VerticalScrollbar->InvalidateAllPositions(FALSE);
		for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
		{
			OwnerList->VerticalScrollbar->ResolveFacePosition(static_cast<EUIWidgetFace>(FaceIndex));
		}
	}
}

/**
 * Setup the left, top, width, and height values that will be used to render the list.  This will typically be the list's 
 * RenderBounds, unless the elements should be rendered in a subportion of the list.
 *
 * @fixme ronp - mmmmm, this is a bit hacky..  we're already doing something similar on the formatting side...seems like
 * we should be able to leverage that work so that we don't get out of sync.  :\
 */
void UUIComp_CollapsingListPresenter::InitializeRenderingParms( FRenderParameters& Parameters, FCanvas* Canvas/*=NULL*/ )
{
	Super::InitializeRenderingParms(Parameters);

	UGearUICollapsingSelectionList* OwnerList = GetOuterUGearUICollapsingSelectionList();
	const FLOAT CaptionHeight = OwnerList->GetMaxLabelHeight();
	Parameters.DrawY += CaptionHeight;

#if ANIMATION_USES_CANVASMASK
	if ( Canvas != NULL && !RenderOffsetPercentage.IsNearlyZero(DELTA) && OwnerList->IsTransitioning() )
	{
		FVector OffsetValue(EC_EventParm);
		OffsetValue.X = RenderOffsetPercentage.X * (OwnerList->RenderBounds[UIFACE_Right] - OwnerList->RenderBounds[UIFACE_Left]);
		OffsetValue.Y = RenderOffsetPercentage.Y * Parameters.DrawYL;

		Canvas->PushMaskRegion( 
			Parameters.DrawX, Parameters.DrawY,
			Parameters.DrawXL + OffsetValue.X, Parameters.DrawYL + OffsetValue.Y
			);

		Canvas->PushRelativeTransform(FTranslationMatrix(OffsetValue));
	}

	const FLOAT IndentPixels = OwnerList->ListIndent.GetValue(OwnerList);
	Parameters.DrawX += IndentPixels;
#endif
}

/* ==========================================================================================================
	UGearUIObjectList
========================================================================================================== */
IMPLEMENT_CLASS(UGearUIObjectList);

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
void UGearUIObjectList::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);

	if ( inOwnerScene != NULL )
	{
		inOwnerScene->RegisterTickableObject(this);
	}
}

/**
 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
 *
 * This version adds the BackgroundImageComponent (if non-NULL) to the StyleSubscribers array.
 */
void UGearUIObjectList::InitializeStyleSubscribers()
{
	Super::InitializeStyleSubscribers();

	VALIDATE_COMPONENT(SelectionOverlayImage);
	AddStyleSubscriber(SelectionOverlayImage);
}

/** Initializes the vertical scrollbar for the optionlist. */
void UGearUIObjectList::InitializeScrollbars()
{
	if ( GIsGame && VerticalScrollbar == NULL )
	{
		VerticalScrollbar = Cast<UUIScrollbar>(CreateWidget(this, UUIScrollbar::StaticClass()));
		VerticalScrollbar->ScrollbarOrientation = UIORIENT_Vertical;

		InsertChild( VerticalScrollbar );
	}
}

#define USE_CONSOLE_WIDGETS 1//CONSOLE

/** Uses the currently bound list element provider to generate a set of child option widgets. */
void UGearUIObjectList::RegenerateOptions()
{
	if ( GeneratedObjects.Num() > 0 )
	{
		//@todo ronp - refactor this into a function

		// before removing the current list of options, check whether the selection hint is 
		// one of our children.  If so, it's probably docked to one of the widgets we're about to
		// remove so reparent the selection hint back to the scene.
		UUIScene* ContainerScene = GetScene();
		if ( ContainerScene != NULL )
		{
			UUIObject* SelectionHint = ContainerScene->eventGetFocusHint(TRUE);
			if ( SelectionHint != NULL )
			{
				UUIScreenObject* HintParent = SelectionHint->GetParent();
				if ( HintParent != NULL && HintParent != ContainerScene && ContainsChild(SelectionHint) )
				{
					if ( HintParent->ReparentChild(SelectionHint, ContainerScene) )
					{
						SelectionHint->eventSetVisibility(FALSE);
					}
				}
			}
		}
	}

	GeneratedObjects.Empty();

	// Remove all children.
	CurrentIndex=PreviousIndex=0;

	TArray<UUIObject*> ChildrenToRemove = Children;
	ChildrenToRemove.RemoveItem(VerticalScrollbar);
	RemoveChildren(ChildrenToRemove);

	// now move them all into the transient package so that StaticAllocateObject doesn't
	// use their archetypes as the new widgets' archetypes due to it finding an object in memory with the same name and outer
	for ( INT ChildIndex = 0; ChildIndex < ChildrenToRemove.Num(); ChildIndex++ )
	{
		UUIObject* Child = ChildrenToRemove(ChildIndex);
		ChildrenToRemove(ChildIndex)->Rename(NULL, UObject::GetTransientPackage(), REN_ForceNoResetLoaders);
	}

	// Generate new options
	if ( DataSource.MarkupString.Len() && DataSource.ResolveMarkup(this) )
	{
		DataProvider = DataSource->ResolveListElementProvider(DataSource.DataStoreField.ToString());

		if(DataProvider)
		{
			TArray<INT> ListElements;
			DataProvider->GetListElements(DataSource.DataStoreField, ListElements);
			for(INT ElementIdx=0; ElementIdx<ListElements.Num(); ElementIdx++)
			{
				TScriptInterface<IUIListElementCellProvider> ElementProvider = DataProvider->GetElementCellValueProvider(DataSource.DataStoreField, ListElements(ElementIdx));
				if(ElementProvider)
				{
					UUIDataProvider_MenuItem* OptionProvider = Cast<UUIDataProvider_MenuItem>(ElementProvider->GetUObjectInterfaceUIListElementCellProvider());
					if(OptionProvider)
					{
						FName NewOptionName(NAME_None), NewOptionLabelName(NAME_None);
						{
							// to make it possible for us to search for specific option widgets, use a deterministic name
							// based on their data binding
							INT DelimPos = OptionProvider->DataStoreMarkup.InStr(TEXT("."), TRUE, TRUE);
							if ( DelimPos == INDEX_NONE )
							{
								DelimPos = OptionProvider->DataStoreMarkup.InStr(TEXT(":"), TRUE, TRUE);
							}
							if ( DelimPos != INDEX_NONE )
							{
								// also strip off the trailing >
								FString OptionName = OptionProvider->DataStoreMarkup.Mid(DelimPos+1, OptionProvider->DataStoreMarkup.Len() - DelimPos - 2);
								NewOptionName = *OptionName;
								NewOptionLabelName = *(OptionName+TEXT("Label"));
							}
						}

						// Create a label for the option.
						UUIObject* NewOptionObj = NULL;
						UUIObject* NewOptionLabelObject = NULL;
						if ( OptionProvider->OptionType != MENUOT_CollapsingList )
						{
							NewOptionLabelObject = CreateWidget(this, UUILabel::StaticClass(), NULL, NewOptionLabelName);
							UUILabel* NewOptionLabel = Cast<UUILabel>(NewOptionLabelObject);
							
							if(NewOptionLabel)
							{
								InsertChild(NewOptionLabel);
								if(OptionProvider->FriendlyName.Len())
								{
									NewOptionLabel->SetDataStoreBinding(OptionProvider->FriendlyName);
								}
								else
								{
									NewOptionLabel->SetDataStoreBinding(OptionProvider->CustomFriendlyName);
								}
								NewOptionLabel->SetEnabled(FALSE);
								if ( NewOptionLabel->StringRenderComponent != NULL )
								{
									NewOptionLabel->StringRenderComponent->StringStyle.DefaultStyleTag = LabelCaptionStyleName;
								}

								if ( LabelBackgroundStyleName != NAME_None )
								{
									if ( NewOptionLabel->LabelBackground == NULL )
									{
										NewOptionLabel->LabelBackground = ConstructObject<UUIComp_DrawImage>(UUIComp_DrawImage::StaticClass(), NewOptionLabel);
									}

									NewOptionLabel->LabelBackground->ImageStyle.DefaultStyleTag = LabelBackgroundStyleName;
								}
								else if ( NewOptionLabel->LabelBackground != NULL )
								{
									// clear it
									NewOptionLabel->LabelBackground = NULL;
								}
							}
						}

						switch(static_cast<EMenuOptionType>(OptionProvider->OptionType))
						{
						case MENUOT_Slider:
							{
								UUISlider* NewOption = Cast<UUISlider>(CreateWidget(this, SliderClass, NULL, NewOptionName));
								if(NewOption)
								{	
									NewOptionObj = NewOption;
									NewOptionObj->TabIndex = ElementIdx;
									NewOption->SliderValue = OptionProvider->RangeData;
									InsertChild(NewOption);
									NewOption->SetDataStoreBinding(OptionProvider->DataStoreMarkup);
								}				
							}
							break;

						case MENUOT_EditBox:
							{
								UUIEditBox* NewOption = Cast<UUIEditBox>(CreateWidget(this, EditboxClass, NULL, NewOptionName));
								if(NewOption)
								{	
									NewOptionObj = NewOption;
									NewOption->MaxCharacters = OptionProvider->EditBoxMaxLength;
									NewOption->TabIndex = ElementIdx;
									NewOption->InitialValue=TEXT("");
									NewOption->CharacterSet = OptionProvider->EditboxAllowedChars;
									InsertChild(NewOption);
									NewOption->SetValue(TEXT(""));
									NewOption->SetDataStoreBinding(OptionProvider->DataStoreMarkup);
								}	
							}
							break;

						case MENUOT_CollapsingList:
							{
								UGearUICollapsingSelectionList* NewOption = Cast<UGearUICollapsingSelectionList>(CreateWidget(this, UGearUICollapsingSelectionList::StaticClass(), NULL, NewOptionName));
								if ( NewOption )
								{
									NewOptionObj = NewOption;
									NewOption->TabIndex = ElementIdx;
									InsertChild(NewOption);

									NewOption->WrapType = LISTWRAP_Jump;
									if(OptionProvider->FriendlyName.Len())
									{
										NewOption->SetDataStoreBinding(OptionProvider->FriendlyName, UCONST_COLLAPSESELECTION_CAPTION_DATABINDING_INDEX);
									}
									else
									{
										NewOption->SetDataStoreBinding(OptionProvider->CustomFriendlyName, UCONST_COLLAPSESELECTION_CAPTION_DATABINDING_INDEX);
									}

									NewOption->SetDataStoreBinding(OptionProvider->DataStoreMarkup, UCONST_COLLAPSESELECTION_LIST_DATABINDING_INDEX);
									NewOption->CaptionSize = CollapsingListCaptionSize;
									NewOption->ValueSize = CollapsingListCaptionSize;
									NewOption->ValueSize.Value = 1.f - CollapsingListCaptionSize.Value;

									if ( OptionProvider->SchemaCellFields.Num() > 0 )
									{
										TScriptInterface<IUIListElementCellProvider> SchemaProvider = NewOption->CellDataComponent->GetCellSchemaProvider();
										// retrieve the list of data field tags that this list element provider supports
										TMap<FName,FString> AvailableCellTags;
										if ( SchemaProvider )
										{
											SchemaProvider->GetElementCellTags(NewOption->DataSource.DataStoreField, AvailableCellTags);

											INT InsertIndex=0;
											for ( INT SchemaIdx = 0; SchemaIdx < OptionProvider->SchemaCellFields.Num(); SchemaIdx++ )
											{
												FName CellField = OptionProvider->SchemaCellFields(SchemaIdx);
												if ( AvailableCellTags.HasKey(CellField) )
												{
													NewOption->ClearCellBinding(InsertIndex);
													NewOption->InsertSchemaCell(InsertIndex++, CellField, *AvailableCellTags.Find(CellField));
												}
												else
												{
													debugf(TEXT("%s: SchemaCellField %i (%s) is not supported by the schema cell provider '%s'"),
														*NewOption->GetName(), SchemaIdx, *CellField.ToString(), *SchemaProvider->GetUObjectInterfaceUIListElementCellProvider()->GetName());
												}
											}
										}

										NewOption->RefreshSubscriberValue(UCONST_COLLAPSESELECTION_LIST_DATABINDING_INDEX);
									}
								}
							}
							break;
#if !USE_CONSOLE_WIDGETS
						case MENUOT_Spinner:
							{
								UUINumericEditBox* NewOption = Cast<UUINumericEditBox>(CreateWidget(this, SpinnerClass, NULL, NewOptionName));
								if(NewOption)
								{	
									NewOptionObj = NewOption;
									NewOptionObj->TabIndex = ElementIdx;
									NewOption->NumericValue = OptionProvider->RangeData;
									InsertChild(NewOption);
									NewOption->SetDataStoreBinding(OptionProvider->DataStoreMarkup);
								}	
							}
							break;

						case MENUOT_CheckBox:
							{
								UUICheckbox* NewOption = Cast<UUICheckbox>(CreateWidget(this, CheckboxClass, NULL, NewOptionName));
								if(NewOption)
								{	
									NewOptionObj = NewOption;
									NewOptionObj->TabIndex = ElementIdx;
									InsertChild(NewOption);
									NewOption->SetDataStoreBinding(OptionProvider->DataStoreMarkup);
								}	
							}
							break;

						default:
							{
								UUIComboBox* NewCombo = Cast<UUIComboBox>(CreateWidget(this, ComboBoxClass, NULL, NewOptionName));
								if(NewCombo)
								{	
									NewOptionObj = NewCombo;
									NewOptionObj->TabIndex = ElementIdx;
									InsertChild(NewCombo);

									//NewOption->SetupChildStyles();	// Need to call this to set the default combobox value since we changed the markup for the list.
									NewCombo->ComboEditbox->bReadOnly = !OptionProvider->bEditableCombo;
									if ( OptionProvider->bNumericCombo && NewCombo->ComboButton != NULL )
									{
										NewCombo->ComboEditbox->CharacterSet=CHARSET_NumericOnly;
									}

									if ( NewCombo->ComboButton != NULL )
									{
										NewCombo->ComboButton->SetCaption(TEXT(""));
									}
								}
							}
#else

						default:
							{
								// If we are on Console, create an option button, otherwise create a combobox.
								UUIOptionList* NewOption = Cast<UUIOptionList>(CreateWidget(this, ConsoleOptionClass, NULL, NewOptionName));
								if(NewOption)
								{
									NewOptionObj = NewOption;
									NewOptionObj->TabIndex = ElementIdx;
									InsertChild(NewOption);
									NewOption->SetDataStoreBinding(OptionProvider->DataStoreMarkup);
								}							
							}
#endif
							break;
						}

						// Store a reference to the new object
						if(NewOptionObj)
						{
							FGeneratedObjectInfo* OptionInfo = new(GeneratedObjects) FGeneratedObjectInfo;
							OptionInfo->LabelObj = NewOptionLabelObject;
							OptionInfo->OptionObj = NewOptionObj;
							OptionInfo->OptionProviderName = OptionProvider->GetFName();
							OptionInfo->OptionProvider = OptionProvider;

							if ( NewOptionLabelObject != NULL )
							{
								NewOptionLabelObject->SetEnabled(FALSE);
							}

							if ( !IsEnabled(UCONST_TEMP_SPLITSCREEN_INDEX) )
							{
								NewOptionObj->SetEnabled(FALSE);
							}
						}
					}
				}
			}

			for ( INT OptionIdx = 1; OptionIdx < GeneratedObjects.Num(); OptionIdx++ )
			{
				UUIObject* PreviousWidget = GeneratedObjects(OptionIdx - 1).OptionObj;
				UUIObject* CurrentWidget = GeneratedObjects(OptionIdx).OptionObj;

				PreviousWidget->SetForcedNavigationTarget(UIFACE_Bottom, CurrentWidget);
				CurrentWidget->SetForcedNavigationTarget(UIFACE_Top, PreviousWidget);
			}

			// Make focus for the first and last objects wrap around.
			if ( bClosedListNavigation && GeneratedObjects.Num() )
			{
				UUIObject* FirstWidget = GeneratedObjects(0).OptionObj;
				UUIObject* LastWidget = GeneratedObjects(GeneratedObjects.Num()-1).OptionObj;

				if ( FirstWidget && LastWidget )
				{
					FirstWidget->SetForcedNavigationTarget(UIFACE_Top, LastWidget);
					LastWidget->SetForcedNavigationTarget(UIFACE_Bottom, FirstWidget);
				}
			}
		}
	}

	// Instance a prefab BG.
	if(BGPrefab)
	{
		if(BGPrefabInstance == NULL)
		{
			BGPrefabInstance = BGPrefab->InstancePrefab(Owner, TEXT("BGPrefab"));
		}

		if(BGPrefabInstance!=NULL)
		{
			// Set some private behavior for the prefab.
			BGPrefabInstance->SetPrivateBehavior(UCONST_PRIVATE_NotEditorSelectable, TRUE);
			BGPrefabInstance->SetPrivateBehavior(UCONST_PRIVATE_TreeHiddenRecursive, TRUE);

			// Add the prefab to the list.
			InsertChild(BGPrefabInstance,0);
		}
	}

	InitializeScrollbars();

	eventSetupOptionBindings();
	if ( DELEGATE_IS_SET(OnRegeneratedOptions) )
	{
		delegateOnRegeneratedOptions(this);
	}

	RequestSceneUpdate(TRUE,TRUE,FALSE,TRUE);

	// the only reason this call to RepositionOptions is necessary is for GearUICollapsingSelectionList - due to the fact that it
	// sets the horizontal subregion for its labels in PreSceneUpdate (which would be before the object list had set the width of the
	// collapsing selection list), we must call RepositionOptions immediately to ensure that the selection list has the correct width
	// when PreSceneUpdate is called.
	RepositionOptions();
	RequestOptionListReformat();
}

/**
 * Repositions all option widgets.
 */
void UGearUIObjectList::ResolveFacePosition( EUIWidgetFace Face )
{
	Super::ResolveFacePosition(Face);

	if ( bRepositionOptions && HasResolvedAllFaces() )
	{
		// if we've resolved all faces and the flag is set indicating that options need to be repositioned, do that now.
		RepositionOptions();
		bRepositionOptions = FALSE;
	}
}

/**
* Routes rendering calls to children of this screen object.
*
* @param	Canvas	the canvas to use for rendering
* @param	UIPostProcessGroup	Group determines current pp pass that is being rendered
*/
void UGearUIObjectList::Render_Children( FCanvas* Canvas, EUIPostProcessGroup UIPostProcessGroup )
{
	UUIScene* OwnerScene = GetScene();

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
					RenderList.AddItem(FocusedPlayerControl);
				}
			}
		}
	}

	UUIObject* SelectedChild = NULL, *SelectedLabel = NULL;
	if ( GeneratedObjects.IsValidIndex(CurrentIndex) )
	{
		SelectedLabel = GeneratedObjects(CurrentIndex).LabelObj;
		SelectedChild = GeneratedObjects(CurrentIndex).OptionObj;
	}

	const UBOOL bRotationSupported = OwnerScene->bSupportsRotation;
	for ( INT i = 0; i < RenderList.Num(); i++ )
	{
		UUIObject* Child = RenderList(i);

		// apply the widget's rotation
		if ( Child->IsVisible() && Child->Opacity > KINDA_SMALL_NUMBER )
		{
			// add this widget to the scene's render stack
			OwnerScene->RenderStack.Push(Child);

			// apply the widget's transform matrix
			if ( bRotationSupported )
			{
				Canvas->PushRelativeTransform(Child->GenerateTransformMatrix(FALSE));
			}

			// use the widget's ZDepth as the sorting key for the canvas
			Canvas->PushDepthSortKey(appCeil(Child->GetZDepth()));

			if ( SelectionOverlayImage != NULL && BGPrefabInstance == NULL
			&&	(Child == SelectedChild || Child == SelectedLabel) )
			{
				const FLOAT ResScale = GetAspectRatioAutoScaleFactor();

				const UBOOL bScrollbarVisible = VerticalScrollbar != NULL && VerticalScrollbar->IsVisible();
				const FLOAT FinalScrollBarWidth = bScrollbarVisible ? VerticalScrollbar->GetScrollZoneWidth() + 8.f * ResScale : 0.f;
				const FLOAT OptionRightMargin = ResScale * 10;
				const FLOAT OptionPaddingPixels = OptionPadding.GetValue(this);

				OverlayImageBounds[UIFACE_Left] = RenderBounds[UIFACE_Left];
				OverlayImageBounds[UIFACE_Top] = Child->RenderBounds[UIFACE_Top] - OptionPaddingPixels * 0.5f;
				OverlayImageBounds[UIFACE_Right] = RenderBounds[UIFACE_Right] - (FinalScrollBarWidth + OptionRightMargin);
				OverlayImageBounds[UIFACE_Bottom] = OverlayImageBounds[UIFACE_Top] + OptionHeight.GetValue(this) + OptionPaddingPixels;

				FRenderParameters Parameters(
					OverlayImageBounds[UIFACE_Left], OverlayImageBounds[UIFACE_Top],
					OverlayImageBounds[UIFACE_Right] - OverlayImageBounds[UIFACE_Left],
					OverlayImageBounds[UIFACE_Bottom] - OverlayImageBounds[UIFACE_Top],
					NULL, GetViewportHeight()
					);

				SelectionOverlayImage->RenderComponent(Canvas, Parameters);
				SelectedChild = SelectedLabel = NULL;
			}

			// now render the child
			Render_Child(Canvas, Child, UIPostProcessGroup);

			// restore the previous sort key
			Canvas->PopDepthSortKey();

			if ( bRotationSupported )
			{
				// restore the previous transform
				Canvas->PopTransform();
			}
		}
	}

	// restore the previous global fade value
	Canvas->AlphaModulate = CurrentAlphaModulation;
}


/** Repositions the previously generated options. */
void UGearUIObjectList::RepositionOptions()
{
	FVector2D ViewportSize;
	GetViewportSize(ViewportSize);

	const FLOAT OwnerWidth = GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport);
	const FLOAT OwnerHeight = GetBounds(UIORIENT_Vertical, EVALPOS_PixelViewport);
	const FLOAT AnimationTime = 0.025f;
	const FLOAT TextPaddingPercentage = 0.125f;
	const FLOAT ResScale = ViewportSize.Y / 768.0f; // The option height and padding amounts are for 1024x768 so use that as our scaling factor.

	const FLOAT OptionHeightInPixels = OptionHeight.GetValue(this);
	const FLOAT OptionPaddingPixels = OptionPadding.GetValue(this);
	const FLOAT OptionTopMargin = 0.f;
	const FLOAT OptionRightMargin = 10.0f*ResScale;
	const FLOAT FinalOptionHeight = (OptionPaddingPixels+OptionHeightInPixels);
	MaxVisibleItems = appFloor(GetBounds(UIORIENT_Vertical, EVALPOS_PixelViewport) / FinalOptionHeight);
	FLOAT CenteringPadding = 0.0f;		//Padding used to center options when there arent enough to scroll.

	if(VerticalScrollbar != NULL)
	{
		VerticalScrollbar->eventSetVisibility((GeneratedObjects.Num() > MaxVisibleItems));
	}

	const UBOOL bScrollbarVisible = VerticalScrollbar != NULL && VerticalScrollbar->IsVisible();
	const FLOAT FinalScrollBarWidth = bScrollbarVisible ? VerticalScrollbar->GetScrollZoneWidth() + 8.f * ResScale : 0.f;
	const FLOAT OptionOffsetPercentage = (1.f - OptionSize.GetValue(this, UIEXTENTEVAL_PercentSelf));

	FLOAT OptionY = 0.0f;

	// Max number of visible options
	INT MiddleItem = MaxVisibleItems / 2;
	INT TopItem = 0;
	INT OldTopItem = 0;

	if(GeneratedObjects.Num() > MaxVisibleItems)
	{
		// Calculate old top item
		if(PreviousIndex > MiddleItem)
		{
			OldTopItem = PreviousIndex - MiddleItem;
		}
		OldTopItem = Clamp<INT>(OldTopItem, 0, GeneratedObjects.Num()-MaxVisibleItems);

		// Calculate current top item
		if(CurrentIndex > MiddleItem)
		{
			TopItem = CurrentIndex - MiddleItem;
		}
		TopItem = Clamp<INT>(TopItem, 0, GeneratedObjects.Num()-MaxVisibleItems);
	}
	//@fixme ronp - add an alignment property to control this...for now, disabled
// 	else	// Center the list if we dont have a scrollbar.
// 	{
// 		CenteringPadding = (OwnerHeight-GeneratedObjects.Num()*FinalOptionHeight) / 2.0f;
// 	}

	// Loop through all generated objects and reposition them.
	FLOAT InterpTop = 1.0f;

	if ( bAnimatingBGPrefab )
	{
		FLOAT TimeElapsed = GWorld->GetRealTimeSeconds() - StartMovementTime;
		if(TimeElapsed > AnimationTime)
		{
			TimeElapsed = AnimationTime;
		}
		
		InterpTop = TimeElapsed / AnimationTime;

		// Ease In
		InterpTop = 1.0f - InterpTop;
		InterpTop = appPow(InterpTop, 2);
		InterpTop = 1.0f - InterpTop;
	}

	// Animate list movement 
	InterpTop = InterpTop * (TopItem-OldTopItem) + OldTopItem;
	OptionY = -InterpTop * FinalOptionHeight + CenteringPadding;
	const FLOAT FadeDist = 1.0f;

	// this is the padding (in pixels) to apply to the labels' vertical position
	//@todo ronp - why don't we just set their vertical alignment to centered?
	const FLOAT LabelVertPadding = OptionHeightInPixels * TextPaddingPercentage;

	// this is the number of pixels from the right edge of the list the right edge of the options should be
	const FLOAT OptionRightPadding = FinalScrollBarWidth + OptionRightMargin;

	const FLOAT AdjustedOptionWidth = ((1.f - OptionOffsetPercentage) * OwnerWidth) - OptionRightPadding;

	for(INT OptionIdx = 0; OptionIdx<GeneratedObjects.Num(); OptionIdx++)
	{
		UUIObject* NewOptionObj = GeneratedObjects(OptionIdx).OptionObj;
		UUIObject* NewOptionLabelObject = GeneratedObjects(OptionIdx).LabelObj;
		FLOAT WidgetOpacity = 1.0f;
		
		// Calculate opacity for elements off the visible area of the screen.
		FLOAT CurrentRelativeTop = OptionIdx - InterpTop;
		if(CurrentRelativeTop < 0.0f)
		{
			CurrentRelativeTop = Max<FLOAT>(CurrentRelativeTop, -FadeDist);
			WidgetOpacity = 1.0f - CurrentRelativeTop / -FadeDist;
		}
		else if(CurrentRelativeTop - MaxVisibleItems + 1 > 0.0f)
		{
			CurrentRelativeTop = Min<FLOAT>(CurrentRelativeTop - MaxVisibleItems + 1, FadeDist);
			WidgetOpacity = 1.0f - CurrentRelativeTop / FadeDist;
		}

		// Position Label
		if ( NewOptionLabelObject != NULL )
		{
			NewOptionLabelObject->Opacity = WidgetOpacity;
			NewOptionLabelObject->SetPosition(0.f, UIFACE_Left, EVALPOS_PercentageOwner);
			NewOptionLabelObject->SetPosition(1.0f, UIFACE_Right, EVALPOS_PercentageOwner);
			NewOptionLabelObject->SetPosition(OptionY + LabelVertPadding, UIFACE_Top, EVALPOS_PixelOwner);
			NewOptionLabelObject->SetPosition(OptionHeightInPixels - LabelVertPadding, UIFACE_Bottom, EVALPOS_PixelOwner);

			// Position Widget
			NewOptionObj->Opacity = WidgetOpacity;
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Left, EVALPOS_PercentageOwner);
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Right, EVALPOS_PercentageOwner);
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Top, EVALPOS_PercentageOwner);
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Bottom, EVALPOS_PercentageOwner);

			if(NewOptionObj->IsA(UUICheckbox::StaticClass()))	// Keep checkbox's square
			{
				NewOptionObj->SetPosition(AdjustedOptionWidth - OptionHeightInPixels, UIFACE_Left, EVALPOS_PixelOwner);
				NewOptionObj->SetPosition(AdjustedOptionWidth, UIFACE_Right, EVALPOS_PixelOwner);
			}
			else
			{
				NewOptionObj->SetPosition(OptionOffsetPercentage, UIFACE_Left, EVALPOS_PercentageOwner);
				NewOptionObj->SetPosition(AdjustedOptionWidth, UIFACE_Right, EVALPOS_PixelOwner);

	#if USE_CONSOLE_WIDGETS
				if ( NewOptionObj->IsA(UUIOptionList::StaticClass()) )
				{
					UUIOptionList* OptionList = static_cast<UUIOptionList*>(NewOptionObj);
					if ( OptionList->StringRenderComponent != NULL )
					{
						OptionList->StringRenderComponent->EnableSubregion(UIORIENT_Horizontal, TRUE);
						OptionList->StringRenderComponent->SetSubregionAlignment(UIORIENT_Horizontal, UIALIGN_Center);

						const FLOAT SubregionSize = OptionList->IncrementButton->GetPosition(UIFACE_Left, EVALPOS_PixelViewport) - OptionList->DecrementButton->GetPosition(UIFACE_Right, EVALPOS_PixelViewport);
						OptionList->StringRenderComponent->SetSubregionSize(UIORIENT_Horizontal, SubregionSize / AdjustedOptionWidth, UIEXTENTEVAL_PercentSelf);
					}
				}
	#endif
			}
		}
		else
		{
			NewOptionObj->Opacity = WidgetOpacity;
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Left, EVALPOS_PercentageOwner);
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Right, EVALPOS_PercentageOwner);
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Top, EVALPOS_PercentageOwner);
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Bottom, EVALPOS_PercentageOwner);

			NewOptionObj->SetPosition(0.f, UIFACE_Left, EVALPOS_PercentageOwner);
			NewOptionObj->SetPosition(OwnerWidth - OptionRightPadding, UIFACE_Right, EVALPOS_PixelOwner);
		}

		NewOptionObj->SetPosition(OptionY + OptionTopMargin, UIFACE_Top, EVALPOS_PixelOwner);

		// Collapsing selection lists manager their own item height.  If we allow it to be set here
		// it will create a feedback loop, causing the UI to be updated every frame
		UGearUICollapsingSelectionList* CollapsingList = Cast<UGearUICollapsingSelectionList>(NewOptionObj);
		if( CollapsingList == NULL )
		{
			NewOptionObj->SetPosition(OptionHeightInPixels, UIFACE_Bottom);
		}
		else if ( CollapsingList->IsCollapsed() )
		{
			CollapsingList->SetPosition(CollapsingList->GetMaxLabelHeight(), UIFACE_Bottom, EVALPOS_PixelOwner);
		}


		// Store bounds
		GeneratedObjects(OptionIdx).OptionX = 0;
		if ( NewOptionLabelObject != NULL )
		{
			GeneratedObjects(OptionIdx).OptionWidth = AdjustedOptionWidth;
		}
		else
		{
			GeneratedObjects(OptionIdx).OptionWidth = OwnerWidth - OptionRightPadding;
		}
		GeneratedObjects(OptionIdx).OptionY = OptionY;
		GeneratedObjects(OptionIdx).OptionHeight = OptionHeightInPixels;

		// Increment position
		OptionY += FinalOptionHeight;
	}

	// Position the background prefab.
	if ( BGPrefabInstance || SelectionOverlayImage != NULL )
	{
		FLOAT TopFace = GeneratedObjects.IsValidIndex(CurrentIndex) ? GeneratedObjects(CurrentIndex).OptionY : 0.0f;
		if ( bAnimatingBGPrefab )
		{
			FLOAT Distance = GeneratedObjects(CurrentIndex).OptionY - (GeneratedObjects.IsValidIndex(PreviousIndex) ? GeneratedObjects(PreviousIndex).OptionY : 0.f);
			FLOAT TimeElapsed = GWorld->GetRealTimeSeconds() - StartMovementTime;
			if ( TimeElapsed - AnimationTime > -DELTA )
			{
				bAnimatingBGPrefab = FALSE;
				TimeElapsed = AnimationTime;
			}

			// Used to interpolate the scaling to create a smooth looking selection effect.
			// Ease in
			const FLOAT Alpha = appPow(1.0f - (TimeElapsed / AnimationTime), 2);
			TopFace += -Distance * Alpha;
		}

		if ( BGPrefabInstance != NULL )
		{
			// Make the entire prefab percentage owner
			BGPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Top, EVALPOS_PercentageOwner);
			BGPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Bottom, EVALPOS_PercentageOwner);
			BGPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Left, EVALPOS_PercentageOwner);
			BGPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Right, EVALPOS_PercentageOwner);

			BGPrefabInstance->SetPosition(0.0f, UIFACE_Left, EVALPOS_PercentageOwner);
			BGPrefabInstance->SetPosition(1.0f-FinalScrollBarWidth/OwnerWidth, UIFACE_Right, EVALPOS_PercentageOwner);
			BGPrefabInstance->SetPosition(TopFace-OptionPaddingPixels/2.0f, UIFACE_Top, EVALPOS_PixelOwner);
			BGPrefabInstance->SetPosition(FinalOptionHeight, UIFACE_Bottom);
		}
		else
		{
			OverlayImageBounds[UIFACE_Left] = GetPosition(UIFACE_Left, EVALPOS_PixelViewport);
			OverlayImageBounds[UIFACE_Top] = GetPosition(UIFACE_Top, EVALPOS_PixelViewport) + TopFace - OptionPaddingPixels * 0.5f;
			OverlayImageBounds[UIFACE_Right] = OverlayImageBounds[UIFACE_Left] + (OwnerWidth - OptionRightPadding);
			OverlayImageBounds[UIFACE_Bottom] = OverlayImageBounds[UIFACE_Top] + FinalOptionHeight;
		}
	}

	// Refresh scrollbars
	if(VerticalScrollbar != NULL && GeneratedObjects.Num()>0)
	{
		// Initialize the Vertical Scrollbar values
		const INT TotalItems = GeneratedObjects.Num();

		// Initialize marker size to be proportional to the visible area of the list
		FLOAT MarkerSize = 1.0f / (FLOAT)TotalItems;
		VerticalScrollbar->SetMarkerSize( MarkerSize );

		// Initialize the nudge value to be the size of one item
		VerticalScrollbar->SetNudgeSizePercent( 1.f / (FLOAT)TotalItems );

		// Since we do not have a horizontal scrollbar yet, we can disable the corner padding
		VerticalScrollbar->EnableCornerPadding(FALSE);

		// Initialize the marker position only if we are not scrolling.
		UUIInteraction* UIController = GetCurrentUIController();
		if ( UIController != NULL )
		{
			if(UIController->GetOuterUGameViewportClient()->bUIMouseCaptureOverride==FALSE)
			{
				if ( TotalItems > 1 )
				{
					VerticalScrollbar->SetMarkerPosition( ((FLOAT)CurrentIndex) / (TotalItems-1) );
				}
				else
				{
					VerticalScrollbar->SetMarkerPosition(0.f);
				}
			}
		}
	}
}

void UGearUIObjectList::SetSelectedOptionIndex(INT OptionIdx)
{
	PreviousIndex = CurrentIndex;
	CurrentIndex = OptionIdx;

	// Change widget state
	if(GeneratedObjects.IsValidIndex(PreviousIndex) && GeneratedObjects(PreviousIndex).LabelObj)
	{
		GeneratedObjects(PreviousIndex).LabelObj->SetEnabled(FALSE);
	}

	if(GeneratedObjects.IsValidIndex(CurrentIndex) && GeneratedObjects(CurrentIndex).LabelObj)
	{
		GeneratedObjects(CurrentIndex).LabelObj->SetEnabled(TRUE);
	}

	bAnimatingBGPrefab = TRUE;
	StartMovementTime = GWorld->GetRealTimeSeconds();
}

/**
 * Marks the list of options to be repositioned during the next tick.
 */
void UGearUIObjectList::RequestOptionListReformat()
{
	bRepositionOptions = TRUE;
	RequestSceneUpdate(FALSE, TRUE);
}

/**
 * Marks the list of options to be updated next tick.
 */
void UGearUIObjectList::RequestOptionListRegeneration()
{
	bRegenOptions = TRUE;
	RequestSceneUpdate(TRUE, TRUE);
}

/**
 * Wrapper for verifying that a widget is contained in this object list.
 */
UBOOL UGearUIObjectList::IsValidListOption( UUIObject* CheckObj ) const
{
	UBOOL bResult = FALSE;

	if ( CheckObj != NULL && CheckObj->GetOwner() == this && FindObjectIndexByRef(CheckObj) != INDEX_NONE )
	{
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Finds the position of an object using the name of the data provider it's bound to.
 *
 * @param	ProviderName	the data provider tag to search for
 *
 * @return	the index [into the GeneratedObjects array] for a widget that is bound to a data provider with the specified tag, or
 *			INDEX_NONE if there are none.
 */
INT UGearUIObjectList::FindObjectIndexByName( FName ProviderName ) const
{
	INT Result = INDEX_NONE;

	if ( ProviderName != NAME_None )
	{
		for ( INT ObjIndex = 0; ObjIndex < GeneratedObjects.Num(); ObjIndex++ )
		{
			const FGeneratedObjectInfo& ObjInfo = GeneratedObjects(ObjIndex);
			if ( ObjInfo.OptionProviderName == ProviderName )
			{
				Result = ObjIndex;
				break;
			}
		}
	}

	return Result;
}

/**
 * Find the position of an object using a reference to the object.
 *
 * @param	SearchObj	the object to search for
 *
 * @return	the index [into the GeneratedObjects array] for a widget that is in the GeneratedObjects array, or
 *			INDEX_NONE if no matching objects were found.
 */
INT UGearUIObjectList::FindObjectIndexByRef( UUIObject* SearchObj ) const
{
	INT Result = INDEX_NONE;

	if ( SearchObj != NULL )
	{
		for ( INT ObjIndex = 0; ObjIndex < GeneratedObjects.Num(); ObjIndex++ )
		{
			const FGeneratedObjectInfo& ObjInfo = GeneratedObjects(ObjIndex);
			if ( ObjInfo.OptionObj == SearchObj )
			{
				Result = ObjIndex;
				break;
			}
		}
	}

	return Result;
}

/**
 * Get a reference to the option widget that is currently selected.
 *
 * @return	a reference to the child widget that is currently selected for the specified player, or NULL if this list doesn't have a valid
 *			selection.
 */
UUIObject* UGearUIObjectList::GetSelectedOption() const
{
	UUIObject* Result = NULL;

	if ( GeneratedObjects.IsValidIndex(CurrentIndex) )
	{
		const FGeneratedObjectInfo& ObjInfo = GeneratedObjects(CurrentIndex);
		if ( ObjInfo.OptionObj != NULL )
		{
			Result = ObjInfo.OptionObj;
		}
	}

	return Result;
}

void UGearUIObjectList::Tick( FLOAT DeltaTime )
{
	if ( /*bAnimatingBGPrefab ||*/ bRegenOptions )
	{
		const INT PlayerIndex = GetPlayerOwnerIndex();

		FName SelectedOptionName = NAME_None;
		if ( bRegenOptions )
		{
			if ( GIsGame && IsFocused(PlayerIndex) && GeneratedObjects.Num() > 0 )
			{
				UUIObject* SelectedOption = GetSelectedOption();
				if ( SelectedOption != NULL && SelectedOption->IsFocused(PlayerIndex) )
				{
					SelectedOptionName = GeneratedObjects(CurrentIndex).OptionProviderName;
				}
				else
				{
					SelectedOption = GetFocusedControl(FALSE, PlayerIndex);
					if ( SelectedOption != NULL )
					{
						const INT ObjIndex = FindObjectIndexByRef(SelectedOption);
						if ( GeneratedObjects.IsValidIndex(ObjIndex) )
						{
							SelectedOptionName = GeneratedObjects(ObjIndex).OptionProviderName;
						}
					}
				}

				if ( SelectedOption != NULL )
				{
					SelectedOption->KillFocus(NULL, PlayerIndex);
				}
			}

			RegenerateOptions();
			InitializeComboboxWidgets();
			bRegenOptions = FALSE;
		}

		if ( GeneratedObjects.Num() && SelectedOptionName != NAME_None )
		{
			const INT ObjIndex = Max(0, FindObjectIndexByName(SelectedOptionName));
			if ( GeneratedObjects.IsValidIndex(ObjIndex) )
			{
				GeneratedObjects(ObjIndex).OptionObj->SetFocus(NULL, PlayerIndex);
			}
		}
	}
}

// UObject interface
void UGearUIObjectList::GetSupportedUIActionKeyNames(TArray<FName> &out_KeyNames)
{
	Super::GetSupportedUIActionKeyNames(out_KeyNames);

	out_KeyNames.AddItem(UIKEY_Clicked);
}

/**
 * Called when a property is modified that could potentially affect the widget's position onscreen.
 */
void UGearUIObjectList::RefreshPosition()
{
	Super::RefreshPosition();

	RequestOptionListReformat();
}

/** Initializes combobox widgets. */
void UGearUIObjectList::InitializeComboboxWidgets()
{
	FVector2D ViewportSize;
	GetViewportSize(ViewportSize);

	const FLOAT ResScale = GetAspectRatioAutoScaleFactor();

	// Setup combobox bindings
	for ( INT OptionIdx = 0; OptionIdx < GeneratedObjects.Num(); OptionIdx++ )
	{
		UUIComboBox* ComboOption = Cast<UUIComboBox>(GeneratedObjects(OptionIdx).OptionObj);
		if ( ComboOption != NULL )
		{
			UUIDataProvider_MenuItem* OptionDataProvider = Cast<UUIDataProvider_MenuItem>(GeneratedObjects(OptionIdx).OptionProvider);
			if ( OptionDataProvider != NULL )
			{
				FUIProviderFieldValue CurrentStringValue(EC_EventParm);

				ComboOption->ComboList->RowHeight.SetValue(ComboOption->ComboList, 28.0f*ResScale, UIEXTENTEVAL_Pixels);
				ComboOption->ComboList->SetDataStoreBinding(OptionDataProvider->DataStoreMarkup);
			}
		}
	}
}

/** === UIDataSourceSubscriber interface === */
/**
 * Sets the data store binding for this object to the text specified.
 *
 * @param	MarkupText			a markup string which resolves to data exposed by a data store.  The expected format is:
 *								<DataStoreTag:DataFieldTag>
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 */
void UGearUIObjectList::SetDataStoreBinding( const FString& MarkupText, INT BindingIndex/*=-1*/ )
{
	if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		SetDefaultDataBinding(MarkupText,BindingIndex);
	}
	else if ( DataSource.MarkupString != MarkupText )
	{
		Modify();
        DataSource.MarkupString = MarkupText;

		RefreshSubscriberValue(BindingIndex);
	}
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
FString UGearUIObjectList::GetDataStoreBinding( INT BindingIndex/*=-1*/ ) const
{
	if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		return GetDefaultDataBinding(BindingIndex);
	}
	return DataSource.MarkupString;
}

/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
UBOOL UGearUIObjectList::RefreshSubscriberValue( INT BindingIndex/*=INDEX_NONE*/ )
{
	if ( DELEGATE_IS_SET(OnRefreshSubscriberValue) && delegateOnRefreshSubscriberValue(this, BindingIndex) )
	{
		return TRUE;
	}
	else if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		return ResolveDefaultDataBinding(BindingIndex);
	}
	else
	{
		// Regenerate options.
		RequestOptionListRegeneration();
		return TRUE;
	}
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
void UGearUIObjectList::NotifyDataStoreValueUpdated( UUIDataStore* SourceDataStore, UBOOL bValuesInvalidated, FName PropertyTag, UUIDataProvider* SourceProvider, INT ArrayIndex )
{
	const UBOOL bBoundToDataStore = (SourceDataStore == DataSource.ResolvedDataStore &&	(PropertyTag == NAME_None || PropertyTag == DataSource.DataStoreField));
	LOG_DATAFIELD_UPDATE(SourceDataStore,bValuesInvalidated,PropertyTag,SourceProvider,ArrayIndex);


// 	TArray<UUIDataStore*> BoundDataStores;
// 	GetBoundDataStores(BoundDataStores);
// 
// 	if (BoundDataStores.ContainsItem(SourceDataStore)
	//@todo ronp - rather than checking SourceDataStore against DataSource, we should call GetBoundDataStores and check whether SourceDataStore is 
	// contained in that array so that cell strings which contain data store markup can be updated from this function....but if the SourceDataStore
	// IS linked through a cell string, the data store will need to pass the correct index
	if ( bBoundToDataStore )
	{
		RefreshSubscriberValue(DataSource.BindingIndex);
	}
}

/**
 * Retrieves the list of data stores bound by this subscriber.
 *
 * @param	out_BoundDataStores		receives the array of data stores that subscriber is bound to.
 */
void UGearUIObjectList::GetBoundDataStores( TArray<UUIDataStore*>& out_BoundDataStores )
{
	GetDefaultDataStores(out_BoundDataStores);
	// add overall data store to the list
	if ( DataSource )
	{
		out_BoundDataStores.AddUniqueItem(*DataSource);
	}
}

/**
 * Notifies this subscriber to unbind itself from all bound data stores
 */
void UGearUIObjectList::ClearBoundDataStores()
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

/* === UObject interface === */
/**
 * Called when a property value has been changed in the editor.
 */
void UGearUIObjectList::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if( PropertyName == TEXT("BGPrefab")
			||	PropertyName == TEXT("LabelStyleName")
			||	PropertyName == TEXT("bClosedListNavigation")
			||	PropertyName == TEXT("CollapsingListCaptionSize"))
			{
				RequestOptionListRegeneration();
			}
			else if(PropertyName == TEXT("OptionSize")
				||	PropertyName == TEXT("OptionPadding")
				||	PropertyName == TEXT("OptionHeight") )
			{
				//RefreshFormatting();
				RequestSceneUpdate(FALSE, TRUE);
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
 * This version removes all items from the Children array and flags the widget to regenerate them.
 */
void UGearUIObjectList::PreSave()
{
	Super::PreSave();

	TArray<UUIObject*> ChildrenToRemove = Children;
	ChildrenToRemove.RemoveItem(VerticalScrollbar);
	RemoveChildren(ChildrenToRemove);

	// now move them all into the transient package so that StaticAllocateObject doesn't
	// use their archetypes as the new widgets' archetypes due to it finding an object in memory with the same name and outer
	for ( INT ChildIndex = 0; ChildIndex < ChildrenToRemove.Num(); ChildIndex++ )
	{
		UUIObject* Child = ChildrenToRemove(ChildIndex);
		ChildrenToRemove(ChildIndex)->Rename(NULL, UObject::GetTransientPackage(), REN_ForceNoResetLoaders);
	}

	GeneratedObjects.Empty();
	RequestOptionListRegeneration();
}

IMPLEMENT_CLASS(UGearUINavigationList);

// EOF



