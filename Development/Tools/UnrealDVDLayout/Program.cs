using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace UnrealDVDLayout
{
    static class Program
    {
        [STAThread]
        static void Main( string[] Arguments )
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault( false );

            // Work from the branch root
            string Path = Environment.CurrentDirectory.Replace( "\\Binaries", "" );
            Environment.CurrentDirectory = Path;

            // Create the window
            UnrealDVDLayout MainWindow = new UnrealDVDLayout();
            MainWindow.Init();

            if( Arguments.Length > 0 )
            {
                // UnrealDVDLayout Game Platform lang lang lang
                MainWindow.HandleCommandLine( Arguments );
            }
            else
            {
                MainWindow.Show();

                while( MainWindow.Ticking )
                {
                    Application.DoEvents();

                    MainWindow.DoPopulateListBoxes();

                    // Yield a little time to the system
                    System.Threading.Thread.Sleep( 50 );
                }
            }

            MainWindow.Destroy();
        }
    }
}