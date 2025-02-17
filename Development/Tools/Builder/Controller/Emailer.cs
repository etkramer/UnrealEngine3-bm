using System;
using System.Collections.Generic;
using System.Drawing;
using System.Management;
using System.Net;
using System.Net.Mail;
using System.Text;
using Controller;

namespace Controller
{
    class Emailer
    {
        string MailServer = Properties.Settings.Default.MailServer;

        Main Parent;
        P4 SCC;

        public Emailer( Main InParent, P4 InP4 )
        {
            Parent = InParent;
            SCC = InP4;
            Parent.Log( "Emailer created successfully", Color.DarkGreen );
        }

        private string EmailAddressToPerforceUser( string Address )
        {
            string PerforceUser = Address;

            int AtIndex = Address.IndexOf( '@' );
            if( AtIndex > 0 )
            {
                PerforceUser = Address.Substring( 0, AtIndex );
                PerforceUser = PerforceUser.Replace( '.', '_' );
            }

            return ( PerforceUser );
        }

        private void SendSMS( string To, string Subject, string Body )
        {
        }

        private void SendMail( string To, string Subject, string Body, bool CCQA, bool CCEngineLeads, MailPriority Priority )
        {
#if DEBUG
            To = "john.scott@epicgames.com";
#endif
            try
            {
                SmtpClient Client = new SmtpClient( MailServer );
                if( Client != null )
                {
                    MailAddress AddrTo;

                    MailMessage Message = new MailMessage();

                    Message.From = new MailAddress( "build@epicgames.com" );
                    Message.Priority = Priority;

                    if( To.Length > 0 )
                    {
                        string[] Addresses = To.Split( ';' );
                        foreach( string Address in Addresses )
                        {
                            if( Address.Length > 0 )
                            {
                                AddrTo = new MailAddress( Address );
                                Message.To.Add( AddrTo );
                            }
                        }
                    }
#if !DEBUG
                    if( CCQA )
                    {
                        AddrTo = new MailAddress( "SupportQA@epicgames.com" );
                        Message.CC.Add( AddrTo );
                    }

                    if( CCEngineLeads )
                    {
                        AddrTo = new MailAddress( "Engine-Leads@epicgames.com" );
                        Message.CC.Add( AddrTo );

                        Message.ReplyTo = AddrTo;
                    }

                    AddrTo = new MailAddress( "john.scott@epicgames.com" );
                    Message.Bcc.Add( AddrTo );

                    AddrTo = new MailAddress( "derek.cornish@epicgames.com" );
                    Message.Bcc.Add( AddrTo );

					AddrTo = new MailAddress( "nick.atamas@epicgames.com" );
                    Message.Bcc.Add( AddrTo );
#endif
                    Message.Subject = Subject;
                    Message.Body = Body;
                    Message.IsBodyHtml = false;

                    Client.Send( Message );

                    Parent.Log( "Email sent to: " + To, Color.Orange );
                }
            }
            catch( Exception e )
            {
                Parent.Log( "Failed to send email to: " + To, Color.Orange );
                Parent.Log( "'" + e.Message + "'", Color.Orange );
            }
        }

        private string BuildTime( ScriptParser Builder )
        {
            StringBuilder Result = new StringBuilder();

            Result.Append( "'" + Builder.BuildDescription + "' started at " + Builder.BuildStartedAt );
            Result.Append( " and ended at " + Builder.BuildEndedAt );
            Result.Append( " taking " );

            TimeSpan Duration = Builder.BuildEndedAt - Builder.BuildStartedAt;
            if( Duration.Hours > 0 )
            {
                Result.Append( Duration.Hours.ToString() + " hour(s) " );
            }
            if( Duration.Hours > 0 || Duration.Minutes > 0 )
            {
                Result.Append( Duration.Minutes.ToString() + " minute(s) " );
            }
            Result.Append( Duration.Seconds.ToString() + " second(s)" + Environment.NewLine );

            return( Result.ToString() );
        }

        private string AddOperator( ScriptParser Builder, int CommandID, string To )
        {
            if( Builder.Operator.Length > 0 && Builder.Operator != "AutoTimer" && Builder.Operator != "LocalUser" )
            {
                if( To.Length > 0 )
                {
                    To += ";";
                }
                To += Builder.Operator + "@epicgames.com";
            }
            return ( To );
        }

        private string AddKiller( string Killer, int CommandID, string To )
        {
            if( Killer.Length > 0 && Killer != "LocalUser" )
            {
                if( To.Length > 0 )
                {
                    To += ";";
                }
                To += Killer + "@epicgames.com";
            }

            return ( To );
        }

        private string GetWMIValue( string Key, ManagementObject ManObject )
        {
            Object Value;

            try
            {
                Value = ManObject.GetPropertyValue( Key );
                if( Value != null )
                {
                    return ( Value.ToString() );
                }
            }
            catch
            {
            }

            return ( "" );
        }

        private string Signature()
        {
            return ( Environment.NewLine + "Cheers" + Environment.NewLine + Parent.MachineName + Environment.NewLine );
        }

        public void SendRestartedMail()
        {
            StringBuilder Body = new StringBuilder();

            string Subject = "[BUILDER] Builder synced and restarted!";

            Body.Append( "Controller compiled: " + Parent.CompileDateTime.ToString() + Environment.NewLine + Environment.NewLine );
            Body.Append( "MSVC version: " + Parent.MSVCVersion + Environment.NewLine );
            Body.Append( "DirectX version: " + Parent.DXVersion + Environment.NewLine );
            Body.Append( "XDK version: " + Parent.XDKVersion + Environment.NewLine );
            Body.Append( "PS3 SDK version: " + Parent.PS3SDKVersion + Environment.NewLine );
            Body.Append( Environment.NewLine + "Path: " + Environment.GetEnvironmentVariable( "PATH" ) + Environment.NewLine + Environment.NewLine );

            ManagementObjectSearcher Searcher = new ManagementObjectSearcher( "Select * from CIM_Processor" );
            ManagementObjectCollection Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                Body.Append( GetWMIValue( "Name", Object ) + Environment.NewLine );
                break;
            }

            Searcher = new ManagementObjectSearcher( "Select * from Win32_Processor" );
            Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                string NumberOfProcs = GetWMIValue( "NumberOfLogicalProcessors", Object );
                Body.Append( "Processors: " + NumberOfProcs + Environment.NewLine );
                if( NumberOfProcs.Length > 0 )
                {
                    // Best guess at number of processors
                    int NumProcessors = Int32.Parse( NumberOfProcs );
                    if( NumProcessors < 2 )
                    {
                        NumProcessors = 2;
                    }
                    Parent.NumProcessors = NumProcessors;

                    // Best guess at number of jobs to spawn
                    int NumJobs = ( NumProcessors * 3 ) / 2;
                    if( NumJobs < 2 )
                    {
                        NumJobs = 2;
                    }
                    else if( NumJobs > 8 )
                    {
                        NumJobs = 8;
                    }
                    Parent.NumJobs = NumJobs;
                }
                break;
            }

            Searcher = new ManagementObjectSearcher( "Select * from CIM_BIOSElement" );
            Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                Body.Append( GetWMIValue( "Name", Object ) + Environment.NewLine );
            }

            Body.Append( Environment.NewLine );

            Searcher = new ManagementObjectSearcher( "Select * from CIM_PhysicalMemory" );
            Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                string Capacity = GetWMIValue( "Capacity", Object );
                string Speed = GetWMIValue( "Speed", Object );
                Body.Append( "Memory: " + Capacity + " bytes at " + Speed + " MHz" + Environment.NewLine );
            }

            Body.Append( Environment.NewLine );

            Searcher = new ManagementObjectSearcher( "Select * from Win32_LogicalDisk" );
            Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                string DriveType = GetWMIValue( "DriveType", Object );
                if( DriveType == "3" )
                {
                    Int64 Size = 0;
                    Int64 FreeSpace = 0;

                    string Name = GetWMIValue( "Caption", Object );
                    string SizeInfo = GetWMIValue( "Size", Object );
                    string FreeSpaceInfo = GetWMIValue( "FreeSpace", Object );

                    try
                    {
                        Size = Int64.Parse( SizeInfo ) / ( 1024 * 1024 * 1024 );
                        FreeSpace = Int64.Parse( FreeSpaceInfo ) / ( 1024 * 1024 * 1024 );
                    }
                    catch
                    {
                    }

                    Body.Append( "'" + Name + "' hard disk: " + Size.ToString() + "GB (" + FreeSpace.ToString() + "GB free)" + Environment.NewLine );
                }
            }

            Body.Append( Signature() );

            SendMail( "john.scott@epicgames.com", Subject, Body.ToString(), false, false, MailPriority.Low );
        }

        public void SendTriggeredMail( ScriptParser Builder, int CommandID )
        {
            string To = AddOperator( Builder, CommandID, Builder.TriggerAddress );

            string Subject = "[BUILDER] You triggered a build!";
            StringBuilder Body = new StringBuilder();

            Body.Append( "Build type: '" + Builder.BuildDescription + "'" + Environment.NewLine );
            Body.Append( Signature() );

            SendMail( To, Subject, Body.ToString(), true, false, MailPriority.Low );
        }

        public void SendKilledMail( ScriptParser Builder, int CommandID, int BuildLogID, string Killer )
        {
            string To = AddOperator( Builder, CommandID, "" );
            To = AddKiller( Killer, CommandID, To );

            string Subject = "[BUILDER] '" + Builder.BuildDescription + "' KIA!";
            StringBuilder Body = new StringBuilder();

            Body.Append( "Build type: '" + Builder.BuildDescription );
            Body.Append( "' from changelist " + Builder.LabelInfo.Changelist + Environment.NewLine );
            Body.Append( "Build started at " + Builder.BuildStartedAt );
            Body.Append( " and ended at " + Builder.BuildEndedAt + Environment.NewLine );
            Body.Append( "Started by: " + Builder.Operator + Environment.NewLine );
            Body.Append( "Killed by: " + Killer + Environment.NewLine );
            Body.Append( Signature() );

            SendMail( To, Subject, Body.ToString(), true, false, MailPriority.High );
        }

        public void SendFailedMail( ScriptParser Builder, int CommandID, int BuildLogID, string FailureMessage, string LogFileName, string OptionalDistro )
        {
            // It's a job, so tag the failure message to the active label
            if( CommandID == 0 )
            {
                string JobStatus = Environment.NewLine + "Job failed on " + Parent.MachineName + ":" + Environment.NewLine;
                JobStatus += "Detailed log copied to: " + Properties.Settings.Default.FailedLogLocation.Replace( '/', '\\' ) + "\\" + LogFileName + Environment.NewLine + Environment.NewLine;
                JobStatus += FailureMessage;
                JobStatus += Environment.NewLine;

                SCC.TagMessage( Builder, JobStatus );
                return;
            }

            // It's a normal build script - send a mail as usual
            string To = AddOperator( Builder, CommandID, Builder.FailAddress + ";" + OptionalDistro );
            string Subject = "[BUILDER] '" + Builder.BuildDescription + "' failed!";

            StringBuilder Body = new StringBuilder();

            Body.Append( "Build type: '" + Builder.BuildDescription );
            Body.Append( "' from changelist " + Builder.LabelInfo.Changelist + Environment.NewLine );
            Body.Append( "Build started at " + Builder.BuildStartedAt );
            Body.Append( " and ended at " + Builder.BuildEndedAt + Environment.NewLine );
            Body.Append( Environment.NewLine + "Detailed log copied to: " + Properties.Settings.Default.FailedLogLocation.Replace( '/', '\\' ) + "\\" + LogFileName + Environment.NewLine );

            Body.Append( Environment.NewLine + FailureMessage + Environment.NewLine );
            Body.Append( Signature() );

            SendMail( To, Subject, Body.ToString(), true, false, MailPriority.High );
        }

        // Sends mail stating the version of the build that was just created
        public void SendSucceededMail( ScriptParser Builder, int CommandID, int BuildLogID, string FinalStatus )
        {
            string To = AddOperator( Builder, CommandID, Builder.SuccessAddress );

            string Label = Builder.LabelInfo.GetLabelName();
            string Subject = "[BUILDER] New '" + Builder.BuildDescription + "' build! (" + Label + ")";
            StringBuilder Body = new StringBuilder();

            Body.Append( BuildTime( Builder ) );

            if( Builder.CreateLabel )
            {
                if( Label.Length > 0 )
                {
                    Body.Append( Environment.NewLine + "Build is labeled '" + Label + "'" + Environment.NewLine + Environment.NewLine );
                }
            }

            Body.Append( Environment.NewLine + FinalStatus + Environment.NewLine );
            Body.Append( Signature() );

            SendMail( To, Subject, Body.ToString(), true, false, MailPriority.Normal );
        }

        // Sends mail stating the version of the build that was used to create the data
        public void SendPromotedMail( ScriptParser Builder, int CommandID )
        {
            string To = AddOperator( Builder, CommandID, Builder.SuccessAddress );

            string Subject = "[BUILDER] '" + Builder.BuildDescription + "' Promoted! (" + Builder.LabelInfo.GetLabelName() + ")";
            StringBuilder Body = new StringBuilder();

            string Label = Builder.LabelInfo.GetLabelName();
            if( Label.Length > 0 )
            {
                Body.Append( Environment.NewLine + "The build labeled '" + Label + "' was promoted (details below)" + Environment.NewLine + Environment.NewLine );
            }

            Body.Append( Environment.NewLine + Builder.GetChanges( "" ) + Environment.NewLine );
            Body.Append( Signature() );

            SendMail( To, Subject, Body.ToString(), true, false, MailPriority.Normal );
        }

        // Sends mail stating the changes on a per user basis
        public void SendUserChanges( ScriptParser Builder, string User )
        {
            string Subject = "'QA Build Changes' (" + Builder.LabelInfo.GetLabelName() + ")";
            StringBuilder Body = new StringBuilder();

            Body.Append( "----BUNs----" + Environment.NewLine );
            Body.Append( "Stuff licensees should know prior to integrating this build or should know in order to continue developing with this build." + Environment.NewLine );
            Body.Append( "* Here's an entirely new tool/feature" + Environment.NewLine );
            Body.Append( "* This change requires a re-save or re-compile" + Environment.NewLine );
            Body.Append( "* This subtle change might be tricky to merge" + Environment.NewLine );
            Body.Append( "* There has been a major upgrade or change to this tool/feature/procedure" + Environment.NewLine );
            Body.Append( Environment.NewLine );
            Body.Append( "----SUMMARY----" + Environment.NewLine );
            Body.Append( "Please start all lines in the summary with uppercase letters and try to use the grouping outlined below to add your changes." + Environment.NewLine );
            Body.Append( Environment.NewLine );
            Body.Append( "General:" + Environment.NewLine );
            Body.Append( "   * Engine" + Environment.NewLine );
            Body.Append( "      * Misc" + Environment.NewLine );
            Body.Append( "         * This is an example of a general bug fix in the engine" + Environment.NewLine );
            Body.Append( "      * Animation" + Environment.NewLine );
            Body.Append( "      * Audio" + Environment.NewLine );
            Body.Append( "      * Cooker" + Environment.NewLine );
            Body.Append( "      * Live" + Environment.NewLine );
            Body.Append( "      * Loc" + Environment.NewLine );
            Body.Append( "      * Optimizations" + Environment.NewLine );
            Body.Append( "      * Particles" + Environment.NewLine );
            Body.Append( "      * Physics" + Environment.NewLine );
            Body.Append( "      * Script Compiler" + Environment.NewLine );
            Body.Append( "   * Rendering" + Environment.NewLine );
            Body.Append( "      * Misc" + Environment.NewLine );
            Body.Append( "      * Optimizations" + Environment.NewLine );
            Body.Append( "   * Editor" + Environment.NewLine );
            Body.Append( "      * Misc" + Environment.NewLine );
            Body.Append( "      * AnimSet Viewer" + Environment.NewLine );
            Body.Append( "      * Kismet" + Environment.NewLine );
            Body.Append( "      * Matinee" + Environment.NewLine );
            Body.Append( "      * Preferences" + Environment.NewLine );
            Body.Append( "      * Sentinel" + Environment.NewLine );
            Body.Append( "      * UI Editor" + Environment.NewLine );
            Body.Append( "      * Viewports" + Environment.NewLine );
            Body.Append( "   * Tools" + Environment.NewLine );
            Body.Append( "      * Build system" + Environment.NewLine );
            Body.Append( "      * CIS" + Environment.NewLine );
            Body.Append( "      * UnrealConsole" + Environment.NewLine );
            Body.Append( "      * UnrealDatabaseProxy" + Environment.NewLine );
            Body.Append( "      * UnrealFrontend" + Environment.NewLine );
            Body.Append( "      * UnrealProp" + Environment.NewLine );
            Body.Append( "   * UI" + Environment.NewLine );	
            Body.Append( "      * Misc" + Environment.NewLine );
            Body.Append( "      * Data stores" + Environment.NewLine );
            Body.Append( "      * UI Editor" + Environment.NewLine );
            Body.Append( "" + Environment.NewLine );
            Body.Append( "Xbox 360:" + Environment.NewLine );
            Body.Append( "   * Rendering" + Environment.NewLine );
            Body.Append( "   * Misc" + Environment.NewLine );
            Body.Append( Environment.NewLine );
            Body.Append( "PS3:" + Environment.NewLine );
            Body.Append( "   * Rendering" + Environment.NewLine );
            Body.Append( "   * Misc" + Environment.NewLine );

            string Label = Builder.LabelInfo.GetLabelName();
            if( Label.Length > 0 )
            {
                Body.Append( Environment.NewLine + "The build labeled '" + Label + "' is the QA build (details below)" + Environment.NewLine + Environment.NewLine );
            }

            string PerforceUser = EmailAddressToPerforceUser( User );
            Body.Append( Environment.NewLine + Builder.GetChanges( PerforceUser ) + Environment.NewLine );
            Body.Append( Signature() );

            SendMail( User, Subject, Body.ToString(), true, true, MailPriority.Normal );
        }

        // Send a mail stating a cook has been copied to the network
        public void SendPublishedMail( ScriptParser Builder, int CommandID, int BuildLogID )
        {
            string To = AddOperator( Builder, CommandID, Builder.SuccessAddress );

            string Subject = "[BUILDER] '" + Builder.BuildDescription + "' Published! (" + Builder.LabelInfo.GetFolderName( false ) + ")";
            StringBuilder Body = new StringBuilder();

            Body.Append( BuildTime( Builder ) );

            List<string> Dests = Builder.GetPublishDestinations();
            if( Dests.Count > 0 )
            {
                Body.Append( Environment.NewLine + "Build was published to -" + Environment.NewLine );
                foreach( string Dest in Dests )
                {
                    Body.Append( "\t" + Dest + Environment.NewLine );
                }
            }

            string Label = Builder.SyncedLabel;
            if( Label.Length > 0 )
            {
                Body.Append( Environment.NewLine + "The build labeled '" + Label + "' was used to cook the data (details below)" + Environment.NewLine + Environment.NewLine );
            }

            Body.Append( Environment.NewLine + Builder.GetChanges( "" ) + Environment.NewLine );
            Body.Append( Signature() );

            SendMail( To, Subject, Body.ToString(), true, false, MailPriority.Normal );
        }

        public void SendMakingInstallMail( ScriptParser Builder, int CommandID, int BuildLogID )
        {
            string To = AddOperator( Builder, CommandID, Builder.SuccessAddress );

            string Subject = "[BUILDER] '" + Builder.BuildDescription + "' installable made! (" + Builder.Dependency + "_Install)";
            StringBuilder Body = new StringBuilder();

            Body.Append( BuildTime( Builder ) );

            Body.Append( Environment.NewLine + "Install files were copied to -" + Environment.NewLine );
            Body.Append( "\t'" + Builder.LabelInfo.GetFolderName( true ) + "_Install'" + Environment.NewLine );

            string Label = Builder.LabelInfo.GetLabelName();
            if( Label.Length > 0 )
            {
                Body.Append( Environment.NewLine + "The build labeled '" + Label + "' was used to cook the data (details below)" + Environment.NewLine + Environment.NewLine );
            }

            Body.Append( Environment.NewLine + Builder.GetChanges( "" ) + Environment.NewLine );
            Body.Append( Signature() );

            SendMail( To, Subject, Body.ToString(), true, false, MailPriority.Normal );
        }

        public void SendUpToDateMail( ScriptParser Builder, int CommandID, string FinalStatus )
        {
            string To = AddOperator( Builder, CommandID, "" );

            string Subject = "[BUILDER] '" + Builder.BuildDescription + "' already up to date!";
            StringBuilder Body = new StringBuilder();

            Body.Append( Environment.NewLine + FinalStatus + Environment.NewLine );
            Body.Append( Signature() );

            SendMail( To, Subject, Body.ToString(), false, false, MailPriority.Normal );
        }

        public void SendAlreadyInProgressMail( string Operator, int CommandID, string BuildType )
        {
            // Don't send in progress emails for CIS type builds
            if( BuildType.StartsWith( "CIS" ) )
            {
                return;
            }

            string To = Operator + "@epicgames.com";
            string Subject = "[BUILDER] '" + BuildType + "' is already building!";
            StringBuilder Body = new StringBuilder();

            string FinalStatus = "Build '" + BuildType + "' not retriggered because it is already building.";

            Body.Append( Environment.NewLine + FinalStatus + Environment.NewLine );
            Body.Append( Signature() );

            SendMail( To, Subject, Body.ToString(), false, false, MailPriority.Normal );
        }

        public void SendStatusMail( ScriptParser Builder, int CommandID, string FinalStatus )
        {
            string To = AddOperator( Builder, CommandID, Builder.SuccessAddress );

            string Subject = "[BUILDER] Drive status";
            StringBuilder Body = new StringBuilder();

            Body.Append( "Build storage summary" + Environment.NewLine );
            Body.Append( Environment.NewLine + FinalStatus + Environment.NewLine );
            Body.Append( Signature() );

            SendMail( To, Subject, Body.ToString(), true, false, MailPriority.Normal );
        }

        public void SendGlitchMail()
        {
            string To = "john.scott@epicgames.com";

            string Subject = "[BUILDER] NETWORK GLITCH";
            StringBuilder Body = new StringBuilder();

            Body.Append( Environment.NewLine + "Network diagnostics" + Environment.NewLine + Environment.NewLine );

            ManagementObjectSearcher Searcher = new ManagementObjectSearcher( "Select * from Win32_NetworkConnection" );
            ManagementObjectCollection Collection = Searcher.Get();

            foreach( ManagementObject Object in Collection )
            {
                Body.Append( GetWMIValue( "LocalName", Object ) + Environment.NewLine );
                Body.Append( GetWMIValue( "Name", Object ) + Environment.NewLine );
                Body.Append( GetWMIValue( "RemoteName", Object ) + Environment.NewLine );
                Body.Append( GetWMIValue( "ProviderName", Object ) + Environment.NewLine );
                Body.Append( GetWMIValue( "ResourceType", Object ) + Environment.NewLine );

                Body.Append( GetWMIValue( "Caption", Object ) + Environment.NewLine );
                Body.Append( GetWMIValue( "ConnectionState", Object ) + Environment.NewLine );
                Body.Append( GetWMIValue( "ConnectionType", Object ) + Environment.NewLine );
                Body.Append( GetWMIValue( "DisplayType", Object ) + Environment.NewLine );
                Body.Append( GetWMIValue( "Status", Object ) + Environment.NewLine );
                Body.Append( Environment.NewLine );
            }

            Body.Append( Signature() );

            SendMail( To, Subject, Body.ToString(), true, false, MailPriority.High );
        }

        public void RedFlash( ScriptParser Builder )
        {
            string To = Builder.CommandLine;
            string Body = "Please check your email!";
            SendSMS( To, "RED FLASH!", Body );
        }
    }
}
