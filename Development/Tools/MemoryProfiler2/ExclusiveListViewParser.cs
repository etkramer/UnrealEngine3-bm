using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace MemoryProfiler2
{
	/**
	 * Class parsing snapshot into a tree from a given root node.
	 */
	public static class FExclusiveListViewParser
	{
		public static void ParseSnapshot( ListView ExclusiveListView, List<FCallStackAllocationInfo> InCallStackList, FStreamInfo StreamInfo, MainWindow MainMProfWindow, bool bShouldSortBySize )
		{
			MainMProfWindow.UpdateStatus("Updating exclusive list view for " + MainMProfWindow.CurrentFilename);

			ExclusiveListView.BeginUpdate();

			// Make a copy of the sorted call stack allocation list and resort by size.
            List<FCallStackAllocationInfo> CallStackList = InCallStackList;
			if( bShouldSortBySize )
			{
                CallStackList.Sort( CompareSize );
			}
			else
			{
				CallStackList.Sort( CompareCount );
			}

			// Figure out total size and count for percentages.
			long TotalSize = 0;
			long TotalCount = 0;
			foreach( FCallStackAllocationInfo AllocationInfo in CallStackList )
			{
				TotalSize += AllocationInfo.Size;
				TotalCount += AllocationInfo.Count;
			}

			// Clear out existing entries and add top 100.
			ExclusiveListView.Items.Clear();
			for( int i=0; i<Math.Min(100,CallStackList.Count); i++ )
			{
				FCallStackAllocationInfo AllocationInfo = CallStackList[i];
			
				string SizeInKByte		= String.Format( "{0:0}", (float) AllocationInfo.Size / 1024 ).PadLeft( 10, ' ' );
				string SizePercent		= String.Format( "{0:0.00}", (float) AllocationInfo.Size / TotalSize * 100 ).PadLeft( 10, ' ' );
				string Count			= String.Format( "{0:0}", AllocationInfo.Count ).PadLeft( 10, ' ' );
				string CountPercent		= String.Format( "{0:0.00}", (float) AllocationInfo.Count / TotalCount * 100 ).PadLeft( 10, ' ' );

				string[] Row = new string[]
				{
					SizeInKByte,
					SizePercent,
					Count,
					CountPercent
				};

				ListViewItem Item = new ListViewItem(Row);
				Item.Tag = AllocationInfo;
				ExclusiveListView.Items.Add( Item );
			}

			ExclusiveListView.EndUpdate();
		}

		/**
		 * Compare helper function, sorting FCallStackAllocation by size.
		 */
		private static int CompareSize( FCallStackAllocationInfo A, FCallStackAllocationInfo B )
		{
			return Math.Sign( B.Size - A.Size );
		}	

		/**
		 * Compare helper function, sorting FCallStackAllocation by count.
		 */
		private static int CompareCount( FCallStackAllocationInfo A, FCallStackAllocationInfo B )
		{
			return Math.Sign( B.Count - A.Count );
		}	

	};
}