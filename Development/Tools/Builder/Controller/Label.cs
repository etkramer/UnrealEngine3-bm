using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;

namespace Controller
{
    class LabelInfo
    {
        // Hook back to main controller
        private Main Parent = null;

        // List of games that this build represents
        public List<GameConfig> Games = new List<GameConfig>();

        // Name of the platform this label refers to - blank is all platforms
        private RevisionType LocalRevisionType = RevisionType.Invalid;
        public RevisionType RevisionType
        {
            get { return ( LocalRevisionType ); }
            set { LocalRevisionType = value; }
        }

        // The branch this revision lives in
        private string LocalBranch = "UnrealEngine3";
        public string Branch
        {
            get { return ( LocalBranch ); }
            set { LocalBranch = value; }
        }

        // Name of the game this label refers to - blank is all games
        private string LocalGame = "";
        public string Game
        {
            get { return ( LocalGame ); }
            set { LocalGame = value; }
        }

        // Name of the platform this label refers to - blank is all platforms
        private string LocalPlatform = "";
        public string Platform
        {
            get { return ( LocalPlatform ); }
            set { LocalPlatform = value; }
        }

        // Timestamp of this label
        private DateTime LocalTimestamp = DateTime.Now;
        public DateTime Timestamp
        {
            get { return ( LocalTimestamp ); }
            set { LocalTimestamp = value; }
        }

        // The changelist of this revision
        private int LocalChangelist = 0;
        public int Changelist
        {
            get { return ( LocalChangelist ); }
            set { LocalChangelist = value; }
        }

        // The four digit version to apply to Windows binaries
        private Version LocalBuildVersion = new Version( 1, 0, 0, 131 );
        public Version BuildVersion
        {
            get { return ( LocalBuildVersion ); }
            set { LocalBuildVersion = value; }
        }
        
        // Up to two build defines used in this compile this build
        private string LocalDefineA = "";
        private string LocalDefineB = "";

        public string DefineA
        {
            get { return ( LocalDefineA ); }
            set { LocalDefineA = value; }
        }

        public string DefineB
        {
            get { return ( LocalDefineB ); }
            set { LocalDefineB = value; }
        }

        // The language of this revision (if any)
        private string LocalLanguage = "";
        public string Language
        {
            get { return ( LocalLanguage ); }
            set { LocalLanguage = value; }
        }

        // The type of build being built
        private string LocalBuildType = "";
        public string BuildType
        {
            get { return ( LocalBuildType ); }
            set { LocalBuildType = value; }
        }

		// The perforce label description
		private string LocalDescription = "";
		public string Description
		{
			get { return (LocalDescription); }
			set { LocalDescription = value; }
		}

        public LabelInfo( Main InParent, ScriptParser Builder )
        {
            Parent = InParent;
            Branch = Builder.SourceControlBranch;
            Timestamp = Builder.BuildStartedAt;
        }

        public List<GameConfig> GetGameConfigs()
        {
            return ( Games );
        }

        public int SafeStringToInt( string Number )
        {
            int Result = 0;

            try
            {
                Number = Number.Trim();
                Result = Int32.Parse( Number );
            }
            catch
            {
            }

            return ( Result );
        }

        public string GetRootLabelName()
        {
            string TimeStampString = Timestamp.Year + "-"
                        + Timestamp.Month.ToString( "00" ) + "-"
                        + Timestamp.Day.ToString( "00" ) + "_"
                        + Timestamp.Hour.ToString( "00" ) + "."
                        + Timestamp.Minute.ToString( "00" );

            string LabelName = Branch + "_[" + TimeStampString + "]";

            return ( LabelName );
        }

        // Gets a suitable label for using in Perforce
        public string GetLabelName()
        {
            string TimeStampString = Timestamp.Year + "-"
                        + Timestamp.Month.ToString( "00" ) + "-"
                        + Timestamp.Day.ToString( "00" ) + "_"
                        + Timestamp.Hour.ToString( "00" ) + "."
                        + Timestamp.Minute.ToString( "00" );

            string LabelName = Branch + "_[" + TimeStampString + "]";

            if( DefineA.Length > 0 )
            {
                LabelName += "_[" + DefineA + "]";
            }

            if( DefineB.Length > 0 )
            {
                LabelName += "_[" + DefineB + "]";
            }

            return ( LabelName );
        }

        public void ClearDefines()
        {
            DefineA = "";
            DefineB = "";
        }

        public void AddDefine( string Define )
        {
            if( DefineA.Length == 0 || Define == DefineA )
            {
                DefineA = Define;
            }
            else if( DefineB.Length == 0 || Define == DefineB )
            {
                DefineB = Define;
            }
            else
            {
                Parent.Log( "Error, unable to add define '" + Define + "'", Color.Red );
            }
        }

        // Extract a define (of potentially two) from a build label
        private string GetDefine( string Label )
        {
            int OpenBracketIndex, CloseBracketIndex;
            string Define = "";

            OpenBracketIndex = Label.IndexOf( '[' );
            if( OpenBracketIndex > 0 )
            {
                CloseBracketIndex = Label.IndexOf( ']', OpenBracketIndex );
                if( CloseBracketIndex > 0 )
                {
                    Define = Label.Substring( OpenBracketIndex + 1, CloseBracketIndex - OpenBracketIndex - 1 );
                }
            }

            return ( Define );
        }

        // Extract a define (of potentially three) from a build label
        private string GetLanguage( string Label )
        {
            int OpenBracketIndex, CloseBracketIndex;
            string Lang = "";

            OpenBracketIndex = Label.IndexOf( '{' );
            if( OpenBracketIndex > 0 )
            {
                CloseBracketIndex = Label.IndexOf( '}', OpenBracketIndex );
                if( CloseBracketIndex > 0 )
                {
                    Lang = Label.Substring( OpenBracketIndex + 1, CloseBracketIndex - OpenBracketIndex - 1 );
                }
            }

            // Make sure it is a standard 3 letter code
            if( Lang.Length != 3 )
            {
                Lang = "";
            }

            return ( Lang );
        }

        // Sets up the label data from an existing Perforce label
        private bool LabelFromLabel( string Label )
        {
            int OpenBracketIndex;

            OpenBracketIndex = Label.IndexOf( '[' );
            if( OpenBracketIndex >= 1 )
            {
                // Extract the branch
                string ExtractedBranch = Label.Substring( 0, OpenBracketIndex - 1 );
#if !DEBUG
                if( ExtractedBranch == Branch )
#endif
                {
                    Label = Label.Substring( OpenBracketIndex );

                    if( Label.Length >= "[YYYY-MM-DD_HH.MM]".Length )
                    {
                        // Extract the timestamp
                        string TimeStampString = Label.Substring( 1, "YYYY-MM-DD_HH.MM".Length );

                        int Year = SafeStringToInt( TimeStampString.Substring( 0, 4 ) );
                        int Month = SafeStringToInt( TimeStampString.Substring( 5, 2 ) );
                        int Day = SafeStringToInt( TimeStampString.Substring( 8, 2 ) );
                        int Hour = SafeStringToInt( TimeStampString.Substring( 11, 2 ) );
                        int Minute = SafeStringToInt( TimeStampString.Substring( 14, 2 ) );

                        Timestamp = new DateTime( Year, Month, Day, Hour, Minute, 0 );

                        Label = Label.Substring( "[YYYY-MM-DD_HH.MM]".Length );

                        // Extract the defines (if any)
                        DefineA = GetDefine( Label );
                        if( DefineA.Length > 0 )
                        {
                            Label = Label.Substring( DefineA.Length + "_[]".Length );
                            DefineB = GetDefine( Label );
                            if( DefineB.Length > 0 )
                            {
                                Label = Label.Substring( DefineB.Length + "_[]".Length );
                            }
                        }

                        Language = GetLanguage( Label );
                        return ( true );
                    }
                }
            }
            return ( false );
        }

        // Gets a suitable folder name for publishing data to, or getting data from
        public string GetFolderName( bool AddLanguage )
        {
            string TimeStampString = Timestamp.Year + "-"
                        + Timestamp.Month.ToString( "00" ) + "-"
                        + Timestamp.Day.ToString( "00" ) + "_"
                        + Timestamp.Hour.ToString( "00" ) + "."
                        + Timestamp.Minute.ToString( "00" );

            string FolderName = Game + "_" + Platform + "_[" + TimeStampString + "]";

            if( DefineA.Length > 0 )
            {
                FolderName += "_[" + DefineA + "]";
            }

            if( DefineB.Length > 0 )
            {
                FolderName += "_[" + DefineB + "]";
            }

            if( AddLanguage && Language.Length > 0 )
            {
                FolderName += "_{" + Language + "}";
            }

            return( FolderName );
        }

        // Sets up the label data from an existing folder name
        private bool LabelFromFolder( string Folder )
        {
            int UnderscoreIndex;

            UnderscoreIndex = Folder.IndexOf( '_' );
            if( UnderscoreIndex > 0 )
            {
                // Extract the branch
                Game = Folder.Substring( 0, UnderscoreIndex );
                Folder = Folder.Substring( UnderscoreIndex + 1 );

                UnderscoreIndex = Folder.IndexOf( '_' );
                if( UnderscoreIndex > 0 )
                {
                    Platform = Folder.Substring( 0, UnderscoreIndex );
                    Folder = Folder.Substring( UnderscoreIndex + 1 );

                    if( Folder.Length >= "[YYYY-MM-DD_HH.MM]".Length )
                    {
                        // Extract the timestamp
                        string TimeStampString = Folder.Substring( 1, "YYYY-MM-DD_HH.MM".Length );

                        int Year = SafeStringToInt( TimeStampString.Substring( 0, 4 ) );
                        int Month = SafeStringToInt( TimeStampString.Substring( 5, 2 ) );
                        int Day = SafeStringToInt( TimeStampString.Substring( 8, 2 ) );
                        int Hour = SafeStringToInt( TimeStampString.Substring( 11, 2 ) );
                        int Minute = SafeStringToInt( TimeStampString.Substring( 14, 2 ) );

                        Timestamp = new DateTime( Year, Month, Day, Hour, Minute, 0 );

                        Folder = Folder.Substring( "[YYYY-MM-DD_HH.MM]".Length );

                        // Extract the defines (if any)
                        DefineA = GetDefine( Folder );
                        if( DefineA.Length > 0 )
                        {
                            Folder = Folder.Substring( DefineA.Length + "_[]".Length );
                            DefineB = GetDefine( Folder );
                            if( DefineB.Length > 0 )
                            {
                                Folder = Folder.Substring( DefineB.Length + "_[]".Length );
                            }
                        }

                        Language = GetLanguage( Folder );

                        return ( true );
                    }
                }
            }

            return ( false );
        }

        private int CountGames( List<GameConfig> GameList, GameConfig InGame )
        {
            int Count = 0;
            foreach( GameConfig Game in GameList )
            {
                if( Game.Similar( InGame ) )
                {
                    Count++;
                }
            }

            return ( Count );
        }

        public List<GameConfig> ConsolidateGames()
        {
            List<GameConfig> NewGames = new List<GameConfig>();
            List<GameConfig> MiscGames = new List<GameConfig>();

            // Create a list of games that are compiled for all platforms
            for( int i = 0; i < Games.Count; i++ )
            {
                if( CountGames( NewGames, Games[i] ) == 0 )
                {
                    int GameCount = CountGames( Games, Games[i] );
                    if( GameCount >= 2 )
                    {
                        GameConfig NewGame = new GameConfig( GameCount, Games[i] );
                        NewGames.Add( NewGame );
                    }
                }
            }

            // Copy anything left over
            for( int i = 0; i < Games.Count; i++ )
            {
                if( CountGames( NewGames, Games[i] ) == 0 )
                {
                    MiscGames.Add( Games[i] );
                }
            }

            NewGames.AddRange( MiscGames );
            return ( NewGames );
        }

        // Creates the description string from the available data
        public void CreateLabelDescription()
        {
            // Otherwise, build a string of the configs built using what
            StringBuilder DescriptionBuilder = new StringBuilder();

            DescriptionBuilder.Append( "[BUILDER] '" + BuildType + "' built from changelist: " + Changelist.ToString() + Environment.NewLine + Environment.NewLine );

            DescriptionBuilder.Append( "Engine version: " + BuildVersion.Build.ToString() + Environment.NewLine + Environment.NewLine );

            DescriptionBuilder.Append( "Configurations built:" + Environment.NewLine );
            if( Games.Count > 0 )
            {
                List<GameConfig> ConsolidatedGames = ConsolidateGames();

                foreach( GameConfig Config in ConsolidatedGames )
                {
                    DescriptionBuilder.Append( "\t" + Config.ToString() + Environment.NewLine );
                }
            }
            else
            {
                DescriptionBuilder.Append( "\tNone!" + Environment.NewLine );
            }

            DescriptionBuilder.Append( Environment.NewLine + "Visual Studio version: " + Parent.MSVCVersion + " with DirectX " + Parent.DXVersion + Environment.NewLine );
            DescriptionBuilder.Append( Environment.NewLine + "XDK version: " + Parent.XDKVersion + " and PS3 SDK: " + Parent.PS3SDKVersion + " were used." + Environment.NewLine );

            Description = DescriptionBuilder.ToString();
        }

        // Extract integers from the label description
        private int GetInteger( string Key, string Info )
        {
            int LabelInteger = 0;

            if( Info != null )
            {
                int Index = Info.IndexOf( Key );
                if( Index >= 0 )
                {
                    string ListText = Info.Substring( Index + Key.Length ).Trim();
                    string[] Parms = ListText.Split( " \t\r\n".ToCharArray() );
                    if( Parms.Length > 0 )
                    {
                        LabelInteger = SafeStringToInt( Parms[0] );
                    }
                }
            }

            return ( LabelInteger );
        }

        // Callback from GetLabelInfo()
        public void HandleDescription( string InDescription )
        {
            // Get the changelist this was built from
            Changelist = GetInteger( "changelist:", InDescription );
            // Get the engine version of the label
            BuildVersion = new Version( BuildVersion.Major,
                                        BuildVersion.Minor,
                                        GetInteger( "Engine version:", InDescription ),
                                        BuildVersion.Revision );

            Description = InDescription;
        }

        // Create a new instance of a label, extract info from the name and then from Perforce
        public void Init( P4 SCC, ScriptParser Builder )
        {
            bool ValidLabel = false;

            // No cross branch operations
            Branch = Builder.SourceControlBranch;

            // Work out what type of dependency and extract the relevant data
            string Dependency = Builder.Dependency;
            if( Dependency.Length > 0 )
            {
                // Dependency could be a Perforce label or a folder name derived from a Perforce label
                if( Dependency.StartsWith( Parent.LabelRoot ) )
                {
                    ValidLabel = LabelFromLabel( Dependency );
                }
                else
                {
                    ValidLabel = LabelFromFolder( Dependency );
                }

                // Update the remaining info from perforce
                // Always use the root label name as everything is derived from that
                ValidLabel &= SCC.GetLabelInfo( this, Builder, GetRootLabelName() );
            }

            if( ValidLabel )
            {
                Parent.Log( "Label '" + GetLabelName() + "' successfully created.", Color.DarkGreen );
                RevisionType = RevisionType.Label;
            }
            else
            {
                Parent.Log( "Label '" + GetLabelName() + "' failed to be created.", Color.Red );
                RevisionType = RevisionType.Invalid;
            }
        }

        public void Init( ScriptParser Builder, int InChangeList )
        {
            // No cross branch operations
            Branch = Builder.SourceControlBranch;

            // Set the changelist
            Changelist = InChangeList;

            RevisionType = RevisionType.ChangeList;
        }
    }
}
