using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Text;
using System.Xml;
using System.Xml.Serialization;
using System.Windows.Forms;

namespace UnrealDVDLayout
{
    public partial class UnrealDVDLayout : Form
    {
        public enum VerbosityLevel
        {
            Silent,
            Critical,
            Simple,
            Informative,
            Complex,
            Verbose,
            ExtraVerbose
        };

        public enum ObjectType
        {
            Reserved,
            Volume,
            Directory,
            File,
        };

        public const long SectorsPerLayer = 1783936;
        public const long BytesPerSector = 2048;

        public bool Ticking = false;
        public bool Interactive = true;
        public bool TOCLoaded = false;
        public bool UpdateFiles = false;
        public bool UpdateGroups = false;
        public bool UpdateLayers = false;
        public DateTime LastUpdate = DateTime.Now;
        public TOC TableOfContents = null;
        public XboxGameDiscLayout DiscLayout = null;
        public SettableOptions Options = null;
        public Configuration Config = new Configuration();

        delegate void DelegateAddLine( VerbosityLevel Verbosity, string Line, Color TextColor );

        public UnrealDVDLayout()
        {
            InitializeComponent();
        }

        public bool Init()
        {
            Ticking = true;

            Options = ReadXml<SettableOptions>( "UnrealDVDLayout.Options.xml" );

            InitMRU();
            GameToolStripComboBox.SelectedItem = Options.GameName;

            return ( true );
        }

        public bool Destroy()
        {
            if( TableOfContents != null )
            {
                string ProjectFileName = Options.GameName + "Game\\Build\\Layouts\\UnrealDVDLayout.Project.xml";
                WriteXml<TOCGroups>( ProjectFileName, TableOfContents.Groups );
            }

            WriteXml<SettableOptions>( "UnrealDVDLayout.Options.xml", Options );

            return ( true );
        }

        private void InitMRU()
        {
            if( Options.MRU.Count < 5 )
            {
                Options.MRU.Clear();
                Options.MRU.Add( "" );
                Options.MRU.Add( "" );
                Options.MRU.Add( "" );
                Options.MRU.Add( "" );
                Options.MRU.Add( "" );
            }

            Recent0MenuItem.Text = Options.MRU[0];
            Recent1MenuItem.Text = Options.MRU[1];
            Recent2MenuItem.Text = Options.MRU[2];
            Recent3MenuItem.Text = Options.MRU[3];  
            Recent4MenuItem.Text = Options.MRU[4];

            Recent0MenuItem.Visible = ( Recent0MenuItem.Text.Length != 0 );
            Recent1MenuItem.Visible = ( Recent1MenuItem.Text.Length != 0 );
            Recent2MenuItem.Visible = ( Recent2MenuItem.Text.Length != 0 );
            Recent3MenuItem.Visible = ( Recent3MenuItem.Text.Length != 0 );
            Recent4MenuItem.Visible = ( Recent4MenuItem.Text.Length != 0 );
        }

        private void AddToMRU( string[] FileNames )
        {
            foreach( string FileName in FileNames )
            {
                if( Options.MRU.Contains( FileName ) )
                {
                    Options.MRU.Remove( FileName );
                    Options.MRU.Add( "" );
                }

                Options.MRU.Insert( 0, FileName );
                Options.MRU.RemoveAt( 5 );
            }
            InitMRU();
        }

        private void GameToolStripComboBox_Changed( object sender, EventArgs e )
        {
            Options.GameName = ( string )GameToolStripComboBox.SelectedItem;
        }

        private void QuitMenuItem_Click( object sender, EventArgs e )
        {
            Ticking = false;
        }

        private void OptionsMenuItem_Click( object sender, EventArgs e )
        {
            OptionsDialog DisplayOptions = new OptionsDialog( this, Options );
            DisplayOptions.ShowDialog();
        }

        private void UnrealDVDLayout_FormClosed( object sender, FormClosedEventArgs e )
        {
            Ticking = false;
        }

        private void GroupListView_KeyPress( object sender, KeyEventArgs e )
        {
            if( e.KeyCode == Keys.Delete )
            {
                foreach( ListViewItem GroupName in GroupListView.SelectedItems )
                {
                    TableOfContents.Groups.TOCGroupLayer0.Remove( GroupName.Text );
                    TableOfContents.Groups.TOCGroupLayer1.Remove( GroupName.Text );

                    TableOfContents.RemoveGroup( GroupName.Text );
                }

                PopulateListBoxes( true, true, true );
            }
        }

        private void Layer0ListView_KeyPress( object sender, KeyEventArgs e )
        {
            if( e.KeyCode == Keys.Delete )
            {
                foreach( ListViewItem GroupName in Layer0ListView.SelectedItems )
                {
                    TOCGroup Group = TableOfContents.GetGroup( GroupName.Text );
                    TableOfContents.AddGroupToLayer( Group, -1 ); 
                    
                    TableOfContents.Groups.TOCGroupLayer0.Remove( GroupName.Text );
                }

                PopulateListBoxes( false, true, true );
            }
        }

        private void Layer1ListView_KeyPress( object sender, KeyEventArgs e )
        {
            if( e.KeyCode == Keys.Delete )
            {
                foreach( ListViewItem GroupName in Layer1ListView.SelectedItems )
                {
                    TOCGroup Group = TableOfContents.GetGroup( GroupName.Text );
                    TableOfContents.AddGroupToLayer( Group, -1 );

                    TableOfContents.Groups.TOCGroupLayer1.Remove( GroupName.Text );
                }

                PopulateListBoxes( false, true, true );
            }
        }

        private void GroupListView_DoubleClick( object sender, EventArgs e )
        {
            GroupProperties GroupProp = new GroupProperties();

            TOCGroup Group = TableOfContents.GetGroup( GroupListView.FocusedItem.Text );
            GroupProp.Init( TableOfContents, Group );

            if( GroupProp.ShowDialog() == DialogResult.OK )
            {
                GroupProp.ApplyChanges();
            }

            PopulateListBoxes( true, true, true );
        }

        private void AddToGroupItem_Click( object sender, EventArgs e )
        {
            ToolStripItem Item = ( ToolStripItem )sender;
            TOCGroup Group = TableOfContents.GetGroup( Item.Text );

            foreach( ListViewItem Selected in FileListView.SelectedItems )
            {
                TableOfContents.AddFileToGroup( Group, Selected.Text );
            }

            PopulateListBoxes( true, true, true );
        }

        private void FileListBoxMenuStrip_Opening( object sender, CancelEventArgs e )
        {
            List<string> GroupNames = TableOfContents.GetGroupNames();

            AddToGroupToolStripMenuItem.DropDownItems.Clear();
            foreach( string GroupName in GroupNames )
            {
                ToolStripItem Item = AddToGroupToolStripMenuItem.DropDownItems.Add( GroupName );
                Item.Click += new System.EventHandler( AddToGroupItem_Click );
            }
        }

        private void GroupMoveUpButton_Click( object sender, EventArgs e )
        {
            foreach( ListViewItem Selected in GroupListView.SelectedItems )
            {
                TOCGroup Group = TableOfContents.GetGroup( Selected.Text );
                TableOfContents.MoveGroupInGroups( Group, 1 );
            }

            PopulateListBoxes( false, true, true );
        }

        private void GroupMoveDownButton_Click( object sender, EventArgs e )
        {
            foreach( ListViewItem Selected in GroupListView.SelectedItems )
            {
                TOCGroup Group = TableOfContents.GetGroup( Selected.Text );
                TableOfContents.MoveGroupInGroups( Group, -1 );
            }

            PopulateListBoxes( false, true, true );
        }

        private void AddToLayer0MenuItem_Click( object sender, EventArgs e )
        {
            foreach( ListViewItem Selected in GroupListView.SelectedItems )
            {
                TOCGroup Group = TableOfContents.GetGroup( Selected.Text );
                TableOfContents.AddGroupToLayer( Group, 0 );
            }

            PopulateListBoxes( false, true, true );
        }

        private void AddToLayer1MenuItem_Click( object sender, EventArgs e )
        {
            foreach( ListViewItem Selected in GroupListView.SelectedItems )
            {
                TOCGroup Group = TableOfContents.GetGroup( Selected.Text );
                TableOfContents.AddGroupToLayer( Group, 1 );
            }

            PopulateListBoxes( false, true, true );
        }

        private void RemoveFromDiscMenuItem_Click( object sender, EventArgs e )
        {
            foreach( ListViewItem Selected in GroupListView.SelectedItems )
            {
                TOCGroup Group = TableOfContents.GetGroup( Selected.Text );
                TableOfContents.AddGroupToLayer( Group, -1 );
            }

            PopulateListBoxes( false, true, true );
        }

        private void Layer0MoveUpButton_Click( object sender, EventArgs e )
        {
            foreach( ListViewItem Selected in Layer0ListView.SelectedItems )
            {
                TOCGroup Group = TableOfContents.GetGroup( Selected.Text );
                TableOfContents.MoveGroupInLayer( Group, 0, 1 );
            }

            PopulateListBoxes( false, false, true );
        }

        private void Layer0MoveDownButton_Click( object sender, EventArgs e )
        {
            foreach( ListViewItem Selected in Layer0ListView.SelectedItems )
            {
                TOCGroup Group = TableOfContents.GetGroup( Selected.Text );
                TableOfContents.MoveGroupInLayer( Group, 0, -1 );
            }

            PopulateListBoxes( false, false, true );
        }

        private void Layer1MoveUpButton_Click( object sender, EventArgs e )
        {
            foreach( ListViewItem Selected in Layer1ListView.SelectedItems )
            {
                TOCGroup Group = TableOfContents.GetGroup( Selected.Text );
                TableOfContents.MoveGroupInLayer( Group, 1, 1 );
            }

            PopulateListBoxes( false, false, true );
        }

        private void Layer1MoveDownButton_Click( object sender, EventArgs e )
        {
            foreach( ListViewItem Selected in Layer1ListView.SelectedItems )
            {
                TOCGroup Group = TableOfContents.GetGroup( Selected.Text );
                TableOfContents.MoveGroupInLayer( Group, 1, -1 );
            }

            PopulateListBoxes( false, false, true );
        }

        private void AddGroupToXGD( string GroupName )
        {
            // Get the group in order and sort the component files
            TOCGroup Group = TableOfContents.GetGroup( GroupName );
            List<TOCInfo> Entries = TableOfContents.GetEntriesInGroup( Group );
            TableOfContents.ApplySort( Entries );

            // Add to the DVD
            foreach( TOCInfo Entry in Entries )
            {
                DiscLayout.AddObject( Entry, Group.Layer );
            }

            DiscLayout.AddPadObject( Group.GroupName, Group.Layer );
        }

        private void CreateXGDFile()
        {
            // Create XGD file
            DiscLayout = new XboxGameDiscLayout( TableOfContents.SourceFolder );

            // Work out the size required for each directory
            TableOfContents.CalcDirectorySizes();

            // Add all the objects to layer 0
            foreach( string GroupName in TableOfContents.Groups.TOCGroupLayer0 )
            {
                AddGroupToXGD( GroupName );
            }

            // Add all the objects to layer 1
            foreach( string GroupName in TableOfContents.Groups.TOCGroupLayer1 )
            {
                AddGroupToXGD( GroupName );
            }

            // Add all the objects to the scratch layer
            foreach( TOCGroup Group in TableOfContents.Groups.TOCGroupEntries )
            {
                if( Group.Layer == -1 )
                {
                    AddGroupToXGD( Group.GroupName );
                }
            }

            // Shunt all objects to the fast end of the disc
            DiscLayout.Finalise();

            Log( UnrealDVDLayout.VerbosityLevel.Informative, DiscLayout.GetSummary(), Color.Blue );
        }

        private void ImportTOCXGDFile( string[] FileNames )
        {
            if( FileNames.Length == 0 )
            {
                Log( VerbosityLevel.Informative, "No files selected!", Color.Red );
                return;
            }

            foreach( string FileName in FileNames )
            {
                if( !FileName.Contains( "Game" ) )
                {
                    Error( "Need a game name in the path (Game) for '" + FileName + "'" );
                    return;
                }
            }

            if( FileNames[0].ToLower().EndsWith( ".xgd" ) )
            {
                if( FileNames.Length > 1 )
                {
                    Warning( "Cannot merge XGDs; only '" + FileNames[0] + "' loaded" );
                }

                DiscLayout = ReadXml<XboxGameDiscLayout>( FileNames[0] );
                Text = "Unreal DVD Layout Tool : XGD File Loaded";
            }
            else
            {
                Log( VerbosityLevel.Informative, "Attempting to read " + FileNames.Length.ToString() + " TOC files.", Color.Blue );

                // Strip off the TOCName
                int LastSlash = FileNames[0].LastIndexOf( '\\' );
                string TOCName = FileNames[0].Substring( 0, LastSlash );

                // Strip of the game name
                LastSlash = TOCName.LastIndexOf( '\\' );
                string GameName = TOCName.Substring( LastSlash + 1 );
                string TOCFolder = "";
                if( LastSlash >= 0 )
                {
                    TOCFolder = TOCName.Substring( 0, LastSlash );
                }

                Options.GameName = GameName.Replace( "Game", "" );
                GameToolStripComboBox.SelectedItem = Options.GameName;

                TableOfContents = new TOC( this, TOCFolder );
                foreach( string FileName in FileNames )
                {
                    string RelativeName = FileName.Substring( Environment.CurrentDirectory.Length );
                    TableOfContents.Read( FileName, RelativeName );
                }

                // Get the relevant system files
                TableOfContents.GetSystemUpdateFile();
                TableOfContents.GetXex( GameName );
                TableOfContents.GetXdb( GameName );

                // Create the folder objects
                TableOfContents.CreateFolders();

                // Check for duplicate files
                TableOfContents.CheckDuplicates();

                Text = "Unreal DVD Layout Tool : " + TableOfContents.GetSummary();

                // Load in the project file - we have the game name
                string ProjectFileName = GameName + "\\Build\\Layouts\\UnrealDVDLayout.Project.xml";
                TableOfContents.Groups = ReadXml<TOCGroups>( ProjectFileName );

                // Apply the correct sorts to all the groups
                TableOfContents.UpdateTOCFromGroups();
                TableOfContents.FinishSetup();

                // Populate the UI if we have one
                if( Interactive )
                {
                    PopulateListBoxes( true, true, true );
                }

                // Create a sample layout 
                CreateXGDFile();
            }

            TOCLoaded = true;
        }

        private void ImportTOCMenu_Click( object sender, EventArgs e )
        {
            GenericOpenFileDialog.Title = "Select TOC or XGD file to import...";
            GenericOpenFileDialog.Filter = "ToC Files (*.txt)|*.txt|XGD Files (*.xgd)|*.xgd";
            GenericOpenFileDialog.InitialDirectory = Environment.CurrentDirectory;
            if( GenericOpenFileDialog.ShowDialog() == DialogResult.OK )
            {
                ImportTOCXGDFile( GenericOpenFileDialog.FileNames );
                AddToMRU( GenericOpenFileDialog.FileNames );
            }
        }

        private void SaveXGDMenu_Click( object sender, EventArgs e )
        {
            if( !TOCLoaded )
            {
                Error( "No TOC loaded to be able to save ...." );
            }
            else
            {
                // Create a layout based on the latest data
                CreateXGDFile();

                // Save it out
                GenericSaveFileDialog.Title = "Select XGD file to export...";
                GenericSaveFileDialog.InitialDirectory = Environment.CurrentDirectory;
                if( GenericSaveFileDialog.ShowDialog() == DialogResult.OK )
                {
                    Log( VerbosityLevel.Informative, "Saving ... '" + GenericSaveFileDialog.FileName + "'", Color.Blue );
                    if( WriteXml<XboxGameDiscLayout>( GenericSaveFileDialog.FileName, DiscLayout ) )
                    {
                        Log( VerbosityLevel.Informative, " ... successful", Color.Blue );
                    }
                }
            }
        }

        private void RecentMenuItem_Click( object sender, EventArgs e )
        {
            ToolStripMenuItem Item = ( ToolStripMenuItem )sender;
            string[] FileNames = new string[] { Item.Text };
            ImportTOCXGDFile( FileNames );
            AddToMRU( FileNames );
        }

        private TOCGroup PopulateLayer( ListView LayerListView, string GroupName )
        {
            TOCGroup Group = TableOfContents.GetGroup( GroupName );

            ListViewItem Item = LayerListView.Items.Add( GroupName );
            Item.Selected = Group.LayerSelected;
            Group.LayerSelected = false;

            float SizeMB = Group.Size / ( 1024.0f * 1024.0f );
            Item.SubItems.Add( SizeMB.ToString( "f2" ) );

            return ( Group );
        }

        public void PopulateListBoxes( bool FilesDirty, bool GroupsDirty, bool LayersDirty )
        {
            UpdateFiles = FilesDirty;
            UpdateGroups = GroupsDirty;
            UpdateLayers = LayersDirty;

            TimeSpan Delta = new TimeSpan( 300 * 10000 );
            LastUpdate = DateTime.Now + Delta;
        }

        public void DoPopulateListBoxes()
        {
            if( !UpdateFiles && !UpdateGroups && !UpdateLayers )
            {
                return;
            }

            if( DateTime.Now < LastUpdate )
            {
                return;
            }

            FileListView.SuspendLayout();
            GroupListView.SuspendLayout();
            Layer0ListView.SuspendLayout();
            Layer1ListView.SuspendLayout();

            TableOfContents.UpdateTOCFromGroups();

            long Layer0Free = SectorsPerLayer;
            long Layer1Free = SectorsPerLayer;

            if( UpdateFiles )
            {
                FileListView.Items.Clear();

                // Add in each entry
                foreach( TOCInfo TOCEntry in TableOfContents.TOCFileEntries )
                {
                    if( TOCEntry.Group == null )
                    {
                        FileListView.Items.Add( TOCEntry.Path );
                    }
                }
            }

            // Add in each group
            if( TableOfContents.Groups != null )
            {
                if( UpdateGroups )
                {
                    GroupListView.Items.Clear();

                    foreach( TOCGroup Group in TableOfContents.Groups.TOCGroupEntries )
                    {
                        // Populate layer list view
                        ListViewItem Item = GroupListView.Items.Add( Group.GroupName );
                        Item.Selected = Group.GroupSelected;
                        Group.GroupSelected = false;

                        switch( Group.Layer )
                        {
                            case -1:
                                Item.ForeColor = Color.Red;
                                break;
                            case 0:
                                Item.ForeColor = Color.Blue;
                                break;
                            case 1:
                                Item.ForeColor = Color.Green;
                                break;
                        }

                        float SizeMB = Group.Size / ( 1024.0f * 1024.0f );
                        Item.SubItems.Add( SizeMB.ToString( "f2" ) );
                    }
                }

                if( UpdateLayers )
                {
                    Layer0ListView.Items.Clear();

                    foreach( string GroupName in TableOfContents.Groups.TOCGroupLayer0 )
                    {
                        TOCGroup Group = PopulateLayer( Layer0ListView, GroupName );
                        Layer0Free -= Group.SectorSize;
                    }

                    float Layer0Percent = ( float )( SectorsPerLayer - Layer0Free ) * 100.0f / SectorsPerLayer;
                    Layer0Label.Text = "Layer 0 : " + Layer0Percent.ToString( "f1" ) + "% full (" + Layer0Free.ToString() + " sectors free)";

                    Layer1ListView.Items.Clear();

                    foreach( string GroupName in TableOfContents.Groups.TOCGroupLayer1 )
                    {
                        TOCGroup Group = PopulateLayer( Layer1ListView, GroupName );
                        Layer1Free -= Group.SectorSize;
                    }

                    float Layer1Percent = ( float )( SectorsPerLayer - Layer1Free ) * 100.0f / SectorsPerLayer;
                    Layer1Label.Text = "Layer 1 : " + Layer1Percent.ToString( "f1" ) + "% full (" + Layer1Free.ToString() + " sectors free)";
                }
            }

            UpdateFiles = false;
            UpdateGroups = false;
            UpdateLayers = false;

            FileListView.ResumeLayout();
            GroupListView.ResumeLayout();
            Layer0ListView.ResumeLayout();
            Layer1ListView.ResumeLayout();
        }

        // Save each original TOC with incorporated sector offsets
        public void SaveTOCs()
        {
            Dictionary<string, StreamWriter> TOCs = new Dictionary<string, StreamWriter>();
            List<TOCInfo> TOCEntries = new List<TOCInfo>();

            // Find all the TOC files and open a stream for them.
            foreach( TOCInfo TOCEntry in TableOfContents.TOCFileEntries )
            {
                if( TOCEntry.Layer != -1 && TOCEntry.OwnerTOC != null )
                {
                    if( !TOCs.ContainsKey( TOCEntry.OwnerTOC ) )
                    {
                        StreamWriter TOC = new StreamWriter( TOCEntry.OwnerTOC );
                        TOCs.Add( TOCEntry.OwnerTOC, TOC );
                    }
                }
            }

            // Iterate over the TOCs and write the entries
            foreach( TOCInfo TOCEntry in TableOfContents.TOCFileEntries )
            {
                if( TOCEntry.Layer != -1 && TOCEntry.OwnerTOC != null )
                {
                    if( TOCEntry.IsTOC )
                    {
                        // Remember these so we can refresh the size
                        TOCEntries.Add( TOCEntry );
                    }
                    else
                    {
                        string Line = TOCEntry.Size.ToString() + " " + TOCEntry.DecompressedSize.ToString() + " " + TOCEntry.LBA.ToString() + " ";
                        Line += ".." + TOCEntry.Path + " " + TOCEntry.CRCString;

                        TOCs[TOCEntry.OwnerTOC].WriteLine( Line );
                    }
                }
            }

            // Close all the streams
            foreach( StreamWriter TOC in TOCs.Values )
            {
                TOC.Close();
            }

            // Update to the new sizes
            foreach( TOCInfo TOCEntry in TOCEntries )
            {
                TOCEntry.DeriveData( TOCEntry.OwnerTOC );
            }
        }

        public void HandleCommandLine( string[] Arguments )
        {
            // Let the program know we're running from the command line
            Interactive = false;

            if( Arguments.Length < 3 )
            {
                Error( "Not enough parameters; usage 'UnrealDVDLayout <Game> <Platform> <Language> [Language]'" );
                return;
            }

            // Grab the common data
            string Game = Arguments[0] + "Game";
            string Platform = Arguments[1];
            if( Platform.ToLower() == "xenon" )
            {
                Platform = "Xbox360";
            }

            Log( VerbosityLevel.Informative, "Creating DVD layout for " + Game + " " + Platform, Color.Blue );

            // Work out the list of TOCs to load
            string TOCFileRoot = Environment.CurrentDirectory + "\\" + Game + "\\" + Platform + "TOC";
            List<string> TOCFileNames = new List<string>();
            for( int Index = 2; Index < Arguments.Length; Index++ )
            {
                if( Arguments[Index].ToUpper() == "INT" )
                {
                    TOCFileNames.Add( TOCFileRoot + ".txt" );
                }
                else
                {
                    TOCFileNames.Add( TOCFileRoot + "_" + Arguments[Index].ToUpper() + ".txt" );
                }
            }

            // Read in and setup a DVD layout
            ImportTOCXGDFile( TOCFileNames.ToArray() );

            // Save the updated TOCs
            SaveTOCs();

            // Create a new XGD with the updated TOC sizes
            CreateXGDFile();

            // Create the XGD name
            string XGDFileName = Game + "\\Build\\Layouts\\Layout";
            for( int Index = 2; Index < Arguments.Length; Index++ )
            {
                // Only add extension if this language has localised audio
                if( Config.AudioLocalisation[Arguments[Index].ToUpper()] )
                {
                    XGDFileName += "_" + Arguments[Index].ToUpper();
                }
            }
            XGDFileName += ".XGD";

            // Write out the XGD file
            WriteXml<XboxGameDiscLayout>( XGDFileName, DiscLayout );
        }

        private void FileListContext_NewGroup( object sender, EventArgs e )
        {
        }

        private bool HandleRegExpDialog( GroupRegExp Group )
        {
            if( Group.ShowDialog() == DialogResult.OK )
            {
                string Expression = Group.GetExpression();
                string GroupName = Group.GetGroupName();

                if( TableOfContents.CollateFiles( Expression, GroupName ) )
                {
                    PopulateListBoxes( true, true, true );
                    return ( true );
                }
                else
                {
                    return ( false );
                }
            }

            return ( true );
        }

        private void MakeFromRegExpMenuItem_Click( object sender, EventArgs e )
        {
            GroupRegExp NewGroup = new GroupRegExp();

            NewGroup.SetExpression( FileListView.FocusedItem.Text.Replace( "\\", "\\\\" ) );
            NewGroup.SetGroupNames( TableOfContents.GetGroupNames() );

            while( !HandleRegExpDialog( NewGroup ) )
            {
                Log( VerbosityLevel.Informative, "Invalid regular expression; please try again", Color.Blue );
            }
        }

        public void Log( VerbosityLevel Verbosity, string Line, Color TextColour )
        {
            if( Verbosity > Options.Verbosity )
            {
                return;
            }

            if( Line == null || !Ticking )
            {
                return;
            }

            // if we need to, invoke the delegate
            if( InvokeRequired )
            {
                Invoke( new DelegateAddLine( Log ), new object[] { Verbosity, Line, TextColour } );
                return;
            }

            DateTime Now = DateTime.Now;
            string FullLine = Now.ToLongTimeString() + ": " + Line;

            if( Interactive )
            {
                // Write to the log window
                MainLogWindow.Focus();
                MainLogWindow.SelectionLength = 0;

                // Only set the color if it is different than the foreground colour
                if( MainLogWindow.SelectionColor != TextColour )
                {
                    MainLogWindow.SelectionColor = TextColour;
                }

                MainLogWindow.AppendText( FullLine + "\r\n" );
            }
            else
            {
                // Write to the console
                Console.WriteLine( FullLine );
            }
        }

        public void Log( VerbosityLevel Verbosity, Array Lines, Color TextColour )
        {
            foreach( string Line in Lines )
            {
                Log( Verbosity, Line, TextColour );
            }
        }

        public void Error( string Line )
        {
            Log( VerbosityLevel.Critical, "Error: " + Line, Color.Red );
        }

        public void Warning( string Line )
        {
            Log( VerbosityLevel.Simple, "Warning: " + Line, Color.Orange );
        }

        protected void XmlSerializer_UnknownAttribute( object sender, XmlAttributeEventArgs e )
        {
        }

        protected void XmlSerializer_UnknownNode( object sender, XmlNodeEventArgs e )
        {
        }

        private T ReadXml<T>( string FileName ) where T: new()
        {
            T Instance = new T();
            try
            {
                using( Stream XmlStream = new FileStream( FileName, FileMode.Open, FileAccess.Read, FileShare.None, 256 * 1024, false ) )
                {
                    // Creates an instance of the XmlSerializer class so we can read the settings object
                    XmlSerializer ObjSer = new XmlSerializer( typeof( T ) );
                    // Add our callbacks for a busted XML file
                    ObjSer.UnknownNode += new XmlNodeEventHandler( XmlSerializer_UnknownNode );
                    ObjSer.UnknownAttribute += new XmlAttributeEventHandler( XmlSerializer_UnknownAttribute );

                    // Create an object graph from the XML data
                    Instance = ( T )ObjSer.Deserialize( XmlStream );
                }
            }
            catch( Exception E )
            {
                System.Diagnostics.Debug.WriteLine( E.Message );
            }

            return ( Instance );
        }

        private bool WriteXml<T>( string FileName, T Instance )
        {
            Stream XmlStream = null;
            try
            {
                using( XmlStream = new FileStream( FileName, FileMode.Create, FileAccess.Write, FileShare.None, 256 * 1024, false ) )
                {
                    XmlSerializer ObjSer = new XmlSerializer( typeof( T ) );

                    // Add our callbacks for a busted XML file
                    ObjSer.UnknownNode += new XmlNodeEventHandler( XmlSerializer_UnknownNode );
                    ObjSer.UnknownAttribute += new XmlAttributeEventHandler( XmlSerializer_UnknownAttribute );

                    ObjSer.Serialize( XmlStream, Instance );
                }
            }
            catch( Exception E )
            {
                System.Diagnostics.Debug.WriteLine( E.Message );
                return ( false );
            }

            return ( true );
        }
    }

    public class SettableOptions
    {
        [CategoryAttribute( "Settings" )]
        [DescriptionAttribute( "The amount of text spewed to the window." )]
        public UnrealDVDLayout.VerbosityLevel Verbosity
        {
            get { return ( LocalVerbosity ); }
            set { LocalVerbosity = value; }
        }
        [XmlEnumAttribute]
        private UnrealDVDLayout.VerbosityLevel LocalVerbosity = UnrealDVDLayout.VerbosityLevel.Informative;

        [CategoryAttribute( "Settings" )]
        [DescriptionAttribute( "The game we are creating a DVD layout for." )]
        public string GameName
        {
            get { return ( LocalGameName ); }
            set { LocalGameName = value; }
        }
        [XmlEnumAttribute]
        private string LocalGameName = "Example";

        [CategoryAttribute( "MRU" )]
        [DescriptionAttribute( "The most recently loaded files." )]
        public List<string> MRU
        {
            get { return ( LocalMRU ); }
            set { LocalMRU = value; }
        }
        [XmlAttribute]
        private List<string> LocalMRU = new List<string>();
    }

    // FIXME: Serialise this out
    public class Configuration
    {
        [CategoryAttribute( "Audio Localisation" )]
        [DescriptionAttribute( "Whether to localise the audio." )]
        public Dictionary<string, bool> AudioLocalisation
        {
            get { return ( LocalAudioLocalisation ); }
            set { LocalAudioLocalisation = value; }
        }
        [XmlEnumAttribute]
        private Dictionary<string, bool> LocalAudioLocalisation = new Dictionary<string, bool>();

        public Configuration()
        {
            AudioLocalisation.Add( "INT", true );
            AudioLocalisation.Add( "FRA", true );
            AudioLocalisation.Add( "ITA", true );
            AudioLocalisation.Add( "DEU", true );
            AudioLocalisation.Add( "ESN", true );
            AudioLocalisation.Add( "ESM", true );
            AudioLocalisation.Add( "RUS", false );
            AudioLocalisation.Add( "POL", false );
            AudioLocalisation.Add( "HUN", false );
            AudioLocalisation.Add( "CZE", false );
            AudioLocalisation.Add( "SLO", false );
            AudioLocalisation.Add( "JPN", false );
            AudioLocalisation.Add( "KOR", false );
            AudioLocalisation.Add( "CHT", false );
        }
    }
}