/*=============================================================================
	EditorPrivate.h: Unreal editor private header file.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INCL_EDITOR_PRIVATE_H_
#define _INCL_EDITOR_PRIVATE_H_

/*-----------------------------------------------------------------------------
	Advance editor private definitions.
-----------------------------------------------------------------------------*/

//
// Things to set in mapSetBrush.
//
enum EMapSetBrushFlags				
{
	MSB_BrushColor	= 1,			// Set brush color.
	MSB_Group		= 2,			// Set group.
	MSB_PolyFlags	= 4,			// Set poly flags.
	MSB_CSGOper		= 8,			// Set CSG operation.
};

// Byte describing effects for a mesh triangle.
enum EJSMeshTriType
{
	// Triangle types. Mutually exclusive.
	MTT_Normal				= 0,	// Normal one-sided.
	MTT_NormalTwoSided      = 1,    // Normal but two-sided.
	MTT_Translucent			= 2,	// Translucent two-sided.
	MTT_Masked				= 3,	// Masked two-sided.
	MTT_Modulate			= 4,	// Modulation blended two-sided.
	MTT_Placeholder			= 8,	// Placeholder triangle for positioning weapon. Invisible.
	// Bit flags. 
	MTT_Unlit				= 16,	// Full brightness, no lighting.
	MTT_Flat				= 32,	// Flat surface, don't do bMeshCurvy thing.
	MTT_Environment			= 64,	// Environment mapped.
	MTT_NoSmooth			= 128,	// No bilinear filtering on this poly's texture.
};


/*-----------------------------------------------------------------------------
	Editor public includes.
-----------------------------------------------------------------------------*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
	Editor private includes.
-----------------------------------------------------------------------------*/

/**
 * Provides access to the FEditorModeTools singleton.
 */
class FEditorModeTools& GEditorModeTools();

extern struct FEditorLevelViewportClient* GCurrentLevelEditingViewportClient;

/** Tracks the last level editing viewport client that received a key press. */
extern struct FEditorLevelViewportClient* GLastKeyLevelEditingViewportClient;

/*-----------------------------------------------------------------------------
	Exporters.
-----------------------------------------------------------------------------*/

class UTextBufferExporterTXT : public UExporter
{
	DECLARE_CLASS(UTextBufferExporterTXT,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Out, FFeedbackContext* Warn, DWORD PortFlags=0 );
};

#if 1
class USoundExporterWAV : public UExporter
{
	DECLARE_CLASS(USoundExporterWAV,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex = 0, DWORD PortFlags=0 );
};

#else
class USoundExporterOGG : public UExporter
{
	DECLARE_CLASS(USoundExporterOGG,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex = 0, DWORD PortFlags=0 );
};
#endif

class USoundSurroundExporterWAV : public UExporter
{
	DECLARE_CLASS(USoundSurroundExporterWAV,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();

	/** 
	* Number of binary files to export for this object. Should be 1 in the vast majority of cases. Noted exception would be multichannel sounds
	* which have upto 8 raw waves stored within them.
	*/
	virtual INT GetFileCount( void ) const;

	/** 
	 * Differentiates the filename for objects with multiple files to export. Only needs to be overridden if GetFileCount() returns > 1.
	 */
	virtual FString GetUniqueFilename( const TCHAR* Filename, INT FileIndex );

	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex = 0, DWORD PortFlags=0 );
};

class UClassExporterUC : public UExporter
{
	DECLARE_CLASS(UClassExporterUC,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 );
};

class UObjectExporterT3D : public UExporter
{
	DECLARE_CLASS(UObjectExporterT3D,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 );
};

class UPolysExporterT3D : public UExporter
{
	DECLARE_CLASS(UPolysExporterT3D,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 );
};

class UModelExporterT3D : public UExporter
{
	DECLARE_CLASS(UModelExporterT3D,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 );
};

class ULevelExporterT3D : public UExporter
{
	DECLARE_CLASS(ULevelExporterT3D,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 );
	virtual void ExportPackageObject(FExportPackageParams& ExpPackageParams);
	virtual void ExportPackageInners(FExportPackageParams& ExpPackageParams);
};

class ULevelExporterSTL : public UExporter
{
	DECLARE_CLASS(ULevelExporterSTL,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 );
};

class UPackageExporterT3D : public UExporter
{
	DECLARE_CLASS(UPackageExporterT3D,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 );
	virtual void ExportPackageObject(FExportPackageParams& ExpPackageParams);
	virtual void ExportPackageInners(FExportPackageParams& ExpPackageParams);
};

class UTextureExporterPCX : public UExporter
{
	DECLARE_CLASS(UTextureExporterPCX,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex = 0, DWORD PortFlags=0 );
};

class UTextureExporterBMP : public UExporter
{
	DECLARE_CLASS(UTextureExporterBMP,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex = 0, DWORD PortFlags=0 );
	UBOOL ExportBinary(const BYTE* Data, EPixelFormat Format, INT SizeX, INT SizeY, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, DWORD PortFlags = 0);
};
class UTextureExporterTGA : public UExporter
{
	DECLARE_CLASS(UTextureExporterTGA,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex = 0, DWORD PortFlags=0 );
};
class UStaticMeshExporterOBJ : public UExporter
{
	DECLARE_CLASS(UStaticMeshExporterOBJ,UExporter,0,Editor)
	/**
	* Initializes property values for intrinsic classes.  It is called immediately after the class default object
	* is initialized against its archetype, but before any objects of this class are created.
	*/
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 );
};

/** exports a render target's surface to a targa formatted file or archive */
class URenderTargetExporterTGA : public UExporter
{
	DECLARE_CLASS(URenderTargetExporterTGA,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex = 0, DWORD PortFlags=0 );
};

/** exports a single face of the cube render target's surfaces to a targa formatted file or archive */
class URenderTargetCubeExporterTGA : public UExporter
{
	DECLARE_CLASS(URenderTargetCubeExporterTGA,UExporter,0,Editor)

	/** cube map face to export - see ECubeTargetFace */
	BYTE	CubeFace;

	void StaticConstructor();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex = 0, DWORD PortFlags=0 );
};

class ULevelExporterOBJ : public UExporter
{
	DECLARE_CLASS(ULevelExporterOBJ,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 );
};
class UPolysExporterOBJ : public UExporter
{
	DECLARE_CLASS(UPolysExporterOBJ,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 );
};
class USequenceExporterT3D : public UExporter
{
	DECLARE_CLASS(USequenceExporterT3D,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 );
};

/*-----------------------------------------------------------------------------
	Terrain-related exporters
-----------------------------------------------------------------------------*/
class UTerrainHeightMapExporter;
struct FFilterLimit;
class UTerrainLayerSetup;
struct FTerrainDecoration;
struct FTerrainDecorationInstance;

class UTerrainExporterT3D : public UExporter
{
	static UBOOL			s_bHeightMapExporterArrayFilled;
	static TArray<UClass*>	s_HeightMapExporterArray;
	UObjectExporterT3D*		ObjectExporter;
	UBOOL					bExportingTerrainOnly;

	DECLARE_CLASS(UTerrainExporterT3D,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	UBOOL ExportHeightMapData(ATerrain* Terrain, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	UBOOL ExportInfoData(ATerrain* Terrain, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	UBOOL ExportLayerData(ATerrain* Terrain, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	UBOOL ExportFilterLimit(FFilterLimit& FilterLimit, const TCHAR* Name, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	UBOOL ExportLayerSetup(UTerrainLayerSetup* Setup, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	UBOOL ExportDecoLayerData(ATerrain* Terrain, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	UBOOL ExportDecoration(FTerrainDecoration& Decoration, INT Index, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	UBOOL ExportDecorationInstance(FTerrainDecorationInstance& DecorationInst, INT Index, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	UBOOL ExportAlphaMapData(ATerrain* Terrain, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);

public:
	const UBOOL	IsExportingTerrainOnly() const								{	return bExportingTerrainOnly;						}
	void		SetIsExportingTerrainOnly(UBOOL bExportingTerrainOnlyIn)	{	bExportingTerrainOnly = bExportingTerrainOnlyIn;	}

private:
	UBOOL FindHeightMapExporters(FFeedbackContext* Warn);
};

struct FTerrainLayer;

class UTerrainHeightMapExporter : public UExporter
{
	DECLARE_CLASS(UTerrainHeightMapExporter,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	virtual UBOOL ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	virtual UBOOL ExportHeightData(ATerrain* Terrain, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	virtual UBOOL ExportHeightDataToFile(ATerrain* Terrain, const TCHAR* Directory, FFeedbackContext* Warn, DWORD PortFlags=0);
	virtual UBOOL ExportLayerDataToFile(ATerrain* Terrain, FTerrainLayer* Layer, const TCHAR* Directory, FFeedbackContext* Warn, DWORD PortFlags=0);
};

class UTerrainHeightMapExporterTextT3D : public UTerrainHeightMapExporter
{
	DECLARE_CLASS(UTerrainHeightMapExporterTextT3D,UTerrainHeightMapExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	virtual UBOOL ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	virtual UBOOL ExportHeightData(ATerrain* Terrain, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	virtual UBOOL ExportHeightDataToFile(ATerrain* Terrain, const TCHAR* Directory, FFeedbackContext* Warn, DWORD PortFlags=0);
	virtual UBOOL ExportLayerDataToFile(ATerrain* Terrain, FTerrainLayer* Layer, const TCHAR* Directory, FFeedbackContext* Warn, DWORD PortFlags=0);
};

class UTerrainHeightMapExporterG16BMPT3D : public UTerrainHeightMapExporter
{
	DECLARE_CLASS(UTerrainHeightMapExporterG16BMPT3D,UTerrainHeightMapExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	virtual UBOOL ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	virtual UBOOL ExportHeightData(ATerrain* Terrain, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
	virtual UBOOL ExportHeightDataToFile(ATerrain* Terrain, const TCHAR* Directory, FFeedbackContext* Warn, DWORD PortFlags=0);
	virtual UBOOL ExportLayerDataToFile(ATerrain* Terrain, FTerrainLayer* Layer, const TCHAR* Directory, FFeedbackContext* Warn, DWORD PortFlags=0);
};

/** exports a render target's surface to a targa formatted file or archive */
class USpeedTreeExporter : public UExporter
{
	DECLARE_CLASS(USpeedTreeExporter,UExporter,0,Editor)
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex = 0, DWORD PortFlags=0 );
};

/*-----------------------------------------------------------------------------
	'Extended' T3D exporters...
-----------------------------------------------------------------------------*/
class UExporterT3DX : public UExporter
{
	DECLARE_CLASS(UExporterT3DX,UExporter,0,Editor)

	void ExportObjectStructProperty(const FString& PropertyName, const FString& ScriptStructName, BYTE* DataValue, UObject* ParentObject, FOutputDevice& Ar, DWORD PortFlags);
	void ExportBooleanProperty(const FString& PropertyName, UBOOL BooleanValue, FOutputDevice& Ar, DWORD PortFlags);
	void ExportFloatProperty(const FString& PropertyName, FLOAT FloatValue, FOutputDevice& Ar, DWORD PortFlags);
	void ExportIntProperty(const FString& PropertyName, INT IntValue, FOutputDevice& Ar, DWORD PortFlags);
	void ExportStringProperty(const FString& PropertyName, const FString& StringValue, FOutputDevice& Ar, DWORD PortFlags);
	void ExportObjectProperty(const FString& PropertyName, const UObject* InExportingObject, const UObject* InObject, FOutputDevice& Ar, DWORD PortFlags);

	void ExportUntypedBulkData(FUntypedBulkData& BulkData, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags);
	void ExportBinaryBlob(INT BlobSize, BYTE* BlobData, FOutputDevice& Ar, DWORD PortFlags);

	virtual UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 ) { return FALSE; };
};

class UMaterialExporterT3D : public UExporterT3DX
{
	DECLARE_CLASS(UMaterialExporterT3D,UExporterT3DX,0,Editor)

	static const int VersionMax = 0;
	static const int VersionMin = 0;

	/**
	* Initializes property values for intrinsic classes.  It is called immediately after the class default object
	* is initialized against its archetype, but before any objects of this class are created.
	*/
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
};

class UStaticMeshExporterT3D : public UExporterT3DX
{
	DECLARE_CLASS(UStaticMeshExporterT3D,UExporterT3DX,0,Editor)

	static const int VersionMax = 0;
	static const int VersionMin = 0;

	/**
	* Initializes property values for intrinsic classes.  It is called immediately after the class default object
	* is initialized against its archetype, but before any objects of this class are created.
	*/
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);

	void ExportStaticMeshElement(const FString& Name, const FStaticMeshElement& SMElement, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags);
	void ExportStaticMeshTriangleBulkData(FStaticMeshTriangleBulkData& SMTBulkData, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags);
	UBOOL ExportRenderData(FStaticMeshRenderData& Model, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);
};

class UTextureExporterT3D : public UExporterT3DX
{
	DECLARE_CLASS(UTextureExporterT3D,UExporterT3DX,0,Editor)

	static const int VersionMax = 0;
	static const int VersionMin = 0;

	/**
	* Initializes property values for intrinsic classes.  It is called immediately after the class default object
	* is initialized against its archetype, but before any objects of this class are created.
	*/
	void InitializeIntrinsicPropertyValues();
	UBOOL ExportText(const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0);

	UBOOL ExportText_Texture2D(const FExportObjectInnerContext* Context, UTexture2D* InTexture2D, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags);
	UBOOL ExportText_TextureCube(const FExportObjectInnerContext* Context, UTextureCube* InTextureCube, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags);
};

/*-----------------------------------------------------------------------------
	Helpers.
-----------------------------------------------------------------------------*/

// Accepts a triangle (XYZ and UV values for each point) and returns a poly base and UV vectors
// NOTE : the UV coords should be scaled by the texture size
static inline void FTexCoordsToVectors(const FVector& V0, const FVector& UV0,
									   const FVector& V1, const FVector& InUV1,
									   const FVector& V2, const FVector& InUV2,
									   FVector* InBaseResult, FVector* InUResult, FVector* InVResult )
{
	// Create polygon normal.
	FVector PN = FVector((V0-V1) ^ (V2-V0));
	PN = PN.SafeNormal();

	FVector UV1( InUV1 );
	FVector UV2( InUV2 );

	// Fudge UV's to make sure no infinities creep into UV vector math, whenever we detect identical U or V's.
	if( ( UV0.X == UV1.X ) || ( UV2.X == UV1.X ) || ( UV2.X == UV0.X ) ||
		( UV0.Y == UV1.Y ) || ( UV2.Y == UV1.Y ) || ( UV2.Y == UV0.Y ) )
	{
		UV1 += FVector(0.004173f,0.004123f,0.0f);
		UV2 += FVector(0.003173f,0.003123f,0.0f);
	}

	//
	// Solve the equations to find our texture U/V vectors 'TU' and 'TV' by stacking them 
	// into a 3x3 matrix , one for  u(t) = TU dot (x(t)-x(o) + u(o) and one for v(t)=  TV dot (.... , 
	// then the third assumes we're perpendicular to the normal. 
	//
	FMatrix TexEqu = FMatrix::Identity;
	TexEqu.SetAxis( 0, FVector(	V1.X - V0.X, V1.Y - V0.Y, V1.Z - V0.Z ) );
	TexEqu.SetAxis( 1, FVector( V2.X - V0.X, V2.Y - V0.Y, V2.Z - V0.Z ) );
	TexEqu.SetAxis( 2, FVector( PN.X,        PN.Y,        PN.Z        ) );
	TexEqu = TexEqu.Inverse();

	const FVector UResult( UV1.X-UV0.X, UV2.X-UV0.X, 0.0f );
	const FVector TUResult = TexEqu.TransformNormal( UResult );

	const FVector VResult( UV1.Y-UV0.Y, UV2.Y-UV0.Y, 0.0f );
	const FVector TVResult = TexEqu.TransformNormal( VResult );

	//
	// Adjust the BASE to account for U0 and V0 automatically, and force it into the same plane.
	//				
	FMatrix BaseEqu = FMatrix::Identity;
	BaseEqu.SetAxis( 0, TUResult );
	BaseEqu.SetAxis( 1, TVResult ); 
	BaseEqu.SetAxis( 2, FVector( PN.X, PN.Y, PN.Z ) );
	BaseEqu = BaseEqu.Inverse();

	const FVector BResult = FVector( UV0.X - ( TUResult|V0 ), UV0.Y - ( TVResult|V0 ),  0.0f );

	*InBaseResult = - 1.0f *  BaseEqu.TransformNormal( BResult );
	*InUResult = TUResult;
	*InVResult = TVResult;

}

/*-----------------------------------------------------------------------------
	Misc.
-----------------------------------------------------------------------------*/

class UEditorPlayer : public ULocalPlayer
{
	DECLARE_CLASS(UEditorPlayer,ULocalPlayer,CLASS_Intrinsic,Editor);

	// FExec interface.
	virtual UBOOL Exec(const TCHAR* Cmd,FOutputDevice& Ar);
};


#endif // include-once blocker

