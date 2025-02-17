//------------------------------------------------------------------------------
// Point, size and rectangle primitives.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxPrimitives.h"
#include "FxUtil.h"

namespace OC3Ent
{

namespace Face
{

FxRect::FxRect(const FxPoint& point1, const FxPoint& point2)
{
	x = point1.x;
	y = point1.y;
	width = point2.x - point1.x;
	height = point2.y - point1.y;

	if (width < 0)
	{
		width = -width;
		x = point2.x;
	}
	width++;

	if (height < 0)
	{
		height = -height;
		y = point2.y;
	}
	height++;
}

FxRect::FxRect(const FxPoint& point, const FxSize2D& size)
{
	x = point.x; y = point.y;
	width = size.x; height = size.y;
}

bool FxRect::operator==(const FxRect& rect) const
{
	return ((x == rect.x) &&
		(y == rect.y) &&
		(width == rect.width) &&
		(height == rect.height));
}

FxRect& FxRect::operator += (const FxRect& rect)
{
	*this = (*this + rect);
	return ( *this ) ;
}

FxRect FxRect::operator + (const FxRect& rect) const
{
	FxInt32 x1 = FxMin(this->x, rect.x);
	FxInt32 y1 = FxMin(this->y, rect.y);
	FxInt32 y2 = FxMax(y+height, rect.height+rect.y);
	FxInt32 x2 = FxMax(x+width, rect.width+rect.x);
	return FxRect(x1, y1, x2-x1, y2-y1);
}

FxRect& FxRect::Inflate(FxInt32 dx, FxInt32 dy)
{
	x -= dx;
	y -= dy;
	width += 2*dx;
	height += 2*dy;

	// check that we didn't make the rectangle invalid by accident (you almost
	// never want to have negative coords and never want negative size)
	if ( x < 0 )
		x = 0;
	if ( y < 0 )
		y = 0;

	// what else can we do?
	if ( width < 0 )
		width = 0;
	if ( height < 0 )
		height = 0;

	return *this;
}

bool FxRect::Inside(FxInt32 cx, FxInt32 cy) const
{
	return ( (cx >= x) && (cy >= y)
		&& ((cy - y) < height)
		&& ((cx - x) < width)
		);
}

FxRect& FxRect::Intersect(const FxRect& rect)
{
	FxInt32 x2 = GetRight(),
		y2 = GetBottom();

	if ( x < rect.x )
		x = rect.x;
	if ( y < rect.y )
		y = rect.y;
	if ( x2 > rect.GetRight() )
		x2 = rect.GetRight();
	if ( y2 > rect.GetBottom() )
		y2 = rect.GetBottom();

	width = x2 - x + 1;
	height = y2 - y + 1;

	if ( width <= 0 || height <= 0 )
	{
		width =
			height = 0;
	}

	return *this;
}

bool FxRect::Intersects(const FxRect& rect) const
{
	FxRect r = Intersect(rect);

	// if there is no intersection, both width and height are 0
	return r.width != 0;
}

} // namespace Face

} // namespace OC3Ent
