using System;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime;
using System.Text;
using Interop.p4com;

namespace UnrealLoc
{
    class P4
    {
        UnrealLoc Main = null;
        private Interop.p4com.p4 IP4 = null;

        public P4( UnrealLoc InMain )
        {
            Main = InMain;

            try
            {
                IP4 = new Interop.p4com.p4();
            }
            catch
            {
                Main.Log( UnrealLoc.VerbosityLevel.Critical, "Perforce connection FAILED", Color.Red );
                IP4 = null;
            }
        }

        public bool Checkout( LanguageInfo Lang, string FileSpec )
        {
            Array Output;
            bool Success = false;

            IP4.Connect();
            IP4.Client = Main.Options.ClientSpec;
            IP4.Cwd = Environment.CurrentDirectory;
            IP4.ExceptionLevel = 2;

            try
            {
                Main.Log( UnrealLoc.VerbosityLevel.Informative , "Executing 'P4 edit " + FileSpec + "'", Color.Black );
                Output = IP4.run( "edit " + FileSpec );
                Main.Log( UnrealLoc.VerbosityLevel.Complex, Output, Color.Black );

                Success = true;
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                Main.Error( Lang, "(Perforce) Edit " + ex.Message );
            }

            IP4.Disconnect();
            return ( Success );
        }

        public bool AddToSourceControl( LanguageInfo Lang, string FileSpec )
        {
            Array Output;
            bool Success = false;

            IP4.Connect();
            IP4.Client = Main.Options.ClientSpec;
            IP4.Cwd = Environment.CurrentDirectory;
            IP4.ExceptionLevel = 2;

            try
            {
                Main.Log( UnrealLoc.VerbosityLevel.Informative, "Executing 'p4 fstat " + FileSpec + "'", Color.Black );
                Output = IP4.run( "fstat " + FileSpec );

                foreach( string Line in Output )
                {
                    if( Line.ToLower().Contains( "headaction" ) && Line.ToLower().Contains( "delete" ) )
                    {
                        throw new System.Runtime.InteropServices.COMException();
                    }
                }
            }
            catch( System.Runtime.InteropServices.COMException ex )
            {
                try
                {
                    Main.Log( UnrealLoc.VerbosityLevel.Informative, "Executing 'P4 add -t utf16 " + FileSpec + "'", Color.Black );
                    Output = IP4.run( "add -t utf16 " + FileSpec );
                    Main.Log( UnrealLoc.VerbosityLevel.Complex, Output, Color.Black );

                    Success = true;
                }
                catch
                {
                    Main.Error( Lang, "(Perforce) Add " + ex.Message );
                }
            }

            IP4.Disconnect();
            return ( Success );
        }

        public bool RevertUnchanged()
        {
            Array Output;
            bool Success = false;

            IP4.Connect();
            IP4.Client = Main.Options.ClientSpec;
            IP4.Cwd = Environment.CurrentDirectory;
            IP4.ExceptionLevel = 2;

            try
            {
                Main.Log( UnrealLoc.VerbosityLevel.Informative, "Executing 'P4 revert -a'", Color.Black );
                Output = IP4.run( "revert -a" );
                Main.Log( UnrealLoc.VerbosityLevel.Complex, Output, Color.Black );

                Success = true;
            }
            catch
            {
            }

            IP4.Disconnect();
            return ( Success );
        }
    }
}
