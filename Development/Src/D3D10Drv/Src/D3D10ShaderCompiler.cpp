/*=============================================================================
	D3D10ShaderCompiler.cpp: D3D shader compiler implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3D10DrvPrivate.h"

/**
 * An implementation of the D3DX include interface to access a FShaderCompilerEnvironment.
 */
class FD3DIncludeEnvironment : public ID3D10Include
{
public:

	STDMETHOD(Open)(D3D10_INCLUDE_TYPE Type,LPCSTR Name,LPCVOID ParentData,LPCVOID* Data,UINT* Bytes)
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

		return S_OK;
	}

	STDMETHOD(Close)(LPCVOID Data)
	{
		delete Data;
		return S_OK;
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
	case CFLAG_PreferFlowControl: return D3D10_SHADER_PREFER_FLOW_CONTROL;
	case CFLAG_Debug: return D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
	case CFLAG_AvoidFlowControl: return D3D10_SHADER_AVOID_FLOW_CONTROL;
	default: return 0;
	};
}

/**
 * D3D10CreateShaderCompileCommandLine - takes shader parameters used to compile with the DX10
 * compiler and returns an fxc command to compile from the command line
 */
static FString D3D10CreateShaderCompileCommandLine(
	const FString& ShaderPath, 
	const FString& IncludePath, 
	const TCHAR* EntryFunction, 
	const TCHAR* ShaderProfile, 
	D3D10_SHADER_MACRO *Macros,
	DWORD CompileFlags
	)
{
	// fxc is our command line compiler
	FString FXCCommandline = FString(TEXT("fxc ")) + ShaderPath;

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
	if(CompileFlags & D3D10_SHADER_PREFER_FLOW_CONTROL)
	{
		CompileFlags &= ~D3D10_SHADER_PREFER_FLOW_CONTROL;
		FXCCommandline += FString(TEXT(" /Gfp"));
	}
	if(CompileFlags & D3D10_SHADER_DEBUG)
	{
		CompileFlags &= ~D3D10_SHADER_DEBUG;
		FXCCommandline += FString(TEXT(" /Zi"));
	}
	if(CompileFlags & D3D10_SHADER_SKIP_OPTIMIZATION)
	{
		CompileFlags &= ~D3D10_SHADER_SKIP_OPTIMIZATION;
		FXCCommandline += FString(TEXT(" /Od"));
	}
	if(CompileFlags & D3D10_SHADER_AVOID_FLOW_CONTROL)
	{
		CompileFlags &= ~D3D10_SHADER_AVOID_FLOW_CONTROL;
		FXCCommandline += FString(TEXT(" /Gfa"));
	}
	if(CompileFlags & D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY)
	{
		CompileFlags &= ~D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY;
		FXCCommandline += FString(TEXT(" /Gec"));
	}
	if(CompileFlags & D3D10_SHADER_OPTIMIZATION_LEVEL3)
	{
		CompileFlags &= ~D3D10_SHADER_OPTIMIZATION_LEVEL3;
		FXCCommandline += FString(TEXT(" /O3"));
	}
	checkf(CompileFlags == 0, TEXT("Unhandled d3d10 shader compiler flag!"));

	// add the target instruction set
	FXCCommandline += FString(TEXT(" /T ")) + ShaderProfile;

	// add a pause on a newline
	FXCCommandline += FString(TEXT(" \r\n pause"));
	return FXCCommandline;
}

/**
 * Uses D3D10 to PreProcess a shader (resolve all #includes and #defines) and dumps it out for debugging
 */
static void D3D10PreProcessShader(
	const TCHAR* SourceFilename,
	const FString& SourceFile,
	const TArray<D3D10_SHADER_MACRO>& Macros,
	FD3DIncludeEnvironment& IncludeEnvironment,
	const TCHAR* ShaderPath
	)
{
	TRefCountPtr<ID3D10Blob> ShaderText;
	TRefCountPtr<ID3D10Blob> PreProcessErrors;

	HRESULT PreProcessHR = D3D10PreprocessShader(
		TCHAR_TO_ANSI(*SourceFile),
		SourceFile.Len(),
		TCHAR_TO_ANSI(SourceFilename),
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

/** Compiles a D3D10 shader through the D3DX Dll */
static UBOOL D3D10CompileShaderThroughDll(
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	const TCHAR* ShaderProfile,
	DWORD CompileFlags,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output,
	TArray<D3D10_SHADER_MACRO>& Macros,
	UBOOL bSilent = FALSE
	)
{
	TRefCountPtr<ID3D10Blob> Shader;
	TRefCountPtr<ID3D10Blob> Errors;

	const FString SourceFile = LoadShaderSourceFile(SourceFilename);
	FD3DIncludeEnvironment IncludeEnvironment(Environment);

	HRESULT Result = D3DX10CompileFromMemory(
		TCHAR_TO_ANSI(*SourceFile),
		SourceFile.Len(),
		TCHAR_TO_ANSI(SourceFilename),
		&Macros(0),
		&IncludeEnvironment,
		TCHAR_TO_ANSI(FunctionName),
		TCHAR_TO_ANSI(ShaderProfile),
		CompileFlags,
		0,
		NULL,
		Shader.GetInitReference(),
		Errors.GetInitReference(),
		NULL
		);

	if (FAILED(Result))
	{
		if (Errors)
		{
			// Copy the error text to the output.
			FString* ErrorString = new(Output.Errors) FString(ANSI_TO_TCHAR(Errors->GetBufferPointer()));

			if (!bSilent)
			{
				// Log the compilation error.
				warnf( NAME_Warning, TEXT("Shader compile error: %s"), **ErrorString );
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

		return TRUE;
	}
}

/** Compiles a D3D10 shader through a worker process */
static UBOOL D3D10CompileShaderThroughWorker(
	UINT ThreadId,
	const TCHAR* SourceFilename,
	LPCSTR FunctionName,
	LPCSTR ShaderProfile,
	LPCSTR IncludePath,
	DWORD CompileFlags,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output,
	TArray<D3D10_SHADER_MACRO>& Macros,
	UBOOL bSilent = FALSE
	)
{
	TArray<BYTE> WorkerInput;
	// Presize to avoid lots of allocations
	WorkerInput.Empty(1000);
	// Setup the input for the worker app, everything that is needed to compile the shader.
	// Note that any format changes here also need to be done in the worker
	// Write a job type so the worker app can know which compiler to invoke
	EWorkerJobType JobType = WJT_D3D10Shader;
	WorkerInputAppendValue(JobType, WorkerInput);
	// Version number so we can detect stale data
	const BYTE D3D10ShaderCompileWorkerInputVersion = 0;
	WorkerInputAppendValue(D3D10ShaderCompileWorkerInputVersion, WorkerInput);
	WorkerInputAppendMemory(TCHAR_TO_ANSI(SourceFilename), appStrlen(SourceFilename), WorkerInput);
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
		D3D10_SHADER_MACRO CurrentMacro = Macros(MacroIndex);
		check( CurrentMacro.Name);
		WorkerInputAppendMemory(CurrentMacro.Name, appStrlen(CurrentMacro.Name), WorkerInput);
		WorkerInputAppendMemory(CurrentMacro.Definition, appStrlen(CurrentMacro.Definition), WorkerInput);
	}

	TArray<BYTE> WorkerOutput;
	// Invoke the worker, this will return when the shader has been compiled
	GShaderCompilingThreadManager->WorkerCompile(ThreadId, JobType, WorkerInput, WorkerOutput);

	INT CurrentPosition = 0;
	const BYTE D3D10ShaderCompileWorkerOutputVersion = 0;
	// Read the worker output in the same format that it was written
	BYTE ReadVersion;
	WorkerOutputReadValue(ReadVersion, CurrentPosition, WorkerOutput);
	check(ReadVersion == D3D10ShaderCompileWorkerOutputVersion);
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

	if (FAILED(CompileResult))
	{
		
		// Copy the error text to the output.
		FString CompileErrors = TEXT("Compile Failed without warnings!");
		if (ErrorString.Len() > 0)
		{
			CompileErrors = ErrorString;
		}
		new(Output.Errors) FString(CompileErrors);

		if (!bSilent)
		{
			// Log the compilation error.
			warnf( NAME_Warning, TEXT("Shader compile error: %s"), *CompileErrors );
		}

		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/**
 * The D3D10/HLSL shader compiler.
 */ 
UBOOL D3D10CompileShader(
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

	// Translate the input environment's definitions to D3DXMACROs.
	TArray<D3D10_SHADER_MACRO> Macros;
	for(TMap<FName,FString>::TConstIterator DefinitionIt(Environment.Definitions);DefinitionIt;++DefinitionIt)
	{
		FString Name = DefinitionIt.Key().ToString();
		FString Definition = DefinitionIt.Value();

		D3D10_SHADER_MACRO* Macro = new(Macros) D3D10_SHADER_MACRO;
		ANSICHAR* tName = new ANSICHAR[Name.Len() + 1];
		strncpy_s(tName,Name.Len() + 1,TCHAR_TO_ANSI(*Name),Name.Len() + 1);
		Macro->Name = tName;
		ANSICHAR* tDefinition = new ANSICHAR[Definition.Len() + 1];
		strncpy_s(tDefinition,Definition.Len() + 1,TCHAR_TO_ANSI(*Definition),Definition.Len() + 1);
		Macro->Definition = tDefinition;
	}

	// set the COMPILER type
	D3D10_SHADER_MACRO* Macro = new(Macros) D3D10_SHADER_MACRO;
#define COMPILER_NAME "COMPILER_HLSL"
	ANSICHAR* tName1 = new ANSICHAR[strlen(COMPILER_NAME) + 1];
	strcpy_s(tName1, strlen(COMPILER_NAME) + 1, COMPILER_NAME);
	Macro->Name = tName1;

	ANSICHAR* tDefinition1 = new ANSICHAR[2];
	strcpy_s(tDefinition1, 2, "1");
	Macro->Definition = tDefinition1;

	// set the SM4_PROFILE definition
	static const char* ProfileName = "SM4_PROFILE";
	D3D10_SHADER_MACRO* ProfileMacro = new(Macros) D3D10_SHADER_MACRO;
	ProfileMacro->Name = appStrcpyANSI(new ANSICHAR[strlen(ProfileName) + 1],strlen(ProfileName) + 1,ProfileName);
	ProfileMacro->Definition = appStrcpyANSI(new ANSICHAR[2],2,"1");

	// set the SUPPORTS_DEPTH_TEXTURES if needed
	if( GSupportsDepthTextures )
	{
		D3D10_SHADER_MACRO* MacroDepthSupport = new(Macros) D3D10_SHADER_MACRO;
		ANSICHAR* tName2 = new ANSICHAR[strlen("SUPPORTS_DEPTH_TEXTURES") + 1];
		strcpy_s(tName2, strlen("SUPPORTS_DEPTH_TEXTURES") + 1, "SUPPORTS_DEPTH_TEXTURES");
		MacroDepthSupport->Name = tName2;

		ANSICHAR* tDefinition2 = new ANSICHAR[2];
		strcpy_s(tDefinition2, 2, "1");
		MacroDepthSupport->Definition = tDefinition2;
	}

	DWORD CompileFlags = 0;
	CompileFlags = D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY;
	if (DEBUG_SHADERS) 
	{
		//add the debug flags
		CompileFlags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
	}
	else
	{
		CompileFlags |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
	}

	for(INT FlagIndex = 0;FlagIndex < Environment.CompilerFlags.Num();FlagIndex++)
	{
		//accumulate flags set by the shader
		CompileFlags |= TranslateCompilerFlag(Environment.CompilerFlags(FlagIndex));
	}

	UBOOL bShaderCompileFailed = TRUE;
	const TCHAR* ShaderPath = TEXT("..") PATH_SEPARATOR TEXT("Engine") PATH_SEPARATOR TEXT("Shaders");
	const FString PreprocessorOutputDir = FString(ShaderPath) * ShaderPlatformToText((EShaderPlatform)Target.Platform) * FString(ShaderSubDir);

	TCHAR ShaderProfile[32];
	{
		//set defines and profiles for the appropriate shader paths
		if (Target.Frequency == SF_Pixel)
		{
			appStrcpy(ShaderProfile, TEXT("ps_4_0"));
		}
		else
		{
			appStrcpy(ShaderProfile, TEXT("vs_4_0"));
		}

		// Terminate the Macros list.
		D3D10_SHADER_MACRO* TerminatorMacro = new(Macros) D3D10_SHADER_MACRO;
		TerminatorMacro->Name = NULL;
		TerminatorMacro->Definition = NULL;

		// If this is a multi threaded compile, we must use the worker, otherwise compile the shader directly.
		if (GShaderCompilingThreadManager->IsMultiThreadedCompile())
		{
			bShaderCompileFailed = !D3D10CompileShaderThroughWorker(
				ThreadId,
				SourceFilename,
				TCHAR_TO_ANSI(FunctionName),
				TCHAR_TO_ANSI(ShaderProfile),
				TCHAR_TO_ANSI(ShaderPath),
				CompileFlags,
				Environment,
				Output,
				Macros,
				bSilent
				);
		}
		else
		{
			bShaderCompileFailed = !D3D10CompileShaderThroughDll(
				SourceFilename,
				FunctionName,
				ShaderProfile,
				CompileFlags,
				Environment,
				Output,
				Macros,
				bSilent
				);
		}
	}

	// If we are dumping out preprocessor data
	// @todo - also dump out shader data when compilation fails
	if (bDebugDump)
	{
		// just in case the preprocessed shader dir has not been created yet
		GFileManager->MakeDirectory( *PreprocessorOutputDir, true );

		// save out include files from the environment definitions
		// Note: Material.usf and VertexFactory.usf vary between materials/vertex factories
		// this is handled because fxc will search for the includes in the same directory as the main shader before searching the include path 
		// otherwise it would find a stale Material.usf and VertexFactory.usf left behind by other platforms
		for(TMap<FString,FString>::TConstIterator IncludeIt(Environment.IncludeFiles); IncludeIt; ++IncludeIt)
		{
			FString IncludePath = PreprocessorOutputDir * IncludeIt.Key();
			appSaveStringToFile(IncludeIt.Value(), *IncludePath);
		}

		const FString SaveFileName = PreprocessorOutputDir * SourceFilename;
		const FString SourceFile = LoadShaderSourceFile(SourceFilename);
		appSaveStringToFile(SourceFile, *(SaveFileName + TEXT(".usf")));

		//allow dumping the preprocessed shader
		FD3DIncludeEnvironment IncludeEnvironment(Environment);
		D3D10PreProcessShader(SourceFilename, SourceFile, Macros, IncludeEnvironment, *PreprocessorOutputDir);

		const FString AbsoluteShaderPath = FString(appBaseDir()) * PreprocessorOutputDir;
		const FString AbsoluteIncludePath = FString(appBaseDir()) * ShaderPath;
		// get the fxc command line
		FString FXCCommandline = D3D10CreateShaderCompileCommandLine(
			AbsoluteShaderPath * SourceFilename + TEXT(".usf"), 
			AbsoluteIncludePath, 
			FunctionName, 
			ShaderProfile,
			&Macros(0),
			CompileFlags);

		appSaveStringToFile(FXCCommandline, *(SaveFileName + TEXT(".bat")));

		if (bDebugDump && !bShaderCompileFailed)
		{
			// Disassemble the shader
			TRefCountPtr<ID3D10Blob> DisassemblyBuffer;
			VERIFYD3D10RESULT(D3DX10DisassembleShader((DWORD*)Output.Code.GetData(),Output.Code.Num(),FALSE,NULL,DisassemblyBuffer.GetInitReference()));
			appSaveStringToFile( ANSI_TO_TCHAR((ANSICHAR*)DisassemblyBuffer->GetBufferPointer()), *(PreprocessorOutputDir * SourceFilename + TEXT(".asm")) );
		}
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


	// Read the constant table description.
	ID3D10ShaderReflection* Reflector = NULL;
	VERIFYD3D10RESULT(D3DX10ReflectShader(Output.Code.GetData(),Output.Code.Num(),(ID3D10ShaderReflection1**)&Reflector));
	D3D10_SHADER_DESC ShaderDesc;
	Reflector->GetDesc(&ShaderDesc);

	// Constant buffers
	for(UINT i=0; i<ShaderDesc.BoundResources; i++ )
	{
		D3D10_SHADER_INPUT_BIND_DESC BindDesc;
		Reflector->GetResourceBindingDesc(i,&BindDesc);
		if(BindDesc.Type == D3D10_SIT_CBUFFER || BindDesc.Type == D3D10_SIT_TBUFFER)
		{
			UINT CBIndex = BindDesc.BindPoint;
			ID3D10ShaderReflectionConstantBuffer* ConstantBuffer = Reflector->GetConstantBufferByName(BindDesc.Name);
			D3D10_SHADER_BUFFER_DESC CBDesc;
			ConstantBuffer->GetDesc(&CBDesc);

			if( CBDesc.Size > MAX_CONSTANT_BUFFER_SIZE && CBDesc.Size != BONE_CONSTANT_BUFFER_SIZE )
			{
				appErrorf(TEXT("Set MAX_CONSTANT_BUFFER_SIZE to >= \'%d\'"), CBDesc.Size);
			}

			// Track all of the variables in this constant buffer.
			for(UINT ConstantIndex = 0;ConstantIndex < CBDesc.Variables;ConstantIndex++)
			{
				ID3D10ShaderReflectionVariable* Variable = ConstantBuffer->GetVariableByIndex(ConstantIndex);
				D3D10_SHADER_VARIABLE_DESC VariableDesc;
				Variable->GetDesc(&VariableDesc);
				if(VariableDesc.uFlags & D3D10_SVF_USED)
				{
					Output.ParameterMap.AddParameterAllocation(
						ANSI_TO_TCHAR(VariableDesc.Name),
						CBIndex,
						VariableDesc.StartOffset,
						VariableDesc.Size,
						0
						);
				}
			}
		}
		else if(BindDesc.Type == D3D10_SIT_SAMPLER)
		{
			// Find the texture that goes with this sampler
			for(UINT R2 = 0;R2 < ShaderDesc.BoundResources;R2++)
			{
				D3D10_SHADER_INPUT_BIND_DESC TextureDesc;
				Reflector->GetResourceBindingDesc(R2,&TextureDesc);

				if(TextureDesc.Type == D3D10_SIT_TEXTURE)
				{
					if(!strcmp(TextureDesc.Name,BindDesc.Name))
					{
						Output.ParameterMap.AddParameterAllocation(
							ANSI_TO_TCHAR(BindDesc.Name),
							0,
							TextureDesc.BindPoint,
							TextureDesc.BindCount,
							BindDesc.BindPoint
							);
						break;
					}
				}
			}
		}
	}

	// Set the number of instructions.
	Output.NumInstructions = ShaderDesc.InstructionCount;

	// Reflector is a com interface, so it needs to be released.
	Reflector->Release();

	// Pass the target through to the output.
	Output.Target = Target;

	return TRUE;
}
