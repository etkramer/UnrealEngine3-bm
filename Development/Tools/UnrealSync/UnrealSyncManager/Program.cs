using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Drawing;
using System.Diagnostics;
using System.Reflection;
using System.IO;
using System.Runtime.InteropServices;

namespace UnrealSync.Manager
{
    static class Program
    {
		const uint BSF_ALLOWSFW = 0x00000080;
		const uint BSF_POSTMESSAGE = 0x00000010;
		const uint BSF_FORCEIFHUNG = 0x00000020;
		const uint BSM_APPLICATIONS = 0x00000008;

		static int sysDefinedShowWindowMessage;

		/// <summary>
		/// See <see href="http://msdn2.microsoft.com/en-us/library/ms644947.aspx"/> for the MSDN docs.
		/// </summary>
		/// <param name="lpString">Name of the windows message to create.</param>
		/// <returns>A new system wide windows message.</returns>
		[DllImport("user32.dll")]
		static extern int RegisterWindowMessage(string lpString);

		/// <summary>
		/// See <see href="http://msdn2.microsoft.com/en-us/library/ms644932.aspx"/> for the MSDN docs.
		/// </summary>
		/// <param name="dwFlags">Flags.</param>
		/// <param name="lpdwRecipients">The recipients.</param>
		/// <param name="uiMessage">The message to broadcast.</param>
		/// <param name="wParam">LPARAM.</param>
		/// <param name="lParam">WPARAM.</param>
		/// <returns>A positive number if it succeeds.</returns>
		[DllImport("user32.dll")]
		static extern int BroadcastSystemMessage(uint dwFlags,
												out uint lpdwRecipients,
												int uiMessage,
												IntPtr wParam,
												IntPtr lParam);

		/// <summary>
		/// This class filters messages for the system wide UnrealSync message.
		/// </summary>
		class MessageFilter : IMessageFilter
		{
			UnrealSyncApplicationContext appCtx;

			/// <summary>
			/// Constructor.
			/// </summary>
			/// <param name="appCtx">The application context.</param>
			public MessageFilter(UnrealSyncApplicationContext appCtx)
			{
				if(appCtx == null)
				{
					throw new ArgumentNullException("appCtx");
				}

				this.appCtx = appCtx;
			}

			#region IMessageFilter Members

			/// <summary>
			/// Filters messages.
			/// </summary>
			/// <param name="m">The message to be filtered.</param>
			/// <returns>True if the message was handled.</returns>
			public bool PreFilterMessage(ref Message m)
			{
				if(m.Msg == Program.sysDefinedShowWindowMessage)
				{
					appCtx.ShowMainWindow();
					return true;
				}

				return false;
			}

			#endregion
		}

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
			sysDefinedShowWindowMessage = RegisterWindowMessage("UnrealSync");

			bool bMutexCreated;

			using(System.Threading.Mutex mutex = new System.Threading.Mutex(true, "UnrealSyncMutex", out bMutexCreated))
			{
				if(!bMutexCreated)
				{
					uint recipients = BSM_APPLICATIONS;
					int ret = BroadcastSystemMessage(BSF_ALLOWSFW | BSF_FORCEIFHUNG | BSF_POSTMESSAGE, out recipients, sysDefinedShowWindowMessage, IntPtr.Zero, IntPtr.Zero);

					return;
				}

				Application.EnableVisualStyles();
				Application.SetCompatibleTextRenderingDefault(false);

				bool bStartVisible = true;

				if(args.Length > 0 && args[0].ToLower() == "-hidden")
				{
					bStartVisible = false;
				}

				using(UnrealSyncApplicationContext appCtx = new UnrealSyncApplicationContext(bStartVisible))
				{
					MessageFilter filter = new MessageFilter(appCtx);
					
					Application.AddMessageFilter(filter);
					Application.Run(appCtx);
					Application.RemoveMessageFilter(filter);
				}
			}
        }
    }
}