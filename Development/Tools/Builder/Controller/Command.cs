using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Text;
using System.Xml;
using System.Xml.Serialization;
using System.Runtime.InteropServices;

namespace Controller
{
    public class JobInfo
    {
        [XmlAttribute]
        public string Name = "";

        [XmlAttribute]
        public string Command = "";

        [XmlAttribute]
        public string Parameter = "";
    }

    public class JobDescriptions
    {
        [XmlArray( "Jobs" )]
        public JobInfo[] Jobs = new JobInfo[0];

        public JobDescriptions()
        {
        }
    }

    class Command
    {
        public Main Parent = null;

        private P4 SCC = null;
        private ScriptParser Builder = null;

        private StreamWriter Log;
        private ERRORS ErrorLevel = ERRORS.None;
        private BuildProcess CurrentBuild = null;
        private DateTime StartTime = DateTime.Now;
        private DateTime LastRespondingTime = DateTime.Now;

        public COMMANDS LastExecutedCommand = COMMANDS.Error;

        private string CommonCommandLine = "-unattended -nopause -buildmachine -forcelogflush";

        private string GetPlatform()
        {
            return ( "-Platform=" + Builder.LabelInfo.Platform );
        }

        public Command( Main InParent, P4 InSCC, ScriptParser InBuilder )
        {
            Parent = InParent;
            SCC = InSCC;
            Builder = InBuilder;
        }

        public ERRORS GetErrorLevel()
        {
            return ( ErrorLevel );
        }

        public BuildProcess GetCurrentBuild()
        {
            return ( CurrentBuild );
        }

        private string[] ExtractParameters( string CommandLine )
        {
            List<string> Parameters = new List<string>();
            string Parameter = "";
            bool InQuotes = false;

            foreach( char Ch in CommandLine )
            {
                if( Ch == '\"' )
                {
                    if( InQuotes )
                    {
                        if( Parameter.Length > 0 )
                        {
                            Parameters.Add( Parameter );
                            Parameter = "";
                        }
                        InQuotes = false;
                    }
                    else
                    {
                        InQuotes = true;
                    }
                }
                else
                {
                    if( !InQuotes && Ch == ' ' || Ch == '\t' )
                    {
                        if( Parameter.Length > 0 )
                        {
                            Parameters.Add( Parameter );
                            Parameter = "";
                        }
                    }
                    else
                    {
                        Parameter += Ch;
                    }
                }
            }

            if( Parameter.Length > 0 )
            {
                Parameters.Add( Parameter );
            }

            return ( Parameters.ToArray() );
        }

        private void CleanIniFiles( GameConfig Config, StreamWriter Log )
        {
            string ConfigFolder = Config.GetConfigFolderName();
            DirectoryInfo Dir = new DirectoryInfo( ConfigFolder );

            foreach( FileInfo File in Dir.GetFiles() )
            {
                Builder.Write( Log, " ... checking: " + File.Name );
                if( File.IsReadOnly == false )
                {
                    File.Delete();
                    Builder.Write( Log, " ...... deleted: " + File.Name );
                }
            }
        }

        private void SetReadOnlyState( string FileSpec, bool ReadOnly )
        {
            int Offset = FileSpec.LastIndexOf( '/' );
            string Dir = FileSpec.Substring( 0, Offset );
            string File = FileSpec.Substring( Offset + 1, FileSpec.Length - Offset - 1 );

            DirectoryInfo Dirs = new DirectoryInfo( Dir );
            foreach( FileInfo Info in Dirs.GetFiles( File ) )
            {
                Info.IsReadOnly = ReadOnly;
            }
        }

        private void SCC_CheckConsistency()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckConsistency ), false, Encoding.ASCII );

                FileInfo Info = new FileInfo( "c:\\depot\\UnrealEngine3\\Binaries\\human_anya_dialog" );
                if( Info.Exists )
                {
                    Builder.Write( Log, "Error, THE WORLD IS ENDING - PLEASE NOTIFY SCOTT SHERMAN AND JOHN SCOTT IMMEDIATELY" );
                    Builder.Write( Log, "Error, THE WORLD IS ENDING - THIS MEANS *YOU* AND THIS MEANS *NOW*" );
                    ErrorLevel = ERRORS.TheWorldIsEnding;
                    Log.Close();
                    return;
                }

                SCC.CheckConsistency( Builder, Log, Builder.CommandLine );

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckConsistency;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling CheckConsistency" );
                    Log.Close();
                }
            }
        }

        private void SCC_Sync()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_Sync ), false, Encoding.ASCII );

                // Name of a build type that we wish to get the last good build from
                Builder.SyncedLabel = Parent.GetLabelToSync();
                SCC.SyncToRevision( Builder, Log, Builder.SyncedLabel, "..." );

                // Don't get the list of changelists for CIS type builds
                if( ( Builder.LabelInfo.RevisionType != RevisionType.ChangeList ) && Builder.LastGoodBuild != 0 )
                {
                    SCC.GetChangesSinceLastBuild( Builder, Log );
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_Sync;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling sync" );
                    Log.Close();
                }
            }
        }

        private void SCC_ArtistSync()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_ArtistSync ), false, Encoding.ASCII );

                // Name of a build type that we wish to get the last good build from
                Builder.SyncedLabel = Parent.GetLabelToSync();
                SCC.SyncToRevision( Builder, Log, Builder.SyncedLabel, "..." );

                // Don't get the list of changelists for CIS type builds
                if( ( Builder.LabelInfo.RevisionType != RevisionType.ChangeList ) && Builder.LastGoodBuild != 0 )
                {
                    SCC.GetChangesSinceLastBuild( Builder, Log );
                }

                // Additionally sync some content to head
                string FileSpec = Builder.LabelInfo.Game + "Game/__Trashcan/...";
                SCC.SyncToRevision( Builder, Log, "#head", FileSpec );

                FileSpec = Builder.LabelInfo.Game + "Game/Content/...";
                SCC.SyncToRevision( Builder, Log, "#head", FileSpec );

                FileSpec = Builder.LabelInfo.Game + "Game/Localization/...";
                SCC.SyncToRevision( Builder, Log, "#head", FileSpec );

                FileSpec = Builder.LabelInfo.Game + "Game/Content/Interface/...";
                SCC.SyncToRevision( Builder, Log, "#head", FileSpec );

                FileSpec = Builder.LabelInfo.Game + "Game/Content/RefShaderCache...";
                SCC.SyncToRevision( Builder, Log, Builder.SyncedLabel, FileSpec );

                FileSpec = Builder.LabelInfo.Game + "Game/Content/RefContentTagsIndex...";
                SCC.SyncToRevision( Builder, Log, Builder.SyncedLabel, FileSpec );

                FileSpec = Builder.LabelInfo.Game + "Game/ContentNotForShip/...";
                SCC.SyncToRevision( Builder, Log, "#head", FileSpec );

                FileSpec = Builder.LabelInfo.Game + "Game/Movies/...";
                SCC.SyncToRevision( Builder, Log, "#head", FileSpec );

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_ArtistSync;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling artist sync" );
                    Log.Close();
                }
            }
        }

        private void SCC_GetChanges()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_GetChanges ), false, Encoding.ASCII );

                // Name of a build type that we wish to get the last good build from
                Builder.SyncedLabel = Parent.GetLabelToSync();
                if( Builder.LastGoodBuild != 0 )
                {
                    SCC.GetChangesSinceLastBuild( Builder, Log );
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_GetChanges;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while getting user changes" );
                    Log.Close();
                }
            }
        }

        private void SCC_SyncSingleChangeList()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_SyncSingleChangeList ), false, Encoding.ASCII );

                // Name of a build type that we wish to get the last good build from
                SCC.SyncSingleChangeList( Builder, Log );

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_SyncSingleChangeList;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling sync single changelist" );
                    Log.Close();
                }
            }
        }

        private void SCC_Checkout()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_Checkout ), false, Encoding.ASCII );

                if( SCC.CheckoutFileSpec( Builder, Log, Builder.CommandLine, false ) )
                {
                    Builder.FilesCheckedOut = true;
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_Checkout;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout" );
                    Log.Close();
                }
            }
        }

        private void SCC_OpenForDelete()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_OpenForDelete ), false, Encoding.ASCII );

                if( SCC.OpenForDeleteFileSpec( Builder, Log, Builder.CommandLine ) )
                {
                    Builder.FilesCheckedOut = true;
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_OpenForDelete;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling open for delete" );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutGame()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutGame ), false, Encoding.ASCII );

                GameConfig Config = Builder.AddCheckedOutGame();

                string[] Executables = Config.GetExecutableNames();
                foreach( string Executable in Executables )
                {
                    if( Executable.Length > 0 )
                    {
                        if( SCC.CheckoutFileSpec( Builder, Log, Executable, false ) )
                        {
                            Builder.FilesCheckedOut = true;
                        }
                    }
                }

                string[] SymbolFiles = Config.GetSymbolFileNames();
                foreach( string SymbolFile in SymbolFiles )
                {
                    if( SymbolFile.Length > 0 )
                    {
                        if( SCC.CheckoutFileSpec( Builder, Log, SymbolFile, false ) )
                        {
                            Builder.FilesCheckedOut = true;
                        }
                    }
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutGame;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout game." );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutContentTag()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutContentTag ), false, Encoding.ASCII );

                GameConfig Config = Builder.CreateGameConfig();

                string ContentTags = Config.GetContentTagName();
                if( SCC.CheckoutFileSpec( Builder, Log, ContentTags, false ) )
                {
                    Builder.FilesCheckedOut = true;
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutContentTag;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout content tags." );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutLayout()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutLayout ), false, Encoding.ASCII );

                GameConfig Config = Builder.CreateGameConfig();
                string Layout = Config.GetLayoutFileName( Builder.GetLanguages().ToArray() );

                if( SCC.CheckoutFileSpec( Builder, Log, Layout, false ) )
                {
                    Builder.FilesCheckedOut = true;
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutLayout;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout layout." );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutShader()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutShader ), false, Encoding.ASCII );

                GameConfig Config = Builder.CreateGameConfig();

                string ShaderFile = Config.GetRefShaderName();
                if( SCC.CheckoutFileSpec( Builder, Log, ShaderFile, false ) )
                {
                    Builder.FilesCheckedOut = true;
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutShader;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout shader." );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutDialog()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutDialog ), false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 2 )
                {
                    Builder.Write( Log, "Error, not enough parameters for CheckoutDialog." );
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig( Parms[0] );
                    Queue<string> Languages = Builder.GetLanguages();
                    Queue<string> ValidLanguages = new Queue<string>();

                    foreach( string Language in Languages )
                    {
                        string DialogFile = Config.GetDialogFileName( Language, Parms[1] );
                        if( SCC.CheckoutFileSpec( Builder, Log, DialogFile, false ) )
                        {
                            Builder.FilesCheckedOut = true;
                            ValidLanguages.Enqueue( Language );
                        }
                    }

                    Builder.SetValidLanguages( ValidLanguages );
                }

                // Some files are allowed to not exist (and fail checkout)
                ErrorLevel = ERRORS.None;
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutDialog;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout dialog" );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutFonts()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutFonts ), false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 2 )
                {
                    Builder.Write( Log, "Error, not enough parameters for CheckoutFonts" );
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig( Parms[0] );
                    Queue<string> Languages = Builder.GetLanguages();
                    Queue<string> ValidLanguages = new Queue<string>();

                    foreach( string Language in Languages )
                    {
                        string FontFile = Config.GetFontFileName( Language, Parms[1] );
                        if( SCC.CheckoutFileSpec( Builder, Log, FontFile, false ) )
                        {
                            Builder.FilesCheckedOut = true;
                            ValidLanguages.Enqueue( Language );
                        }
                    }

                    Builder.SetValidLanguages( ValidLanguages );
                }

                // Some files are allowed to not exist (and fail checkout)
                ErrorLevel = ERRORS.None;
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutFonts;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout dialog" );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutLocPackage()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutLocPackage ), false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 1 )
                {
                    Builder.Write( Log, "Error, not enough parameters for CheckoutLocPackage" );
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig( "Example" );
                    Queue<string> Languages = Builder.GetLanguages();
                    Queue<string> ValidLanguages = new Queue<string>();

                    foreach( string Language in Languages )
                    {
                        string PackageFile = Config.GetPackageFileName( Language, Parms[0] );
                        if( SCC.CheckoutFileSpec( Builder, Log, PackageFile, false ) )
                        {
                            Builder.FilesCheckedOut = true;
                            ValidLanguages.Enqueue( Language );
                        }
                    }

                    Builder.SetValidLanguages( ValidLanguages );
                }

                // Some files are allowed to not exist (and fail checkout)
                ErrorLevel = ERRORS.None;
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutFonts;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout dialog" );
                    Log.Close();
                }
            }
        }

        private void SCC_CheckoutGDF()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CheckoutGDF ), false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 2 )
                {
                    Builder.Write( Log, "Error, incorrect number of parameters for CheckoutGDF" );
                    ErrorLevel = ERRORS.SCC_CheckoutGDF;
                }
                else
                {
                    Queue<string> Languages = Builder.GetLanguages();

                    foreach( string Lang in Languages )
                    {
                        string GDFFileName = Parms[1] + "/" + Lang.ToUpper() + "/" + Parms[0] + "Game.gdf.xml";
                        if( SCC.CheckoutFileSpec( Builder, Log, GDFFileName, false ) )
                        {
                            Builder.FilesCheckedOut = true;
                        }
                    }
                }

                // Some files are allowed to not exist (and fail checkout)
                ErrorLevel = ERRORS.None;
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CheckoutGDF;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling checkout dialog" );
                    Log.Close();
                }
            }
        }

        private void MakeWritable()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.MakeWritable ), false, Encoding.ASCII );

                SetReadOnlyState( Builder.CommandLine, false );

                Builder.AddMadeWritableFileSpec( Builder.CommandLine );

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.MakeWritable;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while making files writable" );
                    Log.Close();
                }
            }
        }

        private void SCC_Submit()
        {
            if( Builder.FilesCheckedOut )
            {
                try
                {
                    Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_Submit ), false, Encoding.ASCII );
                    int ChangeList = SCC.Submit( Log, Builder, 0 );

                    ErrorLevel = SCC.GetErrorLevel();
                    if( ErrorLevel == ERRORS.SCC_Submit )
                    {
                        // Interrogate P4 to see if any files needs resolving
                        if( SCC.GetIncorrectCheckedOutFiles( Builder ) )
                        {
                            SCC.Resolve( Log, Builder );

                            ErrorLevel = SCC.GetErrorLevel();
                            if( ErrorLevel == ERRORS.None )
                            {
                                SCC.Submit( Log, Builder, ChangeList );
                                ErrorLevel = SCC.GetErrorLevel();
                            }
                        }
                    }

                    if( ErrorLevel == ERRORS.None )
                    {
                        Builder.FilesCheckedOut = false;
                    }

                    Log.Close();
                }
                catch
                {
                    ErrorLevel = ERRORS.SCC_Submit;
                    if( Log != null )
                    {
                        Builder.Write( Log, "Error, exception while calling submit" );
                        Log.Close();
                    }
                }
            }
            else
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_Submit ), false, Encoding.ASCII );
                ErrorLevel = ERRORS.SCC_Submit;
                Log.Close();
            }
        }

        private void SCC_CreateNewLabel()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_CreateNewLabel ), false, Encoding.ASCII );

                SCC.CreateNewLabel( Log, Builder );
                Builder.Dependency = Builder.LabelInfo.GetLabelName();

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_CreateNewLabel;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while creating a new Perforce label" );
                    Log.Close();
                }
            }
        }

        private void SCC_UpdateLabelDescription()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_UpdateLabelDescription ), false, Encoding.ASCII );

                SCC.UpdateLabelDescription( Log, Builder );

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_UpdateLabelDescription;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while updating a Perforce label" );
                    Log.Close();
                }
            }
        }

        private void SCC_Revert()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_Revert ), false, Encoding.ASCII );

                Parent.Log( "[STATUS] Reverting: '...'", Color.Magenta );
                SCC.Revert( Builder, Log, "..." );

                Builder.FilesCheckedOut = false;
                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_Revert;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling revert" );
                    Log.Close();
                }
            }
        }

        private void SCC_RevertFile()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_RevertFile ), false, Encoding.ASCII );

                Parent.Log( "[STATUS] Reverting: '" + Builder.CommandLine + "'", Color.Magenta );
                SCC.Revert( Builder, Log, Builder.CommandLine );

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_RevertFile;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while calling revert" );
                    Log.Close();
                }
            }
        }

        private void MakeReadOnly()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.MakeReadOnly ), false, Encoding.ASCII );

                List<string> WriteableFiles = Builder.GetMadeWritableFiles();
                foreach( string FileSpec in WriteableFiles )
                {
                    Parent.Log( "[STATUS] Making read only: '" + FileSpec + "'", Color.Magenta );
                    SetReadOnlyState( FileSpec, true );
                }

                Builder.ClearMadeWritableFiles();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.MakeReadOnly;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while making files read only" );
                    Log.Close();
                }
            }
        }

        private void SCC_Tag()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_Tag ), false, Encoding.ASCII );

                SCC.Tag( Builder, Log );
                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_Tag;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while tagging build" );
                    Log.Close();
                }
            }
        }

        private void SCC_TagFile()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_TagFile ), false, Encoding.ASCII );

                SCC.TagFile( Builder, Builder.CommandLine, Log );
                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_TagFile;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while tagging file" );
                    Log.Close();
                }
            }
        }

        private void SCC_TagPCS()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_TagPCS ), false, Encoding.ASCII );

                GameConfig Config = Builder.CreateGameConfig();
                SCC.TagFile( Builder, Config.GetRefShaderName(), Log );
                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_TagPCS;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while tagging PCS" );
                    Log.Close();
                }
            }
        }

        private void SCC_TagExe()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_TagExe ), false, Encoding.ASCII );

                GameConfig Config = Builder.CreateGameConfig();

                // Tag the executable names
                string[] ExeNames = Config.GetExecutableNames();
                foreach( string ExeName in ExeNames )
                {
                    if( ExeName.Length > 0 )
                    {
                        SCC.TagFile( Builder, ExeName, Log );
                    }
                }

                // Tag the symbol files
                string[] DebugNames = Config.GetSymbolFileNames();
                foreach( string DebugName in DebugNames )
                {
                    if( DebugName.Length > 0 )
                    {
                        SCC.TagFile( Builder, DebugName, Log );
                    }
                }

                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_TagExe;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while tagging executable and symbol files." );
                    Log.Close();
                }
            }
        }

        private void SCC_TagContentTag()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_TagContentTag ), false, Encoding.ASCII );

                GameConfig Config = Builder.CreateGameConfig();
                SCC.TagFile( Builder, Config.GetContentTagName(), Log );
                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_TagContentTag;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while tagging content tag index." );
                    Log.Close();
                }
            }
        }

        private void SCC_TagLayout()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SCC_TagLayout ), false, Encoding.ASCII );

                GameConfig Config = Builder.CreateGameConfig();
                SCC.TagFile( Builder, Config.GetLayoutFileName( Builder.GetLanguages().ToArray() ), Log );
                ErrorLevel = SCC.GetErrorLevel();
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SCC_TagLayout;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while tagging layout file." );
                    Log.Close();
                }
            }
        }
        private void AddSimpleJob( StreamWriter Log, string[] Parameters )
        {
            JobInfo Job = new JobInfo();

            Job.Name = "Job";
            if( Builder.LabelInfo.Game.Length > 0 )
            {
                Job.Name += "_" + Builder.LabelInfo.Game;
            }
            if( Builder.LabelInfo.Platform.Length > 0 )
            {
                Job.Name += "_" + Builder.LabelInfo.Platform;
            }
            Job.Name += "_" + Parameters[1];

            Job.Command = Parameters[0];
            Job.Parameter = Parameters[1];

            for( int i = 2; i < Parameters.Length; i++ )
            {
                Job.Parameter += " " + Parameters[i];
            }

            Parent.AddJob( Job );

            Builder.Write( Log, "Added Job: " + Job.Name );
        }

        private void AddJob()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.AddJob ), false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 2 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: AddJob <Script> <File1> [File2...]." );
                    ErrorLevel = ERRORS.AddJob;
                }
                else
                {
                    AddSimpleJob( Log, Parms );
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.AddJob;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while adding a job." );
                    Log.Close();
                }
            }
        }

        private void AddUnrealGameJob()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.AddUnrealGameJob ), false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 2 && Parms.Length != 3 )
                {
                    Builder.Write( Log, "Error, incorrect number of parameters. Usage: AddUnrealGameJob <Config> <Game1> [ScriptExtension]." );
                    ErrorLevel = ERRORS.AddUnrealGameJob;
                }
                else
                {
                    string OrigConfig = Builder.BuildConfiguration;
                    string OrigGame = Builder.LabelInfo.Game;

                    Builder.BuildConfiguration = Parms[0];
                    Builder.LabelInfo.Game = Parms[1];

                    string[] Parameters = new string[2] { "Jobs/UnrealGameJob", Builder.BuildConfiguration };
                    if( Parms.Length == 3 )
                    {
                        Parameters[0] += "_" + Parms[2];
                    }
                    AddSimpleJob( Log, Parameters );

                    GameConfig Config = Builder.CreateGameConfig( false );
                    Builder.LabelInfo.Games.Add( Config );

                    Builder.LabelInfo.Game = OrigGame;
                    Builder.BuildConfiguration = OrigConfig;
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.AddUnrealGameJob;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while adding a UnrealBuild compile job." );
                    Log.Close();
                }
            }
        }

        private void AddUnrealFullGameJob()
        {
            try
            {
                Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.AddUnrealFullGameJob ), false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 2 && Parms.Length != 3 )
                {
                    Builder.Write( Log, "Error, incorrect number of parameters. Usage: AddUnrealFullGameJob <Config> Game [Script Extension]." );
                    ErrorLevel = ERRORS.AddUnrealFullGameJob;
                }
                else
                {
                    string OrigConfig = Builder.BuildConfiguration;
                    string OrigGame = Builder.LabelInfo.Game;

                    Builder.BuildConfiguration = Parms[0];
                    Builder.LabelInfo.Game = Parms[1];

                    string[] Parameters = new string[2] { "Jobs/UnrealFullGameJob", Builder.BuildConfiguration };
                    if( Parms.Length == 3 )
                    {
                        Parameters[0] += "_" + Parms[2];
                    }
                    AddSimpleJob( Log, Parameters );

                    GameConfig Config = Builder.CreateGameConfig( false );
                    Builder.LabelInfo.Games.Add( Config );

                    Builder.LabelInfo.Game = OrigGame;
                    Builder.BuildConfiguration = OrigConfig;
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.AddUnrealFullGameJob;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while adding a full game job." );
                    Log.Close();
                }
            }
        }

        private string GetWorkingDirectory( string Path, ref string Solution )
        {
            string Directory = "";
            Solution = "";

            int Index = Path.LastIndexOf( '/' );
            if( Index >= 0 )
            {
                Directory = Path.Substring( 0, Index );
                Solution = Path.Substring( Index + 1, Path.Length - Index - 1 );
            }

            return( Directory );
        }

        private void MS_Build()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.MSBuild ), false, Encoding.ASCII );
                
                Builder.Write( Log, Parent.SetRequestedSDKs() );
                Builder.HandleMSVCDefines( Log );

                string CommandLine = "";
                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 1 )
                {
                    Builder.Write( Log, "Error, incorrect number of parameters. Usage: MSBuild <Project>" );
                    ErrorLevel = ERRORS.MSBuild;
                }
                else
                {
                    CommandLine = Parms[0] + " /verbosity:normal /target:Rebuild /property:Configuration=\"" + Builder.BuildConfiguration + "\"";

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Builder.MSApplication, CommandLine, "Development/Src", false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.MSBuild;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to build using MSBuild" );
                    Log.Close();
                }
            }
        }

        private void MSVC_Clean()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.MSVCClean ), false, Encoding.ASCII );
              
                Builder.Write( Log, Parent.SetRequestedSDKs() );
                Builder.HandleMSVCDefines( Log );

                string CommandLine = "";
                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 || Parms.Length > 2 )
                {
                    Builder.Write( Log, "Error, incorrect number of parameters. Usage: MSVCClean <Solution> [Project]" );
                    ErrorLevel = ERRORS.MSVCClean;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig();

                    if( Parms.Length == 1 )
                    {
                        CommandLine = Parms[0] + ".sln /clean \"" + Config.GetBuildConfiguration() + "\"";
                    }
                    else if( Parms.Length == 2 )
                    {
                        CommandLine = Parms[0] + ".sln /project " + Config.GetProjectName( Parms[1] ) + " /clean \"" + Config.GetBuildConfiguration() + "\"";
                    }

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Builder.MSVCApplication, CommandLine, "", false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.MSVCClean;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to clean using devenv" );
                    Log.Close();
                }
            }
        }

        private void MSVC_Build()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.MSVCBuild ), false, Encoding.ASCII );
                
                Builder.Write( Log, Parent.SetRequestedSDKs() );
                Builder.HandleMSVCDefines( Log );

                string CommandLine = "";
                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 || Parms.Length > 2 )
                {
                    Builder.Write( Log, "Error, incorrect number of parameters. Usage: MSVCBuild <Solution> [Project]" );
                    ErrorLevel = ERRORS.MSVCBuild;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig();

                    if( Parms.Length == 1 )
                    {
                        CommandLine = Parms[0] + ".sln /build \"" + Config.GetBuildConfiguration() + "\"";
                    }
                    else if( Parms.Length == 2 )
                    {
                        CommandLine = Parms[0] + ".sln /project " + Config.GetProjectName( Parms[1] ) + " /build \"" + Config.GetBuildConfiguration() + "\"";
                    }

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Builder.MSVCApplication, CommandLine, "", false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.MSVCBuild;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to build using devenv" );
                    Log.Close();
                }
            }
        }

        private void GCC_Clean()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.GCCClean );

                string ReturnCode = Parent.SetRequestedSDKs();
                if( ReturnCode.StartsWith( "Error, " ) )
                {
                    StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                    Builder.Write( Log, ReturnCode );
                    Log.Close();

                    ErrorLevel = ERRORS.GCCClean;
                    return;
                }

                GameConfig Config = Builder.CreateGameConfig();

                string GameName = "GAMENAME=" + Builder.CommandLine.ToUpper() + "GAME";
                string BuildConfig = "BUILDTYPE=" + Config.GetMakeConfiguration();
                string CommandLine = "/c " + Builder.MakeApplication + " -C Development/Src/PS3 USE_IB=true JOBS=" + Parent.NumJobs.ToString() + " " + GameName + " " + BuildConfig + " clean > " + LogFileName + " 2>&1";

                CurrentBuild = new BuildProcess( Parent, Builder, CommandLine, "" );
                ErrorLevel = CurrentBuild.GetErrorLevel();
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.GCCClean;
            }
        }

        private void GCC_Build()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.GCCBuild );

                string ReturnCode = Parent.SetRequestedSDKs();
                if( ReturnCode.StartsWith( "Error, " ) )
                {
                    StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                    Builder.Write( Log, ReturnCode );
                    Log.Close();

                    ErrorLevel = ERRORS.GCCBuild;
                    return;
                }

                GameConfig Config = Builder.CreateGameConfig();

                string GameName = "GAMENAME=" + Builder.CommandLine.ToUpper() + "GAME";
                string BuildConfig = "BUILDTYPE=" + Config.GetMakeConfiguration();
                string Defines = Builder.HandleGCCDefines();

                string CommandLine = "/c " + Builder.MakeApplication + " -C Development/Src/PS3 USE_IB=true PCHS= JOBS=" + Parent.NumJobs.ToString() + " " + GameName + " " + BuildConfig + " " + Defines + " -j 2 > " + LogFileName + " 2>&1";

                CurrentBuild = new BuildProcess( Parent, Builder, CommandLine, "" );
                ErrorLevel = CurrentBuild.GetErrorLevel();
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.GCCBuild;
            }
        }

        private void Unreal_Build()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.UnrealBuild ), false, Encoding.ASCII );
       
                Builder.Write( Log, Parent.SetRequestedSDKs() );

                string Executable = "Development/Intermediate/UnrealBuildTool/Release/UnrealBuildTool.exe";
                GameConfig Config = Builder.CreateGameConfig();

                string CommandLine = Config.GetUBTCommandLine();
                if( Builder.UnityStressTest )
                {
					CommandLine += " -StressTestUnity";
				}
				if (Builder.UnityDisable)
				{
					CommandLine += " -DisableUnity";
				}

                CommandLine += Builder.HandleUBTDefines();

                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "Development/Src", false );
                ErrorLevel = CurrentBuild.GetErrorLevel();

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.UnrealBuild;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to build using UnrealBuildTool" );
                    Log.Close();
                }
            }
        }

        private void Shader_Clean()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.ShaderClean );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                Builder.Write( Log, Parent.SetRequestedSDKs() );

                string ShaderName;
                FileInfo Info;

                GameConfig Config = Builder.CreateGameConfig();

                // Delete ref shader cache
                ShaderName = Config.GetRefShaderName();
                Builder.Write( Log, " ... checking for: " + ShaderName );
                Info = new FileInfo( ShaderName );
                if( Info.Exists )
                {
                    Info.IsReadOnly = false;
                    Info.Delete();
                    Builder.Write( Log, " ...... deleted: " + ShaderName );
                }

                // Delete local shader cache
                ShaderName = Config.GetLocalShaderName();
                Builder.Write( Log, " ... checking for: " + ShaderName );
                Info = new FileInfo( ShaderName );
                if( Info.Exists )
                {
                    Info.IsReadOnly = false;
                    Info.Delete();
                    Builder.Write( Log, " ...... deleted: " + ShaderName );
                }

                CleanIniFiles( Config, Log );

                ErrorLevel = ERRORS.None;
                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.ShaderClean;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to clean precompiled shaders" );
                    Log.Close();
                }
            }
        }

        private void Shader_Build()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.ShaderBuild );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                Builder.Write( Log, Parent.SetRequestedSDKs() );

                GameConfig Config = Builder.CreateGameConfig();

                string Executable = Config.GetComName();
                string CommandLine = "precompileshaders platform=" + Builder.LabelInfo.Platform + " -refcache -ALLOW_PARALLEL_PRECOMPILESHADERS " + CommonCommandLine;

                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true );
                ErrorLevel = CurrentBuild.GetErrorLevel();

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.ShaderBuild;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to build precompiled shaders" );
                    Log.Close();
                }
            }
        }

        private void Shader_BuildState()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.ShaderBuildState );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                Builder.Write( Log, Parent.SetRequestedSDKs() );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 2 )
                {
                    Builder.Write( Log, "Error, missing required parameter(s). Usage: ShaderBuild <Game> <Map> [Map...]" );
                    ErrorLevel = ERRORS.ShaderBuild;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig( Parms[0] );

                    string Executable = Config.GetComName();
                    string CommandLine = "BuildPatchedShaderStates platform=" + Builder.LabelInfo.Platform;

                    for( int i = 1; i < Parms.Length; i++ )
                    {
                        CommandLine += " " + Parms[i];
                    }

                    CommandLine += " " + CommonCommandLine;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.ShaderBuildState;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to build shader states" );
                    Log.Close();
                }
            }
        }

        private void PS3MakePatchBinary()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.PS3MakePatchBinary );
                string[] Parameters = ExtractParameters( Builder.CommandLine );
                if( Parameters.Length != 2 )
                {
                    ErrorLevel = ERRORS.PS3MakePatchBinary;
                }
                else
                {
                    string CommandLine = "/c make_fself_npdrm " + Parameters[0] + " " + Parameters[1] + "  > " + LogFileName + " 2>&1";

                    CurrentBuild = new BuildProcess( Parent, Builder, CommandLine, "" );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.PS3MakePatchBinary;
            }
        }

        private void PS3MakePatch()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.PS3MakePatch ), false, Encoding.ASCII );

                string[] Parameters = ExtractParameters( Builder.CommandLine );

                string PS3RootDir = Parent.EnableMinGW();

                DirectoryInfo MinGWFolder = new DirectoryInfo( PS3RootDir + "\\MinGW" );
                if( !MinGWFolder.Exists )
                {
                    ErrorLevel = ERRORS.PS3MakePatch;
                    Builder.Write( Log, "Error, failed to rename MinGW folder" );
                }
                else if( Parameters.Length != 2 )
                {
                    ErrorLevel = ERRORS.PS3MakePatch;
                    Builder.Write( Log, "Error, incorrect number of parameters" );
                }
                else
                {
                    string Executable = "make_package_npdrm.exe";
                    string CommandLine = "--patch " + Parameters[0] + " " + Parameters[1];

                    GameConfig Config = Builder.CreateGameConfig();
                    string WorkingDirectory = Config.GetPatchFolderName();

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, WorkingDirectory, true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                Builder.Write( Log, "Error, exception while starting to make PS3 patch" );
                ErrorLevel = ERRORS.PS3MakePatch;
            }
        }

        private void Blast()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.Blast );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );

                string[] Parameters = ExtractParameters( Builder.CommandLine );
                if( Parameters.Length != 1 )
                {
                    ErrorLevel = ERRORS.Blast;
                }
                else
                {
                    string Executable = Builder.BlastTool;
                    string CommandLine = Parameters[0] + ".xlast /build /Install:Local";
                    string WorkingDirectory = Builder.LabelInfo.Game + "Game\\DLC\\Xenon\\" + Builder.ModName;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, WorkingDirectory, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.Blast;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to blast" );
                    Log.Close();
                }
            }
        }

        private void BuildScript()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.BuildScript );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                Builder.Write( Log, Parent.SetRequestedSDKs() );

                if( Builder.CommandLine.Length == 0 )
                {
                    Builder.Write( Log, "Error, missing required parameter. Usage: BuildScript <Game>." );
                    ErrorLevel = ERRORS.BuildScript;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig( Builder.CommandLine );

                    string Executable = Config.GetComName();
                    string CommandLine = "make -full -silentbuild " + CommonCommandLine + " " + Builder.GetScriptConfiguration();

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.BuildScript;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to build script" );
                    Log.Close();
                }
            }
        }

        private void DeletLocalShaderCache( string Platform )
        {
            GameConfig Game = Builder.CreateGameConfig( Builder.LabelInfo.Game, Platform );
            Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + Game.GetLocalShaderName() + "'" );
            FileInfo LCSInfo = new FileInfo( Game.GetLocalShaderName() );
            if( LCSInfo.Exists )
            {
                LCSInfo.IsReadOnly = false;
                LCSInfo.Delete();
                Builder.Write( Log, " ... done" );
            }
        }

        private void DeleteGlobalShaderCache( string Platform )
        {
            GameConfig Game = Builder.CreateGameConfig( Builder.LabelInfo.Game, Platform );
            Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + Game.GetGlobalShaderName() + "'" );
            FileInfo GCSInfo = new FileInfo( Game.GetGlobalShaderName() );
            if( GCSInfo.Exists )
            {
                GCSInfo.IsReadOnly = false;
                GCSInfo.Delete();
                Builder.Write( Log, " ... done" );
            }
        }

        private void DeletePatternFromFolder( string Folder, string Pattern )
        {
            Builder.Write( Log, "Attempting delete of '" + Pattern + "' from '" + Folder + "'" );
            DirectoryInfo DirInfo = new DirectoryInfo( Folder );
            if( DirInfo.Exists )
            {
                Builder.Write( Log, "Deleting '" + Pattern + "' from: '" + Builder.SourceControlBranch + "/" + Folder + "'" );
                FileInfo[] Files = DirInfo.GetFiles( Pattern );
                foreach( FileInfo File in Files )
                {
                    if( File.Exists )
                    {
                        File.IsReadOnly = false;
                        File.Delete();
                    }
                }
            }
        }

        private void PreHeat()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.PreHeatMapOven ), false, Encoding.ASCII );

                if( Builder.CommandLine.Length > 0 )
                {
                    Builder.Write( Log, "Error, too many parameters. Usage: PreheatMapOven." );
                    ErrorLevel = ERRORS.PreHeatMapOven;
                }
                else if( Builder.LabelInfo.Game.Length == 0 )
                {
                    Builder.Write( Log, "Error, no game defined for PreheatMapOven." );
                    ErrorLevel = ERRORS.PreHeatMapOven;
                }
                else
                {
                    // Delete the cooked folder to start from scratch
                    string CookedFolder = Builder.LabelInfo.Game + "Game/Cooked" + Builder.LabelInfo.Platform;
                    Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + CookedFolder + "'" );
                    if( Directory.Exists( CookedFolder ) )
                    {
                        Parent.DeleteDirectory( CookedFolder, 0 );
                        Builder.Write( Log, " ... done" );
                    }

                    // Delete the config folder to start from scratch
                    string ConfigFolder = Builder.LabelInfo.Game + "Game/Config/" + Builder.LabelInfo.Platform + "/Cooked";
                    Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + ConfigFolder + "'" );
                    if( Directory.Exists( ConfigFolder ) )
                    {
                        Parent.DeleteDirectory( ConfigFolder, 0 );
                        Builder.Write( Log, " ... done" );
                    }

                    // Delete any stale TOC files
                    DeletePatternFromFolder( Builder.LabelInfo.Game + "Game", "*.txt" );

                    // Delete the config folder to start from scratch
                    DeletePatternFromFolder( Builder.LabelInfo.Game + "Game/Localization", "Coalesced*" );

                    // Delete the local shader caches
                    DeletLocalShaderCache( Builder.LabelInfo.Platform );
                    if( Builder.LabelInfo.Platform.ToLower() == "pc" )
                    {
                        DeletLocalShaderCache( "PC_SM2" );
                        DeletLocalShaderCache( "PC_SM4" );
                    }

                    // Delete the local global caches
                    DeleteGlobalShaderCache( Builder.LabelInfo.Platform );
                    if( Builder.LabelInfo.Platform.ToLower() == "pc" )
                    {
                        DeleteGlobalShaderCache( "PC_SM2" );
                        DeleteGlobalShaderCache( "PC_SM4" );
                    }

                    Builder.Write( Log, "Deleting ini files" );
                    GameConfig Game = Builder.CreateGameConfig();
                    CleanIniFiles( Game, Log );

                    Builder.ClearPublishDestinations();
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.PreHeatMapOven;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while cleaning cooked data." );
                    Log.Close();
                }
            }
        }

        private void PreHeatDLC()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.PreHeatDLC ), false, Encoding.ASCII );

                if( Builder.CommandLine.Length > 0 )
                {
                    Builder.Write( Log, "Error, too many parameters. Usage: PreheatDLC." );
                    ErrorLevel = ERRORS.PreHeatDLC;
                }
                else if( Builder.LabelInfo.Game.Length == 0 )
                {
                    Builder.Write( Log, "Error, no game defined for PreheatDLC." );
                    ErrorLevel = ERRORS.PreHeatDLC;
                }
                else if( Builder.ModName.Length == 0 )
                {
                    Builder.Write( Log, "Error, no modname defined for PreheatDLC." );
                    ErrorLevel = ERRORS.PreHeatDLC;
                }
                else
                {
                    // Delete the DLC cooked folder to start from scratch
                    string CookedFolder = Builder.LabelInfo.Game + "Game/DLC/" + Builder.LabelInfo.Platform + "/" + Builder.ModName + "/Cooked" + Builder.LabelInfo.Platform;
                    Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + CookedFolder + "'" );
                    if( Directory.Exists( CookedFolder ) )
                    {
                        Parent.DeleteDirectory( CookedFolder, 0 );
                        Builder.Write( Log, " ... done" );
                    }

                    CookedFolder = Builder.LabelInfo.Game + "Game/DLC/" + Builder.LabelInfo.Platform + "/" + Builder.ModName + "/Online";
                    Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + CookedFolder + "'" );
                    if( Directory.Exists( CookedFolder ) )
                    {
                        Parent.DeleteDirectory( CookedFolder, 0 );
                        Builder.Write( Log, " ... done" );
                    }
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.PreHeatDLC;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while cleaning cooked DLC data." );
                    Log.Close();
                }
            }
        }


        private void Clean()
        {
            StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.Clean ), false, Encoding.ASCII );

            try
            {
                // Delete object and other compilation work files
                string IntermediateFolder = "Development/Intermediate";
                Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + IntermediateFolder + "'" );

                if( Directory.Exists( IntermediateFolder ) )
                {
                    Parent.DeleteDirectory( IntermediateFolder, 0 );
                    Builder.Write( Log, " ... done" );
                }
            }
            catch( System.Exception Ex )
            {
                ErrorLevel = ERRORS.Cleanup;
                Parent.Log( Ex.Message, Color.Red );
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while cleaning up: '" + Ex.Message + "'" );
                }
            }

            Log.Close();
        }

        private void Cleanup()
        {
            StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.Cleanup ), false, Encoding.ASCII );
            Builder.Write( Log, "Cleaning up branch: '" + Builder.SourceControlBranch + "'" );

            try
            {
                // Delete all cooked data
                string CookedFolder = Builder.LabelInfo.Game + "Game/Cooked" + Builder.LabelInfo.Platform;
                Builder.Write( Log, "Deleting: '" + Builder.SourceControlBranch + "/" + CookedFolder + "'" );

                if( Directory.Exists( CookedFolder ) )
                {
                    Parent.DeleteDirectory( CookedFolder, 0 );
                    Builder.Write( Log, " ... done" );
                }

                // Delete the global shader caches
                DeleteGlobalShaderCache( Builder.LabelInfo.Platform );
                if( Builder.LabelInfo.Platform.ToLower() == "pc" )
                {
                    DeleteGlobalShaderCache( "PC_SM2" );
                    DeleteGlobalShaderCache( "PC_SM4" );
                }

                // Delete the local shader caches
                DeletLocalShaderCache( Builder.LabelInfo.Platform );
                if( Builder.LabelInfo.Platform.ToLower() == "pc" )
                {
                    DeletLocalShaderCache( "PC_SM2" );
                    DeletLocalShaderCache( "PC_SM4" );
                }
            }
            catch( System.Exception Ex )
            {
                ErrorLevel = ERRORS.Cleanup;
                Parent.Log( Ex.Message, Color.Red );
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while cleaning up: '" + Ex.Message + "'" );
                }
            }

            Log.Close();
        }

        private void Cook()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.CookMaps );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                Builder.Write( Log, Parent.SetRequestedSDKs() );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                GameConfig Config = Builder.CreateGameConfig();

                string Executable = Config.GetComName();
                string CommandLine = "CookPackages " + GetPlatform();

                for( int i = 0; i < Parms.Length; i++ )
                {
                    CommandLine += " " + Parms[i];
                }

                CommandLine += " -alwaysRecookmaps -alwaysRecookScript " + CommonCommandLine;

                string Language = Builder.LabelInfo.Language;
                if( Language.Length == 0 )
                {
                    Language = "INT";
                }

                CommandLine += " -LanguageForCooking=" + Language + Builder.GetCompressionConfiguration() + Builder.GetScriptConfiguration() + Builder.GetCookConfiguration();
                CommandLine += Builder.GetContentPath() + Builder.GetModName();

                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true );
                ErrorLevel = CurrentBuild.GetErrorLevel();

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CookMaps;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to cook" );
                    Log.Close();
                }
            }
        }

        private void CookSounds()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.CookSounds );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                Builder.Write( Log, Parent.SetRequestedSDKs() );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                GameConfig Config = Builder.CreateGameConfig();

                string Executable = Config.GetComName();
                string CommandLine = "ResavePackages -ResaveClass=SoundNodeWave -ForceSoundRecook";

                string Language = Builder.LabelInfo.Language;
                for( int i = 0; i < Parms.Length; i++ )
                {
                    CommandLine += " -Package=" + Parms[i];
                    if( Language.Length > 0 && Language != "INT" )
                    {
                        CommandLine += "_" + Language;
                    }
                }

                CommandLine += " " + CommonCommandLine;

                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true );
                ErrorLevel = CurrentBuild.GetErrorLevel();

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CookSounds;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to cook sounds" );
                    Log.Close();
                }
            }
        }

        private void CreateHashes()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.CreateHashes );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                Builder.Write( Log, Parent.SetRequestedSDKs() );

                GameConfig Config = Builder.CreateGameConfig();

                string Executable = Config.GetComName();
                string CommandLine = "CookPackages " + GetPlatform();

                CommandLine += " -sha -inisonly " + CommonCommandLine;

                string Language = Builder.LabelInfo.Language;
                if( Language.Length == 0 )
                {
                    Language = "INT";
                }
                CommandLine += " -LanguageForCooking=" + Language + Builder.GetScriptConfiguration();

                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true );
                ErrorLevel = CurrentBuild.GetErrorLevel();

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CreateHashes;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to create hashes" );
                    Log.Close();
                }
            }
        }

        private void Wrangle()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.Wrangle );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                Builder.Write( Log, Parent.SetRequestedSDKs() );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 2 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: Wrangle <Game> <Map1> [Map2...]." );
                    ErrorLevel = ERRORS.Wrangle;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig( Parms[0] );

                    Config.DeleteCutdownPackages( Parent );

                    string Executable = Config.GetComName();
                    string CommandLine = "WrangleContent ";

                    for( int i = 1; i < Parms.Length; i++ )
                    {
                        CommandLine += "-" + Parms[i] + "section ";
                    }

                    CommandLine += "-nosaveunreferenced -removeeditoronly " + CommonCommandLine;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.Wrangle;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to wrangle" );
                    Log.Close();
                }
            }
        }

        private void Publish( COMMANDS Command, ERRORS Error, string Tagset, bool AddToReport, string AdditionalOptions )
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( Command ), false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: " + Command.ToString() + " <Dest1> [Dest2...]" );
                    ErrorLevel = Error;
                }
                else
                {
                    string Executable = "Binaries/CookerSync.exe";
                    string CommandLine = Builder.LabelInfo.Game + " -p " + Builder.LabelInfo.Platform;

                    string Language = Builder.LabelInfo.Language;
                    if( Language.Length == 0 )
                    {
                        Language = "INT";
                    }

                    CommandLine += " -r " + Language;
                    CommandLine += " -b " + Builder.SourceControlBranch;
                    CommandLine += " -x " + Tagset;
                    CommandLine += " -crc -v" + Builder.GetInstallConfiguration();
                    if( Builder.ForceCopy )
                    {
                        CommandLine += " -f";
                    }
                    CommandLine += AdditionalOptions;

                    for( int i = 0; i < Parms.Length; i++ )
                    {
                        string PublishFolder = Parms[i].Replace( '/', '\\' ) + "\\" + Builder.LabelInfo.GetFolderName( Builder.AppendLanguage );
                        CommandLine += " " + PublishFolder;
                        if( AddToReport )
                        {
                            Builder.AddPublishDestination( PublishFolder );
                        }
                    }

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, Environment.CurrentDirectory + "/Binaries", false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = Error;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to publish" );
                    Log.Close();
                }
            }
        }

        private void Publish()
        {
            Publish( COMMANDS.Publish, ERRORS.Publish, "CompleteBuild", true, "" );
        }

        private void PublishLanguage()
        {
            Publish( COMMANDS.PublishLanguage, ERRORS.PublishLanguage, "Loc", false, "" );
        }

        private void PublishLayout()
        {
            Publish( COMMANDS.PublishLayout, ERRORS.PublishLayout, "Layout", true, " -notoc" );
        }

        private void PublishLayoutLanguage()
        {
            Publish( COMMANDS.PublishLayout, ERRORS.PublishLayout, "Loc", false, " -notoc" );
        }

        private void PublishDLC()
        {
            Publish( COMMANDS.PublishDLC, ERRORS.PublishDLC, "DLC", true, "" );
        }

        private void GetCookedData( COMMANDS BuildCommand, ERRORS Error, string TagSet )
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( BuildCommand ), false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: " + BuildCommand.ToString() + " <Source>" );
                    ErrorLevel = Error;
                }
                else
                {
                    string SourceFolder = Builder.LabelInfo.GetFolderName( Builder.AppendLanguage );
                    string Path = Parms[0] + "/" + SourceFolder + "/" + Builder.SourceControlBranch + "/Binaries";
                    string Executable = Environment.CurrentDirectory + "/Binaries/CookerSync.exe";
                    string CommandLine = Builder.LabelInfo.Game + " -p " + Builder.LabelInfo.Platform;
                    CommandLine += " -b " + Builder.SourceControlBranch;
                    CommandLine += " -notoc -x " + TagSet;
                    CommandLine += " -crc -v " + Parent.RootFolder;

                    string Language = Builder.LabelInfo.Language;
                    if( Language.Length == 0 )
                    {
                        Language = "INT";
                    }

                    CommandLine += " -r " + Language;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, Path, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = Error;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to get a cooked build/language" );
                    Log.Close();
                }
            }
        }

        private void GetCookedBuild()
        {
            GetCookedData( COMMANDS.GetCookedBuild, ERRORS.GetCookedBuild, "CookedData" );
        }

        private void GetCookedLanguage()
        {
            GetCookedData( COMMANDS.GetCookedLanguage, ERRORS.GetCookedLanguage, "Loc" );
        }

        private void GetInstallableBuild()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.GetInstallableBuild ), false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: GetInstallableBuild <Source>" );
                    ErrorLevel = ERRORS.GetInstallableBuild;
                }
                else
                {
                    string PublishedFolder = Builder.LabelInfo.GetFolderName( true );
                    string DestFolder = Builder.LabelInfo.GetFolderName( false );

                    string Path = Parms[0] + "/" + PublishedFolder + "/" + Builder.SourceControlBranch + "/Binaries";
                    Parent.Log( "Source folder: " + Path, Color.DarkGreen );
                    string Executable = Environment.CurrentDirectory + "/Binaries/CookerSync.exe";
                    string CommandLine = Builder.LabelInfo.Game + " -p " + Builder.LabelInfo.Platform;
                    CommandLine += " -b " + Builder.SourceControlBranch;
                    CommandLine += " -notoc -x Shipping";
                    CommandLine += " -crc -v C:\\Builds\\" + DestFolder;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, Path, false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();

                    Builder.CopyDestination = "C:\\Builds\\" + PublishedFolder;
                }
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.GetInstallableBuild;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to get an installable build" );
                    Log.Close();
                }
            }
        }

        private void BuildInstaller()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.BuildInstaller ), false, Encoding.ASCII );

                // Make sure we have the latest IS project files
                SCC.SyncBuildScripts( Builder.SourceControlBranch, "/Development/Install/..." );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: BuildInstaller <Package>" );
                    ErrorLevel = ERRORS.BuildInstaller;
                }
                else
                {
                    string PublishedFolder = Builder.LabelInfo.GetFolderName( true );

                    string Executable = Builder.ISDevLocation;
                    string CommandLine = "-p Development/Install/" + Parms[0] + "/" + Parms[0] + ".ism";

                    CommandLine += " -l PATH_TO_UNREALENGINE3_FILES=\"C:\\Builds\\" + PublishedFolder + "\\" + Builder.SourceControlBranch + "\"";

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();

                    // Force any future copies to go over the install folder
                    Builder.CopyDestination = "C:\\Install";
                }
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.BuildInstaller;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to building the installer" );
                    Log.Close();
                }
            }
        }

        private void CopyInstaller()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.CopyInstaller ), false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length < 1 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: BuildInstaller <Dest>" );
                    ErrorLevel = ERRORS.CopyInstaller;
                }
                else
                {
                    string DestFolder = Parms[0] + "/" + Builder.LabelInfo.GetFolderName( true ) + "_Install";
                    string Executable = "Binaries/ISCopyFiles.exe";
                    string CommandLine = "C:/Install " + DestFolder;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CopyInstaller;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to copy the installer" );
                    Log.Close();
                }
            }
        }

        private void CreateDVDLayout()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.CreateDVDLayout );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 1 )
                {
                    Builder.Write( Log, "Error, missing package name. Usage: Conform <Package>." );
                    ErrorLevel = ERRORS.Conform;
                }
                else
                {
                    string Executable = "Binaries/UnrealDVDLayout.exe";
                    string CommandLine = Builder.LabelInfo.Game + " " + Builder.LabelInfo.Platform;

                    Queue<string> Langs = Builder.GetLanguages();
                    foreach( string Lang in Langs )
                    {
                        CommandLine += " " + Lang.ToUpper();
                    }

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CreateDVDLayout;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to CreateDVDLayout" );
                    Log.Close();
                }
            }
        }

        private void Conform()
        {
            try
            {
                string Package, LastPackage;

                string LastLanguage = Builder.GetValidLanguages().Dequeue();
                string Language = Builder.GetValidLanguages().Peek();

                string LogFileName = Builder.GetLogFileName( COMMANDS.Conform, Language );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                Builder.Write( Log, Parent.SetRequestedSDKs() );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 1 )
                {
                    Builder.Write( Log, "Error, missing package name. Usage: Conform <Package>." );
                    ErrorLevel = ERRORS.Conform;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig();
                    string Executable = Config.GetComName();

                    if( LastLanguage == "INT" )
                    {
                        LastPackage = Parms[0];
                    }
                    else
                    {
                        LastPackage = Parms[0] + "_" + LastLanguage;
                    }
                    Package = Parms[0] + "_" + Language;

                    string CommandLine = "conform " + Package + " " + LastPackage + " " + CommonCommandLine;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.Conform;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to conform dialog" );
                    Log.Close();
                }
            }
        }

        private void CrossBuildConform()
        {
            try
            {
                string Package, LastPackage;

                string LogFileName = Builder.GetLogFileName( COMMANDS.CrossBuildConform, Builder.LabelInfo.Language );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                Builder.Write( Log, Parent.SetRequestedSDKs() );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 1 )
                {
                    Builder.Write( Log, "Error, missing package name. Usage: CrossBuildConform <Package>." );
                    ErrorLevel = ERRORS.CrossBuildConform;
                }
                else
                {
                    GameConfig Config = Builder.CreateGameConfig();
                    string Executable = Config.GetComName();

                    Package = Parms[0] + "_" + Builder.LabelInfo.Language + ".upk";
                    LastPackage = Builder.SourceBuild + "/" + Builder.SourceControlBranch + "/" + Package;

                    string CommandLine = "conform " + Package + " " + LastPackage + " " + CommonCommandLine;

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CrossBuildConform;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting cross build conform." );
                    Log.Close();
                }
            }
        }

        private void CreateContentTags()
        {
            try
            {
                string LogFileName = Builder.GetLogFileName( COMMANDS.CreateContentTags );
                StreamWriter Log = new StreamWriter( LogFileName, false, Encoding.ASCII );
                Builder.Write( Log, Parent.SetRequestedSDKs() );

                GameConfig Config = Builder.CreateGameConfig();
                string Executable = Config.GetComName();
                string CommandLine = "Editor.BuildContentTagIndex " + CommonCommandLine;

                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", true );
                ErrorLevel = CurrentBuild.GetErrorLevel();

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CreateContentTags;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception while starting to create content tags." );
                    Log.Close();
                }
            }
        }

        private void BumpEngineCpp( ScriptParser Builder, List<string> Lines )
        {
            // Bump ENGINE_VERSION and BUILT_FROM_CHANGELIST
            int BumpIncrement = 1;
            if( Builder.CommandLine.Length > 0 )
            {
                BumpIncrement = Builder.SafeStringToInt( Builder.CommandLine );
            }

            for( int i = 0; i < Lines.Count; i++ )
            {
                string[] Parms = Lines[i].Split( " \t".ToCharArray() );

                if( Parms.Length == 3 && Parms[0].ToUpper() == "#DEFINE" )
                {
                    if( Parms[1].ToUpper() == "MAJOR_VERSION" )
                    {
                        Builder.LabelInfo.BuildVersion = new Version( Builder.LabelInfo.SafeStringToInt( Parms[2] ),
                                                                        Builder.LabelInfo.BuildVersion.Minor,
                                                                        Builder.LabelInfo.BuildVersion.Build,
                                                                        Builder.LabelInfo.BuildVersion.Revision );
                    }

                    if( Parms[1].ToUpper() == "MINOR_VERSION" )
                    {
                        Builder.LabelInfo.BuildVersion = new Version( Builder.LabelInfo.BuildVersion.Major,
                                                                        Builder.LabelInfo.SafeStringToInt( Parms[2] ),
                                                                        Builder.LabelInfo.BuildVersion.Build,
                                                                        Builder.LabelInfo.BuildVersion.Revision );
                    }

                    if( Parms[1].ToUpper() == "ENGINE_VERSION" )
                    {
                        Builder.LabelInfo.BuildVersion = new Version( Builder.LabelInfo.BuildVersion.Major,
                                                                        Builder.LabelInfo.BuildVersion.Minor,
                                                                        Builder.LabelInfo.SafeStringToInt( Parms[2] ) + BumpIncrement,
                                                                        Builder.LabelInfo.BuildVersion.Revision );

                        Lines[i] = "#define\tENGINE_VERSION\t" + Builder.LabelInfo.BuildVersion.Build.ToString();
                    }

                    if( Parms[1].ToUpper() == "PRIVATE_VERSION" )
                    {
                        Builder.LabelInfo.BuildVersion = new Version( Builder.LabelInfo.BuildVersion.Major,
                                                                        Builder.LabelInfo.BuildVersion.Minor,
                                                                        Builder.LabelInfo.BuildVersion.Build,
                                                                        Builder.LabelInfo.SafeStringToInt( Parms[2] ) );
                    }

                    if( Parms[1].ToUpper() == "BUILT_FROM_CHANGELIST" )
                    {
                        Lines[i] = "#define\tBUILT_FROM_CHANGELIST\t" + Builder.LabelInfo.Changelist.ToString();
                    }
                }
            }
        }

        private string GetHexVersion( int EngineVersion )
        {
            string HexVersion = "";
            char[] HexDigits = "0123456789abcdef".ToCharArray();

            int MajorVer = Builder.LabelInfo.BuildVersion.Major;
            int MinorVer = Builder.LabelInfo.BuildVersion.Minor;

            // First 4 bits is major version
            HexVersion += HexDigits[MajorVer & 0xf];
            // Next 4 bits is minor version
            HexVersion += HexDigits[MinorVer & 0xf];
            // Next 16 bits is build number
            HexVersion += HexDigits[( EngineVersion >> 12 ) & 0xf];
            HexVersion += HexDigits[( EngineVersion >> 8 ) & 0xf];
            HexVersion += HexDigits[( EngineVersion >> 4 ) & 0xf];
            HexVersion += HexDigits[( EngineVersion >> 0 ) & 0xf];
            // Client code is required to have the first 4 bits be 0x8, where server code is required to have the first 4 bits be 0xC.
            HexVersion += HexDigits[0x8];
            // DiscID varies for different languages
            HexVersion += HexDigits[0x1];

            return ( HexVersion );
        }

        private void BumpEngineXml( ScriptParser Builder, List<string> Lines )
        {
            // Bump build version in Live! stuff
            for( int i = 0; i < Lines.Count; i++ )
            {
                if( Lines[i].Trim().StartsWith( "build=" ) )
                {
                    Lines[i] = "     build=\"" + Builder.LabelInfo.BuildVersion.Build.ToString() + "\"";
                }
                else if( Lines[i].Trim().StartsWith( "<titleversion>" ) )
                {
                    Lines[i] = "  <titleversion>" + GetHexVersion( Builder.LabelInfo.BuildVersion.Build ) + "</titleversion>";
                }
                else if( Lines[i].Trim().StartsWith( "<VersionNumber versionNumber=" ) )
                {
                    Lines[i] = "      <VersionNumber versionNumber=\"" + Builder.LabelInfo.BuildVersion.ToString() + "\" />";
                }
            }
        }

        private void BumpEngineHeader( ScriptParser Builder, List<string> Lines )
        {
            // Bump build version in Live! stuff
            for( int i = 0; i < Lines.Count; i++ )
            {
                if( Lines[i].Trim().StartsWith( "#define BUILD_NUM" ) )
                {
                    Lines[i] = "#define BUILD_NUM " + Builder.LabelInfo.BuildVersion.Build.ToString();
                }
                else if( Lines[i].Trim().StartsWith( "#define VER_STRING" ) )
                {
                    Lines[i] = "#define VER_STRING \"" + Builder.LabelInfo.BuildVersion.ToString() + "\"";
                }
            }
        }

        private void BumpEngineProperties( ScriptParser Builder, List<string> Lines, string TimeStamp, int ChangeList )
        {
            // Bump build version in properties file
            for( int i = 0; i < Lines.Count; i++ )
            {
                if( Lines[i].Trim().StartsWith( "timestampForBVT=" ) )
                {
                    Lines[i] = "timestampForBVT=" + TimeStamp;
                }
                else if( Lines[i].Trim().StartsWith( "changelistBuiltFrom=" ) )
                {
                    Lines[i] = "changelistBuiltFrom=" + ChangeList.ToString();
                }
            }
        }

        private void BumpVersionFile( ScriptParser Builder, string File, bool GetVersion )
        {
            List<string> Lines = new List<string>();
            string Line;

            // Check to see if the version file is writable (otherwise the StreamWriter creation will exception)
            FileInfo Info = new FileInfo( File );
            if( Info.IsReadOnly )
            {
                ErrorLevel = ERRORS.BumpEngineVersion;
                Builder.Write( Log, "Error, version file is read only '" + File + "'" );
                return;
            }

            // Read in existing file
            StreamReader Reader = new StreamReader( File );
            if( Reader == null )
            {
                ErrorLevel = ERRORS.BumpEngineVersion;
                Builder.Write( Log, "Error, failed to open for reading '" + File + "'" );
                return;
            }

            Line = Reader.ReadLine();
            while( Line != null )
            {
                Lines.Add( Line );

                Line = Reader.ReadLine();
            }

            Reader.Close();

            // Bump the version dependent on the file extension
            if( GetVersion && Info.Extension.ToLower() == ".cpp" )
            {
                BumpEngineCpp( Builder, Lines );
            }
            else
            {
                if( Info.Extension.ToLower() == ".xml" )
                {
                    BumpEngineXml( Builder, Lines );
                }
                else if( Info.Extension.ToLower() == ".properties" )
                {
                    BumpEngineProperties( Builder, Lines, Builder.GetTimeStamp(), Builder.LabelInfo.Changelist );
                }
                else if( Info.Extension.ToLower() == ".h" )
                {
                    BumpEngineHeader( Builder, Lines );
                }
                else
                {
                    ErrorLevel = ERRORS.BumpEngineVersion;
                    Builder.Write( Log, "Error, invalid extension for '" + File + "'" );
                    return;
                }
            }

            // Write out version
            StreamWriter Writer;
            if( File.ToLower().IndexOf( ".gdf.xml" ) >= 0 )
            {
                Writer = new StreamWriter( File, false, Encoding.Unicode );
            }
            else
            {
                Writer = new StreamWriter( File, false, Encoding.ASCII );
            }

            if( Writer == null )
            {
                ErrorLevel = ERRORS.BumpEngineVersion;
                Builder.Write( Log, "Error, failed to open for writing '" + File + "'" );
                return;
            }

            foreach( string SingleLine in Lines )
            {
                Writer.Write( SingleLine + Environment.NewLine );
            }

            Writer.Close();
        }

        private void BumpEngineVersion()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.BumpEngineVersion ), false, Encoding.ASCII );

                string File = Builder.EngineVersionFile;
                BumpVersionFile( Builder, File, true );

                string[] Files = Builder.MiscVersionFiles.Split( ";".ToCharArray() );
                foreach( string XmlFile in Files )
                {
                    BumpVersionFile( Builder, XmlFile, false );
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.BumpEngineVersion;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while bumping engine version in '" + Builder.EngineVersionFile + "'" );
                    Log.Close();
                }
            }
        }

        private int GetVersionFile( ScriptParser Builder, string File )
        {
            List<string> Lines = new List<string>();
            string Line;
            int NewEngineVersion = 0;

            // Check to see if the version file is writable (otherwise the StreamWriter creation will exception)
            FileInfo Info = new FileInfo( File );

            // Read in existing file
            StreamReader Reader = new StreamReader( File );
            if( Reader == null )
            {
                ErrorLevel = ERRORS.GetEngineVersion;
                Builder.Write( Log, "Error, failed to open for reading '" + File + "'" );
                return ( 0 );
            }

            Line = Reader.ReadLine();
            while( Line != null )
            {
                Lines.Add( Line );

                Line = Reader.ReadLine();
            }

            Reader.Close();

            for( int i = 0; i < Lines.Count; i++ )
            {
                string[] Parms = Lines[i].Split( " \t".ToCharArray() );

                if( Parms.Length == 3 && Parms[0].ToUpper() == "#DEFINE" )
                {
                    if( Parms[1].ToUpper() == "ENGINE_VERSION" )
                    {
                        NewEngineVersion = Builder.SafeStringToInt( Parms[2] );
                    }
                }
            }

            return ( NewEngineVersion );
        }

        private void GetEngineVersion()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.GetEngineVersion ), false, Encoding.ASCII );

                int EngineVersion = GetVersionFile( Builder, Builder.EngineVersionFile );
                Builder.LabelInfo.BuildVersion = new Version( Builder.LabelInfo.BuildVersion.Major,
                                                                Builder.LabelInfo.BuildVersion.Minor,
                                                                EngineVersion,
                                                                Builder.LabelInfo.BuildVersion.Revision );

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.BumpEngineVersion;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while getting engine version in '" + Builder.EngineVersionFile + "'" );
                    Log.Close();
                }
            }
        }

        private void UpdateGDFVersion()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.UpdateGDFVersion ), false, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 2 )
                {
                    Builder.Write( Log, "Error, too few parameters. Usage: UpdateGDFVersion <Game> <ResourcePath>." );
                    ErrorLevel = ERRORS.UpdateGDFVersion;
                }
                else
                {
                    int EngineVersion = Builder.LabelInfo.BuildVersion.Build;
                    Queue<string> Languages = Builder.GetLanguages();

                    foreach( string Lang in Languages )
                    {
                        string GDFFileName = Parms[1] + "/" + Lang.ToUpper() + "/" + Parms[0] + "Game.gdf.xml";
                        BumpVersionFile( Builder, GDFFileName, false );
                    }
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.BumpEngineVersion;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while bumping engine version in '" + Builder.EngineVersionFile + "'" );
                    Log.Close();
                }
            }
        }

        private void UpdateSourceServer()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.UpdateSourceServer ), false, Encoding.ASCII );

                string Executable = "Cmd.exe";
                string CommandLine = "/c \"" + Builder.SourceServerCmd + "\" " + Environment.CurrentDirectory + " " + Environment.CurrentDirectory + "\\Binaries";
                CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false );

                ErrorLevel = CurrentBuild.GetErrorLevel();
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.UpdateSourceServer;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while updating source server." );
                    Log.Close();
                }
            }
        }

        private void DeleteIndexFile( StreamWriter Log, string File )
        {
            FileInfo Info = new FileInfo( File );
            if( Info.Exists )
            {
                Info.IsReadOnly = false;
                Info.Delete();
                Builder.Write( Log, "Deleted: " + File );
            }
        }

        private void CreateCommandList( string Title, string Exec, string SymbolFile )
        {
            string Executable = "";
            string ExePath = "";
            string SymPath = "";

            string SymStore = Builder.SymbolStoreLocation;
            string ChangeList = Builder.LabelInfo.Changelist.ToString();
            string Version = Builder.LabelInfo.BuildVersion.Build.ToString();

            Builder.AddUpdateSymStore( "delete" );
            Builder.AddUpdateSymStore( "status " + Exec );

            // Symstore requires \ for some reason
            Executable = Environment.CurrentDirectory + "\\" + Exec.Replace( '/', '\\' );
            ExePath = Executable.Substring( 0, Executable.LastIndexOf( '\\' ) );

            SymbolFile = Environment.CurrentDirectory + "\\" + SymbolFile.Replace( '/', '\\' );
            SymPath = SymbolFile.Substring( 0, SymbolFile.LastIndexOf( '\\' ) );

            Builder.AddUpdateSymStore( "add /g " + ExePath + " /p /l /f " + Executable + " /x exe_index.txt /a /o" );
            Builder.AddUpdateSymStore( "add /g " + SymPath + " /p /l /f " + SymbolFile + " /x pdb_index.txt /a /o" );
            Builder.AddUpdateSymStore( "add /g " + ExePath + " /y exe_index.txt /l /s " + SymStore + " /t " + Title + " /v " + ChangeList + " /c " + Version + " /o /compress" );
            Builder.AddUpdateSymStore( "add /g " + SymPath + " /y pdb_index.txt /l /s " + SymStore + " /t " + Title + " /v " + ChangeList + " /c " + Version + " /o /compress" );
        }

        private void UpdateSymbolServer()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.UpdateSymbolServer ), false, Encoding.ASCII );

                if( Builder.LabelInfo.BuildVersion.Build == 0 )
                {
                    Builder.Write( Log, "Error, while updating symbol server." );
                    Builder.Write( Log, "Error, no engine version found; has BumpEngineVersion been called?" );
                    ErrorLevel = ERRORS.UpdateSymbolServer;
                }
                else
                {
                    List<GameConfig> GameConfigs = Builder.LabelInfo.GetGameConfigs();

                    foreach( GameConfig Config in GameConfigs )
                    {
                        if( Config.IsLocal )
                        {
                            string Title = Config.GetTitle();
                            string[] Executables = Config.GetExecutableNames();
                            string[] SymbolFiles = Config.GetSymbolFileNames();

                            if( Executables[0].Length > 0 && SymbolFiles[0].Length > 0 )
                            {
                                CreateCommandList( Title, Executables[0], SymbolFiles[0] );
                            }
                        }
                    }

                    // Clean up the files that may have permissions problems
                    Builder.AddUpdateSymStore( "delete" );
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.UpdateSymbolServer;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while updating symbol server." );
                    Log.Close();
                }
            }
        }

        private void UpdateSymbolServerTick()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.UpdateSymbolServerTick ), true, Encoding.ASCII );

                if( Builder.UpdateSymStoreEmpty() )
                {
                    // Force a finish
                    CurrentBuild = null;
                    Builder.Write( Log, "Finished!" );
                    Log.Close();
                }
                else
                {
                    string Executable = Builder.SymbolStoreApp;
                    string CommandLine = Builder.PopUpdateSymStore();

                    if( CommandLine == "delete" )
                    {
                        // Delete the index files
                        DeleteIndexFile( Log, "exe_index.txt" );
                        DeleteIndexFile( Log, "pdb_index.txt" );

                        CommandLine = Builder.PopUpdateSymStore();
                    }

                    if( CommandLine.StartsWith( "status " ) )
                    {
                        Parent.Log( "[STATUS] Updating symbol server for '" + CommandLine.Substring( "status ".Length ) + "'", Color.Magenta );

                        CommandLine = Builder.PopUpdateSymStore();
                    }

                    if( Builder.UpdateSymStoreEmpty() )
                    {
                        // Force a finish
                        CurrentBuild = null;
                        Builder.Write( Log, "Finished!" );
                        Log.Close();
                        return;
                    }

                    CurrentBuild = new BuildProcess( Parent, Builder, Log, Executable, CommandLine, "", false );
                    ErrorLevel = CurrentBuild.GetErrorLevel();
                }

                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.UpdateSymbolServer;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while updating symbol server." );
                    Log.Close();
                }
            }
        }

        private void CheckSigned()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.CheckSigned ), true, Encoding.ASCII );

                Parent.Log( "[STATUS] Checking '" + Builder.CommandLine + "' for signature", Color.Magenta );

                string SignToolName = Builder.SignToolName;
                string CommandLine = "verify /pa /v " + Builder.CommandLine;

                CurrentBuild = new BuildProcess( Parent, Builder, Log, SignToolName, CommandLine, "", false );
                ErrorLevel = ERRORS.CheckSigned;
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.CheckSigned;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while checking for signed binaries." );
                    Log.Close();
                }
            }
        }

        private void Sign()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.Sign ), true, Encoding.ASCII );

                Parent.Log( "[STATUS] Signing '" + Builder.CommandLine + "'", Color.Magenta );

                string SignToolName = Builder.SignToolName;
                string CommandLine = "sign /f Development/Builder/Auth/EpicGames.pfx /v " + Builder.CommandLine;

                CurrentBuild = new BuildProcess( Parent, Builder, Log, SignToolName, CommandLine, "", false );
                ErrorLevel = CurrentBuild.GetErrorLevel();
                StartTime = DateTime.Now;
            }
            catch
            {
                ErrorLevel = ERRORS.Sign;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, while signing binaries." );
                    Log.Close();
                }
            }
        }

        private void SimpleCopy()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SimpleCopy ), true, Encoding.ASCII );

                string PathName = Builder.CommandLine.Trim();
                Builder.Write( Log, "Copying: " + PathName );
                Builder.Write( Log, " ... to: " + Builder.CopyDestination );

                FileInfo Source = new FileInfo( PathName );
                if( Source.Exists )
                {
                    // Get the filename
                    int FileNameOffset = PathName.LastIndexOf( '/' );
                    string FileName = PathName.Substring( FileNameOffset + 1 );
                    string DestPathName = Builder.CopyDestination + "/" + FileName;

                    // Create the dest folder if it doesn't exist
                    DirectoryInfo DestDir = new DirectoryInfo( Builder.CopyDestination );
                    if( !DestDir.Exists )
                    {
                        Builder.Write( Log, " ... creating: " + DestDir.FullName );
                        DestDir.Create();
                    }

                    // Copy the file
                    FileInfo Dest = new FileInfo( DestPathName );
                    if( Dest.Exists )
                    {
                        Builder.Write( Log, " ... deleting: " + Dest.FullName );
                        Dest.IsReadOnly = false;
                        Dest.Delete();
                    }
                    Source.CopyTo( DestPathName, true );
                }
                else
                {
                    Builder.Write( Log, "Error, source file does not exist for copying" );
                    ErrorLevel = ERRORS.SimpleCopy;
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SimpleCopy;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception copying file" );
                    Log.Close();
                }
            }
        }

        private void SourceBuildCopy()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SourceBuildCopy ), true, Encoding.ASCII );

                string DestFile = Builder.CommandLine.Trim();
                string SourceFile = Builder.SourceBuild + "/" + Builder.SourceControlBranch + "/" + DestFile;

                Builder.Write( Log, "Copying: " );
                Builder.Write( Log, " ... from: " + SourceFile );
                Builder.Write( Log, " ... to: " + DestFile );

                FileInfo Source = new FileInfo( SourceFile );
                if( Source.Exists )
                {
                    FileInfo Dest = new FileInfo( DestFile );
                    if( Dest.Exists && Dest.IsReadOnly )
                    {
                        Dest.IsReadOnly = false;
                    }

                    // Create the dest folder if it doesn't exist
                    DirectoryInfo DestDir = new DirectoryInfo( Dest.DirectoryName );
                    if( !DestDir.Exists )
                    {
                        Builder.Write( Log, " ... creating: " + DestDir.FullName );
                        DestDir.Create();
                    }

                    Source.CopyTo( DestFile, true );
                }
                else
                {
                    Builder.Write( Log, "Error, source file does not exist for copying" );
                    ErrorLevel = ERRORS.SourceBuildCopy;
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SourceBuildCopy;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception copying file from alternate build" );
                    Log.Close();
                }
            }
        }

        private void SimpleDelete()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SimpleDelete ), true, Encoding.ASCII );

                string PathName = Builder.CommandLine.Trim();

                Builder.Write( Log, "Deleting: " + PathName );

                FileInfo Source = new FileInfo( PathName );
                if( Source.Exists )
                {
                    Source.Delete();
                }
                else
                {
                    Builder.Write( Log, "Error, source file does not exist for deletion" );
                    ErrorLevel = ERRORS.SimpleDelete;
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SimpleDelete;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception deleting file" );
                    Log.Close();
                }
            }
        }

        private void SimpleRename()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.SimpleRename ), true, Encoding.ASCII );

                string[] Parms = Builder.CommandLine.Split( " \t".ToCharArray() );
                if( Parms.Length != 2 )
                {
                    Builder.Write( Log, "Error, while renaming file (wrong number of parameters)" );
                    ErrorLevel = ERRORS.SimpleRename;
                }
                else
                {
                    string BaseFolder = "";
                    if( Builder.CopyDestination.Length > 0 )
                    {
                        BaseFolder = Builder.CopyDestination + "/" + Builder.SourceControlBranch + "/";
                    }

                    Builder.Write( Log, "Renaming: " );
                    Builder.Write( Log, " ... from: " + BaseFolder + Parms[0] );
                    Builder.Write( Log, " ... to: " + BaseFolder + Parms[1] );

                    FileInfo Source = new FileInfo( BaseFolder + Parms[0] );
                    if( Source.Exists )
                    {
                        FileInfo Dest = new FileInfo( BaseFolder + Parms[1] );
                        if( Dest.Exists )
                        {
                            Dest.IsReadOnly = false;
                            Dest.Delete();
                        }
                        Source.IsReadOnly = false;

                        Source.CopyTo( Dest.FullName );
                        Source.Delete();
                    }
                    else
                    {
                        Builder.Write( Log, "Error, source file does not exist for renaming" );
                        ErrorLevel = ERRORS.SimpleRename;
                    }
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.SimpleRename;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception renaming file" );
                    Log.Close();
                }
            }
        }

        private void RenamedCopy()
        {
            try
            {
                StreamWriter Log = new StreamWriter( Builder.GetLogFileName( COMMANDS.RenamedCopy ), true, Encoding.ASCII );

                string[] Parms = ExtractParameters( Builder.CommandLine );
                if( Parms.Length != 2 )
                {
                    Builder.Write( Log, "Error, while renaming and copying file (wrong number of parameters)" );
                    ErrorLevel = ERRORS.RenamedCopy;
                }
                else
                {
                    Builder.Write( Log, "Renaming and copying: " );
                    Builder.Write( Log, " ... from: " + Parms[0] );
                    Builder.Write( Log, " ... to: " + Parms[1] );

                    FileInfo Source = new FileInfo( Parms[0] );
                    if( Source.Exists )
                    {
                        FileInfo Dest = new FileInfo( Parms[1] );
                        if( !Dest.Exists || ( Dest.Exists && !Dest.IsReadOnly ) )
                        {
                            if( Dest.Exists )
                            {
                                Builder.Write( Log, " ... deleting: " + Dest.FullName );
                                Dest.Delete();
                            }
                            else
                            {
                                Builder.Write( Log, " ... creating: " + Dest.DirectoryName );
                                Parent.EnsureDirectoryExists( Dest.DirectoryName );
                            }

                            Source.IsReadOnly = false;
                            Source.CopyTo( Dest.FullName );
                        }
                        else
                        {
                            Builder.Write( Log, "Error, destination file is read only" );
                            ErrorLevel = ERRORS.RenamedCopy;
                        }
                    }
                    else
                    {
                        Builder.Write( Log, "Error, source file does not exist for renaming and copying" );
                        ErrorLevel = ERRORS.RenamedCopy;
                    }
                }

                Log.Close();
            }
            catch
            {
                ErrorLevel = ERRORS.RenamedCopy;
                if( Log != null )
                {
                    Builder.Write( Log, "Error, exception renaming and copying file" );
                    Log.Close();
                }
            }
        }

        public MODES Execute( COMMANDS CommandID )
        {
            LastExecutedCommand = CommandID;

            switch( CommandID )
            {
                case COMMANDS.SCC_CheckConsistency:
                    SCC_CheckConsistency();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_Sync:
                    SCC_Sync();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_ArtistSync:
                    SCC_ArtistSync();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_GetChanges:
                    SCC_GetChanges();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_SyncSingleChangeList:
                    SCC_SyncSingleChangeList();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_Checkout:
                    SCC_Checkout();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_OpenForDelete:
                    SCC_OpenForDelete();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutGame:
                    SCC_CheckoutGame();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutContentTag:
                    SCC_CheckoutContentTag();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutShader:
                    SCC_CheckoutShader();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutDialog:
                    SCC_CheckoutDialog();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutFonts:
                    SCC_CheckoutFonts();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutLocPackage:
                    SCC_CheckoutLocPackage();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutGDF:
                    SCC_CheckoutGDF();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CheckoutLayout:
                    SCC_CheckoutLayout();
                    return ( MODES.Finalise );

                case COMMANDS.MakeWritable:
                    MakeWritable();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_Submit:
                    SCC_Submit();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_CreateNewLabel:
                    SCC_CreateNewLabel();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_UpdateLabelDescription:
                    SCC_UpdateLabelDescription();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_Revert:
                    SCC_Revert();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_RevertFile:
                    SCC_RevertFile();
                    return ( MODES.Finalise );

                case COMMANDS.MakeReadOnly:
                    MakeReadOnly();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_Tag:
                    SCC_Tag();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_TagFile:
                    SCC_TagFile();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_TagPCS:
                    SCC_TagPCS();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_TagExe:
                    SCC_TagExe();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_TagContentTag:
                    SCC_TagContentTag();
                    return ( MODES.Finalise );

                case COMMANDS.SCC_TagLayout:
                    SCC_TagLayout();
                    return ( MODES.Finalise );

                case COMMANDS.AddJob:
                    AddJob();
                    return ( MODES.Finalise );

                case COMMANDS.AddUnrealGameJob:
                    AddUnrealGameJob();
                    return ( MODES.Finalise );

                case COMMANDS.AddUnrealFullGameJob:
                    AddUnrealFullGameJob();
                    return ( MODES.Finalise );

                case COMMANDS.Clean:
                    Clean();
                    return ( MODES.Finalise );

                case COMMANDS.MSBuild:
                    MS_Build();
                    return ( MODES.Monitor );

                case COMMANDS.MSVCClean:
                    MSVC_Clean();
                    return ( MODES.Monitor );

                case COMMANDS.MSVCBuild:
                    MSVC_Build();
                    return ( MODES.Monitor );

                case COMMANDS.GCCClean:
                    GCC_Clean();
                    return ( MODES.Monitor );

                case COMMANDS.GCCBuild:
                    GCC_Build();
                    return ( MODES.Monitor );

                case COMMANDS.UnrealBuild:
                    Unreal_Build();
                    return ( MODES.Monitor );

                case COMMANDS.ShaderClean:
                    Shader_Clean();
                    return ( MODES.Finalise );

                case COMMANDS.ShaderBuild:
                    Shader_Build();
                    return ( MODES.Monitor );

                case COMMANDS.ShaderBuildState:
                    Shader_BuildState();
                    return ( MODES.Monitor );

                case COMMANDS.PS3MakePatchBinary:
                    PS3MakePatchBinary();
                    return ( MODES.Monitor );

                case COMMANDS.PS3MakePatch:
                    PS3MakePatch();
                    return ( MODES.Monitor );

                case COMMANDS.BuildScript:
                    BuildScript();
                    return ( MODES.Monitor );

                case COMMANDS.PreHeatMapOven:
                    PreHeat();
                    return ( MODES.Finalise );

                case COMMANDS.PreHeatDLC:
                    PreHeatDLC();
                    return ( MODES.Finalise );

                case COMMANDS.CookMaps:
                    Cook();
                    return ( MODES.Monitor );

                case COMMANDS.CookSounds:
                    CookSounds();
                    return ( MODES.Monitor );

                case COMMANDS.CreateHashes:
                    CreateHashes();
                    return ( MODES.Monitor );

                case COMMANDS.Wrangle:
                    Wrangle();
                    return ( MODES.Monitor );

                case COMMANDS.Publish:
                    Publish();
                    return ( MODES.Monitor );

                case COMMANDS.PublishLanguage:
                    PublishLanguage();
                    return ( MODES.Monitor );

                case COMMANDS.PublishLayout:
                    PublishLayout();
                    return ( MODES.Monitor );

                case COMMANDS.PublishLayoutLanguage:
                    PublishLayoutLanguage();
                    return ( MODES.Monitor );

                case COMMANDS.PublishDLC:
                    PublishDLC();
                    return ( MODES.Monitor );

                case COMMANDS.GetCookedBuild:
                    GetCookedBuild();
                    return ( MODES.Monitor );

                case COMMANDS.GetCookedLanguage:
                    GetCookedLanguage();
                    return ( MODES.Monitor );

                case COMMANDS.GetInstallableBuild:
                    GetInstallableBuild();
                    return ( MODES.Monitor );

                case COMMANDS.BuildInstaller:
                    BuildInstaller();
                    return ( MODES.Monitor );

                case COMMANDS.CopyInstaller:
                    CopyInstaller();
                    return ( MODES.Monitor );

                case COMMANDS.CreateDVDLayout:
                    CreateDVDLayout();
                    return ( MODES.Monitor );

                case COMMANDS.Conform:
                    Conform();
                    return ( MODES.Monitor );

                case COMMANDS.CreateContentTags:
                    CreateContentTags();
                    return ( MODES.Monitor );

                case COMMANDS.CrossBuildConform:
                    CrossBuildConform();
                    return ( MODES.Monitor );

                case COMMANDS.BumpEngineVersion:
                    BumpEngineVersion();
                    return ( MODES.Finalise );

                case COMMANDS.GetEngineVersion:
                    GetEngineVersion();
                    return ( MODES.Finalise );

                case COMMANDS.UpdateGDFVersion:
                    UpdateGDFVersion();
                    return ( MODES.Finalise );

                case COMMANDS.UpdateSourceServer:
                    UpdateSourceServer();
                    return ( MODES.Monitor );

                case COMMANDS.UpdateSymbolServer:
                    UpdateSymbolServer();
                    return ( MODES.Finalise );

                case COMMANDS.UpdateSymbolServerTick:
                    UpdateSymbolServerTick();
                    return ( MODES.Monitor );

                case COMMANDS.Blast:
                    Blast();
                    return ( MODES.Monitor );

                case COMMANDS.CheckSigned:
                    CheckSigned();
                    return ( MODES.Monitor );

                case COMMANDS.Sign:
                    Sign();
                    return ( MODES.Monitor );

                case COMMANDS.Cleanup:
                    Cleanup();
                    return ( MODES.Finalise );

                case COMMANDS.SimpleCopy:
                    SimpleCopy();
                    return ( MODES.Finalise );

                case COMMANDS.SourceBuildCopy:
                    SourceBuildCopy();
                    return ( MODES.Finalise );

                case COMMANDS.SimpleDelete:
                    SimpleDelete();
                    return ( MODES.Finalise );

                case COMMANDS.SimpleRename:
                    SimpleRename();
                    return ( MODES.Finalise );

                case COMMANDS.RenamedCopy:
                    RenamedCopy();
                    return ( MODES.Finalise );
            }

            return ( MODES.Init );
        }

        public MODES IsFinished()
        {
            // Also check for timeout
            if( CurrentBuild != null )
            {
                if( CurrentBuild.IsFinished )
                {
                    CurrentBuild.Cleanup();
                    return ( MODES.Finalise );
                }

                if( DateTime.Now - StartTime > Builder.GetTimeout() )
                {
                    CurrentBuild.Kill();
                    ErrorLevel = ERRORS.TimedOut;
                    return ( MODES.Finalise );
                }

                if( !CurrentBuild.IsResponding() )
                {
                    if( DateTime.Now - LastRespondingTime > Builder.GetRespondingTimeout() )
                    {
                        CurrentBuild.Kill();
                        ErrorLevel = ERRORS.Crashed;
                        return ( MODES.Finalise );
                    }
                }
                else
                {
                    LastRespondingTime = DateTime.Now;
                }

                return ( MODES.Monitor );
            }

            // No running build? Something went wrong
            return ( MODES.Finalise );
        }

        public void Kill()
        {
            if( CurrentBuild != null )
            {
                CurrentBuild.Kill();
            }
        }
    }
}
