/*=============================================================================
	UnObjVer.h: Unreal object version.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Prevents incorrect files from being loaded.

#define PACKAGE_FILE_TAG			0x9E2A83C1
#define PACKAGE_FILE_TAG_SWAPPED	0xC1832A9E

#if GAMENAME == BMGAME
#define BATMAN 1
#endif

//
//	Package file version log:
//
//	491
//	-	Min version for content resave
#define VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD				491
//	492
//	-	Static mesh version bump, package version bumped to ease resaving
#define VER_STATICMESH_VERSION_16							492
//	493
//	-	Used 16 bit float UVs for skeletal meshes
#define VER_USE_FLOAT16_SKELETAL_MESH_UVS					493
//	494
//	-	Store two tangent basis vectors instead of three to save memory (skeletal mesh vertex buffers)
#define VER_SKELETAL_MESH_REMOVE_BINORMAL_TANGENT_VECTOR	494
//	495
//	-	Terrain collision data stored in world space.
#define VER_TERRAIN_COLLISION_WORLD_SPACE					495
//	496
//	-	Removed DecalManager ref from UWorld
#define VER_REMOVED_DECAL_MANAGER_FROM_UWORLD				496
//	497
//	-	Modified SpeedTree vertex factory shader parameters.
#define VER_SPEEDTREE_SHADER_CHANGE							497
//	498
//	-	Fix height-fog pixel shader 4-layer 
#define VER_HEIGHTFOG_PIXELSHADER_START_DIST_FIX			498
//	499
//	-	MotionBlurShader recompile (added clamping to render target extents)
#define VER_MOTIONBLURSHADER_RECOMPILE_VER2					499
//	500
// -	Separate pass for LDR BLEND_Modulate transparency mode
// -	Modulate preserves dest alpha (depth)
#define VER_SM2_BLENDING_SHADER_FIXES						500
//	501
//	-	Terrain material fallback support
#define VER_ADDED_TERRAIN_MATERIAL_FALLBACK					501
//	502
//	-	Modified shaders to support D3D10, and upgraded to August DirectX SDK
#define VER_D3D10_SHADER_PARAMETER_CHANGES					502
//	503
//	-	Added support for multi-column collections to UIDynamicFieldProvider
#define VER_ADDED_MULTICOLUMN_SUPPORT						503
//	504
//	-	Serialize cached displacement values for terrain
#define VER_TERRAIN_SERIALIZE_DISPLACEMENTS					504
//	505
//	-	Fixed bug which allowed multiple instances of a UIState class get added to style data maps
#define	VER_REMOVED_PREFAB_STYLE_DATA						505
//	506
//	-	Exposed separate horizontal and vertical texture scale for material texture lookups
// -  Various font changes that affected serialization
#define VER_FONT_FORMAT_AND_UV_TILING_CHANGES				506
//	507
//	-	Changed UTVehicleFactory to use a string for class reference in its defaults
#define VER_UTVEHICLEFACTORY_USE_STRING_CLASS				507
//	508
//	-	Fixed the special 0.0f value in the velocity buffer that is used to select between background velocity or dynamic velocity
#define VER_BACKGROUNDVELOCITYVALUE							508
//	509
//	-	Reset vehicle usage flags on some NavigationPoints that had been incorrectly set
#define VER_FIXED_NAV_VEHICLE_USAGE_FLAGS					509
//	510
//	-	Changed Texture2DComposite to inherit from Texture instead of Texture2D.
#define VER_TEXTURE2DCOMPOSITE_BASE_CHANGE					510
//	511
//	-	Fixed fonts serializing all members twice.
#define VER_FIXED_FONTS_SERIALIZATION						511
//	512
//	-	Bump to break full version content to work with demo
#define VER_FULL_VERSION_OF_UT3_BUMP						512
//	513
//	-	Throw away Atrac3 data, create MP3 data
#define VER_UPGRADE_TO_MP3									513
//	514
//	-	
#define VER_STATICMESH_FRAGMENTINDEX						514
//	515
//	- Added Draw SkelTree Manager. Added FColor to FMeshBone serialization.	
#define VER_SKELMESH_DRAWSKELTREEMANAGER					515
//	516
//	- Added AdditionalPackagesToCook to FPackageFileSummary	
#define VER_ADDITIONAL_COOK_PACKAGE_SUMMARY					516
//	517
//	- Add neighbour info to FFragmentInfo
#define	VER_FRAGMENT_NEIGHBOUR_INFO							517
//	518
//	- Added interior fragment index
#define	VER_FRAGMENT_INTERIOR_INDEX							518
//	519
//	- Added bCanBeDestroyed and bRootFragment
#define	VER_FRAGMENT_DESTROY_FLAGS							519
//	520
//	- Add exterior surface normal and neighbour area info to FFragmentInfo
#define VER_FRAGMENT_EXT_NORMAL_NEIGH_DIM					520
//	521
//	- Add core mesh 3d offset and scale
#define VER_FRACTURE_CORE_SCALE_OFFSET						521
//	522
//	- Updated mp3 format to account for multichannel sounds
#define VER_MP3_FORMAT_UPDATE								522
//	523
//	- Moved particle SpawnRate and Burst info into their own module.
#define VER_PARTICLE_SPAWN_AND_BURST_MOVE					523
//	524
//	- Share modules across particle LOD levels where possible.
#define VER_PARTICLE_LOD_MODULE_SHARE						524
//	525
//	- Fixing up TypeData modules not getting pushed into lower LODs
#define VER_PARTICLE_LOD_MODULE_TYPEDATA_FIXUP				525
//	526
//	- Save off PlaneBias with FSM
#define VER_FRACTURE_SAVE_PLANEBIAS							526
//	527
//	- Fixing up LOD distributions... (incorrect archetypes caused during Spawn converion)
#define VER_PARTICLE_LOD_DIST_FIXUP							527
//	528
//	- Added DiffusePower to material inputs
#define VER_DIFFUSEPOWER									528
//	529
//	- Changed default DiffusePower value
#define VER_DIFFUSEPOWER_DEFAULT							529
//	530
//	- Allow for '0' in the particle burst list CountLow slot...
#define VER_PARTICLE_BURST_LIST_ZERO						530
//	531
//	- Added AttenAllowedParameter to FModShadowMeshPixelShader
#define VER_MODSHADOWMESHPIXELSHADER_ATTENALLOWED			531
//	532
//	- Support for mesh simplification tool.  Static mesh version bump (added named reference to high res source mesh.)
#define VER_STATICMESH_VERSION_18							532
//	533
//	- Added automatic fog volume components to simplify workflow
#define VER_AUTOMATIC_FOGVOLUME_COMPONENT					533
//	534
//	- Added an optional array of skeletal mesh weights/bones for instancing 
#define VER_ADDED_EXTRA_SKELMESH_VERTEX_INFLUENCES			534
//	535
//	- Added an optional array of skeletal mesh weights/bones for instancing 
#define VER_UNIFORM_DISTRIBUTION_BAKING_UPDATE				535
//	536
//	- Replaced classes for sequences associated with PrefabInstances
#define VER_FIXED_PREFAB_SEQUENCES							536
//	537
//	- Changed FInputKeyAction's list of sequence actions to a list of sequence output links
#define VER_MADE_INPUTKEYACTION_OUTPUT_LINKS				537
//	538
//	- Moved global shaders from UShaderCache to a single global shader cache file.
#define VER_GLOBAL_SHADER_FILE								538
//	539
//	- Using MSEnc to encode mp3s rather than MP3Enc
#define VER_MP3ENC_TO_MSENC									539
//	540
//	- Fixing up LODValidity...
#define VER_EMITTER_LODVALIDITY_FIX							540
//	541
//	- Added optional external specification of static vertex normals.
#define VER_STATICMESH_EXTERNAL_VERTEX_NORMALS				541
//	542
//	- Removed 2x2 normal transform for decal materials
#define VER_DECAL_MATERIAL_IDENDITY_NORMAL_XFORM			542
//	543
//	- Removed FObjectExport::ComponentMap
#define VER_REMOVED_COMPONENT_MAP							543
//	544
//	- Fixed back uniform distributions with lock flags set to something other than NONE
#define VER_LOCKED_UNIFORM_DISTRIBUTION_BAKING				544
//	545
//	- Fixed Kismet sequences with illegal names
#define VER_FIXED_KISMET_SEQUENCE_NAMES						545
//	546
//	- Added fluid lightmap support
#define VER_ADDED_FLUID_LIGHTMAPS							546
//	547
//	- Fixing up LODValidity and spawn module outers...
#define VER_EMITTER_LODVALIDITY_FIX2						547
//	548
//	- Fixing incorrect default properties for new foliage parameters
#define VER_FIX_DEFAULT_FOLIAGE_PARAMETERS					548
//	549
//	- Add FSM core rotation and 'no physics' flag on chunks
#define VER_FRACTURE_CORE_ROTATION_PERCHUNKPHYS				549
//	550
//	- New curve auto-tangent calculations; Clamped auto tangent support
#define VER_NEW_CURVE_AUTO_TANGENTS							550
//	551
//	- Removed 2x2 normal transform from decal vertices 
#define VER_DECAL_REMOVED_2X2_NORMAL_TRANSFORM				551
//	552
//	- Updated decal vertex factories
#define VER_DECAL_VERTEX_FACTORY_VER1						552
//	553
//	- Updated fluid vertex factories
#define VER_FLUID_VERTEX_FACTORY							553
//	554
//	- Updated decal vertex factories
#define VER_DECAL_VERTEX_FACTORY_VER2						554
//	555
//	- Updated the fluid detail normalmap
#define VER_FLUID_DETAIL_UPDATE								555
//	556
//	- Fixup particle systems with incorrect distance arrays...
#define VER_PARTICLE_LOD_DISTANCE_FIXUP						556
//	557
//	- Added FSM build version
#define VER_FRACTURE_NONCRITICAL_BUILD_VERSION				557
//	558
//	- Added DynamicParameter support for particles
#define VER_DYNAMICPARAMETERS_ADDED							558
//	559
//	- Added travelspeed parameter to the fluid detail normalmap
#define VER_FLUID_DETAIL_UPDATE2							559
//	560
//	- /** replaced bAcceptsDecals,bAcceptsDecalsDuringGameplay with bAcceptsStaticDecals,bAcceptsDynamicDecals */
#define VER_UPDATED_DECAL_USAGE_FLAGS						560
//	561
//	- incremented DOFBloomGather pixel version; added SceneMultipler
#define VER_DOFBLOOMGATHER_SCENEMULTIPLIER					561
//	562
//	- Added bounced lighting settings to LightComponent
#define VER_LIGHTCOMPONENT_BOUNCEDLIGHTING					562
//	563
//	- Made bOverrideNormal override the full tangent basis.
#define VER_OVERRIDETANGENTBASIS							563
//	564
//	- Made LightComponent bounced lighting settings multiplicative with direct lighting.
#define VER_BOUNCEDLIGHTING_DIRECTMODULATION				564
//	565
//	- Added a shader parameter to the FDistortionApplyScreenPixelShader
#define VER_DISTORTIONAPPLYPIXELSHADER_UPDATE				565
//	566
//	- Reduced FStateFrame::LatentAction to WORD
#define VER_REDUCED_STATEFRAME_LATENTACTION_SIZE			566
//	567
//	- Added GUIDs for updating texture file cache
#define VER_ADDED_TEXTURE_FILECACHE_GUIDS					567
//	568
//	- Fixed scene color and scene depth usage
#define VER_FIXED_SCENECOLOR_USAGE							568
//	569
//	- Renamed UPrimitiveComponent::CullDistance to MaxDrawDistance
#define VER_RENAMED_CULLDISTANCE							569
//	570
//	- Fixing up InterpolationMethod mismatches in emitter LOD levels...
#define VER_EMITTER_INTERPOLATIONMETHOD_FIXUP				570
//	571
//	- Fixing up LensFlare ScreenPercentageMaps
#define VER_LENSFLARE_SCREENPERCENTAGEMAP_FIXUP				571
//	572
//	- Updated decal vertex factories
#define VER_DECAL_VERTEX_FACTORY_VER3						572
//	573
//	- Reimplemented particle LOD check distance time
#define VER_PARTICLE_LOD_CHECK_DISTANCE_TIME_FIX			573
//	574
//	- Decal physical material entry fixups
#define VER_DECAL_PHYS_MATERIAL_ENTRY_FIXUP					574
//	575
//	- Added persisitent FaceFXAnimSet to the world...
#define VER_WORLD_PERSISTENT_FACEFXANIMSET					575
//	576
// - depcreated redundant editor window position
// - Delete var - SkelControlBase: ControlPosX, ControlPosY, MaterialExpression: EditorX, EditorY
#define VER_DEPRECATED_EDITOR_POSITION					576

//  21
// - BM1 release
#define VER_BATMAN1												21

// !!
// !! NOTE: when adding a new version, change the VER_LATEST_ENGINE macro below. This will avoid needing to change UnObjVer.cpp. 
// !!
// !! WARNING: Only modify this in //depot/UnrealEngine3/Development/Src/Core/Inc/UnObjVer.h on the Epic Perforce server. All 
// !! WARNING: other places should modify VER_LATEST_ENGINE_LICENSEE instead.
// !!
#define VER_LATEST_ENGINE									VER_DEPRECATED_EDITOR_POSITION
#define VER_LATEST_ENGINE_LICENSEE							VER_BATMAN1

// Cooked packages loaded with an older package version are recooked
#define VER_LATEST_COOKED_PACKAGE							87
#define VER_LATEST_COOKED_PACKAGE_LICENSEE					0


// Version access.

extern INT			GEngineVersion;					// Engine build number, for displaying to end users.
extern INT			GBuiltFromChangeList;			// Built from changelist, for displaying to end users.


extern INT			GEngineMinNetVersion;			// Earliest engine build that is network compatible with this one.
extern INT			GEngineNegotiationVersion;		// Base protocol version to negotiate in network play.

extern INT			GPackageFileVersion;			// The current Unrealfile version.
extern INT			GPackageFileMinVersion;			// The earliest file version that can be loaded with complete backward compatibility.
extern INT			GPackageFileLicenseeVersion;	// Licensee Version Number.

extern INT          GPackageFileCookedContentVersion;  // version of the cooked content
