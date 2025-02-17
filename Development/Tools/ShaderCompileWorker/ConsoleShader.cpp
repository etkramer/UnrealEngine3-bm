#include "ShaderCompileWorker.h"
#include "..\..\Src\Engine\Inc\UnConsoleTools.h"

FConsoleSupport* ConsoleSupport[WJT_JobTypeMax] = {NULL};
FConsoleShaderPrecompiler* ShaderPrecompiler[WJT_JobTypeMax] = {NULL};

const BYTE ConsoleShaderCompileWorkerInputVersion = 1;
const BYTE ConsoleShaderCompileWorkerOutputVersion = 0;

/** Loads console support DLL's */
bool LoadConsoleSupport(EWorkerJobType JobType, const wstring& GameName)
{
	if (!ConsoleSupport[JobType])
	{
		HANDLE ConsoleDLL = NULL;

		TCHAR PlatformDllPathAndFile[ 2048 ];
		_tgetcwd( PlatformDllPathAndFile, 2048 );
		if (JobType == WJT_XenonShader)
		{
			//@todo - don't hardcode dll name
			_tcscat_s( PlatformDllPathAndFile, TEXT("\\Xenon\\XeTools.dll") );
		}
		else if (JobType == WJT_PS3Shader)
		{
			//@todo - don't hardcode dll name
			_tcscat_s( PlatformDllPathAndFile, TEXT("\\PS3\\PS3Tools.dll") );
		}

		// Some DLLs (such as XeTools) may have load-time references to other DLLs that reside in
		// same directory, so we need to make sure Windows knows to look in that folder for dependencies.
		// By specifying LOAD_WITH_ALTERED_SEARCH_PATH, LoadLibrary will look in the target DLL's
		// directory for the necessary dependent DLL files.
		ConsoleDLL = LoadLibraryEx( PlatformDllPathAndFile, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );

		if (ConsoleDLL)
		{
			// look for the main entry point function that returns a pointer to the ConsoleSupport subclass
			FuncGetConsoleSupport SupportProc = (FuncGetConsoleSupport)GetProcAddress((HMODULE)ConsoleDLL, "GetConsoleSupport");
			ConsoleSupport[JobType] = SupportProc ? SupportProc() : NULL;
			if (ConsoleSupport[JobType])
			{
#ifdef _DEBUG
				ConsoleSupport[JobType]->Initialize(GameName.c_str(), TEXT("Debug"));
#else
				ConsoleSupport[JobType]->Initialize(GameName.c_str(), TEXT("Release"));
#endif

				ShaderPrecompiler[JobType] = ConsoleSupport[JobType]->GetGlobalShaderPrecompiler();
				CHECKF(ShaderPrecompiler[JobType] != NULL, TEXT("Failed to create shader precompiler"));
				return true;
			}
			else
			{
				FreeLibrary((HMODULE)ConsoleDLL);
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool ConsoleCompileShader(EWorkerJobType JobType, BYTE InputVersion, FILE* InputFile, const TCHAR* WorkingDirectory, vector<BYTE>& OutputData)
{
	CHECKF(JobType == WJT_XenonShader || JobType == WJT_PS3Shader, TEXT("Unsupported job type"));
	CHECKF(InputVersion == ConsoleShaderCompileWorkerInputVersion, TEXT("Wrong version for a console shader"));

	wstring GameName;
	ParseUnicodeString(InputFile, GameName);

	if (!LoadConsoleSupport(JobType, GameName))
	{
		ERRORF((wstring(TEXT("Failed to load console support dll for ")) + GetJobTypeName(JobType)).c_str());
	}
	CHECKF(ShaderPrecompiler[JobType] != NULL, TEXT("Console precompiler was not initialized"));

	string SourceFileName;
	ParseAnsiString(InputFile, SourceFileName);

	size_t NumCharsConverted;
	const size_t WorkingDirectoryLen = wcslen(WorkingDirectory); 
	LPSTR WorkingDirectoryAnsi = new CHAR[WorkingDirectoryLen + 1];
	// Convert working directory from Unicode to Ansi so it can be used by the precompiler
	errno_t ErrorCode = wcstombs_s(&NumCharsConverted, WorkingDirectoryAnsi, WorkingDirectoryLen + 1, WorkingDirectory, WorkingDirectoryLen + 1);
	CHECKF(ErrorCode == 0, TEXT("Failed to convert working directory to ansi"));
	string SourceFilePath = string(WorkingDirectoryAnsi) + "\\" + SourceFileName + ".usf";

	string FunctionName;
	ParseAnsiString(InputFile, FunctionName);

	BYTE bIsVertexShader;
	ReadValue(InputFile, bIsVertexShader);

	DWORD CompileFlags;
	ReadValue(InputFile, CompileFlags);

	UINT NumIncludes;
	ReadValue(InputFile, NumIncludes);

	for (UINT IncludeIndex = 0; IncludeIndex < NumIncludes; IncludeIndex++)
	{
		FInclude NewInclude;
		ParseAnsiString(InputFile, NewInclude.IncludeName);
		string IncludeFilePath = string(WorkingDirectoryAnsi) + "\\" + NewInclude.IncludeName;
		ParseAnsiString(InputFile, NewInclude.IncludeFile);
		bool bWroteFile = WriteFileA(IncludeFilePath, NewInclude.IncludeFile);
		CHECKF(bWroteFile, TEXT("Failed to write out include file for console shader"));
	}

	string Definitions;
	ParseAnsiString(InputFile, Definitions);

	BYTE bIsDumpingShaderPDBs;
	ReadValue(InputFile, bIsDumpingShaderPDBs);

	string ShaderPDBPath;
	ParseAnsiString(InputFile, ShaderPDBPath);

	// allocate a huge buffer for constants and bytecode
	// @GEMINI_TODO: Validate this in the dll or something by passing in the size
	const UINT BytecodeBufferAllocatedSize = 1024 * 1024; // 1M
	BYTE* BytecodeBuffer = new BYTE[BytecodeBufferAllocatedSize]; 
	// Zero the bytecode in case the dll doesn't write to all of it,
	// since the engine caches shaders with the same bytecode.
	ZeroMemory(BytecodeBuffer, BytecodeBufferAllocatedSize);
	char* ErrorBuffer = new char[256 * 1024]; // 256k
	char* ConstantBuffer = new char[256 * 1024]; // 256k
	// to avoid crashing if the DLL doesn't set these
	ConstantBuffer[0] = 0;
	ErrorBuffer[0] = 0;
	INT BytecodeSize = 0;

	// call the DLL precompiler
	bool bSucceeded = ShaderPrecompiler[JobType]->PrecompileShader(
		SourceFilePath.c_str(), 
		FunctionName.c_str(),
		bIsVertexShader == 0 ? false : true, 
		CompileFlags, 
		Definitions.c_str(), 
		bIsDumpingShaderPDBs == 0 ? false : true,
		ShaderPDBPath.c_str(),
		BytecodeBuffer, 
		BytecodeSize, 
		ConstantBuffer, 
		ErrorBuffer);

	WriteValue(OutputData, ConsoleShaderCompileWorkerOutputVersion);
	WriteValue(OutputData, JobType);
	BYTE bSuccededByte = bSucceeded;
	WriteValue(OutputData, bSuccededByte);
	WriteValue(OutputData, BytecodeSize);
	if (BytecodeSize > 0)
	{
		WriteArray(OutputData, BytecodeBuffer, BytecodeSize);
	}
	UINT ConstantBufferLength = (UINT)strlen(ConstantBuffer);
	WriteValue(OutputData, ConstantBufferLength);
	if (ConstantBufferLength > 0)
	{
		WriteArray(OutputData, ConstantBuffer, ConstantBufferLength);
	}
	UINT ErrorBufferLength = (UINT)strlen(ErrorBuffer);
	WriteValue(OutputData, ErrorBufferLength);
	if (ErrorBufferLength > 0)
	{
		WriteArray(OutputData, ErrorBuffer, ErrorBufferLength);
	}

	delete [] WorkingDirectoryAnsi;
	delete [] BytecodeBuffer;
	delete [] ErrorBuffer;
	delete [] ConstantBuffer;

	return true;
}
