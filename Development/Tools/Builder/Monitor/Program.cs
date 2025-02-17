using System;
using System.IO;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Diagnostics;

namespace Monitor
{
	static class Monitor
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main( string[] Arguments )
		{
#if !DEBUG
			if( Arguments.Length == 0 )
			{
				bool Success = false;

				while( !Success )
				{
					try
					{
						FileInfo ExeInstance = new FileInfo( "MonitorInstance.exe" );
						if( ExeInstance.Exists )
						{
							ExeInstance.IsReadOnly = false;
							ExeInstance.Delete();
						}

						FileInfo Executable = new FileInfo( "Monitor.exe" );
						Executable.CopyTo( "MonitorInstance.exe", true );

						Process Instance = new Process();
						Instance.StartInfo.FileName = "MonitorInstance.exe";
						Instance.StartInfo.Arguments = "0";
						Instance.Start();

						Success = true;
					}
					catch
					{
					}
				}
				return;
			}
#endif
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			
			// Move to run from the root folder (like Controller)
			string OriginalDirectory = Environment.CurrentDirectory;
			if (!OriginalDirectory.ToLower().EndsWith("development\\builder"))
			{
				MessageBox.Show("Controller must be run from the 'Development\\Builder' folder!", "Controller Fatal Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}
			Environment.CurrentDirectory = OriginalDirectory.Substring(0, OriginalDirectory.Length - "\\Development\\Builder".Length);

			// Create the window
			Main MainWindow = new Main();
			MainWindow.Init();

			while (MainWindow.Ticking)
			{
				// Process the application message loop
				Application.DoEvents();
				MainWindow.Run();
			
				// Yield a little time to the system
				System.Threading.Thread.Sleep(500);
			}

			MainWindow.Destroy();
		
			// Restart the process if it's been requested
			if (MainWindow.Restart)
			{
				Environment.CurrentDirectory = OriginalDirectory;

				Process Instance = new Process();
				Instance.StartInfo.FileName = "Monitor.exe";
				Instance.StartInfo.Arguments = "";
				Instance.Start();
			}
		}
	}
}