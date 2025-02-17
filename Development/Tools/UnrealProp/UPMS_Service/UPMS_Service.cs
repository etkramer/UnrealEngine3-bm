using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Channels.Tcp;
using System.Runtime.InteropServices;
using System.ServiceProcess;
using System.Text;
using System.Threading;
using RemotableType;

namespace UnrealProp
{
    public partial class UPMS_Service : ServiceBase
    {
        // For consistent formatting to the US standard (05/29/2008 11:33:52)
        public const string US_TIME_DATE_STRING_FORMAT = "MM/dd/yyyy HH:mm:ss";
        
        public static IUPMS_Interface IUPMS = null;
        public static List<string> UPDSList = new List<string>();

        private static DateTime StartTime;
        private static DateTime LastStatusTime;

        public static int NewBuildsTotal;

        public UPMS_Service()
        {
            InitializeComponent();
        }

        public void DebugRun( string[] Args )
        {
            OnStart( Args );
            Console.WriteLine( "Press enter to exit" );
            Console.Read();
            OnStop();
        }

        public void NewBuild()
        {
            NewBuildsTotal++;
        }

        private void SendStartEmail()
        {
            string Message = "Started at: " + StartTime.ToString( US_TIME_DATE_STRING_FORMAT ) + Environment.NewLine;
            long MasterCacheSize = IUPMS.PlatformBuildFiles_GetTotalSize() / ( 1024 * 1024 * 1024 );
            Message += "Master cache size is: " + MasterCacheSize.ToString() + " GB" + Environment.NewLine;
            Message += Environment.NewLine + "Cheers" + Environment.NewLine + "UnrealProp" + Environment.NewLine;
            IUPMS.Utils_SendEmail( -1, -1, "UPMS Started", Message, 1 );
        }

        public void SendStatusEmail()
        {
            string Message;

            if( DateTime.Now - LastStatusTime > new TimeSpan( 3, 0, 0 ) )
            {
                TimeSpan Span = DateTime.Now - StartTime;
                Message = "Uptime:               " + Span.Days.ToString() + " days and " + Span.Hours + " hours." + Environment.NewLine;
                long MasterCacheSize = IUPMS.PlatformBuildFiles_GetTotalSize() / ( 1024 * 1024 * 1024 );
                Message += "Master cache size is: " + MasterCacheSize.ToString() + " GB" + Environment.NewLine;
                Message += "New builds:           " + NewBuildsTotal.ToString() + Environment.NewLine;
                Message += Environment.NewLine + "Cheers" + Environment.NewLine + "UnrealProp" + Environment.NewLine;
                IUPMS.Utils_SendEmail( -1, -1, "UPMS Status", Message, 1 );

                NewBuildsTotal = 0;
                LastStatusTime = DateTime.Now;
            }
        }

        protected override void OnStart( string[] args )
        {
            // Configure the remoting service
            TcpChannel ServerChannel = new TcpChannel( 9090 );
            ChannelServices.RegisterChannel( ServerChannel, true );

            RemotingConfiguration.RegisterWellKnownServiceType( typeof( UPMS_Implementation ), "UPMS", WellKnownObjectMode.Singleton );

#if DEBUG
            IUPMS = ( IUPMS_Interface )Activator.GetObject( typeof( IUPMS_Interface ), "tcp://localhost:9090/UPMS" );
#else
            IUPMS = ( IUPMS_Interface )Activator.GetObject( typeof( IUPMS_Interface ), Properties.Settings.Default.RemotableHost );
#endif
            
            // Grab the location of the builds from the config
            string BuildRepositoryPath = Properties.Settings.Default.BuildRepositoryPath;
            char[] chars = { '\\', '/' };
            BuildRepositoryPath = BuildRepositoryPath.TrimEnd( chars ) + "\\";

            // add support for many repositories separated ';'
            BuildDiscoverer.Init( this, BuildRepositoryPath );

            // Initialise and start the worker threads
#if !DEBUG
            BuildAnalyzer.Init();
            BuildDeleter.Init();
#endif
            TaskScheduler.Init();

            StartTime = DateTime.Now;
            LastStatusTime = StartTime;
            SendStartEmail();

            Log.WriteLine( "UPMS", Log.LogType.Important, "Service has been successfully started!" );
        }

        protected override void OnStop()
        {
            BuildDiscoverer.Stop();
            BuildAnalyzer.Release();
            TaskScheduler.Release();

            Log.WriteLine( "UPMS", Log.LogType.Important, "Service has been stopped!" );
        }
    }

    public class UPMS_Implementation : MarshalByRefObject, IUPMS_Interface
    {
        // platforms
        public Platforms Platform_GetList()
        {
            return DataHelper.Platform_GetList();
        }

        public Platforms Platform_GetListForProject( short ProjectID )
        {
            return DataHelper.Platform_GetListForProject( ProjectID );
        }

        // projects
        public Projects Project_GetList()
        {
            return DataHelper.Project_GetList();
        }

        // platform build
        public PlatformBuilds PlatformBuild_GetListForProject( short ProjectID )
        {
            return DataHelper.PlatformBuild_GetListForProject( ProjectID );
        }

        public void PlatformBuild_Delete( long PlatformBuildID )
        {
            DataHelper.PlatformBuild_Delete( PlatformBuildID );
        }

        public PlatformBuilds PlatformBuild_GetListForBuild( long PlatformBuildID )
        {
            return DataHelper.PlatformBuild_GetListForBuild( PlatformBuildID );
        }

        public PlatformBuilds PlatformBuild_GetListForProjectPlatformAndStatus( short ProjectID, short PlatformID, short StatusID )
        {
            return DataHelper.PlatformBuild_GetListForProjectPlatformAndStatus( ProjectID, PlatformID, StatusID );
        }

        public PlatformBuilds PlatformBuild_GetListForProjectPlatformUserAndStatus( short ProjectID, short PlatformID, int UserNameID, short StatusID )
        {
            return DataHelper.PlatformBuild_GetListForProjectPlatformUserAndStatus( ProjectID, PlatformID, UserNameID, StatusID );
        }

        public PlatformBuildFiles PlatformBuild_GetFiles( long PlatformBuildID )
        {
            return DataHelper.PlatformBuild_GetFiles( PlatformBuildID );
        }

        public int PlatformBuild_GetCount()
        {
            return DataHelper.PlatformBuild_GetCount();
        }

        public long PlatformBuildFiles_GetTotalSize()
        {
            return DataHelper.PlatformBuildFiles_GetTotalSize();
        }

        public CachedFileInfo CachedFileInfo_GetNewestFiles( int Percent )
        {
            return DataHelper.CachedFileInfo_GetNewestFiles( Percent );
        }

        public void PlatformBuildFiles_Update( PlatformBuildFiles Files )
        {
            DataHelper.PlatformBuildFiles_Update( Files );
        }

        public long PlatformBuild_GetBuildSize( long PlatformBuildID )
        {
            return DataHelper.PlatformBuild_GetBuildSize( PlatformBuildID );
        }

        public string PlatformBuild_GetTitle( long PlatformBuildID )
        {
            return DataHelper.PlatformBuild_GetTitle( PlatformBuildID );
        }

        public string PlatformBuild_GetPlatformName( long PlatformBuildID )
        {
            return DataHelper.PlatformBuild_GetPlatformName( PlatformBuildID );
        }

        public string PlatformBuild_GetRepositoryPath( long PlatformBuildID )
        {
            return DataHelper.PlatformBuild_GetRepositoryPath( PlatformBuildID );
        }

        public void PlatformBuild_ChangeStatus( long PlatformBuildID, BuildStatus StatusID, string BuildName )
        {
            DataHelper.PlatformBuild_ChangeStatus( PlatformBuildID, StatusID, BuildName );
        }

        public void PlatformBuild_ChangeTime( long PlatformBuildID, DateTime TimeStamp )
        {
            DataHelper.PlatformBuild_ChangeTime( PlatformBuildID, TimeStamp );
        }

        public int PlatformBuild_GetAnalyzingProgress( long PlatformBuildID )
        {
            return BuildAnalyzer.PlatformBuild_GetAnalyzingProgress( PlatformBuildID );
        }

        public bool CachedFileInfo_FileExists( string Hash )
        {
            return DataHelper.CachedFileInfo_FileExists( Hash );
        }

        public string PlatformBuild_GetProject( long PlatformBuildID )
        {
            return DataHelper.PlatformBuild_GetProject( PlatformBuildID );
        }

        public DescriptionWithID[] PlatformBuild_GetSimpleListForProjectAndPlatform( short ProjectID, short PlatformID )
        {
            return DataHelper.PlatformBuild_GetSimpleListForProjectAndPlatform( ProjectID, PlatformID );
        }

        public PlatformBuildStatuses PlatformBuildStatus_GetList()
        {
            return DataHelper.PlatformBuildStatus_GetList();
        }

        // clients
        public ClientMachines ClientMachine_GetListForPlatform( short PlatformID )
        {
            return( DataHelper.ClientMachine_GetListForPlatform( PlatformID ) );
        }

        public ClientMachines ClientMachine_GetListForPlatformAndUser( short PlatformID, int UserNameID )
        {
            return ( DataHelper.ClientMachine_GetListForPlatformAndUser( PlatformID, UserNameID ) );
        }

        public ClientMachines ClientMachine_GetListForPlatformGroupUser( string Platform, string Group, string UserName )
        {
            return( DataHelper.ClientMachine_GetListForPlatformGroupUser( Platform, Group, UserName ) );
        }

        public long ClientMachine_Update( int ClientMachineID, string Platform, string Name, string Path, string ClientGroup, string User, string Email, bool Reboot )
        {
            return DataHelper.ClientMachine_Update( ClientMachineID, Platform, Name, Path, ClientGroup, User, Email, Reboot );
        }

        public void ClientMachine_Delete( int ClientMachineID )
        {
            DataHelper.ClientMachine_Delete( ClientMachineID );
        }

        public ClientGroups ClientGroups_GetByPlatform( string Platform )
        {
            return( DataHelper.ClientGroups_GetByPlatform( Platform ) );
        }
        
        // distribution servers
        public string[] DistributionServer_GetConnectedList()
        {
            return ( UPMS_Service.UPDSList.ToArray() );
        }

        public string[] DistributionServer_GetListFromTasks()
        {
            return DataHelper.DistributionServer_GetListFromTasks();
        }

        public void DistributionServer_Register( string MachineName )
        {
            UPMS_Service.UPDSList.Add( MachineName );
        }

        public void DistributionServer_Unregister( string MachineName )
        {
            UPMS_Service.UPDSList.Remove( MachineName );
        }

        // tasks
        public long Task_AddNew( long PlatformBuildID, DateTime ScheduleTime, int ClientMachineID, string UserName, bool RunAfterProp, string BuildConfig, string CommandLine, bool Recurring )
        {
            return DataHelper.Task_AddNew( PlatformBuildID, ScheduleTime, ClientMachineID, UserName, RunAfterProp, BuildConfig, CommandLine, Recurring );
        }

        public void Task_Delete( long ID )
        {
            DataHelper.Task_Delete( ID );
        }

        // Get a task that is assigned to a UPDS
        public long Task_GetAssignedTask( string UPDSName )
        {
            return ( DataHelper.Task_GetAssignedTask( UPDSName ) );
        }

        public bool Task_UpdateStatus( long TaskID, TaskStatus StatusID, int Progress, string Error )
        {
            bool ValidTask = DataHelper.Task_UpdateStatus( TaskID, StatusID, Progress, Error );
            TaskScheduler.OnUpdateTaskStatus( TaskID, ( short )StatusID );
            return ( ValidTask );
        }

        public Tasks Task_GetList()
        {
            return DataHelper.Task_GetList();
        }

        public Tasks Task_GetByID( long ID )
        {
            return DataHelper.Task_GetByID( ID );
        }

        public TaskStatuses TaskStatus_GetList()
        {
            return DataHelper.TaskStatus_GetList();
        }

        public string[] BuildConfigs_GetForPlatform( string Platform )
        {
            return ( DataHelper.BuildConfigs_GetForPlatform( Platform ) );
        }

        public int User_GetID( string UserName, string Email )
        {
            return( DataHelper.User_GetID( UserName, Email ) );
        }

        public DescriptionWithID[] User_GetListFromTasks()
        {
            return DataHelper.User_GetListFromTasks();
        }

        public DescriptionWithID[] User_GetListFromTargets()
        {
            return DataHelper.User_GetListFromTargets();
        }

        public DescriptionWithID[] User_GetListFromBuilds()
        {
            return DataHelper.User_GetListFromBuilds();
        }

        public void Utils_SendEmail( int TaskerUserNameID, int TaskeeUserNameID, string Subject, string Message, int Importance )
        {
            Mailer.SendEmail( TaskerUserNameID, TaskeeUserNameID, Subject, Message, Importance );
        }

        public void Utils_UpdateStats( string Project, string Platform, long Bytes, DateTime Scheduled )
        {
            DataHelper.Utils_UpdateStats( Project, Platform, Bytes, Scheduled );
        }

        public long Utils_GetStats( string Project, string Platform, DateTime Since, ref long BytesPropped, ref float DataRate )
        {
            return ( DataHelper.Utils_GetStats( Project, Platform, Since, ref BytesPropped, ref DataRate ) );
        }

        public void News_Add( string News )
        {
            DataHelper.News_Add( News );
        }

        public string News_Get()
        {
            return ( DataHelper.News_Get() );
        }
    }

    public class Log
    {
        public enum LogType
        {
            Debug = 0,
            Important = 1,
            Error = 2,
        }

        private static StreamWriter LogFile = null;

        public static void WriteLine( string Label, LogType Type, string Message, params object[] Args )
        {
            string Format = DateTime.Now.ToString( UPMS_Service.US_TIME_DATE_STRING_FORMAT ) + " [" + Label + "] ";
            if( Type == LogType.Error )
            {
                Format += "ERROR!!! ";
            }
            string RawString = string.Format( Format + Message, Args );

            Trace.WriteLine( RawString );
            Console.WriteLine( RawString );

            if( LogFile == null )
            {
                string LogFilePath = "UPMS_" + DateTime.Now.ToString( "yyyy_MM_dd-HH_mm_ss" ) + ".txt";
                LogFile = new StreamWriter( LogFilePath, false, Encoding.ASCII );
            }

            LogFile.WriteLine( RawString );

            if( Type == LogType.Error )
            {
                UPMS_Service.IUPMS.Utils_SendEmail( -1, -1, "UPMS reported error!", RawString, 2 );
                LogFile.Flush();
                // Sleep 5 minutes after any error
                Thread.Sleep( 5 * 60 * 1000 );
            }
        }
    }
}


