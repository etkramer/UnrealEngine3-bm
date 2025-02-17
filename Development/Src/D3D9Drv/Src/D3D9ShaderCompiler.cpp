/*=============================================================================
	D3D9ShaderCompiler.cpp: D3D shader compiler implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D9DrvPrivate.h"

/**
 * An implementation of the D3DX include interface to access a FShaderCompilerEnvironment.
 */
class FD3DIncludeEnvironment : public ID3DXInclude
{
public:

	STDMETHOD(Open)(D3DXINCLUDE_TYPE Type,LPCSTR Name,LPCVOID ParentData,LPCVOID* Data,UINT* Bytes)
	{
		FString Filename(ANSI_TO_TCHAR(Name));
		FString FileContents;

		FString* OverrideContents = Environment.IncludeFiles.Find(*Filename);
		if(OverrideContents)
		{
			FileContents = *OverrideContents;
		}
		else
		{
			FileContents = LoadShaderSourceFile(*Filename);
		}

		// Convert the file contents to ANSI.
		ANSICHAR* AnsiFileContents = new ANSICHAR[FileContents.Len() + 1];
		appStrncpyANSI( AnsiFileContents, TCHAR_TO_ANSI(*FileContents), FileContents.Len()+1 );

		// Write the result to the output parameters.
		*Data = (LPCVOID)AnsiFileContents;
		*Bytes = FileContents.Len();

		return D3D_OK;
	}

	STDMETHOD(Close)(LPCVOID Data)
	{
		delete Data;
		return D3D_OK;
	}

	FD3DIncludeEnvironment(const FShaderCompilerEnvironment& InEnvironment):
		Environment(InEnvironment)
	{}

private:

	FShaderCompilerEnvironment Environment;
};

/**
 * TranslateCompilerFlag - translates the platform-independent compiler flags into D3DX defines
 * @param CompilerFlag - the platform-independent compiler flag to translate
 * @return DWORD - the value of the appropriate D3DX enum
 */
static DWORD TranslateCompilerFlag(ECompilerFlags CompilerFlag)
{
	switch(CompilerFlag)
	{
	case CFLAG_PreferFlowControl: return D3DXSHADER_PREFER_FLOW_CONTROL;
	case CFLAG_Debug: return D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
	case CFLAG_AvoidFlowControl: return D3DXSHADER_AVOID_FLOW_CONTROL;
	case CFLAG_SkipValidation: return D3DXSHADER_SKIPVALIDATION;
	default: return 0;
	};
}

/**
 * D3D9CreateShaderCompileCommandLine - takes shader parameters used to compile with the DX9
 * compiler and returns a fxc command to compile from the command line
 */
static FString D3D9CreateShaderCompileCommandLine(
	const FString& ShaderPath, 
	const FString& IncludePath, 
	const TCHAR* EntryFunction, 
	const TCHAR* ShaderProfile, 
	D3DXMACRO *Macros,
	DWORD CompileFlags
	)
{
	// fxc is our command line compiler
	// make sure we are using the DX SDK fxc and not the XDK one
	// surround with quotes since the DX_SDKDIR path probably has spaces
	FString FXCCommandline = FString(TEXT("\"%DXSDK_DIR%\\Utilities\\Bin\\x86\\fxc\" ")) + ShaderPath;

	// add definitions
	if(Macros != NULL)
	{
		for (int i = 0; Macros[i].Name != NULL; i++)
		{
			FXCCommandline += FString(TEXT(" /D ")) + ANSI_TO_TCHAR(Macros[i].Name) + TEXT("=") + ANSI_TO_TCHAR(Macros[i].Definition);
		}
	}

	// add the entry point reference
	FXCCommandline += FString(TEXT(" /E ")) + EntryFunction;

	// add the include path
	FXCCommandline += FString(TEXT(" /I ")) + IncludePath;

	// go through and add other switches
	if(CompileFlags & D3DXSHADER_PREFER_FLOW_CONTROL)
	{
		CompileFlags &= ~D3DXSHADER_PREFER_FLOW_CONTROL;
		FXCCommandline += FString(TEXT(" /Gfp"));
	}
	if(CompileFlags & D3DXSHADER_DEBUG)
	{
		CompileFlags &= ~D3DXSHADER_DEBUG;
		FXCCommandline += FString(TEXT(" /Zi"));
	}
	if(CompileFlags & D3DXSHADER_SKIPOPTIMIZATION)
	{
		CompileFlags &= ~D3DXSHADER_SKIPOPTIMIZATION;
		FXCCommandline += FString(TEXT(" /Od"));
	}
	if(CompileFlags & D3DXSHADER_AVOID_FLOW_CONTROL)
	{
		CompileFlags &= ~D3DXSHADER_AVOID_FLOW_CONTROL;
		FXCCommandline += FString(TEXT(" /Gfa"));
	}
	if(CompileFlags & D3DXSHADER_SKIPVALIDATION)
	{
		CompileFlags &= ~D3DXSHADER_SKIPVALIDATION;
		FXCCommandline += FString(TEXT(" /Vd"));
	}
	checkf(CompileFlags == 0, TEXT("Unhandled d3d9 shader compiler flag!"));

	// add the target instruction set
	FXCCommandline += FString(TEXT(" /T ")) + ShaderProfile;

	// add a pause on a newline
	FXCCommandline += FString(TEXT(" \r\n pause"));
	return FXCCommandline;
}

/**
 * Uses D3DX to PreProcess a shader (resolve all #includes and #defines) and dumps it out for debugging
 */
static void D3D9PreProcessShader(
	const TCHAR* SourceFilename,
	const FString& SourceFile,
	const TArray<D3DXMACRO>& Macros,
	FD3DIncludeEnvironment& IncludeEnvironment,
	const TCHAR* ShaderPath
	)
{
	TRefCountPtr<ID3DXBuffer> ShaderText;
	TRefCountPtr<ID3DXBuffer> PreProcessErrors;

	HRESULT PreProcessHR = D3DXPreprocessShader(
		TCHAR_TO_ANSI(*SourceFile),
		SourceFile.Len(),			
		&Macros(0),
		&IncludeEnvironment,
		ShaderText.GetInitReference(),
		PreProcessErrors.GetInitReference());

	if(FAILED(PreProcessHR))
	{
		warnf( NAME_Warning, TEXT("Preprocess failed for shader %s: %s"), SourceFilename, ANSI_TO_TCHAR(PreProcessErrors->GetBufferPointer()) );
	}
	else
	{
		appSaveStringToFile(
			FString(ANSI_TO_TCHAR(ShaderText->GetBufferPointer())), 
			*(FString(ShaderPath) * FString(SourceFilename) + TEXT(".pre")));
	}
}

/**
 * Filters out unwanted shader compile warnings
 */
static void D3D9FilterShaderCompileWarnings(const FString& CompileWarnings, TArray<FString>& FilteredWarnings)
{
	TArray<FString> WarningArray;
	FString OutWarningString = TEXT("");
	CompileWarnings.ParseIntoArray(&WarningArray, TEXT("\n"), TRUE);
	
	//go through each warning line
	for (INT WarningIndex = 0; WarningIndex < WarningArray.Num(); WarningIndex++)
	{
		//suppress "warning X3557: Loop only executes for 1 iteration(s), forcing loop to unroll"
		if (WarningArray(WarningIndex).InStr(TEXT("X3557")) == INDEX_NONE)
		{
			FilteredWarnings.AddUniqueItem(WarningArray(WarningIndex));
		}
	}
}

/**
 * This wraps D3DXCompileShader in a __try __except block to catch occasional crashes in the function.
 */
static HRESULT D3D9SafeCompileShader(
	LPCSTR pSrcData,
	UINT srcDataLen,
	TArray<D3DXMACRO>& Defines,
	INT AttributeMacroIndex,
	LPD3DXINCLUDE pInclude,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	DWORD Flags,
	LPD3DXBUFFER* ppShader,
	LPD3DXBUFFER* ppErrorMsgs,
	LPD3DXCONSTANTTABLE * ppConstantTable,
	const TCHAR* DebugName
	)
{
	// Reset the COMPILER_SUPPORTS_ATTRIBUTES definition to true since this function may be called multiple times on the same shader
	D3DXMACRO* MacroCompilerSupportsAttributes = &Defines(AttributeMacroIndex);
	delete MacroCompilerSupportsAttributes->Definition;
	MacroCompilerSupportsAttributes->Definition = appStrcpyANSI(new ANSICHAR[2], 2, "1");
	__try
	{
		return D3DXCompileShader(
			pSrcData,
			srcDataLen,			
			&Defines(0),
			pInclude,
			pFunctionName,
			pProfile,
			Flags,
			ppShader,
			ppErrorMsgs,
			ppConstantTable
			);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// If the first try to compile the shader crashed, try a second time.
		warnf(NAME_Warning,TEXT("D3DXCompileShader threw exception while compiling %s!  Trying a second time..."),DebugName);

		__try
		{
			return D3DXCompileShader(
				pSrcData,
				srcDataLen,			
				&Defines(0),
				pInclude,
				pFunctionName,
				pProfile,
				Flags,
				ppShader,
				ppErrorMsgs,
				ppConstantTable
				);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			// If the second try failed, try using the legacy shader compiler.
			warnf(NAME_Warning,TEXT("Second call to D3DXCompileShader also threw an exception!  Trying with the legacy shader compiler..."));
			
			// Set the COMPILER_SUPPORTS_ATTRIBUTES definition to false, since the legacy shader compiler didn't support them.
			delete MacroCompilerSupportsAttributes->Definition;
			MacroCompilerSupportsAttributes->Definition = appStrcpyANSI(new ANSICHAR[2], 2, "0");
			__try
			{
				return D3DXCompileShader(
					pSrcData,
					srcDataLen,			
					&Defines(0),
					pInclude,
					pFunctionName,
					pProfile,
					Flags | D3DXSHADER_USE_LEGACY_D3DX9_31_DLL,
					ppShader,
					ppErrorMsgs,
					ppConstantTable
					);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				// If the legacy shader compiler crashed, return a generic error.
				warnf(NAME_Warning,TEXT("The legacy shader compiler also failed!  Giving up on this shader."));
				return E_FAIL;
			}
		}
	}
}

/** Wraps D3DXDisassembleShader in a __try __except block to catch unpredictable crashes in the function. */
static HRESULT D3D9SafeDisassembleShader(
	CONST DWORD * pShader,
	BOOL EnableColorCode,
	LPCSTR pComments,
	LPD3DXBUFFER* ppDisassembly
	)
{
	__try
	{
		return D3DXDisassembleShader(pShader, EnableColorCode, pComments, ppDisassembly);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		warnf(NAME_Warning, TEXT("Caught exception from D3DXDisassembleShader: pShader=%x"), pShader);
		return -1;
	}
}

/** Wraps GetConstantDesc in a __try __except block to catch unpredictable crashes in the function. */
static HRESULT D3D9SafeGetConstantDesc(
	ID3DXConstantTable* ConstantTable,
	D3DXHANDLE ConstantHandle,
	D3DXCONSTANT_DESC* ConstantDesc,
	UINT* NumConstants
	)
{
	__try
	{
		return ConstantTable->GetConstantDesc(ConstantHandle,ConstantDesc,NumConstants);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		warnf(NAME_Warning, TEXT("Caught exception from ConstantTable->GetConstantDesc()"));
		return -1;
	}
}

/** A wrapper for D3DXCONSTANT_DESC with an embedded copy of the name, so it can be serialized. */
#pragma pack(push,4)
struct FD3D9NamedConstantDesc
{
	enum { MaxNameLength = 128 };

	char Name[MaxNameLength];
	D3DXCONSTANT_DESC ConstantDesc;
};
#pragma pack(pop)

/** Compiles a D3D9 shader through D3DXCompileShader */
static UBOOL D3D9CompileShaderThroughDll(
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	const TCHAR* ShaderProfile,
	DWORD CompileFlags,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output,
	TArray<D3DXMACRO>& Macros,
	INT AttributeMacroIndex,
	TArray<FD3D9NamedConstantDesc>& Constants,
	FString& DisassemblyString,
	UBOOL bSilent = FALSE
	)
{
	const FString SourceFile = LoadShaderSourceFile(SourceFilename);

	TRefCountPtr<ID3DXBuffer> Shader;
	TRefCountPtr<ID3DXBuffer> Errors;
	FD3DIncludeEnvironment IncludeEnvironment(Environment);

	//temporarily turn off d3dx debug messages, since we will log them if there is an error
	//this means warnings will be suppressed unless there was a compile error
	D3DXDebugMute(TRUE);

	TRefCountPtr<ID3DXConstantTable> ConstantTable;
	HRESULT Result = D3D9SafeCompileShader(
		TCHAR_TO_ANSI(*SourceFile),
		SourceFile.Len(),			
		Macros,
		AttributeMacroIndex,
		&IncludeEnvironment,
		TCHAR_TO_ANSI(FunctionName),
		TCHAR_TO_ANSI(ShaderProfile),
		CompileFlags,
		Shader.GetInitReference(),
		Errors.GetInitReference(),
		ConstantTable.GetInitReference(),
		*FString::Printf(TEXT("%s/%s for profile %s"),SourceFilename,FunctionName,ShaderProfile)
		);

	//restore d3dx debug messages
	D3DXDebugMute(FALSE);

	if(FAILED(Result))
	{
		// Copy the error text to the output.
		void* ErrorBuffer = Errors ? Errors->GetBufferPointer() : NULL;
		if (ErrorBuffer)
		{
			D3D9FilterShaderCompileWarnings(ANSI_TO_TCHAR(ErrorBuffer), Output.Errors);
		}
		else
		{
			Output.Errors.AddItem(TEXT("Compile Failed without warnings!"));
		}

		if (!bSilent)
		{
			// Log the compilation errors
			for (INT ErrorIndex = 0; ErrorIndex < Output.Errors.Num(); ErrorIndex++)
			{
				warnf(NAME_Warning, TEXT("Shader compile error: %s"), *Output.Errors(ErrorIndex));
			}
		}

		return FALSE;
	}
	else
	{
		UINT NumShaderBytes = Shader->GetBufferSize();
		Output.Code.Empty(NumShaderBytes);
		Output.Code.Add(NumShaderBytes);
		appMemcpy(&Output.Code(0),Shader->GetBufferPointer(),NumShaderBytes);

		// Read the constant table output from the shader bytecode.
		D3DXCONSTANTTABLE_DESC ConstantTableDesc;
		VERIFYD3D9RESULT(ConstantTable->GetDesc(&ConstantTableDesc));

		// Read the constant descriptions out of the shader bytecode.
		for(UINT ConstantIndex = 0;ConstantIndex < ConstantTableDesc.Constants;ConstantIndex++)
		{
			// Read the constant description.
			D3DXHANDLE ConstantHandle = ConstantTable->GetConstant(NULL,ConstantIndex);
			D3DXCONSTANT_DESC ConstantDesc;
			UINT NumConstants = 1;
			VERIFYD3D9RESULT(D3D9SafeGetConstantDesc(ConstantTable, ConstantHandle, &ConstantDesc, &NumConstants));

			// Copy the constant and its name into a self-contained data structure, and add it to the constant array.
			FD3D9NamedConstantDesc NamedConstantDesc;
			strncpy_s(NamedConstantDesc.Name,FD3D9NamedConstantDesc::MaxNameLength,ConstantDesc.Name,_TRUNCATE);
			NamedConstantDesc.ConstantDesc = ConstantDesc;
			Constants.AddItem(NamedConstantDesc);
		}

		// Disassemble the shader to determine the instruction count.
		TRefCountPtr<ID3DXBuffer> DisassemblyBuffer;
		VERIFYD3D9RESULT(D3D9SafeDisassembleShader((const DWORD*)Shader->GetBufferPointer(),FALSE,NULL,DisassemblyBuffer.GetInitReference()));
		DisassemblyString = ANSI_TO_TCHAR((ANSICHAR*)DisassemblyBuffer->GetBufferPointer());

		return TRUE;
	}
}

/** Compiles a D3D9 shader through a worker application */
static UBOOL D3D9CompileShaderThroughWorker(
	UINT ThreadId,
	const TCHAR* SourceFilename,
	LPCSTR FunctionName,
	LPCSTR ShaderProfile,
	LPCSTR IncludePath,
	DWORD CompileFlags,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output,
	TArray<D3DXMACRO>& Macros,
	TArray<FD3D9NamedConstantDesc>& Constants,
	FString& DisassemblyString,
	UBOOL bSilent = FALSE
	)
{
	TArray<BYTE> WorkerInput;
	// Presize to avoid lots of allocations
	WorkerInput.Empty(1000);
	// Setup the input for the worker app, everything that is needed to compile the shader.
	// Note that any format changes here also need to be done in the worker
	// Write a job type so the worker app can know which compiler to invoke
	EWorkerJobType JobType = WJT_D3D9Shader;
	WorkerInputAppendValue(JobType, WorkerInput);
	// Version number so we can detect stale data
	const BYTE D3D9ShaderCompileWorkerInputVersion = 0;
	WorkerInputAppendValue(D3D9ShaderCompileWorkerInputVersion, WorkerInput);
	const FString SourceFile = LoadShaderSourceFile(SourceFilename);
	WorkerInputAppendMemory(TCHAR_TO_ANSI(*SourceFile), SourceFile.Len(), WorkerInput);
	WorkerInputAppendMemory(FunctionName, appStrlen(FunctionName) * sizeof(CHAR), WorkerInput);
	WorkerInputAppendMemory(ShaderProfile, appStrlen(ShaderProfile) * sizeof(CHAR), WorkerInput);
	WorkerInputAppendValue(CompileFlags, WorkerInput);
	WorkerInputAppendMemory(IncludePath, appStrlen(IncludePath) * sizeof(CHAR), WorkerInput);

	INT NumIncludes = Environment.IncludeFiles.Num();
	WorkerInputAppendValue(NumIncludes, WorkerInput);

	for(TMap<FString,FString>::TConstIterator IncludeIt(Environment.IncludeFiles); IncludeIt; ++IncludeIt)
	{
		const FString& IncludeName = IncludeIt.Key();
		WorkerInputAppendMemory(TCHAR_TO_ANSI(*IncludeName), IncludeName.Len(), WorkerInput);

		const FString& IncludeFile = IncludeIt.Value();
		WorkerInputAppendMemory(TCHAR_TO_ANSI(*IncludeFile), IncludeFile.Len(), WorkerInput);
	}

	INT NumMacros = Macros.Num() - 1;
	WorkerInputAppendValue(NumMacros, WorkerInput);

	for (INT MacroIndex = 0; MacroIndex < Macros.Num() - 1; MacroIndex++)
	{
		D3DXMACRO CurrentMacro = Macros(MacroIndex);
		check( CurrentMacro.Name);
		WorkerInputAppendMemory(CurrentMacro.Name, appStrlen(CurrentMacro.Name), WorkerInput);
		WorkerInputAppendMemory(CurrentMacro.Definition, appStrlen(CurrentMacro.Definition), WorkerInput);
	}

	TArray<BYTE> WorkerOutput;
	// Invoke the worker, this will return when the shader has been compiled
	GShaderCompilingThreadManager->WorkerCompile(ThreadId, JobType, WorkerInput, WorkerOutput);

	INT CurrentPosition = 0;
	const BYTE D3D9ShaderCompileWorkerOutputVersion = 0;
	// Read the worker output in the same format that it was written
	BYTE ReadVersion;
	WorkerOutputReadValue(ReadVersion, CurrentPosition, WorkerOutput);
	check(ReadVersion == D3D9ShaderCompileWorkerOutputVersion);
	EWorkerJobType OutJobType;
	WorkerOutputReadValue(OutJobType, CurrentPosition, WorkerOutput);
	check(OutJobType == JobType);
	HRESULT CompileResult;
	WorkerOutputReadValue(CompileResult, CurrentPosition, WorkerOutput);
	UINT ByteCodeLength;
	WorkerOutputReadValue(ByteCodeLength, CurrentPosition, WorkerOutput);
	Output.Code.Empty(ByteCodeLength);
	Output.Code.Add(ByteCodeLength);
	if (ByteCodeLength > 0)
	{
		WorkerOutputReadMemory(&Output.Code(0), Output.Code.Num(), CurrentPosition, WorkerOutput);
	}
	UINT ErrorStringLength;
	WorkerOutputReadValue(ErrorStringLength, CurrentPosition, WorkerOutput);
	
	FString ErrorString;
	if (ErrorStringLength > 0)
	{
		ANSICHAR* ErrorBuffer = new ANSICHAR[ErrorStringLength + 1];
		WorkerOutputReadMemory(ErrorBuffer, ErrorStringLength, CurrentPosition, WorkerOutput);
		ErrorBuffer[ErrorStringLength] = 0;
		ErrorString = FString(ANSI_TO_TCHAR(ErrorBuffer));
		delete [] ErrorBuffer;
	}	

	UINT ConstantArrayLength;
	WorkerOutputReadValue(ConstantArrayLength,CurrentPosition,WorkerOutput);

	if(ConstantArrayLength > 0)
	{
		const INT NumConstants = ConstantArrayLength / sizeof(FD3D9NamedConstantDesc);
		Constants.Empty(NumConstants);
		Constants.Add(NumConstants);
		WorkerOutputReadMemory(&Constants(0),ConstantArrayLength,CurrentPosition,WorkerOutput);
	}

	UINT DisassemblyStringLength;
	WorkerOutputReadValue(DisassemblyStringLength, CurrentPosition, WorkerOutput);

	if (DisassemblyStringLength > 0)
	{
		ANSICHAR* DisassemblyStringAnsi = new ANSICHAR[DisassemblyStringLength + 1];
		WorkerOutputReadMemory(DisassemblyStringAnsi, DisassemblyStringLength, CurrentPosition, WorkerOutput);
		DisassemblyStringAnsi[DisassemblyStringLength] = 0;
		DisassemblyString = FString(ANSI_TO_TCHAR(DisassemblyStringAnsi));
		delete [] DisassemblyStringAnsi;
	}	

	if (FAILED(CompileResult))
	{
		// Copy the error text to the output.
		if (ErrorString.Len() > 0)
		{
			D3D9FilterShaderCompileWarnings(ErrorString, Output.Errors);
		}
		else
		{
			Output.Errors.AddItem(TEXT("Compile Failed without warnings!"));
		}

		if (!bSilent)
		{
			// Log the compilation errors
			for (INT ErrorIndex = 0; ErrorIndex < Output.Errors.Num(); ErrorIndex++)
			{
				warnf(NAME_Warning, TEXT("Shader compile error: %s"), *Output.Errors(ErrorIndex));
			}
		}

		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/**
 * The D3DX/HLSL shader compiler.
 */ 
UBOOL D3D9CompileShader(
	UINT ThreadId,
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	FShaderTarget Target,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output,
	UBOOL bSilent = FALSE,
	UBOOL bDebugDump = FALSE,
	const TCHAR* ShaderSubDir = NULL
	)
{
	// ShaderSubDir must be valid if we are dumping debug shader data
	checkSlow(!bDebugDump || ShaderSubDir != NULL);
	// Must not be doing a multithreaded compile if we are dumping debug shader data
	checkSlow(!bDebugDump || !GShaderCompilingThreadManager->IsMultiThreadedCompile());

	TArray<FD3D9NamedConstantDesc> Constants;
	FString DisassemblyString;

	// Translate the input environment's definitions to D3DXMACROs.
	TArray<D3DXMACRO> Macros;
	for(TMap<FName,FString>::TConstIterator DefinitionIt(Environment.Definitions);DefinitionIt;++DefinitionIt)
	{
		FString Name = DefinitionIt.Key().ToString();
		FString Definition = DefinitionIt.Value();

		D3DXMACRO* Macro = new(Macros) D3DXMACRO;
		Macro->Name = new ANSICHAR[Name.Len() + 1];
		appStrncpyANSI( (ANSICHAR*)Macro->Name, TCHAR_TO_ANSI(*Name), Name.Len()+1 );
		Macro->Definition = new ANSICHAR[Definition.Len() + 1];
		appStrncpyANSI( (ANSICHAR*)Macro->Definition, TCHAR_TO_ANSI(*Definition), Definition.Len()+1 );
	}
	
	// set the COMPILER type
	D3DXMACRO* Macro = new(Macros) D3DXMACRO;
#define COMPILER_NAME "COMPILER_HLSL"
	Macro->Name = appStrcpyANSI(
		new ANSICHAR[strlen(COMPILER_NAME) + 1], 
		strlen(COMPILER_NAME) + 1,
		COMPILER_NAME
		);
	Macro->Definition = appStrcpyANSI(new ANSICHAR[2], 2, "1");
	
	DWORD CompileFlags = 0;
	if (DEBUG_SHADERS) 
	{
		//add the debug flags
		CompileFlags |= D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
	}

	for(INT FlagIndex = 0;FlagIndex < Environment.CompilerFlags.Num();FlagIndex++)
	{
		//accumulate flags set by the shader
		CompileFlags |= TranslateCompilerFlag(Environment.CompilerFlags(FlagIndex));
	}

	UBOOL bShaderCompileFailed = TRUE;
	const FString ShaderPath = TEXT("..") PATH_SEPARATOR TEXT("Engine") PATH_SEPARATOR TEXT("Shaders");
	const FString PreprocessorOutputDir = ShaderPath * ShaderPlatformToText((EShaderPlatform)Target.Platform) * FString(ShaderSubDir);

	TCHAR ShaderProfile[32];
	{
		//set defines and profiles for the appropriate shader paths
		if (Target.Platform == SP_PCD3D_SM3)
		{
			if (Target.Frequency == SF_Pixel)
			{
				appStrcpy(ShaderProfile, TEXT("ps_3_0"));
			}
			else
			{
				appStrcpy(ShaderProfile, TEXT("vs_3_0"));
			}

			// Set SM3_PROFILE, which indicates that we are compiling for sm3.  
			D3DXMACRO* MacroSM3Profile = new(Macros) D3DXMACRO;
			MacroSM3Profile->Name = appStrcpyANSI(
				new ANSICHAR[strlen("SM3_PROFILE") + 1], 
				strlen("SM3_PROFILE") + 1,
				"SM3_PROFILE"
				);
			MacroSM3Profile->Definition = appStrcpyANSI(new ANSICHAR[2], 2, "1");
		}
		else if (Target.Platform == SP_PCD3D_SM2)
		{
			if (Target.Frequency == SF_Pixel)
			{
				appStrcpy(ShaderProfile, TEXT("ps_2_0"));
			}
			else
			{
				appStrcpy(ShaderProfile, TEXT("vs_2_0"));
			}
			
			// Set the SUPPORTS_FP_BLENDING shader define.
			// It defaults to TRUE in Definitions.usf, so it's not necessary to set it in that case.
			D3DXMACRO* MacroManualFPBlend = new(Macros) D3DXMACRO;
			MacroManualFPBlend->Name = appStrcpyANSI(
				new ANSICHAR[strlen("SUPPORTS_FP_BLENDING") + 1], 
				strlen("SUPPORTS_FP_BLENDING") + 1,
				"SUPPORTS_FP_BLENDING"
				);
			MacroManualFPBlend->Definition = appStrcpyANSI(new ANSICHAR[2], 2, "0");

			// Set SM2_PROFILE, which indicates that we are compiling for sm2.  
			// This is needed to work around some HLSL compiler bugs with this profile.
			D3DXMACRO* MacroSM2Profile = new(Macros) D3DXMACRO;
			MacroSM2Profile->Name = appStrcpyANSI(
				new ANSICHAR[strlen("SM2_PROFILE") + 1], 
				strlen("SM2_PROFILE") + 1,
				"SM2_PROFILE"
				);
			MacroSM2Profile->Definition = appStrcpyANSI(new ANSICHAR[2], 2, "1");
		}

		// Add a define that indicates whether the compiler supports attributes such as [unroll]
		INT AttributeMacroIndex = Macros.Num();
		D3DXMACRO* MacroCompilerSupportsAttributes = new(Macros) D3DXMACRO;
		MacroCompilerSupportsAttributes->Name = appStrcpyANSI(
			new ANSICHAR[strlen("COMPILER_SUPPORTS_ATTRIBUTES") + 1], 
			strlen("COMPILER_SUPPORTS_ATTRIBUTES") + 1,
			"COMPILER_SUPPORTS_ATTRIBUTES"
			);
		MacroCompilerSupportsAttributes->Definition = appStrcpyANSI(new ANSICHAR[2], 2, "1");

		// Terminate the Macros list.
		D3DXMACRO* TerminatorMacro = new(Macros) D3DXMACRO;
		TerminatorMacro->Name = NULL;
		TerminatorMacro->Definition = NULL;

		// If this is a multi threaded compile, we must use the worker, otherwise compile the shader directly.
		if (GShaderCompilingThreadManager->IsMultiThreadedCompile())
		{
			bShaderCompileFailed = !D3D9CompileShaderThroughWorker(
				ThreadId,
				SourceFilename,
				TCHAR_TO_ANSI(FunctionName),
				TCHAR_TO_ANSI(ShaderProfile),
				TCHAR_TO_ANSI(*ShaderPath),
				CompileFlags,
				Environment,
				Output,
				Macros,
				Constants,
				DisassemblyString,
				bSilent
				);
		}
		else
		{
			bShaderCompileFailed = !D3D9CompileShaderThroughDll(
				SourceFilename,
				FunctionName,
				ShaderProfile,
				CompileFlags,
				Environment,
				Output,
				Macros,
				AttributeMacroIndex,
				Constants,
				DisassemblyString,
				bSilent
				);
		}
	}

	// If we are dumping out preprocessor data
	// @todo - also dump out shader data when compilation fails
	if (bDebugDump)
	{
		// just in case the preprocessed shader dir has not been created yet
		GFileManager->MakeDirectory( *PreprocessorOutputDir, TRUE );

		// save out include files from the environment definitions
		// Note: Material.usf and VertexFactory.usf vary between materials/vertex factories
		// this is handled because fxc will search for the includes in the same directory as the main shader before searching the include path 
		// otherwise it would find a stale Material.usf and VertexFactory.usf left behind by other platforms
		for(TMap<FString,FString>::TConstIterator IncludeIt(Environment.IncludeFiles); IncludeIt; ++IncludeIt)
		{
			FString IncludePath = PreprocessorOutputDir * IncludeIt.Key();
			appSaveStringToFile(IncludeIt.Value(), *IncludePath);
		}

		const FString SourceFile = LoadShaderSourceFile(SourceFilename);
		const FString SaveFileName = PreprocessorOutputDir * SourceFilename;
		appSaveStringToFile(SourceFile, *(SaveFileName + TEXT(".usf")));

		// dump the preprocessed shader
		FD3DIncludeEnvironment IncludeEnvironment(Environment);
		D3D9PreProcessShader(SourceFilename, SourceFile, Macros, IncludeEnvironment, *PreprocessorOutputDir);

		FString AbsoluteShaderPath = FString(appBaseDir()) * PreprocessorOutputDir;
		FString AbsoluteIncludePath = FString(appBaseDir()) * ShaderPath;
		// get the fxc command line
		FString FXCCommandline = D3D9CreateShaderCompileCommandLine(
			AbsoluteShaderPath * SourceFilename + TEXT(".usf"), 
			AbsoluteIncludePath, 
			FunctionName, 
			ShaderProfile,
			&Macros(0),
			CompileFlags);

		appSaveStringToFile(FXCCommandline, *(SaveFileName + TEXT(".bat")));
	}
	
	// Free temporary strings allocated for the macros.
	for(INT MacroIndex = 0;MacroIndex < Macros.Num();MacroIndex++)
	{
		delete Macros(MacroIndex).Name;
		delete Macros(MacroIndex).Definition;
	}

	// if the shader failed to compile we need to tell the caller that Compiling the shader failed
	if( bShaderCompileFailed == TRUE )
	{
		return FALSE;
	}

	// If the shader compile succeeded then this should contain the bytecode
	check(Output.Code.Num() > 0);

	// Map each constant in the table.
	for(INT ConstantIndex = 0;ConstantIndex < Constants.Num();ConstantIndex++)
	{
		// Read the constant description.
		const FD3D9NamedConstantDesc& Constant = Constants(ConstantIndex);

		const UBOOL bIsSampler = Constant.ConstantDesc.RegisterSet == D3DXRS_SAMPLER;
		if(bIsSampler)
		{
			Output.ParameterMap.AddParameterAllocation(
				ANSI_TO_TCHAR(Constant.Name),
				0,
				Constant.ConstantDesc.RegisterIndex,
				Constant.ConstantDesc.RegisterCount,
				Constant.ConstantDesc.RegisterIndex
				);
		}
		else
		{
			Output.ParameterMap.AddParameterAllocation(
				ANSI_TO_TCHAR(Constant.Name),
				0,
				Constant.ConstantDesc.RegisterIndex * sizeof(FLOAT) * 4,
				Constant.ConstantDesc.RegisterCount * sizeof(FLOAT) * 4,
				0
				);
		}
	}

	// Pass the target through to the output.
	Output.Target = Target;

	// Extract the instruction count from the disassembly text.
	Parse(
		*DisassemblyString,
		TEXT("// approximately "),
		(DWORD&)Output.NumInstructions
		);

	if(bDebugDump)
	{
		appSaveStringToFile( DisassemblyString, *(PreprocessorOutputDir * SourceFilename + TEXT(".asm")) );
	}

	return TRUE;
}
