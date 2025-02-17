
using System;
using System.IO;
using System.Xml;
using System.Xml.Serialization;
using System.Text;
using System.Drawing;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Net;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Diagnostics;
using UnrealConsole.Network;
using ConsoleInterface;
using UnrealConsole.Main;
using UnrealConsoleRemoting;
using System.Runtime.Remoting.Channels.Ipc;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting;
using UnrealControls;

namespace UnrealConsole
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class UnrealConsoleWindow : System.Windows.Forms.Form, IUnrealConsole
	{
		// This is used to marshal calls to Print() into the UI thread
		private delegate void PrintDelegate(string Text);
		private delegate void UIThreadCrashDelegate(string Callstack);
		private delegate bool OpenTargetDelegate(string Platform, string Target, bool bClearOutputWindow);
		private delegate ConsoleInterface.PlatformTarget FindTargetDelegate(string Platform, string Target);

		private System.Windows.Forms.ToolStripMenuItem IDM_EXIT;
		private System.Windows.Forms.ToolStripMenuItem IDM_CLEARWINDOW;
		private System.Windows.Forms.ToolStripMenuItem IDM_SAVE;
		private System.Windows.Forms.ToolStripMenuItem FileMenu;
		private System.Windows.Forms.ToolStripMenuItem EditMenu;
		private System.Windows.Forms.MenuStrip MainMenu;
		private System.Windows.Forms.ToolStripSeparator FileSeparator1;
		//private IContainer components;

		/// <summary>
		/// The dialog box used for finding text in the console.
		/// </summary>
		FindDialog mFindDlg = new FindDialog();

		private ToolStripMenuItem IDM_CONNECT;
		private ToolStripMenuItem IDM_CONSOLE;
		private ToolStripMenuItem IDM_REBOOT;
		private ToolStripMenuItem IDM_SCREENCAPTURE;
		private ToolStripMenuItem Menu_EditFind;
		private Properties.Settings LastUsedSettings;
		private StringBuilder TTYBuffer = new StringBuilder();
		private ToolStripMenuItem Menu_ScrollToEnd;
		private ToolStripMenuItem Mene_InvisFindNext;
		private ToolStripMenuItem Menu_InvisFindPrev;
		private ToolStripMenuItem IDM_ALWAYS_LOG;
		private ToolStripSeparator toolStripSeparator1;
		private ToolStripSeparator toolStripSeparator2;
		private ToolStripMenuItem IDM_CLEAR_CMD_HISTORY;
		private ToolStripMenuItem IDM_DELETE_PDB;

		private UnrealControls.DynamicTabControl mMainTabControl;
		private ToolStripMenuItem IDM_CLEARALLWINDOWS;
		private ToolStripMenuItem IDM_CRASHREPORTFILTER;
		private ToolStripMenuItem IDM_CRASHFILTER_SELECTALL;
		private ToolStripMenuItem IDM_CRASHFILTER_DESELECTALL;
		private ToolStripSeparator toolStripMenuItem1;
		private ToolStripMenuItem IDM_CRASHFILTER_DEBUG;
		private ToolStripMenuItem IDM_CRASHFILTER_RELEASE;
		private ToolStripMenuItem IDM_CRASHFILTER_RELEASEFORSHIP;
		private ToolStripMenuItem IDM_SAVEALL;
		private ToolStripMenuItem IDM_DUMPTYPE;
		private ToolStripMenuItem IDM_DUMP_NORMAL;
		private ToolStripMenuItem IDM_DUMP_WITHFULLMEM;
		private bool mSendCommandToAll;

		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="InPlatform">The target platform.</param>
		/// <param name="InTargetName">The name of the target pc/console.</param>
		public UnrealConsoleWindow(string InPlatform, string InTargetName)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			LastUsedSettings = Properties.Settings.Default;

			LoadSettings();

			if(InPlatform != null && InTargetName != null)
			{
				ConsoleInterface.Platform Plat = FindPlatform(InPlatform);

				if(Plat != null)
				{
					Plat.EnumerateAvailableTargets();
				}

				// set the target
				SetTarget(InPlatform, InTargetName);
			}
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose(bool disposing)
		{
			if(disposing)
			{
				//if(components != null)
				//{
				//    components.Dispose();
				//}

				foreach(ConsoleInterface.Platform CurPlatform in ConsoleInterface.DLLInterface.Platforms)
				{
					CurPlatform.Dispose();
				}
			}

			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(UnrealConsoleWindow));
			this.MainMenu = new System.Windows.Forms.MenuStrip();
			this.FileMenu = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_SAVE = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_ALWAYS_LOG = new System.Windows.Forms.ToolStripMenuItem();
			this.FileSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.IDM_EXIT = new System.Windows.Forms.ToolStripMenuItem();
			this.EditMenu = new System.Windows.Forms.ToolStripMenuItem();
			this.Menu_EditFind = new System.Windows.Forms.ToolStripMenuItem();
			this.Menu_ScrollToEnd = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
			this.IDM_CLEARWINDOW = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_CLEARALLWINDOWS = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_CLEAR_CMD_HISTORY = new System.Windows.Forms.ToolStripMenuItem();
			this.Mene_InvisFindNext = new System.Windows.Forms.ToolStripMenuItem();
			this.Menu_InvisFindPrev = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_CONSOLE = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_CONNECT = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.IDM_REBOOT = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_SCREENCAPTURE = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_DELETE_PDB = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_DUMPTYPE = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_DUMP_NORMAL = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_DUMP_WITHFULLMEM = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_CRASHREPORTFILTER = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_CRASHFILTER_SELECTALL = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_CRASHFILTER_DESELECTALL = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.IDM_CRASHFILTER_DEBUG = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_CRASHFILTER_RELEASE = new System.Windows.Forms.ToolStripMenuItem();
			this.IDM_CRASHFILTER_RELEASEFORSHIP = new System.Windows.Forms.ToolStripMenuItem();
			this.mMainTabControl = new UnrealControls.DynamicTabControl();
			this.IDM_SAVEALL = new System.Windows.Forms.ToolStripMenuItem();
			this.MainMenu.SuspendLayout();
			this.SuspendLayout();
			// 
			// MainMenu
			// 
			this.MainMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.FileMenu,
            this.EditMenu,
            this.IDM_CONSOLE,
            this.IDM_CRASHREPORTFILTER});
			this.MainMenu.Location = new System.Drawing.Point(0, 0);
			this.MainMenu.Name = "MainMenu";
			this.MainMenu.Size = new System.Drawing.Size(811, 24);
			this.MainMenu.TabIndex = 0;
			// 
			// FileMenu
			// 
			this.FileMenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.IDM_SAVE,
            this.IDM_SAVEALL,
            this.IDM_ALWAYS_LOG,
            this.FileSeparator1,
            this.IDM_EXIT});
			this.FileMenu.Name = "FileMenu";
			this.FileMenu.Size = new System.Drawing.Size(37, 20);
			this.FileMenu.Text = "&File";
			// 
			// IDM_SAVE
			// 
			this.IDM_SAVE.Name = "IDM_SAVE";
			this.IDM_SAVE.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.S)));
			this.IDM_SAVE.Size = new System.Drawing.Size(224, 22);
			this.IDM_SAVE.Text = "&Save Log...";
			this.IDM_SAVE.Click += new System.EventHandler(this.IDM_SAVE_Click);
			// 
			// IDM_ALWAYS_LOG
			// 
			this.IDM_ALWAYS_LOG.Name = "IDM_ALWAYS_LOG";
			this.IDM_ALWAYS_LOG.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.L)));
			this.IDM_ALWAYS_LOG.Size = new System.Drawing.Size(224, 22);
			this.IDM_ALWAYS_LOG.Text = "&Always Log...";
			this.IDM_ALWAYS_LOG.Click += new System.EventHandler(this.IDM_ALWAYS_LOG_Click);
			// 
			// FileSeparator1
			// 
			this.FileSeparator1.Name = "FileSeparator1";
			this.FileSeparator1.Size = new System.Drawing.Size(221, 6);
			// 
			// IDM_EXIT
			// 
			this.IDM_EXIT.Name = "IDM_EXIT";
			this.IDM_EXIT.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Alt | System.Windows.Forms.Keys.F4)));
			this.IDM_EXIT.Size = new System.Drawing.Size(224, 22);
			this.IDM_EXIT.Text = "E&xit";
			this.IDM_EXIT.Click += new System.EventHandler(this.OnExit);
			// 
			// EditMenu
			// 
			this.EditMenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.Menu_EditFind,
            this.Menu_ScrollToEnd,
            this.toolStripSeparator2,
            this.IDM_CLEARWINDOW,
            this.IDM_CLEARALLWINDOWS,
            this.IDM_CLEAR_CMD_HISTORY,
            this.Mene_InvisFindNext,
            this.Menu_InvisFindPrev});
			this.EditMenu.Name = "EditMenu";
			this.EditMenu.Size = new System.Drawing.Size(39, 20);
			this.EditMenu.Text = "&Edit";
			// 
			// Menu_EditFind
			// 
			this.Menu_EditFind.Name = "Menu_EditFind";
			this.Menu_EditFind.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.F)));
			this.Menu_EditFind.Size = new System.Drawing.Size(227, 22);
			this.Menu_EditFind.Text = "&Find";
			this.Menu_EditFind.Click += new System.EventHandler(this.Menu_EditFind_Click);
			// 
			// Menu_ScrollToEnd
			// 
			this.Menu_ScrollToEnd.Name = "Menu_ScrollToEnd";
			this.Menu_ScrollToEnd.Size = new System.Drawing.Size(227, 22);
			this.Menu_ScrollToEnd.Text = "Scroll to End";
			this.Menu_ScrollToEnd.Click += new System.EventHandler(this.Menu_ScrollToEnd_Click);
			// 
			// toolStripSeparator2
			// 
			this.toolStripSeparator2.Name = "toolStripSeparator2";
			this.toolStripSeparator2.Size = new System.Drawing.Size(224, 6);
			// 
			// IDM_CLEARWINDOW
			// 
			this.IDM_CLEARWINDOW.Name = "IDM_CLEARWINDOW";
			this.IDM_CLEARWINDOW.ShortcutKeys = System.Windows.Forms.Keys.F11;
			this.IDM_CLEARWINDOW.Size = new System.Drawing.Size(227, 22);
			this.IDM_CLEARWINDOW.Text = "&Clear window";
			this.IDM_CLEARWINDOW.Click += new System.EventHandler(this.OnClearWindow);
			// 
			// IDM_CLEARALLWINDOWS
			// 
			this.IDM_CLEARALLWINDOWS.Name = "IDM_CLEARALLWINDOWS";
			this.IDM_CLEARALLWINDOWS.ShortcutKeys = System.Windows.Forms.Keys.F12;
			this.IDM_CLEARALLWINDOWS.Size = new System.Drawing.Size(227, 22);
			this.IDM_CLEARALLWINDOWS.Text = "Clear All Windows";
			this.IDM_CLEARALLWINDOWS.Click += new System.EventHandler(this.IDM_CLEARALLWINDOWS_Click);
			// 
			// IDM_CLEAR_CMD_HISTORY
			// 
			this.IDM_CLEAR_CMD_HISTORY.Name = "IDM_CLEAR_CMD_HISTORY";
			this.IDM_CLEAR_CMD_HISTORY.ShortcutKeys = System.Windows.Forms.Keys.F10;
			this.IDM_CLEAR_CMD_HISTORY.Size = new System.Drawing.Size(227, 22);
			this.IDM_CLEAR_CMD_HISTORY.Text = "Clear Command History";
			this.IDM_CLEAR_CMD_HISTORY.Click += new System.EventHandler(this.IDM_CLEAR_CMD_HISTORY_Click);
			// 
			// Mene_InvisFindNext
			// 
			this.Mene_InvisFindNext.Name = "Mene_InvisFindNext";
			this.Mene_InvisFindNext.ShortcutKeys = System.Windows.Forms.Keys.F3;
			this.Mene_InvisFindNext.Size = new System.Drawing.Size(227, 22);
			this.Mene_InvisFindNext.Text = "InvisFindNext";
			this.Mene_InvisFindNext.Visible = false;
			this.Mene_InvisFindNext.Click += new System.EventHandler(this.Menu_InvisFindNext_Click);
			// 
			// Menu_InvisFindPrev
			// 
			this.Menu_InvisFindPrev.Name = "Menu_InvisFindPrev";
			this.Menu_InvisFindPrev.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Shift | System.Windows.Forms.Keys.F3)));
			this.Menu_InvisFindPrev.Size = new System.Drawing.Size(227, 22);
			this.Menu_InvisFindPrev.Text = "InvisFindPrev";
			this.Menu_InvisFindPrev.Visible = false;
			this.Menu_InvisFindPrev.Click += new System.EventHandler(this.Menu_InvisFindPrev_Click);
			// 
			// IDM_CONSOLE
			// 
			this.IDM_CONSOLE.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.IDM_CONNECT,
            this.toolStripSeparator1,
            this.IDM_REBOOT,
            this.IDM_SCREENCAPTURE,
            this.IDM_DELETE_PDB,
            this.IDM_DUMPTYPE});
			this.IDM_CONSOLE.Name = "IDM_CONSOLE";
			this.IDM_CONSOLE.Size = new System.Drawing.Size(62, 20);
			this.IDM_CONSOLE.Text = "Console";
			this.IDM_CONSOLE.DropDownOpening += new System.EventHandler(this.IDM_CONSOLE_DropDownOpening);
			// 
			// IDM_CONNECT
			// 
			this.IDM_CONNECT.Name = "IDM_CONNECT";
			this.IDM_CONNECT.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
			this.IDM_CONNECT.Size = new System.Drawing.Size(190, 22);
			this.IDM_CONNECT.Text = "Connect...";
			this.IDM_CONNECT.Click += new System.EventHandler(this.IDM_CONNECT_Click);
			// 
			// toolStripSeparator1
			// 
			this.toolStripSeparator1.Name = "toolStripSeparator1";
			this.toolStripSeparator1.Size = new System.Drawing.Size(187, 6);
			// 
			// IDM_REBOOT
			// 
			this.IDM_REBOOT.Name = "IDM_REBOOT";
			this.IDM_REBOOT.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.R)));
			this.IDM_REBOOT.Size = new System.Drawing.Size(190, 22);
			this.IDM_REBOOT.Text = "Reboot Target";
			this.IDM_REBOOT.Click += new System.EventHandler(this.IDM_REBOOT_Click);
			// 
			// IDM_SCREENCAPTURE
			// 
			this.IDM_SCREENCAPTURE.Name = "IDM_SCREENCAPTURE";
			this.IDM_SCREENCAPTURE.ShortcutKeys = System.Windows.Forms.Keys.F9;
			this.IDM_SCREENCAPTURE.Size = new System.Drawing.Size(190, 22);
			this.IDM_SCREENCAPTURE.Text = "Screen Capture";
			this.IDM_SCREENCAPTURE.Click += new System.EventHandler(this.IDM_SCREENCAPTURE_Click);
			// 
			// IDM_DELETE_PDB
			// 
			this.IDM_DELETE_PDB.Name = "IDM_DELETE_PDB";
			this.IDM_DELETE_PDB.Size = new System.Drawing.Size(190, 22);
			this.IDM_DELETE_PDB.Text = "Delete PDB\'s";
			this.IDM_DELETE_PDB.Click += new System.EventHandler(this.IDM_DELETE_PDB_Click);
			// 
			// IDM_DUMPTYPE
			// 
			this.IDM_DUMPTYPE.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.IDM_DUMP_NORMAL,
            this.IDM_DUMP_WITHFULLMEM});
			this.IDM_DUMPTYPE.Name = "IDM_DUMPTYPE";
			this.IDM_DUMPTYPE.Size = new System.Drawing.Size(190, 22);
			this.IDM_DUMPTYPE.Text = "Dump Type";
			// 
			// IDM_DUMP_NORMAL
			// 
			this.IDM_DUMP_NORMAL.Name = "IDM_DUMP_NORMAL";
			this.IDM_DUMP_NORMAL.Size = new System.Drawing.Size(169, 22);
			this.IDM_DUMP_NORMAL.Text = "Normal";
			this.IDM_DUMP_NORMAL.Click += new System.EventHandler(this.IDM_DUMP_NORMAL_Click);
			// 
			// IDM_DUMP_WITHFULLMEM
			// 
			this.IDM_DUMP_WITHFULLMEM.Name = "IDM_DUMP_WITHFULLMEM";
			this.IDM_DUMP_WITHFULLMEM.Size = new System.Drawing.Size(169, 22);
			this.IDM_DUMP_WITHFULLMEM.Text = "With Full Memory";
			this.IDM_DUMP_WITHFULLMEM.Click += new System.EventHandler(this.IDM_DUMP_WITHFULLMEM_Click);
			// 
			// IDM_CRASHREPORTFILTER
			// 
			this.IDM_CRASHREPORTFILTER.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.IDM_CRASHFILTER_SELECTALL,
            this.IDM_CRASHFILTER_DESELECTALL,
            this.toolStripMenuItem1,
            this.IDM_CRASHFILTER_DEBUG,
            this.IDM_CRASHFILTER_RELEASE,
            this.IDM_CRASHFILTER_RELEASEFORSHIP});
			this.IDM_CRASHREPORTFILTER.Name = "IDM_CRASHREPORTFILTER";
			this.IDM_CRASHREPORTFILTER.Size = new System.Drawing.Size(116, 20);
			this.IDM_CRASHREPORTFILTER.Text = "Crash Report Filter";
			this.IDM_CRASHREPORTFILTER.DropDownOpening += new System.EventHandler(this.IDM_CRASHREPORTFILTER_DropDownOpening);
			// 
			// IDM_CRASHFILTER_SELECTALL
			// 
			this.IDM_CRASHFILTER_SELECTALL.Name = "IDM_CRASHFILTER_SELECTALL";
			this.IDM_CRASHFILTER_SELECTALL.Size = new System.Drawing.Size(157, 22);
			this.IDM_CRASHFILTER_SELECTALL.Text = "Select All";
			this.IDM_CRASHFILTER_SELECTALL.Click += new System.EventHandler(this.IDM_CRASHFILTER_SELECTALL_Click);
			// 
			// IDM_CRASHFILTER_DESELECTALL
			// 
			this.IDM_CRASHFILTER_DESELECTALL.Name = "IDM_CRASHFILTER_DESELECTALL";
			this.IDM_CRASHFILTER_DESELECTALL.Size = new System.Drawing.Size(157, 22);
			this.IDM_CRASHFILTER_DESELECTALL.Text = "Deselect All";
			this.IDM_CRASHFILTER_DESELECTALL.Click += new System.EventHandler(this.IDM_CRASHFILTER_DESELECTALL_Click);
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(154, 6);
			// 
			// IDM_CRASHFILTER_DEBUG
			// 
			this.IDM_CRASHFILTER_DEBUG.Name = "IDM_CRASHFILTER_DEBUG";
			this.IDM_CRASHFILTER_DEBUG.Size = new System.Drawing.Size(157, 22);
			this.IDM_CRASHFILTER_DEBUG.Text = "Debug";
			this.IDM_CRASHFILTER_DEBUG.Click += new System.EventHandler(this.IDM_CRASHFILTER_DEBUG_Click);
			// 
			// IDM_CRASHFILTER_RELEASE
			// 
			this.IDM_CRASHFILTER_RELEASE.Name = "IDM_CRASHFILTER_RELEASE";
			this.IDM_CRASHFILTER_RELEASE.Size = new System.Drawing.Size(157, 22);
			this.IDM_CRASHFILTER_RELEASE.Text = "Release";
			this.IDM_CRASHFILTER_RELEASE.Click += new System.EventHandler(this.IDM_CRASHFILTER_RELEASE_Click);
			// 
			// IDM_CRASHFILTER_RELEASEFORSHIP
			// 
			this.IDM_CRASHFILTER_RELEASEFORSHIP.Name = "IDM_CRASHFILTER_RELEASEFORSHIP";
			this.IDM_CRASHFILTER_RELEASEFORSHIP.Size = new System.Drawing.Size(157, 22);
			this.IDM_CRASHFILTER_RELEASEFORSHIP.Text = "Release for Ship";
			this.IDM_CRASHFILTER_RELEASEFORSHIP.Click += new System.EventHandler(this.IDM_CRASHFILTER_RELEASEFORSHIP_Click);
			// 
			// mMainTabControl
			// 
			this.mMainTabControl.Dock = System.Windows.Forms.DockStyle.Fill;
			this.mMainTabControl.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
			this.mMainTabControl.Location = new System.Drawing.Point(0, 24);
			this.mMainTabControl.Name = "mMainTabControl";
			this.mMainTabControl.SelectedIndex = 0;
			this.mMainTabControl.SelectedTab = null;
			this.mMainTabControl.Size = new System.Drawing.Size(811, 452);
			this.mMainTabControl.TabIndex = 0;
			this.mMainTabControl.SelectedIndexChanged += new System.EventHandler<System.EventArgs>(this.mMainTabControl_SelectedIndexChanged);
			this.mMainTabControl.ControlRemoved += new System.Windows.Forms.ControlEventHandler(this.MainTabControl_ControlRemoved);
			// 
			// IDM_SAVEALL
			// 
			this.IDM_SAVEALL.Name = "IDM_SAVEALL";
			this.IDM_SAVEALL.ShortcutKeys = ((System.Windows.Forms.Keys)(((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Shift)
						| System.Windows.Forms.Keys.S)));
			this.IDM_SAVEALL.Size = new System.Drawing.Size(224, 22);
			this.IDM_SAVEALL.Text = "Save All Logs...";
			this.IDM_SAVEALL.Click += new System.EventHandler(this.IDM_SAVEALL_Click);
			// 
			// UnrealConsoleWindow
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(811, 476);
			this.Controls.Add(this.mMainTabControl);
			this.Controls.Add(this.MainMenu);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.KeyPreview = true;
			this.MainMenuStrip = this.MainMenu;
			this.Name = "UnrealConsoleWindow";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
			this.Text = "Unreal Console";
			this.MainMenu.ResumeLayout(false);
			this.MainMenu.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] Args)
		{
			// parse arguments

			try
			{
				Application.EnableVisualStyles();

				string TargetName = null;
				string Platform = null;
				// go over argument list
				foreach(string Arg in Args)
				{
					if(Arg.ToLower().StartsWith("platform="))
					{
						Platform = Arg.Substring(9);
					}
					else if(Arg.ToLower().StartsWith("-platform="))
					{
						Platform = Arg.Substring(10);
					}
					// anything else if platform name
					else
					{
						// make sure it's not already set
						if(TargetName == null)
						{
							if(Arg.ToLower().StartsWith("target="))
							{
								TargetName = Arg.Substring(7);
							}
							else
							{
								TargetName = Arg;
							}
						}
					}
				}

				// force load all of the platforms
				DLLInterface.LoadPlatforms(PlatformType.All);

				UnrealConsoleWindow ConsoleApp = new UnrealConsoleWindow(Platform, TargetName);
				RemoteUCObject.InternalUnrealConsole = ConsoleApp;

				Process CurProc = Process.GetCurrentProcess();
				IpcChannel Channel = new IpcChannel(CurProc.Id.ToString());
				ChannelServices.RegisterChannel(Channel, true);

				RemotingConfiguration.RegisterWellKnownServiceType(typeof(RemoteUCObject), CurProc.Id.ToString(), WellKnownObjectMode.SingleCall);

				Console.WriteLine("Remoting server created!");
				Console.WriteLine("Remoting channel name: {0}", Channel.ChannelName);
				Console.WriteLine("Remoting channel priority: {0}", Channel.ChannelPriority.ToString());
				Console.WriteLine("Channel URI's:");

				// Show the URIs associated with the channel.
				System.Runtime.Remoting.Channels.ChannelDataStore ChannelData = (System.Runtime.Remoting.Channels.ChannelDataStore)Channel.ChannelData;

				foreach(string URI in ChannelData.ChannelUris)
				{
					Console.WriteLine(URI);
				}

				Console.WriteLine("Channel URL's:");

				// Parse the channel's URI.
				string[] Urls = Channel.GetUrlsForUri(CurProc.Id.ToString());
				
				if(Urls.Length > 0)
				{
					string ObjectUrl = Urls[0];
					string ObjectUri;
					string ChannelUri = Channel.Parse(ObjectUrl, out ObjectUri);

					Console.WriteLine("Object URI: {0}.", ObjectUri);
					Console.WriteLine("Channel URI: {0}.", ChannelUri);
					Console.WriteLine("Object URL: {0}.", ObjectUrl);
					Console.WriteLine("+++++++++++++++++++++++++++++++++++++++");
				}


				CurProc.Dispose();

				Application.Run(ConsoleApp);
			}
// in debug mode let exceptions fall through so the vs.net debugger can catch them at the spot in the src where they occur
#if !DEBUG
			catch(Exception ex)
			{
				using(ExceptionBox Box = new ExceptionBox(ex))
				{
					Box.ShowDialog();
				}
			}
#endif
			finally
			{
				Properties.Settings.Default.Save();
			}
		}

		/// <summary>
		/// Loads user settings.
		/// </summary>
		void LoadSettings()
		{
			LastUsedSettings = Properties.Settings.Default;

			this.Size = LastUsedSettings.MainWindowSize;
			IDM_ALWAYS_LOG.Checked = LastUsedSettings.AlwaysLog;
		}

		/// <summary>
		/// Saves user settings.
		/// </summary>
		void WriteSettings()
		{
			Properties.Settings SettingsCache = Properties.Settings.Default;

			SettingsCache.MainWindowSize = this.Size;
		}

		/// <summary>
		/// Callback for when the form is closing.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);

			WriteSettings();
		}

		/// <summary>
		/// Callback for clicking the Exit menu option.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnExit(object sender, System.EventArgs e)
		{
			this.Close();
		}

		/// <summary>
		/// Finds a target with the specified name/IP on the specified platform.
		/// </summary>
		/// <param name="NewPlatformName">The platform the target belongs to.</param>
		/// <param name="NewTargetName">The name, debug channel IP address, or title IP address for the target to retrieve.</param>
		/// <returns>null if the target could not be found.</returns>
		static ConsoleInterface.PlatformTarget FindTarget(string NewPlatformName, string NewTargetName)
		{
			ConsoleInterface.PlatformTarget RetTarget = null;

			if(NewPlatformName != null && NewTargetName != null)
			{
				ConsoleInterface.Platform NewPlatform = FindPlatform(NewPlatformName);

				if(NewPlatform != null)
				{
					IPAddress Addr;

					IPAddress.TryParse(NewTargetName, out Addr);

					if(Addr != null && BitConverter.ToUInt32(Addr.GetAddressBytes(), 0) == 0)
					{
						Addr = null;
					}

					foreach(ConsoleInterface.PlatformTarget CurTarget in NewPlatform.Targets)
					{
						if((Addr != null && (CurTarget.DebugIPAddress.Equals(Addr) || CurTarget.IPAddress.Equals(Addr))) || CurTarget.Name.Equals(NewTargetName, StringComparison.InvariantCultureIgnoreCase))
						{
							RetTarget = CurTarget;
							break;
						}
					}
				}
			}

			return RetTarget;
		}

		/// <summary>
		/// Finds the platform with the supplied name.
		/// </summary>
		/// <param name="NewPlatformName">The name of the platform to search for.</param>
		/// <returns>null if the platform with the specified name could not be found.</returns>
		private static ConsoleInterface.Platform FindPlatform(string NewPlatformName)
		{
			ConsoleInterface.Platform NewPlatform = null;

			foreach(ConsoleInterface.Platform CurPlatform in DLLInterface.Platforms)
			{
				if(CurPlatform.Name.Equals(NewPlatformName, StringComparison.InvariantCultureIgnoreCase))
				{
					NewPlatform = CurPlatform;
					break;
				}
			}
			return NewPlatform;
		}

		/// <summary>
		/// Sets the target, and if no target is given, then it will popup a dialog to choose one
		/// </summary>
		private void SetTarget(string NewPlatformName, string NewTargetName)
		{
			ConsoleInterface.PlatformTarget[] NewTargets = new PlatformTarget[] { FindTarget(NewPlatformName, NewTargetName) };

			if(NewTargets[0] == null)
			{
				using(NetworkConnectionDialog Dlg = new NetworkConnectionDialog())
				{
					if(Dlg.ShowDialog(this) == DialogResult.OK)
					{
						NewTargets = Dlg.SelectedTargets;
					}
					else
					{
						return;
					}
				}
			}

			if(NewTargets.Length > 0)
			{
				foreach(PlatformTarget CurTarg in NewTargets)
				{
					ConsoleTargetTabPage TargetTab = FindTargetTab(CurTarg);

					if(TargetTab == null)
					{
						mMainTabControl.TabPages.Add(new ConsoleTargetTabPage(CurTarg, Properties.Settings.Default.AlwaysLog, mSendCommandToAll));
					}
					else
					{
						mMainTabControl.SelectedTab = TargetTab;
					}
				}
			}
		}

		/// <summary>
		/// Finds the tab that owns the specified target.
		/// </summary>
		/// <param name="PlatformName">The platform the target belongs to.</param>
		/// <param name="TargetName">The name, debug channel IP, or title IP of the requested target.</param>
		/// <returns>null if the tab that the specified target belongs to does not exist.</returns>
		ConsoleTargetTabPage FindTargetTab(string PlatformName, string TargetName)
		{
			ConsoleInterface.PlatformTarget Targ = FindTarget(PlatformName, TargetName);

			if(Targ != null)
			{
				return FindTargetTab(Targ);
			}

			return null;
		}

		/// <summary>
		/// Find the tab that owns the specified target.
		/// </summary>
		/// <param name="Target">The target who's tab is being searched for.</param>
		/// <returns>null if the tab could not be found.</returns>
		ConsoleTargetTabPage FindTargetTab(ConsoleInterface.PlatformTarget Target)
		{
			foreach(ConsoleTargetTabPage CurTab in mMainTabControl.TabPages)
			{
				if(CurTab.Target == Target)
				{
					return CurTab;
				}
			}

			return null;
		}

		/// <summary>
		/// Callback for connection to a target.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_CONNECT_Click(object sender, EventArgs e)
		{
			// change targets with the popup dialog ("NullPlatform" to make sure the dialog pops up)
			SetTarget(null, null);
		}

		/// <summary>
		/// Callback for clearing the TTY buffer.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnClearWindow(object sender, System.EventArgs e)
		{
			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				CurTab.TTYText.Clear();
			}
		}

		/// <summary>
		/// Callback for rebooting the current target.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_REBOOT_Click(object sender, EventArgs e)
		{
			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				CurTab.Target.Reboot();
			}
		}

		/// <summary>
		/// Callback for capturing a screen shot from the current target.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_SCREENCAPTURE_Click(object sender, EventArgs e)
		{
			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				string TempPath = Path.GetTempFileName();

				if(CurTab.Target.ParentPlatform.Type == ConsoleInterface.PlatformType.PS3)
				{
					TempPath = Path.Combine("PS3\\Screenshots\\", Path.GetFileNameWithoutExtension(TempPath) + ".bmp");
				}
				else
				{
					TempPath = Path.Combine(Path.GetDirectoryName(TempPath), Path.GetFileNameWithoutExtension(TempPath) + ".bmp");
				}
				
				// make DLL grab the screenshot
				if(!CurTab.Target.ScreenShot(TempPath))
				{
					MessageBox.Show("Failed to take a screenshot.");
					return;
				}

				Image Screenshot = null;
				try
				{
					// load the screenshot image
					Screenshot = Bitmap.FromFile(TempPath);

					ScreenShotForm NewFrm = new ScreenShotForm(string.Format("Screenshot from \'{0}\' taken at {3:h:mm:ss tt}: {1}x{2}", CurTab.Target.Name, Screenshot.Width, Screenshot.Height, DateTime.Now), Screenshot);

					// Normally this would be bad because we are never calling Dispose() on NewFrm but since
					// ScreenShotForm calls Dispose() in its OnClosed() for this very reason it's not an issue
					NewFrm.Show();
				}
				catch(Exception ex)
				{
					using(ExceptionBox Box = new ExceptionBox(ex))
					{
						Box.ShowDialog(this);
					}
				}
			}
		}

		/// <summary>
		/// Callback for saving the text buffer to disk.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_SAVE_Click(object sender, EventArgs e)
		{
			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				// save the log window to a text file
				using(SaveFileDialog Dialog = new SaveFileDialog())
				{
					Dialog.DefaultExt = "txt";
					Dialog.FileName = string.Format("{0}_{1}.txt", CurTab.Target.Name, CurTab.Target.ParentPlatform.Name); 
					Dialog.Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";

					if(Dialog.ShowDialog(this) == DialogResult.OK && Dialog.FileName.Length > 0)
					{
						// write the string to a file
						try
						{
							CurTab.TTYText.SaveToFile(Dialog.FileName);
						}
						catch(Exception)
						{
							MessageBox.Show(this, string.Format("Could not save \'{0}\'!", Dialog.FileName), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
						}
					}
				}
			}
        }

		/// <summary>
		/// Callback for saving the text buffer of all tabs to disk.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_SAVEALL_Click(object sender, EventArgs e)
		{
			ConsoleTargetTabPage CurSelectedTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurSelectedTab != null)
			{
				// save the log window to a text file
				using(SaveFileDialog Dialog = new SaveFileDialog())
				{
					Dialog.DefaultExt = "txt";
					Dialog.FileName = "%TARGETNAME%_%PLATFORMNAME%.txt";
					Dialog.Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";

					if(Dialog.ShowDialog(this) == DialogResult.OK && Dialog.FileName.Length > 0)
					{
						// write the string to a file

						foreach(ConsoleTargetTabPage CurTab in mMainTabControl.TabPages)
						{
							string CurFileName = Dialog.FileName.Replace("%TARGETNAME%", CurTab.Target.Name).Replace("%PLATFORMNAME%", CurTab.Target.ParentPlatform.Type.ToString());

							try
							{
								CurTab.TTYText.SaveToFile(CurFileName);
							}
							catch(Exception)
							{
								MessageBox.Show(this, string.Format("Could not save \'{0}\'!", CurFileName), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
							}
						}
					}
				}
			}
        }

		/// <summary>
		/// Callback for enabling logging.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
        private void IDM_ALWAYS_LOG_Click(object sender, EventArgs e)
        {
			Properties.Settings.Default.AlwaysLog = !Properties.Settings.Default.AlwaysLog;
			IDM_ALWAYS_LOG.Checked = Properties.Settings.Default.AlwaysLog;

			foreach(ConsoleTargetTabPage CurTab in mMainTabControl.TabPages)
			{
				CurTab.LogOutput = Properties.Settings.Default.AlwaysLog;
			}
        }

		/// <summary>
		/// Callback for bringing up the Find dialog box.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Menu_EditFind_Click(object sender, EventArgs e)
		{
			if(!mFindDlg.Visible)
			{
				if(mFindDlg.IsDisposed)
				{
					mFindDlg = new FindDialog();
				}

				mFindDlg.Show(this);
			}
			else
			{
				mFindDlg.Focus();
			}

			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				this.mFindDlg.TextBox = CurTab.TTYText;
				this.mFindDlg.UpdateFindTextFromSelectedText();
			}
		}

		/// <summary>
		/// Callback scrolls the cursor to the end of the TTY text buffer.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Menu_ScrollToEnd_Click(object sender, EventArgs e)
		{
			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				CurTab.TTYText.Focus();
				CurTab.TTYText.ScrollToEnd();
			}
		}

		/// <summary>
		/// Callback for Find Next.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Menu_InvisFindNext_Click(object sender, EventArgs e)
		{
			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				this.mFindDlg.TextBox = CurTab.TTYText;
				this.mFindDlg.ExecuteFindNext(true);
			}
		}

		/// <summary>
		/// Callback for Find Previous.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Menu_InvisFindPrev_Click(object sender, EventArgs e)
		{
			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				this.mFindDlg.TextBox = CurTab.TTYText;
				this.mFindDlg.ExecuteFindNext(false);
			}
		}

		/// <summary>
		/// Event handler.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void mMainTabControl_SelectedIndexChanged(object sender, EventArgs e)
		{
			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				this.mFindDlg.TextBox = CurTab.TTYText;
			}
		}

		/// <summary>
		/// Event handler.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void MainTabControl_ControlRemoved(object sender, ControlEventArgs e)
		{
			ConsoleTargetTabPage Tab = e.Control as ConsoleTargetTabPage;

			if(Tab != null)
			{
				Tab.Dispose();
			}
		}

		/// <summary>
		/// Event handler.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_CLEAR_CMD_HISTORY_Click(object sender, EventArgs e)
		{
			Properties.Settings.Default.CommandHistory.Clear();

			foreach(ConsoleTargetTabPage CurPage in mMainTabControl.TabPages)
			{
				CurPage.ClearCommandHistoryState();
			}
		}

		/// <summary>
		/// A version of <see cref="OpenTarget"/> that gets called on the UI thread.
		/// </summary>
		/// <param name="Platform">The platform the target belongs to.</param>
		/// <param name="Target">The name, debug channel IP, or title IP of the requested target.</param>
		/// <param name="bClearOutputWindow">True if the output for the specified target is to be cleared.</param>
		/// <returns>True if the target is located.</returns>
		private bool UIThreadOpenTarget(string Platform, string Target, bool bClearOutputWindow)
		{
			if(this.WindowState == FormWindowState.Minimized)
			{
				this.WindowState = FormWindowState.Normal;
			}

			this.Activate();

			PlatformTarget PlatTarget = FindTarget(Platform, Target);

			if(PlatTarget == null)
			{
				return false;
			}

			ConsoleTargetTabPage Tab = FindTargetTab(PlatTarget);

			if(Tab == null)
			{
				mMainTabControl.TabPages.Add(new ConsoleTargetTabPage(PlatTarget, Properties.Settings.Default.AlwaysLog, mSendCommandToAll));
			}
			else
			{
				mMainTabControl.SelectedTab = Tab;

				if(bClearOutputWindow)
				{
					Tab.TTYText.Clear();
				}
			}

			return true;
		}

		/// <summary>
		/// Event handler.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_CONSOLE_DropDownOpening(object sender, EventArgs e)
		{
			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				IDM_DELETE_PDB.Enabled = CurTab.Target.ParentPlatform.Type == ConsoleInterface.PlatformType.Xbox360;

				if(CurTab.Target.CrashDumpType == DumpType.Normal)
				{
					IDM_DUMP_NORMAL.Checked = true;
					IDM_DUMP_WITHFULLMEM.Checked = false;
				}
				else
				{
					IDM_DUMP_NORMAL.Checked = false;
					IDM_DUMP_WITHFULLMEM.Checked = true;
				}
			}
		}

		/// <summary>
		/// Event handler.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_DELETE_PDB_Click(object sender, EventArgs e)
		{
			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				string TargetName = CurTab.Target.Name;
				string[] PDBs = Directory.GetFiles(string.Format("Temp\\{0}", TargetName), "*.pdb", SearchOption.AllDirectories);
				int NumDeleted = 0;

				foreach(string CurPDB in PDBs)
				{
					try
					{
						File.Delete(CurPDB);
						++NumDeleted;
					}
					catch(Exception ex)
					{
						System.Diagnostics.Debug.WriteLine(ex.ToString());
					}
				}

				MessageBox.Show(this, "Deleted {0} pdb's.", TargetName, MessageBoxButtons.OK, MessageBoxIcon.Information);
			}
		}

		/// <summary>
		/// Global event handler for key down messages.
		/// </summary>
		/// <param name="e">Information about the event.</param>
		protected override void OnKeyDown(KeyEventArgs e)
		{
			if(e.KeyCode == Keys.ControlKey)
			{
				mSendCommandToAll = true;

				foreach(ConsoleTargetTabPage CurTab in mMainTabControl.TabPages)
				{
					CurTab.SendCommandsToAll = mSendCommandToAll;
				}
			}

			base.OnKeyDown(e);
		}

		/// <summary>
		/// Global event handler for key up messages.
		/// </summary>
		/// <param name="e">Information about the event.</param>
		protected override void OnKeyUp(KeyEventArgs e)
		{
			if(e.KeyCode == Keys.ControlKey)
			{
				mSendCommandToAll = false;

				foreach(ConsoleTargetTabPage CurTab in mMainTabControl.TabPages)
				{
					CurTab.SendCommandsToAll = mSendCommandToAll;
				}
			}

			base.OnKeyUp(e);
		}

		/// <summary>
		/// Event handler for when the window loses focus.
		/// </summary>
		/// <param name="e">Information about the event.</param>
		protected override void OnDeactivate(EventArgs e)
		{
			base.OnDeactivate(e);

			mSendCommandToAll = false;

			foreach(ConsoleTargetTabPage CurTab in mMainTabControl.TabPages)
			{
				CurTab.SendCommandsToAll = mSendCommandToAll;
			}
		}

		/// <summary>
		/// Event handler for clearing the text of every window.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_CLEARALLWINDOWS_Click(object sender, EventArgs e)
		{
			foreach(ConsoleTargetTabPage CurTab in mMainTabControl.TabPages)
			{
				CurTab.TTYText.Clear();
			}
		}

		/// <summary>
		/// Event handler for updating the menu options for the crash report filters.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_CRASHREPORTFILTER_DropDownOpening(object sender, EventArgs e)
		{
			IDM_CRASHFILTER_DEBUG.Checked = (Properties.Settings.Default.CrashFilter & CrashReportFilter.Debug) == CrashReportFilter.Debug;
			IDM_CRASHFILTER_RELEASE.Checked = (Properties.Settings.Default.CrashFilter & CrashReportFilter.Release) == CrashReportFilter.Release;
			IDM_CRASHFILTER_RELEASEFORSHIP.Checked = (Properties.Settings.Default.CrashFilter & CrashReportFilter.ReleaseForShip) == CrashReportFilter.ReleaseForShip;
		}

		/// <summary>
		/// Event handler for selecting all crash report filters.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_CRASHFILTER_SELECTALL_Click(object sender, EventArgs e)
		{
			Properties.Settings.Default.CrashFilter = CrashReportFilter.All;
			UpdateTargetFilterStates();
		}

		/// <summary>
		/// Event handler for deselecting all crash report filters.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_CRASHFILTER_DESELECTALL_Click(object sender, EventArgs e)
		{
			Properties.Settings.Default.CrashFilter = CrashReportFilter.None;
			UpdateTargetFilterStates();
		}

		/// <summary>
		/// Event handler for toggling the debug crash report filtering state.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_CRASHFILTER_DEBUG_Click(object sender, EventArgs e)
		{
			if(IDM_CRASHFILTER_DEBUG.Checked)
			{
				Properties.Settings.Default.CrashFilter &= ~CrashReportFilter.Debug;
			}
			else
			{
				Properties.Settings.Default.CrashFilter |= CrashReportFilter.Debug;
			}

			UpdateTargetFilterStates();
		}

		/// <summary>
		/// Event handler for toggling the release crash report filtering state.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_CRASHFILTER_RELEASE_Click(object sender, EventArgs e)
		{
			if(IDM_CRASHFILTER_RELEASE.Checked)
			{
				Properties.Settings.Default.CrashFilter &= ~CrashReportFilter.Release;
			}
			else
			{
				Properties.Settings.Default.CrashFilter |= CrashReportFilter.Release;
			}

			UpdateTargetFilterStates();
		}

		/// <summary>
		/// Event handler for toggling the release for ship crash report filtering state.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_CRASHFILTER_RELEASEFORSHIP_Click(object sender, EventArgs e)
		{
			if(IDM_CRASHFILTER_RELEASEFORSHIP.Checked)
			{
				Properties.Settings.Default.CrashFilter &= ~CrashReportFilter.ReleaseForShip;
			}
			else
			{
				Properties.Settings.Default.CrashFilter |= CrashReportFilter.ReleaseForShip;
			}

			UpdateTargetFilterStates();
		}

		/// <summary>
		/// Updates the filtering state for all tab pages.
		/// </summary>
		private void UpdateTargetFilterStates()
		{
			foreach(ConsoleTargetTabPage CurTab in mMainTabControl.TabPages)
			{
				CurTab.Target.CrashFilter = Properties.Settings.Default.CrashFilter;
			}
		}

		/// <summary>
		/// Event handler for setting the dump type to Normal.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_DUMP_NORMAL_Click(object sender, EventArgs e)
		{
			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				if(!IDM_DUMP_NORMAL.Checked)
				{
					CurTab.Target.CrashDumpType = DumpType.Normal;
					CurTab.SaveDumpType();
				}
			}
		}

		/// <summary>
		/// Event handler for setting the dump type to WithFullMemory.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void IDM_DUMP_WITHFULLMEM_Click(object sender, EventArgs e)
		{
			ConsoleTargetTabPage CurTab = (ConsoleTargetTabPage)mMainTabControl.SelectedTab;

			if(CurTab != null)
			{
				if(!IDM_DUMP_WITHFULLMEM.Checked)
				{
					CurTab.Target.CrashDumpType = DumpType.WithFullMemory;
					CurTab.SaveDumpType();
				}
			}
		}

		#region IUnrealConsole Members

		/// <summary>
		/// Opens a tab for the specified target.
		/// </summary>
		/// <param name="Platform">The platform the target belongs to.</param>
		/// <param name="Target">The name, debug channel IP, or title IP of the requested target.</param>
		/// <param name="bClearOutputWindow">True if the output for the specified target is to be cleared.</param>
		/// <returns>True if the target is located.</returns>
		public bool OpenTarget(string Platform, string Target, bool bClearOutputWindow)
		{
			// marshal to the UI thread
			return (bool)this.Invoke(new OpenTargetDelegate(this.UIThreadOpenTarget), Platform, Target, bClearOutputWindow);
		}

		/// <summary>
		/// Searches for the specified target.
		/// </summary>
		/// <param name="Platform">The platform the target belongs to.</param>
		/// <param name="Target">The name, debug channel IP, or title IP of the requested target.</param>
		/// <returns>True if the target is located.</returns>
		public bool HasTarget(string Platform, string Target)
		{
			return this.Invoke(new FindTargetDelegate(UnrealConsoleWindow.FindTarget), Platform, Target) != null;
		}

		#endregion
	}
}