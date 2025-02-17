/*=============================================================================
	ShaderCompiler.cpp: Platform independent shader compilation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnConsoleTools.h"

UBOOL CanBlendWithFPRenderTarget(UINT Platform)
{
	if(Platform == SP_PCD3D_SM2)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

UBOOL CanAccessFacingRegister(UINT Platform)
{
	if(Platform != SP_PCD3D_SM2)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
* Converts shader platform to human readable string. 
*
* @param ShaderPlatform	Shader platform enum
* @return text representation of enum
*/
const TCHAR* ShaderPlatformToText( EShaderPlatform ShaderPlatform )
{
	switch( ShaderPlatform )
	{
	case SP_PCD3D_SM2:
		return TEXT("PC-D3D-SM2");
		break;
	case SP_PCD3D_SM3:
		return TEXT("PC-D3D-SM3");
		break;
	case SP_XBOXD3D:
		return TEXT("Xbox360");
		break;
	case SP_PS3:
		return TEXT("PS3");
		break;
	case SP_PCD3D_SM4:
		return TEXT("PC-D3D-SM4");
		break;
	default:
		return TEXT("Unknown");
		break;
	}
}

FConsoleShaderPrecompiler* GConsoleShaderPrecompilers[SP_NumPlatforms] = { NULL, NULL, NULL, NULL};

DECLARE_STATS_GROUP(TEXT("ShaderCompiling"),STATGROUP_ShaderCompiling);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Total Material Shader Compiling Time"),STAT_ShaderCompiling_MaterialShaders,STATGROUP_ShaderCompiling);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Total Global Shader Compiling Time"),STAT_ShaderCompiling_GlobalShaders,STATGROUP_ShaderCompiling);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("RHI Compile Time"),STAT_ShaderCompiling_RHI,STATGROUP_ShaderCompiling);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("CRCing Shader Files"),STAT_ShaderCompiling_CRCingShaderFiles,STATGROUP_ShaderCompiling);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Loading Shader Files"),STAT_ShaderCompiling_LoadingShaderFiles,STATGROUP_ShaderCompiling);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("HLSL Translation"),STAT_ShaderCompiling_HLSLTranslation,STATGROUP_ShaderCompiling);

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Total Material Shaders"),STAT_ShaderCompiling_NumTotalMaterialShaders,STATGROUP_ShaderCompiling);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Special Material Shaders"),STAT_ShaderCompiling_NumSpecialMaterialShaders,STATGROUP_ShaderCompiling);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Terrain Material Shaders"),STAT_ShaderCompiling_NumTerrainMaterialShaders,STATGROUP_ShaderCompiling);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Decal Material Shaders"),STAT_ShaderCompiling_NumDecalMaterialShaders,STATGROUP_ShaderCompiling);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Particle Material Shaders"),STAT_ShaderCompiling_NumParticleMaterialShaders,STATGROUP_ShaderCompiling);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Skinned Material Shaders"),STAT_ShaderCompiling_NumSkinnedMaterialShaders,STATGROUP_ShaderCompiling);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Lit Material Shaders"),STAT_ShaderCompiling_NumLitMaterialShaders,STATGROUP_ShaderCompiling);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Unlit Material Shaders"),STAT_ShaderCompiling_NumUnlitMaterialShaders,STATGROUP_ShaderCompiling);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Transparent Material Shaders"),STAT_ShaderCompiling_NumTransparentMaterialShaders,STATGROUP_ShaderCompiling);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Opaque Material Shaders"),STAT_ShaderCompiling_NumOpaqueMaterialShaders,STATGROUP_ShaderCompiling);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Masked Material Shaders"),STAT_ShaderCompiling_NumMaskedMaterialShaders,STATGROUP_ShaderCompiling);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Shaders w/ Resources Initialized"),STAT_ShaderCompiling_NumShadersInitialized,STATGROUP_ShaderCompiling);

extern UBOOL D3D9CompileShader(
	UINT ThreadId,
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	FShaderTarget Target,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output,
	UBOOL bSilent,
	UBOOL bDebugDump = FALSE,
	const TCHAR* ShaderSubDir = NULL
	);

extern UBOOL D3D10CompileShader(
	UINT ThreadId,
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	FShaderTarget Target,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output,
	UBOOL bSilent,
	UBOOL bDebugDump = FALSE,
	const TCHAR* ShaderSubDir = NULL
	);

extern bool PrecompileConsoleShader(UINT ThreadId, const TCHAR* SourceFilename, const TCHAR* FunctionName, const FShaderTarget& Target, const FShaderCompilerEnvironment& Environment, FShaderCompilerOutput& Output);

extern bool PreprocessConsoleShader(const TCHAR* SourceFilename, const TCHAR* FunctionName, const FShaderTarget& Target, const FShaderCompilerEnvironment& Environment, const TCHAR* ShaderSubDir);

// A distinct timer for each platform to tease out time spent
// compiling for multiple platforms with a single PCS command
DOUBLE GRHIShaderCompileTime_Total     = 0.0;
DOUBLE GRHIShaderCompileTime_PS3       = 0.0;
DOUBLE GRHIShaderCompileTime_XBOXD3D   = 0.0;
DOUBLE GRHIShaderCompileTime_PCD3D_SM2 = 0.0;
DOUBLE GRHIShaderCompileTime_PCD3D_SM3 = 0.0;
DOUBLE GRHIShaderCompileTime_PCD3D_SM4 = 0.0;

FShaderCompileThreadRunnable::FShaderCompileThreadRunnable(FShaderCompilingThreadManager* InManager) :
	Manager(InManager),
	Thread(NULL),
	WorkerAppId(0),
	ThreadId(InManager->NextThreadId++),
	bTerminatedByError(FALSE),
	bCopiedShadersToWorkingDirectory(FALSE)
{
}

/** Entry point for shader compiling threads, all but the main thread start here. */
DWORD FShaderCompileThreadRunnable::Run()
{
	while (TRUE)
	{
		// Break out of the loop if the main thread is signaling us to.
		if (Manager->KillThreadsCounter.GetValue() != 0)
		{
			break;
		}
		// Only enter ThreadLoop if the main thread has incremented ActiveCounter
		else if (ActiveCounter.GetValue() > 0)
		{
#if _MSC_VER && !XBOX
			extern INT CreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo );
			if(!appIsDebuggerPresent())
			{
				__try
				{
					// Do the work
					Manager->ThreadLoop(ThreadId);
				}
				__except( CreateMiniDump( GetExceptionInformation() ) )
				{
					ErrorMessage = GErrorHist;

					// Use a memory barrier to ensure that the main thread sees the write to ErrorMessage before
					// the write to bTerminatedByError.
					appMemoryBarrier();

					bTerminatedByError = TRUE;
					ActiveCounter.Decrement();
					break;
				}
			}
			else
#endif
			{
				Manager->ThreadLoop(ThreadId);
			}
			ActiveCounter.Decrement();
		}
		else
		{
			// Yield CPU time while waiting for work, sleep for 10ms
			//@todo - shut this thread down if not used for some amount of time
			appSleep(0.01f);
		}
	}

	return 0;
}

/** Called by the main thread only, reports exceptions in the worker threads */
void FShaderCompileThreadRunnable::CheckHealth() const
{
	if (bTerminatedByError)
	{
		GErrorHist[0] = 0;
		GIsCriticalError = FALSE;
		GError->Logf(TEXT("Shader Compiling thread %u exception:\r\n%s"), ThreadId, *ErrorMessage);
	}
}

/** Helper function that appends memory to a buffer */
void WorkerInputAppendMemory(const void* Ptr, INT Size, TArray<BYTE>& Buffer)
{
	INT OldNum = Buffer.Add(sizeof(INT) + Size);
	appMemcpy(&Buffer(OldNum), &Size, sizeof(Size));
	appMemcpy(&Buffer(OldNum) + sizeof(Size), Ptr, Size);
}

/** Helper function that reads memory from a buffer */
void WorkerOutputReadMemory(void* Dest, UINT Size, INT& CurrentPosition, const TArray<BYTE>& Buffer)
{
	appMemcpy(Dest, &Buffer(CurrentPosition), Size);
	CurrentPosition += Size;
}

FShaderCompilingThreadManager* GShaderCompilingThreadManager = NULL;

FShaderCompilingThreadManager::FShaderCompilingThreadManager() :
	NextThreadId(0),
	NumUnusedShaderCompilingThreads(0),
	ThreadedShaderCompileThreshold(4),
	bAllowMultiThreadedShaderCompile(FALSE),
	bMultithreadedCompile(FALSE),
	ShaderCompileWorkerName(TEXT("UE3ShaderCompileWorker.exe"))
{
	// Read values from the engine ini
	verify(GConfig->GetBool( TEXT("DevOptions.Shaders"), TEXT("bAllowMultiThreadedShaderCompile"), bAllowMultiThreadedShaderCompile, GEngineIni ));
	INT TempValue;
	verify(GConfig->GetInt( TEXT("DevOptions.Shaders"), TEXT("NumUnusedShaderCompilingThreads"), TempValue, GEngineIni ));
	NumUnusedShaderCompilingThreads = TempValue;
	// Use all the cores on the build machines
	if (ParseParam(appCmdLine(), TEXT("BUILDMACHINE")) || ParseParam(appCmdLine(), TEXT("USEALLAVAILABLECORES")))
	{
		NumUnusedShaderCompilingThreads = 0;
	}
	verify(GConfig->GetInt( TEXT("DevOptions.Shaders"), TEXT("ThreadedShaderCompileThreshold"), TempValue, GEngineIni ));
	ThreadedShaderCompileThreshold = TempValue;
	verify(GConfig->GetBool( TEXT("DevOptions.Shaders"), TEXT("bDumpShaderPDBs"), bDumpShaderPDBs, GEngineIni ));
}

FString FShaderCompilingThreadManager::GetShaderPDBPath() const
{
	return FString(appBaseDir()) + TEXT("..") PATH_SEPARATOR TEXT("Engine") PATH_SEPARATOR TEXT("Shaders") PATH_SEPARATOR TEXT("PDBDump") PATH_SEPARATOR;
}

/** Adds a job to the compile queue, called by the main thread only. */
void FShaderCompilingThreadManager::AddJob(const FShaderCompileJob& NewJob)
{
	CompileQueue.AddItem(NewJob);
}

/** Launches the worker, returns the launched Process Id. */
DWORD FShaderCompilingThreadManager::LaunchWorker(const FString& WorkingDirectory, DWORD ProcessId, UINT ThreadId)
{
	// Setup the parameters that the worker application needs
	const FString WorkerParameters = WorkingDirectory + TEXT(" ") + appItoa(ProcessId) + TEXT(" ") + appItoa(ThreadId);
	// Launch the worker process
	void* WorkerHandle = appCreateProc(*ShaderCompileWorkerName, *WorkerParameters);
	checkf(WorkerHandle, TEXT("Couldn't launch %s! Make sure the exe is in your binaries folder."), *ShaderCompileWorkerName);
	DWORD WorkerId = 0;
#if _WINDOWS
	// Get the worker's process Id from the returned handle
	WorkerId = GetProcessId(WorkerHandle);
	check(WorkerId > 0);
	CloseHandle(WorkerHandle);
#endif
	return WorkerId;
}

/** Processing loop shared by all threads.  The Main thread has an Id of 0. */
void FShaderCompilingThreadManager::ThreadLoop(UINT CurrentThreadId)
{
	UBOOL bIsDone = FALSE;
	while (!bIsDone)
	{
		// Atomically read and increment the next job index to process.
		INT JobIndex = NextShaderToProcess.Increment() - 1;

		// Only continue if JobIndex is valid and no other threads have encountered a shader compile error
		if (JobIndex < CompileQueue.Num() && ShaderCompileErrorCounter.GetValue() == 0)
		{
			FShaderCompileJob& CurrentJob = CompileQueue(JobIndex);
			
			// Main thread only
			if (CurrentThreadId == 0)
			{
				for(INT ThreadIndex = 0;ThreadIndex < Threads.Num();ThreadIndex++)
				{
					Threads(ThreadIndex).CheckHealth();
				}
			}

#if _WINDOWS
			FString DebugDumpDir;
			check(CurrentJob.ShaderType);
			FString DirectoryFriendlyShaderTypeName = CurrentJob.ShaderType->GetName();
			// Replace template directives with valid symbols so it can be used as a windows directory name
			DirectoryFriendlyShaderTypeName.ReplaceInline(TEXT("<"),TEXT("("));
			DirectoryFriendlyShaderTypeName.ReplaceInline(TEXT(">"),TEXT(")"));
			FString ShaderSubDir = MaterialName;
			if (CurrentJob.VFType)
			{
				ShaderSubDir = ShaderSubDir * CurrentJob.VFType->GetName();
			}
			ShaderSubDir = ShaderSubDir * DirectoryFriendlyShaderTypeName;

			if (CurrentJob.Target.Platform == SP_PCD3D_SM4)
			{
				CurrentJob.bSucceeded = D3D10CompileShader(
					CurrentThreadId,
					*CurrentJob.SourceFilename,
					*CurrentJob.FunctionName,
					CurrentJob.Target,
					CurrentJob.Environment,
					CurrentJob.Output,
					bSilent,
					bDebugDump,
					*ShaderSubDir
				);
			}
			else if (CurrentJob.Target.Platform == SP_PCD3D_SM3 || CurrentJob.Target.Platform == SP_PCD3D_SM2)
			{
				CurrentJob.bSucceeded = D3D9CompileShader(
					CurrentThreadId,
					*CurrentJob.SourceFilename,
					*CurrentJob.FunctionName,
					CurrentJob.Target,
					CurrentJob.Environment,
					CurrentJob.Output,
					bSilent,
					bDebugDump,
					*ShaderSubDir);
			}
			else
			{
				// Console shaders use the global shader precompiler for that platform
				check(GConsoleShaderPrecompilers[CurrentJob.Target.Platform]);
				if (bDebugDump)
				{
					check(CurrentThreadId == 0);
					// Preprocess instead of compile the shader
					CurrentJob.bSucceeded = PreprocessConsoleShader(
						*CurrentJob.SourceFilename, 
						*CurrentJob.FunctionName, 
						CurrentJob.Target, 
						CurrentJob.Environment, 
						*ShaderSubDir);

					if (CurrentJob.bSucceeded)
					{
						// Make sure Output gets initialized with correct bytecode etc
						CurrentJob.bSucceeded = PrecompileConsoleShader(
							CurrentThreadId,
							*CurrentJob.SourceFilename, 
							*CurrentJob.FunctionName, 
							CurrentJob.Target, 
							CurrentJob.Environment, 
							CurrentJob.Output);
					}
				}
				else
				{
					CurrentJob.bSucceeded = PrecompileConsoleShader(
						CurrentThreadId,
						*CurrentJob.SourceFilename, 
						*CurrentJob.FunctionName, 
						CurrentJob.Target, 
						CurrentJob.Environment, 
						CurrentJob.Output);
				}
			}
#else
			{
				appErrorf(TEXT("Attempted to compile \'%s\' shader for platform %d on console."),*CurrentJob.SourceFilename,CurrentJob.Target.Platform);
			}
#endif
			if (!CurrentJob.bSucceeded)
			{
				// Indicate to the other threads that a shader compile error was encountered, they should abort
				ShaderCompileErrorCounter.Increment();
				bIsDone = TRUE;
			}
		}
		else
		{
			// Processing has begun for all jobs
			bIsDone = TRUE;
		}
	}
}

/** Compiles a single job through the worker application.  This is called from all threads. */
void FShaderCompilingThreadManager::WorkerCompile(
	UINT ThreadId,
	EWorkerJobType JobType, 
	TArray<BYTE>& WorkerInput, 
	TArray<BYTE>& WorkerOutput)
{
	FShaderCompileThreadRunnable& CurrentThread = Threads(ThreadId);
	DWORD ProcessId = 0;
#if _WINDOWS
	// Get the current process Id, this will be used by the worker app to shut down when it's parent is no longer running.
	ProcessId = GetCurrentProcessId();
#endif
	FString WorkingDirectory = TEXT("..") PATH_SEPARATOR TEXT("Engine") PATH_SEPARATOR TEXT("Shaders") PATH_SEPARATOR TEXT("WorkingDirectory") PATH_SEPARATOR;
	// Use a working directory unique to this process and thread so that it will not conflict with other processes or threads in this same process.
	WorkingDirectory += appItoa(ProcessId) + PATH_SEPARATOR + appItoa(ThreadId);

	// For console shaders, the precompiler expects a shader's includes to be on disk in the same directory as the main shader,
	// But multiple threads are writing out Material.usf and VertexFactory.usf at the same time,
	// So we must duplicate all the shader files into each working directory.
	// Only do the copy once, the first time it is needed.
	if ((JobType == WJT_XenonShader || JobType == WJT_PS3Shader) && !CurrentThread.bCopiedShadersToWorkingDirectory)
	{
		const TCHAR* SearchString = TEXT("..") PATH_SEPARATOR TEXT("Engine") PATH_SEPARATOR TEXT("Shaders") PATH_SEPARATOR TEXT("*.usf");
		TArray<FString> ShaderFiles;
		GFileManager->FindFiles(ShaderFiles, SearchString, TRUE, FALSE);
		for (INT ShaderIndex = 0; ShaderIndex < ShaderFiles.Num(); ShaderIndex++)
		{
			const FString SourcePath = FString(TEXT("..") PATH_SEPARATOR TEXT("Engine") PATH_SEPARATOR TEXT("Shaders") PATH_SEPARATOR) + ShaderFiles(ShaderIndex);
			const FString DestPath = WorkingDirectory * ShaderFiles(ShaderIndex);
			GFileManager->Copy(*DestPath, *SourcePath, TRUE, TRUE);
		}
		CurrentThread.bCopiedShadersToWorkingDirectory = TRUE;
	}

	// Write out the file that the worker app is waiting for, which has all the information needed to compile the shader.
	//@todo - use memory mapped files for faster transfer
	FString TransferFileName = WorkingDirectory * TEXT("WorkerInput.bin");
	FArchive* TransferFile = GFileManager->CreateFileWriter(*TransferFileName, FILEWRITE_EvenIfReadOnly | FILEWRITE_NoFail);
	check(TransferFile);
	TransferFile->Serialize(&WorkerInput(0), WorkerInput.Num());
	TransferFile->Close();
	delete TransferFile;

	FString OutputFileName = WorkingDirectory * TEXT("WorkerOutput.bin");
	UBOOL bLoadedFile = FALSE;
	UBOOL bLaunchedOnce = FALSE;
	while (!bLoadedFile)
	{
		// Block until the worker writes out the output file
		bLoadedFile = appLoadFileToArray(WorkerOutput, *OutputFileName, GFileManager, FILEREAD_Silent);
		if (!bLoadedFile)
		{
			if (CurrentThread.WorkerAppId == 0 || !appIsApplicationRunning(CurrentThread.WorkerAppId))
			{
				// Try to load the file again since it may have been written out since the last load attempt
				bLoadedFile = appLoadFileToArray(WorkerOutput, *OutputFileName, GFileManager, FILEREAD_Silent);
				if (!bLoadedFile)
				{
					if (bLaunchedOnce)
					{
						// Check that the worker is still running, if not we are in a deadlock
						// Normal errors in the worker will still write out the output file, 
						// This should only happen if a breakpoint in the debugger caused the worker to exit due to inactivity.
						appErrorf(TEXT("%s terminated unexpectedly! ThreadId=%u"), 
							*ShaderCompileWorkerName,
							ThreadId
							);
					}

					// Store the Id with this thread so that we will know not to launch it again
					CurrentThread.WorkerAppId = LaunchWorker(WorkingDirectory, ProcessId, ThreadId);
					bLaunchedOnce = TRUE;
				}
			}
			// Yield CPU time while we are waiting
			appSleep(0.01f);
		}
	}

	INT CurrentPosition = 0;
	const BYTE WorkerErrorOutputVersion = 0;
	BYTE ReadVersion;
	WorkerOutputReadValue(ReadVersion, CurrentPosition, WorkerOutput);
	EWorkerJobType OutJobType;
	WorkerOutputReadValue(OutJobType, CurrentPosition, WorkerOutput);
	if (OutJobType == WJT_WorkerError)
	{
		// The worker terminated with an error, read it out of the file and assert on it.
		check(ReadVersion == WorkerErrorOutputVersion);
		UINT ErrorStringSize;
		WorkerOutputReadValue(ErrorStringSize, CurrentPosition, WorkerOutput);
		TCHAR* ErrorBuffer = new TCHAR[ErrorStringSize / sizeof(TCHAR) + 1];
		WorkerOutputReadMemory(ErrorBuffer, ErrorStringSize, CurrentPosition, WorkerOutput);
		ErrorBuffer[ErrorStringSize / sizeof(TCHAR)] = 0;
		appErrorf(TEXT("%s for thread %u terminated with message: \n %s"), *ShaderCompileWorkerName, ThreadId, ErrorBuffer);
		delete [] ErrorBuffer;
	}

	// Delete the output file now that we have consumed it, to avoid reading stale data on the next compile loop.
	GFileManager->Delete(*OutputFileName, TRUE, TRUE);
}

/** Flushes all pending jobs for the given material. */
UBOOL FShaderCompilingThreadManager::FinishCompiling(TArray<FShaderCompileJob>& Results, const FString& InMaterialName, UBOOL bInSilent, UBOOL bInDebugDump)
{
	MaterialName = InMaterialName;
	bSilent = bInSilent;
	bDebugDump = bInDebugDump;

	ShaderCompileErrorCounter.Reset();
	
	// Calculate how many Threads we should use for compiling shaders
	INT NumShaderCompilingThreads = bAllowMultiThreadedShaderCompile ? Max<INT>(1,GNumHardwareThreads - NumUnusedShaderCompilingThreads) : 1;
	// Don't use multiple threads if we are dumping debug data or there are less jobs in the queue than the threshold.
	if (bDebugDump || (UINT)CompileQueue.Num() < ThreadedShaderCompileThreshold)
	{
		NumShaderCompilingThreads = 1;
	}

	NextShaderToProcess.Reset();

	bMultithreadedCompile = NumShaderCompilingThreads > 1;

	// Recreate threads if necessary
	if (bMultithreadedCompile && Threads.Num() != NumShaderCompilingThreads)
	{
		warnf(NAME_DevShaders, TEXT("Using multithreaded shader compiling with %u threads"), NumShaderCompilingThreads);
		// Stop the shader compiling threads
		// Signal the threads to break out of their loop
		KillThreadsCounter.Increment();
		for(INT ThreadIndex = 1;ThreadIndex < Threads.Num();ThreadIndex++)
		{
			// Wait for the thread to exit
			Threads(ThreadIndex).Thread->WaitForCompletion();
			// Report any unhandled exceptions by the thread
			Threads(ThreadIndex).CheckHealth();
			// Destroy the thread
			GThreadFactory->Destroy(Threads(ThreadIndex).Thread);
		}
		// All threads have terminated
		Threads.Empty();
		// Reset the kill counter
		KillThreadsCounter.Decrement();

		// Create the shader compiling threads
		for(INT ThreadIndex = 0;ThreadIndex < NumShaderCompilingThreads;ThreadIndex++)
		{
			const FString ThreadName = FString::Printf(TEXT("ShaderCompilingThread%i"), NextThreadId);
			FShaderCompileThreadRunnable* ThreadRunnable = new(Threads) FShaderCompileThreadRunnable(this);
			// Index 0 is the main thread, it does not need a runnable thread
			if (ThreadIndex > 0)
			{
				ThreadRunnable->Thread = GThreadFactory->CreateThread(ThreadRunnable, *ThreadName, 0, 0, 0, TPri_Normal);
			}
		}
	}

	// Signal to the Threads to start processing CompileQueue
	for(INT ThreadIndex = 1;ThreadIndex < NumShaderCompilingThreads;ThreadIndex++)
	{
		Threads(ThreadIndex).ActiveCounter.Increment();
	}

	STAT(DOUBLE RHICompileTime = 0);
	{
		SCOPE_SECONDS_COUNTER(RHICompileTime);
		// Enter the job loop with the main thread
		// When this returns, all jobs will have begun
		ThreadLoop(0);

		// Wait for the shader compiling threads to finish their jobs
		for(INT ThreadIndex = 1;ThreadIndex < Threads.Num();ThreadIndex++)
		{
			while (Threads(ThreadIndex).ActiveCounter.GetValue() > 0)
			{
				// Yield CPU time while waiting
				appSleep(0);
				Threads(ThreadIndex).CheckHealth();
			}
		}
	}
	INC_FLOAT_STAT_BY(STAT_ShaderCompiling_RHI,(FLOAT)RHICompileTime);

	if (CompileQueue.Num() > 0)
	{
		switch (CompileQueue(0).Target.Platform)
		{
			case SP_PS3:
				STAT(GRHIShaderCompileTime_PS3 += RHICompileTime);
				break;
			case SP_XBOXD3D:
				STAT(GRHIShaderCompileTime_XBOXD3D += RHICompileTime);
				break;
			case SP_PCD3D_SM2:
				STAT(GRHIShaderCompileTime_PCD3D_SM2 += RHICompileTime);
				break;
			case SP_PCD3D_SM3:
				STAT(GRHIShaderCompileTime_PCD3D_SM3 += RHICompileTime);
				break;
			case SP_PCD3D_SM4:
				STAT(GRHIShaderCompileTime_PCD3D_SM4 += RHICompileTime);
				break;
			default:
				break;
		}
		STAT(GRHIShaderCompileTime_Total += RHICompileTime);
	}

	NextThreadId = 0;
	Results = CompileQueue;
	CompileQueue.Empty();
	return TRUE;
}

/** Enqueues a shader compile job with GShaderCompilingThreadManager. */
void BeginCompileShader(
	FVertexFactoryType* VFType,
	FShaderType* ShaderType,
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	FShaderTarget Target,
	const FShaderCompilerEnvironment& InEnvironment
	)
{
	FShaderCompilerEnvironment Environment( InEnvironment );

	// #define PIXELSHADER and VERTEXSHADER accordingly
	{
		Environment.Definitions.Set( TEXT("PIXELSHADER"),  (Target.Frequency == SF_Pixel) ? TEXT("1") : TEXT("0") );
		Environment.Definitions.Set( TEXT("VERTEXSHADER"), (Target.Frequency == SF_Vertex) ? TEXT("1") : TEXT("0") );
	}

	if (!GShaderCompilingThreadManager)
	{
		GShaderCompilingThreadManager = new FShaderCompilingThreadManager();
	}

	// Create a new job and enqueue it with the shader compiling manager
	FShaderCompileJob NewJob(VFType, ShaderType, SourceFilename, FunctionName, Target, Environment);
	GShaderCompilingThreadManager->AddJob(NewJob);
}

FSimpleScopedTimer::FSimpleScopedTimer(const TCHAR* InInfoStr, FName InSuppressName) :
	InfoStr(InInfoStr),
	SuppressName(InSuppressName),
	bAlreadyStopped(FALSE)
{
	StartTime = appSeconds();
}

void FSimpleScopedTimer::Stop(UBOOL DisplayLog)
{
	if (!bAlreadyStopped)
	{
		bAlreadyStopped = TRUE;
		DOUBLE TimeElapsed = appSeconds() - StartTime;
		if (DisplayLog)
		{
			warnf((EName)SuppressName.GetIndex(), TEXT("		[%s] took [%.4f] s"),*InfoStr,TimeElapsed);
		}
	}
}

FSimpleScopedTimer::~FSimpleScopedTimer()
{
	Stop(TRUE);
}
