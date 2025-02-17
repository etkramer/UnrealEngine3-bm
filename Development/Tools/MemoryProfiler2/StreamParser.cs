using System;
using System.IO;
using System.Collections.Generic;
using System.Windows.Forms;
using StandaloneSymbolParser;

namespace MemoryProfiler2
{	
	public static class FStreamParser
	{
		/**
		 * Parses the passed in token sream and returns list of snapshots.
		 */
		public static List<FStreamSnapshot> Parse(FStreamInfo StreamInfo,MainWindow MainMProfWindow)
		{
			MainMProfWindow.UpdateStatus("Loading header information for " + MainMProfWindow.CurrentFilename);

			// Create binary reader and file info object from filename.
			FileStream ParserFileStream = File.OpenRead(StreamInfo.FileName);
			BinaryReader BinaryStream = new BinaryReader(ParserFileStream,System.Text.Encoding.ASCII);

			// Serialize header.
			FProfileDataHeader Header = new FProfileDataHeader(BinaryStream);

			// Determine whether read file has magic header. If no, try again byteswapped.
			if(Header.Magic != FProfileDataHeader.ExpectedMagic)
			{
				// Seek back to beginning of stream before we retry.
				ParserFileStream.Seek(0,SeekOrigin.Begin);

				// Use big endian reader. It transparently endian swaps data on read.
				BinaryStream = new BinaryReaderBigEndian(ParserFileStream);
				
				// Serialize header a second time.
				Header = new FProfileDataHeader(BinaryStream);
			}

			// At this point we should have a valid header. If no, throw an exception.
			if( Header.Magic != FProfileDataHeader.ExpectedMagic )
			{
				throw new InvalidDataException();
			}

			// Initialize shared information across snapshots, namely names, callstacks and addresses.
			StreamInfo.Initialize(Header.NameTableEntries,Header.CallStackTableEntries,Header.CallStackAddressTableEntries);

			// Keep track of current position as it's where the token stream starts.
			long TokenStreamOffset = ParserFileStream.Position;

			// Seek to name table and serialize it.
			ParserFileStream.Seek(Header.NameTableOffset,SeekOrigin.Begin);
			for(int NameIndex = 0;NameIndex < Header.NameTableEntries;NameIndex++)
			{
				UInt32 Length = BinaryStream.ReadUInt32();
				StreamInfo.NameArray[NameIndex] = new string(BinaryStream.ReadChars((int)Length));
			}

			// Seek to callstack address array and serialize it.                
			ParserFileStream.Seek(Header.CallStackAddressTableOffset,SeekOrigin.Begin);
			for(int AddressIndex = 0;AddressIndex < Header.CallStackAddressTableEntries;AddressIndex++)
			{
				StreamInfo.CallStackAddressArray[AddressIndex] = new FCallStackAddress(BinaryStream, Header.bShouldSerializeSymbolInfo);
			}

			// Seek to callstack array and serialize it.
			ParserFileStream.Seek(Header.CallStackTableOffset,SeekOrigin.Begin);
			for(int CallStackIndex = 0;CallStackIndex < Header.CallStackTableEntries;CallStackIndex++)
			{
				StreamInfo.CallStackArray[CallStackIndex] = new FCallStack(BinaryStream);
			}

			// We need to look up symbol information ourselves if it wasn't serialized.
			if( !Header.bShouldSerializeSymbolInfo )
			{
				LookupSymbols( Header, StreamInfo, MainMProfWindow );
			}

			// Snapshot used for parsing. A copy will be made if a special token is encountered. Otherwise it
			// will be returned as the only snaphot at the end.
			FStreamSnapshot Snapshot = new FStreamSnapshot("End",StreamInfo);
			List<FStreamSnapshot> SnapshotList = new List<FStreamSnapshot>();

			MainMProfWindow.UpdateStatus("Parsing " + MainMProfWindow.CurrentFilename);

			// Seek to beginning of token stream.
			ParserFileStream.Seek(TokenStreamOffset,SeekOrigin.Begin);

			// Parse tokens till we reach the end of the stream.
			FStreamToken Token = new FStreamToken();
			while(Token.ReadNextToken(BinaryStream))
			{
				switch(Token.Type)
				{
					// Malloc
					case EProfilingPayloadType.TYPE_Malloc:
						HandleMalloc( Token, ref Snapshot );
						break;
					// Free
					case EProfilingPayloadType.TYPE_Free:
						HandleFree( Token, ref Snapshot );
						break;
					// Realloc
					case EProfilingPayloadType.TYPE_Realloc:
						HandleFree( Token, ref Snapshot );
						Token.Pointer = Token.NewPointer;
						HandleMalloc( Token, ref Snapshot );
						break;
					// Status/ payload.
					case EProfilingPayloadType.TYPE_Other:
						switch(Token.SubType)
						{
							// Should never receive EOS marker as ReadNextToken should've returned false.
							case EProfilingPayloadSubType.SUBTYPE_EndOfStreamMarker:
								throw new InvalidDataException();
							// Create snapshot.
							case EProfilingPayloadSubType.SUBTYPE_SnapshotMarker:
								FStreamSnapshot MarkerSnapshot = Snapshot.DeepCopy();
								SnapshotList.Add(MarkerSnapshot);
								break;
							// Unhandled.
							default:
								throw new InvalidDataException();
						}
						break;
					// Unhandled.
					default:
						throw new InvalidDataException();
				}
			}

			// Closes the file so it can potentially be opened for writing.
			ParserFileStream.Close();

			// Add snapshot in end state to the list and return it.
			SnapshotList.Add(Snapshot);

			MainMProfWindow.UpdateStatus("Finalizing snapshots for " + MainMProfWindow.CurrentFilename);

			// Finalize snapshots. This entails creating the sorted snapshot list.
			foreach( FStreamSnapshot SnapshotToFinalize in SnapshotList )
			{
				SnapshotToFinalize.FinalizeSnapshot(null);
			}

			return SnapshotList;
		}

		/** Updates internal state with allocation. */
		private static void HandleMalloc( FStreamToken Token, ref FStreamSnapshot Snapshot )
		{
		    // Disregard requests that didn't result in an allocation.
			if( Token.Pointer != 0 && Token.Size > 0 )
			{
				// Keep track of size associated with pointer and also current callstack.
                FCallStackAllocationInfo PointerInfo = new FCallStackAllocationInfo( Token.Size, Token.CallStackIndex, 1 );
				Snapshot.PointerToPointerInfoMap.Add( Token.Pointer, PointerInfo );
                // Add size to lifetime churn tracking.
                Snapshot.LifetimeCallStackList[Token.CallStackIndex].Add( Token.Size, 1 );
			}
		}

		/** Updates internal state with free. */
		private static void HandleFree( FStreamToken Token, ref FStreamSnapshot Snapshot )
		{
			// @todo: We currently seem to free/ realloc pointers we didn't allocate, which seems to be a bug that needs to 
			// be fixed in the engine. The below gracefully handles freeing pointers that either never have been allocated 
			// or are already freed.
			try
			{
				// Remove freed pointer if it is in the array.
				Snapshot.PointerToPointerInfoMap.Remove(Token.Pointer);
			}
			catch( System.Collections.Generic.KeyNotFoundException )
			{
			}
		}

		/** Lookup symbols for callstack addresses in passed in StreamInfo. */
		private static void LookupSymbols( FProfileDataHeader Header, FStreamInfo StreamInfo, MainWindow MainMProfWindow )
		{
			MainMProfWindow.UpdateStatus("Looking up symbols for " + MainMProfWindow.CurrentFilename);

			// Proper Symbol parser will be created based on platform.
			SymbolParser ConsoleSymbolParser = null;

            // Search for symbols in the same directory as the mprof first to allow packaging of previous results
            // PS3
            if (Header.Platform == 0x00000008)
            {
                ConsoleSymbolParser = new PS3SymbolParser();
                int LastPathSeparator = MainMProfWindow.CurrentFilename.LastIndexOf('\\');
                if (LastPathSeparator != -1)
                {
                    string CurrentPath = MainMProfWindow.CurrentFilename.Substring(0, LastPathSeparator);
                    if (!ConsoleSymbolParser.LoadSymbols(CurrentPath + "\\" + Header.ExecutableName + "xelf"))
                    {
                        ConsoleSymbolParser = null;
                    }
                }
            }
            // Xbox 360
            else if (Header.Platform == 0x00000004)
            {
                ConsoleSymbolParser = new XenonSymbolParser();
                int LastPathSeparator = MainMProfWindow.CurrentFilename.LastIndexOf('\\');
                if (LastPathSeparator != -1)
                {
                    string CurrentPath = MainMProfWindow.CurrentFilename.Substring(0, LastPathSeparator);
                    if (!ConsoleSymbolParser.LoadSymbols(CurrentPath + "\\" + Header.ExecutableName + ".exe"))
                    {
                        ConsoleSymbolParser = null;
                    }
                }
            }

            // If symbols weren't found in the same directory as the mprof, search the intermediate pdb locations
            if (ConsoleSymbolParser == null)
            {
                // PS3
                if (Header.Platform == 0x00000008)
                {
                    ConsoleSymbolParser = new PS3SymbolParser();
                    if (!ConsoleSymbolParser.LoadSymbols(Header.ExecutableName + "xelf"))
                    {
                        ConsoleSymbolParser = null;
                    }
                }
                // Xbox 360
                else if (Header.Platform == 0x00000004)
                {
                    ConsoleSymbolParser = new XenonSymbolParser();
                    if (!ConsoleSymbolParser.LoadSymbols("..\\" + Header.ExecutableName + ".exe"))
                    {
                        ConsoleSymbolParser = null;
                    }
                }
            }

			// If console parses is null it means that either the platform is not supported or the symbols couldn't
			// be loaded.
			if( ConsoleSymbolParser == null )
			{
				MessageBox.Show("Failed to load symbols for " + Header.ExecutableName);
				throw new InvalidDataException();
			}

			// Create mapping from string to index in name array.
			Dictionary<string, int> NameToIndexMap = new Dictionary<string,int>();

			// Propagate existing name entries to map.
			for( int NameIndex=0; NameIndex<StreamInfo.NameArray.Length; NameIndex++ )
			{
				NameToIndexMap.Add( StreamInfo.NameArray[NameIndex], NameIndex );
			}

			// Current index is incremented whenever a new name is added.
			int CurrentNameIndex = StreamInfo.NameArray.Length;

			// Iterate over all addresses and look up symbol information.
			foreach( FCallStackAddress Address in StreamInfo.CallStackAddressArray )
			{
				// Look up symbol info via console support DLL.
				string Filename = "";
				string Function = "";
				ConsoleSymbolParser.ResolveAddressToSymboInfo( (uint) Address.ProgramCounter, ref Filename, ref Function, ref Address.LineNumber );
	
				// Look up filename index.
				if( NameToIndexMap.ContainsKey(Filename) )
				{
					// Use existing entry.
					Address.FilenameIndex = NameToIndexMap[Filename];
				}
				// Not found, so we use global name index to set new one.
				else
				{
					// Set name in map associated with global ever increasing index.
					Address.FilenameIndex = CurrentNameIndex++;
					NameToIndexMap.Add( Filename, Address.FilenameIndex );
				}

				// Look up function index.
				if( NameToIndexMap.ContainsKey(Function) )
				{
					// Use existing entry.
					Address.FunctionIndex = NameToIndexMap[Function];
				}
				// Not found, so we use global name index to set new one.
				else
				{
					// Set name in map associated with global ever increasing index.
					Address.FunctionIndex = CurrentNameIndex++;
					NameToIndexMap.Add( Function, Address.FunctionIndex );
				} 
			}

			// Create new name array based on dictionary.
			StreamInfo.NameArray = new String[CurrentNameIndex];
			foreach (KeyValuePair<string, int> NameMapping in NameToIndexMap)
			{
				StreamInfo.NameArray[NameMapping.Value] = NameMapping.Key;
			}

			// Unload symbols again.
			ConsoleSymbolParser.UnloadSymbols();
		}
	};
}