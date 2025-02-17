using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Management;
using System.Runtime.InteropServices;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Channels.Tcp;
using System.ServiceProcess;
using System.Text;
using System.Threading;
using RemotableType;

namespace UnrealProp
{
    public partial class UPDS_Service : ServiceBase
    {
        // For consistent formatting to the US standard (05/29/2008 11:33:52)
        public const string US_TIME_DATE_STRING_FORMAT = "MM/dd/yyyy HH:mm:ss";

        public static IUPMS_Interface IUPMS = null;
        public static string MachineName = "";

        public UPDS_Service()
        {
            InitializeComponent();
        }

        static void AppExceptionHandler( object sender, UnhandledExceptionEventArgs args )
        {
            Exception E = ( Exception )args.ExceptionObject;
            Log.WriteLine( "Application exception", Log.LogType.Error, E.ToString() );
        }

        public void DebugRun( string[] Args )
        {
            OnStart( Args );
            Console.WriteLine( "Press enter to exit" );
            Console.Read();
            OnStop();
        }

        private void GetMachineName()
        {
            ManagementObjectSearcher Searcher = new ManagementObjectSearcher( "Select * from Win32_ComputerSystem" );
            ManagementObjectCollection Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                Object Value;

                Value = Object.GetPropertyValue( "Name" );
                if( Value != null )
                {
                    MachineName = Value.ToString();
                }
                break;
            }
        }

        protected override void OnStart( string[] Args )
        {
            // Configure the remoting service
            ChannelServices.RegisterChannel( new TcpClientChannel(), true );

#if DEBUG
            IUPMS = ( IUPMS_Interface )Activator.GetObject( typeof( IUPMS_Interface ), "tcp://localhost:9090/UPMS" );
#else
            IUPMS = ( IUPMS_Interface )Activator.GetObject( typeof( IUPMS_Interface ), Properties.Settings.Default.RemotableHost );
#endif

            if( IUPMS == null )
            {
                Log.WriteLine( "UPDS", Log.LogType.Error, "No UPMS found - exiting!" );
                return;
            }

            CacheSystem.Init();
            IdleBuildsCacher.Init();
            TaskPoller.Init();

            GetMachineName();

            IUPMS.DistributionServer_Register( MachineName );

            string Message = "Started at: " + DateTime.Now.ToString( US_TIME_DATE_STRING_FORMAT ) + Environment.NewLine;
            long LocalCacheSize = CacheSystem.GetCacheSize() / ( 1024 * 1024 * 1024 );
            Message += "Local cache size is: " + LocalCacheSize.ToString() + " GB" + Environment.NewLine;
            Message += Environment.NewLine + "Cheers" + Environment.NewLine + "UnrealProp" + Environment.NewLine;
            IUPMS.Utils_SendEmail( -1, -1, "UPDS started on '" + MachineName + "'", Message, 1 );

            // Register application exception handler
            AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler( AppExceptionHandler );

            Log.WriteLine( "UPDS", Log.LogType.Important, "Service has been successfully started!" );
        }

        protected override void OnStop()
        {
            IUPMS.DistributionServer_Unregister( MachineName );

            TaskExecutor.StopAllTasks();
            IdleBuildsCacher.Release();

            Log.WriteLine( "UPDS", Log.LogType.Important, "Service has been stopped!" );
        }
    }

    public class Utils
    {
        static public long CopyFile( string SourceFileName, string DestFileName, ref string Error )
        {
            try
            {
                byte[] ReadBytes;
                long BufferSize = 10 * 1024 * 1024;
                FileStream SourceFileStream = null;
                FileStream DestFileStream = null;
                FileInfo SourceFileInfo = null;

                try
                {
                    FileInfo DestFile = new FileInfo( DestFileName );
                    if( DestFile.Exists )
                    {
                        DestFile.IsReadOnly = false;
                        DestFile.Delete();
                    }

                    DestFileStream = DestFile.Create();
                }
                catch( Exception Ex )
                {
                    Error = "Failed to create destination file: " + DestFileName + " with exception " + Ex.ToString();
                    return ( -1 );
                }

                try
                {
                    SourceFileStream = File.OpenRead( SourceFileName );
                }
                catch( Exception Ex )
                {
                    DestFileStream.Close();
                    Error = "Failed to open source file: " + SourceFileName + " with exception " + Ex.ToString();
                    return ( -1 );
                }

                try
                {
                    SourceFileInfo = new FileInfo( SourceFileStream.Name );
                }
                catch( Exception Ex )
                {
                    SourceFileStream.Close();
                    DestFileStream.Close();
                    Error = "Failed to get FileInfo on source file: " + SourceFileName + " with exception " + Ex.ToString();
                    return ( -1 );
                }

                long FileLength = SourceFileInfo.Length;
                long OriginalFileLength = FileLength;
                if( FileLength < BufferSize )
                {
                    BufferSize = FileLength;
                }

                if( BufferSize > 0 )
                {
                    ReadBytes = new byte[BufferSize];

                    while( FileLength > 0 )
                    {
                        if( FileLength < BufferSize )
                        {
                            BufferSize = FileLength;
                        }

                        try
                        {
                            SourceFileStream.Read( ReadBytes, 0, ( int )BufferSize );
                        }
                        catch( Exception Ex )
                        {
                            Error = "Stream read on: " + SourceFileName + " failed with exception " + Ex.ToString();
                            OriginalFileLength = -1;
                            break;
                        }

                        try
                        {
                            DestFileStream.Write( ReadBytes, 0, ( int )BufferSize );
                        }
                        catch( Exception Ex )
                        {
                            Error = "Stream write on: " + DestFileName + " failed with exception " + Ex.ToString();
                            OriginalFileLength = -1;
                            break;
                        }

                        FileLength -= BufferSize;
                    }
                }

                SourceFileStream.Close();
                DestFileStream.Close();

                return ( OriginalFileLength );
            }
            catch( Exception Ex )
            {
                Error = "Unhandled exception: " + Ex.ToString();
            }

            return ( -1 );
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
            string Format = DateTime.Now.ToString( UPDS_Service.US_TIME_DATE_STRING_FORMAT ) + " [" + Label + "] ";
            if( Type == LogType.Error )
            {
                Format += "ERROR!!! ";
            }
            string RawString = string.Format( Format + Message, Args );

            Trace.WriteLine( RawString );
            Console.WriteLine( RawString );

            if( LogFile == null )
            {
                string LogFilePath = "UPDS_" + DateTime.Now.ToString( "yyyy_MM_dd-HH_mm_ss" ) + ".txt";
                LogFile = new StreamWriter( LogFilePath, false, Encoding.ASCII );
            }

            LogFile.WriteLine( RawString );

            if( Type == LogType.Error )
            {
                // Suppress the disconnected warning
                if( RawString.Contains( "HRESULT: 0x82DA0101" ) )
                {
                    return;
                }

                UPDS_Service.IUPMS.Utils_SendEmail( -1, -1, "UPDS '" + UPDS_Service.MachineName + "' reported error!", RawString, 2 );
                LogFile.Flush();
                // Sleep 60 seconds after any error
                Thread.Sleep( 60 * 1000 );
            }
        }
    }
}

