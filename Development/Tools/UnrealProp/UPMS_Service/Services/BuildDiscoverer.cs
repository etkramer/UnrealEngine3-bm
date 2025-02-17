using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.IO;
using System.Data;
using System.Data.SqlClient;
using System.Configuration;

namespace UnrealProp
{
    // Working thread for discovering new builds in build repository
    public static class BuildDiscoverer
    {
        static UPMS_Service Owner = null;
        static string BuildRepositoryPath;
        static Thread Thread;
        static Random Rnd = new Random( 95 );

        static public void Init( UPMS_Service InOwner, string RepositoryPath )
        {
            Owner = InOwner;
            BuildRepositoryPath = RepositoryPath;
            Thread = new Thread( new ThreadStart( DiscoveringProc ) );
            Thread.Start();
            Log.WriteLine( "UPMS BUILD DISCOVERER", Log.LogType.Important, "Initialised!" );
        }

        static public void Stop()
        {
            Thread.Abort();
            Thread = null;
        }

        // recursive finding specific file in directory tree
        static void GetBuildListFromRepository( DirectoryInfo DirInfo, string FileName, ref List<string> List, ref int SubLevel )
        {
            DirectoryInfo[] Dirs = DirInfo.GetDirectories();
            FileInfo[] Files = DirInfo.GetFiles( FileName );

            foreach( FileInfo File in Files )
            {
                List.Add( File.FullName );
            }

            SubLevel--;
            if( SubLevel >= 0 )
            {
                foreach( DirectoryInfo SubDir in Dirs )
                {
                    GetBuildListFromRepository( SubDir, FileName, ref List, ref SubLevel );
                }
            }
            SubLevel++;
        }

        // Main thread to discover builds
        static void DiscoveringProc()
        {
            Thread.Sleep( 3000 );

            while( true )
            {
                // Send out periodic status emails
                Owner.SendStatusEmail();

                // Build repository \ game name \ platform \ label  
                // e.g.
                // \\prop-01\Builds\Gear\PC\Gear_PC_[2007-12-04_02.00]
                // \\prop-01\Builds\UT\PC\UT_PC_[2007-10-21_00.11]_[SHIPPING_PC_GAME=1]
                // \\prop-01\Builds\UT\PS3\UT_PS3_[2007-11-14_20.12]_Fixed76801
                //
                // Discoverer looks for copy_complete.txt in these locations

#if !DEBUG
                try
                {
#endif
                    string BuildPath = BuildRepositoryPath + "\\";
                    Log.WriteLine( "UPMS BUILD DISCOVERER", Log.LogType.Important, "is looking for new builds in: " + BuildPath );

                    DirectoryInfo DirInfo = new DirectoryInfo( BuildPath );
                    int Depth = 5;
                    List<string> BuildList = new List<string>();

                    GetBuildListFromRepository( DirInfo, "*TOC.txt", ref BuildList, ref Depth );

                    int NewBuilds = 0;

                    // looking for new build in build list
                    foreach( string Path in BuildList )
                    {
                        if( Path.IndexOf( "UnrealEngine3" ) < 0 )
                        {
                            Log.WriteLine( "UPMS BUILD DISCOVERER", Log.LogType.Error, "Invalid folder name!!! No branch name: " + Path );
                            continue;
                        }

                        // Set up build parameters
                        string ProjectName = "";
                        string Platform = "";
                        string DefineA = "";
                        string DefineB = "";
                        string UserName = "UnrealProp";

                        // Get whether this is an official build
                        bool OfficialBuild = false;
                        if( Path.IndexOf( "User\\" ) < 0 )
                        {
                            OfficialBuild = true;
                            UserName = "BuildMachine";
                        }

                        // Get the path of the build for later
                        string BuildFolder = Path.Substring( 0, Path.IndexOf( "UnrealEngine3" ) );

                        // Get the unique folder name
                        string PublishFolder = BuildFolder.TrimEnd( '\\' );
                        PublishFolder = PublishFolder.Substring( PublishFolder.LastIndexOf( '\\' ) + 1 );

                        if( PublishFolder.Length < "X_Y_[YYYY-MM-DD_HH.MM]".Length )
                        {
                            Log.WriteLine( "UPMS BUILD DISCOVERER", Log.LogType.Error, "Invalid folder name!!! Folder name too small: " + Path );
                            continue;
                        }

                        string[] Parts = PublishFolder.Split( '_' );
                        if( Parts.Length < 3 )
                        {
                            Log.WriteLine( "UPMS BUILD DISCOVERER", Log.LogType.Error, "Invalid folder name!!! No game and platform info: " + Path );
                            continue;
                        }

                        // Gear
                        ProjectName = Parts[0];
                        // PC
                        Platform = Parts[1];
                        // TimeStamp
                        DateTime TimeStamp = DateTime.ParseExact( Parts[2] + "_" + Parts[3], "[yyyy-MM-dd_HH.mm]", null );
                        if( OfficialBuild )
                        {
                            if( Parts.Length == 5 )
                            {
                                DefineA = Parts[4].Trim( "[]".ToCharArray() ).ToUpper();
                            }
                            else if( Parts.Length == 6 )
                            {
                                DefineA = Parts[4].Trim( "[]".ToCharArray() ).ToUpper();
                                DefineB = Parts[5].Trim( "[]".ToCharArray() ).ToUpper();
                            }
                        }
                        else
                        {
                            // Uploaded from UFE, has the format [User] on the end
                            if( Parts.Length == 5 )
                            {
                                UserName = Parts[4].Trim( "[]".ToCharArray() ).ToLower();

                                // Capitalise
                                string[] SplitName = UserName.Split( ".".ToCharArray() );
                                if( SplitName.Length == 2 )
                                {
                                    UserName = SplitName[0].Substring( 0, 1 ).ToUpper() + SplitName[0].Substring( 1 );
                                    UserName += ".";
                                    UserName += SplitName[1].Substring( 0, 1 ).ToUpper() + SplitName[1].Substring( 1 );
                                }
                            }
                        }

                        // looking for known platform in part[1]
                        if( !DataHelper.Platform_IsValid( Platform ) )
                        {
                            Log.WriteLine( "UPMS BUILD DISCOVERER", Log.LogType.Error, "Unknown Platform!!! Cannot add build:" + Path );
                            continue;
                        }

                        // Shrink the publish folder down as we have a 40 character limit
                        PublishFolder = PublishFolder.Replace( "_" + Platform + "_", "_" );

                        int OpenIndex = PublishFolder.IndexOf( '[' );
                        int CloseIndex = PublishFolder.IndexOf( ']' );
                        if( OpenIndex < 0 || CloseIndex < 0 )
                        {
                            Log.WriteLine( "UPMS BUILD DISCOVERER", Log.LogType.Error, "Invalid folder name!!! No timestamp: " + Path );
                            continue;
                        }

                        // Chop out the year
                        PublishFolder = PublishFolder.Substring( 0, OpenIndex + 1 ) + PublishFolder.Substring( OpenIndex + 6, PublishFolder.Length - OpenIndex - 6 );

                        // Just truncate as a last resort
                        if( PublishFolder.Length > 40 )
                        {
                            PublishFolder = PublishFolder.Substring( 0, 40 );
                        }

                        // Add this folder to the build repository database
                        if( DataHelper.PlatformBuild_AddNew( ProjectName, Platform, DefineA, DefineB, UserName, PublishFolder, BuildFolder, TimeStamp ) )
                        {
                            Log.WriteLine( "UPMS BUILD DISCOVERER", Log.LogType.Important, "found new build in repository: " + BuildFolder + " and registered it to database!" );
                            NewBuilds++;
                            Owner.NewBuild();
                        }
                    }

                    if( NewBuilds == 0 )
                    {
                        Log.WriteLine( "UPMS BUILD DISCOVERER", Log.LogType.Debug, "not found any new builds in build repository: " + BuildPath + "." );
                    }

                    // 55-65 sec interval
                    Thread.Sleep( Rnd.Next( 55000, 65000 ) ); 
#if !DEBUG
                }
                catch( Exception Ex )
                {
                    if( Ex.GetType() != typeof( System.Threading.ThreadAbortException ) )
                    {
                        Log.WriteLine( "UPMS BUILD DISCOVERER", Log.LogType.Error, "Unhandled exception: " + Ex.ToString() );
                    }
                }
#endif
            }
        }
    }
}
