//--------------------------------------------------------------------------------------
// File: GameuxInstallHelper.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <gameux.h>
#include <msi.h>
#include <msiquery.h>

//--------------------------------------------------------------------------------------
// UNICODE/ANSI define mappings
//--------------------------------------------------------------------------------------
#ifdef UNICODE
    #define AddToGameExplorer AddToGameExplorerW
    #define RetrieveGUIDForApplication RetrieveGUIDForApplicationW
    #define RemoveRichSavedGames RemoveRichSavedGamesW
    #define SetupRichSavedGames SetupRichSavedGamesW
#else
    #define AddToGameExplorer AddToGameExplorerA
    #define RetrieveGUIDForApplication RetrieveGUIDForApplicationA
    #define RemoveRichSavedGames RemoveRichSavedGamesA
    #define SetupRichSavedGames SetupRichSavedGamesA
#endif 

//--------------------------------------------------------------------------------------
// Given a game instance GUID and path to GDF binary, registers game with Game Explorer
//
// [in] strGDFBinPath: the full path to the GDF binary 
// [in] strGameInstallPath: the full path to the folder where the game is installed.  
//                          This folder will be under the protection of parental controls after this call
// [in] InstallScope: if the game is being installed for all users or just the current user 
// [in] pGameInstanceGUID: the guid of the game instance.  create this guid prior to calling and pass this to other functions 
//                         once the game is added, you can call RetrieveGUIDForApplication() to look it up again during uninstall.
//--------------------------------------------------------------------------------------
STDAPI AddToGameExplorerW( WCHAR* strGDFBinPath, WCHAR* strGameInstallPath, GAME_INSTALL_SCOPE InstallScope, GUID* pGameInstanceGUID );
STDAPI AddToGameExplorerA( CHAR* strGDFBinPath, CHAR* strGameInstallPath, GAME_INSTALL_SCOPE InstallScope, GUID* pGameInstanceGUID );

//--------------------------------------------------------------------------------------
// Register with Media Center using the data from a GDF binary
//
// [in] strGDFBinPath: the full path to the GDF binary 
// [in] strGameInstallPath: the full path to the folder where the game is installed.  
//                          this function will write metadata files for media center registration here.
//                          the process must have write access to this folder.  
// [in] InstallScope: if the game is being installed for all users or just the current user 
// [in] strExePath: the full path to the exe that should be launched from media center
// [in] strCommandLineArgs: any additional command line args for the exe when launched from media center
// [in] bUseRegisterMCEApp: if true, it creates an XML and calls RegisterMCEApp.exe, otherwise it creates an MCL in a known location
//--------------------------------------------------------------------------------------
STDAPI RegisterWithMediaCenterW( WCHAR* strGDFBinPath, WCHAR* strGameInstallPath, GAME_INSTALL_SCOPE InstallScope, WCHAR* strExePath, WCHAR* strCommandLineArgs, bool bUseRegisterMCEApp );
STDAPI RegisterWithMediaCenterA( CHAR* strGDFBinPath, CHAR* strGameInstallPath, GAME_INSTALL_SCOPE InstallScope, CHAR* strExePath, CHAR* strCommandLineArgs, bool bUseRegisterMCEApp );

//--------------------------------------------------------------------------------------
// Unregister with Media Center 
//
// [in] strGameInstallPath: the full path to the folder where the game is installed
// [in] InstallScope: if the game is being installed for all users or just the current user 
// [in] strExePath: the full path to the exe that should be launched from media center
// [in] bUseRegisterMCEApp: if true, it uses RegisterMCEApp.exe, otherwise it removes the MCL from a known location
//--------------------------------------------------------------------------------------
STDAPI UnRegisterWithMediaCenterW( WCHAR* strGameInstallPath, GAME_INSTALL_SCOPE InstallScope, WCHAR* strExePath, bool bUseRegisterMCEApp );
STDAPI UnRegisterWithMediaCenterA( CHAR* strGameInstallPath, GAME_INSTALL_SCOPE InstallScope, CHAR* strExePath, bool bUseRegisterMCEApp );

//--------------------------------------------------------------------------------------
// Given a path to a GDF binary that has already been registered, returns a game instance GUID
//
// [in] strGDFBinPath: the full path to the GDF binary 
// [out] pGameInstanceGUID: the guid of the game instance
//--------------------------------------------------------------------------------------
STDAPI RetrieveGUIDForApplicationW( WCHAR* strGDFBinPath, GUID* pGameInstanceGUID );
STDAPI RetrieveGUIDForApplicationA( CHAR* strGDFBinPath, GUID* pGameInstanceGUID );

//--------------------------------------------------------------------------------------
// Given a game instance GUID, unregisters a game from Game Explorer
//
// [in] pGameInstanceGUID: the guid of the game instance
//--------------------------------------------------------------------------------------
STDAPI RemoveFromGameExplorer( GUID* pGameInstanceGUID );

//--------------------------------------------------------------------------------------
// Creates a unique game instance GUID
//
// [out] pGameInstanceGUID: the guid of the game instance
//--------------------------------------------------------------------------------------
STDAPI GenerateGUID( GUID* pGameInstanceGUID );

//--------------------------------------------------------------------------------------
// Given a a game instance GUID, creates a task shortcut in the proper location
//
// [in] strGameInstallPath: the full path to the folder where the game is installed
// [in] pGameInstanceGUID: the guid of the game instance
// [in] bSupportTask: if false, this task is a play task.  if true, this task is a support task.  note: support tasks can only be web hyperlinks
// [in] nTaskID: 0 based index of task.  up to 6 support tasks and 5 support tasks are allowed
// [in] strTaskName: name of the task that the user will see
// [in] strLaunchPath: path to the task (eg. c:\program files\examplegame\examplegame.exe) or web page (eg http://www.microsoft.com)
// [in] strCommandLineArgs: optional command line arguements
//--------------------------------------------------------------------------------------
STDAPI CreateTaskW( GAME_INSTALL_SCOPE InstallScope, GUID* pGameInstanceGUID, BOOL bSupportTask, int nTaskID, WCHAR* strTaskName, WCHAR* strLaunchPath, WCHAR* strCommandLineArgs );
STDAPI CreateTaskA( GAME_INSTALL_SCOPE InstallScope, GUID* pGameInstanceGUID, BOOL bSupportTask, int nTaskID, CHAR* strTaskName, CHAR* strLaunchPath, CHAR* strCommandLineArgs );

//--------------------------------------------------------------------------------------
// This removes all the tasks associated with a game instance GUID
// Pass in a valid GameInstance GUID that was passed to AddGame()
//
// [in] pGameInstanceGUID: the guid of the game instance
//--------------------------------------------------------------------------------------
STDAPI RemoveTasks( GUID* pGameInstanceGUID ); 

//--------------------------------------------------------------------------------------
// Creates the registry keys to enable rich saved games.  The game still needs to use 
// the rich saved game header as defined in the documentation and support loading a 
// saved game from the command line.
//
// [in] strSavedGameExtension: extension of the rich saved game for game.  must begin with a period. ex: .ExampleSaveGame
// [in] strLaunchPath: path to exe to launch when rich saved game is double clicked.  should be enclosed in quotes.  ex: "%ProgramFiles%\ExampleGame\ExampleGame.exe"
// [in] strCommandLineToLaunchSavedGame: optional command line args. should be enclosed in quotes.  ex: "%1".  If NULL, it defaults to "%1"
//--------------------------------------------------------------------------------------
STDAPI SetupRichSavedGamesW( WCHAR* strSavedGameExtension, WCHAR* strLaunchPath, WCHAR* strCommandLineToLaunchSavedGame = NULL );
STDAPI SetupRichSavedGamesA( CHAR* strSavedGameExtension, CHAR* strLaunchPath, CHAR* strCommandLineToLaunchSavedGame = NULL );

//-----------------------------------------------------------------------------
// Removes the registry keys to enable rich saved games.  
//
// [in] strSavedGameExtension: extension of the rich saved game for game.  must begin with a period. ex: .ExampleSaveGame
//-----------------------------------------------------------------------------
STDAPI RemoveRichSavedGamesW( WCHAR* strSavedGameExtension );
STDAPI RemoveRichSavedGamesA( CHAR* strSavedGameExtension );

//--------------------------------------------------------------------------------------
// For use during an MSI custom action install. 
// This sets up the CustomActionData properties for the deferred custom actions. 
//--------------------------------------------------------------------------------------
UINT WINAPI SetMSIGameExplorerProperties( MSIHANDLE hModule );

//--------------------------------------------------------------------------------------
// For use during an MSI custom action install. 
// This adds the game to the Game Explorer
//--------------------------------------------------------------------------------------
UINT WINAPI AddToGameExplorerUsingMSI( MSIHANDLE hModule );

//--------------------------------------------------------------------------------------
// For use during an MSI custom action install. 
// This removes the game to the Game Explorer
//--------------------------------------------------------------------------------------
UINT WINAPI RemoveFromGameExplorerUsingMSI( MSIHANDLE hModule );



