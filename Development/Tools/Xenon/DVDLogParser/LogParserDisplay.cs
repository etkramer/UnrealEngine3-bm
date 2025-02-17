using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.IO;
using System.Threading;
using System.Windows.Forms;

namespace DVDLogParser
{
	/// <summary>
	/// Summary description for LogParserDisplay.
	/// </summary>
	public class LogParserDisplay : System.Windows.Forms.Form
	{
		private System.ComponentModel.IContainer components;

		private System.DateTime LastTime = System.DateTime.UtcNow;
		private int LastUpdate = -1;
		private int Milliseconds = 0;

		private System.Windows.Forms.MainMenu LogParserDisplayMenu;
		private System.Windows.Forms.MenuItem MenuFolder_File;
		private System.Windows.Forms.MenuItem MenuItem_File_Exit;
		private System.Windows.Forms.MenuItem MenuFolder_About;
		private System.Windows.Forms.MenuItem MenuItem_Help_About;
		private System.Windows.Forms.MenuItem MenuItem_File_SetLogFile;
		private System.Windows.Forms.MenuItem MenuItem_Separator1;
		private System.Windows.Forms.MenuItem MenuItem_File_Preferences;
		private System.Windows.Forms.MenuItem MenuItem_File_ParseLogFile;
		private System.Windows.Forms.MenuItem MenuItem_File_SaveCSV;

		private System.Windows.Forms.ColumnHeader ListView_HeaderBlocksRead;
		private System.Windows.Forms.ToolBar ToolBar;
		private System.Windows.Forms.ImageList Toolbar_ImageList;
		private System.Windows.Forms.ToolBarButton ToolbarButton_Open;
		private System.Windows.Forms.ToolBarButton ToolBarButton_Save;
		private System.Windows.Forms.ToolBarButton ToolBarButton_Help;
		private System.Windows.Forms.ToolBarButton ToolBarButton_Play;
		private System.Windows.Forms.ToolBarButton ToolBarButton_Pause;
		private System.Windows.Forms.ToolBarButton ToolBarButton_Stop;

		private System.Windows.Forms.ListView ListView_Main;
		private System.Windows.Forms.ColumnHeader ListView_HeaderFiles;
		private System.Windows.Forms.ColumnHeader ListView_HeaderTotalTime;
		private System.Windows.Forms.ColumnHeader ListView_HeaderTotalSeekTime;
		private System.Windows.Forms.ColumnHeader ListView_HeaderTotalReadTime;
		private System.Windows.Forms.ColumnHeader ListView_HeaderAverageTime;
		private System.Windows.Forms.ColumnHeader ListView_HeaderAverageSeekTime;
		private System.Windows.Forms.ColumnHeader ListView_HeaderAverageReadTime;

		private System.Windows.Forms.Label DisplayTime;
		private System.Windows.Forms.ToolBarButton ToolBarButton_Prefs;
		private System.Windows.Forms.OpenFileDialog OpenLogFileDialog;
		private System.Windows.Forms.SaveFileDialog SaveCSVFileDialog;

		private LogParser DataParser = null;
		private bool bTicking = true;
		private int UpdateRateDeciseconds = 10;
		private int HistorySeconds = 20;

		const int STATE_PLAYING = 0;
		const int STATE_PAUSED = 1;
		private int ParseState = STATE_PLAYING;

		public LogParserDisplay()
		{
			// Required for Windows Form Designer support
			InitializeComponent();

			this.Show();
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			bTicking = false;

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
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(LogParserDisplay));
			this.LogParserDisplayMenu = new System.Windows.Forms.MainMenu();
			this.MenuFolder_File = new System.Windows.Forms.MenuItem();
			this.MenuItem_File_SetLogFile = new System.Windows.Forms.MenuItem();
			this.MenuItem_File_ParseLogFile = new System.Windows.Forms.MenuItem();
			this.MenuItem_File_SaveCSV = new System.Windows.Forms.MenuItem();
			this.MenuItem_File_Preferences = new System.Windows.Forms.MenuItem();
			this.MenuItem_Separator1 = new System.Windows.Forms.MenuItem();
			this.MenuItem_File_Exit = new System.Windows.Forms.MenuItem();
			this.MenuFolder_About = new System.Windows.Forms.MenuItem();
			this.MenuItem_Help_About = new System.Windows.Forms.MenuItem();
			this.ListView_Main = new System.Windows.Forms.ListView();
			this.ListView_HeaderFiles = new System.Windows.Forms.ColumnHeader();
			this.ListView_HeaderBlocksRead = new System.Windows.Forms.ColumnHeader();
			this.ListView_HeaderTotalTime = new System.Windows.Forms.ColumnHeader();
			this.ListView_HeaderTotalSeekTime = new System.Windows.Forms.ColumnHeader();
			this.ListView_HeaderTotalReadTime = new System.Windows.Forms.ColumnHeader();
			this.ListView_HeaderAverageTime = new System.Windows.Forms.ColumnHeader();
			this.ListView_HeaderAverageSeekTime = new System.Windows.Forms.ColumnHeader();
			this.ListView_HeaderAverageReadTime = new System.Windows.Forms.ColumnHeader();
			this.ToolBar = new System.Windows.Forms.ToolBar();
			this.ToolbarButton_Open = new System.Windows.Forms.ToolBarButton();
			this.ToolBarButton_Save = new System.Windows.Forms.ToolBarButton();
			this.ToolBarButton_Prefs = new System.Windows.Forms.ToolBarButton();
			this.ToolBarButton_Help = new System.Windows.Forms.ToolBarButton();
			this.ToolBarButton_Play = new System.Windows.Forms.ToolBarButton();
			this.ToolBarButton_Pause = new System.Windows.Forms.ToolBarButton();
			this.ToolBarButton_Stop = new System.Windows.Forms.ToolBarButton();
			this.Toolbar_ImageList = new System.Windows.Forms.ImageList(this.components);
			this.DisplayTime = new System.Windows.Forms.Label();
			this.OpenLogFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.SaveCSVFileDialog = new System.Windows.Forms.SaveFileDialog();
			this.SuspendLayout();
			// 
			// LogParserDisplayMenu
			// 
			this.LogParserDisplayMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																								 this.MenuFolder_File,
																								 this.MenuFolder_About});
			// 
			// MenuFolder_File
			// 
			this.MenuFolder_File.Index = 0;
			this.MenuFolder_File.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																							this.MenuItem_File_SetLogFile,
																							this.MenuItem_File_ParseLogFile,
																							this.MenuItem_File_SaveCSV,
																							this.MenuItem_File_Preferences,
																							this.MenuItem_Separator1,
																							this.MenuItem_File_Exit});
			this.MenuFolder_File.Text = "File";
			// 
			// MenuItem_File_SetLogFile
			// 
			this.MenuItem_File_SetLogFile.Index = 0;
			this.MenuItem_File_SetLogFile.Shortcut = System.Windows.Forms.Shortcut.CtrlL;
			this.MenuItem_File_SetLogFile.Text = "Set Log File For Real Time Processing";
			this.MenuItem_File_SetLogFile.Click += new System.EventHandler(this.DVDLogParser_SetLogFile);
			// 
			// MenuItem_File_ParseLogFile
			// 
			this.MenuItem_File_ParseLogFile.Index = 1;
			this.MenuItem_File_ParseLogFile.Shortcut = System.Windows.Forms.Shortcut.CtrlE;
			this.MenuItem_File_ParseLogFile.Text = "Parse Entire Log File";
			this.MenuItem_File_ParseLogFile.Click += new System.EventHandler(this.File_ParseLogFile);
			// 
			// MenuItem_File_SaveCSV
			// 
			this.MenuItem_File_SaveCSV.Index = 2;
			this.MenuItem_File_SaveCSV.Shortcut = System.Windows.Forms.Shortcut.CtrlS;
			this.MenuItem_File_SaveCSV.Text = "Save .CSV File";
			this.MenuItem_File_SaveCSV.Click += new System.EventHandler(this.File_SaveCSVFile);
			// 
			// MenuItem_File_Preferences
			// 
			this.MenuItem_File_Preferences.Index = 3;
			this.MenuItem_File_Preferences.Shortcut = System.Windows.Forms.Shortcut.CtrlP;
			this.MenuItem_File_Preferences.Text = "Preferences";
			this.MenuItem_File_Preferences.Click += new System.EventHandler(this.DVDLogParser_Preferences);
			// 
			// MenuItem_Separator1
			// 
			this.MenuItem_Separator1.Index = 4;
			this.MenuItem_Separator1.Text = "-";
			// 
			// MenuItem_File_Exit
			// 
			this.MenuItem_File_Exit.Index = 5;
			this.MenuItem_File_Exit.Shortcut = System.Windows.Forms.Shortcut.CtrlX;
			this.MenuItem_File_Exit.Text = "Exit";
			this.MenuItem_File_Exit.Click += new System.EventHandler(this.DVDLogParser_Exit);
			// 
			// MenuFolder_About
			// 
			this.MenuFolder_About.Index = 1;
			this.MenuFolder_About.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																							 this.MenuItem_Help_About});
			this.MenuFolder_About.Text = "Help";
			// 
			// MenuItem_Help_About
			// 
			this.MenuItem_Help_About.Index = 0;
			this.MenuItem_Help_About.Shortcut = System.Windows.Forms.Shortcut.F1;
			this.MenuItem_Help_About.Text = "About";
			this.MenuItem_Help_About.Click += new System.EventHandler(this.DVDLogParser_About);
			// 
			// ListView_Main
			// 
			this.ListView_Main.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
																							this.ListView_HeaderFiles,
																							this.ListView_HeaderBlocksRead,
																							this.ListView_HeaderTotalTime,
																							this.ListView_HeaderTotalSeekTime,
																							this.ListView_HeaderTotalReadTime,
																							this.ListView_HeaderAverageTime,
																							this.ListView_HeaderAverageSeekTime,
																							this.ListView_HeaderAverageReadTime});
			this.ListView_Main.FullRowSelect = true;
			this.ListView_Main.Location = new System.Drawing.Point(0, 0);
			this.ListView_Main.Name = "ListView_Main";
			this.ListView_Main.Size = new System.Drawing.Size(968, 512);
			this.ListView_Main.Sorting = System.Windows.Forms.SortOrder.Ascending;
			this.ListView_Main.TabIndex = 0;
			this.ListView_Main.View = System.Windows.Forms.View.Details;
			this.ListView_Main.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.ListView_Main_ColumnClick);
			// 
			// ListView_HeaderFiles
			// 
			this.ListView_HeaderFiles.Text = "File(s)";
			this.ListView_HeaderFiles.Width = 250;
			// 
			// ListView_HeaderBlocksRead
			// 
			this.ListView_HeaderBlocksRead.Text = "Blocks Read";
			// 
			// ListView_HeaderTotalTime
			// 
			this.ListView_HeaderTotalTime.Text = "Total (ms)";
			this.ListView_HeaderTotalTime.Width = 120;
			// 
			// ListView_HeaderTotalSeekTime
			// 
			this.ListView_HeaderTotalSeekTime.Text = "Total Seek (ms)";
			this.ListView_HeaderTotalSeekTime.Width = 120;
			// 
			// ListView_HeaderTotalReadTime
			// 
			this.ListView_HeaderTotalReadTime.Text = "Total Read (ms)";
			this.ListView_HeaderTotalReadTime.Width = 120;
			// 
			// ListView_HeaderAverageTime
			// 
			this.ListView_HeaderAverageTime.Text = "Average (ms)";
			this.ListView_HeaderAverageTime.Width = 120;
			// 
			// ListView_HeaderAverageSeekTime
			// 
			this.ListView_HeaderAverageSeekTime.Text = "Average Seek (ms)";
			this.ListView_HeaderAverageSeekTime.Width = 120;
			// 
			// ListView_HeaderAverageReadTime
			// 
			this.ListView_HeaderAverageReadTime.Text = "Average Read (ms)";
			this.ListView_HeaderAverageReadTime.Width = 120;
			// 
			// ToolBar
			// 
			this.ToolBar.Buttons.AddRange(new System.Windows.Forms.ToolBarButton[] {
																					   this.ToolbarButton_Open,
																					   this.ToolBarButton_Save,
																					   this.ToolBarButton_Prefs,
																					   this.ToolBarButton_Help,
																					   this.ToolBarButton_Play,
																					   this.ToolBarButton_Pause,
																					   this.ToolBarButton_Stop});
			this.ToolBar.ButtonSize = new System.Drawing.Size(16, 16);
			this.ToolBar.DropDownArrows = true;
			this.ToolBar.ImageList = this.Toolbar_ImageList;
			this.ToolBar.Location = new System.Drawing.Point(0, 0);
			this.ToolBar.Name = "ToolBar";
			this.ToolBar.ShowToolTips = true;
			this.ToolBar.Size = new System.Drawing.Size(992, 28);
			this.ToolBar.TabIndex = 1;
			this.ToolBar.ButtonClick += new System.Windows.Forms.ToolBarButtonClickEventHandler(this.Toolbar_ButtonClick);
			// 
			// ToolbarButton_Open
			// 
			this.ToolbarButton_Open.ImageIndex = 0;
			this.ToolbarButton_Open.ToolTipText = "Open log file";
			// 
			// ToolBarButton_Save
			// 
			this.ToolBarButton_Save.ImageIndex = 1;
			this.ToolBarButton_Save.ToolTipText = "Save .CSV file";
			// 
			// ToolBarButton_Prefs
			// 
			this.ToolBarButton_Prefs.ImageIndex = 6;
			// 
			// ToolBarButton_Help
			// 
			this.ToolBarButton_Help.ImageIndex = 2;
			this.ToolBarButton_Help.ToolTipText = "Help";
			// 
			// ToolBarButton_Play
			// 
			this.ToolBarButton_Play.ImageIndex = 3;
			// 
			// ToolBarButton_Pause
			// 
			this.ToolBarButton_Pause.ImageIndex = 4;
			this.ToolBarButton_Pause.Style = System.Windows.Forms.ToolBarButtonStyle.ToggleButton;
			// 
			// ToolBarButton_Stop
			// 
			this.ToolBarButton_Stop.ImageIndex = 5;
			// 
			// Toolbar_ImageList
			// 
			this.Toolbar_ImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth24Bit;
			this.Toolbar_ImageList.ImageSize = new System.Drawing.Size(16, 16);
			this.Toolbar_ImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("Toolbar_ImageList.ImageStream")));
			this.Toolbar_ImageList.TransparentColor = System.Drawing.Color.Transparent;
			// 
			// DisplayTime
			// 
			this.DisplayTime.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.DisplayTime.Location = new System.Drawing.Point(184, 8);
			this.DisplayTime.Name = "DisplayTime";
			this.DisplayTime.Size = new System.Drawing.Size(128, 16);
			this.DisplayTime.TabIndex = 2;
			this.DisplayTime.Text = "Time: 0.000sec";
			// 
			// OpenLogFileDialog
			// 
			this.OpenLogFileDialog.Filter = "Log files (*.log)|*.log|All files (*.*)|*.*";
			this.OpenLogFileDialog.InitialDirectory = "..\\..\\Wargame\\Build";
			this.OpenLogFileDialog.RestoreDirectory = true;
			this.OpenLogFileDialog.Title = "Open log file from DVD emulation";
			// 
			// SaveCSVFileDialog
			// 
			this.SaveCSVFileDialog.Filter = "csv files (*.csv)|*.csv|All files (*.*)|*.*";
			this.SaveCSVFileDialog.Title = "Save .CSV spreadsheet";
			// 
			// LogParserDisplay
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(7, 13);
			this.ClientSize = new System.Drawing.Size(992, 574);
			this.Controls.Add(this.DisplayTime);
			this.Controls.Add(this.ToolBar);
			this.Controls.Add(this.ListView_Main);
			this.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Menu = this.LogParserDisplayMenu;
			this.Name = "LogParserDisplay";
			this.Text = "DVDLogParser";
			this.SizeChanged += new System.EventHandler(this.DVDLogParser_SizeChanged);
			this.ResumeLayout(false);

		}
		#endregion

		public void SetHistorySeconds( int Seconds )
		{
			HistorySeconds = Seconds;
		}

		public void SetUpdateRateSeconds( float Seconds )
		{
			UpdateRateDeciseconds = ( int )( Seconds * 10.0f );
		}

		public void Log( string Text )
		{
			System.Diagnostics.Debug.Write( Text + "\r\n" );
		}

		public string GetLogFileName()
		{
			if( this.OpenLogFileDialog.ShowDialog( this ) == DialogResult.OK && this.OpenLogFileDialog.FileName.Length > 0 )
			{
				return( this.OpenLogFileDialog.FileName );
			}				

			return( "" );
		}

		public void DVDLogParser_AddToListView( ListViewItem[] Items )
		{
			ListView_Main.Items.Clear();
			ListView_Main.Items.AddRange( Items );
		}

		class ListView_Comparer : IComparer
		{
			private int Column;
			private ListView Parent;

			public ListView_Comparer( ListView parent )
			{
				Parent = parent;
				Column = 0;
			}
			public ListView_Comparer( ListView parent, int Col )
			{
				Parent = parent;
				Column = Col;
			}
			public int Compare( object x, object y )
			{
				ListViewItem ObjectX = ( ListViewItem )x;
				ListViewItem ObjectY = ( ListViewItem )y;
				string NameX = ObjectX.SubItems[Column].Text;
				string NameY = ObjectY.SubItems[Column].Text;

				if( Column == 0 )
				{
					if( Parent.Sorting == System.Windows.Forms.SortOrder.Ascending )
					{
						return( String.Compare( NameX, NameY ) );
					}
					else
					{
						return( String.Compare( NameY, NameX ) );
					}
				}
				else
				{
					int X = int.Parse( NameX );
					int Y = int.Parse( NameY );
					if( Parent.Sorting == System.Windows.Forms.SortOrder.Ascending )
					{
						return( X - Y );
					}
					else
					{
						return( Y - X );
					}
				}
			}
		}

		private void ListView_Main_ColumnClick( object sender, System.Windows.Forms.ColumnClickEventArgs e )
		{
			if( ListView_Main.Sorting == System.Windows.Forms.SortOrder.Ascending )
			{
				ListView_Main.Sorting = SortOrder.Descending;
			}
			else
			{
				ListView_Main.Sorting = SortOrder.Ascending;
			}
			ListView_Main.ListViewItemSorter = new ListView_Comparer( ListView_Main, e.Column );
		}

		private void DVDLogParser_SizeChanged( object sender, System.EventArgs e )
		{
			System.Drawing.Size ListViewSize = new Size();
			ListViewSize.Height = ListView_Main.Parent.Size.Height - 55 - ToolBar.Size.Height;
			ListViewSize.Width = ListView_Main.Parent.Size.Width - 10;
			ListView_Main.Size = ListViewSize;		

			ListView_Main.Top = ToolBar.Size.Height;
		}

		private void DVDLogParser_Exit( object sender, System.EventArgs e )
		{
			this.Close();
			this.Dispose();			
		}

		private void DVDLogParser_About( object sender, System.EventArgs e )
		{
			new About();	
		}	

		private void DVDLogParser_SetLogFile( object sender, System.EventArgs e )
		{
			string LogFileName = GetLogFileName();
			if( LogFileName.Length > 0 )
			{
				DataParser = new LogParser( this );
				if( DataParser.Construct( LogFileName ) )
				{
					Text = "DVDLogParser " + LogFileName;
					DVDLogParser_Play();
				}
				else
				{
					DataParser.Destroy();
					DataParser = null;
				}
			}			
		}

		private void File_ParseLogFile( object sender, System.EventArgs e )
		{
			string LogFileName = GetLogFileName();
			if( LogFileName.Length > 0 )
			{
				DataParser = new LogParser( this );
				if( DataParser.Construct( LogFileName ) )
				{
					Text = "DVDLogParser " + LogFileName;
					DataParser.ParseEntireFile();
					DataParser.CreateListView();
				}

				DataParser.Destroy();
				DataParser = null;
			}	
		}

		private void File_SaveCSVFile( object sender, System.EventArgs e )
		{
			// Get a filename to save to
			if( this.SaveCSVFileDialog.ShowDialog( this ) != DialogResult.OK || this.SaveCSVFileDialog.FileName.Length == 0 )
			{
				return;
			}	

			try
			{
				StreamWriter CSVFile = new StreamWriter( SaveCSVFileDialog.FileName );

				// Write headers
				foreach( ColumnHeader Header in ListView_Main.Columns )
				{
					CSVFile.Write( Header.Text + "," );
				}
				CSVFile.Write( "\r\n" );

				// Write elements
				foreach( ListViewItem Line in ListView_Main.Items )
				{
					for( int i = 0; i < Line.SubItems.Count; i++ )
					{
						CSVFile.Write( Line.SubItems[i].Text + "," );
					}

					CSVFile.Write( "\r\n" );
				}

				CSVFile.Close();
			}
			catch( System.Exception )
			{
				MessageBox.Show( this, "Failed to write file \"" + SaveCSVFileDialog.FileName + "\"", "File Write Error", 
					MessageBoxButtons.OK, MessageBoxIcon.Error );
			}
		}

		private void DVDLogParser_Play()
		{
			LastTime = System.DateTime.UtcNow;
			ParseState = STATE_PLAYING;
		}

		private void DVDLogParser_Pause()
		{
			ParseState = STATE_PAUSED;
		}

		private void DVDLogParser_Stop()
		{
			LastUpdate = -1;
			Milliseconds = 0;
			DVDLogParser_Pause();
		}

		private void Toolbar_ButtonClick( object sender, System.Windows.Forms.ToolBarButtonClickEventArgs e )
		{
			switch( e.Button.ImageIndex )
			{
				case 0:
					File_ParseLogFile( sender, e );
					break;

				case 1:
					File_SaveCSVFile( sender, e );
					break;

				case 2:
					DVDLogParser_About( sender, e );
					break;			

				case 3:
					DVDLogParser_Play();					
					ToolBarButton_Pause.Pushed = false;
					break;

				case 4:
					if( e.Button.Pushed == true )
					{
						DVDLogParser_Pause();
					}
					else
					{
						DVDLogParser_Play();					
					}
					break;

				case 5:
					if( DataParser != null )
					{
						DataParser.Reset();
					}
					DVDLogParser_Stop();
					break;

				case 6:
					DVDLogParser_Preferences( null, null );
					break;
			}
		}

		private void DVDLogParser_Preferences( object sender, System.EventArgs e )
		{
			new Preferences( this, HistorySeconds, UpdateRateDeciseconds );		
		}

		private void Tick()
		{
			if( DataParser != null )
			{
				if( ParseState == STATE_PLAYING )
				{
					System.TimeSpan Delta = System.DateTime.UtcNow - LastTime;
					LastTime = System.DateTime.UtcNow;
					Milliseconds += ( int )( Delta.Ticks / 10000 );
				}

				// Display the time
				float Seconds = Milliseconds / 1000.0f;
				DisplayTime.Text = "Time: " + Seconds.ToString() + "sec";

				// Grab the next line
				DataParser.ParseUpto( Milliseconds );

				// If it's been over a second since the last update, average out the events
				if( ParseState == STATE_PAUSED || Milliseconds > LastUpdate + ( UpdateRateDeciseconds * 100 ) )
				{
					if( Milliseconds != LastUpdate )
					{
						DataParser.Update( HistorySeconds );
						LastUpdate = Milliseconds;
					}
				}
			}
		}

		private void Destroy()
		{
			if( DataParser != null )
			{
				DataParser.Destroy();
			}
		}

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
        [STAThread]
		static void Main( string[] Args ) 
		{
			LogParserDisplay MainWindow = new LogParserDisplay();

			while( MainWindow.bTicking )
			{
				Application.DoEvents();
				MainWindow.Tick();

				System.Threading.Thread.Sleep( 1 );
			}

			MainWindow.Destroy();
		}
	}
}
