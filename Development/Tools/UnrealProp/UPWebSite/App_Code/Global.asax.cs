using System;
using System.Collections;
using System.Configuration;
using System.Data;
using System.Data.SqlClient;
using System.DirectoryServices;
using System.IO;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Channels.Tcp;
using System.Security.Principal;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.HtmlControls;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using RemotableType;
using UnrealProp;

namespace UnrealProp
{
    public class EpicProvider : XmlSiteMapProvider
    {
        public override bool IsAccessibleToUser( HttpContext Context, SiteMapNode Node )
        {
            foreach( string Role in Node.Roles )
            {
                if( Global.IsInRole( Role, Context.User.Identity.Name ) )
                {
                    return ( true );
                }
            }

            // If no roles defined, allow viewing
            return ( Node.Roles.Count == 0 );
        }
    }

    public class Global : HttpApplication
    {
        private static IUPMS_Interface LocalIUPMS = null;
        private static object ClientLock = new Object();

        static private bool RecursiveGroupCheck( DirectoryEntry RootDirEntry, string Group, string UserName )
        {
            string Query = "(&(objectCategory=user)(memberOf=CN=" + Group + ",OU=Security Groups,DC=epicgames,DC=net)(CN=" + UserName + "))";
            DirectorySearcher DirSearcher = new DirectorySearcher( RootDirEntry, Query );
            // 500ms timeout
            DirSearcher.ClientTimeout = new TimeSpan( 500 * 10000 );

            if( DirSearcher.FindAll().Count > 0 )
            {
                return ( true );
            }

            Query = "(&(objectCategory=group)(memberOf=CN=" + Group + ",OU=Security Groups,DC=epicgames,DC=net))";
            DirSearcher = new DirectorySearcher( RootDirEntry, Query );

            foreach( SearchResult ChildGroup in DirSearcher.FindAll() )
            {
                if( RecursiveGroupCheck( RootDirEntry, ( string )ChildGroup.Properties["CN"][0], UserName ) )
                {
                    return ( true );
                }
            }

            return ( false );
        }

        static public bool IsUser( string User )
        {
            DirectoryEntry RootDirEntry = new DirectoryEntry();
            string UserName = GetUserName( User ).Replace( '.', ' ' );

            bool IsUser = true;
            try
            {
                IsUser = RecursiveGroupCheck( RootDirEntry, "UnrealProp Users", UserName );
            }
            catch
            {
            }

            return ( IsUser );
        }

        static public bool IsAdmin( string User )
        {
            DirectoryEntry RootDirEntry = new DirectoryEntry();
            string UserName = GetUserName( User ).Replace( '.', ' ' );

            bool IsAdmin = false;
            try
            {
                IsAdmin = RecursiveGroupCheck( RootDirEntry, "UnrealProp Admins", UserName );
            }
            catch
            {
            }

            return ( IsAdmin );
        }

        static public bool IsInRole( string Role, string User )
        {
            DirectoryEntry RootDirEntry = new DirectoryEntry();
            string UserName = GetUserName( User ).Replace( '.', ' ' );

            bool IsInRole = false;
            if( Role == "UnrealProp Users" )
            {
                // Default in case the server is down
                IsInRole = true;
            }

            try
            {
                IsInRole = RecursiveGroupCheck( RootDirEntry, Role, UserName );
            }
            catch
            {
            }

            return ( IsInRole );
        }

        static public string GetUserName( string User )
        {
            string[] Parms = User.Split( '\\' );
            return ( Parms[1] );
        }

        // We only want one client per instance of the app because it is inefficient to create a new client for every request.
        static public IUPMS_Interface IUPMS
        {
            get
            {
                // lazy init the client
                lock( ClientLock )
                {
                    if( LocalIUPMS == null )
                    {
                        // Configure the remoting service
                        ChannelServices.RegisterChannel( new TcpClientChannel(), true );
#if false
                        LocalIUPMS = ( IUPMS_Interface )Activator.GetObject( typeof( IUPMS_Interface ), "tcp://localhost:9090/UPMS" );
#else
                        LocalIUPMS = ( IUPMS_Interface )Activator.GetObject( typeof( IUPMS_Interface ), "tcp://prop-01:9090/UPMS" );
#endif
                    }
                    return ( LocalIUPMS );
                }
            }
        }

        static public Platforms Platform_GetList()
        {
            return ( IUPMS.Platform_GetList() );
        }

        static public Projects Project_GetList()
        {
            return ( IUPMS.Project_GetList() );
        }

        static public Platforms Platform_GetListForProject( short ProjectID )
        {
            return ( IUPMS.Platform_GetListForProject( ProjectID ) );
        }

        static public PlatformBuilds PlatformBuild_GetListForProject( short ProjectID )
        {
            return ( IUPMS.PlatformBuild_GetListForProject( ProjectID ) );
        }

        static public void PlatformBuild_Delete( long PlatformBuildID )
        {
            IUPMS.PlatformBuild_Delete( PlatformBuildID );
        }

        static public PlatformBuilds PlatformBuild_GetListForBuild( long PlatformBuildID )
        {
            return ( IUPMS.PlatformBuild_GetListForBuild( PlatformBuildID ) );
        }

        static public PlatformBuilds PlatformBuild_GetListForProjectPlatformAndStatus( short ProjectID, short PlatformID, short StatusID )
        {
            return ( IUPMS.PlatformBuild_GetListForProjectPlatformAndStatus( ProjectID, PlatformID, StatusID ) );
        }

        static public PlatformBuilds PlatformBuild_GetListForProjectPlatformUserAndStatus( short ProjectID, short PlatformID, int UserNameID, short StatusID )
        {
            return ( IUPMS.PlatformBuild_GetListForProjectPlatformUserAndStatus( ProjectID, PlatformID, UserNameID, StatusID ) );
        }

        static public DescriptionWithID[] PlatformBuild_GetSimpleListForProjectAndPlatform( short ProjectID, short platformID )
        {
            return ( IUPMS.PlatformBuild_GetSimpleListForProjectAndPlatform( ProjectID, platformID ) );
        }

        static public PlatformBuildFiles PlatformBuild_GetFiles( long PlatformBuildID )
        {
            return ( IUPMS.PlatformBuild_GetFiles( PlatformBuildID ) );
        }

        static public int PlatformBuild_GetCount()
        {
            return ( IUPMS.PlatformBuild_GetCount() );
        }

        static public long PlatformBuildFiles_GetTotalSize()
        {
            return ( IUPMS.PlatformBuildFiles_GetTotalSize() );
        }

        static public CachedFileInfo CachedFileInfo_GetNewestFiles( int Percent )
        {
            return ( IUPMS.CachedFileInfo_GetNewestFiles( Percent ) );
        }

        static public void PlatformBuildFiles_Update( PlatformBuildFiles Files )
        {
            IUPMS.PlatformBuildFiles_Update( Files );
        }

        static public long PlatformBuild_GetBuildSize( long PlatformBuildID )
        {
            return ( IUPMS.PlatformBuild_GetBuildSize( PlatformBuildID ) );
        }

        static public string PlatformBuild_GetTitle( long PlatformBuildID )
        {
            return ( IUPMS.PlatformBuild_GetTitle( PlatformBuildID ) );
        }

        static public string PlatformBuild_GetPlatformName( long PlatformBuildID )
        {
            return ( IUPMS.PlatformBuild_GetPlatformName( PlatformBuildID ) );
        }

        static public string PlatformBuild_GetRepositoryPath( long PlatformBuildID )
        {
            return ( IUPMS.PlatformBuild_GetRepositoryPath( PlatformBuildID ) );
        }

        static public void PlatformBuild_ChangeStatus( long PlatformBuildID, BuildStatus StatusID, string BuildName )
        {
            IUPMS.PlatformBuild_ChangeStatus( PlatformBuildID, StatusID, BuildName );
        }

        static public void PlatformBuild_ChangeTime( long PlatformBuildID, DateTime TimeStamp )
        {
            IUPMS.PlatformBuild_ChangeTime( PlatformBuildID, TimeStamp );
        }

        static public int PlatformBuild_GetAnalyzingProgress( long PlatformBuildID )
        {
            return ( IUPMS.PlatformBuild_GetAnalyzingProgress( PlatformBuildID ) );
        }

        static public bool CachedFileInfo_FileExists( string Hash )
        {
            return ( IUPMS.CachedFileInfo_FileExists( Hash ) );
        }

        static public string PlatformBuild_GetProject( long PlatformBuildID )
        {
            return ( IUPMS.PlatformBuild_GetProject( PlatformBuildID ) );
        }

        static public PlatformBuildStatuses PlatformBuildStatus_GetList()
        {
            return ( IUPMS.PlatformBuildStatus_GetList() );
        }

        static public ClientMachines ClientMachine_GetListForPlatform( short PlatformID )
        {
            return ( IUPMS.ClientMachine_GetListForPlatform( PlatformID ) );
        }

        static public ClientMachines ClientMachine_GetListForPlatformAndUser( short PlatformID, int UserNameID )
        {
            return ( IUPMS.ClientMachine_GetListForPlatformAndUser( PlatformID, UserNameID ) );
        }

        static public ClientMachines ClientMachine_GetListForPlatformGroupUser( string Platform, string Group, string UserName )
        {
            return ( IUPMS.ClientMachine_GetListForPlatformGroupUser( Platform, Group, UserName ) );
        }

        static public long ClientMachine_Update( int ClientMachineID, string Platform, string Name, string Path, string ClientGroup, string User, string Email, bool Reboot )
        {
            return ( IUPMS.ClientMachine_Update( ClientMachineID, Platform, Name, Path, ClientGroup, User, Email, Reboot ) );
        }

        static public void ClientMachine_Delete( int ClientMachineID )
        {
            IUPMS.ClientMachine_Delete( ClientMachineID );
        }

        static public ClientGroups ClientGroups_GetByPlatform( string Platform )
        {
            return( IUPMS.ClientGroups_GetByPlatform( Platform ) );
        }

        static public string[] DistributionServer_GetConnectedList()
        {
            return ( IUPMS.DistributionServer_GetConnectedList() );
        }

        static public string[] DistributionServer_GetListFromTasks()
        {
            return ( IUPMS.DistributionServer_GetListFromTasks() );
        }

        static public long Task_AddNew( long PlatformBuildID, DateTime ScheduleTime, int ClientMachineID, string UserName, bool RunAfterProp, string BuildConfig, string CommandLine, bool Recurring )
        {
            return ( IUPMS.Task_AddNew( PlatformBuildID, ScheduleTime, ClientMachineID, UserName, RunAfterProp, BuildConfig, CommandLine, Recurring ) );
        }

        static public void Task_Delete( long id )
        {
            IUPMS.Task_Delete( id );
        }

        static public bool Task_UpdateStatus( long TaskID, TaskStatus StatusID, int Progress, string Error )
        {
            return( IUPMS.Task_UpdateStatus( TaskID, StatusID, Progress, Error ) );
        }

        static public Tasks Task_GetList()
        {
            return ( IUPMS.Task_GetList() );
        }

        static public Tasks Task_GetByID( long ID )
        {
            return ( IUPMS.Task_GetByID( ID ) );
        }

        static public TaskStatuses TaskStatus_GetList()
        {
            return ( IUPMS.TaskStatus_GetList() );
        }

        static public string[] BuildConfigs_GetForPlatform( string Platform )
        {
            return ( IUPMS.BuildConfigs_GetForPlatform( Platform ) );
        }

        static public int User_GetID( string UserName, string Email )
        {
            return ( IUPMS.User_GetID( UserName, Email ) );
        }

        static public DescriptionWithID[] User_GetListFromTasks()
        {
            return ( IUPMS.User_GetListFromTasks() );
        }

        static public DescriptionWithID[] User_GetListFromTargets()
        {
            return ( IUPMS.User_GetListFromTargets() );
        }

        static public DescriptionWithID[] User_GetListFromBuilds()
        {
            return ( IUPMS.User_GetListFromBuilds() );
        }

        static public void Utils_SendEmail( int TaskerUserNameID, int TaskeeUserNameID, string Subject, string Message, int Importance )
        {
            Utils_SendEmail( TaskerUserNameID, TaskeeUserNameID, Subject, Message, Importance );
        }

        void Utils_UpdateStats( string Project, string Platform, long Bytes, DateTime Scheduled )
        {
            Utils_UpdateStats( Project, Platform, Bytes, Scheduled );
        }

        long Utils_GetStats( string Project, string Platform, DateTime Since, ref long Bytes, ref float DataRate )
        {
            return ( Utils_GetStats( Project, Platform, Since, ref Bytes, ref DataRate ) );
        }

        static public void News_Add( string News )
        {
            IUPMS.News_Add( News );
        }

        static public string News_Get()
        {
            return( IUPMS.News_Get() );
        }
    }
}