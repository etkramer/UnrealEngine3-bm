//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//
// This holds all of the base UI Objects
//=============================================================================
#include "UTGame.h"
#include "UTGameUIClasses.h"
#include "EngineUISequenceClasses.h"
#include "CanvasScene.h"

IMPLEMENT_CLASS(UUIComp_DrawTeamColoredImage);
IMPLEMENT_CLASS(UUIComp_UTGlowString);
IMPLEMENT_CLASS(UUTUI_Widget);
IMPLEMENT_CLASS(UUTUIComboBox);
IMPLEMENT_CLASS(UUTUIList);
IMPLEMENT_CLASS(UUTUINumericEditBox);
IMPLEMENT_CLASS(UUTUIMenuList);
IMPLEMENT_CLASS(UUTUIButtonBar);
IMPLEMENT_CLASS(UUTUIButtonBarButton);
IMPLEMENT_CLASS(UUIComp_UTUIMenuListPresenter);
IMPLEMENT_CLASS(UUTDrawPanel);
IMPLEMENT_CLASS(UUTTabPage);
IMPLEMENT_CLASS(UUTUITabControl);
IMPLEMENT_CLASS(UUTSimpleList);
IMPLEMENT_CLASS(UUTSimpleImageList);
IMPLEMENT_CLASS(UUTUIMeshWidget);
IMPLEMENT_CLASS(UUTDrawPlayerListPanel);
IMPLEMENT_CLASS(UUTUIScene_MidGameMenu);
IMPLEMENT_CLASS(UUTUIScene_SaveProfile);

/*=========================================================================================
  UTUI_Widget - UI_Widgets are collections of other widgets that share game logic.
  ========================================================================================= */

/** 
 * Cache a reference to the scene owner
 * @see UUIObject.Intiailize
 */

void UUTUI_Widget::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	UTSceneOwner = Cast<UUTUIScene>(inOwnerScene);	
	UUTUIScene::AutoPlaceChildren(this);

	Super::Initialize(inOwnerScene, inOwner);
}

/** 
 * !!! WARNING !!! This function does not check the destination and assumes it is valid.
 *
 * LookupProperty - Finds a property of a source actor and returns it's value.
 *
 * @param		SourceActor			The actor to search
 * @param		SourceProperty		The property to look up
 * @out param 	DestPtr				A Point to the storgage of the value
 *
 * @Returns TRUE if the look up succeeded
 */

UBOOL UUTUI_Widget::LookupProperty(AActor *SourceActor, FName SourceProperty, BYTE* DestPtr)
{
	if ( SourceActor && SourceProperty != NAME_None )
	{
		UProperty* Prop = FindField<UProperty>( SourceActor->GetClass(), SourceProperty);
		if ( Prop )
		{
			BYTE* PropLoc = (BYTE*) SourceActor + Prop->Offset;
			if ( Cast<UIntProperty>(Prop) )
			{
				UIntProperty* IntProp = Cast<UIntProperty>(Prop);
				IntProp->CopySingleValue( (INT*)(DestPtr), PropLoc);
				return TRUE;
			}
			else if ( Cast<UBoolProperty>(Prop) )
			{
				UBoolProperty* BoolProp = Cast<UBoolProperty>(Prop);
				BoolProp->CopySingleValue( (UBOOL*)(DestPtr), PropLoc);
				return TRUE;
			}
			else if (Cast<UFloatProperty>(Prop) )
			{
				UFloatProperty* FloatProp = Cast<UFloatProperty>(Prop);
				FloatProp->CopySingleValue( (FLOAT*)(DestPtr), PropLoc);
				return TRUE;
			}
			else if (Cast<UByteProperty>(Prop) )
			{
				UByteProperty* ByteProp = Cast<UByteProperty>(Prop);
				ByteProp->CopySingleValue( (BYTE*)(DestPtr), PropLoc);
				return TRUE;
			}
			else
			{
				debugf(TEXT("Unhandled Property Type (%s)"),*Prop->GetName());
				return FALSE;
			}
		}
	}
	debugf(TEXT("Illegal Proptery Operation (%s) %s"),*GetName(), *SourceProperty.ToString());
	return FALSE;
}		

/*=========================================================================================
  UUIComp_UTGlowString is a special UIComp_DrawString that will render the string twice using
  two styles
  ========================================================================================= */
/**
 * Returns the combo style data being used by this string rendering component.  If the component's StringStyle is not set, the style data
 * will be pulled from the owning widget's PrimaryStyle, if possible.
 *
 * This version resolves the additional style reference property declared by the UTGlowString component.
 *
 * @param	DesiredMenuState	the menu state for the style data to retrieve; if not specified, uses the owning widget's current menu state.
 * @param	SourceSkin			the skin to use for resolving this component's combo style; only relevant when the component's combo style is invalid
 *								(or if TRUE is passed for bClearExistingValue). If the combo style is invalid and a value is not specified, returned value
 *								will be NULL.
 * @param	bClearExistingValue	used to force the component's combo style to be re-resolved from the specified skin; if TRUE, you must supply a valid value for
 *								SourceSkin.
 *
 * @return	the combo style data used to render this component's string for the specified menu state.
 */
UUIStyle_Combo* UUIComp_UTGlowString::GetAppliedStringStyle( UUIState* DesiredMenuState/*=NULL*/, UUISkin* SourceSkin/*=NULL*/, UBOOL bClearExistingValue/*=FALSE*/ )
{
	UUIObject* OwnerWidget = GetOuterUUIObject();

	// if no menu state was specified, use the owning widget's current menu state
	if ( DesiredMenuState == NULL )
	{
		DesiredMenuState = OwnerWidget->GetCurrentState();
	}
	check(DesiredMenuState);

	// only attempt to resolve if the GlowStyle has a valid style reference
	if ( GlowStyle.GetResolvedStyle() != NULL
	||	(SourceSkin != NULL && GlowStyle.AssignedStyleID.IsValid()) )
	{
		const UBOOL bIsStyleManaged = OwnerWidget->IsPrivateBehaviorSet(UCONST_PRIVATE_ManagedStyle);
		if ( bClearExistingValue && !bIsStyleManaged )
		{
			GlowStyle.InvalidateResolvedStyle();
		}

		// get the UIStyle corresponding to the UIStyleReference that we're going to pull from
		// GetResolvedStyle() is guaranteed to return a valid style if a valid UISkin is passed in, unless the style reference is completely invalid

		// If the owner widget's style is being managed by someone else, then StyleToResolve's AssignedStyleID will always be zero, so never pass in
		// a valid skin reference.  This style should have already been resolved by whoever is managing the style, and if the resolved style doesn't live
		// in the currently active skin (for example, it's being inherited from a base skin package), GetResolvedStyle will clear the reference and reset
		// the ResolvedStyle back to the default style for this style reference.
		UUIStyle* ResolvedStyle = GlowStyle.GetResolvedStyle(bIsStyleManaged ? NULL : SourceSkin);
		checkf(ResolvedStyle, TEXT("Unable to resolve style reference (%s)' for '%s.%s'"), *GlowStyle.DefaultStyleTag.ToString(), *OwnerWidget->GetWidgetPathName(), *GetName());
	}

	// always return the draw string component's primary string style
	return Super::GetAppliedStringStyle(DesiredMenuState, SourceSkin, bClearExistingValue);
}

/**
 * Resolves the glow style for this string rendering component.
 *
 * @param	ActiveSkin			the skin the use for resolving the style reference.
 * @param	bClearExistingValue	if TRUE, style references will be invalidated first.
 * @param	CurrentMenuState	the menu state to use for resolving the style data; if not specified, uses the current
 *								menu state of the owning widget.
 * @param	StyleProperty		if specified, only the style reference corresponding to the specified property
 *								will be resolved; otherwise, all style references will be resolved.
 */
UBOOL UUIComp_UTGlowString::NotifyResolveStyle(UUISkin* ActiveSkin,UBOOL bClearExistingValue,UUIState* CurrentMenuState/*=NULL*/,const FName StylePropertyName/*=NAME_None*/)
{
	UBOOL bResult = FALSE;

	// the UIComp_DrawString will call GetAppliedStringStyle (which will cause the glow style to be resolved) if the StylePropertyName is None or StringStyle
	// so we only need to do something if the StylePropertyName is GlowStyle since our parent class won't handle that.
	if ( StylePropertyName == TEXT("GlowStyle") )
	{
		GetAppliedStringStyle(CurrentMenuState, ActiveSkin, bClearExistingValue);
		bResult = GlowStyle.GetResolvedStyle() != NULL;

		// don't think we ever want to apply the GlowStyle to the string from NotifyResolveStyle - that will happen in Render_String
#if 0
		if ( ValueString != NULL )
		{
			// apply this component's per-instance style settings
			FUICombinedStyleData FinalStyleData(ComboStyleData);
			CustomizeAppliedStyle(FinalStyleData);

			// apply the style data to the string
			if ( ValueString->SetStringStyle(FinalStyleData) )
			{
				ReapplyFormatting();
			}
		}
#endif
	}

	return Super::NotifyResolveStyle(ActiveSkin,bClearExistingValue,CurrentMenuState,StylePropertyName) || bResult;
}

#if 0
/** 
 * Resolve a style Reference to a given style. See UUIComp_DrawString for more inf.
 *
 * @Params	StyleToResolve		The style to resolve
 */
UUIStyle_Combo* UUIComp_UTGlowString::InternalResolve_StringStyle(FUIStyleReference StyleToResolve)
{
	UUIStyle_Combo* Result = NULL;
	UUIObject* OwnerWidget = GetOuterUUIObject();

	UUIStyle* ResolvedStyle = StyleToResolve.GetResolvedStyle( OwnerWidget->GetActiveSkin() );
	if ( ResolvedStyle )
	{
		// retrieve the style data for this menu state
		UUIStyle_Data* StyleData = ResolvedStyle->GetStyleForState( OwnerWidget->GetCurrentState() );

		if ( StyleData )
		{
			Result = Cast<UUIStyle_Combo>(StyleData);
		}
	}

	return Result;
}
#endif

/**
 * We override InternalRender_String so that we can render twice.  Once with the glow style, once with the normal
 * style
 *
 * @param	Canvas		the canvas to use for rendering this string
 * @param	Parameters	The render parameters for this string
 */

void UUIComp_UTGlowString::InternalRender_String( FCanvas* Canvas, FRenderParameters& Parameters )
{
	checkSlow(ValueString);

	if ( ValueString != NULL )
	{
#if 0
		UUIStyle_Combo* DrawStyle = InternalResolve_StringStyle(GlowStyle);

		if ( DrawStyle )
		{
			ValueString->SetStringStyle(DrawStyle);
/*
			if (GIsGame)
			{
				RefreshAppliedStyleData();
			}
*/
			ValueString->Render_String(Canvas, Parameters);
		}

		if ( GIsGame )
		{
			DrawStyle = InternalResolve_StringStyle(StringStyle);
			if ( DrawStyle )
			{
				/*
				ValueString->SetStringStyle(DrawStyle);
				RefreshAppliedStyleData();
				*/

				ValueString->Render_String(Canvas, Parameters);
			}
		}
#else
		UUIObject* OwnerWidget = GetOuterUUIObject();
		UUIState* CurrentMenuState = OwnerWidget->GetCurrentState(OwnerWidget->GetBestPlayerIndex());
		check(CurrentMenuState);

		UUIStyle_Combo* StateStyleData = NULL;

		UUIStyle* DrawStyle = GlowStyle.GetResolvedStyle();
		if ( DrawStyle )
		{
			StateStyleData = Cast<UUIStyle_Combo>(DrawStyle->GetStyleForState(CurrentMenuState));
			if ( StateStyleData != NULL )
			{
				FUICombinedStyleData FinalStyleData(StateStyleData);
				CustomizeAppliedStyle(FinalStyleData);

				if ( ValueString->SetStringStyle(FinalStyleData) )
				{
					ValueString->Render_String(Canvas, Parameters);
				}
			}
		}

		if ( GIsGame )
		{
			DrawStyle = StringStyle.GetResolvedStyle();
			if ( DrawStyle )
			{
				StateStyleData = Cast<UUIStyle_Combo>(DrawStyle->GetStyleForState(CurrentMenuState));
				if ( StateStyleData != NULL )
				{
					FUICombinedStyleData FinalStyleData(StateStyleData);
					CustomizeAppliedStyle(FinalStyleData);

					if ( ValueString->SetStringStyle(FinalStyleData) )
					{
						ValueString->Render_String(Canvas, Parameters);
					}
				}
			}
		}
#endif
	}
}

/*=========================================================================================
  UIComp_DrawTeamColoredImage - Automatically teamcolor an image
  ========================================================================================= */
/**
 * We override RenderComponent to make sure we can adjust the color
 *
 * @param	Canvas		the canvas to render the image to
 * @param	Parameters	the bounds for the region that this texture can render to.
 */
void UUIComp_DrawTeamColoredImage::RenderComponent( FCanvas* Canvas, FRenderParameters Parameters )
{
	if ( ImageRef != NULL && Canvas != NULL )
	{
		// Last Team Color is always the Default
		FLinearColor TeamColor = TeamColors( TeamColors.Num() -1 );

		if ( GIsGame )
		{
			AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
			if (WorldInfo->GRI != NULL && WorldInfo->GRI->GameClass != NULL)
			{
				AGameInfo* GI = Cast<AGameInfo>(GWorld->GetWorldInfo()->GRI->GameClass->GetDefaultActor());
				if (GI != NULL && GI->bTeamGame)
				{
					UUIObject* OwnerWidget = GetOuterUUIObject();
					AUTPlayerController* UTPC = Cast<AUTPlayerController>( OwnerWidget->GetPlayerOwner()->Actor );
					if ( UTPC && UTPC->PlayerReplicationInfo && UTPC->PlayerReplicationInfo->Team )
					{
						INT TeamIndex = UTPC->PlayerReplicationInfo->Team->TeamIndex;
						if ( TeamIndex <= TeamColors.Num() )
						{
							TeamColor = TeamColors( TeamIndex );

							if (UTPC->bPulseTeamColor)
							{
								FLOAT Perc = Abs( appSin( WorldInfo->TimeSeconds * 5 ) );
								TeamColor.R =  TeamColor.R * (2.0 * Perc) + 0.5;
								TeamColor.G =  TeamColor.G * (2.0 * Perc) + 0.5;
								TeamColor.B =  TeamColor.B * (2.0 * Perc) + 0.5;
							}

						}
					}
				}
			}
		}
		else
		{
			if ( EditorTeamIndex < TeamColors.Num() )
			{
				TeamColor = TeamColors( EditorTeamIndex );
			}
		}

		StyleCustomization.SetCustomDrawColor(TeamColor);
		if ( HasValidStyleReference(NULL) )
		{
			RefreshAppliedStyleData();
		}
	}

	Super::RenderComponent(Canvas, Parameters);
}

/*=========================================================================================
	UUTUIMenuList - UT UI FrontEnd Menu List
========================================================================================= */

namespace
{
	const INT MenuListItemPadding = 0;
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
void UUTUIMenuList::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	// Initialize Widget
	Super::Initialize(inOwnerScene, inOwner);


}

/** Populates the simple list's string list. */
void UUTUIMenuList::RegenerateOptions()
{
	InvalidateAllPositions();
	eventEmpty();
	MenuListItems.Empty();

	if ( DataSource.ResolveMarkup(this) )
	{
		DataProvider = DataSource->ResolveListElementProvider(DataSource.DataStoreField.ToString());

		if(DataProvider)
		{
			DataProvider->GetListElements(DataSource.DataStoreField, MenuListItems);

			for(INT ItemIdx=0; ItemIdx<MenuListItems.Num(); ItemIdx++)
			{
				FUIProviderFieldValue OutValue(EC_EventParm);

				if(GetCellFieldValue(this, TEXT("FriendlyName"), MenuListItems(ItemIdx),OutValue))
				{
					eventAddItem(OutValue.StringValue);
				}
				else if(GetCellFieldValue(this, DataSource.DataStoreField, MenuListItems(ItemIdx),OutValue))
				{
					eventAddItem(OutValue.StringValue);
				}
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
void UUTUIMenuList::SetDataStoreBinding( const FString& MarkupText, INT BindingIndex/*=-1*/ )
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
FString UUTUIMenuList::GetDataStoreBinding( INT BindingIndex/*=-1*/ ) const
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
UBOOL UUTUIMenuList::RefreshSubscriberValue( INT BindingIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult;

	if ( DELEGATE_IS_SET(OnRefreshSubscriberValue) && delegateOnRefreshSubscriberValue(this, BindingIndex) )
	{
		bResult = TRUE;
	}
	else if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		bResult = ResolveDefaultDataBinding(BindingIndex);
	}
	else 
	{
		RegenerateOptions();
		bResult = TRUE;
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
void UUTUIMenuList::NotifyDataStoreValueUpdated( UUIDataStore* SourceDataStore, UBOOL bValuesInvalidated, FName PropertyTag, UUIDataProvider* SourceProvider, INT ArrayIndex )
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
void UUTUIMenuList::GetBoundDataStores( TArray<UUIDataStore*>& out_BoundDataStores )
{
	GetDefaultDataStores(out_BoundDataStores);

	// add overall data store to the list
	if ( DataSource )
	{
		out_BoundDataStores.AddUniqueItem(*DataSource);
	}

	if ( SelectedIndexDataSource )
	{
		out_BoundDataStores.AddUniqueItem(*SelectedIndexDataSource);
	}
}

/**
 * Notifies this subscriber to unbind itself from all bound data stores
 */
void UUTUIMenuList::ClearBoundDataStores()
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



/**
 * Gets the cell field value for a specified list and list index.
 *
 * @param InObject		List to get the cell field value for.
 * @param InCellTag		Tag to get the value for.
 * @param InListIndex	Index to get the value for.
 * @param OutValue		Storage variable for the final value.
 */
UBOOL UUTUIMenuList::GetCellFieldValue(UUIObject* InObject, FName InCellTag, INT InListIndex, FUIProviderFieldValue &OutValue)
{
	UBOOL bResult = FALSE;

	UUIList* InList = Cast<UUIList>(InObject);
	if (InList != NULL && InList->DataProvider)
	{
		TScriptInterface<IUIListElementCellProvider> CellProvider = InList->DataProvider->GetElementCellValueProvider(InList->DataSource.DataStoreField, InListIndex);
		if ( CellProvider )
		{
			FUIProviderFieldValue CellValue(EC_EventParm);
			if ( CellProvider->GetCellFieldValue(InList->DataSource.DataStoreField,InCellTag, InListIndex, CellValue) == TRUE )
			{
				OutValue = CellValue;
				bResult = TRUE;
			}
		}
	}
	else 
	{
		UUTUIMenuList* InSimpleList = Cast<UUTUIMenuList>(InObject);
		if (InSimpleList != NULL && InSimpleList->DataProvider)
		{
			TScriptInterface<IUIListElementCellProvider> CellProvider = InSimpleList->DataProvider->GetElementCellValueProvider(InSimpleList->DataSource.DataStoreField, InListIndex);
			if ( CellProvider )
			{
				FUIProviderFieldValue CellValue(EC_EventParm);
				if ( CellProvider->GetCellFieldValue(InSimpleList->DataSource.DataStoreField, InCellTag, InListIndex, CellValue) == TRUE )
				{
					OutValue = CellValue;
					bResult = TRUE;
				}
			}
		}
	}

	return bResult;
}

/** returns the first list index the has the specified value for the specified cell, or INDEX_NONE if it couldn't be found */
INT UUTUIMenuList::FindCellFieldString(UUIObject* InObject, FName InCellTag, const FString& FindValue, UBOOL bCaseSensitive)
{
	INT Result = INDEX_NONE;

	UUIList* InList = Cast<UUIList>(InObject);
	if (InList != NULL && InList->DataProvider)
	{
		FUIProviderFieldValue CellValue(EC_EventParm);
		
		for (INT i = 0; i < InList->Items.Num(); i++)
		{
			TScriptInterface<IUIListElementCellProvider> CellProvider = InList->DataProvider->GetElementCellValueProvider(InList->DataSource.DataStoreField, i);
			if ( CellProvider && CellProvider->GetCellFieldValue(InList->DataSource.DataStoreField, InCellTag, i, CellValue) &&
				(bCaseSensitive ? CellValue.StringValue == FindValue : appStricmp(*CellValue.StringValue, *FindValue) == 0) )
			{
				Result = i;
				break;
			}
		}
	}
	else
	{
		UUTUIMenuList* InMenuList = Cast<UUTUIMenuList>(InObject);
		if (InMenuList != NULL && InMenuList->DataProvider)
		{
			FUIProviderFieldValue CellValue(EC_EventParm);
			TArray<INT> ListItems;
			InMenuList->DataProvider->GetListElements(InMenuList->DataSource.DataStoreField, ListItems);

			for (INT ItemIdx = 0; ItemIdx < ListItems.Num(); ItemIdx++)
			{
				TScriptInterface<IUIListElementCellProvider> CellProvider = InMenuList->DataProvider->GetElementCellValueProvider(InMenuList->DataSource.DataStoreField, ItemIdx);

				if(CellProvider && CellProvider->GetCellFieldValue(InMenuList->DataSource.DataStoreField, InCellTag, ItemIdx, CellValue))
				{
					if (bCaseSensitive ? CellValue.StringValue == FindValue : (appStricmp(*CellValue.StringValue, *FindValue) == 0) )
					{
						Result = ItemIdx;
						break;
					}
				}
			}
		}
	}

	return Result;
}

/*=========================================================================================
	UUIComp_UTUIMenuListPresenter - List presenter for the UTUIMenuList
========================================================================================= */

/**
 * Initializes the component's prefabs.
 */
void UUIComp_UTUIMenuListPresenter::InitializePrefabs()
{
	UUIList* Owner = GetOuterUUIList();
	
	if(Owner)
	{	
		FName SelectedPrefabName = TEXT("SelectedItem_Prefab");

		// Remove all children
		Owner->RemoveChildren(Owner->Children);
		InstancedPrefabs.Empty();

		// @todo: This is here to make old instances of this widget work, will remove before ship.
		if(NormalItemPrefab==NULL)
		{
			NormalItemPrefab=SelectedItemPrefab;
		}

		// @todo: For now, only generate prefabs ingame, change this later.
		if(GIsGame && Owner->DataProvider)
		{
			if(NormalItemPrefab)
			{
				TArray<INT> Elements;
				Owner->DataProvider->GetListElements(Owner->DataSource.DataStoreField, Elements);
				const INT NumItems = Elements.Num();
				const FLOAT AngleDelta = 360.0f / NumItems;
	
				for(INT ItemIdx=0; ItemIdx<NumItems; ItemIdx++)
				{
					FName ItemName = *FString::Printf(TEXT("Item_%i"), ItemIdx);
					UUIPrefabInstance* NewPrefabInstance = NormalItemPrefab->InstancePrefab(Owner, ItemName);

					if(NewPrefabInstance!=NULL)
					{
						// Set some private behavior for the prefab.
						NewPrefabInstance->SetPrivateBehavior(UCONST_PRIVATE_NotEditorSelectable, TRUE);
						NewPrefabInstance->SetPrivateBehavior(UCONST_PRIVATE_TreeHiddenRecursive, TRUE);
						
						// Add the prefab to the list.
						Owner->InsertChild(NewPrefabInstance);

						// Make the entire prefab percentage owner
						NewPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Top, EVALPOS_PercentageOwner);
						NewPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Bottom, EVALPOS_PercentageOwner);
						NewPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Left, EVALPOS_PercentageOwner);
						NewPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Right, EVALPOS_PercentageOwner);



						// Store info about the newly generated prefab.
						INT NewIdx = InstancedPrefabs.AddZeroed(1);
						FInstancedPrefabInfo &Info = InstancedPrefabs(NewIdx);

						Info.PrefabInstance = NewPrefabInstance;

						// Resolve references to all of the objects we will be touching within the prefab.
						Info.ResolvedObjects.AddZeroed(PrefabMarkupReplaceList.Num());
						for(INT ChildIdx=0; ChildIdx<PrefabMarkupReplaceList.Num(); ChildIdx++)
						{
							FName WidgetTag = PrefabMarkupReplaceList(ChildIdx).WidgetTag;
							UUIObject* ResolvedObject = NewPrefabInstance->FindChild(WidgetTag, TRUE);

							Info.ResolvedObjects(ChildIdx) = ResolvedObject;
						}
					}	
				}
			}
		}
	}
}

/**
 * Renders the elements in this list.
 *
 * @param	RI					the render interface to use for rendering
 */
void UUIComp_UTUIMenuListPresenter::Render_List( FCanvas* Canvas )
{
	
}

/**
 * Renders the list element specified.
 *
 * @param	Canvas			the canvas to use for rendering
 * @param	ElementIndex	the index for the list element to render
 * @param	Parameters		Used for various purposes:
 *							DrawX:		[in]	specifies the pixel location of the start of the horizontal bounding region that should be used for
 *												rendering this element
 *										[out]	unused
 *							DrawY:		[in]	specifies the pixel Y location of the bounding region that should be used for rendering this list element.
 *										[out]	Will be set to the Y position of the rendering "pen" after rendering this element.  This is the Y position for rendering
 *												the next element should be rendered
 *							DrawXL:		[in]	specifies the pixel location of the end of the horizontal bounding region that should be used for rendering this element.
 *										[out]	unused
 *							DrawYL:		[in]	specifies the height of the bounding region, in pixels.  If this value is not large enough to render the specified element,
 *												the element will not be rendered.
 *										[out]	Will be reduced by the height of the element that was rendered. Thus represents the "remaining" height available for rendering.
 *							DrawFont:	[in]	specifies the font to use for retrieving the size of the characters in the string
 *							Scale:		[in]	specifies the amount of scaling to apply when rendering the element
 */
void UUIComp_UTUIMenuListPresenter::Render_ListElement( FCanvas* Canvas, INT ElementIndex, FRenderParameters& Parameters )
{
	
}

/**
 * Called when a member property value has been changed in the editor.
 */
void UUIComp_UTUIMenuListPresenter::PostEditChange( UProperty* PropertyThatChanged )
{
	// Reinitialize prefabs if the selected item prefab changed.
	if(PropertyThatChanged->GetName()==TEXT("SelectedItemPrefab"))
	{
		InitializePrefabs();
	}
}

/**
 * Updates the prefab widgets we are dynamically changing the markup of to use a new list row for their datasource.
 *
 * @param NewIndex	New list row index to rebind the widgets with.
 */
void UUIComp_UTUIMenuListPresenter::UpdatePrefabMarkup()
{
	UUIList* Owner = GetOuterUUIList();

	if(Owner)
	{
		TArray<INT> CurrentListItems;
		Owner->DataProvider->GetListElements(Owner->DataSource.DataStoreField, CurrentListItems);

		for(INT PrefabIdx=0; PrefabIdx<InstancedPrefabs.Num(); PrefabIdx++)
		{
			FInstancedPrefabInfo &Info = InstancedPrefabs(PrefabIdx);
			UUIPrefabInstance* PrefabInstance = Info.PrefabInstance;

			for(INT ChildIdx=0; ChildIdx<PrefabMarkupReplaceList.Num(); ChildIdx++)
			{
				if(Info.ResolvedObjects.IsValidIndex(ChildIdx))
				{
					UUIObject* ResolvedObject = Info.ResolvedObjects(ChildIdx);
					if(ResolvedObject && CurrentListItems.IsValidIndex(PrefabIdx))
					{
						INT DataSourceIndex = CurrentListItems(PrefabIdx);

						// get the UIListElementCellProvider for the specified element
						TScriptInterface<IUIListElementCellProvider> CellProvider = Owner->DataProvider->GetElementCellValueProvider(Owner->DataSource.DataStoreField, DataSourceIndex);
						if ( CellProvider )
						{
							FName CellFieldName = PrefabMarkupReplaceList(ChildIdx).CellTag;
							FUIProviderFieldValue CellValue(EC_EventParm);
							if ( CellProvider->GetCellFieldValue(Owner->DataSource.DataStoreField, CellFieldName, DataSourceIndex, CellValue) == TRUE )
							{
								// if the cell provider is a UIDataProvider, generate the markup string required to access this property
								UUIDataProvider* CellProviderObject = Cast<UUIDataProvider>(Owner->DataProvider.GetObject());
								if ( CellProviderObject != NULL )
								{
									// get the data field path to the collection data field that the list is bound to...
									FString CellFieldMarkup;
									FString DataSourceMarkup = CellProviderObject->BuildDataFieldPath(*Owner->DataSource, Owner->DataSource.DataStoreField);
									if ( DataSourceMarkup.Len() != 0 )
									{
										// then append the list index and the cell name
										CellFieldMarkup = FString::Printf(TEXT("<%s;%i.%s>"), *DataSourceMarkup, DataSourceIndex, *CellFieldName.ToString());
									}
									else
									{
										// if the CellProviderObject doesn't support the data field that the list is bound to, the CellProviderObject is an internal provider of
										// the data store that the list is bound to.  allow the data provider to generate its own markup by passing in the CellFieldName
										CellFieldMarkup = CellProviderObject->GenerateDataMarkupString(*Owner->DataSource,CellFieldName);
									}

									IUIDataStoreSubscriber* Subscriber = static_cast<IUIDataStoreSubscriber*>(ResolvedObject->GetInterfaceAddress(IUIDataStoreSubscriber::UClassType::StaticClass()));
									if ( Subscriber != NULL )
									{
										Subscriber->SetDataStoreBinding(CellFieldMarkup);
									}
								}		
							}
						}
					}
				}
			}
		}
	}
}

/** Updates the position of the selected item prefab. */
void UUIComp_UTUIMenuListPresenter::UpdatePrefabPosition()
{
	UUTUIMenuList* Owner = Cast<UUTUIMenuList>(GetOuterUUIList());

	if(Owner)
	{
		const FLOAT RotationTime = 0.125f;
		const FLOAT ZRadius = 500.0f;
		const INT SelectedIndex = Owner->Selection;
		
		FLOAT ItemY = 0.0f;

		for(INT PrefabIdx=0; PrefabIdx<InstancedPrefabs.Num(); PrefabIdx++)
		{
			FInstancedPrefabInfo &Info = InstancedPrefabs(PrefabIdx);
			UUIPrefabInstance* PrefabInstance = Info.PrefabInstance;

			FLOAT Distance = SelectedIndex - PrefabIdx;

			// If we are changing items, then smoothly move to the new selection item.
			if(Owner->bIsRotating)
			{
				FLOAT TimeElapsed = GWorld->GetRealTimeSeconds() - Owner->StartRotationTime;
				if(TimeElapsed > RotationTime)
				{
					Owner->bIsRotating = FALSE;
					TimeElapsed = RotationTime;
				}

				// Used to interpolate the scaling to create a smooth looking selection effect.
				const FLOAT Diff = (SelectedIndex - Owner->OldSelection);
				FLOAT Alpha = 1.0f - (TimeElapsed / RotationTime);

/*
				// Smooth out the interpolation
				if (Alpha < 0.5f) 
				{
					Alpha *= 2.0f;
					Alpha = appPow(Alpha,3.0f);
					Alpha /= 2.0f;
				}
				else
				{
					Alpha = (1.0f-Alpha);
					Alpha *= 2.0f;
					Alpha = appPow(Alpha,3.0f);
					Alpha /= 2.0f;
					Alpha = (1.0f - Alpha);
				}
*/

				Distance += -Diff*Alpha;
			}

			// Set the item's position
			const INT ItemRange = 2;
			Distance = Clamp<FLOAT>(Distance, -ItemRange, ItemRange);
			Distance /= ItemRange;
			Distance *= (PI / 2.0f);

			const FLOAT ScaleFactor = Max<FLOAT>(appCos(Distance),0.5f);
			const FLOAT ItemHeight = SelectedItemHeight*ScaleFactor;

			PrefabInstance->Opacity = Max<FLOAT>(appCos(Distance),0.5f);
			PrefabInstance->Opacity *= PrefabInstance->Opacity;

			PrefabInstance->SetPosition(ItemY, UIFACE_Top, EVALPOS_PixelOwner);
			PrefabInstance->SetPosition(PrefabInstance->GetPosition(UIFACE_Top, EVALPOS_PixelViewport) + ItemHeight, UIFACE_Bottom, EVALPOS_PixelViewport);
			
			PrefabInstance->SetPosition(0.0f, UIFACE_Left, EVALPOS_PercentageOwner);
			PrefabInstance->SetPosition(1.0f, UIFACE_Right, EVALPOS_PercentageOwner);

			ItemY += ItemHeight + MenuListItemPadding;
		}


		Owner->RequestSceneUpdate(FALSE,TRUE,FALSE,TRUE);
	}
}


/*=========================================================================================
	UUTUICollectionCheckBox - Checkbox subclass that works on collection datasources.
========================================================================================= */
IMPLEMENT_CLASS(UUTUICollectionCheckBox);


void UUTUICollectionCheckBox::ResolveListElementProvider()
{
	TScriptInterface<IUIListElementProvider> Result;
	if ( ValueDataSource.ResolveMarkup( this ) )
	{
		DataProvider = ValueDataSource->ResolveListElementProvider(ValueDataSource.DataStoreField.ToString());
	}
}

/** @return Returns the number of possible values for the field we are bound to. */
INT UUTUICollectionCheckBox::GetNumValues()
{
	INT NumValues = 0;

	if(DataProvider)
	{
		NumValues = DataProvider->GetElementCount(ValueDataSource.DataStoreField);
	}
	
	return NumValues;
}

/** 
 * @param Index		List index to get the value of.
 *
 * @return Returns the number of possible values for the field we are bound to. 
 */
UBOOL UUTUICollectionCheckBox::GetListValue(INT ListIndex, FString &OutValue)
{
	UBOOL bResult = FALSE;

	if(DataProvider)
	{
		FUIProviderFieldValue FieldValue;
		appMemzero(&FieldValue, sizeof(FUIProviderFieldValue));

		TScriptInterface<class IUIListElementCellProvider> ValueProvider = DataProvider->GetElementCellValueProvider(ValueDataSource.DataStoreField, ListIndex);

		if(ValueProvider)
		{
			bResult = ValueProvider->GetCellFieldValue(ValueDataSource.DataStoreField, ValueDataSource.DataStoreField, ListIndex, FieldValue);

			if(bResult)
			{
				OutValue = FieldValue.StringValue;
			}
		}
	}

	return bResult;
}	



/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
UBOOL UUTUICollectionCheckBox::RefreshSubscriberValue( INT BindingIndex/*=INDEX_NONE*/ )
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
	else if ( ValueDataSource.ResolveMarkup( this ) )
	{
		ResolveListElementProvider();

		FUIProviderFieldValue ResolvedValue(EC_EventParm);
		if ( ValueDataSource.GetBindingValue(ResolvedValue)
		&&	(ResolvedValue.StringValue.Len() > 0 || ResolvedValue.ArrayValue.Num() > 0) )
		{
			FString CheckValue;

			if ( ResolvedValue.ArrayValue.Num() > 0 )
			{
				SetValue( ResolvedValue.ArrayValue(0) == 1, GetBestPlayerIndex() );
			}
			else
			{
				// Get the 2nd element of the list and see if it matches the string value, if so, we are checked.
				if(GetListValue(1,CheckValue) && CheckValue==ResolvedValue.StringValue)
				{
					SetValue(TRUE);
				}
				else
				{
					SetValue(FALSE);
				}
			}

			InvalidateAllPositions();
			bResult=TRUE;
		}
	}


	return bResult;
}

/**
 * Resolves this subscriber's data store binding and publishes this subscriber's value to the appropriate data store.
 *
 * @param	out_BoundDataStores	contains the array of data stores that widgets have saved values to.  Each widget that
 *								implements this method should add its resolved data store to this array after data values have been
 *								published.  Once SaveSubscriberValue has been called on all widgets in a scene, OnCommit will be called
 *								on all data stores in this array.
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	TRUE if the value was successfully published to the data store.
 */
UBOOL UUTUICollectionCheckBox::SaveSubscriberValue( TArray<UUIDataStore*>& out_BoundDataStores, INT BindingIndex/*=INDEX_NONE*/ )
{
	FUIProviderScriptFieldValue Value(EC_EventParm);
	Value.PropertyTag = ValueDataSource.DataStoreField;
	Value.ArrayValue.AddItem(bIsChecked);
	GetListValue(bIsChecked, Value.StringValue);

	GetBoundDataStores(out_BoundDataStores);
	const UBOOL bResult = ValueDataSource.SetBindingValue(Value);
	return bResult;
}


/**
 * Changed the checked state of this checkbox and activates a checked event.
 *
 * @param	bShouldBeChecked	TRUE to turn the checkbox on, FALSE to turn it off
 * @param	PlayerIndex			the index of the player that generated the call to SetValue; used as the PlayerIndex when activating
 *								UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 */
void UUTUICollectionCheckBox::SetValue( UBOOL bShouldBeChecked, INT PlayerIndex/*=INDEX_NONE*/ )
{
	Super::SetValue(bShouldBeChecked, PlayerIndex);
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
void UUTUICollectionCheckBox::NotifyDataStoreValueUpdated( class UUIDataStore* SourceDataStore, UBOOL bValuesInvalidated, FName PropertyTag, class UUIDataProvider* SourceProvider, INT ArrayIndex )
{
	const UBOOL bBoundToDataStore = (SourceDataStore == ValueDataSource.ResolvedDataStore && (PropertyTag == NAME_None || PropertyTag == ValueDataSource.DataStoreField));
	LOG_DATAFIELD_UPDATE(SourceDataStore,bValuesInvalidated,PropertyTag,SourceProvider,ArrayIndex);


	if ( bBoundToDataStore )
	{
		FUIProviderFieldValue ResolvedValue(EC_EventParm);
		if ( ValueDataSource.GetBindingValue(ResolvedValue) && ResolvedValue.StringValue.Len() > 0 )
		{
			FString CheckValue;

			// Get the 2nd element of the list and see if it matches the string value, if so, we are checked.
			if ( !DataProvider )
			{
				ResolveListElementProvider();
			}

			if(GetListValue(1,CheckValue) && CheckValue==ResolvedValue.StringValue)
			{
				Super::SetValue(TRUE);
			}
			else
			{
				Super::SetValue(FALSE);
			}
		}
	}
}


/*=========================================================================================
	UUTUIEditBox - UT specific version of the editbox widget
========================================================================================= */
IMPLEMENT_CLASS(UUTUIEditBox);


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
void UUTUIEditBox::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);

	// Change the editbox background style from just being the default image style.
	BackgroundImageComponent->ImageStyle.DefaultStyleTag = TEXT("UTEditBoxBackground");
	RequestSceneUpdate(FALSE,FALSE,FALSE,TRUE);
}


/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @param	BindingIndex		indicates which data store binding should be modified.  Valid values and their meanings are:
 *									-1:	all data sources
 *									0:	list data source
 *									1:	caption data source
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
UBOOL UUTUIEditBox::RefreshSubscriberValue( INT BindingIndex/*=INDEX_NONE*/ )
{
	return Super::RefreshSubscriberValue(BindingIndex);
}


/**
 * Resolves this subscriber's data store binding and publishes this subscriber's value to the appropriate data store.
 *
 * @param	out_BoundDataStores	contains the array of data stores that widgets have saved values to.  Each widget that
 *								implements this method should add its resolved data store to this array after data values have been
 *								published.  Once SaveSubscriberValue has been called on all widgets in a scene, OnCommit will be called
 *								on all data stores in this array.
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	TRUE if the value was successfully published to the data store.
 */
UBOOL UUTUIEditBox::SaveSubscriberValue( TArray<UUIDataStore*>& out_BoundDataStores, INT BindingIndex/*=INDEX_NONE*/ )
{
	return Super::SaveSubscriberValue(out_BoundDataStores, BindingIndex);
}

/*=========================================================================================
	UUTUIList  - UT specific version of the list widget
========================================================================================= */
/* === UIObject interface === */
/**
 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
 *
 * This version adds the BackgroundImageComponent (if non-NULL) to the StyleSubscribers array.
 */
void UUTUIList::InitializeStyleSubscribers()
{
	Super::InitializeStyleSubscribers();

	VALIDATE_COMPONENT(BackgroundImageComponent);
	AddStyleSubscriber(BackgroundImageComponent);
}

/* === UUIScreenObject interface === */
/**
 * Render this widget.  This version routes the render call to the draw components, if applicable.
 *
 * @param	Canvas	the canvas to use for rendering this widget
 */
void UUTUIList::Render_Widget( FCanvas* Canvas )
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

	Super::Render_Widget(Canvas);
}

/* === UObject interface === */
/**
 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
 */
void UUTUIList::PreEditChange( FEditPropertyChain& PropertyThatChanged )
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

				// if the value of the StringRenderComponent itself was changed
				if ( MemberProperty == ModifiedProperty && BackgroundImageComponent != NULL )
				{
					// the user either cleared the value of the BackgroundRenderComponent or is assigning a new value to it.
					// Unsubscribe the current component from our list of style resolvers.
					RemoveStyleSubscriber(BackgroundImageComponent);
				}
			}
		}
	}
}

/**
 * Called when a property value from a member struct or array has been changed in the editor.
 */
void UUTUIList::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UBOOL bHandled = FALSE;
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			const FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("BackgroundImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundRenderComponent itself was changed
				if ( MemberProperty == ModifiedProperty )
				{
					if ( BackgroundImageComponent != NULL )
					{
						// the newly created image component won't have the correct StyleResolverTag, so fix that now
						UUIComp_DrawImage* BackgroundComponentTemplate = GetArchetype<UUTUIList>()->BackgroundImageComponent;
						if ( BackgroundComponentTemplate != NULL )
						{
							BackgroundImageComponent->StyleResolverTag = BackgroundComponentTemplate->StyleResolverTag;
						}
						else
						{
							BackgroundImageComponent->StyleResolverTag = TEXT("Background Image");
						}

						// user created a new caption background component - add it to the list of style subscribers
						AddStyleSubscriber(BackgroundImageComponent);

						// finally initialize the component's image
						BackgroundImageComponent->SetImage(BackgroundImageComponent->GetImage());
					}
				}
			}
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Resolves this subscriber's data store binding and publishes this subscriber's value to the appropriate data store.
 *
 * @param	out_BoundDataStores	contains the array of data stores that widgets have saved values to.  Each widget that
 *								implements this method should add its resolved data store to this array after data values have been
 *								published.  Once SaveSubscriberValue has been called on all widgets in a scene, OnCommit will be called
 *								on all data stores in this array.
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	TRUE if the value was successfully published to the data store.
 */
UBOOL UUTUIList::SaveSubscriberValue( TArray<UUIDataStore*>& out_BoundDataStores, INT BindingIndex/*=INDEX_NONE*/ )
{
	if(bAllowSaving)
	{
		return Super::SaveSubscriberValue(out_BoundDataStores, BindingIndex);
	}
	else
	{
		GetBoundDataStores(out_BoundDataStores);
		return FALSE;
	}
}


/*=========================================================================================
	UUTUIComboBox - UT specific version of the combobox widget
========================================================================================= */
/**
 * Called whenever the selected item is modified.  Activates the SliderValueChanged kismet event and calls the OnValueChanged
 * delegate.
 *
 * @param	PlayerIndex		the index of the player that generated the call to this method; used as the PlayerIndex when activating
 *							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 * @param	NotifyFlags		optional parameter for individual widgets to use for passing additional information about the notification.
 */
void UUTUIComboBox::NotifyValueChanged( INT PlayerIndex, INT NotifyFlags )
{
	Super::NotifyValueChanged(PlayerIndex, NotifyFlags);

	if((NotifyFlags&UCONST_INDEX_CHANGED_NOTIFY_MASK)==UCONST_INDEX_CHANGED_NOTIFY_MASK)
	{
		TArray<UUIDataStore*> BoundDataStores;
		SaveSubscriberValue(BoundDataStores);
	}
}


/* === IUIDataStoreSubscriber interface === */
/**
 * Sets the data store binding for this object to the text specified.
 *
 * @param	MarkupText			a markup string which resolves to data exposed by a data store.  The expected format is:
 *								<DataStoreTag:DataFieldTag>
 * @param	BindingIndex		indicates which data store binding should be modified.  Valid values and their meanings are:
 *									0:	list data source
 *									1:	caption data source
 */
void UUTUIComboBox::SetDataStoreBinding( const FString& MarkupText, INT BindingIndex/*=INDEX_NONE*/ )
{
	Super::SetDataStoreBinding(MarkupText, BindingIndex);
}

/**
 * Retrieves the markup string corresponding to the data store that this object is bound to.
 *
 * @param	BindingIndex		indicates which data store binding should be modified.  Valid values and their meanings are:
 *									0:	list data source
 *									1:	caption data source
 *
 * @return	a datastore markup string which resolves to the datastore field that this object is bound to, in the format:
 *			<DataStoreTag:DataFieldTag>
 */
FString UUTUIComboBox::GetDataStoreBinding( INT BindingIndex/*=INDEX_NONE*/ ) const
{
	return Super::GetDataStoreBinding(BindingIndex);
}

/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @param	BindingIndex		indicates which data store binding should be modified.  Valid values and their meanings are:
 *									-1:	all data sources
 *									0:	list data source
 *									1:	caption data source
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
UBOOL UUTUIComboBox::RefreshSubscriberValue( INT BindingIndex/*=INDEX_NONE*/ )
{
	if ( DELEGATE_IS_SET(OnRefreshSubscriberValue) && delegateOnRefreshSubscriberValue(this, BindingIndex) )
	{
		return TRUE;
	}
	else if(BindingIndex==INDEX_NONE)
	{
		if ( ComboList != NULL && ComboEditbox != NULL )
		{
			FUIProviderFieldValue FieldValue(EC_EventParm);
			if(GetDataStoreFieldValue(ComboList->GetDataStoreBinding(), FieldValue, GetScene(), GetPlayerOwner()))
			{
				ComboEditbox->SetValue(FieldValue.StringValue, GetBestPlayerIndex(), TRUE);
				if ( FieldValue.ArrayValue.Num() > 0 )
				{
					ComboList->SetIndex(FieldValue.ArrayValue(0), TRUE, TRUE);
				}
				else
				{
					ComboList->SetIndex(ComboList->FindItemIndex(FieldValue.StringValue), FALSE, TRUE);
				}
			}
		}
		InvalidateAllPositions();
		return TRUE;
	}
	else
	{
		return Super::RefreshSubscriberValue(BindingIndex);
	}
}


/**
 * Resolves this subscriber's data store binding and publishes this subscriber's value to the appropriate data store.
 *
 * @param	out_BoundDataStores	contains the array of data stores that widgets have saved values to.  Each widget that
 *								implements this method should add its resolved data store to this array after data values have been
 *								published.  Once SaveSubscriberValue has been called on all widgets in a scene, OnCommit will be called
 *								on all data stores in this array.
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	TRUE if the value was successfully published to the data store.
 */
UBOOL UUTUIComboBox::SaveSubscriberValue( TArray<UUIDataStore*>& out_BoundDataStores, INT BindingIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( ComboEditbox != NULL && ComboList != NULL && !Cast<UUTUIList>(ComboList)->bAllowSaving )
	{
		// Send the current value the datastore.
		FUIProviderFieldValue FieldValue(EC_EventParm);
		FieldValue.StringValue = ComboEditbox->GetValue();

		if ( ComboList->Items.IsValidIndex(ComboList->Index) )
		{
			FieldValue.ArrayValue.AddItem(ComboList->Items(ComboList->Index));
		}
	 
		GetBoundDataStores(out_BoundDataStores);
		bResult = SetDataStoreFieldValue(ComboList->GetDataStoreBinding(), FieldValue, GetScene(), GetPlayerOwner());
	}

	return bResult;
}


/**
 * Retrieves the list of data stores bound by this subscriber.
 *
 * @param	out_BoundDataStores		receives the array of data stores that subscriber is bound to.
 */
void UUTUIComboBox::GetBoundDataStores( TArray<UUIDataStore*>& out_BoundDataStores )
{
	Super::GetBoundDataStores(out_BoundDataStores);
}

/**
 * Notifies this subscriber to unbind itself from all bound data stores
 */
void UUTUIComboBox::ClearBoundDataStores()
{
	Super::ClearBoundDataStores();
}


/** Initializes styles for child widgets. */
void UUTUIComboBox::SetupChildStyles()
{
	UUISkin* ActiveSkin = GetActiveSkin();
	if ( ComboButton != NULL )
	{
		ComboButton->SetDataStoreBinding(TEXT(""));
		
		if ( ActiveSkin != NULL )
		{
			UUIStyle* Style = ActiveSkin->FindStyle(ToggleButtonStyleName);
			if ( Style != NULL && ComboButton->BackgroundImageComponent != NULL )
			{
				ComboButton->BackgroundImageComponent->ImageStyle.SetStyle(Style);
			}

			Style = ActiveSkin->FindStyle(ToggleButtonCheckedStyleName);
			if ( Style != NULL && ComboButton->CheckedBackgroundImageComponent != NULL )
			{
				ComboButton->CheckedBackgroundImageComponent->ImageStyle.SetStyle(Style);
			}
		}
	}

	if ( ComboList != NULL && ActiveSkin != NULL )
	{
		//@todo - why is this hardcoded?
		UUIStyle* Style = ActiveSkin->FindStyle(TEXT("DropdownMenuContent"));
		if ( Style != NULL )
		{
			ComboList->ItemOverlayStyle[ELEMENT_Normal].SetStyle(Style);
		}

		UUTUIList* UTComboList = Cast<UUTUIList>(ComboList);
		if ( UTComboList != NULL )
		{
			Style = ActiveSkin->FindStyle(ListBackgroundStyleName);
			if ( UTComboList->BackgroundImageComponent == NULL )
			{
				// BackgroundImageComponent was added to UTUIList after existing instances were created, so this block
				// of code creates background image components for previously existing combo box lists, or lists of combo boxes
				// that are created at runtime.
				UUIComp_DrawImage* BackgroundComponentTemplate = UTComboList->GetArchetype<UUTUIList>()->BackgroundImageComponent;
				UUIComp_DrawImage* BackgroundImage = ConstructObject<UUIComp_DrawImage>(
					UUIComp_DrawImage::StaticClass(), UTComboList,
					BackgroundComponentTemplate ? BackgroundComponentTemplate->GetFName() : NAME_None,
					UTComboList->GetMaskedFlags(RF_PropagateToSubObjects),
					BackgroundComponentTemplate, UTComboList
					);

				UTComboList->BackgroundImageComponent = BackgroundImage;
				BackgroundImage->StyleResolverTag = TEXT("Background Image");

				// user created a new background component - add it to the list of style subscribers
				UTComboList->AddStyleSubscriber(BackgroundImage);

				// finally initialize the component's image
				BackgroundImage->SetImage(BackgroundImage->GetImage());
			}

			if ( Style != NULL && UTComboList->BackgroundImageComponent != NULL )
			{
				UTComboList->BackgroundImageComponent->ImageStyle.SetStyle(Style);
			}
		}
	}

	if ( ComboEditbox != NULL )
	{
		if ( ActiveSkin != NULL )
		{
			UUIStyle* Style = ActiveSkin->FindStyle(EditboxBGStyleName);
			if ( Style != NULL && ComboEditbox->BackgroundImageComponent != NULL )
			{
				ComboEditbox->BackgroundImageComponent->ImageStyle.SetStyle(Style);
			}

			Style = ActiveSkin->FindStyle(TEXT("UTComboBoxEditboxStyle"));
			if ( Style != NULL && ComboEditbox->StringRenderComponent != NULL )
			{
				ComboEditbox->StringRenderComponent->StringStyle.SetStyle(Style);
			}
		}

		if ( ComboList != NULL )
		{
			// Get the default value for the edit box.
			FUIProviderFieldValue FieldValue(EC_EventParm);
			if(GetDataStoreFieldValue(ComboList->GetDataStoreBinding(), FieldValue) && FieldValue.HasValue())
			{
				ComboEditbox->SetDataStoreBinding(FieldValue.StringValue);
			}
		}
	}
}


/*=========================================================================================
  UUTDrawPanel - A panel that gives normal Canvas drawing functions to script
  ========================================================================================= */

void UUTDrawPanel::PostRender_Widget( FCanvas* NewCanvas )
{
	Super::PostRender_Widget(NewCanvas);

	// Create a temporary canvas if there isn't already one.
	UCanvas* CanvasObject = FindObject<UCanvas>(UObject::GetTransientPackage(),TEXT("CanvasObject"));
	if( !CanvasObject )
	{
		CanvasObject = ConstructObject<UCanvas>(UCanvas::StaticClass(),UObject::GetTransientPackage(),TEXT("CanvasObject"));
		CanvasObject->AddToRoot();
	}
	if (CanvasObject)
	{
		CanvasObject->Canvas = NewCanvas;

		CanvasObject->Init();

		FVector2D ViewportSize;
		GetViewportSize(ViewportSize);

		ResolutionScale = ViewportSize.Y / 768;

		pLeft	= GetPosition(UIFACE_Left, EVALPOS_PixelViewport);
		pTop	= GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
		pWidth	= GetPosition(UIFACE_Right, EVALPOS_PixelViewport) - pLeft;
		pHeight	= GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport) - pTop;

		CanvasObject->SizeX = (bUseFullViewport) ? appTrunc( ViewportSize.X ) : appTrunc( pWidth ); 
		CanvasObject->SizeY = (bUseFullViewport) ? appTrunc( ViewportSize.Y ) : appTrunc( pHeight ); 

		CanvasObject->SceneView = NULL;
		CanvasObject->Update();

		CanvasObject->OrgX = (bUseFullViewport) ? 0 : pLeft; 
		CanvasObject->OrgY = (bUseFullViewport) ? 0 : pTop; 

		if ( !DELEGATE_IS_SET(DrawDelegate) || !delegateDrawDelegate( CanvasObject) )
		{
			Canvas = CanvasObject;
			eventDrawPanel();
			Canvas = NULL;
		}

		CanvasObject->SizeX = appTrunc(ViewportSize.X);
		CanvasObject->SizeY = appTrunc(ViewportSize.Y);
		CanvasObject->OrgX = 0.0;
		CanvasObject->OrgY = 0.0;
		CanvasObject->Init();
		CanvasObject->Update();
	}
}

void UUTDrawPanel::Draw2DLine(INT X1,INT Y1,INT X2,INT Y2,FColor LineColor)
{
	check(Canvas);
	DrawLine2D(Canvas->Canvas, FVector2D(Canvas->OrgX+X1, Canvas->OrgY+Y1), FVector2D(Canvas->OrgX+X2, Canvas->OrgY+Y2), LineColor);
}

/*=========================================================================================
	UUTUIButtonBar
========================================================================================= */
/**
 * Determines whether this widget can become the focused control.
 *
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player to check focus availability
 *
 * @return	TRUE if this widget (or any of its children) is capable of becoming the focused control.
 */
UBOOL UUTUIButtonBar::CanAcceptFocus( INT PlayerIndex ) const
{
#if CONSOLE
	return FALSE;
#else
	return Super::CanAcceptFocus(PlayerIndex);
#endif
}


/*=========================================================================================
	UUTUIButtonBarButton
========================================================================================= */
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
void UUTUIButtonBarButton::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	// Make sure this button isnt focusable, mouseoverable, or pressable on console.
#if CONSOLE
	DefaultStates.RemoveItem(UUIState_Focused::StaticClass());
	DefaultStates.RemoveItem(UUIState_Active::StaticClass());
	DefaultStates.RemoveItem(UUIState_Pressed::StaticClass());

	for(INT StateIdx=0; StateIdx<InactiveStates.Num(); StateIdx++)
	{
		if(InactiveStates(StateIdx)->IsA(UUIState_Focused::StaticClass()) || InactiveStates(StateIdx)->IsA(UUIState_Active::StaticClass()) || InactiveStates(StateIdx)->IsA(UUIState_Pressed::StaticClass()))
		{
			InactiveStates.Remove(StateIdx);
			StateIdx--;
		}
	}

	if ( BackgroundImageComponent != NULL )
	{
		BackgroundImageComponent->ImageStyle.DefaultStyleTag = TEXT("UTButtonBarButtonBG");
	}
#else
	if ( BackgroundImageComponent != NULL )
	{
		BackgroundImageComponent->ImageStyle.DefaultStyleTag = TEXT("UTButtonBarButtonBG_PC");
	}
#endif

	Super::Initialize(inOwnerScene, inOwner);
}
/**
 * Determines whether this widget can become the focused control.
 *
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player to check focus availability
 *
 * @return	TRUE if this widget (or any of its children) is capable of becoming the focused control.
 */
UBOOL UUTUIButtonBarButton::CanAcceptFocus( INT PlayerIndex ) const
{
#if CONSOLE
	return FALSE;
#else
	return Super::CanAcceptFocus(PlayerIndex);
#endif
}

/*=========================================================================================
	UUIComp_UTDrawStateImage
========================================================================================= */
IMPLEMENT_CLASS(UUIComp_UTDrawStateImage)

/**
 * Applies the current style data (including any style data customization which might be enabled) to the component's image.
 */
void UUIComp_UTDrawStateImage::RefreshAppliedStyleData()
{
	if ( ImageRef == NULL )
	{
		// we have no image if we've never been assigned a value...if this is the case, create one
		// so that the style's DefaultImage will be rendererd
		SetImage(NULL);
	}
	else
	{
		// get the style data that should be applied to the image
		UUIStyle_Image* ImageStyleData = GetAppliedImageStyle(ImageState->GetDefaultObject<UUIState>());

		// ImageStyleData will be NULL if this component has never resolved its style
		if ( ImageStyleData != NULL )
		{
			// apply this component's per-instance image style settings
			FUICombinedStyleData FinalStyleData(ImageStyleData);
			CustomizeAppliedStyle(FinalStyleData);

			// apply the style data to the image
			ImageRef->SetImageStyle(FinalStyleData);
		}
	}
}

/*=========================================================================================
	UUTUISlider
========================================================================================= */
IMPLEMENT_CLASS(UUTUISlider)

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
void UUTUISlider::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);

	if(CaptionRenderComponent)
	{
		CaptionRenderComponent->StringStyle.DefaultStyleTag = TEXT("UTSliderText");
		CaptionRenderComponent->SetAutoScaling(UIAUTOSCALE_Justified);
		CaptionRenderComponent->EnableSubregion(UIORIENT_Horizontal);
		CaptionRenderComponent->EnableSubregion(UIORIENT_Vertical);

		UpdateCaption();
	}
}

/**
 * Render this button.
 *
 * @param	Canvas	the FCanvas to use for rendering this widget
 */
void UUTUISlider::Render_Widget( FCanvas* Canvas )
{
	FRenderParameters Parameters(
		RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top],
		RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left],
		RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top],
		NULL, GetViewportHeight()
	);

	FVector2D ViewportSize;
	GetViewportSize(ViewportSize);
	FLOAT ScaleFactor = ViewportSize.Y / 768.0f;  // We use 1024x768 as our base res for pixel values.

	// first, render the slider background
	if ( BackgroundImageComponent != NULL )
	{
		BackgroundImageComponent->RenderComponent(Canvas, Parameters);
	}

	// next, render the slider bar
	if ( SliderBarImageComponent != NULL )
	{
		FRenderParameters BarRenderParameters = Parameters;
		if ( SliderOrientation == UIORIENT_Horizontal )
		{
			BarRenderParameters.DrawXL = GetMarkerPosition() - BarRenderParameters.DrawX + MarkerWidth.GetValue(this) / 2.0f;
			BarRenderParameters.DrawYL = BarSize.GetValue(this);
			BarRenderParameters.DrawY += (Parameters.DrawYL - BarRenderParameters.DrawYL) / 2;
		}
		else
		{
			BarRenderParameters.DrawYL = GetMarkerPosition() - BarRenderParameters.DrawY;
		}

		SliderBarImageComponent->RenderComponent(Canvas, BarRenderParameters);
	}

	// next, render the marker 
	if ( MarkerImageComponent != NULL )
	{
		FLOAT MarkerPosition = GetMarkerPosition();
		FRenderParameters MarkerRenderParameters(GetViewportHeight());

		if ( SliderOrientation == UIORIENT_Horizontal )
		{
			MarkerRenderParameters.DrawX = MarkerPosition;
			MarkerRenderParameters.DrawY = RenderBounds[UIFACE_Top];
			MarkerRenderParameters.DrawXL = MarkerWidth.GetValue(this);
			MarkerRenderParameters.DrawYL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];

			MarkerRenderParameters.ImageExtent.Y = MarkerHeight.GetValue(this);
		}
		else
		{
			FLOAT ActualMarkerHeight = MarkerHeight.GetValue(this);

			MarkerRenderParameters.DrawX = RenderBounds[UIFACE_Left];
			MarkerRenderParameters.DrawY = MarkerPosition + ActualMarkerHeight;
			MarkerRenderParameters.DrawXL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
			MarkerRenderParameters.DrawYL = ActualMarkerHeight;

			MarkerRenderParameters.ImageExtent.X = MarkerWidth.GetValue(this);
		}

		MarkerImageComponent->RenderComponent(Canvas, MarkerRenderParameters);
	}

	// Render the slider caption.
	if(CaptionRenderComponent != NULL)
	{
		FLOAT VerticalTextPadding=8.0f*ScaleFactor;
		FLOAT HorizontalTextPadding = 3.0f*ScaleFactor;
		Canvas->PushRelativeTransform(FTranslationMatrix(FVector(-Parameters.DrawXL-HorizontalTextPadding,VerticalTextPadding/2.0f,0)));
		{	
			//CaptionRenderComponent->SetSubregionOffset(UIORIENT_Horizontal, 0, EVALPOS_PixelViewport);
			//CaptionRenderComponent->SetSubregionOffset(UIORIENT_Vertical, 0, EVALPOS_PixelViewport);
			CaptionRenderComponent->SetSubregionSize(UIORIENT_Horizontal, Parameters.DrawYL, UIEXTENTEVAL_Pixels);
			CaptionRenderComponent->SetSubregionSize(UIORIENT_Vertical, Parameters.DrawYL-VerticalTextPadding, UIEXTENTEVAL_Pixels);
			CaptionRenderComponent->Render_String(Canvas);
		}
		Canvas->PopTransform();
	}

}


/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
UBOOL UUTUISlider::RefreshSubscriberValue( INT BindingIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	bResult = Super::RefreshSubscriberValue(BindingIndex);

	if ( bResult && (BindingIndex == INDEX_NONE || BindingIndex == DataSource.BindingIndex) )
	{
		UpdateCaption();
	}

	return bResult;
}

/**
 * Called whenever the value of the slider is modified.  Activates the SliderValueChanged kismet event and calls the OnValueChanged
 * delegate.
 *
 * @param	PlayerIndex		the index of the player that generated the call to this method; used as the PlayerIndex when activating
 *							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 * @param	NotifyFlags		optional parameter for individual widgets to use for passing additional information about the notification.
 */
void UUTUISlider::NotifyValueChanged( INT PlayerIndex, INT NotifyFlags )
{
	Super::NotifyValueChanged(PlayerIndex, NotifyFlags);

	UpdateCaption();
}

/** Updates the slider's caption. */
void UUTUISlider::UpdateCaption()
{
	if(CaptionRenderComponent != NULL)
	{
		FString NewValue = FString::Printf(TEXT("%i"), appTrunc(SliderValue.GetCurrentValue()));

		if(CaptionRenderComponent->GetValue() != NewValue)
		{
			CaptionRenderComponent->SetValue(NewValue);
		}
	}
}

/*=========================================================================================
	UUTUIOptionButton
========================================================================================= */
IMPLEMENT_CLASS(UUTUIOptionButton)

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
void UUTUIOptionButton::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	// Initialize String Component
	if ( StringRenderComponent != NULL )
	{
		TScriptInterface<IUIDataStoreSubscriber> Subscriber(this);
		StringRenderComponent->InitializeComponent(&Subscriber);
		StringRenderComponent->EnableSubregion(UIORIENT_Vertical);
	}

	// Set style tags for the arrow buttons.
	if ( ArrowLeftButton != NULL )
	{
		check(ArrowLeftButton);
		check(ArrowLeftButton->BackgroundImageComponent);

		// the StyleResolverTags must match the name of the property in the owning scrollbar control in order for SetWidgetStyle to work correctly,
		// but since UIScrollbarButton will use either IncrementStyle or DecrementStyle depending on which one this is, it will be set dynamically
		// by the owning scrollbar when this button is created.
		ArrowLeftButton->BackgroundImageComponent->StyleResolverTag = TEXT("DecrementStyle");

		// Because we manage the styles for our buttons, and the buttons' styles are actually stored in their StyleSubscribers, we need to
		// initialize the StyleSubscribers arrays for the buttons prior to calling Super::Initialize() so that the first time OnResolveStyles
		// is called, the buttons will be ready to receive those styles.
		ArrowLeftButton->InitializeStyleSubscribers();
	}

	if ( ArrowRightButton != NULL )
	{
		check(ArrowRightButton);
		check(ArrowRightButton->BackgroundImageComponent);

		// the StyleResolverTags must match the name of the property in the owning scrollbar control in order for SetWidgetStyle to work correctly,
		// but since UIScrollbarButton will use either IncrementStyle or DecrementStyle depending on which one this is, it will be set dynamically
		// by the owning scrollbar when this button is created.
		ArrowRightButton->BackgroundImageComponent->StyleResolverTag = TEXT("IncrementStyle");

		// Because we manage the styles for our buttons, and the buttons' styles are actually stored in their StyleSubscribers, we need to
		// initialize the StyleSubscribers arrays for the buttons prior to calling Super::Initialize() so that the first time OnResolveStyles
		// is called, the buttons will be ready to receive those styles.
		ArrowRightButton->InitializeStyleSubscribers();
	}

	// Must initialize after the component
	Super::Initialize(inOwnerScene, inOwner);

	// Get current list element provider and selected index.
	ResolveListElementProvider();
// 	RefreshSubscriberValue();

	// Set private flags for the widget if we are in-game.
	if(GIsGame)
	{
		INT Flags = (UCONST_PRIVATE_NotFocusable|UCONST_PRIVATE_TreeHidden|UCONST_PRIVATE_NotEditorSelectable|UCONST_PRIVATE_ManagedStyle); 
		ArrowLeftButton->SetPrivateBehavior(Flags, TRUE);
		ArrowRightButton->SetPrivateBehavior(Flags, TRUE);
	}

	// Update states for arrows.
	ActivateArrowState(NULL, OPTBUT_ArrowRight);
	ActivateArrowState(NULL, OPTBUT_ArrowLeft);
}

/**
 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
 *
 * This version adds the LabelBackground (if non-NULL) to the StyleSubscribers array.
 */
void UUTUIOptionButton::InitializeStyleSubscribers()
{
	Super::InitializeStyleSubscribers();

	AddStyleSubscriber(BackgroundImageComponent);
	AddStyleSubscriber(StringRenderComponent);
}

/**
 * Called when a style reference is resolved successfully.
 *
 * @param	ResolvedStyle			the style resolved by the style reference
 * @param	StyleProperty			the name of the style reference property that was resolved.
 * @param	ArrayIndex				the array index of the style reference that was resolved.  should only be >0 for style reference arrays.
 * @param	bInvalidateStyleData	if TRUE, the resolved style is different than the style that was previously resolved by this style reference.
 */
void UUTUIOptionButton::OnStyleResolved( UUIStyle* ResolvedStyle, const FStyleReferenceId& StyleProperty, INT ArrayIndex, UBOOL bInvalidateStyleData )
{
	Super::OnStyleResolved(ResolvedStyle,StyleProperty,ArrayIndex,bInvalidateStyleData);

	FString StylePropertyName = StyleProperty.GetStyleReferenceName();
	if( StylePropertyName == TEXT("IncrementStyle"))
	{
		// propagate the IncrementStyle to the increment button
		ArrowRightButton->SetWidgetStyle(ResolvedStyle,StyleProperty,ArrayIndex);
	}
	else if( StylePropertyName == TEXT("DecrementStyle"))
	{
		// propagate the DecrementStyle to the decrement button
		ArrowLeftButton->SetWidgetStyle(ResolvedStyle,StyleProperty,ArrayIndex);
	}
}


/**
 * Adds the specified face to the DockingStack for the specified widget
 *
 * @param	DockingStack	the docking stack to add this docking node to.  Generally the scene's DockingStack.
 * @param	Face			the face that should be added
 *
 * @return	TRUE if a docking node was added to the scene's DockingStack for the specified face, or if a docking node already
 *			existed in the stack for the specified face of this widget.
 */
UBOOL UUTUIOptionButton::AddDockingNode( TLookupMap<FUIDockingNode>& DockingStack, EUIWidgetFace Face )
{
	return StringRenderComponent 
		? StringRenderComponent->AddDockingNode(DockingStack, Face)
		: Super::AddDockingNode(DockingStack, Face);
}

/**
 * Evaluates the Position value for the specified face into an actual pixel value.  Should only be
 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
 *
 * @param	Face	the face that should be resolved
 */
void UUTUIOptionButton::ResolveFacePosition( EUIWidgetFace Face )
{
	Super::ResolveFacePosition(Face);

	if ( StringRenderComponent != NULL )
	{
		StringRenderComponent->ResolveFacePosition(Face);
	}
}

/**
 * Marks the Position for any faces dependent on the specified face, in this widget or its children,
 * as out of sync with the corresponding RenderBounds.
 *
 * @param	Face	the face to modify; value must be one of the EUIWidgetFace values.
 */
void UUTUIOptionButton::InvalidatePositionDependencies( BYTE Face )
{
	Super::InvalidatePositionDependencies(Face);
	if ( StringRenderComponent != NULL )
	{
		StringRenderComponent->InvalidatePositionDependencies(Face);
	}
}

/**
 * Called when a property is modified that could potentially affect the widget's position onscreen.
 */
void UUTUIOptionButton::RefreshPosition()
{
	Super::RefreshPosition();

	RefreshFormatting();
}

/**
 * Called to globally update the formatting of all UIStrings.
 */
void UUTUIOptionButton::RefreshFormatting( UBOOL bRequestSceneUpdate/*=TRUE*/ )
{
	Super::RefreshFormatting(bRequestSceneUpdate);
	if ( StringRenderComponent != NULL )
	{
		StringRenderComponent->ReapplyFormatting(bRequestSceneUpdate);
	}
}

void UUTUIOptionButton::ResolveListElementProvider()
{
	TScriptInterface<IUIListElementProvider> Result;
	if ( DataSource.ResolveMarkup( this ) )
	{
		DataProvider = DataSource->ResolveListElementProvider(DataSource.DataStoreField.ToString());
	}
}

/** @return Returns the number of possible values for the field we are bound to. */
INT UUTUIOptionButton::GetNumValues()
{
	INT NumValues = 0;

	if(DataProvider)
	{
		NumValues = DataProvider->GetElementCount(DataSource.DataStoreField);
	}
	
	return NumValues;
}

/** 
 * @param Index		List index to get the value of.
 *
 * @return Returns the number of possible values for the field we are bound to. 
 */
UBOOL UUTUIOptionButton::GetListValue(INT ListIndex, FString &OutValue)
{
	UBOOL bResult = FALSE;

	if(DataProvider)
	{
		TArray<INT> ValueIds;
		if (DataProvider->GetListElements(DataSource.DataStoreField, ValueIds)
		&&	ValueIds.IsValidIndex(ListIndex) )
		{
			TScriptInterface<class IUIListElementCellProvider> ValueProvider = DataProvider->GetElementCellValueProvider(DataSource.DataStoreField, ValueIds(ListIndex));
			if(ValueProvider)
			{
				FUIProviderFieldValue FieldValue(EC_EventParm);
				if ( ValueProvider->GetCellFieldValue(DataSource.DataStoreField, DataSource.DataStoreField, ValueIds(ListIndex), FieldValue) )
				{
					OutValue = FieldValue.StringValue;
					bResult = TRUE;
				}
			}
		}
	}

	return bResult;
}	
	
/**
 * Retrieves all values (along with their associated ids) for the collection this option button is bound to.
 *
 * @param	out_ValueCollection		receives the list of ids/strings contained in the collection this button is bound to
 *
 * @return	TRUE if the output value was successfully filled in (even if it's empty); FALSE if the button isn't bound or the data store
 *			couldn't be resolved.
 */
UBOOL UUTUIOptionButton::GetCollectionValues( TMap<INT,FString>& out_ValueCollection )
{
	UBOOL bResult = FALSE;

	if ( DataProvider )
	{
		out_ValueCollection.Empty();
		
		TArray<INT> ValueIds;
		if ( DataProvider->GetListElements(DataSource.DataStoreField, ValueIds) )
		{
			for ( INT Idx = 0; Idx < ValueIds.Num(); Idx++ )
			{
				const INT ValueId = ValueIds(Idx);
				TScriptInterface<IUIListElementCellProvider> CellValueProvider = DataProvider->GetElementCellValueProvider(DataSource.DataStoreField, ValueId);
				if ( CellValueProvider )
				{
					FUIProviderFieldValue FieldValue(EC_EventParm);
					if ( CellValueProvider->GetCellFieldValue(DataSource.DataStoreField, DataSource.DataStoreField, ValueId, FieldValue) )
					{
						out_ValueCollection.Set(ValueId, *FieldValue.StringValue);
						bResult = TRUE;
					}
				}
			}
		}
	}

	return bResult;
}

/** Updates the string value using the current index. */
void UUTUIOptionButton::UpdateCurrentStringValue()
{
	FString OutValue;

	if(GetListValue(CurrentIndex, OutValue))
	{
		StringRenderComponent->SetValue(OutValue);
	}
}

/**
 * Generates a array of UI Action keys that this widget supports.
 *
 * @param	out_KeyNames	Storage for the list of supported keynames.
 */
void UUTUIOptionButton::GetSupportedUIActionKeyNames(TArray<FName> &out_KeyNames )
{
	Super::GetSupportedUIActionKeyNames(out_KeyNames);

	out_KeyNames.AddItem(UIKEY_MoveSelectionRight);
	out_KeyNames.AddItem(UIKEY_MoveSelectionLeft);
}

/**
 * Render this button.
 *
 * @param	Canvas	the FCanvas to use for rendering this widget
 */
void UUTUIOptionButton::Render_Widget( FCanvas* Canvas )
{
	const FLOAT ButtonSpacingPixels = ButtonSpacing.GetValue(this, EVALPOS_PixelOwner);
	const FLOAT ButtonSize = ArrowRightButton->GetPosition(UIFACE_Right, EVALPOS_PixelViewport)-ArrowLeftButton->GetPosition(UIFACE_Left, EVALPOS_PixelViewport);
	const FLOAT WidgetHeight = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];

	// Render background
	if ( BackgroundImageComponent != NULL )
	{
		FRenderParameters Parameters(
			RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top],
			ArrowLeftButton->GetPosition(UIFACE_Left, EVALPOS_PixelViewport)-RenderBounds[UIFACE_Left]-ButtonSpacingPixels,
			RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top],
			NULL, GetViewportHeight()
		);

		BackgroundImageComponent->RenderComponent(Canvas, Parameters);
	}

	if ( bCustomPlacement )
	{
		StringRenderComponent->Render_String(Canvas);
		return;
	}

	// Render String Text
	const FLOAT TextPaddingPercent=0.15f;
	const FLOAT TextPaddingPixels = WidgetHeight*TextPaddingPercent;
	Canvas->PushRelativeTransform(FTranslationMatrix(FVector(-(ButtonSize+ButtonSpacingPixels), TextPaddingPixels, 0.0f)));
	{
		StringRenderComponent->SetSubregionSize(UIORIENT_Vertical, WidgetHeight-TextPaddingPixels*2, UIEXTENTEVAL_Pixels);
		StringRenderComponent->Render_String(Canvas);
	}
	Canvas->PopTransform();
}

/**
 * Handles input events for this button.
 *
 * @param	EventParms		the parameters for the input event
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UUTUIOptionButton::ProcessInputKey( const struct FSubscribedInputEventParameters& EventParms )
{
	UBOOL bResult = FALSE;
	// Move Left Event
	if ( EventParms.InputAliasName == UIKEY_MoveSelectionLeft )
	{
		if(IsFocused(EventParms.PlayerIndex)==FALSE)
		{
			SetFocus(NULL, EventParms.PlayerIndex);
		}

		if ( EventParms.EventType == IE_Pressed )
		{
			if(HasPrevValue())
			{
				ActivateArrowState(UUIState_Pressed::StaticClass(), OPTBUT_ArrowLeft);
			}

			bResult = TRUE;
		}
		else if ( EventParms.EventType == IE_Released || EventParms.EventType == IE_Repeat )
		{
			if ( ArrowLeftButton->IsPressed(EventParms.PlayerIndex) )
			{
				eventOnMoveSelectionLeft(EventParms.PlayerIndex);
				if ( EventParms.EventType == IE_Repeat )
				{
					ActivateArrowState(UUIState_Pressed::StaticClass(), OPTBUT_ArrowLeft);
				}
			}
			bResult = TRUE;
		}
	}

	// Move Right Event
	if ( EventParms.InputAliasName == UIKEY_MoveSelectionRight )
	{
		if(IsFocused(EventParms.PlayerIndex)==FALSE)
		{
			SetFocus(NULL,EventParms.PlayerIndex);
		}

		if ( EventParms.EventType == IE_Pressed )
		{
			if(HasNextValue())
			{
				ActivateArrowState(UUIState_Pressed::StaticClass(), OPTBUT_ArrowRight);
			}

			bResult = TRUE;
		}
		else if ( EventParms.EventType == IE_Released || EventParms.EventType == IE_Repeat )
		{
			if ( ArrowRightButton->IsPressed(EventParms.PlayerIndex) )
			{
				eventOnMoveSelectionRight(EventParms.PlayerIndex);
				if ( EventParms.EventType == IE_Repeat )
				{
					ActivateArrowState(UUIState_Pressed::StaticClass(), OPTBUT_ArrowRight);
				}
			}

			bResult = TRUE;
		}
	}

	// Make sure to call the superclass's implementation after trying to consume input ourselves so that
	// we can respond to events defined in the super's class.
	if(bResult == FALSE)
	{
		bResult = Super::ProcessInputKey(EventParms);
	}

	return bResult;
}

/**
 * Handles input events for this button.
 *
 * @param	InState		State class to activate, if NULL, the arrow will be enabled or disabled state depending on whether the option button can move selection in the direction of the arrow.
 * @param	Arrow		Which arrow to activate the state for, LEFT is 0, RIGHT is 1.
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
void UUTUIOptionButton::ActivateArrowState( UClass* InState, EOptionButtonArrow Arrow )
{
	if(InState == NULL)
	{
		InState = GetArrowEnabledState(Arrow);
	}

	if(Arrow == OPTBUT_ArrowRight)
	{
		if(InState != UUIState_Focused::StaticClass())
		{
			ArrowRightButton->DeactivateStateByClass(UUIState_Focused::StaticClass(),0);
		}

		if(InState != UUIState_Pressed::StaticClass())
		{
			ArrowRightButton->DeactivateStateByClass(UUIState_Pressed::StaticClass(),0);
		}

		if(InState != UUIState_Disabled::StaticClass())
		{
			ArrowRightButton->SetEnabled(TRUE);
		}

		ArrowRightButton->ActivateStateByClass(InState,0);
	}
	else
	{
		if(InState != UUIState_Focused::StaticClass())
		{
			ArrowLeftButton->DeactivateStateByClass(UUIState_Focused::StaticClass(),0);
		}

		if(InState != UUIState_Pressed::StaticClass())
		{
			ArrowLeftButton->DeactivateStateByClass(UUIState_Pressed::StaticClass(),0);
		}

		if(InState != UUIState_Disabled::StaticClass())
		{
			ArrowLeftButton->SetEnabled(TRUE);
		}

		ArrowLeftButton->ActivateStateByClass(InState,0);
	}
}

/**
 * Returns the current arrow state(Enabled or disabled) depending on whether or not the widget can shift left or right.
 */
UClass* UUTUIOptionButton::GetArrowEnabledState(EOptionButtonArrow Arrow)
{
	UClass* State = UUIState_Enabled::StaticClass();

	if(Arrow == OPTBUT_ArrowLeft)
	{
		if ( !HasPrevValue() )
		{
			State = UUIState_Disabled::StaticClass();
		}
		else if ( IsFocused(GetBestPlayerIndex()) )
		{
			State = UUIState_Focused::StaticClass();
		}
	}
	else
	{
		if( !HasNextValue() )
		{
			State = UUIState_Disabled::StaticClass();
		}
		else if ( IsFocused(GetBestPlayerIndex()) )
		{
			State = UUIState_Focused::StaticClass();
		}
	}

	return State;
}


/**
 * @return TRUE if the CurrentIndex is at the start of the ValueMappings array, FALSE otherwise.
 */
UBOOL UUTUIOptionButton::HasPrevValue()
{
	if(bWrapOptions)
	{
		return (GetNumValues()>1); 
	}
	else
	{
		return (CurrentIndex>0);
	}
}

/**
 * @return TRUE if the CurrentIndex is at the start of the ValueMappings array, FALSE otherwise.
 */
UBOOL UUTUIOptionButton::HasNextValue()
{
	if(bWrapOptions)
	{
		return (GetNumValues()>1);
	}
	else
	{
		return (CurrentIndex<(GetNumValues()-1));
	}
}

/**
 * Moves the current index back by 1 in the valuemappings array if it isn't already at the front of the array.
 */
void UUTUIOptionButton::SetPrevValue()
{
	if(DataProvider)
	{
		const INT NumValues = GetNumValues();

		if(CurrentIndex > 0)
		{
			SetCurrentIndex(CurrentIndex-1);
		}
		else if((NumValues>1) && bWrapOptions)
		{
			SetCurrentIndex(NumValues - 1);
		}
	}
}

/**
 * Moves the current index forward by 1 in the valuemappings array if it isn't already at the end of the array.
 */
void UUTUIOptionButton::SetNextValue()
{
	if(DataProvider)
	{
		const INT NumValues = GetNumValues();

		if(NumValues - 1 > CurrentIndex && DataProvider)
		{
			SetCurrentIndex(CurrentIndex+1);
		}
		else if((NumValues>1) && bWrapOptions)
		{
			SetCurrentIndex(0);
		}
	}
}

/**
 * Activates the UIState_Focused menu state and updates the pertinent members of FocusControls.
 *
 * @param	FocusedChild	the child of this widget that should become the "focused" control for this widget.
 *							A value of NULL indicates that there is no focused child.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated the focus change.
 */
UBOOL UUTUIOptionButton::GainFocus( UUIObject* FocusedChild, INT PlayerIndex )
{
	// Make sure to call super first since it updates our focused state properly.
	const UBOOL bResult = Super::GainFocus(FocusedChild, PlayerIndex);

	// Update states for arrows.
	if ( FocusedChild == NULL )
	{
		ActivateArrowState(NULL, OPTBUT_ArrowRight);
		ActivateArrowState(NULL, OPTBUT_ArrowLeft);
	}

	return bResult;
}

/**
 * Deactivates the UIState_Focused menu state and updates the pertinent members of FocusControls.
 *
 * @param	FocusedChild	the child of this widget that is currently "focused" control for this widget.
 *							A value of NULL indicates that there is no focused child.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated the focus change.
 */
UBOOL UUTUIOptionButton::LoseFocus( UUIObject* FocusedChild, INT PlayerIndex )
{
	// Make sure to call super first since it updates our focused state properly.
	const UBOOL bResult = Super::LoseFocus(FocusedChild, PlayerIndex);

	// Update states for arrows.
	if ( FocusedChild == NULL )
	{
		ActivateArrowState(NULL, OPTBUT_ArrowRight);
		ActivateArrowState(NULL, OPTBUT_ArrowLeft);
	}
	return bResult;
}

/**
 * Resolves this subscriber's data store binding and publishes this subscriber's value to the appropriate data store.
 *
 * @param	out_BoundDataStores	contains the array of data stores that widgets have saved values to.  Each widget that
 *								implements this method should add its resolved data store to this array after data values have been
 *								published.  Once SaveSubscriberValue has been called on all widgets in a scene, OnCommit will be called
 *								on all data stores in this array.
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	TRUE if the value was successfully published to the data store.
 */
UBOOL UUTUIOptionButton::SaveSubscriberValue( TArray<UUIDataStore*>& out_BoundDataStores, INT BindingIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;
	FUIProviderFieldValue FieldValue(EC_EventParm);
	GetBoundDataStores(out_BoundDataStores);

	if(GetListValue(CurrentIndex,FieldValue.StringValue))
	{
		bResult = SetDataStoreFieldValue(DataSource.MarkupString, FieldValue, GetScene(), GetPlayerOwner());
	}

	return bResult;
}


/**
 * Called when the option button's index has changed.
 *
 * @param	PreviousIndex	the list's Index before it was changed
 * @param	PlayerIndex		the index of the player associated with this index change.
 */
void UUTUIOptionButton::NotifyIndexChanged( INT PreviousIndex, INT PlayerIndex )
{
	// Update the text for the option button
	UpdateCurrentStringValue();

	// update arrow state
	ActivateArrowState(NULL, OPTBUT_ArrowLeft);
	ActivateArrowState(NULL, OPTBUT_ArrowRight);

	if(GIsGame)
	{
		// activate the on value changed event
		if ( DELEGATE_IS_SET(OnValueChanged) )
		{
			// notify unrealscript that the user has submitted the selection
			delegateOnValueChanged(this,PlayerIndex);
		}

		ActivateEventByClass(PlayerIndex,UUIEvent_ValueChanged::StaticClass(), this);
	}
}

/**
 * @return Returns the current index for the option button.
 */
INT UUTUIOptionButton::GetCurrentIndex()
{
	return CurrentIndex;
}

/**
 * Sets a new index for the option button.
 *
 * @param NewIndex		New index for the option button.
 */
void UUTUIOptionButton::SetCurrentIndex(INT NewIndex)
{
	if(GetNumValues() > NewIndex && NewIndex >= 0)
	{
		INT PreviousIndex = CurrentIndex;
		CurrentIndex = NewIndex;

		// Activate index changed events.
		TArray<INT> Players;
		GetInputMaskPlayerIndexes(Players);

		INT PlayerIdx = 0;

		if(Players.Num() == 1)
		{
			PlayerIdx = Players(0);
		}

		NotifyIndexChanged(PreviousIndex, PlayerIdx);
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
void UUTUIOptionButton::SetDataStoreBinding( const FString& MarkupText, INT BindingIndex/*=-1*/ )
{
	if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		SetDefaultDataBinding(MarkupText,BindingIndex);
	}
	else if ( DataSource.MarkupString != MarkupText )
	{
		Modify();
        DataSource.MarkupString = MarkupText;

		// Resolve the list element provider.
		ResolveListElementProvider();

		// Refresh 
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
FString UUTUIOptionButton::GetDataStoreBinding( INT BindingIndex/*=-1*/ ) const
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
UBOOL UUTUIOptionButton::RefreshSubscriberValue( INT BindingIndex/*=INDEX_NONE*/ )
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
	else if ( StringRenderComponent != NULL && IsInitialized() )
	{
		// if we have a data store binding or we're trying to clear the data store binding, use the markup string
		if ( DataSource.MarkupString.Len() > 0 || DataSource )
		{
			FUIProviderFieldValue ResolvedValue(EC_EventParm);
			FString ValueString;
			if (DataSource.ResolveMarkup(this)
			&&	DataSource.GetBindingValue(ResolvedValue))
			{
				StringRenderComponent->SetValue(ResolvedValue.StringValue);
				ValueString = ResolvedValue.StringValue;
			}
			else
			{
				StringRenderComponent->SetValue(DataSource.MarkupString);
				ValueString = DataSource.MarkupString;
			}

			// Loop through all list values and try to match the current string value to a index.
			TMap<INT,FString> Values;
			CurrentIndex = 0;
			if ( GetCollectionValues(Values) )
			{
				if ( ResolvedValue.ArrayValue.Num() > 0 )
				{
					TArray<INT> ValueIds;
					Values.GenerateKeyArray(ValueIds);

					CurrentIndex = ValueIds.FindItemIndex(ResolvedValue.ArrayValue(0));
				}
				else if ( ValueString.Len() > 0 )
				{
					TArray<FString> ValueStrings;
					Values.GenerateValueArray(ValueStrings);

					CurrentIndex = ValueStrings.FindItemIndex(ValueString);
				}

				ActivateArrowState(NULL, OPTBUT_ArrowLeft);
				ActivateArrowState(NULL, OPTBUT_ArrowRight);
			}
		}
		bResult = TRUE;
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
void UUTUIOptionButton::NotifyDataStoreValueUpdated( UUIDataStore* SourceDataStore, UBOOL bValuesInvalidated, FName PropertyTag, UUIDataProvider* SourceProvider, INT ArrayIndex )
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
void UUTUIOptionButton::GetBoundDataStores( TArray<UUIDataStore*>& out_BoundDataStores )
{
	GetDefaultDataStores(out_BoundDataStores);
	// add overall data store to the list
	if ( DataSource )
	{
		out_BoundDataStores.AddUniqueItem(*DataSource);
	}

	// get any embedded data stores from the string
	if ( StringRenderComponent != NULL )
	{
		StringRenderComponent->GetResolvedDataStores(out_BoundDataStores);
	}
}

/**
 * Notifies this subscriber to unbind itself from all bound data stores
 */
void UUTUIOptionButton::ClearBoundDataStores()
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


/*=========================================================================================
	UTSimpleList
========================================================================================= */

IMPLEMENT_COMPARE_CONSTREF(FSimpleListData,UTUI_BaseWidgets, 
	{ 
		return appStricmp(*A.Text,*B.Text);
	} 
);




INT UUTSimpleList::Find(const FString& SearchText)
{
	for (INT i=0;i<List.Num();i++)
	{
		if ( List(i).Text == SearchText )
		{
			return i;
		}
	}
	return INDEX_NONE;
}

INT UUTSimpleList::FindTag(INT SearchTag)
{
	for (INT i=0;i<List.Num();i++)
	{
		if ( List(i).Tag == SearchTag )
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void UUTSimpleList::SortList()
{
	Sort <USE_COMPARE_CONSTREF(FSimpleListData,UTUI_BaseWidgets) > ( &List(0), List.Num() );
}


void UUTSimpleList::UpdateAnimation(FLOAT DeltaTime)
{
	UBOOL bNeedsInvalidation = FALSE;

	if(bTransitioning)
	{
		SelectionAlpha = (GWorld->GetWorldInfo()->RealTimeSeconds - StartSelectionTime) / TransitionTime;

		if(SelectionAlpha > 1.0f)
		{
			SelectionAlpha = 1.0f;
			bTransitioning = FALSE;
		}

		FLOAT SelectionIndex = (Selection - OldSelection) * SelectionAlpha + OldSelection;

		eventSetItemSelectionIndex(SelectionIndex);
	}

	if ( bNeedsInvalidation )
	{
		bInvalidated = TRUE;
	}
}

/**
 * Called when the scene receives a notification that the viewport has been resized.  Propagated down to all children.
 *
 * @param	OldViewportSize		the previous size of the viewport
 * @param	NewViewportSize		the new size of the viewport
 */
void UUTSimpleList::NotifyResolutionChanged( const FVector2D& OldViewportSize, const FVector2D& NewViewportSize )
{
	Super::NotifyResolutionChanged(OldViewportSize, NewViewportSize);

	bTransitioning=TRUE;
	eventSizeList();
	UpdateAnimation(0);
	eventRefreshBarPosition();
}


void UUTSimpleList::PostEditChange(UProperty* PropertyThatChanged)
{
	if(PropertyThatChanged && PropertyThatChanged->GetName()==TEXT("Selection"))
	{
		if (List.Num() > 0)
		{
			eventSelectItem(Selection);
		}
	}
	else
	{
		if (List.Num() > 0)
		{
			eventSelectItem(0);
		}
	}
}



/* ==========================================================================================================
	UUTUIMeshWidget
========================================================================================================== */
/**
 * Attach and initialize any 3D primitives for this widget and its children.
 *
 * @param	CanvasScene		the scene to use for attaching 3D primitives
 */
void UUTUIMeshWidget::InitializePrimitives( FCanvasScene* CanvasScene )
{
	// initialize the light component
	if ( DefaultLight != NULL )
	{
		CanvasScene->AddLight(DefaultLight);
		DefaultLight->ConditionalDetach();
		DefaultLight->ConditionalAttach(CanvasScene,NULL,GenerateTransformMatrix());
	}

	if ( DefaultLight2 != NULL )
	{
		CanvasScene->AddLight(DefaultLight2);
		DefaultLight2->ConditionalDetach();
		DefaultLight2->ConditionalAttach(CanvasScene,NULL,GenerateTransformMatrix());
	}

	// initialize the SKMesh component
	if ( SkeletalMeshComp != NULL )
	{
		SkeletalMeshComp->ConditionalDetach();
		SkeletalMeshComp->ConditionalAttach(CanvasScene,NULL,GenerateTransformMatrix());
	}

	Super::InitializePrimitives(CanvasScene);
}

/**
 * Updates 3D primitives for this widget.
 *
 * @param	CanvasScene		the scene to use for updating any 3D primitives
 */
void UUTUIMeshWidget::UpdateWidgetPrimitives( FCanvasScene* CanvasScene )
{
	Super::UpdateWidgetPrimitives(CanvasScene);

	FMatrix LocalToScreenMatrix = GetPrimitiveTransform(this);

	// update the static mesh component's transform
	/*if ( Mesh != NULL )
	{
		FVector EulerRotation(-90.0f,GWorld->GetTimeSeconds()*90.0f,0.0f);
		Mesh->Rotation = Mesh->Rotation.MakeFromEuler(EulerRotation);
		Mesh->ConditionalUpdateTransform();
		CanvasScene->UpdatePrimitiveTransform(Mesh);
	}*/

	if(SkeletalMeshComp != NULL)
	{
		FVector EulerRotation(0,-GWorld->GetTimeSeconds()*90.0f,0.0f);
		FVector OriginPoint = SkeletalMeshComp->SkeletalMesh->Origin + SkeletalMeshComp->SkeletalMesh->Bounds.Origin;

		FTranslationMatrix OriginMat(-OriginPoint);
		FRotationMatrix TwistMat(FRotationMatrix(FRotator::MakeFromEuler(FVector(-90,0,0))));
		FRotationMatrix RotationMat(FRotator::MakeFromEuler(EulerRotation));
		FLOAT Scale = 4.0f;

		if(BaseHeight > 0.0f)
		{
			FVector2D ViewportSize;
			GetViewportSize(ViewportSize);
			FLOAT ResScale = ViewportSize.Y / 768.0f; // We use 1024x768 as our base res
			Scale = BaseHeight / SkeletalMeshComp->SkeletalMesh->Bounds.SphereRadius * ResScale;
		}

		SkeletalMeshComp->Translation = FVector(0,0,0);
		SkeletalMeshComp->Rotation = FRotator::MakeFromEuler(FVector(0,0,0));
		SkeletalMeshComp->ConditionalUpdateTransform(OriginMat*TwistMat*RotationMat*FScaleMatrix(Scale)*LocalToScreenMatrix);

		
	}

	if( DefaultLight != NULL )
	{
		FRotationMatrix RotMatrix(FRotator::MakeFromEuler(LightDirection));
		DefaultLight->ConditionalUpdateTransform(RotMatrix*LocalToScreenMatrix);
	}

	if( DefaultLight2 != NULL )
	{
		FRotationMatrix RotMatrix(FRotator::MakeFromEuler(LightDirection2));
		DefaultLight2->ConditionalUpdateTransform(RotMatrix*LocalToScreenMatrix);
	}
}

/* ==========================================================================================================
	UUITabControl
========================================================================================================== */
/**
 * Set up the docking links between the tab control, buttons, and pages, based on the TabDockFace.
 */
void UUTUITabControl::SetupDockingRelationships()
{
	Super::SetupDockingRelationships();

#if CONSOLE
	if ( PrevPageCalloutLabel == NULL )
	{
		PrevPageCalloutLabel = Cast<UUILabel>(CreateWidget(this, UUILabel::StaticClass(), NULL, TEXT("lblPrevPageCallout")));
		PrevPageCalloutLabel->StringRenderComponent->StringStyle.DefaultStyleTag = CalloutLabelStyleName;
		InsertChild(PrevPageCalloutLabel);
		PrevPageCalloutLabel->SetDataStoreBinding("<StringAliasMap:ShiftUp>");
		PrevPageCalloutLabel->StringRenderComponent->eventEnableAutoSizing(UIORIENT_Horizontal, TRUE);
	}

	if ( NextPageCalloutLabel == NULL )
	{
		NextPageCalloutLabel = Cast<UUILabel>(CreateWidget(this, UUILabel::StaticClass(), NULL, TEXT("lblNextPageCallout")));
		NextPageCalloutLabel->StringRenderComponent->StringStyle.DefaultStyleTag = CalloutLabelStyleName;

		InsertChild(NextPageCalloutLabel);
		NextPageCalloutLabel->SetDataStoreBinding("<StringAliasMap:ShiftDown>");
		NextPageCalloutLabel->StringRenderComponent->eventEnableAutoSizing(UIORIENT_Horizontal, TRUE);
	}

	FLOAT ActualButtonHeight = TabButtonSize.GetValue(this);
	FLOAT ButtonVerticalPadding = TabButtonPadding[UIORIENT_Vertical].GetValue(this);
	if ( PrevPageCalloutLabel != NULL )
	{
		PrevPageCalloutLabel->SetDockTarget(UIFACE_Left, this, UIFACE_Left);

		if ( Pages.Num() > 0 )
		{
			UUITabButton* FirstTabButton = Pages(0)->TabButton;
			if ( FirstTabButton != NULL )
			{
				PrevPageCalloutLabel->SetDockTarget(UIFACE_Top, FirstTabButton, UIFACE_Top);
				PrevPageCalloutLabel->SetDockTarget(UIFACE_Bottom, FirstTabButton, UIFACE_Bottom);
				FirstTabButton->SetDockTarget(UIFACE_Left, PrevPageCalloutLabel, UIFACE_Right);
			}
		}
		else
		{
			PrevPageCalloutLabel->SetDockTarget(UIFACE_Top, this, UIFACE_Top);
			PrevPageCalloutLabel->SetDockTarget(UIFACE_Left, this, UIFACE_Left);
			PrevPageCalloutLabel->SetDockParameters(UIFACE_Bottom, this, UIFACE_Top, ActualButtonHeight);
		}
	}

	if ( NextPageCalloutLabel != NULL )
	{
		if ( Pages.Num() > 0 )
		{
			UUITabButton* LastTabButton = Pages.Last()->TabButton;
			if ( LastTabButton != NULL )
			{
				NextPageCalloutLabel->SetDockTarget(UIFACE_Top, LastTabButton, UIFACE_Top);
				NextPageCalloutLabel->SetDockTarget(UIFACE_Bottom, LastTabButton, UIFACE_Bottom);
				NextPageCalloutLabel->SetDockTarget(UIFACE_Left, LastTabButton, UIFACE_Right);
			}
		}
		else
		{
			NextPageCalloutLabel->SetDockTarget(UIFACE_Top, this, UIFACE_Top);
			NextPageCalloutLabel->SetDockParameters(UIFACE_Bottom, this, UIFACE_Top, ActualButtonHeight);
		}
	}

#endif
}

/**
 * Sets focus to the child widget that is next in the specified direction in the navigation network within this widget.
 *
 * This version doesn't let forced navigation start targetting mode.
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
UBOOL UUTUITabControl::NavigateFocus( UUIScreenObject* Sender, BYTE Direction, INT PlayerIndex/*=0*/, BYTE* bFocusChanged/*=NULL*/  )
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
		bResult = Super::NavigateFocus(this, Direction, PlayerIndex, bFocusChanged);
	}

	return bResult;
}


/* ==========================================================================================================
	UUTTabPage
========================================================================================================== */

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
void UUTTabPage::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);

	SetForcedNavigationTarget(UIFACE_Left, NULL);
	SetForcedNavigationTarget(UIFACE_Right, NULL);
}

/** Tick callback for this widget. */
void UUTTabPage::Tick_Widget(FLOAT DeltaTime)
{
	if(DELEGATE_IS_SET(OnTick))
	{
		delegateOnTick(DeltaTime);
	}
}

void UUTSimpleImageList::UpdateAnimation(FLOAT DeltaTime)
{
	UBOOL bNeedsInvalidation = FALSE;

	if(bTransitioning)
	{
		SelectionAlpha = (GWorld->GetWorldInfo()->RealTimeSeconds - StartSelectionTime) / TransitionTime;

		if(SelectionAlpha > 1.0f)
		{
			SelectionAlpha = 1.0f;
			bTransitioning = FALSE;
		}

		FLOAT SelectionIndex = (Selection - OldSelection) * SelectionAlpha + OldSelection;

		for(INT ItemIdx=0; ItemIdx<List.Num(); ItemIdx++)
		{
			FLOAT Distance = ItemIdx - SelectionIndex;
			Distance = Clamp<FLOAT>(Distance, -BubbleRadius, BubbleRadius);
			Distance /= BubbleRadius;

			List(ItemIdx).TransitionAlpha = 1.0f - Abs<FLOAT>(Distance);
			List(ItemIdx).CurMultiplier = eventGetItemScale(ItemIdx, SelectionIndex);
		}
	}

	if ( bNeedsInvalidation )
	{
		bInvalidated = TRUE;
	}
}

void UUTDrawPlayerListPanel::Tick_Widget(FLOAT DeltaTime)
{
	eventTick_Widget(DeltaTime);
}

void UUTUIScene_MidGameMenu::Tick(FLOAT DeltaTime)
{
//	eventTick(DeltaTime);

    if ( GWorld->GetMapName() == TEXT("UTFRONTEND") )
    {
		eventCloseScene(this, FALSE);
		return;
	}

	AWorldInfo* WI = GWorld->GetWorldInfo();
	if ( WI )
	{
		AUTGameReplicationInfo* GRI = Cast<AUTGameReplicationInfo>(WI->GRI);
		if ( GRI )
		{
			// If we are in a seamless transition, flag it
			if ( GSeamlessTravelHandler.IsInTransition() )
			{
				if ( !bLoading )
				{
					eventBeginLoading();
				}

				FRotator R = LoadingRotator->Rotation.Rotation;
				R.Yaw += appTrunc(65535.0 * DeltaTime);
				LoadingRotator->Rotation.Rotation = R;
				LoadingRotator->UpdateRotationMatrix();
			}
			else
			{
				if ( bLoading )
				{
					eventEndLoading();
				}
			

				if (GRI->bMatchIsOver)
				{
					eventUpdateVote(GRI);
				}
				else if ( GRI->bMatchHasBegun && bOkToAutoClose )
				{
					eventCloseScene(this);
				}
			}
		}
	}
	Super::Tick(DeltaTime);
}

UBOOL UUTUIScene_MidGameMenu::PreChildrenInputKey( INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed,UBOOL bGamepad)
{
	if (bWaitingForReady && Event == IE_Released && Key == FName(TEXT("XBoxTypeS_A")) )
	{
		if (SceneClient)
		{
			bWaitingForReady = false;
			eventCloseScene(this);
		}
		return true;
	}
	return Super::PreChildrenInputKey(ControllerId, Key, Event, AmountDepressed, bGamepad);
}

void UUTUIScene_SaveProfile::Tick(FLOAT DeltaTime)
{
	Super::Tick(DeltaTime);

	OnScreenTime += DeltaTime;
	if ( bShutdown )
	{
		// if we've been open for at least a half second and haven't begun the profile save process yet for some reason,
		// do that now...
		if ( OnScreenTime > 0.5f && !bProfileSaved )
		{
			eventPerformSave();
		}

		// if we've saved the profile, and it's time to close the scene, do so
		if ( bProfileSaved
		&&  (!bUseTimedClose || OnScreenTime > MinOnScreenTime) )
		{
			bShutdown = FALSE;
			eventShutDown();
		}
	}
}

