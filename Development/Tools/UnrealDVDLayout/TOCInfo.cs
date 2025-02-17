using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Xml;
using System.Xml.Serialization;
using System.Text;
using System.Text.RegularExpressions;

namespace UnrealDVDLayout
{
    public class TOCInfo
    {
        // The type of the TOC entry
        public UnrealDVDLayout.ObjectType Type;
        // DVD destination layer (0 or 1)
        public int Layer = -1;
        // Exact byte size of file
        public long Size = 0;
        // Size in 2048 byte sectors
        public long SectorSize = 0;
        // Size of the file when decompressed
        public long DecompressedSize = 0;
        // Index of file, used for sorting read requests
        public long LBA = 0;
        // The original TOC file this entry came from
        public string OwnerTOC;
        // Full path to file
        public string Path = "";
        // The folder name
        public string SubDirectory = "";
        // The file name
        public string Name = "";
        // Set to true if the file is included more than once
        public bool Duplicate = false;
        // Set to true if the file is a XEX file
        public bool IsXEX = false;
        // Set to true if the file is a TOC file
        public bool IsTOC = false;
        // First numeric value in the string (used for sorting)
        public int Numeric = 0;
        // The hex string of the 128 bit checksum
        public string CRCString = "";
        // The name of the group this entry currently resides in
        public TOCGroup Group = null;

        public TOCInfo( string Line )
        {
            string[] Elements = Line.Split( ' ' );
            if( Elements.Length == 5 )
            {
                try
                {
                    Type = UnrealDVDLayout.ObjectType.File;
                    Size = Int64.Parse( Elements[0] );
                    SectorSize = ( Size + ( UnrealDVDLayout.BytesPerSector - 1 ) ) / UnrealDVDLayout.BytesPerSector;
                    DecompressedSize = Int64.Parse( Elements[1] );
                    Path = Elements[3].TrimStart( '.' );
                    CRCString = Elements[4];
                }
                catch
                {
                    Path = "";
                }
            }
        }

        public TOCInfo( FileInfo Info, string RelativeName )
        {
            try
            {
                Type = UnrealDVDLayout.ObjectType.File;
                Size = Info.Length;
                SectorSize = ( Size + ( UnrealDVDLayout.BytesPerSector - 1 ) ) / UnrealDVDLayout.BytesPerSector;
                Path = RelativeName;
            }
            catch
            {
                Path = "";
            }
        }

        public TOCInfo( UnrealDVDLayout.ObjectType InType, string Folder )
        {
            Type = InType;
            SectorSize = 1;
            Path = Folder;
        }

        public void DeriveData( string TOCFileName )
        {
            // Set the owning TOC
            OwnerTOC = TOCFileName;

            // Create the split name
            int SlashIndex = Path.LastIndexOf( '\\' );
            SubDirectory = Path.Substring( 0, SlashIndex );
            Name = Path.Substring( SlashIndex + 1 );

            // Check for XEX file
            IsXEX = Name.ToLower().Contains( ".xex" );

            // Check for a TOC
            if( OwnerTOC != null )
            {
                if( OwnerTOC.Contains( Name ) )
                {
                    IsTOC = true;
                    Size = 0;
                }
            }

            // Set up the TOC files
            if( OwnerTOC != null && Size == 0 )
            {
                FileInfo Info = new FileInfo( OwnerTOC );
                if( Info.Exists )
                {
                    Size = Info.Length;
                    SectorSize = ( Size + ( UnrealDVDLayout.BytesPerSector - 1 ) ) / UnrealDVDLayout.BytesPerSector;
                }
            }

            if( SubDirectory.Length == 0 )
            {
                SubDirectory = "\\";
            }

            // Find the numeric value
            string Number = "";
            bool ParsingNumber = false;
            foreach( char Letter in Name )
            {
                if( Letter >= '0' && Letter <= '9' )
                {
                    ParsingNumber = true;
                    Number += Letter;
                }
                else if( ParsingNumber )
                {
                    break;
                }
            }

            if( Number.Length > 0 )
            {
                Numeric = Int32.Parse( Number );
            }
        }
    }

    public enum GroupSortType
    {
        Alphabetical,
        ReverseAlphabetical,
        FirstNumeric
    };

    public class TOCGroup
    {
        [XmlAttribute]
        public string GroupName;

        [XmlAttribute]
        public List<string> Files = new List<string>();

        [XmlAttribute]
        public string RegExp = "";

        [XmlAttribute]
        public GroupSortType SortType = GroupSortType.Alphabetical;

        [XmlIgnore]
        public int Layer = -1;

        [XmlIgnore]
        public long Size = 0;

        [XmlIgnore]
        public long SectorSize = 0;

        [XmlIgnore]
        public bool GroupSelected = false;

        [XmlIgnore]
        public bool LayerSelected = false;

        public TOCGroup()
        {
        }
    }

    public class TOCGroups
    {
        // All groups that exist
        [XmlElement]
        public List<TOCGroup> TOCGroupEntries = new List<TOCGroup>();

        // Groups assigned to layer 0
        [XmlElement]
        public List<string> TOCGroupLayer0 = new List<string>();

        // Groups assigned to layer 1
        [XmlElement]
        public List<string> TOCGroupLayer1 = new List<string>();

        public TOCGroups()
        {
        }
    }

    public class TOC
    {
        private UnrealDVDLayout Main = null;
        private float TotalSizeGB = 0.0f;

        public string SourceFolder = "";
        public List<TOCInfo> TOCFileEntries = new List<TOCInfo>();
        public TOCGroups Groups = new TOCGroups();

        public TOC( UnrealDVDLayout InMain, string TOCFolder )
        {
            Main = InMain;
            SourceFolder = TOCFolder;
        }

        public int CompareTOCInfo( TOCInfo X, TOCInfo Y )
        {
            if( X.Group != null && X.Group == Y.Group )
            {
                switch( X.Group.SortType )
                {
                    case GroupSortType.FirstNumeric:
                        if( X.Numeric != Y.Numeric )
                        {
                            return ( X.Numeric - Y.Numeric );
                        }

                        if( Y.Name.Length != X.Name.Length )
                        {
                            return ( Y.Name.Length - X.Name.Length );
                        }

                        return ( String.Compare( X.Path, Y.Path, true ) );

                    case GroupSortType.ReverseAlphabetical:
                        return ( String.Compare( Y.Path, X.Path, true ) );
                }

                // ... else fall through to the default alphabetical
            }
            return ( String.Compare( X.Path, Y.Path, true ) );
        }

        public string GetSummary()
        {
            return ( TOCFileEntries.Count.ToString() + " files totalling " + TotalSizeGB.ToString() + " GB" );
        }

        // Find all unique folders and add an entry for them
        public void CreateFolders()
        {
            List<string> Folders = new List<string>();

            // Get the unique folder names
            foreach( TOCInfo TOCEntry in TOCFileEntries )
            {
                string[] FolderNames = TOCEntry.SubDirectory.Split( '\\' );
                string FolderName = "";

                for( int Index = 1; Index < FolderNames.Length; Index++ )
                {
                    FolderName += "\\" + FolderNames[Index];
                    if( !Folders.Contains( FolderName ) )
                    {
                        Folders.Add( FolderName );
                    }
                }
            }

            // Put them in a reasonable order
            Folders.Sort();

            // Create the relevant TOC entries
            foreach( string Folder in Folders )
            {
                if( Folder.Length > 1 )
                {
                    TOCInfo TOCEntry = new TOCInfo( UnrealDVDLayout.ObjectType.Directory, Folder );
                    TOCEntry.DeriveData( null );
                    TOCFileEntries.Add( TOCEntry );
                }
            }
        }

        public bool GetSystemUpdateFile()
        {
            string SystemUpdatePath = SourceFolder + "\\$SystemUpdate";
            Directory.CreateDirectory( SystemUpdatePath );

            // Get the path to the update file for this XDK
            string Path = Environment.GetEnvironmentVariable( "XEDK" );
            Path += "\\redist\\xbox\\$SystemUpdate";

            DirectoryInfo DirInfo = new DirectoryInfo( Path );
            foreach( FileInfo Info in DirInfo.GetFiles() )
            {
                Info.CopyTo( SystemUpdatePath + "\\" + Info.Name, true );

                FileInfo SystemUpdateFileInfo = new FileInfo( SystemUpdatePath + "\\" + Info.Name );

                TOCInfo TOCFileEntry = new TOCInfo( SystemUpdateFileInfo, "\\$SystemUpdate\\" + Info.Name );
                TOCFileEntry.DeriveData( null );

                if( TOCFileEntry.Path.Length > 0 )
                {
                    TOCFileEntries.Add( TOCFileEntry );
                    return ( true );
                }
            }

            return ( false );
        }

        public bool GetXex( string GameName )
        {
            string XexName = SourceFolder + "\\" + GameName + "-XeReleaseLTCG.xex";
            FileInfo Info = new FileInfo( XexName );
            if( Info.Exists )
            {
                FileInfo XexInfo = new FileInfo( SourceFolder + "\\default.xex" );
                if( XexInfo.Exists )
                {
                    XexInfo.IsReadOnly = false;
                }

                Info.CopyTo( SourceFolder + "\\default.xex", true );

                XexInfo = new FileInfo( SourceFolder + "\\default.xex" );

                TOCInfo TOCFileEntry = new TOCInfo( XexInfo, "\\default.xex" );
                TOCFileEntry.DeriveData( null );

                if( TOCFileEntry.Path.Length > 0 )
                {
                    TOCFileEntries.Add( TOCFileEntry );
                    return ( true );
                }
            }

            return ( false );
        }

        public bool GetXdb( string GameName )
        {
            string XdbName = SourceFolder + "\\Binaries\\Xenon\\Lib\\XeReleaseLTCG\\" + GameName + "-XeReleaseLTCG.xdb";
            FileInfo Info = new FileInfo( XdbName );
            if( Info.Exists )
            {
                string NewXdbName = SourceFolder + "\\" + GameName + "-XeReleaseLTCG.xdb";

                FileInfo XdbInfo = new FileInfo( NewXdbName );
                if( XdbInfo.Exists )
                {
                    XdbInfo.IsReadOnly = false;
                }

                Info.CopyTo( NewXdbName, true );

                XdbInfo = new FileInfo( NewXdbName );

                TOCInfo TOCFileEntry = new TOCInfo( XdbInfo, "\\" + GameName + "-XeReleaseLTCG.xdb" );
                TOCFileEntry.DeriveData( null );

                if( TOCFileEntry.Path.Length > 0 )
                {
                    TOCFileEntries.Add( TOCFileEntry );
                    return ( true );
                }
            }

            return ( false );
        }

        public bool Read( string TOCFileName, string RelativeName )
        {
            // Read in and parse the TOC
            FileInfo TOCFile = new FileInfo( TOCFileName );
            if( !TOCFile.Exists )
            {
                Main.Error( "TOC file does not exist!" );
                return ( false );
            }

            StreamReader TOCFileStream = new StreamReader( TOCFileName );

            while( !TOCFileStream.EndOfStream )
            {
                string Line = TOCFileStream.ReadLine();

                TOCInfo TOCFileEntry = new TOCInfo( Line );
                TOCFileEntry.DeriveData( TOCFileName );

                if( TOCFileEntry.Path.Length > 0 )
                {
                    // Explicitly add the TOC as the last entry
                    if( !TOCFileEntry.IsTOC )
                    {
                        TOCFileEntries.Add( TOCFileEntry );
                    }
                }
                else
                {
                    Main.Warning( "Invalid TOC entry '" + Line + "' in '" + TOCFileName );
                }
            }

            TOCFileStream.Close();

            // Add in the TOC
            TOCInfo TOCEntry = new TOCInfo( TOCFile, RelativeName );
            TOCEntry.DeriveData( TOCFileName );
            TOCFileEntries.Add( TOCEntry );

            return ( true );
        }

        public void CheckDuplicates()
        {
            // All groups are null, so this is a simple alpha sort
            TOCFileEntries.Sort( CompareTOCInfo );

            TOCInfo LastTOCEntry = null;
            foreach( TOCInfo TOCEntry in TOCFileEntries )
            {
                if( LastTOCEntry != null )
                {
                    if( LastTOCEntry.Path == TOCEntry.Path )
                    {
                        TOCEntry.Duplicate = true;
                        Main.Warning( "Duplicate TOC entry '" + TOCEntry.Path + "'" );
                    }
                }

                LastTOCEntry = TOCEntry;
            }
        }

        public bool FinishSetup()
        {
            // Sort for a basic layout
            TOCFileEntries.Sort( CompareTOCInfo );

            // Calculate sizes
            long TotalSize = 0;
            foreach( TOCInfo TOCFileEntry in TOCFileEntries )
            {
                TotalSize += TOCFileEntry.Size;
            }
            TotalSizeGB = TotalSize / ( 1024.0f * 1024.0f * 1024.0f );

            foreach( string GroupName in Groups.TOCGroupLayer0 )
            {
                TOCGroup Group = GetGroup( GroupName );
                Group.Layer = 0;
            }

            foreach( string GroupName in Groups.TOCGroupLayer1 )
            {
                TOCGroup Group = GetGroup( GroupName );
                Group.Layer = 1;
            }

            Main.Log( UnrealDVDLayout.VerbosityLevel.Informative, "TOC contained " + GetSummary(), Color.Blue );
            return ( true );
        }

        public List<string> GetGroupNames()
        {
            List<string> GroupNames = new List<string>();

            foreach( TOCGroup Group in Groups.TOCGroupEntries )
            {
                GroupNames.Add( Group.GroupName );
            }

            return ( GroupNames );
        }

        public TOCGroup GetGroup( string GroupName )
        {
            foreach( TOCGroup Group in Groups.TOCGroupEntries )
            {
                if( Group.GroupName == GroupName )
                {
                    return ( Group );
                }
            }

            TOCGroup NewGroup = new TOCGroup();
            NewGroup.GroupName = GroupName;
            Groups.TOCGroupEntries.Add( NewGroup );

            Main.Log( UnrealDVDLayout.VerbosityLevel.Informative, " ... adding new group: '" + GroupName + "'", Color.Blue );

            return ( NewGroup );
        }

        public List<TOCInfo> GetEntriesInGroup( TOCGroup Group )
        {
            List<TOCInfo> Entries = new List<TOCInfo>();
            foreach( TOCInfo Entry in TOCFileEntries )
            {
                if( Entry.Group == Group )
                {
                    Entries.Add( Entry );
                }
            }

            return ( Entries );
        }

        public void RemoveGroup( string GroupName )
        {
            if( GroupName != null )
            {
                foreach( TOCGroup Group in Groups.TOCGroupEntries )
                {
                    if( Group.GroupName == GroupName )
                    {
                        Main.Log( UnrealDVDLayout.VerbosityLevel.Informative, " ... removing group: '" + GroupName + "'", Color.Blue );
                        Groups.TOCGroupEntries.Remove( Group );
                        break;
                    }
                }
            }
        }

        public void AddFileToGroup( TOCGroup Group, string FileName )
        {
            if( !Group.Files.Contains( FileName ) )
            {
                Group.Files.Add( FileName );
            }
        }

        public void AddGroupToLayer( TOCGroup Group, int Layer )
        {
            Group.Layer = Layer;
            switch( Layer )
            {
                case 0:
                    if( !Groups.TOCGroupLayer0.Contains( Group.GroupName ) )
                    {
                        Groups.TOCGroupLayer0.Add( Group.GroupName );
                    }
                    break;

                case 1:
                    if( !Groups.TOCGroupLayer1.Contains( Group.GroupName ) )
                    {
                        Groups.TOCGroupLayer1.Add( Group.GroupName );
                    }
                    break;

                case -1:
                    Groups.TOCGroupLayer0.Remove( Group.GroupName );
                    Groups.TOCGroupLayer1.Remove( Group.GroupName );
                    break;
            }
        }

        public void MoveGroup<T>( List<T> Groups, T Group, int Direction )
        {
            if( Direction == 1 )
            {
                int Index = Groups.IndexOf( Group );
                if( Index > 0 )
                {
                    Groups.Remove( Group );
                    Groups.Insert( Index - 1, Group );
                }
            }
            else if( Direction == -1 )
            {
                int Index = Groups.IndexOf( Group );
                if( Index < Groups.Count - 1 )
                {
                    Groups.Remove( Group );
                    Groups.Insert( Index + 1, Group );
                }
            }
        }

        public void MoveGroupInGroups( TOCGroup Group, int Direction )
        {
            MoveGroup<TOCGroup>( Groups.TOCGroupEntries, Group, Direction );
            Group.GroupSelected = true;
        }

        public void MoveGroupInLayer( TOCGroup Group, int Layer, int Direction )
        {
            if( Layer == 0 )
            {
                MoveGroup<string>( Groups.TOCGroupLayer0, Group.GroupName, Direction );
            }
            else
            {
                MoveGroup<string>( Groups.TOCGroupLayer1, Group.GroupName, Direction );
            }

            Group.LayerSelected = true;
        }

        public bool CollateFiles( string Expression, string GroupName )
        {
            // Sanity check the reg exp
            try
            {
                Regex RX = new Regex( Expression, RegexOptions.IgnoreCase | RegexOptions.Compiled );
            }
            catch
            {
                return ( false );
            }

            TOCGroup Group = GetGroup( GroupName );
            Group.RegExp = Expression;

            return ( true );
        }

        public void ApplySort( List<TOCInfo> Entries )
        {
            Entries.Sort( CompareTOCInfo );
        }

        public void CalcDirectorySizes()
        {
            Dictionary<string, TOCInfo> Dirs = new Dictionary<string, TOCInfo>();

            // Find all directories
            foreach( TOCInfo TOCEntry in TOCFileEntries )
            {
                TOCGroup Group = TOCEntry.Group;
                if( Group != null && Group.Layer != -1 )
                {
                    if( TOCEntry.Type == UnrealDVDLayout.ObjectType.Directory )
                    {
                        TOCEntry.Size = 0;
                        TOCEntry.SectorSize = 0;
                        Dirs.Add( TOCEntry.Path, TOCEntry );
                    }
                }
            }

            Main.Log( UnrealDVDLayout.VerbosityLevel.Informative, "Found " + Dirs.Count + " directories", Color.Blue );
   
            // Work out the size required for each directory
            foreach( TOCInfo TOCEntry in TOCFileEntries )
            {
                TOCGroup Group = TOCEntry.Group;
                if( Group != null && Group.Layer != -1 )
                {
                    if( Dirs.ContainsKey( TOCEntry.SubDirectory ) )
                    {
                        // 14 bytes per entry and DWORD aligned
                        int DirEntryLength = ( 14 + TOCEntry.Name.Length + 3 ) & -4;

                        // Cannot cross sector boundary
                        long CurrentSize = Dirs[TOCEntry.SubDirectory].Size;
                        long CurrentOffset = ( CurrentSize & ( UnrealDVDLayout.BytesPerSector - 1 ) );
                        if( CurrentOffset + DirEntryLength > UnrealDVDLayout.BytesPerSector )
                        {
                            CurrentSize += UnrealDVDLayout.BytesPerSector - CurrentOffset;
                        }
                        CurrentSize += DirEntryLength;

                        Dirs[TOCEntry.SubDirectory].Size = CurrentSize;
                    }
                }
            }

            // Work out the sector sizes
            foreach( string Dir in Dirs.Keys )
            {
                Dirs[Dir].SectorSize = ( Dirs[Dir].Size + ( UnrealDVDLayout.BytesPerSector - 1 ) ) / UnrealDVDLayout.BytesPerSector;
                Main.Log( UnrealDVDLayout.VerbosityLevel.Informative, "Dir " + Dir + " = " + Dirs[Dir].Size.ToString(), Color.Blue );
            }
        }

        public void UpdateTOCFromGroups()
        {
            // Clear out the in group settings
            foreach( TOCInfo TOCEntry in TOCFileEntries )
            {
                TOCEntry.Group = null;
            }

            // Zero out all the group sizes
            foreach( TOCGroup Group in Groups.TOCGroupEntries )
            {
                Group.Size = 0;
                Group.SectorSize = 0;
            }

            // Filter out anything that passes the regular expression
            foreach( TOCGroup Group in Groups.TOCGroupEntries )
            {
                Regex RX = new Regex( Group.RegExp, RegexOptions.IgnoreCase | RegexOptions.Compiled );

                foreach( TOCInfo TOCEntry in TOCFileEntries )
                {
                    if( TOCEntry.Group == null && !TOCEntry.Duplicate )
                    {
                        Match RegExpMatch = RX.Match( TOCEntry.Path );
                        if( RegExpMatch.Success )
                        {
                            TOCEntry.Group = Group;
                            Group.Size += TOCEntry.Size;
                            Group.SectorSize += TOCEntry.SectorSize;
                        }
                        else
                        {
                            // Try each special case if the regexp doesn't pass
                            foreach( string File in Group.Files )
                            {
                                if( File == TOCEntry.Path )
                                {
                                    TOCEntry.Group = Group;
                                    Group.Size += TOCEntry.Size;
                                    Group.SectorSize += TOCEntry.SectorSize;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
