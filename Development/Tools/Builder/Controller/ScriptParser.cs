using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Text;

namespace Controller
{
    class ScriptParser
    {
        enum COLLATION
        {
            Engine,
            Example,
            Wrath,
            UT,
            War,
            Gear,
            ExampleMaps,
            UTMaps,
            WarMaps,
            GearMaps,
            ExampleContent,
            UTContent,
            WarContent,
            GearContent,
            PC,
            Xenon,
            PS3,
            Loc,
            G4WLive,
            Build,
            UnrealProp,
            Install,
            Tools,
            Security,
            PureAwesome,
            Count,
        }

        class ChangeList
        {
            public int Number;
            public string User;
            public int Time;
            public string Description;
            public COLLATION Collate;
            public List<string> Files = new List<string>();

            public ChangeList()
            {
                Number = 0;
                Time = 0;
                Collate = COLLATION.Engine;
            }

            public void CheckId( string Id, COLLATION CollationType )
            {
                string Test;
                int Index;

                Test = "#" + Id.ToLower();
                Index = Description.ToLower().IndexOf( Test );
                if( Index >= 0 )
                {
                    Collate = CollationType;
                    Description = Description.Remove( Index, Test.Length );
                }

                Test = "# " + Id.ToLower();
                Index = Description.ToLower().IndexOf( Test );
                if( Index >= 0 )
                {
                    Collate = CollationType;
                    Description = Description.Remove( Index, Test.Length );
                }

                Test = "[" + Id.ToLower() + "]";
                Index = Description.ToLower().IndexOf( Test );
                if( Index >= 0 )
                {
                    Collate = CollationType;
                    Description = Description.Remove( Index, Test.Length );
                }
            }

            public void CleanDescription()
            {
                string[] Parms = Description.Replace( "\r", "" ).Split( " \t".ToCharArray() );
                Description = "\t";

                for( int i = 0; i < Parms.Length; i++ )
                {
                    if( Parms[i].Length < 1 )
                    {
                        continue;
                    }

                    int Newline = Parms[i].IndexOf( '\n' );
                    while( Newline >= 0 )
                    {
                        Description += Parms[i].Substring( 0, Newline );
                        Description += Environment.NewLine + "\t";

                        Parms[i] = Parms[i].Substring( Newline + 1 );
                        Newline = Parms[i].IndexOf( '\n' );
                    }

                    Description += Parms[i] + " ";
                }

                Description.TrimEnd( " \t\r\n".ToCharArray() );
            }
        }

        class CollationType
        {
            public string Title;
            public bool Active;

            public CollationType( string InTitle )
            {
                Title = InTitle;
                Active = false;
            }

            static public COLLATION GetReportType( string CollType )
            {
                if( Enum.IsDefined( typeof( COLLATION ), CollType ) )
                {
                    COLLATION Report = ( COLLATION )Enum.Parse( typeof( COLLATION ), CollType, true );
                    return ( Report );
                }

                return ( COLLATION.Engine );
            }
        }

        private string LogFileRootName = "";

        // Location of DevEnv
        private string LocalMSVCApplication = Environment.GetEnvironmentVariable( "VS80COMNTOOLS" ) + "../IDE/Devenv.com";
        public string MSVCApplication
        {
            get { return ( LocalMSVCApplication ); }
            set { LocalMSVCApplication = value; }
        }

        // Location of MSBuild.exe
        private string LocalMSApplication = Environment.GetEnvironmentVariable( "FrameworkDir" ) + "/" + Environment.GetEnvironmentVariable( "FrameworkVersion" ) + "/MSBuild.exe";
        public string MSApplication
        {
            get { return ( LocalMSApplication ); }
            set { LocalMSApplication = value; }
        }

        // Location of signing tool
        private string LocalSignToolName = Environment.GetEnvironmentVariable( "VS80COMNTOOLS" ) + "bin/SignTool.exe";
        public string SignToolName
        {
            get { return ( LocalSignToolName ); }
            set { LocalSignToolName = value; }
        }

        // Location of source server perl script
        private string LocalSourceServerCmd = Environment.GetEnvironmentVariable( "VS80COMNTOOLS" ) + "../../../Debugging Tools for Windows/sdk/srcsrv/p4index.cmd";
        public string SourceServerCmd
        {
            get { return ( LocalSourceServerCmd ); }
            set { LocalSourceServerCmd = value; }
        }

        // Location of symstore
        private string LocalSymbolStoreApp = Environment.GetEnvironmentVariable( "VS80COMNTOOLS" ) + "../../../Debugging Tools for Windows/symstore.exe";
        public string SymbolStoreApp
        {
            get { return ( LocalSymbolStoreApp ); }
            set { LocalSymbolStoreApp = value; }
        }

        // Location of XDK tools
        private string LocalBlastTool = Environment.GetEnvironmentVariable( "XEDK" ) + "\\bin\\win32\\blast.exe";
        public string BlastTool
        {
            get { return ( LocalBlastTool ); }
            set { LocalBlastTool = value; }
        }

        // The location of the symbol store database
        private string LocalSymbolStoreLocation = Properties.Settings.Default.SymbolStoreLocation;
        public string SymbolStoreLocation
        {
            get { return ( LocalSymbolStoreLocation ); }
            set { LocalSymbolStoreLocation = value; }
        }        

        // Location of make for PS3
        private string LocalMakeApplication = "IBMake.exe";
        public string MakeApplication
        {
            get { return ( LocalMakeApplication ); }
            set { LocalMakeApplication = value; }
        }

        // Location of the executable to build an InstallShield project
        private string LocalISDevLocation = "C:/Program Files/Macrovision/IS2008/System/ISCmdBld.exe";
        public string ISDevLocation
        {
            get { return ( LocalISDevLocation ); }
            set { LocalISDevLocation = value; }
        }

        // Requested PS3 SDK - blank means use latest
        private string LocalRequestedPS3SDK = "";
        public string RequestedPS3SDK
        {
            get { return ( LocalRequestedPS3SDK ); }
            set { LocalRequestedPS3SDK = value; }
        }

        // The master file that contains the version to use
        private string LocalEngineVersionFile = "Development/Src/Core/Src/UnObjVer.cpp";
        public string EngineVersionFile
        {
            get { return ( LocalEngineVersionFile ); }
            set { LocalEngineVersionFile = value; }
        }

        // The misc version files that the engine version is propagated to
        private string LocalMiscVersionFiles = "Development/Src/ExampleGame/Live/xex.xml;Development/Src/GearGame/Live/xex.xml;Development/Src/UTGame/Live/xex.xml;Binaries/build.properties";
        public string MiscVersionFiles
        {
            get { return ( LocalMiscVersionFiles ); }
            set { LocalMiscVersionFiles = value; }
        }

		// The current job to copy down from server
		private string LocalCurrentJobName = "";
		public string CurrentJobName
		{
			get { return (LocalCurrentJobName); }
			set { LocalCurrentJobName = value; }
		}

        // The most recent changelist that was successful for this build
        private int LocalLastGoodBuild = 0;
        public int LastGoodBuild
        {
            get { return ( LocalLastGoodBuild ); }
            set { LocalLastGoodBuild = value; }
        }

        // The branch that we are compiling from e.g. UnrealEngine3-Delta
        private string LocalSourceControlBranch = "";
        public string SourceControlBranch
        {
            get { return ( LocalSourceControlBranch ); }
            set { LocalSourceControlBranch = value; }
        }

        // Cached string containing the build description
        private string LocalBuildDescription = "";
        public string BuildDescription
        {
            get { return ( LocalBuildDescription ); }
            set { LocalBuildDescription = value; }
        }

        // Cached string containing the name of the user who triggered the build
        private string LocalOperator = "";
        public string Operator
        {
            get { return ( LocalOperator ); }
            set { LocalOperator = value; }
        }

        // Cached string containing the name of the user who killed the build
        private string LocalKiller = "";
        public string Killer
        {
            get { return ( LocalKiller ); }
            set { LocalKiller = value; }
        }

        // The most recent label that was synced to
        private string LocalSyncedLabel = "";
        public string SyncedLabel
        {
            get { return ( LocalSyncedLabel ); }
            set { LocalSyncedLabel = value; }
        }

        // Time the local build started
        private DateTime LocalBuildStartedAt;
        public DateTime BuildStartedAt
        {
            get { return ( LocalBuildStartedAt ); }
            set { LocalBuildStartedAt = value; }
        }

        // Time the local build ended
        private DateTime LocalBuildEndedAt;
        public DateTime BuildEndedAt
        {
            get { return ( LocalBuildEndedAt ); }
            set { LocalBuildEndedAt = value; }
        }

        // Time the local build started waiting for network bandwidth
        private DateTime LocalStartWaitForConch;
        public DateTime StartWaitForConch
        {
            get { return ( LocalStartWaitForConch ); }
            set { LocalStartWaitForConch = value; }
        }

        // The last time the local build sent a status update
        private DateTime LocalLastWaitForConchUpdate;
        public DateTime LastWaitForConchUpdate
        {
            get { return ( LocalLastWaitForConchUpdate ); }
            set { LocalLastWaitForConchUpdate = value; }
        }

        // Whether a label has already been created to host the files
        private bool LocalNewLabelCreated = false;
        public bool NewLabelCreated
        {
            get { return ( LocalNewLabelCreated ); }
            set { LocalNewLabelCreated = value; }
        }

        // Whether to create a label when submitting files
        private bool LocalCreateLabel = false;
        public bool CreateLabel
        {
            get { return ( LocalCreateLabel ); }
            set { LocalCreateLabel = value; }
        }

        // Whether files have been checked out
        private bool LocalFilesCheckedOut = false;
        public bool FilesCheckedOut
        {
            get { return ( LocalFilesCheckedOut ); }
            set { LocalFilesCheckedOut = value; }
        }

        // Whether to append the language to the publish folder
        private bool LocalAppendLanguage = false;
        public bool AppendLanguage
        {
            get { return ( LocalAppendLanguage ); }
            set { LocalAppendLanguage = value; }
        }

        // Whether to block other publish operations to throttle the network
        private bool LocalBlockOnPublish = false;
        public bool BlockOnPublish
        {
            get { return ( LocalBlockOnPublish ); }
            set { LocalBlockOnPublish = value; }
        }

        // Whether to ignore timestamps when publishing
        private bool LocalForceCopy = false;
        public bool ForceCopy
        {
            get { return ( LocalForceCopy ); }
            set { LocalForceCopy = value; }
        }

        // Whether to enable unity stress testing
        private bool LocalUnityStressTest = false;
        public bool UnityStressTest
        {
            get { return ( LocalUnityStressTest ); }
            set { LocalUnityStressTest = value; }
        }

		// Whether to enable unity source file concatenation
		private bool LocalUnityDisable = false;
		public bool UnityDisable
		{
			get { return (LocalUnityDisable); }
			set { LocalUnityDisable = value; }
		}

		// The root location where the cooked files are copied to
        private string LocalPublishFolder = "";
        public string PublishFolder
        {
            get { return ( LocalPublishFolder ); }
            set { LocalPublishFolder = value; }
        }

        // The email addresses to send to when a build is triggered
        private string LocalTriggerAddress = "";
        public string TriggerAddress
        {
            get { return ( LocalTriggerAddress ); }
            set { LocalTriggerAddress = value; }
        }

        // The email addresses to send to when a build fails
        private string LocalFailAddress = "";
        public string FailAddress
        {
            get { return ( LocalFailAddress ); }
            set { LocalFailAddress = value; }
        }

        // The email addresses to send to when a build is successful
        private string LocalSuccessAddress = "";
        public string SuccessAddress
        {
            get { return ( LocalSuccessAddress ); }
            set { LocalSuccessAddress = value; }
        }

        // The SMS address to send ccs of emails
        private string LocalSMSAddress = "";
        public string SMSAddress
        {
            get { return ( LocalSMSAddress ); }
            set { LocalSMSAddress = value; }
        }

        // The label or build that this build syncs to
        private string LocalDependency = "";
        public string Dependency
        {
            get { return ( LocalDependency ); }
            set { LocalDependency = value; }
        }

        // The other build needed from cross platform conforming
        private string LocalSourceBuild = "";
        public string SourceBuild
        {
            get { return ( LocalSourceBuild ); }
            set { LocalSourceBuild = value; }
        }
        
        // A C++ define to be passed to the compiler
        private string LocalBuildDefine = "";
        public string BuildDefine
        {
            get { return ( LocalBuildDefine ); }
            set { LocalBuildDefine = value; }
        }

        // Destination folder of simple copy operations
        private string LocalCopyDestination = "";
        public string CopyDestination
        {
            get { return ( LocalCopyDestination ); }
            set { LocalCopyDestination = value; }
        }

        // The current command line for the builder command
        private string LocalCommandLine = "";
        public string CommandLine
        {
            get { return ( LocalCommandLine ); }
            set { LocalCommandLine = value; }
        }

        // The current mod name we are cooking for
        private string LocalModName = "";
        public string ModName
        {
            get { return ( LocalModName ); }
            set { LocalModName = value; }
        }

        // The current command line for the builder command
        private string LocalLogFileName = "";
        public string LogFileName
        {
            get { return ( LocalLogFileName ); }
            set { LocalLogFileName = value; }
        }

        // If there is a 'submit' at some point in the build script
        private bool LocalIsBuilding = false;
        public bool IsBuilding
        {
            get { return ( LocalIsBuilding ); }
            set { LocalIsBuilding = value; }
        }

        // If there is a 'tag' at some point in the build script
        private bool LocalIsPromoting = false;
        public bool IsPromoting
        {
            get { return ( LocalIsPromoting ); }
            set { LocalIsPromoting = value; }
        }

        // If there is a 'publish' at some point in the build script
        private bool LocalIsPublishing = false;
        public bool IsPublishing
        {
            get { return ( LocalIsPublishing ); }
            set { LocalIsPublishing = value; }
        }

        // If there is a 'BuildInstaller' at some point in the build script
        private bool LocalIsMakingInstall = false;
        public bool IsMakingInstall
        {
            get { return ( LocalIsMakingInstall ); }
            set { LocalIsMakingInstall = value; }
        }

        // If there is a 'UserChanges' at some point in the build script
        private bool LocalIsSendingQAChanges = false;
        public bool IsSendingQAChanges
        {
            get { return ( LocalIsSendingQAChanges ); }
            set { LocalIsSendingQAChanges = value; }
        }

        // Creates a unique log file name for language independent operations
        public string GetLogFileName( COMMANDS Command )
        {
            LogFileName = LogFileRootName + "_" + LineCount.ToString() + "_" + Command.ToString() + ".txt";
            return ( LogFileName );
        }

        // Creates a unique log file name for language dependent operations
        public string GetLogFileName( COMMANDS Command, string Lang )
        {
            LogFileName = LogFileRootName + "_" + LineCount.ToString() + "_" + Command.ToString() + "_" + Lang + ".txt";
            return ( LogFileName );
        }

        List<ChangeList> ChangeLists = new List<ChangeList>();
        CollationType[] CollationTypes = new CollationType[( int )COLLATION.Count];
        Queue<COLLATION> Reports = new Queue<COLLATION>();

        private Main Parent = null;
        private StreamReader Script = null;
        private int LineCount = 0;
        public LabelInfo LabelInfo;

        // Symbol store
        private Queue<string> SymStoreCommands = new Queue<string>();

        // Normally set once at the beginning of the file
        // List of language variants to attempt to conform
        private Queue<string> Languages = new Queue<string>();
        // Subset of languages that have a valid loc'd variant
        private Queue<string> ValidLanguages;
        private bool CheckErrors = true;
        private bool CheckWarnings = true;
        private TimeSpan OperationTimeout = new TimeSpan( 0, 10, 0 );
        private TimeSpan RespondingTimeout = new TimeSpan( 0, 10, 0 );

        private string ClientSpec = "";

        // Changed multiple times during script execution
        private List<string> MadeWritableFiles = new List<string>();
        private List<string> PublishDestinations = new List<string>();

        public string BuildConfiguration = "Release";
        private string ScriptConfiguration = "";
        private string CookConfiguration = "";
        private string InstallConfiguration = "";
        private string ContentPath = "";
        private string CompressionConfiguration = "size";

        // Working variables
        private COMMANDS CommandID = COMMANDS.Config;
        private ERRORS ErrorLevel = ERRORS.None;

        public ScriptParser( Main InParent, string ScriptFileName, string Description, string Branch, int LastGoodChangeList, DateTime BuildStarted, string InOperator )
        {
            Parent = InParent;
            try
            {
                SourceControlBranch = Branch;
                LastGoodBuild = LastGoodChangeList;
                BuildStartedAt = BuildStarted;
                BuildDescription = Description;
                Operator = InOperator;
                StartWaitForConch = BuildStarted;

                string ScriptFile = "Development/Builder/Scripts/" + ScriptFileName + ".build";
                string NewScriptFile = "Development/Builder/Scripts/Current.build";

                FileInfo ScriptInfo = new FileInfo( NewScriptFile );
                if( ScriptInfo.Exists )
                {
                    ScriptInfo.IsReadOnly = false;
                    ScriptInfo.Delete();
                }

                FileInfo Info = new FileInfo( ScriptFile );
                Info.CopyTo( NewScriptFile, true );

                Script = new StreamReader( NewScriptFile );
                Parent.Log( "Using build script '" + SourceControlBranch + "/" + ScriptFile + "'", Color.Magenta );

                LogFileRootName = "Development/Builder/Logs/Builder_[" + GetTimeStamp() + "]";

                LabelInfo = new LabelInfo( Parent, this );
                LabelInfo.BuildType = Description;

                // Make sure there are no defines hanging around
                Environment.SetEnvironmentVariable( "CL", "" );

                CollationTypes[( int )COLLATION.Engine] = new CollationType( "Engine Code Changes" );
                CollationTypes[( int )COLLATION.Wrath] = new CollationType( "Wrath Changes" );
                CollationTypes[( int )COLLATION.Example] = new CollationType( "ExampleGame Changes" );
                CollationTypes[( int )COLLATION.UT] = new CollationType( "UTGame Changes" );
                CollationTypes[( int )COLLATION.War] = new CollationType( "WarGame/Delta Changes" );
                CollationTypes[( int )COLLATION.Gear] = new CollationType( "GearGame Changes" );
                CollationTypes[( int )COLLATION.ExampleMaps] = new CollationType( "ExampleGame Map Changes" );
                CollationTypes[( int )COLLATION.UTMaps] = new CollationType( "UTGame Map Changes" );
                CollationTypes[( int )COLLATION.WarMaps] = new CollationType( "WarGame/Delta Map Changes" );
                CollationTypes[( int )COLLATION.GearMaps] = new CollationType( "GearGame Map Changes" );
                CollationTypes[( int )COLLATION.ExampleContent] = new CollationType( "ExampleGame Content Changes" );
                CollationTypes[( int )COLLATION.UTContent] = new CollationType( "UTGame Content Changes" );
                CollationTypes[( int )COLLATION.WarContent] = new CollationType( "WarGame/Delta Content Changes" );
                CollationTypes[( int )COLLATION.GearContent] = new CollationType( "GearGame Content Changes" );
                CollationTypes[( int )COLLATION.PC] = new CollationType( "PC Specific Changes" );
                CollationTypes[( int )COLLATION.Xenon] = new CollationType( "Xenon Changes" );
                CollationTypes[( int )COLLATION.PS3] = new CollationType( "PS3 Changes" );
                CollationTypes[( int )COLLATION.Loc] = new CollationType( "Localisation Changes" );
                CollationTypes[( int )COLLATION.G4WLive] = new CollationType( "G4W Live Changes" );
                CollationTypes[( int )COLLATION.Build] = new CollationType( "Build System Changes" );
                CollationTypes[( int )COLLATION.UnrealProp] = new CollationType( "UnrealProp Changes" );
                CollationTypes[( int )COLLATION.Install] = new CollationType( "Installer Changes" );
                CollationTypes[( int )COLLATION.Tools] = new CollationType( "Tools Changes" );
                CollationTypes[( int )COLLATION.Security] = new CollationType( "Security Changes" );
                CollationTypes[( int )COLLATION.PureAwesome] = new CollationType( "Cliffy B's Pure Awesomeness" );

                // Set the order of the reports 
                Reports.Enqueue( COLLATION.Engine );
                Reports.Enqueue( COLLATION.PC );
                Reports.Enqueue( COLLATION.Xenon );
                Reports.Enqueue( COLLATION.PS3 );
                Reports.Enqueue( COLLATION.PureAwesome );

                Reports.Enqueue( COLLATION.UT );
                Reports.Enqueue( COLLATION.UTMaps );
                Reports.Enqueue( COLLATION.UTContent );

                Reports.Enqueue( COLLATION.Wrath );

                Reports.Enqueue( COLLATION.War );
                Reports.Enqueue( COLLATION.WarMaps );
                Reports.Enqueue( COLLATION.WarContent );

                Reports.Enqueue( COLLATION.Gear );
                Reports.Enqueue( COLLATION.GearMaps );
                Reports.Enqueue( COLLATION.GearContent );

                Reports.Enqueue( COLLATION.Example );
                Reports.Enqueue( COLLATION.ExampleMaps );
                Reports.Enqueue( COLLATION.ExampleContent );

                Reports.Enqueue( COLLATION.Loc );
                Reports.Enqueue( COLLATION.G4WLive );
                Reports.Enqueue( COLLATION.Tools );
                Reports.Enqueue( COLLATION.Security );
                Reports.Enqueue( COLLATION.Install );
                Reports.Enqueue( COLLATION.Build );
                Reports.Enqueue( COLLATION.UnrealProp );
            }
            catch
            {
                Parent.Log( "Error, problem loading build script", Color.Red );
            }
        }

        public void Destroy()
        {
            Parent.Log( LineCount.ToString() + " lines of build script parsed", Color.Magenta );
            if( Script != null )
            {
                Script.Close();
            }
        }

        public void Write( StreamWriter Log, string Output )
        {
            if( Log != null )
            {
                try
                {
                    Log.Write( DateTime.Now.ToLongTimeString() + ": " + Output + Environment.NewLine );
                }
                catch( Exception Ex )
                {
                    Parent.Log( "Log write exception: exception '" + Ex.Message + "'", Color.Red );
                    Parent.Log( "Log write exception: intended to write '" + Output + "'", Color.Red );
                }
            }
        }

        public Queue<string> GetLanguages()
        {
            return ( Languages );
        }

        public void SetValidLanguages( Queue<string> ValidLangs )
        {
            ValidLanguages = ValidLangs;
        }

        public Queue<string> GetValidLanguages()
        {
            return ( ValidLanguages );
        }

        public bool GetCheckErrors()
        {
            return ( CheckErrors );
        }

        public bool GetCheckWarnings()
        {
            return ( CheckWarnings );
        }

        public string GetScriptConfiguration()
        {
            if( ScriptConfiguration.Length > 0 )
            {
                string[] Configs = ScriptConfiguration.Split( " \t".ToCharArray() );
                string ScriptConfig = "";
                foreach( string Config in Configs )
                {
                    if( Config.ToLower() == "fr" )
                    {
                        ScriptConfig += " -final_release";
                    }
                    else
                    {
                        ScriptConfig += " -" + Config;
                    }
                }

                return ( ScriptConfig );
            }
            return ( "" );
        }

        public string GetCookConfiguration()
        {
            if( CookConfiguration.Length > 0 )
            {
                return ( " -" + CookConfiguration );
            }
            return ( "" );
        }

        public string GetInstallConfiguration()
        {
            if( InstallConfiguration.ToLower() == "target" )
            {
                return ( " -t" );
            }
            if( InstallConfiguration.ToLower() == "demo" )
            {
                return ( " -d" );
            } 
            return ( "" );
        }

        public string GetContentPath()
        {
            if( ContentPath.Length > 0 )
            {
                return ( " -paths=" + ContentPath + " -cookfordemo" );
            }
            return ( "" );
        }

        public string GetModName()
        {
            if( ModName.Length > 0 )
            {
                return ( " -usermodname=" + ModName + " -user" );
            }
            return ( "" );
        }

        public string GetCompressionConfiguration()
        {
            if( CompressionConfiguration.ToLower() == "speed" )
            {
                return ( " -NOPACKAGECOMPRESSION" );
            }
            return ( "" );
        }

        public void ClearPublishDestinations()
        {
            PublishDestinations.Clear();
        }

        public void AddPublishDestination( string Dest )
        {
            PublishDestinations.Add( Dest );
        }

        public List<string> GetPublishDestinations()
        {
            return ( PublishDestinations );
        }

        public void AddUpdateSymStore( string Command )
        {
            SymStoreCommands.Enqueue( Command );
        }

        public string PopUpdateSymStore()
        {
            if( !UpdateSymStoreEmpty() )
            {
                return ( SymStoreCommands.Dequeue() );
            }

            return ( "" );
        }

        public bool UpdateSymStoreEmpty()
        {
            return ( SymStoreCommands.Count == 0 );
        }

        public string GetClientSpec()
        {
            return ( ClientSpec );
        }

        public void SetErrorLevel( ERRORS Error )
        {
            ErrorLevel = Error;
        }

        public ERRORS GetErrorLevel()
        {
            return ( ErrorLevel );
        }

        public string GetTimeStamp()
        {
            string TimeStamp = BuildStartedAt.Year + "-"
                        + BuildStartedAt.Month.ToString( "00" ) + "-"
                        + BuildStartedAt.Day.ToString( "00" ) + "_"
                        + BuildStartedAt.Hour.ToString( "00" ) + "."
                        + BuildStartedAt.Minute.ToString( "00" );
            return ( TimeStamp );
        }

        public void SetCommandID( COMMANDS ID )
        {
            CommandID = ID;
        }

        public COMMANDS GetCommandID()
        {
            return ( CommandID );
        }

        public int SafeStringToInt( string Number )
        {
            int Result = 0;

            try
            {
                Number = Number.Trim();
                Result = Int32.Parse( Number );
            }
            catch
            {
            }

            return ( Result );
        }

        public void AddMadeWritableFileSpec( string FileSpec )
        {
            MadeWritableFiles.Add( FileSpec );
        }

        public List<string> GetMadeWritableFiles()
        {
            return ( MadeWritableFiles );
        }

        public void ClearMadeWritableFiles()
        {
            MadeWritableFiles.Clear();
        }

        public GameConfig AddCheckedOutGame()
        {
            GameConfig Config = new GameConfig( CommandLine, LabelInfo.Platform, BuildConfiguration, true );
            LabelInfo.Games.Add( Config );

            return ( Config );
        }

        public GameConfig CreateGameConfig( string Game, string InPlatform )
        {
            GameConfig Config = new GameConfig( Game, InPlatform, BuildConfiguration, true );

            return ( Config );
        }

        public GameConfig CreateGameConfig( string Game )
        {
            GameConfig Config = new GameConfig( Game, LabelInfo.Platform, BuildConfiguration, true );

            return ( Config );
        }

        public GameConfig CreateGameConfig()
        {
            GameConfig Config = new GameConfig( LabelInfo.Game, LabelInfo.Platform, BuildConfiguration, true );

            return ( Config );
        }

        public GameConfig CreateGameConfig( bool Local )
        {
            GameConfig Config = new GameConfig( LabelInfo.Game, LabelInfo.Platform, BuildConfiguration, Local );

            return ( Config );
        }

        public TimeSpan GetTimeout()
        {
            return ( OperationTimeout );
        }

        public TimeSpan GetRespondingTimeout()
        {
            return ( RespondingTimeout );
        }

        // Set the CL env var based on required defines
        public void HandleMSVCDefines( StreamWriter Log )
        {
            Environment.SetEnvironmentVariable( "CL", "" );

            string EnvVar = "";
            if( LabelInfo.DefineA.Length > 0 )
            {
                EnvVar += "/D " + LabelInfo.DefineA;
                if( LabelInfo.DefineB.Length > 0 )
                {
                    EnvVar += " /D " + LabelInfo.DefineB;
                }
            }

            if( EnvVar.Length > 0 )
            {
                Environment.SetEnvironmentVariable( "CL", EnvVar );
            }

            Write( Log, "Setting CL=" + EnvVar );
        }

        // Passed in on the command line, nothing special to do here
        public string HandleGCCDefines()
        {
            Environment.SetEnvironmentVariable( "CL", "" );

            string CommandLine = "";
            if( LabelInfo.DefineA.Length > 0 )
            {
                CommandLine = "ADD_DEFINES=\"" + LabelInfo.DefineA;
                if( LabelInfo.DefineB.Length > 0 )
                {
                    CommandLine += " " + LabelInfo.DefineB;
                }
                CommandLine += "\"";
            }

            return ( CommandLine );
        }

        // Passed in on the command line, nothing special to do here
        public string HandleUBTDefines()
        {
            Environment.SetEnvironmentVariable( "CL", "" );

            string CommandLine = "";
            if( LabelInfo.DefineA.Length > 0 )
            {
                CommandLine += " -define " + LabelInfo.DefineA;
                if( LabelInfo.DefineB.Length > 0 )
                {
                    CommandLine += " -define " + LabelInfo.DefineB;
                }
            }

            return ( CommandLine );
        }

        public COMMANDS ParseNextLine()
        {
            string Line;

            if( Script == null )
            {
                ErrorLevel = ERRORS.NoScript;
                CommandID = COMMANDS.Error;
                return( CommandID );
            }

            Line = Script.ReadLine();
            if( Line == null )
            {
                Parent.Log( "[STATUS] Script parsing completed", Color.Magenta );
                CommandID = COMMANDS.Finished;
                return( CommandID );
            }

            LineCount++;
            CommandID = COMMANDS.Config;
            CommandLine = "";
            string[] Parms = Line.Split( " \t".ToCharArray() );
            if( Parms.Length > 0 )
            {
                string Command = Parms[0].ToLower();
                CommandLine = Line.Substring( Parms[0].Length ).Trim();
                CommandLine = CommandLine.Replace( '\\', '/' );
                // Need to keep the double backslashes for network locations
                CommandLine = CommandLine.Replace( "//", "\\\\" );

                if( Command.Length == 0 || Command == "//" )
                {
                    // Comment - ignore
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "status" )
                {
                    // Status string - just echo
                    Parent.Log( "[STATUS] " + Parent.ExpandString( CommandLine, SourceControlBranch ), Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "watchstart" )
                {
                    // Start a timer
                    string PerfKeyName = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "[WATCHSTART " + PerfKeyName + "]", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "watchstop" )
                {
                    // Stop a timer
                    Parent.Log( "[WATCHSTOP]", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "checkerrors" )
                {
                    CheckErrors = true;
                    Parent.Log( "Checking for errors ENABLED", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "ignoreerrors" )
                {
                    CheckErrors = false;
                    Parent.Log( "Checking for errors DISABLED", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "checkwarnings" )
                {
                    CheckWarnings = true;
                    Parent.Log( "Checking for warnings ENABLED", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "ignorewarnings" )
                {
                    CheckWarnings = false;
                    Parent.Log( "Checking for warnings DISABLED", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "languagespecific" )
                {
                    AppendLanguage = true;
                    Parent.Log( "Publish folder has the language appended", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "languageagnostic" )
                {
                    AppendLanguage = false;
                    Parent.Log( "Publish folder does NOT have the language appended", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "createlabel" )
                {
                    CreateLabel = true;
                    Parent.Log( "A label will be created", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "updatelabel" )
                {
                    CommandID = COMMANDS.UpdateLabel;
                }
                else if( Command == "updatefolder" )
                {
                    CommandID = COMMANDS.UpdateFolder;
                }
                else if( Command == "tag" )
                {
                    Parent.Log( "The latest build label will be copied to '" + CommandLine + "'", Color.Magenta );
                    IsPromoting = true;
                    CommandID = COMMANDS.SCC_Tag;
                }
                else if( Command == "tagfile" )
                {
                    Parent.Log( "The file '" + CommandLine + "' will be tagged to the dependency", Color.Magenta );
                    CommandID = COMMANDS.SCC_TagFile;
                }
                else if( Command == "tagpcs" )
                {
                    Parent.Log( "The precompiled shader file will be tagged to the dependency", Color.Magenta );
                    CommandID = COMMANDS.SCC_TagPCS;
                }
                else if( Command == "tagexe" )
                {
                    Parent.Log( "The compiled exe file(s) will be tagged to the dependency", Color.Magenta );
                    CommandID = COMMANDS.SCC_TagExe;
                }
                else if( Command == "tagcontenttag" )
                {
                    Parent.Log( "The content tags package will be tagged to the dependency", Color.Magenta );
                    CommandID = COMMANDS.SCC_TagContentTag;
                }
                else if( Command == "taglayout" )
                {
                    Parent.Log( "The layout will be tagged to the dependency", Color.Magenta );
                    CommandID = COMMANDS.SCC_TagLayout;
                } 
                else if( Command == "report" )
                {
                    Reports.Clear();

                    for( int i = 1; i < Parms.Length; i++ )
                    {
                        COLLATION Collation = CollationType.GetReportType( Parms[i] );
                        Reports.Enqueue( Collation );
                    }
                }
                else if( Command == "triggeraddress" )
                {
                    // Email addresses to use when the build is triggered
                    TriggerAddress = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Triggeraddress set to '" + TriggerAddress + "'", Color.Magenta );
                    CommandID = COMMANDS.TriggerMail;
                }
                else if( Command == "failaddress" )
                {
                    // Email addresses to use if the build fails
                    FailAddress = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Failaddress set to '" + FailAddress + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "successaddress" )
                {
                    // Email addresses to use if the build succeeds
                    SuccessAddress = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Successaddress set to '" + SuccessAddress + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "sms" )
                {
                    // Email addresses to use if the build succeeds
                    SMSAddress = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "SMS address set to '" + SMSAddress + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "redflash" )
                {
                    // Email addresses to use if the build succeeds
                    SMSAddress = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Paging '" + SMSAddress + "'", Color.Magenta );
                    CommandID = COMMANDS.RedFlash;
                }
                else if( Command == "define" )
                {
                    // Define for the compiler to use
                    LabelInfo.ClearDefines();

                    BuildDefine = Parent.ExpandString( CommandLine, SourceControlBranch );
                    if( BuildDefine.Length > 0 )
                    {
                        string[] DefineParms = BuildDefine.Split( " \t".ToCharArray() );
                        foreach( string Define in DefineParms )
                        {
                            LabelInfo.AddDefine( Define );
                        }
                    }
                    Parent.Log( "Define set to '" + BuildDefine + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "language" )
                {
                    // Language for the cooker to use
                    LabelInfo.Language = Parent.ExpandString( CommandLine, SourceControlBranch ).ToUpper();
                    Parent.Log( "Language set to '" + LabelInfo.Language + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "languages" )
                {
                    // List of languages used to conform
                    Languages.Clear();

                    string LangLine = Parent.ExpandString( CommandLine, SourceControlBranch ).ToUpper();
                    string[] LangParms = LangLine.Split( " \t".ToCharArray() );
                    foreach( string Lang in LangParms )
                    {
                        if( Lang.Length > 0 )
                        {
                            Languages.Enqueue( Lang );
                        }
                    }

                    Parent.Log( "Number of languages to process: " + Languages.Count, Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "clientspec" )
                {
                    // Clientspec for source control to use
                    ClientSpec = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "ClientSpec set to '" + ClientSpec + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "timeout" )
                {
                    // Number of minutes to failure
                    int OpTimeout = SafeStringToInt( CommandLine );
                    OperationTimeout = new TimeSpan( 0, OpTimeout, 0 );
                    Parent.Log( "Timeout set to " + OpTimeout.ToString() + " minutes", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "msvcapplication" )
                {
                    // MSVC command line app
                    MSVCApplication = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "MSVC executable set to '" + MSVCApplication + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "msapplication" )
                {
                    // VCBuild command line app
                    MSApplication = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "MSBuild executable set to '" + MSApplication + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "makeapplication" )
                {
                    // make app
                    MakeApplication = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Make executable set to '" + MakeApplication + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "isdevlocation" )
                {
                    // InstallShield app
                    ISDevLocation = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "InstallShield executable set to '" + ISDevLocation + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "symbolstoreapplication" )
                {
                    // Sign tool location
                    SymbolStoreApp = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Symbol store executable set to '" + SymbolStoreApp + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "signtoolapplication" )
                {
                    // Sign tool location
                    SignToolName = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Sign tool executable set to '" + SignToolName + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "sourceservercommand" )
                {
                    // bat/perl script to index pdbs
                    SourceServerCmd = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Source server script set to '" + SourceServerCmd + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "symbolstorelocation" )
                {
                    // bat/perl script to index pdbs
                    SymbolStoreLocation = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Symbol store location set to '" + SymbolStoreLocation + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "useps3sdk" )
                {
                    // The PS3 SDK version to use
                    RequestedPS3SDK = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Requested PS3 SDK version '" + RequestedPS3SDK + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "versionfile" )
                {
                    // Main version file
                    EngineVersionFile = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Engine version file set to '" + EngineVersionFile + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "miscversionfiles" )
                {
                    // Version files that have a version propagated to
                    MiscVersionFiles = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Misc version files set to '" + MiscVersionFiles + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "game" )
                {
                    // Game we are interested in
                    LabelInfo.Game = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Game set to '" + LabelInfo.Game + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "platform" )
                {
                    // Platform we are interested in
                    LabelInfo.Platform = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Platform set to '" + LabelInfo.Platform + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "buildconfig" )
                {
                    // Build we are interested in
                    BuildConfiguration = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Build configuration set to '" + BuildConfiguration + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "scriptconfig" )
                {
                    // Script release or final_release we are interested in
                    ScriptConfiguration = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Script configuration set to '" + ScriptConfiguration + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "cookconfig" )
                {
                    // Cook config we are interested in (eg. CookForServer - dedicated server trimming)
                    CookConfiguration = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Cook configuration set to '" + CookConfiguration + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "modname" )
                {
                    // The mod name we are cooking for
                    ModName = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "ModName set to '" + ModName + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "installconfig" )
                {
                    // Install we are interested in
                    InstallConfiguration = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Install configuration set to '" + InstallConfiguration + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "contentpath" )
                {
                    // Used after a wrangle
                    ContentPath = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "ContentPath set to '" + ContentPath + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "compressionconfig" )
                {
                    // Set to size or speed
                    CompressionConfiguration = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Compression biased for '" + CompressionConfiguration + "'", Color.Magenta );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "dependency" )
                {
                    Dependency = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "Dependency set to '" + Dependency + "'", Color.DarkGreen );
                    CommandID = COMMANDS.SetDependency;
                }
                else if( Command == "sourcebuild" )
                {
                    SourceBuild = Parent.ExpandString( CommandLine, SourceControlBranch );
                    Parent.Log( "SourceBuild set to '" + SourceBuild + "'", Color.DarkGreen );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "copydest" )
                {
                    CopyDestination = Parent.ExpandString( CommandLine, SourceControlBranch );
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "blockonpublish" )
                {
                    BlockOnPublish = true;
                    CommandID = COMMANDS.Config;
                }
                else if( Command == "forcecopy" )
                {
                    ForceCopy = true;
                    CommandID = COMMANDS.Config;
                }
				else if (Command == "unity")
				{
					string UnityParameter = Parent.ExpandString(CommandLine, SourceControlBranch);
					if (UnityParameter == "disable")
					{
						UnityDisable = true;
					}
					else if (UnityParameter == "stresstest")
					{
						UnityStressTest = true;
					}
					Parent.Log("Unity set to '" + UnityParameter + "'", Color.Magenta);
					CommandID = COMMANDS.Config;
				}
				else if (Command == "cleanup")
                {
                    CommandID = COMMANDS.Cleanup;
                }
                else if( Command == "copy" )
                {
                    CommandID = COMMANDS.SimpleCopy;
                }
                else if( Command == "sourcebuildcopy" )
                {
                    CommandID = COMMANDS.SourceBuildCopy;
                }
                else if( Command == "delete" )
                {
                    CommandID = COMMANDS.SimpleDelete;
                }
                else if( Command == "rename" )
                {
                    CommandID = COMMANDS.SimpleRename;
                }
                else if( Command == "renamecopy" )
                {
                    CommandID = COMMANDS.RenamedCopy;
                }
                else if( Command == "checkconsistency" )
                {
                    CommandID = COMMANDS.SCC_CheckConsistency;
                }
                else if( Command == "getcookedbuild" )
                {
                    CommandID = COMMANDS.GetCookedBuild;
                }
                else if( Command == "getcookedlanguage" )
                {
                    CommandID = COMMANDS.GetCookedLanguage;
                }
                else if( Command == "getinstallablebuild" )
                {
                    CommandID = COMMANDS.GetInstallableBuild;
                }
                else if( Command == "buildinstaller" )
                {
                    IsMakingInstall = true;
                    CommandID = COMMANDS.BuildInstaller;
                }
                else if( Command == "copyinstaller" )
                {
                    CommandID = COMMANDS.CopyInstaller;
                }
                else if( Command == "createdvdlayout" )
                {
                    CommandID = COMMANDS.CreateDVDLayout;
                }
                else if( Command == "sync" )
                {
                    CommandID = COMMANDS.SCC_Sync;
                }
                else if( Command == "artistsync" )
                {
                    CommandID = COMMANDS.SCC_ArtistSync;
                }
                else if( Command == "getchanges" )
                {
                    CommandID = COMMANDS.SCC_GetChanges;
                }
                else if( Command == "sendqachanges" )
                {
                    CommandID = COMMANDS.Config;
                    IsSendingQAChanges = true;
                }
                else if( Command == "syncsinglechangelist" )
                {
                    CommandID = COMMANDS.SCC_SyncSingleChangeList;
                }
                else if( Command == "checkout" )
                {
                    CommandID = COMMANDS.SCC_Checkout;
                }
                else if( Command == "openfordelete" )
                {
                    CommandID = COMMANDS.SCC_OpenForDelete;
                }
                else if( Command == "checkoutgame" )
                {
                    CommandID = COMMANDS.SCC_CheckoutGame;
                }
                else if( Command == "checkoutcontenttag" )
                {
                    CommandID = COMMANDS.SCC_CheckoutContentTag;
                }
                else if( Command == "checkoutshader" )
                {
                    CommandID = COMMANDS.SCC_CheckoutShader;
                }
                else if( Command == "checkoutdialog" )
                {
                    CommandID = COMMANDS.SCC_CheckoutDialog;
                }
                else if( Command == "checkoutfonts" )
                {
                    CommandID = COMMANDS.SCC_CheckoutFonts;
                }
                else if( Command == "checkoutlocpackage" )
                {
                    CommandID = COMMANDS.SCC_CheckoutLocPackage;
                }
                else if( Command == "checkoutgdf" )
                {
                    CommandID = COMMANDS.SCC_CheckoutGDF;
                }
                else if( Command == "checkoutlayout" )
                {
                    CommandID = COMMANDS.SCC_CheckoutLayout;
                }
                else if( Command == "makewritable" )
                {
                    CommandID = COMMANDS.MakeWritable;
                }
                else if( Command == "submit" )
                {
                    IsBuilding = true;
                    CommandID = COMMANDS.SCC_Submit;
                }
                else if( Command == "createnewlabel" )
                {
                    CommandID = COMMANDS.SCC_CreateNewLabel;
                }
                else if( Command == "updatelabeldescription" )
                {
                    CommandID = COMMANDS.SCC_UpdateLabelDescription;
                }
                else if( Command == "revert" )
                {
                    CommandID = COMMANDS.SCC_Revert;
                }
                else if( Command == "revertfile" )
                {
                    CommandID = COMMANDS.SCC_RevertFile;
                }
                else if( Command == "waitforjobs" )
                {
                    CommandID = COMMANDS.WaitForJobs;
                }
                else if( Command == "addjob" )
                {
                    CommandID = COMMANDS.AddJob;
                }
                else if( Command == "addunrealgamejob" )
                {
                    CommandID = COMMANDS.AddUnrealGameJob;
                }
                else if( Command == "addunrealfullgamejob" )
                {
                    CommandID = COMMANDS.AddUnrealFullGameJob;
                }
                else if( Command == "clean" )
                {
                    CommandID = COMMANDS.Clean;
                }
                else if( Command == "msbuild" )
                {
                    CommandID = COMMANDS.MSBuild;
                }
                else if( Command == "unrealbuild" )
                {
                    CommandID = COMMANDS.UnrealBuild;
                }
                else if( Command == "msvcclean" )
                {
                    CommandID = COMMANDS.MSVCClean;
                }
                else if( Command == "msvcbuild" )
                {
                    CommandID = COMMANDS.MSVCBuild;
                }
                else if( Command == "msvcfull" )
                {
                    CommandID = COMMANDS.MSVCFull;
                }
                else if( Command == "gccclean" )
                {
                    CommandID = COMMANDS.GCCClean;
                }
                else if( Command == "gccbuild" )
                {
                    CommandID = COMMANDS.GCCBuild;
                }
                else if( Command == "gccfull" )
                {
                    CommandID = COMMANDS.GCCFull;
                }
                else if( Command == "shaderclean" )
                {
                    CommandID = COMMANDS.ShaderClean;
                }
                else if( Command == "shaderbuild" )
                {
                    CommandID = COMMANDS.ShaderBuild;
                }
                else if( Command == "shaderbuildstate" )
                {
                    CommandID = COMMANDS.ShaderBuildState;
                }
                else if( Command == "shaderfull" )
                {
                    CommandID = COMMANDS.ShaderFull;
                }
                else if( Command == "ps3makepatchbinary" )
                {
                    CommandID = COMMANDS.PS3MakePatchBinary;
                }
                else if( Command == "ps3makepatch" )
                {
                    CommandID = COMMANDS.PS3MakePatch;
                }
                else if( Command == "buildscript" )
                {
                    CommandID = COMMANDS.BuildScript;
                }
                else if( Command == "preheatmapoven" )
                {
                    CommandID = COMMANDS.PreHeatMapOven;
                }
                else if( Command == "preheatdlc" )
                {
                    CommandID = COMMANDS.PreHeatDLC;
                }
                else if( Command == "cookmaps" )
                {
                    CommandID = COMMANDS.CookMaps;
                }
                else if( Command == "cooksounds" )
                {
                    CommandID = COMMANDS.CookSounds;
                }
                else if( Command == "createhashes" )
                {
                    CommandID = COMMANDS.CreateHashes;
                }
                else if( Command == "wrangle" )
                {
                    CommandID = COMMANDS.Wrangle;
                }
                else if( Command == "publish" )
                {
                    IsPublishing = true;
                    CommandID = COMMANDS.Publish;
                }
                else if( Command == "publishlanguage" )
                {
                    IsPublishing = true;
                    CommandID = COMMANDS.PublishLanguage;
                }
                else if( Command == "publishlayout" )
                {
                    IsPublishing = true;
                    CommandID = COMMANDS.PublishLayout;
                }
                else if( Command == "publishlayoutlanguage" )
                {
                    IsPublishing = true;
                    CommandID = COMMANDS.PublishLayoutLanguage;
                }
                else if( Command == "publishdlc" )
                {
                    IsPublishing = true;
                    CommandID = COMMANDS.PublishDLC;
                }
                else if( Command == "conform" )
                {
                    CommandID = COMMANDS.Conform;
                }
                else if( Command == "createcontenttags" )
                {
                    CommandID = COMMANDS.CreateContentTags;
                }
                else if( Command == "crossbuildconform" )
                {
                    CommandID = COMMANDS.CrossBuildConform;
                }
                else if( Command == "trigger" )
                {
                    CommandID = COMMANDS.Trigger;
                }
                else if( Command == "bumpengineversion" )
                {
                    CommandID = COMMANDS.BumpEngineVersion;
                }
                else if( Command == "getengineversion" )
                {
                    CommandID = COMMANDS.GetEngineVersion;
                }
                else if( Command == "updategdfversion" )
                {
                    CommandID = COMMANDS.UpdateGDFVersion;
                }
                else if( Command == "updatesymbolserver" )
                {
                    CommandID = COMMANDS.UpdateSourceServer;
                }
                else if( Command == "blast" )
                {
                    CommandID = COMMANDS.Blast;
                }
                else if( Command == "sign" )
                {
                    CommandID = COMMANDS.CheckSigned;
                }
                else if( Command == "wait" )
                {
                    CommandID = COMMANDS.Wait;
                }
                else if( Command == "checkspace" )
                {
                    CommandID = COMMANDS.CheckSpace;
                }
                else
                {
                    CommandLine = Line;
                    ErrorLevel = ERRORS.IllegalCommand;
                    CommandID = COMMANDS.Error;
                }
            }
            return ( CommandID );
        }

        // Goes over a changelist description and extracts the relevant information
        public void ProcessChangeList( Array Output )
        {
            ChangeList CL = new ChangeList();

            foreach( string Line in Output )
            {
                string[] Parms = Line.Split( " \t".ToCharArray() );
                if( Parms.Length > 0 )
                {
                    if( Parms[0] == "change" )
                    {
                        CL.Number = SafeStringToInt( Parms[1] );
                        if( CL.Number > LabelInfo.Changelist )
                        {
                            LabelInfo.Changelist = CL.Number;
                        }
                    }
                    else if( Parms[0] == "user" )
                    {
                        CL.User = Parms[1];
                    }
                    else if( Parms[0] == "time" )
                    {
                        CL.Time = SafeStringToInt( Parms[1] );
                    }
                    else if( Parms[0] == "desc" )
                    {
                        CL.Description = Line.Substring( "desc ".Length );
                    }
                    else if( Parms[0].StartsWith( "depotFile" ) )
                    {
                        CL.Files.Add( Line.Substring( "depotFile0 //depot/".Length ) );
                    }
                }
            }

            if( !CL.Description.StartsWith( "[BUILDER] " ) && !CL.Description.StartsWith( Parent.LabelRoot ) )
            {
                ChangeLists.Add( CL );
            }
        }

        private void CollateGameChanges( ChangeList CL, string File, string Game, string MapExt, COLLATION CollCode, COLLATION CollMap, COLLATION CollContent )
        {
            string CleanFile = File.ToLower().Replace( '\\', '/' );

            if( CleanFile.IndexOf( MapExt ) >= 0 )
            {
                CL.Collate = CollMap;
            }
            else if( CleanFile.IndexOf( "development/src/" + Game ) >= 0 )
            {
                CL.Collate = CollCode;
            }
            else if( CleanFile.IndexOf( Game + "game/" ) >= 0 )
            {
                CL.Collate = CollContent;
            }
        }

        private void CollateChanges()
        {
            foreach( ChangeList CL in ChangeLists )
            {
                string Desc = CL.Description.ToLower();

                // Special codewords
                CL.CheckId( "Gears2", COLLATION.Gear );
                CL.CheckId( "Gears 2", COLLATION.Gear );
                CL.CheckId( "Gears", COLLATION.Gear );

                CL.CheckId( "Envy", COLLATION.UT );
                CL.CheckId( "Delta", COLLATION.War );
                CL.CheckId( "360", COLLATION.Xenon );
                CL.CheckId( "XBox360", COLLATION.Xenon );
                CL.CheckId( "XBox", COLLATION.Xenon );
                CL.CheckId( "Xenon", COLLATION.Xenon );
                CL.CheckId( "Live", COLLATION.G4WLive );
                CL.CheckId( "G4W", COLLATION.G4WLive );

                CL.CheckId( "Pure Awesome", COLLATION.PureAwesome );

                // Generic enum conversions
                foreach( COLLATION CollationType in Enum.GetValues( typeof( COLLATION ) ) )
                {
                    CL.CheckId( CollationType.ToString(), CollationType );
                }

                // Check files that were checked in
                foreach( string File in CL.Files )
                {
                    if( CL.Collate == COLLATION.Engine )
                    {
                        CollateGameChanges( CL, File, "ut", ".ut3", COLLATION.UT, COLLATION.UTMaps, COLLATION.UTContent );
                        CollateGameChanges( CL, File, "gear", ".gear", COLLATION.Gear, COLLATION.GearMaps, COLLATION.GearContent );
                        CollateGameChanges( CL, File, "war", ".war", COLLATION.War, COLLATION.WarMaps, COLLATION.WarContent );
                        CollateGameChanges( CL, File, "example", ".umap", COLLATION.Example, COLLATION.ExampleMaps, COLLATION.ExampleContent );
                    }
                }

                CollationTypes[( int )CL.Collate].Active = true;

                // Remove identifiers and blank lines
                CL.CleanDescription();
            }
        }

        private void GetCollatedChanges( StringBuilder Changes, COLLATION Collate, string User )
        {
            CollationType CollType = CollationTypes[( int )Collate];
            if( CollType.Active )
            {
                bool CollTypeHasEntries = false;

                foreach( ChangeList CL in ChangeLists )
                {
                    if( CL.Collate == Collate && ( User.Length == 0 || User.ToLower() == CL.User.ToLower() ) )
                    {
                        CollTypeHasEntries = true;
                        break;
                    }
                }

                if( CollTypeHasEntries )
                {
                    Changes.Append( "------------------------------------------------------------" + Environment.NewLine );
                    Changes.Append( "\t" + CollType.Title + Environment.NewLine );
                    Changes.Append( "------------------------------------------------------------" + Environment.NewLine + Environment.NewLine );

                    foreach( ChangeList CL in ChangeLists )
                    {
                        if( CL.Collate == Collate && ( User.Length == 0 || User.ToLower() == CL.User.ToLower() ) )
                        {
                            DateTime Time = new DateTime( 1970, 1, 1 );
                            Time += new TimeSpan( ( long )CL.Time * 10000 * 1000 );
                            Changes.Append( "Change: " + CL.Number.ToString() + " by " + CL.User + " on " + Time.ToLocalTime().ToString() + Environment.NewLine );
                            Changes.Append( CL.Description + Environment.NewLine );
                        }
                    }
                }
            }
        }

        public string GetChanges( string User )
        {
            StringBuilder Changes = new StringBuilder();

            Changes.Append( LabelInfo.Description );

            Changes.Append( Environment.NewLine );

            CollateChanges();

            foreach( COLLATION Collation in Reports )
            {
                GetCollatedChanges( Changes, Collation, User );
            }

            return ( Changes.ToString() );
        }
	}
}
