//------------------------------------------------------------------------------
// Definitions for elements that can be contained in a workspace.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxWorkspaceElements.h"
#include "FxMath.h"
#include "FxSessionProxy.h"
#include "FxAnimController.h"
#include "FxVM.h"

#if defined(WIN32) && defined(USE_FX_RENDERER)
	#include "FxRenderWidgetOC3.h"
#endif

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	// wx-specific includes
#endif

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// FxWorkspaceElement
//------------------------------------------------------------------------------
FxWorkspaceElement::FxWorkspaceElement()
	: _canRotate(FxFalse)
	, _pos(0.5f, 0.5f)		// Default to the middle of the workspace.
	, _scale(0.1f, 0.1f)	// Default to 1/10th workspace square.
	, _mouseAction(MA_None)
{
	SetRotation(0.0f); // Default to no rotation.
	OnTransformChanged();
}

void FxWorkspaceElement::SetWorkspaceRect(const wxRect& workspaceRect)
{
	_workspaceRect = workspaceRect;
	OnTransformChanged();
}

const wxRect& FxWorkspaceElement::GetWorkspaceRect() const
{
	return _workspaceRect;
}

const FxVec2& FxWorkspaceElement::GetPosition() const
{
	return _pos;
}

const FxVec2& FxWorkspaceElement::GetScale() const
{
	return _scale;
}

FxReal FxWorkspaceElement::GetRotation() const
{
	return _rot;
}

void FxWorkspaceElement::SetPosition(FxReal x, FxReal y, FxInt32 snap)
{
	if( snap != -1 )
	{
		FxInt32 xInt = static_cast<FxInt32>(x * 100);
		FxInt32 yInt = static_cast<FxInt32>(y * 100);
		x = static_cast<FxReal>((xInt / snap) * snap) / 100.0f;
		y = static_cast<FxReal>((yInt / snap) * snap) / 100.0f;
	}
	_pos = FxVec2(x,y);
	OnTransformChanged();
}

void FxWorkspaceElement::SetPosition(const FxVec2& pos,	FxInt32 snap)
{
	if( snap != -1 )
	{
		FxInt32 xInt = static_cast<FxInt32>(pos.x * 100);
		FxInt32 yInt = static_cast<FxInt32>(pos.y * 100);
		_pos.x = static_cast<FxReal>((xInt / snap) * snap) / 100.0f;
		_pos.y = static_cast<FxReal>((yInt / snap) * snap) / 100.0f;
	}
	else
	{
		_pos = pos;
	}
	OnTransformChanged();
}

void FxWorkspaceElement::SetScale(FxReal x, FxReal y, FxInt32 xsnap, FxInt32 ysnap)
{
	if( xsnap != -1 )
	{
		FxInt32 xInt = static_cast<FxInt32>(x * 100);
		x = static_cast<FxReal>((xInt / xsnap) * xsnap) / 100.0f;

	}
	if( ysnap != -1 )
	{
		FxInt32 yInt = static_cast<FxInt32>(y * 100);
		y = static_cast<FxReal>((yInt / ysnap) * ysnap) / 100.0f;
	}
	_scale = FxVec2(x,y);
	OnTransformChanged();
}

void FxWorkspaceElement::SetScale(const FxVec2& scale, FxInt32 xsnap, FxInt32 ysnap)
{
	if( xsnap != -1 )
	{
		FxInt32 xInt = static_cast<FxInt32>(scale.x * 100);
		_scale.x = static_cast<FxReal>((xInt / xsnap) * xsnap) / 100.0f;
	}
	else
	{
		_scale.x = scale.x;
	}
	if( ysnap != -1 )
	{
		FxInt32 yInt = static_cast<FxInt32>(scale.y * 100);
		_scale.y = static_cast<FxReal>((yInt / ysnap) * ysnap) / 100.0f;
	}
	else
	{
		_scale.y = scale.y;
	}
	OnTransformChanged();
}

void FxWorkspaceElement::SetRotation(FxReal rot, FxInt32 snap)
{
	if( snap != -1 )
	{
		rot = static_cast<FxReal>((static_cast<FxInt32>(rot) / snap) * snap);
	}

	static const FxReal PiOverOneEighty = 3.1415926535f/180.0f;
	_rot = rot;
	_sinRot = FxSin(-_rot * PiOverOneEighty);
	_cosRot = FxCos(-_rot * PiOverOneEighty);
	_sinNegRot = FxSin(_rot * PiOverOneEighty);
	_cosNegRot = FxCos(_rot * PiOverOneEighty);
	OnTransformChanged();
}

void FxWorkspaceElement::DrawOn(wxDC* pDC, FxBool editMode, FxBool selected)
{
	pDC->SetClippingRegion(_workspaceRect);

	FxInt32 cachedFunction = pDC->GetLogicalFunction();
	pDC->SetLogicalFunction(wxINVERT);
	pDC->DrawLine(_tl, _tr);
	pDC->DrawLine(_tr, _br);
	pDC->DrawLine(_br, _bl);
	pDC->DrawLine(_bl, _tl);
	pDC->SetLogicalFunction(cachedFunction);

	// Draw the control handles.
	if( editMode )
	{
		DrawControlHandles(pDC, selected);
	}

	pDC->DestroyClippingRegion();
}

FxBool FxWorkspaceElement::ProcessMouse(wxMouseEvent& event, FxInt32& needsRefresh, FxBool editMode)
{
	if( editMode )
	{
		static const FxInt32 posSnap = 2;
		static const FxInt32 scaleSnap = 2;
		static const FxInt32 rotSnap = 15;
		if( event.LeftDown() )
		{
			if( IsNearControl(_mc, event.GetPosition()) )
			{
				_mouseAction = MA_Translate;
			}
			else if( IsNearControl(_sc, event.GetPosition()) )
			{
				_mouseAction = MA_Rotate;
			}
			else if( IsNearControl(_tc, event.GetPosition()) )
			{
				_mouseAction = MA_ScaleN;
			}
			else if( IsNearControl(_bc, event.GetPosition()) )
			{
				_mouseAction = MA_ScaleS;
			}
			else if( IsNearControl(_rc, event.GetPosition()) )
			{
				_mouseAction = MA_ScaleE;
			}
			else if( IsNearControl(_lc, event.GetPosition()) )
			{
				_mouseAction = MA_ScaleW;
			}
			else if( IsNearControl(_tr, event.GetPosition()) )
			{
				_mouseAction = MA_ScaleNE;
			}
			else if( IsNearControl(_br, event.GetPosition()) )
			{
				_mouseAction = MA_ScaleSE;
			}
			else if( IsNearControl(_tl, event.GetPosition()) )
			{
				_mouseAction = MA_ScaleNW;
			}
			else if( IsNearControl(_bl, event.GetPosition()) )
			{
				_mouseAction = MA_ScaleSW;
			}

			if( _mouseAction >= MA_ScaleN )
			{
				_cachePos = _pos;
				_cacheScale = _scale;
			}

			return FxTrue;
		}

		else if( event.Dragging() && event.LeftIsDown() )
		{
			wxPoint eventPos = event.GetPosition();
			if( _mouseAction == MA_Translate )
			{
				// Set the new control position.
				FxReal xPos = static_cast<FxReal>(eventPos.x - _workspaceRect.GetLeft())/static_cast<FxReal>(_workspaceRect.GetWidth());
				FxReal yPos = static_cast<FxReal>(eventPos.y - _workspaceRect.GetTop()) /static_cast<FxReal>(_workspaceRect.GetHeight());
				SetPosition(xPos, yPos, event.ControlDown() ? posSnap : -1);
				needsRefresh |= WRF_Workspaces;
			}
			else if( _mouseAction == MA_Rotate )
			{
				FxReal xSign = _scale.x < 0.0f ? -1.0f : 1.0f;
				FxReal ySign = _scale.y < 0.0f ? -1.0f : 1.0f;
				FxReal a = (eventPos.x - _mc.x) * xSign;
				FxReal b = (eventPos.y - _mc.y) * ySign;
				FxReal newAngle = FxAtan(a / b) * (180.0f/3.1415926535f);
				newAngle *= xSign * ySign;
				if( b >= 0.f )
				{
					newAngle += 180.0f;
				}

				SetRotation(newAngle, event.ControlDown() ? rotSnap : -1);
				needsRefresh |= WRF_Workspaces;
			}
			else if( _mouseAction == MA_ScaleN )
			{
				FxVec2 mousePos = WorkspaceToElement(eventPos, _cachePos);

				FxReal newYScale = (_cachePos.y + 0.5 * _cacheScale.y) - mousePos.y;
				SetScale(_scale.x, newYScale, -1, event.ControlDown() ? scaleSnap : -1);

				FxReal xPosModifier = _sinRot * 0.5f * (_scale.y - _cacheScale.y);
				FxReal yPosModifier = _cosRot * 0.5f * (_scale.y - _cacheScale.y);

				_pos.x = _cachePos.x + xPosModifier;
				_pos.y = _cachePos.y - yPosModifier;

				OnTransformChanged();
				needsRefresh |= WRF_Workspaces;
			}
			else if( _mouseAction == MA_ScaleS )
			{
				FxVec2 mousePos = WorkspaceToElement(eventPos, _cachePos);

				FxReal newYScale = mousePos.y - (_cachePos.y - 0.5 * _cacheScale.y);
				SetScale(_scale.x, newYScale, -1, event.ControlDown() ? scaleSnap : -1);

				FxReal xPosModifier = _sinRot * 0.5f * (_scale.y - _cacheScale.y);
				FxReal yPosModifier = _cosRot * 0.5f * (_scale.y - _cacheScale.y);

				_pos.x = _cachePos.x - xPosModifier;
				_pos.y = _cachePos.y + yPosModifier;

				OnTransformChanged();
				needsRefresh |= WRF_Workspaces;
			}
			else if( _mouseAction == MA_ScaleW )
			{
				FxVec2 mousePos = WorkspaceToElement(eventPos, _cachePos);

				FxReal newXScale = (_cachePos.x + 0.5 * _cacheScale.x) - mousePos.x;
				SetScale(newXScale, _scale.y, event.ControlDown() ? scaleSnap : -1, -1);

				FxReal xPosModifier = _cosRot * 0.5f * (_scale.x - _cacheScale.x);
				FxReal yPosModifier = _sinRot * 0.5f * (_scale.x - _cacheScale.x);

				_pos.x = _cachePos.x - xPosModifier;
				_pos.y = _cachePos.y - yPosModifier;

				OnTransformChanged();
				needsRefresh |= WRF_Workspaces;
			}
			else if( _mouseAction == MA_ScaleE )
			{
				FxVec2 mousePos = WorkspaceToElement(eventPos, _cachePos);

				FxReal newXScale = mousePos.x - (_cachePos.x - 0.5 * _cacheScale.x);
				SetScale(newXScale, _scale.y, event.ControlDown() ? scaleSnap : -1, -1);

				FxReal xPosModifier = _cosRot * 0.5f * (_scale.x - _cacheScale.x);
				FxReal yPosModifier = _sinRot * 0.5f * (_scale.x - _cacheScale.x);

				_pos.x = _cachePos.x + xPosModifier;
				_pos.y = _cachePos.y + yPosModifier;

				OnTransformChanged();
				needsRefresh |= WRF_Workspaces;
			}
			else if( _mouseAction == MA_ScaleNE )
			{
				FxVec2 mousePos = WorkspaceToElement(eventPos, _cachePos);

				FxReal enewXScale = mousePos.x - (_cachePos.x - 0.5 * _cacheScale.x);
				FxReal nnewYScale = (_cachePos.y + 0.5 * _cacheScale.y) - mousePos.y;
				FxInt32 snap = event.ControlDown() ? scaleSnap : -1;
				SetScale(enewXScale, nnewYScale, snap, snap);

				FxReal exPosModifier = _cosRot * 0.5f * (_scale.x - _cacheScale.x);
				FxReal eyPosModifier = _sinRot * 0.5f * (_scale.x - _cacheScale.x);
				FxReal nxPosModifier = _sinRot * 0.5f * (_scale.y - _cacheScale.y);
				FxReal nyPosModifier = _cosRot * 0.5f * (_scale.y - _cacheScale.y);

				_pos.x = _cachePos.x + exPosModifier + nxPosModifier;
				_pos.y = _cachePos.y + eyPosModifier - nyPosModifier;

				OnTransformChanged();
				needsRefresh |= WRF_Workspaces;
			}
			else if( _mouseAction == MA_ScaleSE )
			{
				FxVec2 mousePos = WorkspaceToElement(eventPos, _cachePos);

				FxReal enewXScale = mousePos.x - (_cachePos.x - 0.5 * _cacheScale.x);
				FxReal snewYScale = mousePos.y - (_cachePos.y - 0.5 * _cacheScale.y);
				FxInt32 snap = event.ControlDown() ? scaleSnap : -1;
				SetScale(enewXScale, snewYScale, snap, snap);

				FxReal exPosModifier = _cosRot * 0.5f * (_scale.x - _cacheScale.x);
				FxReal eyPosModifier = _sinRot * 0.5f * (_scale.x - _cacheScale.x);
				FxReal sxPosModifier = _sinRot * 0.5f * (_scale.y - _cacheScale.y);
				FxReal syPosModifier = _cosRot * 0.5f * (_scale.y - _cacheScale.y);

				_pos.x = _cachePos.x + exPosModifier - sxPosModifier;
				_pos.y = _cachePos.y + eyPosModifier + syPosModifier;

				OnTransformChanged();
				needsRefresh |= WRF_Workspaces;
			}
			else if( _mouseAction == MA_ScaleNW )
			{
				FxVec2 mousePos = WorkspaceToElement(eventPos, _cachePos);

				FxReal wnewXScale = (_cachePos.x + 0.5 * _cacheScale.x) - mousePos.x;
				FxReal nnewYScale = (_cachePos.y + 0.5 * _cacheScale.y) - mousePos.y;
				FxInt32 snap = event.ControlDown() ? scaleSnap : -1;
				SetScale(wnewXScale, nnewYScale, snap, snap);

				FxReal wxPosModifier = _cosRot * 0.5f * (_scale.x - _cacheScale.x);
				FxReal wyPosModifier = _sinRot * 0.5f * (_scale.x - _cacheScale.x);
				FxReal nxPosModifier = _sinRot * 0.5f * (_scale.y - _cacheScale.y);
				FxReal nyPosModifier = _cosRot * 0.5f * (_scale.y - _cacheScale.y);

				_pos.x = _cachePos.x - wxPosModifier + nxPosModifier;
				_pos.y = _cachePos.y - wyPosModifier - nyPosModifier;

				OnTransformChanged();
				needsRefresh |= WRF_Workspaces;
			}
			else if( _mouseAction == MA_ScaleSW )
			{
				FxVec2 mousePos = WorkspaceToElement(eventPos, _cachePos);

				FxReal wnewXScale = (_cachePos.x + 0.5 * _cacheScale.x) - mousePos.x;
				FxReal snewYScale = mousePos.y - (_cachePos.y - 0.5 * _cacheScale.y);
				FxInt32 snap = event.ControlDown() ? scaleSnap : -1;
				SetScale(wnewXScale, snewYScale, snap, snap);

				FxReal wxPosModifier = _cosRot * 0.5f * (_scale.x - _cacheScale.x);
				FxReal wyPosModifier = _sinRot * 0.5f * (_scale.x - _cacheScale.x);
				FxReal sxPosModifier = _sinRot * 0.5f * (_scale.y - _cacheScale.y);
				FxReal syPosModifier = _cosRot * 0.5f * (_scale.y - _cacheScale.y);

				_pos.x = _cachePos.x - wxPosModifier - sxPosModifier;
				_pos.y = _cachePos.y - wyPosModifier + syPosModifier;

				OnTransformChanged();
				needsRefresh |= WRF_Workspaces;
			}

			return FxTrue;
		}

		else if( event.LeftUp() )
		{
			if( _mouseAction == MA_Translate || _mouseAction == MA_Rotate ||
				_mouseAction == MA_ScaleN || _mouseAction == MA_ScaleS ||
				_mouseAction == MA_ScaleE || _mouseAction == MA_ScaleW || 
				_mouseAction == MA_ScaleNE || _mouseAction == MA_ScaleSE ||
				_mouseAction == MA_ScaleNW || _mouseAction == MA_ScaleSW )
			{
				OnTransformChanged();
				needsRefresh |= WRF_Workspaces;
				_mouseAction = MA_None;
			}
			return FxFalse;
		}
	}
	return FxFalse;
}

FxWorkspaceElement::HitResult FxWorkspaceElement::HitTest(const wxPoint& pt, FxBool editMode)
{
	if( editMode )
	{
		if( IsNearControl(_tc, pt) )
		{
			return HR_ScaleN;
		}
		else if( IsNearControl(_bc, pt) )
		{
			return HR_ScaleS;
		}
		else if( IsNearControl(_rc, pt) )
		{
			return HR_ScaleE;
		}
		else if( IsNearControl(_lc, pt) )
		{
			return HR_ScaleW;
		}
		else if( IsNearControl(_tr, pt) )
		{
			return HR_ScaleNE;
		}
		else if( IsNearControl(_br, pt) )
		{
			return HR_ScaleSE;
		}
		else if( IsNearControl(_tl, pt) )
		{
			return HR_ScaleNW;
		}
		else if( IsNearControl(_bl, pt) )
		{
			return HR_ScaleSW;
		}
		else if( _canRotate && IsNearControl(_sc, pt) )
		{
			return HR_Rotate;
		}
		else if( IsNearControl(_mc, pt) )
		{
			return HR_Translate;
		}
	}

	FxVec2 elem = WorkspaceToElement(pt);
	FxReal halfXScale = 0.5 * _scale.x;
	FxReal halfYScale = 0.5 * _scale.y;
	FxReal minX = FxMin(-halfXScale + _pos.x, halfXScale + _pos.x);
	FxReal maxX = FxMax(-halfXScale + _pos.x, halfXScale + _pos.x);
	FxReal minY = FxMin(-halfYScale + _pos.y, halfYScale + _pos.y);
	FxReal maxY = FxMax(-halfYScale + _pos.y, halfYScale + _pos.y);
	if( minX <= elem.x && elem.x <= maxX &&	minY <= elem.y && elem.y <= maxY )
	{
		return HR_Passed;
	}
	return HR_Failed;
}

void FxWorkspaceElement::OnTransformChanged()
{
	FxReal halfXScale = 0.5 * _scale.x;
	FxReal halfYScale = 0.5 * _scale.y;

	_mc = ElementToWorkspace(FxVec2(0.0f,		  0.0f));

	_tl = ElementToWorkspace(FxVec2(-halfXScale, -halfYScale));
	_tr = ElementToWorkspace(FxVec2( halfXScale, -halfYScale));
	_br = ElementToWorkspace(FxVec2( halfXScale,  halfYScale));
	_bl = ElementToWorkspace(FxVec2(-halfXScale,  halfYScale));

	_lc = ElementToWorkspace(FxVec2(-halfXScale,  0.0f));
	_rc = ElementToWorkspace(FxVec2( halfXScale,  0.0f));
	_tc = ElementToWorkspace(FxVec2(0.0f,        -halfYScale));
	_bc = ElementToWorkspace(FxVec2(0.0f,         halfYScale));

	FxVec2 scLine(_tc.x - _mc.x, _tc.y - _mc.y);
	scLine.Normalize();
	_sc = wxPoint(_tc.x + (scLine.x * 15), _tc.y + (scLine.y * 15));
}

wxPoint FxWorkspaceElement::GetCenter()
{
	return _mc;
}

// Drawing
void FxWorkspaceElement::DrawControlHandles(wxDC* pDC, FxBool FxUnused(selected))
{
	pDC->SetClippingRegion(_workspaceRect);

	FxInt32 cachedFunction = pDC->GetLogicalFunction();
	
	pDC->SetPen(*wxGREEN_PEN);
	pDC->SetBrush(*wxGREEN_BRUSH);

	pDC->SetLogicalFunction(wxCOPY);

	pDC->DrawCircle(_lc, 2);
	pDC->DrawCircle(_rc, 2);
	pDC->DrawCircle(_tc, 2);
	pDC->DrawCircle(_bc, 2);
	pDC->DrawCircle(_tl, 2);
	pDC->DrawCircle(_bl, 2);
	pDC->DrawCircle(_tr, 2);
	pDC->DrawCircle(_br, 2);
	if( _canRotate )
	{
		pDC->DrawCircle(_sc, 3);
	}
	pDC->DrawCircle(_mc, 4);

	pDC->SetLogicalFunction(cachedFunction);

	pDC->DestroyClippingRegion();
}

// Returns true if the mouse is over a control point.
FxBool FxWorkspaceElement::IsNearControl(const wxPoint& controlPoint, const wxPoint mousePoint)
{
	return controlPoint.x - 3 <= mousePoint.x && mousePoint.x <= controlPoint.x + 3 &&
		   controlPoint.y - 3 <= mousePoint.y && mousePoint.y <= controlPoint.y + 3;
}

wxPoint FxWorkspaceElement::ElementToWorkspace(const FxVec2& elemCoord)
{
	FxVec2 rotatedCoord(
		_cosRot * elemCoord.x - _sinRot * elemCoord.y,
		_sinRot * elemCoord.x + _cosRot * elemCoord.y);
	rotatedCoord += _pos;
	return wxPoint(
		rotatedCoord.x * _workspaceRect.GetWidth() + _workspaceRect.GetLeft(),
		rotatedCoord.y * _workspaceRect.GetHeight() + _workspaceRect.GetTop());
}

FxVec2 FxWorkspaceElement::WorkspaceToElement(const wxPoint& workspaceCoord)
{
	FxVec2 elementCoord(
		static_cast<FxReal>(workspaceCoord.x - _workspaceRect.GetLeft()) / static_cast<FxReal>(_workspaceRect.GetWidth()),
		static_cast<FxReal>(workspaceCoord.y - _workspaceRect.GetTop())  / static_cast<FxReal>(_workspaceRect.GetHeight()));
	elementCoord -= _pos;
	FxVec2 rotatedCoord(
		_cosNegRot * elementCoord.x - _sinNegRot * elementCoord.y,
		_sinNegRot * elementCoord.x + _cosNegRot * elementCoord.y);
	rotatedCoord += _pos;
	return rotatedCoord;
}

FxVec2 FxWorkspaceElement::WorkspaceToElement(const wxPoint& workspaceCoord, const FxVec2& centre)
{
	FxVec2 elementCoord(
						static_cast<FxReal>(workspaceCoord.x - _workspaceRect.GetLeft()) / static_cast<FxReal>(_workspaceRect.GetWidth()),
						static_cast<FxReal>(workspaceCoord.y - _workspaceRect.GetTop())  / static_cast<FxReal>(_workspaceRect.GetHeight()));
	elementCoord -= centre;
	FxVec2 rotatedCoord(
					_cosNegRot * elementCoord.x - _sinNegRot * elementCoord.y,
					_sinNegRot * elementCoord.x + _cosNegRot * elementCoord.y);
	rotatedCoord+= centre;
	return rotatedCoord;
}

//------------------------------------------------------------------------------
// FxWorkspaceElementSlider
//------------------------------------------------------------------------------
FxWorkspaceElementSlider::FxWorkspaceElementSlider()
	: _faceGraph(NULL)
	, _nodeLinkX(NULL)
	, _nodeLinkY(NULL)
	, _anim(NULL)
	, _animLinkX(NULL)
	, _animLinkY(NULL)
	, _curveIndexX(FxInvalidIndex)
	, _curveIndexY(FxInvalidIndex)
	, _animController(NULL)
	, _originalOffsetXValue(0.0f)
	, _originalOffsetYValue(0.0f)
	, _originalXValue(0.0f)
	, _originalYValue(0.0f)
	, _cachedXValue(FxFalse)
	, _cachedYValue(FxFalse)
	, _mode(WM_Combiner)
	, _keyExistsX(FxFalse)
	, _keyExistsY(FxFalse)
	, _valX(0.75f)
	, _valY(0.25f)
	, _autoKey(FxFalse)
	, _mousing(FxFalse)
	, _highlight(FxFalse)
	, _typeMode(FxFalse)
	, _typingX(FxFalse)
	, _bufferPos(0)
{
	_canRotate = FxTrue;
}

const FxString& FxWorkspaceElementSlider::GetNodeNameXAxis() const
{
	return _nodeNameX;
}

const FxString& FxWorkspaceElementSlider::GetNodeNameYAxis() const
{
	return _nodeNameY;
}

void FxWorkspaceElementSlider::SetNodeNameXAxis(const FxString& nodeNameX)
{
	_nodeNameX = nodeNameX;
}

void FxWorkspaceElementSlider::SetNodeNameYAxis(const FxString& nodeNameY)
{
	_nodeNameY = nodeNameY;
}

FxString FxWorkspaceElementSlider::BuildKeyXCommand()
{
	if( !_cachedXValue && _nodeLinkX )
	{
		// Cache the values.
		FxReal currentValue = (_nodeLinkX->GetMax() - _nodeLinkX->GetMin()) * _valX + _nodeLinkX->GetMin();
		if( _animController && _animLinkX )
		{
			// Get the correct original value for the node.
			if( !_cachedXValue )
			{
				_originalOffsetXValue = _animLinkX->EvaluateAt(_currentTime);
				_cachedXValue = FxTrue;
			}

			if( _nodeLinkX->GetNumInputLinks() > 0 )
			{
				FxCompiledFaceGraph* pCG = NULL;
				FxSessionProxy::GetCompiledFaceGraph(&pCG);
				FxAssert(pCG);
				FxSize nodeIndex = pCG->FindNodeIndex(_nodeLinkX->GetName());
				FxAssert(FxInvalidIndex != nodeIndex);
				pCG->nodes[nodeIndex].trackValue = _animLinkX->EvaluateAt(_currentTime);
				FxSessionProxy::UpdateActorInstance();
				FxSessionProxy::GetRegister(_nodeLinkX->GetName(), _originalXValue);
			}
			else
			{
				_originalXValue = _animLinkX->EvaluateAt(_currentTime);
				FxSessionProxy::UpdateActorInstance();
			}

			FxSessionProxy::SetRegister(_nodeLinkX->GetName(), currentValue - _originalXValue, RO_Add);
		}
	}

	wxString xCommand; 
	if( _nodeLinkX && _animLinkX )
	{
		// Check for virtual curve.
		FxReal value;
		if( _nodeLinkX->GetNumInputLinks() > 0 )
		{
			FxReal originalRegisterValue = 0.0f;
			FxSessionProxy::GetOriginalRegisterValue(_nodeLinkX->GetName(), originalRegisterValue);
			value = _originalOffsetXValue + originalRegisterValue;
		}
		else
		{
			FxSessionProxy::GetRegister(_nodeLinkX->GetName(), value);
		}

		// If there is not already a key at this time actually set a key
		// with the appropriate value.
		_curveIndexX = _animController->GetCurveIndex(_animLinkX->GetName());
		FxSize originalKeyIndex = _animController->FindKeyAtTime(_curveIndexX, _currentTime);
		if( FxInvalidIndex == originalKeyIndex )
		{
			// Insert key.
			xCommand = wxString::Format(wxT("key -curveIndex %d -insert -time %f -value %f -noClearSelection"),
				_curveIndexX, _currentTime, value);
		}
		else
		{
			// There was a key at this time already so edit it.
			xCommand = wxString::Format(wxT("key -curveIndex %d -edit -keyIndex %d -value %f -noClearSelection"),
				_curveIndexX, originalKeyIndex, value);
		}
		FxSessionProxy::SetRegister(_nodeLinkX->GetName(), 0.0f, RO_None);
	}
	return FxString(xCommand.mb_str(wxConvLibc));
}

FxString FxWorkspaceElementSlider::BuildKeyYCommand()
{
	if( !_cachedYValue && _nodeLinkY )
	{
		// Cache the values.
		FxReal currentValue = (_nodeLinkY->GetMax() - _nodeLinkY->GetMin()) * _valY + _nodeLinkY->GetMin();
		if( _animController && _animLinkY )
		{
			// Get the correct original value for the node.
			if( !_cachedYValue )
			{
				_originalOffsetYValue = _animLinkY->EvaluateAt(_currentTime);
				_cachedYValue = FxTrue;
			}

			if( _nodeLinkY->GetNumInputLinks() > 0 )
			{
				FxCompiledFaceGraph* pCG = NULL;
				FxSessionProxy::GetCompiledFaceGraph(&pCG);
				FxAssert(pCG);
				FxSize nodeIndex = pCG->FindNodeIndex(_nodeLinkY->GetName());
				FxAssert(FxInvalidIndex != nodeIndex);
				pCG->nodes[nodeIndex].trackValue = _animLinkY->EvaluateAt(_currentTime);
				FxSessionProxy::UpdateActorInstance();
				FxSessionProxy::GetRegister(_nodeLinkY->GetName(), _originalXValue);
			}
			else
			{
				_originalYValue = _animLinkY->EvaluateAt(_currentTime);
				FxSessionProxy::UpdateActorInstance();
			}

			FxSessionProxy::SetRegister(_nodeLinkY->GetName(), currentValue - _originalYValue, RO_Add);
		}
	}

	// Handle the y-axis input.
	wxString yCommand;
	if( _nodeLinkY && _animLinkY )
	{
		// Check for virtual curve.
		FxReal value;
		if( _nodeLinkY->GetNumInputLinks() > 0 )
		{
			FxReal originalRegisterValue = 0.0f;
			FxSessionProxy::GetOriginalRegisterValue(_nodeLinkY->GetName(), originalRegisterValue);
			value = _originalOffsetYValue + originalRegisterValue;
		}
		else
		{
			FxSessionProxy::GetRegister(_nodeLinkY->GetName(), value);
		}

		// If there is not already a key at this time actually set a key
		// with the appropriate value.
		_curveIndexY = _animController->GetCurveIndex(_animLinkY->GetName());
		FxSize originalKeyIndex = _animController->FindKeyAtTime(_curveIndexY, _currentTime);
		if( FxInvalidIndex == originalKeyIndex )
		{
			// Insert key.
			yCommand = wxString::Format(wxT("key -curveIndex %d -insert -time %f -value %f -noClearSelection"),
				_curveIndexY, _currentTime, value);
		}
		else
		{
			// There was a key at this time already so edit it.
			yCommand = wxString::Format(wxT("key -curveIndex %d -edit -keyIndex %d -value %f -noClearSelection"),
				_curveIndexY, originalKeyIndex, value);
		}
		FxSessionProxy::SetRegister(_nodeLinkY->GetName(), 0.0f, RO_None);
	}
	return FxString(yCommand.mb_str(wxConvLibc));
}

void FxWorkspaceElementSlider::LinkToNodes(FxFaceGraph* pFaceGraph)
{
	_faceGraph = pFaceGraph;
	if( pFaceGraph )
	{
		_nodeLinkX = _faceGraph->FindNode(_nodeNameX.GetData());
		_nodeLinkY = _faceGraph->FindNode(_nodeNameY.GetData());
	}
}

void FxWorkspaceElementSlider::LinkToAnim(FxAnim* pAnim)
{
	_anim = pAnim;
	if( _anim )
	{
		FxSize xIndex = _anim->FindAnimCurve(_nodeNameX.GetData());
		FxSize yIndex = _anim->FindAnimCurve(_nodeNameY.GetData());
		_animLinkX = xIndex == FxInvalidIndex ? NULL : &_anim->GetAnimCurveM(xIndex);
		_animLinkY = yIndex == FxInvalidIndex ? NULL : &_anim->GetAnimCurveM(yIndex);

		if( _animController )
		{
			_curveIndexX = _animLinkX ? _animController->GetCurveIndex(_animLinkX->GetName()) : FxInvalidIndex;
			_curveIndexY = _animLinkY ? _animController->GetCurveIndex(_animLinkY->GetName()) : FxInvalidIndex;
		}
	}
	else
	{
		_animLinkX = NULL;
		_animLinkY = NULL;
		_curveIndexX = FxInvalidIndex;
		_curveIndexY = FxInvalidIndex;
	}
}

void FxWorkspaceElementSlider::LinkToAnimController(FxAnimController* pAnimController)
{
	_animController = pAnimController;
}

void FxWorkspaceElementSlider::SetCurrentTime(FxReal currentTime)
{
	_cachedXValue = FxFalse;
	_cachedYValue = FxFalse;
	_currentTime  = currentTime;

	UpdateSlider();
}

void FxWorkspaceElementSlider::DrawOn(wxDC* pDC, FxBool editMode, FxBool selected)
{
	pDC->SetClippingRegion(_workspaceRect);

	FxInt32 cachedFunction = pDC->GetLogicalFunction();
	pDC->SetLogicalFunction(wxINVERT);

	// Calculate the position of the handle in workspace coordinates.
	FxReal distXFromPos = 0.0f;
	if( !_nodeNameX.IsEmpty() )
	{
		if( _valX < 0.5f )
		{
			distXFromPos = -(0.5f - _valX);
		}
		else
		{
			distXFromPos = _valX - 0.5f;
		}
	}

	FxReal distYFromPos = 0.0f;
	if( !_nodeNameY.IsEmpty() )
	{
		FxReal tempValY = 1.0f - _valY;
		if( tempValY < 0.5f )
		{
			distYFromPos = -(0.5f - tempValY);
		}
		else
		{
			distYFromPos = tempValY - 0.5f;
		}
	}

	distXFromPos *= _scale.x;
	distYFromPos *= _scale.y;

	wxPoint handleCentre  = ElementToWorkspace(FxVec2(
		distXFromPos, distYFromPos));


	// Draw the caption.
	if( _highlight || selected )
	{
		wxString xCaption(wxString::Format(wxT("x: %s"), wxString::FromAscii(_nodeNameX.GetData())));
		wxString yCaption(wxString::Format(wxT("y: %s"), wxString::FromAscii(_nodeNameY.GetData())));
		FxInt32 cursorX = 0;
		if( !editMode && _nodeLinkX )
		{
			xCaption += wxT(" = ");
			if( _typeMode && _typingX )
			{
				FxInt32 x, trash;
				pDC->GetTextExtent(xCaption, &x, &trash);
				cursorX += x;
				pDC->GetTextExtent(_buffer.Mid(0, _bufferPos), &x, &trash);
				cursorX += x;
				xCaption += _buffer;
			}
			else
			{
				FxReal currentValue = (_nodeLinkX->GetMax() - _nodeLinkX->GetMin()) * _valX + _nodeLinkX->GetMin();
				xCaption += wxString::Format(wxT("%0.3f"), currentValue);
			}
		}
		if( !editMode && _nodeLinkY )
		{
			yCaption += wxT(" = ");
			if( _typeMode && !_typingX )
			{
				FxInt32 x, trash;
				pDC->GetTextExtent(yCaption, &x, &trash);
				cursorX += x;
				pDC->GetTextExtent(_buffer.Mid(0, _bufferPos), &x, &trash);
				cursorX += x;
				yCaption += _buffer;
			}
			else
			{
				FxReal currentValue = (_nodeLinkY->GetMax() - _nodeLinkY->GetMin()) * _valY + _nodeLinkY->GetMin();
				yCaption += wxString::Format(wxT("%0.3f"), currentValue);
			}
		}
		FxInt32 minX = FxMin(_bl.x, FxMin(_br.x, FxMin(_tl.x, _tr.x)));
		FxInt32 minY = FxMin(_bl.y, FxMin(_br.y, FxMin(_tl.y, _tr.y)));
		minY -= 4;
		FxInt32 xExtent,yExtent;
		FxInt32 maxXExtent = FX_INT32_MIN;
		if( !_nodeNameX.IsEmpty() && !_nodeNameY.IsEmpty() )
		{
			pDC->GetTextExtent(xCaption, &xExtent, &yExtent);
			maxXExtent = FxMax(maxXExtent, xExtent);
			pDC->GetTextExtent(yCaption, &xExtent, &yExtent);
			maxXExtent = FxMax(maxXExtent, xExtent);
			pDC->DrawText(xCaption, minX + 1, minY - yExtent * 2 - 2);
			pDC->DrawText(yCaption, minX + 1, minY - yExtent);
			if( _typeMode )
			{
				FxInt32 yPos = minY - yExtent * (_typingX ? 2 : 1) - (_typingX ? 2 : 0);
				pDC->DrawLine(cursorX + minX, yPos + yExtent, cursorX + minX, yPos);
			}
		}
		else if( !_nodeNameX.IsEmpty() )
		{
			pDC->GetTextExtent(xCaption, &xExtent, &yExtent);
			maxXExtent = xExtent;
			pDC->DrawText(xCaption, minX + 1, minY - yExtent);
			if( _typeMode && _typingX )
			{
					pDC->DrawLine(cursorX + minX, minY - yExtent, cursorX + minX, minY);
			}
		}
		else if( !_nodeNameY.IsEmpty() )
		{
			pDC->GetTextExtent(yCaption, &xExtent, &yExtent);
			maxXExtent = xExtent;
			pDC->DrawText(yCaption, minX + 1, minY - yExtent);
			if( _typeMode && !_typingX )
			{
				pDC->DrawLine(cursorX + minX, minY - yExtent, cursorX + minX, minY);
			}
		}

		FxReal margin = 20.f;
		if( !(0.f - margin <= _rot && _rot <= 0.f + margin ||
			90.f - margin <= _rot && _rot <= 90.f + margin ||
			180.f - margin <= _rot && _rot <= 180.f + margin ||
			270.f - margin <= _rot && _rot <= 270.f + margin ||
			-90.f + margin <= _rot && _rot <= -90.f - margin) )
		{
			pDC->SetPen(wxPen(pDC->GetTextForeground(), 1, wxDOT));
			pDC->SetLogicalFunction(wxCOPY);
			pDC->DrawLine(wxPoint(minX, minY), wxPoint(minX + maxXExtent, minY));
			pDC->DrawLine(wxPoint(minX + maxXExtent, minY), handleCentre);
			pDC->SetPen(*wxBLACK_PEN);
			pDC->SetLogicalFunction(wxINVERT);
		}
	}

	if( editMode && !_nodeNameX.IsEmpty() ||
		!editMode && _nodeLinkX )
	{
		if( _mode == WM_Animation )
		{
			pDC->SetLogicalFunction(wxCOPY);
			if( _keyExistsX && _animLinkX )
			{
				pDC->SetPen(*wxRED_PEN);
			}
			else
			{
				if( _cachedXValue || _cachedYValue )
				{
					pDC->SetPen(wxPen(wxColour(255,255,0)));
				}
				else
				{
					pDC->SetPen(*wxGREEN_PEN);
				}
			}
		}
		wxPoint handleXTopEnd = ElementToWorkspace(FxVec2(
			distXFromPos, -(0.5 * _scale.y)));
		handleXTopEnd.y += 1; // Remove overdraw.
		wxPoint handleXBotEnd = ElementToWorkspace(FxVec2(
			distXFromPos, 0.5 * _scale.y));
		pDC->DrawLine(handleXTopEnd, handleXBotEnd);
	}
	if( editMode && !_nodeNameY.IsEmpty() ||
		!editMode && _nodeLinkY )
	{
		if( _mode == WM_Animation && _animLinkY )
		{
			pDC->SetLogicalFunction(wxCOPY);
			if( _keyExistsY )
			{
				pDC->SetPen(*wxRED_PEN);
			}
			else
			{
				if( _cachedXValue || _cachedYValue )
				{
					pDC->SetPen(wxPen(wxColour(255,255,0)));
				}
				else
				{
					pDC->SetPen(*wxGREEN_PEN);
				}
			}
		}
		wxPoint handleYLeftEnd  = ElementToWorkspace(FxVec2(
			-(0.5 * _scale.x), distYFromPos));
		handleYLeftEnd.x += 1; // Remove overdraw.
		wxPoint handleYRightEnd = ElementToWorkspace(FxVec2(
			0.5 * _scale.x, distYFromPos));
		pDC->DrawLine(handleYLeftEnd, handleYRightEnd);
	}

	pDC->SetLogicalFunction(wxINVERT);
	pDC->SetBrush(*wxTRANSPARENT_BRUSH);
	pDC->SetPen(*wxBLACK_PEN);
	//@todo scale circle?
	pDC->DrawCircle(handleCentre.x, handleCentre.y, 5);

	if( selected & !editMode )
	{
		pDC->SetPen(wxPen(*wxBLACK, 3));
	}

	if( (_nodeLinkY && !FxRealEquality(_valY, 1.0f)) ||
		!_nodeLinkY || selected )
	{
		pDC->DrawLine(_tl, _tr);
	}
	if( (_nodeLinkX && !FxRealEquality(_valX, 1.0f)) ||
		!_nodeLinkX || selected )
	{
		pDC->DrawLine(_tr, _br);
	}
	if( (_nodeLinkY && !FxRealEquality(_valY, 0.0f)) ||
		!_nodeLinkY || selected )
	{
		pDC->DrawLine(_br, _bl);
	}
	if( (_nodeLinkX && !FxRealEquality(_valX, 0.0f)) ||
		!_nodeLinkX || selected )
	{
		pDC->DrawLine(_bl, _tl);
	}

	pDC->SetPen(*wxBLACK_PEN);

	pDC->SetLogicalFunction(cachedFunction);

	pDC->DestroyClippingRegion();

	if( editMode && selected )
	{
		DrawControlHandles(pDC, selected);
	}
}

// Process a mouse movement.
FxBool FxWorkspaceElementSlider::ProcessMouse(wxMouseEvent& event, FxInt32& needsRefresh, FxBool editMode)
{
	if( editMode )
	{
		// If we're in edit mode, allow the base class to handle all the nasty
		// scaling, rotating and transforming code.
		return FxWorkspaceElement::ProcessMouse(event, needsRefresh, editMode);
	}
	else
	{
		if( event.LeftDown() )
		{
			_mousing = FxTrue;
		}

		// Otherwise, we just need to compute a new position for the slider 
		// x and y axes.
		if( _mousing && (event.LeftDown() || event.Dragging() && event.LeftIsDown()) )
		{
			FxVec2 mousePos = WorkspaceToElement(event.GetPosition());
			FxReal valX = (mousePos.x - (_pos.x - 0.5 * _scale.x)) / _scale.x;
			FxReal valY = 1.0 - ((mousePos.y - (_pos.y - 0.5 * _scale.y)) / _scale.y);
			valX = FxClamp(0.0f, valX, 1.0f);
			valY = FxClamp(0.0f, valY, 1.0f);

			SetNodeValues(valX, valY);

			needsRefresh = WRF_All;
		}

		if( _mousing && event.LeftUp() )
		{
			if( _autoKey )
			{
				OnSetKey();
			}
			_mousing = FxFalse;
			needsRefresh = WRF_All;
		}

		if( event.RightUp() || event.LeftDClick())
		{
			// Reset the slider to "zero".
			FxReal valX = 0.0f;
			FxReal valY = 0.0f;
			if( _nodeLinkX )
			{
				FxReal nodeDefaultValue = FxClamp(_nodeLinkX->GetMin(), 0.0f, _nodeLinkX->GetMax());
				valX = (nodeDefaultValue - _nodeLinkX->GetMin()) / ( _nodeLinkX->GetMax() - _nodeLinkX->GetMin());
			}
			if( _nodeLinkY )
			{
				FxReal nodeDefaultValue = FxClamp(_nodeLinkY->GetMin(), 0.0f, _nodeLinkY->GetMax());
				valY = (nodeDefaultValue - _nodeLinkY->GetMin()) / ( _nodeLinkY->GetMax() - _nodeLinkY->GetMin());
			}

			SetNodeValues(valX, valY);
			if( _autoKey )
			{
				OnSetKey();
			}

			needsRefresh = WRF_All;
		}

		if( event.MiddleDown() )
		{
			OnSetKey();
			UpdateSlider();

			needsRefresh = WRF_All;
		}

		if( event.LeftDown() || event.Dragging() )
		{
			return FxTrue;
		}
		return FxFalse;
	}
}

// Process a key event.
FxBool FxWorkspaceElementSlider::ProcessKeyDown(wxKeyEvent& event, FxInt32& needsRefresh, FxBool editMode)
{
	FxBool retval = FxFalse;
	if( !editMode )
	{
		if( !_typeMode )
		{
			_buffer = wxEmptyString;
			_bufferPos = 0;
		}
		
		FxInt32 keyCode = event.GetKeyCode();
		if( !_typeMode && (keyCode == 'X' || keyCode == 'Y') )
		{
			// Check if we start with an x, y.
			if( keyCode == 'X' )
			{
				_typeMode = FxTrue;
				_typingX = FxTrue;
				retval = FxTrue;
			}
			else if( keyCode == 'Y' )
			{
				_typeMode = FxTrue;
				_typingX = FxFalse;
				retval = FxTrue;
			}
		}
		else if( (wxT('0') <= keyCode && keyCode <= wxT('9')) ||
			(WXK_NUMPAD0 <= keyCode && keyCode <= WXK_NUMPAD9) ||
			WXK_SUBTRACT == keyCode || WXK_DECIMAL == keyCode ||
			WXK_NUMPAD_SUBTRACT == keyCode || WXK_NUMPAD_DECIMAL == keyCode ||
			wxT('-') == keyCode || wxT('.') == keyCode )
		{
			if( !_typeMode )
			{
				_typeMode = FxTrue;
				_typingX = _nodeLinkX != NULL;
			}

			if( WXK_SUBTRACT == keyCode || WXK_NUMPAD_SUBTRACT == keyCode || wxT('-') == keyCode )
			{
				// Only insert a negate if there isn't one and we're at the 
				// beginning of the buffer.
				if( _bufferPos == 0 && _buffer.Find('-') == -1 )
				{
					_buffer.insert(_bufferPos, 1, wxT('-'));
				}
			}
			else if( WXK_DECIMAL == keyCode || WXK_NUMPAD_DECIMAL == keyCode|| wxT('.') == keyCode )
			{
				// Only insert a decimal if there isn't one.
				if( _buffer.Find('.') == -1 )
				{
					_buffer.insert(_bufferPos, 1, wxT('.'));
				}
			}
			else // typed a number.
			{
				if( WXK_NUMPAD0 <= keyCode && keyCode <= WXK_NUMPAD9 )
				{
					_buffer.insert(_bufferPos, 1, keyCode - WXK_NUMPAD0 + '0');
				}
				else
				{
					// Easy - append the key code.
					_buffer.insert(_bufferPos, 1, keyCode);
				}
			}

			++_bufferPos;
			retval = FxTrue;
		}
		else if( _typeMode )
		{
			if( keyCode == WXK_RETURN || keyCode == WXK_NUMPAD_ENTER )
			{
				// Finalize the edit.
				ApplyBufferChanges();
				if( _autoKey )
				{
					OnSetKey();
				}

				_typeMode = FxFalse;
				retval = FxTrue;
			}
			else if( keyCode == WXK_ESCAPE )
			{
				// cancel type mode.
				_typeMode = FxFalse;
				retval = FxTrue;
			}	
			else if( keyCode == WXK_DELETE )
			{
				// remove the next thing in the buffer
				_buffer.Remove(_bufferPos, 1);
				retval = FxTrue;
			}
			else if( keyCode == WXK_BACK )
			{
				// remove the prev thing in the buffer
				if( _bufferPos != 0 )
				{
					_buffer.Remove(_bufferPos-1, 1);
					--_bufferPos;
					retval = FxTrue;
				}
			}
			else if( keyCode == WXK_LEFT )
			{
				// move to the left.
				_bufferPos = FxClamp<FxInt32>(0, _bufferPos - 1, _buffer.Length());
				retval = FxTrue;
			}
			else if( keyCode == WXK_RIGHT )
			{
				// move to the right.
				_bufferPos = FxClamp<FxInt32>(0, _bufferPos + 1, _buffer.Length());
				retval = FxTrue;
			}
		}

		// If we didn't process the key yet, check for other input.
		if( !retval )
		{
			// Process setting a key.
			if( keyCode == WXK_RETURN || keyCode == WXK_NUMPAD_ENTER )
			{
				OnSetKey();
				UpdateSlider();

				needsRefresh = WRF_All;
			}

			// Process resetting the slider
			if( keyCode == WXK_SPACE )
			{
				// Reset the slider to "zero".
				FxReal valX = 0.0f;
				FxReal valY = 0.0f;
				if( _nodeLinkX )
				{
					FxReal nodeDefaultValue = FxClamp(_nodeLinkX->GetMin(), 0.0f, _nodeLinkX->GetMax());
					valX = (nodeDefaultValue - _nodeLinkX->GetMin()) / ( _nodeLinkX->GetMax() - _nodeLinkX->GetMin());
				}
				if( _nodeLinkY )
				{
					FxReal nodeDefaultValue = FxClamp(_nodeLinkY->GetMin(), 0.0f, _nodeLinkY->GetMax());
					valY = (nodeDefaultValue - _nodeLinkY->GetMin()) / ( _nodeLinkY->GetMax() - _nodeLinkY->GetMin());
				}

				SetNodeValues(valX, valY);
				if( _autoKey )
				{
					OnSetKey();
				}

				needsRefresh = WRF_All;
			}

			// Process key-controlled slider movement. 
			// For more details about what the heck is going on here, check
			// FogBugz case 36.
			if( keyCode == WXK_LEFT || keyCode == WXK_RIGHT ||
				keyCode == WXK_DOWN || keyCode == WXK_UP )
			{
				// Ensure rotation is in [0, 360]
				FxInt32 rot = FxNearestInteger(_rot);
				if( rot < 0 )
				{
					rot %= 360;
					rot += 360;
				}
				else if( rot > 360 )
				{
					rot %= 360;
				}

				// How many "clicks" on a slider.  Lower number equals more "clicks"
				// Control down resets the slider to a given side
				// Shift down moves the slider with 10 intermediate clicks
				// No modifier moves the slider with 100 intermediate clicks.
				FxReal  incrementPrecisionHoriz = event.ControlDown() ? 1.0f : event.ShiftDown() ? 0.1f : 0.01f;
				FxReal  incrementPrecisionVert  = incrementPrecisionHoriz;

				// Using the rotation of the slider as a guide, choose which 
				// values to modify for a given arrow key.
				FxReal* pHorizVal  = NULL;
				FxReal* pVertVal   = NULL;
				FxBool  flipParams = FxFalse;
				if( rot <= 45.f || rot >= 315.f)
				{
					// 0 rot
					pHorizVal = &_valX;
					pVertVal  = &_valY;
				}
				else if( rot > 45.f && rot < 135.f )
				{
					// 90 rot
					pHorizVal  = &_valY;
					pVertVal   = &_valX;
					flipParams = FxTrue;
					incrementPrecisionHoriz *= -1.0f;
				}
				else if( rot >= 135.f && rot < 225.f )
				{
					// 180 rot
					pHorizVal = &_valX;
					pVertVal  = &_valY;
					incrementPrecisionHoriz *= -1.0f;
					incrementPrecisionVert  *= -1.0f;
				}
				else if( rot >= 225.f && rot < 315.f )
				{
					// 270 rot
					pHorizVal  = &_valY;
					pVertVal   = &_valX;
					flipParams = FxTrue;
					incrementPrecisionVert *= -1.0f;
				}

				// Take into account the scale.
				if( _scale.x < 0 )
				{
					if( !flipParams )
					{
						incrementPrecisionHoriz *= -1.0f;
					}
					else
					{
						incrementPrecisionVert *= -1.0f;
					}
				}
				if( _scale.y < 0 )
				{
					if( !flipParams )
					{
						incrementPrecisionVert *= -1.0f;
					}
					else
					{
						incrementPrecisionHoriz *= -1.0f;
					}
				}

				// Now use the key code to modify a value that was chosen earlier.
				if( keyCode == WXK_LEFT )
				{
					*pHorizVal = FxClamp(0.0f, *pHorizVal - incrementPrecisionHoriz, 1.0f);
				}
				else if(keyCode == WXK_RIGHT )
				{
					*pHorizVal = FxClamp(0.0f, *pHorizVal + incrementPrecisionHoriz, 1.0f);
				}
				else if( keyCode == WXK_DOWN )
				{
					*pVertVal = FxClamp(0.0f, *pVertVal - incrementPrecisionVert, 1.0f);
				}
				else if( keyCode == WXK_UP )
				{
					*pVertVal = FxClamp(0.0f, *pVertVal + incrementPrecisionVert, 1.0f);
				}

				// Finally, some code paths require flipping the parameters to
				// the following function.
				if( flipParams )
				{
					SetNodeValues(*pVertVal, *pHorizVal);
				}
				else
				{
					SetNodeValues(*pHorizVal, *pVertVal);
				}

				// If we're auto-keying, do so.
				if( _autoKey )
				{
					OnSetKey();
				}
				retval = FxTrue;
			}
		}
	}

	if( retval )
	{
		needsRefresh = WRF_All;
	}
	return retval;
}

void FxWorkspaceElementSlider::UpdateSlider()
{
	if( _nodeLinkX )
	{
		FxReal currentValue = 0.0f;
		if( WM_Mapping == _mode || WM_Combiner == _mode )
		{
			FxSessionProxy::GetOriginalRegisterValue(_nodeLinkX->GetName(), currentValue);
		}
		else if( WM_Animation == _mode )
		{
			if( _animController )
			{
				_animController->Evaluate(_currentTime);
			}
			FxSessionProxy::GetRegister(_nodeLinkX->GetName(), currentValue);

			// Check to see if there is an actual key at the current time.
			if( _animController )
			{
				//@todo this needs to be turned back on.
				// If the curve is owned by the analysis, the user cannot edit keys so
				// Studio does not present them to the user.
				//if( !_animController->IsCurveOwnedByAnalysis(_curveIndex) )
				//{
				FxSize originalKeyIndex = _animController->FindKeyAtTime(_curveIndexX, _currentTime);
				_keyExistsX = originalKeyIndex != FxInvalidIndex;
				//}
			}
		}

		_valX = (currentValue - _nodeLinkX->GetMin()) / (_nodeLinkX->GetMax() - _nodeLinkX->GetMin());
	}

	if( _nodeLinkY )
	{
		FxReal currentValue = 0.0f;
		if( WM_Mapping == _mode || WM_Combiner == _mode )
		{
			FxSessionProxy::GetOriginalRegisterValue(_nodeLinkY->GetName(), currentValue);
		}
		else if( WM_Animation == _mode )
		{
			if( _animController )
			{
				_animController->Evaluate(_currentTime);
			}
			FxSessionProxy::GetRegister(_nodeLinkY->GetName(), currentValue);

			// Check to see if there is an actual key at the current time.
			if( _animController )
			{
				//@todo this needs to be turned back on.
				// If the curve is owned by the analysis, the user cannot edit keys so
				// Studio does not present them to the user.
				//if( !_animController->IsCurveOwnedByAnalysis(_curveIndex) )
				//{
				FxSize originalKeyIndex = _animController->FindKeyAtTime(_curveIndexY, _currentTime);
				_keyExistsY = originalKeyIndex != FxInvalidIndex;
				//}
			}
		}

		_valY = (currentValue - _nodeLinkY->GetMin()) / (_nodeLinkY->GetMax() - _nodeLinkY->GetMin());
	}
}

void FxWorkspaceElementSlider::SetMode(FxWorkspaceMode mode)
{
	_mode = mode;
	UpdateSlider();
}

void FxWorkspaceElementSlider::SetOption(FxBool autoKey)
{
	_autoKey = autoKey;
}

void FxWorkspaceElementSlider::SetHighlight(FxBool highlight)
{
	if( _highlight && !highlight )
	{
		//@todo cancel the editing.
		_buffer = wxEmptyString;
		_bufferPos = 0;
		_typeMode = FxFalse;
	}

	_highlight = highlight;	
}

FxBool FxWorkspaceElementSlider::GetHighlight()
{
	return _highlight;
}

void FxWorkspaceElementSlider::SetNodeValues(FxReal valX, FxReal valY)
{
	if( WM_Animation == _mode )
	{
		EnsureCurvesExist();
	}

	if( _nodeLinkX )
	{
		_valX = valX;
		// Map the [0,1] value into the [min,max] range.
		FxReal currentValue = (_nodeLinkX->GetMax() - _nodeLinkX->GetMin()) * _valX + _nodeLinkX->GetMin();

		if( WM_Mapping == _mode || WM_Combiner == _mode )
		{
			FxSessionProxy::SetRegister(_nodeLinkX->GetName(), currentValue, RO_Add);
			FxSessionProxy::UpdateActorInstance();
		}
		else if( WM_Animation == _mode )
		{
			if( _animController && _animLinkX )
			{
				FxSessionProxy::SetRegister(_nodeLinkX->GetName(), 0.0f, RO_None);

				if( _animController )
				{
					_animController->Evaluate(_currentTime);
				}
				// Get the correct original value for the node.
				if( !_cachedXValue )
				{
					_originalOffsetXValue = _animLinkX->EvaluateAt(_currentTime);
					_cachedXValue = FxTrue;
				}

				FxSessionProxy::GetRegister(_nodeLinkX->GetName(), _originalXValue);
				FxSessionProxy::SetRegister(_nodeLinkX->GetName(), currentValue - _originalXValue, RO_Add);
			}
		}
	}

	if( _nodeLinkY )
	{
		_valY = valY;
		// Map the [0,1] value into the [min,max] range.
		FxReal currentValue = (_nodeLinkY->GetMax() - _nodeLinkY->GetMin()) * _valY + _nodeLinkY->GetMin();

		if( WM_Mapping == _mode || WM_Combiner == _mode )
		{
			FxSessionProxy::SetRegister(_nodeLinkY->GetName(), currentValue, RO_Add);
			FxSessionProxy::UpdateActorInstance();
		}
		else if( WM_Animation == _mode )
		{
			if( _animController && _animLinkY )
			{
				FxSessionProxy::SetRegister(_nodeLinkY->GetName(), 0.0f, RO_None);

				if( _animController )
				{
					_animController->Evaluate(_currentTime);
				}
				// Get the correct original value for the node.
				if( !_cachedYValue )
				{
					_originalOffsetYValue = _animLinkY->EvaluateAt(_currentTime);
					_cachedYValue = FxTrue;
				}

				FxSessionProxy::GetRegister(_nodeLinkY->GetName(), _originalYValue);
				FxSessionProxy::SetRegister(_nodeLinkY->GetName(), currentValue - _originalYValue, RO_Add);
			}
		}
	}
}

// Set a key value, when in animation mode.
void FxWorkspaceElementSlider::OnSetKey()
{
	if( _animController && WM_Animation == _mode )
	{
		// If the values haven't been cached, the user hasn't moved the slider
		// yet, so force a value cache by simulating a slider movement without
		// actually changing the current value in the slider.
		if( !_cachedXValue && !_cachedYValue )
		{
			SetNodeValues(_valX, _valY);
		}

		wxString xCommand;
		wxString yCommand;

		// Handle the x-axis input.
		if( _nodeLinkX && _animLinkX )
		{
			// Check for virtual curve.
			FxReal value;
			if( _nodeLinkX->GetNumInputLinks() > 0 )
			{
				FxReal originalRegisterValue = 0.0f;
				FxSessionProxy::GetOriginalRegisterValue(_nodeLinkX->GetName(), originalRegisterValue);
				value = _originalOffsetXValue + originalRegisterValue;
			}
			else
			{
				FxSessionProxy::GetRegister(_nodeLinkX->GetName(), value);
			}

			// If there is not already a key at this time actually set a key
			// with the appropriate value.
			_curveIndexX = _animController->GetCurveIndex(_animLinkX->GetName());
			FxSize originalKeyIndex = _animController->FindKeyAtTime(_curveIndexX, _currentTime);
			if( FxInvalidIndex == originalKeyIndex )
			{
				// Insert key.
				xCommand = wxString::Format(wxT("key -curveIndex %d -insert -time %f -value %f -noClearSelection"),
					_curveIndexX, _currentTime, value);
			}
			else
			{
				// There was a key at this time already so edit it.
				xCommand = wxString::Format(wxT("key -curveIndex %d -edit -keyIndex %d -value %f -noClearSelection"),
					_curveIndexX, originalKeyIndex, value);
			}
		}

		// Handle the y-axis input.
		if( _nodeLinkY && _animLinkY )
		{
			// Check for virtual curve.
			FxReal value;
			if( _nodeLinkY->GetNumInputLinks() > 0 )
			{
				FxReal originalRegisterValue = 0.0f;
				FxSessionProxy::GetOriginalRegisterValue(_nodeLinkY->GetName(), originalRegisterValue);
				value = _originalOffsetYValue + originalRegisterValue;
			}
			else
			{
				FxSessionProxy::GetRegister(_nodeLinkY->GetName(), value);
			}

			// If there is not already a key at this time actually set a key
			// with the appropriate value.
			_curveIndexY = _animController->GetCurveIndex(_animLinkY->GetName());
			FxSize originalKeyIndex = _animController->FindKeyAtTime(_curveIndexY, _currentTime);
			if( FxInvalidIndex == originalKeyIndex )
			{
				// Insert key.
				yCommand = wxString::Format(wxT("key -curveIndex %d -insert -time %f -value %f -noClearSelection"),
					_curveIndexY, _currentTime, value);
			}
			else
			{
				// There was a key at this time already so edit it.
				yCommand = wxString::Format(wxT("key -curveIndex %d -edit -keyIndex %d -value %f -noClearSelection"),
					_curveIndexY, originalKeyIndex, value);
			}
		}


		FxBool needToEndBatch = FxFalse;
		if( !xCommand.IsEmpty() && !yCommand.IsEmpty() )
		{
			FxVM::ExecuteCommand("batch");
			needToEndBatch = FxTrue;
		}
		if( !xCommand.IsEmpty() )
		{
			FxVM::ExecuteCommand(FxString(xCommand.mb_str(wxConvLibc)));
		}
		if( !yCommand.IsEmpty() )
		{
			FxVM::ExecuteCommand(FxString(yCommand.mb_str(wxConvLibc)));
		}
		if( needToEndBatch )
		{
			FxVM::ExecuteCommand("execBatch -editedcurves");
		}

		// This should be done in all cases.
		_cachedXValue = FxFalse;
		_cachedYValue = FxFalse;
		// Once the keys have been set we can disable the registers.
		if( _nodeLinkX )
		{
			FxSessionProxy::SetRegister(_nodeLinkX->GetName(), 0.0f, RO_None);
		}
		if( _nodeLinkY )
		{
			FxSessionProxy::SetRegister(_nodeLinkY->GetName(), 0.0f, RO_None);
		}
		_animController->Evaluate(_currentTime);
	}
}

// Add the curves, if necessary.
void FxWorkspaceElementSlider::EnsureCurvesExist()
{
	if( _anim && ((_nodeLinkX && !_animLinkX) || (_nodeLinkY && !_animLinkY)) )
	{
		wxBusyCursor wait;
		wxString xCommand;
		wxString yCommand;
		FxString selAnimGroup;
		FxSessionProxy::GetSelectedAnimGroup(selAnimGroup);

		if( _nodeLinkX && !_animLinkX )
		{
			xCommand = wxString::Format(wxT("curve -group \"%s\" -anim \"%s\" -add -name \"%s\" -owner \"user\""),
				wxString(selAnimGroup.GetData(), wxConvLibc), wxString(_anim->GetNameAsCstr(), wxConvLibc),
				wxString(_nodeNameX.GetData(), wxConvLibc) );
		}
		if( _nodeLinkY && !_animLinkY )
		{
			yCommand = wxString::Format(wxT("curve -group \"%s\" -anim \"%s\" -add -name \"%s\" -owner \"user\""),
				wxString(selAnimGroup.GetData(), wxConvLibc), wxString(_anim->GetNameAsCstr(), wxConvLibc),
				wxString(_nodeNameY.GetData(), wxConvLibc) );
		}

		if( !xCommand.IsEmpty() && !yCommand.IsEmpty() )
		{
			FxVM::ExecuteCommand("batch");
		}
		if( !xCommand.IsEmpty() )
		{
			FxVM::ExecuteCommand(FxString(xCommand.mb_str(wxConvLibc)));
		}
		if( !yCommand.IsEmpty() )
		{
			FxVM::ExecuteCommand(FxString(yCommand.mb_str(wxConvLibc)));
		}
		if( !xCommand.IsEmpty() && !yCommand.IsEmpty() )
		{
			FxVM::ExecuteCommand("execBatch -addedcurves");
		}

		LinkToAnim(_anim);
	}
}

void FxWorkspaceElementSlider::ApplyBufferChanges()
{
	FxReal val = FxAtof(_buffer.mb_str(wxConvLibc));

	if( _typingX )
	{
		if( _nodeLinkX )
		{
			_valX = (val - _nodeLinkX->GetMin()) / (_nodeLinkX->GetMax() - _nodeLinkX->GetMin());
			_valX = FxClamp(0.0f, _valX, 1.0f);
		}
	}
	else
	{
		if( _nodeLinkY )
		{
			_valY = (val - _nodeLinkY->GetMin()) / (_nodeLinkY->GetMax() - _nodeLinkY->GetMin());
			_valY = FxClamp(0.0f, _valY, 1.0f);
		}
	}

	SetNodeValues(_valX, _valY);
}

//------------------------------------------------------------------------------
// FxWorkspaceElementBackground
//------------------------------------------------------------------------------
FxWorkspaceElementBackground::FxWorkspaceElementBackground()
	: _imageData(NULL)
	, _imageDataSize(0)
	, _bitmap(wxNullBitmap)
{
}

// Construct from an image.
FxWorkspaceElementBackground::FxWorkspaceElementBackground(const wxImage& image)
	: _imageData(NULL)
	, _imageDataSize(0)
	, _bitmap(wxNullBitmap)
{
	SetImage(image);
}

// Copy ctor.
FxWorkspaceElementBackground::FxWorkspaceElementBackground(const FxWorkspaceElementBackground& other)
	: FxWorkspaceElement(other)
	, _imageData(NULL)
	, _imageDataSize(0)
	, _bitmap(other._bitmap)
	, _original(other._original.Copy())
{
}

// Assignment.
FxWorkspaceElementBackground& FxWorkspaceElementBackground::operator=(const FxWorkspaceElementBackground& other)
{
	FxWorkspaceElement::operator=(other);
	_imageData = NULL;
	_imageDataSize = 0;

	_bitmap = other._bitmap;
	_original = other._original.Copy();

	return *this;
}

// Destructor.
FxWorkspaceElementBackground::~FxWorkspaceElementBackground()
{
	if( _imageData )
	{
		free(_imageData);
		_imageData = NULL;
	}
}

// Set the background image.
void FxWorkspaceElementBackground::SetImage(const wxImage& image)
{
	_original = image.Copy();
}

void FxWorkspaceElementBackground::DrawOn(wxDC* pDC, FxBool editMode, FxBool selected)
{
	pDC->SetClippingRegion(_workspaceRect);

	wxMemoryDC bmpDC;
	FxInt32 x = _workspaceRect.GetWidth() * _pos.x + _workspaceRect.GetLeft() - 0.5 * _bitmap.GetWidth();
	FxInt32 y = _workspaceRect.GetHeight() * _pos.y + _workspaceRect.GetTop() - 0.5 * _bitmap.GetHeight();
	bmpDC.SelectObject(_bitmap);
	pDC->Blit(x, y, _bitmap.GetWidth(), _bitmap.GetHeight(), &bmpDC, 0, 0);
	bmpDC.SelectObject(wxNullBitmap);

	pDC->DestroyClippingRegion();

	if( editMode && selected )
	{
		DrawControlHandles(pDC, selected);
	}
}

void FxWorkspaceElementBackground::OnTransformChanged()
{
	// Call the default implementation to recompute the bounds.
	FxWorkspaceElement::OnTransformChanged();

	FxInt32 width  = _workspaceRect.GetWidth() * _scale.x;
	FxInt32 height = _workspaceRect.GetHeight() * _scale.y;
	if( _bitmap.GetWidth() != width ||
		_bitmap.GetHeight() != height )
	{
		_bitmap = wxBitmap(_original.Scale(width, height));
	}
}

//------------------------------------------------------------------------------
// FxWorkspaceElementViewport
//------------------------------------------------------------------------------
FxWorkspaceElementViewport::FxWorkspaceElementViewport()
	: _cameraName(FxName::NullName)
{
#if defined(WIN32) && defined(USE_FX_RENDERER)
	_pOffscreenViewport = new FxRenderWidgetOC3OffscreenViewport();
#else
	//@todo Implement your own derived offscreen viewport here.
	_pOffscreenViewport = NULL;
#endif
}

FxWorkspaceElementViewport::FxWorkspaceElementViewport(const FxWorkspaceElementViewport& other)
{
	_pos = other._pos;
	_rot = other._rot;
	_scale = other._scale;
#if defined(WIN32) && defined(USE_FX_RENDERER)
	_pOffscreenViewport = new FxRenderWidgetOC3OffscreenViewport();
#else
	//@todo Implement your own derived offscreen viewport here.
	_pOffscreenViewport = NULL;
#endif
	_cameraName = other._cameraName;
}

FxWorkspaceElementViewport& FxWorkspaceElementViewport::operator=(const FxWorkspaceElementViewport& other)
{
	if( _pOffscreenViewport )
	{
		delete _pOffscreenViewport;
		_pOffscreenViewport = NULL;
	}
	_pos = other._pos;
	_rot = other._rot;
	_scale = other._scale;
#if defined(WIN32) && defined(USE_FX_RENDERER)
	_pOffscreenViewport = new FxRenderWidgetOC3OffscreenViewport();
#else
	//@todo Implement your own derived offscreen viewport here.
	_pOffscreenViewport = NULL;
#endif
	_cameraName = other._cameraName;
	return *this;
}

FxWorkspaceElementViewport::~FxWorkspaceElementViewport()
{
	delete _pOffscreenViewport;
	_pOffscreenViewport = NULL;
}

void FxWorkspaceElementViewport::DrawOn(wxDC* pDC, FxBool editMode, FxBool selected)
{
	pDC->SetClippingRegion(_workspaceRect);
	if( _pOffscreenViewport )
	{
		_pOffscreenViewport->Render(_cameraName);
		const FxByte* pImage = _pOffscreenViewport->GetImage();
		if( pImage )
		{
			FxInt32 w, h;
			_pOffscreenViewport->GetRenderTargetSize(w, h);
			wxImage image(w, h, const_cast<FxByte*>(pImage), true);
			wxBitmap imageBitmap(image, 24);
			wxMemoryDC bitmapDC;
			bitmapDC.SelectObject(imageBitmap);
			FxInt32 x = _workspaceRect.GetWidth() * _pos.x + _workspaceRect.GetLeft() - 0.5 * imageBitmap.GetWidth();
			FxInt32 y = _workspaceRect.GetHeight() * _pos.y + _workspaceRect.GetTop() - 0.5 * imageBitmap.GetHeight();
			pDC->Blit(wxPoint(x, y), bitmapDC.GetSize(), &bitmapDC, bitmapDC.GetLogicalOrigin());
		}
	}
	pDC->DestroyClippingRegion();

	if( editMode && selected )
	{
		DrawControlHandles(pDC, selected);
	}
}

void FxWorkspaceElementViewport::OnTransformChanged()
{
	FxWorkspaceElement::OnTransformChanged();

	if( _pOffscreenViewport )
	{
		FxInt32 width  = _workspaceRect.GetWidth() * _scale.x;
		FxInt32 height = _workspaceRect.GetHeight() * _scale.y;

		if( width != 0 && height != 0 )
		{
			_pOffscreenViewport->SetRenderTargetSize(width, height);
		}
	}
}

const FxName& FxWorkspaceElementViewport::GetCameraName() const
{
	return _cameraName;
}

void FxWorkspaceElementViewport::SetCameraName(const FxName& cameraName)
{
	_cameraName = cameraName;
	
}

} // namespace Face

} // namespace OC3Ent
