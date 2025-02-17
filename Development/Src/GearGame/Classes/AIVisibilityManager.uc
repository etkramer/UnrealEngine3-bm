/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AIVisibilityManager extends Object
	native(AI);

/** Number deep we should start in the controller list for queueing up vis requests
 *   (facilitates round-robin sight check queueing)
 */
var int ControllerIttStartPoint;

/** debug bool, when this is enabled all visibility test results will be drawn **/
var bool bDrawVisTests;

/** List of free linecheckresutls **/
var private const native noexport pointer FreeLineCheckResList{LineCheckResult};
/** List of linecheckresults which are currently awaiting update by physics **/
var private const native noexport pointer BusyLineCheckResList{LineCheckResult};

var private const native noexport pointer PendingLineCheckHead{LineCheckResult};
var private const native noexport pointer PendingLineCheckTail{LineCheckResult};

/** when Init() is called, a pool of LineCheckResults will be created, with this many elements in it **/
const PoolSize = 50;
const MaxLineChecksPerFrame = 15;

//@note: if you change this, grenade fog volumes need to be fixed up to use the correct collision for blocking AI visibility
const bUseAsyncLineChecksForVisibility = 0;

cpptext
{
	class LineCheckResult: public FAsyncLineCheckResult
	{

	public:
		typedef UBOOL (AGearAI::*ShouldLineCheckCb)( APawn *E, FVector& LineChkStart, FVector& LineChkEnd );
		typedef void (AGearAI::*FinishedCallback)(APawn* TestedPawn, UBOOL bVisible, FVector& VisLoc );

		LineCheckResult() :
		  FAsyncLineCheckResult()
		  ,TestingAI(NULL)
		  ,PawnToTestAgainst(NULL)
		  ,bStale(FALSE)
		  ,Next(NULL)
		  ,FinishedCallBackFunction(NULL)
		  ,ShouldCheckFunction(NULL)
		  ,CheckStart(0.f)
		  ,CheckEnd(0.f)
		  {}

		void InitLineCheck(AGearAI* InTestingAI, APawn* InPawnToTestAgainst, ShouldLineCheckCb ShouldCheckCallBack, FinishedCallback FinCallback)
		{
			TestingAI = InTestingAI;
			PawnToTestAgainst = InPawnToTestAgainst;
			FinishedCallBackFunction = FinCallback;
			ShouldCheckFunction = ShouldCheckCallBack;
			bCheckStarted = FALSE;
			bCheckCompleted = FALSE;
			bHit = FALSE;
			bStale=FALSE;
		}

		void TestFinished()
		{
			if(bStale == FALSE && TestingAI && FinishedCallBackFunction)
			{
				((*TestingAI).*FinishedCallBackFunction)(PawnToTestAgainst,!bHit,CheckEnd);
			}
		}

		virtual void Serialize( FArchive& Ar )
		{
			Ar << TestingAI;
			Ar << PawnToTestAgainst;
		}

		/**
		 * calls ShouldCheckCallback to determine endpoints for linecheck, and see if we should early out
		 * @return returns TRUE if a sight check was queued
		 */
		UBOOL TriggerLineCheck();

		/** Pawn we're testing from **/
		AGearAI* TestingAI;
		/** Pawn we're testing to **/
		APawn* PawnToTestAgainst;

		/** Bool that indicates one or both of the pawns involved in this linecheck have been deleted, so don't call the callback if this is true! **/
		UBOOL bStale;

		LineCheckResult* Next;

		FinishedCallback FinishedCallBackFunction;
		ShouldLineCheckCb ShouldCheckFunction;

		// The start/end of this line check
		// Visibility position is marked at CheckEnd because target likely moved during the async check
		FVector CheckStart, CheckEnd;
	};

public:
	UAIVisibilityManager() :
	  FreeLineCheckResList(NULL)
	  ,BusyLineCheckResList(NULL)
	  ,PendingLineCheckHead(NULL)
	  ,PendingLineCheckTail(NULL)
	{
		if (!IsTemplate())
		{
			Init();
		}
	}
	virtual void FinishDestroy()
	{
		Super::FinishDestroy();
		Flush();
	}


	/** RequestSightCheckForAToB
		* Will return the current known status of the test requested, and kick off a new test if needed
		* @param PawnA  - the pawn who wants to do a sight check
		* @param PawnB  - the pawn to sight check against (e.g. PawnA wants to know if it can see pawnB)
		* @param FromPt - start point for Line Check
		* @param ToPt   - end point for Line Check
		* @param Cb	  - callback function to call when we have a result
		*/
	UBOOL RequestSightCheck(AGearAI* InTestingAI, APawn* PawnToTest, LineCheckResult::ShouldLineCheckCb ShouldCheckCb, LineCheckResult::FinishedCallback FinishedCb);
	/** iterates through the list of pending tests to see if they're done **/
	void Tick(FLOAT DeltaTime);
	/** wipes all records and pending tests MT->how do we do this safely? (physics might still have a ptr) **/
	void Flush();

	void Init();

	virtual void AddReferencedObjects(TArray<UObject*>& ObjectArray);
private:
   /** AddBackToPool
	* moves LineCheckRes from the busy list to the free list
	* @param LineCheckResToMove - the line check to move
	* @param PrevLineCheckRes - the line check result in the list just before the one we're moving
    */
	void AddBackToPool(LineCheckResult* LineCheckResToMove, LineCheckResult* PrevLineCheckRes, LineCheckResult*& DestPool, LineCheckResult*& SrcPool);

	/**
	 * iterates over busy line check results checking to see if they've gotten a result back yet, if so move them back to free lsit
	 */
	void UpdateBusyLineChecks();

	/**
	 * initiates requests to physics for pending line checks until we hit the max per frame
	 */
	void DoPhysRequestsForPendingLineChecks();

	/**
	 * (replaces ShowSelf() iterates over all controllers and queues up line check requests when neccesary
	 *  This replaces ShowSelf so that we can rotate through the controller list round-robin style and make sure everyone gets
	 *  a sight result
	 */
	void QueueUpVisRequests();

	UBOOL SuccessfullyShowedController(AController* Controller);

	LineCheckResult* FreeLineCheckResList;
	LineCheckResult* BusyLineCheckResList;
	LineCheckResult* PendingLineCheckHead;
	LineCheckResult* PendingLineCheckTail;

}

final native function NotifyPawnDestroy(Pawn Pawn);

// flushes the pool, and re-initializes it
final native function Reset();
