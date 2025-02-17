/*=============================================================================
	VTuneIntegration.h: Provides integrated VTune timing support.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if _MSC_VER
#pragma once
#endif

#ifndef __VTuneIntegration_h__
#define __VTuneIntegration_h__



/**
 * FVTuneWrapper
 *
 * Interface to various VTune API functions, dynamically linked
 */
class FVTuneWrapper
{

	/** Friends with scope classes so that members can remain private.  This class is for internal use only.
	    It's in the header file for inlining purposes. */
	friend class FScopedVTuneBase;


public:

	/** Returns true if VTune functionality is available (VTuneAPI.dll was loaded).  If we haven't yet
	    attempted to load the dll file, calling IsActive() will load and initialize on demand. */
	static UBOOL IsActive()
	{
		if( !bHasInitialized )
		{
			InitializeOnDemand();
		}
		return ( VTuneAPIDLLInterface != NULL );
	}


	/** Pauses VTune */
	static void PauseVTune()
	{
		if( IsActive() )
		{
			VTPauseFunction();
			bIsPaused = TRUE;
		}
	}


	/** Resumes VTune */
	static void ResumeVTune()
	{
		if( IsActive() )
		{
			VTResumeFunction();
			bIsPaused = FALSE;
		}
	}


private:

	/**
	 * Initializes FVTuneWrapper
	 */
	static void InitializeOnDemand();


private:
	
	/** Function pointer type for VTPause() */
	typedef void ( *VTPauseFunctionPtr )( void );

	/** Function pointer type for VTResume() */
	typedef void ( *VTResumeFunctionPtr )( void );


	/** Static: True if we're currently initialized (or have attempted to initialize, since startup) */
	static UBOOL bHasInitialized;

	/** Static: DLL handle for VTuneAPI.dll (HMODULE) */
	static void* VTuneAPIDLLInterface;

	/** Static: VTPause() function pointer */
	static VTPauseFunctionPtr VTPauseFunction;

	/** Static: VTResume() function pointer */
	static VTResumeFunctionPtr VTResumeFunction;

	/** Static: Number of timers currently running.  VTune timers are always 'global inclusive'. */
	static INT TimerCount;

	/** Static: Whether or not VTune is currently paused (as far as we know.) */
	static UBOOL bIsPaused;

};



/**
 * FBaseVTuneTimer
 *
 * Base class for FScopedVTuneTimer and FScopedVTuneExcluder
 */
class FScopedVTuneBase
{

protected:

	/**
	 * Pauses or Resumes VTune and keeps track of the prior state so it can be restored later.
	 *
	 * @param	bWantPause		TRUE if this timer should 'include' code, or FALSE to 'exclude' code
	 *
	 **/
	void StartScopedTimer( const UBOOL bWantPause )
	{
		// Store the current state of VTune
		bWasPaused = FVTuneWrapper::bIsPaused;

		// If the current VTune state isn't set to what we need, or if the global VTune sampler isn't currently
		// running, then start it now
		if( FVTuneWrapper::TimerCount == 0 ||
			bWantPause != FVTuneWrapper::bIsPaused )
		{
			if( bWantPause )
			{
				FVTuneWrapper::PauseVTune();
			}
			else
			{
				FVTuneWrapper::ResumeVTune();
			}
		}

		// Increment number of overlapping timers
		++FVTuneWrapper::TimerCount;
	}


	/** Stops the scoped timer and restores VTune to its prior state */
	void StopScopedTimer()
	{
		// Make sure a timer was already started
		check( FVTuneWrapper::TimerCount > 0 );
		if( FVTuneWrapper::TimerCount > 0 )
		{
			// Decrement timer count
			--FVTuneWrapper::TimerCount;

			// Restore the previous state of VTune
			if( bWasPaused != FVTuneWrapper::bIsPaused )
			{
				if( bWasPaused )
				{
					FVTuneWrapper::PauseVTune();
				}
				else
				{
					FVTuneWrapper::ResumeVTune();
				}
			}
		}
	}


private:

	/** Stores the previous 'Paused?' state of VTune before this scope started */
	UBOOL bWasPaused;

};



/**
 * FScopedVTuneTimer
 *
 * Use this to include a body of code in VTune's captured session using 'Resume' and 'Pause' cues.  It
 * can safely be embedded within another 'timer' or 'excluder' scope.
 */
class FScopedVTuneTimer
	: public FScopedVTuneBase
{

public:

	/** Constructor */
	FScopedVTuneTimer()
	{
		// 'Timer' scopes will always 'resume' VTune
		const UBOOL bWantPause = FALSE;
		StartScopedTimer( bWantPause );
	}


	/** Destructor */
	~FScopedVTuneTimer()
	{
		StopScopedTimer();
	}

};



/**
 * FScopedVTuneExcluder
 *
 * Use this to EXCLUDE a body of code from VTune's captured session.  It can safely be embedded
 * within another 'timer' or 'excluder' scope.
 */
class FScopedVTuneExcluder
	: public FScopedVTuneBase
{

public:

	/** Constructor */
	FScopedVTuneExcluder()
	{
		// 'Excluder' scopes will always 'pause' VTune
		const UBOOL bWantPause = TRUE;
		StartScopedTimer( bWantPause );
	}


	/** Destructor */
	~FScopedVTuneExcluder()
	{
		StopScopedTimer();
	}

};



#endif // __VTuneIntegration_h__
