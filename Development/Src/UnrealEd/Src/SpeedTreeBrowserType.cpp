/*=============================================================================
	SpeedTreeBrowserType.cpp: SpeedTree generic browser implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "GenericBrowserContextMenus.h"
#include "SpeedTree.h"

IMPLEMENT_CLASS(UGenericBrowserType_SpeedTree);

#if WITH_SPEEDTREE
class WxMBGenericBrowserContext_SpeedTree : public WxMBGenericBrowserContext
{
public:
	WxMBGenericBrowserContext_SpeedTree(void)
	{
		Append(IDMN_ObjectContext_Editor, *LocalizeUnrealEd("SpeedTreeEditor"), TEXT(""));
		AppendObjectMenu( );
	}
};
#endif

void UGenericBrowserType_SpeedTree::Init( )
{
#if WITH_SPEEDTREE
	SupportInfo.AddItem(FGenericBrowserTypeInfo(USpeedTree::StaticClass( ), FColor(100, 255, 100), new WxMBGenericBrowserContext_SpeedTree));
#endif
}

UBOOL UGenericBrowserType_SpeedTree::ShowObjectEditor(UObject* InObject)
{
#if WITH_SPEEDTREE
	WxSpeedTreeEditor* SpeedTreeEditor = new WxSpeedTreeEditor(GApp->EditorFrame, -1, CastChecked<USpeedTree>(InObject));
	SpeedTreeEditor->Show(1);
	return TRUE;
#else
	return FALSE;
#endif
}

