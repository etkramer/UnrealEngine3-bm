using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Management;
using System.Runtime.InteropServices;
using System.ServiceProcess;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Forms;

namespace Controller
{
    enum MODES
    {
        Init,
        Monitor,
        Wait,
		WaitForJobs,
		Finalise,
        Exit,
    }

    enum COMMANDS
    {
        Error,
        Config,
        TriggerMail,
        Cleanup,
        SCC_CheckConsistency,
        SCC_Sync,
        SCC_ArtistSync,
        SCC_GetChanges,
        SCC_SyncSingleChangeList,
        SCC_Checkout,
        SCC_OpenForDelete,
        SCC_CheckoutGame,
        SCC_CheckoutShader,
        SCC_CheckoutDialog,
        SCC_CheckoutFonts,
        SCC_CheckoutLocPackage,
        SCC_CheckoutGDF,
        SCC_CheckoutContentTag,
        SCC_CheckoutLayout,
        SCC_Submit,
        SCC_CreateNewLabel,
        SCC_DeleteLabel,
        SCC_UpdateLabelDescription,
        SCC_Revert,
        SCC_RevertFile,
        SCC_Tag,
        SCC_TagFile,
        SCC_TagPCS,
        SCC_TagExe,
        SCC_TagContentTag,
        SCC_TagLayout,
        WaitForJobs,
		AddJob,
        AddUnrealGameJob,
        AddUnrealFullGameJob,
        SetDependency,
        MakeWritable,
        MakeReadOnly,
        Clean,
        MSBuild,
        MSVCClean,
        MSVCBuild,
        UnrealBuild,
        UsePS3SDK,
        GCCClean,
        GCCBuild,
        ShaderClean,
        ShaderBuild,
        ShaderBuildState,
        BuildScript,
        PreHeatMapOven,
        PreHeatDLC,
        CookMaps,
        CookSounds,
        CreateHashes,
        Wrangle,
        Publish,
        PublishLanguage,
        PublishLayout,
        PublishLayoutLanguage,
        PublishDLC,
        GetCookedBuild,
        GetCookedLanguage,
        GetInstallableBuild,
        BuildInstaller,
        CopyInstaller,
        CreateDVDLayout,
        Conform,
        CreateContentTags,
        CrossBuildConform,
        Finished,
        Trigger,
        BumpEngineVersion,
        GetEngineVersion,
        UpdateGDFVersion,
        UpdateSourceServer,
        UpdateSymbolServer,
        UpdateSymbolServerTick,
        Blast,
        PS3MakePatchBinary,
        PS3MakePatch,
        CheckSigned,
        Sign,
        SimpleCopy,
        SourceBuildCopy,
        SimpleDelete,
        SimpleRename,
        RenamedCopy,
        Wait,
        CheckSpace,
        UpdateLabel,
        UpdateFolder,
        RedFlash,

        VCFull,       // Composite command - clean then build
        MSVCFull,       // Composite command - clean then build
        GCCFull,        // Composite command - clean then build
        ShaderFull,     // Composite command - clean then build
    }

    enum ERRORS
    {
        None,
        TheWorldIsEnding,
        NoScript,
        IllegalCommand,
        TimedOut,
        WaitTimedOut,
        FailedJobs,
        Crashed,
        Cleanup,
        SCC_CheckConsistency,
        SCC_Sync,
        SCC_ArtistSync,
        SCC_GetChanges,
        SCC_SyncSingleChangeList,
        SCC_Checkout,
        SCC_OpenForDelete,
        SCC_CheckoutGame,
        SCC_CheckoutShader,
        SCC_CheckoutDialog,
        SCC_CheckoutFonts,
        SCC_CheckoutLocPackage,
        SCC_CheckoutGDF,
        SCC_CheckoutContentTag,
        SCC_CheckoutLayout,
        SCC_Resolve,
        SCC_Submit,
        SCC_CreateNewLabel,
        SCC_DeleteLabel,
        SCC_UpdateLabelDescription,
        SCC_Revert,
        SCC_RevertFile,
        SCC_GetLatest,
        SCC_Tag,
        SCC_TagMessage,
        SCC_TagFile,
        SCC_TagPCS,
        SCC_TagExe,
        SCC_TagContentTag,
        SCC_TagLayout,
        WaitForJobs,
		AddJob,
        AddUnrealGameJob,
        AddUnrealFullGameJob,
        SCC_GetClientRoot,
        SetDependency,
        MakeWritable,
        MakeReadOnly, 
        Process,
        Clean,
        MSBuild,
        MSVCClean,
        MSVCBuild,
        UnrealBuild,
        UsePS3SDK,
        GCCClean,
        GCCBuild,
        ShaderClean,
        ShaderBuild,
        ShaderBuildState,
        BuildScript,
        PreHeatMapOven,
        PreHeatDLC,
        CookMaps,
        CookSounds,
        CreateHashes,
        Wrangle,
        CookingSuccess,
        CookerSyncSuccess,
        Publish,
        PublishLanguage,
        PublishLayout,
        PublishLayoutLanguage,
        PublishDLC,
        GetCookedBuild,
        GetCookedLanguage,
        GetInstallableBuild,
        BuildInstaller,
        CopyInstaller,
        CreateDVDLayout,
        Conform,
        CreateContentTags,
        CrossBuildConform,
        BumpEngineVersion,
        GetEngineVersion,
        UpdateGDFVersion,
        UpdateSourceServer,
        UpdateSymbolServer,
        UpdateSymbolServerTick,
        Blast,
        PS3MakePatchBinary,
        PS3MakePatch,
        CheckSigned,
        Sign,
        SimpleCopy,
        SourceBuildCopy,
        SimpleDelete,
        SimpleRename,
        RenamedCopy,
        Wait,
        CheckSpace,
        UpdateLabel,
        UpdateFolder,
        RedFlash
    }

    enum RevisionType
    {
        Invalid,
        ChangeList,
        Label
    }

    public partial class Main : Form
    {
        // For consistent formatting to the US standard (05/29/2008 11:33:52)
        public const string US_TIME_DATE_STRING_FORMAT = "MM/dd/yyyy HH:mm:ss"; 

        public string LabelRoot = "UnrealEngine3";

        BuilderDB DB = null;
        Emailer Mailer = null;
        P4 SCC = null;
        Watcher Watcher = null;

        private DateTime LastPingTime = DateTime.Now;
        private DateTime LastTempTime = DateTime.Now;
        private DateTime LastBuildPollTime = DateTime.Now;
        private DateTime LastJobPollTime = DateTime.Now;
        private DateTime LastRestartTime = DateTime.Now;
        private DateTime LastSystemDownTime = DateTime.Now;
        private DateTime LastBuildKillTime = DateTime.Now;
        private DateTime LastJobKillTime = DateTime.Now;
        private DateTime LastCheckForJobTime = DateTime.Now;

		// The next scheduled time we'll update the database
		DateTime NextMaintenanceTime = DateTime.Now;
		DateTime NextStatsUpdateTime = DateTime.Now;

		// Time periods, in seconds, between various checks
		uint MaintenancePeriod = 30;
		uint StatsUpdatePeriod = 600;

		/// 
		/// A collection of simple performance counters used to get basic monitoring stats
		/// 
		private class PerformanceStats
		{
			// CPU stats
			public PerformanceCounter CPUBusy = null;

			// Disk stats
			public PerformanceCounter LogicalDiskReadLatency = null;
			public PerformanceCounter LogicalDiskWriteLatency = null;
			public PerformanceCounter LogicalDiskTransferLatency = null;
			public PerformanceCounter LogicalDiskQueueLength = null;
			public PerformanceCounter LogicalDiskReadQueueLength = null;

			// Memory stats
			public PerformanceCounter SystemMemoryAvailable = null;
		};
		PerformanceStats LocalPerfStats = null;
		
		// The root folder of the clientspec - where all the branches reside
        private string LocalRootFolder = "";
        public string RootFolder
        {
            get { return ( LocalRootFolder ); }
            set { LocalRootFolder = value; }
        }

        // A timestamp of when this builder was compiled
        private DateTime LocalCompileDateTime;
        public DateTime CompileDateTime
        {
            get { return ( LocalCompileDateTime ); }
            set { LocalCompileDateTime = value; }
        }

        // The installed version of MSVC
        private string LocalMSVCVersion = "";
        public string MSVCVersion
        {
            get { return ( LocalMSVCVersion ); }
            set { LocalMSVCVersion = value; }
        }

        // The installed version of DirectX
        private string LocalDXVersion = "";
        public string DXVersion
        {
            get { return ( LocalDXVersion ); }
            set { LocalDXVersion = value; }
        }
        
        // The local current version of the XDK
        private string LocalXDKVersion = "";
        public string XDKVersion
        {
            get { return ( LocalXDKVersion ); }
            set { LocalXDKVersion = value; }
        }

        // The local current version of the PS3 SDK
        private string LocalPS3SDKVersion = "";
        public string PS3SDKVersion
        {
            get { return ( LocalPS3SDKVersion ); }
            set { LocalPS3SDKVersion = value; }
        }

        // Whether this machine is rated as being quicker than average
        private bool LocalMachineIs64Bit = false;
        public bool MachineIs64Bit
        {
            get { return ( LocalMachineIs64Bit ); }
            set { LocalMachineIs64Bit = value; }
        }

        // Number of processors in this machine - used to define number of threads to use when choice is available
        private int LocalNumProcessors = 2;
        public int NumProcessors
        {
            get { return ( LocalNumProcessors ); }
            set { LocalNumProcessors = value; }
        }

        // Number of parallel jobs to use (based on the number of logical processors)
        private int LocalNumJobs = 4;
        public int NumJobs
        {
            get { return ( LocalNumJobs ); }
            set { LocalNumJobs = value; }
        }

        // Whether this machine is locked to a specific build
        private int LocalMachineLock = 0;
        public int MachineLock
        {
            get { return ( LocalMachineLock ); }
            set { LocalMachineLock = value; }
        }

        // A unique id for each job set
        private long LocalJobSpawnTime = 0;
        public long JobSpawnTime
        {
            get { return ( LocalJobSpawnTime ); }
            set { LocalJobSpawnTime = value; }
        }

        // The number of jobs placed into the job table
        private int LocalNumJobsToWaitFor = 0;
        public int NumJobsToWaitFor
        {
            get { return ( LocalNumJobsToWaitFor ); }
            set { LocalNumJobsToWaitFor = value; }
        }

		// The number of jobs completed so far
		private int LocalNumCompletedJobs = 0;
		public int NumCompletedJobs
		{
			get { return (LocalNumCompletedJobs); }
			set { LocalNumCompletedJobs = value; }
		}

        private string[] PotentialBranches = new string[] { "UnrealEngine3", "UnrealEngine3-Builder", "UnrealEngine3-UT3-PC", "UnrealEngine3-UT3-PS3", "UnrealEngine3-UT3-PS3-Cert", "UnrealEngine3-UT3-X360", "UnrealEngine3-UT3-X360-Cert" };
        public List<string> AvailablePS3SDKs = new List<string>();

        public bool Ticking = true;
        public bool Restart = false;
        public string MachineName = "";
        private string LoggedOnUser = "";
        private string SourceBranch = "";

        private int CommandID = 0;
        private int JobID = 0;
        private int BuilderID = 0;
        private int BuildLogID = 0;
        private int Promotable = 0;
        private Command CurrentCommand = null;
        private Queue<COMMANDS> PendingCommands = new Queue<COMMANDS>();
        private int BlockingBuildID = 0;
        private int BlockingBuildLogID = 0;
        private DateTime BlockingBuildStartTime = DateTime.Now;
        private ScriptParser Builder = null;
        private MODES Mode = 0;
        private string FinalStatus = "";

        delegate void DelegateAddLine( string Line, Color TextColor );
        delegate void DelegateSetStatus( string Line );
        delegate void DelegateMailGlitch();

        private void SetupSyncServices()
        {
#if FALSE
            // get a reference to the installed service
			System.ServiceProcess.ServiceController Service = null;
			try
			{
				Service = new ServiceController( "CookerSyncHost" );
                // This will exception if the service is not installed
                Service.Stop();

                // is the source control version of the service newer than the service exe?
                FileInfo SourceService = new FileInfo( "CookerSyncService.exe" );
                FileInfo ServiceInstance = new FileInfo( "CookerSyncServiceInstance.exe" );

                // if times are different, update it (or if was never copied, or if it's not running)
                if( !ServiceInstance.Exists || SourceService.LastWriteTime.CompareTo( ServiceInstance.LastWriteTime ) != 0 )
                {
                    // now copy over the .exe 
                    if( ServiceInstance.Exists )
                    {
                        ServiceInstance.IsReadOnly = false;
                        ServiceInstance.Delete();
                    }

                    // update the executable
                    SourceService.CopyTo( "CookerSyncServiceInstance.exe", true );
                }

                // start the service back up again
                Service.Start();

                // Notify that the service restarted fine
                Log( "CookerSyncHost service restarted", Color.Green );
            }
            catch
            {
                Log( "FAILED to locate CookerSyncHost service", Color.Red );
            }
#endif
        }

        // Recursively mark each file in the folder as read write so it can be deleted
        private void MarkReadWrite( string Directory )
        {
            DirectoryInfo DirInfo = new DirectoryInfo( Directory );
            if( DirInfo.Exists )
            {
                FileInfo[] Infos = DirInfo.GetFiles();
                foreach( FileInfo Info in Infos )
                {
                    if( Info.Exists )
                    {
                        Info.IsReadOnly = false;
                    }
                }

                DirectoryInfo[] SubDirInfo = DirInfo.GetDirectories();
                foreach( DirectoryInfo Info in SubDirInfo )
                {
                    if( Info.Exists )
                    {
                        MarkReadWrite( Info.FullName );
                    }
                }
            }
        }

        // Recursively delete an entire directory tree
        public void DeleteDirectory( string Path, int DaysOld )
        {
            DirectoryInfo DirInfo = new DirectoryInfo( Path );
            if( DirInfo.Exists )
            {
                DirectoryInfo[] Directories = DirInfo.GetDirectories();
                foreach( DirectoryInfo Directory in Directories )
                {
                    TimeSpan Age = DateTime.UtcNow - Directory.CreationTimeUtc;
                    TimeSpan MaxAge = new TimeSpan( DaysOld, 0, 0, 0 );
                    if( Age > MaxAge )
                    {
                        MarkReadWrite( Directory.FullName );
                        Directory.Delete( true );
                    }
                }

                FileInfo[] Files = DirInfo.GetFiles();
                foreach( FileInfo File in Files )
                {
                    TimeSpan Age = DateTime.UtcNow - File.CreationTimeUtc;
                    TimeSpan MaxAge = new TimeSpan( DaysOld, 0, 0, 0 );
                    if( Age > MaxAge )
                    {
                        File.IsReadOnly = false;
                        File.Delete();
                    }
                }
            }
        }

        // Copies one folder to another
        public bool CopyDirectory( string SourceDir, string DestDir )
        {
            try
            {
                DirectoryInfo Source = new DirectoryInfo( SourceDir );
                if( !Source.Exists )
                {
                    return ( false );
                }

                DirectoryInfo Dest = new DirectoryInfo( DestDir );
                Dest.Create();

                foreach( FileInfo File in Source.GetFiles() )
                {
                    File.CopyTo( DestDir + "\\" + File.Name );
                }

                foreach( DirectoryInfo Dir in Source.GetDirectories() )
                {
                    if( !CopyDirectory( Dir.FullName, DestDir + "\\" + Dir.Name ) )
                    {
                        return ( false );
                    }
                }
            }
            catch
            {
                return ( false );
            }

            return ( true );
        }

        // Ensure the base build folder exists to copy builds to
        public void EnsureDirectoryExists( string Path )
        {
            DirectoryInfo Dir = new DirectoryInfo( Path );
            if( !Dir.Exists )
            {
                Dir.Create();
            }
        }

        public string SetRequestedSDKs()
        {
            // Compiling the tools requires a current SDK
            if( Builder.LabelInfo.Platform.ToLower() == "ps3" || Builder.LabelInfo.Platform.ToLower() == "pc" )
            {
                if( AvailablePS3SDKs.Count == 0 )
                {
                    return ( "No available PS3 SDKs!" );
                }

                if( Builder.RequestedPS3SDK.Length == 0 )
                {
                    Builder.RequestedPS3SDK = AvailablePS3SDKs[AvailablePS3SDKs.Count - 1];
                }

                // Is the requested SDK already the current?
                if( Builder.RequestedPS3SDK.Length > 0 && Builder.RequestedPS3SDK == PS3SDKVersion )
                {
                    return( " ... current PS3 SDK (" + PS3SDKVersion + ") is already correct." );
                }

                // Current is an incorrect version - delete current and copy the new one
                string RootDir = GetPS3RootDir();
                DeleteDirectory( RootDir + "\\current", 0 );

                if( !CopyDirectory( RootDir + "\\" + Builder.RequestedPS3SDK, RootDir + "\\current" ) )
                {
                    return( "Error, failed to copy PS3 SDK: " + Builder.RequestedPS3SDK );
                }

                PS3SDKVersion = GetPS3SDKVersion( GetPS3RootDir() + "\\current" );
                
                return ( " ... current PS3 SDK set to: " + Builder.RequestedPS3SDK );
            }

            // Compiling the tools requires a current SDK
            if( Builder.LabelInfo.Platform.ToLower() == "xenon" || Builder.LabelInfo.Platform.ToLower() == "pc" )
            {
                //TODO
            }
            return ( "" );
        }

        // Extract the time and date of compilation from the version number
        private void CalculateCompileDateTime()
        {
            System.Version Version = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
            CompileDateTime = new DateTime( Version.Build * TimeSpan.TicksPerDay + Version.Revision * TimeSpan.TicksPerSecond * 2 ).AddYears( 1999 );

            Log( "Controller compiled on " + CompileDateTime.ToString(), Color.Blue );
        }

        private void GetInfo()
        {
            ManagementObjectSearcher Searcher = new ManagementObjectSearcher( "Select * from Win32_ComputerSystem" );
            ManagementObjectCollection Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                Object Value;

                Value = Object.GetPropertyValue( "UserName" );
                if( Value != null )
                {
                    LoggedOnUser = Value.ToString();
                }

                Value = Object.GetPropertyValue( "Name" );
                if( Value != null )
                {
                    MachineName = Value.ToString();
                }

                Log( "Welcome \"" + LoggedOnUser + "\" running on \"" + MachineName + "\"", Color.Blue );
                break;
            }
        }

        private void GetMachineRating()
        {
            MachineIs64Bit = false;

            try
            {
                ManagementObjectSearcher Searcher = new ManagementObjectSearcher( "Select * from Win32_OperatingSystem" );
                ManagementObjectCollection Collection = Searcher.Get();

                foreach( ManagementObject Object in Collection )
                {
                    Object Value;

                    Value = Object.GetPropertyValue( "OSArchitecture" );
                    if( Value != null )
                    {
                        MachineIs64Bit = Value.ToString().Contains( "64-bit" );
                    }
                }
            }
            catch
            {
            }
        }

        private void ValidatePCEnvVars()
        {
            try
            {
                string[] PS3EnvVars = new string[] { "VSINSTALLDIR", "VCINSTALLDIR", "FrameworkDir", "FrameworkVersion", "FrameworkSDKDir", "INCLUDE", "LIB", "LIBPATH" };

                foreach( string EnvVar in PS3EnvVars )
                {
                    string EnvVarValue = Environment.GetEnvironmentVariable( EnvVar );
                    if( EnvVarValue != null )
                    {
                        Log( " ...... EnvVar '" + EnvVar + "' = '" + EnvVarValue + "'", Color.Blue );
                    }
                    else
                    {
                        Log( " ...... Failed to find environment variable '" + EnvVar + "'!", Color.Red );
                    }
                }
            }
            catch
            {
            }
        }

        private void GetMSVCVersion()
        {
            try
            {
                string MSVCExe = Environment.GetEnvironmentVariable( "VS80COMNTOOLS" ) + "/../IDE/Devenv.exe";
                if( MSVCExe != null )
                {
                    FileVersionInfo VI = FileVersionInfo.GetVersionInfo( MSVCExe );
                    MSVCVersion = VI.FileDescription + " version " + VI.ProductVersion;
                    Log( "Compiling with: " + MSVCVersion, Color.Blue );
                }
                else
                {
                    Log( "Could not find VS80COMNTOOLS environment variable", Color.Blue );
                }

                ValidatePCEnvVars();
            }
            catch
            {
                Ticking = false;
            }
        }

        private void ValidatePS3EnvVars()
        {
            try
            {
                string[] PS3EnvVars = new string[] { "CELL_SDK", "CELL_SDK_WIN", "SCE_PS3_ROOT" };

                foreach( string EnvVar in PS3EnvVars )
                {
                    string EnvVarValue = Environment.GetEnvironmentVariable( EnvVar );
                    if( EnvVarValue != null )
                    {
                        Log( " ...... EnvVar '" + EnvVar + "' = '" + EnvVarValue + "'", Color.Blue );
                    }
                }
            }
            catch
            {
            }
        }

        private string GetPS3RootDir()
        {
            string RootDir = "";
            
            try
            {
                RootDir = Environment.GetEnvironmentVariable( "SCE_PS3_ROOT" ).ToLower();
                if( RootDir != null && RootDir.EndsWith( "/current" ) )
                {
                    RootDir = RootDir.Substring( 0, RootDir.Length - "/current".Length );
                }
            }
            catch
            {
            }

            return ( RootDir );
        }

        private string GetPS3SDKVersion( string Path )
        {
            string Line = "";
            try
            {
                string PS3SDK = Path + "/version-SDK";
                StreamReader Reader = new StreamReader( PS3SDK );
                if( Reader != null )
                {
                    Line = Reader.ReadToEnd();
                    Reader.Close();
                }
            }
            catch
            {
            }

            return ( Line.Trim() );
        }

        private void GetAvailablePS3SDKs()
        {
            try
            {
                DirectoryInfo RootDirInfo = new DirectoryInfo( GetPS3RootDir() );
                foreach( DirectoryInfo Dir in RootDirInfo.GetDirectories() )
                {
                    if( Dir.Name.Length == "000.000".Length )
                    {
                        string Version = GetPS3SDKVersion( Dir.FullName );
                        if( Version == Dir.Name )
                        {
                            AvailablePS3SDKs.Add( Dir.Name );
                        }
                    }
                }

                AvailablePS3SDKs.Sort();
            }
            catch
            {
            }

            Log( "Available PS3 SDKs ...", Color.Blue );
            foreach( string PS3SDK in AvailablePS3SDKs )
            {
                Log( " ... " + PS3SDK, Color.Blue );
            }
        }

        private void GetCurrentPS3SDKVersion()
        {
            PS3SDKVersion = GetPS3SDKVersion( GetPS3RootDir() + "\\current" );

            if( PS3SDKVersion.Length > 0 )
            {
                Log( " ... using PS3 SDK: " + PS3SDKVersion, Color.Blue );

                ValidatePS3EnvVars();
            }
        }

        private void GetXDKVersion()
        {
            try
            {
                string Line;

                string XDK = Environment.GetEnvironmentVariable( "XEDK" ) + "/include/win32/vs2005/xdk.h";
                if( XDK != null )
                {
                    StreamReader Reader = new StreamReader( XDK );
                    if( Reader != null )
                    {
                        Line = Reader.ReadLine();
                        while( Line != null )
                        {
                            if( Line.StartsWith( "#define" ) )
                            {
                                int Offset = Line.IndexOf( "_XDK_VER" );
                                if( Offset >= 0 )
                                {
                                    XDKVersion = Line.Substring( Offset + "_XDK_VER".Length ).Trim();
                                    break;
                                }
                            }

                            Line = Reader.ReadLine();
                        }
                        Reader.Close();
                        Log( " ... using XDK: " + XDKVersion, Color.Blue );
                    }
                }
                else
                {
                    Log( "Could not find XDK environment variable", Color.Blue );
                }
            }
            catch
            {
            }
        }

        private void GetDirectXVersion()
        {
            try
            {
                DXVersion = "UNKNOWN";
                string DirectXLocation = Environment.GetEnvironmentVariable( "DXSDK_DIR" );
                if( DirectXLocation != null )
                {
                    int OpenParenIndex = DirectXLocation.LastIndexOf( '(' );
                    int CloseParenIndex = DirectXLocation.LastIndexOf( ')' );
                    if( OpenParenIndex >= 0 && CloseParenIndex >= 0 && CloseParenIndex > OpenParenIndex )
                    {
                        DXVersion = DirectXLocation.Substring( OpenParenIndex + 1, CloseParenIndex - OpenParenIndex - 1 );
                        Log( " ... found " + DXVersion + " DirectX", Color.Blue );
                    }
                    else
                    {
                        Log( " ... did not find any version of DirectX", Color.Blue );
                    }
                }
                else
                {
                    Log( " ... did not find DirectX environment variable", Color.Blue );
                }
            }
            catch
            {
                Ticking = false;
            }
        }

        private void BroadcastMachine()
        {
            string Query, Command;
            int ID;

            // Clearing out zombie connections if the process stopped unexpectedly
            Query = "SELECT ID FROM [Builders] WHERE ( Machine ='" + MachineName + "' AND State != 'Dead' AND State != 'Zombied' )";
            ID = DB.ReadInt( Query );
            while( ID != 0 )
            {
                Command = "UPDATE [Builders] SET State = 'Zombied' WHERE ( ID = " + ID.ToString() + " )";
                DB.Update( Command );

                Log( "Cleared zombie connection " + ID.ToString() + " from database", Color.Orange );
                ID = DB.ReadInt( Query );
            }

            // Clear out zombie commands
            Query = "SELECT ID FROM [Commands] WHERE ( Machine ='" + MachineName + "' )";
            ID = DB.ReadInt( Query );
            if( ID != 0 )
            {
                DB.Delete( "Commands", ID, "Machine" );
                DB.Delete( "Commands", ID, "BuildLogID" );
            }

            // Clear out any orphaned jobs
            Query = "SELECT ID FROM [Jobs] WHERE ( Machine ='" + MachineName + "' AND Complete = 0 )";
            ID = DB.ReadInt( Query );
            if( ID != 0 )
            {
                DB.SetBool( "Jobs", ID, "Complete", true );
                Log( "Cleared orphaned job " + ID.ToString() + " from database", Color.Orange );
            }

            Log( "Registering '" + MachineName + "' with database", Color.Blue );

            // Insert machine as new row
            string OS64Bit = "0";
            if( MachineIs64Bit )
            {
                OS64Bit = "1";
            }
            Command = "INSERT INTO [Builders] ( Machine, State, StartTime, OS64Bit ) VALUES ( '" + MachineName + "', 'Connected', '" + DateTime.Now.ToString( US_TIME_DATE_STRING_FORMAT ) + "', " + OS64Bit + " )";
            DB.Update( Command );

            Query = "SELECT @@IDENTITY";
            ID = DB.ReadDecimal( Query );
            if( ID != 0 )
            {
                foreach( string Branch in PotentialBranches )
                {
                    if( SCC.BranchExists( Branch ) )
                    {
                        Command = "INSERT INTO [Branches] ( BuilderID, Branch ) VALUES ( " + ID.ToString() + ", '" + Branch + "' )";
                        DB.Update( Command );
                    }
                }
#if DEBUG
                Command = "INSERT INTO [Branches] ( BuilderID, Branch ) VALUES ( " + ID.ToString() + ", 'UnrealEngine3-Builder' )";
                DB.Update( Command );
#endif
            }
        }

        private void MaintainMachine()
        {
			// Check to see if it's time for maintenance
			if (DateTime.Now >= NextMaintenanceTime)
			{
				// Schedule the next update
				NextMaintenanceTime = DateTime.Now.AddSeconds(MaintenancePeriod);

				string Command = "SELECT ID FROM [Builders] WHERE ( Machine = '" + MachineName + "' AND State != 'Dead' AND State != 'Zombied' )";
                int ID = DB.ReadInt( Command );
                if( ID != 0 )
                {
					// We'll always update the time
					Command = "UPDATE [Builders] SET CurrentTime = '" + DateTime.Now.ToString( US_TIME_DATE_STRING_FORMAT ) + "'";

					// Check to see if it's time for the big stats update as well
					if (DateTime.Now >= NextStatsUpdateTime)
					{
						// Schedule the next update
						NextStatsUpdateTime = DateTime.Now.AddSeconds(StatsUpdatePeriod);

						// Combine the stats with the time
						Command += (", CPUBusy = " + LocalPerfStats.CPUBusy.NextValue().ToString()
								  + ", DiskReadLatency = " + LocalPerfStats.LogicalDiskReadLatency.NextValue().ToString()
								  + ", DiskWriteLatency = " + LocalPerfStats.LogicalDiskWriteLatency.NextValue().ToString()
								  + ", DiskTransferLatency = " + LocalPerfStats.LogicalDiskTransferLatency.NextValue().ToString()
								  + ", DiskQueueLength = " + LocalPerfStats.LogicalDiskQueueLength.NextValue().ToString()
								  + ", DiskReadQueueLength = " + LocalPerfStats.LogicalDiskReadQueueLength.NextValue().ToString()
								  + ", SystemMemoryAvailable = " + LocalPerfStats.SystemMemoryAvailable.NextValue().ToString());
					}
					Command += " WHERE ( ID = " + ID.ToString() + " )";

					DB.Update( Command );
				}

                LastPingTime = DateTime.Now;
            }
        }

        private int PollForBuild()
        {
            int ID = 0;

            // Check every 5 seconds
            TimeSpan PingTime = new TimeSpan( 0, 0, 5 );
            if( DateTime.Now - LastBuildPollTime > PingTime )
            {
                ID = DB.PollForBuild( MachineName );
                LastBuildPollTime = DateTime.Now;

                // Check for build already running
                string Query = "SELECT Machine FROM [Commands] WHERE ( ID = " + ID.ToString() + " )";
                string Machine = DB.ReadString( Query );
                if( Machine.Length > 0 )
                {
                    string BuildType = DB.GetString( "Commands", ID, "Description" );
                    string Operator = DB.GetString( "Commands", ID, "Operator" );
                    Log( "[STATUS] Suppressing retrigger of '" + BuildType + "'", Color.Magenta );
                    Mailer.SendAlreadyInProgressMail( Operator, ID, BuildType );

                    ID = 0;
                }
            }

            return ( ID );
        }

        private int PollForJob()
        {
            int ID = 0;

            if( MachineLock == 0 )
            {
                // Check every 5 seconds
                TimeSpan PingTime = new TimeSpan( 0, 0, 5 );
                if( DateTime.Now - LastJobPollTime > PingTime )
                {
                    ID = DB.PollForJob( MachineName );
                    LastJobPollTime = DateTime.Now;
                }
            }

            return ( ID );
        }

        private int PollForKillBuild()
        {
            int ID = 0;

            // Check every 2 seconds
            TimeSpan PingTime = new TimeSpan( 0, 0, 2 );
            if( DateTime.Now - LastBuildKillTime > PingTime )
            {
                ID = DB.PollForKillBuild();
                LastBuildKillTime = DateTime.Now;
            }

            return ( ID );
        }

        private int PollForKillJob()
        {
            int ID = 0;

            // Check every 2 seconds
            TimeSpan PingTime = new TimeSpan( 0, 0, 2 );
            if( DateTime.Now - LastJobKillTime > PingTime )
            {
                ID = DB.PollForKillJob();
                LastJobKillTime = DateTime.Now;
            }

            return ( ID );
        }

        private void UnbroadcastMachine()
        {
            Log( "Unregistering '" + MachineName + "' from database", Color.Blue );

            string Command = "SELECT [ID] FROM [Builders] WHERE ( Machine = '" + MachineName + "' AND State != 'Dead' AND State != 'Zombied' )";
            int ID = DB.ReadInt( Command );
            if( ID != 0 )
            {
                Command = "UPDATE Builders SET EndTime = '" + DateTime.Now.ToString( US_TIME_DATE_STRING_FORMAT ) + "' WHERE ( ID = " + ID.ToString() + " )";
                DB.Update( Command ); 

                Command = "UPDATE Builders SET State = 'Dead' WHERE ( ID = " + ID.ToString() + " )";
                DB.Update( Command );
            }
        }

        private void CheckRestart()
        {
            // Check every 5 seconds
            TimeSpan PingTime = new TimeSpan( 0, 0, 5 );
            if( DateTime.Now - LastRestartTime > PingTime )
            {
                string Command = "SELECT [ID] FROM [Builders] WHERE ( Machine = '" + MachineName + "' AND State != 'Dead' AND State != 'Zombied' )";
                int ID = DB.ReadInt( Command );
                if( ID != 0 )
                {
                    Command = "SELECT [Restart] FROM Builders WHERE ( ID = " + ID.ToString() + " AND Restart is not NULL )";
                    if( DB.ReadBool( Command ) )
                    {
                        SCC.SyncBuildScripts( SourceBranch, "/Development/Builder/..." );
                        Restart = true;
                        Ticking = false;
                    }
                }
                else
                {
                    // Error reading from DB, so wait a while for things to settle, then restart
                    System.Threading.Thread.Sleep( 1000 * 60 );

                    Restart = true;
                    Ticking = false;
                }

                LastRestartTime = DateTime.Now;
            }
        }

        protected void CheckSystemDown()
        {
            // Check every 5 minutes
            TimeSpan PingTime = new TimeSpan( 0, 5, 0 );
            if( DateTime.Now - LastSystemDownTime > PingTime )
            {
                string CommandString = "SELECT Value FROM [Variables] WHERE ( Variable = 'StatusMessage' )";
                string Message = DB.ReadString( CommandString );

                if( Message.Length > 0 )
                {
                    // Just exit - don't restart
                    Ticking = false;
                }

                LastSystemDownTime = DateTime.Now;
            }
        }

        private void GetSourceBranch()
        {
            string CWD = Environment.CurrentDirectory;
            int Index = CWD.LastIndexOf( '\\' );
            if( Index >= 0 )
            {
                RootFolder = CWD.Substring( 0, Index );
                SourceBranch = CWD.Substring( Index + 1 );
            }
        }

        public void Init()
        {
            // Show log window
            Show();

            DB = new BuilderDB( this );
            SCC = new P4( this );
            Mailer = new Emailer( this, SCC );
            Watcher = new Watcher( this );

			// Create performance monitoring objects
			LocalPerfStats = new PerformanceStats();
			LocalPerfStats.CPUBusy = new PerformanceCounter("Processor", "% Processor Time", "_Total");
			LocalPerfStats.LogicalDiskReadLatency = new PerformanceCounter("LogicalDisk", "Avg. Disk sec/Read", "_Total");
			LocalPerfStats.LogicalDiskWriteLatency = new PerformanceCounter("LogicalDisk", "Avg. Disk sec/Write", "_Total");
			LocalPerfStats.LogicalDiskTransferLatency = new PerformanceCounter("LogicalDisk", "Avg. Disk sec/Transfer", "_Total");
			LocalPerfStats.LogicalDiskQueueLength = new PerformanceCounter("LogicalDisk", "Avg. Disk Queue Length", "_Total");
			LocalPerfStats.LogicalDiskReadQueueLength = new PerformanceCounter("LogicalDisk", "Avg. Disk Read Queue Length", "_Total");
			LocalPerfStats.SystemMemoryAvailable = new PerformanceCounter("Memory", "Available Bytes");
			
			Application.DoEvents();

            SetupSyncServices();
            GetInfo();
            CalculateCompileDateTime();
            GetMSVCVersion();
            GetDirectXVersion();
            GetAvailablePS3SDKs();
            GetCurrentPS3SDKVersion();
            GetXDKVersion();
            GetSourceBranch();
            GetMachineRating();

            Application.DoEvents();

            // Something went wrong during setup - sleep and retry
            if( !Ticking )
            {
                System.Threading.Thread.Sleep( 30000 );
                Restart = true;
                return;
            }

            // Register with DB
            BroadcastMachine();
            // Inform user of restart
            Mailer.SendRestartedMail();

            // If this machine locked to a build then don't allow it to grab normal ones
            string Query = "SELECT [ID] FROM Commands WHERE ( '" + MachineName + "' LIKE MachineLock )";
            MachineLock = DB.ReadInt( Query );

            Log( "Running from root folder '" + RootFolder + "'", Color.DarkGreen );
        }

        public void Destroy()
        {
            UnbroadcastMachine();

            if( DB != null )
            {
                DB.Destroy();
            }
        }

        public Main()
        {
            InitializeComponent();
        }

        public void Log( string Line, Color TextColour )
        {
            if( Line == null || !Ticking )
            {
                return;
            }

            // if we need to, invoke the delegate
            if( InvokeRequired )
            {
                Invoke( new DelegateAddLine( Log ), new object[] { Line, TextColour } );
                return;
            }

            DateTime Now = DateTime.Now;
            string FullLine = Now.ToLongTimeString() + ": " + Line;

            TextBox_Log.Focus();
            TextBox_Log.SelectionLength = 0;

            // Only set the color if it is different than the foreground colour
            if( TextBox_Log.SelectionColor != TextColour )
            {
                TextBox_Log.SelectionColor = TextColour;
            }

            TextBox_Log.AppendText( FullLine + Environment.NewLine );

            CheckStatusUpdate( Line );
        }

        private void HandleWatchStatus( string Line )
        {
            string KeyName = "";
            int Value = -1;

            if( Line.StartsWith( "[WATCHSTART " ) )
            {
                KeyName = Line.Substring( "[WATCHSTART ".Length ).TrimEnd( "]".ToCharArray() );
                Watcher.WatchStart( KeyName );
                Value = -1;
            }
            else if( Line.StartsWith( "[WATCHSTOP]" ) )
            {
                Value = Watcher.WatchStop( ref KeyName );
            }
            else if( Line.StartsWith( "[WATCHTIME " ) )
            {
                KeyName = "";
                string KeyValue = Line.Substring( "[WATCHTIME ".Length ).TrimEnd( "]".ToCharArray() );

                string[] Split = KeyValue.Split( " \t".ToCharArray() );
                if( Split.Length == 2 )
                {
                    KeyName = Split[0];
                    Value = Builder.SafeStringToInt( Split[1] );
                }
            }

            // Write the perf data to the database
            if( KeyName.Length > 0 && Value >= 0 )
            {
                DB.WritePerformanceData( MachineName, KeyName, Value );
            }
        }

        public void CheckStatusUpdate( string Line )
        {
            if( Line == null || !Ticking || BuildLogID == 0 )
            {
                return;
            }

            // Handle any special controls
            if( Line.StartsWith( "[STATUS] " ) )
            {
                Line = Line.Substring( "[STATUS] ".Length );
                SetStatus( Line.Trim() );
            }
            else if( Line.IndexOf( "------" ) >= 0 )
            {
                Line = Line.Substring( Line.IndexOf( "------" ) + "------".Length );
                Line = Line.Substring( 0, Line.IndexOf( "------" ) );
                SetStatus( Line.Trim() );
            }
            else if( Line.StartsWith( "[WATCH" ) )
            {
                HandleWatchStatus( Line );
            }

            if( Line.IndexOf( "=> NETWORK " ) >= 0 )
            {
                MailGlitch();
            }
        }

        private long GetDirectorySize( string Directory )
        {
            long CurrentSize = 0;

            DirectoryInfo DirInfo = new DirectoryInfo( Directory );
            if( DirInfo.Exists )
            {
                FileInfo[] Infos = DirInfo.GetFiles();
                foreach( FileInfo Info in Infos )
                {
                    if( Info.Exists )
                    {
                        CurrentSize += Info.Length;
                    }
                }

                DirectoryInfo[] SubDirInfo = DirInfo.GetDirectories();
                foreach( DirectoryInfo Info in SubDirInfo )
                {
                    if( Info.Exists )
                    {
                        CurrentSize += GetDirectorySize( Info.FullName );
                    }
                }
            }

            return ( CurrentSize );
        }

        private string GetDirectoryStatus( ScriptParser Builder )
        {
            StringBuilder Status = new StringBuilder();

            try
            {
                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                long Total = 0;

                foreach( string Directory in Parms )
                {
                    long BytesUsed = GetDirectorySize( Directory ) / ( 1024 * 1024 * 1024 );
                    Total += BytesUsed;

                    Status.Append( "'" + Directory + "' consumes " + BytesUsed.ToString() + " GB" + Environment.NewLine );
                }

                float TotalTB = ( float )( Total / 1024.0 );
                Status.Append( Environment.NewLine + "All builds consume " + TotalTB.ToString( "#.00" ) + " TB" + Environment.NewLine );

                DriveInfo DI = new DriveInfo( "R" );
                float FreeSpace = ( float )( DI.AvailableFreeSpace / ( 1024.0 * 1024.0 * 1024.0 * 1024.0 ) );
                Status.Append( Environment.NewLine + "Drive R: has " + FreeSpace.ToString( "#.00" ) + " TB free" );
            }
            catch
            {
                Status = new StringBuilder();
                Status.Append( "ERROR while interrogating directories" );
            }
            return ( Status.ToString() );
        }

        public void MailGlitch()
        {
            if( InvokeRequired )
            {
                Invoke( new DelegateMailGlitch( MailGlitch ), new object[] {} );
                return;
            }

            Mailer.SendGlitchMail();
        }

        public void KillProcess( string ProcessName )
        {
            try
            {
                Process[] ChildProcesses = Process.GetProcessesByName( ProcessName );
                foreach( Process ChildProcess in ChildProcesses )
                {
                    Log( " ... killing: '" + ChildProcess.ProcessName + "'", Color.Red );
                    ChildProcess.Kill();
                }
            }
            catch
            {
                Log( " ... killing failed", Color.Red );
            }
        }

        public void SetStatus( string Line )
        {
            // if we need to, invoke the delegate
            if( InvokeRequired )
            {
                Invoke( new DelegateSetStatus( SetStatus ), new object[] { Line } );
                return;
            }

            if( Line.Length > 127 )
            {
                Line = Line.Substring( 0, 127 );
            }

            DB.SetString( "BuildLog", BuildLogID, "CurrentStatus", Line );
        }

        public void Log( Array Lines, Color TextColour )
        {
            foreach( string Line in Lines )
            {
                Log( Line, TextColour );
            }
        }

        public string ExpandCommandParameter( string Input, string Parameter, string TableEntry )
        {
            while( Input.Contains( Parameter ) )
            {
                Input = Input.Replace( Parameter, DB.GetString( "Commands", CommandID, TableEntry ) );
            }

            return ( Input );
        }

        public string ExpandJobParameter( string Input, string Parameter, string TableEntry )
        {
            while( Input.Contains( Parameter ) )
            {
				Input = Input.Replace(Parameter, DB.GetString( "Jobs", JobID, TableEntry ) );
            }

            return ( Input );
        }

        public string ExpandString( string Input, string Branch )
        {
            // Expand predefined constants
            Input = ExpandCommandParameter( Input, "%DatabaseParameter%", "Parameter" );
            Input = ExpandCommandParameter( Input, "%DatabaseConfig%", "Config" );
            Input = ExpandCommandParameter( Input, "%Language%", "Language" );
            Input = ExpandJobParameter( Input, "%JobParameter%", "Parameter" );
            Input = ExpandJobParameter( Input, "%JobGame%", "Game" );
            Input = ExpandJobParameter( Input, "%JobPlatform%", "Platform" );

            // Expand variables from variables table
            string[] Parms = Input.Split( " \t".ToCharArray() );
            for( int i = 0; i < Parms.Length; i++ )
            {
                if( Parms[i].StartsWith( "#" ) )
                {
                    Parms[i] = DB.GetVariable( Branch, Parms[i].Substring( 1 ) );
                }
            }

            // Reconstruct Command line
            string Line = "";
            foreach( string Parm in Parms )
            {
                Line += Parm + " ";
            }

            return ( Line.Trim() );
        }

        public string EnableMinGW()
        {
            DirectoryInfo MinGW = new DirectoryInfo( GetPS3RootDir() + "\\MinGW_Disabled" );
            if( MinGW.Exists )
            {
                MinGW.MoveTo( GetPS3RootDir() + "\\MinGW" );
            }

            return ( GetPS3RootDir() );
        }

        public void DisableMinGW()
        {
            DirectoryInfo MinGW = new DirectoryInfo( GetPS3RootDir() + "\\MinGW" );
            if( MinGW.Exists )
            {
                MinGW.MoveTo( GetPS3RootDir() + "\\MinGW_Disabled" );
            }
        }

        // Any cleanup that needs to happen when the build fails
        private void FailCleanup()
        {
            if( Builder.NewLabelCreated )
            {
                SCC.DeleteLabel( Builder, Builder.LabelInfo.GetLabelName() );
                Builder.NewLabelCreated = false;
            }
        }

        // Cleanup that needs to happen on success or failure
        private void Cleanup()
        {
            // Revert all open files
            CurrentCommand = new Command( this, SCC, Builder );
            CurrentCommand.Execute( COMMANDS.SCC_Revert );

            // Mark as read only any files marked writable
            CurrentCommand = new Command( this, SCC, Builder );
            CurrentCommand.Execute( COMMANDS.MakeReadOnly );

            // Kill the msdev crash dialog if it exists
            KillProcess( "DW20" );
            // Kill the ProDG compiler if necessary
            KillProcess( "vsimake" );
            // Kill the command prompt
            KillProcess( "cmd" );
            // Kill the autoreporter which mysteriously stays open now
            KillProcess( "Autoreporter" );
            // Kill the the pdb handler
            KillProcess( "mspdbsrv" );

            bool LoopingCommand = DB.GetBool( "Commands", CommandID, "Looping" );
            int LoopingCommandID = CommandID;

            DB.SetString( "Builders", BuilderID, "State", "Connected" );

            if( CommandID != 0 )
            {
                DB.Delete( "Commands", CommandID, "Machine" );
                DB.Delete( "Commands", CommandID, "Killing" );
                DB.Delete( "Commands", CommandID, "BuildLogID" );
                DB.Delete( "Commands", CommandID, "ConchHolder" );
            }
            else if( JobID != 0 )
            {
                DB.SetBool( "Jobs", JobID, "Complete", true );
            }

            // Set the DB up with the result of the build
            DB.SetString( "BuildLog", BuildLogID, "CurrentStatus", FinalStatus );

            // Remove any active watchers
            Watcher = new Watcher( this );

            FinalStatus = "";
            CurrentCommand = null;
            Builder.Destroy();
            Builder = null;
            BlockingBuildID = 0;
            BlockingBuildLogID = 0;
            Promotable = 0;
            JobID = 0;
            NumJobsToWaitFor = 0;
            CommandID = 0;
            BuilderID = 0;
            BuildLogID = 0;

            if( LoopingCommand )
            {
                DB.Trigger( LoopingCommandID );
            }

            TimeSpan OneHour = new TimeSpan( 1, 0, 0 );
            LastSystemDownTime = DateTime.Now - OneHour; 
            LastRestartTime = LastSystemDownTime;
        }

        private MODES HandleComplete()
        {
            Builder.BuildEndedAt = DateTime.Now;
            DB.SetDateTime( "BuildLog", BuildLogID, "BuildEnded", Builder.BuildEndedAt );

            if( CommandID != 0 )
            {
                if( Builder.IsPromoting )
                {
                    // There was a 'tag' command in the script
                    Mailer.SendPromotedMail( Builder, CommandID );
                }
                else if( Builder.IsPublishing )
                {
                    // There was a 'publish' command in the script
                    Mailer.SendPublishedMail( Builder, CommandID, BuildLogID );
                }
                else if( Builder.IsBuilding )
                {
                    // There was a 'submit' command in the script
                    Mailer.SendSucceededMail( Builder, CommandID, BuildLogID, Builder.GetChanges( "" ) );
                }
                else if( Builder.IsMakingInstall )
                {
                    // There was a 'buildinstall' command in the script
                    Mailer.SendMakingInstallMail( Builder, CommandID, BuildLogID );
                }
                else if( Builder.IsSendingQAChanges )
                {
                    string[] DistroList = Builder.SuccessAddress.Split( ";".ToCharArray() );
                    foreach( string Email in DistroList )
                    {
                        Mailer.SendUserChanges( Builder, Email );
                    }
                }

                if( Builder.LabelInfo.Changelist > Builder.LastGoodBuild )
                {
                    DB.SetLastGoodBuild( CommandID, Builder.LabelInfo.Changelist, DateTime.Now );

                    string Label = Builder.LabelInfo.GetLabelName();
                    if( Builder.CreateLabel || Builder.NewLabelCreated )
                    {
                        DB.SetString( "Commands", CommandID, "LastGoodLabel", Label );
                        DB.SetString( "BuildLog", BuildLogID, "BuildLabel", Label );
                    }

                    if( Builder.IsPromoting )
                    {
                        DB.SetString( "Commands", CommandID, "LastGoodLabel", Label );
                    }
                }
            }
            else if( JobID != 0 )
            {
                DB.SetBool( "Jobs", JobID, "Succeeded", true );
            }

            FinalStatus = "Succeeded";
            return ( MODES.Exit );
        }

        public string GetLabelToSync()
        {
            // If we have a valid label - use that
            if( Builder.LabelInfo.RevisionType == RevisionType.Label )
            {
                string Label = Builder.LabelInfo.GetLabelName();
                Log( "Label revision: @" + Label, Color.DarkGreen );
                return ( "@" + Label );
            }
            else
            {
                // ... or there may just be a changelist
                int Changelist = Builder.LabelInfo.Changelist;
                if( Changelist != 0 )
                {
                    Log( "Changelist revision: @" + Changelist.ToString(), Color.DarkGreen );
                    return ( "@" + Changelist.ToString() );
                }
            }

            // No label to sync - sync to head
            Log( "Invalid or nonexistant label, default: #head", Color.DarkGreen );
            return ( "#head" );
        }

        public string GetChangeListToSync()
        {
            if( Builder.LabelInfo.RevisionType == RevisionType.Label )
            {
                string Changelist = Builder.LabelInfo.Changelist.ToString();
                Log( "Changelist revision: @" + Changelist, Color.DarkGreen );
                return ( "@" + Changelist );
            }

            // No label to sync - sync to head
            Log( "No changelist found, default: #head", Color.DarkGreen );
            return ( "#head" );
        }

        private string GetTaggedMessage()
        {
            SCC.RefreshLabelInfo( Builder );

            string FailureMessage = Builder.LabelInfo.Description;
            int Index = Builder.LabelInfo.Description.IndexOf( "Job failed" );
            if( Index > 0 )
            {
                FailureMessage = FailureMessage.Substring( Index );
            }
            return ( FailureMessage );
        }

        public void AddJob( JobInfo Job )
        {
            string CommandString = "INSERT INTO [Jobs] ( Name, Command, Platform, Game, Parameter, Branch, Label, Machine, BuildLogID, Compatible64Bit, Active, Complete, Succeeded, Copied, Killing, SpawnTime ) VALUES ( '";
            CommandString += Job.Name + "','" + Job.Command + "','" + Builder.LabelInfo.Platform + "','";
			CommandString += Builder.LabelInfo.Game + "','" + Job.Parameter + "','" + Builder.SourceControlBranch + "','";
			CommandString += Builder.Dependency + "','',0,1,0,0,0,0,0," + JobSpawnTime.ToString() + " )";
            DB.Update( CommandString );

            NumJobsToWaitFor++;

            Log( "Added job: " + Job.Name, Color.DarkGreen );
        }

        // Kill an in progress build
        private void KillBuild( int ID )
        {
            if( CommandID > 0 && CommandID == ID )
            {
                Log( "[STATUS] Killing build ...", Color.Red );

                // Kill the active command
                if( CurrentCommand != null )
                {
                    CurrentCommand.Kill();
                }

                // Kill all associated jobs
                string CommandString = "UPDATE [Jobs] SET Killing = 1 WHERE ( SpawnTime = " + JobSpawnTime.ToString() + " )";
                DB.Update( CommandString );

                // Clean up
                FailCleanup();
             
                Mode = MODES.Exit;
                Log( "Process killed", Color.Red );

                string Killer = DB.GetString( "Commands", CommandID, "Killer" );
                Mailer.SendKilledMail( Builder, CommandID, BuildLogID, Killer );

                Cleanup();
            }
        }

        private void KillJob( int ID )
        {
            if( JobID > 0 && JobID == ID )
            {
                Log( "[STATUS] Killing job ...", Color.Red );

                if( CurrentCommand != null )
                {
                    CurrentCommand.Kill();
                }

                Mode = MODES.Exit;
                Log( "Process killed", Color.Red );

                Cleanup();
            }
        }

        private void SpawnBuild( int ID )
        {
            string CommandString;

            DeleteDirectory( "Development\\Builder\\Logs", 5 );

            DeleteDirectory( "C:\\Builds", 0 );
            EnsureDirectoryExists( "C:\\Builds" );

            DeleteDirectory( "C:\\Install", 0 );
            EnsureDirectoryExists( "C:\\Install" );

            CommandID = ID;

            DateTime BuildStarted = DateTime.Now;
            string Script = DB.GetString( "Commands", CommandID, "Command" );
            string Description = DB.GetString( "Commands", CommandID, "Description" );
            string SourceControlBranch = DB.GetString( "Commands", CommandID, "Branch" );
            int LastGoodBuild = DB.GetInt( "Commands", CommandID, "LastGoodChangeList" );

            Environment.CurrentDirectory = RootFolder + "\\" + SourceControlBranch;
            EnsureDirectoryExists( "Development\\Builder\\Logs" );

            // Make sure we have the latest build scripts
            SCC.SyncBuildScripts( SourceControlBranch, "/Development/Builder/..." );

            // Make sure there are no pending kill commands
            DB.Delete( "Commands", CommandID, "Killing" );
            // Clear out killer (if any)
            DB.SetString( "Commands", CommandID, "Killer", "" );

            string Operator = DB.GetString( "Commands", CommandID, "Operator" );

            Builder = new ScriptParser( this, Script, Description, SourceControlBranch, LastGoodBuild, BuildStarted, Operator );

            CommandString = "SELECT [ID] FROM [Builders] WHERE ( Machine = '" + MachineName + "' AND State != 'Dead' AND State != 'Zombied' )";
            BuilderID = DB.ReadInt( CommandString );

            // Set builder to building
            DB.SetString( "Builders", BuilderID, "State", "Building" );

            // Add a new entry with the command
            CommandString = "INSERT INTO [BuildLog] ( BuildStarted ) VALUES ( '" + BuildStarted.ToString() + "' )";
            DB.Update( CommandString );

            ProcedureParameter ParmTable = new ProcedureParameter( "TableName", "BuildLog", SqlDbType.Text, "BuildLog".Length );
            BuildLogID = DB.ReadInt( "GetNewID", ParmTable, null, null );

            if( BuildLogID != 0 )
            {
                CommandString = "UPDATE [BuildLog] SET CurrentStatus = 'Spawning', Machine = '" + MachineName + "', Command = '" + Script + "' WHERE ( ID = " + BuildLogID.ToString() + " )";
                DB.Update( CommandString );

                CommandString = "UPDATE [Commands] SET BuildLogID = " + BuildLogID.ToString() + ", Machine = '" + MachineName + "' WHERE ( ID = " + CommandID.ToString() + " )";
                DB.Update( CommandString );

                CommandString = "SELECT Promotable FROM [Commands] WHERE ( ID = " + CommandID.ToString() + " )";
                Promotable = DB.ReadInt( CommandString );
                DB.SetInt( "BuildLog", BuildLogID, "Promotable", Promotable );
            }

            SCC.Init();
            PendingCommands.Clear();
            JobSpawnTime = DateTime.UtcNow.Ticks;

            Mode = MODES.Init;
        }

        private void SpawnJob( int ID )
        {
            string CommandString;

            DeleteDirectory( "C:\\Builds", 0 );
            EnsureDirectoryExists( "C:\\Builds" );

            DeleteDirectory( "C:\\Install", 0 );
            EnsureDirectoryExists( "C:\\Install" );

            JobID = ID;

            DateTime BuildStarted = DateTime.Now;
            string JobName = DB.GetString( "Jobs", JobID, "Name" );
            string Script = DB.GetString( "Jobs", JobID, "Command" );
            string SourceControlBranch = DB.GetString( "Jobs", JobID, "Branch" );

            Environment.CurrentDirectory = RootFolder + "\\" + SourceControlBranch;
            EnsureDirectoryExists( "Development\\Builder\\Logs" );

            // Make sure we have the latest build scripts
            SCC.SyncBuildScripts( SourceControlBranch, "/Development/Builder/..." );

            string Operator = DB.GetString( "Commands", CommandID, "Operator" );

            Builder = new ScriptParser( this, Script, JobName, SourceControlBranch, 0, BuildStarted, Operator );

            CommandString = "SELECT [ID] FROM [Builders] WHERE ( Machine = '" + MachineName + "' AND State != 'Dead' AND State != 'Zombied' )";
            BuilderID = DB.ReadInt( CommandString );

            // Set builder to building
            DB.SetString( "Builders", BuilderID, "State", "Building" );

            // Add a new entry with the command
            CommandString = "INSERT INTO [BuildLog] ( BuildStarted ) VALUES ( '" + BuildStarted.ToString() + "' )";
            DB.Update( CommandString );

            ProcedureParameter ParmTable = new ProcedureParameter( "TableName", "BuildLog", SqlDbType.Text, "BuildLog".Length );
            BuildLogID = DB.ReadInt( "GetNewID", ParmTable, null, null );

            if( BuildLogID != 0 )
            {
                CommandString = "UPDATE [BuildLog] SET CurrentStatus = 'Spawning', Machine = '" + MachineName + "', Command = '" + Script + "' WHERE ( ID = " + BuildLogID.ToString() + " )";
                DB.Update( CommandString );

                CommandString = "UPDATE [Jobs] SET BuildLogID = " + BuildLogID.ToString() + ", Machine = '" + MachineName + "' WHERE ( ID = " + JobID.ToString() + " )";
                DB.Update( CommandString );
            }

            // Grab game and platform
            CommandString = "SELECT [Game] FROM [Jobs] WHERE ( ID = " + JobID.ToString() + " )";
            Builder.LabelInfo.Game = DB.ReadString( CommandString );

			CommandString = "SELECT [Platform] FROM [Jobs] WHERE ( ID = " + JobID.ToString() + " )";
			Builder.LabelInfo.Platform = DB.ReadString( CommandString );

			Builder.Dependency = DB.GetString( "Jobs", JobID, "Label" );
            Builder.LabelInfo.Init( SCC, Builder );

            SCC.Init();
            PendingCommands.Clear();

            Mode = MODES.Init;
        }

        private MODES HandleError()
        {
            int TimeOutMinutes;
            bool CheckCookingSuccess = ( Builder.GetCommandID() == COMMANDS.CookMaps );
            bool CheckCookerSyncSuccess = ( Builder.GetCommandID() == COMMANDS.Publish );
            string GeneralStatus = "";
            string Status = "Succeeded";

            // Internal error?
            ERRORS ErrorLevel = Builder.GetErrorLevel();

            if( CurrentCommand != null && ErrorLevel == ERRORS.None )
            {
                // ...or error that requires parsing the log
                ErrorLevel = CurrentCommand.GetErrorLevel();

                LogParser Parser = new LogParser( Builder );
                bool ReportEverything = ( ErrorLevel >= ERRORS.SCC_Sync && ErrorLevel <= ERRORS.SCC_GetClientRoot );
                Status = Parser.Parse( ReportEverything, CheckCookingSuccess, CheckCookerSyncSuccess, ref ErrorLevel );
            }
#if !DEBUG
            // If we were cooking, and didn't find the cooking success message, set to fail
            if( CheckCookingSuccess )
            {
                if( ErrorLevel == ERRORS.CookingSuccess )
                {
                    ErrorLevel = ERRORS.None;
                }
                else if( ErrorLevel == ERRORS.None && Status == "Succeeded" )
                {
                    ErrorLevel = ERRORS.CookMaps;
                    Status = "Could not find cooking successful message";
                }
            }
#endif

            // If we were publishing, and didn't find the publish success message, set to fail
            if( CheckCookerSyncSuccess )
            {
                // Free up the conch so other builders can publish
                DB.Delete( "Commands", CommandID, "ConchHolder" );

                if( ErrorLevel == ERRORS.CookerSyncSuccess )
                {
                    ErrorLevel = ERRORS.None;
                }
                else if( ErrorLevel == ERRORS.None && Status == "Succeeded" )
                {
                    ErrorLevel = ERRORS.Publish;
                    Status = "Could not find publish completion message";
                }
            }

            // Check for total success
            if( ErrorLevel == ERRORS.None && Status == "Succeeded" )
            {
                return ( MODES.Init );
            }

            // If we were checking to see if a file was signed, conditionally add another command
            if( ErrorLevel == ERRORS.CheckSigned )
            {
                // Error checking to see if file was signed - so sign it
                if( CurrentCommand.GetCurrentBuild().GetExitCode() != 0 )
                {
                    PendingCommands.Enqueue( COMMANDS.Sign );
                }
                return ( MODES.Init );
            }

            // Copy the failure log to a common network location
            FileInfo LogFile = new FileInfo( "None" );
            if( Builder.LogFileName.Length > 0 )
            {
                LogFile = new FileInfo( Builder.LogFileName );
                try
                {
                    if( LogFile.Exists )
                    {
                        LogFile.CopyTo( Properties.Settings.Default.FailedLogLocation + "/" + LogFile.Name );
                    }
                }
                catch
                {
                }
            }

            if( Status == "Succeeded" )
            {
                Status = "Could not find error";
            }

            // Set the end time as we've now finished
            Builder.BuildEndedAt = DateTime.Now;
            DB.SetDateTime( "BuildLog", BuildLogID, "BuildEnded", Builder.BuildEndedAt );

            // Handle specific errors
            switch( ErrorLevel )
            {
                case ERRORS.None:
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );

                    Log( "LOG ERROR: " + Builder.GetCommandID() + " " + Builder.CommandLine + " failed", Color.Red );
                    Log( Status, Color.Red );
                    break;

                case ERRORS.NoScript:
                    Status = "No build script";
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;

                case ERRORS.IllegalCommand:
                    Status = "Illegal command: '" + Builder.CommandLine + "'";
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;

                case ERRORS.SCC_Submit:
                    GeneralStatus = Builder.GetCommandID() + " " + Builder.CommandLine + " failed with error '" + ErrorLevel.ToString() + "'";
                    GeneralStatus += Environment.NewLine + Environment.NewLine + Status;

                    GeneralStatus += SCC.GetIncorrectCheckedOutFiles();

                    Log( "ERROR: " + GeneralStatus, Color.Red );
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, GeneralStatus, LogFile.Name, "" );
                    break;

                case ERRORS.SCC_Sync:
                case ERRORS.SCC_Checkout:
                case ERRORS.SCC_Revert:
                case ERRORS.SCC_Tag:
                case ERRORS.SCC_GetClientRoot:
                case ERRORS.MakeWritable:
                case ERRORS.MakeReadOnly:
                case ERRORS.UpdateSymbolServer:
                    GeneralStatus = Builder.GetCommandID() + " " + Builder.CommandLine + " failed with error '" + ErrorLevel.ToString() + "'";
                    GeneralStatus += Environment.NewLine + Environment.NewLine + Status;
                    Log( "ERROR: " + GeneralStatus, Color.Red );
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, GeneralStatus, LogFile.Name, "" );
                    break;
                
                case ERRORS.Process:
                case ERRORS.MSVCClean:
                case ERRORS.MSVCBuild:
                case ERRORS.GCCBuild:
                case ERRORS.GCCClean:
                case ERRORS.ShaderBuild:
                case ERRORS.ShaderClean:
                case ERRORS.BuildScript:
                case ERRORS.CookMaps:
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;
                
                case ERRORS.Publish:
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, Status, LogFile.Name, "IT@epicgames.com" );
                    break;

                case ERRORS.CheckSigned:
                    return ( MODES.Init );

                case ERRORS.TimedOut:
                    TimeOutMinutes = ( int )Builder.GetTimeout().TotalMinutes;
                    GeneralStatus = "'" + Builder.GetCommandID() + " " + Builder.CommandLine + "' TIMED OUT after " + TimeOutMinutes.ToString() + " minutes";
                    GeneralStatus += Environment.NewLine;
                    Log( "ERROR: " + GeneralStatus, Color.Red );
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, GeneralStatus, LogFile.Name, "" );
                    break;

                case ERRORS.WaitTimedOut:
                    TimeOutMinutes = ( int )Builder.GetTimeout().TotalMinutes;
                    Status = "Waiting for '" + Builder.CommandLine + "' TIMED OUT after " + TimeOutMinutes.ToString() + " minutes";
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;

                case ERRORS.FailedJobs:
                    Status = "All jobs completed, but one or more failed." + Environment.NewLine + GetTaggedMessage();
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;

                case ERRORS.Crashed:
                    int NotRespondingMinutes = ( int )Builder.GetRespondingTimeout().TotalMinutes;
                    Status = "'" + Builder.GetCommandID() + " " + Builder.CommandLine + "' was not responding for " + NotRespondingMinutes.ToString() + " minutes; presumed crashed.";
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;

                case ERRORS.TheWorldIsEnding:
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, Status, LogFile.Name, "" ); 
                    break;

                default:
                    Status = "'" + Builder.GetCommandID() + " " + Builder.CommandLine + "' unhandled error '" + ErrorLevel.ToString() + "'";
                    Log( "ERROR: " + Status, Color.Red );
                    Mailer.SendFailedMail( Builder, CommandID, BuildLogID, Status, LogFile.Name, "" );
                    break;
            }

            // Any cleanup that needs to happen only in a failure case
            FailCleanup();

            FinalStatus = "Failed";
            return ( MODES.Exit );
        }

        private void RunBuild()
        {
            switch( Mode )
            {
                case MODES.Init:
                    COMMANDS NextCommand;

                    CurrentCommand = null;
                    
                    // Get a new command ...
                    if( PendingCommands.Count > 0 )
                    {
                        // ... either pending 
                        NextCommand = PendingCommands.Dequeue();
                        Builder.SetCommandID( NextCommand );
                    }
                    else
                    {
                        // ... or from the script
                        NextCommand = Builder.ParseNextLine();
                    }

                    // Expand out any variables
                    Builder.CommandLine = ExpandString( Builder.CommandLine, Builder.SourceControlBranch ); ;

                    switch( NextCommand )
                    {
                        case COMMANDS.Error:
                            Mode = MODES.Finalise;
                            break;

                        case COMMANDS.Finished:
                            Mode = HandleComplete();
                            break;

                        case COMMANDS.Config:
                            break;

                        case COMMANDS.TriggerMail:
                            Mailer.SendTriggeredMail( Builder, CommandID );
                            Mode = MODES.Finalise;
                            break;

                        case COMMANDS.Wait:
                            string Query = "SELECT [ID] FROM [Commands] WHERE ( Description = '" + Builder.CommandLine + "' )";
                            BlockingBuildID = DB.ReadInt( Query );
                            BlockingBuildStartTime = DateTime.Now;
                            Mode = MODES.Wait;
                            break;

						case COMMANDS.WaitForJobs:
							Log( "[STATUS] Waiting for " + ( NumJobsToWaitFor - NumCompletedJobs ).ToString() + " jobs.", Color.Magenta );
							Mode = MODES.WaitForJobs;
							break;

                        case COMMANDS.SetDependency:
                            string CommandQuery = "SELECT [ID] FROM [Commands] WHERE ( Description = '" + Builder.Dependency + "' )";
                            int DependentCommandID = DB.ReadInt( CommandQuery );
                            if( DependentCommandID == 0 )
                            {
                                Builder.LabelInfo.Init( SCC, Builder );
                            }
                            else
                            {
                                Builder.LabelInfo.Init( Builder, DB.GetInt( "Commands", DependentCommandID, "LastGoodChangeList" ) );
                            }
                            Mode = MODES.Finalise;
                            break;

                        case COMMANDS.MSVCFull:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.MSVCClean );

                            PendingCommands.Enqueue( COMMANDS.MSVCBuild );
                            break;

                        case COMMANDS.GCCFull:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.GCCClean );

                            PendingCommands.Enqueue( COMMANDS.GCCBuild );
                            break;

                        case COMMANDS.ShaderFull:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.ShaderClean );

                            PendingCommands.Enqueue( COMMANDS.ShaderBuild );
                            break;

                        case COMMANDS.UpdateSourceServer:
                            if( Builder.LabelInfo.Platform.ToLower() != "ps3" )
                            {
                                CurrentCommand = new Command( this, SCC, Builder );
                                Mode = CurrentCommand.Execute( COMMANDS.UpdateSourceServer );

                                PendingCommands.Enqueue( COMMANDS.UpdateSymbolServer );
                            }
                            else
                            {
                                Log( "Suppressing UpdateSourceServer for PS3 ", Color.DarkGreen );
                                Mode = MODES.Finalise;
                            }
                            break;

                        case COMMANDS.UpdateSymbolServer:
                            if( Builder.LabelInfo.Platform.ToLower() != "ps3" )
                            {
                                CurrentCommand = new Command( this, SCC, Builder );
                                Mode = CurrentCommand.Execute( COMMANDS.UpdateSymbolServer );

                                PendingCommands.Enqueue( COMMANDS.UpdateSymbolServerTick );
                            }
                            else
                            {
                                Log( "Suppressing UpdateSymbolServer for PS3 ", Color.DarkGreen );
                                Mode = MODES.Finalise;
                            } 
                            break;

                        case COMMANDS.UpdateSymbolServerTick:
                            if( Builder.LabelInfo.Platform.ToLower() != "ps3" )
                            {
                                CurrentCommand = new Command( this, SCC, Builder );
                                Mode = CurrentCommand.Execute( COMMANDS.UpdateSymbolServerTick );

                                if( CurrentCommand.GetCurrentBuild() != null )
                                {
                                    PendingCommands.Enqueue( COMMANDS.UpdateSymbolServerTick );
                                }
                            }
                            else
                            {
                                Log( "Suppressing UpdateSymbolServerTick for PS3 ", Color.DarkGreen );
                                Mode = MODES.Finalise;
                            } 
                            break;

                        case COMMANDS.Trigger:
                            string BuildType = ExpandString( Builder.CommandLine, Builder.SourceControlBranch );
                            Log( "[STATUS] Triggering build '" + BuildType + "'", Color.Magenta );
                            if( !DB.Trigger( CommandID, BuildType ) )
                            {
                                Log( "[STATUS] Suppressing retrigger of '" + BuildType + "'", Color.Magenta );
                                string Operator = DB.GetString( "Commands", CommandID, "Operator" );
                                Mailer.SendAlreadyInProgressMail( Operator, CommandID, BuildType );
                            }
                            break;

                        case COMMANDS.Conform:
                            if( Builder.GetValidLanguages().Count > 2 )
                            {
                                PendingCommands.Enqueue( COMMANDS.Conform );
                            }

                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( COMMANDS.Conform );
                            break;

                        case COMMANDS.SCC_Submit:
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( NextCommand );

                            if( Builder.CreateLabel && SCC.GetErrorLevel() == ERRORS.None )
                            {
                                string CommandString;

                                string Label = Builder.LabelInfo.GetLabelName();
                                if( Promotable > 0 )
                                {
                                    CommandString = "UPDATE Variables SET Value = '" + Label + "' WHERE ( Variable = 'LatestBuild' AND Branch = '" + Builder.SourceControlBranch + "' )";
                                    DB.Update( CommandString );
                                }

                                CommandString = "UPDATE Variables SET Value = '" + Label + "' WHERE ( Variable = 'LatestRawBuild' AND Branch = '" + Builder.SourceControlBranch + "' )";
                                DB.Update( CommandString );
                            }
                            break;

                        case COMMANDS.CheckSpace:
                            try
                            {
                                string FinalStatus = GetDirectoryStatus( Builder );

                                Mailer.SendStatusMail( Builder, CommandID, FinalStatus );
                            }
                            catch
                            {
                            }
                            break;

                        case COMMANDS.UpdateLabel:
                            string LabelCmdLine = ExpandString( Builder.CommandLine, Builder.SourceControlBranch );
                            string UpdateLabel = "UPDATE Variables SET Value = '" + Builder.LabelInfo.GetLabelName() + "' WHERE ( Variable = '" + LabelCmdLine + "' AND Branch = '" + Builder.SourceControlBranch + "' )";
                            DB.Update( UpdateLabel );
                            break;

                        case COMMANDS.UpdateFolder:
                            string FolderCmdLine = ExpandString( Builder.CommandLine, Builder.SourceControlBranch );
                            string UpdateFolder = "UPDATE Variables SET Value = '" + Builder.LabelInfo.GetFolderName( Builder.AppendLanguage ) + "' WHERE ( Variable = '" + FolderCmdLine + "' AND Branch = '" + Builder.SourceControlBranch + "' )";
                            DB.Update( UpdateFolder );
                            break;
                        
                        case COMMANDS.Publish:
                            if( Builder.BlockOnPublish )
                            {
                                if( !DB.AvailableBandwidth( CommandID ) )
                                {
                                    PendingCommands.Enqueue( COMMANDS.Publish );

                                    if( Builder.StartWaitForConch == Builder.BuildStartedAt )
                                    {
                                        Builder.StartWaitForConch = DateTime.Now;
                                        Builder.LastWaitForConchUpdate = Builder.StartWaitForConch;

                                        Log( "[STATUS] Waiting for network bandwidth ( 00:00:00 )", Color.Yellow );
                                    }
                                    else if( DateTime.Now - Builder.LastWaitForConchUpdate > new TimeSpan( 0, 0, 5 ) )
                                    {
                                        Builder.LastWaitForConchUpdate = DateTime.Now;
                                        TimeSpan Taken = DateTime.Now - Builder.StartWaitForConch;
                                        Log( "[STATUS] Waiting for network bandwidth ( " + Taken.Hours.ToString( "00" ) + ":" + Taken.Minutes.ToString( "00" ) + ":" + Taken.Seconds.ToString( "00" ) + " )", Color.Yellow );
                                    }
                                }
                                else
                                {
                                    Log( "[STATUS] Network bandwidth acquired - publishing!", Color.Magenta );

                                    CurrentCommand = new Command( this, SCC, Builder );
                                    Mode = CurrentCommand.Execute( NextCommand );
                                }
                            }
                            else
                            {
                                CurrentCommand = new Command( this, SCC, Builder );
                                Mode = CurrentCommand.Execute( NextCommand );
                            }
                            break;

                        case COMMANDS.RedFlash:
                            Mailer.RedFlash( Builder );
                            break;

                        default:
                            // Fire off a new process safely
                            CurrentCommand = new Command( this, SCC, Builder );
                            Mode = CurrentCommand.Execute( NextCommand );

                            // Handle the returned info (special case)
                            if( NextCommand == COMMANDS.SCC_Sync )
                            {
                                DB.SetInt( "BuildLog", BuildLogID, "ChangeList", Builder.LabelInfo.Changelist );
                            }
                            break;
                    }
                    break;

                case MODES.Monitor:
                    // Check for completion
                    Mode = CurrentCommand.IsFinished();
                    break;

                case MODES.Wait:
                    if( BlockingBuildID != 0 )
                    {
                        // Has the child build been updated to the same build?
                        int LastGoodChangeList = DB.GetInt( "Commands", BlockingBuildID, "LastGoodChangeList" );
                        if( LastGoodChangeList >= Builder.LabelInfo.Changelist )
                        {
                            Mode = MODES.Finalise;
                        }
                        else
                        {
                            // Try to get the build log
                            if( BlockingBuildLogID == 0 )
                            {
                                BlockingBuildLogID = DB.GetInt( "Commands", BlockingBuildID, "BuildLogID" );
                            }

                            // Try and get when the build started
                            if( BlockingBuildLogID != 0 )
                            {
                                BlockingBuildStartTime = DB.GetDateTime( "BuildLog", BlockingBuildLogID, "BuildStarted" );
                            }

                            // Check to see if the build timed out (default time is when wait was started)
                            if( DateTime.Now - BlockingBuildStartTime > Builder.GetTimeout() )
                            {
                                Builder.SetErrorLevel( ERRORS.WaitTimedOut );
                                Mode = MODES.Finalise;
                            }
                        }
                    }
                    else
                    {
                        Mode = MODES.Finalise;
                    }
                    break;

                case MODES.WaitForJobs:
                    // Check every 1 seconds
                    TimeSpan WaitForJobsTime = new TimeSpan( 0, 0, 1 );
                    if( DateTime.Now - LastCheckForJobTime > WaitForJobsTime )
                    {
                        string WaitQuery = "SELECT COUNT( ID ) FROM Jobs WHERE ( SpawnTime = " + JobSpawnTime.ToString() + " AND Complete = 1 )";
                        int Count = DB.ReadInt( WaitQuery );

                        if( NumCompletedJobs != Count )
                        {
                            int RemainingJobs = NumJobsToWaitFor - Count;
                            Log( "[STATUS] Waiting for " + RemainingJobs.ToString() + " package jobs.", Color.Magenta );
                            NumCompletedJobs = Count;
                        }

                        if( Count == NumJobsToWaitFor )
                        {
                            WaitQuery = "SELECT COUNT( ID ) FROM Jobs WHERE ( SpawnTime = " + JobSpawnTime.ToString() + " AND Succeeded = 1 )";
                            Count = DB.ReadInt( WaitQuery );
                            if( Count != NumJobsToWaitFor )
                            {
                                Builder.SetErrorLevel( ERRORS.FailedJobs );
                            }

                            NumJobsToWaitFor = 0;
                            JobSpawnTime = DateTime.UtcNow.Ticks;
                            Mode = MODES.Finalise;
                        }
                    }
                    break;

                case MODES.Finalise:
                    if( CurrentCommand != null )
                    {
                        switch( CurrentCommand.LastExecutedCommand )
                        {
                            case COMMANDS.PS3MakePatch:
                                DisableMinGW();
                                break;
                        }
                    }
				
					// Analyse logs and restart or exit
                    Mode = HandleError();
                    break;

                case MODES.Exit:
                    Cleanup();
                    break;
            }
        }

        public void Run()
        {
            if( DB == null )
            {
                return;
            }

            // Ping the server to say we're still alive every 30 seconds
            MaintainMachine();

            if( BuildLogID != 0 )
            {
                RunBuild();

                int ID = PollForKillBuild();
                if( ID != 0 )
                {
                    KillBuild( ID );
                }

                ID = PollForKillJob();
                if( ID != 0 )
                {
                    KillJob( ID );
                }
            }
            else
            {
                // Check for restarts
                CheckRestart();
                CheckSystemDown();

                if( Ticking )
                {
                    // Poll the DB for commands
                    int ID = PollForBuild();
                    if( ID != 0 )
                    {
                        SpawnBuild( ID );
                        return;
                    }

                    ID = PollForJob();
                    if( ID != 0 )
                    {
                        SpawnJob( ID );
                        return;
                    }
                }
            }
        }

        private void Main_FormClosed( object sender, FormClosedEventArgs e )
        {
            DB.SetString( "Commands", CommandID, "Killer", "LocalUser" );

            KillBuild( CommandID );
            Ticking = false;
        }
    }
}