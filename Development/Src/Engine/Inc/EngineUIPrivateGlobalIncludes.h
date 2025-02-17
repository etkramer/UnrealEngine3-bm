/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


/*===========================================================================
    Class and struct declarations which are coupled to
	EngineUIPrivateClasses.h but shouldn't be declared inside a class
===========================================================================*/

#ifndef NAMES_ONLY

#include "UnPrefab.h"	// Required for access to FPrefabUpdateArc

#ifndef __ENGINEUIPRIVATEGLOBALINCLUDES_H__
#define __ENGINEUIPRIVATEGLOBALINCLUDES_H__

// Forward declaration for the animation data struct
struct FUIAnimSequence;

/**
 * Converts and stores the render bounds for a widget as vectors, for use in line checks.  Used by the code that generates the
 * automatic navigation links for widgets.
 */
struct FClosestLinkCalculationBox
{
	/** Constructor */
	FClosestLinkCalculationBox( UUIObject* InObject );

	/**
	 * Determines whether the widget specified is a valid navigation link for the specified face of this widget.
	 *
	 * @param	Other	the bounds data for the widget that will possibly become a navigation link for our widget
	 * @param	Face	the face that is being evaluated
	 *
	 * @return	FALSE if the widget specified is on the wrong side of this widget (i.e. if we're evaluating the left
	 *			face, then any widgets that have a right face that is further left than our left face cannot become
	 *			navigation links)
	 */
	UBOOL IsValid( const FClosestLinkCalculationBox& Other, const EUIWidgetFace Face ) const
	{
		UBOOL bResult = FALSE;

		if ( Other.Obj != Obj )
		{
			switch( Face )
			{
			case UIFACE_Top:
				// Target's bottom must be less than source's top
				bResult = Other.Extent.Y <= Origin.Y;
				break;

			case UIFACE_Bottom:
				// Target's top must be greater than source's bottom
				bResult = Other.Origin.Y >= Extent.Y;
				break;

			case UIFACE_Left:
				// Target's right must be less than source's left
				bResult = Other.Extent.X <= Origin.X;
				break;

			case UIFACE_Right:
				// Target's left must be greater than source's right
				bResult = Other.Origin.X >= Extent.X;
				break;
			}
		}

		return bResult;
	}

	/**
	 * Gets the vectors representing the end points of the specified face.
	 *
	 * @param	Face		the face to get endpoints for.
	 * @param	StartPoint	[out] the left or top endpoint of the specified face
	 * @param	EndPoint	[out] the right or bottom point of the specified face
	 */
	void GetFaceVectors( const EUIWidgetFace Face, FVector& StartPoint, FVector& EndPoint ) const
	{
		StartPoint = Origin;
		EndPoint = Extent;

		switch ( Face )
		{
		case UIFACE_Top:
			EndPoint.Y = Origin.Y;
			break;

		case UIFACE_Bottom:
			StartPoint.Y = Extent.Y;
			break;

		case UIFACE_Left:
			EndPoint.X = StartPoint.X;
			break;

		case UIFACE_Right:
			StartPoint.X = Extent.X;
			break;
		}
	}

	/** the widget that this struct is tracking render bounds for */
	UUIObject*	Obj;

	/** stores the value of the widget's Top-left corner */
	FVector		Origin;

	/** stores the value of the widget's lower-right corner */
	FVector		Extent;
};

/**
 * Specialized version of FPrefabUpdateArc used for handling UIPrefabs.
 */
class FUIPrefabUpdateArc : public FPrefabUpdateArc
{
	/** Allow UIPrefabInstance to access all our personals */
	friend class UUIPrefabInstance;

public:
	FUIPrefabUpdateArc() : FPrefabUpdateArc()
	{
		bInstanceSubobjectsOnLoad = FALSE;
	}
};

/**
 * Specialized version of FArchiveReplaceObjectRef used for handling UIPrefabs.  It extends the functionality of FArchiveReplaceObjectRef in two ways:
 *	- allows the caller to specify a set of objects which should NOT be recursively serialized by this archive
 */
class FUIPrefabReplaceObjectRefArc : public FArchiveReplaceObjectRef<UObject>
{
public:
	/**
	 * Initializes variables and starts the serialization search
	 *
	 * @param InSearchObject		The object to start the search on
	 * @param ReplacementMap		Map of objects to find -> objects to replace them with (null zeros them)
	 * @param inSerializeExclusionMap
	 *								Map of objects that this archive should NOT call Serialize on.  They will be added to the SerializedObjects map
	 *								before the process begins.
	 * @param bNullPrivateRefs		Whether references to non-public objects not contained within the SearchObject
	 *								should be set to null
	 * @param bIgnoreOuterRef		Whether we should replace Outer pointers on Objects.
	 * @param bIgnoreArchetypeRef	Whether we should replace the ObjectArchetype reference on Objects.
	 */
	FUIPrefabReplaceObjectRefArc
	(
		UObject* InSearchObject,
		const TMap<UObject*,UObject*>& inReplacementMap,
		const TLookupMap<UObject*>* inSerializeExclusionMap,
		UBOOL bNullPrivateRefs,
		UBOOL bIgnoreOuterRef,
		UBOOL bIgnoreArchetypeRef
	)
	: FArchiveReplaceObjectRef<UObject>(InSearchObject,inReplacementMap,bNullPrivateRefs,bIgnoreOuterRef,bIgnoreArchetypeRef,TRUE)
	{
		if ( inSerializeExclusionMap != NULL )
		{
			for ( INT ExclusionIndex = 0; ExclusionIndex < inSerializeExclusionMap->Num(); ExclusionIndex++ )
			{
				UObject* ExcludedObject = (*inSerializeExclusionMap)(ExclusionIndex);
				if ( ExcludedObject != NULL && ExcludedObject != SearchObject )
				{
					SerializedObjects.Add(ExcludedObject);
				}
			}
		}

		SerializeSearchObject();
	}
};

enum EUISceneTickStats
{
	STAT_UISceneTickTime = STAT_UIDrawingTime+1,
	STAT_UISceneUpdateTime,
	STAT_UIPreRenderCallbackTime,
	STAT_UIRefreshFormatting,
	STAT_UIRebuildDockingStack,
	STAT_UIResolveScenePositions,
	STAT_UIRebuildNavigationLinks,
	STAT_UIRefreshWidgetStyles,

	STAT_UIAddDockingNode,
	STAT_UIAddDockingNode_String,

	STAT_UIResolvePosition,
	STAT_UIResolvePosition_String,
	STAT_UIResolvePosition_List,
	STAT_UIResolvePosition_AutoAlignment,

	STAT_UISceneRenderTime,

	STAT_UIGetStringFormatParms,
	STAT_UIApplyStringFormatting,

	STAT_UIParseString,

	STAT_UISetWidgetPosition,
	STAT_UIGetWidgetPosition,
	STAT_UICalculateBaseValue,
	STAT_UIGetPositionValue,
	STAT_UISetPositionValue,

	STAT_UIProcessInput,
};

#if SUPPORTS_DEBUG_LOGGING

#define LOG_DATAFIELD_UPDATE(SourceDataStore,bValuesInvalidated,PropertyTag,SourceProvider,ArrayIndex) \
	debugf(NAME_DevDataStore, TEXT("NotifyDataStoreUpdated PropertyTag:%s %s bValuesInvalidated:%i ArrayIndex:%i DS:%s    Provider:%s   Widget:%s"), \
	*PropertyTag.ToString(), bBoundToDataStore ? GTrue : GFalse, bValuesInvalidated, ArrayIndex, *SourceDataStore->GetName(), *SourceProvider->GetPathName(), *GetPathName());

/** the number of spaces to indent focus chain debug log messages */
extern INT FocusDebugIndent;

#else	//	SUPPORTS_DEBUG_LOGGING

#define LOG_DATAFIELD_UPDATE(SourceDataStore,bValuesInvalidated,PropertyTag,SourceProvider,ArrayIndex)

#endif	//	SUPPORTS_DEBUG_LOGGING

#endif	// __ENGINEUIPRIVATEGLOBALINCLUDES_H__
#endif	// NAMES_ONLY
