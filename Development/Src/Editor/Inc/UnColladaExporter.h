/*=============================================================================
	COLLADA exporter for Unreal Engine 3.
	Based on Feeling Software's FCollada.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNCOLLADAEXPORTER_H__
#define __UNCOLLADAEXPORTER_H__

// Forward declarations
class FCDocument;
class FCDAsset;
class FCDAnimated;
class FCDConversionFunctor;
class FCDEntity;
class FCDEntityInstance;
class FCDGeometry;
class FCDGeometryInstance;
class FCDMaterial;
class FCDParameterAnimatable;
class FCDSceneNode;

class UInterpData;
class UInterpTrackMove;
class UInterpTrackFloatProp;
class UInterpTrackVectorProp;

namespace UnCollada
{

struct CExporterActor;
typedef TArray<CExporterActor*> CExporterActorMap;

/**
 * Main COLLADA Exporter class.
 * Except for CImporter, consider the other classes as private.
 */
class CExporter
{
public:
	/**
	 * Returns the exporter singleton. It will be created on the first request.
	 */
	static CExporter* GetInstance();

	/**
	 * Creates and readies an empty document for export.
	 */
	void CreateDocument();

	/**
	 * Retrieve the currently open COLLADA document.
	 */
	inline FCDocument* GetColladaDocument()
	{
		return ColladaDocument;
	}

	/**
	 * Exports the basic scene information to the COLLADA document.
	 */
	void ExportLevelMesh( ULevel* Level, USeqAct_Interp* MatineeSequence );

	/**
	 * Exports the light-specific information for a UE3 light actor.
	 */
	void ExportLight( ALight* Actor, USeqAct_Interp* MatineeSequence );

	/**
	 * Exports the camera-specific information for a UE3 camera actor.
	 */
	void ExportCamera( ACameraActor* Actor, USeqAct_Interp* MatineeSequence );

	/**
	 * Exports the mesh and the actor information for a UE3 brush actor.
	 */
	void ExportBrush(ABrush* Actor);
	void ExportModel(UModel* Model, FCDGeometry* ColladaGeometry, FCDGeometryInstance* ColladaInstance);

	/**
	 * Exports the mesh and the actor information for a UE3 static mesh actor.
	 */
	void ExportStaticMesh( AActor* Actor, UStaticMeshComponent* StaticMeshComponent, USeqAct_Interp* MatineeSequence );

	/**
	 * Exports the profile_COMMON information for a UE3 material.
	 */
	FCDMaterial* ExportMaterial(UMaterial* Material);

	/**
	 * Exports the given Matinee sequence information into a COLLADA document.
	 */
	void ExportMatinee(class USeqAct_Interp* MatineeSequence);

	/**
	 * Writes the COLLADA document to disk and releases it by calling the CloseDocument() function.
	 */
	void WriteToFile(const TCHAR* Filename);

	/**
	 * Closes the COLLADA document, releasing its memory.
	 */
	void CloseDocument();

private:
	FCDocument* ColladaDocument;
	CExporterActorMap ColladaActors;

	CExporter();
	~CExporter();

	/**
	 * Writes out the asset information about the UE3 level and user in the COLLADA document.
	 */
	void ExportAsset(FCDAsset* ColladaAsset);

	/**
	 * Exports the basic information about a UE3 actor and buffers it.
	 * This function creates one COLLADA scene node for the actor with its placement.
	 * This scene node will always have four transformations: one translation vector and three Euler rotations.
	 * The transformations will appear in the order: {translation, rot Z, rot Y, rot X} as per the COLLADA specifications.
	 */
	CExporterActor* ExportActor( AActor* Actor, USeqAct_Interp* MatineeSequence );

	/**
	 * Exports the Matinee movement track into the COLLADA animation library.
	 */
	void ExportMatineeTrackMove(CExporterActor* ColladaActor, UInterpTrackInstMove* MoveTrackInst, UInterpTrackMove* MoveTrack);

	/**
	 * Exports the Matinee float property track into the COLLADA animation library.
	 */
	void ExportMatineeTrackFloatProp(CExporterActor* ColladaActor, UInterpTrackFloatProp* PropTrack);

	/**
	 * Exports the Matinee vector property track into the COLLADA animation library.
	 */
	void ExportMatineeTrackVectorProp(CExporterActor* ColladaActor, UInterpTrackVectorProp* PropTrack);

	/**
	 * Exports a given interpolation curve into the COLLADA animation curve.
	 */
	void ExportAnimatedVector(const TCHAR* Name, FCDAnimated* ColladaAnimated, INT ColladaAnimatedIndex, UInterpTrackMove* MoveTrack, UInterpTrackInstMove* MoveTrackInst, UBOOL bPosCurve, INT CurveIndex, FCDConversionFunctor* Conversion);
	void ExportAnimatedFloat(const TCHAR* Name, FCDAnimated* ColladaAnimated, FInterpCurveFloat* Curve, FCDConversionFunctor* Conversion);

	/**
	 * Finds the given UE3 actor in the already-exported list of structures
	 */
	CExporterActor* FindActor(AActor* Actor);
};

/**
 * COLLADA exported animatable property.
 * This private structure links the UE3 Matinee animatable float and vector property with the FCollada animatable elements.
 */
struct CExporterAnimatableProperty
{
public:
	FCDAnimated* ColladaAnimated;
	TArray<FLOAT*> ColladaValues;
	FString PropertyName;
	FCDConversionFunctor* Conversion;
};

/**
 * COLLADA exported actor information structure.
 * This private structure is used to hold the information about an actor that is saved inside one document.
 */
struct CExporterActor
{
public:
	AActor* Actor;
	FCDSceneNode* ColladaNode;
	FCDEntity* ColladaEntity;
	FCDEntityInstance* ColladaInstance;
	TArray<CExporterAnimatableProperty*> Properties;

public:
	CExporterActor();
	~CExporterActor();

	// Instantiate the given COLLADA entity: this will also create the proper pivot node.
	void Instantiate(FCDEntity* ColladaEntity);

	// Find the property link structure for the given property name
	CExporterAnimatableProperty* FindProperty(const TCHAR* Name);

	// Create a property link structure with the information given.
	CExporterAnimatableProperty* CreateProperty(const TCHAR* Name, FCDAnimated* ColladaAnimated, FLOAT* ColladaValues, INT ColladaValueCount=1, FCDConversionFunctor* Conversion=NULL);
};

};

#endif // __UNCOLLADAEXPORTER_H__
