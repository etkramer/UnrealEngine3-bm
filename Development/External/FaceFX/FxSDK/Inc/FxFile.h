//------------------------------------------------------------------------------
// This class is responsible for abstracting away the platform-dependant aspects
// of file manipulation.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFile_H__
#define FxFile_H__

#include "FxPlatform.h"

namespace OC3Ent
{

namespace Face
{

/// An abstraction of platform-specific file handling.
/// \ingroup support
class FxFile
{
public:
	/// The modes in which to open this file.
	enum FxFileMode
	{
		FM_Read   = 1,	///< Read
		FM_Write  = 2,	///< Write
		FM_Text   = 4,	///< Text
		FM_Binary = 8	///< Binary
	};

	/// Default constructor.
	FxFile();
	/// Constructor. Opens the file in the selected mode.
	/// \param filename The filename to open.  Should be constructed using 
	/// FxPlatformPathSeparator.
	/// \param mode The mode in which to open the file.
	FxFile( const FxChar* filename, FxInt32 mode = FM_Read | FM_Binary );
	/// Destructor. Closes the file.
	~FxFile();

	/// Returns FxTrue if the file represented by filename exists.
	/// \param filename The filename to check.  Should be constructed using
	/// FxPlatformPathSeparator.
	/// \return FxTrue if the file exists or FxFalse otherwise.
	static FxBool FX_CALL Exists( const FxChar* filename );

	/// Reads the binary file located at filename into memory and returns a
	/// C-style array of FxBytes representing the file.  If the file cannot
	/// be loaded NULL is returned and numBytes is set to zero.
	/// \param filename The filename to load.  Should be constructed using
	/// FxPlatformPathSeparator.
	/// \param numBytes This is set to the number of bytes in the returned
	/// byte array representing the binary file in memory.
	/// \note This function internally allocates enough memory to contain the
	/// entire file and it is the caller's responsibility to free that memory
	/// by calling FxFree().
	static FxByte* FX_CALL ReadBinaryFile( const FxChar* filename, FxSize& numBytes );

	/// Returns FxTrue if the file is in a valid state or FxFalse otherwise.
	FxBool IsValid( void ) const;

	/// Opens the file in the selected mode.
	/// \param filename The filename to open.  Should be constructed using 
	/// FxPlatformPathSeparator.
	/// \param mode The mode in which to open the file.
	/// \return FxTrue if the file is in a valid state or FxFalse otherwise.
	FxBool Open( const FxChar* filename, FxInt32 mode = FM_Read | FM_Binary );
	/// Closes the file.
	void Close( void );

	/// Reads bytes from the file.
	FxSize Read( FxByte* data, FxSize numBytes );
	/// Writes bytes to the file.
	FxSize Write( const FxByte* data, FxSize numBytes );

	/// Returns the current position in the file relative to the beginning.
	FxInt32 Tell( void ) const;
	/// Sets the current position in the file relative to the beginning.
	void Seek( const FxInt32 pos );

	/// Returns the size of the file (in bytes).
	/// \note Only valid in \p FM_Read mode.
	FxInt32 Length( void ) const;

private:
	/// A pointer to the file.
	void* _file;
	/// The size of the file (in bytes).  Only valid in \p FM_Read mode.
	FxInt32 _length;
};

} // namespace Face

} // namespace OC3Ent

#endif
