/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class ShadowMap2D extends Object
	native
	noexport;

/** The texture which contains the shadow-map data. */
var private const ShadowMapTexture2D Texture;

/** The scale which is applied to the shadow-map coordinates before sampling the shadow-map textures. */
var private const Vector2D CoordinateScale;

/** The bias which is applied to the shadow-map coordinates before sampling the shadow-map textures. */
var private const Vector2D CoordinateBias;

/** The GUID of the light which this shadow-map is for. */
var private const Guid LightGuid;
