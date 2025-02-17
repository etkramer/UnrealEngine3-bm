/*----------------------------------------------------------------------
    This Software and Related Documentation are Proprietary to Ageia
    Technologies, Inc.

    Copyright 2007 Ageia Technologies, Inc. St. Louis, MO
    Unpublished -
    All Rights Reserved Under the Copyright Laws of the United States.

    Restricted Rights Legend:  Use, Duplication, or Disclosure by
    the Government is Subject to Restrictions as Set Forth in
    Paragraph (c)(1)(ii) of the Rights in Technical Data and
    Computer Software Clause at DFARS 252.227-7013.  Ageia
    Technologies Inc.
-----------------------------------------------------------------------*/


#ifndef __AG_PERFMON_EVENT_SRC_API___
#define __AG_PERFMON_EVENT_SRC_API___

#include "AgPerfMonEventAPI.h"

typedef AgPmHANDLE  (AgPmCreateSourceConnection_FUNC)(AgU32 version);
typedef bool        (AgPmDestroySourceConnection_FUNC)(AgPmHANDLE hconn);
typedef AgEventID   (AgPmRegisterEvent_FUNC)(AgPmHANDLE hconn, const char *name);
typedef bool        (AgPmSubmitEvent_FUNC)(AgPmHANDLE hconn, AgEventID id, unsigned int data0, unsigned int data1, unsigned char data2);
typedef bool        (AgPmEventEnabled_FUNC)(AgPmHANDLE hconn, AgEventID id);
typedef bool        (AgPmEventLoggingEnabled_FUNC)(AgPmHANDLE hconn);


/** Event Source API */
AgPmHANDLE  AgPmCreateSourceConnection();
bool        AgPmDestroySourceConnection(AgPmHANDLE hconn);   
AgEventID   AgPmRegisterEvent(AgPmHANDLE hconn, const char *name);
bool        AgPmSubmitEvent(AgPmHANDLE hconn, AgEventID id, unsigned int data0, unsigned int data1, unsigned char data2);
bool        AgPmRegisterEventAsPassive(AgPmHANDLE hconn, AgEventID id, void *context, AgPassiveEventCallback funcPtr);
bool        AgPmEventEnabled(AgPmHANDLE hconn, AgEventID id);
bool        AgPmEventLoggingEnabled(AgPmHANDLE hconn);

#define AG_PERFMON_EV_START				0x00
#define AG_PERFMON_EV_STOP				0x01
#define AG_PERFMON_EV_STAT  			0x02
#define AG_PERFMON_EV_DEBUG  			0x03


#define DEFINE_EVENT(name) name,
enum AgPerfEventEnum
{
#include "AgPerfMonEventDefs.h"
	AgPerfEventNumEvents
};
#undef DEFINE_EVENT


class AgPerfUtils
{
public:

	AgPerfUtils();
    ~AgPerfUtils();

	AgEventID registerEvent(const char *);
	bool isEnabled();
	void startEvent(AgEventID id, AgU16 data = 0);
	void stopEvent(AgEventID id, AgU16 data = 0);
	void statEvent(AgEventID id, AgU32 stat);
	void statEvent(AgEventID id, AgU32 stat, AgU32 ident);
	void debugEvent(AgEventID id, AgU32 data0, AgU32 data1);

	inline AgEventID getEventId(int index) { return mEventIds[index]; };

	AgU32 GetThreadPriority();
    unsigned char GetProcID (void);

private:
    typedef struct _THREAD_BASIC_INFORMATION {
        AgU32               ExitStatus;
        void               *TebBaseAddress;
        AgU32               ClientId;
        AgU32               reserved;
        AgU32               AffinityMask;
        AgU32               Priority;
        AgU32               BasePriority;
    } THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

    typedef AgU32 ( __stdcall tinfo_FUNC)(void *ThreadHandle,
        AgU32 ThreadInformationClass,
        PTHREAD_BASIC_INFORMATION ThreadInformation,
        AgU32 ThreadInformationLength,
        AgU32 *ReturnLength);

    tinfo_FUNC			*mNtQueryThreadInfo;
    void				*mhNTDLL;
	static AgPmHANDLE	mhAgPm;

	AgEventID			mEventIds[AgPerfEventNumEvents+1];
	static const char *	mEventNames[AgPerfEventNumEvents+1];
};




#endif /** _AG_PERFMON_EVENT_SRC_API___ */
