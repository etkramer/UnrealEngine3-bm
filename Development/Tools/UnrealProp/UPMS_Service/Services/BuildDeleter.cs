using System;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Security;
using System.Text;
using System.Threading;
using RemotableType;
using RemotableType.PlatformBuildFilesTableAdapters;

namespace UnrealProp
{
    // Working thread for analysing discovered builds
    public static class BuildDeleter
    {
        static Random Rnd = new Random( 5000 );
        static Thread Thread;

        static public void Init()
        {
            Thread = new Thread( new ThreadStart( DeletingProc ) );
            Thread.Start();
            Log.WriteLine( "UPMS BUILD CLEANER", Log.LogType.Important, "Initialised!" );
        }

        static public void Release()
        {
            Thread.Abort();
            Thread = null;
        }

        static void RecursiveDeleteFolder( string Path )
        {
            DirectoryInfo DirInfo = new DirectoryInfo( Path );

            if( DirInfo.Exists )
            {
                foreach( FileInfo File in DirInfo.GetFiles() )
                {
                    File.IsReadOnly = false;
                    File.Delete();
                }

                foreach( DirectoryInfo Dir in DirInfo.GetDirectories() )
                {
                    RecursiveDeleteFolder( Dir.FullName );
                }

                DirInfo.Delete();
            }
        }

        static void DeleteBuild( DataRow Row )
        {
#if !DEBUG
            try
            {
#endif
                Log.WriteLine( "UPMS BUILD CLEANER", Log.LogType.Important, "Started deleting build:" + Row["Path"].ToString().Trim() );

                // Analyzing files
                string Path = Row["Path"].ToString().Trim();

                // Get the ID of the build to delete
                long PlatformBuildID = Convert.ToInt64( Row["ID"] );

                // Delete the files and folders
                RecursiveDeleteFolder( Path );

                DataHelper.PlatformBuild_Delete( PlatformBuildID );

                Log.WriteLine( "UPMS BUILD CLEANER", Log.LogType.Important, "Finished deleting build: " + Row["Path"].ToString().Trim() );
#if !DEBUG
            }
            catch( Exception Ex )
            {
                Log.WriteLine( "UPMS BUILD CLEANER", Log.LogType.Error, "Unhandled exception: " + Ex.ToString() );
            }
#endif
        }

        // Main thread to analyse a build
        static void DeletingProc()
        {
            Thread.Sleep( 10000 );

            while( true )
            {
#if !DEBUG
                try
                {
#endif
                    // Search for deleted builds
                    DataSet DS = DataHelper.GetDataSet( "SELECT * FROM [PlatformBuilds] WHERE ( StatusID = 6 )" );
                    foreach( DataRow Row in DS.Tables[0].Rows )
                    {
                        DeleteBuild( Row );
                    }

                    // 55-65 sec interval
                    Thread.Sleep( Rnd.Next( 55000, 65000 ) );
#if !DEBUG
                }
                catch( Exception Ex )
                {
                    if( Ex.GetType() != typeof( System.Threading.ThreadAbortException ) )
                    {
                        Log.WriteLine( "UPMS BUILD CLEANER", Log.LogType.Error, "Unhandled exception: " + Ex.ToString() );
                    }
                }
#endif
            }
        }
    }
}
