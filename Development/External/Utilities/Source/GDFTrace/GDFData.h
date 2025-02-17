//--------------------------------------------------------------------------------------
// File: GDFData.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once

struct GDFRatingData
{
    WCHAR strRatingSystemGUID[256];
    WCHAR strRatingSystem[256];
    WCHAR strRatingIDGUID[256];
    WCHAR strRatingID[256];
    WCHAR strDescriptorGUID[32][256];
    WCHAR strDescriptor[32][256];
};

struct GDFData
{
    WORD wLanguage;
    WCHAR strLanguage[256];
    WCHAR strValidation[512];

    GDFRatingData ratingData[16];

    WCHAR strGameID[256];
    WCHAR strName[512];
    WCHAR strDescription[1025];
    WCHAR strReleaseDate[256];
    WCHAR strGenre[256];
    WCHAR strVersion[256];
    WCHAR strSavedGameFolder[256];
    int nSPRMin;
    int nSPRRecommended;
    WCHAR strDeveloper[256];
    WCHAR strDeveloperLink[256];
    WCHAR strPublisher[256];
    WCHAR strPublisherLink[256];

    WCHAR strExe[32][512];
};


HRESULT GetGDFData( GDFData* pGDFData, WCHAR* strGDFBinPath, WORD wLanguage = LANG_NEUTRAL );

