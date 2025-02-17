/*
	Copyright (C) 2006 Feeling Software Inc
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUBoundingBox.h
	This file contains the FUBoundingBox class.
*/

#ifndef _FU_BOUNDINGBOX_H_
#define _FU_BOUNDINGBOX_H_

/**
	An axis-aligned bounding box.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUBoundingBox
{
private:
	FMVector3 minimum;
	FMVector3 maximum;

public:
	/** Empty constructor.
		The minimum and maximum bounds are set at the largest and most impossible values. */
	FUBoundingBox();

	/** Constructor.
		@param minimum The minimum bounds of the bounding box.
		@param maximum The maximum bounds of the bounding box. */
	FUBoundingBox(const FMVector3& minimum, const FMVector3& maximum);

	/** Copy constructor.
		@param copy The bounding box to duplicate. */
	FUBoundingBox(const FUBoundingBox& copy);

	/** Destructor. */
	~FUBoundingBox();

	/** Resets the bounding box.
		The minimum and maximum bounds are set at the largest and most impossible values.
		Including a freshly reset bounding box to a valid bounding box will have no effect. */
	void Reset();

	/** Retrieves whether the bounding box contains valid information.
		An invalid bounding box has a minimum value that is greater than the maximum value.
		Reseting the bounding box and the empty constructor generate invalid bounding boxes on purpose.
		@return The validity state of the bounding box. */
	bool IsValid();

	/** Retrieves the minimum bounds of the bounding box.
		@return The minimum bounds. */
	inline const FMVector3& GetMin() const { return minimum; }

	/** Retrieves the maximum bounds of the bounding box.
		@return The maximum bounds. */
	inline const FMVector3& GetMax() const { return maximum; }

	/** Sets the minimum bounds of the bounding box.
		@param _min The new minimum bounds of the bounding box. */
	inline void SetMin(const FMVector3& _min) { minimum = _min; }

	/** Sets the maximum bounds of the bounding box.
		@param _max The new maximum bounds of the bounding box. */
	inline void SetMax(const FMVector3& _max) { maximum = _max; }

	/** Retrieves the center of the bounding box.
		@return The center of the bounding box. */
	inline FMVector3 GetCenter() const { return (minimum + maximum) / 2.0f; }

	/** Extends the bounding box to include the given 3D coordinate.
		@param point A 3D coordinate to include in the bounding box. */
	inline void Include(const FMVector3& point) { minimum.x = min(minimum.x, point.x); minimum.y = min(minimum.y, point.y); minimum.z = min(minimum.z, point.z); maximum.x = max(maximum.x, point.x); maximum.y = max(maximum.y, point.y); maximum.z = max(maximum.z, point.z); }

	/** Extends the bounding box to include another bounding box.
		@param boundingBox A bounding box to include in this bounding box. */
	inline void Include(const FUBoundingBox& boundingBox) { const FMVector3& n = boundingBox.minimum; const FMVector3& x = boundingBox.maximum; minimum.x = min(minimum.x, n.x); minimum.y = min(minimum.y, n.y); minimum.z = min(minimum.z, n.z); maximum.x = max(maximum.x, x.x); maximum.y = max(maximum.y, x.y); maximum.z = max(maximum.z, x.z); }

	/** Transform the bounding box into another space basis.
		@param transform The transformation matrix to go into the other space basis.
		@return The transformed bounding box. */
	FUBoundingBox Transform(const FMMatrix44& transform) const;
};

#endif // _FU_BOUNDINGBOX_H_