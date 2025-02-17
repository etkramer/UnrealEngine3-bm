/*
	Copyright (C) 2006 Feeling Software Inc
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FUtils/FUBoundingBox.h"

//
// FUBoundingBox
//

FUBoundingBox::FUBoundingBox()
:	minimum(FLT_MAX, FLT_MAX, FLT_MAX)
,	maximum(-FLT_MAX, -FLT_MAX, -FLT_MAX)
{
}

FUBoundingBox::FUBoundingBox(const FMVector3& _min, const FMVector3& _max)
:	minimum(_min), maximum(_max)
{
}

FUBoundingBox::FUBoundingBox(const FUBoundingBox& copy)
:	minimum(copy.minimum), maximum(copy.maximum)
{
}

FUBoundingBox::~FUBoundingBox()
{
}

void FUBoundingBox::Reset()
{
	minimum = FMVector3(FLT_MAX, FLT_MAX, FLT_MAX);
	maximum = FMVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}

bool FUBoundingBox::IsValid()
{
	return minimum.x < maximum.x && minimum.y < maximum.y && minimum.z < maximum.z;
}

FUBoundingBox FUBoundingBox::Transform(const FMMatrix44& transform) const
{
	FUBoundingBox transformedBoundingBox;

	FMVector3 testPoints[6] =
	{
		FMVector3(minimum.x, maximum.y, minimum.z), FMVector3(minimum.x, maximum.y, maximum.z),
		FMVector3(maximum.x, maximum.y, minimum.z), FMVector3(minimum.x, minimum.y, maximum.z),
		FMVector3(maximum.x, minimum.y, minimum.z), FMVector3(maximum.x, minimum.y, maximum.z)
	};

	for (size_t i = 0; i < 6; ++i)
	{
		testPoints[i] = transform.TransformCoordinate(testPoints[i]);
		transformedBoundingBox.Include(testPoints[i]);
	}
	transformedBoundingBox.Include(transform.TransformCoordinate(minimum));
	transformedBoundingBox.Include(transform.TransformCoordinate(maximum));

	return transformedBoundingBox;
}

