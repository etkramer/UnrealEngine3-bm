/**
* This class manages screenshots on disk.
* Functionality includes saving, enumerating, deleting, loading, and uploading.
* 
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

#include "GearGame.h"

IMPLEMENT_CLASS(UGearScreenshotManager);

/** A global GearScreenshotIO used by all managers. */
//TODO: should this be managed some other way?
static FGearScreenshotIO GScreenshotIO;

/**
 * Initializes the object.
 *
 * @return True if the initializiation succeeded.
 */
UBOOL UGearScreenshotManager::Init()
{
	//TODO: how should this be managed?
	ScreenshotIO = &GScreenshotIO;
	return TRUE;
}

/**
 * Starts an async task which saves the screenshots to disk.
 * If the function returns true, then any OnSaveScreenshotComplete delegates will be called when the save completes.
 *
 * @param Image The pre-compressed JPG file to save to disk.
 * @param Thumbnail The pre-compressed PNG file to use as a thumbnail.
 * @param Info Info to save along with the screenshot.  This can be used for UI display, and is converted to metadata for upload.
 *
 * @return True if the async operation is started successfully.  The save is not complete until the delegate is called.
 */
UBOOL UGearScreenshotManager::SaveScreenshot(const TArray<BYTE>& Image,const TArray<BYTE>& Thumbnail,const struct FScreenshotInfo& Info)
{
	check(ScreenshotIO);

	return ScreenshotIO->Save(GetPlayerControllerId(), Image, Thumbnail, Info, &SaveScreenshotCompleteDelegates);
}

/**
 * Deletes a screenshot from disk.
 *
 * @param ID The unique ID of the screenshot to delete.
 * @param DeviceID The device from which to delete the screenshot.
 *
 * @return True if the delete succeeds.
 */
UBOOL UGearScreenshotManager::DeleteScreenshot(FGuid ID, int DeviceID)
{
	check(ScreenshotIO);
	return ScreenshotIO->Delete(GetPlayerControllerId(), ID, DeviceID);
}

/**
 * Gets a list of screenshots on disk.
 *
 * @param Screenshots If the function succeeds, the array will contain a list of this player's screenshots on disk.
 *
 * @return True if the enumeration succeeds.
 */
UBOOL UGearScreenshotManager::EnumerateScreenshots(TArray<FSavedScreenshotInfo>& Screenshots)
{
	check(ScreenshotIO);
	return ScreenshotIO->Enumerate(GetPlayerControllerId(), Screenshots, &EnumerateScreenshotsCompleteDelegates);
}

/**
 * Loads a screenshot from disk.
 *
 * @param ID The unique ID of the screenshot to load.
 * @param DeviceID The device from which to load the screenshot.
 * @param Image If the function succeeds, this will contain the JPG file for this screenshot.
 * @param Info If the function succeeds, this will contain the info saved along with the screenshot.
 *
 * @return True if the screenshot was loaded successfully.
 */
UBOOL UGearScreenshotManager::LoadScreenshot(const FGuid& ID, int DeviceID, TArray<BYTE>& Image, FScreenshotInfo& Info)
{
	check(ScreenshotIO);
	return ScreenshotIO->Load(GetPlayerControllerId(), ID, DeviceID, Image, Info, &LoadScreenshotCompleteDelegates);
}

/**
 * Wrapper for getting a reference to the LocalPlayer associated with this screenshot manager.
 */
ULocalPlayer* UGearScreenshotManager::GetPlayerOwner() const
{
	return Cast<ULocalPlayer>(GetOuterAGearPC()->Player);
}

/**
 * Wrapper for getting the controller id of the player that owns this screenshot manager.
 *
 * @return	the controller ID for the player associated with this screenshot manager, or 255 if the owning player doesn't have a valid
 *			local player object.
 */
INT UGearScreenshotManager::GetPlayerControllerId() const
{
	INT Result = 255;

	ULocalPlayer* PlayerOwner = GetPlayerOwner();
	if ( PlayerOwner != NULL )
	{
		Result = PlayerOwner->ControllerId;
	}

	return Result;
}

#if USE_XeD3D_RHI
/**
* Takes a raw JPG in memory and stores it in a texture.
*/
static void RenderThread_LoadJPGIntoTexture(const TArray<BYTE>& Image, UTexture2DDynamic* Texture)
{
	FTexture2DDynamicResource* TextureResource = (FTexture2DDynamicResource*)Texture->Resource;
	IDirect3DTexture9* D3DTexture = TextureResource->GetTexture2DRHI();
	IDirect3DSurface9* Surface;
	verify(SUCCEEDED( D3DTexture->GetSurfaceLevel(0, &Surface) ));
	DWORD StartTime = appSeconds();
	verify(SUCCEEDED( D3DXLoadSurfaceFromFileInMemory(Surface,
		NULL, NULL,
		Image.GetData(), Image.Num(),
		NULL, D3DX_FILTER_NONE,  //should this be a different filter value?
		(D3DCOLOR)0, NULL) ));
	debugf(TEXT("D3DXLoadSurfaceFromFileInMemory(): %0.3f"), (appSeconds() - StartTime) * 1000);
	Surface->Release();
}
#endif

void UGearScreenshotManager::ScreenshotToTexture(const TArray<BYTE>& Image,class UTexture2DDynamic*& Texture)
{
#if USE_XeD3D_RHI
	if (Texture != NULL)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			LoadJPGIntoTexture,
			const TArray<BYTE>*,Image,&Image,
			UTexture2DDynamic*,Texture,Texture,
		{
			RenderThread_LoadJPGIntoTexture(*Image, Texture);
		});

		FlushRenderingCommands();
	}
#endif
}

/** Internal function used to convert a GUID to a string */
void UGearScreenshotManager::GuidToString(const FGuid InGuid,FString& OutString)
{
	OutString = InGuid.String();
}

/** Internal function used to convert a string to a blob of ANSI chars */
void UGearScreenshotManager::StringToAnsiBlob(const FString& InString,TArray<BYTE>& OutBlob)
{
	INT StringLen;
	INT Index;
	BYTE* BlobPtr;
	StringLen = InString.Len();
	OutBlob.Empty(StringLen);
	if(appIsPureAnsi(*InString))
	{
		OutBlob.AddZeroed(StringLen);
		BlobPtr = OutBlob.GetData();
		for(Index = 0; Index < StringLen; Index++)
		{
			BlobPtr[Index] = (BYTE)InString[Index];
		}
	}
}

/**
 * Internal functions used for writing XML elements to a string.
 */

static void XmlWriteIndents(FString& Xml, INT Indent)
{
	while(Indent-- > 0)
	{
		Xml += TEXT('\t');
	}
}

void UGearScreenshotManager::XmlWriteStartTag(FString& Xml,const FString& TagName,INT Indent)
{
	XmlBeginStartTag(Xml, TagName, Indent);
	XmlEndStartTag(Xml, FALSE);
}

void UGearScreenshotManager::XmlBeginStartTag(FString& Xml,const FString& TagName,INT Indent)
{
	XmlWriteIndents(Xml, Indent);
	Xml += FString::Printf(TEXT("<%s"), *TagName);
}

void UGearScreenshotManager::XmlEndStartTag(FString& Xml,UBOOL Close)
{
	if(Close)
	{
		Xml += TEXT(" /");
	}
	Xml += TEXT(">\r\n");
}

void UGearScreenshotManager::XmlWriteEndTag(FString& Xml,const FString& TagName,INT Indent)
{
	XmlWriteIndents(Xml, Indent);
	Xml += FString::Printf(TEXT("</%s>\r\n"), *TagName);
}

void UGearScreenshotManager::XmlWriteAttributeString(FString& Xml,const FString& Attribute,const FString& Value)
{
	Xml += FString::Printf(TEXT(" %s=\"%s\""), *Attribute, *Value);
}

void UGearScreenshotManager::XmlWriteAttributeBool(FString& Xml,const FString& Attribute,UBOOL Value)
{
	XmlWriteAttributeString(Xml, Attribute, Value?TEXT("true"):TEXT("false"));
}

void UGearScreenshotManager::XmlWriteAttributeInt(FString& Xml,const FString& Attribute,INT Value)
{
	XmlWriteAttributeString(Xml, Attribute, appItoa(Value));
}

void UGearScreenshotManager::XmlWriteAttributeFloat(FString& Xml,const FString& Attribute,FLOAT Value)
{
	XmlWriteAttributeString(Xml, Attribute, FString::Printf(TEXT("%f"), Value));
}

void UGearScreenshotManager::XmlWriteAttributeQword(FString& Xml,const FString& Attribute,INT Value1,INT Value2)
{
	XmlWriteAttributeString(Xml, Attribute, FString::Printf(TEXT("%08X%08X"), Value1,Value2));
}

void UGearScreenshotManager::XmlWriteContent(FString& Xml,const FString& Content)
{
	Xml += Content;
}
