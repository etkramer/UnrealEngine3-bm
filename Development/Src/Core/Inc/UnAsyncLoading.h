/*=============================================================================
	UnAsyncLoading.h: Unreal async loading definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * Structure containing intermediate data required for async loading of all imports and exports of a
 * ULinkerLoad.
 */
struct FAsyncPackage : public FSerializableObject
{
	/**
	 * Constructor
	 */
	FAsyncPackage(const FString& InPackageName, const FGuid* InPackageGuid)
	:	PackageName					( InPackageName			)
	,	PackageGuid					( InPackageGuid != NULL ? *InPackageGuid : FGuid(0,0,0,0) )
	,	Linker						( NULL					)
	,	ImportIndex					( 0						)
	,	ExportIndex					( 0						)
	,	PreLoadIndex				( 0						)
	,	PostLoadIndex				( 0						)
	,	TimeLimit					( FLT_MAX				)
	,	bUseTimeLimit				( FALSE					)
	,	bTimeLimitExceeded			( FALSE					)
	,	TickStartTime				( 0						)
	,	LastObjectWorkWasPerformedOn( NULL					)
	,	LastTypeOfWorkPerformed		( NULL					)
	,	LoadStartTime				( 0.0					)
	,	LoadPercentage				( 0						)
#if TRACK_DETAILED_ASYNC_STATS
	,	TickCount					( 0						)
	,	TickLoopCount				( 0						)
	,	CreateLinkerCount			( 0						)
	,	FinishLinkerCount			( 0						)
	,	CreateImportsCount			( 0						)
	,	CreateExportsCount			( 0						)
	,	PreLoadObjectsCount			( 0						)
	,	PostLoadObjectsCount		( 0						)
	,	FinishObjectsCount			( 0						)
	,	TickTime					( 0.0					)
	,	CreateLinkerTime			( 0.0					)
	,	FinishLinkerTime			( 0.0					)
	,	CreateImportsTime			( 0.0					)
	,	CreateExportsTime			( 0.0					)
	,	PreLoadObjectsTime			( 0.0					)
	,	PostLoadObjectsTime			( 0.0					)
	,	FinishObjectsTime			( 0.0					)
#endif
	{}

	/**
	 * Ticks the async loading code.
	 *
	 * @param	InbUseTimeLimit		Whether to use a time limit
	 * @param	InTimeLimit			Soft limit to time this function may take
	 *
	 * @return	TRUE if package has finished loading, FALSE otherwise
	 */
	UBOOL Tick( UBOOL bUseTimeLimit, FLOAT TimeLimit );

	/**
	 * @return Estimated load completion percentage.
	 */
	FLOAT GetLoadPercentage() const;

	/**
	 * @return Time load begun. This is NOT the time the load was requested in the case of other pending requests.
	 */
	DOUBLE GetLoadStartTime() const;

	/**
	 * Emulates UObject::ResetLoaders for the package's Linker objects, hence deleting it. 
	 */
	void ResetLoader();

	/**
	 * Returns the name of the package to load.
	 */
	const FString& GetPackageName() const
	{
		return PackageName;
	}

	void AddCompletionCallback(const FAsyncCompletionCallbackInfo& Callback)
	{
		CompletionCallbacks.AddUniqueItem(Callback);
	}

	/**
	 * Serialize the linke for garbage collection.
	 * 
	 * @param Ar The archive to serialize with
	 */
	virtual void Serialize( FArchive& Ar )
	{
		Ar << Linker;
	}

private:
	/** Name of the package to load.																	*/
	FString						PackageName;
	/** GUID of the package to load, or the zeroed invalid GUID for "don't care" */
	FGuid						PackageGuid;
	/** Linker which is going to have its exports and imports loaded									*/
	ULinkerLoad*				Linker;
	/** Call backs called when we finished loading this package											*/
	TArray<FAsyncCompletionCallbackInfo>	CompletionCallbacks;
	/** Current index into linkers import table used to spread creation over several frames				*/
	INT							ImportIndex;
	/** Current index into linkers export table used to spread creation over several frames				*/
	INT							ExportIndex;
	/** Current index into GObjLoaded array used to spread routing PreLoad over several frames			*/
	INT							PreLoadIndex;
	/** Current index into GObjLoaded array used to spread routing PostLoad over several frames			*/
	INT							PostLoadIndex;
	/** Currently used time limit for this tick.														*/
	FLOAT						TimeLimit;
	/** Whether we are using a time limit for this tick.												*/
	UBOOL						bUseTimeLimit;
	/** Whether we already exceed the time limit this tick.												*/
	UBOOL						bTimeLimitExceeded;
	/** The time taken when we started the tick.														*/
	DOUBLE						TickStartTime;
	/** Last object work was performed on. Used for debugging/ logging purposes.						*/
	UObject*					LastObjectWorkWasPerformedOn;
	/** Last type of work performed on object.															*/
	TCHAR*						LastTypeOfWorkPerformed;
	/** Time load begun. This is NOT the time the load was requested in the case of pending requests.	*/
	DOUBLE						LoadStartTime;
	/** Estimated load percentage.																		*/
	FLOAT						LoadPercentage;

public:
#if TRACK_DETAILED_ASYNC_STATS
	/** Number of times Tick function has been called.													*/
	INT							TickCount;
	/** Number of iterations in loop inside Tick.														*/
	INT							TickLoopCount;

	/** Number of iterations for CreateLinker.															*/
	INT							CreateLinkerCount;
	/** Number of iterations for FinishLinker.															*/
	INT							FinishLinkerCount;
	/** Number of iterations for CreateImports.															*/
	INT							CreateImportsCount;
	/** Number of iterations for CreateExports.															*/
	INT							CreateExportsCount;
	/** Number of iterations for PreLoadObjects.														*/
	INT							PreLoadObjectsCount;
	/** Number of iterations for PostLoadObjects.														*/
	INT							PostLoadObjectsCount;
	/** Number of iterations for FinishObjects.															*/
	INT							FinishObjectsCount;

	/** Total time spent in Tick.																		*/
	DOUBLE						TickTime;
	/** Total time spent in	CreateLinker.																*/
	DOUBLE						CreateLinkerTime;
	/** Total time spent in FinishLinker.																*/
	DOUBLE						FinishLinkerTime;
	/** Total time spent in CreateImports.																*/
	DOUBLE						CreateImportsTime;
	/** Total time spent in CreateExports.																*/
	DOUBLE						CreateExportsTime;
	/** Total time spent in PreLoadObjects.																*/
	DOUBLE						PreLoadObjectsTime;
	/** Total time spent in PostLoadObjects.															*/
	DOUBLE						PostLoadObjectsTime;
	/** Total time spent in FinishObjects.																*/
	DOUBLE						FinishObjectsTime;
#endif

private:
	/**
	 * Gives up time slice if time limit is enabled.
	 *
	 * @return TRUE if time slice can be given up, FALSE otherwise
	 */
	UBOOL GiveUpTimeSlice();
	/**
	 * Returns whether time limit has been exceeded.
	 *
	 * @return TRUE if time limit has been exceeded (and is used), FALSE otherwise
	 */
	UBOOL IsTimeLimitExceeded();
	/**
	 * Begin async loading process. Simulates parts of UObject::BeginLoad.
	 *
	 * Objects created during BeginAsyncLoad and EndAsyncLoad will have RF_AsyncLoading set
	 */
	void BeginAsyncLoad();
	/**
	 * End async loading process. Simulates parts of UObject::EndLoad(). FinishObjects 
	 * simulates some further parts once we're fully done loading the package.
	 */
	void EndAsyncLoad();
	/**
	 * Create linker async. Linker is not finalized at this point.
	 *
	 * @return TRUE
	 */
	UBOOL CreateLinker();
	/**
	 * Finalizes linker creation till time limit is exceeded.
	 *
	 * @return TRUE if linker is finished being created, FALSE otherwise
	 */
	UBOOL FinishLinker();
	/** 
	 * Create imports till time limit is exceeded.
	 *
	 * @return TRUE if we finished creating all imports, FALSE otherwise
	 */
	UBOOL CreateImports();
	/**
	 * Create exports till time limit is exceeded.
	 *
	 * @return TRUE if we finished creating and preloading all exports, FALSE otherwise.
	 */
	UBOOL CreateExports();
	/**
	 * Preloads aka serializes all loaded objects.
	 *
	 * @return TRUE if we finished serializing all loaded objects, FALSE otherwise.
	 */
	UBOOL PreLoadObjects();
	/**
	 * Route PostLoad to all loaded objects. This might load further objects!
	 *
	 * @return TRUE if we finished calling PostLoad on all loaded objects and no new ones were created, FALSE otherwise
	 */
	UBOOL PostLoadObjects();
	/**
	 * Finish up objects and state, which means clearing the RF_AsyncLoading flag on newly created ones
	 *
	 * @return TRUE
	 */
	UBOOL FinishObjects();
};


