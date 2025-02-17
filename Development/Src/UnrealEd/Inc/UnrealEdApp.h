/*=============================================================================
	UnrealEdApp.h: The main application class

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNREALEDAPP_H__
#define __UNREALEDAPP_H__

// Forward declarations
class FSourceControlIntegration;
class UUnrealEdEngine;
class WxDlgActorFactory;
class WxDlgActorSearch;
class WxDlgBindHotkeys;
class WxDlgBuildProgress;
class WxDlgGeometryTools;
class WxDlgMapCheck;
class WxDlgTipOfTheDay;
class WxDlgTransform;
class WxDlgAutoConvexCollision;
class WxPropertyWindow;
class WxPropertyWindowFrame;
class WxTerrainEditor;

#include "../../Unrealed/src/ConvexDecompTool.h"

/*-----------------------------------------------------------------------------
	FAutoConvexCollision

	Utility class that handles the actual creation of blocking volumes
	when the "Auto Convex Collision" dialog is used on selected static meshes.
-----------------------------------------------------------------------------*/

class FAutoConvexCollision : public FConvexDecompOptionHook
{
	/** Handler for the FConvexDecompOptionHook hook */
	virtual void DoDecomp(INT Depth, INT MaxVerts, FLOAT CollapseThresh);
	virtual void DecompOptionsClosed();
	virtual void DecompNewVolume();

	TArray<FPoly*> GetSelectedPolygons();
	void SetCollisionTypeOnSelectedActors( BYTE InCollisionType );

	/** The last blocking volume that was created.  This will be deleted when the user next hits the Apply button. */
	ABlockingVolume* LastCreatedVolume;
};

/*-----------------------------------------------------------------------------
	WxUnrealEdApp
-----------------------------------------------------------------------------*/

class WxUnrealEdApp :
	public WxLaunchApp,
	// The interface for receiving notifications
	public FCallbackEventDevice
{
	/**
	 * Uses INI file configuration to determine which class to use to create
	 * the editor's frame. Will fall back to Epic's hardcoded frame if it is
	 * unable to create the configured one properly
	 */
	WxEditorFrame* CreateEditorFrame(void);


public:
	WxEditorFrame* EditorFrame;							// The overall editor frame (houses everything)

	FString LastDir[LD_MAX];
	FString MapExt;

	/** A mapping from wx keys to unreal keys. */
	TMap<INT, FName>	WxKeyToUnrealKeyMap;

	WxDlgSurfaceProperties* DlgSurfProp;
	FSourceControlIntegration* SCC;
	WxTerrainEditor* TerrainEditor;
	WxDlgGeometryTools* DlgGeometryTools;
	WxDlgBuildProgress* DlgBuildProgress;
	WxDlgMapCheck* DlgMapCheck;
	WxDlgLoadErrors* DlgLoadErrors;
	WxDlgActorSearch* DlgActorSearch;
	WxDlgActorFactory* DlgActorFactory;
	WxDlgTipOfTheDay* DlgTipOfTheDay;
	WxDlgBindHotkeys* DlgBindHotkeys;
	WxDlgTransform* DlgTransform;
	FAutoConvexCollision* AutoConvexHelper;
	WxConvexDecompOptions* DlgAutoConvexCollision;

	UTexture2D *MaterialEditor_RightHandleOn, *MaterialEditor_RightHandle,
		*MaterialEditor_RGBA, *MaterialEditor_R, *MaterialEditor_G, *MaterialEditor_B, *MaterialEditor_A,
		*MaterialEditor_LeftHandle, *MaterialEditor_ControlPanelFill, *MaterialEditor_ControlPanelCap,
		*UpArrow, *DownArrow, *MaterialEditor_LabelBackground, *MaterialEditor_Delete;
	TMap<INT,UClass*> ShaderExpressionMap;		// For relating menu ID's to shader expression classes

	WxPropertyWindowFrame*	ObjectPropertyWindow;
	WxPropertyWindow* CurrentPropertyWindow;

	TArray<class WxKismet*>	KismetWindows;

	/** Pointer to Sentinel stats-viewing tool. */
	class WxSentinel* SentinelTool;

	virtual bool OnInit();
	virtual int OnExit();

	/** Generate a mapping of wx keys to unreal key names */
	void GenerateWxToUnrealKeyMap();

	/**
	 * @return Returns a unreal key name given a wx key event.
	 */
	virtual FName GetKeyName(wxKeyEvent &Event);

	/**
	 * Performs any required cleanup in the case of a fatal error.
	 */
	virtual void ShutdownAfterError();

// FCallbackEventDevice interface

	/**
	 * Routes the event to the appropriate handlers
	 *
	 * @param InType the event that was fired
	 * @param InFlags the flags for this event
	 */
	virtual void Send(ECallbackEventType InType,DWORD InFlags);

	/**
	 * Routes the event to the appropriate handlers
	 *
	 * @param InType the event that was fired
	 */
	virtual void Send(ECallbackEventType InType);

	/**
	 * Routes the event to the appropriate handlers
	 *
	 * @param InObject the relevant object for this event
	 */
	virtual void Send(ECallbackEventType InType, UObject* InObject);

	/**
	 * Routes the event to the appropriate handlers
	 *
	 * @param InType the event that was fired
	 * @param InEdMode the FEdMode that is changing
	 */
	virtual void Send(ECallbackEventType InType,FEdMode* InEdMode);

	/**
	 * Notifies all observers that are registered for this event type
	 * that the event has fired
	 *
	 * @param InType the event that was fired
	 * @param InString the string information associated with this event
	 * @param InObject the object associated with this event
	 */
	virtual void Send(ECallbackEventType InType,const FString& InString, UObject* InObject);

	/** Returns a handle to the current terrain editor. */
	WxTerrainEditor* GetTerrainEditor()
	{
		return TerrainEditor;
	};

	/**
	*  Updates text and value for various progress meters.
	*
	*	@param StatusText				New status text
	*	@param ProgressNumerator		Numerator for the progress meter (its current value).
	*	@param ProgressDenominitator	Denominiator for the progress meter (its range).
	*/
	void StatusUpdateProgress(const TCHAR* StatusText, INT ProgressNumerator, INT ProgressDenominator);

	/**
	* Returns whether or not the map build in progressed was cancelled by the user. 
	*/
	UBOOL GetMapBuildCancelled() const;

	/**
	* Sets the flag that states whether or not the map build was cancelled.
	*
	* @param InCancelled	New state for the cancelled flag.
	*/
	void SetMapBuildCancelled( UBOOL InCancelled );

	// Callback handlers.
	void CB_SelectionChanged();
	void CB_SurfProps();
	void CB_ActorProps();
	void CB_CameraModeChanged();
	void CB_ActorPropertiesChanged();
	void CB_ObjectPropertyChanged(UObject* Object);
	void CB_RefreshPropertyWindows();
	void CB_DisplayLoadErrors();
	void CB_RefreshEditor();
	void CB_MapChange( DWORD InFlags );
	void CB_RedrawAllViewports();
	void CB_Undo();
	void CB_EndPIE();
	void CB_EditorModeEnter(const FEdMode& InEdMode);

private:
	
	/** Stores whether or not the current map build was cancelled. */
	UBOOL bCancelBuild;
};

#endif // __UNREALEDAPP_H__
