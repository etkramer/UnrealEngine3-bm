using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace Controller
{
    class LogParser
    {
        private ScriptParser Builder = null;
        private StreamReader Log = null;
        private string LastProject;
        private string FinalError;
        private bool FoundAnyError = false;
        private bool FoundError = false;

        public LogParser( ScriptParser InBuilder )
        {
            Builder = InBuilder;

            try
            {
                Log = new StreamReader( Builder.LogFileName );
            }
            catch
            {
            }
        }

        public string Parse( bool ReportEntireLog, bool CheckCookingSuccess, bool CheckCookerSyncSuccess, ref ERRORS ErrorLevel )
        {
            string Line;
            int LinesToGrab = 0;

            if( Log == null )
            {
                return ( "Failed to open log: '" + Builder.LogFileName + "'" );
            }

            // Read in the entire log file
            Line = Log.ReadLine();
            while( Line != null )
            {
                // Check for a project status line
                if( Line.IndexOf( "------" ) >= 0 || Line.IndexOf( "Entering directory" ) >= 0 )
                {
                    LastProject = Line;
                    FoundError = false;
                }
                // Grab any extra lines if we have been told to
                else if( LinesToGrab > 0 )
                {
                    FinalError += Line + Environment.NewLine;
                    LinesToGrab--;
                }
                else if( Line.IndexOf( "M: make" ) >= 0 )
                {
                    FinalError += Line + Environment.NewLine;
                }
                else if( CheckCookingSuccess && ErrorLevel == ERRORS.None &&
                         ( Line.IndexOf( "Success - 0 error(s)" ) >= 0 ) )
                {
                    ErrorLevel = ERRORS.CookingSuccess;
                }
                else if( CheckCookerSyncSuccess && ErrorLevel == ERRORS.None &&
                         ( Line.IndexOf( "[SYNCING WITH FOLDERS FINISHED]" ) >= 0 ) )
                {
                    ErrorLevel = ERRORS.CookerSyncSuccess;
                }                
                // Check for errors
                else if( Builder.GetCheckErrors() &&
                         ( Line.IndexOf( " : error" ) >= 0
                         || Line.IndexOf( "Error:" ) >= 0
                         || Line.IndexOf( ": error:" ) >= 0
                         || Line.IndexOf( ": error C" ) >= 0
                         || Line.IndexOf( "cl : Command line error" ) >= 0
                         || Line.IndexOf( "SYMSTORE ERROR:" ) >= 0
                         || Line.IndexOf( "PROCESS ERROR:" ) >= 0
                         || Line.IndexOf( ": fatal error" ) >= 0
                         || Line.IndexOf( "] Error" ) >= 0
                         || Line.IndexOf( "is not recognized as an internal or external command" ) >= 0
                         || Line.IndexOf( "Parameter format not correct" ) >= 0
                         || Line.IndexOf( "internal compiler error" ) >= 0
                         || Line.IndexOf( "error MS" ) >= 0
                         || Line.IndexOf( "Critical: appError" ) >= 0
                         || Line.IndexOf( "The system cannot find the path specified" ) >= 0 ) )
                {
                    if( !FoundError )
                    {
                        FinalError += LastProject + Environment.NewLine;
                    }
                    FinalError += Line + Environment.NewLine;
                    FoundError = true;
                    FoundAnyError = true;
                }
                // Check for script compile errors
                else if( Builder.GetCheckErrors() &&
                         ( Line.IndexOf( "warning treated as error" ) >= 0
                         || Line.IndexOf( "warnings being treated as errors" ) >= 0 ) )
                {
                    if( !FoundError )
                    {
                        FinalError += LastProject + Environment.NewLine;
                    }
                    FinalError += Line + Environment.NewLine;
                    FoundError = true;
                    FoundAnyError = true;
                    LinesToGrab = 6;
                }
                // Check for script compile errors
                else if( Builder.GetCheckErrors() &&
                         ( Line.IndexOf( "Error," ) >= 0 ) )
                {
                    if( !FoundError )
                    {
                        FinalError += LastProject + Environment.NewLine;
                    }
                    FinalError += Line + Environment.NewLine;
                    FoundError = true;
                    FoundAnyError = true;
                }
                // Check for UBT errors
                else if( Builder.GetCheckErrors() &&
                         ( Line.IndexOf( "UnrealBuildTool.BuildException:" ) >= 0 )
                         || Line.IndexOf( "UnrealBuildTool error:" ) >= 0 )
                {
                    FinalError += Line + Environment.NewLine;
                    FoundError = true;
                    FoundAnyError = true;
                    LinesToGrab = 10;
                }

                // Check for app crashing
                else if( Builder.GetCheckErrors() &&
                    ( Line.IndexOf( "=== Critical error: ===" ) >= 0 ) )
                {
                    FinalError += Line + Environment.NewLine;
                    FoundError = true;
                    FoundAnyError = true;
                    // Grab start of callstack
                    LinesToGrab = 10;
                }
                // Check for app errors
                else if( Builder.GetCheckErrors() &&
                         ( Line.IndexOf( ": Failure -" ) >= 0
                         || Line.IndexOf( "appError" ) >= 0 ) )
                {
                    FinalError += Line + Environment.NewLine;
                    FoundError = true;
                    FoundAnyError = true;
                    LinesToGrab = 2;
                }
                // Check for app errors
                else if( Builder.GetCheckErrors() &&
                         ( Line.IndexOf( "The following files were specified on the command line:" ) >= 0 ) )
                         
                {
                    FinalError += Line + Environment.NewLine;
                    FoundError = true;
                    FoundAnyError = true;
                    LinesToGrab = 4;
                }
                // Check for CookerSync fails
                else if( Builder.GetCheckErrors() &&
                         ( Line.IndexOf( ": Exception was" ) >= 0
                         || Line.IndexOf( "==> " ) >= 0 ) )
                {
                    FinalError += Line + Environment.NewLine;
                    FoundError = true;
                    FoundAnyError = true;
                    LinesToGrab = 1;
                }
                // Check for P4 sync errors
                else if( Builder.GetCheckErrors() &&
                         ( Line.IndexOf( "can't edit exclusive file already opened" ) >= 0 ) )
                {
                    FinalError += Line + Environment.NewLine;
                    FoundError = true;
                    FoundAnyError = true;
                    LinesToGrab = 1;

                    ErrorLevel = ERRORS.SCC_Checkout;
                }
                // Check for MSVC compile and link warnings
                else if( Builder.GetCheckWarnings() &&
                         ( Line.IndexOf( " : warning" ) >= 0
                         || Line.IndexOf( ": => " ) >= 0
                         || Line.IndexOf( ": warning:" ) >= 0 ) )
                {
                    if( !FoundError )
                    {
                        FinalError += LastProject + Environment.NewLine;
                    }
                    FinalError += Line + Environment.NewLine;
                    FoundError = true;
                    FoundAnyError = true;
                }
                else if( ReportEntireLog )
                {
                    FoundAnyError = true;
                    if( Line.Length > 0 )
                    {
                        FinalError += Line + Environment.NewLine;
                    }
                }

                Line = Log.ReadLine();
            }

            Log.Close();

            if( FoundAnyError )
            {
                return ( FinalError );
            }

            return ( "Succeeded" );
        }
    }
}
