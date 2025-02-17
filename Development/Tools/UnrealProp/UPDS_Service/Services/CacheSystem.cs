using System;
using System.Collections.Generic;
using System.Text;
using System.Collections;
using System.IO;
using System.Threading;
using System.Security.AccessControl;
using System.Security.Permissions;
using System.Security.Principal;
using System.Runtime.InteropServices;
using RemotableType;

namespace UnrealProp
{
    public class CachedFile
    {
        public long Size;
    }

    public class FileToCache
    {
        public string RepositoryPath;
        public long FileSize;
        public bool IdleMode = false;
        public bool Force = false;

        public FileToCache( string InRepositoryPath, long InFileSize, bool InIdleMode, bool InForce )
        {
            RepositoryPath = InRepositoryPath;
            FileSize = InFileSize;
            IdleMode = InIdleMode;
            Force = InForce;
        }
    }

    static class CacheSystem
    {
        static Object SyncObject = new Object();

        static string CachePath = Properties.Settings.Default.CacheLocation + "\\";
        static Hashtable CachedFiles = Hashtable.Synchronized( new Hashtable() );
        static Hashtable FilesToCache = Hashtable.Synchronized( new Hashtable() );
        static Thread Worker;

        static public int GetNumberFilesToCache()
        {
            return ( FilesToCache.Count );
        }

        static public long GetCacheSize()
        {
            long CacheSize = 0;

            lock( CachedFiles.SyncRoot )
            {
                IDictionaryEnumerator Enumerator = CachedFiles.GetEnumerator();
                while( Enumerator.MoveNext() )
                {
                    string Hash = Enumerator.Key.ToString();
                    CachedFile CachedFile = CachedFiles[Hash] as CachedFile;
                    CacheSize += CachedFile.Size;
                }
            }

            return ( CacheSize );
        }

        static public void Init()
        {
            CachedFiles.Clear();

            Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Important, "Creating cache folder: " + CachePath );
            Directory.CreateDirectory( CachePath );
            char[] HexChars = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
            foreach( char HiNybble in HexChars )
            {
                foreach( char LoNybble in HexChars )
                {
                    string Path = CachePath + HiNybble + LoNybble;
                    Directory.CreateDirectory( Path );

                    // enumerate cached files on disk
                    DirectoryInfo DirInfo = new DirectoryInfo( Path );
                    FileInfo[] Files = DirInfo.GetFiles();
                    foreach( FileInfo File in Files )
                    {
                        CachedFile NewCachedFile = new CachedFile();
                        NewCachedFile.Size = File.Length;
                        CachedFiles.Add( File.Name, NewCachedFile );
                    }
                }
            }

            Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Important, "Initialised!" );
            Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Important, "Found {0} files in local cache ({1} GB)", CachedFiles.Count, GetCacheSize() / ( 1024 * 1024 * 1024 ) );

            Worker = new Thread( new ThreadStart( FileCacherProc ) );
            Worker.Start();
        }

        static public bool PrecacheBuild( PlatformBuildFiles Files, string RepositoryPath, bool IdleMode )
        {
            // end of idling
            if( IdleMode == false )
            {
                Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Debug, "Removing files marked to cache idly from caching list" );

                ArrayList KeysToRemove = new ArrayList();

                // remove all idle files from cache
                lock( FilesToCache.SyncRoot )
                {
                    IDictionaryEnumerator Enumerator = FilesToCache.GetEnumerator();
                    while( Enumerator.MoveNext() )
                    {
                        FileToCache FTC = ( FileToCache )Enumerator.Value;
                        if( FTC.IdleMode )
                        {
                            KeysToRemove.Add( Enumerator.Key );
                        }
                    }
                }

                Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Debug, " ... removing " + KeysToRemove.Count.ToString() + " files" );
                foreach( string Key in KeysToRemove )
                {
                    FilesToCache.Remove( Key );
                }
            }

            Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Debug, "Adding " + Files.Tables[0].Rows.Count.ToString() + " required build files" );
            foreach( PlatformBuildFiles.PlatformBuildFilesRow File in Files.Tables[0].Rows )
            {
                AddFileToCache( File.Hash.Trim(), RepositoryPath + "\\" + File.Path.Trim(), File.Size, IdleMode, false );
            }

            return( true );
        }

        static public void DeleteOrphans()
        {
            lock( CachedFiles.SyncRoot )
            {
                if( TaskExecutor.TaskCount == 0 )
                {
                    List<string> Hashes = new List<string>();

                    IDictionaryEnumerator Enumerator = CachedFiles.GetEnumerator();
                    while( Enumerator.MoveNext() )
                    {
                        Hashes.Add( Enumerator.Key.ToString() );
                    }

                    if( Hashes.Count > 500 )
                    {
                        // Check 500 random files per pass
                        Random Rnd = new Random();
                        for( int Index = 0; Index < 500; Index++ )
                        {
                            int HashIndex = Rnd.Next( Hashes.Count );
                            string Hash = Hashes[HashIndex];
                            if( Hash.Length > 0 )
                            {
                                if( !UPDS_Service.IUPMS.CachedFileInfo_FileExists( Hash ) )
                                {
                                    string DestFileName = GetCachedFilePath( Hash );
                                    File.Delete( DestFileName );

                                    CachedFiles.Remove( Hash );

                                    Hashes[HashIndex] = "";

                                    Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Debug, "Purged orphaned cache file: (" + Hash + ")" );
                                }
                            }
                        }
                    }
                }
            }
        }

        static public void DeleteOldFiles()
        {
            lock( CachedFiles.SyncRoot )
            {
                if( TaskExecutor.TaskCount == 0 )
                {
                    // Check to see the size of the cache
                    long CacheSize = GetCacheSize();
                    long LocalCacheLimit = Int64.Parse( Properties.Settings.Default.CacheSizeGB ) * 1024 * 1024 * 1024;
                    if( CacheSize > LocalCacheLimit )
                    {
                        long MasterCacheSize = UPDS_Service.IUPMS.PlatformBuildFiles_GetTotalSize();
                        int Percent = ( int )( ( CacheSize * 100 ) / MasterCacheSize ) + 5;

                        // Find the files that should be in the cache
                        CachedFileInfo FilesToCheck = UPDS_Service.IUPMS.CachedFileInfo_GetNewestFiles( Percent );

                        // Delete files from end until cache is back under control
                        for( int FileIndex = FilesToCheck.Tables[0].Rows.Count - 1; FileIndex >= 0; FileIndex-- )
                        {
                            CachedFileInfo.CachedFileInfoRow Row = ( CachedFileInfo.CachedFileInfoRow )FilesToCheck.Tables[0].Rows[FileIndex];
                            string DestFileName = GetCachedFilePath( Row.Hash );

                            FileInfo Info = new FileInfo( DestFileName );
                            if( Info.Exists )
                            {
                                Info.IsReadOnly = false;
                                Info.Delete();

                                CacheSize -= Row.Size;

                                CachedFiles.Remove( Row.Hash );
                                Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Debug, "Purged old cache file: (" + Row.Hash + ")" );

                                if( CacheSize < LocalCacheLimit )
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        static public bool IsFileAvailable( PlatformBuildFiles.PlatformBuildFilesRow File, string RepositoryPath )
        {
            bool Available = CachedFiles.ContainsKey( File.Hash.Trim() );

            if( !Available )
            {
                AddFileToCache( File.Hash.Trim(), RepositoryPath + "\\" + File.Path.Trim(), File.Size, false, false );
            }

            return( Available );
        }

        static public string GetCachedFilePath( string Hash )
        {
            return CachePath + Hash.Substring( 0, 2 ) + "\\" + Hash;
        }

        static private bool CacheFile( string Hash, string PathFromRepository, long FileSize, bool Force )
        {
            Mutex mCheck = new Mutex( false, Hash );
            mCheck.WaitOne();

            if( CachedFiles.ContainsKey( Hash ) )
            {
                if( !Force )
                {
                    Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Debug, "Already in cached files hash: (" + Hash + ") " + PathFromRepository );
                    mCheck.ReleaseMutex();
                    return true;
                }
                else
                {
                    Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Debug, "Force recache of: (" + Hash + ") " + PathFromRepository );
                    CachedFiles.Remove( Hash );
                }
            }

            CachedFile NewCachedFile = new CachedFile();
            string DestFileName = GetCachedFilePath( Hash );

            if( !File.Exists( DestFileName ) )
            {
                string ErrorString = "";

                NewCachedFile.Size = Utils.CopyFile( PathFromRepository, DestFileName, ref ErrorString );
                if( NewCachedFile.Size >= 0 )
                {
                    Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Debug, "File copied from server: (" + Hash + ") " + PathFromRepository );
                }
                else
                {
                    Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Error, "Utils.CopyFile: (" + Hash + ") " + PathFromRepository + " : " + ErrorString );
                }
            }
            else
            {
                Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Debug, "File found in disk cache: (" + Hash + ") " + PathFromRepository );
            }

            CachedFiles.Add( Hash, NewCachedFile );

            mCheck.ReleaseMutex();
            return true;
        }

        static public bool AddFileToCache( string Hash, string PathFromRepository, long Size, bool IdleMode, bool Force )
        {
            if( !FilesToCache.ContainsKey( Hash ) )
            {
                FileToCache CacheData = new FileToCache( PathFromRepository, Size, IdleMode, Force );
                FilesToCache.Add( Hash, CacheData );
            }

            return( true );
        }

        static private bool AddFileToCache( string Hash, FileToCache CacheData )
        {
            if( !FilesToCache.ContainsKey( Hash ) )
            {
                FilesToCache.Add( Hash, CacheData );
            }

            return( true );
        }

        static void FileCacherProc()
        {
            while( true )
            {
#if !DEBUG
                try
                {
#endif
                    while( FilesToCache.Count > 0 )
                    {
                        string Hash;
                        FileToCache CacheData;

                        Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Debug, "Caching " + FilesToCache.Count.ToString() + " files" );

                        lock( FilesToCache.SyncRoot )
                        {
                            IDictionaryEnumerator Enumerator = FilesToCache.GetEnumerator();
                            Enumerator.MoveNext();
                            Hash = Enumerator.Key.ToString();
                            CacheData = FilesToCache[Hash] as FileToCache;

                            FilesToCache.Remove( Hash );
                        }

                        if( CacheFile( Hash, CacheData.RepositoryPath, CacheData.FileSize, CacheData.Force ) )
                        {
                            if( CacheData.IdleMode )
                            {
                                // sleep 0.5 on idle mode
                                Thread.Sleep( 500 );
                            }
                        }
                        else
                        {
                            // If the cache failed, readd the file to be cached
                            lock( FilesToCache.SyncRoot )
                            {
                                AddFileToCache( Hash, CacheData );
                            }
                        }

                        Thread.Sleep( 10 );
                    }

                    Thread.Sleep( 100 );
#if !DEBUG
                }
                catch( Exception Ex )
                {
                    Log.WriteLine( "UPDS CACHE SYSTEM", Log.LogType.Error, "Unhandled exception: " + Ex.ToString() );
                }
#endif
            }
        }
    }
}
