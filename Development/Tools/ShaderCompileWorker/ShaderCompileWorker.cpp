// ShaderCompileWorker.cpp : Defines the entry point for the console application.
//

#include "ShaderCompileWorker.h"

FILE* LogFile = NULL;

/** Logs a string to the log file and debug output */
void Log(const TCHAR* LogString)
{
	CHECKF(LogFile != NULL, TEXT("Trying to log but log file has not been initialized"));
	wstring LogStringWithNewline = LogString;
	LogStringWithNewline += TEXT("\n");
	fputws(LogStringWithNewline.c_str(), LogFile);
	fflush(LogFile);
	//Build machine picks this up, so only output when being debugged
	if (IsDebuggerPresent())
	{
		OutputDebugString(LogStringWithNewline.c_str());
	}
}

void ExitCleanup(UINT ExitCode)
{
	Log(TEXT("Exiting"));
	fclose(LogFile);
	ExitProcess(ExitCode);
}

/** Writes the given data to a file, Unicode version */
bool WriteFileW(const wstring& FilePath, const void* Contents, size_t Size)
{
	FILE* OutputFile = NULL;
	errno_t ErrorCode = _wfopen_s(&OutputFile, FilePath.c_str(), TEXT("wb"));
	if (ErrorCode != 0)
	{
		return false;
	}
	size_t NumWrote = fwrite(Contents, Size, 1, OutputFile);
	fclose(OutputFile);
	if (NumWrote == 1)
	{
		return true;
	}
	return false;
}

/** Writes the given data to a file, Ansi version */
bool WriteFileA(const string& FilePath, const string& Contents)
{
	FILE* OutputFile = NULL;
	errno_t ErrorCode = fopen_s(&OutputFile, FilePath.c_str(), "wb");
	if (ErrorCode != 0)
	{
		return false;
	}
	size_t NumWrote = fwrite(Contents.c_str(), Contents.size(), 1, OutputFile);
	fclose(OutputFile);
	if (NumWrote == 1)
	{
		return true;
	}
	return false;
}

/** Returns true if the file is present on disk */
bool IsFilePresentW(const wstring& FilePath)
{
	FILE* TestFile = NULL;
	errno_t InputOpenReturn = _wfopen_s(&TestFile, FilePath.c_str(), TEXT("rb"));
	if (InputOpenReturn == 0)
	{
		fclose(TestFile);
	}
	return InputOpenReturn == 0;
}

/** Directory where UE3 expects the output file, used by ERRORF and CHECKF to report errors. */
wstring WorkingDirectory;

/** Handles a critical error.  In release, the output file that UE3 is expecting is written with the error message. */
void Error(const TCHAR* ErrorString, const TCHAR* Filename, UINT LineNum, const TCHAR* FunctionName)
{
	if (IsDebuggerPresent())
	{
		DebugBreak();
	}
	else
	{
#if _DEBUG
		assert(0);
#else

		TCHAR* FilenameCopy = _wcsdup(Filename);
		const TCHAR* LastPathSeparator = wcsrchr(FilenameCopy, L'\\');
		if (LastPathSeparator == NULL)
		{
			LastPathSeparator = FilenameCopy;
		}
		TCHAR LineNumString[25];
		_itow_s(LineNum, LineNumString, 25, 10);
		wstring ErrorMessage = wstring(TEXT("CriticalError: ")) + wstring(LastPathSeparator + 1) + TEXT(",") + FunctionName + TEXT(":") + LineNumString + TEXT("  ") + ErrorString;
		delete [] FilenameCopy;
		OutputDebugString(ErrorMessage.c_str());
		if (LogFile != NULL)
		{
			Log(ErrorMessage.c_str());
		}

		{
			// Write the error message to the output file so that UE3 can assert with the error
			const TCHAR* OutputFileName = TEXT("\\WorkerOutput.bin");
			wstring OutputFilePath = WorkingDirectory + OutputFileName;
			vector<BYTE> OutputData;
			const BYTE ErrorOutputVersion = 0;
			WriteValue(OutputData, ErrorOutputVersion);
			const EWorkerJobType JobType = WJT_WorkerError;
			WriteValue(OutputData, JobType);
			WriteValue(OutputData, ErrorMessage.size() * sizeof(TCHAR));
			if (ErrorMessage.size() > 0)
			{
				WriteArray(OutputData, ErrorMessage.c_str(), (UINT)(ErrorMessage.size() * sizeof(TCHAR)));
			}
			bool bWroteOutputFile = WriteFileW(OutputFilePath, &OutputData.front(), OutputData.size());
			assert(bWroteOutputFile);
		}
		// Sleep a bit before exiting to make sure that UE3 checks for the output file happen before checks that the worker app is still running
		Sleep(100);
#endif
	}
	ExitCleanup(1);
}

/** Throws a critical error if Expression is false. */
void Check(bool Expression, const TCHAR* ExpressionString, const TCHAR* ErrorString, const TCHAR* Filename, UINT LineNum, const TCHAR* FunctionName)
{
	if (!Expression)
	{
		wstring AssertString = wstring(TEXT("Assertion failed: ")) + ExpressionString + TEXT(" '") + ErrorString + TEXT("'");
		Error(AssertString.c_str(), Filename, LineNum, FunctionName);
	}
}

INT CreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo )
{
	//@todo - implement
	return EXCEPTION_EXECUTE_HANDLER;
}

void CreateLogFile(const TCHAR* WorkingDirectory)
{
	const TCHAR* LogFileName = TEXT("\\WorkerLog.txt");
	wstring LogFilePath = wstring(WorkingDirectory) + LogFileName;
	errno_t Return = _wfopen_s(&LogFile, LogFilePath.c_str(), TEXT("w"));
	CHECKF(Return == 0, (wstring(TEXT("Couldn't open log file ")) + LogFilePath).c_str());
	Log(TEXT("LogFile opened"));
}

/** 
 * Reads an Ansi string from the file, and NULL terminates the string.
 * StringLen is the string length WITH the terminator.
 */
void ReadAnsiString(FILE* File, LPSTR String, UINT StringLen)
{
	size_t NumRead = fread(String,StringLen - 1,1,File);
	CHECKF(NumRead == 1, TEXT("Read wrong amount"));
	String[StringLen - 1] = 0;
}

/** 
 * Reads a std::string from a file, where the string is stored in the file with length first, then string without NULL terminator.
 */
void ParseAnsiString(FILE* File, string& String)
{
	UINT StringLenWithoutNULL;
	ReadValue(File, StringLenWithoutNULL);
	// Allocate space for the string and the terminator
	LPSTR TempString = new CHAR[StringLenWithoutNULL + 1];
	ReadAnsiString(File, TempString, StringLenWithoutNULL + 1);
	String = TempString;
	delete [] TempString;
}

/** 
 * Reads an Ansi string from the file, and returns the string and the length including the terminator.
 * Note: Caller is responsible for freeing String 
 */
void ParseAnsiString(FILE* File, LPCSTR& String, UINT& StringLength)
{
	UINT StringLenWithoutNULL;
	ReadValue(File, StringLenWithoutNULL);
	LPSTR NewString = new CHAR[StringLenWithoutNULL + 1];
	ReadAnsiString(File, NewString, StringLenWithoutNULL + 1);
	String = NewString;
	StringLength = StringLenWithoutNULL + 1;
}

/** 
 * Reads a Unicode string from the file, and NULL terminates the string.
 * StringLen is the string length WITH the terminator.
 */
void ReadUnicodeString(FILE* File, TCHAR* String, UINT StringLen)
{
	size_t NumRead = fread(String,(StringLen - 1) * sizeof(TCHAR),1,File);
	CHECKF(NumRead == 1, TEXT("Read wrong amount"));
	String[StringLen - 1] = 0;
}

/** 
 * Reads a std::wstring from a file, where the string is stored in the file with length first, then string without NULL terminator.
 */
void ParseUnicodeString(FILE* File, wstring& String)
{
	UINT StringSizeWithoutNULL;
	ReadValue(File, StringSizeWithoutNULL);
	UINT StringLengthWithoutNULL = StringSizeWithoutNULL / sizeof(TCHAR);
	TCHAR* TempString = new TCHAR[StringLengthWithoutNULL + 1];
	ReadUnicodeString(File, TempString, StringLengthWithoutNULL + 1);
	String = TempString;
	delete [] TempString;
}

/** 
 * Writes memory into a buffer.
 */
void WriteArray(vector<BYTE>& Buffer, const void* Value, UINT Length)
{
	const BYTE* ValueAsByteStart = reinterpret_cast<const BYTE*>(Value);
	const BYTE* ValueAsByteEnd = ValueAsByteStart + Length;
	Buffer.insert(Buffer.end(), ValueAsByteStart, ValueAsByteEnd);
}

wstring GetJobTypeName(EWorkerJobType JobType)
{
	if (JobType == WJT_D3D9Shader)
	{
		return TEXT("WJT_D3D9Shader");
	}
	else if (JobType == WJT_D3D10Shader)
	{
		return TEXT("WJT_D3D10Shader");
	}
	else if (JobType == WJT_XenonShader)
	{
		return TEXT("WJT_XenonShader");
	}
	else if (JobType == WJT_PS3Shader)
	{
		return TEXT("WJT_PS3Shader");
	}
	else if (JobType == WJT_WorkerError)
	{
		return TEXT("WJT_WorkerError");
	}
	ERRORF(TEXT("Unknown job type"));
	return TEXT("");
}

DWORD LastCompileTime = 0;

/** Called in the idle loop, checks for conditions under which the helper should exit */
void CheckExitConditions(const TCHAR* ParentId, const TCHAR* WorkingDirectory)
{
	// Don't do these if the debugger is present
	//@todo - don't do these if UE3 is being debugged either
	if (!IsDebuggerPresent())
	{
		// The Win32 parent process Id was passed on the commandline
		int ParentProcessId = _wtoi(ParentId);
		CHECKF(ParentProcessId > 0, TEXT("Invalid parent process Id"));

		const TCHAR* InputFileName = TEXT("\\WorkerInput.bin");
		wstring InputFilePath = wstring(WorkingDirectory) + InputFileName;

		bool bParentStillRunning = true;
		HANDLE ParentProcessHandle = OpenProcess(SYNCHRONIZE, false, ParentProcessId);
		// If we couldn't open the process then it is no longer running, exit
		if (ParentProcessHandle == NULL)
		{
			CHECKF(!IsFilePresentW(InputFilePath), TEXT("Exiting due to OpenProcess(ParentProcessId) failing and the input file is present!"));
			Log(TEXT("Couldn't OpenProcess, Parent process no longer running, exiting"));
			ExitCleanup(0);
		}
		else
		{
			// If we did open the process, that doesn't mean it is still running
			// The process object stays alive as long as there are handles to it
			// We need to check if the process has signaled, which indicates that it has exited
			DWORD WaitResult = WaitForSingleObject(ParentProcessHandle, 0);
			if (WaitResult != WAIT_TIMEOUT)
			{
				CHECKF(!IsFilePresentW(InputFilePath), TEXT("Exiting due to WaitForSingleObject(ParentProcessHandle) signaling and the input file is present!"));
				Log(TEXT("WaitForSingleObject signaled, Parent process no longer running, exiting"));
				ExitCleanup(0);
			}
			CloseHandle(ParentProcessHandle);
		}

		DWORD CurrentTime = timeGetTime();
		// If we have been idle for 20 seconds then exit
		if (CurrentTime - LastCompileTime > 20000)
		{
			Log(TEXT("No jobs found for 20 seconds, exiting"));
			ExitCleanup(0);
		}
	}
}

/** 
 * Main entrypoint, guarded by a try ... except.
 * This expects 4 parameters:
 *		The image path and name
 *		The working directory path, which has to be unique to the instigating process and thread.
 *		The parent process Id
 *		The thread Id corresponding to this worker
 */
int GuardedMain(int argc, _TCHAR* argv[])
{
	assert(argc == 4);

	// After WorkingDirectory is initialized it is safe to use CHECKF and ERRORF
	WorkingDirectory = argv[1];

	// After CreateLogFile it is safe to use Log
	CreateLogFile(argv[1]);

	//@todo - would be nice to change application name or description to have the ThreadId in it for debugging purposes
	SetConsoleTitle(argv[3]);
	// Filename that UE3 will write when it wants a job processed.
	//@todo - use memory mapped files for faster transfer
	// http://msdn.microsoft.com/en-us/library/aa366551(VS.85).aspx
	const TCHAR* InputFileName = TEXT("\\WorkerInput.bin");
	wstring InputFilePath = wstring(argv[1]) + InputFileName;

	LastCompileTime = timeGetTime();

	Log(TEXT("Entering job loop"));
	while (true)
	{
		FILE* InputFile = NULL;
		errno_t InputOpenReturn = 1;
		while (InputOpenReturn != 0)
		{
			// Try to open the input file that we are going to process
			InputOpenReturn = _wfopen_s(&InputFile, InputFilePath.c_str(), TEXT("rb"));
			if (InputOpenReturn != 0)
			{
				CheckExitConditions(argv[2], argv[1]);
				// Give up CPU time while we are waiting
				Sleep(10);
			}
		}
		CHECKF(InputFile != NULL, TEXT("Input file was not opened successfully"));
		Log(TEXT(""));
		Log(TEXT("Processing shader"));
		LastCompileTime = timeGetTime();

		// Read the job type out of the file
		EWorkerJobType JobType;
		ReadValue(InputFile, JobType);

		// Read the version number
		BYTE InputVersion;
		ReadValue(InputFile, InputVersion);

		vector<BYTE> OutputData;
		Log((wstring(TEXT("Job is ")) + GetJobTypeName(JobType)).c_str());

		// Call the appropriate shader compiler for each job type
		if (JobType == WJT_D3D9Shader)
		{
			extern bool D3D9CompileShader(BYTE InputVersion, FILE* InputFile, vector<BYTE>& OutputData);
			D3D9CompileShader(InputVersion, InputFile, OutputData);
		}
		else if (JobType == WJT_D3D10Shader)
		{
			extern bool D3D10CompileShader(BYTE InputVersion, FILE* InputFile, vector<BYTE>& OutputData);
			D3D10CompileShader(InputVersion, InputFile, OutputData);
		}
		else if (JobType == WJT_XenonShader || JobType == WJT_PS3Shader)
		{
			extern bool ConsoleCompileShader(EWorkerJobType JobType, BYTE InputVersion, FILE* InputFile, const TCHAR* WorkingDirectory, vector<BYTE>& OutputData);
			ConsoleCompileShader(JobType, InputVersion, InputFile, argv[1], OutputData);
		}
		else
		{
			ERRORF(TEXT("Unsupported job type"));
		}
		CHECKF(OutputData.size() > 0, TEXT("OutputData size was invalid"));

		fclose(InputFile);
		// Remove the input file so that it won't get processed more than once
		int RemoveResult = _wremove(InputFilePath.c_str());
		if (RemoveResult < 0)
		{
			int ErrorCode = errno;
			if (ErrorCode == 13)
			{
				ERRORF(TEXT("Couldn't delete input file, opened by another process"));
			}
			ERRORF(TEXT("Couldn't delete input file, is it readonly?"));
		}

		Log(TEXT("Writing output"));

		const TCHAR* OutputFileName = TEXT("\\WorkerOutput.bin");
		wstring OutputFilePath = wstring(argv[1]) + OutputFileName;
		// Write the output file that will indicate to UE3 that compilation has finished
		bool bWroteOutputFile = WriteFileW(OutputFilePath, &OutputData.front(), OutputData.size());
		CHECKF(bWroteOutputFile, (wstring(TEXT("Failed to write output file to ") + OutputFilePath).c_str()));
	}
	Log(TEXT("Exiting main loop"));
}

int _tmain(int argc, _TCHAR* argv[])
{
	int ReturnCode = 0;
	if (IsDebuggerPresent())
	{
		ReturnCode = GuardedMain(argc, argv);
	}
	else
	{
		__try
		{
			ReturnCode = GuardedMain(argc, argv);
		}
		__except( CreateMiniDump( GetExceptionInformation() ) )
		{
			//@todo - need to get the callstack and transfer to UE3
			ERRORF(TEXT("Unhandled exception!  Exiting"));
		}
	}

	ExitCleanup(ReturnCode);
	return ReturnCode;
}

