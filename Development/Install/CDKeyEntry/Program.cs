using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Threading;
using System.Windows.Forms;

namespace CDKeyEntry
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            string Region = "INT";

            Environment.CurrentDirectory = Application.StartupPath;

            string[] Args = Environment.GetCommandLineArgs();
            if( Args.Length > 1 )
            {
                if( Args[1].Length == 2 )
                {
                    Thread.CurrentThread.CurrentUICulture = new CultureInfo( Args[1] );
                    Region = ConvertRegion( Args[1] );
                }
            }

            if( Args.Length > 2 )
            {
                WriteToIniFile( Region, Args[2] );
            }
            else
            {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault( false );
                Application.Run( new CDKeyEntry() );
            }
        }

        static string ConvertRegion( string Culture )
        {
            string Region = "INT";
            switch( Culture )
            {
                case "fr":
                    Region = "FRA";
                    break;
                case "de":
                    Region = "DEU";
                    break;
                case "it":
                    Region = "ITA";
                    break;
                case "es":
                    Region = "ESN";
                    break;
                case "ru":
                    Region = "RUS";
                    break;
            }

            return ( Region );
        }

        static void WriteToIniFile( string Region, string IniFile )
        {
            List<string> Lines = new List<string>();
            string Line, EncryptedCDKey;

            // Make sure CD key exists
            FileInfo KeyInfo = new FileInfo( "ValidCDKey" );
            if( !KeyInfo.Exists )
            {
                return;
            }

            // Read in the encrypted CD key
            StreamReader KeyReader = new StreamReader( "ValidCDKey" );
            EncryptedCDKey = KeyReader.ReadLine();
            KeyReader.Close();

            // Make sure ini file exists and is writable
            FileInfo IniInfo = new FileInfo( IniFile );
            if( !IniInfo.Exists )
            {
                return;
            }
            IniInfo.IsReadOnly = false;

            // Read ini file
            StreamReader Reader = new StreamReader( IniFile );
            if( Reader != null )
            {
                Line = Reader.ReadLine();
                while( Line != null )
                {
                    if( Line.ToLower().StartsWith( "encryptedproductkey=" ) )
                    {
                        Line = "EncryptedProductKey=" + EncryptedCDKey;
                    }

                    if( Line.ToLower().StartsWith( "language=" ) )
                    {
                        Line = "Language=" + Region;
                    }

                    Lines.Add( Line );
                    Line = Reader.ReadLine();
                }

                Reader.Close();

                // Write out ini file
                StreamWriter Writer = new StreamWriter( IniFile );
                if( Writer != null )
                {
                    foreach( string Entry in Lines )
                    {
                        Writer.WriteLine( Entry );
                    }

                    Writer.Close();
                }
            }
        }
    }
}