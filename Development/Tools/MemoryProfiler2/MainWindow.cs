using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace MemoryProfiler2
{
    public partial class MainWindow : Form
    {
		/** List of snapthots created by parser and used by various visualizers and dumping tools. */
		private List<FStreamSnapshot> SnapshotList;
		/** Current active snapshot. Either a diff if start is != start of actual snapshot if start == start. */
		private FStreamSnapshot CurrentSnapshot;
		/** Current stream info shared across snapshots. */
		private FStreamInfo CurrentStreamInfo;
		/** Current file name associated with open file. */
		public string CurrentFilename;

        public MainWindow()
        {
            InitializeComponent();
            // Set initial sort/ allocation criteria to default values.
            SortCriteriaComboBox.SelectedIndex = 0;
            AllocationTypeComboBox.SelectedIndex = 0;
        }

        public MainWindow( string Filename)
        {
            InitializeComponent();
            // Set initial sort/ allocation criteria to default values.
            SortCriteriaComboBox.SelectedIndex = 0;
            AllocationTypeComboBox.SelectedIndex = 0;
            ParseFile(Filename);
        }

		/**
		 * Updates current snapshot based on selected values in diff start/ end combo boxes.
		 */
		private void UpdateCurrentSnapshot()
		{
			// Start == "Start"
			if( DiffStartComboBox.SelectedIndex == 0 )
			{
				// End == Start == "Start"
				if( DiffEndComboBox.SelectedIndex == 0 )
				{
					// "Start" is a pseudo snapshot so there is nothing to compare here.
					CurrentSnapshot = null;
				}
				else
				{
					// Comparing to "Start" means we can simply use the current index (-1 as Start doesn't have index associated).
					CurrentSnapshot = SnapshotList[DiffEndComboBox.SelectedIndex-1];
				}
			}
			// Start != "Start"
			else
			{
				// End == "Start"
				if( DiffEndComboBox.SelectedIndex == 0 )
				{
					// Comparing to "Start" means we can simply use the current index (-1 as Start doesn't have index associated).
					CurrentSnapshot = SnapshotList[DiffStartComboBox.SelectedIndex-1];
				}
				// Start != End and both are != "Start"
				else
				{
					// Do a real diff in this case and create a new snapshot.
					CurrentSnapshot = FStreamSnapshot.DiffSnapshots(SnapshotList[DiffStartComboBox.SelectedIndex-1],SnapshotList[DiffEndComboBox.SelectedIndex-1]);
				}
			}
		}

		private void GoButton_Click(object sender,EventArgs e)
		{
			// Update the current snapshot based on combo box settings.
			UpdateCurrentSnapshot();
			// If valid, parse it into the call graph view tree.
			if( CurrentSnapshot != null )
			{
                // Index 0 == sort by size
                // Index 1 == sort by count.
				bool bShouldSortBySize = SortCriteriaComboBox.SelectedIndex == 0;
                
                // Index 0 == list active allocations
                // Index 1 == list lifetime allocations
                List<FCallStackAllocationInfo> CallStackList = null;
                if( AllocationTypeComboBox.SelectedIndex == 0 )
                {
                    CallStackList = CurrentSnapshot.ActiveCallStackList;
                }
                else
                {
                    CallStackList = CurrentSnapshot.LifetimeCallStackList;
                }

                // Parse snaphots into views.
				FCallGraphTreeViewParser.ParseSnapshot( CallGraphTreeView, CallStackList, CurrentStreamInfo, this, bShouldSortBySize );
				FExclusiveListViewParser.ParseSnapshot( ExclusiveListView, CallStackList, CurrentStreamInfo, this, bShouldSortBySize );
			}
			else
			{
				// Clear the views to signal error
				ResetViews();
			}
			UpdateStatus("Displaying " + CurrentFilename);
		}

		/**
		 * Resets combobox items and various views into data
		 */ 
		private void ResetComboBoxAndViews()
		{
			// Reset combobox items.
			DiffStartComboBox.Items.Clear();
			DiffEndComboBox.Items.Clear();
            // Reset vies.
            ResetViews();
		}
		
		/**
		 *  Resets the various views into the data
		 */
		private void ResetViews()
		{
			// Reset callgraph tree.
			CallGraphTreeView.Nodes.Clear();
			// Reset exclusive list view.
			ExclusiveListView.Items.Clear();
			// Reset call graph view in exclusive tab.
			ExclusiveSingleCallStackView.Items.Clear();

		}

		private void openToolStripMenuItem_Click(object sender,EventArgs e)
		{
			// Create a file open dialog for selecting the .mprof file.
			OpenFileDialog OpenMProfFileDialog = new OpenFileDialog();
			OpenMProfFileDialog.Filter = "Profiling Data (*.mprof)|*.mprof";
			OpenMProfFileDialog.InitialDirectory = System.IO.Directory.GetCurrentDirectory();
			OpenMProfFileDialog.RestoreDirectory = true;
            OpenMProfFileDialog.ShowDialog();
	
			// Reset combobox items and various views into data
            ResetComboBoxAndViews();

            ParseFile(OpenMProfFileDialog.FileName);
		}

        private void ParseFile( string InCurrentFilename )
        {
            CurrentFilename = InCurrentFilename;

            // Only parse if we have a valid file.
            if (CurrentFilename != "")
            {
                try
                {
                    // Create a new stream info from the opened file.
                    CurrentStreamInfo = new FStreamInfo(CurrentFilename);
                    // Parse the file into snapshots. At the very least one for the end state.
                    SnapshotList = FStreamParser.Parse(CurrentStreamInfo, this);

                    // Add Start, End values and user generated snapshots in between.
                    DiffStartComboBox.Items.Add("Start");
                    DiffEndComboBox.Items.Add("Start");
                    // Count-1 as End is implicit last snapshot in list.
                    for (int i = 0; i < SnapshotList.Count - 1; i++)
                    {
                        string SnapshotDescription = "Snapshot " + i;
                        DiffStartComboBox.Items.Add(SnapshotDescription);
                        DiffEndComboBox.Items.Add(SnapshotDescription);
                    }
                    DiffStartComboBox.Items.Add("End");
                    DiffEndComboBox.Items.Add("End");

                    // Start defaults to "Start" and End defaults to "End" being selected.
                    DiffStartComboBox.SelectedIndex = 0;
                    DiffEndComboBox.SelectedIndex = DiffEndComboBox.Items.Count - 1;
                }
                catch (System.IO.InvalidDataException)
                {
                    // Reset combobox items and various views into data
                    ResetComboBoxAndViews();
                }
            }

            UpdateStatus("Parsed " + CurrentFilename);
        }

		private void exportToCSVToolStripMenuItem_Click(object sender,EventArgs e)
		{
			// Bring up dialog for user to pick filename to export data to.
			SaveFileDialog ExportToCSVFileDialog = new SaveFileDialog();
			ExportToCSVFileDialog.Filter = "CallGraph in CSV (*.csv)|*.csv";
			ExportToCSVFileDialog.InitialDirectory = System.IO.Directory.GetCurrentDirectory();
			ExportToCSVFileDialog.RestoreDirectory = true;
            ExportToCSVFileDialog.ShowDialog();

            // Determine whether to export lifetime or active allocations.
            bool bShouldExportActiveAllocations = AllocationTypeComboBox.SelectedIndex == 0;
			// Export call graph from current snapshot to CSV.
			CurrentSnapshot.ExportToCSV( ExportToCSVFileDialog.FileName, bShouldExportActiveAllocations );
		}

		/**
		 * Updates the status string label with the passed in string
		 * 
		 * @param	Status		New status to set
		 */
		public void UpdateStatus( string Status )
		{
			StatusStripLabel.Text = Status;
			Refresh();
		}

		/**
		 * Update 2nd list view with full graph of selected row.
		 */ 
		private void ExclusiveListView_SelectedIndexChanged(object sender,EventArgs e)
		{
			ExclusiveSingleCallStackView.BeginUpdate();
			ExclusiveSingleCallStackView.Items.Clear();

			// We can only display a single selected element.
			if( ExclusiveListView.SelectedItems.Count == 1 )
			{
				FCallStackAllocationInfo AllocationInfo = ExclusiveListView.SelectedItems[0].Tag as FCallStackAllocationInfo;
				CurrentStreamInfo.CallStackArray[AllocationInfo.CallStackIndex].AddToListView( CurrentStreamInfo, ExclusiveSingleCallStackView, true );
			}

			ExclusiveSingleCallStackView.EndUpdate();
		}

		private void CallGraphTreeView_NodeMouseClick(object sender,TreeNodeMouseClickEventArgs EventArgs)
		{
			if( EventArgs.Button == MouseButtons.Right )
			{
				// Expand/ collapse all nodes in subtree if double clicked on.
				if( EventArgs.Node.IsExpanded )
				{
					EventArgs.Node.Collapse(false);
				}
				// Expand subtree recursively... this might be slow.
				else
				{
					EventArgs.Node.ExpandAll();
				}
			}
		}
    }
}