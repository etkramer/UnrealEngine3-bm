using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Text;

namespace UnrealLoc
{
    public class FileEntry
    {
        private UnrealLoc Main = null;
        private LanguageInfo Lang = null;

        private string LocalFileName = "";
        public string FileName
        {
            get { return ( LocalFileName ); }
            set { LocalFileName = value; }
        }

        private string LocalExtension = "";
        public string Extension
        {
            get { return ( LocalExtension ); }
            set { LocalExtension = value; }
        }
        
        private string LocalRelativeName = "";
        public string RelativeName
        {
            get { return ( LocalRelativeName ); }
            set { LocalRelativeName = value; }
        }

        private bool LocalHasNewLocEntries = false;
        public bool HasNewLocEntries
        {
            get { return ( LocalHasNewLocEntries ); }
            set { LocalHasNewLocEntries = value; }
        }

        private ObjectEntryHandler FileObjectEntryHandler;

        public List<ObjectEntry> GetObjectEntries()
        {
            return ( FileObjectEntryHandler.GetObjectEntries() );
        }

        public int GetObjectCount()
        {
            return ( FileObjectEntryHandler.GetObjectCount() );
        }

        public void GenerateLocObjects( FileEntry DefaultFE )
        {
            FileObjectEntryHandler.GenerateLocObjects( DefaultFE );
        }

        public void RemoveOrphans()
        {
            FileObjectEntryHandler.RemoveOrphans();
        }

        public bool CreateDirectory( string FolderName )
        {
            DirectoryInfo DirInfo = new DirectoryInfo( FolderName );
            DirInfo.Create();

            return( true );
        }

        public bool WriteLocFiles()
        {
            Main.Log( UnrealLoc.VerbosityLevel.Informative, " ... creating loc file: " + RelativeName, Color.Black );

            FileInfo LocFileInfo = new FileInfo( RelativeName );
            CreateDirectory( LocFileInfo.DirectoryName );

            if( Main.AddToSourceControl( Lang, RelativeName ) )
            {
                Lang.FilesCreated++;
            }

            if( LocFileInfo.Exists && LocFileInfo.IsReadOnly )
            {
                Main.Checkout( Lang, RelativeName );
                LocFileInfo = new FileInfo( RelativeName );
            }

            if( !LocFileInfo.IsReadOnly || !LocFileInfo.Exists )
            {
                StreamWriter File = new StreamWriter( RelativeName, false, System.Text.Encoding.Unicode );
                FileObjectEntryHandler.WriteLocFiles( File );
                File.Close();
                return ( true );
            }

            return ( false );
        }

        public bool WriteDiffLocFiles( string Folder )
        {
            string LocFileName = Folder + "\\" + Lang.LangID + "\\" + FileName + "." + Lang.LangID;
            Main.Log( UnrealLoc.VerbosityLevel.Informative, " ... creating loc diff file: " + LocFileName, Color.Black );
            FileInfo LocFileInfo = new FileInfo( LocFileName );
            CreateDirectory( LocFileInfo.DirectoryName );

            if( LocFileInfo.Exists )
            {
                LocFileInfo.IsReadOnly = false;
                LocFileInfo.Delete();
            }

            StreamWriter File = new StreamWriter( LocFileName, false, System.Text.Encoding.Unicode );
            FileObjectEntryHandler.WriteDiffLocFiles( File );
            File.Close();

            return ( false );
        }

        public bool ImportText( string FileName )
        {
            return( FileObjectEntryHandler.ImportText( FileName ) );
        }

        public FileEntry( UnrealLoc InMain, LanguageInfo InLang, string InName, string InExtension, string InRelativeName )
        {
            Main = InMain;
            Lang = InLang;
            FileName = InName;
            Extension = InExtension;
            RelativeName = InRelativeName;
            FileObjectEntryHandler = new ObjectEntryHandler( Main, Lang, this );
            FileObjectEntryHandler.FindObjects();
        }

        public FileEntry( UnrealLoc InMain, LanguageInfo InLang, FileEntry DefaultFE )
        {
            Main = InMain;
            Lang = InLang;
            FileName = DefaultFE.FileName;
            Extension = "." + Lang.LangID;

            RelativeName = DefaultFE.RelativeName;
            RelativeName = RelativeName.Replace( ".INT", Extension.ToUpper() );
            RelativeName = RelativeName.Replace( "\\INT\\", "\\" + Lang.LangID + "\\" );

            FileObjectEntryHandler = new ObjectEntryHandler( Main, Lang, this );
        }
    }

    public class FileEntryHandler
    {
        private UnrealLoc Main = null;
        private LanguageInfo Lang = null;
        public List<FileEntry> FileEntries;

        public FileEntryHandler( UnrealLoc InMain, LanguageInfo InLang )
        {
            Main = InMain;
            Lang = InLang;
            FileEntries = new List<FileEntry>();
        }

        public FileEntry CreateFile( FileEntry DefaultFileEntry )
        {
            FileEntry FileElement = new FileEntry( Main, Lang, DefaultFileEntry );
            FileEntries.Add( FileElement );

            Main.Log( UnrealLoc.VerbosityLevel.Informative, "Created file '" + FileElement.RelativeName + "'", Color.Blue );
            return ( FileElement );
        }

        public bool AddFile( string LangFolder, FileInfo File )
        {
            // Key off root name
            string FileName = File.Name.Substring( 0, File.Name.Length - File.Extension.Length ).ToLower();
            foreach( FileEntry FE in FileEntries )
            {
                if( FE.FileName == FileName )
                {
                    return ( false );
                }
            }

            // Remember extension, root name and full name
            string Extension = File.Extension.ToUpper();
            string RelativeName = LangFolder + "\\" + FileName + Extension;

            FileEntry FileElement = new FileEntry( Main, Lang, FileName, Extension, RelativeName );
            FileEntries.Add( FileElement );
            return ( true );
        }
#if false
        public bool AddNewFile( FileInfo Info )
        {
            bool IsNewFile = true;

            // Key off root name
            string FileName = Info.Name.Substring( 0, Info.Name.Length - Info.Extension.Length ).ToLower();

            // Remember extension, root name and full name
            string Extension = Info.Extension.ToUpper();
            string RelativeName = GetRelativePath() + "\\" + FileName + Extension;

            foreach( FileEntry FE in FileEntries )
            {
                if( FE.FileName == FileName )
                {
                    // Remove old instance of file if it exists
                    FileEntries.Remove( FE );
                    // ... and make it writable
                    Main.Checkout( Lang, RelativeName );
                    IsNewFile = false;
                    break;
                }
            }

            // Add the file if it's new
            if( IsNewFile )
            {
                Main.AddToSourceControl( Lang, RelativeName );
            }

            Info.CopyTo( RelativeName, true );
            Info = new FileInfo( RelativeName );
            // We checked it out or added it - make sure it is writable
            Info.IsReadOnly = false;

            FileEntry FileElement = new FileEntry( Main, Lang, FileName, Extension, RelativeName );
            FileEntries.Add( FileElement );

            Lang.LocFilesGenerated = false;
            return ( true );
        }
#endif

        private FileEntry FileExists( string DefaultFileName )
        {
            foreach( FileEntry ExistingFE in FileEntries )
            {
                if( DefaultFileName == ExistingFE.FileName )
                {
                    return ( ExistingFE );
                }
            }

            return ( null );
        }

        public List<FileEntry> GetFileEntries()
        {
            return ( FileEntries );
        }

        public int GetCount()
        {
            return( FileEntries.Count );
        }

        public bool GenerateLocFiles( LanguageInfo DefaultLangInfo )
        {
            List<FileEntry> DefaultFileEntries = DefaultLangInfo.GetFileEntries();
            foreach( FileEntry DefaultFE in DefaultFileEntries )
            {
                FileEntry LocFE = FileExists( DefaultFE.FileName );
                if( LocFE == null )
                {
                    LocFE = CreateFile( DefaultFE );
                }

                LocFE.GenerateLocObjects( DefaultFE );
            }

            return ( true );
        }

        public void RemoveOrphans()
        {
            Main.Log( UnrealLoc.VerbosityLevel.Simple, "Removing orphans for: " + Lang.LangID, Color.Green );
            foreach( FileEntry FE in FileEntries )
            {
                FE.RemoveOrphans();
            }
        }

        public bool WriteLocFiles()
        {
            foreach( FileEntry FE in FileEntries )
            {
                FE.WriteLocFiles();
            }

            return ( true );
        }

        public bool WriteDiffLocFiles( string Folder )
        {
            foreach( FileEntry FE in FileEntries )
            {
                if( FE.HasNewLocEntries )
                {
                    FE.WriteDiffLocFiles( Folder );
                }
            }

            return ( true );
        }

        public string GetRelativePath()
        {
            string LangFolder = "";

            if( Main.GameName == "Engine" )
            {
                LangFolder = Main.GameName + "\\Localization\\" + Lang.LangID;
            }
            else
            {
                LangFolder = Main.GameName + "Game\\Localization\\" + Lang.LangID;
            }

            return ( LangFolder );
        }

        public bool FindLocFiles()
        {
            string LangFolder = GetRelativePath();

            string ExtLangID = "." + Lang.LangID;

            DirectoryInfo DirInfo = new DirectoryInfo( LangFolder );
            if( DirInfo.Exists )
            {
                foreach( FileInfo File in DirInfo.GetFiles() )
                {
                    if( File.Extension.ToUpper() == ExtLangID )
                    {
                        AddFile( LangFolder, File );
                    }
                }

                Main.Log( UnrealLoc.VerbosityLevel.Informative, " ... found " + GetCount().ToString() + " files for " + Lang.LangID, Color.Black );
                return ( true );
            }

            return ( false );
        }

        public bool ImportText( string FileName )
        {
            string RootName = FileName.Substring( FileName.LastIndexOf( '\\' ) + 1 ).ToLower();
            RootName = RootName.Replace( "." + Lang.LangID.ToLower(), "" );
            FileEntry FE = FileExists( RootName );
            if( FE == null )
            {
                Main.Log( UnrealLoc.VerbosityLevel.Simple, " ... INT version of '" + RootName + "' not found.", Color.Red );
                return ( false );
            }

            return( FE.ImportText( FileName ) );
        }
    }
}
