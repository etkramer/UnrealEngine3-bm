/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * This file contains the Xenon specific collision routines
 */


/**
 * Performs a sphere vs box intersection test using Arvo's algorithm:
 *
 *	for each i in (x, y, z)
 *		if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
 *		else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2
 *
 * This version does the work without any branches
 *
 * @param Sphere the sphere being tested against the AABB
 * @param AABB the box being tested against
 *
 * @return Whether the sphere/box intersect or not.
 */
FORCEINLINE UBOOL SphereAABBIntersectionTestIntrinsics(XMVECTOR SphereCenter,
	XMVECTOR RadiusSquared,XMVECTOR AABBMin,XMVECTOR AABBMax)
{
	// Accumulates the distance as we iterate axis
	XMVECTOR DistSquared = XMVectorZero();
	// Get the masks for determining which is min/max for the axis
	XMVECTOR MinMask = XMVectorLess(SphereCenter,AABBMin);
	XMVECTOR MaxMask = XMVectorGreater(SphereCenter,AABBMax);
	// Compute all the deltas at once
	XMVECTOR MinDelta = XMVectorSubtract(SphereCenter,AABBMin);
	XMVECTOR MaxDelta = XMVectorSubtract(SphereCenter,AABBMax);
	// Move the correct delta values into the correct component using the masks
	DistSquared = XMVectorSelect(DistSquared,MinDelta,MinMask);
	DistSquared = XMVectorSelect(DistSquared,MaxDelta,MaxMask);
	// Square the selected deltas
	DistSquared = XMVector3Dot(DistSquared,DistSquared);
	// If the distance is less than or equal to the radius, they intersect
	return XMVector4LessOrEqual(DistSquared,RadiusSquared);
}

/**
 * Provided as a convenience wrapper. See above.
 *
 * @param Sphere the sphere being tested against the AABB
 * @param AABB the box being tested against
 *
 * @return Whether the sphere/box intersect or not.
 */
FORCEINLINE UBOOL SphereAABBIntersectionTest(const FSphere& Sphere,const FBox& AABB)
{
	// Load sphere
	XMVECTOR SphereCenter = XMLoadFloat3((const XMFLOAT3*)&Sphere);
	XMVECTOR RadiusSquared = XMVectorReplicate(Sphere.W);
	RadiusSquared = XMVectorMultiply(RadiusSquared,RadiusSquared);;
	// Load box
	XMVECTOR Min = XMLoadFloat3((const XMFLOAT3*)&AABB.Min);
	XMVECTOR Max = XMLoadFloat3((const XMFLOAT3*)&AABB.Max);
	// Use the intrinsics version
	return SphereAABBIntersectionTestIntrinsics(SphereCenter,RadiusSquared,Min,Max);
}

/**
 * Converts a FBoxSphereBounds into a FBox for the above comparison
 *
 * @param Sphere the sphere being tested against the AABB
 * @param AABB the box being tested against
 *
 * @return Whether the sphere/box intersect or not.
 */
FORCEINLINE UBOOL SphereAABBIntersectionTest(const FSphere& Sphere,const FBoxSphereBounds& Bounds)
{
	// Load sphere
	XMVECTOR SphereCenter = XMLoadFloat3((const XMFLOAT3*)&Sphere);
	XMVECTOR RadiusSquared = XMVectorReplicate(Sphere.W);
	RadiusSquared = XMVectorMultiply(RadiusSquared,RadiusSquared);;
	// Load box
	XMVECTOR Center = XMLoadFloat3((const XMFLOAT3*)&Bounds.Origin);
	XMVECTOR Extent = XMLoadFloat3((const XMFLOAT3*)&Bounds.BoxExtent);
	// Put into min/max form
	XMVECTOR Min = XMVectorSubtract(Center,Extent);
	XMVECTOR Max = XMVectorAdd(Center,Extent);
	// Use the intrinsics version
	return SphereAABBIntersectionTestIntrinsics(SphereCenter,RadiusSquared,Min,Max);
}

/**
* Provided as a convenience wrapper. See above.
*
* @param Sphere the sphere being tested against the AABB
* @param InRadiusSquared the size of the sphere being tested
* @param AABB the box being tested against
*
* @return Whether the sphere/box intersect or not.
*/
FORCEINLINE UBOOL SphereAABBIntersectionTest(const FVector& Sphere,FLOAT InRadiusSquared,const FBox& AABB)
{
	// Load sphere
	XMVECTOR SphereCenter = XMLoadFloat3((const XMFLOAT3*)&Sphere);
	XMVECTOR RadiusSquared = XMVectorReplicate(InRadiusSquared);
	// Load box
	XMVECTOR Min = XMLoadFloat3((const XMFLOAT3*)&AABB.Min);
	XMVECTOR Max = XMLoadFloat3((const XMFLOAT3*)&AABB.Max);
	// Use the intrinsics version
	return SphereAABBIntersectionTestIntrinsics(SphereCenter,RadiusSquared,Min,Max);
}

/**
* Converts a FBoxSphereBounds into a FBox for the above comparison
*
* @param Sphere the sphere being tested against the AABB
* @param InRadiusSquared the size of the sphere being tested
* @param AABB the box being tested against
*
* @return Whether the sphere/box intersect or not.
*/
FORCEINLINE UBOOL SphereAABBIntersectionTest(const FVector& Sphere,FLOAT InRadiusSquared,const FBoxSphereBounds& Bounds)
{
	// Load sphere
	XMVECTOR SphereCenter = XMLoadFloat3((const XMFLOAT3*)&Sphere);
	XMVECTOR RadiusSquared = XMVectorReplicate(InRadiusSquared);
	// Load box
	XMVECTOR Center = XMLoadFloat3((const XMFLOAT3*)&Bounds.Origin);
	XMVECTOR Extent = XMLoadFloat3((const XMFLOAT3*)&Bounds.BoxExtent);
	// Put into min/max form
	XMVECTOR Min = XMVectorSubtract(Center,Extent);
	XMVECTOR Max = XMVectorAdd(Center,Extent);
	// Use the intrinsics version
	return SphereAABBIntersectionTestIntrinsics(SphereCenter,RadiusSquared,Min,Max);
}
