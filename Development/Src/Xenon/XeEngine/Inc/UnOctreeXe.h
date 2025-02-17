/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * This file contains the Xenon specific octree routines
 */

/**
 * Struct containing pre-calculated info for sphere/box tests
 */
struct FRadiusOverlapCheck
{
	/** Center of the sphere */
	XMVECTOR SphereCenter;
	/** Size of the sphere pre-squared */
	XMVECTOR RadiusSquared;

	/**
	 * Precalculates the information for faster compares
	 *
	 * @param Location the center of the sphere being tested
	 * @param Radius the size of the sphere being tested
	 */
	FRadiusOverlapCheck(const FVector& Sphere,FLOAT Radius)
	{
		// Load sphere
		SphereCenter = XMLoadFloat3((const XMFLOAT3*)&Sphere);
		RadiusSquared = XMVectorReplicate(Radius);
		RadiusSquared = XMVectorMultiply(RadiusSquared,RadiusSquared);;
	}

	/**
	 * Tests the sphere against a FBoxSphereBounds
	 *
	 * @param Bounds the bounds to test against
	 */
	FORCEINLINE UBOOL SphereBoundsTest(const FBoxSphereBounds& Bounds)
	{
		// Load box
		XMVECTOR Center = XMLoadFloat3((const XMFLOAT3*)&Bounds.Origin);
		XMVECTOR Extent = XMLoadFloat3((const XMFLOAT3*)&Bounds.BoxExtent);
		// Put into min/max form
		XMVECTOR Min = XMVectorSubtract(Center,Extent);
		XMVECTOR Max = XMVectorAdd(Center,Extent);
		// Use the intrinsics version
		return SphereAABBIntersectionTestIntrinsics(SphereCenter,RadiusSquared,Min,Max);
	}
};
