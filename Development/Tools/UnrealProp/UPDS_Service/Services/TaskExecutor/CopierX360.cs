using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.IO;
using System.Collections;
using XDevkit;
using RemotableType;

namespace UnrealProp
{
    class CopierX360 : CopierTask
    {
        private X360FileManager XFileManager = new X360FileManager();

        public CopierX360( Tasks.TasksRow Task )
        {
            TaskData = Task;
            Worker = new Thread( new ThreadStart( WorkerProc ) );
            Worker.Start();
        }

        override public void WorkerProc()
        {
            IXboxConsole Target = null;

            try
            {
                const ulong SpaceNeeded = 10L * 1024L * 1024L * 1024L;

                string HashedFileName;
                string FileName;
                string DestPath;
                string DestFolder;
                short LastProgress = -1;
                int MaxErrors = 10;
                bool TaskCancelled = false;
                Hashtable CreatedDirectories = new Hashtable();

                string TargetName = TaskData.Path;
                string DestinationPath = UPDS_Service.IUPMS.PlatformBuild_GetTitle( TaskData.PlatformBuildID ).Replace( '=', '-' );
                string RepositoryPath = UPDS_Service.IUPMS.PlatformBuild_GetRepositoryPath( TaskData.PlatformBuildID );
                FilesToCopy = UPDS_Service.IUPMS.PlatformBuild_GetFiles( TaskData.PlatformBuildID );
                long BuildSize = UPDS_Service.IUPMS.PlatformBuild_GetBuildSize( TaskData.PlatformBuildID );

                // Precache all files in this build (before the reboot)
                CacheSystem.PrecacheBuild( FilesToCopy, RepositoryPath, false );

                // Connect to Xenon
                Target = XFileManager.ConnectToTarget( TargetName );
                if( Target == null )
                {
                    throw new Exception( "Failed to connect to console: " + TargetName + " - is it turned on?" );
                }

                // Optionally reboot console
                if( TaskData.Reboot )
                {
                    if( !XFileManager.RebootTarget( Target ) )
                    {
                        throw new Exception( "Failed to reboot console: " + TargetName );
                    }

                    Log.WriteLine( "UPDS CopierXenon", Log.LogType.Debug, "After reboot console..." );
                    // Wait for console reboot.
                    Thread.Sleep( 30000 );
                    Log.WriteLine( "UPDS CopierXenon", Log.LogType.Debug, "After sleep..." );
                }

                // Reconnect to target
                Target = XFileManager.ConnectToTarget( TargetName );
                if( Target == null )
                {
                    throw new Exception( "Failed to connect to console after reboot: " + TargetName + " - is it turned on?" );
                }

                Log.WriteLine( "UPDS CopierXenon", Log.LogType.Debug, "After Connect to console..." );

                // Delete the destination folder to ensure a clean start
                XFileManager.DeleteDirectory( Target, DestinationPath );

                Log.WriteLine( "UPDS CopierXenon", Log.LogType.Debug, "After delete directory..." );

                if( XFileManager.AvailableSpace( Target ) < SpaceNeeded )
                {
                    throw new Exception( "Not enough space available for build on: " + TargetName );
                }
                
                int NumberFilesToCopy = FilesToCopy.Tables[0].Rows.Count;
                long NumberBytesCopied = 0;
                do
                {
                    foreach( PlatformBuildFiles.PlatformBuildFilesRow Row in FilesToCopy.Tables[0].Rows )
                    {
                        if( Row.RowState != System.Data.DataRowState.Modified )
                        {
                            if( CacheSystem.IsFileAvailable( Row, RepositoryPath ) )
                            {
                                NumberBytesCopied += Row.Size;
                                TaskData.Progress = ( short )( ( NumberBytesCopied * 100.0 ) / ( ( double )BuildSize ) );

                                // Get the cached file name
                                HashedFileName = CacheSystem.GetCachedFilePath( Row.Hash.Trim() );
                                FileName = Row.Path.Trim();
                                if( FileName.StartsWith( "UnrealEngine3" ) )
                                {
                                    FileName = FileName.Substring( FileName.IndexOf( '\\' ) + 1 );
                                }

                                // Create any new required folders
                                DestPath = DestinationPath + FileName;
                                DestFolder = Path.GetDirectoryName( DestPath );
                                if( !CreatedDirectories.ContainsKey( DestFolder ) )
                                {
                                    XFileManager.CreateDirectory( Target, DestFolder );
                                    CreatedDirectories.Add( DestFolder, true );
                                }

                                FileInfo HashedFileInfo = new FileInfo( HashedFileName );
                                if( HashedFileInfo.Exists )
                                {
                                    // Send the file to the Xenon
                                    if( XFileManager.SendFile( Target, HashedFileName, DestPath ) )
                                    {
                                        Row.SetModified();
                                        NumberFilesToCopy--;

                                        Log.WriteLine( "UPDS CopierXenon", Log.LogType.Debug, TaskData.Progress + "%: " + Row.Path );
                                        if( TaskData.Progress != LastProgress )
                                        {
                                            LastProgress = TaskData.Progress;
                                            if( !UPDS_Service.IUPMS.Task_UpdateStatus( TaskData.ID, TaskStatus.InProgress, TaskData.Progress, "In progress" ) )
                                            {
                                                TaskCancelled = true;
                                                break;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        Log.WriteLine( "UPDS CopierXenon", Log.LogType.Error, "Failed to send file: " + HashedFileName + " to: " + DestPath + " with exception " + XFileManager.ErrorString );
                                        MaxErrors--;
                                        if( MaxErrors <= 0 )
                                        {
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    // Recache the file if it doesn't exist
                                    CacheSystem.AddFileToCache( Row.Hash.Trim(), RepositoryPath + "\\" + Row.Path.Trim(), Row.Size, false, true );
                                    Log.WriteLine( "UPDS CopierXenon", Log.LogType.Debug, "Recaching: " + Row.Path );
                                }
                            }
                        }

                        Thread.Sleep( 10 );
                    }
                }
                while( NumberFilesToCopy > 0 && MaxErrors > 0 && !TaskCancelled );

                // Mark as 100% complete
                TaskData.Progress = 100;
                Log.WriteLine( "UPDS CopierXenon", Log.LogType.Debug, "Task complete!" );
                TaskExecutor.ConfirmTask( this );

                if( TaskCancelled )
                {
                    SendCancelledEmail( DestinationPath, TargetName, TaskData.AssignedUPDS );
                    XFileManager.DisconnectFromTarget( Target );
                }
                else
                {
                    if( MaxErrors <= 0 )
                    {
                        throw new Exception( "Exceeded max number of SendFile errors" );
                    }
                    string LongTargetName = TargetName + " (" + TaskData.FriendlyName + ")";
                    SendSuccessEmail( DestinationPath, LongTargetName, TaskData.AssignedUPDS, BuildSize );
                    if( TaskData.RunAfterProp )
                    {
                        string Game = UPDS_Service.IUPMS.PlatformBuild_GetProject( TaskData.PlatformBuildID );
                        if( !XFileManager.RebootAndLaunchTarget( Target, DestinationPath, Game, TaskData.BuildConfig, TaskData.CommandLine ) )
                        {
                            SendFailedToLaunchEmail( DestinationPath, TaskData.BuildConfig, TaskData.CommandLine );
                        }
                    }

                    XFileManager.DisconnectFromTarget( Target );
                }
            }
            catch( Exception Ex )
            {
                string Error = Ex.Message;
                if( !TaskExecutor.TaskError( this, Ex.Message ) )
                {
                    Error = "Additional exception in TaskExecutor.TaskError" + Environment.NewLine + Environment.NewLine + Error;
                }

                XFileManager.DisconnectFromTarget( Target );
                SendFailureEmail( Error );
            }
        }
    }
}
