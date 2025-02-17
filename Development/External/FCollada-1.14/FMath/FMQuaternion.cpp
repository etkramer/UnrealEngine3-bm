/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FMath/FMath.h"
#include "FMath/FMVector3.h"
#include "FMath/FMQuaternion.h"

// [Glaforte 03-08-2006] VERY EXPERIMENTAL CODE: DON'T USE.

FMQuaternion::FMQuaternion(const FMVector3& axis, float angle)
{
	float s = sinf(angle / 2.0f);
	x = axis.x * s;
	y = axis.y * s;
	z = axis.z * s;
	w = cosf(angle / 2.0f);
}

FMQuaternion FMQuaternion::operator*(const FMQuaternion& q) const
{
	FMQuaternion r;
	r.x = w * q.x + x * q.w + y * q.z - z * q.y;
	r.y = w * q.y + y * q.w + z * q.x - x * q.z;
	r.z = w * q.z + z * q.w + x * q.y - y * q.x;
	r.w = w * q.w - x * q.x - y * q.y - z * q.z;
	return r;
}

FMQuaternion FMQuaternion::EulerRotationQuaternion(float x, float y, float z)
{
	FMQuaternion qx(FMVector3::XAxis, x);
	FMQuaternion qy(FMVector3::YAxis, y);
	FMQuaternion qz(FMVector3::ZAxis, z);
	return qx * qy * qz;
}

void FMQuaternion::ToAngleAxis(FMVector3& axis, float& angle)
{
	angle = 2.0f * acosf(w);
	float s = sinf(angle / 2.0f);
	if (!IsEquivalent(s, 0.0f))
	{
		axis.x = x / s;
		axis.y = y / s;
		axis.z = z / s;
		axis.NormalizeIt();
	}
	else
	{
		// If s == 0, then angle == 0 and there is no rotation: assign any axis.
		axis = FMVector3::XAxis;
	}
}

// [GLaforte - 31/08/2006: this is a work in progress...]
FMVector3 FMQuaternion::ToEuler(FMVector3* UNUSED(previousAngles))
{
	FMVector3 angles;

	// Convert the quaternion into Euler angles.
	float siny = 2.0f * (x * z + y * w);
	if (siny > 1.0f - FLT_TOLERANCE) // singularity at north pole
	{ 
		angles.y = (float) FMath::Pi / 2;

//D		angles.x = 2 * atan2(x,w);
//D		angles.z = 0;
	}
	else if (siny < -1.0f + FLT_TOLERANCE) // singularity at south pole
	{
		angles.y = (float) -FMath::Pi / 2;

//D		angles.x = -2 * atan2(x,w);
//D		angles.z = 0;
	}
	else
	{
		angles.y = asinf(siny);
		float cosy = cosf(angles.y);

		// I derived:
		angles.x = atan2f(2.0f * (x*w+y*z) / cosy, x*x+y*y - 1 - cosy); //		angles.x = atan2f(2*y*w-2*x*z, 1 - 2*y*y - 2*z*z);
//D		angles.z = atan2f(2*x*w-2*y*z, 1 - 2*x*x - 2*z*z);
	}

	return angles;
}

FMMatrix44 FMQuaternion::ToMatrix()
{
	FMVector3 axis; float angle;
	ToAngleAxis(axis, angle);
	return FMMatrix44::AxisRotationMatrix(axis, angle);
}
