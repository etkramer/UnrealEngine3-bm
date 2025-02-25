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

#ifndef INCLUDED_EDITOR_ENUMS
#define INCLUDED_EDITOR_ENUMS 1


#endif // !INCLUDED_EDITOR_ENUMS
#endif // !NO_ENUMS

#if !ENUMS_ONLY

#ifndef NAMES_ONLY
#define AUTOGENERATE_NAME(name) extern FName EDITOR_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#endif

AUTOGENERATE_NAME(Build)

#ifndef NAMES_ONLY

#ifndef INCLUDED_EDITOR_CLASSES
#define INCLUDED_EDITOR_CLASSES 1

struct BrushBuilder_eventBuild_Parms
{
    UBOOL ReturnValue;
    BrushBuilder_eventBuild_Parms(EEventParm)
    {
    }
};
class UBrushBuilder : public UObject
{
public:
    //## BEGIN PROPS BrushBuilder
    FStringNoInit BitmapFilename;
    FStringNoInit ToolTip;
    TArrayNoInit<FVector> Vertices;
    TArrayNoInit<FBuilderPoly> Polys;
    FName Group;
    BITFIELD MergeCoplanars:1;
    //## END PROPS BrushBuilder

    virtual void BeginBrush(UBOOL InMergeCoplanars,FName InGroup);
    virtual UBOOL EndBrush();
    virtual INT GetVertexCount();
    virtual FVector GetVertex(INT I);
    virtual INT GetPolyCount();
    virtual UBOOL BadParameters(const FString& msg=TEXT(""));
    virtual INT Vertexv(FVector V);
    virtual INT Vertex3f(FLOAT X,FLOAT Y,FLOAT Z);
    virtual void Poly3i(INT Direction,INT I,INT J,INT K,FName ItemName=NAME_None,UBOOL bIsTwoSidedNonSolid=FALSE);
    virtual void Poly4i(INT Direction,INT I,INT J,INT K,INT L,FName ItemName=NAME_None,UBOOL bIsTwoSidedNonSolid=FALSE);
    virtual void PolyBegin(INT Direction,FName ItemName=NAME_None);
    virtual void Polyi(INT I);
    virtual void PolyEnd();
    DECLARE_FUNCTION(execBeginBrush)
    {
        P_GET_UBOOL(InMergeCoplanars);
        P_GET_NAME(InGroup);
        P_FINISH;
        BeginBrush(InMergeCoplanars,InGroup);
    }
    DECLARE_FUNCTION(execEndBrush)
    {
        P_FINISH;
        *(UBOOL*)Result=EndBrush();
    }
    DECLARE_FUNCTION(execGetVertexCount)
    {
        P_FINISH;
        *(INT*)Result=GetVertexCount();
    }
    DECLARE_FUNCTION(execGetVertex)
    {
        P_GET_INT(I);
        P_FINISH;
        *(FVector*)Result=GetVertex(I);
    }
    DECLARE_FUNCTION(execGetPolyCount)
    {
        P_FINISH;
        *(INT*)Result=GetPolyCount();
    }
    DECLARE_FUNCTION(execBadParameters)
    {
        P_GET_STR_OPTX(msg,TEXT(""));
        P_FINISH;
        *(UBOOL*)Result=BadParameters(msg);
    }
    DECLARE_FUNCTION(execVertexv)
    {
        P_GET_STRUCT(FVector,V);
        P_FINISH;
        *(INT*)Result=Vertexv(V);
    }
    DECLARE_FUNCTION(execVertex3f)
    {
        P_GET_FLOAT(X);
        P_GET_FLOAT(Y);
        P_GET_FLOAT(Z);
        P_FINISH;
        *(INT*)Result=Vertex3f(X,Y,Z);
    }
    DECLARE_FUNCTION(execPoly3i)
    {
        P_GET_INT(Direction);
        P_GET_INT(I);
        P_GET_INT(J);
        P_GET_INT(K);
        P_GET_NAME_OPTX(ItemName,NAME_None);
        P_GET_UBOOL_OPTX(bIsTwoSidedNonSolid,FALSE);
        P_FINISH;
        Poly3i(Direction,I,J,K,ItemName,bIsTwoSidedNonSolid);
    }
    DECLARE_FUNCTION(execPoly4i)
    {
        P_GET_INT(Direction);
        P_GET_INT(I);
        P_GET_INT(J);
        P_GET_INT(K);
        P_GET_INT(L);
        P_GET_NAME_OPTX(ItemName,NAME_None);
        P_GET_UBOOL_OPTX(bIsTwoSidedNonSolid,FALSE);
        P_FINISH;
        Poly4i(Direction,I,J,K,L,ItemName,bIsTwoSidedNonSolid);
    }
    DECLARE_FUNCTION(execPolyBegin)
    {
        P_GET_INT(Direction);
        P_GET_NAME_OPTX(ItemName,NAME_None);
        P_FINISH;
        PolyBegin(Direction,ItemName);
    }
    DECLARE_FUNCTION(execPolyi)
    {
        P_GET_INT(I);
        P_FINISH;
        Polyi(I);
    }
    DECLARE_FUNCTION(execPolyEnd)
    {
        P_FINISH;
        PolyEnd();
    }
    UBOOL eventBuild()
    {
        BrushBuilder_eventBuild_Parms Parms(EC_EventParm);
        Parms.ReturnValue=FALSE;
        ProcessEvent(FindFunctionChecked(EDITOR_Build),&Parms);
        return Parms.ReturnValue;
    }
    DECLARE_ABSTRACT_CLASS(UBrushBuilder,UObject,0,Editor)
    NO_DEFAULT_CONSTRUCTOR(UBrushBuilder)
};

class UCubeBuilder : public UBrushBuilder
{
public:
    //## BEGIN PROPS CubeBuilder
    FLOAT X;
    FLOAT Y;
    FLOAT Z;
    FLOAT WallThickness;
    FName GroupName;
    BITFIELD Hollow:1;
    BITFIELD Tessellated:1;
    //## END PROPS CubeBuilder

    DECLARE_CLASS(UCubeBuilder,UBrushBuilder,0,Editor)
    NO_DEFAULT_CONSTRUCTOR(UCubeBuilder)
};

struct FAssetReferences
{
    TArrayNoInit<FString> AssetReference;

    /** Constructors */
    FAssetReferences() {}
    FAssetReferences(EEventParm)
    {
        appMemzero(this, sizeof(FAssetReferences));
    }
};

struct FTagInfo
{
    FStringNoInit AssetTypeName;
    class UClass* AssetType;
    TArrayNoInit<FName> Tags;
    TArrayNoInit<struct FAssetReferences> Assets;

    /** Constructors */
    FTagInfo() {}
    FTagInfo(EEventParm)
    {
        appMemzero(this, sizeof(FTagInfo));
    }
};

class UContentTagIndex : public UObject
{
public:
    //## BEGIN PROPS ContentTagIndex
    INT VersionInfo;
    TArrayNoInit<struct FTagInfo> DefaultTags;
    TArrayNoInit<struct FTagInfo> Tags;
    //## END PROPS ContentTagIndex

    DECLARE_CLASS(UContentTagIndex,UObject,0|CLASS_Config,Editor)
    static const TCHAR* StaticConfigName() {return TEXT("Editor");}

	typedef TMap<UClass*,TMap<FString,TArray<FName> > > AssetMapType;

	/** Searches through DefaultsTags to find the matching entry based on AssetType */
	FTagInfo* FindDefaultTagInfoForClass(UClass *ClassToFind);

	/** Searches through Tags to find the matching entry based on AssetType, creating a new entry if none was found. */
	FTagInfo& GetTagInfo(UClass *ClassToFind);

	/** Adds a new content reference to the matching tags inside of the supplied TagInfo. */
	void AddContentReference(FTagInfo &TagInfo, TArray<FName> &AssetTags, FString AssetReference);

	/** Builds a map of asset types to assets to tags for parsing/merging/etc. */
	void BuildAssetTypeToAssetMap(AssetMapType &OutAssetTypeToAssetMap);

	/** Loads the reference content tag index and merges the results into this index */
	void MergeFromRefContentTagIndex();

	/** 
	 *  Fills in the content tags for the specified object.  This is used to get around having no
	 *  common parent (other than UObject) for the various asset types.
	 */
	static void ApplyTagsToObjects(TArray<UObject*> &Objects, UClass* ObjectsClass, TArray<FName> &TagsToApply);

	/** Gets the cumulative set of tags from the supplied objects. */
	static void GetTagsFromObjects(TArray<UObject*> &Objects, UClass* ObjectsClass, TArray<FName> &OutTags);

	/** Gets/creates the local content tag index. */
	static UContentTagIndex* GetLocalContentTagIndex();

	/** Saves the local index */
	static void SaveLocalContentTagIndex();
};

class UEditorUserSettings : public UObject
{
public:
    //## BEGIN PROPS EditorUserSettings
    BITFIELD AllowFlightCameraToRemapKeys:1;
    FColor PreviewThumbnailBackgroundColor;
    FColor PreviewThumbnailTranslucentMaterialBackgroundColor;
    //## END PROPS EditorUserSettings

    DECLARE_CLASS(UEditorUserSettings,UObject,0|CLASS_Config,Editor)
    static const TCHAR* StaticConfigName() {return TEXT("EditorUserSettings");}

    NO_DEFAULT_CONSTRUCTOR(UEditorUserSettings)
};

class UGeomModifier : public UObject
{
public:
    //## BEGIN PROPS GeomModifier
    FStringNoInit Description;
    BITFIELD bPushButton:1;
    BITFIELD bInitialized:1;
    //## END PROPS GeomModifier

    DECLARE_ABSTRACT_CLASS(UGeomModifier,UObject,0,Editor)
	/**
	 * @return		The modifier's description string.
	 */
	const FString& GetModifierDescription() const;

	/**
	 * @return		TRUE if the key was handled by this editor mode tool.
	 */
	virtual UBOOL InputKey(struct FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event);

	/**
	 * @return		TRUE if the delta was handled by this editor mode tool.
	 */
	virtual UBOOL InputDelta(struct FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale);

	/**
	 * Applies the modifier.  Does nothing if the editor is not in geometry mode.
	 *
	 * @return		TRUE if something happened.
	 */
 	UBOOL Apply();

	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

	/**
	 * Gives the individual modifiers a chance to do something the first time they are activated.
	 */
	virtual void Initialize();
	
	/**
	 * Starts the modification of geometry data.
	 */
	UBOOL StartModify();

	/**
	 * Ends the modification of geometry data.
	 */
	UBOOL EndModify();

	/**
	 * Handles the starting of transactions against the selected ABrushes.
	 */
	void StartTrans();
	
	/**
	 * Handles the stopping of transactions against the selected ABrushes.
	 */
	void EndTrans();

protected:
	/**
	 * Interface for displaying error messages.
	 *
	 * @param	InErrorMsg		The error message to display.
	 */
	void GeomError(const FString& InErrorMsg);
	
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();
};

class UGeomModifier_Edit : public UGeomModifier
{
public:
    //## BEGIN PROPS GeomModifier_Edit
    //## END PROPS GeomModifier_Edit

    DECLARE_CLASS(UGeomModifier_Edit,UGeomModifier,0,Editor)
	/**
	 * @return		TRUE if the delta was handled by this editor mode tool.
	 */
	virtual UBOOL InputDelta(struct FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale);	
};

class UGeomModifier_Clip : public UGeomModifier_Edit
{
public:
    //## BEGIN PROPS GeomModifier_Clip
    BITFIELD bFlipNormal:1 GCC_BITFIELD_MAGIC;
    BITFIELD bSplit:1;
    //## END PROPS GeomModifier_Clip

    DECLARE_CLASS(UGeomModifier_Clip,UGeomModifier_Edit,0,Editor)
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

	/**
	 * @return		TRUE if the key was handled by this editor mode tool.
	 */
	virtual UBOOL InputKey(struct FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event);

protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();

private:
 	void ApplyClip();
};

class UGeomModifier_Create : public UGeomModifier_Edit
{
public:
    //## BEGIN PROPS GeomModifier_Create
    //## END PROPS GeomModifier_Create

    DECLARE_CLASS(UGeomModifier_Create,UGeomModifier_Edit,0,Editor)
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();
};

class UGeomModifier_Delete : public UGeomModifier_Edit
{
public:
    //## BEGIN PROPS GeomModifier_Delete
    //## END PROPS GeomModifier_Delete

    DECLARE_CLASS(UGeomModifier_Delete,UGeomModifier_Edit,0,Editor)
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();
};

class UGeomModifier_Extrude : public UGeomModifier_Edit
{
public:
    //## BEGIN PROPS GeomModifier_Extrude
    INT Length;
    INT Segments;
    //## END PROPS GeomModifier_Extrude

    DECLARE_CLASS(UGeomModifier_Extrude,UGeomModifier_Edit,0,Editor)
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

	/**
	 * Gives the individual modifiers a chance to do something the first time they are activated.
	 */
	virtual void Initialize();

protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();
 	
private:
 	void Apply(INT InLength, INT InSegments);
};

class UGeomModifier_Flip : public UGeomModifier_Edit
{
public:
    //## BEGIN PROPS GeomModifier_Flip
    //## END PROPS GeomModifier_Flip

    DECLARE_CLASS(UGeomModifier_Flip,UGeomModifier_Edit,0,Editor)
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();
};

class UGeomModifier_Lathe : public UGeomModifier_Edit
{
public:
    //## BEGIN PROPS GeomModifier_Lathe
    INT TotalSegments;
    INT Segments;
    BYTE Axis;
    //## END PROPS GeomModifier_Lathe

    DECLARE_CLASS(UGeomModifier_Lathe,UGeomModifier_Edit,0,Editor)
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

	/**
	 * Gives the individual modifiers a chance to do something the first time they are activated.
	 */
	virtual void Initialize();

protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();

private:
	void Apply(INT InTotalSegments, INT InSegments, EAxis InAxis);
};

class UGeomModifier_Split : public UGeomModifier_Edit
{
public:
    //## BEGIN PROPS GeomModifier_Split
    //## END PROPS GeomModifier_Split

    DECLARE_CLASS(UGeomModifier_Split,UGeomModifier_Edit,0,Editor)
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();
};

class UGeomModifier_Triangulate : public UGeomModifier_Edit
{
public:
    //## BEGIN PROPS GeomModifier_Triangulate
    //## END PROPS GeomModifier_Triangulate

    DECLARE_CLASS(UGeomModifier_Triangulate,UGeomModifier_Edit,0,Editor)
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();
};

class UGeomModifier_Turn : public UGeomModifier_Edit
{
public:
    //## BEGIN PROPS GeomModifier_Turn
    //## END PROPS GeomModifier_Turn

    DECLARE_CLASS(UGeomModifier_Turn,UGeomModifier_Edit,0,Editor)
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();
};

class UGeomModifier_Weld : public UGeomModifier_Edit
{
public:
    //## BEGIN PROPS GeomModifier_Weld
    //## END PROPS GeomModifier_Weld

    DECLARE_CLASS(UGeomModifier_Weld,UGeomModifier_Edit,0,Editor)
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);
	
protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();
};

class ULightingChannelsObject : public UObject
{
public:
    //## BEGIN PROPS LightingChannelsObject
    FLightingChannelContainer LightingChannels;
    //## END PROPS LightingChannelsObject

    DECLARE_CLASS(ULightingChannelsObject,UObject,0,Editor)
    NO_DEFAULT_CONSTRUCTOR(ULightingChannelsObject)
};

class UEditorViewportInput : public UInput
{
public:
    //## BEGIN PROPS EditorViewportInput
    class UEditorEngine* Editor;
    //## END PROPS EditorViewportInput

    DECLARE_CLASS(UEditorViewportInput,UInput,0|CLASS_Transient|CLASS_Config,Editor)
	virtual UBOOL Exec(const TCHAR* Cmd,FOutputDevice& Ar);
};

#endif // !INCLUDED_EDITOR_CLASSES
#endif // !NAMES_ONLY

AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execPolyEnd);
AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execPolyi);
AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execPolyBegin);
AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execPoly4i);
AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execPoly3i);
AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execVertex3f);
AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execVertexv);
AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execBadParameters);
AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execGetPolyCount);
AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execGetVertex);
AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execGetVertexCount);
AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execEndBrush);
AUTOGENERATE_FUNCTION(UBrushBuilder,-1,execBeginBrush);

#ifndef NAMES_ONLY
#undef AUTOGENERATE_NAME
#undef AUTOGENERATE_FUNCTION
#endif

#ifdef STATIC_LINKING_MOJO
#ifndef EDITOR_NATIVE_DEFS
#define EDITOR_NATIVE_DEFS

DECLARE_NATIVE_TYPE(Editor,UBrushBuilder);
DECLARE_NATIVE_TYPE(Editor,UContentTagIndex);
DECLARE_NATIVE_TYPE(Editor,UCubeBuilder);
DECLARE_NATIVE_TYPE(Editor,UEditorUserSettings);
DECLARE_NATIVE_TYPE(Editor,UEditorViewportInput);
DECLARE_NATIVE_TYPE(Editor,UGeomModifier);
DECLARE_NATIVE_TYPE(Editor,UGeomModifier_Clip);
DECLARE_NATIVE_TYPE(Editor,UGeomModifier_Create);
DECLARE_NATIVE_TYPE(Editor,UGeomModifier_Delete);
DECLARE_NATIVE_TYPE(Editor,UGeomModifier_Edit);
DECLARE_NATIVE_TYPE(Editor,UGeomModifier_Extrude);
DECLARE_NATIVE_TYPE(Editor,UGeomModifier_Flip);
DECLARE_NATIVE_TYPE(Editor,UGeomModifier_Lathe);
DECLARE_NATIVE_TYPE(Editor,UGeomModifier_Split);
DECLARE_NATIVE_TYPE(Editor,UGeomModifier_Triangulate);
DECLARE_NATIVE_TYPE(Editor,UGeomModifier_Turn);
DECLARE_NATIVE_TYPE(Editor,UGeomModifier_Weld);
DECLARE_NATIVE_TYPE(Editor,ULightingChannelsObject);

#define AUTO_INITIALIZE_REGISTRANTS_EDITOR \
	UAnalyzeContentCommandlet::StaticClass(); \
	UAnalyzeCookedContentCommandlet::StaticClass(); \
	UAnalyzeCookedPackagesCommandlet::StaticClass(); \
	UAnalyzeFallbackMaterialsCommandlet::StaticClass(); \
	UAnalyzeReferencedContentCommandlet::StaticClass(); \
	UAnalyzeScriptCommandlet::StaticClass(); \
	UAnimSetFactoryNew::StaticClass(); \
	UAnimTreeFactoryNew::StaticClass(); \
	UBatchExportCommandlet::StaticClass(); \
	UBrushBuilder::StaticClass(); \
	GNativeLookupFuncs[Lookup++] = &FindEditorUBrushBuilderNative; \
	UBuildContentTagIndexCommandlet::StaticClass(); \
	UCameraAnimFactory::StaticClass(); \
	UChangePrefabSequenceClassCommandlet::StaticClass(); \
	UClassExporterUC::StaticClass(); \
	UClassFactoryUC::StaticClass(); \
	UColladaFactory::StaticClass(); \
	UCompressAnimationsCommandlet::StaticClass(); \
	UConformCommandlet::StaticClass(); \
	UContentTagIndex::StaticClass(); \
	UCookPackagesCommandlet::StaticClass(); \
	UCreateDefaultStyleCommandlet::StaticClass(); \
	UCubeBuilder::StaticClass(); \
	UCurveEdPresetCurveFactoryNew::StaticClass(); \
	UCutDownContentCommandlet::StaticClass(); \
	UDecalMaterialFactoryNew::StaticClass(); \
	UDiffPackagesCommandlet::StaticClass(); \
	UDumpEmittersCommandlet::StaticClass(); \
	UDumpLightmapInfoCommandlet::StaticClass(); \
	UDumpShadersCommandlet::StaticClass(); \
	UEditorComponent::StaticClass(); \
	UEditorEngine::StaticClass(); \
	UEditorPlayer::StaticClass(); \
	UEditorUserSettings::StaticClass(); \
	UEditorViewportInput::StaticClass(); \
	UEdModeComponent::StaticClass(); \
	UExamineOutersCommandlet::StaticClass(); \
	UExporterT3DX::StaticClass(); \
	UExportLocCommandlet::StaticClass(); \
	UFindDuplicateKismetObjectsCommandlet::StaticClass(); \
	UFindEmitterMismatchedLODsCommandlet::StaticClass(); \
	UFindEmitterModifiedLODsCommandlet::StaticClass(); \
	UFindEmitterModuleLODErrorsCommandlet::StaticClass(); \
	UFindQuestionableTexturesCommandlet::StaticClass(); \
	UFindRenamedPrefabSequencesCommandlet::StaticClass(); \
	UFindStaticActorsRefsCommandlet::StaticClass(); \
	UFindTexturesWithMissingPhysicalMaterialsCommandlet::StaticClass(); \
	UFixAmbiguousMaterialParametersCommandlet::StaticClass(); \
	UFixupEmittersCommandlet::StaticClass(); \
	UFixupRedirectsCommandlet::StaticClass(); \
	UFixupSourceUVsCommandlet::StaticClass(); \
	UFonixFactory::StaticClass(); \
	UFontFactory::StaticClass(); \
	UGeomModifier::StaticClass(); \
	UGeomModifier_Clip::StaticClass(); \
	UGeomModifier_Create::StaticClass(); \
	UGeomModifier_Delete::StaticClass(); \
	UGeomModifier_Edit::StaticClass(); \
	UGeomModifier_Extrude::StaticClass(); \
	UGeomModifier_Flip::StaticClass(); \
	UGeomModifier_Lathe::StaticClass(); \
	UGeomModifier_Split::StaticClass(); \
	UGeomModifier_Triangulate::StaticClass(); \
	UGeomModifier_Turn::StaticClass(); \
	UGeomModifier_Weld::StaticClass(); \
	ULensFlareFactoryNew::StaticClass(); \
	ULevelExporterOBJ::StaticClass(); \
	ULevelExporterSTL::StaticClass(); \
	ULevelExporterT3D::StaticClass(); \
	ULevelFactory::StaticClass(); \
	ULightingChannelsObject::StaticClass(); \
	UListCorruptedComponentsCommandlet::StaticClass(); \
	UListDistanceCrossFadeNodesCommandlet::StaticClass(); \
	UListEmittersUsingModuleCommandlet::StaticClass(); \
	UListLoopingEmittersCommandlet::StaticClass(); \
	UListPackagesReferencingCommandlet::StaticClass(); \
	UListPSysFixedBoundSettingCommandlet::StaticClass(); \
	UListScriptReferencedContentCommandlet::StaticClass(); \
	UListSoundNodeWavesCommandlet::StaticClass(); \
	ULoadPackageCommandlet::StaticClass(); \
	ULocSoundInfoCommandlet::StaticClass(); \
	UMakeCommandlet::StaticClass(); \
	UMaterialExporterT3D::StaticClass(); \
	UMaterialFactory::StaticClass(); \
	UMaterialFactoryNew::StaticClass(); \
	UMaterialInstanceConstantFactoryNew::StaticClass(); \
	UMaterialInstanceTimeVaryingFactoryNew::StaticClass(); \
	UMergePackagesCommandlet::StaticClass(); \
	UMineCookedPackagesCommandlet::StaticClass(); \
	UModelExporterT3D::StaticClass(); \
	UModelFactory::StaticClass(); \
	UObjectExporterT3D::StaticClass(); \
	UPackageExporterT3D::StaticClass(); \
	UPackageFactory::StaticClass(); \
	UParticleSystemFactoryNew::StaticClass(); \
	UPatchScriptCommandlet::StaticClass(); \
	UPerformMapCheckCommandlet::StaticClass(); \
	UPerformTerrainMaterialDumpCommandlet::StaticClass(); \
	UPersistentCookerData::StaticClass(); \
	UPhysicalMaterialFactoryNew::StaticClass(); \
	UPhysXParticleSystemFactoryNew::StaticClass(); \
	UPIEToNormalCommandlet::StaticClass(); \
	UPkgInfoCommandlet::StaticClass(); \
	UPolysExporterT3D::StaticClass(); \
	UPolysFactory::StaticClass(); \
	UPostProcessFactoryNew::StaticClass(); \
	UPrecompileShadersCommandlet::StaticClass(); \
	URebuildMapCommandlet::StaticClass(); \
	URenderTargetCubeExporterTGA::StaticClass(); \
	URenderTargetExporterTGA::StaticClass(); \
	UReplaceActorCommandlet::StaticClass(); \
	UResavePackagesCommandlet::StaticClass(); \
	UScaleAudioVolumeCommandlet::StaticClass(); \
	USequenceExporterT3D::StaticClass(); \
	USequenceFactory::StaticClass(); \
	USetMaterialUsageCommandlet::StaticClass(); \
	USetPackageFlagsCommandlet::StaticClass(); \
	USetTextureLODGroupCommandlet::StaticClass(); \
	UShowObjectCountCommandlet::StaticClass(); \
	UShowStylesCommandlet::StaticClass(); \
	UShowTaggedPropsCommandlet::StaticClass(); \
	USkeletalMeshFactory::StaticClass(); \
	USoundCueFactoryNew::StaticClass(); \
	USoundExporterWAV::StaticClass(); \
	USoundFactory::StaticClass(); \
	USoundGroupInfoCommandlet::StaticClass(); \
	USoundModeFactory::StaticClass(); \
	USoundSurroundExporterWAV::StaticClass(); \
	USoundSurroundFactory::StaticClass(); \
	USoundTTSFactory::StaticClass(); \
	USpeedTreeFactory::StaticClass(); \
	UStaticMeshExporterOBJ::StaticClass(); \
	UStaticMeshExporterT3D::StaticClass(); \
	UStaticMeshFactory::StaticClass(); \
	UStripSourceCommandlet::StaticClass(); \
	UTerrainExporterT3D::StaticClass(); \
	UTerrainFactory::StaticClass(); \
	UTerrainHeightMapExporter::StaticClass(); \
	UTerrainHeightMapExporterG16BMPT3D::StaticClass(); \
	UTerrainHeightMapExporterTextT3D::StaticClass(); \
	UTerrainHeightMapFactory::StaticClass(); \
	UTerrainHeightMapFactoryG16BMP::StaticClass(); \
	UTerrainHeightMapFactoryG16BMPT3D::StaticClass(); \
	UTerrainHeightMapFactoryTextT3D::StaticClass(); \
	UTerrainLayerSetupFactoryNew::StaticClass(); \
	UTerrainMaterialFactoryNew::StaticClass(); \
	UTestCompressionCommandlet::StaticClass(); \
	UTestWordWrapCommandlet::StaticClass(); \
	UTextBufferExporterTXT::StaticClass(); \
	UTextureCubeFactoryNew::StaticClass(); \
	UTextureExporterBMP::StaticClass(); \
	UTextureExporterPCX::StaticClass(); \
	UTextureExporterT3D::StaticClass(); \
	UTextureExporterTGA::StaticClass(); \
	UTextureFactory::StaticClass(); \
	UTextureMovieFactory::StaticClass(); \
	UTextureRenderTargetCubeFactoryNew::StaticClass(); \
	UTextureRenderTargetFactoryNew::StaticClass(); \
	UTransactor::StaticClass(); \
	UTransBuffer::StaticClass(); \
	UTrueTypeFontFactory::StaticClass(); \
	UTrueTypeMultiFontFactory::StaticClass(); \
	UUT3MapStatsCommandlet::StaticClass(); \
	UWrangleContentCommandlet::StaticClass(); \

#endif // EDITOR_NATIVE_DEFS

#ifdef NATIVES_ONLY
NATIVE_INFO(UBrushBuilder) GEditorUBrushBuilderNatives[] = 
{ 
	MAP_NATIVE(UBrushBuilder,execPolyEnd)
	MAP_NATIVE(UBrushBuilder,execPolyi)
	MAP_NATIVE(UBrushBuilder,execPolyBegin)
	MAP_NATIVE(UBrushBuilder,execPoly4i)
	MAP_NATIVE(UBrushBuilder,execPoly3i)
	MAP_NATIVE(UBrushBuilder,execVertex3f)
	MAP_NATIVE(UBrushBuilder,execVertexv)
	MAP_NATIVE(UBrushBuilder,execBadParameters)
	MAP_NATIVE(UBrushBuilder,execGetPolyCount)
	MAP_NATIVE(UBrushBuilder,execGetVertex)
	MAP_NATIVE(UBrushBuilder,execGetVertexCount)
	MAP_NATIVE(UBrushBuilder,execEndBrush)
	MAP_NATIVE(UBrushBuilder,execBeginBrush)
	{NULL,NULL}
};
IMPLEMENT_NATIVE_HANDLER(Editor,UBrushBuilder);

#endif // NATIVES_ONLY
#endif // STATIC_LINKING_MOJO

#ifdef VERIFY_CLASS_SIZES
VERIFY_CLASS_OFFSET_NODIE(U,BrushBuilder,BitmapFilename)
VERIFY_CLASS_OFFSET_NODIE(U,BrushBuilder,Group)
VERIFY_CLASS_SIZE_NODIE(UBrushBuilder)
VERIFY_CLASS_OFFSET_NODIE(U,ContentTagIndex,VersionInfo)
VERIFY_CLASS_OFFSET_NODIE(U,ContentTagIndex,Tags)
VERIFY_CLASS_SIZE_NODIE(UContentTagIndex)
VERIFY_CLASS_OFFSET_NODIE(U,CubeBuilder,X)
VERIFY_CLASS_OFFSET_NODIE(U,CubeBuilder,GroupName)
VERIFY_CLASS_SIZE_NODIE(UCubeBuilder)
VERIFY_CLASS_OFFSET_NODIE(U,EditorComponent,GridColorHi)
VERIFY_CLASS_OFFSET_NODIE(U,EditorComponent,GridColorLo)
VERIFY_CLASS_OFFSET_NODIE(U,EditorComponent,PerspectiveGridSize)
VERIFY_CLASS_OFFSET_NODIE(U,EditorComponent,PivotColor)
VERIFY_CLASS_OFFSET_NODIE(U,EditorComponent,PivotSize)
VERIFY_CLASS_OFFSET_NODIE(U,EditorComponent,BaseBoxColor)
VERIFY_CLASS_SIZE_NODIE(UEditorComponent)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,TempModel)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,Trans)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,Results)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,ActorProperties)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,LevelProperties)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,Bad)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,Bkgnd)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,BkgndHi)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,BadHighlight)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,MaterialArrow)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,MaterialBackdrop)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,PreviewSoundCue)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,PreviewAudioComponent)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,TexPropCube)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,TexPropSphere)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,TexPropPlane)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,TexPropCylinder)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,TerrainEditBrush)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,ClickFlags)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,ParentContext)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,ClickLocation)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,ClickPlane)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,MouseMovement)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,ViewportClients)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,FarClippingPlane)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,DetailMode)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,FOVAngle)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,AutoSaveDir)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,GameCommandLine)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,EditPackages)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,EditPackagesInPath)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,EditPackagesOutPath)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,FRScriptOutputPath)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,HeightMapExportClassName)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,ActorFactories)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,UserOpenedFile)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,InEditorGameURLOptions)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,PlayWorld)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,PlayWorldLocation)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,PlayWorldRotation)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,PlayWorldDestination)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,PlayInEditorViewportIndex)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,InEditorPropagator)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,RemotePropagator)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,ScratchRenderTarget)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,StreamingBoundsTexture)
VERIFY_CLASS_OFFSET_NODIE(U,EditorEngine,UserSettings)
VERIFY_CLASS_SIZE_NODIE(UEditorEngine)
VERIFY_CLASS_OFFSET_NODIE(U,EditorUserSettings,PreviewThumbnailBackgroundColor)
VERIFY_CLASS_OFFSET_NODIE(U,EditorUserSettings,PreviewThumbnailTranslucentMaterialBackgroundColor)
VERIFY_CLASS_SIZE_NODIE(UEditorUserSettings)
VERIFY_CLASS_OFFSET_NODIE(U,EditorViewportInput,Editor)
VERIFY_CLASS_SIZE_NODIE(UEditorViewportInput)
VERIFY_CLASS_SIZE_NODIE(UEdModeComponent)
VERIFY_CLASS_OFFSET_NODIE(U,GeomModifier,Description)
VERIFY_CLASS_SIZE_NODIE(UGeomModifier)
VERIFY_CLASS_SIZE_NODIE(UGeomModifier_Clip)
VERIFY_CLASS_SIZE_NODIE(UGeomModifier_Create)
VERIFY_CLASS_SIZE_NODIE(UGeomModifier_Delete)
VERIFY_CLASS_SIZE_NODIE(UGeomModifier_Edit)
VERIFY_CLASS_OFFSET_NODIE(U,GeomModifier_Extrude,Length)
VERIFY_CLASS_OFFSET_NODIE(U,GeomModifier_Extrude,Segments)
VERIFY_CLASS_SIZE_NODIE(UGeomModifier_Extrude)
VERIFY_CLASS_SIZE_NODIE(UGeomModifier_Flip)
VERIFY_CLASS_OFFSET_NODIE(U,GeomModifier_Lathe,TotalSegments)
VERIFY_CLASS_OFFSET_NODIE(U,GeomModifier_Lathe,Axis)
VERIFY_CLASS_SIZE_NODIE(UGeomModifier_Lathe)
VERIFY_CLASS_SIZE_NODIE(UGeomModifier_Split)
VERIFY_CLASS_SIZE_NODIE(UGeomModifier_Triangulate)
VERIFY_CLASS_SIZE_NODIE(UGeomModifier_Turn)
VERIFY_CLASS_SIZE_NODIE(UGeomModifier_Weld)
VERIFY_CLASS_OFFSET_NODIE(U,LightingChannelsObject,LightingChannels)
VERIFY_CLASS_SIZE_NODIE(ULightingChannelsObject)
#endif // VERIFY_CLASS_SIZES
#endif // !ENUMS_ONLY

#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif
