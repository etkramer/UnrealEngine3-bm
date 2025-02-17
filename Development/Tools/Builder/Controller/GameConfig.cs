using System;
using System.Collections.Generic;
using System.Text;

namespace Controller
{
    public class GameConfig
    {
        public GameConfig( string InGame, string InPlatform, string InConfiguration, bool InLocal )
        {
            GameName = InGame + "Game";
            Platform = InPlatform;
            Configuration = InConfiguration;
            IsLocal = InLocal;
        }

        public GameConfig( int Count, GameConfig Game )
        {
            switch( Count )
            {
                case 2:
                    GameName = "Some Games";
                    break;

                case 3:
                    GameName = "All Games";
                    break;
            }
            Platform = Game.Platform;
            Configuration = Game.Configuration;
        }

        public string GetPlatform()
        {
            return ( Platform );
        }

        //                  PC                  Xenon & PS3
        // Debug            Debug               Debug
        // Release          Release             Release
        // Shipping         ReleaseShippingPC   ReleaseLTCG
        // Shipping-Debug   *None*              ReleaseLTCG-DebugConsole

        public string GetOldConfiguration()
        {
            string OldConfiguration = Configuration;
            switch( Configuration.ToLower() )
            {
                case "debug":
                case "release":
                default:
                    break;

                case "shipping":
                    switch( Platform.ToLower() )
                    {
                        case "pc":
                            OldConfiguration = "ReleaseShippingPC";
                            break;

                        case "xenon":
                        case "ps3":
                            OldConfiguration = "ReleaseLTCG";
                            break;
                    }
                    break;

                case "shipping-debug":
                    switch( Platform.ToLower() )
                    {
                        case "xenon":
                        case "ps3":
                            OldConfiguration = "ReleaseLTCG-DebugConsole";
                            break;
                    } 
                    break;
            }

            return ( OldConfiguration );
        }

        // Used for building all patforms in all configurations using MSDev
        public string GetBuildConfiguration()
        {
            string OldConfiguration = GetOldConfiguration();

            if( Platform.ToLower() == "xenon" )
            {
                return ( "Xe" + OldConfiguration );
            }
            else if( Platform.ToLower() == "ps3" )
            {
                return ( "PS3_" + OldConfiguration );
            }

            return ( OldConfiguration );
        }

        // Used for the PS3 make application
        public string GetMakeConfiguration()
        {
            string OldConfiguration = GetOldConfiguration();

            if( Platform.ToLower() == "ps3" )
            {
                if( OldConfiguration.ToLower() == "debug" )
                {
                    return ( "debug" );
                }
                else if( OldConfiguration.ToLower() == "releaseltcg" )
                {
                    return ( "final_release" );
                }
                else if( OldConfiguration.ToLower() == "releaseltcg-debugconsole" )
                {
                    return ( "final_release_debugconsole" );
                }
                else
                {
                    return ( "release" );
                }
            }

            return ( OldConfiguration );
        }

        public string GetConfiguration()
        {
            string OldConfiguration = GetOldConfiguration();

            if( Platform.ToLower() == "xenon" )
            {
                return ( "Xe" + OldConfiguration );
            }
            else if( Platform.ToLower() == "ps3" )
            {
                return ( "PS3" + OldConfiguration );
            }

            return ( OldConfiguration );
        }

        // FIXME
        public string GetProjectName( string Game )
        {
            string ProjectName = Platform + "Launch-" + Game + "Game";
            return ( ProjectName );
        }

        public string[] GetExecutableNames()
        {
            string[] Configs = new string[2] { "", "" };

            if( Platform.ToLower() == "pc" )
            {
                if( Configuration.ToLower() == "release" )
                {
                    Configs[0] = "Binaries/" + GameName + ".exe";
                }
                else if( Configuration.ToLower() == "releaseltcg" )
                {
                    Configs[0] = "Binaries/LTCG-" + GameName + ".exe";
                }
                else if( Configuration.ToLower() == "debug" )
                {
                    Configs[0] = "Binaries/DEBUG-" + GameName + ".exe";
                }
                else if( Configuration.ToLower() == "release-g4wlive" )
                {
                    Configs[0] = "Binaries/" + GameName + "-G4WLive.exe";
                }
                else if( Configuration.ToLower() == "releaseltcg-g4wlive" )
                {
                    Configs[0] = "Binaries/" + GameName + "LTCG-G4WLive.exe";
                }
                else if( Configuration.ToLower() == "releaseshippingpc" || Configuration.ToLower() == "shipping" )
                {
                    if( GameName.ToLower() == "utgame" )
                    {
                        Configs[0] = "Binaries/UT3.exe";
                    }
                    else
                    {
                        Configs[0] = "Binaries/ShippingPC-" + GameName + ".exe";
                    }
                }
            }
            else if( Platform.ToLower() == "xenon" )
            {
                Configs[0] = GameName + "-" + GetConfiguration() + ".exe";
                Configs[1] = GameName + "-" + GetConfiguration() + ".xex";
            }
            else if( Platform.ToLower() == "ps3" )
            {
                Configs[0] = "Binaries/" + GameName.ToUpper() + "-" + GetConfiguration() + ".elf";
                Configs[1] = "Binaries/" + GameName.ToUpper() + "-" + GetConfiguration() + ".xelf";
            }

            return ( Configs );
        }

        public string GetComName()
        {
            string ComName = "";

            if( Configuration.ToLower() == "release-g4wlive" || Configuration.ToLower() == "releaseltcg-g4wlive" )
            {
                ComName = "Binaries/" + GameName + "-G4WLive.exe";
            }
            else
            {
                ComName = "Binaries/" + GameName + ".exe";
            }

            return ( ComName );
        }

        public string[] GetSymbolFileNames()
        {
            string[] Configs = new string[2] { "", "" };

            if( Platform.ToLower() == "pc" )
            {
                if( Configuration.ToLower() == "release" )
                {
                    Configs[0] = "Binaries/Lib/" + GetConfiguration() + "/" + GameName + ".pdb";
                }
                else if( Configuration.ToLower() == "releaseltcg" )
                {
                    Configs[0] = "Binaries/Lib/" + GetConfiguration() + "/LTCG-" + GameName + ".pdb";
                }
                else if( Configuration.ToLower() == "debug" )
                {
                    Configs[0] = "Binaries/Lib/" + GetConfiguration() + "/DEBUG-" + GameName + ".pdb";
                }
                else if( Configuration.ToLower() == "release-g4wlive" )
                {
                    Configs[0] = "Binaries/Lib/" + GetConfiguration() + "/" + GameName + "-G4WLive.pdb";
                }
                else if( Configuration.ToLower() == "releaseltcg-g4wlive" )
                {
                    Configs[0] = "Binaries/Lib/" + GetConfiguration() + "/" + GameName + "LTCG-G4WLive.pdb";
                }
                else if( Configuration.ToLower() == "releaseshippingpc" || Configuration.ToLower() == "shipping" )
                {
                    if( GameName.ToLower() == "utgame" )
                    {
                        Configs[0] = "Binaries/Lib/" + GetConfiguration() + "/UT3.pdb";
                    }
                    else
                    {
                        Configs[0] = "Binaries/Lib/" + GetConfiguration() + "/ShippingPC-" + GameName + ".pdb";
                    }
                }
            }
            else if( Platform.ToLower() == "xenon" )
            {
                Configs[0] = "Binaries/Xenon/lib/" + GetConfiguration() + "/" + GameName + "-" + GetConfiguration() + ".pdb";
                Configs[1] = "Binaries/Xenon/lib/" + GetConfiguration() + "/" + GameName + "-" + GetConfiguration() + ".xdb";
            }
            else if( Platform.ToLower() == "ps3" )
            {
            }

            return ( Configs );
        }

        private string GetShaderName( string ShaderType )
        {
            string ShaderName = GameName + "/Content/" + ShaderType + "ShaderCache";

            if( Platform.ToLower() == "pc" )
            {
                ShaderName += "-PC-D3D-SM3.upk";
            }
            else if( Platform.ToLower() == "xenon" )
            {
                ShaderName += "-Xbox360.upk";
            }
            else if( Platform.ToLower() == "ps3" )
            {
                ShaderName += "-PS3.upk";
            }
            else if( Platform.ToLower() == "pc_sm2" )
            {
                ShaderName += "-PC-D3D-SM2.upk";
            }
            else if( Platform.ToLower() == "pc_sm4" )
            {
                ShaderName += "-PC-D3D-SM4.upk";
            }

            return ( ShaderName );
        }

        public string GetRefShaderName()
        {
            return ( GetShaderName( "Ref" ) );
        }

        public string GetLocalShaderName()
        {
            return ( GetShaderName( "Local" ) );
        }

        public string GetGlobalShaderName()
        {
            string GlobalShaderName = GetShaderName( "Global" );
            return ( GlobalShaderName.Replace( ".upk", ".bin" ) );
        }
        
        public string GetContentTagName()
        {
            return ( GameName + "/Content/RefContentTagsIndex.upk" );
        }

        public string GetConfigFolderName()
        {
            string Folder = GameName + "/config";
            return ( Folder );
        }

        public string GetCookedFolderName()
        {
            string Folder = GameName + "/Cooked" + Platform;
            return ( Folder );
        }

        public string GetPatchFolderName()
        {
            string Folder = GameName + "/Build/" + Platform + "/Patch";
            return ( Folder );
        }
        
        public string GetDialogFileName( string Language, string RootName )
        {
            string DialogName;

            if( Language == "INT" )
            {
                DialogName = GameName + "/Content/Sounds/" + Language + "/" + RootName + ".upk";
            }
            else
            {
                DialogName = GameName + "/Content/Sounds/" + Language + "/" + RootName + "_" + Language + ".upk";
            }
            return ( DialogName );
        }

        public string GetFontFileName( string Language, string RootName )
        {
            string FontName;

            if( Language == "INT" )
            {
                FontName = GameName + "/Content/" + RootName + ".upk";
            }
            else
            {
                FontName = GameName + "/Content/" + RootName + "_" + Language + ".upk";
            }
            return ( FontName );
        }

        public string GetPackageFileName( string Language, string RootName )
        {
            string PackageName;

            if( Language == "INT" )
            {
                PackageName = RootName + ".upk";
            }
            else
            {
                PackageName = RootName + "_" + Language + ".upk";
            }
            return ( PackageName );
        }

        public string GetLayoutFileName( string[] Languages )
        {
            string LayoutName = GameName + "/Build/Layouts/Layout";
            foreach( string Lang in Languages )
            {
                LayoutName += "_" + Lang.ToUpper();
            }
            LayoutName += ".XGD";
            return ( LayoutName );
        }

        public string GetTitle()
        {
            return ( "UnrealEngine3-" + Platform );
        }

        override public string ToString()
        {
            return ( GameName + " (" + Platform + " - " + Configuration + ")" ); 
        }

        public bool Similar( GameConfig Game )
        {
            return( Game.Configuration == Configuration && Game.Platform == Platform );
        }

        public void DeleteCutdownPackages( Main Parent )
        {
            Parent.DeleteDirectory( GameName + "/CutdownPackages", 0 );
        }

        private string GetUBTPlatform()
        {
            switch( Platform.ToLower() )
            {
                case "pc":
                    return ( "Win32" );
                case "xenon":
                    return ( "Xbox360" );
            }
            
            return( Platform );
        }

        private string GetUBTConfig()
        {
            switch( Configuration.ToLower() )
            {
                case "shipping-debug":
                    return ( "ShippingDebugConsole" );
            }

            return ( Configuration );
        }

        public string GetUBTCommandLine()
        {
            string UBTPlatform = GetUBTPlatform();
            string UBTConfig = GetUBTConfig();

            string[] Executables = GetExecutableNames();
            if( UBTPlatform.ToLower() == "xbox360" )
            {
                // Need the xex for UBT
                Executables[0] = Executables[1];
            }

            return ( UBTPlatform + " " + UBTConfig + " " + GameName + " -output ../../" + Executables[0] + " -noxge -verbose" );
        }

        // Name of the game
        private string GameName;
        // Platform
        private string Platform;
        // Build configuration eg. release
        private string Configuration;
        // Whether the build is compiled locally
        public bool IsLocal;
    }
}
