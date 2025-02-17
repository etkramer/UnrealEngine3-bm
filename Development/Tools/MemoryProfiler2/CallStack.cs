using System;
using System.IO;
using System.Collections.Generic;
using System.Windows.Forms;

namespace MemoryProfiler2
{
    /**
     * Encapsulates callstack information
     */
    public class FCallStack
    {
        /** CRC of callstack pointers. */
        private Int32 CRC;

        /** Callstack as indices into address list, from top to bottom. */
        public List<int> AddressIndices;

		/** Whether this callstack is truncated. */
		public bool	bIsTruncated;

        /**
         * Constructor
         * 
         * @param	BinaryStream	Stream to read from
         */
        public FCallStack(BinaryReader BinaryStream)
        {
            // Read CRC of original callstack.
            CRC = BinaryStream.ReadInt32();

            // Create new arrays.
            AddressIndices = new List<int>();

            // Read call stack address indices and parse into arrays.
            int AddressIndex = BinaryStream.ReadInt32();
            while(AddressIndex >= 0)
            {
                AddressIndices.Add(AddressIndex);
                AddressIndex = BinaryStream.ReadInt32();
            }

			// Normal callstacks are -1 terminated, whereof truncated ones are -2 terminated.
			if( AddressIndex == -2 )
			{
				bIsTruncated = true;
			}
			else
			{
				bIsTruncated = false;
			}

            // We added bottom to top but prefer top bottom for hierarchical view.
            AddressIndices.Reverse();
        }

		/**
		 * Compares two callstacks for sorting.
		 * 
		 * @param	A	First callstack to compare
		 * @param	B	Second callstack to compare
		 */
		public static int Compare( FCallStack A, FCallStack B )
		{
			// Not all callstacks have the same depth. Figure out min for comparision.
			int MinSize = Math.Min( A.AddressIndices.Count, B.AddressIndices.Count );

			// Iterate over matching size and compare.
			for( int i=0; i<MinSize; i++ )
			{
				// Sort by address
				if( A.AddressIndices[i] > B.AddressIndices[i] )
				{
					return 1;
				}
				else if( A.AddressIndices[i] < B.AddressIndices[i] )
				{
					return -1;
				}
			}

			// If we got here it means that the subset of addresses matches. In theory this means
			// that the callstacks should have the same size as you can't have the same address
			// doing the same thing, but let's simply be thorough and handle this case if the
			// stackwalker isn't 100% accurate.

			// Matching length means matching callstacks.
			if( A.AddressIndices.Count == B.AddressIndices.Count )
			{
				return 0;
			}
			// Sort by additional length.
			else
			{
				return A.AddressIndices.Count > B.AddressIndices.Count ? 1 : -1;
			}
		}

		/**
		 * Converts the current callstack to a string.
		 */
		public string ToString( FStreamInfo StreamInfo, bool bShowFunctionName, bool bShowFileName, bool bShowLineNumber, bool bShowFromBottomUp, bool bUseNewLineForEachAddress )
		{
			// Iterate over all entries in specified order and add them to string.
			string ResultString = "";
			for( int i=0; i<AddressIndices.Count; i++ )
			{
				// Handle iterating over addresses in reverse order.
				int AddressIndexIndex = bShowFromBottomUp ? AddressIndices.Count - 1 - i : i;
				FCallStackAddress Address = StreamInfo.CallStackAddressArray[AddressIndices[AddressIndexIndex]];
				// Function
				if( bShowFunctionName )
				{
					ResultString += StreamInfo.NameArray[Address.FunctionIndex] + " ";
				}
				// File
				if( bShowFileName )
				{
					ResultString += StreamInfo.NameArray[Address.FilenameIndex];
					// Line, only shown if file is.
					if( bShowLineNumber )
					{
						ResultString += ":" + Address.LineNumber;
					}
					ResultString += " ";
				}

				// Use " -> " to deliminate call stack entries.
				if( i != AddressIndices.Count-1 )
				{
					if( bUseNewLineForEachAddress )
					{
						ResultString += Environment.NewLine;
					}
					else
					{
						ResultString += " -> ";
					}
				}
			}

			// Ensure string doesn't end in space.
			ResultString.TrimEnd();
			return ResultString;
		}

		public void AddToListView( FStreamInfo StreamInfo, ListView CallStackListView, bool bShowFromBottomUp )
		{
			for( int i=0; i<AddressIndices.Count; i++ )
			{
				// Handle iterating over addresses in reverse order.
				int AddressIndexIndex = bShowFromBottomUp ? AddressIndices.Count - 1 - i : i;
				FCallStackAddress Address = StreamInfo.CallStackAddressArray[AddressIndices[AddressIndexIndex]];
				
				string[] Row = new string[]
				{
					StreamInfo.NameArray[Address.FunctionIndex],
					StreamInfo.NameArray[Address.FilenameIndex],
					Address.LineNumber.ToString()
				};
				CallStackListView.Items.Add( new ListViewItem(Row) );
			}
		}

    };
}