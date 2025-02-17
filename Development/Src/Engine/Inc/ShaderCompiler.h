/*=============================================================================
	ShaderCompiler.h: Platform independent shader compilation definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __SHADERCOMPILER_H__
#define __SHADERCOMPILER_H__

enum EShaderFrequency
{
	SF_Vertex			= 0,
	SF_Pixel			= 1,

	SF_NumBits			= 1
};

/** @warning: update ShaderPlatformToText and GetMaterialPlatform when the below changes */
enum EShaderPlatform
{
	SP_PCD3D_SM3		= 0,
	SP_PS3				= 1,
	SP_XBOXD3D			= 2,
	SP_PCD3D_SM2		= 3,
	SP_PCD3D_SM4		= 4,

	SP_NumPlatforms		= 5,
	SP_NumBits			= 3,
};

/**
 * Checks whether a shader platform supports hardware blending with FP render targets.
 * @param Platform - The shader platform to check.
 * @return TRUE if the platform supports hardware blending with FP render targets.
 */
extern UBOOL CanBlendWithFPRenderTarget(UINT Platform);

/**
 * Checks whether a shader platform supports the VFACE register.  If it doesn't support
 * it, two-sided lit materials need to be rendered twice.
 * @return TRUE if the platform supports the VFACE register.
 */
extern UBOOL CanAccessFacingRegister(UINT Platform);

/**
 * Converts shader platform to human readable string. 
 *
 * @param ShaderPlatform	Shader platform enum
 * @return text representation of enum
 */
extern const TCHAR* ShaderPlatformToText( EShaderPlatform ShaderPlatform );

/**
 * Material shader platforms represent the different versions of FMaterialResources 
 * that will have to be stored for a single UMaterial.  Note that multiple shader platforms 
 * can map to a single material platform.
 */
enum EMaterialShaderPlatform
{
	MSP_BASE				=0,
	MSP_SM3                 =0,
	MSP_SM2                 =1,
	MSP_MAX                 =2
};

/** The current material platform that corresponds to GRHIShaderPlatform */
extern EMaterialShaderPlatform GCurrentMaterialPlatform;

/**
 * Gets the material platform matching the input shader platform
 */
FORCEINLINE EMaterialShaderPlatform GetMaterialPlatform(EShaderPlatform ShaderPlatform)
{
	//MaterialPlatformTable needs to be updated if SP_NumPlatforms changes
	checkSlow(SP_NumPlatforms == 5);
	static const EMaterialShaderPlatform MaterialPlatformTable[] = { MSP_SM3, MSP_SM3, MSP_SM3, MSP_SM2, MSP_SM3 };
	checkSlow(ShaderPlatform >= 0 && ShaderPlatform < SP_NumPlatforms);
	return MaterialPlatformTable[ShaderPlatform];
}

struct FShaderTarget
{
	BITFIELD Frequency : SF_NumBits;
	BITFIELD Platform : SP_NumBits;
};

enum ECompilerFlags
{
	CFLAG_PreferFlowControl = 0,
	CFLAG_Debug,
	CFLAG_AvoidFlowControl,
	/** Disable shader validation */
	CFLAG_SkipValidation
};

/**
 * A map of shader parameter names to registers allocated to that parameter.
 */
class FShaderParameterMap
{
public:
	UBOOL FindParameterAllocation(const TCHAR* ParameterName,WORD& OutBufferIndex,WORD& OutBaseIndex,WORD& OutSize,WORD& OutSamplerIndex) const;
	void AddParameterAllocation(const TCHAR* ParameterName,WORD BufferIndex,WORD BaseIndex,WORD Size,WORD SamplerIndex);
	/** Checks that all parameters are bound and asserts if any aren't in a debug build */
	void VerifyBindingsAreComplete(const TCHAR* ShaderTypeName, EShaderFrequency Frequency) const;

	DWORD GetCRC() const
	{
		DWORD ParameterCRC = 0;
		for(TMap<FString,FParameterAllocation>::TConstIterator ParameterIt(ParameterMap);ParameterIt;++ParameterIt)
		{
			const FString& ParamName = ParameterIt.Key();
			const FParameterAllocation& ParamValue = ParameterIt.Value();
			ParameterCRC = appMemCrc(*ParamName, ParamName.Len() * sizeof(TCHAR), ParameterCRC);
			ParameterCRC = appMemCrc(&ParamValue, sizeof(ParamValue), ParameterCRC);
		}
		return ParameterCRC;
	}

private:
	struct FParameterAllocation
	{
		WORD BufferIndex;
		WORD BaseIndex;
		WORD Size;
		WORD SamplerIndex;
		mutable UBOOL bBound;

		FParameterAllocation() :
			bBound(FALSE)
		{}
	};

	// Verify that FParameterAllocation does not contain any padding, which would cause FShaderParameterMap::GetCRC to operate on garbage data
	checkAtCompileTime(sizeof(FParameterAllocation) == sizeof(WORD) * 4 + sizeof(UBOOL), FParameterAllocationContainsPadding);

	TMap<FString,FParameterAllocation> ParameterMap;
};

/**
 * The environment used to compile a shader.
 */
struct FShaderCompilerEnvironment
{
	TMap<FString,FString> IncludeFiles;
	TMap<FName,FString> Definitions;
	TArray<ECompilerFlags> CompilerFlags;
};

/**
 * The output of the shader compiler.
 */
struct FShaderCompilerOutput
{
	FShaderCompilerOutput() :
		NumInstructions(0)
	{}

	FShaderParameterMap ParameterMap;
	TArray<FString> Errors;
	FShaderTarget Target;
	TArray<BYTE> Code;
	UINT NumInstructions;
};

/**
 * Loads the shader file with the given name.
 * @return The contents of the shader file.
 */
extern FString LoadShaderSourceFile(const TCHAR* Filename);

/** Enqueues a shader compile job with GShaderCompilingThreadManager. */
extern void BeginCompileShader(
	class FVertexFactoryType* VFType,
	class FShaderType* ShaderType,
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	FShaderTarget Target,
	const FShaderCompilerEnvironment& Environment
	);

/** Stores all of the input and output information used to compile a single shader. */
class FShaderCompileJob
{
public:
	/** Vertex factory type that this shader belongs to, may be NULL */
	FVertexFactoryType* VFType;
	/** Shader type that this shader belongs to, must be valid */
	FShaderType* ShaderType;
	/** Input for the shader compile */
	FString SourceFilename;
	FString FunctionName;
	FShaderTarget Target;
	FShaderCompilerEnvironment Environment;
	/** Output of the shader compile */
	UBOOL bSucceeded;
	FShaderCompilerOutput Output;

	FShaderCompileJob(
		FVertexFactoryType* InVFType,
		FShaderType* InShaderType,
		const TCHAR* InSourceFilename,
		const TCHAR* InFunctionName,
		FShaderTarget InTarget,
		const FShaderCompilerEnvironment& InEnvironment) 
		:
		VFType(InVFType),
		ShaderType(InShaderType),
		SourceFilename(InSourceFilename),
		FunctionName(InFunctionName),
		Target(InTarget),
		Environment(InEnvironment),
		bSucceeded(FALSE)
	{
	}
};

/** Shader compiling thread, managed by FShaderCompilingThreadManager */
class FShaderCompileThreadRunnable : public FRunnable
{
	friend class FShaderCompilingThreadManager;
private:
	/** The manager for this thread */
	class FShaderCompilingThreadManager* Manager;
	/** The runnable thread */
	FRunnableThread* Thread;
	/** 
	 * Counter used to suspend and activate compilation.  
	 * The main thread increments this when Manager->CompileQueue can be processed by the thread, 
	 * and the thread decrements it when it is finished. 
	 */
	FThreadSafeCounter ActiveCounter;
	/** Process Id of the worker application associated with this thread */
	DWORD WorkerAppId;
	/** Unique Id assigned by Manager and the index of this thread into Manager->Threads */
	UINT ThreadId;
	/** If the thread has been terminated by an unhandled exception, this contains the error message. */
	FString ErrorMessage;
	/** TRUE if the thread has been terminated by an unhandled exception. */
	UBOOL bTerminatedByError;
	/** Indicates whether shaders have been copied to the working directory for this run */
	UBOOL bCopiedShadersToWorkingDirectory;

public:
	/** Initialization constructor. */
	FShaderCompileThreadRunnable(class FShaderCompilingThreadManager* InManager);

	// FRunnable interface.
	virtual UBOOL Init(void) { return TRUE; }
	virtual void Exit(void) {}
	virtual void Stop(void) {}
	virtual DWORD Run(void);

	/** Checks the thread's health, and passes on any errors that have occured.  Called by the main thread. */
	void CheckHealth() const;
};

enum EWorkerJobType
{
	WJT_D3D9Shader = 0,
	WJT_D3D10Shader,
	WJT_XenonShader,
	WJT_PS3Shader,
	WJT_WorkerError,
	WJT_JobTypeMax
};

/** Helper function that writes an arbitrary typed value to a buffer */
template<class T>
void WorkerInputAppendValue(T& Value, TArray<BYTE>& Buffer)
{
	INT OldNum = Buffer.Add(sizeof(Value));
	appMemcpy(&Buffer(OldNum), &Value, sizeof(Value));
}

/** Helper function that appends memory to a buffer */
void WorkerInputAppendMemory(const void* Ptr, INT Size, TArray<BYTE>& Buffer);

/** Helper function that reads an arbitrary typed value from a buffer */
template<class T>
void WorkerOutputReadValue(T& Value, INT& CurrentPosition, const TArray<BYTE>& Buffer)
{
	appMemcpy(&Value, &Buffer(CurrentPosition), sizeof(Value));
	CurrentPosition += sizeof(Value);
}

/** Helper function that reads memory from a buffer */
void WorkerOutputReadMemory(void* Dest, UINT Size, INT& CurrentPosition, const TArray<BYTE>& Buffer);

/** Manages parallel shader compilation */
class FShaderCompilingThreadManager
{
	friend class FShaderCompileThreadRunnable;
private:

	/** Job queue, flushed by FinishCompiling.  Threads can only read from this when their ActiveCounter is incremented by the main thread. */
	TArray<FShaderCompileJob> CompileQueue;
	/** The next index into CompileQueue which processing hasn't started for yet. */
	FThreadSafeCounter NextShaderToProcess;
	/** Incremented by the main thread, Indicates to all threads that they must exit. */
	FThreadSafeCounter KillThreadsCounter;
	/** The threads spawned for shader compiling. */
	TIndirectArray<FShaderCompileThreadRunnable> Threads;
	/** Counter used to give each thread a unique Id. */
	UINT NextThreadId;
	/** Number of hardware threads that should not be used by shader compiling. */
	UINT NumUnusedShaderCompilingThreads;
	/** If there are less jobs than this, shader compiling will not use multiple threads. */
	UINT ThreadedShaderCompileThreshold;
	/** Whether to allow multi threaded shader compiling. */
	UBOOL bAllowMultiThreadedShaderCompile;
	/** TRUE if the current compile is multi threaded. */
	UBOOL bMultithreadedCompile;
	/** TRUE if the current compile should not emit warnings. */
	UBOOL bSilent;
	/** TRUE if the current compile is dumping debug shader data.  This forces the single-threaded compile path. */
	UBOOL bDebugDump;
	/** TRUE if the current compile should dump UPDB's.  This is compatible with the multi-threaded compile path. */
	UBOOL bDumpShaderPDBs;
	/** Incremented by each thread that finds a shader compile error, used to break out of compiling. */
	FThreadSafeCounter ShaderCompileErrorCounter;
	/** Name of the material of the current compile. */
	FString MaterialName;
	/** Name of the shader worker application. */
	const FString ShaderCompileWorkerName;

	/** Launches the worker, returns the launched Process Id. */
	DWORD LaunchWorker(const FString& WorkingDirectory, DWORD ProcessId, UINT ThreadId);

	/** Processing loop shared by all threads.  The Main thread has an Id of 0. */
	void ThreadLoop(UINT CurrentThreadId);

public:
	
	FShaderCompilingThreadManager();

	/** Returns TRUE if the current job is multi threaded. */
	UBOOL IsMultiThreadedCompile() const { return bMultithreadedCompile; }

	/** Returns TRUE if the current job should dump shader PDBs. */
	UBOOL IsDumpingShaderPDBs() const { return bDumpShaderPDBs; }

	FString GetShaderPDBPath() const;

	/** Adds a job to the compile queue, called from the main thread. */
	void AddJob(const FShaderCompileJob& NewJob);

	/** Compiles a single job through the worker application, called from all the threads. */
	void WorkerCompile(UINT ThreadId, EWorkerJobType JobType, TArray<BYTE>& WorkerInput, TArray<BYTE>& WorkerOutput);

	/** Flushes all pending jobs for the given material, called from the main thread. */
	UBOOL FinishCompiling(TArray<FShaderCompileJob>& Results, const FString& MaterialName, UBOOL bSilent, UBOOL bDebugDump);
};

/** The global shader compiling thread manager. */
extern FShaderCompilingThreadManager* GShaderCompilingThreadManager;

/** The shader precompilers for each platform.  These are only set during the console shader compilation while cooking or in the PrecompileShaders commandlet. */
extern class FConsoleShaderPrecompiler* GConsoleShaderPrecompilers[SP_NumPlatforms];

enum EShaderCompilingStats
{
	STAT_ShaderCompiling_MaterialShaders = STAT_ShaderCompilingFirstStat,
	STAT_ShaderCompiling_GlobalShaders,
	STAT_ShaderCompiling_RHI,
	STAT_ShaderCompiling_LoadingShaderFiles,
	STAT_ShaderCompiling_CRCingShaderFiles,
	STAT_ShaderCompiling_HLSLTranslation,
	STAT_ShaderCompiling_NumTotalMaterialShaders,
	STAT_ShaderCompiling_NumSpecialMaterialShaders,
	STAT_ShaderCompiling_NumTerrainMaterialShaders,
	STAT_ShaderCompiling_NumDecalMaterialShaders,
	STAT_ShaderCompiling_NumParticleMaterialShaders,
	STAT_ShaderCompiling_NumSkinnedMaterialShaders,
	STAT_ShaderCompiling_NumLitMaterialShaders,
	STAT_ShaderCompiling_NumUnlitMaterialShaders,
	STAT_ShaderCompiling_NumTransparentMaterialShaders,
	STAT_ShaderCompiling_NumOpaqueMaterialShaders,
	STAT_ShaderCompiling_NumMaskedMaterialShaders,
	STAT_ShaderCompiling_NumShadersInitialized
};

/** A simple timer which prints the time that an instance was in scope. */
class FSimpleScopedTimer
{
public:
	FSimpleScopedTimer(const TCHAR* InInfoStr, FName InSuppressName);
	void Stop(UBOOL DisplayLog = TRUE);
	~FSimpleScopedTimer();

private:
	DOUBLE StartTime;
	FString InfoStr;
	FName SuppressName;
	UBOOL bAlreadyStopped;
};

/** A simple thread-safe accumulate counter that times how long an instance was in scope. */
class FSimpleScopedAccumulateCycleTimer
{
public:
	FSimpleScopedAccumulateCycleTimer(DWORD& InCounter) :
	  Counter(InCounter)
	  {
		  StartTime = appCycles();
	  }

	  ~FSimpleScopedAccumulateCycleTimer()
	  {
		  appInterlockedAdd((INT*)&Counter, appCycles() - StartTime);
	  }

private:
	DWORD StartTime;
	DWORD& Counter;
};

#endif // __SHADERCOMPILER_H__
