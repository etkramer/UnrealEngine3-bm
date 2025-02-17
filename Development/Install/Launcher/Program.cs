using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Security.Permissions;

namespace EpicGames
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
			try
			{
				Application.EnableVisualStyles();
				Application.SetCompatibleTextRenderingDefault(false);

				Application.Run(new Launcher());
			}
			// This usually gets thrown when trying to run the application from a location on the network that is untrusted
			catch(MethodAccessException)
			{
				MessageBox.Show(Properties.Resources.UntrustedLocation_Message, Properties.Resources.UntrustedLocation_Caption);
			}
			// Anything else means something really bad happend
			catch(Exception e)
			{
				// So tell the user what happend and maybe he'll post it on the forums so we can figure out what's going wrong
				using(ExceptionBox Dlg = new ExceptionBox(e.ToString()))
				{
					Dlg.ShowDialog();
				}
			}
        }
    }
}