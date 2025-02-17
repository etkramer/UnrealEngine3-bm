//-----------------------------------------------------------------------------
// File: GameuxRichSavedGame.h
//
// Desc: Header for rich save game functionality
//
// (C) Copyright Microsoft Corp.  All rights reserved.
//-----------------------------------------------------------------------------
#define RM_MAXLENGTH    1024
#define RM_MAGICNUMBER  'HMGR'

#pragma pack(push)
#pragma pack(1)
typedef struct _RICH_GAME_MEDIA_HEADER
{
    DWORD       dwMagicNumber;
    DWORD       dwHeaderVersion;
    DWORD       dwHeaderSize;
    LARGE_INTEGER liThumbnailOffset;
    DWORD       dwThumbnailSize;
    GUID        guidGameId;
    WCHAR       szGameName[RM_MAXLENGTH];
    WCHAR       szSaveName[RM_MAXLENGTH];
    WCHAR       szLevelName[RM_MAXLENGTH];
    WCHAR       szComments[RM_MAXLENGTH];
} RICH_GAME_MEDIA_HEADER;
#pragma pack(pop)
