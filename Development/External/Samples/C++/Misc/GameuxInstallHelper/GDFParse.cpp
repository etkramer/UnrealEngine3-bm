//--------------------------------------------------------------------------------------
// File: GDFParse.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#include <windows.h>
#include <rpcsal.h>
#include <gameux.h>
#include <crtdbg.h>
#include <strsafe.h>
#include <assert.h>
#include <shellapi.h>
#include <shlobj.h>

#include "GDFParse.h"


//--------------------------------------------------------------------------------------
CGDFParse::CGDFParse(void)
{
    HRESULT hr = CoInitialize( 0 );
    m_bCleanupCOM = SUCCEEDED(hr); 
    m_pRootNode = NULL;
}


//--------------------------------------------------------------------------------------
CGDFParse::~CGDFParse(void)
{
    SAFE_RELEASE( m_pRootNode );
    if( m_bCleanupCOM )
        CoUninitialize();
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::ExtractXML( WCHAR* strGDFBinPath )
{
    SAFE_RELEASE( m_pRootNode );

    // Extract the GDF XML from the GDF binary 
    HMODULE hGDFDll = LoadLibrary( strGDFBinPath );
    if( hGDFDll )
    {
        // Find resource will pick the right ID_GDF_XML_STR based on the current language
        HRSRC hrsrc = FindResource( hGDFDll, ID_GDF_XML_STR, L"DATA" ); 
        if( hrsrc ) 
        { 
            HGLOBAL hgResource = LoadResource( hGDFDll, hrsrc ); 
            if( hgResource ) 
            { 
                BYTE* pResourceBuffer = (BYTE*)LockResource( hgResource ); 
                if( pResourceBuffer ) 
                { 
                    DWORD dwGDFXMLSize = SizeofResource( hGDFDll, hrsrc );
                    if( dwGDFXMLSize )
                    {
                        // HGLOBAL from LoadResource() needs to be copied for CreateStreamOnHGlobal() to work
                        HGLOBAL hgResourceCopy = GlobalAlloc( GMEM_MOVEABLE, dwGDFXMLSize );
                        if( hgResource )
                        {
                            LPVOID pCopy = GlobalLock( hgResourceCopy );
                            if( pCopy )
                            {
                                CopyMemory( pCopy, pResourceBuffer, dwGDFXMLSize );
                                GlobalUnlock( hgResource );

                                IStream* piStream = NULL;
                                CreateStreamOnHGlobal( hgResourceCopy, TRUE, &piStream ); 
                                if( piStream )
                                {
                                    IXMLDOMDocument *pDoc = NULL;
                                    HRESULT hr;

                                    // Load the XML into a IXMLDOMDocument object
                                    hr = CoCreateInstance( CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
                                                           IID_IXMLDOMDocument, (void**)&pDoc );
                                    if( SUCCEEDED(hr) ) 
                                    {
                                        IPersistStreamInit* pPersistStreamInit = NULL;
                                        hr = pDoc->QueryInterface( IID_IPersistStreamInit, (void**) &pPersistStreamInit );
                                        if( SUCCEEDED(hr) ) 
                                        {
                                            hr = pPersistStreamInit->Load( piStream );
                                            if( SUCCEEDED(hr) ) 
                                            {
                                                // Get the root node to the XML doc and store it 
                                                pDoc->QueryInterface( IID_IXMLDOMNode, (void**)&m_pRootNode );
                                            }
                                            SAFE_RELEASE( pPersistStreamInit );
                                        }
                                        SAFE_RELEASE( pDoc );
                                    }
                                    SAFE_RELEASE( piStream );
                                }
                            }
                            GlobalFree( hgResourceCopy );
                        }
                    }
                } 
            } 
        } 
    }

    if( m_pRootNode )
        return S_OK;
    else
        return E_FAIL;
}


//-----------------------------------------------------------------------------
// Converts a string to a GUID
//-----------------------------------------------------------------------------
BOOL ConvertStringToGUID( const WCHAR* strIn, GUID* pGuidOut )
{
    UINT aiTmp[10];

    if( swscanf( strIn, L"{%8X-%4X-%4X-%2X%2X-%2X%2X%2X%2X%2X%2X}",
                    &pGuidOut->Data1, 
                    &aiTmp[0], &aiTmp[1], 
                    &aiTmp[2], &aiTmp[3],
                    &aiTmp[4], &aiTmp[5],
                    &aiTmp[6], &aiTmp[7],
                    &aiTmp[8], &aiTmp[9] ) != 11 )
    {
        ZeroMemory( pGuidOut, sizeof(GUID) );
        return FALSE;
    }
    else
    {
        pGuidOut->Data2       = (USHORT) aiTmp[0];
        pGuidOut->Data3       = (USHORT) aiTmp[1];
        pGuidOut->Data4[0]    = (BYTE) aiTmp[2];
        pGuidOut->Data4[1]    = (BYTE) aiTmp[3];
        pGuidOut->Data4[2]    = (BYTE) aiTmp[4];
        pGuidOut->Data4[3]    = (BYTE) aiTmp[5];
        pGuidOut->Data4[4]    = (BYTE) aiTmp[6];
        pGuidOut->Data4[5]    = (BYTE) aiTmp[7];
        pGuidOut->Data4[6]    = (BYTE) aiTmp[8];
        pGuidOut->Data4[7]    = (BYTE) aiTmp[9];
        return TRUE;
    }
}


//--------------------------------------------------------------------------------------
// Various get methods
//--------------------------------------------------------------------------------------
HRESULT CGDFParse::GetName( WCHAR* strDest, int cchDest ) { return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Name", strDest, cchDest ); }
HRESULT CGDFParse::GetDescription( WCHAR* strDest, int cchDest ) { return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Description", strDest, cchDest ); }
HRESULT CGDFParse::GetReleaseDate( WCHAR* strDest, int cchDest ) { return GetXMLValue( L"//GameDefinitionFile/GameDefinition/ReleaseDate", strDest, cchDest ); }
HRESULT CGDFParse::GetGenre( WCHAR* strDest, int cchDest ) { return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Genres/Genre", strDest, cchDest ); }
HRESULT CGDFParse::GetDeveloper( WCHAR* strDest, int cchDest ) { return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Developers/Developer", strDest, cchDest ); }
HRESULT CGDFParse::GetPublisher( WCHAR* strDest, int cchDest ) { return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Publishers/Publisher", strDest, cchDest ); }
HRESULT CGDFParse::GetGameID( WCHAR* strDest, int cchDest ) { return GetXMLAttrib( L"//GameDefinitionFile/GameDefinition", L"gameID", strDest, cchDest );  }
/*
HRESULT CGDFParse::GetSavedGameFolder( WCHAR* strDest, int cchDest ) 
{ 
    WCHAR strSubFolder[MAX_PATH];   
    WCHAR strBaseFolder[MAX_PATH];
    GetXMLAttrib( L"//GameDefinitionFile/GameDefinition/SavedGames", L"path", strSubFolder, 256 );  

    HRESULT hr;
    hr = GetXMLAttrib( L"//GameDefinitionFile/GameDefinition/SavedGames", L"baseKnownFolderID", strBaseFolder, 256 );  
    if( FAILED(hr) )
    {
        GUID guidBaseFolder;
        ConvertStringToGUID( strBaseFolder, &guidBaseFolder ); 

        CoInitialize(NULL);
        IKnownFolderManager* pManager = NULL;
        IKnownFolder* pFolder = NULL;
        
        bool bFound = false
        HRESULT hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pManager) );
        if (SUCCEEDED(hr))
        {
            hr = pManager->GetFolder( guidBaseFolder, &pFolder );
            if( SUCCEEDED(hr) )
            {
                WCHAR* wstrPathInCoTaskMem = NULL;
                CoTaskMemFree( wstrPathInCoTaskMem );

                hr = pFolder->GetPath( KF_FLAG_DEFAULT_PATH | KF_FLAG_DONT_VERIFY, &wstrPathInCoTaskMem );
                if( SUCCEEDED(hr) )
                {
                    StringCchCopy( strBaseFolder, MAX_PATH, wstrPathInCoTaskMem );                    
                    bFound = true;
                }

                CoTaskMemFree( wstrPathInCoTaskMem );

                pFolder->Release();
                pFolder = NULL;
            }

            pManager->Release();
            pManager = NULL;
        }

        if( !bFound )
        {
            hr = SHGetFolderPath( guidBaseFolder, NULL, NULL, SHGFP_TYPE_CURRENT, strBaseFolder );
            if(SUCCEEDED(hr))
                bFound = true;
        }

        if( bFound )
            StringCchPrintf( strDest, cchDest, L"%s%s", strBasePath, strSubFolder );          
    }
    else
    {
        StringCchCopy( strDest, cchDest, L"%s", strSubFolder );          
        bFound = true;
    }

    if( bFound )
        return S_OK;
    else
        return E_FAIL;
}
*/

HRESULT CGDFParse::GetWinSPR( int* pnMin, int* pnRecommended ) 
{ 
    WCHAR strDest[256];

    GetXMLAttrib( L"//GameDefinitionFile/GameDefinition/WindowsSystemPerformanceRating", L"minimum", strDest, 256 ); 
    *pnMin = _wtoi( strDest );

    GetXMLAttrib( L"//GameDefinitionFile/GameDefinition/WindowsSystemPerformanceRating", L"recommended", strDest, 256 ); 
    *pnRecommended = _wtoi( strDest );

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::ExtractGDFThumbnail( WCHAR* strGDFBinPath, WCHAR* strDestFilePath )
{
    HGLOBAL hgResource = NULL; 
    HRSRC   hrsrc   = NULL; 
    BYTE* pGDFThumbnailBuffer = NULL;
    DWORD dwGDFThumbnailSize = 0;

    HMODULE hGDFDll = LoadLibrary( strGDFBinPath );
    if( hGDFDll )
    {
        // Extract GDF thumbnail 
        hrsrc = FindResource( hGDFDll, ID_GDF_THUMBNAIL_STR, L"DATA" ); 
        if( hrsrc ) 
        { 
            hgResource = LoadResource( hGDFDll, hrsrc ); 
            if( hgResource ) 
            { 
                BYTE* pResourceBuffer = (BYTE*)LockResource( hgResource ); 
                if( pResourceBuffer ) 
                { 
                    dwGDFThumbnailSize = SizeofResource( hGDFDll, hrsrc );
                    if( dwGDFThumbnailSize )
                    {
                        pGDFThumbnailBuffer = new BYTE[dwGDFThumbnailSize];
                        memcpy( pGDFThumbnailBuffer, pResourceBuffer, dwGDFThumbnailSize ); 

                        HANDLE hFileThumbnail = CreateFile( strDestFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
                        if( hFileThumbnail != INVALID_HANDLE_VALUE )
                        {
                            DWORD dwWritten;
                            WriteFile( hFileThumbnail, pGDFThumbnailBuffer, dwGDFThumbnailSize, &dwWritten, NULL );
                            CloseHandle( hFileThumbnail );
                        }

                        SAFE_DELETE_ARRAY( pGDFThumbnailBuffer );
                    }
                } 
            } 
        }
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::GetXMLValue( WCHAR* strXPath, WCHAR* strValue, int cchValue )
{
    assert( m_pRootNode );  // must call CGDFParse::ExtractXML() first
    if( !m_pRootNode )
        return E_FAIL; 

    IXMLDOMNode* pNode = NULL;
    m_pRootNode->selectSingleNode( strXPath, &pNode );
    if( pNode == NULL )
        return E_FAIL;

    VARIANT v;
    IXMLDOMNode* pChild = NULL;    
    pNode->get_firstChild(&pChild);
    if( pChild )
    {
        HRESULT hr = pChild->get_nodeTypedValue(&v);
        if( SUCCEEDED(hr) && v.vt == VT_BSTR )
            StringCchCopy( strValue, cchValue, v.bstrVal );
        VariantClear(&v);
        SAFE_RELEASE( pChild );
    }
    SAFE_RELEASE( pNode );

    return S_OK;
}

//--------------------------------------------------------------------------------------
HRESULT CGDFParse::GetXMLAttrib( WCHAR* strXPath, WCHAR* strAttribName, WCHAR* strValue, int cchValue )
{
    bool bFound = false;
    assert( m_pRootNode );  // must call CGDFParse::ExtractXML() first
    if( !m_pRootNode )
        return E_FAIL; 

    IXMLDOMNode* pNode = NULL;
    m_pRootNode->selectSingleNode( strXPath, &pNode );
    if( pNode == NULL )
        return E_FAIL;

    IXMLDOMNamedNodeMap *pIXMLDOMNamedNodeMap = NULL;
    BSTR bstrAttributeName = ::SysAllocString(strAttribName);
    IXMLDOMNode* pIXMLDOMNode = NULL;    
    
    HRESULT hr;
    VARIANT v;
    hr = pNode->get_attributes(&pIXMLDOMNamedNodeMap);
    if(SUCCEEDED(hr) && pIXMLDOMNamedNodeMap)
    {
        hr = pIXMLDOMNamedNodeMap->getNamedItem(bstrAttributeName, &pIXMLDOMNode);
        if(SUCCEEDED(hr) && pIXMLDOMNode)
        {
            pIXMLDOMNode->get_nodeValue(&v);
            if( SUCCEEDED(hr) && v.vt == VT_BSTR )
            {
                StringCchCopy( strValue, cchValue, v.bstrVal );
                bFound = true;
            }
            VariantClear(&v);
            SAFE_RELEASE( pIXMLDOMNode );
        }
        SAFE_RELEASE( pIXMLDOMNamedNodeMap );
    }

    ::SysFreeString(bstrAttributeName);
    bstrAttributeName = NULL;

    SAFE_RELEASE( pNode );

    if( !bFound )
        return E_FAIL;
    else
        return S_OK;
}

