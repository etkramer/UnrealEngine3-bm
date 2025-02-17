//-----------------------------------------------------------------------------
// File: GDFTrace.cpp
//
// Desc: Windows code that calls GameuxInstallHelper sample dll and displays the results.
//
// (C) Copyright Microsoft Corp.  All rights reserved.
//-----------------------------------------------------------------------------
#define _WIN32_DCOM
#define _CRT_SECURE_NO_DEPRECATE
#include <rpcsal.h>
#include <gameux.h>
#include <shellapi.h>
#include <strsafe.h>
#include <shlobj.h>
#include <wbemidl.h>
#include <objbase.h>
#define NO_SHLWAPI_STRFCNS
#include <shlwapi.h>
#include "GDFParse.h"
#include "RatingsDB.h"
#include "GDFData.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

struct SETTINGS
{
    WCHAR strGDFBinPath[MAX_PATH];
    WCHAR strInstallPath[MAX_PATH];
    bool bMuteGDF;
    bool bMuteWarnings;
    bool bQuiet;    
};

SETTINGS g_settings;
int g_nNumberOfWarnings = 0;

bool ParseCommandLine( SETTINGS* pSettings );
bool IsNextArg( WCHAR*& strCmdLine, WCHAR* strArg );
void DisplayUsage();


//-----------------------------------------------------------------------------
void OutputGDFData( GDFData* pGDFData )
{
    wprintf( L"Language: %s (0x%0.4x)\n", pGDFData->strLanguage, pGDFData->wLanguage );
    wprintf( L"\tName: %s\n", pGDFData->strName );
    wprintf( L"\tDescription: %s\n", pGDFData->strDescription );
    wprintf( L"\tRelease Date: %s\n", pGDFData->strReleaseDate );
    wprintf( L"\tGenre: %s\n", pGDFData->strGenre );

    for( int iRating=0; iRating<16; iRating++ )
    {
        if( pGDFData->ratingData[iRating].strRatingSystem[0] == 0 )
            break;
        wprintf( L"\tRating: %s, %s", pGDFData->ratingData[iRating].strRatingSystem, pGDFData->ratingData[iRating].strRatingID );

        for( int iDescriptor=0; iDescriptor<32; iDescriptor++ )
        {
            if( pGDFData->ratingData[iRating].strDescriptor[iDescriptor][0] == 0 )
                break;
            wprintf( L", %s", pGDFData->ratingData[iRating].strDescriptor[iDescriptor] );
        }

        wprintf( L"\n" );
    }

    wprintf( L"\tVersion: %s\n", pGDFData->strVersion );
    wprintf( L"\tSaved Game Folder: %s\n", pGDFData->strSavedGameFolder );
    wprintf( L"\tWinSPR Min: %d\n", pGDFData->nSPRMin );
    wprintf( L"\tWinSPR Recommended: %d\n", pGDFData->nSPRRecommended );
    wprintf( L"\tDeveloper: %s\n", pGDFData->strDeveloper );
    wprintf( L"\tDeveloper Link: %s\n", pGDFData->strDeveloperLink );
    wprintf( L"\tPublisher: %s\n", pGDFData->strPublisher );
    wprintf( L"\tPublisher Link: %s\n", pGDFData->strPublisherLink );

    for( int iGame=0; iGame<32; iGame++ )
    {   
        if( pGDFData->strExe[iGame][0] == 0 ) 
            break;
        wprintf( L"\tEXE: %s\n", pGDFData->strExe[iGame] );
    }

    wprintf( L"\n" );
}


//-----------------------------------------------------------------------------
bool FindRatingSystem( WCHAR* strRatingSystemGUID, GDFData* pGDFData, int* pRatingIndex )
{
    for( int iRating=0; iRating<16; iRating++ )
    {
        if( pGDFData->ratingData[iRating].strRatingSystemGUID[0] == 0 )
            return false;

        if( _wcsicmp( strRatingSystemGUID, pGDFData->ratingData[iRating].strRatingSystemGUID ) == 0 )
        {
            *pRatingIndex = iRating;
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
void OutputWarning( LPCWSTR strMsg, ... )
{
    WCHAR strBuffer[1024];   
    va_list args;
    va_start(args, strMsg);
    StringCchVPrintfW( strBuffer, 512, strMsg, args );
    strBuffer[1023] = L'\0';
    va_end(args);
    wprintf( strBuffer );

    g_nNumberOfWarnings++;
}


//-----------------------------------------------------------------------------
bool g_bOuputLangHeader = false;
void EnsureOutputRatingHeader( GDFData* pGDFData1, GDFData* pGDFData2 )
{
    if( !g_bOuputLangHeader ) 
    {
        wprintf( L"\tComparing %s [0x%0.4x] with %s [0x%0.4x]\n", pGDFData1->strLanguage, pGDFData1->wLanguage, pGDFData2->strLanguage, pGDFData2->wLanguage );
        g_bOuputLangHeader = true;
    }
}

//-----------------------------------------------------------------------------
void OutputRatingWarning( GDFData* pGDFData1, GDFData* pGDFData2, LPCWSTR strMsg, ... )
{
    WCHAR strBuffer[1024];
    EnsureOutputRatingHeader( pGDFData1, pGDFData2 );
    
    va_list args;
    va_start(args, strMsg);
    StringCchVPrintfW( strBuffer, 512, strMsg, args );
    strBuffer[1023] = L'\0';
    va_end(args);

    OutputWarning( strBuffer );
}


//-----------------------------------------------------------------------------
bool CompareRatingSystems( GDFData* pGDFData1, GDFData* pGDFData2 )
{
    bool bWarningsFound = false;

    for( int iRating1=0; iRating1<16; iRating1++ )
    {
        GDFRatingData* pRating1 = &pGDFData1->ratingData[iRating1];
        if( pRating1->strRatingSystemGUID[0] == 0 )
            break;

        int iRating2 = 0;
        if( FindRatingSystem( pRating1->strRatingSystemGUID, pGDFData2, &iRating2 ) )
        {
            //wprintf( L"\t\tInfo: Rating system %s found in %s lang\n", pRating1->strRatingSystem, pGDFData2->strLanguage );

            GDFRatingData* pRating2 = &pGDFData2->ratingData[iRating2];
            if( _wcsicmp( pRating1->strRatingID, pRating2->strRatingID ) != 0 )
            {
                OutputRatingWarning( pGDFData1, pGDFData2, L"\tWarning: %s rating mismatch: %s vs %s \n", pRating1->strRatingSystem, pRating1->strRatingID, pRating2->strRatingID );
                bWarningsFound = true;
            }                    
            else
            {
                //wprintf( L"\t\tInfo: %s rating match: %s vs %s \n", pRating1->strRatingSystem, pRating1->strRatingID, pRating2->strRatingID );
            }

            for( int iDescriptor1=0; iDescriptor1<32; iDescriptor1++ )
            {
                if( pRating1->strDescriptor[iDescriptor1][0] == 0 )
                    break;

                bool bFound = false;
                for( int iDescriptor2=0; iDescriptor2<32; iDescriptor2++ )
                {
                    if( pRating2->strDescriptor[iDescriptor2][0] == 0 )
                        break;

                    if( _wcsicmp( pRating1->strDescriptor[iDescriptor1], pRating2->strDescriptor[iDescriptor2] ) == 0 )
                    {
                        bFound = true;
                        break;
                    }
                }
                if( !bFound )
                {
                    OutputRatingWarning( pGDFData1, pGDFData2, L"\tWarning: %s rating descriptor not found: %s\n", pRating1->strRatingSystem, pRating1->strDescriptor[iDescriptor1] );
                    bWarningsFound = true;
                }
                else
                {
                    //wprintf( L"\t\tInfo: %s rating descriptor found: %s\n", pRating1->strRatingSystem, pRating1->strDescriptor[iDescriptor1] );
                }
            }
        }
        else
        {
            OutputRatingWarning( pGDFData1, pGDFData2, L"\tWarning: Rating system %s not found in %s lang\n", pRating1->strRatingSystem, pGDFData2->strLanguage );
            bWarningsFound = true;
        }
    }

    return bWarningsFound;
}


//-----------------------------------------------------------------------------
void CompareGDFData( GDFData* pGDFData1, GDFData* pGDFData2, bool bQuiet )
{
    g_bOuputLangHeader = false;
    bool bSPRWarningsFound = false;
    bool bGameIDWarningsFound = false;

    if( pGDFData1->nSPRMin != pGDFData2->nSPRMin )
    {
        bSPRWarningsFound = true;
        OutputRatingWarning( pGDFData1, pGDFData2, L"\t\tWarning: Mismatched SPR min: %d vs %d\n", pGDFData1->nSPRMin, pGDFData2->nSPRMin );
    }
    if( pGDFData1->nSPRRecommended != pGDFData2->nSPRRecommended )
    {
        bSPRWarningsFound = true;
        OutputRatingWarning( pGDFData1, pGDFData2, L"\t\tWarning: Mismatched SPR recommended: %d vs %d\n", pGDFData1->nSPRRecommended, pGDFData2->nSPRRecommended );
    }

    if( _wcsicmp( pGDFData1->strGameID, pGDFData2->strGameID ) != 0 )
    {
        bGameIDWarningsFound = true;
        OutputRatingWarning( pGDFData1, pGDFData2, L"\t\tWarning: Mismatched game ID guid: %s vs %s\n", pGDFData1->strGameID, pGDFData2->strGameID );
    }

    bool bExeWarningsFound = false;
    for( int iGame=0; iGame<32; iGame++ )
    {   
        if( pGDFData1->strExe[iGame][0] == 0 && pGDFData2->strExe[iGame][0] == 0 )
            break;
        if( _wcsicmp( pGDFData1->strExe[iGame], pGDFData2->strExe[iGame] ) != 0 )
        {
            bExeWarningsFound = true;
            OutputRatingWarning( pGDFData1, pGDFData2, L"\t\tWarning: Game EXE mismatch: %s vs %s\n", pGDFData1->strExe[iGame], pGDFData2->strExe[iGame] );
        }
    }

    bool bWarningsFound1 = CompareRatingSystems( pGDFData1, pGDFData2 );
    bool bWarningsFound2 = CompareRatingSystems( pGDFData2, pGDFData1  );

    if( !bWarningsFound1 && !bWarningsFound2 && !bExeWarningsFound && !bSPRWarningsFound && !bGameIDWarningsFound )
    {
        // Matching Game ID, ratings, exes, and SPR data
        if( !bQuiet )
        {
            EnsureOutputRatingHeader( pGDFData1, pGDFData2 );
            wprintf( L"\t\tNo data mismatch found\n" );    
        }
    }
}


//-----------------------------------------------------------------------------
HRESULT ScanForWarnings( GDFData* pGDFDataList, int nNumLangs, bool bQuiet )
{
    wprintf( L"Warnings:\n" );

    // Loop through all languages and warn if there's no language neutral 
    bool bFoundNeutral = false;
    for( int iLang1=0; iLang1<nNumLangs; iLang1++ )
    {
        GDFData* pGDFData1 = &pGDFDataList[iLang1];
        if( LOBYTE(pGDFData1->wLanguage) == LANG_NEUTRAL ) 
        {
            bFoundNeutral = true;
        }
    }
    if( !bFoundNeutral ) 
        OutputWarning( L"\tWarning: Language neutral not found.  Adding one is highly recommended to cover all other languages\n" );

    // Warn if there's any missing data or if there were XML validation warnings
    for( int iLang=0; iLang<nNumLangs; iLang++ )
    {
        WCHAR strHeader[256];
        StringCchPrintf( strHeader, 256, L"\t%s (0x%0.4x): ", pGDFDataList[iLang].strLanguage, pGDFDataList[iLang].wLanguage );

        if( pGDFDataList[iLang].strValidation[0] != 0  )
            OutputWarning( L"%s%s\n", strHeader, pGDFDataList[iLang].strValidation );
        else if( !bQuiet )
            wprintf( L"%sNo validation warnings found\n", strHeader );

        if( pGDFDataList[iLang].strPublisher[0] == 0 )
            OutputWarning( L"%sPublisher field is blank\n", strHeader );
        if( pGDFDataList[iLang].strPublisherLink[0] == 0 )
            OutputWarning( L"%sPublisher link field is blank\n", strHeader );
        if( pGDFDataList[iLang].strDeveloper[0] == 0 )
            OutputWarning( L"%sDeveloper field is blank\n", strHeader );
        if( pGDFDataList[iLang].strDeveloperLink[0] == 0 )
            OutputWarning( L"%sDeveloper link field is blank\n", strHeader );
        if( pGDFDataList[iLang].strGenre[0] == 0 )
            OutputWarning( L"%sGenre field is blank\n", strHeader );
        if( pGDFDataList[iLang].strDescription[0] == 0 )
            OutputWarning( L"%sDescription field is blank\n", strHeader );
        if( pGDFDataList[iLang].nSPRMin == pGDFDataList[iLang].nSPRRecommended )
            OutputWarning( L"%sWinSPR minimum and recommended are the same.  Ensure this is intended.\n", strHeader );
        if( pGDFDataList[iLang].strExe[0][0] == 0 )
            OutputWarning( L"%sNo EXEs listed\n", strHeader );
        if( pGDFDataList[iLang].ratingData[0].strRatingSystemGUID[0] == 0 )
            OutputWarning( L"%sNo ratings data found\n", strHeader );

        for( int iRating=0; iRating<16; iRating++ )
        {
            if( pGDFDataList[iLang].ratingData[iRating].strRatingSystemGUID[0] == 0 )
                break;
            if( wcscmp( pGDFDataList[iLang].ratingData[iRating].strRatingIDGUID, L"{6B9EB3C0-B49A-4708-A6E6-F5476CE7567B}" ) == 0 )
            {
                OutputWarning( L"%sUnsupported CERO rating found.  Use latest GDFMaker to fix.\n", strHeader );
            }
        }
    }

    // Loop through all languages comparing GDF data and printing warnings
    for( int iLang1=0; iLang1<nNumLangs; iLang1++ )
    {
        for( int iLang2=iLang1+1; iLang2<nNumLangs; iLang2++ )
        {
            GDFData* pGDFData1 = &pGDFDataList[iLang1];
            GDFData* pGDFData2 = &pGDFDataList[iLang2];

            CompareGDFData( pGDFData1, pGDFData2, bQuiet );
        }
    }

    if( g_nNumberOfWarnings == 0 )
    {
        wprintf( L"\tNo warnings found\n" );
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
HRESULT ProcessGDF( WCHAR* strGDFBinPath, bool bMuteWarnings, bool bMuteGDF, bool bQuiet )
{
    HRESULT hr;

    CRatingsDB ratingsDB;
    ratingsDB.LoadDB();

    CGDFParse gdfParse;
    gdfParse.EnumLangs( strGDFBinPath );

    GDFData* pGDFDataList = new GDFData[gdfParse.GetNumLangs()];
    ZeroMemory( pGDFDataList, sizeof(GDFData)*gdfParse.GetNumLangs() );

    for( int iLang=0; iLang<gdfParse.GetNumLangs(); iLang++ )
    {
        WORD wLang = gdfParse.GetLang( iLang );

        hr = GetGDFData( &pGDFDataList[iLang], strGDFBinPath, wLang );
        if( FAILED(hr) )
        {
            wprintf( L"Couldn't load GDF data from: %s (wLang:0x%0.4x)\n", strGDFBinPath, wLang );
            if( pGDFDataList[iLang].strValidation[0] != 0 )
            {
                wprintf( L"%s\n", pGDFDataList[iLang].strValidation );
            }
            continue;
        }
    }

    if( !bMuteGDF )
    {
        for( int iLang=0; iLang<gdfParse.GetNumLangs(); iLang++ )
        {
            OutputGDFData( &pGDFDataList[iLang] );
        }
    }

    if( !bMuteWarnings )
        ScanForWarnings( pGDFDataList, gdfParse.GetNumLangs(), bQuiet );

    return S_OK;
}


//-----------------------------------------------------------------------------
// Entry point to the program. Initializes everything, and pops
// up a message box with the results of the GameuxInstallHelper calls
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    SETTINGS settings;
    memset(&settings, 0, sizeof(SETTINGS));
    settings.bQuiet = true;

    if( !ParseCommandLine( &settings ) )
        return 0;

    ProcessGDF( settings.strGDFBinPath, settings.bMuteWarnings, settings.bMuteGDF, settings.bQuiet );

    if( g_nNumberOfWarnings == 0 )
        return 0;
    else
        return 1;
}


//--------------------------------------------------------------------------------------
// Parses the command line for parameters.  See DXUTInit() for list 
//--------------------------------------------------------------------------------------
bool ParseCommandLine( SETTINGS* pSettings )
{
    WCHAR* strCmdLine;
//    WCHAR* strArg;

    int nNumArgs;
    WCHAR** pstrArgList = CommandLineToArgvW( GetCommandLine(), &nNumArgs );
    for( int iArg=1; iArg<nNumArgs; iArg++ )
    {
        strCmdLine = pstrArgList[iArg];

        // Handle flag args
        if( *strCmdLine == L'/' || *strCmdLine == L'-' )
        {
            strCmdLine++;

            if( IsNextArg( strCmdLine, L"mutegdf" ) )
            {
                pSettings->bMuteGDF = true;
                continue;
            }

            if( IsNextArg( strCmdLine, L"mutewarnings" ) )
            {
                pSettings->bMuteWarnings = true;
                continue;
            }

            if( IsNextArg( strCmdLine, L"noisy" ) )
            {
                pSettings->bQuiet = false;
                continue;
            }

            if( IsNextArg( strCmdLine, L"?" ) )
            {
                DisplayUsage();
                return false;
            }
        }
        else 
        {
            // Handle non-flag args as separate input files
            StringCchPrintf( pSettings->strGDFBinPath, MAX_PATH, L"%s", strCmdLine );
            continue;
        }
    }

    if( pSettings->strGDFBinPath[0] == 0 )
    {
        DisplayUsage();
        return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------
bool IsNextArg( WCHAR*& strCmdLine, WCHAR* strArg )
{
    int nArgLen = (int) wcslen(strArg);
    if( _wcsnicmp( strCmdLine, strArg, nArgLen ) == 0 && strCmdLine[nArgLen] == 0 )
        return true;

    return false;
}


//--------------------------------------------------------------------------------------
void DisplayUsage()
{
    wprintf( L"\n" );
    wprintf( L"GDFTrace - a command line tool that displays GDF metadata contained\n" );
    wprintf( L"           in a binary and highlights any warnings\n" );
    wprintf( L"\n" );
    wprintf( L"Usage: GDFTrace.exe [options] <gdf binary>\n" );
    wprintf( L"\n" );
    wprintf( L"where:\n" ); 
    wprintf( L"\n" ); 
    wprintf( L"  [/mutegdf]     \tmutes output of GDF data\n" );
    wprintf( L"  [/mutewarnings]\tmutes output of warnings\n" );
    wprintf( L"  [/noisy]       \tenables output of success\n" );
    wprintf( L"  <gdf binary>\tThe path to the GDF binary\n" );
    wprintf( L"\n" );
    wprintf( L"After running, %%ERRORLEVEL%% will be 0 if no warnings are found,\n" );
    wprintf( L"and 1 otherwise.\n" );
    wprintf( L"\n" );
    wprintf( L"As an example, you can use GDFExampleBinary.dll found in the DXSDK.\n" );
}
