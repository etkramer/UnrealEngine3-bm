//------------------------------------------------------------------------------
// Point, size and rectangle primitives.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxPrimitives_H__
#define FxPrimitives_H__

#include "FxPlatform.h"

namespace OC3Ent
{

namespace Face
{

class FxPoint
{
public:
	FxPoint()
		: x(0), y(0)
	{ }
	FxPoint(FxInt32 xx, FxInt32 yy)
		: x(xx), y(yy)
	{ }

	FxInt32 x;
	FxInt32 y;
};


class FxSize2D
{
public:
	FxSize2D()
		: x(0), y(0)
	{ }
	FxSize2D( FxInt32 w, FxInt32 h)
		: x(w), y(h)
	{ }

	FxInt32 GetWidth() const { return x; }
	FxInt32 GetHeight() const { return y; }

	FxInt32 x;
	FxInt32 y;
};

class FxRect
{
public:
	FxRect()
		: x(0), y(0), width(0), height(0)
	{ }
	FxRect(FxInt32 xx, FxInt32 yy, FxInt32 ww, FxInt32 hh)
		: x(xx), y(yy), width(ww), height(hh)
	{ }
	FxRect(const FxPoint& topLeft, const FxPoint& bottomRight);
	FxRect(const FxPoint& pos, const FxSize2D& size);

	FxInt32 GetX() const { return x; }
	void SetX(FxInt32 xx) { x = xx; }

	FxInt32 GetY() const { return y; }
	void SetY(FxInt32 yy) { y = yy; }

	FxInt32 GetWidth() const { return width; }
	void SetWidth(FxInt32 w) { width = w; }

	FxInt32 GetHeight() const { return height; }
	void SetHeight(FxInt32 h) { height = h; }

	FxPoint GetPosition() const { return FxPoint(x, y); }
	void SetPosition( const FxPoint &p ) { x = p.x; y = p.y; }

	FxSize2D GetSize() const { return FxSize2D(width, height); }
	void SetSize( const FxSize2D &s ) { width = s.GetWidth(); height = s.GetHeight(); }

	FxPoint GetTopLeft() const { return GetPosition(); }
	FxPoint GetLeftTop() const { return GetTopLeft(); }
	void SetTopLeft(const FxPoint &p) { SetPosition(p); }
	void SetLeftTop(const FxPoint &p) { SetTopLeft(p); }

	FxPoint GetBottomRight() const { return FxPoint(GetRight(), GetBottom()); }
	FxPoint GetRightBottom() const { return GetBottomRight(); }
	void SetBottomRight(const FxPoint &p) { SetRight(p.x); SetBottom(p.y); }
	void SetRightBottom(const FxPoint &p) { SetBottomRight(p); }

	FxInt32 GetLeft()   const { return x; }
	FxInt32 GetTop()    const { return y; }
	FxInt32 GetBottom() const { return y + height - 1; }
	FxInt32 GetRight()  const { return x + width - 1; }

	void SetLeft(FxInt32 left) { x = left; }
	void SetRight(FxInt32 right) { width = right - x + 1; }
	void SetTop(FxInt32 top) { y = top; }
	void SetBottom(FxInt32 bottom) { height = bottom - y + 1; }

	// operations with rect
	FxRect& Inflate(FxInt32 dx, FxInt32 dy);
	FxRect& Inflate(FxInt32 d) { return Inflate(d, d); }
	FxRect Inflate(FxInt32 dx, FxInt32 dy) const
	{
		FxRect r = *this;
		r.Inflate(dx, dy);
		return r;
	}

	FxRect& Deflate(FxInt32 dx, FxInt32 dy) { return Inflate(-dx, -dy); }
	FxRect& Deflate(FxInt32 d) { return Inflate(-d); }
	FxRect Deflate(FxInt32 dx, FxInt32 dy) const
	{
		FxRect r = *this;
		r.Deflate(dx, dy);
		return r;
	}

	void Offset(FxInt32 dx, FxInt32 dy) { x += dx; y += dy; }
	void Offset(const FxPoint& pt) { Offset(pt.x, pt.y); }

	FxRect& Intersect(const FxRect& rect);
	FxRect Intersect(const FxRect& rect) const
	{
		FxRect r = *this;
		r.Intersect(rect);
		return r;
	}

	FxRect operator+(const FxRect& rect) const;
	FxRect& operator+=(const FxRect& rect);

	bool operator==(const FxRect& rect) const;
	bool operator!=(const FxRect& rect) const { return !(*this == rect); }

	// return true if the point is (not strictly) inside the rect
	bool Inside(FxInt32 x, FxInt32 y) const;
	bool Inside(const FxPoint& pt) const { return Inside(pt.x, pt.y); }

	// return true if the rectangles have a non empty intersection
	bool Intersects(const FxRect& rect) const;

protected:
	FxInt32 x;
	FxInt32 y;
	FxInt32 width;
	FxInt32 height;
};

} // namespace Face

} // namespace OC3Ent

#endif
