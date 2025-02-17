//------------------------------------------------------------------------------
// This class is responsible for managing the time range for all the 
// time-synchronized windows in the system.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxTimeManager.h"
#include "FxConsole.h"
#include "FxMath.h"
#include "FxPhonWordList.h"

namespace OC3Ent
{

namespace Face
{

FxTimeManager* FxTimeManager::_pInst = NULL;
FxBool FxTimeManager::_destroyed = FxFalse;
FxPhonWordList* FxTimeManager::_pPhonWordList = NULL;

static const FxReal FxMinimumTimeSpan = 0.01f;
static const FxReal FxMaximumTimeSpan = 1000.0f;

void FxTimeSubscriber::VerifyPendingTimeChange( FxReal& FxUnused(min), FxReal& FxUnused(max) )
{
}

FxTimeSubscriberInfo::FxTimeSubscriberInfo( FxTimeSubscriber* sub, FxInt32 pri )
	: pSub( sub )
	, priority( pri )
{
}

FxBool FxTimeSubscriberInfo::operator<( const FxTimeSubscriberInfo& other )
{
	return priority < other.priority;
}

FxBool FxTimeSubscriberGroup::RegisterSubscriber( FxTimeSubscriber* pSubscriber )
{
	// Ensure the subscriber is not already in the system
	for( SubscriberList::Iterator i = _subscribers.Begin(); 
		 i != _subscribers.End(); ++i )
	{
		if( pSubscriber == (*i).pSub ) return FxFalse;
	}

	_subscribers.PushBack( FxTimeSubscriberInfo( pSubscriber, pSubscriber->GetPaintPriority() ) );
	_subscribers.Sort();

	return FxTrue;
}

FxBool FxTimeSubscriberGroup::UnregisterSubscriber( FxTimeSubscriber* pSubscriber )
{
	for( SubscriberList::Iterator i = _subscribers.Begin();
		i != _subscribers.End(); ++i )
	{
		if( pSubscriber == (*i).pSub )
		{
			_subscribers.Remove( i );
			return FxTrue;
		}
	}
	return FxFalse;
}

void FxTimeSubscriberGroup::NotifySubscribers()
{
	for( SubscriberList::Iterator i = _subscribers.Begin(); 
		i != _subscribers.End(); ++i )
	{
		if( (*i).pSub )
		{
			(*i).pSub->OnNotifyTimeChange();
		}
	}
}

void FxTimeSubscriberGroup::VerifyPendingTimeChange( FxReal& minimum, FxReal& maximum )
{
	for( SubscriberList::Iterator i = _subscribers.Begin(); 
		i != _subscribers.End(); ++i )
	{
		if( (*i).pSub ) 
		{
			(*i).pSub->VerifyPendingTimeChange(minimum, maximum);
		}
	}
}

FxBool FxTimeSubscriberGroup::RequestTimeChange( FxReal minimum, FxReal maximum )
{
	if( maximum - minimum > FxMinimumTimeSpan && maximum - minimum < FxMaximumTimeSpan )
	{
		VerifyPendingTimeChange(minimum, maximum);
		_minTime = minimum;
		_maxTime = maximum;
		NotifySubscribers();
		return FxTrue;
	}
	return FxFalse;
}

void FxTimeSubscriberGroup::GetTimes( FxReal& minimum, FxReal& maximum )
{
	minimum = _minTime;
	maximum = _maxTime;
}

FxReal FxTimeSubscriberGroup::GetMinimumTime()
{
	FxReal minTime = FX_REAL_MAX;
	for( SubscriberList::Iterator i = _subscribers.Begin(); 
		i != _subscribers.End(); ++i )
	{
		minTime = FxMin( minTime, (*i).pSub->GetMinimumTime() );
	}
	if( FX_REAL_MAX == minTime )
	{
		minTime = 0.0f;
	}
	return minTime;
}

FxReal FxTimeSubscriberGroup::GetMaximumTime()
{
	FxReal maxTime = static_cast<FxReal>(FX_INT32_MIN);
	for( SubscriberList::Iterator i = _subscribers.Begin(); 
		i != _subscribers.End(); ++i )
	{
		maxTime = FxMax( maxTime, (*i).pSub->GetMaximumTime() );
	}
	if( static_cast<FxReal>(FX_INT32_MIN) == maxTime )
	{
		maxTime = 1.0f;
	}
	return maxTime;
}

FxTimeManager* FxTimeManager::Instance()
{
	if( !_pInst )
	{
		_pInst = new FxTimeManager();
	}
	return _pInst;
}

void FxTimeManager::Startup( void )
{
	_destroyed = FxFalse;
	Instance();
}

void FxTimeManager::Shutdown( void )
{
	if( _pInst )
	{
		delete _pInst;
		_pInst = NULL;
		_destroyed = FxTrue;
	}
}

FxInt32 FxTimeManager::CreateGroup()
{
	if( !_destroyed )
	{
		return Instance()->_groups.Length();
	}
	return 0;
}

FxBool FxTimeManager::RegisterSubscriber( FxTimeSubscriber* pSubscriber, FxInt32 group  )
{
	if( !_destroyed && pSubscriber )
	{
		FxTimeManager* pInst = Instance();
		
		// Make sure we have enough room in the group array
		if( pInst->_groups.Length() <= static_cast<FxSize>(group) )
		{
			pInst->_groups.Insert( FxTimeSubscriberGroup(), group );
		}
		return pInst->_groups[group].RegisterSubscriber( pSubscriber );
	}
	return FxFalse;
}

FxBool FxTimeManager::UnregisterSubscriber( FxTimeSubscriber* pSubscriber, FxInt32 group )
{
	if( !_destroyed )
	{
		return Instance()->_groups[group].UnregisterSubscriber( pSubscriber );
	}
	return FxFalse;
}

FxBool FxTimeManager::RequestTimeChange( FxReal minimum, FxReal maximum, FxInt32 group )
{
	if( !_destroyed )
	{
		// Make sure we have enough room in the group array
		if( Instance()->_groups.Length() <= static_cast<FxSize>(group) )
		{
			Instance()->_groups.Insert( FxTimeSubscriberGroup(), group );
		}

		return Instance()->_groups[group].RequestTimeChange( minimum, maximum );
	}
	return FxFalse;
}

void FxTimeManager::GetTimes( FxReal& minimum, FxReal& maximum, FxInt32 group )
{
	if( !_destroyed )
	{
		Instance()->_groups[group].GetTimes( minimum, maximum );
	}
}

FxReal FxTimeManager::GetMinimumTime( FxInt32 group )
{
	if( !_destroyed )
	{
		return Instance()->_groups[group].GetMinimumTime();
	}
	return 0.0f;
}

FxReal FxTimeManager::GetMaximumTime( FxInt32 group )
{
	if( !_destroyed )
	{
		return Instance()->_groups[group].GetMaximumTime();
	}
	return 0.0f;
}

FxReal FxTimeManager::CoordToTime( FxInt32 coord, const wxRect& rect, FxInt32 group )
{
	if( !_destroyed )
	{
		FxReal minTime, maxTime;
		Instance()->_groups[group].GetTimes( minTime, maxTime );

		return ( static_cast<FxReal>( coord - rect.GetLeft() ) / 
				static_cast<FxReal>( rect.GetWidth() ) ) * 
				( maxTime - minTime ) + minTime;
	}
	return 0.0f;
}

FxInt32 FxTimeManager::TimeToCoord( FxReal time, const wxRect& rect, FxInt32 group )
{
	if( !_destroyed )
	{
		FxReal minTime, maxTime;
		Instance()->_groups[group].GetTimes( minTime, maxTime );

		return ( ( time - minTime ) / ( maxTime - minTime ) ) * rect.GetWidth() + rect.GetLeft();
	}
	return 0;
}

/// Returns the array of grid info to display.
void FxTimeManager::GetGridInfos( FxArray<FxTimeGridInfo>& gridInfos, const wxRect& rect, FxInt32 group, FxTimeGridFormat gridFormat, FxReal minTimeOverride, FxReal maxTimeOverride )
{
	gridInfos.Clear();
	if( !_destroyed )
	{
		// If the user didn't override what format to use, take it from the current options.
		if( gridFormat == TGF_Invalid )
		{
			gridFormat = FxStudioOptions::GetTimeGridFormat();
		}

		FxReal minTime, maxTime;
		Instance()->_groups[group].GetTimes( minTime, maxTime );
		if( minTimeOverride != FxInvalidValue && maxTimeOverride != FxInvalidValue)
		{
			minTime = minTimeOverride;
			maxTime = maxTimeOverride;
		}
		if( minTime != FxInvalidValue && maxTime != FxInvalidValue )
		{
			if( gridFormat != TGF_Phonemes && gridFormat != TGF_Words )
			{
				FxReal frameRate = FxStudioOptions::GetFrameRate();

				FxReal  minGridSpacing = 0.001f;
				if( gridFormat == TGF_Milliseconds )
				{
					minGridSpacing = 1.f;
					minTime *= 1000.f;
					maxTime *= 1000.f;
				}
				else if( gridFormat == TGF_Frames )
				{
					minGridSpacing = 1.f;
					minTime *= frameRate;
					maxTime *= frameRate;
				}

				FxReal pixelsPerTime  = static_cast<FxReal>(rect.GetWidth()) / (maxTime-minTime);
				FxInt32 minPixelsPerTimeGrid = 35;

				FxReal increment = minGridSpacing;
				FxInt32 num = 0;
				while( increment * pixelsPerTime < minPixelsPerTimeGrid )
				{
					FxReal spacing = 0.0f;
					if(num & 0x1)
					{
						spacing = FxPow( 10.f, 0.5f*((FxReal)(num-1)) + 1.f );
					}
					else
					{
						spacing = 0.5f * FxPow( 10.f, 0.5f*((FxReal)(num)) + 1.f );
					}
					increment = minGridSpacing * spacing;
					num++;
				}
				
				wxString formatString(wxT("%s"));
				if( gridFormat == TGF_Seconds )
				{
					formatString = num < 2 ? wxT("%3.3f") : wxT("%3.2f");
				}
				else if( gridFormat == TGF_Milliseconds || gridFormat == TGF_Frames )
				{
					formatString = wxT("%.0f");
				}
				num = FxFloor(minTime/increment);

				gridInfos.Reserve(rect.GetWidth() / minPixelsPerTimeGrid + 1);

				FxReal time = 0.0f;
				while( (time = num * increment) < maxTime )
				{
					wxString label = wxString::Format(formatString, time);
					FxReal outTime = time;
					if( gridFormat == TGF_Milliseconds )
					{
						outTime *= 0.001f;
					}
					else if( gridFormat == TGF_Frames )
					{
						outTime /= frameRate;
					}
					gridInfos.PushBack(FxTimeGridInfo(outTime, label));
					num++;
				}
			}
			else if( gridFormat == TGF_Phonemes && _pPhonWordList )
			{
				for( FxSize i = 0; i < _pPhonWordList->GetNumPhonemes(); ++i )
				{
					FxReal phonTime = _pPhonWordList->GetPhonemeStartTime(i);
					if( minTime <= phonTime && phonTime < maxTime )
					{
						const FxPhonExtendedInfo& phonInfo = FxGetPhonemeInfo(_pPhonWordList->GetPhonemeEnum(i));
						wxString label;
						switch( FxStudioOptions::GetPhoneticAlphabet() )
						{
						default:
						case PA_DEFAULT:
							label = phonInfo.talkback;
							break;
						case PA_IPA:
							label = phonInfo.ipa;
							break;
						case PA_SAMPA:
							label = phonInfo.sampa;
							break;
						}
						gridInfos.PushBack(FxTimeGridInfo(phonTime, label));
					}
				}
			}
			else if( gridFormat == TGF_Words && _pPhonWordList )
			{
				for( FxSize i = 0; i < _pPhonWordList->GetNumWords(); ++i )
				{
					FxReal wordTime = _pPhonWordList->GetWordStartTime(i);
					if( minTime <= wordTime && wordTime < maxTime )
					{
						gridInfos.PushBack(FxTimeGridInfo(wordTime, _pPhonWordList->GetWordText(i)));
					}
				}
			}
		}

		// Fill out the x-values
		for( FxSize i = 0; i < gridInfos.Length(); ++i )
		{
			gridInfos[i].xValue = FxTimeManager::TimeToCoord(gridInfos[i].time, rect, group);
		}
	}
}

void FxTimeManager::SetPhonWordList( FxPhonWordList* pPhonWordList )
{
	_pPhonWordList = pPhonWordList;
}

FxTimeManager::FxTimeManager()
{
}

FxTimeManager::~FxTimeManager()
{
}

wxString FxFormatTime( FxReal time, FxTimeGridFormat gridFormat )
{
	switch( gridFormat )
	{
	default:
	case TGF_Seconds:
		return wxString::Format(wxT("%.3f"), time);
	case TGF_Milliseconds:
		return wxString::Format(wxT("%.0f"), time * 1000.f);
	case TGF_Frames:
		return wxString::Format(wxT("%.0f"), time * FxStudioOptions::GetFrameRate());
	};
}

FxReal FxUnformatTime( const wxString& timeString, FxTimeGridFormat gridFormat )
{
	switch( gridFormat )
	{
	default:
	case TGF_Seconds:
		return FxAtof(timeString.mb_str(wxConvLibc));
	case TGF_Milliseconds:
		return FxAtof(timeString.mb_str(wxConvLibc)) * 0.001f;
	case TGF_Frames:
		return FxAtof(timeString.mb_str(wxConvLibc)) / FxStudioOptions::GetFrameRate();
	}
}

} // namespace Face

} // namespace OC3Ent
