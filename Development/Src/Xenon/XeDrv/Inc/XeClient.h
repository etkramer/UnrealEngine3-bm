/*=============================================================================
	XeDrv.h: Unreal Xenon client driver.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Xenon platform driver.
#include "XeDrv.h"

// Forward declarations.
class FXenonViewport;

//
//	UXenonClient - Xenon-specific client code.
//
class UXenonClient : public UClient
{
	DECLARE_CLASS(UXenonClient,UClient,CLASS_Transient|CLASS_Config,XeDrv)


	// Variables.
	TArray<FXenonViewport*>				Viewports;
	UEngine*							Engine;

	/** Map used to associate VK_*'s as reported by the OS with FName's as consumed by the engine	*/
	TMap<BYTE,FName>					KeyMapVirtualToName;
	/** Reverse map to associate FName's used by the engine with virtual key's reported by the OS	*/
	TMap<FName,BYTE>					KeyMapNameToVirtual;

	// Audio device.
	UClass*								AudioDeviceClass;
	UAudioDevice*						AudioDevice;

	// Constructors.
	UXenonClient();
	void StaticConstructor();

	// UObject interface.
	virtual void Serialize(FArchive& Ar);
	virtual void FinishDestroy();

	// UClient interface.
	virtual void Init(UEngine* InEngine);
	virtual void Tick(FLOAT Tick);
	virtual UBOOL Exec(const TCHAR* Cmd,FOutputDevice& Ar);
	/**
	 * PC only function used by e.g. the script debugger.
	 *
	 * @param InValue	Unused.
	 */
	virtual void AllowMessageProcessing( UBOOL /*InValue*/ ) {}

	virtual FViewportFrame* CreateViewportFrame(FViewportClient* ViewportClient,const TCHAR* InName,UINT SizeX,UINT SizeY,UBOOL Fullscreen = 0);
	virtual FViewport* CreateWindowChildViewport(FViewportClient* ViewportClient,void* ParentWindow,UINT SizeX=0,UINT SizeY=0,INT InPosX = -1, INT InPosY = -1);
	virtual void CloseViewport(FViewport* Viewport);

	virtual class UAudioDevice* GetAudioDevice() { return AudioDevice; }

	/**
	 * Retrieves the name of the key mapped to the specified character code.
	 *
	 * @param	KeyCode	the key code to get the name for; should be the key's ANSI value
	 */
	virtual FName GetVirtualKeyName( INT KeyCode ) const;

	/** Function to immediately stop any force feedback vibration that might be going on this frame. */
	virtual void ForceClearForceFeedback();
};
