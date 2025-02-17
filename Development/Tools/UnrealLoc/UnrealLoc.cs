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

// TODO:
// Handle comments
// Need better validation
// Need to get a valid list of chars per language
// Need to handle when an English string changes

namespace UnrealLoc
{
    public partial class UnrealLoc : Form
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

        public enum CodePage
        {
            UnicodeUTF16 = 1200,
            CentralEuro = 1250,
            Cyrillic = 1251,
            WesternEuro = 1252
        };

        private UnrealControls.OutputWindowDocument MainLogDoc = new UnrealControls.OutputWindowDocument();
        public bool Ticking = false;
        public SettableOptions Options = null;
        public string GameName = "UT";
        private List<LanguageInfo> LanguageInfos = new List<LanguageInfo>();
        private P4 SourceControl = null;
        private Validator TheCorrector = null;
        public LanguageInfo DefaultLanguageInfo = null;

        delegate void DelegateAddLine( VerbosityLevel Verbosity, string Line, Color TextColor );

        public UnrealLoc()
        {
            InitializeComponent();
            MainLogWindow.Document = MainLogDoc;
        }

        public void Init()
        {
            Ticking = true;

            Options = ReadOptions<SettableOptions>( "UnrealLoc.Options.xml" );

            LanguageInfos.Add( new LanguageInfo( this, "INT", CheckBox_INT ) );
            LanguageInfos.Add( new LanguageInfo( this, "FRA", CheckBox_FRA ) );
            LanguageInfos.Add( new LanguageInfo( this, "ITA", CheckBox_ITA ) );
            LanguageInfos.Add( new LanguageInfo( this, "DEU", CheckBox_DEU ) );
            LanguageInfos.Add( new LanguageInfo( this, "ESN", CheckBox_ESN ) );
            LanguageInfos.Add( new LanguageInfo( this, "ESM", CheckBox_ESM ) );
            LanguageInfos.Add( new LanguageInfo( this, "RUS", CheckBox_RUS ) );
            LanguageInfos.Add( new LanguageInfo( this, "POL", CheckBox_POL ) );
            LanguageInfos.Add( new LanguageInfo( this, "HUN", CheckBox_HUN ) );
            LanguageInfos.Add( new LanguageInfo( this, "CZE", CheckBox_CZE ) );
            LanguageInfos.Add( new LanguageInfo( this, "SLO", CheckBox_SLO ) );
            LanguageInfos.Add( new LanguageInfo( this, "JPN", CheckBox_JPN ) );
            LanguageInfos.Add( new LanguageInfo( this, "KOR", CheckBox_KOR ) );
            LanguageInfos.Add( new LanguageInfo( this, "CHT", CheckBox_CHT ) );

            Log( UnrealLoc.VerbosityLevel.Critical, "Welcome to UnrealLoc - (c) 2008", Color.Black );

            TheCorrector = new Validator( this );

            PickGame Picker = new PickGame( this );
            Picker.ShowDialog();

            GameName = Picker.GameName;

            if( GameName.Length > 0 )
            {
                Log( UnrealLoc.VerbosityLevel.Critical, "Scanning loc data for game " + GameName, Color.Green );

                Show();

                ScanLocFolders();

                Log( UnrealLoc.VerbosityLevel.Critical, " ... finished scanning loc data", Color.Green );
            }
            else
            {
                Ticking = false;
            }
        }

        public void Destroy()
        {
            WriteOptions<SettableOptions>( Options, "UnrealLoc.Options.xml" );
        }

        private void UnrealLoc_FormClosed( object sender, FormClosedEventArgs e )
        {
            Ticking = false;
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

            MainLogDoc.AppendText( TextColour, FullLine + "\r\n" );
        }

        public void Log( VerbosityLevel Verbosity, Array Lines, Color TextColour )
        {
            foreach( string Line in Lines )
            {
                Log( Verbosity, Line, TextColour );
            }
        }

        public void Warning( LanguageInfo Lang, string Line )
        {
            Lang.AddWarning( Line );
        }

        public void Error( LanguageInfo Lang, string Line )
        {
            Lang.AddError( Line );
        }

        public bool Checkout( LanguageInfo Lang, string Name )
        {
            if( !Options.bAutoCheckOut )
            {
                return ( false );
            }

            if( SourceControl == null )
            {
                SourceControl = new P4( this );
            }

            bool Success = SourceControl.Checkout( Lang, Name );
            return ( Success );
        }

        public bool AddToSourceControl( LanguageInfo Lang, string Name )
        {
            if( !Options.bAutoAdd )
            {
                return ( false );
            } 
            
            if( SourceControl == null )
            {
                SourceControl = new P4( this );
            }

            bool Success = SourceControl.AddToSourceControl( Lang, Name );
            return ( Success );
        }

        public bool RevertUnchanged()
        {
            if( SourceControl == null )
            {
                SourceControl = new P4( this );
            }

            SourceControl.RevertUnchanged();
            return ( true );
        }

        private void RecursiveGetLangFiles( string LangID, string Folder, List<string> Files )
        {
            DirectoryInfo DirInfo = new DirectoryInfo( Folder );

            foreach( FileInfo Info in DirInfo.GetFiles() )
            {
                if( Info.Extension.ToUpper() == LangID )
                {
                    Files.Add( Info.FullName );
                }
            }

            foreach( DirectoryInfo Dirs in DirInfo.GetDirectories() )
            {
                RecursiveGetLangFiles( LangID, Dirs.FullName, Files );
            }
        }

        private void DumpErrorsAndWarnings()
        {
            foreach( LanguageInfo Lang in LanguageInfos )
            {
                if( Lang.LangExists && Lang.LangCheckBox.Checked )
                {
                    Lang.ErrorSummary();
                    Lang.WarningSummary();
                }
            }
        }

        private void ScanLocFolders()
        {
            // Scan the existing data for the current state of the loc data
            foreach( LanguageInfo Lang in LanguageInfos )
            {
                Lang.LangFileHandler = new FileEntryHandler( this, Lang );
                Lang.LangCheckBox.Checked = false;

                if( !Lang.FindLocFiles() )
                {
                    continue;
                }

                Lang.LangExists = true;
                Lang.LangCheckBox.Checked = Lang.LangExists;

                if( Lang.LangID == "INT" )
                {
                    DefaultLanguageInfo = Lang;
                }

                Application.DoEvents();
            }

            // Dump any warnings or errors that were found
            DumpErrorsAndWarnings();
        }

        private void Button_GenLocFiles_Click( object Sender, EventArgs E )
        {
            Log( VerbosityLevel.Critical, "Generating loc files for " + GameName + " ...", Color.Green );

            foreach( LanguageInfo Lang in LanguageInfos )
            {
                if( Lang.LangCheckBox.Checked )
                {
                    if( !Lang.LocFilesGenerated )
                    {
                        Lang.GenerateLocFiles( DefaultLanguageInfo );
                    }

                    Lang.WriteLocFiles();
                }

                Application.DoEvents();
            }

            RevertUnchanged();

            // Dump any warnings or errors that were found
            DumpErrorsAndWarnings();

            Log( VerbosityLevel.Critical, " ... finished generating missing loc files", Color.Green );
        }

        private void Button_SaveWarnings_Click( object sender, EventArgs e )
        {
            GenericFolderBrowser.Description = "Select the folder where you wish the warnings and errors to be written...";
            if( GenericFolderBrowser.ShowDialog() == DialogResult.OK )
            {
                foreach( LanguageInfo Lang in LanguageInfos )
                {
                    if( Lang.LangExists && Lang.LangCheckBox.Checked )
                    {
                        string ReportName = GenericFolderBrowser.SelectedPath + "\\Report_" + Lang.LangID + ".txt";
                        Log( VerbosityLevel.Simple, "Writing: " + ReportName, Color.Black );
                        StreamWriter Writer = new StreamWriter( ReportName, false, System.Text.Encoding.Unicode );
                        Lang.ErrorSummary( Writer );
                        Lang.WarningSummary( Writer );
                        Writer.Close();
                    }
                }
            }
        }

        private void Button_GenDiffFiles_Click( object sender, EventArgs e )
        {
            Log( VerbosityLevel.Critical, "Generating the missing loc entries for " + GameName + " ...", Color.Green );

            GenericFolderBrowser.Description = "Select the folder where you wish the files with loc changes to go...";
            if( GenericFolderBrowser.ShowDialog() == DialogResult.OK )
            {
                foreach( LanguageInfo Lang in LanguageInfos )
                {
                    if( Lang.LangCheckBox.Checked )
                    {
                        if( !Lang.LocFilesGenerated )
                        {
                            Lang.GenerateLocFiles( DefaultLanguageInfo );
                        }

                        Lang.WriteDiffLocFiles( GenericFolderBrowser.SelectedPath );
                    }
                }
            }

            Log( VerbosityLevel.Critical, " ... finished generating the missing loc entries", Color.Green );
        }

        private void Button_ImportText_Click( object Sender, EventArgs E )
        {
            Log( VerbosityLevel.Critical, "Importing localised text into " + GameName + " ...", Color.Green );

            GenericFolderBrowser.Description = "Select the folder where the loc data to import resides...";
            GenericFolderBrowser.SelectedPath = Environment.CurrentDirectory;
            if( GenericFolderBrowser.ShowDialog() == DialogResult.OK )
            {
                List<string> LangFiles = new List<string>();

                // Update with the data from the new files
                foreach( LanguageInfo Lang in LanguageInfos )
                {
#if false 
                    // If we're INT, add any new files and replace any updated ones
                    if( Lang.LangID == "INT" )
                    {
                        RecursiveGetLangFiles( "." + Lang.LangID, GenericFolderBrowser.SelectedPath, LangFiles );
                        Lang.AddNewDefaultFiles( LangFiles );
                    }
#endif

                    if( Lang.LangCheckBox.Checked )
                    {
                        if( !Lang.LocFilesGenerated )
                        {
                            Lang.GenerateLocFiles( DefaultLanguageInfo );
                        }

                        RecursiveGetLangFiles( "." + Lang.LangID, GenericFolderBrowser.SelectedPath, LangFiles );
                        Lang.ImportText( LangFiles );
                    }
                }

                // Dump any warnings or errors that were found
                DumpErrorsAndWarnings();

                Application.DoEvents();
            }
        }

        private void QuitToolStripMenuItem_Click( object sender, EventArgs e )
        {
            Ticking = false;
        }

        private void OptionsToolStripMenuItem_Click( object sender, EventArgs e )
        {
            OptionsDialog DisplayOptions = new OptionsDialog( this, Options );
            DisplayOptions.ShowDialog();
        }

        public string Validate( LanguageInfo Lang, ref LocEntry LE )
        {
            LE.Validate( Lang, LE.DefaultLE );
            return ( TheCorrector.Validate( Lang, ref LE ) );
        }

        protected void XmlSerializer_UnknownAttribute( object sender, XmlAttributeEventArgs e )
        {
        }

        protected void XmlSerializer_UnknownNode( object sender, XmlNodeEventArgs e )
        {
        }

        private T ReadOptions<T>( string OptionsFileName ) where T : new()
        {
            T Instance = new T();
            Stream XmlStream = null;
            try
            {
                // Get the XML data stream to read from
                XmlStream = new FileStream( OptionsFileName, FileMode.Open, FileAccess.Read, FileShare.None, 256 * 1024, false );

                // Creates an instance of the XmlSerializer class so we can read the settings object
                XmlSerializer ObjSer = new XmlSerializer( typeof( T ) );
                // Add our callbacks for a busted XML file
                ObjSer.UnknownNode += new XmlNodeEventHandler( XmlSerializer_UnknownNode );
                ObjSer.UnknownAttribute += new XmlAttributeEventHandler( XmlSerializer_UnknownAttribute );

                // Create an object graph from the XML data
                Instance = ( T )ObjSer.Deserialize( XmlStream );
            }
            catch( Exception E )
            {
                System.Diagnostics.Debug.WriteLine( E.Message );
            }
            finally
            {
                if( XmlStream != null )
                {
                    // Done with the file so close it
                    XmlStream.Close();
                }
            }

            return ( Instance );
        }

        private void WriteOptions<T>( T Data, string OptionsFileName )
        {
            Stream XmlStream = null;
            try
            {
                XmlStream = new FileStream( OptionsFileName, FileMode.Create, FileAccess.Write, FileShare.None, 256 * 1024, false );
                XmlSerializer ObjSer = new XmlSerializer( typeof( T ) );

                // Add our callbacks for a busted XML file
                ObjSer.UnknownNode += new XmlNodeEventHandler( XmlSerializer_UnknownNode );
                ObjSer.UnknownAttribute += new XmlAttributeEventHandler( XmlSerializer_UnknownAttribute );

                ObjSer.Serialize( XmlStream, Data );
            }
            catch( Exception E )
            {
                System.Diagnostics.Debug.WriteLine( E.Message );
            }
            finally
            {
                if( XmlStream != null )
                {
                    // Done with the file so close it
                    XmlStream.Close();
                }
            }
        }
    }

    public class SettableOptions
    {
        [CategoryAttribute( "Settings" )]
        [DescriptionAttribute( "The amount of text spewed to the window." )]
        public UnrealLoc.VerbosityLevel Verbosity
        {
            get { return ( LocalVerbosity ); }
            set { LocalVerbosity = value; }
        }
        [XmlEnumAttribute]
        private UnrealLoc.VerbosityLevel LocalVerbosity = UnrealLoc.VerbosityLevel.Informative;

        [CategoryAttribute( "Source Control" )]
        [DescriptionAttribute( "Automatically checkout files that need to be changed." )]
        public bool bAutoCheckOut
        {
            get { return ( bLocalAutoCheckOut ); }
            set { bLocalAutoCheckOut = value; }
        }
        [XmlAttribute]
        private bool bLocalAutoCheckOut = false;

        [CategoryAttribute( "Source Control" )]
        [DescriptionAttribute( "Automatically add files that need to be added." )]
        public bool bAutoAdd
        {
            get { return ( bLocalAutoAdd ); }
            set { bLocalAutoAdd = value; }
        }
        [XmlAttribute]
        private bool bLocalAutoAdd = false;

        [CategoryAttribute( "Ellipses" )]
        [DescriptionAttribute( "Automatically convert ellipses and 4 or 5 periods in sequence to 3 periods." )]
        public bool bRemoveEllipses
        {
            get { return ( bLocalRemoveEllipses ); }
            set { bLocalRemoveEllipses = value; }
        }
        [XmlAttribute]
        private bool bLocalRemoveEllipses = false;

        [CategoryAttribute( "Ellipses" )]
        [DescriptionAttribute( "Automatically convert 3, 4 and 5 periods to an ellipsis." )]
        public bool bAddEllipses
        {
            get { return ( bLocalAddEllipses ); }
            set { bLocalAddEllipses = value; }
        }
        [XmlAttribute]
        private bool bLocalAddEllipses = false;

        [CategoryAttribute( "Source Control" )]
        [DescriptionAttribute( "ClientSpec to operate on - blank is default." )]
        public string ClientSpec
        {
            get { return ( LocalClientSpec ); }
            set { LocalClientSpec = value; }
        }
        [XmlTextAttribute]
        private string LocalClientSpec = "";
    }

    public class LanguageInfo
    {
        private UnrealLoc Main = null;
        private List<string> Warnings = new List<string>();
        private List<string> Errors = new List<string>();

        public int FilesCreated = 0;
        public int ObjectsCreated = 0;
        public int LocCreated = 0;
        public int NumOrphansRemoved = 0;

        private string LocalLangID = "";
        public string LangID
        {
            get { return ( LocalLangID ); }
            set { LocalLangID = value; }
        }

        private CheckBox LocalLangCheckBox = null;
        public CheckBox LangCheckBox
        {
            get { return ( LocalLangCheckBox ); }
            set { LocalLangCheckBox = value; }
        }

        private bool LocalLangExists = false;
        public bool LangExists
        {
            get { return ( LocalLangExists ); }
            set { LocalLangExists = value; }
        }

        private FileEntryHandler LocalLangFileHandler = null;
        public FileEntryHandler LangFileHandler
        {
            get { return ( LocalLangFileHandler ); }
            set { LocalLangFileHandler = value; }
        }

        private bool LocalLocFilesGenerated = false;
        public bool LocFilesGenerated
        {
            get { return ( LocalLocFilesGenerated ); }
            set { LocalLocFilesGenerated = value; }
        }

        public LanguageInfo( UnrealLoc InMain, string ID, CheckBox CB )
        {
            Main = InMain;

            LangID = ID;
            LangCheckBox = CB;
        }

        public void AddWarning( string Line )
        {
            Warnings.Add( Line );
        }

        public void AddError( string Line )
        {
            Errors.Add( Line );
        }

        public void WarningSummary()
        {
            if( Warnings.Count > 0 && Warnings.Count < 25 )
            {
                Main.Log( UnrealLoc.VerbosityLevel.Informative, "Warning summary for " + LangID + " -", Color.Black );
                foreach( string Warning in Warnings )
                {
                    Main.Log( UnrealLoc.VerbosityLevel.Informative, Warning, Color.Orange );
                }
            }
            else if( Warnings.Count > 0 )
            {
                Main.Log( UnrealLoc.VerbosityLevel.Informative, "Warning summary for " + LangID + " - " + Warnings.Count.ToString() + " warnings", Color.Black );
            }
        }

        public void ErrorSummary()
        {
            if( Errors.Count > 0 )
            {
                Main.Log( UnrealLoc.VerbosityLevel.Critical, "Error summary for " + LangID + " -", Color.Black );
                foreach( string Error in Errors )
                {
                    Main.Log( UnrealLoc.VerbosityLevel.Critical, Error, Color.Red );
                }
            }
        }

        public void WarningSummary( StreamWriter Writer )
        {
            if( Warnings.Count > 0 )
            {
                Writer.WriteLine( "Warning summary for " + LangID + " -" );
                foreach( string Warning in Warnings )
                {
                    Writer.WriteLine( Warning );
                }
            }
        }

        public void ErrorSummary( StreamWriter Writer )
        {
            if( Errors.Count > 0 )
            {
                Writer.WriteLine( "Error summary for " + LangID + " -" );
                foreach( string Error in Errors )
                {
                    Writer.WriteLine( Error );
                }
            }
        }

        public List<FileEntry> GetFileEntries()
        {
            return ( LangFileHandler.GetFileEntries() );
        }

        public bool FindLocFiles()
        {
            return( LangFileHandler.FindLocFiles() );
        }

        public void GenerateLocFiles( LanguageInfo DefaultLang )
        {
            FilesCreated = 0;
            ObjectsCreated = 0;
            LocCreated = 0;
            NumOrphansRemoved = 0;

            // Generate all the files, objects and entries missing from the loc languages
            LangFileHandler.GenerateLocFiles( DefaultLang );

            // Remove keys that only exist in localised files
            LangFileHandler.RemoveOrphans();

            Main.Log( UnrealLoc.VerbosityLevel.Simple, " ... " + FilesCreated.ToString() + " files created", Color.Black );
            Main.Log( UnrealLoc.VerbosityLevel.Simple, " ... " + ObjectsCreated.ToString() + " objects created", Color.Black );
            Main.Log( UnrealLoc.VerbosityLevel.Simple, " ... " + LocCreated.ToString() + " loc entries created", Color.Black );
            Main.Log( UnrealLoc.VerbosityLevel.Simple, " ... " + NumOrphansRemoved.ToString() + " orphans removed", Color.Black );

            LocFilesGenerated = true;
        }

        public bool WriteLocFiles()
        {
            Main.Log( UnrealLoc.VerbosityLevel.Simple, " ... creating loc data for language: " + LangID, Color.Black );
            return ( LangFileHandler.WriteLocFiles() );
        }

        public bool WriteDiffLocFiles( string Folder )
        {
            Main.Log( UnrealLoc.VerbosityLevel.Simple, " ... creating loc diff data for language: " + LangID, Color.Black );
            return ( LangFileHandler.WriteDiffLocFiles( Folder ) );
        }

#if false
        public bool AddNewDefaultFiles( List<string> LangFiles )
        {
            foreach( string LangFile in LangFiles )
            {
                Main.Log( UnrealLoc.VerbosityLevel.Informative, " ... checking for add '" + LangFile + "'", Color.Black );
                FileInfo File = new FileInfo( LangFile );
                LangFileHandler.AddNewFile( File );
            }

            return ( true );
        }
#endif

        public bool ImportText( List<string> LangFiles )
        {
            foreach( string LangFile in LangFiles )
            {
                Main.Log( UnrealLoc.VerbosityLevel.Informative, " ... importing '" + LangFile + "'", Color.Black );
                LangFileHandler.ImportText( LangFile );
            }

            return ( true );
        }

        public string Validate( ref LocEntry LE )
        {
            return( Main.Validate( this, ref LE ) );
        }
    }
}