/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	Based on the FS Import classes:
	Copyright (C) 2005-2006 Feeling Software Inc
	Copyright (C) 2005-2006 Autodesk Media Entertainment
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FU_FILE_MANAGER_H_
#define _FU_FILE_MANAGER_H_

class FUFile;

class FCOLLADA_EXPORT FUFileManager
{
private:
	FStringList pathStack;
	bool forceAbsolute;

public:
	FUFileManager();
	~FUFileManager();

	// Root path stack
	const fstring& GetCurrentPath() { return pathStack.back(); }
	void PushRootPath(const fstring& path);
	void PopRootPath();
	void PushRootFile(const fstring& filename);
	void PopRootFile();

	// Some file access
	FUFile* OpenFile(const fstring& filename, bool write=true);

	// Extract information from filenames
	static fstring StripFileFromPath(const fstring& filename);
	static fstring GetFileExtension(const fstring& filename);
	static fstring ExtractNetworkHostname(fstring& filename);

	// Make a file path relative/absolute; returns OS-dependent filepaths
	fstring MakeFilePathAbsolute(const fstring& filePath) const;
	fstring MakeFilePathRelative(const fstring& filePath) const;

	// Transform a file URL into a file path
	fstring GetFilePath(const fstring& fileURL) const;

	// Transform a file path into a file URL
	fstring GetFileURL(const fstring& filepath, bool relative) const;

	// For a relative path, extract the list of the individual paths that must be traversed to get to the file.
	static void ExtractPathStack(const fstring& filename, fstring& hostname, fchar& driveLetter, FStringList& list, bool includeFilename);

	// Force the export of absolute paths and URLs
	void SetForceAbsoluteFlag(bool _forceAbsolute) { forceAbsolute = _forceAbsolute; }

	/** Generates a URI from its components
		@param scheme The access scheme. The empty string is used for relative URIs.
			Examples: file, mailto, https, ftp.
		@param hostname The name of the host for the information.
			The empty string is used for 'localhost'.
		@param filename The full access path and filename for the document.
		@return The newly constructed URI. */
	static fstring GenerateURI(const fstring& scheme, const fstring& hostname, const fstring& filename);

	/** Splits a URI into its components
		@param uri The URI to split.
		@param scheme The access scheme. The empty string is used for relative URIs.
			Examples: file, mailto, https, ftp.
		@param hostname The name of the host for the information.
			The empty string is used for 'localhost'.
		@param filename The full access path and filename for the document. */
	static void SplitURI(const fstring& uri, fstring& scheme, fstring& hostname, fstring& filename);
};

#endif // _FU_FILE_MANAGER_H_

