using System;
using System.IO;
using System.Collections.Generic;

namespace MemoryProfiler2
{
	/**
	 * Helper class encapsulating information for a callstack and associated allocation size. Shared with stream parser.
	 */
	public class FCallStackAllocationInfo
	{
		/** Size of allocation. */
		public long Size;
		/** Index of callstack that performed allocation. */
		public int CallStackIndex;
		/** Number of allocations. */
		public int Count;

		/** Constructor, initializing all member variables to passed in values. */
		public FCallStackAllocationInfo( long InSize, int InCallStackIndex, int InCount )
		{
			Size = InSize;
			CallStackIndex = InCallStackIndex;
			Count = InCount;
		}

        /**
         * Adds the passed in size and count to this callstack info.
         * 
         * @param   SizeToAdd       Size to add
         * @param   CountToAdd      Count to add
         */
        public void Add( long SizeToAdd, int CountToAdd )
        {
            Size += SizeToAdd;
            Count += CountToAdd;
        }

        /**
         * Diffs the two passed in callstack infos and returns the difference.
         * 
         * @param   New     Newer callstack info to subtract older from
         * @param   Old     Older callstack info to subtract from older
         * 
         * @return  diffed callstack info
         */
        public static FCallStackAllocationInfo Diff( FCallStackAllocationInfo New, FCallStackAllocationInfo Old )
        {
            if( New.CallStackIndex != Old.CallStackIndex )
            {
                throw new InvalidDataException();
            }
            return new FCallStackAllocationInfo( New.Size - Old.Size, New.CallStackIndex, New.Count - Old.Count );
        }
	};

	/**
	 * Snapshot of allocation state at a specific time in token stream. 
	 */
	public class FStreamSnapshot
	{
		/** User defined description of time of snapshot. */
		string Description;
		/** Shared stream info this snapshot is using. */
		FStreamInfo StreamInfo;
		/** Mapping from active allocation pointer to further information about allocation. */
		public Dictionary<UInt32,FCallStackAllocationInfo> PointerToPointerInfoMap;
		/** List of callstack allocations. */
		public List<FCallStackAllocationInfo> ActiveCallStackList;
        /** List of lifetime callstack allocations for memory churn. */
        public List<FCallStackAllocationInfo> LifetimeCallStackList;

		/** Constructor, naming the snapshot and initializing map. */
		public FStreamSnapshot( string InDescription, FStreamInfo InStreamInfo )
		{
			Description = InDescription;
			PointerToPointerInfoMap = new Dictionary<UInt32,FCallStackAllocationInfo>();
			StreamInfo = InStreamInfo;
			ActiveCallStackList = new List<FCallStackAllocationInfo>();
            // Presize lifetime callstack array and populate.
            LifetimeCallStackList = new List<FCallStackAllocationInfo>( StreamInfo.CallStackArray.Length );
            for( int CallStackIndex=0; CallStackIndex<StreamInfo.CallStackArray.Length; CallStackIndex++ )
            {
                LifetimeCallStackList.Add( new FCallStackAllocationInfo( 0, CallStackIndex, 0 ) );
            }
		}

		/** Performs a deep copy of the relevant data structures. */
		public FStreamSnapshot DeepCopy()
		{
			// Create new snaphot object.
			FStreamSnapshot Snapshot = new FStreamSnapshot("Copy",StreamInfo);
			// Manually perform a deep copy of PointerToPointerInfoMap
			foreach( KeyValuePair<UInt32,FCallStackAllocationInfo> Info in PointerToPointerInfoMap)
			{
				Snapshot.PointerToPointerInfoMap.Add( Info.Key, new FCallStackAllocationInfo( Info.Value.Size, Info.Value.CallStackIndex, Info.Value.Count ) );
			}
            // Manually perform a deep copy of LifetimeCallstackList
            Snapshot.LifetimeCallStackList.Clear();
            foreach( FCallStackAllocationInfo Info in LifetimeCallStackList )
            {
                Snapshot.LifetimeCallStackList.Add( new FCallStackAllocationInfo( Info.Size, Info.CallStackIndex, Info.Count ) );
            }
			// The ActiveCallStackList is going to be populated in FinalizeSnapshot.
			Snapshot.ActiveCallStackList.Clear();
			// Return deep copy of this snapshot.
			return Snapshot;
		}

		/**
		 * Converts a "pointer to allocation info" map to a "callstack index to allocation info" map, summing up allocation
		 * sizes and counts.
		 */
		private Dictionary<int,FCallStackAllocationInfo> GenerateCallStackMap()
		{
			// Iterate over all allocations and track callstacks and their allocation size.
			Dictionary<int,FCallStackAllocationInfo> CallStackIndexToAllocationInfoMap = new Dictionary<int,FCallStackAllocationInfo>();
			foreach( KeyValuePair<UInt32,FCallStackAllocationInfo> PointerData in PointerToPointerInfoMap)
			{
				int CallStackIndex = PointerData.Value.CallStackIndex;
				long Size = PointerData.Value.Size;

				// If found, add size to callstack.
				if( CallStackIndexToAllocationInfoMap.ContainsKey(CallStackIndex) )
				{
					CallStackIndexToAllocationInfoMap[CallStackIndex].Size += Size;
					CallStackIndexToAllocationInfoMap[CallStackIndex].Count++;
				}
				// If not found, add new entry with size.
				else
				{
					CallStackIndexToAllocationInfoMap.Add( CallStackIndex, new FCallStackAllocationInfo( Size, CallStackIndex, 1 ) );
				}
			}
			return CallStackIndexToAllocationInfoMap;
		}

		/**
		 * Convert "callstack to allocation" mapping (either passed in or generated from pointer map) to callstack 
		 * info list.
		 */
		public void FinalizeSnapshot( Dictionary<int,FCallStackAllocationInfo> CallStackIndexToAllocationInfoMap )
		{
			// Generate mapping of callstack indices to alloction infos unless passed in.
			if( CallStackIndexToAllocationInfoMap == null )
			{
				CallStackIndexToAllocationInfoMap = GenerateCallStackMap();
			}
			
			// Iterate over temporary mapping and convert it to an array.
			ActiveCallStackList.Clear();
			foreach( KeyValuePair<int,FCallStackAllocationInfo> CallStackData in CallStackIndexToAllocationInfoMap )
			{
				ActiveCallStackList.Add( CallStackData.Value );
			}
		}

		/**
		 * Diffs two snapshots and creates a result one.
		 */
		public static FStreamSnapshot DiffSnapshots( FStreamSnapshot Old, FStreamSnapshot New )
		{
			// Snapshots need to be from same stream in order to be comparable.
			if( Old.StreamInfo != New.StreamInfo )
			{
				throw new InvalidDataException();
			}

			// Create result snapshot object.
			FStreamSnapshot ResultSnapshot = new FStreamSnapshot("Diff",New.StreamInfo);

			// Generate callstack index to allocation info mappings for both snapshots. They are used to create the proper
			// 'diff/ subtract' of the two snapshots.
			Dictionary<int,FCallStackAllocationInfo> OldCallStackIndexToAllocationInfoMap = Old.GenerateCallStackMap();
			Dictionary<int,FCallStackAllocationInfo> NewCallStackIndexToAllocationInfoMap = New.GenerateCallStackMap();
			
			// Call stack map that is going to be used to finalize result snapshot.
			Dictionary<int,FCallStackAllocationInfo> ResultCallStackIndexToAllocationInfoMap = new Dictionary<int,FCallStackAllocationInfo>();

			// Iterate over all old allocations and subtract them from new ones. If new ones don't exist the count/ size can be
			// negative. This takes care of all shared allocations and allocations that are present in only the old snapshot.
			foreach( KeyValuePair<int,FCallStackAllocationInfo> CallStackData in OldCallStackIndexToAllocationInfoMap )
			{
				// Local helpers for KeyValuePair.
				int CallStackIndex = CallStackData.Key;
				FCallStackAllocationInfo OldCallStackInfo = CallStackData.Value;

				// Zero initialized values for new size and count.
				long NewSize = 0;
				int NewCount = 0;

				// If callstack is present in new snapshot get the size and count from there.
				if( NewCallStackIndexToAllocationInfoMap.ContainsKey(CallStackIndex) )
				{
					FCallStackAllocationInfo NewCallStackInfo = NewCallStackIndexToAllocationInfoMap[CallStackIndex];
					NewSize = NewCallStackInfo.Size;
					NewCount = NewCallStackInfo.Count;					
				}

				// The result mapping will be New - Old.
				long ResultSize = NewSize - OldCallStackInfo.Size;
				int ResultCount = NewCount - OldCallStackInfo.Count;					
				// Only add results if there are allocations.
				if( ResultCount != 0 )
				{
					ResultCallStackIndexToAllocationInfoMap.Add( CallStackIndex, new FCallStackAllocationInfo(ResultSize,CallStackIndex,ResultCount) );
				}
			}

			// Iterate over all new allocations and handle new callstacks. All other work is already performed in loop above.
			foreach( KeyValuePair<int,FCallStackAllocationInfo> CallStackData in NewCallStackIndexToAllocationInfoMap )
			{				
				// Local helpers for KeyValuePair.
				int CallStackIndex = CallStackData.Key;
				FCallStackAllocationInfo NewCallStackInfo = CallStackData.Value;

				// Old snapshot doesn't contain callstack so this is entirely new.
				if( !OldCallStackIndexToAllocationInfoMap.ContainsKey(CallStackIndex) )
				{
					ResultCallStackIndexToAllocationInfoMap.Add( CallStackIndex, NewCallStackInfo );
				}
			}

            // Iterate over new lifetime callstack info and subtract previous one.
            for( int CallStackIndex=0; CallStackIndex<New.LifetimeCallStackList.Count; CallStackIndex++ )
            {
                ResultSnapshot.LifetimeCallStackList[CallStackIndex] = FCallStackAllocationInfo.Diff( 
                                                                                New.LifetimeCallStackList[CallStackIndex],
                                                                                Old.LifetimeCallStackList[CallStackIndex] );
            }

			// Finalize result snapshot from temporary mapping.
			ResultSnapshot.FinalizeSnapshot( ResultCallStackIndexToAllocationInfoMap );

			return ResultSnapshot;
		}

		/**
		 * Exports this snapshot to a CSV file of the passed in name.
		 * 
		 * @param	FileName	                        File name to export to
         * @param   bShouldExportActiveAllocations      Whether to export active allocations or lifetime allocations
		 */
		public void ExportToCSV( string FileName, bool bShouldExportActiveAllocations )
		{
			// Create stream writer used to output call graphs to CSV.
			StreamWriter CSVWriter = new StreamWriter(FileName);

            // Figure out which list to export.
            List<FCallStackAllocationInfo> CallStackList = null;
            if( bShouldExportActiveAllocations )
            {
                CallStackList = ActiveCallStackList;
            }
            else
            {
                CallStackList = LifetimeCallStackList;
            }

			// Iterate over each unique call graph and spit it out. The sorting is per call stack and not
			// allocation size. Excel can be used to sort by allocation if needed. This sorting is more robust
			// and also what the call graph parsing code needs.
			foreach( FCallStackAllocationInfo AllocationInfo in CallStackList )
			{
                // Skip callstacks with no contribution in this snapshot.
                if( AllocationInfo.Count > 0 )
                {
				    // Dump size first, followed by count.
				    CSVWriter.Write(AllocationInfo.Size + "," + AllocationInfo.Count + ",");

				    // Iterate over ach address in callstack and dump to CSV.
				    FCallStack CallStack = StreamInfo.CallStackArray[AllocationInfo.CallStackIndex];
				    foreach( int AddressIndex in CallStack.AddressIndices )
				    {
					    FCallStackAddress Address = StreamInfo.CallStackAddressArray[AddressIndex];
					    string SymbolFunctionName = StreamInfo.NameArray[Address.FunctionIndex];
					    string SymbolFileName = StreamInfo.NameArray[Address.FilenameIndex];
					    // Write out function followed by filename and then line number if valid
					    if( SymbolFunctionName != "" || SymbolFileName != "" )
					    {
						    CSVWriter.Write(SymbolFunctionName + " @ " + SymbolFileName + ":" + Address.LineNumber + "," );
					    }
					    else
					    {
						    CSVWriter.Write("Unknown,");
					    }
				    }
				    CSVWriter.WriteLine("");
                }
			}

			// Close the file handle now that we're done writing.
			CSVWriter.Close();
		}
	};
}