/*===========================================================================
    C++ class definitions exported from UnrealScript.
    This is automatically generated by the tools.
    DO NOT modify this manually! Edit the corresponding .uc files instead!
    Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
===========================================================================*/
#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,4)
#endif


// Split enums from the rest of the header so they can be included earlier
// than the rest of the header file by including this file twice with different
// #define wrappers. See Engine.h and look at EngineClasses.h for an example.
#if !NO_ENUMS && !defined(NAMES_ONLY)

#ifndef INCLUDED_UTEDITOR_ENUMS
#define INCLUDED_UTEDITOR_ENUMS 1


#endif // !INCLUDED_UTEDITOR_ENUMS
#endif // !NO_ENUMS

#if !ENUMS_ONLY

#ifndef NAMES_ONLY
#define AUTOGENERATE_NAME(name) extern FName UTEDITOR_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#endif


#ifndef NAMES_ONLY

#ifndef INCLUDED_UTEDITOR_CLASSES
#define INCLUDED_UTEDITOR_CLASSES 1

class UGenericBrowserType_UTMapMusicInfo : public UGenericBrowserType
{
public:
    //## BEGIN PROPS GenericBrowserType_UTMapMusicInfo
    //## END PROPS GenericBrowserType_UTMapMusicInfo

    DECLARE_CLASS(UGenericBrowserType_UTMapMusicInfo,UGenericBrowserType,0,UTEditor)
	/**
	* Initialize the supported classes for this browser type.
	*/
	virtual void Init();
};

class UUTUnrealEdEngine : public UUnrealEdEngine
{
public:
    //## BEGIN PROPS UTUnrealEdEngine
    //## END PROPS UTUnrealEdEngine

    DECLARE_CLASS(UUTUnrealEdEngine,UUnrealEdEngine,0|CLASS_Transient|CLASS_Config,UTEditor)
	virtual UBOOL Exec( const TCHAR* Stream, FOutputDevice& Ar );
};

class UUTEditorEngine : public UEditorEngine
{
public:
    //## BEGIN PROPS UTEditorEngine
    //## END PROPS UTEditorEngine

    DECLARE_CLASS(UUTEditorEngine,UEditorEngine,0|CLASS_Transient|CLASS_Config,UTEditor)
	virtual void PostScriptCompile();
};

class UUTEditorUISceneClient : public UEditorUISceneClient
{
public:
    //## BEGIN PROPS UTEditorUISceneClient
    //## END PROPS UTEditorUISceneClient

    DECLARE_CLASS(UUTEditorUISceneClient,UEditorUISceneClient,0|CLASS_Transient,UTEditor)
	/**
	 * Render all the active scenes
	 */
	virtual void RenderScenes( FCanvas* Canvas );

	virtual void Render_Scene( FCanvas* Canvas, UUIScene* Scene, EUIPostProcessGroup UIPostProcessGroup );
};

#endif // !INCLUDED_UTEDITOR_CLASSES
#endif // !NAMES_ONLY


#ifndef NAMES_ONLY
#undef AUTOGENERATE_NAME
#undef AUTOGENERATE_FUNCTION
#endif

#ifdef STATIC_LINKING_MOJO
#ifndef UTEDITOR_NATIVE_DEFS
#define UTEDITOR_NATIVE_DEFS

DECLARE_NATIVE_TYPE(UTEditor,UGenericBrowserType_UTMapMusicInfo);
DECLARE_NATIVE_TYPE(UTEditor,UUTEditorEngine);
DECLARE_NATIVE_TYPE(UTEditor,UUTEditorUISceneClient);
DECLARE_NATIVE_TYPE(UTEditor,UUTUnrealEdEngine);

#define AUTO_INITIALIZE_REGISTRANTS_UTEDITOR \
	UGenericBrowserType_UTMapMusicInfo::StaticClass(); \
	UUTEditorEngine::StaticClass(); \
	UUTEditorUISceneClient::StaticClass(); \
	UUTMapMusicInfoFactoryNew::StaticClass(); \
	UUTUnrealEdEngine::StaticClass(); \

#endif // UTEDITOR_NATIVE_DEFS

#ifdef NATIVES_ONLY
#endif // NATIVES_ONLY
#endif // STATIC_LINKING_MOJO

#ifdef VERIFY_CLASS_SIZES
VERIFY_CLASS_SIZE_NODIE(UGenericBrowserType_UTMapMusicInfo)
VERIFY_CLASS_SIZE_NODIE(UUTEditorEngine)
VERIFY_CLASS_SIZE_NODIE(UUTEditorUISceneClient)
VERIFY_CLASS_SIZE_NODIE(UUTUnrealEdEngine)
#endif // VERIFY_CLASS_SIZES
#endif // !ENUMS_ONLY

#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif
