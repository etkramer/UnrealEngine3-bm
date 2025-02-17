using System;
using System.Collections;
using System.Collections.Specialized;
using System.Drawing;
using System.Diagnostics;
using System.IO;

namespace Controller
{
    class BuildProcess
    {
        public bool IsFinished = false;

        private Main Parent;
        private ScriptParser Builder;
        private Process RunningProcess;
        private ERRORS ErrorLevel = ERRORS.None;
        private bool CaptureOutputDebugString = false;
        private StreamWriter Log = null;
        private string Executable;
        private string CommandLine;
        private int ExitCode;

        public ERRORS GetErrorLevel()
        {
            return ( ErrorLevel );
        }

        public int GetExitCode()
        {
            return ( ExitCode );
        }

        public BuildProcess( Main InParent, ScriptParser InBuilder, StreamWriter InLog, string InExecutable, string InCommandLine, string WorkingDirectory, bool InCaptureOutputDebugString )
        {
            Parent = InParent;
            Builder = InBuilder;
            Log = InLog;
            RunningProcess = new Process();
            Executable = InExecutable;
            CommandLine = InCommandLine;
            CaptureOutputDebugString = InCaptureOutputDebugString;
            ExitCode = 0;

            // Prepare a ProcessStart structure 
            RunningProcess.StartInfo.FileName = Executable;
            RunningProcess.StartInfo.Arguments = CommandLine;
            if( WorkingDirectory.Length > 0 )
            {
                RunningProcess.StartInfo.WorkingDirectory = WorkingDirectory;
            }
            else
            {
                RunningProcess.StartInfo.WorkingDirectory = Environment.CurrentDirectory;
            }

            // Redirect the output.
            RunningProcess.StartInfo.UseShellExecute = false;
            RunningProcess.StartInfo.RedirectStandardOutput = !CaptureOutputDebugString;
            RunningProcess.StartInfo.RedirectStandardError = !CaptureOutputDebugString;
            RunningProcess.StartInfo.CreateNoWindow = true;

            Parent.Log( "Spawning: " + Executable + " " + CommandLine + " (CWD: " + WorkingDirectory + ")", Color.DarkGreen );
            Builder.Write( Log, "Spawning: " + Executable + " " + CommandLine + " (CWD: " + WorkingDirectory + ")" );

            // Spawn the process - try to start the process, handling thrown exceptions as a failure.
            try
            {
                if( !CaptureOutputDebugString )
                {
                    RunningProcess.OutputDataReceived += new DataReceivedEventHandler( PrintLog );
                    RunningProcess.ErrorDataReceived += new DataReceivedEventHandler( PrintLog );
                }
                else
                {
#if !DEBUG
                    DebugMonitor.OnOutputDebugString += new OnOutputDebugStringHandler( CaptureDebugString );
                    DebugMonitor.Start();
#endif
                }
                RunningProcess.Exited += new EventHandler( ProcessExit );

                RunningProcess.Start();

                if( !CaptureOutputDebugString )
                {
                    RunningProcess.BeginOutputReadLine();
                    RunningProcess.BeginErrorReadLine();
                }
                RunningProcess.EnableRaisingEvents = true;
            }
            catch
            {
                RunningProcess = null;
                IsFinished = true;
                Parent.Log( "PROCESS ERROR: Failed to start: " + Executable, Color.Red );

                if( Log != null )
                {
                    Builder.Write( Log, "PROCESS ERROR: Failed to start: " + Executable );
                    Log.Close();
                }
#if !DEBUG
                if( CaptureOutputDebugString )
                {
                    CaptureOutputDebugString = false;
                    DebugMonitor.Stop();
                }
#endif

                ErrorLevel = ERRORS.Process;
            }
        }

        public BuildProcess( Main InParent, ScriptParser InBuilder, string InCommandLine, string WorkingDirectory )
        {
            Parent = InParent;
            Builder = InBuilder;
            RunningProcess = new Process();
            Executable = "Cmd.exe";
            CommandLine = InCommandLine;
            ExitCode = 0;

            // Prepare a ProcessStart structure 
            RunningProcess.StartInfo.FileName = Executable;
            RunningProcess.StartInfo.Arguments = CommandLine;
            if( WorkingDirectory.Length > 0 )
            {
                RunningProcess.StartInfo.WorkingDirectory = WorkingDirectory;
            }
            else
            {
                RunningProcess.StartInfo.WorkingDirectory = Environment.CurrentDirectory;
            }

            // Redirect the output.
            RunningProcess.StartInfo.UseShellExecute = true;
            RunningProcess.StartInfo.CreateNoWindow = true;

            Parent.Log( "Spawning: " + Executable + " " + CommandLine, Color.DarkGreen );

            // Spawn the process - try to start the process, handling thrown exceptions as a failure.
            try
            {
                RunningProcess.Exited += new EventHandler( ProcessExit );

                RunningProcess.Start();

                RunningProcess.EnableRaisingEvents = true;
            }
            catch
            {
                RunningProcess = null;
                IsFinished = true;
                Parent.Log( "PROCESS ERROR: Failed to start: " + Executable, Color.Red );

                if( Log != null )
                {
                    Builder.Write( Log, "PROCESS ERROR: Failed to start: " + Executable );
                    Log.Close();
                }
                ErrorLevel = ERRORS.Process;
            }
        }

        public void Cleanup()
        {
            if( Log != null )
            {
                Log.Close();
                Log = null;
            }

 #if !DEBUG
            if( CaptureOutputDebugString )
            {
                CaptureOutputDebugString = false;
                DebugMonitor.Stop();
            }
#endif
        }

        public void Kill()
        {
            Parent.Log( "Killing active processes ...", Color.Red );
            string Name = RunningProcess.ProcessName.ToLower();

            if( RunningProcess != null )
            {
                Parent.Log( " ... killing: '" + Executable + "'", Color.Red );
                RunningProcess.Kill();
            }

            // If we're running the com version, kill the exe too
            if( Name.IndexOf( ".com" ) >= 0 )
            {
                Name = Name.Replace( ".com", "" );
                Parent.KillProcess( Name );

                if( Name.ToLower() == "devenv" )
                {
                    Parent.KillProcess( "cl_Incredibuild_Interop_0" );
                    Parent.KillProcess( "cl" );
                }
            }

            Cleanup();
            IsFinished = true;
        }

        public bool IsResponding()
        {
            try
            {
                if( RunningProcess != null )
                {
                    return ( RunningProcess.Responding );
                }
            }
            catch
            {
            }

            return ( false );
        }

        public void ProcessExit( object Sender, System.EventArgs e )
        {
            ExitCode = RunningProcess.ExitCode;
            RunningProcess.EnableRaisingEvents = false;
            IsFinished = true;
        }

        public void CaptureDebugString( int PID, string Text )
        {
            if( Log != null && Text != null )
            {
                Builder.Write( Log, Text.Trim() );
            }
        }

        public void PrintLog( object Sender, DataReceivedEventArgs e )
        {
            if( Log != null )
            {
                string Line = e.Data;
                if( Line != null )
                {
                    Builder.Write( Log, Line );
                    Parent.CheckStatusUpdate( Line );
                }
            }
        }
    }
}

