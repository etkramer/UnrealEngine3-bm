/*=============================================================================
	COLLADA common header.
	Based on Feeling Software's Collada import classes [FCollada].
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNCOLLADA_H__
#define __UNCOLLADA_H__

#pragma warning(disable : 4996)
#pragma warning(disable : 4548)

#pragma pack (push,8)
#pragma warning ( push )
#include "FCollada.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDAnimated.h"
#include "FCDocument/FCDAnimation.h"
#include "FCDocument/FCDAnimationChannel.h"
#include "FCDocument/FCDAnimationCurve.h"
#include "FCDocument/FCDAnimationCurveTools.h"
#include "FCDocument/FCDAnimationKey.h"
#include "FCDocument/FCDAnimationMultiCurve.h"
#include "FCDocument/FCDAsset.h"
#include "FCDocument/FCDCamera.h"
#include "FCDocument/FCDController.h"
#include "FCDocument/FCDEffect.h"
#include "FCDocument/FCDEffectStandard.h"
#include "FCDocument/FCDEntity.h"
#include "FCDocument/FCDEntityInstance.h"
#include "FCDocument/FCDExtra.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometryInstance.h"
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometryPolygons.h"
#include "FCDocument/FCDGeometryPolygonsInput.h"
#include "FCDocument/FCDGeometrySource.h"
#include "FCDocument/FCDImage.h"
#include "FCDocument/FCDLibrary.h"
#include "FCDocument/FCDLight.h"
#include "FCDocument/FCDMaterial.h"
#include "FCDocument/FCDMaterialInstance.h"
//#include "FCDocument/FCDMaterialLibrary.h"
#include "FCDocument/FCDMorphController.h"
#include "FCDocument/FCDSceneNode.h"
#include "FCDocument/FCDSceneNodeTools.h"
#include "FCDocument/FCDSkinController.h"
#include "FCDocument/FCDTexture.h"
#include "FUtils/FUFileManager.h"
#include "FUtils/FUDaeSyntax.h"
#pragma warning ( pop )
#pragma pack (pop)

// The Unreal Engine 3 COLLADA profile syntax for the extra exported information
#define DAEUE3_PROFILE_NAME							"UE3"

// Conversion macros
inline FMVector3 ToFMVector3(const FColor& Color) { return FMVector3(Color.R / 255.0f, Color.G / 255.0f, Color.B / 255.0f); }
inline FMVector3 ToFMVector3(const FVector& Vector) { return FMVector3(Vector.X, -Vector.Y, Vector.Z); }
inline FMVector4 ToFMVector4(const FColor& Color) { return FMVector4(Color.R / 255.0f, Color.G / 255.0f, Color.B / 255.0f, Color.A / 255.0f); }

// Custom conversion functors.
class FCDConversionScaleOffsetFunctor : public FCDConversionFunctor
{
public:
	FLOAT ScaleFactor, Offset;
	FCDConversionScaleOffsetFunctor(FLOAT _ScaleFactor = 1.0f, FLOAT _Offset = 0.0f)
		:	ScaleFactor(_ScaleFactor), Offset(_Offset) {}
	virtual ~FCDConversionScaleOffsetFunctor() {}
	virtual FLOAT operator() (FLOAT Value) { return (Value + Offset) * ScaleFactor; }
	FCDConversionScaleOffsetFunctor& Set(FLOAT _ScaleFactor = 1.0f, FLOAT _Offset = 0.0f)
	{
		ScaleFactor = _ScaleFactor;
		Offset = _Offset;
		return *this;
	}
};

#pragma warning(default : 4548)

#endif // __UNCOLLADA_H__
