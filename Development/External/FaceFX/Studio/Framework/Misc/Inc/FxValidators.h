//------------------------------------------------------------------------------
// This file contains all wxValidator derived validators for FaceFx
// data types.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxValidators_H__
#define FxValidators_H__

#include "FxPlatform.h"
#include "FxFaceGraphNode.h"
#include "FxStudioOptions.h"
#include "FxTimeManager.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/valtext.h"
#endif

namespace OC3Ent
{

namespace Face
{

// A validator for FxReal types.
class FxRealValidator : public wxTextValidator
{
	typedef wxTextValidator Super;
public:
	FxRealValidator( FxReal* val, const wxString& formatString = wxT("%.4f") )
		: Super(wxFILTER_NUMERIC)
	{
		_pReal = val;
		_formatString = formatString;
	}
	FxRealValidator( const FxRealValidator& other )
		: Super(wxFILTER_NUMERIC)
	{
		Super::Copy(other);
		_pReal = other._pReal;
		_formatString = other._formatString;
	}
	wxObject* Clone( void ) const
	{
		return new FxRealValidator(*this);
	}

	bool Validate( wxWindow* FxUnused(window) )
	{
		return true;
	}

	bool TransferFromWindow( void )
	{
		if( !m_validatorWindow ) return false;
		if( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
		{
			wxTextCtrl* pText = static_cast<wxTextCtrl*>(m_validatorWindow);

			if( _pReal )
			{
				if( pText->GetValue() == wxEmptyString )
				{
					*_pReal = FxInvalidValue;
				}
				else
				{
					*_pReal = FxAtof(pText->GetValue().mb_str(wxConvLibc));
				}
				return true;
			}
		}
		return false;
	}

	bool TransferToWindow( void )
	{
		if( !m_validatorWindow ) return false;
		if( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
		{
			wxTextCtrl* pText = (wxTextCtrl*)m_validatorWindow;

			if( _pReal )
			{
				pText->SetValue(wxString::Format(_formatString, *_pReal));
				return true;
			}
		}
		return false;
	}

private:
	FxReal* _pReal;
	wxString _formatString;
};

// A validator for FxString types.
class FxStringValidator : public wxTextValidator
{
	typedef wxTextValidator Super;
public:
	FxStringValidator( FxString* val )
		: Super()
	{
		_pString = val;
	}
	FxStringValidator( const FxStringValidator& other )
		: Super()
	{
		Super::Copy(other);
		_pString = other._pString;
	}
	wxObject* Clone( void ) const
	{
		return new FxStringValidator(*this);
	}

	bool Validate( wxWindow* FxUnused(window) )
	{
		return true;
	}

	bool TransferFromWindow( void )
	{
		if( !m_validatorWindow ) return false;
		if( m_validatorWindow->IsKindOf( CLASSINFO(wxTextCtrl) ) )
		{
			wxTextCtrl* pText = static_cast<wxTextCtrl*>(m_validatorWindow);

			if( _pString )
			{
				if( pText->GetValue() == wxEmptyString )
				{
					*_pString = "";
				}
				else
				{
					*_pString = FxString(pText->GetValue().mb_str(wxConvLibc));
				}
				return true;
			}
		}
		return false;
	}

	bool TransferToWindow( void )
	{
		if( !m_validatorWindow ) return false;
		if( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
		{
			wxTextCtrl* pText = (wxTextCtrl*)m_validatorWindow;

			if( _pString )
			{
				pText->SetValue(wxString::FromAscii(_pString->GetData()));
				return true;
			}
		}
		return false;
	}

private:
	FxString* _pString;
};

// A validator for FxReal types, represented to the user as a time.
class FxTimeValidator : public wxTextValidator
{
	typedef wxTextValidator Super;
public:
	FxTimeValidator( FxReal* val, FxBool overrideFormat = FxFalse )
		: Super(wxFILTER_NUMERIC)
	{
		_pReal = val;
		_overrideFormat = overrideFormat;
	}
	FxTimeValidator( const FxTimeValidator& other )
		: Super(wxFILTER_NUMERIC)
	{
		Super::Copy(other);
		_pReal = other._pReal;
		_overrideFormat = other._overrideFormat;
	}
	wxObject* Clone( void ) const
	{
		return new FxTimeValidator(*this);
	}

	bool Validate( wxWindow* FxUnused(window) )
	{
		return true;
	}

	bool TransferFromWindow( void )
	{
		if( !m_validatorWindow ) return false;
		if( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
		{
			wxTextCtrl* pText = static_cast<wxTextCtrl*>(m_validatorWindow);

			if( _pReal )
			{
				if( pText->GetValue() == wxEmptyString )
				{
					*_pReal = FxInvalidValue;
				}
				else
				{
					FxTimeGridFormat timeFormat = FxStudioOptions::GetTimeGridFormat();
					if( _overrideFormat || timeFormat == TGF_Phonemes || timeFormat == TGF_Words )
					{
						timeFormat = FxStudioOptions::GetTimelineGridFormat();
					}
					*_pReal = FxUnformatTime(pText->GetValue(), timeFormat);
				}
				return true;
			}
		}
		return false;
	}

	bool TransferToWindow( void )
	{
		if( !m_validatorWindow ) return false;
		if( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
		{
			wxTextCtrl* pText = (wxTextCtrl*)m_validatorWindow;

			if( _pReal )
			{
				FxTimeGridFormat timeFormat = FxStudioOptions::GetTimeGridFormat();
				if( _overrideFormat || timeFormat == TGF_Phonemes || timeFormat == TGF_Words )
				{
					timeFormat = FxStudioOptions::GetTimelineGridFormat();
				}
				pText->SetValue(FxFormatTime(*_pReal, timeFormat));
				return true;
			}
		}
		return false;
	}

private:
	FxReal* _pReal;
	FxBool  _overrideFormat;
};


} // namespace Face

} // namespace OC3Ent

#endif
