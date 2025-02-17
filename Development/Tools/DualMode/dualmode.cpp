// dualmode.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <ctype.h>

using namespace std;

// our pipe to the executable feeding us console output
HANDLE coutPipe;
// event signalled by executable if we should go away early
HANDLE hDieEvent;
// handle to the executable process
HANDLE hProc=NULL;
// handle to our read thread
HANDLE hReadThread=INVALID_HANDLE_VALUE;

/** Event to tell the game proc to shutdown nicely */
HANDLE hTerminateRemoteAppEvent = NULL;

/** If true, use the event to shutdown */
BOOL bServer = FALSE;

#define CONNECTIMEOUT 1000

// handler for CTRL-C
BOOL WINAPI ConsoleCtrlHandler(DWORD)
{
	if(NULL != hProc)
	{
		if (bServer)
		{
			// Tell the game to die
			SetEvent(hTerminateRemoteAppEvent);
			// Wait for it to go
			WaitForSingleObject(hProc,INFINITE);
		}
		else
		{
			// No way to communicate, so kill it
			TerminateProcess(hProc,0);
		}

		// go back to white if possible
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);	
	}
	ExitProcess(0);
}

// Create named pipes for executable output
// Parameter: process id
BOOL 
CreateNamedPipes(DWORD pid)
{
	TCHAR name[256];

	_stprintf_s(name, 256, _T("\\\\.\\pipe\\%dcout"), pid);
	if (INVALID_HANDLE_VALUE == (coutPipe = CreateNamedPipe(name, 
															PIPE_ACCESS_INBOUND, 
															PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
															1,
															1024,
															1024,
															CONNECTIMEOUT,
															NULL)))
		return 0;
	_stprintf_s(name, 256, _T("dualmode_die_event%d"), pid);
	if(NULL == (hDieEvent = CreateEvent(NULL, TRUE, FALSE, name)))
	{
		return 0;
	}

	if ((hTerminateRemoteAppEvent = CreateEvent(NULL,TRUE,FALSE,_T("ComWrapperShutdown"))) == NULL)
	{
		return 0;
	}

	return 1;
}

// Close all named pipes and other handles
void
CloseHandles()
{
	CloseHandle(coutPipe);
	CloseHandle(hDieEvent);
	CloseHandle(hReadThread);
	CloseHandle(hTerminateRemoteAppEvent);

	// go back to white if possible
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);	
}

// size of buffer we read from executable
#define READ_BUFFER_SIZE 1025
// size of unicode to console codepage conversion buffer
#define AUX_BUFFER_SIZE 4096

#define UNI_COLOR_MAGIC L"`~[~`"

// Thread function that handles incoming bytesreams to be outputed
// on stdout
unsigned __stdcall
OutPipeTh(void*)
{
	TCHAR buffer[READ_BUFFER_SIZE];
	char auxBuffer[AUX_BUFFER_SIZE]; // make sure its big enough
	DWORD count = 0;
	DWORD consoleMode;
	DWORD numWritten;
	HANDLE hStandardOut = GetStdHandle(STD_OUTPUT_HANDLE);
	
	SetConsoleTextAttribute(hStandardOut, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);

	ConnectNamedPipe(coutPipe, NULL);

	// Use direct-to-console codepath if our stdout isn't being redirected somewhere
	// (for example, running as a VC++ external tool)
	// NOTE: this path won't work on Win95 or Win98, as they don't have a unicode console
	// if that's important, should also test for NT here
	if((0 != (GetFileType(hStandardOut) & FILE_TYPE_CHAR))
		&& GetConsoleMode(hStandardOut, &consoleMode))
	{
		// we're writing to a console, can use unicode
		while(ReadFile(coutPipe, buffer, READ_BUFFER_SIZE-1, &count, NULL))
		{
			if (_wcsnicmp(buffer, UNI_COLOR_MAGIC, wcslen(UNI_COLOR_MAGIC)) == 0)
			{
				// here we can change the color of the text to display, it's in the format:
				// ForegroundRed | ForegroundGreen | ForegroundBlue | ForegroundBright | BackgroundRed | BackgroundGreen | BackgroundBlue | BackgroundBright
				// where each value is either 0 or 1 (can leave off trailing 0's), so 
				// blue on bright yellow is "00101101" and red on black is "1"
				// An empty string reverts to the normal gray on black
				TCHAR* ColorData = buffer + wcslen(UNI_COLOR_MAGIC);
				if (_wcsicmp(ColorData, L"") == 0 || _wcsnicmp(ColorData, L"Reset", 5) == 0)
				{
					SetConsoleTextAttribute(hStandardOut, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
				}
				else
				{
					// turn the string into a bunch of 0's and 1's
					TCHAR String[9];
					memset(String, 0, sizeof(String));
					wcsncpy_s(String, 9, ColorData, 8);
					for (TCHAR* S = String; *S; S++)
					{
						if (*S < '0' || *S > '1')
						{
							*S = 0;
						}
						else
						{
							*S -= '0';
						}
					}
					// make the color
					SetConsoleTextAttribute(hStandardOut, 
						(String[0] ? FOREGROUND_RED			: 0) | 
						(String[1] ? FOREGROUND_GREEN		: 0) | 
						(String[2] ? FOREGROUND_BLUE		: 0) | 
						(String[3] ? FOREGROUND_INTENSITY	: 0) | 
						(String[4] ? BACKGROUND_RED			: 0) | 
						(String[5] ? BACKGROUND_GREEN		: 0) | 
						(String[6] ? BACKGROUND_BLUE		: 0) | 
						(String[7] ? BACKGROUND_INTENSITY	: 0) );
				}
			}
			else
			{
				WriteConsoleW(hStandardOut, buffer, count/sizeof(TCHAR), &numWritten, NULL);
			}
		}
	}
	else
	{
		// alternate path, we're not writing directly to the console

		// get the console output codepage
		int nOutputCP = GetConsoleOutputCP();

		while(ReadFile(coutPipe, buffer, READ_BUFFER_SIZE-1, &count, NULL))
		{
			// skip over any color controls
			if (_wcsnicmp(buffer, UNI_COLOR_MAGIC, wcslen(UNI_COLOR_MAGIC)) == 0)
			{
				continue;
			}
			// figure out how much room we'd need to convert to the console code page
		    int charCount = WideCharToMultiByte(nOutputCP, 0, buffer, count/sizeof(TCHAR), 0,
													0, 0, 0);
    
			// can't see how this could happen, since we're only reading 1024 TCHARs at a time
			// and going to the console code page should be smaller, but just to be safe
			if(charCount < AUX_BUFFER_SIZE)
			{
				WideCharToMultiByte( nOutputCP, 0, buffer, count/sizeof(TCHAR), auxBuffer, charCount, 0, 0);
				WriteFile(hStandardOut, auxBuffer, charCount, &numWritten, 0);
			}
			else
			{
				// conversion was too big, use stdio, not ideal
				buffer[count/sizeof(TCHAR)] = 0;
				wcout << buffer << flush;
			}
		}
	}

	return 0;
}


// Start handler pipe handler threads
void RunPipeThreads()
{
	hReadThread = (HANDLE) _beginthreadex(NULL, 0, OutPipeTh, NULL, 0, NULL);
}


int 
wmain(int , wchar_t* argv[])
{
	// create command line string based on program name
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

	_tsplitpath_s(argv[0], drive, dir, fname, ext);
	TCHAR *pArgs = GetCommandLine();

	// skip leading whitespace
	while(*pArgs != '\0' && _istspace(*pArgs))
		pArgs++;

	// skip the executable
	while(*pArgs != '\0' && !_istspace(*pArgs))
	{
		if(*pArgs == '\"')
		{
			// we're in a quote block
			pArgs++;
			while(*pArgs != '\0' && *pArgs != '\"')
				pArgs++;
			if(*pArgs != '\0')
			{
				pArgs++;
			}
		}
		else
		{
			pArgs++;
		}
	}
	if(*pArgs != '\0')
	{
		pArgs++;
	}
	const static DWORD CMDLINE_SIZE = 16384;
	TCHAR cLine[CMDLINE_SIZE];
	_stprintf_s(cLine, CMDLINE_SIZE, _T("%s%s%s.exe %s"), drive, dir, fname, pArgs);

	// Make lower case for consistent comparisons
	_tcslwr_s(pArgs,CMDLINE_SIZE);
	bServer = _tcsstr(pArgs,_T("server")) != NULL;


	// create process in suspended mode
	PROCESS_INFORMATION pInfo;
	STARTUPINFO sInfo;
	memset(&sInfo, 0, sizeof(STARTUPINFO));
	sInfo.cb = sizeof(STARTUPINFO);
//	printf("Calling... %S\n", cLine);
	if (!CreateProcess(NULL,
				  cLine, 
				  NULL,
				  NULL,
				  FALSE,
				  CREATE_SUSPENDED,
				  NULL,
				  NULL,
				  &sInfo,
				  &pInfo))
	{
		cerr << _T("ERROR: Could not create process.") << endl;
		return 1;
	}

	hProc = pInfo.hProcess;

	if (!CreateNamedPipes(pInfo.dwProcessId))
	{
		cerr << _T("ERROR: Could not create named pipes.") << endl;
		return 1;
	}

	RunPipeThreads();

	// resume process
	ResumeThread(pInfo.hThread);

	HANDLE hWait[] = { pInfo.hProcess, hDieEvent };

	DWORD result;

	result = WaitForMultipleObjects(2, hWait, FALSE, INFINITE);

	// wait for pipe read to finish, otherwise we might truncate data
	WaitForSingleObject(hReadThread, INFINITE);

	CloseHandles();

	ULONG exitCode;

	GetExitCodeProcess(pInfo.hProcess, (ULONG*)&exitCode);

	return exitCode;
}
