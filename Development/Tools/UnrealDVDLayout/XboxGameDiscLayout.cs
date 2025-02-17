using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Xml;
using System.Xml.Serialization;

namespace UnrealDVDLayout
{
    public class Version
    {
        [XmlAttribute]
        public string Type;
        [XmlAttribute]
        public int Major;
        [XmlAttribute]
        public int Minor;

        public Version()
        {
        }

        public Version( string InType, int InMajor, int InMinor )
        {
            Type = InType;
            Major = InMajor;
            Minor = InMinor;
        }
    }

    public class ToolVersion
    {
        [XmlAttribute]
        public string Type;
        [XmlAttribute]
        public int Major;
        [XmlAttribute]
        public int Minor;
        [XmlAttribute]
        public int Build;
        [XmlAttribute]
        public int QFE;

        public ToolVersion()
        {
            Type = "app";
            Major = 1;
            Minor = 16;
            Build = 7645;
            QFE = 1;
        }
    }

    public class Tool
    {
        [XmlAttribute]
        public string ID;

        public ToolVersion Version;

        public Tool()
        {
            ID = "GDEngine";
            Version = new ToolVersion();
        }
    }

    public class LastSave
    {
        [XmlAttribute]
        public long DateTime;

        public Tool Tool;

        public LastSave()
        {
            DateTime = 0;
            Tool = new Tool();
        }
    }

    public class Header
    {
        [XmlElement]
        public Version[] Version;

        public LastSave LastSave;

        public Header()
        {
            Version = new Version[] { new Version( "schema", 1, 1 ), new Version( "compat", 1, 0 ) };
            LastSave = new LastSave();
        }
    }

    public class Source
    {
        [XmlAttribute]
        public long Id;
        [XmlAttribute]
        public string PCSrcDirectory;
        [XmlAttribute]
        public string SearchPattern;
        [XmlAttribute]
        public string XboxMediaDirectory;
        [XmlAttribute]
        public string Recurse;

        public Source()
        {
        }

        public Source( string SourceFolder )
        {
            Id = 0;
            PCSrcDirectory = SourceFolder;
            SearchPattern = "*.*";
            XboxMediaDirectory = "\\";
            Recurse = "Yes";
        }
    }

    public class Object
    {
        [XmlAttribute]
        public UnrealDVDLayout.ObjectType Type;
        [XmlAttribute]
        public long Root;
        [XmlAttribute]
        public string SubDirectory;
        [XmlAttribute]
        public string Name;
        [XmlAttribute]
        public long Size;
        [XmlAttribute]
        public long LBA;
        [XmlAttribute]
        public long Blocks;

        public Object()
        {
        }

        // For all user objects placed on the DVD
        public Object( TOCInfo TOCEntry )
        {
            Type = TOCEntry.Type;
            Root = 0;
            LBA = 0;

            Size = TOCEntry.Size;
            Blocks = TOCEntry.SectorSize;
            SubDirectory = TOCEntry.SubDirectory;
            Name = TOCEntry.Name;

            if( Type == UnrealDVDLayout.ObjectType.Directory )
            {
                if( ( Blocks > 1 ) && ( ( Blocks % 2 ) != 0 ) )
                {
                    Blocks++;
                }

                Size = Blocks * UnrealDVDLayout.BytesPerSector;
            }

            // The magic +1
            if( TOCEntry.IsXEX )
            {
                Blocks++;
            }
        }

        // For system objects (volume and reserved)
        public Object( UnrealDVDLayout.ObjectType InType, string InSubdirectory, string InName, long InLBA, long InSize )
        {
            Type = InType;
            SubDirectory = InSubdirectory;
            Name = InName;
            Root = 0;
            Size = InSize;
            LBA = InLBA;
            Blocks = ( Size + ( UnrealDVDLayout.BytesPerSector - 1 ) ) / UnrealDVDLayout.BytesPerSector;
        }

        public Object( string InName, long InBlocks )
        {
            Type = UnrealDVDLayout.ObjectType.File;
            SubDirectory = "\\";
            Name = InName;
            Root = 0;
            Blocks = InBlocks;
            Size = InBlocks * UnrealDVDLayout.BytesPerSector;
            LBA = 0;
        }
    }

    public class Group
    {
        [XmlElement( "Object" )]
        public List<Object> Objects = new List<Object>();

        public long AddObject( Object Entry )
        {
            Objects.Add( Entry );
            return ( Entry.Blocks );
        }
    }

    public class File
    {
        [XmlAttribute]
        public string Location;
        [XmlAttribute]
        public long Sectors;

        [XmlElement( "Object" )]
        public List<Object> Objects = new List<Object>();

        [XmlElement( "Group" )]
        public List<Group> Groups = new List<Group>();

        public File()
        {
        }

        public File( string InLocation, long InSectors )
        {
            Location = InLocation;
            Sectors = InSectors;
        }

        public long AddObject( Object Entry )
        {
            Objects.Add( Entry );
            return ( Entry.Blocks ); 
        }

        public void AddGroup( Group Group )
        {
            Groups.Add( Group );
        }
    }

    public class Disc
    {
        // Reserved on layer0
        // <Group>
        // <Object Type="Directory" Root="0" SubDirectory="&lt;ROOT&gt;" Name="" Size="2048" LBA="1771287" Blocks="1" />
        // <Object Type="Directory" Root="0" SubDirectory="\" Name="$SystemUpdate" Size="2048" LBA="1771288" Blocks="1" />
        // <Object Type="File" Root="0" SubDirectory="\$SystemUpdate" Name="su2008d200_00000000" Size="8257536" LBA="1771289" Blocks="4032" />
        // <Object Type="File" Root="0" SubDirectory="\" Name="default.xex" Size="17641472" LBA="1775321" Blocks="8615" />
        // </Group>
        // <Object Type="Volume" Root="0" Size="0" LBA="32" Blocks="2" />
        // <Object Type="Reserved" Root="0" Size="0" LBA="48" Blocks="4096" />
        [XmlAttribute]
        public int Id;
        [XmlAttribute]
        public int Type;

        [XmlElement]
        public File[] Files;

        // Current incremental LBA
        private long CurrentLayer0StartLBA = 0;
        // Current incremental LBA
        private long CurrentLayer1StartLBA = UnrealDVDLayout.SectorsPerLayer;

        public Disc()
        {
        }

        public Disc( XboxGameDiscLayout Owner )
        {
            Object Entry;

            Id = 0;
            Type = 0;

            Files = new File[]
                {
                    new File( "layer0", UnrealDVDLayout.SectorsPerLayer ),
                    new File( "layer1", UnrealDVDLayout.SectorsPerLayer ),
                    new File( "scratch", 9999999 ),
                };

            Entry = new Object( UnrealDVDLayout.ObjectType.Volume, "", "", 32, 2 * UnrealDVDLayout.BytesPerSector );
            Files[0].AddObject( Entry );
            Entry = new Object( UnrealDVDLayout.ObjectType.Reserved, "", "", 48, 4096 * UnrealDVDLayout.BytesPerSector );
            Files[0].AddObject( Entry );

            CurrentLayer0StartLBA = 48 + 4096;
            CurrentLayer1StartLBA = UnrealDVDLayout.SectorsPerLayer;

            Entry = new Object( UnrealDVDLayout.ObjectType.Directory, "<ROOT>", "", CurrentLayer0StartLBA, UnrealDVDLayout.BytesPerSector );
            CurrentLayer0StartLBA += AddObject( Entry, 0 );
        }

        public long AddObject( Object Entry, int Layer )
        {
            switch( Layer )
            {
                case -1:
                    // Add unplaced files
                    Files[2].AddObject( Entry );
                    break;

                case 0:
                    Entry.LBA = CurrentLayer0StartLBA;
                    Files[0].AddObject( Entry );
                    CurrentLayer0StartLBA += Entry.Blocks;
                    break;

                case 1:
                    Entry.LBA = CurrentLayer1StartLBA;
                    Files[1].AddObject( Entry );
                    CurrentLayer1StartLBA += Entry.Blocks;
                    break;
            }

            if( CurrentLayer0StartLBA > Files[0].Sectors )
            {
                return ( -1 );
            }
            else if( CurrentLayer1StartLBA > Files[1].Sectors + UnrealDVDLayout.SectorsPerLayer )
            {
                return ( -1 );
            }

            return ( Entry.Blocks );
        }

        public long AddPadObject( string ItemName, int Layer, int Alignment )
        {
            long Offset = 0;
            long Blocks = 0;
           
            if( Layer == 0 )
            {
                Offset = CurrentLayer0StartLBA - ( CurrentLayer0StartLBA & -Alignment );
                Blocks = ( Alignment - Offset ) & ( Alignment - 1 );
            }
            else if( Layer == 1 )
            {
                Offset = CurrentLayer1StartLBA - ( CurrentLayer1StartLBA & -Alignment );
                Blocks = ( Alignment - Offset ) & ( Alignment - 1 );
            }

            if( Blocks != 0 )
            {
                string PadName = ( ItemName + ".pad" ).Replace( " ", "_" );

                Object Pad = new Object( PadName, Blocks );
                AddObject( Pad, Layer );

                // Create a physical pad object
                FileInfo PadFileInfo = new FileInfo( PadName );
                FileStream Stream = PadFileInfo.OpenWrite();
                Stream.SetLength( Blocks * UnrealDVDLayout.BytesPerSector );
                Stream.Close();
            }

            return ( Blocks );
        }

        public bool AddGroup( int Layer, Group Group )
        {
            Files[Layer].AddGroup( Group );
            return ( true );
        }

        public void ShuntToEnd()
        {
            long SectorShunt = UnrealDVDLayout.SectorsPerLayer - CurrentLayer0StartLBA;

            if( SectorShunt <= 0 )
            {
                return;
            }

            foreach( Object Entry in Files[0].Objects )
            {
                if( Entry.Type == UnrealDVDLayout.ObjectType.File || Entry.Type == UnrealDVDLayout.ObjectType.Directory )
                {
                    Entry.LBA += SectorShunt;
                }
            }
        }

        public string GetSummary()
        {
            long Layer0Free = UnrealDVDLayout.SectorsPerLayer - CurrentLayer0StartLBA;
            long Layer1Free = ( UnrealDVDLayout.SectorsPerLayer * 2 ) - CurrentLayer1StartLBA; 
            float MBFree = ( Layer0Free + Layer1Free ) / 512.0f;

            float GBUsed = ( CurrentLayer0StartLBA + ( CurrentLayer1StartLBA - UnrealDVDLayout.SectorsPerLayer ) ) / ( 512.0f * 1024.0f );

            return ( GBUsed.ToString() + " GB used in layout (" + MBFree.ToString() + " MB Free)" );
        }
    }

    public class XboxGameDiscLayout
    {
        public Header Header;
        public Source[] Sources;
        public Disc Disc;

        [XmlIgnore]
        public List<string> Folders = new List<string>();

        public XboxGameDiscLayout()
        {
        }

        public XboxGameDiscLayout( string SourceFolder )
        {
            Header = new Header();
            Sources = new Source[] { new Source( SourceFolder ) };
            Disc = new Disc( this );

            Folders.Add( "\\" );
        }

        // Add an object based on a TOC entry
        public bool AddObject( TOCInfo TOCEntry, int Layer )
        {
            Object Entry = new Object( TOCEntry );
            long Blocks = Disc.AddObject( Entry, Layer );
            TOCEntry.Layer = Layer;
            TOCEntry.LBA = Entry.LBA;
            return ( Blocks >= 0 );
        }

        public bool AddPadObject( string GroupName, int Layer )
        {
            long Blocks = Disc.AddPadObject( GroupName, Layer, 16 );
            return ( Blocks >= 0 );
        }

        public void Finalise()
        {
            // Shunt all layer0 files to (faster) end of layer 
            Disc.ShuntToEnd();
        }

        public string GetSummary()
        {
            return ( Disc.GetSummary() );
        }
    }
}
