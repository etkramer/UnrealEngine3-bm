
using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Text;
using System.Collections.Generic;


namespace UnrealConsole.Network
{
	/// <summary>
	/// This dialog shows the user a list of available game instances that
	/// can be connected to
	/// </summary>
	public class NetworkConnectionDialog : System.Windows.Forms.Form
	{
		/// <summary>
		/// Holds the selected server item
		/// </summary>
		public ConsoleInterface.PlatformTarget[] SelectedTargets;

		#region Windows Form Designer generated code

		private System.Windows.Forms.ListView ConnectionList;
		private System.Windows.Forms.ColumnHeader TargetNameHeader;
		private System.Windows.Forms.ColumnHeader PlatformTypeHeader;
		private System.Windows.Forms.Button ConnectButton;
		private System.Windows.Forms.Button CancelBtn;
		private System.Windows.Forms.Button RefreshButton;
		private ColumnHeader TitleIPAddressHeader;
		private ColumnHeader DebugIPAddressHeader;
		private ColumnHeader TypeHeader;
		private CheckBox mCheckBox_ShowAllTargetInfo;

		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		#endregion

		/// <summary>
		/// Default constructor
		/// </summary>
		public NetworkConnectionDialog()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			mCheckBox_ShowAllTargetInfo.Checked = Properties.Settings.Default.ShowAllTargetInformation;

			// Init connection list view
			ConnectionList.ListViewItemSorter = new ListViewItemComparer( ConnectionList, 1 );

			GetPossibleTargets();
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.ConnectionList = new System.Windows.Forms.ListView();
			this.TargetNameHeader = new System.Windows.Forms.ColumnHeader();
			this.PlatformTypeHeader = new System.Windows.Forms.ColumnHeader();
			this.DebugIPAddressHeader = new System.Windows.Forms.ColumnHeader();
			this.TitleIPAddressHeader = new System.Windows.Forms.ColumnHeader();
			this.TypeHeader = new System.Windows.Forms.ColumnHeader();
			this.ConnectButton = new System.Windows.Forms.Button();
			this.CancelBtn = new System.Windows.Forms.Button();
			this.RefreshButton = new System.Windows.Forms.Button();
			this.mCheckBox_ShowAllTargetInfo = new System.Windows.Forms.CheckBox();
			this.SuspendLayout();
			// 
			// ConnectionList
			// 
			this.ConnectionList.AllowColumnReorder = true;
			this.ConnectionList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.ConnectionList.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.TargetNameHeader,
            this.PlatformTypeHeader,
            this.DebugIPAddressHeader,
            this.TitleIPAddressHeader,
            this.TypeHeader});
			this.ConnectionList.FullRowSelect = true;
			this.ConnectionList.HideSelection = false;
			this.ConnectionList.Location = new System.Drawing.Point(12, 35);
			this.ConnectionList.Name = "ConnectionList";
			this.ConnectionList.Size = new System.Drawing.Size(576, 383);
			this.ConnectionList.TabIndex = 0;
			this.ConnectionList.UseCompatibleStateImageBehavior = false;
			this.ConnectionList.View = System.Windows.Forms.View.Details;
			this.ConnectionList.ItemActivate += new System.EventHandler(this.ConnectionList_ItemActivate);
			this.ConnectionList.SelectedIndexChanged += new System.EventHandler(this.ConnectionList_SelectedIndexChanged);
			this.ConnectionList.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.ConnectionList_ColumnClick);
			// 
			// TargetNameHeader
			// 
			this.TargetNameHeader.Text = "Target";
			this.TargetNameHeader.Width = 170;
			// 
			// PlatformTypeHeader
			// 
			this.PlatformTypeHeader.Text = "Platform";
			this.PlatformTypeHeader.Width = 80;
			// 
			// DebugIPAddressHeader
			// 
			this.DebugIPAddressHeader.Text = "Debug IP Address";
			this.DebugIPAddressHeader.Width = 118;
			// 
			// TitleIPAddressHeader
			// 
			this.TitleIPAddressHeader.Text = "TitleIP Address";
			this.TitleIPAddressHeader.Width = 114;
			// 
			// TypeHeader
			// 
			this.TypeHeader.Text = "Type";
			this.TypeHeader.Width = 72;
			// 
			// ConnectButton
			// 
			this.ConnectButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.ConnectButton.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.ConnectButton.Enabled = false;
			this.ConnectButton.Location = new System.Drawing.Point(432, 424);
			this.ConnectButton.Name = "ConnectButton";
			this.ConnectButton.Size = new System.Drawing.Size(75, 23);
			this.ConnectButton.TabIndex = 4;
			this.ConnectButton.Text = "&Connect";
			// 
			// CancelBtn
			// 
			this.CancelBtn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.CancelBtn.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.CancelBtn.Location = new System.Drawing.Point(512, 424);
			this.CancelBtn.Name = "CancelBtn";
			this.CancelBtn.Size = new System.Drawing.Size(75, 23);
			this.CancelBtn.TabIndex = 5;
			this.CancelBtn.Text = "C&ancel";
			// 
			// RefreshButton
			// 
			this.RefreshButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.RefreshButton.Location = new System.Drawing.Point(325, 424);
			this.RefreshButton.Name = "RefreshButton";
			this.RefreshButton.Size = new System.Drawing.Size(75, 23);
			this.RefreshButton.TabIndex = 6;
			this.RefreshButton.Text = "&Refresh";
			this.RefreshButton.Click += new System.EventHandler(this.RefreshButton_Click);
			// 
			// mCheckBox_ShowAllTargetInfo
			// 
			this.mCheckBox_ShowAllTargetInfo.AutoSize = true;
			this.mCheckBox_ShowAllTargetInfo.Location = new System.Drawing.Point(12, 12);
			this.mCheckBox_ShowAllTargetInfo.Name = "mCheckBox_ShowAllTargetInfo";
			this.mCheckBox_ShowAllTargetInfo.Size = new System.Drawing.Size(156, 17);
			this.mCheckBox_ShowAllTargetInfo.TabIndex = 7;
			this.mCheckBox_ShowAllTargetInfo.Text = "Show All Target Information";
			this.mCheckBox_ShowAllTargetInfo.UseVisualStyleBackColor = true;
			this.mCheckBox_ShowAllTargetInfo.CheckedChanged += new System.EventHandler(this.mCheckBox_ShowAllTargetInfo_CheckedChanged);
			// 
			// NetworkConnectionDialog
			// 
			this.AcceptButton = this.ConnectButton;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.CancelBtn;
			this.ClientSize = new System.Drawing.Size(600, 454);
			this.ControlBox = false;
			this.Controls.Add(this.mCheckBox_ShowAllTargetInfo);
			this.Controls.Add(this.RefreshButton);
			this.Controls.Add(this.CancelBtn);
			this.Controls.Add(this.ConnectButton);
			this.Controls.Add(this.ConnectionList);
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "NetworkConnectionDialog";
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Available Targets";
			this.Closing += new System.ComponentModel.CancelEventHandler(this.NetworkConnectionDialog_Closing);
			this.ResumeLayout(false);
			this.PerformLayout();

		}
		#endregion

		/// <summary>
		/// Enumerates all available targets.
		/// </summary>
		private void GetPossibleTargets()
		{
			ConnectionList.Items.Clear();

			ManualResetEvent Event = new ManualResetEvent(false);
			System.Threading.Thread TargetsThread = new Thread(new ParameterizedThreadStart(EnumerateTargetsThreadHandler));
			TargetsThread.Name = "TargetEnumeration UI Thread";
			TargetsThread.SetApartmentState(ApartmentState.STA);
			TargetsThread.IsBackground = true;

			TargetsThread.Start(Event);

			// go over all the platforms
			foreach(ConsoleInterface.Platform CurPlatform in ConsoleInterface.DLLInterface.Platforms)
			{
				CurPlatform.EnumerateAvailableTargets();

				// go over all the targets for this platform
				foreach(ConsoleInterface.PlatformTarget CurTarget in CurPlatform.Targets)
				{
					ListViewItem lvi = null;

					if(Properties.Settings.Default.ShowAllTargetInformation)
					{
						// Add the server info with IP addr
						lvi = new ListViewItem(CurTarget.Name);
						lvi.SubItems.Add(CurPlatform.Name);
						lvi.SubItems.Add(CurTarget.DebugIPAddress.ToString());
						lvi.SubItems.Add(CurTarget.IPAddress.ToString());
						lvi.SubItems.Add(CurTarget.ConsoleType.ToString());
						lvi.Tag = CurTarget;
					}
					else
					{
						lvi = new ListViewItem(CurTarget.TargetManagerName);
						lvi.SubItems.Add(CurPlatform.Name);
						lvi.SubItems.Add("n/a");
						lvi.SubItems.Add("n/a");
						lvi.SubItems.Add("n/a");
						lvi.Tag = CurTarget;
					}

					ConnectionList.Items.Add(lvi);
				}
			}

			Event.Set();
		}

		/// <summary>
		/// Shows a dialog box in a separate thread while all targets are being enumerated.
		/// </summary>
		/// <param name="State">The event that will signal the thread when the targets are finished being enumerated.</param>
		private void EnumerateTargetsThreadHandler(object State)
		{
			ManualResetEvent Event = State as ManualResetEvent;

			if(Event != null)
			{
				Application.Run(new EnumeratingTargetsForm(Event));
			}
		}

		/// <summary>
		/// Rebuilds the connection and sends the server announce request
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void RefreshButton_Click(object sender, System.EventArgs e)
		{
			ConnectionList.Items.Clear();

			GetPossibleTargets();
		}

		/// <summary>
		/// Shuts down any sockets and sets our out variables
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void NetworkConnectionDialog_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			// Store the selected item as the server to connect to
			if (ConnectionList.SelectedItems.Count > 0)
			{
				List<ConsoleInterface.PlatformTarget> Targets = new List<ConsoleInterface.PlatformTarget>();

				foreach(ListViewItem CurItem in ConnectionList.SelectedItems)
				{
					Targets.Add((ConsoleInterface.PlatformTarget)CurItem.Tag);
				}

				SelectedTargets = Targets.ToArray();
			}
		}

		/// <summary>
		/// Handles double clicking on a specific server. Same as clicking once
		/// and closing the dialog via Connect
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void ConnectionList_ItemActivate(object sender, System.EventArgs e)
		{
			DialogResult = DialogResult.OK;
			Close();
		}

		/// <summary>
		/// Event handler for when a column header has been clicked.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void ConnectionList_ColumnClick(object sender, System.Windows.Forms.ColumnClickEventArgs e)
		{
            if( ConnectionList.Sorting == System.Windows.Forms.SortOrder.Ascending )
            {
                ConnectionList.Sorting = SortOrder.Descending;
            }
            else
            {
                ConnectionList.Sorting = SortOrder.Ascending;
            }

            ConnectionList.ListViewItemSorter = new ListViewItemComparer( ConnectionList, e.Column );
		}

		/// <summary>
		/// Event handler for when a target has been selected.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void ConnectionList_SelectedIndexChanged(object sender, EventArgs e)
		{
			ConnectButton.Enabled = ConnectionList.SelectedIndices.Count > 0 && ConnectionList.SelectedIndices[0] >= 0;
		}

		/// <summary>
		/// Event handler for setting the state of target information.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void mCheckBox_ShowAllTargetInfo_CheckedChanged(object sender, EventArgs e)
		{
			Properties.Settings.Default.ShowAllTargetInformation = mCheckBox_ShowAllTargetInfo.Checked;
		}
	}

	/// <summary>
	/// Implements the manual sorting of items by columns.
	/// </summary>
	class ListViewItemComparer : IComparer
	{
        private ListView Parent;
		private int ColumnIndex;

		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="InParent">The owner of the comparer.</param>
        public ListViewItemComparer( ListView InParent )
		{
            Parent = InParent;
			ColumnIndex = 0;
		}

		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="InParent">THe owner of the comparer.</param>
		/// <param name="Column">The column being compared.</param>
		public ListViewItemComparer( ListView InParent, int Column )
		{
            Parent = InParent;
            ColumnIndex = Column;
		}

		/// <summary>
		/// Compares 2 objects for equality.
		/// </summary>
		/// <param name="x">The first object.</param>
		/// <param name="y">The second object.</param>
		/// <returns>0 if they're equal.</returns>
		public int Compare(object x, object y)
		{
            string A, B;
            //int NameIndex;

            switch( ColumnIndex )
            {
                case 0:
                    A = ( ( ListViewItem )x ).SubItems[ColumnIndex].Text;
                    B = ( ( ListViewItem )y ).SubItems[ColumnIndex].Text;

                    // DB: Disable sorting by IP by now.
                    // DB: It's more useful to sort by human-readable computer name!!
                    /*
                    NameIndex = A.IndexOf( '(' );
                    if( NameIndex > 0 )
                    {
                        A = A.Substring( NameIndex + 1 );
                    }

                    NameIndex = B.IndexOf( '(' );
                    if( NameIndex > 0 )
                    {
                        B = B.Substring( NameIndex + 1 );
                    }
                    */
                    break;

                case 1:
                    A = ( ( ListViewItem )x ).SubItems[ColumnIndex].Text;
                    B = ( ( ListViewItem )y ).SubItems[ColumnIndex].Text;
                    break;

                default:
                    A = "";
                    B = "";
                    break;
            }

            if( Parent.Sorting == System.Windows.Forms.SortOrder.Ascending )
            {
                return String.Compare( A, B );
            }
            else
            {
                return String.Compare( B, A );
            }
		}
	}
}
