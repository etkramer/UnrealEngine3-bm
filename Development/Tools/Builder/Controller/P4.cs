using System;
using System.Collections.Generic;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Management;
using System.Text;
using Interop.p4com;

namespace Controller
{
    class P4
    {
        private Main Parent;
        private Interop.p4com.p4 Interface = null;
        private ERRORS ErrorLevel = ERRORS.None;
#if DEBUG
        private bool ClientSpecValidated = true;
#else
        private bool ClientSpecValidated = false;
#endif
        private string CurrentWorkingDirectory = "";
        private string SubmitFileErrorList = "";

        public P4( Main InParent )
        {
            Parent = InParent;
            try
            {
                Interface = new Interop.p4com.p4();
            }
            catch
            {
                Parent.Log( "Perforce connection FAILED", Color.Red );
                Interface = null;
                Parent.Ticking = false;
            }
        }

        public ERRORS GetErrorLevel()
        {
            return ( ErrorLevel );
        }

        public void Init()
        {
            ClientSpecValidated = false;
        }

        private void Write( ScriptParser Builder, StreamWriter Log, Array Output )
        {
            foreach( string Line in Output )
            {
                Builder.Write( Log, Line );
            }
        }

        private bool ValidateClientSpec( ScriptParser Builder )
        {
            if( !ClientSpecValidated )
            {
                string ClientSpec = Builder.GetClientSpec();
                string P4Folder = GetClientRoot( ClientSpec ) + "\\" + Builder.SourceControlBranch;
                CurrentWorkingDirectory = Environment.CurrentDirectory;

                if( CurrentWorkingDirectory.ToLower() != P4Folder.ToLower() )
                {
                    Parent.Log( "P4ERROR: ClientSpec '" + ClientSpec + "' does not operate on '" + CurrentWorkingDirectory + "'", Color.Red );
                    return ( false );
                }

                ClientSpecValidated = true;
            }

            return ( ClientSpecValidated );
        }

        public bool BranchExists( string Branch )
        {
            string P4Folder = GetClientRoot( "" ) + "\\" + Branch;
            DirectoryInfo Dir = new DirectoryInfo( P4Folder );
            return ( Dir.Exists );
        }

        private string GetClientRoot( string ClientSpec )
        {
            Array Output;
            string RootFolder = "<Unknown>";

            ErrorLevel = ERRORS.None;
            Interface.Connect();

            Interface.Client = ClientSpec;
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 1;
            try
            {
                Output = Interface.run( "info" );

                foreach( string Line in Output )
                {
                    int OffsetClient = Line.ToLower().IndexOf( "client" );
                    int OffsetRoot = Line.ToLower().IndexOf( "root" );

                    if( OffsetClient >= 0 && OffsetRoot >= 0 )
                    {
                        // First space after the root is the root folder
                        RootFolder = Line.Substring( Line.IndexOf( ' ', OffsetRoot + "root".Length ) ).Trim();
                        break;
                    }
                }
            }
            catch( System.Runtime.InteropServices.COMException )
            {
                ErrorLevel = ERRORS.SCC_GetClientRoot;
            }

            Interface.Disconnect();

            return ( RootFolder );
        }

        public void CheckConsistency( ScriptParser Builder, StreamWriter Log, string FileSpec )
        {
            Array Output;
            string Command;

            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }

            Command = "diff -se //depot/" + Builder.SourceControlBranch + "/" + FileSpec + "/...";

            ErrorLevel = ERRORS.None;
            Interface.Connect();

            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 1;
            try
            {
                Builder.Write( Log, "Executing 'P4 " + Command + "'" );
                Output = Interface.run( Command );
                Write( Builder, Log, Output );

                if( Output.Length > 0 )
                {
                    foreach( string File in Output )
                    {
                        Builder.Write( Log, "Error: Depot version of '" + File + "' is inconsistent with local version!" );
                    }
                }
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR: Checkconsistency " + ex.Message );
                ErrorLevel = ERRORS.SCC_CheckConsistency;
            }

            Interface.Disconnect();
        }

        public void SyncToRevision( ScriptParser Builder, StreamWriter Log, string Revision, string FileSpec )
        {
            Array Output;
            string Command;

            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }

            Command = "sync //depot/" + Builder.SourceControlBranch + "/" + FileSpec + Revision;

            ErrorLevel = ERRORS.None;
            Interface.Connect();

            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 1;
            try
            {
                Builder.Write( Log, "Executing 'P4 " + Command + "'" );
                Output = Interface.run( Command );
                Write( Builder, Log, Output );
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR: Sync " + ex.Message );
                ErrorLevel = ERRORS.SCC_Sync;
            }

            Interface.Disconnect();
        }

        public void SyncSingleChangeList( ScriptParser Builder, StreamWriter Log )
        {
            Array Output;
            string Command;

            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }

            string ChangeList = Builder.CommandLine.Trim();
            Command = "sync //depot/" + Builder.SourceControlBranch + "/...@" + ChangeList + ",@" + ChangeList;

            ErrorLevel = ERRORS.None;
            Interface.Connect();

            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 1;
            try
            {
                Builder.Write( Log, "Executing 'P4 " + Command + "'" );
                Output = Interface.run( Command );
                Write( Builder, Log, Output );
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR: SyncSingleChangeList " + ex.Message );
                ErrorLevel = ERRORS.SCC_SyncSingleChangeList;
            }

            Interface.Disconnect();
        }

        public bool GetIncorrectCheckedOutFiles( ScriptParser Builder )
        {
            Array OpenedOutput;
            Array FStatOutput;
            SubmitFileErrorList = "";
            bool NeedResolve = false;

            ErrorLevel = ERRORS.None;
            Interface.Connect();

            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 1;
            try
            {
                OpenedOutput = Interface.run( "opened" );

                foreach( string Line in OpenedOutput )
                {
                    bool OtherLocked = false;

                    if( Line.ToLower().StartsWith( "depotfile " ) )
                    {
                        string File = Line.Substring( "depotfile ".Length );

                        FStatOutput = Interface.run( "fstat " + File );

                        int HaveRev = 0;
                        int HeadRev = 0;
                        int OtherOpenCount = 0;
                        foreach( string Stat in FStatOutput )
                        {
                            if( Stat.ToLower().StartsWith( "headrev " ) )
                            {
                                HeadRev = Builder.SafeStringToInt( Stat.Substring( "headrev ".Length ) );
                            }
                            if( Stat.ToLower().StartsWith( "haverev " ) )
                            {
                                HaveRev = Builder.SafeStringToInt( Stat.Substring( "haverev ".Length ) );
                            }
                            if( Stat.ToLower().StartsWith( "otheropen " ) )
                            {
                                OtherOpenCount = Builder.SafeStringToInt( Stat.Substring( "otheropen ".Length ) );
                            }
                            if( Stat.ToLower().StartsWith( "otherlock" ) )
                            {
                                OtherLocked = true;
                            }
                        }

                        if( HaveRev != 0 && HeadRev != 0 )
                        {
                            if( HaveRev != HeadRev )
                            {
                                SubmitFileErrorList += "File not at head revision: '" + File + "'" + Environment.NewLine;
                                NeedResolve = true;
                            }
                        }

                        if( OtherOpenCount > 0 )
                        {
                            if( OtherLocked )
                            {
                                SubmitFileErrorList += "File '" + File + "' is *LOCKED* by:" + Environment.NewLine;

                                for( int i = 0; i < OtherOpenCount; i++ )
                                {
                                    string Key = "otherlock" + i.ToString() + " ";
                                    foreach( string Stat in FStatOutput )
                                    {
                                        if( Stat.ToLower().StartsWith( Key ) )
                                        {
                                            SubmitFileErrorList += "\t" + Stat.Substring( Key.Length ) + Environment.NewLine;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Parent.Log( "P4ERROR: Getting checked out files " + ex.Message, Color.Red );
                ErrorLevel = ERRORS.SCC_Submit;
            }

            Interface.Disconnect();

            return ( NeedResolve );
        }

        public string GetIncorrectCheckedOutFiles()
        {
            return ( SubmitFileErrorList );
        }

        public void SyncBuildScripts( string SourceBranch, string FileSpec )
        {
            Array Output;
            string Command;

            Command = "sync //depot/" + SourceBranch + FileSpec;

            ErrorLevel = ERRORS.None;
            Interface.Connect();

            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 1;
            try
            {
                Output = Interface.run( Command );
                Parent.Log( Output, Color.DarkGreen );
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Parent.Log( "P4ERROR: Sync " + ex.Message, Color.Red );
                ErrorLevel = ERRORS.SCC_Sync;
            }

            Interface.Disconnect();
        }

        public void Revert( ScriptParser Builder, StreamWriter Log, string FileSpec )
        {
#if !DEBUG
            Array Output;
#endif
            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }

            ErrorLevel = ERRORS.None;
            Interface.Connect();

            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 1;
            try
            {
                Builder.Write( Log, "Executing 'P4 revert " + FileSpec + "'" );
#if !DEBUG
                Output = Interface.run( "revert " + FileSpec );
                Write( Builder, Log, Output );
#endif
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR: Revert " + ex.Message );
                ErrorLevel = ERRORS.SCC_Revert;
            }

            Interface.Disconnect();
        }

        public bool CheckoutFileSpec( ScriptParser Builder, StreamWriter Log, string FileSpec, bool Lock )
        {
            Array Output;
            bool Success = false;

            if( !ValidateClientSpec( Builder ) )
            {
                return( Success );
            }

            ErrorLevel = ERRORS.None;
            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 2;

            try
            {
                Builder.Write( Log, "Executing 'P4 edit " + FileSpec + "'" );
                Output = Interface.run( "edit " + FileSpec );
                Write( Builder, Log, Output );

                if( Lock )
                {
                    Builder.Write( Log, "Executing 'P4 lock " + FileSpec + "'" );
                    Output = Interface.run( "lock " + FileSpec );
                    Write( Builder, Log, Output );
                }

                Success = true;
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR: Edit (& Lock) " + ex.Message );
                ErrorLevel = ERRORS.SCC_Checkout;
            }

            Interface.Disconnect();
            return ( Success );
        }

        public bool OpenForDeleteFileSpec( ScriptParser Builder, StreamWriter Log, string FileSpec )
        {
            Array Output;
            bool Success = false;

            if( !ValidateClientSpec( Builder ) )
            {
                return ( Success );
            }

            ErrorLevel = ERRORS.None;
            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 2;

            try
            {
                Builder.Write( Log, "Executing 'P4 delete " + FileSpec + "'" );
                Output = Interface.run( "delete " + FileSpec );
                Write( Builder, Log, Output );

                Success = true;
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR: Delete " + ex.Message );
                ErrorLevel = ERRORS.SCC_OpenForDelete;
            }

            Interface.Disconnect();
            return ( Success );
        }

        public void Resolve( StreamWriter Log, ScriptParser Builder )
        {
            Array Output;

            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }
            
            ErrorLevel = ERRORS.None;

            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 2;

            try
            {
                // Auto resolve all conflicts to accept local versions
                Builder.Write( Log, "Executing 'P4 resolve -ay'" );
                Output = Interface.run( "resolve -ay" );
                Write( Builder, Log, Output );
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR: Resolve " + ex.Message );
                ErrorLevel = ERRORS.SCC_Resolve;
            }

            Interface.Disconnect();
        }

        public int Submit( StreamWriter Log, ScriptParser Builder, int ChangeList )
        {
            Array Output;
            Array SubmittedFiles = new string[] { };

            if( !ValidateClientSpec( Builder ) )
            {
                return( 0 );
            }

            ErrorLevel = ERRORS.None;

            Interface.SetProtocol( "api", "61" );

            Interface.ParseForms();
            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 2;

            try
            {
                // Revert unchanged files
                Builder.Write( Log, "Executing 'P4 revert -a'" );
                Output = Interface.run( "revert -a" );
                Write( Builder, Log, Output );

                // Set the changelist description
                Builder.Write( Log, "Executing 'P4 change -o'" );
                Output = Interface.run( "change -o" );
                Write( Builder, Log, Output );

                Interface.set_Var( "Description", "[BUILDER] '" + Builder.LabelInfo.BuildType + "' built from changelist " + Builder.LabelInfo.Changelist.ToString() );

                if( ChangeList == 0 )
                {
                    // Create a new changelist
                    Builder.Write( Log, "Executing 'P4 change -i'" );
                    Output = Interface.run( "change -i" );
                    Write( Builder, Log, Output );

                    if( Output.Length > 0 )
                    {
                        string CreatedChangeList = ( string )Output.GetValue( 0 );
                        if( CreatedChangeList.IndexOf( "created with" ) >= 0 )
                        {
                            if( CreatedChangeList.Length > "Change ".Length )
                            {
                                CreatedChangeList = CreatedChangeList.Substring( "Change ".Length, CreatedChangeList.IndexOf( ' ' ) );

                                ChangeList = Builder.SafeStringToInt( CreatedChangeList );
                            }
                        }
                    }
                }

                string CL = ChangeList.ToString();
                if( ChangeList != 0 )
                {
                    Builder.Write( Log, "Executing 'P4 submit -i -c " + CL + "'" );
                    SubmittedFiles = Interface.run( "submit -i -c " + CL );
                    Write( Builder, Log, Output );
                }
#if !DEBUG
                if( Builder.NewLabelCreated )
                {
                    foreach( string Line in SubmittedFiles )
                    {
                        if( Line.StartsWith( "edit " ) )
                        {
                            string File = Line.Substring( "edit ".Length );
                            Builder.Write( Log, "Executing 'P4 labelsync -l " + Builder.LabelInfo.GetLabelName() + " " + File + "'" );
                            Output = Interface.run( "labelsync -l " + Builder.LabelInfo.GetLabelName() + " " + File );
                            Write( Builder, Log, Output );
                        }
                    }

                }
                else if( Builder.CreateLabel )
                {
                    // Set all the vars from an arbitrary label (and clear out the submit vars)
                    Builder.Write( Log, "Executing 'P4 label -o QA_APPROVED_CURRENT_BUILD'" );
                    Output = Interface.run( "label -o QA_APPROVED_CURRENT_BUILD" );
                    Write( Builder, Log, Output );

                    Interface.set_Var( "Label", Builder.LabelInfo.GetLabelName() );
                    Parent.Log( "Fullname: " + Builder.LabelInfo.GetLabelName(), Color.DarkGreen );

                    Builder.LabelInfo.CreateLabelDescription();

                    Interface.set_Var( "Description", Builder.LabelInfo.Description );
                    Interface.set_Var( "Options", "unlocked" );

                    string[] Files = new string[1];
                    Files[0] = "//depot/" + Builder.SourceControlBranch + "/...";
                    Interface.set_ArrayVar( "View", ref Files );

                    Builder.Write( Log, "Executing 'P4 label -i'" );
                    Output = Interface.run( "label -i" );
                    Write( Builder, Log, Output );

                    Builder.Write( Log, "Executing 'P4 labelsync -l " + Builder.LabelInfo.GetLabelName() + "'" );
                    Output = Interface.run( "labelsync -l " + Builder.LabelInfo.GetLabelName() );
                    Write( Builder, Log, Output );
                }
#endif
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR: Submit " + ex.Message );
                ErrorLevel = ERRORS.SCC_Submit;
            }

            Interface.Disconnect();

            return ( ChangeList );
        }

        public bool LabelExists( ScriptParser Builder, string LabelName )
        {
            Array Output;
            bool LabelExists = false;

            ErrorLevel = ERRORS.None;

            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 1;

            try
            {
                Parent.Log( "Executing 'P4 label -o " + LabelName + "'", Color.DarkGreen );
                Output = Interface.run( "label -o " + LabelName );
                string Description = Interface.get_Var( "Description" );
                LabelExists = Description.StartsWith( "[BUILDER]" );
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Parent.Log( "P4ERROR: LabelExists " + ex.Message, Color.Red );
            }

            Interface.Disconnect();
            return ( LabelExists );
        }

        public void CreateNewLabel( StreamWriter Log, ScriptParser Builder )
        {
            Array Output;

            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }

            ErrorLevel = ERRORS.None;

            Interface.SetProtocol( "api", "61" );

            Interface.ParseForms();
            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 2;

            try
            {
                // Set all the vars from an arbitrary label (and clear out the submit vars)
                Builder.Write( Log, "Executing 'P4 label -o QA_APPROVED_CURRENT_BUILD'" );
                Output = Interface.run( "label -o QA_APPROVED_CURRENT_BUILD" );
                Write( Builder, Log, Output );

                Interface.set_Var( "Label", Builder.LabelInfo.GetLabelName() );
                Parent.Log( "Fullname: " + Builder.LabelInfo.GetLabelName(), Color.DarkGreen );

                Builder.LabelInfo.CreateLabelDescription();

                Interface.set_Var( "Description", Builder.LabelInfo.Description );
                Interface.set_Var( "Options", "unlocked" );

                string[] Files = new string[1];
                Files[0] = "//depot/" + Builder.SourceControlBranch + "/...";
                Interface.set_ArrayVar( "View", ref Files );

                Builder.Write( Log, "Executing 'P4 label -i'" );
                Output = Interface.run( "label -i" );
                Write( Builder, Log, Output );

                Builder.Write( Log, "Executing 'P4 labelsync -l " + Builder.LabelInfo.GetLabelName() + "'" );
                Output = Interface.run( "labelsync -l " + Builder.LabelInfo.GetLabelName() );
                Write( Builder, Log, Output );

                Builder.NewLabelCreated = true;
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR: CreateNewLabel " + ex.Message );
                ErrorLevel = ERRORS.SCC_CreateNewLabel;
            }

            Interface.Disconnect();
        }


        public void DeleteLabel( ScriptParser Builder, string LabelName )
        {
            Array Output;

            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }

            ErrorLevel = ERRORS.None;

            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 1;

            try
            {
                // Delete temp label for a failed build
                Parent.Log( "Executing 'P4 label -d " + LabelName + "'", Color.Magenta );
                Output = Interface.run( "label -d " + LabelName );
                Parent.Log( Output, Color.Magenta );
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Parent.Log( "P4ERROR: DeleteLabel " + ex.Message, Color.Red );
                ErrorLevel = ERRORS.SCC_DeleteLabel;
            }

            Interface.Disconnect();
        }

        public void UpdateLabelDescription( StreamWriter Log, ScriptParser Builder )
        {
            Array Output;

            if( !Builder.NewLabelCreated )
            {
                Builder.Write( Log, "P4ERROR: UpdateLabelDescription - no label has been created that can be updated" );
                return;
            }

            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }

            ErrorLevel = ERRORS.None;

            Interface.SetProtocol( "api", "61" );

            Interface.ParseForms();
            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 2;

            try
            {
                string LabelName = Builder.LabelInfo.GetLabelName();

                // Set all the vars from an arbitrary label (and clear out the submit vars)
                Builder.Write( Log, "Executing 'P4 label -o " + LabelName + "'" );
                Output = Interface.run( "label -o " + LabelName );
                Write( Builder, Log, Output );

                Parent.Log( "Fullname: " + LabelName, Color.DarkGreen );

                Builder.LabelInfo.CreateLabelDescription();

                Interface.set_Var( "Description", Builder.LabelInfo.Description );

                Builder.Write( Log, "Executing 'P4 label -i'" );
                Output = Interface.run( "label -i" );
                Write( Builder, Log, Output );
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR: UpdateLabelDescription " + ex.Message );
                ErrorLevel = ERRORS.SCC_UpdateLabelDescription;
            }

            Interface.Disconnect();
        }

        public bool GetLabelInfo( LabelInfo Label, ScriptParser Builder, string LabelName )
        {
            Array Output;
            bool LabelFound = false;

            if( !ValidateClientSpec( Builder ) )
            {
                return ( LabelFound );
            }

            ErrorLevel = ERRORS.None;

            Interface.SetProtocol( "api", "61" );

            Interface.ParseForms();
            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 2;

            try
            {
                // Set all the vars from the label which may exist
                Output = Interface.run( "label -o " + LabelName );
                string Description = Interface.get_Var( "Description" );
                Label.HandleDescription( Description );
                LabelFound = true;
            }
            catch( System.Runtime.InteropServices.COMException )
            {
                // This function is allowed to fail
            }

            Interface.Disconnect();
            return( LabelFound );
        }

        public void RefreshLabelInfo( ScriptParser Builder )
        {
            Array Output;

            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }

            ErrorLevel = ERRORS.None;

            Interface.SetProtocol( "api", "61" );

            Interface.ParseForms();
            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 2;

            try
            {
                // Set all the vars from the label which may exist
                Output = Interface.run( "label -o " + Builder.LabelInfo.GetLabelName() );
                string Description = Interface.get_Var( "Description" );
                Builder.LabelInfo.HandleDescription( Description );
            }
            catch( System.Runtime.InteropServices.COMException )
            {
            }

            Interface.Disconnect();
        }

        public void Tag( ScriptParser Builder, StreamWriter Log )
        {
            Array Output;

            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }

            // Get label depending on dependencies
            string Revision = Parent.GetLabelToSync();
            string LabelName = Revision.TrimStart( "@".ToCharArray() );
            if( LabelName == "#head" )
            {
                ErrorLevel = ERRORS.SCC_Tag;
                Builder.Write( Log, "P4ERROR: TagFile - Non existent label, cannot tag to #head" );
                return;
            }

            if( !LabelExists( Builder, LabelName ) )
            {
                ErrorLevel = ERRORS.SCC_Tag;
                Parent.Log( "P4ERROR: Tag - Non existent label, cannot tag to: " + LabelName, Color.DarkGreen );
                return;
            }

            ErrorLevel = ERRORS.None;

            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 1;

            try
            {
                // Delete the old label to prevent lockouts
                Builder.Write( Log, "Executing 'P4 label -d " + Builder.CommandLine + "'" );
                Output = Interface.run( "label -d " + Builder.CommandLine );
                Write( Builder, Log, Output );

                // Set the changelist description
                Builder.Write( Log, "Executing 'P4 tag -l " + Builder.CommandLine + " " + Revision + "'" );
                Output = Interface.run( "tag -l " + Builder.CommandLine + " " + Revision );
                Write( Builder, Log, Output );
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR: Tag " + ex.Message );
                ErrorLevel = ERRORS.SCC_Tag;
            }

            Interface.Disconnect();
        }

        public void TagMessage( ScriptParser Builder, string Message )
        {
            Array Output;

            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }

            string LabelName = Builder.LabelInfo.GetLabelName();
            if( !LabelExists( Builder, LabelName ) )
            {
                // In case we're compiling with a define that hasn't created a label yet
                Parent.Log( "P4ERROR: TagMessage - Full label not found, trying root label", Color.DarkGreen );

                LabelName = Builder.LabelInfo.GetRootLabelName();
                if( !LabelExists( Builder, LabelName ) )
                {
                    ErrorLevel = ERRORS.SCC_TagMessage;
                    Parent.Log( "P4ERROR: TagMessage - Non existent label", Color.DarkGreen );
                    return;
                }
            }

            ErrorLevel = ERRORS.None;

            Interface.ParseForms();
            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 1;

            try
            {
                // Set all the vars from an arbitrary label (and clear out the submit vars)
                Parent.Log( "Executing 'P4 label -o " + LabelName + "'", Color.DarkGreen );
                Output = Interface.run( "label -o " + LabelName );

                // Append the message to the description
                string Description = Interface.get_Var( "Description" );
                Interface.set_Var( "Description", Description + Message );

                Parent.Log( "Executing 'P4 label -i'", Color.DarkGreen );
                Output = Interface.run( "label -i" );
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Parent.Log( "P4ERROR: TagMessage " + ex.Message, Color.Red );
                ErrorLevel = ERRORS.SCC_TagMessage;
            }

            Interface.Disconnect();
        }

        public void TagFile( ScriptParser Builder, string FileName, StreamWriter Log )
        {
            Array Output;

            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }

            // Get label depending on dependencies
            string Revision = Parent.GetLabelToSync();
            string LabelName = Revision.TrimStart( "@".ToCharArray() );
            if( LabelName == "#head" )
            {
                ErrorLevel = ERRORS.SCC_TagFile;
                Builder.Write( Log, "P4ERROR: TagFile - Non existent label, cannot tag file to #head" );
                return;
            }

            if( !LabelExists( Builder, LabelName ) )
            {
                ErrorLevel = ERRORS.SCC_TagFile;
                Builder.Write( Log, "P4ERROR: TagFile - Non existent label, cannot tag file to: " + LabelName );
                return;
            }

            ErrorLevel = ERRORS.None;

            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 1;

            try
            {
                // Set the changelist description
                Builder.Write( Log, "Executing 'P4 tag -l " + LabelName + " " + FileName + "'" );
                Output = Interface.run( "tag -l " + LabelName + " " + FileName );
                Write( Builder, Log, Output );
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR: TagFile " + ex.Message );
                ErrorLevel = ERRORS.SCC_TagFile;
            }

            Interface.Disconnect();
        }

        public void GetChangesSinceLastBuild( ScriptParser Builder, StreamWriter Log )
        {
            Array ChangeOutput;
            Array DescribeOutput;

            if( !ValidateClientSpec( Builder ) )
            {
                return;
            }

            // Get changelist depending on dependencies
            string Revision = Parent.GetChangeListToSync();

            ErrorLevel = ERRORS.None;
            Interface.Tagged();
            Interface.Connect();
            Interface.Client = Builder.GetClientSpec();
            Interface.Cwd = CurrentWorkingDirectory;
            Interface.ExceptionLevel = 2;

            // ... in case there are no changelists to retrieve
            Builder.LabelInfo.Changelist = Builder.LastGoodBuild;

            try
            {
                Builder.Write( Log, "Executing 'P4 changes ...@" + Builder.LastGoodBuild.ToString() + "," + Revision + "'" );
                ChangeOutput = Interface.run( "changes ...@" + Builder.LastGoodBuild.ToString() + "," + Revision );

                foreach( string Line in ChangeOutput )
                {
                    if( Line.ToLower().StartsWith( "change" ) )
                    {
                        string ChangeList = Line.Substring( "change".Length ).Trim();
                        try
                        {
                            DescribeOutput = Interface.run( "describe " + ChangeList );
                            Builder.ProcessChangeList( DescribeOutput );
                        }
                        catch
                        {
                        }
                    }
                }
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Builder.Write( Log, "P4ERROR!: GetChangesSinceLastBuild " + ex.Message );
                ErrorLevel = ERRORS.SCC_Sync;
            }

            Interface.Disconnect();
        }
    }
}
