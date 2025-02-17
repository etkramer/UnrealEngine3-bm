//------------------------------------------------------------------------------
// A way of drawing sliders so they can be more "glanceable"
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxSlider_H__
#define FxSlider_H__

#include "FxPlatform.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/slider.h"
#endif

namespace OC3Ent
{

namespace Face
{

#define FxConjoinedSlider 192828

class FxSlider : public wxWindow
{
public:
	FxSlider(void);
	FxSlider(wxWindow *parent, wxWindowID id,
		FxInt32 value, FxInt32 minValue, FxInt32 maxValue,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, FxInt32 style = FxConjoinedSlider );

	~FxSlider(void);

	// get/set the current slider value (should be in range)
	virtual FxInt32 GetValue() const;
	virtual void SetValue(FxInt32 value);

	// retrieve/change the range
	virtual void SetRange(FxInt32 minValue, FxInt32 maxValue);
	virtual FxInt32 GetMin() const;
	virtual FxInt32 GetMax() const;

	// the line/page size is the increment by which the slider moves when
	// cursor arrow key/page up or down are pressed (clicking the mouse is like
	// pressing PageUp/Down) and are by default set to 1 and 1/10 of the range
	virtual void SetLineSize(FxInt32 lineSize);
	virtual void SetPageSize(FxInt32 pageSize);
	virtual FxInt32 GetLineSize() const;
	virtual FxInt32 GetPageSize() const;

	// these methods get/set the length of the slider poImpInt32er in pixels
	virtual void SetThumbLength(FxInt32 lenPixels);
	virtual FxInt32 GetThumbLength() const;

	void Draw( wxPaintEvent& event );

protected:
	void OnMouse( wxMouseEvent& event );

	void Initialize();

	FxInt32 ValueToCoord( FxInt32 value );
	FxInt32 CoordToValue( FxInt32 coord );

	FxInt32 _minValue;
	FxInt32 _maxValue;
	FxInt32 _currValue;

	FxInt32 _pageSize;
	FxInt32 _lineSize;

	FxInt32 _leftBorder;
	FxInt32 _rightBorder;

	FxInt32 _flags;

	WX_DECLARE_DYNAMIC_CLASS(FxSlider)
	DECLARE_EVENT_TABLE()
};

} // namespace Face

} // namespace OC3Ent

#endif
