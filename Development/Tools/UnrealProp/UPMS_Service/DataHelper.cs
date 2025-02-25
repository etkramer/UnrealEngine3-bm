using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using System.Configuration;
using System.Collections;
using RemotableType;
using RemotableType.CachedFileInfoTableAdapters;
using RemotableType.ClientGroupsTableAdapters;
using RemotableType.ClientMachinesTableAdapters;
using RemotableType.PlatformBuildsTableAdapters;
using RemotableType.PlatformBuildFilesTableAdapters;
using RemotableType.PlatformBuildStatusesTableAdapters;
using RemotableType.PlatformsTableAdapters;
using RemotableType.ProjectsTableAdapters;
using RemotableType.TasksTableAdapters;
using RemotableType.TaskStatusesTableAdapters;

namespace UnrealProp
{
    public class DataHelper
    {
        // For consistent formatting to the US standard (05/29/2008 11:33:52)
        const string US_TIME_DATE_STRING_FORMAT = "MM/dd/yyyy HH:mm:ss";

        static Hashtable PlatformsCache;

        // Member variables
        private static string ConnectionString = string.Empty;

        static DataHelper()
        {
            ConnectionString = Properties.Settings.Default.UPropConnectionString;
        }

        /// <summary>
        /// Insert, update, or delete a record in the database.
        /// </summary>
        /// <param name="sqlQuery">The SQL query to run against the database.</param>
        /// <returns>The number of rows affected by the operation.</returns>
        public static int ExecuteNonQuery( string sqlQuery )
        {
            // Create and open a connection
            SqlConnection connection = new SqlConnection( ConnectionString );
            connection.Open();

            // Create and configure a command
            SqlCommand command = new SqlCommand( sqlQuery, connection );

            // Execute the command
            int numRowsAffected = command.ExecuteNonQuery();

            // Close and dispose
            command.Dispose();
            connection.Close();
            connection.Dispose();

            // Set return value
            return numRowsAffected;
        }

        /// <summary>
        /// Execute a query that returns a scalar value
        /// </summary>
        /// <param name="sqlQuery">The SQL query to run against the database </param>
        /// <returns>The scalar value returned by the database.</returns>
        public static long ExecuteScalar( string sqlQuery )
        {
            // Create and open a connection
            SqlConnection connection = new SqlConnection( ConnectionString );
            connection.Open();

            // Create and configure a command
            SqlCommand command = new SqlCommand( sqlQuery, connection );

            // Execute the command
            long result = Convert.ToInt64( command.ExecuteScalar() );

            // Close and dispose
            command.Dispose();
            connection.Close();
            connection.Dispose();

            // Set return value
            return result;
        }

        public static string ExecuteScalarStr( string sqlQuery )
        {
            // Create and open a connection
            SqlConnection connection = new SqlConnection( ConnectionString );
            connection.Open();

            // Create and configure a command
            SqlCommand command = new SqlCommand( sqlQuery, connection );

            // Execute the command
            string result = command.ExecuteScalar().ToString();

            // Close and dispose
            command.Dispose();
            connection.Close();
            connection.Dispose();

            // Set return value
            return result;
        }

        /// <summary>
        /// Retrieve a data set.
        /// </summary>
        /// <param name="sqlQuery">The SQL Select query to run against the database. </param>
        /// <returns>A populated data set.</returns>
        /// <remarks>This method uses a connection string passed in as an argument
        /// to the constructor for this class.</remarks>
        public static DataSet GetDataSet( string sqlQuery )
        {
            // Create dataset
            DataSet dataSet = new DataSet();

            // Populate dataset
            using( SqlConnection connection = new SqlConnection( ConnectionString ) )
            {
                SqlCommand command = connection.CreateCommand();
                command.CommandText = sqlQuery;
                SqlDataAdapter dataAdapter = new SqlDataAdapter( command );
                dataAdapter.Fill( dataSet );
            }

            // return dataset
            return dataSet;
        }

        // UPDSNAMES -------------------------------------------------------

        public static int UPDSName_GetID( string UPDSName )
        {
            int ID = ( int )ExecuteScalar( "SELECT ID FROM [UPDSNames] WHERE ( LOWER( UPDSName ) = '" + UPDSName.ToLower() + "' )" );
            if( ID <= 0 )
            {
                string SqlQuery = "INSERT [UPDSNames] ( UPDSName ) VALUES ( '" + UPDSName + "' ) SELECT @@Identity";
                ID = ( int )ExecuteScalar( SqlQuery );
            }

            return ( ID );
        }

        // DEFINES ---------------------------------------------------------

        public static int Define_GetID( string Define )
        {
            int ID;

            ID = ( int )ExecuteScalar( "SELECT ID FROM [Defines] WHERE ( LOWER( Define ) = '" + Define.ToLower() + "' )" );
            if( ID <= 0 )
            {
                string SqlQuery = "INSERT [Defines] ( Define ) VALUES ( '" + Define + "' ) SELECT @@Identity";
                ID = ( int )ExecuteScalar( SqlQuery );
            }

            return ( ID );
        }

        public static string Define_Get( int DefineID )
        {
            return( ExecuteScalarStr( "SELECT Define FROM [Defines] WHERE ( ID = " + DefineID.ToString() + " )" ) );
        }

        // PLATFORMS -------------------------------------------------------

        public static bool Platform_IsValid( string Platform )
        {
            return ( Platform_GetID( Platform ) > 0 );
        }

        private static short Platform_GetID( string Platform )
        {
            if( Platform.Length > 0 )
            {
                if( PlatformsCache == null )
                {
                    PlatformsCache = new Hashtable();
                    Platforms Platforms = Platform_GetList();

                    foreach( Platforms.PlatformsRow Row in Platforms.Tables[0].Rows )
                    {
                        PlatformsCache.Add( Row.Name.Trim(), Row.ID );
                    }
                }

                object Obj = PlatformsCache[Platform];
                if( Obj != null )
                {
                    return( ( short )Obj );
                }
            }

            return( -1 );
        }

        private static string Platform_GetName( short PlatformID )
        {
            string SqlQuery = "SELECT Name FROM [Platforms] WHERE ( ID = " + PlatformID.ToString() + " )";
            string Name = ExecuteScalarStr( SqlQuery );
            return ( Name.Trim() );
        }

        public static Platforms Platform_GetList()
        {
            // Create dataset
            Platforms dataSet = new Platforms();

            PlatformsTableAdapter dataAdapter = new PlatformsTableAdapter();
            dataAdapter.Fill( ( Platforms.PlatformsDataTable )( dataSet.Tables[0] ) );

            // return dataset
            return dataSet;
        }

        public static Platforms Platform_GetListForProject( short ProjectID )
        {
            // Create dataset
            Platforms dataSet = new Platforms();
            PlatformsTableAdapter dataAdapter = new PlatformsTableAdapter();
            dataAdapter.FillByProject( ( Platforms.PlatformsDataTable )( dataSet.Tables[0] ), ProjectID );

            // return dataset
            return dataSet;
        }

        // PROJECTS -------------------------------------------------------

        private static short Project_GetID( string ProjectTitle )
        {
            short ID = -1;

            if( ProjectTitle.Length > 0 )
            {
                ID = ( short )ExecuteScalar( "SELECT ID FROM [Projects] WHERE ( LOWER( Title ) = '" + ProjectTitle.ToLower() + "' )" );
                if( ID <= 0 )
                {
                    string SqlQuery = "INSERT [Projects] ( Title ) VALUES ( '" + ProjectTitle + "' ) SELECT @@Identity";
                    ID = ( short )ExecuteScalar( SqlQuery );
                }
            }

            return( ID );
        }

        private static string Project_GetName( short ProjectID )
        {
            string SqlQuery = "SELECT Title FROM [Projects] WHERE ( ID = " + ProjectID.ToString() + " )";
            string Name = ExecuteScalarStr( SqlQuery );
            return ( Name.Trim() );
        }

        public static Projects Project_GetList()
        {
            // Create dataset
            Projects dataSet = new Projects();

            ProjectsTableAdapter dataAdapter = new ProjectsTableAdapter();
            dataAdapter.Fill( ( Projects.ProjectsDataTable )( dataSet.Tables[0] ) );

            // return dataset
            return dataSet;
        }

        // PLATFORM BUILDS -------------------------------------------------------

        public static PlatformBuilds PlatformBuild_GetListForProject( short ProjectID )
        {
            // Create dataset
            PlatformBuilds DataSet = new PlatformBuilds();
            
            PlatformBuildsTableAdapter DataAdapter = new PlatformBuildsTableAdapter();
            DataAdapter.FillByProject( ( PlatformBuilds.PlatformBuildsDataTable )( DataSet.Tables[0] ), ProjectID );
  
            return DataSet;
        }

        public static void PlatformBuild_Delete( long PlatformBuildID )
        {
            // Delete all the files, and potentially 'leak' the Path and CachedFileInfo
            DataHelper.ExecuteNonQuery( "DELETE FROM [PlatformBuildFiles] WHERE ( PlatformBuildID = " + PlatformBuildID.ToString() + " )" );

            // Delete all the tasks performed on that build from Tasks
            DataHelper.ExecuteNonQuery( "DELETE FROM [Tasks] WHERE ( PlatformBuildID = " + PlatformBuildID.ToString() + " )" );

            // Delete the build from PlatformBuilds
            DataHelper.ExecuteNonQuery( "DELETE FROM [PlatformBuilds] WHERE ( ID = " + PlatformBuildID.ToString() + " )" );
        }

        public static void PlatformBuild_ChangeStatus( long PlatformBuildID, BuildStatus StatusID, string BuildName )
        {
            // Update the status of the build
            short WorkID = ( short )StatusID;
            ExecuteNonQuery( "UPDATE [PlatformBuilds] SET StatusID = " + WorkID.ToString() + " WHERE ( ID = " + PlatformBuildID.ToString() + " )" );
            if( BuildName != null )
            {
                if( BuildName.Length > 40 )
                {
                    BuildName = BuildName.Substring( 0, 40 );
                }

                ExecuteNonQuery( "UPDATE [PlatformBuilds] SET Title = '" + BuildName + "' WHERE ( ID = " + PlatformBuildID.ToString() + " )" );
            }
        }

        public static void PlatformBuild_ChangeSize( long PlatformBuildID )
        {
            long Size = ExecuteScalar( "SELECT SUM( CachedFileInfo.Size ) FROM [PlatformBuildFiles] INNER JOIN [CachedFileInfo] ON PlatformBuildFiles.CachedFileInfoID = CachedFileInfo.ID WHERE ( PlatformBuildFiles.PlatformBuildID = " + PlatformBuildID.ToString() + " )" );
            ExecuteNonQuery( "UPDATE [PlatformBuilds] SET Size = " + Size.ToString() + " WHERE ( ID = " + PlatformBuildID.ToString() + " )" );
        }

        public static void PlatformBuild_ChangeTime( long PlatformBuildID, DateTime TimeStamp )
        {
            ExecuteNonQuery( "UPDATE [PlatformBuilds] SET DiscoveryTime = '" + TimeStamp.ToString( US_TIME_DATE_STRING_FORMAT ) + "' WHERE ( ID = " + PlatformBuildID.ToString() + " )" );
            ExecuteNonQuery( "UPDATE [CachedFileInfo] SET DateAndTime = '" + TimeStamp.ToString( US_TIME_DATE_STRING_FORMAT ) + "' FROM [PlatformBuildFiles] INNER JOIN [CachedFileInfo] ON PlatformBuildFiles.CachedFileInfoID = CachedFileInfo.ID WHERE ( PlatformBuildFiles.PlatformBuildID = " + PlatformBuildID.ToString() + " )" );
        }

        public static bool PlatformBuild_AddNew( string ProjectName, string Platform, string DefineA, string DefineB, string UserName, string BuildName, string Path, DateTime BuildTime )
        {
            // Canonise path name
            Path = Path.TrimEnd( "\\/".ToCharArray() );
            Path = Path.Replace( '/', '\\' );

            // Check for existing build
            long ID = ExecuteScalar( "SELECT ID FROM [PlatformBuilds] WHERE ( LOWER( Path ) = '" + Path.ToLower() + "' )" );
            if( ID <= 0 )
            {
                // New build! Add to DB
                short PlatformID = Platform_GetID( Platform );
                short ProjectID = Project_GetID( ProjectName );
                int DefineAID = Define_GetID( DefineA );
                int DefineBID = Define_GetID( DefineB );
                int UserNameID = User_GetID( UserName, "" );

                string SqlQuery = "INSERT [PlatformBuilds] ( Special, PlatformID, ProjectID, DefineAID, DefineBID, UserNameID, Title, Path, Size, StatusID, DiscoveryTime, BuildTime )";
                SqlQuery += " Values ( 0, " + PlatformID.ToString() + ", " + ProjectID.ToString() + ", ";
                SqlQuery += DefineAID.ToString() + ", " + DefineBID.ToString() + ", ";
                SqlQuery += UserNameID.ToString() + ", '" + BuildName + "', '" + Path + "', 0, 1, ";
                SqlQuery += "'" + DateTime.Now.ToString( US_TIME_DATE_STRING_FORMAT ) + "', '" + BuildTime.ToString( US_TIME_DATE_STRING_FORMAT ) + "' )";
                SqlQuery += " SELECT @@Identity";
                ID = ExecuteScalar( SqlQuery );

                // Build is new - return that we should add it
                return ( true );
            }

            return ( false );
        }

        public static PlatformBuilds PlatformBuild_GetListForBuild( long PlatformBuildID )
        {
            // Create dataset
            PlatformBuilds dataSet = new PlatformBuilds();

            PlatformBuildsTableAdapter dataAdapter = new PlatformBuildsTableAdapter();
            dataAdapter.FillByBuildRO( ( PlatformBuilds.PlatformBuildsDataTable )( dataSet.Tables[0] ), PlatformBuildID );

            // return dataset
            return dataSet;
        }

        public static PlatformBuilds PlatformBuild_GetListForProjectPlatformUserAndStatus( short ProjectID, short PlatformID, int UserNameID, short StatusID )
        {
            // Create dataset
            PlatformBuilds BuildList = new PlatformBuilds();

            PlatformBuildsTableAdapter DataAdapter = new PlatformBuildsTableAdapter();
            DataAdapter.FillByProjectPlatformAndStatus( ( PlatformBuilds.PlatformBuildsDataTable )( BuildList.Tables[0] ), ProjectID, PlatformID, StatusID, UserNameID );

            // return dataset
            return BuildList;
        }

        public static PlatformBuilds PlatformBuild_GetListForProjectPlatformAndStatus( short ProjectID, short PlatformID, short StatusID )
        {
            return ( PlatformBuild_GetListForProjectPlatformUserAndStatus( ProjectID, PlatformID, 1, StatusID ) );
        }

        private static long PlatformBuild_GetLatestBuild( long PlatformBuildID )
        {
            string SqlQuery = "SELECT ID FROM [PlatformBuilds] WHERE ( Special = 1 AND ID = " + PlatformBuildID.ToString() + " )";
            long ID = ExecuteScalar( SqlQuery );
            if( ID > 0 )
            {
                // Build is special - need to redirect
                PlatformBuildID = 0;

                SqlQuery = "SELECT PlatformID FROM [PlatformBuilds] WHERE ( ID = " + ID.ToString() + " )";
                short PlatformID = ( short )ExecuteScalar( SqlQuery );

                SqlQuery = "SELECT ProjectID FROM [PlatformBuilds] WHERE ( ID = " + ID.ToString() + " )";
                short ProjectID = ( short )ExecuteScalar( SqlQuery );

                SqlQuery = "SELECT DefineAID FROM [PlatformBuilds] WHERE ( ID = " + ID.ToString() + " )";
                int DefineAID = ( int )ExecuteScalar( SqlQuery );

                SqlQuery = "SELECT DefineBID FROM [PlatformBuilds] WHERE ( ID = " + ID.ToString() + " )";
                int DefineBID = ( int )ExecuteScalar( SqlQuery );

                if( PlatformID > 0 && ProjectID > 0 && DefineAID > 0 && DefineBID > 0 )
                {
                    SqlQuery = "SELECT ID FROM [PlatformBuilds] WHERE ( Special = 0 AND ";
                    SqlQuery += "PlatformID = " + PlatformID.ToString() + " AND ProjectID = " + ProjectID.ToString() +" AND ";
                    SqlQuery += "DefineAID = " + DefineAID.ToString() + " AND DefineBID = " + DefineBID.ToString() + " AND ";
                    SqlQuery += "UserNameID = 3 AND StatusID = 3 ) ORDER BY BuildTime DESC";
                    PlatformBuildID = ExecuteScalar( SqlQuery );
                }
            }

            return ( PlatformBuildID );
        }

        private static long PlatformBuild_GetSpecial( string Title, short ProjectID, short PlatformID, int DefineAID, int DefineBID )
        {
            string SqlQuery = "SELECT ID FROM [PlatformBuilds] WHERE ( Special = 1 AND ProjectID = " + ProjectID.ToString() + " AND PlatformID = " + PlatformID.ToString() +
                              " AND Title = '" + Title + "' AND DefineAID = " + DefineAID.ToString() + " AND DefineBID = " + DefineBID.ToString() + " )";
            long ID = ExecuteScalar( SqlQuery );
            if( ID <= 0 )
            {
                SqlQuery = "INSERT [PlatformBuilds] ( PlatformID, Special, ProjectID, Title, DefineAID, DefineBID, UserNameID, Path, Size, StatusID, DiscoveryTime )";
                SqlQuery += " Values ( " + PlatformID.ToString() + ", 1, " + ProjectID.ToString() + ", '" + Title + "', " + DefineAID.ToString() + ", " + DefineBID.ToString() + ", 2, '', 0, 3, '" + DateTime.Now.ToString( US_TIME_DATE_STRING_FORMAT ) + "' )"; 
                SqlQuery += " SELECT @@Identity";
                ID = ExecuteScalar( SqlQuery );
            }

            return( ID );
        }

        private class BuildDefinePair
        {
            public int DefineAID;
            public int DefineBID;

            public BuildDefinePair()
            {
                DefineAID = 1;
                DefineBID = 1;
            }

            public BuildDefinePair( int AID, int BID )
            {
                DefineAID = AID;
                DefineBID = BID;
            }
        }

        private static BuildDefinePair[] GetConditionalBuilds( DataSet BuildSet )
        {
            List<BuildDefinePair> Conditions = new List<BuildDefinePair>();

            // Always have the vanilla pair
            Conditions.Add( new BuildDefinePair() );

            foreach( DataRow Build in BuildSet.Tables[0].Rows )
            {
                BuildDefinePair BDP = new BuildDefinePair( ( int )Build[4], ( int )Build[5] );

                bool Exists = false;
                foreach( BuildDefinePair ExistingBDP in Conditions )
                {
                    if( ExistingBDP.DefineAID == BDP.DefineAID && ExistingBDP.DefineBID == BDP.DefineBID )
                    {
                        Exists = true;
                        break;
                    }
                }

                if( !Exists )
                {
                    Conditions.Add( BDP );
                }
            }

            return ( Conditions.ToArray() );
        }

        public static DescriptionWithID[] PlatformBuild_GetSimpleListForProjectAndPlatform( short ProjectID, short PlatformID )
        {
            ArrayList Array = new ArrayList();

            DataSet BuildSet = DataHelper.GetDataSet(
                "SELECT ID, DiscoveryTime, Title, Path, DefineAID, DefineBID FROM [PlatformBuilds] " +
                "WHERE ( PlatformBuilds.StatusID = 3 AND PlatformBuilds.ProjectID = " + ProjectID.ToString() + " AND PlatformBuilds.PlatformID = " + PlatformID.ToString() + " AND PlatformBuilds.Special = 0 ) " +
                "ORDER BY DiscoveryTime DESC" );

            if( BuildSet.Tables[0].Rows.Count > 0 )
            {
                string Project = Project_GetName( ProjectID );
                string Platform = Platform_GetName( PlatformID );
                BuildDefinePair[] ConditionalBuilds = GetConditionalBuilds( BuildSet );

                foreach( BuildDefinePair Condition in ConditionalBuilds )
                {
                    DescriptionWithID Latest = new DescriptionWithID();

                    Latest.Description = "Latest " + Project + " " + Platform + " Build";
                    if( Condition.DefineAID != 1 && Condition.DefineBID != 1 )
                    {
                        string DefineA = Define_Get( Condition.DefineAID );
                        string DefineB = Define_Get( Condition.DefineBID );
                        Latest.Description += " (" + DefineA + " & " + DefineB + ")";
                    }
                    else if( Condition.DefineAID != 1 )
                    {
                        string DefineA = Define_Get( Condition.DefineAID );
                        Latest.Description += " (" + DefineA + ")";
                    }
                    Latest.ID = PlatformBuild_GetSpecial( Latest.Description, ProjectID, PlatformID, Condition.DefineAID, Condition.DefineBID );

                    Array.Add( Latest );
                }
            }

            foreach( DataRow Build in BuildSet.Tables[0].Rows )
            {
                DescriptionWithID Desc = new DescriptionWithID();
                Desc.ID = ( long )Build[0];
                Desc.Description = ( ( DateTime )Build[1] ).ToString( US_TIME_DATE_STRING_FORMAT + " | " ) + Build[2].ToString().Trim();
                Array.Add( Desc );
            }

            return ( DescriptionWithID[] )Array.ToArray( typeof( DescriptionWithID ) );
        }

        public static string PlatformBuild_GetRepositoryPath( long PlatformBuildID )
        {
            return ExecuteScalarStr( "SELECT Path FROM [PlatformBuilds] WHERE ( ID = " + PlatformBuildID.ToString() + " )" ).Trim();
        }

        public static PlatformBuildFiles PlatformBuild_GetFiles( long PlatformBuildID )
        {
            PlatformBuildFiles FileSet = new PlatformBuildFiles();

            PlatformBuildFilesTableAdapter FileSetAdapter = new PlatformBuildFilesTableAdapter();
            FileSetAdapter.FillByPlatformBuild( ( PlatformBuildFiles.PlatformBuildFilesDataTable )( FileSet.Tables[0] ), PlatformBuildID );

            return FileSet;
        }

        public static int PlatformBuild_GetCount()
        {
            return( ( int )ExecuteScalar( "SELECT COUNT( ID ) FROM [PlatformBuilds]" ) );
        }

        public static long PlatformBuildFiles_GetTotalSize()
        {
            long TotalSize = 0;

            long BuildCount = ExecuteScalar( "SELECT COUNT( ID ) FROM [PlatformBuildFiles]" );
            if( BuildCount > 0 )
            {
                // This should count the CachedFileInfo size once for each instance it occurs in the PlatformBuildFiles
                TotalSize = ExecuteScalar( "SELECT SUM( CachedFileInfo.Size ) FROM [PlatformBuildFiles] INNER JOIN [CachedFileInfo] ON PlatformBuildFiles.CachedFileInfoID = CachedFileInfo.ID" );
            }

            return ( TotalSize );
        }

        public static CachedFileInfo CachedFileInfo_GetNewestFiles( int Percent )
        {
            CachedFileInfo FileSet = new CachedFileInfo();

            CachedFileInfoTableAdapter FileSetAdapter = new CachedFileInfoTableAdapter();
            FileSetAdapter.FillByNewestFiles( ( CachedFileInfo.CachedFileInfoDataTable )( FileSet.Tables[0] ), Percent );

            return FileSet;
        }

        private static long PlatformBuildFiles_GetPathID( string Path )
        {
            long ID = ExecuteScalar( "SELECT ID FROM [Paths] WHERE ( LOWER( Path ) = '" + Path.ToLower() + "' )" );
            if( ID <= 0 )
            {
                string SqlQuery = "INSERT [Paths] ( Path ) VALUES ( '" + Path + "' ) SELECT @@Identity";
                ID = ExecuteScalar( SqlQuery );
            }

            return( ID );
        }

        private static long PlatformBuildFiles_GetCachedFileInfoID( string Hash, DateTime DateAndTime, long Size )
        {
            long ID = ExecuteScalar( "SELECT ID FROM [CachedFileInfo] WHERE ( LOWER( Hash ) = '" + Hash.ToLower() + "' AND Size = " + Size.ToString() + " )" );
            if( ID <= 0 )
            {
                string SqlQuery = "INSERT [CachedFileInfo] ( Hash, DateAndTime, Size ) VALUES ( '" + Hash + "', '" + DateAndTime.ToString( US_TIME_DATE_STRING_FORMAT ) + "', " + Size.ToString() + " ) SELECT @@Identity";
                ID = ExecuteScalar( SqlQuery );
            }
            else
            {
                string SqlQuery = "UPDATE [CachedFileInfo] SET DateAndTime = '" + DateAndTime.ToString( US_TIME_DATE_STRING_FORMAT ) + "' WHERE ( ID = " + ID.ToString() + " )";
                ExecuteNonQuery( SqlQuery );
            }

            return( ID );
        }

        public static void PlatformBuildFiles_Update( PlatformBuildFiles Files )
        {
            foreach( PlatformBuildFiles.PlatformBuildFilesRow File in Files.Tables[0].Rows )
            {
                long PathID = PlatformBuildFiles_GetPathID( File.Path );
                if( PathID > 0 )
                {
                    long CachedFileInfoID = PlatformBuildFiles_GetCachedFileInfoID( File.Hash, File.DateAndTime, File.Size );
                    if( CachedFileInfoID > 0 )
                    {
                        string SqlQuery = "INSERT [PlatformBuildFiles] ( PlatformBuildID, PathID, CachedFileInfoID ) VALUES ( " + File.PlatformBuildID.ToString() + ", " + PathID.ToString() + ", " + CachedFileInfoID.ToString() + " )";
                        ExecuteScalar( SqlQuery );
                    }
                }
            }
        }

        public static long PlatformBuild_GetBuildSize( long PlatformBuildID )
        {
            return ExecuteScalar( "SELECT Size FROM [PlatformBuilds] WHERE ( ID = " + PlatformBuildID.ToString() + " )" );
        }

        public static BuildStatus PlatformBuild_GetStatus( long PlatformBuildID )
        {
            return ( BuildStatus )ExecuteScalar( "SELECT StatusID FROM [PlatformBuilds] WHERE ( ID = " + PlatformBuildID.ToString() + " )" );
        }

        public static short PlatformBuild_GetPlatformID( long PlatformBuildID )
        {
            return ( short )ExecuteScalar( "SELECT PlatformID FROM [PlatformBuilds] WHERE ( ID = " + PlatformBuildID.ToString() + " )" );
        }

        public static string PlatformBuild_GetTitle( long PlatformBuildID )
        {
            return ExecuteScalarStr( "SELECT Title FROM [PlatformBuilds] WHERE ( ID = " + PlatformBuildID.ToString() + " )" ).Trim();
        }

        public static string PlatformBuild_GetPlatformName( long PlatformBuildID )
        {
            return ExecuteScalarStr( "SELECT Platforms.Name FROM [PlatformBuilds] INNER JOIN [Platforms] ON PlatformBuilds.PlatformID = Platforms.ID WHERE ( PlatformBuilds.ID = " + PlatformBuildID.ToString() + " )" ).Trim();
        }

        public static bool CachedFileInfo_FileExists( string Hash )
        {
            long CachedFileInfoID = ExecuteScalar( "SELECT ID FROM [CachedFileInfo] WHERE ( Hash = '" + Hash + "' )" );
            return ( CachedFileInfoID > 0 );
        }

        public static string PlatformBuild_GetProject( long PlatformBuildID )
        {
            return ExecuteScalarStr( "SELECT Projects.Title FROM [PlatformBuilds] INNER JOIN [Projects] ON PlatformBuilds.ProjectID = Projects.ID WHERE ( PlatformBuilds.ID = " + PlatformBuildID.ToString() + " )" ).Trim();
        }

        public static PlatformBuildStatuses PlatformBuildStatus_GetList()
        {
            // Create dataset
            PlatformBuildStatuses BuildStatuses = new PlatformBuildStatuses();

            PlatformBuildStatusesTableAdapter DataAdapter = new PlatformBuildStatusesTableAdapter();
            DataAdapter.Fill( ( PlatformBuildStatuses.PlatformBuildStatusesDataTable )( BuildStatuses.Tables[0] ) );

            // return dataset
            return BuildStatuses;
        }

        // CLIENT MACHINES  -------------------------------------------------------

        public static int ClientGroup_GetID( string ClientGroup )
        {
            int ID = ( int )ExecuteScalar( "SELECT ID FROM [ClientGroups] WHERE ( LOWER( GroupName ) = '" + ClientGroup.ToLower() + "' )" );
            if( ID <= 0 )
            {
                string SqlQuery = "INSERT [ClientGroups] ( GroupName ) Values ( '" + ClientGroup + "' ) SELECT @@Identity";
                ID = ( int )ExecuteScalar( SqlQuery );
            }

            return( ID );
        }

        public static ClientMachines ClientMachine_GetListForPlatformAndUser( short PlatformID, int UserNameID )
        {
            // Create dataset
            ClientMachines ClientMachineSet = new ClientMachines();

            if( UserNameID > 0 )
            {
                ClientMachinesTableAdapter ClientMachineAdapter = new ClientMachinesTableAdapter();
                ClientMachineAdapter.FillForPlatform( ( ClientMachines.ClientMachinesDataTable )( ClientMachineSet.Tables[0] ), PlatformID, UserNameID );
            }

            // return dataset
            return( ClientMachineSet );
        }

        public static ClientMachines ClientMachine_GetListForPlatformAndGroup( short PlatformID, int ClientGroupID )
        {
            // Create dataset
            ClientMachines ClientMachineSet = new ClientMachines();

            if( ClientGroupID > 0 )
            {
                ClientMachinesTableAdapter ClientMachineAdapter = new ClientMachinesTableAdapter();
                ClientMachineAdapter.FillForPlatformAndGroup( ( ClientMachines.ClientMachinesDataTable )( ClientMachineSet.Tables[0] ), PlatformID, ClientGroupID );
            }

            // return dataset
            return ( ClientMachineSet );
        }

        public static ClientMachines ClientMachine_GetListForPlatform( short PlatformID )
        {
            return ( ClientMachine_GetListForPlatformAndUser( PlatformID, 1 ) );
        }

        public static ClientMachines ClientMachine_GetListForPlatformGroupUser( string Platform, string ClientGroup, string UserName )
        {
            short PlatformID = Platform_GetID( Platform );
            int ClientGroupID = ClientGroup_GetID( ClientGroup );

            if( ClientGroupID == 1 )
            {
                // Get my targets
                int UserNameID = User_GetID( UserName, "" );
                return ( ClientMachine_GetListForPlatformAndUser( PlatformID, UserNameID ) );
            }

            // Get members of group
            return ( ClientMachine_GetListForPlatformAndGroup( PlatformID, ClientGroupID ) );
        }

        public static long ClientMachine_Update( int ClientMachineID, string Platform, string Name, string Path, string ClientGroup, string User, string Email, bool Reboot )
        {
            int UserID = User_GetID( User, Email );
            int ClientGroupID = ClientGroup_GetID( ClientGroup );
            short PlatformID = Platform_GetID( Platform );

            if( Platform == "PC" )
            {
                Reboot = false;
            }

            if( UserID > 0 && ClientGroupID > 0 && PlatformID > 0 )
            {
                ClientMachinesTableAdapter ClientMachinesAdapter = new ClientMachinesTableAdapter();
                if( ClientMachineID == -1 )
                {
                    ClientMachineID = Int32.Parse( ClientMachinesAdapter.InsertQuery( PlatformID, Path, Name, ClientGroupID, UserID, Reboot ).ToString() );
                }
                else
                {
                    ClientMachinesAdapter.UpdateQuery( PlatformID, Path, Name, ClientGroupID, UserID, Reboot, ClientMachineID );
                }

                return( ClientMachineID );
            }

            return( -1 );
        }

        public static void ClientMachine_Delete( int ClientMachineID )
        {
            ExecuteNonQuery( "DELETE FROM [Tasks] WHERE ( ClientMachineID = " + ClientMachineID.ToString() + " )" );
            ExecuteNonQuery( "DELETE FROM [ClientMachines] WHERE ( ID = " + ClientMachineID.ToString() + " )" );
        }

        public static ClientGroups ClientGroups_GetByPlatform( string Platform )
        {
            ClientGroups GroupSet = new ClientGroups();
            short PlatformID = Platform_GetID( Platform );

            ClientGroupsTableAdapter ClientGroupTableAdapter = new ClientGroupsTableAdapter();
            ClientGroupTableAdapter.FillForPlatform( ( ClientGroups.ClientGroupsDataTable )( GroupSet.Tables[0] ), PlatformID );

            return ( GroupSet );
        }

        // TASKS -------------------------------------------------------

        public static long Task_AddNew( long PlatformBuildID, DateTime ScheduleTime, int ClientMachineID, string UserName, bool RunAfterProp, string BuildConfig, string CommandLine, bool Recurring )
        {
            TasksTableAdapter Adapter = new TasksTableAdapter();

            short PlatformID = PlatformBuild_GetPlatformID( PlatformBuildID );
            int UserNameID = User_GetID( UserName, "" );
            int CommandLineID = CommandLine_GetID( CommandLine );
            short BuildConfigID = BuildConfigs_GetBuildConfigID( PlatformID, BuildConfig );

            // Add the new task to the database
            return ( Adapter.InsertQuery( PlatformBuildID, ScheduleTime, Recurring, DateTime.Now, ClientMachineID, 1, UserNameID, 1, RunAfterProp, BuildConfigID, CommandLineID ) );
        }

        public static void Task_Delete( long ID )
        {
            ExecuteNonQuery( "DELETE FROM [Tasks] WHERE ( ID = " + ID.ToString() + " )" );
        }

        public static long Task_GetAssignedTask( string UPDSName )
        {
            int UPDSID = UPDSName_GetID( UPDSName );
            long TaskID = ExecuteScalar( "SELECT ID FROM [Tasks] WHERE ( AssignedUPDSID = " + UPDSID.ToString() + " AND StatusID = 1 )" );
            return ( TaskID );
        }

        public static Tasks Task_GetList()
        {
            // Create dataset
            Tasks TaskDataSet = new Tasks();

            TasksTableAdapter TaskDataAdapter = new TasksTableAdapter();
            TaskDataAdapter.Fill( ( Tasks.TasksDataTable )TaskDataSet.Tables[0] );

            // return dataset
            return( TaskDataSet );
        }

        public static Tasks Task_GetScheduledList()
        {
            // Create dataset
            Tasks TasksDataSet = new Tasks();

            TasksTableAdapter TaskDataAdapter = new TasksTableAdapter();
            TaskDataAdapter.FillScheduled( ( Tasks.TasksDataTable )TasksDataSet.Tables[0] );

            // return dataset
            return TasksDataSet;
        }

        public static Tasks Task_GetByID( long TaskID )
        {
            // Create dataset
            Tasks TaskDataSet = new Tasks();

            TasksTableAdapter TaskDataAdapter = new TasksTableAdapter();
            TaskDataAdapter.FillByID( ( Tasks.TasksDataTable )TaskDataSet.Tables[0], TaskID );

            // If it's a special build, redirect to the real build
            Tasks.TasksRow Task = ( Tasks.TasksRow )TaskDataSet.Tables[0].Rows[0];
            // Get the most recent full build (if the build is special)
            Task.PlatformBuildID = PlatformBuild_GetLatestBuild( Task.PlatformBuildID );

            // return dataset
            return TaskDataSet;
        }

        public static TaskStatuses TaskStatus_GetList()
        {
            // single task for detail view 

            // Create dataset
            TaskStatuses dataSet = new TaskStatuses();

            TaskStatusesTableAdapter dataAdapter = new TaskStatusesTableAdapter();
            dataAdapter.Fill( ( TaskStatuses.TaskStatusesDataTable )dataSet.Tables[0] );

            // return dataset
            return dataSet;
        }

        public static void Task_AssignToUPDS( long TaskID, string UPDSName )
        {
            int UPDSNameID = UPDSName_GetID( UPDSName );
            if( UPDSNameID > 0 )
            {
                ExecuteNonQuery( "UPDATE [Tasks] SET AssignedUPDSID = " + UPDSNameID.ToString() + " WHERE ( ID = " + TaskID.ToString() + " )" );
            }
        }

        public static bool Task_UpdateStatus( long TaskID, TaskStatus StatusID, int Progress, string Error )
        {   
            // Check for server canceling the task
            short Status = ( short )ExecuteScalar( "SELECT StatusID FROM [Tasks] WHERE ( ID = " + TaskID.ToString() + " )" );
            if( Status == ( short )TaskStatus.Canceled )
            {
                return ( false );
            }

            // Don't cancel finished of failed tasks
            if( StatusID == TaskStatus.Canceled )
            {
                if( Status != ( short )TaskStatus.InProgress && Status != ( short )TaskStatus.Scheduled )
                {
                    StatusID = ( TaskStatus )Status;
                }
            }

            TasksTableAdapter DataAdapter = new TasksTableAdapter();
            int ErrorID = Error_GetID( Error );
            DataAdapter.UpdateStatus( ( short )StatusID, ErrorID, TaskID );
            if( StatusID == TaskStatus.Finished || StatusID == TaskStatus.Canceled )
            {
                ExecuteNonQuery( "UPDATE [Tasks] SET Progress = 100 WHERE ( ID = " + TaskID.ToString() + " )" );
                DataAdapter.UpdateCompletionTime( DateTime.Now, TaskID );
            }
            else
            {
                ExecuteNonQuery( "UPDATE [Tasks] SET Progress = " + Progress.ToString() + " WHERE ( ID = " + TaskID.ToString() + " )" );
            }

            return( true );
        }

        private static short BuildConfigs_GetBuildConfigID( short PlatformID, string BuildConfig )
        {
            short ID = ( short )ExecuteScalar( "SELECT ID FROM [BuildConfigs] WHERE ( PlatformID = " + PlatformID.ToString() + " AND LOWER( BuildConfig ) = '" + BuildConfig.ToLower() + "' )" );
            return ( ID );
        }

        public static string[] BuildConfigs_GetForPlatform( string Platform )
        {
            ArrayList Configs = new ArrayList();

            short PlatformID = Platform_GetID( Platform );

            if( PlatformID > 0 )
            {
                DataSet BuildConfigDataSet = DataHelper.GetDataSet( "SELECT BuildConfig FROM [BuildConfigs] WHERE ( PlatformID = " + PlatformID.ToString() + " )" );
                foreach( DataRow Row in BuildConfigDataSet.Tables[0].Rows )
                {
                    Configs.Add( Row[0].ToString().Trim() );
                }
            }

            return ( string[] )Configs.ToArray( typeof( string ) );
        }

        // ERRORS --------------------------------------------------------------

        public static int Error_GetID( string Error )
        {
            int ID;

            // Cap the string length to 4095 chars
            if( Error.Length > 4095 )
            {
                Error = Error.Remove( 4095 );
            }

            ID = ( int )ExecuteScalar( "SELECT ID FROM [Errors] WHERE ( LOWER( Error ) = '" + Error.ToLower() + "' )" );
            if( ID <= 0 )
            {
                string SqlQuery = "INSERT [Errors] ( Error ) VALUES ( '" + Error + "' ) SELECT @@Identity";
                ID = ( int )ExecuteScalar( SqlQuery );
            }

            return ( ID );
        }

        // COMMANDLINES --------------------------------------------------------

        public static int CommandLine_GetID( string CommandLine )
        {
            int ID;

            ID = ( int )ExecuteScalar( "SELECT ID FROM [CommandLines] WHERE ( LOWER( CommandLine ) = '" + CommandLine.ToLower() + "' )" );
            if( ID <= 0 )
            {
                string SqlQuery = "INSERT [CommandLines] ( CommandLine ) VALUES ( '" + CommandLine + "' ) SELECT @@Identity";
                ID = ( int )ExecuteScalar( SqlQuery );
            }

            return ( ID );
        }
        
        // USERNAMES -----------------------------------------------------------

        public static string User_GetEmail( int UserNameID )
        {
            string Email = ExecuteScalarStr( "SELECT Email FROM [UserNames] WHERE ( ID = " + UserNameID.ToString() + " )" );
            return ( Email );
        }

        public static int User_GetID( string UserName, string Email )
        {
            int ID;

            ID = ( int )ExecuteScalar( "SELECT ID FROM [UserNames] WHERE ( LOWER( UserName ) = '" + UserName.ToLower() + "' )" );
            if( ID <= 0 )
            {
                string SqlQuery = "INSERT [UserNames] ( UserName, Email ) VALUES ( '" + UserName + "', '" + Email + "' ) SELECT @@Identity";
                ID = ( int )ExecuteScalar( SqlQuery );
            }

            return ( ID );
        }

        private static DescriptionWithID[] User_GetList( string SqlQuery )
        {
            ArrayList Users = new ArrayList();

            DataSet UserDataSet = DataHelper.GetDataSet( SqlQuery );
            foreach( DataRow User in UserDataSet.Tables[0].Rows )
            {
                DescriptionWithID UserName = new DescriptionWithID();

                // Interesting (and required) cast
                UserName.ID = ( long )( int )User[0];
                UserName.Description = User[1].ToString().Trim();

                Users.Add( UserName );
            }

            return ( DescriptionWithID[] )Users.ToArray( typeof( DescriptionWithID ) );
        }

        public static DescriptionWithID[] User_GetListFromTasks()
        {
            return ( User_GetList( "SELECT DISTINCT UserNameID, UserNames.UserName FROM [Tasks] INNER JOIN [UserNames] ON UserNameID = UserNames.ID" ) );
        }

        public static DescriptionWithID[] User_GetListFromTargets()
        {
            return ( User_GetList( "SELECT DISTINCT UserNameID, UserNames.UserName FROM [ClientMachines] INNER JOIN [UserNames] ON UserNameID = UserNames.ID" ) );
        }

        public static DescriptionWithID[] User_GetListFromBuilds()
        {
            return ( User_GetList( "SELECT DISTINCT UserNameID, UserNames.UserName FROM [PlatformBuilds] INNER JOIN [UserNames] ON UserNameID = UserNames.ID" ) );
        }

        public static string[] DistributionServer_GetListFromTasks()
        {
            ArrayList Servers = new ArrayList();

            DataSet UPDServers = DataHelper.GetDataSet( "SELECT DISTINCT AssignedUPDSID, UPDSNames.UPDSName FROM [Tasks] INNER JOIN [UPDSNames] ON AssignedUPDSID = UPDSNames.ID WHERE ( AssignedUPDSID != 1 )" );
            foreach( DataRow UPDS in UPDServers.Tables[0].Rows )
            {
                Servers.Add( UPDS[1].ToString().Trim() );
            }

            return ( string[] )Servers.ToArray( typeof( string ) );
        }

        // UTILS -------------------------------------------------------

        public static void Utils_UpdateStats( string Project, string Platform, long Bytes, DateTime Scheduled )
        {
            short ProjectID = Project_GetID( Project );
            short PlatformID = Platform_GetID( Platform );

            if( ProjectID > 0 && PlatformID > 0 )
            {
                string ScheduledTime = Scheduled.ToString( US_TIME_DATE_STRING_FORMAT );
                string CompletionTime = DateTime.Now.ToString( US_TIME_DATE_STRING_FORMAT );
                string SqlQuery = "INSERT [Stats] ( ProjectID, PlatformID, Count, Scheduled, Completed ) VALUES ( " + ProjectID.ToString() + ", " + PlatformID.ToString() + ", " + Bytes.ToString() + ", '" + ScheduledTime + "', '" + CompletionTime + "' )";
                ExecuteNonQuery( SqlQuery );
            }
        }

        public static long Utils_GetStats( string Project, string Platform, DateTime Since, ref long BytesPropped, ref float DataRate )
        {
            short ProjectID = Project_GetID( Project );
            short PlatformID = Platform_GetID( Platform );

            string SqlQuery = "SELECT COUNT( Count ) FROM [Stats]";
            SqlQuery += " WHERE ( " + ProjectID.ToString() + " = ProjectID OR " + ProjectID.ToString() + " = -1 )";
            SqlQuery += " AND ( " + PlatformID.ToString() + " = PlatformID OR " + PlatformID.ToString() + " = -1 )";
            long NumProps = ExecuteScalar( SqlQuery );

            BytesPropped = 0;
            DataRate = 0.0f;
            if( NumProps > 0 )
            {
                SqlQuery = "SELECT SUM( Count ) FROM [Stats]";
                SqlQuery += " WHERE ( " + ProjectID.ToString() + " = ProjectID OR " + ProjectID.ToString() + " = -1 )";
                SqlQuery += " AND ( " + PlatformID.ToString() + " = PlatformID OR " + PlatformID.ToString() + " = -1 )";
                BytesPropped = ExecuteScalar( SqlQuery );

                SqlQuery = "SELECT SUM( DATEDIFF( s, Scheduled, Completed ) ) FROM [Stats]";
                SqlQuery += " WHERE ( " + ProjectID.ToString() + " = ProjectID OR " + ProjectID.ToString() + " = -1 )";
                SqlQuery += " AND ( " + PlatformID.ToString() + " = PlatformID OR " + PlatformID.ToString() + " = -1 )";
                DataRate = BytesPropped / ( 1024.0f * 1024.0f * ExecuteScalar( SqlQuery ) );
            }

            return ( NumProps );
        }

        // NEWS --------------------------------------------------------

        public static void News_Add( string News )
        {
            StreamWriter NewsWriter = new StreamWriter( "News.txt", false, Encoding.ASCII );
            if( NewsWriter != null )
            {
                NewsWriter.Write( News );
                NewsWriter.Close();
            }
        }

        public static string News_Get()
        {
            string News = "";

            try
            {
                StreamReader NewsReader = new StreamReader( "News.txt" );
                if( NewsReader != null )
                {
                    News = NewsReader.ReadToEnd();
                    NewsReader.Close();
                }
            }
            catch
            {
            }

            return ( News );
        }
    }
}
