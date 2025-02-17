/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	Based on the FS Import classes:
	Copyright (C) 2005-2006 Feeling Software Inc
	Copyright (C) 2005-2006 Autodesk Media Entertainment
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _DAE_SYNTAX_H_
#define _DAE_SYNTAX_H_

// COLLADA Versioning information
#define DAE_NAMESPACE_ATTRIBUTE						"xmlns"
#define DAE_SCHEMA_LOCATION							"http://www.collada.org/2005/11/COLLADASchema"
#define DAE_VERSION_ATTRIBUTE						"version"
#define DAE_SCHEMA_VERSION							"1.4.1"

// COLLADA 1.4 elements
#define DAE_LIBRARY_ANIMATION_ELEMENT				"library_animations"
#define DAE_LIBRARY_ANIMATION_CLIP_ELEMENT			"library_animation_clips"
#define DAE_LIBRARY_CAMERA_ELEMENT					"library_cameras"
#define DAE_LIBRARY_CONTROLLER_ELEMENT				"library_controllers"
#define DAE_LIBRARY_EFFECT_ELEMENT					"library_effects"
#define DAE_LIBRARY_FFIELDS_ELEMENT					"library_force_fields"
#define DAE_LIBRARY_GEOMETRY_ELEMENT				"library_geometries"
#define DAE_LIBRARY_IMAGE_ELEMENT					"library_images"
#define DAE_LIBRARY_LIGHT_ELEMENT					"library_lights"
#define DAE_LIBRARY_MATERIAL_ELEMENT				"library_materials"
#define DAE_LIBRARY_NODE_ELEMENT					"library_nodes"
#define DAE_LIBRARY_PMATERIAL_ELEMENT				"library_physics_materials"
#define DAE_LIBRARY_PMODEL_ELEMENT					"library_physics_models"
#define DAE_LIBRARY_PSCENE_ELEMENT					"library_physics_scenes"
#define DAE_LIBRARY_VSCENE_ELEMENT					"library_visual_scenes"

#define DAE_INSTANCE_ANIMATION_ELEMENT				"instance_animation"
#define DAE_INSTANCE_CAMERA_ELEMENT					"instance_camera"
#define DAE_INSTANCE_CONTROLLER_ELEMENT				"instance_controller"
#define DAE_INSTANCE_EFFECT_ELEMENT					"instance_effect"
#define DAE_INSTANCE_GEOMETRY_ELEMENT				"instance_geometry"
#define DAE_INSTANCE_LIGHT_ELEMENT					"instance_light"
#define DAE_INSTANCE_MATERIAL_ELEMENT				"instance_material"
#define DAE_INSTANCE_NODE_ELEMENT					"instance_node"
#define DAE_INSTANCE_VSCENE_ELEMENT					"instance_visual_scene"

#define DAE_ANIMCLIP_ELEMENT						"animation_clip"
#define DAE_BIND_ELEMENT							"bind"
#define DAE_BIND_VERTEX_INPUT_ELEMENT				"bind_vertex_input"
#define DAE_BIND_TEXTURE_SURFACE_ELEMENT			"bind_texture_surface"
#define DAE_BINDMATERIAL_ELEMENT					"bind_material"
#define DAE_COLOR_ELEMENT							"color"
#define DAE_CONTROL_VERTICES_ELEMENT				"control_vertices"
#define DAE_EFFECT_ELEMENT							"effect"
#define DAE_MIP_LEVELS								"mip_levels"
#define DAE_MIPMAP_GENERATE							"mipmap_generate"
#define DAE_SAMPLER_ELEMENT							"sampler"
#define DAE_SKELETON_ELEMENT						"skeleton"
#define DAE_TARGETS_ELEMENT							"targets"
#define DAE_TECHNIQUE_COMMON_ELEMENT				"technique_common"
#define DAE_VIEWPORT_RATIO							"viewport_ratio"
#define DAE_VSCENE_ELEMENT							"visual_scene"
#define DAE_WEIGHTS_ELEMENT							"vertex_weights"
#define DAE_VERTEXCOUNT_ELEMENT						"vcount"

#define DAE_INITASNULL_ELEMENT						"init_as_null"
#define DAE_INITASTARGET_ELEMENT					"init_as_target"
#define DAE_INITCUBE_ELEMENT						"init_cube"
#define DAE_INITVOLUME_ELEMENT						"init_volume"
#define DAE_INITPLANAR_ELEMENT						"init_planar"
#define DAE_INITFROM_ELEMENT						"init_from"
#define DAE_ALL_ELEMENT								"all"
#define DAE_PRIMARY_ELEMENT							"primary"
#define DAE_FACE_ELEMENT							"face"
#define DAE_ORDER_ELEMENT							"order"

#define DAE_FX_PROFILE_COMMON_ELEMENT				"profile_COMMON"
#define DAE_FX_PROFILE_CG_ELEMENT					"profile_CG"
#define DAE_FX_PROFILE_HLSL_ELEMENT					"profile_HLSL"
#define DAE_FX_PROFILE_GLSL_ELEMENT					"profile_GLSL"
#define DAE_FX_PROFILE_GLES_ELEMENT					"profile_GLES"

#define DAE_FXCMN_ANNOTATE_ELEMENT					"annotate"
#define DAE_FXCMN_BIND_ELEMENT						"bind"
#define DAE_FXCMN_BOOL_ELEMENT						"bool"
#define DAE_FXCMN_CODE_ELEMENT						"code"
#define DAE_FXCMN_COMPILERTARGET_ELEMENT			"compiler_target"
#define DAE_FXCMN_COMPILEROPTIONS_ELEMENT			"compiler_options"
#define DAE_FXCMN_INT_ELEMENT						"int"
#define DAE_FXCMN_HALF_ELEMENT						"half"
#define DAE_FXCMN_HALF2_ELEMENT						"half2"
#define DAE_FXCMN_HALF3_ELEMENT						"half3"
#define DAE_FXCMN_HALF4_ELEMENT						"half4"
#define DAE_FXCMN_HALF4X4_ELEMENT					"half4x4"
#define DAE_FXCMN_FLOAT_ELEMENT						"float"
#define DAE_FXCMN_FLOAT2_ELEMENT					"float2"
#define DAE_FXCMN_FLOAT3_ELEMENT					"float3"
#define DAE_FXCMN_FLOAT4_ELEMENT					"float4"
#define DAE_FXCMN_FLOAT4X4_ELEMENT					"float4x4"
#define DAE_FXCMN_HINT_ELEMENT						"technique_hint"
#define DAE_FXCMN_INCLUDE_ELEMENT					"include"
#define DAE_FXCMN_SURFACE_ELEMENT					"surface"
#define DAE_FXCMN_SAMPLER1D_ELEMENT					"sampler1D"
#define DAE_FXCMN_SAMPLER2D_ELEMENT					"sampler2D"
#define DAE_FXCMN_SAMPLER3D_ELEMENT					"sampler3D"
#define DAE_FXCMN_SAMPLERCUBE_ELEMENT				"samplerCUBE"
#define DAE_FXCMN_SEMANTIC_ELEMENT					"semantic"
#define DAE_FXCMN_SETPARAM_ELEMENT					"setparam"
#define DAE_FXCMN_NEWPARAM_ELEMENT					"newparam"
#define DAE_FXCMN_STRING_ELEMENT					"string"
#define DAE_FXCMN_NAME_ELEMENT						"name"

#define DAE_FXCMN_VERTEX_SHADER						"VERTEX"
#define DAE_FXCMN_FRAGMENT_SHADER					"FRAGMENT"

#define DAE_FXSTD_CONSTANT_ELEMENT					"constant"
#define DAE_FXSTD_LAMBERT_ELEMENT					"lambert"
#define DAE_FXSTD_PHONG_ELEMENT						"phong"
#define DAE_FXSTD_BLINN_ELEMENT						"blinn"
#define DAE_FXSTD_COLOR_ELEMENT						"color"
#define DAE_FXSTD_FLOAT_ELEMENT						"float"
#define DAE_FXSTD_TEXTURE_ELEMENT					"texture"
#define DAE_FXSTD_TEXTURE_ATTRIBUTE					"texture"
#define DAE_FXSTD_TEXTURESET_ATTRIBUTE				"texcoord"

#define DAE_CONTROLLER_SKIN_ELEMENT					"skin"
#define DAE_CONTROLLER_MORPH_ELEMENT				"morph"

#define DAE_CAMERA_PERSP_ELEMENT					"perspective"
#define DAE_CAMERA_ORTHO_ELEMENT					"orthographic"

#define DAE_RGB_ZERO_ELEMENT						"RGB_ZERO"
#define DAE_A_ONE_ELEMENT							"A_ONE"

#define DAE_ASPECT_CAMERA_PARAMETER					"aspect_ratio"
#define DAE_XFOV_CAMERA_PARAMETER					"xfov"	
#define DAE_YFOV_CAMERA_PARAMETER					"yfov"	
#define DAE_ZNEAR_CAMERA_PARAMETER					"znear"	
#define DAE_ZFAR_CAMERA_PARAMETER					"zfar"	
#define DAE_XMAG_CAMERA_PARAMETER					"xmag"	
#define DAE_YMAG_CAMERA_PARAMETER					"ymag"

#define DAE_AMBIENT_MATERIAL_PARAMETER				"ambient"
#define DAE_BUMP_MATERIAL_PARAMETER					"bump"
#define DAE_DIFFUSE_MATERIAL_PARAMETER				"diffuse"
#define DAE_EMISSION_MATERIAL_PARAMETER				"emission"
#define DAE_TRANSPARENCY_MATERIAL_PARAMETER			"transparency"
#define DAE_TRANSPARENT_MATERIAL_PARAMETER			"transparent"
#define DAE_OPAQUE_MATERIAL_ATTRIBUTE				"opaque"
#define DAE_REFLECTIVE_MATERIAL_PARAMETER			"reflective"
#define DAE_REFLECTIVITY_MATERIAL_PARAMETER			"reflectivity"
#define DAE_SHININESS_MATERIAL_PARAMETER			"shininess"
#define DAE_SPECULAR_MATERIAL_PARAMETER				"specular"

#define DAE_LIGHT_AMBIENT_ELEMENT					"ambient"		
#define DAE_LIGHT_POINT_ELEMENT						"point"
#define DAE_LIGHT_DIRECTIONAL_ELEMENT				"directional"
#define DAE_LIGHT_SPOT_ELEMENT						"spot"

#define DAE_COLOR_LIGHT_PARAMETER					"color"
#define DAE_CONST_ATTENUATION_LIGHT_PARAMETER		"constant_attenuation"
#define DAE_LIN_ATTENUATION_LIGHT_PARAMETER			"linear_attenuation"
#define DAE_QUAD_ATTENUATION_LIGHT_PARAMETER		"quadratic_attenuation"
#define DAE_FALLOFFEXPONENT_LIGHT_PARAMETER			"falloff_exponent"
#define DAE_FALLOFFANGLE_LIGHT_PARAMETER			"falloff_angle"

#define DAE_BINDSHAPEMX_SKIN_PARAMETER				"bind_shape_matrix"

#define DAE_CONTRIBUTOR_ASSET_ELEMENT				"contributor"
#define DAE_AUTHOR_ASSET_PARAMETER					"author"
#define DAE_AUTHORINGTOOL_ASSET_PARAMETER			"authoring_tool"
#define DAE_CREATED_ASSET_PARAMETER					"created"
#define DAE_COMMENTS_ASSET_PARAMETER				"comments"
#define DAE_COPYRIGHT_ASSET_PARAMETER				"copyright"
#define DAE_KEYWORDS_ASSET_PARAMETER				"keywords"
#define DAE_MODIFIED_ASSET_PARAMETER				"modified"
#define DAE_REVISION_ASSET_PARAMETER				"revision"
#define DAE_SOURCEDATA_ASSET_PARAMETER				"source_data"
#define DAE_SUBJECT_ASSET_PARAMETER					"subject"
#define DAE_TITLE_ASSET_PARAMETER					"title"
#define DAE_UNITS_ASSET_PARAMETER					"unit"
#define DAE_UPAXIS_ASSET_PARAMETER					"up_axis"

#define DAE_PHYSICS_STATIC_FRICTION					"static_friction"
#define DAE_PHYSICS_DYNAMIC_FRICTION				"dynamic_friction"
#define DAE_PHYSICS_RESTITUTION						"restitution"

// COLLADA 1.4 attributes
#define DAE_CLOSED_ATTRIBUTE						"closed"
#define DAE_CLOSEDU_ATTRIBUTE						"closed_u"
#define DAE_CLOSEDV_ATTRIBUTE						"closed_v"
#define DAE_COUNT_ATTRIBUTE							"count"
#define DAE_DEGREE_ATTRIBUTE						"degree"
#define DAE_UDEGREE_ATTRIBUTE						"udegree"
#define DAE_VDEGREE_ATTRIBUTE						"vdegree"
#define DAE_END_ATTRIBUTE							"end"
#define	DAE_ID_ATTRIBUTE							"id"
#define DAE_MATERIAL_ATTRIBUTE						"material"
#define DAE_METERS_ATTRIBUTE						"meter"
#define DAE_METHOD_ATTRIBUTE						"method"
#define DAE_NAME_ATTRIBUTE							"name"
#define DAE_OFFSET_ATTRIBUTE						"offset"
#define DAE_PLATFORM_ATTRIBUTE						"platform"
#define DAE_PROFILE_ATTRIBUTE						"profile"
#define DAE_REF_ATTRIBUTE							"ref"
#define DAE_SEMANTIC_ATTRIBUTE						"semantic"
#define DAE_INPUT_SEMANTIC_ATTRIBUTE				"input_semantic"
#define DAE_INPUT_SET_ATTRIBUTE						"input_set"
#define DAE_SET_ATTRIBUTE							"set"
#define DAE_SID_ATTRIBUTE							"sid"
#define DAE_START_ATTRIBUTE							"start"
#define DAE_STRIDE_ATTRIBUTE						"stride"
#define DAE_SOURCE_ATTRIBUTE						"source"
#define DAE_SURFACE_ATTRIBUTE						"surface"
#define DAE_SYMBOL_ATTRIBUTE						"symbol"
#define	DAE_TARGET_ATTRIBUTE						"target"
#define DAE_TEXTURE_ATTRIBUTE						"texture"
#define DAE_URL_ATTRIBUTE							"url"
#define DAE_STAGE_ATTRIBUTE							"stage"

#define	DAE_MIP_ATTRIBUTE							"mip"
#define DAE_SLICE_ATTRIBUTE							"slice"
#define	DAE_FACE_ATTRIBUTE							"face"

// COLLADA 1.4 types
#define DAE_FLOAT_TYPE								"float"
#define DAE_IDREF_TYPE								"IDREF"
#define DAE_MATRIX_TYPE								"float4x4"
#define DAE_NAME_TYPE								"Name"

// COLLADA 1.4 enumerations
#define DAE_AMBIENT_TEXTURE_CHANNEL					"AMBIENT"
#define DAE_BUMP_TEXTURE_CHANNEL					"BUMP"
#define DAE_DIFFUSE_TEXTURE_CHANNEL					"DIFFUSE"
#define DAE_DISPLACEMENT_TEXTURE_CHANNEL			"DISPLACEMENT"
#define DAE_EMISSION_TEXTURE_CHANNEL				"GLOW"
#define DAE_FILTER_TEXTURE_CHANNEL					"FILTER"
#define	DAE_OPACITY_TEXTURE_CHANNEL					"OPACITY"
#define DAE_REFLECTION_TEXTURE_CHANNEL				"REFLECTION"
#define DAE_REFRACTION_TEXTURE_CHANNEL				"REFRACTION"
#define DAE_SHININESS_TEXTURE_CHANNEL				"SHININESS"
#define DAE_SPECULAR_TEXTURE_CHANNEL				"SPECULAR"
#define DAE_SPECULARLEVEL_TEXTURE_CHANNEL			"SPECULAR-LEVEL"
#define DAE_TRANSPARENT_TEXTURE_CHANNEL				"TRANSPARENT"

#define DAE_NORMALIZED_MORPH_METHOD					"NORMALIZED"
#define DAE_RELATIVE_MORPH_METHOD					"RELATIVE"

#define DAE_WEIGHT_MORPH_INPUT						"MORPH_WEIGHT"
#define DAE_WEIGHT_MORPH_INPUT_DEPRECATED			"WEIGHT"
#define DAE_TARGET_MORPH_INPUT						"MORPH_TARGET"
#define DAE_TARGET_MORPH_INPUT_DEPRECATED			"TARGET"

#define DAE_TIME_TARGET								"TIME"

#define DAE_JOINT_NODE_TYPE							"JOINT"
#define DAE_NODE_NODE_TYPE							"NODE"

// spline inputs
#define DAE_CVS_SPLINE_INPUT						"POSITION"
//#define DAE_INTERPOLATION_SPLINE_INPUT			"INTERPOLATION"
//#define DAE_IN_TANGENT_SPLINE_INPUT				"IN_TANGENT"
//#define DAE_OUT_TANGENT_SPLINE_INPUT				"OUT_TANGENT"
//#define DAE_CONTINUITY_SPLINE_INPUT				"CONTINUITY"
//#define DAE_LINEAR_STEPS_SPLINE_INPUT				"LINEAR_STEPS"
#define DAE_KNOT_SPLINE_INPUT						"KNOTS"
#define DAE_WEIGHT_SPLINE_INPUT						"WEIGHTS"

// spline types
#define DAE_BEZIER_SPLINE_TYPE						"BEZIER"
#define DAE_NURBS_SPLINE_TYPE						"NURBS"
#define DAE_UNKNOWN_SPLINE_TYPE						"UNKNOWN"

// spline forms
#define DAE_OPEN_SPLINE_FORM						"OPEN"
#define DAE_CLOSED_SPLINE_FORM						"CLOSED"
#define DAE_UNKNOWN_SPLINE_FORM						"UNKNOWN"

// NURBS surface types
#define DAE_VERTEX_NURBS_SURF_TYPE					"VERTEX"
#define DAE_TEXTURE_NURBS_SURF_TYPE					"TEXTURE"
#define DAE_NORMAL_NURBS_SURF_TYPE					"NORMAL"
#define DAE_UNKNOWN_NURBS_SURF_TYPE					"UNKNOWN"

// additional inputs for NURBS surfaces
#define DAE_UKNOT_NURBS_INPUT						"UKNOTS"
#define DAE_VKNOT_NURBS_INPUT						"VKNOTS"

// Collada 1.4 physics
#define DAE_PHYSICS_MATERIAL_ELEMENT				"physics_material"
#define DAE_PHYSICS_MODEL_ELEMENT					"physics_model"
#define DAE_PHYSICS_SCENE_ELEMENT					"physics_scene"
#define DAE_INSTANCE_PHYSICS_MATERIAL_ELEMENT		"instance_physics_material"
#define DAE_INSTANCE_PHYSICS_MODEL_ELEMENT			"instance_physics_model"
#define DAE_INSTANCE_PHYSICS_SCENE_ELEMENT			"instance_physics_scene"
#define DAE_INSTANCE_RIGID_BODY_ELEMENT				"instance_rigid_body"
#define DAE_INSTANCE_RIGID_CONSTRAINT_ELEMENT		"instance_rigid_constraint"
#define DAE_INSTANCE_FORCE_FIELD_ELEMENT			"instance_force_field"
#define DAE_TIME_STEP_ATTRIBUTE						"time_step"
#define DAE_GRAVITY_ATTRIBUTE						"gravity"
#define DAE_RESTITUTION_ATTRIBUTE					"restitution"
#define DAE_STATIC_FRICTION_ATTRIBUTE				"static_friction"
#define DAE_DYNAMIC_FRICTION_ATTRIBUTE				"dynamic_friction"
#define DAE_VELOCITY_ELEMENT						"velocity"
#define DAE_ANGULAR_VELOCITY_ELEMENT				"angular_velocity"
#define DAE_BODY_ATTRIBUTE							"body"
#define DAE_CONSTRAINT_ATTRIBUTE					"constraint"
#define DAE_MASS_FRAME_ELEMENT						"mass_frame"
#define DAE_LIMITS_ELEMENT							"limits"
#define DAE_LINEAR_ELEMENT							"linear"
#define DAE_ANGULAR_ELEMENT							"angular"
#define DAE_SWING_CONE_AND_TWIST_ELEMENT			"swing_cone_and_twist"
#define DAE_CONVEX_HULL_OF_ATTRIBUTE				"convex_hull_of"
#define DAE_HEIGHT_ELEMENT							"height"

// Physics extension. Currently in prototype phase.
#define DAE_MIN_ELEMENT								"min"
#define DAE_MAX_ELEMENT								"max"
#define DAE_SHAPE_ELEMENT							"shape"
#define	DAE_RIGID_BODY_ELEMENT						"rigid_body"
#define DAE_DYNAMIC_ELEMENT							"dynamic"
#define DAE_HOLLOW_ELEMENT							"hollow"
#define DAE_MASS_ELEMENT							"mass"
#define DAE_BOX_ELEMENT								"box"
#define DAE_SPHERE_ELEMENT							"sphere"
#define DAE_CAPSULE_ELEMENT							"capsule"
#define DAE_CYLINDER_ELEMENT						"cylinder"
#define DAE_ELLIPSOID_ELEMENT						"ellipsoid"
#define DAE_TAPERED_CAPSULE_ELEMENT					"tapered_capsule"
#define DAE_TAPERED_CYLINDER_ELEMENT				"tapered_cylinder"
#define DAE_PLANE_ELEMENT							"plane"
#define DAE_FORMAT_ELEMENT							"format"
#define DAE_FORMAT_HINT_ELEMENT						"format_hint"
#define DAE_PRECISION_ELEMENT						"precision"
#define DAE_OPTION_ELEMENT							"option"
#define DAE_HALF_EXTENTS_ELEMENT					"half_extents"
#define DAE_EQUATION_ELEMENT						"equation"
#define DAE_SIZE_ELEMENT							"size"
#define DAE_RADIUS_ELEMENT							"radius"
#define DAE_RADIUS1_ELEMENT							"radius1"
#define DAE_RADIUS2_ELEMENT							"radius2"
#define DAE_CONVEX_MESH_ELEMENT						"convex_mesh"
#define DAE_INERTIA_ELEMENT							"inertia"
#define DAE_DENSITY_ELEMENT							"density"
#define DAE_CENTER_OF_MASS_ELEMENT					"center_of_mass"
#define DAE_DYNAMICS_ELEMENT						"dynamics"
#define DAE_RIGID_CONSTRAINT_ELEMENT				"rigid_constraint"
#define DAE_FORCE_FIELD_ELEMENT						"force_field"
#define DAE_ATTACHMENT_ELEMENT						"attachment"
#define DAE_REF_ATTACHMENT_ELEMENT					"ref_attachment"
#define DAE_ROT_LIMIT_MIN_ELEMENT					"rot_limit_min"
#define DAE_ROT_LIMIT_MAX_ELEMENT					"rot_limit_max"
#define DAE_TRANS_LIMIT_MIN_ELEMENT					"trans_limit_min"
#define DAE_TRANS_LIMIT_MAX_ELEMENT					"trans_limit_max"
#define DAE_ENABLED_ELEMENT							"enabled"
#define DAE_INTERPENETRATE_ELEMENT					"interpenetrate"
#define DAE_SPRING_ELEMENT							"spring"
#define DAE_STIFFNESS_ELEMENT						"stiffness"
#define DAE_DAMPING_ELEMENT							"damping"
#define DAE_TARGET_VALUE_ELEMENT					"target_value"
#define DAE_ANNOTATE_ELEMENT						"annotate"
#define DAE_TRUE_KEYWORD							"true"
#define DAE_FALSE_KEYWORD							"false"

// COLLADA 1.4.1 <format_hint> elements
#define DAE_FORMAT_HINT_RGB_VALUE					"RGB"
#define DAE_FORMAT_HINT_RGBA_VALUE					"RGBA"
#define DAE_FORMAT_HINT_L_VALUE						"L"
#define DAE_FORMAT_HINT_LA_VALUE					"LA"
#define DAE_FORMAT_HINT_D_VALUE						"D"
#define DAE_FORMAT_HINT_XYZ_VALUE					"XYZ"
#define DAE_FORMAT_HINT_XYZW_VALUE					"XYZW"
#define DAE_FORMAT_HINT_SNORM_VALUE					"SNORM"
#define DAE_FORMAT_HINT_UNORM_VALUE					"UNORM"
#define DAE_FORMAT_HINT_SINT_VALUE					"SINT"
#define DAE_FORMAT_HINT_UINT_VALUE					"UINT"
#define DAE_FORMAT_HINT_FLOAT_VALUE					"FLOAT"
#define DAE_FORMAT_HINT_LOW_VALUE					"LOW"
#define DAE_FORMAT_HINT_MID_VALUE					"MID"
#define DAE_FORMAT_HINT_HIGH_VALUE					"HIGH"
#define DAE_FORMAT_HINT_SRGB_GAMMA_VALUE			"SRGB_GAMMA"
#define DAE_FORMAT_HINT_NORMALIZED3_VALUE			"NORMALIZED3"
#define DAE_FORMAT_HINT_NORMALIZED4_VALUE			"NORMALIZED4"
#define DAE_FORMAT_HINT_COMPRESSABLE_VALUE			"COMPRESSABLE"


// COLLADA 1.3 elements
#define	DAE_ACCESSOR_ELEMENT						"accessor"
#define DAE_ANIMATION_ELEMENT						"animation"
#define DAE_ASSET_ELEMENT							"asset"
#define DAE_CAMERA_ELEMENT							"camera"
#define DAE_CHANNEL_ELEMENT							"channel"
#define DAE_CHANNELS_ELEMENT						"channels"
#define DAE_COLLADA_ELEMENT							"COLLADA"
#define DAE_COMBINER_ELEMENT						"combiner"				// [Deprecated 1.4]
#define DAE_CONTROLLER_ELEMENT						"controller"
#define DAE_DEPTH_ELEMENT							"depth"
#define DAE_EXTRA_ELEMENT							"extra"
#define DAE_RANGE_ELEMENT							"range"
#define DAE_FLOAT_ARRAY_ELEMENT						"float_array"
#define DAE_GEOMETRY_ELEMENT						"geometry"
#define DAE_HOLE_ELEMENT							"h"
#define DAE_IDREF_ARRAY_ELEMENT						"IDREF_array"
#define	DAE_IMAGE_ELEMENT							"image"
#define DAE_INPUT_ELEMENT							"input"
#define DAE_INT_ARRAY_ELEMENT						"int_array"
#define DAE_INSTANCE_ELEMENT						"instance"				// [Deprecated 1.4]
#define DAE_JOINTS_ELEMENT							"joints"
#define DAE_LIBRARY_ELEMENT							"library"				// [Deprecated 1.4]
#define DAE_LIGHT_ELEMENT							"light"
#define DAE_LINES_ELEMENT							"lines"
#define DAE_LINESTRIPS_ELEMENT						"linestrips"
#define DAE_LOOKAT_ELEMENT							"lookat"
#define DAE_MATERIAL_ELEMENT						"material"
#define DAE_MATRIX_ELEMENT							"matrix"
#define DAE_MESH_ELEMENT							"mesh"
#define DAE_NAME_ARRAY_ELEMENT						"Name_array"
#define DAE_NODE_ELEMENT							"node"
#define DAE_OPTICS_ELEMENT							"optics"
#define DAE_PARAMETER_ELEMENT						"param"
#define DAE_PASS_ELEMENT							"pass"
#define DAE_POLYGON_ELEMENT							"p"
#define DAE_POLYGONHOLED_ELEMENT					"ph"
#define DAE_POLYGONS_ELEMENT						"polygons"
#define DAE_POLYLIST_ELEMENT						"polylist"
#define DAE_PROGRAM_ELEMENT							"program"				// [Deprecated 1.4]
#define DAE_PARAMETERIZED_SURFACE_ELEMENT			"psurface"
#define DAE_REST_LENGTH_ELEMENT1_3					"rest_length"			// [Deprecated 1.4] - Replaced by "target_value"
#define DAE_ROTATE_ELEMENT							"rotate"
#define	DAE_SCALE_ELEMENT							"scale"
#define DAE_SCENE_ELEMENT							"scene"
#define	DAE_SHADER_ELEMENT							"shader"
#define	DAE_SOURCE_ELEMENT							"source"
#define DAE_SPLINE_ELEMENT							"spline"
#define DAE_SKEW_ELEMENT							"skew"
#define DAE_TECHNIQUE_ELEMENT						"technique"
#define DAE_TEXTURE_ELEMENT							"texture"
#define DAE_TEXTURE_SURFACE_ELEMENT					"texture_surface"
#define	DAE_TRANSLATE_ELEMENT						"translate"
#define DAE_TRIANGLES_ELEMENT						"triangles"
#define DAE_TRIFANS_ELEMENT							"trifans"
#define DAE_TRIM_GROUP_ELEMENT						"trim_group"
#define DAE_TRISTRIPS_ELEMENT						"tristrips"
#define DAE_VERTEX_ELEMENT							"v"
#define DAE_VERTICES_ELEMENT						"vertices"
#define DAE_WIDTH_ELEMENT							"width"

// COLLADA 1.3 attributes
#define DAE_IDX_ATTRIBUTE							"idx"					// [Deprecated 1.4] - Replaced by 'offset'
#define DAE_TYPE_ATTRIBUTE							"type"					// [Deprecated 1.4]

// COLLADA 1.3 enumerations
#define DAE_COMMON_PROFILE							"COMMON"				// [Deprecated 1.4] - Replaced by the <technique_common> element

#define DAE_COLOR_INPUT								"COLOR"
#define DAE_GEOBINORMAL_INPUT						"BINORMAL"
#define DAE_GEOTANGENT_INPUT						"TANGENT"
#define DAE_IMAGE_INPUT								"IMAGE"
#define DAE_MAPPING_INPUT							"UV"
#define DAE_NORMAL_INPUT							"NORMAL"
#define DAE_POSITION_INPUT							"POSITION"
#define DAE_TEXCOORD_INPUT							"TEXCOORD"
#define DAE_TEXBINORMAL_INPUT						"TEXBINORMAL"
#define DAE_TEXTANGENT_INPUT						"TEXTANGENT"
#define DAE_TEXTURE_INPUT							"TEXTURE"
#define DAE_VERTEX_INPUT							"VERTEX"
#define DAE_BINDMATRIX_SKIN_INPUT					"INV_BIND_MATRIX"
#define DAE_BINDNORMAL_SKIN_INPUT					"BIND_SHAPE_NORMAL"		// [Deprecated 1.4]
#define DAE_BINDPOS_SKIN_INPUT						"BIND_SHAPE_POSITION"	// [Deprecated 1.4]
#define DAE_JOINT_SKIN_INPUT						"JOINT"
#define DAE_JOINTWEIGHT_SKIN_INPUT					"JOINTS_AND_WEIGHTS"	// [Deprecated 1.4]
#define DAE_WEIGHT_SKIN_INPUT						"WEIGHT"

#define DAE_INPUT_ANIMATION_INPUT					"INPUT"
#define DAE_OUTPUT_ANIMATION_INPUT					"OUTPUT"
#define DAE_INTANGENT_ANIMATION_INPUT				"IN_TANGENT"
#define DAE_OUTTANGENT_ANIMATION_INPUT				"OUT_TANGENT"
#define DAE_INTERPOLATION_ANIMATION_INPUT			"INTERPOLATION"

#define DAE_ANIMATION_TYPE							"ANIMATION" 		// [Deprecated 1.4]
#define DAE_CAMERA_TYPE								"CAMERA"			// [Deprecated 1.4]
#define DAE_CONTROLLER_TYPE							"CONTROLLER"		// [Deprecated 1.4]
#define DAE_EFFECT_TYPE								"EFFECT"			// [Deprecated 1.4]
#define DAE_IMAGE_TYPE								"IMAGE"				// [Deprecated 1.4]
#define DAE_GEOMETRY_TYPE							"GEOMETRY"			// [Deprecated 1.4]
#define DAE_LIGHT_TYPE								"LIGHT"				// [Deprecated 1.4]
#define DAE_MATERIAL_TYPE							"MATERIAL"			// [Deprecated 1.4]
#define DAE_TEXTURE_TYPE							"TEXTURE"			// [Deprecated 1.4]

#define DAE_PERSPECTIVE_CAMERA_TYPE					"PERSPECTIVE"		// [Deprecated 1.4]
#define DAE_ORTHOGRAPHIC_CAMERA_TYPE				"ORTHOGRAPHIC"		// [Deprecated 1.4]

#define DAE_XFOV_CAMERA_PARAMETER1_3				"XFOV"				// [Deprecated 1.4]
#define DAE_YFOV_CAMERA_PARAMETER1_3				"YFOV"				// [Deprecated 1.4]
#define DAE_ZNEAR_CAMERA_PARAMETER1_3				"ZNEAR"				// [Deprecated 1.4]
#define DAE_ZFAR_CAMERA_PARAMETER1_3				"ZFAR"				// [Deprecated 1.4]
#define DAE_LEFT_CAMERA_PARAMETER1_3				"LEFT"				// [Deprecated 1.4]
#define DAE_RIGHT_CAMERA_PARAMETER1_3				"RIGHT"				// [Deprecated 1.4]
#define DAE_TOP_CAMERA_PARAMETER1_3					"TOP"				// [Deprecated 1.4]
#define DAE_BOTTOM_CAMERA_PARAMETER1_3				"BOTTOM"			// [Deprecated 1.4]

#define DAE_AMBIENT_LIGHT_TYPE						"AMBIENT"			// [Deprecated 1.4]			
#define DAE_POINT_LIGHT_TYPE						"POINT"				// [Deprecated 1.4]
#define DAE_DIRECTIONAL_LIGHT_TYPE					"DIRECTIONAL"		// [Deprecated 1.4]
#define DAE_SPOT_LIGHT_TYPE							"SPOT"				// [Deprecated 1.4]

#define DAE_COLOR_LIGHT_PARAMETER1_3				"COLOR"				// [Deprecated 1.4]
#define DAE_ATTENUATION_LIGHT_PARAMETER1_3			"ATTENUATION"		// [Deprecated 1.4]
#define DAE_ATTENUATIONSCALE_LIGHT_PARAMETER1_3		"ATTENUATION_SCALE"	// [Deprecated 1.4]
#define DAE_FALLOFF_LIGHT_PARAMETER1_3				"FALLOFF"			// [Deprecated 1.4]
#define DAE_FALLOFFSCALE_LIGHT_PARAMETER1_3			"FALLOFF_SCALE"		// [Deprecated 1.4]
#define DAE_ANGLE_LIGHT_PARAMETER1_3				"ANGLE"				// [Deprecated 1.4]

#define DAE_X_UP									"X_UP"
#define DAE_Y_UP									"Y_UP"
#define DAE_Z_UP									"Z_UP"

#define	DAE_NONE_INTERPOLATION						"NONE"				// [Deprecated 1.3] ColladaMaya exported this until September 2005
#define	DAE_STEP_INTERPOLATION						"STEP"
#define DAE_LINEAR_INTERPOLATION					"LINEAR"
#define DAE_BEZIER_INTERPOLATION					"BEZIER"

#define DAE_CONSTANT_FUNCTION						"CONSTANT"
#define DAE_LINEAR_FUNCTION							"LINEAR"
#define DAE_QUADRATIC_FUNCTION						"QUADRATIC"

#define DAE_CONSTANT_MATERIAL_PROGRAM				"CONSTANT"			// [Deprecated 1.4]
#define DAE_LAMBERT_MATERIAL_PROGRAM				"LAMBERT"			// [Deprecated 1.4]
#define DAE_PHONG_MATERIAL_PROGRAM					"PHONG"				// [Deprecated 1.4]

#define DAE_AMBIENT_MATERIAL_PARAMETER1_3			"AMBIENT"			// [Deprecated 1.4]
#define DAE_DIFFUSE_MATERIAL_PARAMETER1_3			"DIFFUSE"			// [Deprecated 1.4]
#define DAE_EMISSION_MATERIAL_PARAMETER1_3			"EMISSION"			// [Deprecated 1.4]
#define DAE_TRANSPARENCY_MATERIAL_PARAMETER1_3		"TRANSPARENCY"		// [Deprecated 1.4]
#define DAE_TRANSPARENT_MATERIAL_PARAMETER1_3		"TRANSPARENT"		// [Deprecated 1.4]
#define DAE_REFLECTIVE_MATERIAL_PARAMETER1_3		"REFLECTIVE"		// [Deprecated 1.4]
#define DAE_REFLECTIVITY_MATERIAL_PARAMETER1_3		"REFLECTIVITY"		// [Deprecated 1.4]
#define DAE_SHININESS_MATERIAL_PARAMETER1_3			"SHININESS"			// [Deprecated 1.4]
#define DAE_SPECULAR_MATERIAL_PARAMETER1_3			"SPECULAR"			// [Deprecated 1.4]

// COLLADA 1.2 elements
#define DAE_ARRAY_ELEMENT							"array"				// [Deprecated 1.3]

// COLLADA 1.4 Shared elements
#define DAESHD_DOUBLESIDED_PARAMETER				"double_sided"

// COLLADA 1.3 Shared elements
#define DAESHD_INTENSITY_LIGHT_PARAMETER1_3			"INTENSITY"			// [Deprecated 1.4]

// COLLADA 1.4 Max-specific profile
#define DAEMAX_MAX_PROFILE							"MAX3D"

#define DAEMAX_TARGET_CAMERA_PARAMETER				"target"
#define DAEMAX_ASPECTRATIO_LIGHT_PARAMETER			"aspect_ratio"
#define DAEMAX_TARGET_LIGHT_PARAMETER				"target"
#define DAEMAX_DEFAULT_TARGET_DIST_LIGHT_PARAMETER	"target_default_dist"
#define DAEMAX_OUTERCONE_LIGHT_PARAMETER			"outer_cone"
#define DAEMAX_OVERSHOOT_LIGHT_PARAMETER			"overshoot"
#define DAEMAX_SPECLEVEL_MATERIAL_PARAMETER			"spec_level"
#define DAEMAX_DISPLACEMENT_MATERIAL_PARAMETER		"displacement"
#define DAEMAX_EMISSIONLEVEL_MATERIAL_PARAMETER		"emission_level"
#define DAEMAX_FACETED_MATERIAL_PARAMETER			"faceted"
#define DAEMAX_FILTERCOLOR_MATERIAL_PARAMETER		"filter_color"
#define DAEMAX_INDEXOFREFRACTION_MATERIAL_PARAMETER "index_of_refraction"
#define DAEMAX_USERPROPERTIES_NODE_PARAMETER		"user_properties"
#define DAEMAX_WIREFRAME_MATERIAL_PARAMETER			"wireframe"
#define DAEMAX_FACEMAP_MATERIAL_PARAMETER			"face_map"
#define DAEMAX_AMOUNT_TEXTURE_PARAMETER				"amount"
#define DAEMAX_CAMERA_TARGETDISTANCE_PARAMETER		"target_distance"

// Extra parameters for Max lights
#define DAEMAX_DECAY_TYPE_PARAMETER					"decay_type"
#define DAEMAX_DECAY_START_PARAMETER				"decay_start"
#define DAEMAX_USE_NEAR_ATTEN_PARAMETER				"use_near_attenuation"
#define DAEMAX_NEAR_ATTEN_START_PARAMETER			"near_attenuation_start"
#define DAEMAX_NEAR_ATTEN_END_PARAMETER				"near_attenuation_end"
#define DAEMAX_USE_FAR_ATTEN_PARAMETER				"use_far_attenuation"
#define DAEMAX_FAR_ATTEN_START_PARAMETER			"far_attenuation_start"
#define DAEMAX_FAR_ATTEN_END_PARAMETER				"far_attenuation_end"

// Extra parameters for Max SkyLight
#define DAEMAX_SKY_LIGHT							"skylight"
#define DAEMAX_SKY_RAYS_PER_SAMPLE_PARAMETER		"rays_per_sample"
#define DAEMAX_SKY_RAY_BIAS_PARAMETER				"ray_bias"
#define DAEMAX_SKY_CAST_SHADOWS_PARAMETER			"cast_shadows"
#define DAEMAX_SKY_COLOR_MAP						"color_map"
#define DAEMAX_SKY_COLOR_MAP_ON_PARAMETER			"color_map_on"
#define DAEMAX_SKY_COLOR_MAP_AMOUNT_PARAMETER		"color_map_amount"
#define DAEMAX_SKY_SKYMODE							"sky_mode"
#define DAEMAX_SKY_INTENSITY_ON						"intensity_on"

// Extra parameters for Max Depth of Field multi-pass camera effect
#define DAEMAX_CAMERA_MOTIONBLUR_ELEMENT			"motion_blur"
#define DAEMAX_CAMERA_MB_DISPLAYPASSES_PARAMETER	"display_passes"
#define DAEMAX_CAMERA_MB_TOTALPASSES_PARAMETER		"total_passes"
#define DAEMAX_CAMERA_MB_DURATION_PARAMETER			"duration"
#define DAEMAX_CAMERA_MB_BIAS_PARAMETER				"bias"
#define DAEMAX_CAMERA_MB_NORMWEIGHTS_PARAMETER		"normalized_weights"
#define DAEMAX_CAMERA_MB_DITHERSTRENGTH_PARAMETER	"dither_strength"
#define DAEMAX_CAMERA_MB_TILESIZE_PARAMETER			"tile_size"
#define DAEMAX_CAMERA_MB_DISABLEFILTER_PARAMETER	"disable_filtering"
#define DAEMAX_CAMERA_MB_DISABLEANTIALIAS_PARAMETER	"disable_antialiasing"

// Extra parameters for Max NURBS surfaces
#define DAEMAX_NURBS_FLIP_NORMALS_PARAMETER			"flip_normals"

// COLLADA 1.3 Max-specific profile
#define DAEMAX_TARGET_CAMERA_PARAMETER1_3			"TARGET"			// [Deprecated 1.4]
#define DAEMAX_ASPECTRATIO_LIGHT_PARAMETER1_3		"ASPECT_RATIO"		// [Deprecated 1.4]
#define DAEMAX_TARGET_LIGHT_PARAMETER1_3			"TARGET"			// [Deprecated 1.4]
#define DAEMAX_OUTERCONE_LIGHT_PARAMETER1_3			"OUTERCONE"			// [Deprecated 1.4]
#define DAEMAX_OVERSHOOT_LIGHT_PARAMETER1_3			"OVERSHOOT"			// [Deprecated 1.4]
#define DAEMAX_SPECLEVEL_MATERIAL_PARAMETER1_3		"SPEC_LEVEL"		// [Deprecated 1.4]
#define DAEMAX_EMISSIONLEVEL_MATERIAL_PARAMETER1_3	"EMISSION_LEVEL"	// [Deprecated 1.4]
#define DAEMAX_FACETED_MATERIAL_PARAMETER1_3		"FACETED"			// [Deprecated 1.4]
#define DAEMAX_USERPROPERTIES_NODE_PARAMETER1_3		"USER_PROPERTIES"	// [Deprecated 1.4]

// MAYA Profile Syntax Definitions

// COLLADA 1.4 Maya-specific definitions
#define DAEMAYA_MAYA_PROFILE							"MAYA"

#define DAEMAYA_VAPERTURE_PARAMETER						"vertical_aperture"
#define DAEMAYA_HAPERTURE_PARAMETER						"horizontal_aperture"
#define DAEMAYA_LENSSQUEEZE_PARAMETER					"lens_squeeze"

#define DAEMAYA_PENUMBRA_LIGHT_PARAMETER				"penumbra_angle"
#define DAEMAYA_DROPOFF_LIGHT_PARAMETER					"dropoff"

#define DAEMAYA_NOTE_PARAMETER							"note"
#define DAEMAYA_ENDTIME_PARAMETER						"end_time"
#define DAEMAYA_STARTTIME_PARAMETER						"start_time"

#define DAEMAYA_DRIVER_INPUT							"DRIVER"
#define DAEMAYA_EXTRA_INPUT								"EXTRA"
#define DAEMAYA_WEIGHT_INPUT							"WEIGHT"

// COLLADA 1.3 Maya-specific definitions
#define DAEMAYA_BLINDNAME_PARAMETER						"BLINDNAME"
#define DAEMAYA_BLINDTYPEID_PARAMETER					"BLINDTYPEID"
#define DAEMAYA_DOUBLE_SIDED_PARAMETER					"DOUBLE_SIDED"
#define DAEMAYA_LAYER_PARAMETER							"layer"
#define DAEMAYA_LONGNAME_PARAMETER						"long_name"
#define DAEMAYA_PREINFINITY_PARAMETER					"pre_infinity"
#define DAEMAYA_POSTINFINITY_PARAMETER					"post_infinity"
#define DAEMAYA_PREINFINITY_PARAMETER1_3				"PRE_INFINITY"
#define DAEMAYA_POSTINFINITY_PARAMETER1_3				"POST_INFINITY"
#define DAEMAYA_SHORTNAME_PARAMETER						"short_name"

#define DAEMAYA_MAYA_NOTE_PARAMETER1_3					"MAYA_NOTE"					// [Deprecated 1.4]
#define DAEMAYA_VISIBILITY_PARAMETER1_3					"VISIBILITY"				// [Deprecated 1.4]
#define DAEMAYA_ENDTIME_PARAMETER1_3					"END_TIME"					// [Deprecated 1.4]
#define DAEMAYA_STARTTIME_PARAMETER1_3					"START_TIME"				// [Deprecated 1.4]

#define DAEMAYA_VAPERTURE_PARAMETER1_3					"VERTICAL_APERTURE"			// [Deprecated 1.4]
#define DAEMAYA_HAPERTURE_PARAMETER1_3					"HORIZONTAL_APERTURE"		// [Deprecated 1.4]
#define DAEMAYA_LENSSQUEEZE_PARAMETER1_3				"LENS_SQUEEZE"				// [Deprecated 1.4]

#define DAEMAYA_TEXTURE_WRAPU_PARAMETER					"wrapU"
#define DAEMAYA_TEXTURE_WRAPV_PARAMETER					"wrapV"
#define DAEMAYA_TEXTURE_MIRRORU_PARAMETER				"mirrorU"
#define DAEMAYA_TEXTURE_MIRRORV_PARAMETER				"mirrorV"
#define DAEMAYA_TEXTURE_COVERAGEU_PARAMETER				"coverageU"
#define DAEMAYA_TEXTURE_COVERAGEV_PARAMETER				"coverageV"
#define DAEMAYA_TEXTURE_TRANSFRAMEU_PARAMETER			"translateFrameU"
#define DAEMAYA_TEXTURE_TRANSFRAMEV_PARAMETER			"translateFrameV"
#define DAEMAYA_TEXTURE_ROTFRAME_PARAMETER				"rotateFrame"
#define DAEMAYA_TEXTURE_STAGGER_PARAMETER				"stagger"
#define DAEMAYA_TEXTURE_REPEATU_PARAMETER				"repeatU"
#define DAEMAYA_TEXTURE_REPEATV_PARAMETER				"repeatV"
#define DAEMAYA_TEXTURE_OFFSETU_PARAMETER				"offsetU"
#define DAEMAYA_TEXTURE_OFFSETV_PARAMETER				"offsetV"
#define DAEMAYA_TEXTURE_ROTATEUV_PARAMETER				"rotateUV"
#define DAEMAYA_TEXTURE_NOISEU_PARAMETER				"noiseU"
#define DAEMAYA_TEXTURE_NOISEV_PARAMETER				"noiseV"
#define DAEMAYA_TEXTURE_FAST_PARAMETER					"fast"
#define DAEMAYA_TEXTURE_BLENDMODE_PARAMETER				"blend_mode"
#define DAEMAYA_TEXTURE_BLENDMODE_PARAMETER1_3			"BLEND_MODE"

#define DAEMAYA_PENUMBRA_LIGHT_PARAMETER1_3				"PENUMBRA_ANGLE"			// [Deprecated 1.4]

#define DAEMAYA_CONSTANT_INFINITY						"CONSTANT"			
#define DAEMAYA_LINEAR_INFINITY							"LINEAR"			
#define DAEMAYA_CYCLE_INFINITY							"CYCLE"				
#define DAEMAYA_CYCLE_RELATIVE_INFINITY					"CYCLE_RELATIVE"	
#define DAEMAYA_OSCILLATE_INFINITY						"OSCILLATE"			

#define DAEMAYA_NONE_BLENDMODE							"NONE"
#define DAEMAYA_OVER_BLENDMODE							"OVER"
#define DAEMAYA_IN_BLENDMODE							"IN"
#define DAEMAYA_OUT_BLENDMODE							"OUT"
#define DAEMAYA_ADD_BLENDMODE							"ADD"
#define DAEMAYA_SUBSTRACT_BLENDMODE						"SUBSTRACT"
#define DAEMAYA_MULTIPLY_BLENDMODE						"MULTIPLY"
#define DAEMAYA_DIFFERENCE_BLENDMODE					"DIFFERENCE"
#define DAEMAYA_LIGHTEN_BLENDMODE						"LIGHTEN"
#define DAEMAYA_DARKEN_BLENDMODE						"DARKEN"
#define DAEMAYA_SATURATE_BLENDMODE						"SATURATE"
#define DAEMAYA_DESATURATE_BLENDMODE					"DESATURATE"
#define DAEMAYA_ILLUMINATE_BLENDMODE					"ILLUMINATE"

#define DAEMAYA_PROJECTION_ELEMENT						"projection"
#define DAEMAYA_PROJECTION_TYPE_PARAMETER				"type"
#define DAEMAYA_PROJECTION_MATRIX_PARAMETER				"matrix"

#define DAEMAYA_PROJECTION_PROGRAM						"PROJECTION"				// [Deprecated 1.4]
#define DAEMAYA_PROJECTION_TYPE_PARAMETER1_3			"TYPE"						// [Deprecated 1.4]
#define DAEMAYA_PROJECTION_MATRIX_PARAMETER1_3			"MATRIX"					// [Deprecated 1.4]

// FCollada profile
#define DAE_FCOLLADA_PROFILE							"FCOLLADA"

// FCOLLADA syntax for custom attributes
#define DAEFC_DYNAMIC_ATTRIBUTES_ELEMENT				"dynamic_attributes"
#define DAEFC_BOOLEAN_ATTRIBUTE_TYPE					"bool"
#define DAEFC_FLOAT_ATTRIBUTE_TYPE						"float"
#define DAEFC_INT_ATTRIBUTE_TYPE						"int"
#define DAEFC_STRING_ATTRIBUTE_TYPE						"string"
#define DAEFC_FLOAT3_ATTRIBUTE_TYPE						"float3"
#define DAEFC_FLOAT4_ATTRIBUTE_TYPE						"float4"
#define DAEFC_MATRIX_ATTRIBUTE_TYPE						"float4x4"

// FCOLLADA systax for camera depth of field multi-pass effect
#define DAEFC_CAMERA_DEPTH_OF_FIELD_ELEMENT				"depth_of_field"
#define DAEFC_CAMERA_DOF_USETARGETDIST_PARAMETER		"use_target_dist"
#define DAEFC_CAMERA_DOF_DISPLAYPASSES_PARAMETER		"display_passes"
#define DAEFC_CAMERA_DOF_FOCALDEPTH_PARAMETER			"focal_depth"
#define DAEFC_CAMERA_DOF_TOTALPASSES_PARAMETER			"total_passes"
#define DAEFC_CAMERA_DOF_SAMPLERADIUS_PARAMETER			"sample_radius"
#define DAEFC_CAMERA_DOF_SAMPLEBIAS_PARAMETER			"sample_bias"
#define DAEFC_CAMERA_DOF_NORMWEIGHTS_PARAMETER			"normalized_weights"
#define DAEFC_CAMERA_DOF_DITHERSTR_PARAMETER			"dither_strength"
#define DAEFC_CAMERA_DOF_TILESIZE_PARAMETER				"tile_size"
#define DAEFC_CAMERA_DOF_DISFILTERING_PARAMETER			"disable_filtering"
#define DAEFC_CAMERA_DOF_DISANTIALIAS_PARAMETER			"disable_antialiasing"
#define DAEFC_CAMERA_DOF_USEORIGLOC_PARAMETER			"use_original_location"

// FCollada custom parameters
#define DAEFC_TARGET_PARAMETER							"target"
#define DAEFC_INTENSITY_LIGHT_PARAMETER					"intensity"
#define DAEFC_VISIBILITY_PARAMETER						"visibility"

// nVidia profiles
#define DAENV_NVIMPORT_PROFILE							"NV_import"
#define DAENV_IMPORT_ELEMENT							"import"
#define DAENV_PROFILE_PROPERTY							"profile"
#define DAENV_HLSL_PROFILE								"d3dfx"
#define DAENV_FILENAME_PROPERTY							"url"

// COLLADA Error export syntax

#define DAEERR_UNKNOWN_ELEMENT							"unknown"
#define DAEERR_UNKNOWN_IDREF							"UNKNOWN_IDREF"
#define DAEERR_UNKNOWN_INPUT							"UNKNOWN"
#define DAEERR_UNKNOWN_MORPH_METHOD						"UNKNOWN"

#endif // _DAE_SYNTAX_H_
