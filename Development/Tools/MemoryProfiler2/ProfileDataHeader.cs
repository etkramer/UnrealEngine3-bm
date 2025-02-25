using System;
using System.IO;

namespace MemoryProfiler2
{
	/**
	 * Header written by capture tool
	 */
	public class FProfileDataHeader
	{
		/** Magic number at beginning of data file.					*/
		public const UInt32 ExpectedMagic = 0xDA15F7D8;

		/** Magic to ensure we're opening the right file.			*/
		public UInt32 Magic;
		/** Version number to detect version mismatches.			*/
		public UInt32 Version;
		/** Platform that was captured.								*/
		public UInt32 Platform;
		/** Whether symbol information was serialized.				*/
		public bool	bShouldSerializeSymbolInfo;
		/** Name of executable this information was gathered with.	*/
		public string ExecutableName;

		/** Offset in file for name table.							*/
		public UInt32 NameTableOffset;
		/** Number of name table entries.							*/
		public UInt32 NameTableEntries;

		/** Offset in file for callstack address table.				*/
		public UInt32 CallStackAddressTableOffset;
		/** Number of callstack address entries.					*/
		public UInt32 CallStackAddressTableEntries;

		/** Offset in file for callstack table.						*/
		public UInt32 CallStackTableOffset;
		/** Number of callstack entries.							*/
		public UInt32 CallStackTableEntries;

		/** Number of data files the stream spans.					*/
		public UInt32 NumDataFiles;

		/**
		 * Constructor, serializing header from passed in stream.
		 * 
		 * @param	BinaryStream	Stream to serialize header from.
		 */
		public FProfileDataHeader(BinaryReader BinaryStream)
		{
			// Serialize the file format magic first.
			Magic = BinaryStream.ReadUInt32();

			// Stop serializing data if magic number doesn't match. Most likely endian issue.
			if( Magic == ExpectedMagic )
			{
				// Version info for backward compatible serialization.
				Version = BinaryStream.ReadUInt32();
				// Platform and max backtrace depth.
				Platform = BinaryStream.ReadUInt32();
				// Whether symbol information was serialized.
				bShouldSerializeSymbolInfo = BinaryStream.ReadUInt32() == 0 ? false : true;

				// Name table offset in file and number of entries.
				NameTableOffset = BinaryStream.ReadUInt32();
				NameTableEntries = BinaryStream.ReadUInt32();

				// CallStack address table offset and number of entries.
				CallStackAddressTableOffset = BinaryStream.ReadUInt32();
				CallStackAddressTableEntries = BinaryStream.ReadUInt32();

				// CallStack table offset and number of entries.
				CallStackTableOffset = BinaryStream.ReadUInt32();
				CallStackTableEntries = BinaryStream.ReadUInt32();

				// Number of data files the stream spans.
				NumDataFiles = BinaryStream.ReadUInt32();

				// Name of executable.
				UInt32 ExecutableNameLength = BinaryStream.ReadUInt32();
				ExecutableName = new string(BinaryStream.ReadChars((int)ExecutableNameLength));
				// We serialize a fixed size string. Trim the null characters that make it in by converting char[] to string.
				int RealLength = 0;
				while( ExecutableName[RealLength++] != '\0' ) 
				{
				}
				ExecutableName = ExecutableName.Remove(RealLength-1);
			}
		}
	}
}