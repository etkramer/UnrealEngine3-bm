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
//#include "atlcomcli.h"
#include <rpcsal.h>
#include <gameux.h>
#include <crtdbg.h>
#include <strsafe.h>
#include <assert.h>
#include <shellapi.h>
#include <shlobj.h>
#include <msxml6.h>

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
BOOL CALLBACK CGDFParse::StaticEnumResLangProc( HMODULE hModule, LPCWSTR lpType,
                                                LPCWSTR lpName, WORD wLanguage, LONG_PTR lParam )
{
    CGDFParse* pParse = (CGDFParse*)lParam;
    return pParse->EnumResLangProc( hModule, lpType, lpName, wLanguage );
}


//--------------------------------------------------------------------------------------
BOOL CGDFParse::EnumResLangProc( HMODULE hModule, LPCWSTR lpType, LPCWSTR lpName, WORD wLanguage )
{
    m_Languages[m_LanguageCount] = wLanguage;
    m_LanguageCount++;
    return TRUE;    
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::EnumLangs( WCHAR* strGDFBinPath )
{   
    ZeroMemory( &m_Languages, sizeof(WORD)*256 );
    m_LanguageCount = 0;

    // Extract the GDF XML from the GDF binary 
    HMODULE hGDFDll = LoadLibrary( strGDFBinPath );
    if( hGDFDll )
    {       
        EnumResourceLanguages( hGDFDll, L"DATA", ID_GDF_XML_STR, StaticEnumResLangProc, (LPARAM)this );        
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::LoadXMLinMemory( WCHAR* strGDFBinPath, WORD wLanguage, HGLOBAL* phResourceCopy )
{
   // Extract the GDF XML from the GDF binary 
    HMODULE hGDFDll = LoadLibrary( strGDFBinPath );
    if( hGDFDll )
    {
        // Find resource will pick the right ID_GDF_XML_STR based on the current language
        HRSRC hrsrc = FindResourceEx( hGDFDll, L"DATA", ID_GDF_XML_STR, wLanguage ); 
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
                        *phResourceCopy = GlobalAlloc( GMEM_MOVEABLE, dwGDFXMLSize );
                        if( *phResourceCopy )
                        {
                            LPVOID pCopy = GlobalLock( *phResourceCopy );
                            if( pCopy )
                            {
                                CopyMemory( pCopy, pResourceBuffer, dwGDFXMLSize );
                                GlobalUnlock( *phResourceCopy );
                                return S_OK;
                            }
                        }
                    }
                } 
            } 
        } 
    }
    return E_FAIL;
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::ExtractXML( WCHAR* strGDFBinPath, WORD wLanguage )
{
    SAFE_RELEASE( m_pRootNode );

    HGLOBAL hgResourceCopy = NULL;
    if( SUCCEEDED( LoadXMLinMemory( strGDFBinPath, wLanguage, &hgResourceCopy ) ) )
    {
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
        GlobalFree( hgResourceCopy );
    }

    if( m_pRootNode )
        return S_OK;
    else
        return E_FAIL;
}



//--------------------------------------------------------------------------------------
HRESULT CGDFParse::ValidateXML( WCHAR* strGDFBinPath, WORD wLanguage, WCHAR* strReason, int cchReason )
{
    bool bValidateSuccess = false;
    bool bLoadSuccess = false;
    StringCchCopy( strReason, cchReason, L"" );

    HGLOBAL hgResourceCopy = NULL;
    if( SUCCEEDED( LoadXMLinMemory( strGDFBinPath, wLanguage, &hgResourceCopy ) ) )
    {
        IStream* piStream = NULL;
        CreateStreamOnHGlobal( hgResourceCopy, TRUE, &piStream ); 
        if( piStream )
        {
            IXMLDOMDocument2* pDoc = NULL;
            HRESULT hr;

            // Load the XML into a IXMLDOMDocument object
            hr = CoCreateInstance( CLSID_DOMDocument60, NULL, CLSCTX_INPROC_SERVER, 
                                   IID_IXMLDOMDocument2, (void**)&pDoc );
            if( SUCCEEDED(hr) ) 
            {
                IPersistStreamInit* pPersistStreamInit = NULL;
                hr = pDoc->QueryInterface( IID_IPersistStreamInit, (void**) &pPersistStreamInit );
                if( SUCCEEDED(hr) ) 
                {
                    hr = pPersistStreamInit->Load( piStream );
                    if( SUCCEEDED(hr) ) 
                    {
                        bLoadSuccess = true;
                        bool bSkipValidate = false;

                        HRESULT hr;
                        IXMLDOMSchemaCollection* pSchemaCollection = NULL;
                        hr = CoCreateInstance( CLSID_XMLSchemaCache60, NULL, CLSCTX_INPROC_SERVER, 
                                               IID_IXMLDOMSchemaCollection, (void**)&pSchemaCollection );
                        if( SUCCEEDED(hr) && pSchemaCollection )
                        {
                            VARIANT var;

                            var.vt = VT_BSTR;
                            var.bstrVal = ::SysAllocString( L"GamesExplorerBaseTypes.v1.0.0.0.xsd" );
                            BSTR bstrNameSpace = ::SysAllocString( L"urn:schemas-microsoft-com:GamesExplorerBaseTypes.v1" );
                            hr = pSchemaCollection->add( bstrNameSpace, var );
                            if( FAILED(hr) )
                                bSkipValidate = true;
                            VariantClear( &var );

                            var.vt = VT_BSTR;
                            var.bstrVal = ::SysAllocString( L"GDFSchema.v1.0.0.0.xsd" );
                            bstrNameSpace = ::SysAllocString( L"urn:schemas-microsoft-com:GameDescription.v1" );
                            hr = pSchemaCollection->add( bstrNameSpace, var );
                            if( FAILED(hr) )
                                bSkipValidate = true;
                            VariantClear( &var );

                            var.vt = VT_DISPATCH;
                            var.byref = (VOID*) pSchemaCollection;
                              
                            hr = pDoc->putref_schemas( var ); 
                            if( FAILED(hr) )
                                bSkipValidate = true;
                            VariantClear( &var );

                            IXMLDOMParseError* pParseError = NULL;
                            if( bSkipValidate )
                            {
                                StringCchCopy( strReason, cchReason, L"Could not validate XML" );
                            }
                            else
                            {
                                hr = pDoc->validate( &pParseError );
                                if( hr == S_OK )
                                {
                                    bValidateSuccess = true;
                                }
                                else
                                {
                                    if( pParseError )
                                    {
                                        BSTR bstrReason = NULL;
                                        pParseError->get_reason( &bstrReason );
                                        if( bstrReason )
                                            StringCchCopy( strReason, cchReason, bstrReason );
                                    }
                                }
                            }

                            ::SysFreeString( bstrNameSpace );
                        }
                    }
                    SAFE_RELEASE( pPersistStreamInit );
                }
                SAFE_RELEASE( pDoc );
            }
            SAFE_RELEASE( piStream );
        }
        GlobalFree( hgResourceCopy );
    }

    if( !bLoadSuccess )
    {
        StringCchCopy( strReason, cchReason, L"Could not load XML.  Verify XML is valid" );
        return S_OK;
    }

    if( !bValidateSuccess && strReason[0] == 0 )
    {
        StringCchCopy( strReason, cchReason, L"Unknown reason for XML validation failure" );
        return S_OK;
    }

    return S_OK;
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
HRESULT CGDFParse::GetName( WCHAR* strDest, int cchDest )        { return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Name", strDest, cchDest ); }
HRESULT CGDFParse::GetDescription( WCHAR* strDest, int cchDest ) { return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Description", strDest, cchDest ); }
HRESULT CGDFParse::GetReleaseDate( WCHAR* strDest, int cchDest ) { return GetXMLValue( L"//GameDefinitionFile/GameDefinition/ReleaseDate", strDest, cchDest ); }
HRESULT CGDFParse::GetGenre( WCHAR* strDest, int cchDest )       { return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Genres/Genre", strDest, cchDest ); }
HRESULT CGDFParse::GetDeveloper( WCHAR* strDest, int cchDest )   { return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Developers/Developer", strDest, cchDest ); }
HRESULT CGDFParse::GetPublisher( WCHAR* strDest, int cchDest )   { return GetXMLValue( L"//GameDefinitionFile/GameDefinition/Publishers/Publisher", strDest, cchDest ); }
HRESULT CGDFParse::GetGameID( WCHAR* strDest, int cchDest )      { return GetXMLAttrib( L"//GameDefinitionFile/GameDefinition", L"gameID", strDest, cchDest );  }
HRESULT CGDFParse::GetDeveloperLink( WCHAR* strDest, int cchDest ) { return GetXMLAttrib( L"//GameDefinitionFile/GameDefinition/Developers/Developer", L"URI", strDest, cchDest );  }
HRESULT CGDFParse::GetPublisherLink( WCHAR* strDest, int cchDest ) { return GetXMLAttrib( L"//GameDefinitionFile/GameDefinition/Publishers/Publisher", L"URI", strDest, cchDest );  }
HRESULT CGDFParse::GetVersion( WCHAR* strDest, int cchDest )       { return GetXMLAttrib( L"//GameDefinitionFile/GameDefinition/Version/VersionNumber", L"versionNumber", strDest, cchDest );  }


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::GetGameExe( int iGameExe, WCHAR* strEXE, int cchEXE )
{
    HRESULT hr;
    assert( m_pRootNode );  // must call CGDFParse::ExtractXML() first
    if( !m_pRootNode )
        return E_FAIL; 

    StringCchCopy( strEXE, cchEXE, L"" );

    WCHAR str[512];
    IXMLDOMNode* pNode = NULL;
    StringCchPrintf( str, 512, L"//GameDefinitionFile/GameDefinition/GameExecutables/GameExecutable[%d]", iGameExe );          
    hr = m_pRootNode->selectSingleNode( str, &pNode );
    if( pNode )
    {
        hr = GetAttribFromNode( pNode, L"path", strEXE, cchEXE );
        SAFE_RELEASE( pNode );
        return hr;
    }

    return E_FAIL;
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::GetRatingSystem( int iRating, WCHAR* strRatingSystem, int cchRatingSystem )
{
    HRESULT hr;
    assert( m_pRootNode );  // must call CGDFParse::ExtractXML() first
    if( !m_pRootNode )
        return E_FAIL; 

    StringCchCopy( strRatingSystem, cchRatingSystem, L"" );

    WCHAR str[512];
    IXMLDOMNode* pNode = NULL;
    StringCchPrintf( str, 512, L"//GameDefinitionFile/GameDefinition/Ratings/Rating[%d]", iRating );          
    hr = m_pRootNode->selectSingleNode( str, &pNode );
    if( pNode )
    {
        hr = GetAttribFromNode( pNode, L"ratingSystemID", strRatingSystem, cchRatingSystem );
        SAFE_RELEASE( pNode );
        return hr;
    }

    return E_FAIL;
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::GetRatingID( int iRating, WCHAR* strRatingID, int cchRatingID )
{
    HRESULT hr;
    assert( m_pRootNode );  // must call CGDFParse::ExtractXML() first
    if( !m_pRootNode )
        return E_FAIL; 

    StringCchCopy( strRatingID, cchRatingID, L"" );

    WCHAR str[512];
    IXMLDOMNode* pNode = NULL;
    StringCchPrintf( str, 512, L"//GameDefinitionFile/GameDefinition/Ratings/Rating[%d]", iRating );          
    hr = m_pRootNode->selectSingleNode( str, &pNode );
    if( pNode )
    {
        hr = GetAttribFromNode( pNode, L"ratingID", strRatingID, cchRatingID );
        SAFE_RELEASE( pNode );
        return hr;
    }

    return E_FAIL;
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::GetRatingDescriptor( int iRating, int iDescriptor, WCHAR* strDescriptor, int cchDescriptor )
{
    HRESULT hr;
    assert( m_pRootNode );  // must call CGDFParse::ExtractXML() first
    if( !m_pRootNode )
        return E_FAIL; 

    StringCchCopy( strDescriptor, cchDescriptor, L"" );

    WCHAR str[512];
    IXMLDOMNode* pNode = NULL;
    StringCchPrintf( str, 512, L"//GameDefinitionFile/GameDefinition/Ratings/Rating[%d]", iRating );          
    hr = m_pRootNode->selectSingleNode( str, &pNode );
    if( pNode )
    {
        IXMLDOMNode* pDescriptorNode = NULL;

        WCHAR str[512];
        StringCchPrintf( str, 512, L"Descriptor[%d]", iDescriptor );          
        hr = pNode->selectSingleNode( str, &pDescriptorNode );
        if( pDescriptorNode )
        {
            hr = GetAttribFromNode( pDescriptorNode, L"descriptorID", strDescriptor, cchDescriptor );
        }
        else
        {
            hr = E_FAIL;
        }

        SAFE_RELEASE( pNode );
        return hr;
    }

    return E_FAIL;
}


//--------------------------------------------------------------------------------------
bool IsFolderMatch( WCHAR* strFolderGuid, WCHAR* strGUID, WCHAR* strFolder, WCHAR* strFolderName )
{
    if( _wcsicmp( strFolderGuid, strGUID ) == 0 )  
    {
        StringCchCopy( strFolderName, 256, strFolder );
        return true;
    }
    return false;
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::GetSavedGameFolder( WCHAR* strDest, int cchDest ) 
{ 
    WCHAR strSubFolder[MAX_PATH] = {0};   
    WCHAR strBaseFolder[MAX_PATH] = {0};

    GetXMLAttrib( L"//GameDefinitionFile/GameDefinition/SavedGames", L"path", strSubFolder, 256 );  

    HRESULT hr;
    hr = GetXMLAttrib( L"//GameDefinitionFile/GameDefinition/SavedGames", L"baseKnownFolderID", strBaseFolder, 256 );  
    if( SUCCEEDED(hr) )
    {
        GUID guidBaseFolder;
        ConvertStringToGUID( strBaseFolder, &guidBaseFolder ); 

        WCHAR strFolderName[256];
        ConvertGuidToFolderName( strBaseFolder, strFolderName );
/*
        CoInhritialize(NULL);
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
*/
        StringCchPrintf( strDest, cchDest, L"%s\\%s", strFolderName, strSubFolder );          
    }
    else
    {
        StringCchPrintf( strDest, cchDest, L"%s", strSubFolder );          
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
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


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::GetAttribFromNode( IXMLDOMNode* pNode, WCHAR* strAttrib, WCHAR* strDest, int cchDest )
{
    IXMLDOMNamedNodeMap *pIXMLDOMNamedNodeMap = NULL;
    BSTR bstrAttributeName = ::SysAllocString( strAttrib );
    IXMLDOMNode* pIXMLDOMNode = NULL;    
    bool bFound = false;

    HRESULT hr;
    VARIANT v;
    hr = pNode->get_attributes( &pIXMLDOMNamedNodeMap );
    if(SUCCEEDED(hr) && pIXMLDOMNamedNodeMap)
    {
        hr = pIXMLDOMNamedNodeMap->getNamedItem( bstrAttributeName, &pIXMLDOMNode );
        if(SUCCEEDED(hr) && pIXMLDOMNode)
        {
            pIXMLDOMNode->get_nodeValue(&v);
            if( SUCCEEDED(hr) && v.vt == VT_BSTR )
            {
                StringCchCopy( strDest, cchDest, v.bstrVal );
                bFound = true;
            }
            VariantClear(&v);
            SAFE_RELEASE( pIXMLDOMNode );
        }
        SAFE_RELEASE( pIXMLDOMNamedNodeMap );
    }

    ::SysFreeString(bstrAttributeName);
    bstrAttributeName = NULL;

    if( !bFound )
        return E_FAIL;
    else
        return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CGDFParse::ConvertGuidToFolderName( WCHAR* strFolderGuid, WCHAR* strFolderName )
{
    if( IsFolderMatch( strFolderGuid, L"{D20BEEC4-5CA8-4905-AE3B-BF251EA09B53}", L"FOLDERID_NetworkFolder", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{0AC0837C-BBF8-452A-850D-79D08E667CA7}", L"FOLDERID_ComputerFolder", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{4D9F7874-4E0C-4904-967B-40B0D20C3E4B}", L"FOLDERID_InternetFolder", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{82A74AEB-AEB4-465C-A014-D097EE346D63}", L"FOLDERID_ControlPanelFolder", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{76FC4E2D-D6AD-4519-A663-37BD56068185}", L"FOLDERID_PrintersFolder", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{43668BF8-C14E-49B2-97C9-747784D784B7}", L"FOLDERID_SyncManagerFolder", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{0F214138-B1D3-4a90-BBA9-27CBC0C5389A}", L"FOLDERID_SyncSetupFolder", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{4bfefb45-347d-4006-a5be-ac0cb0567192}", L"FOLDERID_ConflictFolder", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{289a9a43-be44-4057-a41b-587a76d7e7f9}", L"FOLDERID_SyncResultsFolder", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{B7534046-3ECB-4C18-BE4E-64CD4CB7D6AC}", L"FOLDERID_RecycleBinFolder", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{6F0CD92B-2E97-45D1-88FF-B0D186B8DEDD}", L"FOLDERID_ConnectionsFolder", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{FD228CB7-AE11-4AE3-864C-16F3910AB8FE}", L"FOLDERID_Fonts", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{B4BFCC3A-DB2C-424C-B029-7FE99A87C641}", L"FOLDERID_Desktop", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{B97D20BB-F46A-4C97-BA10-5E3608430854}", L"FOLDERID_Startup", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{A77F5D77-2E2B-44C3-A6A2-ABA601054A51}", L"FOLDERID_Programs", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{625B53C3-AB48-4EC1-BA1F-A1EF4146FC19}", L"FOLDERID_StartMenu", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{AE50C081-EBD2-438A-8655-8A092E34987A}", L"FOLDERID_Recent", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{8983036C-27C0-404B-8F08-102D10DCFD74}", L"FOLDERID_SendTo", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{FDD39AD0-238F-46AF-ADB4-6C85480369C7}", L"FOLDERID_Documents", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{1777F761-68AD-4D8A-87BD-30B759FA33DD}", L"FOLDERID_Favorites", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{C5ABBF53-E17F-4121-8900-86626FC2C973}", L"FOLDERID_NetHood", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{9274BD8D-CFD1-41C3-B35E-B13F55A758F4}", L"FOLDERID_PrintHood", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{A63293E8-664E-48DB-A079-DF759E0509F7}", L"FOLDERID_Templates", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{82A5EA35-D9CD-47C5-9629-E15D2F714E6E}", L"FOLDERID_CommonStartup", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{0139D44E-6AFE-49F2-8690-3DAFCAE6FFB8}", L"FOLDERID_CommonPrograms", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{A4115719-D62E-491D-AA7C-E74B8BE3B067}", L"FOLDERID_CommonStartMenu", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{C4AA340D-F20F-4863-AFEF-F87EF2E6BA25}", L"FOLDERID_PublicDesktop", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{62AB5D82-FDC1-4DC3-A9DD-070D1D495D97}", L"FOLDERID_ProgramData", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{B94237E7-57AC-4347-9151-B08C6C32D1F7}", L"FOLDERID_CommonTemplates", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{ED4824AF-DCE4-45A8-81E2-FC7965083634}", L"FOLDERID_PublicDocuments", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{3EB685DB-65F9-4CF6-A03A-E3EF65729F3D}", L"FOLDERID_RoamingAppData", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{F1B32785-6FBA-4FCF-9D55-7B8E7F157091}", L"FOLDERID_LocalAppData", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{A520A1A4-1780-4FF6-BD18-167343C5AF16}", L"FOLDERID_LocalAppDataLow", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{352481E8-33BE-4251-BA85-6007CAEDCF9D}", L"FOLDERID_InternetCache", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{2B0F765D-C0E9-4171-908E-08A611B84FF6}", L"FOLDERID_Cookies", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{D9DC8A3B-B784-432E-A781-5A1130A75963}", L"FOLDERID_History", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{1AC14E77-02E7-4E5D-B744-2EB1AE5198B7}", L"FOLDERID_System", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{D65231B0-B2F1-4857-A4CE-A8E7C6EA7D27}", L"FOLDERID_SystemX86", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{F38BF404-1D43-42F2-9305-67DE0B28FC23}", L"FOLDERID_Windows", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{5E6C858F-0E22-4760-9AFE-EA3317B67173}", L"FOLDERID_Profile", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{33E28130-4E1E-4676-835A-98395C3BC3BB}", L"FOLDERID_Pictures", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{7C5A40EF-A0FB-4BFC-874A-C0F2E0B9FA8E}", L"FOLDERID_ProgramFilesX86", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{DE974D24-D9C6-4D3E-BF91-F4455120B917}", L"FOLDERID_ProgramFilesCommonX86", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{6D809377-6AF0-444b-8957-A3773F02200E}", L"FOLDERID_ProgramFilesX64", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{6365D5A7-0F0D-45e5-87F6-0DA56B6A4F7D}", L"FOLDERID_ProgramFilesCommonX64", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{905e63b6-c1bf-494e-b29c-65b732d3d21a}", L"FOLDERID_ProgramFiles", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{F7F1ED05-9F6D-47A2-AAAE-29D317C6F066}", L"FOLDERID_ProgramFilesCommon", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{724EF170-A42D-4FEF-9F26-B60E846FBA4F}", L"FOLDERID_AdminTools", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{D0384E7D-BAC3-4797-8F14-CBA229B392B5}", L"FOLDERID_CommonAdminTools", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{4BD8D571-6D19-48D3-BE97-422220080E43}", L"FOLDERID_Music", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{18989B1D-99B5-455B-841C-AB7C74E4DDFC}", L"FOLDERID_Videos", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{B6EBFB86-6907-413C-9AF7-4FC2ABF07CC5}", L"FOLDERID_PublicPictures", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{3214FAB5-9757-4298-BB61-92A9DEAA44FF}", L"FOLDERID_PublicMusic", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{2400183A-6185-49FB-A2D8-4A392A602BA3}", L"FOLDERID_PublicVideos", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{8AD10C31-2ADB-4296-A8F7-E4701232C972}", L"FOLDERID_ResourceDir", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{2A00375E-224C-49DE-B8D1-440DF7EF3DDC}", L"FOLDERID_LocalizedResourcesDir", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{C1BAE2D0-10DF-4334-BEDD-7AA20B227A9D}", L"FOLDERID_CommonOEMLinks", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{9E52AB10-F80D-49DF-ACB8-4330F5687855}", L"FOLDERID_CDBurning", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{0762D272-C50A-4BB0-A382-697DCD729B80}", L"FOLDERID_UserProfiles", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{DE92C1C7-837F-4F69-A3BB-86E631204A23}", L"FOLDERID_Playlists", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{15CA69B3-30EE-49C1-ACE1-6B5EC372AFB5}", L"FOLDERID_SamplePlaylists", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{B250C668-F57D-4EE1-A63C-290EE7D1AA1F}", L"FOLDERID_SampleMusic", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{C4900540-2379-4C75-844B-64E6FAF8716B}", L"FOLDERID_SamplePictures", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{859EAD94-2E85-48AD-A71A-0969CB56A6CD}", L"FOLDERID_SampleVideos", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{69D2CF90-FC33-4FB7-9A0C-EBB0F0FCB43C}", L"FOLDERID_PhotoAlbums", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{DFDF76A2-C82A-4D63-906A-5644AC457385}", L"FOLDERID_Public", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{df7266ac-9274-4867-8d55-3bd661de872d}", L"FOLDERID_ChangeRemovePrograms", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{a305ce99-f527-492b-8b1a-7e76fa98d6e4}", L"FOLDERID_AppUpdates", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{de61d971-5ebc-4f02-a3a9-6c82895e5c04}", L"FOLDERID_AddNewPrograms", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{374DE290-123F-4565-9164-39C4925E467B}", L"FOLDERID_Downloads", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{3D644C9B-1FB8-4f30-9B45-F670235F79C0}", L"FOLDERID_PublicDownloads", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{7d1d3a04-debb-4115-95cf-2f29da2920da}", L"FOLDERID_SavedSearches", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{52a4f021-7b75-48a9-9f6b-4b87a210bc8f}", L"FOLDERID_QuickLaunch", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{56784854-C6CB-462b-8169-88E350ACB882}", L"FOLDERID_Contacts", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{A75D362E-50FC-4fb7-AC2C-A8BEAA314493}", L"FOLDERID_SidebarParts", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{7B396E54-9EC5-4300-BE0A-2482EBAE1A26}", L"FOLDERID_SidebarDefaultParts", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{5b3749ad-b49f-49c1-83eb-15370fbd4882}", L"FOLDERID_TreeProperties", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{DEBF2536-E1A8-4c59-B6A2-414586476AEA}", L"FOLDERID_PublicGameTasks", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{054FAE61-4DD8-4787-80B6-090220C4B700}", L"FOLDERID_GameTasks", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{4C5C32FF-BB9D-43b0-B5B4-2D72E54EAAA4}", L"FOLDERID_SavedGames", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{CAC52C1A-B53D-4edc-92D7-6B2E8AC19434}", L"FOLDERID_Games", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{bd85e001-112e-431e-983b-7b15ac09fff1}", L"FOLDERID_RecordedTV", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{98ec0e18-2098-4d44-8644-66979315a281}", L"FOLDERID_SEARCH_MAPI", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{ee32e446-31ca-4aba-814f-a5ebd2fd6d5e}", L"FOLDERID_SEARCH_CSC", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{bfb9d5e0-c6a9-404c-b2b2-ae6db6af4968}", L"FOLDERID_Links", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{f3ce0f7c-4901-4acc-8648-d5d44b04ef8f}", L"FOLDERID_UsersFiles", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{190337d1-b8ca-4121-a639-6d472d16972a}", L"FOLDERID_SearchHome", strFolderName )) return S_OK;
    if( IsFolderMatch( strFolderGuid, L"{2C36C0AA-5812-4b87-BFD0-4CD0DFB19B39}", L"FOLDERID_OriginalImages", strFolderName )) return S_OK;
    StringCchCopy( strFolderName, 256, strFolderGuid ); 
    return S_OK;
}


