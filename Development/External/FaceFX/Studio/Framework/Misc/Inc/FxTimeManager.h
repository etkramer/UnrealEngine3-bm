//------------------------------------------------------------------------------
// This class is responsible for managing the time range for all the 
// time-synchronized windows in the system.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef TimeManager_H__
#define TimeManager_H__

#include "FxList.h"
#include "FxArray.h"
#include "FxStudioOptions.h"

namespace OC3Ent
{

namespace Face
{

const FxInt32 FxDefaultGroup = 0;

class FxTimeSubscriber
{
public:
	virtual void OnNotifyTimeChange() = 0;
	// Should return the minimum time known by the subscriber
	virtual FxReal GetMinimumTime() = 0;
	// Should return the maximum time known by the subscriber
	virtual FxReal GetMaximumTime() = 0;
	// Should return the last paint time of the subscriber
	virtual FxInt32 GetPaintPriority() = 0;

	// Allows a subscriber to modify the requested time change.
	virtual void VerifyPendingTimeChange( FxReal& min, FxReal& max );

	FxInt32 lastPaintTime;
};

class FxTimeSubscriberInfo
{
public:
	FxTimeSubscriberInfo( FxTimeSubscriber* = NULL, FxInt32 priority = 1 );

	FxTimeSubscriber* pSub;
	FxInt32           priority;

	FxBool operator<( const FxTimeSubscriberInfo& other );
};

class FxTimeSubscriberGroup
{
public:
	/// Adds a subscriber to receive notifications
	FxBool RegisterSubscriber( FxTimeSubscriber* pSubscriber );
	/// Removes a subscriber from the notification list
	FxBool UnregisterSubscriber( FxTimeSubscriber* pSubscriber );

	/// Requests a change of the minimum and maximum time values
	FxBool RequestTimeChange( FxReal minimum, FxReal maximum );
	/// Gets the minimum and maximum time values
	void GetTimes( FxReal& minimum, FxReal& maximum );

	/// Returns the minimum time needed by the subscribers
	FxReal GetMinimumTime();
	/// Returns the maximum time needed by the subscribers
	FxReal GetMaximumTime();

	/// Verify a pending time change with all the subscribers.
	void VerifyPendingTimeChange( FxReal& minimum, FxReal& maximum );
	/// Notify all the subscribers that the time has changed
	void NotifySubscribers();


private:
	/// The list of subscribers
	typedef FxList<FxTimeSubscriberInfo> SubscriberList;
	SubscriberList _subscribers;

	/// The minimum and maximum times (in seconds)
	FxReal _minTime;
	FxReal _maxTime;
};

// A grid information structure.
struct FxTimeGridInfo
{
	FxTimeGridInfo( FxReal iTime = 0.0f, wxString iLabel = wxEmptyString, FxInt32 iXVal = 0 )
		: time(iTime)
		, label(iLabel)
		, xValue(iXVal)
	{
	}
	FxReal	 time;
	wxString label;
	FxInt32  xValue;
};

// Forward declare the phoneme/word list
class FxPhonWordList;

class FxTimeManager
{
public:
	/// Returns the one and only instance of the singleton
	static FxTimeManager* Instance();
	/// Starts up the time manager.
	static void Startup( void );
    /// Shuts down the time manager.
	static void Shutdown( void );

	/// Returns a new group ID
	static FxInt32 CreateGroup();

	/// Adds a subscriber to receive notifications
	static FxBool RegisterSubscriber( FxTimeSubscriber* pSubscriber, FxInt32 group = FxDefaultGroup );
	/// Removes a subscriber from the notification list
	static FxBool UnregisterSubscriber( FxTimeSubscriber* pSubscriber, FxInt32 group = FxDefaultGroup );

	/// Requests a change of the minimum and maximum time values for a group
	static FxBool RequestTimeChange( FxReal minimum, FxReal maximum, FxInt32 group = FxDefaultGroup );
	/// Gets the minimum and maximum time values for a group
	static void GetTimes( FxReal& minimum, FxReal& maximum, FxInt32 group = FxDefaultGroup );

	/// Returns the minimum time needed by the subscribers
	static FxReal GetMinimumTime( FxInt32 group = FxDefaultGroup );
	/// Returns the maximum time needed by the subscribers
	static FxReal GetMaximumTime( FxInt32 group = FxDefaultGroup );

	/// Returns the time value for a given coordinate in a rect
	static FxReal CoordToTime( FxInt32 coord, const wxRect& rect, FxInt32 group = FxDefaultGroup );
	/// Returns the coordinate for a given time value in a rect
	static FxInt32 TimeToCoord( FxReal time, const wxRect& rect, FxInt32 group = FxDefaultGroup );

	/// Returns the array of grid info to display.
	static void GetGridInfos( FxArray<FxTimeGridInfo>& gridInfos, const wxRect& rect, FxInt32 group = FxDefaultGroup, FxTimeGridFormat gridFormat = TGF_Invalid, FxReal minTimeOverride = FxInvalidValue, FxReal maxTimeOverride = FxInvalidValue );
	/// Sets the phoneme/word list to use.
	static void SetPhonWordList( FxPhonWordList* pPhonWordList );

private:
	/// Disable construction and destruction for everyone except itself
	FxTimeManager();
	~FxTimeManager();

	/// Make copy construction and assignment break the compile by not providing
	/// definitions.
	FxTimeManager( const FxTimeManager& other );
	FxTimeManager& operator=( const FxTimeManager& other );

	/// The one and only instance
	static FxTimeManager* _pInst;
	/// Whether or not the instance has been destroyed
	static FxBool _destroyed;
	/// The phoneme/word list
	static FxPhonWordList* _pPhonWordList;

	/// The array of subscriber groups
	FxArray<FxTimeSubscriberGroup> _groups;
};

// Two utility functions
wxString FxFormatTime( FxReal time, FxTimeGridFormat gridFormat );
FxReal   FxUnformatTime( const wxString& timeString, FxTimeGridFormat gridFormat );

} // namespace Face

} // namespace OC3Ent

#endif
