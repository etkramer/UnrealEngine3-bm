//------------------------------------------------------------------------------
// A container for the Fonix neural net and dictionary files.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFonixDataArchive_H__
#define FxFonixDataArchive_H__

#include "FxPlatform.h"
#include "FxString.h"
#include "FxArray.h"

namespace OC3Ent
{

namespace Face
{

// The data file information in the header.
struct FxFonixDataArchiveHeaderDataFileInfo
{
	FxString filename;
	FxSize   offset;
};

// The language information in the header.
struct FxFonixDataArchiveHeaderLanguageInfo
{
	FxString name;
	FxString timeoutDate;

	FxArray<FxFonixDataArchiveHeaderDataFileInfo> dataFiles;
};

// The header information for the data archive.
struct FxFonixDataArchiveHeader
{
	FxString customerName;
	FxString project;
	FxString guid;
	FxString version;
	FxString buildId;

	FxArray<FxFonixDataArchiveHeaderLanguageInfo> languages;
};

// A single fonix data file.
struct FxFonixDataFileChunk
{
	FxString filename;
	FxArray<FxByte> fileContents;
};

// The entire fonix data archive.
class FxFonixDataArchive
{
public:
	FxFonixDataArchive();
	~FxFonixDataArchive();

	// Saves the data archive.
	FxBool SaveDataArchive(const FxChar* path);
	// Loads the data archive.
	FxBool LoadDataArchive(const FxChar* path);

	// Loads a data chunk from the archive.
	FxBool LoadDataChunk(const FxChar* fonixFilename, FxArray<FxByte>& dataChunk);

	// Gets the default language.
	FxString GetDefaultLanguage() const;
	// Gets the timeout date for a given language
	FxString GetTimeoutDate(const FxChar* language) const;

	FxFonixDataArchiveHeader      archiveHeader;
	FxArray<FxFonixDataFileChunk> dataFiles;

	static void AddOffset(const FxFonixDataArchiveHeaderDataFileInfo& offsetInfo);

private:
	// The path to the archive.
	FxString _archivePath;

	static FxArray<FxFonixDataArchiveHeaderDataFileInfo> _offsetArray;
};

} // namespace Face

} // namespace OC3Ent

#endif
