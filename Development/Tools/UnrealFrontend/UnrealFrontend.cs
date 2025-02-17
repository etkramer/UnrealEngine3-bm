/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


using System;
using System.Drawing;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Diagnostics;
using System.Management;
using System.Threading;
using System.IO;
using System.Runtime.InteropServices;
using System.Xml;
using System.Xml.Serialization;
using System.Text;
using System.Text.RegularExpressions;
//using CookerTools;
using Pipes;
using Microsoft.Win32;
using System.Net;
using System.Reflection;
using System.Runtime.Remoting.Channels.Ipc;
using UnrealConsoleRemoting;
using System.Runtime.Remoting.Channels;

namespace UnrealFrontend
{
	public enum Configuration
	{
		Debug = 0,
		Release,
		FinalRelease,
		FinalReleaseDebug
	}

	public enum ServerType
	{
		Listen = 0,
		Dedicated
	}

	/// <summary>
	/// Form for displaying information/interacting with the user
	/// </summary>
	public class UnrealFrontendWindow : System.Windows.Forms.Form//, CookerTools.IOutputHandler
	{
		delegate void ControlEnumeratorDelegate(Control Ctrl);
		delegate void AddLineDelegate(Color? TxtColor, string Msg, params object[] Parms);
		delegate void WriteCommandletEventDelegate(Color? TxtColor, string Msg);
		delegate void CommandletFinishedDelegate(Commandlet FinishedCommandlet);

		class PlatformTracker
		{
			int mLastPlatformIndex = -1;
			int mCurPlatformIndex = -1;

			public int LastPlatformIndex
			{
				get { return mLastPlatformIndex; }
			}

			public int CurrentPlatformIndex
			{
				set { mLastPlatformIndex = mCurPlatformIndex; mCurPlatformIndex = value; }
				get { return mCurPlatformIndex; }
			}
		}

		class CookingUserData
		{
			public CommandletAction NextAction;
			public string[] LanguageList;
			public int CurrentLanguage;

			public CookingUserData() { }

			public CookingUserData(CommandletAction NextAction, string[] LanguageList)
			{
				if(LanguageList == null)
				{
					throw new ArgumentNullException("LanguageList");
				}
				else if(LanguageList.Length == 0)
				{
					throw new ArgumentException("LanguageList must have at least 1 item!", "LanguageList");
				}

				this.LanguageList = LanguageList;
				this.NextAction = NextAction;
			}
		}

		#region Variables / enums

		// the directory where log files are stored
		private static readonly string LOG_DIR = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), "UnrealFrontend Logs\\");

		private System.ComponentModel.IContainer components;

		/** Holds all of the map extensions for the various game types. */
		private Dictionary<string, string> MapExtensionDictionary = new Dictionary<string, string>();

		/** A map from platform to targets and their associated state (true or false)*/
		private Dictionary<string, Dictionary<string, bool>> PlatformDefaults = new Dictionary<string, Dictionary<string, bool>>();

		// current version
		private const string VersionStr = "Unreal Frontend v0.9.0";

		/** Path to where to store settings files */
		private string SettingsPath;

		/** Hardcoded name for UFE settings */
		private const string SettingsFilename = "UnrealFrontend.ini";

		public const string UNI_COLOR_MAGIC = "`~[~`";

		// Holds the game name, from the combo box
		private string CachedGameName;

		// number of clients currently active
		private int NumClients = 0;
		// list of processes spawned
		private List<Process> Processes = new List<Process>();

		// Tracks platform index history
		private PlatformTracker mPlatformTracker = new PlatformTracker();

		// make a list of usable platforms
		List<string> UsablePlatforms = new List<string>();

		// used to talk to unreal console
		private IpcChannel UCIpcChannel;

		/** The running Commandlet process, for reading output and canceling.				*/
		private Commandlet CommandletProcess;

		/** A dialog for asking the name of the mod											*/
		private StringQuery ModNameDialog = new StringQuery();

		// The document used to display output.
		private UnrealControls.OutputWindowDocument mOutputDocument = new UnrealControls.OutputWindowDocument();

		// buffer for holding output
		private StringBuilder mOutputBuffer = new StringBuilder(string.Format("\r\n*********************************** Starting new log session at {0} ***********************************\r\n\r\n", DateTime.Now.ToString("H:m:s")));

		// timer for flushing the output buffer
		System.Threading.Timer mOutputTimer;

		enum ECurrentConfig
		{
			Global,
			SP,
			MP,
		};

		#endregion

		/// <summary>
		/// Gets the platform target history.
		/// </summary>
		static PlatformTargetCollection PlatformTargets
		{
			get
			{
				if(Properties.Settings.Default.PlatformTargets == null)
				{
					Properties.Settings.Default.PlatformTargets = new PlatformTargetCollection();
				}

				return Properties.Settings.Default.PlatformTargets;
			}
		}

		/// <summary>
		/// Gets/Sets the executable name override for the current platform and game.
		/// </summary>
		string ExecutableNameOverride
		{
			get
			{
				if(Properties.Settings.Default.Game_ExecutableNameOverride == null)
				{
					Properties.Settings.Default.Game_ExecutableNameOverride = new PlatformGameMapCollection();

					return string.Empty;
				}

				string OverridenName = string.Empty;

				Properties.Settings.Default.Game_ExecutableNameOverride.TryGetMap(ComboBox_Platform.Text, ComboBox_Game.Text, out OverridenName);

				return OverridenName;
			}
			set
			{
				if(value != null)
				{
					if(Properties.Settings.Default.Game_ExecutableNameOverride == null)
					{
						Properties.Settings.Default.Game_ExecutableNameOverride = new PlatformGameMapCollection();
					}

					Properties.Settings.Default.Game_ExecutableNameOverride.SetMap(ComboBox_Platform.Text, ComboBox_Game.Text, value);
				}
			}
		}

		/// <summary>
		/// Gets the executable override state for the current platform and game.
		/// </summary>
		bool OverrideExecutable
		{
			get
			{
				if(Properties.Settings.Default.Game_OverrideExecutable == null)
				{
					Properties.Settings.Default.Game_OverrideExecutable = new PlatformGameBoolCollection();

					return false;
				}

				bool bOverrideExecutable = false;

				Properties.Settings.Default.Game_OverrideExecutable.TryGetValue(ComboBox_Platform.Text, ComboBox_Game.Text, out bOverrideExecutable);

				return bOverrideExecutable;
			}
			set
			{
				if(Properties.Settings.Default.Game_OverrideExecutable == null)
				{
					Properties.Settings.Default.Game_OverrideExecutable = new PlatformGameBoolCollection();
				}

				Properties.Settings.Default.Game_OverrideExecutable.SetValue(ComboBox_Platform.Text, ComboBox_Game.Text, value);
			}
		}

		#region Windows Forms variables

		private System.Windows.Forms.ToolTip FormToolTips;
		private ToolStripLabel ToolStripLabel_Platform;
		private ToolStripComboBox ComboBox_Platform;
		private ToolStripSeparator ToolStripSeparator1;
		private ToolStripLabel ToolStripLabel_Game;
		private ToolStripComboBox ComboBox_Game;
		private ToolStripSeparator ToolStripSeparator2;
		private ToolTip CookMapToolTip;
		private MenuStrip menuStrip1;
		private ToolStripMenuItem MenuMain_File;
		private ToolStripMenuItem MenuMain_File_Exit;
		private ToolStripMenuItem MenuMain_Edit;
		private ToolStripMenuItem MenuMain_Edit_CopyToClipboard;
		private ToolStripMenuItem MenuMain_Edit_CopyToClipboard_SPURL;
		private ToolStripMenuItem MenuMain_Edit_CopyToClipboard_MPURL;
		private ToolStripMenuItem MenuMain_Edit_CopyToClipboard_Compile;
		private ToolStripMenuItem MenuMain_Edit_CopyToClipboard_Cook;
		private ToolStripLabel ToolStripLabel_Config;
		private ToolStripComboBox ComboBox_Configuration;
		private ToolStripStatusLabel toolStripStatusLabel3;
		private ToolStripStatusLabel ToolStripStatusLabel_LocalMap;
		private StatusStrip statusStrip1;
		private ToolStrip ToolStrip_GlobalSettings;
		private ToolStripContainer toolStripContainer1;
		private SplitContainer splitContainer1;
		private TabControl TabControl_Main;
		private TabPage Tab_Game;
		private CheckBox CheckBox_Game_OverrideExecutable;
		private TextBox TextBox_Game_ExecutableNameOverride;
		private Label label11;
		private ComboBox ComboBox_MapToPlay;
		private GroupBox GroupBox_Game_PC;
		private ComboBox ComboBox_Game_Resolution;
		private CheckBox CheckBox_Game_RemoteControl;
		private CheckBox CheckBox_Game_ShowLog;
		private Label label7;
		private Label label6;
		private Label label5;
		private ComboBox ComboBox_Game_ServerType;
		private NumericUpDown NumericUpDown_Game_NumClients;
		private GroupBox GroupBox_Game_Common;
		private ComboBox ComboBox_Game_ExtraOptions;
		private CheckBox CheckBox_Game_CaptureFPSChartInfo;
		private TextBox TextBox_Game_SentinelTag;
		private Label label17;
		private Label label16;
		private CheckBox CheckBox_Game_SentinelRun;
		private ComboBox ComboBox_Game_SentinelType;
		private TextBox TextBox_Game_ExecCommands;
		private Label label4;
		private Label label2;
		private CheckBox CheckBox_Game_NoVSync;
		private CheckBox CheckBox_Game_MultiThreaded;
		private CheckBox CheckBox_Game_NoSound;
		private Button ButtonBrowseMap;
		private CheckBox CheckBox_Game_UseCookedMap;
		private Label label1;
		private TabPage Tab_Cooking;
		private ComboBox ComboBox_Cooking_MapsToCook;
		private GroupBox GroupBox_Cooking_Game_Options;
		private CheckBox CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS;
		private CheckBox CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes;
		private GroupBox GroupBox_Cooking_Options;
		private CheckBox CheckBox_Cooking_DisablePackageCompression;
		private CheckBox CheckBox_Cooking_CookMod;
		private CheckBox CheckBox_Cooking_RunLocalGame;
		private CheckBox CheckBox_Cooking_ForceSoundRecook;
		private CheckBox CheckBox_Cooking_CookFinalReleaseScript;
		private Label label10;
		private CheckBox CheckBox_Cooking_ForceSeekFreeRecook;
		private Button Button_ImportMapList;
		private Label label3;
		private Label label18;
		private TabPage Tab_ConsoleSetup;
		private ListView ListBox_Targets;
		private ColumnHeader ListBox_Targets_Name;
		private ColumnHeader ListBox_Targets_IP;
		private ColumnHeader ListBox_Targets_DebugIP;
		private ColumnHeader ListBox_Targets_Type;
		private TabPage Tab_Compiling;
		private TabPage Tab_Help;
		private RichTextBox RichTextBox_Help;
		private ToolStrip ToolStrip_Actions;
		private ToolStripButton ToolStripButton_Launch;
		private ToolStripSeparator ToolStripSeparator7;
		private ToolStripButton ToolStripButton_Editor;
		private ToolStripSeparator ToolStripSeparator3;
		private ToolStripSplitButton ToolStripButton_Server;
		private ToolStripMenuItem ToolStripButton_AddClient;
		private ToolStripMenuItem ToolStripButton_KillAll;
		private ToolStripSeparator ToolStripSeparator4;
		private ToolStripSeparator ToolStripSeparator5;
		private ToolStripSeparator ToolStripSeparator6;
		private ToolStripButton ToolStripButton_Sync;
		private ToolStripSeparator ToolStripSeparator8;
		private ToolStripButton ToolStripButton_Reboot;
		private ToolStripSeparator ToolStripSeparator10;
		private ToolStripLabel toolStripLabel1;
		private ToolStripSeparator toolStripSeparator11;
		private ToolStripButton ToolStripButton_UnrealProp;
		private ToolStripSeparator toolStripSeparator9;
		private ToolStripLabel toolStripLabel2;
		private ToolStripComboBox ComboBox_CookingConfiguration;
		private System.Windows.Forms.Timer Cooking_MapToCookToolTipTimer;
		private CheckBox CheckBox_Game_ClearUCWindow;
		private ToolStripSplitButton ToolStripButton_Cook;
		private ToolStripMenuItem ToolStripButton_CookGlobalShaders;
		private CheckBox CheckBox_ConsoleTargets_ShowAllInfo;
		private CheckBox CheckBox_ConsoleTargets_CopyDebugInfo;
		private Label label9;
		private CheckBox CheckBox_ConsoleTargets_RebootBeforeCopy;
		private GroupBox GroupBox_Game_GoW2;
		private CheckBox CheckBox_Game_GoW2OverrideSPMap;
		private ComboBox ComboBox_Game_GoW2SPMapName;
		private Label label14;
		private ToolStripButton ToolStripButton_CompileScript;
		private CheckBox CheckBox_Compiling_AutoMode;
		private CheckBox CheckBox_Compiling_VerboseMode;
		private CheckBox CheckBox_Compiling_RunWithCookedData;
		private CheckBox CheckBox_Compiling_FullRecompile;
		private CheckBox CheckBox_Cooking_SkipScriptCompile;
		private ToolStripMenuItem ToolStripButton_FullRecook;
		private ToolStripMenuItem ToolStripButton_CookIntsOnly;
		private ToolStripMenuItem ToolStripButton_CookAllMaps;
		private ComboBox ComboBox_Cooking_AdditionalOptions;
		private UnrealControls.OutputWindowView OutputWindowView_LogWindow;
		private ListView ListView_Cooking_Languages;
		private ColumnHeader ListItem_Cooking_LangName;
		private GroupBox GroupBox_Game_Xbox360;
		private CheckBox CheckBox_Game_EncryptedSockets;
		private NotifyIcon NotifyIcon_CommandletFinished;
		private CheckBox CheckBox_Cooking_ShowBalloon;
		private ComboBox ComboBox_ConsoleTargets_ConsoleBaseDir;
		#endregion

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(UnrealFrontendWindow));
			System.Windows.Forms.ListViewItem listViewItem1 = new System.Windows.Forms.ListViewItem("INT");
			System.Windows.Forms.ListViewItem listViewItem2 = new System.Windows.Forms.ListViewItem("JPN");
			System.Windows.Forms.ListViewItem listViewItem3 = new System.Windows.Forms.ListViewItem("DEU");
			System.Windows.Forms.ListViewItem listViewItem4 = new System.Windows.Forms.ListViewItem("FRA");
			System.Windows.Forms.ListViewItem listViewItem5 = new System.Windows.Forms.ListViewItem("ESM");
			System.Windows.Forms.ListViewItem listViewItem6 = new System.Windows.Forms.ListViewItem("ESN");
			System.Windows.Forms.ListViewItem listViewItem7 = new System.Windows.Forms.ListViewItem("ITA");
			System.Windows.Forms.ListViewItem listViewItem8 = new System.Windows.Forms.ListViewItem("KOR");
			System.Windows.Forms.ListViewItem listViewItem9 = new System.Windows.Forms.ListViewItem("CHT");
			System.Windows.Forms.ListViewItem listViewItem10 = new System.Windows.Forms.ListViewItem("RUS");
			System.Windows.Forms.ListViewItem listViewItem11 = new System.Windows.Forms.ListViewItem("POL");
			System.Windows.Forms.ListViewItem listViewItem12 = new System.Windows.Forms.ListViewItem("HUN");
			System.Windows.Forms.ListViewItem listViewItem13 = new System.Windows.Forms.ListViewItem("CZE");
			System.Windows.Forms.ListViewItem listViewItem14 = new System.Windows.Forms.ListViewItem("SLO");
			UnrealControls.OutputWindowDocument outputWindowDocument1 = new UnrealControls.OutputWindowDocument();
			this.FormToolTips = new System.Windows.Forms.ToolTip(this.components);
			this.CheckBox_Game_EncryptedSockets = new System.Windows.Forms.CheckBox();
			this.CheckBox_Game_RemoteControl = new System.Windows.Forms.CheckBox();
			this.CheckBox_Game_ShowLog = new System.Windows.Forms.CheckBox();
			this.ComboBox_Game_ServerType = new System.Windows.Forms.ComboBox();
			this.CheckBox_Game_ClearUCWindow = new System.Windows.Forms.CheckBox();
			this.CheckBox_Game_CaptureFPSChartInfo = new System.Windows.Forms.CheckBox();
			this.TextBox_Game_SentinelTag = new System.Windows.Forms.TextBox();
			this.CheckBox_Game_SentinelRun = new System.Windows.Forms.CheckBox();
			this.NumericUpDown_Game_NumClients = new System.Windows.Forms.NumericUpDown();
			this.ComboBox_Game_SentinelType = new System.Windows.Forms.ComboBox();
			this.TextBox_Game_ExecCommands = new System.Windows.Forms.TextBox();
			this.CheckBox_Game_NoVSync = new System.Windows.Forms.CheckBox();
			this.CheckBox_Game_MultiThreaded = new System.Windows.Forms.CheckBox();
			this.CheckBox_Game_NoSound = new System.Windows.Forms.CheckBox();
			this.CheckBox_Game_UseCookedMap = new System.Windows.Forms.CheckBox();
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS = new System.Windows.Forms.CheckBox();
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes = new System.Windows.Forms.CheckBox();
			this.CheckBox_Cooking_SkipScriptCompile = new System.Windows.Forms.CheckBox();
			this.CheckBox_Cooking_CookMod = new System.Windows.Forms.CheckBox();
			this.CheckBox_Cooking_RunLocalGame = new System.Windows.Forms.CheckBox();
			this.CheckBox_Cooking_ForceSoundRecook = new System.Windows.Forms.CheckBox();
			this.CheckBox_Cooking_CookFinalReleaseScript = new System.Windows.Forms.CheckBox();
			this.CheckBox_Cooking_ForceSeekFreeRecook = new System.Windows.Forms.CheckBox();
			this.CheckBox_ConsoleTargets_CopyDebugInfo = new System.Windows.Forms.CheckBox();
			this.CheckBox_ConsoleTargets_RebootBeforeCopy = new System.Windows.Forms.CheckBox();
			this.CheckBox_ConsoleTargets_ShowAllInfo = new System.Windows.Forms.CheckBox();
			this.CheckBox_Compiling_AutoMode = new System.Windows.Forms.CheckBox();
			this.CheckBox_Compiling_VerboseMode = new System.Windows.Forms.CheckBox();
			this.CheckBox_Compiling_RunWithCookedData = new System.Windows.Forms.CheckBox();
			this.CheckBox_Compiling_FullRecompile = new System.Windows.Forms.CheckBox();
			this.ToolStrip_GlobalSettings = new System.Windows.Forms.ToolStrip();
			this.ToolStripLabel_Platform = new System.Windows.Forms.ToolStripLabel();
			this.ComboBox_Platform = new System.Windows.Forms.ToolStripComboBox();
			this.ToolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripLabel_Game = new System.Windows.Forms.ToolStripLabel();
			this.ComboBox_Game = new System.Windows.Forms.ToolStripComboBox();
			this.ToolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripLabel_Config = new System.Windows.Forms.ToolStripLabel();
			this.ComboBox_Configuration = new System.Windows.Forms.ToolStripComboBox();
			this.toolStripSeparator9 = new System.Windows.Forms.ToolStripSeparator();
			this.toolStripLabel2 = new System.Windows.Forms.ToolStripLabel();
			this.ComboBox_CookingConfiguration = new System.Windows.Forms.ToolStripComboBox();
			this.CookMapToolTip = new System.Windows.Forms.ToolTip(this.components);
			this.menuStrip1 = new System.Windows.Forms.MenuStrip();
			this.MenuMain_File = new System.Windows.Forms.ToolStripMenuItem();
			this.MenuMain_File_Exit = new System.Windows.Forms.ToolStripMenuItem();
			this.MenuMain_Edit = new System.Windows.Forms.ToolStripMenuItem();
			this.MenuMain_Edit_CopyToClipboard = new System.Windows.Forms.ToolStripMenuItem();
			this.MenuMain_Edit_CopyToClipboard_SPURL = new System.Windows.Forms.ToolStripMenuItem();
			this.MenuMain_Edit_CopyToClipboard_MPURL = new System.Windows.Forms.ToolStripMenuItem();
			this.MenuMain_Edit_CopyToClipboard_Compile = new System.Windows.Forms.ToolStripMenuItem();
			this.MenuMain_Edit_CopyToClipboard_Cook = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripStatusLabel3 = new System.Windows.Forms.ToolStripStatusLabel();
			this.ToolStripStatusLabel_LocalMap = new System.Windows.Forms.ToolStripStatusLabel();
			this.statusStrip1 = new System.Windows.Forms.StatusStrip();
			this.toolStripContainer1 = new System.Windows.Forms.ToolStripContainer();
			this.splitContainer1 = new System.Windows.Forms.SplitContainer();
			this.TabControl_Main = new System.Windows.Forms.TabControl();
			this.Tab_Game = new System.Windows.Forms.TabPage();
			this.GroupBox_Game_Xbox360 = new System.Windows.Forms.GroupBox();
			this.GroupBox_Game_GoW2 = new System.Windows.Forms.GroupBox();
			this.CheckBox_Game_GoW2OverrideSPMap = new System.Windows.Forms.CheckBox();
			this.ComboBox_Game_GoW2SPMapName = new System.Windows.Forms.ComboBox();
			this.label14 = new System.Windows.Forms.Label();
			this.CheckBox_Game_OverrideExecutable = new System.Windows.Forms.CheckBox();
			this.TextBox_Game_ExecutableNameOverride = new System.Windows.Forms.TextBox();
			this.label11 = new System.Windows.Forms.Label();
			this.ComboBox_MapToPlay = new System.Windows.Forms.ComboBox();
			this.GroupBox_Game_PC = new System.Windows.Forms.GroupBox();
			this.ComboBox_Game_Resolution = new System.Windows.Forms.ComboBox();
			this.label7 = new System.Windows.Forms.Label();
			this.label6 = new System.Windows.Forms.Label();
			this.GroupBox_Game_Common = new System.Windows.Forms.GroupBox();
			this.ComboBox_Game_ExtraOptions = new System.Windows.Forms.ComboBox();
			this.label17 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.label16 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.ButtonBrowseMap = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.Tab_Cooking = new System.Windows.Forms.TabPage();
			this.ComboBox_Cooking_AdditionalOptions = new System.Windows.Forms.ComboBox();
			this.ComboBox_Cooking_MapsToCook = new System.Windows.Forms.ComboBox();
			this.GroupBox_Cooking_Game_Options = new System.Windows.Forms.GroupBox();
			this.GroupBox_Cooking_Options = new System.Windows.Forms.GroupBox();
			this.CheckBox_Cooking_ShowBalloon = new System.Windows.Forms.CheckBox();
			this.ListView_Cooking_Languages = new System.Windows.Forms.ListView();
			this.ListItem_Cooking_LangName = new System.Windows.Forms.ColumnHeader();
			this.CheckBox_Cooking_DisablePackageCompression = new System.Windows.Forms.CheckBox();
			this.label10 = new System.Windows.Forms.Label();
			this.Button_ImportMapList = new System.Windows.Forms.Button();
			this.label3 = new System.Windows.Forms.Label();
			this.label18 = new System.Windows.Forms.Label();
			this.Tab_ConsoleSetup = new System.Windows.Forms.TabPage();
			this.ComboBox_ConsoleTargets_ConsoleBaseDir = new System.Windows.Forms.ComboBox();
			this.label9 = new System.Windows.Forms.Label();
			this.ListBox_Targets = new System.Windows.Forms.ListView();
			this.ListBox_Targets_Name = new System.Windows.Forms.ColumnHeader();
			this.ListBox_Targets_IP = new System.Windows.Forms.ColumnHeader();
			this.ListBox_Targets_DebugIP = new System.Windows.Forms.ColumnHeader();
			this.ListBox_Targets_Type = new System.Windows.Forms.ColumnHeader();
			this.Tab_Compiling = new System.Windows.Forms.TabPage();
			this.Tab_Help = new System.Windows.Forms.TabPage();
			this.RichTextBox_Help = new System.Windows.Forms.RichTextBox();
			this.OutputWindowView_LogWindow = new UnrealControls.OutputWindowView();
			this.ToolStrip_Actions = new System.Windows.Forms.ToolStrip();
			this.ToolStripButton_Launch = new System.Windows.Forms.ToolStripButton();
			this.ToolStripSeparator7 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_Editor = new System.Windows.Forms.ToolStripButton();
			this.ToolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_Server = new System.Windows.Forms.ToolStripSplitButton();
			this.ToolStripButton_AddClient = new System.Windows.Forms.ToolStripMenuItem();
			this.ToolStripButton_KillAll = new System.Windows.Forms.ToolStripMenuItem();
			this.ToolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_Cook = new System.Windows.Forms.ToolStripSplitButton();
			this.ToolStripButton_CookGlobalShaders = new System.Windows.Forms.ToolStripMenuItem();
			this.ToolStripButton_FullRecook = new System.Windows.Forms.ToolStripMenuItem();
			this.ToolStripButton_CookIntsOnly = new System.Windows.Forms.ToolStripMenuItem();
			this.ToolStripButton_CookAllMaps = new System.Windows.Forms.ToolStripMenuItem();
			this.ToolStripSeparator5 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_CompileScript = new System.Windows.Forms.ToolStripButton();
			this.ToolStripSeparator6 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_Sync = new System.Windows.Forms.ToolStripButton();
			this.ToolStripSeparator8 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_Reboot = new System.Windows.Forms.ToolStripButton();
			this.ToolStripSeparator10 = new System.Windows.Forms.ToolStripSeparator();
			this.toolStripLabel1 = new System.Windows.Forms.ToolStripLabel();
			this.toolStripSeparator11 = new System.Windows.Forms.ToolStripSeparator();
			this.ToolStripButton_UnrealProp = new System.Windows.Forms.ToolStripButton();
			this.Cooking_MapToCookToolTipTimer = new System.Windows.Forms.Timer(this.components);
			this.NotifyIcon_CommandletFinished = new System.Windows.Forms.NotifyIcon(this.components);
			((System.ComponentModel.ISupportInitialize)(this.NumericUpDown_Game_NumClients)).BeginInit();
			this.ToolStrip_GlobalSettings.SuspendLayout();
			this.menuStrip1.SuspendLayout();
			this.statusStrip1.SuspendLayout();
			this.toolStripContainer1.ContentPanel.SuspendLayout();
			this.toolStripContainer1.TopToolStripPanel.SuspendLayout();
			this.toolStripContainer1.SuspendLayout();
			this.splitContainer1.Panel1.SuspendLayout();
			this.splitContainer1.Panel2.SuspendLayout();
			this.splitContainer1.SuspendLayout();
			this.TabControl_Main.SuspendLayout();
			this.Tab_Game.SuspendLayout();
			this.GroupBox_Game_Xbox360.SuspendLayout();
			this.GroupBox_Game_GoW2.SuspendLayout();
			this.GroupBox_Game_PC.SuspendLayout();
			this.GroupBox_Game_Common.SuspendLayout();
			this.Tab_Cooking.SuspendLayout();
			this.GroupBox_Cooking_Game_Options.SuspendLayout();
			this.GroupBox_Cooking_Options.SuspendLayout();
			this.Tab_ConsoleSetup.SuspendLayout();
			this.Tab_Compiling.SuspendLayout();
			this.Tab_Help.SuspendLayout();
			this.ToolStrip_Actions.SuspendLayout();
			this.SuspendLayout();
			// 
			// FormToolTips
			// 
			this.FormToolTips.AutoPopDelay = 7000;
			this.FormToolTips.InitialDelay = 900;
			this.FormToolTips.ReshowDelay = 100;
			// 
			// CheckBox_Game_EncryptedSockets
			// 
			this.CheckBox_Game_EncryptedSockets.AutoSize = true;
			this.CheckBox_Game_EncryptedSockets.Checked = true;
			this.CheckBox_Game_EncryptedSockets.CheckState = System.Windows.Forms.CheckState.Checked;
			this.CheckBox_Game_EncryptedSockets.Location = new System.Drawing.Point(10, 19);
			this.CheckBox_Game_EncryptedSockets.Name = "CheckBox_Game_EncryptedSockets";
			this.CheckBox_Game_EncryptedSockets.Size = new System.Drawing.Size(138, 17);
			this.CheckBox_Game_EncryptedSockets.TabIndex = 0;
			this.CheckBox_Game_EncryptedSockets.Text = "Use Encrypted Sockets";
			this.FormToolTips.SetToolTip(this.CheckBox_Game_EncryptedSockets, "If unchecked then socket communications are not encrypted (-devcon).");
			this.CheckBox_Game_EncryptedSockets.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Game_RemoteControl
			// 
			this.CheckBox_Game_RemoteControl.Location = new System.Drawing.Point(9, 131);
			this.CheckBox_Game_RemoteControl.Name = "CheckBox_Game_RemoteControl";
			this.CheckBox_Game_RemoteControl.Size = new System.Drawing.Size(121, 17);
			this.CheckBox_Game_RemoteControl.TabIndex = 0;
			this.CheckBox_Game_RemoteControl.Text = "Remote Control";
			this.FormToolTips.SetToolTip(this.CheckBox_Game_RemoteControl, "On PC, controls whether or not the Remote Control side window is displayed [-norc" +
					"].");
			this.CheckBox_Game_RemoteControl.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Game_ShowLog
			// 
			this.CheckBox_Game_ShowLog.AutoSize = true;
			this.CheckBox_Game_ShowLog.Location = new System.Drawing.Point(9, 108);
			this.CheckBox_Game_ShowLog.Name = "CheckBox_Game_ShowLog";
			this.CheckBox_Game_ShowLog.Size = new System.Drawing.Size(74, 17);
			this.CheckBox_Game_ShowLog.TabIndex = 6;
			this.CheckBox_Game_ShowLog.Text = "Show Log";
			this.FormToolTips.SetToolTip(this.CheckBox_Game_ShowLog, "On PC, this will open a log window when the game is launched [-log].");
			this.CheckBox_Game_ShowLog.UseVisualStyleBackColor = true;
			// 
			// ComboBox_Game_ServerType
			// 
			this.ComboBox_Game_ServerType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_Game_ServerType.FormattingEnabled = true;
			this.ComboBox_Game_ServerType.Location = new System.Drawing.Point(9, 33);
			this.ComboBox_Game_ServerType.Name = "ComboBox_Game_ServerType";
			this.ComboBox_Game_ServerType.Size = new System.Drawing.Size(121, 21);
			this.ComboBox_Game_ServerType.TabIndex = 1;
			this.FormToolTips.SetToolTip(this.ComboBox_Game_ServerType, "Sets the type of server that will be created when a multiplayer server session is" +
					" started.");
			// 
			// CheckBox_Game_ClearUCWindow
			// 
			this.CheckBox_Game_ClearUCWindow.AutoSize = true;
			this.CheckBox_Game_ClearUCWindow.Location = new System.Drawing.Point(9, 233);
			this.CheckBox_Game_ClearUCWindow.Name = "CheckBox_Game_ClearUCWindow";
			this.CheckBox_Game_ClearUCWindow.Size = new System.Drawing.Size(164, 17);
			this.CheckBox_Game_ClearUCWindow.TabIndex = 16;
			this.CheckBox_Game_ClearUCWindow.Text = "Clear UnrealConsole Window";
			this.FormToolTips.SetToolTip(this.CheckBox_Game_ClearUCWindow, "Clears the unreal console window associated with the targets being launched if ch" +
					"ecked.");
			this.CheckBox_Game_ClearUCWindow.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Game_CaptureFPSChartInfo
			// 
			this.CheckBox_Game_CaptureFPSChartInfo.AutoSize = true;
			this.CheckBox_Game_CaptureFPSChartInfo.Location = new System.Drawing.Point(113, 164);
			this.CheckBox_Game_CaptureFPSChartInfo.Name = "CheckBox_Game_CaptureFPSChartInfo";
			this.CheckBox_Game_CaptureFPSChartInfo.Size = new System.Drawing.Size(126, 17);
			this.CheckBox_Game_CaptureFPSChartInfo.TabIndex = 14;
			this.CheckBox_Game_CaptureFPSChartInfo.Text = "CaptureFPSChartInfo";
			this.FormToolTips.SetToolTip(this.CheckBox_Game_CaptureFPSChartInfo, "Controls whether or not to capture FPS buckets [?CaptureFPSChartInfo=1].");
			this.CheckBox_Game_CaptureFPSChartInfo.UseVisualStyleBackColor = true;
			// 
			// TextBox_Game_SentinelTag
			// 
			this.TextBox_Game_SentinelTag.Location = new System.Drawing.Point(267, 210);
			this.TextBox_Game_SentinelTag.Name = "TextBox_Game_SentinelTag";
			this.TextBox_Game_SentinelTag.Size = new System.Drawing.Size(125, 20);
			this.TextBox_Game_SentinelTag.TabIndex = 13;
			this.FormToolTips.SetToolTip(this.TextBox_Game_SentinelTag, "Sentinel tag which could be something like:  ParticleTest  or  NewMeshTest or Des" +
					"ignerPlayTest");
			// 
			// CheckBox_Game_SentinelRun
			// 
			this.CheckBox_Game_SentinelRun.AutoSize = true;
			this.CheckBox_Game_SentinelRun.Location = new System.Drawing.Point(245, 164);
			this.CheckBox_Game_SentinelRun.Name = "CheckBox_Game_SentinelRun";
			this.CheckBox_Game_SentinelRun.Size = new System.Drawing.Size(87, 17);
			this.CheckBox_Game_SentinelRun.TabIndex = 11;
			this.CheckBox_Game_SentinelRun.Text = "Sentinel Run";
			this.FormToolTips.SetToolTip(this.CheckBox_Game_SentinelRun, "Controls whether or not this run is going to be a SentinelRun [?DoingASentinelRun" +
					"=1].");
			this.CheckBox_Game_SentinelRun.UseVisualStyleBackColor = true;
			// 
			// NumericUpDown_Game_NumClients
			// 
			this.NumericUpDown_Game_NumClients.Location = new System.Drawing.Point(10, 280);
			this.NumericUpDown_Game_NumClients.Name = "NumericUpDown_Game_NumClients";
			this.NumericUpDown_Game_NumClients.Size = new System.Drawing.Size(120, 20);
			this.NumericUpDown_Game_NumClients.TabIndex = 0;
			this.FormToolTips.SetToolTip(this.NumericUpDown_Game_NumClients, "The number of clients to spawn when \'Server + n clients\' is clicked.");
			// 
			// ComboBox_Game_SentinelType
			// 
			this.ComboBox_Game_SentinelType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_Game_SentinelType.FormattingEnabled = true;
			this.ComboBox_Game_SentinelType.Items.AddRange(new object[] {
            "BotMatch",
            "BVT",
            "FlyThrough",
            "FlyThroughSplitScreen",
            "ManualPerf",
            "Playtest",
            "TestingAtDesk",
            "TravelTheWorld"});
			this.ComboBox_Game_SentinelType.Location = new System.Drawing.Point(126, 207);
			this.ComboBox_Game_SentinelType.Name = "ComboBox_Game_SentinelType";
			this.ComboBox_Game_SentinelType.Size = new System.Drawing.Size(121, 21);
			this.ComboBox_Game_SentinelType.Sorted = true;
			this.ComboBox_Game_SentinelType.TabIndex = 8;
			this.FormToolTips.SetToolTip(this.ComboBox_Game_SentinelType, "The type of Sentinel Run to do");
			// 
			// TextBox_Game_ExecCommands
			// 
			this.TextBox_Game_ExecCommands.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.TextBox_Game_ExecCommands.Location = new System.Drawing.Point(9, 85);
			this.TextBox_Game_ExecCommands.Multiline = true;
			this.TextBox_Game_ExecCommands.Name = "TextBox_Game_ExecCommands";
			this.TextBox_Game_ExecCommands.Size = new System.Drawing.Size(656, 73);
			this.TextBox_Game_ExecCommands.TabIndex = 10;
			this.FormToolTips.SetToolTip(this.TextBox_Game_ExecCommands, "Enter commands to be run in game on the first frame of gameplay [-exec=].");
			// 
			// CheckBox_Game_NoVSync
			// 
			this.CheckBox_Game_NoVSync.AutoSize = true;
			this.CheckBox_Game_NoVSync.Location = new System.Drawing.Point(9, 187);
			this.CheckBox_Game_NoVSync.Name = "CheckBox_Game_NoVSync";
			this.CheckBox_Game_NoVSync.Size = new System.Drawing.Size(74, 17);
			this.CheckBox_Game_NoVSync.TabIndex = 6;
			this.CheckBox_Game_NoVSync.Text = "No VSync";
			this.FormToolTips.SetToolTip(this.CheckBox_Game_NoVSync, "Controls whether or not VSyncing is disabled (no VSync will have a smoother frame" +
					"rate but the screen will tear) [-novsync].");
			this.CheckBox_Game_NoVSync.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Game_MultiThreaded
			// 
			this.CheckBox_Game_MultiThreaded.AutoSize = true;
			this.CheckBox_Game_MultiThreaded.Checked = true;
			this.CheckBox_Game_MultiThreaded.CheckState = System.Windows.Forms.CheckState.Checked;
			this.CheckBox_Game_MultiThreaded.Location = new System.Drawing.Point(9, 210);
			this.CheckBox_Game_MultiThreaded.Name = "CheckBox_Game_MultiThreaded";
			this.CheckBox_Game_MultiThreaded.Size = new System.Drawing.Size(93, 17);
			this.CheckBox_Game_MultiThreaded.TabIndex = 1;
			this.CheckBox_Game_MultiThreaded.Text = "Multi-threaded";
			this.FormToolTips.SetToolTip(this.CheckBox_Game_MultiThreaded, "Controls whether or not the rendering thread is disabled [-onethread].");
			this.CheckBox_Game_MultiThreaded.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Game_NoSound
			// 
			this.CheckBox_Game_NoSound.AutoSize = true;
			this.CheckBox_Game_NoSound.Location = new System.Drawing.Point(9, 164);
			this.CheckBox_Game_NoSound.Name = "CheckBox_Game_NoSound";
			this.CheckBox_Game_NoSound.Size = new System.Drawing.Size(74, 17);
			this.CheckBox_Game_NoSound.TabIndex = 0;
			this.CheckBox_Game_NoSound.Text = "No Sound";
			this.FormToolTips.SetToolTip(this.CheckBox_Game_NoSound, "Controls whether or not sound playback is disabled [-nosound].");
			this.CheckBox_Game_NoSound.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Game_UseCookedMap
			// 
			this.CheckBox_Game_UseCookedMap.AutoSize = true;
			this.CheckBox_Game_UseCookedMap.ForeColor = System.Drawing.SystemColors.ControlText;
			this.CheckBox_Game_UseCookedMap.Location = new System.Drawing.Point(6, 45);
			this.CheckBox_Game_UseCookedMap.Name = "CheckBox_Game_UseCookedMap";
			this.CheckBox_Game_UseCookedMap.Size = new System.Drawing.Size(113, 17);
			this.CheckBox_Game_UseCookedMap.TabIndex = 36;
			this.CheckBox_Game_UseCookedMap.Text = "(Use cooked map)";
			this.FormToolTips.SetToolTip(this.CheckBox_Game_UseCookedMap, "Uses the \"Maps\" string on the Cooking tab as the map.\nIf cooking multiple maps, t" +
					"he game should use the first map in the list and ignore the others.");
			this.CheckBox_Game_UseCookedMap.CheckedChanged += new System.EventHandler(this.OnUseCookedMapChecked);
			// 
			// CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS
			// 
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS.AutoSize = true;
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS.Location = new System.Drawing.Point(9, 42);
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS.Name = "CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS";
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS.Size = new System.Drawing.Size(210, 17);
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS.TabIndex = 1;
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS.Text = "Force Seek Free Recook GUDS Audio";
			this.FormToolTips.SetToolTip(this.CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS, "When this is selected, seekfree packages generated for GUDS audio will be recooke" +
					"d, even if the cooker thinks they are up to date. [-recookseekfreeguds].");
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes
			// 
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes.AutoSize = true;
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes.Location = new System.Drawing.Point(9, 19);
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes.Name = "CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes";
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes.Size = new System.Drawing.Size(209, 17);
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes.TabIndex = 0;
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes.Text = "Force Seek Free Recook Game Types";
			this.FormToolTips.SetToolTip(this.CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes, "When this is selected, seekfree packages generated for game types will be recooke" +
					"d, even if the cooker thinks they are up to date. [-recookseekfreegametypes].");
			this.CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Cooking_SkipScriptCompile
			// 
			this.CheckBox_Cooking_SkipScriptCompile.AutoSize = true;
			this.CheckBox_Cooking_SkipScriptCompile.Location = new System.Drawing.Point(133, 170);
			this.CheckBox_Cooking_SkipScriptCompile.Name = "CheckBox_Cooking_SkipScriptCompile";
			this.CheckBox_Cooking_SkipScriptCompile.Size = new System.Drawing.Size(184, 17);
			this.CheckBox_Cooking_SkipScriptCompile.TabIndex = 11;
			this.CheckBox_Cooking_SkipScriptCompile.Text = "Skip Automatic Script Compilation";
			this.FormToolTips.SetToolTip(this.CheckBox_Cooking_SkipScriptCompile, "Prevents the cooker from checking if scripts need to be recompiled.");
			this.CheckBox_Cooking_SkipScriptCompile.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Cooking_CookMod
			// 
			this.CheckBox_Cooking_CookMod.AutoSize = true;
			this.CheckBox_Cooking_CookMod.Location = new System.Drawing.Point(133, 124);
			this.CheckBox_Cooking_CookMod.Name = "CheckBox_Cooking_CookMod";
			this.CheckBox_Cooking_CookMod.Size = new System.Drawing.Size(180, 17);
			this.CheckBox_Cooking_CookMod.TabIndex = 9;
			this.CheckBox_Cooking_CookMod.Text = "Cook Mod/With Seek Free Data";
			this.FormToolTips.SetToolTip(this.CheckBox_Cooking_CookMod, "Controls whether or not the cooker will use seek free data.\r\nRequired for mods.");
			this.CheckBox_Cooking_CookMod.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Cooking_RunLocalGame
			// 
			this.CheckBox_Cooking_RunLocalGame.AutoSize = true;
			this.CheckBox_Cooking_RunLocalGame.Location = new System.Drawing.Point(133, 101);
			this.CheckBox_Cooking_RunLocalGame.Name = "CheckBox_Cooking_RunLocalGame";
			this.CheckBox_Cooking_RunLocalGame.Size = new System.Drawing.Size(188, 17);
			this.CheckBox_Cooking_RunLocalGame.TabIndex = 8;
			this.CheckBox_Cooking_RunLocalGame.Text = "Run Local Game After Cook/Sync";
			this.FormToolTips.SetToolTip(this.CheckBox_Cooking_RunLocalGame, "After cooking and/or syncing finish, launch the Local (single player) game using " +
					"the options.");
			this.CheckBox_Cooking_RunLocalGame.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Cooking_ForceSoundRecook
			// 
			this.CheckBox_Cooking_ForceSoundRecook.AutoSize = true;
			this.CheckBox_Cooking_ForceSoundRecook.Location = new System.Drawing.Point(133, 78);
			this.CheckBox_Cooking_ForceSoundRecook.Name = "CheckBox_Cooking_ForceSoundRecook";
			this.CheckBox_Cooking_ForceSoundRecook.Size = new System.Drawing.Size(128, 17);
			this.CheckBox_Cooking_ForceSoundRecook.TabIndex = 6;
			this.CheckBox_Cooking_ForceSoundRecook.Text = "Force Sound Recook";
			this.FormToolTips.SetToolTip(this.CheckBox_Cooking_ForceSoundRecook, "If checked, this will force a recook of all sounds [-ForceSoundRecook].");
			this.CheckBox_Cooking_ForceSoundRecook.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Cooking_CookFinalReleaseScript
			// 
			this.CheckBox_Cooking_CookFinalReleaseScript.AutoSize = true;
			this.CheckBox_Cooking_CookFinalReleaseScript.Location = new System.Drawing.Point(133, 55);
			this.CheckBox_Cooking_CookFinalReleaseScript.Name = "CheckBox_Cooking_CookFinalReleaseScript";
			this.CheckBox_Cooking_CookFinalReleaseScript.Size = new System.Drawing.Size(153, 17);
			this.CheckBox_Cooking_CookFinalReleaseScript.TabIndex = 4;
			this.CheckBox_Cooking_CookFinalReleaseScript.Text = "Cook Final Release Scripts";
			this.FormToolTips.SetToolTip(this.CheckBox_Cooking_CookFinalReleaseScript, "Select this option if you have compiled script (MyGame make -finalrelease) and yo" +
					"u want to use those script packages when cooking [-final_release].");
			this.CheckBox_Cooking_CookFinalReleaseScript.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Cooking_ForceSeekFreeRecook
			// 
			this.CheckBox_Cooking_ForceSeekFreeRecook.AutoSize = true;
			this.CheckBox_Cooking_ForceSeekFreeRecook.Location = new System.Drawing.Point(133, 32);
			this.CheckBox_Cooking_ForceSeekFreeRecook.Name = "CheckBox_Cooking_ForceSeekFreeRecook";
			this.CheckBox_Cooking_ForceSeekFreeRecook.Size = new System.Drawing.Size(146, 17);
			this.CheckBox_Cooking_ForceSeekFreeRecook.TabIndex = 0;
			this.CheckBox_Cooking_ForceSeekFreeRecook.Text = "Force Seek Free Recook";
			this.FormToolTips.SetToolTip(this.CheckBox_Cooking_ForceSeekFreeRecook, resources.GetString("CheckBox_Cooking_ForceSeekFreeRecook.ToolTip"));
			this.CheckBox_Cooking_ForceSeekFreeRecook.UseVisualStyleBackColor = true;
			// 
			// CheckBox_ConsoleTargets_CopyDebugInfo
			// 
			this.CheckBox_ConsoleTargets_CopyDebugInfo.AutoSize = true;
			this.CheckBox_ConsoleTargets_CopyDebugInfo.Checked = true;
			this.CheckBox_ConsoleTargets_CopyDebugInfo.CheckState = System.Windows.Forms.CheckState.Checked;
			this.CheckBox_ConsoleTargets_CopyDebugInfo.Location = new System.Drawing.Point(8, 68);
			this.CheckBox_ConsoleTargets_CopyDebugInfo.Name = "CheckBox_ConsoleTargets_CopyDebugInfo";
			this.CheckBox_ConsoleTargets_CopyDebugInfo.Size = new System.Drawing.Size(197, 17);
			this.CheckBox_ConsoleTargets_CopyDebugInfo.TabIndex = 9;
			this.CheckBox_ConsoleTargets_CopyDebugInfo.Text = "Copy files required for symbol lookup";
			this.FormToolTips.SetToolTip(this.CheckBox_ConsoleTargets_CopyDebugInfo, "Check if you wish UnrealConsole to generate callstacks for you. Default is checke" +
					"d.");
			this.CheckBox_ConsoleTargets_CopyDebugInfo.UseVisualStyleBackColor = true;
			// 
			// CheckBox_ConsoleTargets_RebootBeforeCopy
			// 
			this.CheckBox_ConsoleTargets_RebootBeforeCopy.AutoSize = true;
			this.CheckBox_ConsoleTargets_RebootBeforeCopy.Checked = true;
			this.CheckBox_ConsoleTargets_RebootBeforeCopy.CheckState = System.Windows.Forms.CheckState.Checked;
			this.CheckBox_ConsoleTargets_RebootBeforeCopy.Location = new System.Drawing.Point(9, 45);
			this.CheckBox_ConsoleTargets_RebootBeforeCopy.Name = "CheckBox_ConsoleTargets_RebootBeforeCopy";
			this.CheckBox_ConsoleTargets_RebootBeforeCopy.Size = new System.Drawing.Size(122, 17);
			this.CheckBox_ConsoleTargets_RebootBeforeCopy.TabIndex = 6;
			this.CheckBox_ConsoleTargets_RebootBeforeCopy.Text = "Reboot Before Copy";
			this.FormToolTips.SetToolTip(this.CheckBox_ConsoleTargets_RebootBeforeCopy, "Check if you want the target consoles to be rebooted before performing a sync ope" +
					"ration.");
			this.CheckBox_ConsoleTargets_RebootBeforeCopy.UseVisualStyleBackColor = true;
			// 
			// CheckBox_ConsoleTargets_ShowAllInfo
			// 
			this.CheckBox_ConsoleTargets_ShowAllInfo.AutoSize = true;
			this.CheckBox_ConsoleTargets_ShowAllInfo.Location = new System.Drawing.Point(9, 91);
			this.CheckBox_ConsoleTargets_ShowAllInfo.Name = "CheckBox_ConsoleTargets_ShowAllInfo";
			this.CheckBox_ConsoleTargets_ShowAllInfo.Size = new System.Drawing.Size(156, 17);
			this.CheckBox_ConsoleTargets_ShowAllInfo.TabIndex = 1;
			this.CheckBox_ConsoleTargets_ShowAllInfo.Text = "Show All Target Information";
			this.FormToolTips.SetToolTip(this.CheckBox_ConsoleTargets_ShowAllInfo, "Leave this unchecked as an optimization for displaying the list of targets.");
			this.CheckBox_ConsoleTargets_ShowAllInfo.UseVisualStyleBackColor = true;
			this.CheckBox_ConsoleTargets_ShowAllInfo.CheckedChanged += new System.EventHandler(this.CheckBox_ConsoleTargets_ShowAllInfo_CheckedChanged);
			// 
			// CheckBox_Compiling_AutoMode
			// 
			this.CheckBox_Compiling_AutoMode.AutoSize = true;
			this.CheckBox_Compiling_AutoMode.Location = new System.Drawing.Point(8, 72);
			this.CheckBox_Compiling_AutoMode.Name = "CheckBox_Compiling_AutoMode";
			this.CheckBox_Compiling_AutoMode.Size = new System.Drawing.Size(78, 17);
			this.CheckBox_Compiling_AutoMode.TabIndex = 10;
			this.CheckBox_Compiling_AutoMode.Text = "Auto Mode";
			this.FormToolTips.SetToolTip(this.CheckBox_Compiling_AutoMode, "Compile script in fully automatic mode (no prompts, auto .h updating, etc) [-auto" +
					" -unattended -updateinisauto].");
			this.CheckBox_Compiling_AutoMode.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Compiling_VerboseMode
			// 
			this.CheckBox_Compiling_VerboseMode.AutoSize = true;
			this.CheckBox_Compiling_VerboseMode.Location = new System.Drawing.Point(8, 49);
			this.CheckBox_Compiling_VerboseMode.Name = "CheckBox_Compiling_VerboseMode";
			this.CheckBox_Compiling_VerboseMode.Size = new System.Drawing.Size(95, 17);
			this.CheckBox_Compiling_VerboseMode.TabIndex = 9;
			this.CheckBox_Compiling_VerboseMode.Text = "Verbose Mode";
			this.FormToolTips.SetToolTip(this.CheckBox_Compiling_VerboseMode, "Check for verbose output.");
			this.CheckBox_Compiling_VerboseMode.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Compiling_RunWithCookedData
			// 
			this.CheckBox_Compiling_RunWithCookedData.AutoSize = true;
			this.CheckBox_Compiling_RunWithCookedData.Location = new System.Drawing.Point(8, 26);
			this.CheckBox_Compiling_RunWithCookedData.Name = "CheckBox_Compiling_RunWithCookedData";
			this.CheckBox_Compiling_RunWithCookedData.Size = new System.Drawing.Size(137, 17);
			this.CheckBox_Compiling_RunWithCookedData.TabIndex = 8;
			this.CheckBox_Compiling_RunWithCookedData.Text = "Run With Cooked Data";
			this.FormToolTips.SetToolTip(this.CheckBox_Compiling_RunWithCookedData, "Controls whether or not PC games will run with cooked data or the original packag" +
					"es [-seekfreeloading].");
			this.CheckBox_Compiling_RunWithCookedData.UseVisualStyleBackColor = true;
			// 
			// CheckBox_Compiling_FullRecompile
			// 
			this.CheckBox_Compiling_FullRecompile.AutoSize = true;
			this.CheckBox_Compiling_FullRecompile.Location = new System.Drawing.Point(8, 3);
			this.CheckBox_Compiling_FullRecompile.Name = "CheckBox_Compiling_FullRecompile";
			this.CheckBox_Compiling_FullRecompile.Size = new System.Drawing.Size(95, 17);
			this.CheckBox_Compiling_FullRecompile.TabIndex = 6;
			this.CheckBox_Compiling_FullRecompile.Text = "Full Recompile";
			this.FormToolTips.SetToolTip(this.CheckBox_Compiling_FullRecompile, "On the PC set this to true to recompile all .u packages, instead just the out-of-" +
					"date packages [-full].\r\nOn the PS3 set this to true to do a full recompile.");
			this.CheckBox_Compiling_FullRecompile.UseVisualStyleBackColor = true;
			// 
			// ToolStrip_GlobalSettings
			// 
			this.ToolStrip_GlobalSettings.AllowItemReorder = true;
			this.ToolStrip_GlobalSettings.Dock = System.Windows.Forms.DockStyle.None;
			this.ToolStrip_GlobalSettings.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolStripLabel_Platform,
            this.ComboBox_Platform,
            this.ToolStripSeparator1,
            this.ToolStripLabel_Game,
            this.ComboBox_Game,
            this.ToolStripSeparator2,
            this.ToolStripLabel_Config,
            this.ComboBox_Configuration,
            this.toolStripSeparator9,
            this.toolStripLabel2,
            this.ComboBox_CookingConfiguration});
			this.ToolStrip_GlobalSettings.LayoutStyle = System.Windows.Forms.ToolStripLayoutStyle.HorizontalStackWithOverflow;
			this.ToolStrip_GlobalSettings.Location = new System.Drawing.Point(3, 0);
			this.ToolStrip_GlobalSettings.Name = "ToolStrip_GlobalSettings";
			this.ToolStrip_GlobalSettings.Size = new System.Drawing.Size(947, 25);
			this.ToolStrip_GlobalSettings.TabIndex = 33;
			this.ToolStrip_GlobalSettings.Text = "toolStrip1";
			// 
			// ToolStripLabel_Platform
			// 
			this.ToolStripLabel_Platform.Name = "ToolStripLabel_Platform";
			this.ToolStripLabel_Platform.Size = new System.Drawing.Size(56, 22);
			this.ToolStripLabel_Platform.Text = "Platform:";
			// 
			// ComboBox_Platform
			// 
			this.ComboBox_Platform.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_Platform.Name = "ComboBox_Platform";
			this.ComboBox_Platform.Size = new System.Drawing.Size(125, 25);
			this.ComboBox_Platform.SelectedIndexChanged += new System.EventHandler(this.OnPlatformChanged);
			// 
			// ToolStripSeparator1
			// 
			this.ToolStripSeparator1.Name = "ToolStripSeparator1";
			this.ToolStripSeparator1.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripLabel_Game
			// 
			this.ToolStripLabel_Game.Name = "ToolStripLabel_Game";
			this.ToolStripLabel_Game.Size = new System.Drawing.Size(41, 22);
			this.ToolStripLabel_Game.Text = "Game:";
			// 
			// ComboBox_Game
			// 
			this.ComboBox_Game.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_Game.Name = "ComboBox_Game";
			this.ComboBox_Game.Size = new System.Drawing.Size(126, 25);
			this.ComboBox_Game.SelectedIndexChanged += new System.EventHandler(this.OnGameNameChanged);
			// 
			// ToolStripSeparator2
			// 
			this.ToolStripSeparator2.Name = "ToolStripSeparator2";
			this.ToolStripSeparator2.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripLabel_Config
			// 
			this.ToolStripLabel_Config.Name = "ToolStripLabel_Config";
			this.ToolStripLabel_Config.Size = new System.Drawing.Size(126, 22);
			this.ToolStripLabel_Config.Text = "Launch Configuration:";
			// 
			// ComboBox_Configuration
			// 
			this.ComboBox_Configuration.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_Configuration.Name = "ComboBox_Configuration";
			this.ComboBox_Configuration.Size = new System.Drawing.Size(121, 25);
			// 
			// toolStripSeparator9
			// 
			this.toolStripSeparator9.Name = "toolStripSeparator9";
			this.toolStripSeparator9.Size = new System.Drawing.Size(6, 25);
			// 
			// toolStripLabel2
			// 
			this.toolStripLabel2.Name = "toolStripLabel2";
			this.toolStripLabel2.Size = new System.Drawing.Size(193, 22);
			this.toolStripLabel2.Text = "Cooking/Compiling Configuration:";
			// 
			// ComboBox_CookingConfiguration
			// 
			this.ComboBox_CookingConfiguration.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_CookingConfiguration.Name = "ComboBox_CookingConfiguration";
			this.ComboBox_CookingConfiguration.Size = new System.Drawing.Size(121, 25);
			// 
			// CookMapToolTip
			// 
			this.CookMapToolTip.ShowAlways = true;
			// 
			// menuStrip1
			// 
			this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.MenuMain_File,
            this.MenuMain_Edit});
			this.menuStrip1.Location = new System.Drawing.Point(0, 0);
			this.menuStrip1.Name = "menuStrip1";
			this.menuStrip1.Size = new System.Drawing.Size(1424, 24);
			this.menuStrip1.TabIndex = 35;
			this.menuStrip1.Text = "MenuMain";
			// 
			// MenuMain_File
			// 
			this.MenuMain_File.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.MenuMain_File_Exit});
			this.MenuMain_File.Name = "MenuMain_File";
			this.MenuMain_File.Size = new System.Drawing.Size(37, 20);
			this.MenuMain_File.Text = "&File";
			// 
			// MenuMain_File_Exit
			// 
			this.MenuMain_File_Exit.Name = "MenuMain_File_Exit";
			this.MenuMain_File_Exit.Size = new System.Drawing.Size(92, 22);
			this.MenuMain_File_Exit.Text = "E&xit";
			this.MenuMain_File_Exit.Click += new System.EventHandler(this.MenuMain_File_Exit_Click);
			// 
			// MenuMain_Edit
			// 
			this.MenuMain_Edit.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.MenuMain_Edit_CopyToClipboard});
			this.MenuMain_Edit.Name = "MenuMain_Edit";
			this.MenuMain_Edit.Size = new System.Drawing.Size(39, 20);
			this.MenuMain_Edit.Text = "&Edit";
			// 
			// MenuMain_Edit_CopyToClipboard
			// 
			this.MenuMain_Edit_CopyToClipboard.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.MenuMain_Edit_CopyToClipboard_SPURL,
            this.MenuMain_Edit_CopyToClipboard_MPURL,
            this.MenuMain_Edit_CopyToClipboard_Compile,
            this.MenuMain_Edit_CopyToClipboard_Cook});
			this.MenuMain_Edit_CopyToClipboard.Name = "MenuMain_Edit_CopyToClipboard";
			this.MenuMain_Edit_CopyToClipboard.Size = new System.Drawing.Size(171, 22);
			this.MenuMain_Edit_CopyToClipboard.Text = "Copy to Clipboard";
			// 
			// MenuMain_Edit_CopyToClipboard_SPURL
			// 
			this.MenuMain_Edit_CopyToClipboard_SPURL.Name = "MenuMain_Edit_CopyToClipboard_SPURL";
			this.MenuMain_Edit_CopyToClipboard_SPURL.Size = new System.Drawing.Size(204, 22);
			this.MenuMain_Edit_CopyToClipboard_SPURL.Text = "Single Player URL";
			this.MenuMain_Edit_CopyToClipboard_SPURL.Click += new System.EventHandler(this.MenuMain_Edit_CopyToClipboard_SPURL_Click);
			// 
			// MenuMain_Edit_CopyToClipboard_MPURL
			// 
			this.MenuMain_Edit_CopyToClipboard_MPURL.Name = "MenuMain_Edit_CopyToClipboard_MPURL";
			this.MenuMain_Edit_CopyToClipboard_MPURL.Size = new System.Drawing.Size(204, 22);
			this.MenuMain_Edit_CopyToClipboard_MPURL.Text = "Multiplayer URL";
			this.MenuMain_Edit_CopyToClipboard_MPURL.Click += new System.EventHandler(this.MenuMain_Edit_CopyToClipboard_MPURL_Click);
			// 
			// MenuMain_Edit_CopyToClipboard_Compile
			// 
			this.MenuMain_Edit_CopyToClipboard_Compile.Name = "MenuMain_Edit_CopyToClipboard_Compile";
			this.MenuMain_Edit_CopyToClipboard_Compile.Size = new System.Drawing.Size(204, 22);
			this.MenuMain_Edit_CopyToClipboard_Compile.Text = "Compile Command Line";
			this.MenuMain_Edit_CopyToClipboard_Compile.Click += new System.EventHandler(this.MenuMain_Edit_CopyToClipboard_Compile_Click);
			// 
			// MenuMain_Edit_CopyToClipboard_Cook
			// 
			this.MenuMain_Edit_CopyToClipboard_Cook.Name = "MenuMain_Edit_CopyToClipboard_Cook";
			this.MenuMain_Edit_CopyToClipboard_Cook.Size = new System.Drawing.Size(204, 22);
			this.MenuMain_Edit_CopyToClipboard_Cook.Text = "Cooking Command Line";
			this.MenuMain_Edit_CopyToClipboard_Cook.Click += new System.EventHandler(this.MenuMain_Edit_CopyToClipboard_Cook_Click);
			// 
			// toolStripStatusLabel3
			// 
			this.toolStripStatusLabel3.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold);
			this.toolStripStatusLabel3.Name = "toolStripStatusLabel3";
			this.toolStripStatusLabel3.Size = new System.Drawing.Size(76, 19);
			this.toolStripStatusLabel3.Text = "Map to Play:";
			this.toolStripStatusLabel3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// ToolStripStatusLabel_LocalMap
			// 
			this.ToolStripStatusLabel_LocalMap.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Right;
			this.ToolStripStatusLabel_LocalMap.Name = "ToolStripStatusLabel_LocalMap";
			this.ToolStripStatusLabel_LocalMap.Size = new System.Drawing.Size(63, 19);
			this.ToolStripStatusLabel_LocalMap.Text = "LocalMap";
			this.ToolStripStatusLabel_LocalMap.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// statusStrip1
			// 
			this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel3,
            this.ToolStripStatusLabel_LocalMap});
			this.statusStrip1.Location = new System.Drawing.Point(0, 905);
			this.statusStrip1.Name = "statusStrip1";
			this.statusStrip1.Size = new System.Drawing.Size(1424, 24);
			this.statusStrip1.TabIndex = 32;
			this.statusStrip1.Text = "statusStrip1";
			// 
			// toolStripContainer1
			// 
			// 
			// toolStripContainer1.ContentPanel
			// 
			this.toolStripContainer1.ContentPanel.Controls.Add(this.splitContainer1);
			this.toolStripContainer1.ContentPanel.Size = new System.Drawing.Size(1424, 831);
			this.toolStripContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.toolStripContainer1.Location = new System.Drawing.Point(0, 24);
			this.toolStripContainer1.Name = "toolStripContainer1";
			this.toolStripContainer1.Size = new System.Drawing.Size(1424, 881);
			this.toolStripContainer1.TabIndex = 36;
			this.toolStripContainer1.Text = "toolStripContainer1";
			// 
			// toolStripContainer1.TopToolStripPanel
			// 
			this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.ToolStrip_GlobalSettings);
			this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.ToolStrip_Actions);
			// 
			// splitContainer1
			// 
			this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer1.Location = new System.Drawing.Point(0, 0);
			this.splitContainer1.Name = "splitContainer1";
			// 
			// splitContainer1.Panel1
			// 
			this.splitContainer1.Panel1.Controls.Add(this.TabControl_Main);
			// 
			// splitContainer1.Panel2
			// 
			this.splitContainer1.Panel2.Controls.Add(this.OutputWindowView_LogWindow);
			this.splitContainer1.Size = new System.Drawing.Size(1424, 831);
			this.splitContainer1.SplitterDistance = 691;
			this.splitContainer1.SplitterWidth = 2;
			this.splitContainer1.TabIndex = 36;
			// 
			// TabControl_Main
			// 
			this.TabControl_Main.Controls.Add(this.Tab_Game);
			this.TabControl_Main.Controls.Add(this.Tab_Cooking);
			this.TabControl_Main.Controls.Add(this.Tab_ConsoleSetup);
			this.TabControl_Main.Controls.Add(this.Tab_Compiling);
			this.TabControl_Main.Controls.Add(this.Tab_Help);
			this.TabControl_Main.DataBindings.Add(new System.Windows.Forms.Binding("SelectedIndex", global::UnrealFrontend.Properties.Settings.Default, "UFEMainWindowSelectedTab", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
			this.TabControl_Main.Dock = System.Windows.Forms.DockStyle.Fill;
			this.TabControl_Main.Location = new System.Drawing.Point(0, 0);
			this.TabControl_Main.Name = "TabControl_Main";
			this.TabControl_Main.SelectedIndex = global::UnrealFrontend.Properties.Settings.Default.UFEMainWindowSelectedTab;
			this.TabControl_Main.Size = new System.Drawing.Size(691, 831);
			this.TabControl_Main.TabIndex = 23;
			// 
			// Tab_Game
			// 
			this.Tab_Game.AutoScroll = true;
			this.Tab_Game.Controls.Add(this.GroupBox_Game_Xbox360);
			this.Tab_Game.Controls.Add(this.GroupBox_Game_GoW2);
			this.Tab_Game.Controls.Add(this.CheckBox_Game_OverrideExecutable);
			this.Tab_Game.Controls.Add(this.TextBox_Game_ExecutableNameOverride);
			this.Tab_Game.Controls.Add(this.label11);
			this.Tab_Game.Controls.Add(this.ComboBox_MapToPlay);
			this.Tab_Game.Controls.Add(this.GroupBox_Game_PC);
			this.Tab_Game.Controls.Add(this.GroupBox_Game_Common);
			this.Tab_Game.Controls.Add(this.ButtonBrowseMap);
			this.Tab_Game.Controls.Add(this.CheckBox_Game_UseCookedMap);
			this.Tab_Game.Controls.Add(this.label1);
			this.Tab_Game.Location = new System.Drawing.Point(4, 22);
			this.Tab_Game.Name = "Tab_Game";
			this.Tab_Game.Padding = new System.Windows.Forms.Padding(3);
			this.Tab_Game.Size = new System.Drawing.Size(683, 805);
			this.Tab_Game.TabIndex = 4;
			this.Tab_Game.Text = "Game";
			this.Tab_Game.ToolTipText = "This tab handles information on what maps to cook and play.";
			this.Tab_Game.UseVisualStyleBackColor = true;
			this.Tab_Game.Leave += new System.EventHandler(this.Tab_Game_Leave);
			this.Tab_Game.Enter += new System.EventHandler(this.Tab_Game_Enter);
			// 
			// GroupBox_Game_Xbox360
			// 
			this.GroupBox_Game_Xbox360.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.GroupBox_Game_Xbox360.Controls.Add(this.CheckBox_Game_EncryptedSockets);
			this.GroupBox_Game_Xbox360.Location = new System.Drawing.Point(6, 619);
			this.GroupBox_Game_Xbox360.Name = "GroupBox_Game_Xbox360";
			this.GroupBox_Game_Xbox360.Size = new System.Drawing.Size(671, 45);
			this.GroupBox_Game_Xbox360.TabIndex = 45;
			this.GroupBox_Game_Xbox360.TabStop = false;
			this.GroupBox_Game_Xbox360.Text = "Xbox360";
			// 
			// GroupBox_Game_GoW2
			// 
			this.GroupBox_Game_GoW2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.GroupBox_Game_GoW2.Controls.Add(this.CheckBox_Game_GoW2OverrideSPMap);
			this.GroupBox_Game_GoW2.Controls.Add(this.ComboBox_Game_GoW2SPMapName);
			this.GroupBox_Game_GoW2.Controls.Add(this.label14);
			this.GroupBox_Game_GoW2.Location = new System.Drawing.Point(6, 670);
			this.GroupBox_Game_GoW2.Name = "GroupBox_Game_GoW2";
			this.GroupBox_Game_GoW2.Size = new System.Drawing.Size(671, 87);
			this.GroupBox_Game_GoW2.TabIndex = 44;
			this.GroupBox_Game_GoW2.TabStop = false;
			this.GroupBox_Game_GoW2.Text = "Gears of War 2";
			// 
			// CheckBox_Game_GoW2OverrideSPMap
			// 
			this.CheckBox_Game_GoW2OverrideSPMap.AutoSize = true;
			this.CheckBox_Game_GoW2OverrideSPMap.Location = new System.Drawing.Point(7, 59);
			this.CheckBox_Game_GoW2OverrideSPMap.Name = "CheckBox_Game_GoW2OverrideSPMap";
			this.CheckBox_Game_GoW2OverrideSPMap.Size = new System.Drawing.Size(154, 17);
			this.CheckBox_Game_GoW2OverrideSPMap.TabIndex = 2;
			this.CheckBox_Game_GoW2OverrideSPMap.Text = "Override Single Player Map";
			this.CheckBox_Game_GoW2OverrideSPMap.UseVisualStyleBackColor = true;
			// 
			// ComboBox_Game_GoW2SPMapName
			// 
			this.ComboBox_Game_GoW2SPMapName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.ComboBox_Game_GoW2SPMapName.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_Game_GoW2SPMapName.FormattingEnabled = true;
			this.ComboBox_Game_GoW2SPMapName.Location = new System.Drawing.Point(7, 32);
			this.ComboBox_Game_GoW2SPMapName.Name = "ComboBox_Game_GoW2SPMapName";
			this.ComboBox_Game_GoW2SPMapName.Size = new System.Drawing.Size(658, 21);
			this.ComboBox_Game_GoW2SPMapName.TabIndex = 1;
			// 
			// label14
			// 
			this.label14.AutoSize = true;
			this.label14.Location = new System.Drawing.Point(6, 16);
			this.label14.Name = "label14";
			this.label14.Size = new System.Drawing.Size(47, 13);
			this.label14.TabIndex = 0;
			this.label14.Text = "Chapter:";
			// 
			// CheckBox_Game_OverrideExecutable
			// 
			this.CheckBox_Game_OverrideExecutable.AutoSize = true;
			this.CheckBox_Game_OverrideExecutable.Location = new System.Drawing.Point(6, 116);
			this.CheckBox_Game_OverrideExecutable.Name = "CheckBox_Game_OverrideExecutable";
			this.CheckBox_Game_OverrideExecutable.Size = new System.Drawing.Size(122, 17);
			this.CheckBox_Game_OverrideExecutable.TabIndex = 43;
			this.CheckBox_Game_OverrideExecutable.Text = "Override Executable";
			this.CheckBox_Game_OverrideExecutable.UseVisualStyleBackColor = true;
			this.CheckBox_Game_OverrideExecutable.CheckedChanged += new System.EventHandler(this.CheckBox_Game_OverrideExecutable_CheckedChanged);
			// 
			// TextBox_Game_ExecutableNameOverride
			// 
			this.TextBox_Game_ExecutableNameOverride.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.TextBox_Game_ExecutableNameOverride.Location = new System.Drawing.Point(6, 90);
			this.TextBox_Game_ExecutableNameOverride.Name = "TextBox_Game_ExecutableNameOverride";
			this.TextBox_Game_ExecutableNameOverride.ReadOnly = true;
			this.TextBox_Game_ExecutableNameOverride.Size = new System.Drawing.Size(671, 20);
			this.TextBox_Game_ExecutableNameOverride.TabIndex = 42;
			this.TextBox_Game_ExecutableNameOverride.TextChanged += new System.EventHandler(this.TextBox_Game_ExecutableNameOverride_TextChanged);
			// 
			// label11
			// 
			this.label11.AutoSize = true;
			this.label11.Location = new System.Drawing.Point(3, 74);
			this.label11.Name = "label11";
			this.label11.Size = new System.Drawing.Size(137, 13);
			this.label11.TabIndex = 41;
			this.label11.Text = "Executable Name Override:";
			// 
			// ComboBox_MapToPlay
			// 
			this.ComboBox_MapToPlay.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.ComboBox_MapToPlay.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
			this.ComboBox_MapToPlay.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
			this.ComboBox_MapToPlay.FormattingEnabled = true;
			this.ComboBox_MapToPlay.Location = new System.Drawing.Point(6, 19);
			this.ComboBox_MapToPlay.Name = "ComboBox_MapToPlay";
			this.ComboBox_MapToPlay.Size = new System.Drawing.Size(590, 21);
			this.ComboBox_MapToPlay.TabIndex = 40;
			this.ComboBox_MapToPlay.TextChanged += new System.EventHandler(this.TextBox_MapToPlay_TextChanged);
			// 
			// GroupBox_Game_PC
			// 
			this.GroupBox_Game_PC.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.GroupBox_Game_PC.Controls.Add(this.ComboBox_Game_Resolution);
			this.GroupBox_Game_PC.Controls.Add(this.CheckBox_Game_RemoteControl);
			this.GroupBox_Game_PC.Controls.Add(this.CheckBox_Game_ShowLog);
			this.GroupBox_Game_PC.Controls.Add(this.label7);
			this.GroupBox_Game_PC.Controls.Add(this.label6);
			this.GroupBox_Game_PC.Controls.Add(this.ComboBox_Game_ServerType);
			this.GroupBox_Game_PC.Location = new System.Drawing.Point(6, 458);
			this.GroupBox_Game_PC.Name = "GroupBox_Game_PC";
			this.GroupBox_Game_PC.Size = new System.Drawing.Size(671, 155);
			this.GroupBox_Game_PC.TabIndex = 38;
			this.GroupBox_Game_PC.TabStop = false;
			this.GroupBox_Game_PC.Text = "PC";
			// 
			// ComboBox_Game_Resolution
			// 
			this.ComboBox_Game_Resolution.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBox_Game_Resolution.FormattingEnabled = true;
			this.ComboBox_Game_Resolution.Items.AddRange(new object[] {
            "640x480",
            "800x600",
            "1024x768",
            "1152x864",
            "1280x720",
            "1280x768",
            "1280x800",
            "1280x960",
            "1280x1024",
            "1440x900",
            "1600x1200",
            "1680x1050",
            "1920x1080",
            "1920x1200",
            "1920x1440",
            "2048x1536",
            "2560x1600"});
			this.ComboBox_Game_Resolution.Location = new System.Drawing.Point(9, 81);
			this.ComboBox_Game_Resolution.Name = "ComboBox_Game_Resolution";
			this.ComboBox_Game_Resolution.Size = new System.Drawing.Size(121, 21);
			this.ComboBox_Game_Resolution.TabIndex = 7;
			// 
			// label7
			// 
			this.label7.AutoSize = true;
			this.label7.Location = new System.Drawing.Point(6, 65);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(60, 13);
			this.label7.TabIndex = 5;
			this.label7.Text = "Resolution:";
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(6, 16);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(68, 13);
			this.label6.TabIndex = 3;
			this.label6.Text = "Server Type:";
			// 
			// GroupBox_Game_Common
			// 
			this.GroupBox_Game_Common.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.GroupBox_Game_Common.Controls.Add(this.CheckBox_Game_ClearUCWindow);
			this.GroupBox_Game_Common.Controls.Add(this.ComboBox_Game_ExtraOptions);
			this.GroupBox_Game_Common.Controls.Add(this.CheckBox_Game_CaptureFPSChartInfo);
			this.GroupBox_Game_Common.Controls.Add(this.TextBox_Game_SentinelTag);
			this.GroupBox_Game_Common.Controls.Add(this.label17);
			this.GroupBox_Game_Common.Controls.Add(this.label5);
			this.GroupBox_Game_Common.Controls.Add(this.label16);
			this.GroupBox_Game_Common.Controls.Add(this.CheckBox_Game_SentinelRun);
			this.GroupBox_Game_Common.Controls.Add(this.NumericUpDown_Game_NumClients);
			this.GroupBox_Game_Common.Controls.Add(this.ComboBox_Game_SentinelType);
			this.GroupBox_Game_Common.Controls.Add(this.TextBox_Game_ExecCommands);
			this.GroupBox_Game_Common.Controls.Add(this.label4);
			this.GroupBox_Game_Common.Controls.Add(this.label2);
			this.GroupBox_Game_Common.Controls.Add(this.CheckBox_Game_NoVSync);
			this.GroupBox_Game_Common.Controls.Add(this.CheckBox_Game_MultiThreaded);
			this.GroupBox_Game_Common.Controls.Add(this.CheckBox_Game_NoSound);
			this.GroupBox_Game_Common.Location = new System.Drawing.Point(6, 139);
			this.GroupBox_Game_Common.Name = "GroupBox_Game_Common";
			this.GroupBox_Game_Common.Size = new System.Drawing.Size(671, 313);
			this.GroupBox_Game_Common.TabIndex = 37;
			this.GroupBox_Game_Common.TabStop = false;
			this.GroupBox_Game_Common.Text = "Common";
			// 
			// ComboBox_Game_ExtraOptions
			// 
			this.ComboBox_Game_ExtraOptions.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.ComboBox_Game_ExtraOptions.FormattingEnabled = true;
			this.ComboBox_Game_ExtraOptions.Location = new System.Drawing.Point(10, 32);
			this.ComboBox_Game_ExtraOptions.Name = "ComboBox_Game_ExtraOptions";
			this.ComboBox_Game_ExtraOptions.Size = new System.Drawing.Size(655, 21);
			this.ComboBox_Game_ExtraOptions.TabIndex = 15;
			// 
			// label17
			// 
			this.label17.AutoSize = true;
			this.label17.Location = new System.Drawing.Point(264, 189);
			this.label17.Name = "label17";
			this.label17.Size = new System.Drawing.Size(67, 13);
			this.label17.TabIndex = 12;
			this.label17.Text = "Sentinel Tag";
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(6, 262);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(93, 13);
			this.label5.TabIndex = 2;
			this.label5.Text = "Number of Clients:";
			// 
			// label16
			// 
			this.label16.AutoSize = true;
			this.label16.Location = new System.Drawing.Point(123, 189);
			this.label16.Name = "label16";
			this.label16.Size = new System.Drawing.Size(75, 13);
			this.label16.TabIndex = 9;
			this.label16.Text = "Sentinel Type:";
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(6, 69);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(89, 13);
			this.label4.TabIndex = 9;
			this.label4.Text = "Exec Commands:";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(6, 16);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(73, 13);
			this.label2.TabIndex = 7;
			this.label2.Text = "Extra Options:";
			// 
			// ButtonBrowseMap
			// 
			this.ButtonBrowseMap.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.ButtonBrowseMap.Location = new System.Drawing.Point(602, 17);
			this.ButtonBrowseMap.Name = "ButtonBrowseMap";
			this.ButtonBrowseMap.Size = new System.Drawing.Size(75, 23);
			this.ButtonBrowseMap.TabIndex = 35;
			this.ButtonBrowseMap.Text = "&Browse";
			this.ButtonBrowseMap.UseVisualStyleBackColor = true;
			this.ButtonBrowseMap.Click += new System.EventHandler(this.OnBrowseSPClick);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(3, 3);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(66, 13);
			this.label1.TabIndex = 33;
			this.label1.Text = "Map to Play:";
			// 
			// Tab_Cooking
			// 
			this.Tab_Cooking.AutoScroll = true;
			this.Tab_Cooking.Controls.Add(this.ComboBox_Cooking_AdditionalOptions);
			this.Tab_Cooking.Controls.Add(this.ComboBox_Cooking_MapsToCook);
			this.Tab_Cooking.Controls.Add(this.GroupBox_Cooking_Game_Options);
			this.Tab_Cooking.Controls.Add(this.GroupBox_Cooking_Options);
			this.Tab_Cooking.Controls.Add(this.Button_ImportMapList);
			this.Tab_Cooking.Controls.Add(this.label3);
			this.Tab_Cooking.Controls.Add(this.label18);
			this.Tab_Cooking.Location = new System.Drawing.Point(4, 22);
			this.Tab_Cooking.Name = "Tab_Cooking";
			this.Tab_Cooking.Size = new System.Drawing.Size(683, 805);
			this.Tab_Cooking.TabIndex = 7;
			this.Tab_Cooking.Text = "Cooking";
			this.Tab_Cooking.UseVisualStyleBackColor = true;
			this.Tab_Cooking.Leave += new System.EventHandler(this.Tab_Cooking_Leave);
			this.Tab_Cooking.Enter += new System.EventHandler(this.Tab_Cooking_Enter);
			// 
			// ComboBox_Cooking_AdditionalOptions
			// 
			this.ComboBox_Cooking_AdditionalOptions.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.ComboBox_Cooking_AdditionalOptions.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.Suggest;
			this.ComboBox_Cooking_AdditionalOptions.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
			this.ComboBox_Cooking_AdditionalOptions.FormattingEnabled = true;
			this.ComboBox_Cooking_AdditionalOptions.Location = new System.Drawing.Point(14, 318);
			this.ComboBox_Cooking_AdditionalOptions.Name = "ComboBox_Cooking_AdditionalOptions";
			this.ComboBox_Cooking_AdditionalOptions.Size = new System.Drawing.Size(669, 21);
			this.ComboBox_Cooking_AdditionalOptions.TabIndex = 41;
			// 
			// ComboBox_Cooking_MapsToCook
			// 
			this.ComboBox_Cooking_MapsToCook.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.ComboBox_Cooking_MapsToCook.FormattingEnabled = true;
			this.ComboBox_Cooking_MapsToCook.Location = new System.Drawing.Point(6, 18);
			this.ComboBox_Cooking_MapsToCook.Name = "ComboBox_Cooking_MapsToCook";
			this.ComboBox_Cooking_MapsToCook.Size = new System.Drawing.Size(674, 21);
			this.ComboBox_Cooking_MapsToCook.TabIndex = 40;
			// 
			// GroupBox_Cooking_Game_Options
			// 
			this.GroupBox_Cooking_Game_Options.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.GroupBox_Cooking_Game_Options.Controls.Add(this.CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS);
			this.GroupBox_Cooking_Game_Options.Controls.Add(this.CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes);
			this.GroupBox_Cooking_Game_Options.Location = new System.Drawing.Point(11, 353);
			this.GroupBox_Cooking_Game_Options.Name = "GroupBox_Cooking_Game_Options";
			this.GroupBox_Cooking_Game_Options.Size = new System.Drawing.Size(672, 66);
			this.GroupBox_Cooking_Game_Options.TabIndex = 37;
			this.GroupBox_Cooking_Game_Options.TabStop = false;
			this.GroupBox_Cooking_Game_Options.Text = "Game Specific Options";
			// 
			// GroupBox_Cooking_Options
			// 
			this.GroupBox_Cooking_Options.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.GroupBox_Cooking_Options.Controls.Add(this.CheckBox_Cooking_ShowBalloon);
			this.GroupBox_Cooking_Options.Controls.Add(this.ListView_Cooking_Languages);
			this.GroupBox_Cooking_Options.Controls.Add(this.CheckBox_Cooking_SkipScriptCompile);
			this.GroupBox_Cooking_Options.Controls.Add(this.CheckBox_Cooking_DisablePackageCompression);
			this.GroupBox_Cooking_Options.Controls.Add(this.CheckBox_Cooking_CookMod);
			this.GroupBox_Cooking_Options.Controls.Add(this.CheckBox_Cooking_RunLocalGame);
			this.GroupBox_Cooking_Options.Controls.Add(this.CheckBox_Cooking_ForceSoundRecook);
			this.GroupBox_Cooking_Options.Controls.Add(this.CheckBox_Cooking_CookFinalReleaseScript);
			this.GroupBox_Cooking_Options.Controls.Add(this.label10);
			this.GroupBox_Cooking_Options.Controls.Add(this.CheckBox_Cooking_ForceSeekFreeRecook);
			this.GroupBox_Cooking_Options.Location = new System.Drawing.Point(8, 73);
			this.GroupBox_Cooking_Options.Name = "GroupBox_Cooking_Options";
			this.GroupBox_Cooking_Options.Size = new System.Drawing.Size(672, 217);
			this.GroupBox_Cooking_Options.TabIndex = 36;
			this.GroupBox_Cooking_Options.TabStop = false;
			this.GroupBox_Cooking_Options.Text = "Options";
			// 
			// CheckBox_Cooking_ShowBalloon
			// 
			this.CheckBox_Cooking_ShowBalloon.AutoSize = true;
			this.CheckBox_Cooking_ShowBalloon.Location = new System.Drawing.Point(133, 193);
			this.CheckBox_Cooking_ShowBalloon.Name = "CheckBox_Cooking_ShowBalloon";
			this.CheckBox_Cooking_ShowBalloon.Size = new System.Drawing.Size(173, 17);
			this.CheckBox_Cooking_ShowBalloon.TabIndex = 13;
			this.CheckBox_Cooking_ShowBalloon.Text = "Show Balloon After Cook/Sync";
			this.CheckBox_Cooking_ShowBalloon.UseVisualStyleBackColor = true;
			// 
			// ListView_Cooking_Languages
			// 
			this.ListView_Cooking_Languages.CheckBoxes = true;
			this.ListView_Cooking_Languages.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.ListItem_Cooking_LangName});
			listViewItem1.Checked = true;
			listViewItem1.StateImageIndex = 1;
			listViewItem2.StateImageIndex = 0;
			listViewItem3.StateImageIndex = 0;
			listViewItem4.StateImageIndex = 0;
			listViewItem5.StateImageIndex = 0;
			listViewItem6.StateImageIndex = 0;
			listViewItem7.StateImageIndex = 0;
			listViewItem8.StateImageIndex = 0;
			listViewItem9.StateImageIndex = 0;
			listViewItem10.StateImageIndex = 0;
			listViewItem11.StateImageIndex = 0;
			listViewItem12.StateImageIndex = 0;
			listViewItem13.StateImageIndex = 0;
			listViewItem14.StateImageIndex = 0;
			this.ListView_Cooking_Languages.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            listViewItem1,
            listViewItem2,
            listViewItem3,
            listViewItem4,
            listViewItem5,
            listViewItem6,
            listViewItem7,
            listViewItem8,
            listViewItem9,
            listViewItem10,
            listViewItem11,
            listViewItem12,
            listViewItem13,
            listViewItem14});
			this.ListView_Cooking_Languages.LabelWrap = false;
			this.ListView_Cooking_Languages.Location = new System.Drawing.Point(6, 32);
			this.ListView_Cooking_Languages.Name = "ListView_Cooking_Languages";
			this.ListView_Cooking_Languages.Size = new System.Drawing.Size(121, 178);
			this.ListView_Cooking_Languages.TabIndex = 12;
			this.ListView_Cooking_Languages.UseCompatibleStateImageBehavior = false;
			this.ListView_Cooking_Languages.View = System.Windows.Forms.View.Details;
			// 
			// ListItem_Cooking_LangName
			// 
			this.ListItem_Cooking_LangName.Text = "Language";
			this.ListItem_Cooking_LangName.Width = 80;
			// 
			// CheckBox_Cooking_DisablePackageCompression
			// 
			this.CheckBox_Cooking_DisablePackageCompression.AutoSize = true;
			this.CheckBox_Cooking_DisablePackageCompression.Location = new System.Drawing.Point(133, 147);
			this.CheckBox_Cooking_DisablePackageCompression.Name = "CheckBox_Cooking_DisablePackageCompression";
			this.CheckBox_Cooking_DisablePackageCompression.Size = new System.Drawing.Size(170, 17);
			this.CheckBox_Cooking_DisablePackageCompression.TabIndex = 10;
			this.CheckBox_Cooking_DisablePackageCompression.Text = "Disable Package Compression";
			this.CheckBox_Cooking_DisablePackageCompression.UseVisualStyleBackColor = true;
			// 
			// label10
			// 
			this.label10.AutoSize = true;
			this.label10.Location = new System.Drawing.Point(6, 16);
			this.label10.Name = "label10";
			this.label10.Size = new System.Drawing.Size(132, 13);
			this.label10.TabIndex = 2;
			this.label10.Text = "Languages to Cook/Sync:";
			// 
			// Button_ImportMapList
			// 
			this.Button_ImportMapList.Location = new System.Drawing.Point(6, 44);
			this.Button_ImportMapList.Name = "Button_ImportMapList";
			this.Button_ImportMapList.Size = new System.Drawing.Size(91, 23);
			this.Button_ImportMapList.TabIndex = 34;
			this.Button_ImportMapList.Text = "Import Map List";
			this.Button_ImportMapList.UseVisualStyleBackColor = true;
			this.Button_ImportMapList.Click += new System.EventHandler(this.OnImportMapListClick);
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(3, 2);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(75, 13);
			this.label3.TabIndex = 35;
			this.label3.Text = "Maps to cook:";
			// 
			// label18
			// 
			this.label18.AutoSize = true;
			this.label18.Location = new System.Drawing.Point(11, 302);
			this.label18.Name = "label18";
			this.label18.Size = new System.Drawing.Size(126, 13);
			this.label18.TabIndex = 39;
			this.label18.Text = "Additional cooker options";
			// 
			// Tab_ConsoleSetup
			// 
			this.Tab_ConsoleSetup.Controls.Add(this.ComboBox_ConsoleTargets_ConsoleBaseDir);
			this.Tab_ConsoleSetup.Controls.Add(this.CheckBox_ConsoleTargets_CopyDebugInfo);
			this.Tab_ConsoleSetup.Controls.Add(this.label9);
			this.Tab_ConsoleSetup.Controls.Add(this.CheckBox_ConsoleTargets_RebootBeforeCopy);
			this.Tab_ConsoleSetup.Controls.Add(this.CheckBox_ConsoleTargets_ShowAllInfo);
			this.Tab_ConsoleSetup.Controls.Add(this.ListBox_Targets);
			this.Tab_ConsoleSetup.Location = new System.Drawing.Point(4, 22);
			this.Tab_ConsoleSetup.Name = "Tab_ConsoleSetup";
			this.Tab_ConsoleSetup.Padding = new System.Windows.Forms.Padding(3);
			this.Tab_ConsoleSetup.Size = new System.Drawing.Size(683, 805);
			this.Tab_ConsoleSetup.TabIndex = 3;
			this.Tab_ConsoleSetup.Text = "Console Targets";
			this.Tab_ConsoleSetup.ToolTipText = "This tab has settings for console platforms, as well as some utilities like Reboo" +
				"t and Copy data to target.";
			this.Tab_ConsoleSetup.UseVisualStyleBackColor = true;
			// 
			// ComboBox_ConsoleTargets_ConsoleBaseDir
			// 
			this.ComboBox_ConsoleTargets_ConsoleBaseDir.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.ComboBox_ConsoleTargets_ConsoleBaseDir.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.Suggest;
			this.ComboBox_ConsoleTargets_ConsoleBaseDir.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
			this.ComboBox_ConsoleTargets_ConsoleBaseDir.FormattingEnabled = true;
			this.ComboBox_ConsoleTargets_ConsoleBaseDir.Location = new System.Drawing.Point(9, 19);
			this.ComboBox_ConsoleTargets_ConsoleBaseDir.Name = "ComboBox_ConsoleTargets_ConsoleBaseDir";
			this.ComboBox_ConsoleTargets_ConsoleBaseDir.Size = new System.Drawing.Size(668, 21);
			this.ComboBox_ConsoleTargets_ConsoleBaseDir.TabIndex = 10;
			this.ComboBox_ConsoleTargets_ConsoleBaseDir.Text = "UnrealEngine3";
			// 
			// label9
			// 
			this.label9.AutoSize = true;
			this.label9.Location = new System.Drawing.Point(6, 3);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(120, 13);
			this.label9.TabIndex = 7;
			this.label9.Text = "Console Base Directory:";
			// 
			// ListBox_Targets
			// 
			this.ListBox_Targets.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.ListBox_Targets.CheckBoxes = true;
			this.ListBox_Targets.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.ListBox_Targets_Name,
            this.ListBox_Targets_IP,
            this.ListBox_Targets_DebugIP,
            this.ListBox_Targets_Type});
			this.ListBox_Targets.GridLines = true;
			this.ListBox_Targets.Location = new System.Drawing.Point(9, 114);
			this.ListBox_Targets.Name = "ListBox_Targets";
			this.ListBox_Targets.Size = new System.Drawing.Size(668, 688);
			this.ListBox_Targets.TabIndex = 0;
			this.ListBox_Targets.UseCompatibleStateImageBehavior = false;
			this.ListBox_Targets.View = System.Windows.Forms.View.Details;
			// 
			// ListBox_Targets_Name
			// 
			this.ListBox_Targets_Name.Text = "Name";
			this.ListBox_Targets_Name.Width = 188;
			// 
			// ListBox_Targets_IP
			// 
			this.ListBox_Targets_IP.Text = "Title IP Address";
			this.ListBox_Targets_IP.Width = 174;
			// 
			// ListBox_Targets_DebugIP
			// 
			this.ListBox_Targets_DebugIP.Text = "Debug Channel IP Address";
			this.ListBox_Targets_DebugIP.Width = 158;
			// 
			// ListBox_Targets_Type
			// 
			this.ListBox_Targets_Type.Text = "Type";
			this.ListBox_Targets_Type.Width = 110;
			// 
			// Tab_Compiling
			// 
			this.Tab_Compiling.AutoScroll = true;
			this.Tab_Compiling.Controls.Add(this.CheckBox_Compiling_AutoMode);
			this.Tab_Compiling.Controls.Add(this.CheckBox_Compiling_VerboseMode);
			this.Tab_Compiling.Controls.Add(this.CheckBox_Compiling_RunWithCookedData);
			this.Tab_Compiling.Controls.Add(this.CheckBox_Compiling_FullRecompile);
			this.Tab_Compiling.Location = new System.Drawing.Point(4, 22);
			this.Tab_Compiling.Name = "Tab_Compiling";
			this.Tab_Compiling.Size = new System.Drawing.Size(683, 805);
			this.Tab_Compiling.TabIndex = 6;
			this.Tab_Compiling.Text = "Compiling";
			this.Tab_Compiling.UseVisualStyleBackColor = true;
			this.Tab_Compiling.Leave += new System.EventHandler(this.Tab_Compiling_Leave);
			this.Tab_Compiling.Enter += new System.EventHandler(this.Tab_Compiling_Enter);
			// 
			// Tab_Help
			// 
			this.Tab_Help.Controls.Add(this.RichTextBox_Help);
			this.Tab_Help.Location = new System.Drawing.Point(4, 22);
			this.Tab_Help.Name = "Tab_Help";
			this.Tab_Help.Padding = new System.Windows.Forms.Padding(3);
			this.Tab_Help.Size = new System.Drawing.Size(683, 805);
			this.Tab_Help.TabIndex = 5;
			this.Tab_Help.Text = "Help";
			this.Tab_Help.UseVisualStyleBackColor = true;
			// 
			// RichTextBox_Help
			// 
			this.RichTextBox_Help.BackColor = System.Drawing.SystemColors.Window;
			this.RichTextBox_Help.Dock = System.Windows.Forms.DockStyle.Fill;
			this.RichTextBox_Help.Location = new System.Drawing.Point(3, 3);
			this.RichTextBox_Help.Name = "RichTextBox_Help";
			this.RichTextBox_Help.ReadOnly = true;
			this.RichTextBox_Help.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Vertical;
			this.RichTextBox_Help.Size = new System.Drawing.Size(677, 799);
			this.RichTextBox_Help.TabIndex = 1;
			this.RichTextBox_Help.Text = "";
			// 
			// OutputWindowView_LogWindow
			// 
			this.OutputWindowView_LogWindow.AutoScroll = true;
			this.OutputWindowView_LogWindow.BackColor = System.Drawing.SystemColors.Window;
			this.OutputWindowView_LogWindow.Cursor = System.Windows.Forms.Cursors.IBeam;
			this.OutputWindowView_LogWindow.Dock = System.Windows.Forms.DockStyle.Fill;
			outputWindowDocument1.Text = "";
			this.OutputWindowView_LogWindow.Document = outputWindowDocument1;
			this.OutputWindowView_LogWindow.Font = new System.Drawing.Font("Courier New", 9F);
			this.OutputWindowView_LogWindow.Location = new System.Drawing.Point(0, 0);
			this.OutputWindowView_LogWindow.Name = "OutputWindowView_LogWindow";
			this.OutputWindowView_LogWindow.Size = new System.Drawing.Size(731, 831);
			this.OutputWindowView_LogWindow.TabIndex = 0;
			// 
			// ToolStrip_Actions
			// 
			this.ToolStrip_Actions.AllowItemReorder = true;
			this.ToolStrip_Actions.Dock = System.Windows.Forms.DockStyle.None;
			this.ToolStrip_Actions.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolStripButton_Launch,
            this.ToolStripSeparator7,
            this.ToolStripButton_Editor,
            this.ToolStripSeparator3,
            this.ToolStripButton_Server,
            this.ToolStripSeparator4,
            this.ToolStripButton_Cook,
            this.ToolStripSeparator5,
            this.ToolStripButton_CompileScript,
            this.ToolStripSeparator6,
            this.ToolStripButton_Sync,
            this.ToolStripSeparator8,
            this.ToolStripButton_Reboot,
            this.ToolStripSeparator10,
            this.toolStripLabel1,
            this.toolStripSeparator11,
            this.ToolStripButton_UnrealProp});
			this.ToolStrip_Actions.Location = new System.Drawing.Point(3, 25);
			this.ToolStrip_Actions.Name = "ToolStrip_Actions";
			this.ToolStrip_Actions.Size = new System.Drawing.Size(484, 25);
			this.ToolStrip_Actions.TabIndex = 34;
			// 
			// ToolStripButton_Launch
			// 
			this.ToolStripButton_Launch.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_Launch.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_Launch.Image")));
			this.ToolStripButton_Launch.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_Launch.Name = "ToolStripButton_Launch";
			this.ToolStripButton_Launch.Size = new System.Drawing.Size(50, 22);
			this.ToolStripButton_Launch.Text = "Launch";
			this.ToolStripButton_Launch.Click += new System.EventHandler(this.OnLoadMapClick);
			// 
			// ToolStripSeparator7
			// 
			this.ToolStripSeparator7.Name = "ToolStripSeparator7";
			this.ToolStripSeparator7.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_Editor
			// 
			this.ToolStripButton_Editor.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_Editor.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_Editor.Image")));
			this.ToolStripButton_Editor.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_Editor.Name = "ToolStripButton_Editor";
			this.ToolStripButton_Editor.Size = new System.Drawing.Size(42, 22);
			this.ToolStripButton_Editor.Text = "Editor";
			this.ToolStripButton_Editor.Click += new System.EventHandler(this.OnEditorClick);
			// 
			// ToolStripSeparator3
			// 
			this.ToolStripSeparator3.Name = "ToolStripSeparator3";
			this.ToolStripSeparator3.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_Server
			// 
			this.ToolStripButton_Server.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_Server.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolStripButton_AddClient,
            this.ToolStripButton_KillAll});
			this.ToolStripButton_Server.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_Server.Image")));
			this.ToolStripButton_Server.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_Server.Name = "ToolStripButton_Server";
			this.ToolStripButton_Server.Size = new System.Drawing.Size(55, 22);
			this.ToolStripButton_Server.Text = "Server";
			this.ToolStripButton_Server.ButtonClick += new System.EventHandler(this.OnServerClientClick);
			// 
			// ToolStripButton_AddClient
			// 
			this.ToolStripButton_AddClient.Name = "ToolStripButton_AddClient";
			this.ToolStripButton_AddClient.Size = new System.Drawing.Size(130, 22);
			this.ToolStripButton_AddClient.Text = "Add Client";
			this.ToolStripButton_AddClient.Click += new System.EventHandler(this.OnClientClick);
			// 
			// ToolStripButton_KillAll
			// 
			this.ToolStripButton_KillAll.Name = "ToolStripButton_KillAll";
			this.ToolStripButton_KillAll.Size = new System.Drawing.Size(130, 22);
			this.ToolStripButton_KillAll.Text = "Kill All";
			this.ToolStripButton_KillAll.Click += new System.EventHandler(this.OnKillAllClick);
			// 
			// ToolStripSeparator4
			// 
			this.ToolStripSeparator4.MergeAction = System.Windows.Forms.MergeAction.Insert;
			this.ToolStripSeparator4.Name = "ToolStripSeparator4";
			this.ToolStripSeparator4.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_Cook
			// 
			this.ToolStripButton_Cook.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_Cook.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolStripButton_CookGlobalShaders,
            this.ToolStripButton_FullRecook,
            this.ToolStripButton_CookIntsOnly,
            this.ToolStripButton_CookAllMaps});
			this.ToolStripButton_Cook.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_Cook.Image")));
			this.ToolStripButton_Cook.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_Cook.Name = "ToolStripButton_Cook";
			this.ToolStripButton_Cook.Size = new System.Drawing.Size(51, 22);
			this.ToolStripButton_Cook.Text = "Cook";
			this.ToolStripButton_Cook.ButtonClick += new System.EventHandler(this.OnStartCookingClick);
			// 
			// ToolStripButton_CookGlobalShaders
			// 
			this.ToolStripButton_CookGlobalShaders.Name = "ToolStripButton_CookGlobalShaders";
			this.ToolStripButton_CookGlobalShaders.Size = new System.Drawing.Size(180, 22);
			this.ToolStripButton_CookGlobalShaders.Text = "Global Shaders Only";
			this.ToolStripButton_CookGlobalShaders.Click += new System.EventHandler(this.ToolStripButton_CookGlobalShaders_Click);
			// 
			// ToolStripButton_FullRecook
			// 
			this.ToolStripButton_FullRecook.Name = "ToolStripButton_FullRecook";
			this.ToolStripButton_FullRecook.Size = new System.Drawing.Size(180, 22);
			this.ToolStripButton_FullRecook.Text = "Full Recook";
			this.ToolStripButton_FullRecook.Click += new System.EventHandler(this.ToolStripButton_FullRecook_Click);
			// 
			// ToolStripButton_CookIntsOnly
			// 
			this.ToolStripButton_CookIntsOnly.Name = "ToolStripButton_CookIntsOnly";
			this.ToolStripButton_CookIntsOnly.Size = new System.Drawing.Size(180, 22);
			this.ToolStripButton_CookIntsOnly.Text = "Cook .ini/.int\'s Only";
			this.ToolStripButton_CookIntsOnly.Click += new System.EventHandler(this.ToolStripButton_CookIntsOnly_Click);
			// 
			// ToolStripButton_CookAllMaps
			// 
			this.ToolStripButton_CookAllMaps.Name = "ToolStripButton_CookAllMaps";
			this.ToolStripButton_CookAllMaps.Size = new System.Drawing.Size(180, 22);
			this.ToolStripButton_CookAllMaps.Text = "Cook All Maps";
			this.ToolStripButton_CookAllMaps.Click += new System.EventHandler(this.ToolStripButton_CookAllMaps_Click);
			// 
			// ToolStripSeparator5
			// 
			this.ToolStripSeparator5.Name = "ToolStripSeparator5";
			this.ToolStripSeparator5.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_CompileScript
			// 
			this.ToolStripButton_CompileScript.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_CompileScript.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_CompileScript.Image")));
			this.ToolStripButton_CompileScript.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_CompileScript.Name = "ToolStripButton_CompileScript";
			this.ToolStripButton_CompileScript.Size = new System.Drawing.Size(40, 22);
			this.ToolStripButton_CompileScript.Text = "Make";
			this.ToolStripButton_CompileScript.ToolTipText = "Compiles script for the current game and configuration.";
			this.ToolStripButton_CompileScript.Click += new System.EventHandler(this.OnCompileClick);
			// 
			// ToolStripSeparator6
			// 
			this.ToolStripSeparator6.Name = "ToolStripSeparator6";
			this.ToolStripSeparator6.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_Sync
			// 
			this.ToolStripButton_Sync.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_Sync.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_Sync.Image")));
			this.ToolStripButton_Sync.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_Sync.Name = "ToolStripButton_Sync";
			this.ToolStripButton_Sync.Size = new System.Drawing.Size(36, 22);
			this.ToolStripButton_Sync.Text = "Sync";
			this.ToolStripButton_Sync.ToolTipText = "Sync targets";
			this.ToolStripButton_Sync.Click += new System.EventHandler(this.OnCopyToTargetsClick);
			// 
			// ToolStripSeparator8
			// 
			this.ToolStripSeparator8.Name = "ToolStripSeparator8";
			this.ToolStripSeparator8.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_Reboot
			// 
			this.ToolStripButton_Reboot.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_Reboot.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_Reboot.Image")));
			this.ToolStripButton_Reboot.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_Reboot.Name = "ToolStripButton_Reboot";
			this.ToolStripButton_Reboot.Size = new System.Drawing.Size(49, 22);
			this.ToolStripButton_Reboot.Text = "Reboot";
			this.ToolStripButton_Reboot.ToolTipText = "Reboot targets";
			this.ToolStripButton_Reboot.Click += new System.EventHandler(this.OnRebootTargetsClick);
			// 
			// ToolStripSeparator10
			// 
			this.ToolStripSeparator10.Name = "ToolStripSeparator10";
			this.ToolStripSeparator10.Size = new System.Drawing.Size(6, 25);
			// 
			// toolStripLabel1
			// 
			this.toolStripLabel1.Name = "toolStripLabel1";
			this.toolStripLabel1.Size = new System.Drawing.Size(31, 22);
			this.toolStripLabel1.Text = "        ";
			// 
			// toolStripSeparator11
			// 
			this.toolStripSeparator11.Name = "toolStripSeparator11";
			this.toolStripSeparator11.Size = new System.Drawing.Size(6, 25);
			// 
			// ToolStripButton_UnrealProp
			// 
			this.ToolStripButton_UnrealProp.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
			this.ToolStripButton_UnrealProp.Image = ((System.Drawing.Image)(resources.GetObject("ToolStripButton_UnrealProp.Image")));
			this.ToolStripButton_UnrealProp.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.ToolStripButton_UnrealProp.Name = "ToolStripButton_UnrealProp";
			this.ToolStripButton_UnrealProp.Size = new System.Drawing.Size(70, 22);
			this.ToolStripButton_UnrealProp.Text = "UnrealProp";
			this.ToolStripButton_UnrealProp.ToolTipText = "Send to UnrealProp";
			this.ToolStripButton_UnrealProp.Click += new System.EventHandler(this.OnUnrealPropClick);
			// 
			// Cooking_MapToCookToolTipTimer
			// 
			this.Cooking_MapToCookToolTipTimer.Interval = 750;
			this.Cooking_MapToCookToolTipTimer.Tick += new System.EventHandler(this.Cooking_MapToCookToolTipTimer_Tick);
			// 
			// NotifyIcon_CommandletFinished
			// 
			this.NotifyIcon_CommandletFinished.BalloonTipIcon = System.Windows.Forms.ToolTipIcon.Info;
			this.NotifyIcon_CommandletFinished.BalloonTipTitle = "UnrealFrontend Notification";
			this.NotifyIcon_CommandletFinished.Icon = ((System.Drawing.Icon)(resources.GetObject("NotifyIcon_CommandletFinished.Icon")));
			this.NotifyIcon_CommandletFinished.Text = "Commandlet Notification";
			this.NotifyIcon_CommandletFinished.BalloonTipClosed += new System.EventHandler(this.NotifyIcon_CommandletFinished_BalloonTipClosed);
			this.NotifyIcon_CommandletFinished.BalloonTipClicked += new System.EventHandler(this.NotifyIcon_CommandletFinished_BalloonTipClicked);
			// 
			// UnrealFrontendWindow
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(1424, 929);
			this.Controls.Add(this.toolStripContainer1);
			this.Controls.Add(this.statusStrip1);
			this.Controls.Add(this.menuStrip1);
			this.DoubleBuffered = true;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.KeyPreview = true;
			this.MinimumSize = new System.Drawing.Size(727, 393);
			this.Name = "UnrealFrontendWindow";
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Show;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
			this.Text = "Unreal Frontend";
			this.Load += new System.EventHandler(this.OnUnrealFrontendWindowLoad);
			this.Closing += new System.ComponentModel.CancelEventHandler(this.CookFrontEnd_Closing);
			this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.OnUnrealFrontendWindowKeyDown);
			((System.ComponentModel.ISupportInitialize)(this.NumericUpDown_Game_NumClients)).EndInit();
			this.ToolStrip_GlobalSettings.ResumeLayout(false);
			this.ToolStrip_GlobalSettings.PerformLayout();
			this.menuStrip1.ResumeLayout(false);
			this.menuStrip1.PerformLayout();
			this.statusStrip1.ResumeLayout(false);
			this.statusStrip1.PerformLayout();
			this.toolStripContainer1.ContentPanel.ResumeLayout(false);
			this.toolStripContainer1.TopToolStripPanel.ResumeLayout(false);
			this.toolStripContainer1.TopToolStripPanel.PerformLayout();
			this.toolStripContainer1.ResumeLayout(false);
			this.toolStripContainer1.PerformLayout();
			this.splitContainer1.Panel1.ResumeLayout(false);
			this.splitContainer1.Panel2.ResumeLayout(false);
			this.splitContainer1.ResumeLayout(false);
			this.TabControl_Main.ResumeLayout(false);
			this.Tab_Game.ResumeLayout(false);
			this.Tab_Game.PerformLayout();
			this.GroupBox_Game_Xbox360.ResumeLayout(false);
			this.GroupBox_Game_Xbox360.PerformLayout();
			this.GroupBox_Game_GoW2.ResumeLayout(false);
			this.GroupBox_Game_GoW2.PerformLayout();
			this.GroupBox_Game_PC.ResumeLayout(false);
			this.GroupBox_Game_PC.PerformLayout();
			this.GroupBox_Game_Common.ResumeLayout(false);
			this.GroupBox_Game_Common.PerformLayout();
			this.Tab_Cooking.ResumeLayout(false);
			this.Tab_Cooking.PerformLayout();
			this.GroupBox_Cooking_Game_Options.ResumeLayout(false);
			this.GroupBox_Cooking_Game_Options.PerformLayout();
			this.GroupBox_Cooking_Options.ResumeLayout(false);
			this.GroupBox_Cooking_Options.PerformLayout();
			this.Tab_ConsoleSetup.ResumeLayout(false);
			this.Tab_ConsoleSetup.PerformLayout();
			this.Tab_Compiling.ResumeLayout(false);
			this.Tab_Compiling.PerformLayout();
			this.Tab_Help.ResumeLayout(false);
			this.ToolStrip_Actions.ResumeLayout(false);
			this.ToolStrip_Actions.PerformLayout();
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
			try
			{
				Application.EnableVisualStyles();

				// force loading of platform dll's here
				ConsoleInterface.DLLInterface.LoadPlatforms(ConsoleInterface.PlatformType.All);

				UnrealFrontendWindow UnrealFrontend = new UnrealFrontendWindow();
				if(!UnrealFrontend.Init())
				{
					return;
				}

				Application.Run(UnrealFrontend);
			}
			finally
			{
				Properties.Settings.Default.Save();
			}
		}

		/// <summary>
		/// Constructor.
		/// </summary>
		public UnrealFrontendWindow()
		{
			// Required for Windows Form Designer support
			InitializeComponent();

			OutputWindowView_LogWindow.Document = mOutputDocument;

			ComboBox_Configuration.Items.Add(Configuration.Debug);
			ComboBox_Configuration.Items.Add(Configuration.Release);
			ComboBox_Configuration.Items.Add(Configuration.FinalRelease);
			ComboBox_Configuration.Items.Add(Configuration.FinalReleaseDebug);

			ComboBox_CookingConfiguration.Items.Add(Configuration.Debug);
			ComboBox_CookingConfiguration.Items.Add(Configuration.Release);
			ComboBox_CookingConfiguration.Items.Add(Configuration.FinalRelease);
			ComboBox_CookingConfiguration.Items.Add(Configuration.FinalReleaseDebug);

			ComboBox_Game_ServerType.Items.Add(ServerType.Listen);
			ComboBox_Game_ServerType.Items.Add(ServerType.Dedicated);

			foreach(ConsoleInterface.Platform CurPlatform in ConsoleInterface.DLLInterface.Platforms)
			{
				ComboBox_Platform.Items.Add(CurPlatform);
			}

			try
			{
				if(Properties.Settings.Default.ConsoleTargets_ConsoleBaseDir == null || Properties.Settings.Default.ConsoleTargets_ConsoleBaseDir.Trim().Length == 0)
				{
					string FullPath = Path.GetFullPath("..");
					FullPath = FullPath.TrimEnd(new char[] { '\\' });
			
					int FinalSeparator = FullPath.LastIndexOf('\\');

					if(FinalSeparator != -1)
					{
						Properties.Settings.Default.ConsoleTargets_ConsoleBaseDir = FullPath.Substring(FinalSeparator + 1);
					}
				}
			}
			catch(Exception ex)
			{
				Properties.Settings.Default.ConsoleTargets_ConsoleBaseDir = "UnrealEngine3";
				System.Diagnostics.Debug.WriteLine(ex.ToString());
			}

			// Add the extensions for the various game types
			MapExtensionDictionary.Add("ExampleGame", ".umap");
			MapExtensionDictionary.Add("GearGame", ".gear");
			MapExtensionDictionary.Add("UTGame", ".ut3");
			MapExtensionDictionary.Add("WarGame", ".war");

			LoadSettings();

			// add pretty text to the help text
			Font DefaultFont = RichTextBox_Help.SelectionFont;
			Font BoldFont = new Font(DefaultFont.FontFamily, DefaultFont.Size, FontStyle.Bold);

			RichTextBox_Help.SelectionFont = BoldFont;
			RichTextBox_Help.AppendText("Info:\r\n");

			RichTextBox_Help.SelectionFont = DefaultFont;
			RichTextBox_Help.AppendText("This program can be used as a frontend for controlling many aspects of the Unreal Engine. It " +
				"can launch your game on the PC or a console, run multiple clients for testing multiplayer games locally, edit maps, " +
				"compile script code, cook data for PC or console, reboot consoles, etc.\r\nCommandlets (cooking, script compiling) " +
				"will output to the text box on the right.\r\n\r\n");

			RichTextBox_Help.SelectionFont = BoldFont;
			RichTextBox_Help.AppendText("Shortcuts:\r\n");

			RichTextBox_Help.SelectionFont = DefaultFont;
			RichTextBox_Help.AppendText("  F1-F6 - Quickly jump between tabs.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-L - Loads SP map.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-E - Launches editor.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-S - Starts server.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-K - Cooks data.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-M - Compiles script code.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-Y - Syncs data to targets.\r\n");
			RichTextBox_Help.AppendText("  Ctrl-R - Reboots targets.\r\n\r\n");

			RichTextBox_Help.SelectionFont = BoldFont;
			RichTextBox_Help.AppendText("More Help:\r\n");

			RichTextBox_Help.SelectionFont = DefaultFont;
			RichTextBox_Help.AppendText("Most controls have tooltips to explain what they do. Also, visit https://udn.epicgames.com/Three/UnrealFrontend " +
				"for more information.\r\n\r\n");

			ListBox_Targets.ItemCheck += new ItemCheckEventHandler(ListBox_Targets_ItemCheck);

			ComboBox_Cooking_MapsToCook.TextChanged += new EventHandler(TextBox_MapsToCook_TextChanged);
			ComboBox_Cooking_MapsToCook.KeyPress += new KeyPressEventHandler(TextBox_MapsToCook_KeyPress);
			ComboBox_Cooking_MapsToCook.LostFocus += new EventHandler(TextBox_MapsToCook_LostFocus);
			ComboBox_MapToPlay.LostFocus += new EventHandler(TextBox_MapToPlay_LostFocus);
			//ComboBox_MapToPlay.KeyPress += new KeyPressEventHandler(TextBox_MapToPlay_KeyPress);

			mOutputTimer = new System.Threading.Timer(new TimerCallback(this.OnOutputTimer), null, 250, 250);
		}

		/// <summary>
		/// Event handler for when TextBox_MapsToCook loses its input focus.
		/// </summary>
		/// <param name="sender">The object that generated the event.</param>
		/// <param name="e">Information about the event.</param>
		void TextBox_MapsToCook_LostFocus(object sender, EventArgs e)
		{
			Cooking_MapToCookToolTipTimer.Stop();
			CookMapToolTip.Hide(ComboBox_Cooking_MapsToCook.Parent);
		}

		/// <summary>
		/// Event handler for when TextBox_MapToPlay loses its input focus.
		/// </summary>
		/// <param name="sender">The object that generated the event.</param>
		/// <param name="e">Information about the event.</param>
		void TextBox_MapToPlay_LostFocus(object sender, EventArgs e)
		{
			CookMapToolTip.Hide(this);
		}

		/// <summary>
		/// Event handler for when a key has been pressed in the text box that holds onto the list of maps that are to be cooked.
		/// </summary>
		/// <param name="sender">The text box instance.</param>
		/// <param name="e">Information about the event.</param>
		void TextBox_MapsToCook_KeyPress(object sender, KeyPressEventArgs e)
		{
			// When enter is hit fill in the text box with all maps that fit the current search criteria
			if(e.KeyChar == '\r')
			{
				// stop the timer if it's running because building the list can potentially take more than a second
				Cooking_MapToCookToolTipTimer.Stop();

				Cursor CurCursor = this.Cursor;
				this.Cursor = Cursors.WaitCursor;

				BuildMapsToCookString();

				// BuildMapsToCookString() changes the maps to cook text which starts the timer so stop it again
				Cooking_MapToCookToolTipTimer.Stop();

				// BuildMapsToCookString() shows the tool tip so explicitly hide it
				CookMapToolTip.Hide(this);

				this.Cursor = CurCursor;
			}
		}

		/// <summary>
		/// Builds the string of maps to be cooked based on a set of filters.
		/// </summary>
		private void BuildMapsToCookString()
		{
			try
			{
				string DirContent = "..\\" + ComboBox_Game.Text + "\\Content";
				StringBuilder EntryBldr = new StringBuilder();

				string[] SubStrings = ComboBox_Cooking_MapsToCook.Text.Trim().Split(' ');

				for(int i = 0; i < SubStrings.Length; ++i)
				{
					// Check to see if a wild card is present, if one isn't then append our own
					string SearchString = SubStrings[i];

					if(!SearchString.Contains("*") || !SearchString.Contains("?"))
					{
						SearchString += "*";
					}

					// Append the extension of the current game's maps if an extension isn't already provided
					string Extension;
					if(MapExtensionDictionary.TryGetValue(ComboBox_Game.Text, out Extension) && !Path.HasExtension(SearchString))
					{
						SearchString += Extension;
					}

					if(Directory.Exists(DirContent))
					{
						BuildCookMapEntriesString(EntryBldr, DirContent, SearchString);
					}
				}

				// Get rid of trailing space
				if(EntryBldr.Length > 0)
				{
					EntryBldr.Length = EntryBldr.Length - 1;
				}

				ComboBox_Cooking_MapsToCook.Text = EntryBldr.ToString();
				ComboBox_Cooking_MapsToCook.SelectionStart = EntryBldr.Length;
			}
			catch(Exception Error)
			{
				System.Diagnostics.Trace.WriteLine(Error.ToString());
			}
		}

		/// <summary>
		/// Builds the list of maps to be cooked.
		/// </summary>
		/// <param name="EntryBldr">The <see cref="System.StringBuilder"/> containing the resulting list of maps.</param>
		/// <param name="Dir">The directory to search for files in.</param>
		/// <param name="SearchString">The string used to filter files in <see cref="Dir"/>.</param>
		void BuildCookMapEntriesString(StringBuilder EntryBldr, string Dir, string SearchString)
		{
			string[] Files = Directory.GetFiles(Dir, SearchString, SearchOption.AllDirectories);

			foreach(string File in Files)
			{
				if(File.IndexOf("Autosaves", StringComparison.OrdinalIgnoreCase) == -1)
				{
					EntryBldr.Append(Path.GetFileNameWithoutExtension(File));
					EntryBldr.Append(' ');
				}
			}
		}

		/// <summary>
		/// Event handler for when the text box containing the maps to be cooked has been modified.
		/// </summary>
		/// <param name="sender">The text box that has been modified.</param>
		/// <param name="e">Information about the event.</param>
		void TextBox_MapsToCook_TextChanged(object sender, EventArgs e)
		{
			if(!Cooking_MapToCookToolTipTimer.Enabled)
			{
				Cooking_MapToCookToolTipTimer.Start();
			}
			else
			{
				// restart the timer
				Cooking_MapToCookToolTipTimer.Stop();
				Cooking_MapToCookToolTipTimer.Start();
			}

			UpdateMapsToCook();
		}

		/// <summary>
		/// Event handler for when the text box containing the map to be played has been modified.
		/// </summary>
		/// <param name="sender">The text box that has been modified.</param>
		/// <param name="e">Information about the event.</param>
		void TextBox_MapToPlay_TextChanged(object sender, EventArgs e)
		{
			if(ComboBox_MapToPlay.Focused)
			{
				//string[] Filters = new string[] { ComboBox_MapToPlay.Text };

				//string Msg = BuildCookMapToolTip(Filters);

				//CookMapToolTip.Show(Msg, ComboBox_MapToPlay, ComboBox_MapToPlay.Width, ComboBox_MapToPlay.Height, 10000);
				UpdateMapToPlay();
			}

			UpdateInfoText();
		}

		/// <summary>
		/// Creates a tool tip for showing all available maps for the current game type.
		/// </summary>
		private string BuildCookMapToolTip(string[] Filters)
		{
			try
			{
				string DirContent = "..\\" + ComboBox_Game.Text + "\\Content";
				StringBuilder ToolTipBldr = new StringBuilder();

				for(int i = 0; i < Filters.Length; ++i)
				{
					// Check to see if a wild card is present, if one isn't then append our own
					string SearchString = Filters[i];

					if(!SearchString.EndsWith("*", StringComparison.OrdinalIgnoreCase) && !SearchString.EndsWith("?", StringComparison.OrdinalIgnoreCase))
					{
						SearchString += "*";
					}

					// Append the extension of the current game's maps if an extension isn't already provided
					string Extension;
					if(MapExtensionDictionary.TryGetValue(ComboBox_Game.Text, out Extension) && !Path.HasExtension(SearchString))
					{
						SearchString += Extension;
					}

					if(Directory.Exists(DirContent))
					{
						BuildCookMapToolTipString(ToolTipBldr, DirContent, SearchString);
					}
				}

				return ToolTipBldr.ToString();
			}
			catch(Exception Error)
			{
				System.Diagnostics.Trace.WriteLine(Error.ToString());
			}

			return "";
		}

		/// <summary>
		/// Builds the tool tip string used for displaying all maps that fit the supplied search string when typing in the name of a map to cook.
		/// </summary>
		/// <param name="ToolTipBldr">The <see cref="System.StringBuilder"/> object containing the resulting tool tip string.</param>
		/// <param name="Dir">The directory to search for files in.</param>
		/// <param name="SearchString">The string used to filter the files in <see cref="Dir"/>.</param>
		void BuildCookMapToolTipString(StringBuilder ToolTipBldr, string Dir, string SearchString)
		{
			string[] Files = Directory.GetFiles(Dir, SearchString, SearchOption.AllDirectories);

			foreach(string File in Files)
			{
				string FullPath = Path.GetFullPath(File);

				if(FullPath.IndexOf("Autosaves", StringComparison.OrdinalIgnoreCase) == -1)
				{
					ToolTipBldr.Append(FullPath);
					ToolTipBldr.Append(Environment.NewLine);
				}
			}
		}

		/// <summary>
		/// Handles the ItemCheckEvent for the target list box and updates the state of the current platform's targets.
		/// </summary>
		/// <param name="sender">The instance that triggered the event.</param>
		/// <param name="e">Information about the event.</param>
		void ListBox_Targets_ItemCheck(object sender, ItemCheckEventArgs e)
		{
			Dictionary<string, bool> CurrentPlatform;
			bool bChecked = e.NewValue == CheckState.Checked;

			//get the current platform
			if(PlatformDefaults.TryGetValue(ComboBox_Platform.Text, out CurrentPlatform))
			{
				//get the current target
				if(CurrentPlatform.ContainsKey(ListBox_Targets.Items[e.Index].ToString()))
				{
					//update the target's state
					CurrentPlatform[ListBox_Targets.Items[e.Index].ToString()] = bChecked;
				}
			}
		}

		/// <summary>
		/// Updates the list of platform targets from a cached list.
		/// </summary>
		/// <param name="Platform">The platform the targets belong to.</param>
		private void UpdatePlatformTargetList(string Platform)
		{
			if(Platform == null)
			{
				throw new ArgumentNullException("Platform");
			}

			// save the list of checked targets
			PlatformTargetCollection SavedTargets = PlatformTargets;

			List<string> TargetList;
			if(!SavedTargets.TryGetTargets(Platform, out TargetList))
			{
				SavedTargets[Platform] = TargetList = new List<string>();
			}

			TargetList.Clear();

			foreach(TargetListViewItem CurItem in ListBox_Targets.Items)
			{
				if(CurItem.Checked)
				{
					TargetList.Add((string)CurItem.Tag);
				}
			}
		}

		/// <summary>
		/// Called when the UFE window has loaded.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnUnrealFrontendWindowLoad(object sender, EventArgs e)
		{
			string[] cmdparms = Environment.GetCommandLineArgs();

			if(cmdparms.Length == 2 && File.Exists(cmdparms[1]))
			{
				ImportMapList(cmdparms[1]);
				StartCommandlet(CommandletCategory.Cooking, CommandletAction.Cooking_CompileScript, CommandletAction.Cooking);
			}
		}

		/// <summary>
		/// Reads the supported game type and configuration settings. Saves a
		/// set of defaults if missing
		/// </summary>
		protected bool LoadCookerConfigAndMakeGameOptions()
		{
			// Read the global settings XML file
			CookerSettings CookerCfg = ReadCookerSettings();
			if(CookerCfg == null)
			{
				MessageBox.Show("Failed to find the UnrealFrontend_cfg.xml file, which is required to run.", "Failed to find settings file", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return (false);
			}
			else
			{
				// get the path to the directory where we will store settings
				SettingsPath = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments) + "\\My Games\\" + CookerCfg.UserSettingsDirectory;

				// make sure it exists
				Directory.CreateDirectory(SettingsPath);
				Directory.CreateDirectory(SettingsPath + "\\Binaries");

				// PC is always usable
				//if( CookerTools.Activate("PC") )
				//{
				//    UsablePlatforms.Add( "PC" );
				//}

				// query for support of known platforms, and add to the list of good platforms
				//foreach( string PlatformName in CookerCfg.Platforms )
				//{
				//    if( CookerTools.Activate( PlatformName ) )
				//    {
				//        UsablePlatforms.Add( PlatformName );
				//    }
				//}

				ComboBox_Game.Items.AddRange(CookerCfg.Games);
				ComboBox_Game.SelectedIndex = 0;

				//ComboBox_Platform.Items.AddRange(UsablePlatforms.ToArray());
				//ComboBox_Platform.SelectedIndex = 0;

				bool bFound = false;
				for(int i = 0; i < ComboBox_Game.Items.Count; ++i)
				{
					if(ComboBox_Game.Items[i].Equals(Properties.Settings.Default.SelectedGameName))
					{
						bFound = true;
						ComboBox_Game.SelectedIndex = i;
						break;
					}
				}

				if(!bFound)
				{
					Properties.Settings.Default.SelectedGameName = ComboBox_Game.SelectedItem.ToString();
				}


				bFound = false;

				//for(int i = 0; i < ComboBox_Platform.Items.Count; ++i)
				//{
				//    if(ComboBox_Platform.Items[i].Equals(Properties.Settings.Default.SelectedPlatform))
				//    {
				//        bFound = true;
				//        ComboBox_Platform.SelectedIndex = i;
				//        break;
				//    }
				//}

				if(!bFound)
				{
					Properties.Settings.Default.SelectedGameName = ComboBox_Platform.SelectedItem.ToString();
				}
			}
			return (true);
		}

		/// <summary>
		/// Updates mod info for the ps3.
		/// </summary>
		private void UpdatePS3Misc()
		{
			bool bSupportsPS3 = false;
			bool bSupportsPS3Mod = false;
			foreach(string Platform in UsablePlatforms)
			{
				string Lower = Platform.ToLower();
				if(Lower == "ps3")
				{
					bSupportsPS3 = true;
				}
				else if(Lower == "ps3 mod")
				{
					bSupportsPS3Mod = true;
				}
			}

			if(bSupportsPS3)
			{
				ModNameDialog.Title = "Mod name";
				ModNameDialog.Caption = "Please enter the name of this mod. Any mod with the same name will be overwritten on the PS3. Max 127 characters, no symbols or spaces.";
			}
			else
			{
				if(bSupportsPS3Mod)
				{
					ModNameDialog.Title = "Mod name";
					ModNameDialog.Caption = "Please enter the name of this mod. Any mod with the same name will be overwritten on the PS3. Max 127 characters, no symbols or spaces.";
				}
			}
		}

		/// <summary>
		/// Initializes the UFE.
		/// </summary>
		/// <returns>True if the function succeeded.</returns>
		public bool Init()
		{
			// initialize the helper cooker tools
			//CookerTools = new CookerToolsClass(this);

			// Read the application settings and set up some game options
			if(!LoadCookerConfigAndMakeGameOptions())
			{
				return (false);
			}

			// kick off some updates
			ProcessGameNameChanged();

			UpdatePS3Misc();

			UpdateInfoText();

			return true;
		}


		#region Shutdown

		/// <summary>
		/// Helper function for converting combo box items into a string array.
		/// </summary>
		/// <param name="CBox">The combo box whose items are to be converted.</param>
		/// <returns>An array of strings.</returns>
		string[] ComboBoxItemsToStringArray(ComboBox CBox)
		{
			List<string> Items = new List<string>();
			foreach(string CurMap in CBox.Items)
			{
				Items.Add(CurMap);
			}

			return Items.ToArray();
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose(bool bDisposing)
		{
			if(bDisposing)
			{
				if(mOutputTimer != null)
				{
					mOutputTimer.Dispose();
					mOutputTimer = null;
				}

				if(components != null)
				{
					components.Dispose();
				}
			}

			base.Dispose(bDisposing);
		}

		/// <summary>
		/// Event handler for when the UFE is closing.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void CookFrontEnd_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			if(CommandletProcess != null && !CommandletProcess.HasExited)
			{
				if(!CheckedStopCommandlet())
				{
					// Don't exit!
					e.Cancel = true;
				}
			}

			if(e.Cancel != true)
			{
				WriteSettings();

				foreach(ConsoleInterface.Platform CurPlatform in ComboBox_Platform.Items)
				{
					CurPlatform.Dispose();
				}

				ComboBox_Platform.Items.Clear();
			}
		}

		#endregion

		#region Process running helpers (PC/console)

		/// <summary>
		/// Calculate the name of the PC executable for commandlets and PC game
		/// </summary>
		/// <param name="bCommandlet">True if this is a path for a commandlet.</param>
		/// <returns></returns>
		private string GetExecutablePath(bool bCommandlet)
		{
			// Figure out executable path.
			string Executable = string.Empty;
			string Selected = GetConsoleConfigurationString((Configuration)(bCommandlet ? ComboBox_CookingConfiguration.SelectedItem : ComboBox_Configuration.SelectedItem));

			if(CheckBox_Game_OverrideExecutable.Checked)
			{
				Executable = TextBox_Game_ExecutableNameOverride.Text;
			}
			else
			{
				if(Selected == "Debug")
				{
					Executable = "Debug-" + ComboBox_Game.Text + ".exe";
				}
				else if(Selected == "Debug-G4WLive")
				{
					Executable = "Debug-" + ComboBox_Game.Text + "-G4WLive.exe";
				}
				else if(Selected == "Release-G4WLive")
				{
					Executable = ComboBox_Game.Text + "-G4WLive.exe";
				}
				else
				{
					// Gets Release and ReleaseLTCG
					Executable = ComboBox_Game.Text + ".exe";
				}
			}

			return Executable;
		}

		/// <summary>
		/// Start up the given executable
		/// </summary>
		/// <param name="Executable">The string name of the executable to run.</param>
		/// <param name="CommandLIne">Any command line parameters to pass to the program.</param>
		/// <param name="bOutputToTextBox">Specifying true will output </param>
		/// <returns>The running process if successful, null on failure</returns>
		private Process ExecuteProgram(string Executable, string CommandLine, bool bOutputToTextBox)
		{
			ProcessStartInfo StartInfo = new ProcessStartInfo();
			// Prepare a ProcessStart structure 
			StartInfo.FileName = Executable;
			StartInfo.WorkingDirectory = Directory.GetCurrentDirectory();
			StartInfo.Arguments = CommandLine;

			Process NewProcess = null;
			if(bOutputToTextBox)
			{
				// Redirect the output.
				StartInfo.UseShellExecute = false;
				StartInfo.RedirectStandardOutput = false;
				StartInfo.RedirectStandardError = false;
				StartInfo.CreateNoWindow = true;

				AddLine(Color.OrangeRed, "Running: " + Executable + " " + CommandLine);
			}

			// Spawn the process
			// Try to start the process, handling thrown exceptions as a failure.
			try
			{
				NewProcess = Process.Start(StartInfo);
			}
			catch
			{
				NewProcess = null;
			}

			return NewProcess;
		}

		/// <summary>
		/// This runs the PC game for non-commandlets (editor, game, etc)
		/// </summary>
		/// <param name="InitialCmdLine"></param>
		/// <param name="PostCmdLine"></param>
		/// <param name="ConfigType"></param>
		private void LaunchApp(string InitialCmdLine, string PostCmdLine, ECurrentConfig ConfigType, bool bForcePC)
		{
			// put together the final commandline (and generate exec file
			string CmdLine = GetFinalURL(InitialCmdLine, PostCmdLine, ConfigType, true);

			if(bForcePC || ComboBox_Platform.Text == "PC")
			{
				Process NewProcess = ExecuteProgram(GetExecutablePath(false), CmdLine, false);
				if(NewProcess != null)
				{
					Processes.Add(NewProcess);
				}
				else
				{
					MessageBox.Show("Failed to launch game executable (" + GetExecutablePath(false) + ")", "Failed to launch", MessageBoxButtons.OK, MessageBoxIcon.Error);
				}
			}
			else
			{
				CheckedReboot(true, CmdLine);
			}
		}

		/// <summary>
		/// Converts a config enumeration into its string representation.
		/// </summary>
		/// <param name="Config">The enumberation to be converted.</param>
		/// <returns>The appropriate string representation of the supplied enumeration.</returns>
		private string GetConsoleConfigurationString(Configuration Config)
		{
			string RetVal = string.Empty;

			switch(Config)
			{
				case Configuration.Debug:
					{
						RetVal = "Debug";
						break;
					}
				case Configuration.Release:
					{
						RetVal = "Release";
						break;
					}
				case Configuration.FinalRelease:
					{
						RetVal = "ReleaseLTCG";
						break;
					}
				case Configuration.FinalReleaseDebug:
					{
						RetVal = "ReleaseLTCG-DebugConsole";
						break;
					}
			}

			return RetVal;
		}

		/// <summary>
		/// Writes a commandlet event string to the output window.
		/// </summary>
		/// <param name="Clr">The color of the line.</param>
		/// <param name="Message">The message to output.</param>
		private void WriteCommandletEvent(Color? Clr, string Message)
		{
			AddLine(Clr, "\r\n[{0}] {1}\r\n", Message, DateTime.Now.ToString("MMMM d, h:m tt"));
		}

		/// <summary>
		/// Reboot and optionally run the game on a console
		/// </summary>
		/// <param name="bShouldRunGame"></param>
		/// <param name="CommandLine"></param>
		private void CheckedReboot(bool bShouldRunGame, string CommandLine)
		{
			UpdateConsoleBaseDirectoryHistory();

			if(bShouldRunGame)
			{
				WriteCommandletEvent(Color.Green, string.Format("Launching {0} with command line \'{1}\'", ComboBox_Game.Text, CommandLine));
			}
			else
			{
				WriteCommandletEvent(Color.Green, "REBOOTING");
			}

			// Debug/Release etc
			Configuration CurConfiguration = (Configuration)ComboBox_Configuration.SelectedItem;
			string ConfigStr = GetConsoleConfigurationString(CurConfiguration);

			foreach(TargetListViewItem Target in ListBox_Targets.Items)
			{
				if(Target.Checked)
				{
					string TargetName = Target.Target.Name;

					if(bShouldRunGame)
					{
						string BaseDir = ComboBox_ConsoleTargets_ConsoleBaseDir.Text;

						if(BaseDir.EndsWith("\\"))
						{
							BaseDir = BaseDir.Substring(0, BaseDir.Length - 1);
						}

						bool bIsConnected = Target.Target.IsConnected;

						// need to connect to keep track of console state, this is needed for properly launching UC
						if(!bIsConnected)
						{
							if(!Target.Target.Connect())
							{
								AddLine(Color.Red, "Failed connection attempt with target \'{0}\'!", Target.Target.Name);
								continue;
							}
						}

						if(Target.Target.RebootAndRun(ConfigStr, BaseDir, CheckBox_Game_OverrideExecutable.Checked ? TextBox_Game_ExecutableNameOverride.Text : ComboBox_Game.Text, CommandLine, CheckBox_Game_OverrideExecutable.Checked))
						{
							AddLine(null, "Target \'{0}\' successfully launched game!", TargetName);
							LaunchUnrealConsole(Target.Target);
						}
						else
						{
							AddLine(Color.Red, "Target \'{0}\' failed to launch game!", TargetName);
						}

						if(!bIsConnected)
						{
							Target.Target.Disconnect();
						}
					}
					else
					{
						if(Target.Target.Reboot())
						{
							AddLine(null, "Target \'{0}\' successfully rebooted!", TargetName);
						}
						else
						{
							AddLine(Color.Red, "Target \'{0}\' failed to reboot!", TargetName);
						}
					}
				}
			}
		}

		/// <summary>
		/// Launches a new instance of UnrealConsole for the specified target.
		/// </summary>
		/// <param name="Target">The target for unreal console to connect to.</param>
		private void LaunchUnrealConsole(ConsoleInterface.PlatformTarget Target)
		{
			if(Target == null)
			{
				throw new ArgumentNullException("Target");
			}

			try
			{
				if(UCIpcChannel == null)
				{
					UCIpcChannel = new IpcChannel();
					ChannelServices.RegisterChannel(UCIpcChannel, true);
				}

				string Dir = Path.GetDirectoryName(Assembly.GetEntryAssembly().Location);
				Dir = Path.Combine(Dir, "UnrealConsole.exe");
				string TargetName = string.Empty;

				// everything except the PS3 has an accessible debug channel IP but the PS3 has an always accessible name
				TargetName = Target.ParentPlatform.Type == ConsoleInterface.PlatformType.PS3 ? Target.Name : Target.DebugIPAddress.ToString();

				if(Target.IsConnected && Target.ParentPlatform.Type == ConsoleInterface.PlatformType.Xbox360)
				{
					while(Target.State == ConsoleInterface.TargetState.Rebooting)
					{
						Application.DoEvents();
					}
				}

				// See if there is a UC window already open for this...
				Process[] localByName = Process.GetProcessesByName("UnrealConsole");

				bool bOpenedTab = false;
				if(localByName.Length > 0)
				{
					foreach(Process CurProc in localByName)
					{
						try
						{
							RemoteUCObject RemoteObj = (RemoteUCObject)Activator.GetObject(typeof(RemoteUCObject), string.Format("ipc://{0}/{0}", CurProc.Id.ToString()));

							bOpenedTab = RemoteObj.HasTarget(Target.ParentPlatform.Name, TargetName);

							if(bOpenedTab)
							{
								try
								{
									RemoteObj = (RemoteUCObject)Activator.GetObject(typeof(RemoteUCObject), string.Format("ipc://{0}/{0}", CurProc.Id.ToString()));
									RemoteObj.OpenTarget(Target.ParentPlatform.Name, TargetName, CheckBox_Game_ClearUCWindow.Checked);
								}
								catch(Exception ex)
								{
									string ErrStr = ex.ToString();
									System.Diagnostics.Debug.WriteLine(ErrStr);

									AddLine(Color.Orange, "Warning: Could not open target in UnrealConsole instance \'{0}\'", CurProc.Id.ToString());
								}

								break;
							}
						}
						catch(Exception)
						{
						}
					}
				}

				if(!bOpenedTab)
				{
					bOpenedTab = localByName.Length > 0;

					if(bOpenedTab)
					{
						int Ticks = Environment.TickCount;
						bOpenedTab = false;

						while(!bOpenedTab && Environment.TickCount - Ticks < 10000)
						{
							try
							{
								RemoteUCObject RemoteObj = (RemoteUCObject)Activator.GetObject(typeof(RemoteUCObject), string.Format("ipc://{0}/{0}", localByName[0].Id.ToString()));
								bOpenedTab = RemoteObj.OpenTarget(Target.ParentPlatform.Name, TargetName, CheckBox_Game_ClearUCWindow.Checked);
							}
							catch(Exception)
							{
								bOpenedTab = false;
							}
						}

						if(!bOpenedTab)
						{
							AddLine(Color.Orange, "Warning: Could not open target in UnrealConsole instance \'{0}\'", localByName[0].Id.ToString());
						}
					}

					if(!bOpenedTab)
					{
						ProcessStartInfo Info = new ProcessStartInfo(Dir, string.Format("platform={0} target={1}", Target.ParentPlatform.Name, TargetName));
						Info.CreateNoWindow = false;
						Info.UseShellExecute = true;
						Info.Verb = "open";

						Process.Start(Info).Dispose();
					}
				}

				foreach(Process CurProc in localByName)
				{
					CurProc.Dispose();
				}
			}
			catch(Exception ex)
			{
				string ErrStr = ex.ToString();
				System.Diagnostics.Debug.WriteLine(ErrStr);

				AddLine(Color.Red, ErrStr);
			}
		}

		#endregion

		#region Load/save application settings

		/// <summary>
		/// Loads user application settings.
		/// </summary>
		void LoadSettings()
		{
			// load tool strip settings
			ToolStripManager.LoadSettings(this);

			Properties.Settings Settings = Properties.Settings.Default;

			// global settings
			ComboBox_Configuration.SelectedIndex = Settings.ConfigurationSelectedIndex;
			ComboBox_CookingConfiguration.SelectedIndex = Settings.CookingConfigurationSelectedIndex;

			foreach(object Obj in ComboBox_Game.Items)
			{
				if(Obj.ToString().Equals(Settings.SelectedGameName, StringComparison.OrdinalIgnoreCase))
				{
					ComboBox_Game.SelectedItem = Obj;
					break;
				}
			}

			this.Size = Settings.UFEMainWindowSize;
			this.splitContainer1.SplitterDistance = Settings.UFEWindowSplitterDistance;

			// game settings
			if(Settings.MapToPlayHistory != null)
			{
				ComboBox_MapToPlay.Items.AddRange(Settings.MapToPlayHistory);
			}

			if(Settings.Game_ExtraOptionsHistory != null)
			{
				ComboBox_Game_ExtraOptions.Items.AddRange(Settings.Game_ExtraOptionsHistory);
			}

			CheckBox_Game_UseCookedMap.Checked = Settings.Game_UseCookedMap;
			ComboBox_Game_ExtraOptions.Text = Settings.Game_ExtraOptions;
			TextBox_Game_ExecCommands.Text = Settings.Game_ExecCommands;
			CheckBox_Game_NoSound.Checked = Settings.Game_NoSound;
			CheckBox_Game_NoVSync.Checked = Settings.Game_NoVSync;
			CheckBox_Game_MultiThreaded.Checked = Settings.Game_MultiThreaded;
			NumericUpDown_Game_NumClients.Value = Settings.Game_NumClients;
			ComboBox_Game_ServerType.SelectedIndex = Settings.Game_ServerTypeSelectedIndex;
			ComboBox_Game_Resolution.Text = Settings.Game_Resolution;
			CheckBox_Game_ShowLog.Checked = Settings.Game_ShowLog;
			CheckBox_Game_RemoteControl.Checked = Settings.Game_RemoteControl;
			CheckBox_Game_ClearUCWindow.Checked = Settings.Game_ClearUCWindow;
			CheckBox_Game_GoW2OverrideSPMap.Checked = Settings.Game_GoW2OverrideSPMap;
			CheckBox_Game_EncryptedSockets.Checked = Settings.Game_EncryptedSockets;

			// gears2 specific stuff within the game settings
			try
			{
				using(XmlReader Rdr = XmlReader.Create("..\\GearGame\\Config\\Chapters.xml"))
				{
					XmlSerializer Serializer = new XmlSerializer(typeof(GoW2ChaptersConfig));

					GoW2ChaptersConfig Cfg = (GoW2ChaptersConfig)Serializer.Deserialize(Rdr);

					foreach(GoW2ChapterEntry CurEntry in Cfg.Chapters)
					{
						ComboBox_Game_GoW2SPMapName.Items.Add(CurEntry);
					}

					if(Settings.Game_GoW2SelectedChapterIndex < 0 && ComboBox_Game_GoW2SPMapName.Items.Count > 0)
					{
						Settings.Game_GoW2SelectedChapterIndex = 0;
					}

					ComboBox_Game_GoW2SPMapName.SelectedIndex = Settings.Game_GoW2SelectedChapterIndex;
				}
			}
			catch(Exception ex)
			{
				System.Diagnostics.Debug.WriteLine(ex.ToString());
			}

			//cooking settings
			if(Settings.Cooking_MapsToCookHistory != null)
			{
				ComboBox_Cooking_MapsToCook.Items.AddRange(Settings.Cooking_MapsToCookHistory);
			}

			if(Settings.Cooking_AdditionalOptionsHistory != null)
			{
				ComboBox_Cooking_AdditionalOptions.Items.AddRange(Settings.Cooking_AdditionalOptionsHistory);
			}

			if(Settings.Cooking_LanguagesToCookAndSync != null)
			{
				List<string> SelectedLanguages = new List<string>(Settings.Cooking_LanguagesToCookAndSync);

				foreach(ListViewItem CurItem in ListView_Cooking_Languages.Items)
				{
					CurItem.Checked = SelectedLanguages.Contains(CurItem.Text);
				}
			}

			CheckBox_Cooking_ForceSeekFreeRecook.Checked = Settings.Cooking_ForceSeekFreeRecook;
			CheckBox_Cooking_CookFinalReleaseScript.Checked = Settings.Cooking_FinalReleaseScripts;
			CheckBox_Cooking_ForceSoundRecook.Checked = Settings.Cooking_ForceSoundRecook;
			CheckBox_Cooking_RunLocalGame.Checked = Settings.Cooking_RunLocalGame;
			CheckBox_Cooking_CookMod.Checked = Settings.Cooking_UseSeekFreeData;
			CheckBox_Cooking_DisablePackageCompression.Checked = Settings.Cooking_DisablePackageCompression;
			ComboBox_Cooking_AdditionalOptions.Text = Settings.Cooking_AdditionalOptions;
			CheckBox_Cooking_SkipScriptCompile.Checked = Settings.Cooking_SkipScriptCompilation;
			CheckBox_Cooking_ShowBalloon.Checked = Settings.Cooking_ShowBalloon;

			// console target settings
			if(Settings.ConsoleTargets_BaseDirectoryHistory != null)
			{
				ComboBox_ConsoleTargets_ConsoleBaseDir.Items.AddRange(Settings.ConsoleTargets_BaseDirectoryHistory);
			}

			CheckBox_ConsoleTargets_ShowAllInfo.Checked = Settings.ConsoleTargets_ShowAllTargetInfo;
			ComboBox_ConsoleTargets_ConsoleBaseDir.Text = Settings.ConsoleTargets_ConsoleBaseDir;
			CheckBox_ConsoleTargets_RebootBeforeCopy.Checked = Settings.ConsoleTargets_RebootBeforeCopy;
			CheckBox_ConsoleTargets_CopyDebugInfo.Checked = Settings.ConsoleTargets_CopyFilesRequiredForSymbolLookup;

			// compiling settings
			CheckBox_Compiling_FullRecompile.Checked = Settings.Compiling_FullRecompile;
			CheckBox_Compiling_RunWithCookedData.Checked = Settings.Compiling_RunWithCookedData;
			CheckBox_Compiling_VerboseMode.Checked = Settings.Compiling_VerboseMode;
			CheckBox_Compiling_AutoMode.Checked = Settings.Compiling_AutoMode;

			// do this last so that we don't cause extra refreshes of the target list
			bool bHasPlatform = false;
			foreach(object Obj in ComboBox_Platform.Items)
			{
				if(Obj.ToString().Equals(Settings.SelectedPlatform, StringComparison.OrdinalIgnoreCase))
				{
					ComboBox_Platform.SelectedItem = Obj;
					bHasPlatform = true;
					break;
				}
			}

			if(!bHasPlatform && ComboBox_Platform.Items.Count > 0)
			{
				ComboBox_Platform.SelectedIndex = 0;
			}
		}

		/// <summary>
		/// Saves user application settings.
		/// </summary>
		void WriteSettings()
		{
			// save tool strip locations
			ToolStripManager.SaveSettings(this);

			UpdatePlatformTargetList(ComboBox_Platform.Text);

			Properties.Settings Settings = Properties.Settings.Default;

			// global settings
			Settings.ConfigurationSelectedIndex = ComboBox_Configuration.SelectedIndex;
			Settings.CookingConfigurationSelectedIndex = ComboBox_CookingConfiguration.SelectedIndex;
			Settings.UFEMainWindowSize = this.Size;
			Settings.UFEWindowSplitterDistance = this.splitContainer1.SplitterDistance;
			Settings.SelectedGameName = ComboBox_Game.SelectedItem.ToString();
			Settings.SelectedPlatform = ComboBox_Platform.SelectedItem.ToString();


			// game settings
			Settings.MapToPlayHistory = ComboBoxItemsToStringArray(ComboBox_MapToPlay);
			Settings.Game_ExtraOptionsHistory = ComboBoxItemsToStringArray(ComboBox_Game_ExtraOptions);
			Settings.Game_UseCookedMap = CheckBox_Game_UseCookedMap.Checked;
			Settings.Game_ExtraOptions = ComboBox_Game_ExtraOptions.Text;
			Settings.Game_ExecCommands = TextBox_Game_ExecCommands.Text;
			Settings.Game_NoSound = CheckBox_Game_NoSound.Checked;
			Settings.Game_NoVSync = CheckBox_Game_NoVSync.Checked;
			Settings.Game_MultiThreaded = CheckBox_Game_MultiThreaded.Checked;
			Settings.Game_NumClients = NumericUpDown_Game_NumClients.Value;
			Settings.Game_ServerTypeSelectedIndex = ComboBox_Game_ServerType.SelectedIndex;
			Settings.Game_Resolution = ComboBox_Game_Resolution.Text;
			Settings.Game_ShowLog = CheckBox_Game_ShowLog.Checked;
			Settings.Game_RemoteControl = CheckBox_Game_RemoteControl.Checked;
			Settings.Game_ClearUCWindow = CheckBox_Game_ClearUCWindow.Checked;
			Settings.Game_GoW2OverrideSPMap = CheckBox_Game_GoW2OverrideSPMap.Checked;
			Settings.Game_GoW2SelectedChapterIndex = ComboBox_Game_GoW2SPMapName.SelectedIndex;
			Settings.Game_EncryptedSockets = CheckBox_Game_EncryptedSockets.Checked;

			// cooking settings
			List<string> SelectedLanguages = new List<string>();

			foreach(ListViewItem CurItem in ListView_Cooking_Languages.Items)
			{
				if(CurItem.Checked)
				{
					SelectedLanguages.Add(CurItem.Text);
				}
			}
			Settings.Cooking_LanguagesToCookAndSync = SelectedLanguages.ToArray();

			Settings.Cooking_MapsToCookHistory = ComboBoxItemsToStringArray(ComboBox_Cooking_MapsToCook);
			Settings.Cooking_ForceSeekFreeRecook = CheckBox_Cooking_ForceSeekFreeRecook.Checked;
			Settings.Cooking_FinalReleaseScripts = CheckBox_Cooking_CookFinalReleaseScript.Checked;
			Settings.Cooking_ForceSoundRecook = CheckBox_Cooking_ForceSoundRecook.Checked;
			Settings.Cooking_RunLocalGame = CheckBox_Cooking_RunLocalGame.Checked;
			Settings.Cooking_UseSeekFreeData = CheckBox_Cooking_CookMod.Checked;
			Settings.Cooking_DisablePackageCompression = CheckBox_Cooking_DisablePackageCompression.Checked;
			Settings.Cooking_AdditionalOptions = ComboBox_Cooking_AdditionalOptions.Text;
			Settings.Cooking_AdditionalOptionsHistory = ComboBoxItemsToStringArray(ComboBox_Cooking_AdditionalOptions);
			Settings.Cooking_SkipScriptCompilation = CheckBox_Cooking_SkipScriptCompile.Checked;
			Settings.Cooking_ShowBalloon = CheckBox_Cooking_ShowBalloon.Checked;

			// console target settings
			Settings.ConsoleTargets_ShowAllTargetInfo = CheckBox_ConsoleTargets_ShowAllInfo.Checked;
			Settings.ConsoleTargets_ConsoleBaseDir = ComboBox_ConsoleTargets_ConsoleBaseDir.Text;
			Settings.ConsoleTargets_BaseDirectoryHistory = ComboBoxItemsToStringArray(ComboBox_ConsoleTargets_ConsoleBaseDir);
			Settings.ConsoleTargets_RebootBeforeCopy = CheckBox_ConsoleTargets_RebootBeforeCopy.Checked;
			Settings.ConsoleTargets_CopyFilesRequiredForSymbolLookup = CheckBox_ConsoleTargets_CopyDebugInfo.Checked;

			// compiling settings
			Settings.Compiling_FullRecompile = CheckBox_Compiling_FullRecompile.Checked;
			Settings.Compiling_RunWithCookedData = CheckBox_Compiling_RunWithCookedData.Checked;
			Settings.Compiling_VerboseMode = CheckBox_Compiling_VerboseMode.Checked;
			Settings.Compiling_AutoMode = CheckBox_Compiling_AutoMode.Checked;
		}

		/// <summary>
		/// Logs the bad XML information for debugging purposes
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e">The attribute info</param>
		protected void XmlSerializer_UnknownAttribute(object sender, XmlAttributeEventArgs e)
		{
			System.Xml.XmlAttribute attr = e.Attr;
			Console.WriteLine("Unknown attribute " + attr.Name + "='" + attr.Value + "'");
		}

		/// <summary>
		/// Logs the node information for debugging purposes
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e">The info about the bad node type</param>
		protected void XmlSerializer_UnknownNode(object sender, XmlNodeEventArgs e)
		{
			Console.WriteLine("Unknown Node:" + e.Name + "\t" + e.Text);
		}

		/// <summary>
		/// Converts the XML file into an object
		/// </summary>
		/// <returns>The new object settings, or null if the file is missing</returns>
		private CookerSettings ReadCookerSettings()
		{
			Stream XmlStream = null;
			CookerSettings CookerCfg = null;
			try
			{
				// Get the XML data stream to read from
				XmlStream = new FileStream("UnrealFrontend_cfg.xml", FileMode.Open, FileAccess.Read, FileShare.Read, 256 * 1024, false);
				// Creates an instance of the XmlSerializer class so we can
				// read the settings object
				XmlSerializer ObjSer = new XmlSerializer(typeof(CookerSettings));
				// Add our callbacks for a busted XML file
				ObjSer.UnknownNode += new XmlNodeEventHandler(XmlSerializer_UnknownNode);
				ObjSer.UnknownAttribute += new XmlAttributeEventHandler(XmlSerializer_UnknownAttribute);
				// Create an object graph from the XML data
				CookerCfg = (CookerSettings)ObjSer.Deserialize(XmlStream);
			}
			catch(Exception e)
			{
				Console.WriteLine(e.ToString());
			}
			finally
			{
				if(XmlStream != null)
				{
					// Done with the file so close it
					XmlStream.Close();
				}
			}
			return CookerCfg;
		}

		#endregion

		#region Misc

		#region Log window helpers.

		// The proper way to update UI in C#/.NET is to use to all UI updates on the render thread 
		// which is done using InvokeRequired & Invoke to achieve.
		delegate void DelegateAddLine(string Line, Color TextColor);
		delegate void DelegateClear();

		/// <summary>
		/// Adds a line of output text.
		/// </summary>
		/// <param name="TextColor">The color of the text.</param>
		/// <param name="Line">The text to be appended.</param>
		/// <param name="Parms">Parameters controlling the final output string.</param>
		void AddLine(Color? TextColor, string Line, params object[] Parms)
		{
			if(Line == null)
			{
				return;
			}

			string FinalLine = string.Format(Line, Parms);
			mOutputDocument.AppendLine(TextColor, FinalLine);

			lock(mOutputBuffer)
			{
				mOutputBuffer.AppendLine(FinalLine);
			}
		}

		#endregion

		#region Commandlet functionality

		/// <summary>
		/// Cleans up leftover XMA temp files.
		/// </summary>
		private void CleanupXMAWorkFiles()
		{
			try
			{
				// Get temporary files left behind by XMA encoder and nuke them.
				string[] TempFiles = Directory.GetFiles(Path.GetTempPath(), "EncStrm*");
				foreach(string TempFile in TempFiles)
				{
					File.Delete(TempFile);
				}
			}
			catch(Exception Error)
			{
				AddLine(Color.Red, Error.ToString());
			}
		}

		/// <summary>
		/// Sets the state of the executing commandlet.
		/// </summary>
		/// <param name="bIsStarting">True if the commandlet is starting.</param>
		private void SetCommandletState(bool bIsStarting)
		{
			if(bIsStarting)
			{
				// Disable some run buttons while we are running.
				Tab_Compiling.Enabled = false;
				Tab_Cooking.Enabled = false;
				ToolStripButton_Cook.Enabled = false;
				ToolStripButton_CompileScript.Enabled = false;
				ComboBox_CookingConfiguration.Enabled = false;

				if(CommandletProcess != null)
				{
					switch(CommandletProcess.Category)
					{
						case CommandletCategory.Compiling:
							{
								ToolStripButton_CompileScript.Enabled = true;
								ToolStripButton_CompileScript.Text = "Stop Compiling";
								break;
							}

						case CommandletCategory.Cooking:
							{
								ToolStripButton_Cook.Enabled = true;
								ToolStripButton_Cook.Text = "Stop Cooking";
								
								ToolStripButton_CookGlobalShaders.Enabled = false;
								ToolStripButton_CookAllMaps.Enabled = false;
								ToolStripButton_CookIntsOnly.Enabled = false;
								ToolStripButton_FullRecook.Enabled = false;

								break;
							}
						case CommandletCategory.Syncing:
							{
								ToolStripButton_Sync.Enabled = true;
								ToolStripButton_Sync.Text = "Stop Syncing";
								break;
							}
					}
				}
			}
			else
			{
				Tab_Compiling.Enabled = true;
				Tab_Cooking.Enabled = true;
				ToolStripButton_Cook.Enabled = true;
				ToolStripButton_CompileScript.Enabled = true;
				ComboBox_CookingConfiguration.Enabled = true;

				if(CommandletProcess != null)
				{
					switch(CommandletProcess.Category)
					{
						case CommandletCategory.Compiling:
							{
								ToolStripButton_CompileScript.Text = "Make";
								break;
							}
						case CommandletCategory.Cooking:
							{
								ToolStripButton_Cook.Text = "Cook";

								ToolStripButton_CookGlobalShaders.Enabled = true;
								ToolStripButton_CookAllMaps.Enabled = true;
								ToolStripButton_CookIntsOnly.Enabled = true;
								ToolStripButton_FullRecook.Enabled = true;

								break;
							}
						case CommandletCategory.Syncing:
							{
								ToolStripButton_Sync.Text = "Sync";
								break;
							}
					}
				}
			}
		}

		/// <summary>
		/// Starts a commandlet.
		/// </summary>
		/// <param name="CommandletType">The type of commandlet to start.</param>
		private bool StartCommandlet(CommandletCategory Category, CommandletAction Action, object UserData)
		{
			return StartCommandlet(Category, Action, GetExecutablePath(true), UserData);
		}

		/// <summary>
		/// Starts a commandlet.
		/// </summary>
		/// <param name="CommandletType">The type of commandlet to start.</param>
		/// <param name="ExecutablePath">The path to the commandlet.</param>
		private bool StartCommandlet(CommandletCategory Category, CommandletAction Action, string ExecutablePath, object UserData)
		{
			if(CommandletProcess != null)
			{
				if(!CommandletProcess.HasExited)
				{
					AddLine(Color.Orange, "A commandlet is already running!");
					return false;
				}

				CommandletProcess.Dispose();
				CommandletProcess = null;
			}

			// @todo: Move this into XeTools.dll, possibly?
			CleanupXMAWorkFiles();

			StringBuilder CommandLine = new StringBuilder();
			ConsoleInterface.Platform CurPlat = (ConsoleInterface.Platform)ComboBox_Platform.SelectedItem;

			// do per-commandlet stuff here (like figure out command line)
			switch(Category)
			{
				case CommandletCategory.Cooking:
					{
						switch(Action)
						{
							case CommandletAction.Cooking_AllMaps:
							case CommandletAction.Cooking_FullRecook:
							case CommandletAction.Cooking_IniIntsOnly:
							case CommandletAction.Cooking:
								{
									CommandLine.Append(GetCookingCommandLine(Action, (CookingUserData)UserData));

									break;
								}
							case CommandletAction.Cooking_CompileScript:
								{
									
									if(CheckBox_Cooking_CookFinalReleaseScript.Checked)
									{
										CommandLine.Append("make -final_release");
									}
									else
									{
										CommandLine.Append("make");
									}

									break;
								}
							case CommandletAction.Cooking_GlobalShadersOnly:
								{

									CommandLine.AppendFormat("CookPackages -RECOMPILEGLOBALSHADERS platform={0}", CurPlat.Type == ConsoleInterface.PlatformType.Xbox360 ? "xenon" : CurPlat.Type.ToString());

									break;
								}
						}
						
						break;
					}

				case  CommandletCategory.Compiling:
					{
						CommandLine.Append(GetCompileScriptCommandLine());

						break;
					}
				case CommandletCategory.Syncing:
					{
						ConsoleInterface.TOCSettings PlatSettings = CreateTOCSettings();

						if(PlatSettings.TargetsToSync.Count == 0)
						{
							return false;
						}

						string[] LanguagesToSync = (string[])UserData;
						if(LanguagesToSync != null && LanguagesToSync.Length > 0)
						{
							PlatSettings.Languages = LanguagesToSync;
						}

						// recreate the exec file so that it gets copied over in the sync operation
						CreateTempExec();

						string TagSet = "ConsoleSync";
						if(!CheckBox_ConsoleTargets_CopyDebugInfo.Checked)
						{
							TagSet = "ConsoleSyncProgrammer";
						}

						StringBuilder Languages = new StringBuilder();

						//NOTE: Due to a workaround for an issue with cooking multiple languages
						// it's possible for INT to show up in the languages list twice.
						// Use this dictionary to prevent that.
						Dictionary<string, string> FinalLanguagesToSync = new Dictionary<string, string>();

						foreach(string CurLang in PlatSettings.Languages)
						{
							if(!FinalLanguagesToSync.ContainsKey(CurLang))
							{
								FinalLanguagesToSync[CurLang] = CurLang;

								Languages.Append(" -r ");
								Languages.Append(CurLang);
							}
						}

						CommandLine.AppendFormat("{0} -p {1} -x {2}{3} -b \"{4}\"", PlatSettings.GameName, CurPlat.Type.ToString(), TagSet, Languages.ToString(), PlatSettings.TargetBaseDirectory);

						if(CheckBox_ConsoleTargets_RebootBeforeCopy.Checked)
						{
							CommandLine.Append(" -o");
						}

						foreach(string Target in PlatSettings.TargetsToSync)
						{
							CommandLine.Append(' ');
							CommandLine.Append(Target);
						}

						break;
					}
			}

			CommandletProcess = new Commandlet(ExecutablePath, (ConsoleInterface.Platform)ComboBox_Platform.SelectedItem);
			CommandletProcess.UserData = UserData;
			CommandletProcess.Exited += new EventHandler<EventArgs>(CommandletProcess_Exited);
			CommandletProcess.Output += new EventHandler<CommandletOutputEventArgs>(CommandletProcess_Output);

			bool bRet = true;

			try
			{
				CommandletProcess.Start(Category, Action, CommandLine.ToString());

				SetCommandletState(true);

				WriteCommandletEvent(Color.Green, string.Format("COMMANDLET \'{0} {1}\' STARTED", Path.GetFileName(CommandletProcess.ExecutablePath), CommandletProcess.CommandLine));
			}
			catch(Exception ex)
			{
				WriteCommandletEvent(Color.Red, string.Format("COMMANDLET \'{0} {1}\' FAILED", Path.GetFileName(CommandletProcess.ExecutablePath), CommandletProcess.CommandLine));
				AddLine(Color.Red, ex.ToString());

				bRet = false;

				UnRegisterCommandletEvents();
				CommandletProcess.Dispose();
				CommandletProcess = null;
			}

			return bRet;
		}

		/// <summary>
		/// Handles commandlet output.
		/// </summary>
		/// <param name="sender">The commandlet that generated the output.</param>
		/// <param name="e">Information about the output.</param>
		void CommandletProcess_Output(object sender, CommandletOutputEventArgs e)
		{
			System.Diagnostics.Debug.WriteLine(e.Message);

			Commandlet Cmdlet = (Commandlet)sender;
			if(e.Message.StartsWith("Warning"))
			{
				BeginInvoke(new AddLineDelegate(this.AddLine), Color.Orange, e.Message, new object[0]);
			}
			else if(e.Message.StartsWith("Error"))
			{
				BeginInvoke(new AddLineDelegate(this.AddLine), Color.Red, e.Message, new object[0]);
			}
			else
			{
				BeginInvoke(new AddLineDelegate(this.AddLine), Color.Black, e.Message, new object[0]);
			}
		}

		/// <summary>
		/// Event handler for when a commandlet has exited.
		/// </summary>
		/// <param name="sender">The commandlet that exited.</param>
		/// <param name="e">Information about the event.</param>
		void CommandletProcess_Exited(object sender, EventArgs e)
		{
			Commandlet Cmdlet = (Commandlet)sender;

			if(Cmdlet.ExitCode == 0)
			{
				BeginInvoke(new WriteCommandletEventDelegate(this.WriteCommandletEvent), Color.Green, string.Format("COMMANDLET \'{0} {1}\' SUCCEEDED", Path.GetFileName(Cmdlet.ExecutablePath), Cmdlet.CommandLine));
			}
			else
			{
				BeginInvoke(new WriteCommandletEventDelegate(this.WriteCommandletEvent), Color.Red, string.Format("COMMANDLET \'{0} {1}\' FAILED", Path.GetFileName(Cmdlet.ExecutablePath), Cmdlet.CommandLine));
			}

			// marshal the handler into the UI thread so we can change UI state
			BeginInvoke(new CommandletFinishedDelegate(this.OnCommandletFinished), Cmdlet);
		}

		/// <summary>
		/// Event handler for when a commandlet has finished executing. This is called on the UI thread.
		/// </summary>
		/// <param name="FinishedCommandlet">The commandlet that has finished executing.</param>
		void OnCommandletFinished(Commandlet FinishedCommandlet)
		{
			UnRegisterCommandletEvents();

			SetCommandletState(false);

			if(FinishedCommandlet.ExitCode == 0)
			{
				switch(FinishedCommandlet.Category)
				{
					case CommandletCategory.Cooking:
						{
							CookingUserData UserData = (CookingUserData)FinishedCommandlet.UserData;

							if(FinishedCommandlet.Action == CommandletAction.Cooking_CompileScript)
							{
								CommandletAction NewAction = UserData.NextAction;
								UserData.NextAction = CommandletAction.Cooking;

								StartCommandlet(FinishedCommandlet.Category, NewAction, UserData);
								return;
							}

							if(UserData.CurrentLanguage < UserData.LanguageList.Length - 1)
							{
								++UserData.CurrentLanguage;
								StartCommandlet(FinishedCommandlet.Category, UserData.NextAction, UserData);
								return;
							}

							// only copy if the platform allows it
							if(ToolStripButton_Sync.Enabled)
							{
								// Sync up PC/console.
								StartCommandlet(CommandletCategory.Syncing, CommandletAction.Syncing, "CookerSync.exe", UserData.LanguageList);
							}
							else if(ComboBox_Platform.Text != "PC")
							{
								ConsoleInterface.Platform CurPlatform = (ConsoleInterface.Platform)ComboBox_Platform.SelectedItem;

								ConsoleInterface.TOCSettings BuildSettings = CreateTOCSettings();

								string TagSet = "ConsoleSync";
								if(!CheckBox_ConsoleTargets_CopyDebugInfo.Checked)
								{
									TagSet = "ConsoleSyncProgrammer";
								}

								foreach(string CurLang in BuildSettings.Languages)
								{
									CurPlatform.GenerateTOC(TagSet, BuildSettings, CurLang);
								}
							}

							// if the user wants to (and the platform allows it), run the game after cooking is finished
							if(CheckBox_Cooking_RunLocalGame.Checked && !ToolStripButton_Sync.Enabled)
							{
								OnLoadMapClick(null, null);
							}

							break;
						}
					case CommandletCategory.Syncing:
						{
							// if the user wants to, run the game after cooking/syncing is finished
							if(CheckBox_Cooking_RunLocalGame.Checked)
							{
								OnLoadMapClick(null, null);
							}

							break;
						}
				}

				ShowCommandletBalloonTooltip(ToolTipIcon.Info, string.Format("COMMANDLET \'{0} {1}\' SUCCEEDED", Path.GetFileName(FinishedCommandlet.ExecutablePath), FinishedCommandlet.CommandLine));
			}
			else
			{
				ShowCommandletBalloonTooltip(ToolTipIcon.Error, string.Format("COMMANDLET \'{0} {1}\' FAILED", Path.GetFileName(FinishedCommandlet.ExecutablePath), FinishedCommandlet.CommandLine));
			}
		}

		/// <summary>
		/// Shows a balloon tool tip to notify the user that a commandlet has finished.
		/// </summary>
		/// <param name="Message">The message to display to the user.</param>
		private void ShowCommandletBalloonTooltip(ToolTipIcon IconToShow, string Message)
		{
			NotifyIcon_CommandletFinished.Visible = true;
			NotifyIcon_CommandletFinished.BalloonTipIcon = IconToShow;
			NotifyIcon_CommandletFinished.BalloonTipText = Message;

			if(!this.ContainsFocus && CheckBox_Cooking_ShowBalloon.Checked)
			{
				NotifyIcon_CommandletFinished.ShowBalloonTip(5000);
				System.Media.SystemSounds.Exclamation.Play();
			}
		}

		/// <summary>
		/// Removes all registered event handlers from the current commandlet.
		/// </summary>
		private void UnRegisterCommandletEvents()
		{
			if(CommandletProcess != null)
			{
				CommandletProcess.Exited -= new EventHandler<EventArgs>(CommandletProcess_Exited);
				CommandletProcess.Output -= new EventHandler<CommandletOutputEventArgs>(CommandletProcess_Output);
			}
		}

		/// <summary>
		/// Stops the currently executing commandlet if there is one.
		/// </summary>
		private void StopCommandlet()
		{
			// force stop the commandlet if needed
			if(CommandletProcess != null)
			{
				if(!CommandletProcess.HasExited)
				{
					UnRegisterCommandletEvents();

					CommandletProcess.Kill();
				}
			}

			// note that we are stopping the commandlet
			SetCommandletState(false);
		}

		/// <summary>
		/// Asks the user if he wants to stop the currently executing commandlet.
		/// </summary>
		/// <returns>True if the commandlet is no longer running.</returns>
		private bool CheckedStopCommandlet()
		{
			bool bCommandletStopped = true;

			if(CommandletProcess != null && !CommandletProcess.HasExited)
			{
				DialogResult result = MessageBox.Show("Are you sure you want to cancel?", "Stop commandlet", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
				if(result == DialogResult.Yes)
				{
					// StopCommandlet() destroys this info so cache it
					string CommandletName = Path.GetFileName(CommandletProcess.ExecutablePath);
					string CommandletCmdLine = CommandletProcess.CommandLine;

					StopCommandlet();

					WriteCommandletEvent(Color.Orange, string.Format("COMMANDLET '{0} {1}' ABORTED", CommandletName, CommandletCmdLine));
				}
				else
				{
					bCommandletStopped = false;
				}
			}

			return bCommandletStopped;
		}


		#endregion

		#region Misc helper functions

		/// <summary>
		/// Creates the exec temp file.
		/// </summary>
		private void CreateTempExec()
		{
			string TmpExecLocation;

			if(ComboBox_Platform.Text == "PC")
			{
				TmpExecLocation = SettingsPath + "\\Binaries\\UnrealFrontend_TmpExec.txt";
			}
			else
			{
				TmpExecLocation = "UnrealFrontend_TmpExec.txt";
			}

			File.WriteAllLines(TmpExecLocation, TextBox_Game_ExecCommands.Lines);
		}

		/// <summary>
		/// Pops up a dialog box asking the user for a map.
		/// </summary>
		/// <param name="MapName">The map the user selected.</param>
		/// <returns>True if success.</returns>
		private bool BrowseForMap(out string MapName)
		{
			using(OpenFileDialog FileDlg = new OpenFileDialog())
			{
				FileDlg.InitialDirectory = Path.GetFullPath("..\\" + ComboBox_Game.Text + "\\Content\\Maps");
				FileDlg.Filter = "Map Files (*.umap;*.ut3;*.gear)|*.umap;*.ut3;*.gear";
				FileDlg.RestoreDirectory = true;
				
				if(FileDlg.ShowDialog() == DialogResult.OK)
				{
					MapName = Path.GetFileNameWithoutExtension(FileDlg.FileName);

					return true;
				}
				MapName = "";
			}

			return false;
		}

		/// <summary>
		/// Generates the URL for game and engine options.
		/// </summary>
		/// <param name="Config">The configuration to receive the options for.</param>
		/// <param name="GameOptions">Receives the game options URL.</param>
		/// <param name="EngineOptions">Receives the engine options URL.</param>
		private void ParseURLOptions(ECurrentConfig Config, out string GameOptions, out string EngineOptions)
		{
			// get which URL to parse
			string[] Options = ComboBox_Game_ExtraOptions.Text.Split(' ');

			// empty out both outputs
			GameOptions = "";
			EngineOptions = "";
			foreach(string Option in Options)
			{
				if(Option.Length > 0)
				{
					// put all ? options together
					if(Option[0] == '?')
					{
						GameOptions += Option;
					}
					// put all - options together
					else if(Option[0] == '-')
					{
						EngineOptions += " " + Option;
					}
					// ignore anything without ? or -
				}
			}
		}

		/// <summary>
		/// Builds the game execution command line.
		/// </summary>
		/// <returns>The command line that will be used to execute the game with the current options and configuration.</returns>
		public string BuildGameCommandLine()
		{
			StringBuilder Bldr = new StringBuilder();

			if(CheckBox_Game_ShowLog.Checked)
			{
				Bldr.Append("-log ");
			}

			if(CheckBox_Game_NoSound.Checked)
			{
				Bldr.Append("-nosound ");
			}

			if(!CheckBox_Game_MultiThreaded.Checked)
			{
				Bldr.Append("-onethread ");
			}

			if(CheckBox_Game_NoVSync.Checked)
			{
				Bldr.Append("-novsync ");
			}

			if(!CheckBox_Game_RemoteControl.Checked)
			{
				Bldr.Append("-norc ");
			}
			else
			{
				Bldr.Append("-remotecontrol ");
			}

			if(!CheckBox_Game_EncryptedSockets.Checked)
			{
				// Disable secure connections to allow DB connection.
				Bldr.Append("-DEVCON ");
			}

			if(CheckBox_Game_CaptureFPSChartInfo.Checked)
			{
				Bldr.Append("-gCFPSCI=1 ");
			}

			// if we  are doing a sentinel run we need toappend a number of other command line options
			if(CheckBox_Game_SentinelRun.Checked)
			{
				Bldr.Append("-unattended ");
				Bldr.Append("-novsync ");
				Bldr.Append("-fixedseed ");
				//Bldr.Append("-writetohost ");  // prob should have this be implicity on ps3 when not FINAL_RELEASE
			}

			return Bldr.ToString().Trim();
		}

		/// <summary>
		/// Generate a final URL to pass to the game
		/// </summary>
		/// <param name="GameOptions">Game type options (? options)</param>
		/// <param name="PostCmdLine">Engine type options (- options)</param>
		/// <param name="ConfigType">SP, MP or neither</param>
		/// <param name="bCreateExecFile"></param>
		/// <returns></returns>
		private string GetFinalURL(string GameOptions, string EngineOptions, ECurrentConfig ConfigType, bool bCreateExecFile)
		{
			// build the commandline
			StringBuilder CmdLine = new StringBuilder();

			if(GameOptions != null && GameOptions.Length > 0)
			{
				CmdLine.Append(GameOptions);

				if(CheckBox_Game_SentinelRun.Checked)
				{
					// seems to cause issues with gears CmdLine.Append("?automatedperftesting=1");
					CmdLine.Append("?quickstart=1");
					CmdLine.Append("?bTourist=1");
					CmdLine.Append("?gDASR=1");

					if(ComboBox_Game_SentinelType.Text == "FlyThrough")
					{
						CmdLine.Append("?causeevent=Flythrough");
					}
					else if(ComboBox_Game_SentinelType.Text == "FlyThroughSplitScreen")
					{
						CmdLine.Append("?causeevent=ss-Flythrough");
					}

					if(ComboBox_Game_SentinelType.Text.Length > 0)
					{
						CmdLine.Append("?gSTD=" + ComboBox_Game_SentinelType.Text);
					}

					if(TextBox_Game_SentinelTag.Text.Length > 0)
					{
						CmdLine.Append("?gSTDD=" + TextBox_Game_SentinelTag.Text);
					}
				}
			}

			GameOptions = BuildGameCommandLine();

			if(GameOptions != null && GameOptions.Length > 0)
			{
				CmdLine.Append(' ');
				CmdLine.Append(GameOptions);
			}

			if(EngineOptions != null && EngineOptions.Length > 0)
			{
				CmdLine.Append(' ');
				CmdLine.Append(EngineOptions);
			}

			CmdLine.Append(" -Exec=UnrealFrontend_TmpExec.txt");

			// final pass for execs
			if(bCreateExecFile)
			{
				CreateTempExec();
			}

			return CmdLine.ToString();
		}

		#endregion

		#region Key handling

		/// <summary>
		/// Called when a key has been pressed in the UFE window.
		/// </summary>
		/// <param name="sender">The object that generated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnUnrealFrontendWindowKeyDown(object sender, KeyEventArgs e)
		{
			if(e.Modifiers == 0 && (int)e.KeyCode >= (int)Keys.F1 && (int)e.KeyCode <= (int)Keys.F12)
			{
				TabControl_Main.SelectedIndex = (int)e.KeyCode - (int)Keys.F1;
				e.Handled = true;
			}
			// manually handle keyboard shortcuts
			else if(e.Modifiers == Keys.Control)
			{
				// default to handled
				e.Handled = true;

				switch(e.KeyValue)
				{
					case 'L':
						if(ToolStripButton_Launch.Enabled)
						{
							OnLoadMapClick(this, e);
						}
						break;

					case 'S':
						if(ToolStripButton_Server.Enabled)
						{
							OnServerClientClick(this, e);
						}
						break;

					case 'E':
						if(ToolStripButton_Editor.Enabled)
						{
							OnEditorClick(this, e);
						}
						break;

					case 'K':
						if(ToolStripButton_Cook.Enabled)
						{
							OnStartCookingClick(this, e);
						}
						break;

					case 'M':
						if(ToolStripButton_CompileScript.Enabled)
						{
							OnCompileClick(this, e);
						}
						break;

					case 'Y':
						if(ToolStripButton_Sync.Enabled)
						{
							OnCopyToTargetsClick(this, e);
						}
						break;

					case 'R':
						if(ToolStripButton_Reboot.Enabled)
						{
							OnRebootTargetsClick(this, e);
						}
						break;

					default:
						// actually, we didn't handle it
						e.Handled = false;
						break;
				}
			}
		}

		#endregion

		#endregion

		#region Controls

		#region Global controls

		/// <summary>
		/// Handles changes in the selected game.
		/// </summary>
		private void ProcessGameNameChanged()
		{
			// fix up per-game controls based on game
			CachedGameName = ComboBox_Game.Text;

			CheckBox_Game_UseCookedMap.Show();

			TextBox_Game_ExecutableNameOverride.Text = this.ExecutableNameOverride;
			CheckBox_Game_OverrideExecutable.Checked = this.OverrideExecutable;

			if(ComboBox_Game.Text == "GearGame")
			{
				GroupBox_Game_GoW2.Enabled = true;
			}
			else
			{
				GroupBox_Game_GoW2.Enabled = false;
			}

			LoadMapToPlay();
			LoadMapsToCook();
		}

		/// <summary>
		/// Event handler for when the selected game has changed.
		/// </summary>
		/// <param name="sender">The object that generated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnGameNameChanged(object sender, System.EventArgs e)
		{
			ProcessGameNameChanged();

			// focus on something else so that the tool strip buttons will get the mouse-over highlights
			TabControl_Main.Focus();
		}

		/// <summary>
		/// Handles changes to the selected platform.
		/// </summary>
		private void ProcessPlatformChanged()
		{
			ConsoleInterface.Platform Platform = ComboBox_Platform.SelectedItem as ConsoleInterface.Platform;
			Debug.Assert(Platform != null);

			//if the current platform does not exist create a dictionary for it
			if(!PlatformDefaults.ContainsKey(ComboBox_Platform.Text))
			{
				PlatformDefaults[ComboBox_Platform.Text] = new Dictionary<string, bool>();
			}

			if(Platform.Type == ConsoleInterface.PlatformType.PC)
			{
				ToolStripButton_Sync.Enabled = false;
				ToolStripButton_Reboot.Enabled = false;
				ToolStripButton_AddClient.Enabled = true;
				ToolStripButton_KillAll.Enabled = true;
				ListBox_Targets.Enabled = false;
				ComboBox_MapToPlay.Enabled = true;
				ButtonBrowseMap.Enabled = true;
				GroupBox_Game_PC.Enabled = true;
				GroupBox_Game_Xbox360.Enabled = false;
			}
			else
			{
				if(Platform.Type == ConsoleInterface.PlatformType.Xbox360)
				{
					GroupBox_Game_Xbox360.Enabled = true;
				}
				else
				{
					GroupBox_Game_Xbox360.Enabled = false;
				}

				ToolStripButton_Reboot.Enabled = true;
				ToolStripButton_AddClient.Enabled = false;
				ToolStripButton_KillAll.Enabled = false;

				ListBox_Targets.Enabled = true;
				ComboBox_MapToPlay.Enabled = !CheckBox_Game_UseCookedMap.Checked;
				ButtonBrowseMap.Enabled = !CheckBox_Game_UseCookedMap.Checked;
				GroupBox_Game_PC.Enabled = false;

				ToolStripButton_Sync.Enabled = Platform.NeedsToSync;
			}

			UpdateConsoleTargetsTab(false);

			TextBox_Game_ExecutableNameOverride.Text = this.ExecutableNameOverride;
			CheckBox_Game_OverrideExecutable.Checked = this.OverrideExecutable;

			UpdateInfoText();
			LoadMapToPlay();
			LoadMapsToCook();
		}

		/// <summary>
		/// Updates the list of targets and other controls on the Console Targets tab.
		/// </summary>
		/// <param name="bRefreshing">True if the function is being called to update the targets tab for the current platform, false if the platform has changed.</param>
		private void UpdateConsoleTargetsTab(bool bRefreshing)
		{
			if(bRefreshing && mPlatformTracker.CurrentPlatformIndex != -1)
			{
				UpdatePlatformTargetList(ComboBox_Platform.Items[mPlatformTracker.CurrentPlatformIndex].ToString());
			}
			else if(!bRefreshing && mPlatformTracker.LastPlatformIndex != -1)
			{
				UpdatePlatformTargetList(ComboBox_Platform.Items[mPlatformTracker.LastPlatformIndex].ToString());
			}

			// remove existing entries
			ListBox_Targets.Items.Clear();

			ConsoleInterface.Platform Platform = ComboBox_Platform.SelectedItem as ConsoleInterface.Platform;

			if(Platform == null || Platform.Type == ConsoleInterface.PlatformType.PC)
			{
				return;
			}

			Platform.EnumerateAvailableTargets();

			TargetListViewItem DefaultTarget = null;

			List<string> TargetList;
			PlatformTargets.TryGetTargets(ComboBox_Platform.Text, out TargetList);
			int NumTargetsChecked = 0;

			foreach(ConsoleInterface.PlatformTarget CurTarget in Platform.Targets)
			{
				TargetListViewItem Item = new TargetListViewItem(CurTarget, CheckBox_ConsoleTargets_ShowAllInfo.Checked);
				ListBox_Targets.Items.Add(Item);

				if(CurTarget.IsDefault)
				{
					DefaultTarget = Item;
				}

				//Item.Text should contain the target name
				if(TargetList != null && TargetList.Contains((string)Item.Tag))
				{
					Item.Checked = true;
					++NumTargetsChecked;
				}
			}

			if(NumTargetsChecked == 0 && DefaultTarget != null)
			{
				DefaultTarget.Checked = true;
			}
		}

		/// <summary>
		/// Loads the maps to cook for the current platform and game.
		/// </summary>
		private void LoadMapsToCook()
		{
			bool bFoundMap = false;
			string MapName;
			PlatformGameMapCollection PlatformTable = Properties.Settings.Default.Cooking_MapsToCookOnPlatform;

			if(PlatformTable != null)
			{
				if(PlatformTable.TryGetMap(ComboBox_Platform.Text, ComboBox_Game.Text, out MapName))
				{
					ComboBox_Cooking_MapsToCook.Text = MapName;
					bFoundMap = true;
				}
			}
			else
			{
				Properties.Settings.Default.Cooking_MapsToCookOnPlatform = new PlatformGameMapCollection();
			}

			if(!bFoundMap)
			{
				ComboBox_Cooking_MapsToCook.Text = string.Empty;
			}
		}

		/// <summary>
		/// Updates the cached list of maps to cook for the current platform and game.
		/// </summary>
		private void UpdateMapsToCook()
		{
			PlatformGameMapCollection PlatformTable = Properties.Settings.Default.Cooking_MapsToCookOnPlatform;

			if(PlatformTable != null)
			{
				PlatformTable.SetMap(ComboBox_Platform.Text, ComboBox_Game.Text, ComboBox_Cooking_MapsToCook.Text);
			}
			else
			{
				Properties.Settings.Default.Cooking_MapsToCookOnPlatform = new PlatformGameMapCollection();
			}
		}

		/// <summary>
		/// Loads the map to play for the current platform and game.
		/// </summary>
		private void LoadMapToPlay()
		{
			bool bFoundMap = false;
			string MapName;
			PlatformGameMapCollection PlatformTable = Properties.Settings.Default.MapToPlayOnPlatform;

			if(PlatformTable != null)
			{
				if(PlatformTable.TryGetMap(ComboBox_Platform.Text, ComboBox_Game.Text, out MapName))
				{
					ComboBox_MapToPlay.Text = MapName;
					bFoundMap = true;
				}
			}
			else
			{
				Properties.Settings.Default.MapToPlayOnPlatform = new PlatformGameMapCollection();
			}

			if(!bFoundMap)
			{
				ComboBox_MapToPlay.Text = string.Empty;
			}
		}

		/// <summary>
		/// Updates the cached list of maps to play for the current platform and game.
		/// </summary>
		private void UpdateMapToPlay()
		{
			PlatformGameMapCollection PlatformTable = Properties.Settings.Default.MapToPlayOnPlatform;

			if(PlatformTable != null)
			{
				PlatformTable.SetMap(ComboBox_Platform.Text, ComboBox_Game.Text, ComboBox_MapToPlay.Text);
			}
			else
			{
				Properties.Settings.Default.MapToPlayOnPlatform = new PlatformGameMapCollection();
			}
		}

		/// <summary>
		/// Event handler for when the selected platform changes.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnPlatformChanged(object sender, EventArgs e)
		{
			// to my great dismay I must track this myself
			mPlatformTracker.CurrentPlatformIndex = ComboBox_Platform.SelectedIndex;

			ProcessPlatformChanged();

			// focus on something else so that the tool strip buttons will get the mouse-over highlights
			TabControl_Main.Focus();
		}

		/// <summary>
		/// Copies certain values from across all pages to the status bar so the user
		/// doesn't have to switch tabs just to see important info
		/// </summary>
		private void UpdateInfoText()
		{
			// set each status label accordingly
			ToolStripStatusLabel_LocalMap.Text = GetSelectedMapSP();
			if(ToolStripStatusLabel_LocalMap.Text == "")
			{
				ToolStripStatusLabel_LocalMap.Text = "<none>";
			}
		}

		#endregion

		#region Local tab

		/// <summary>
		/// Returns the map to use for SP game/editor based on user options
		/// </summary>
		/// <returns>The map to launch with</returns>
		private string GetSelectedMapSP()
		{
			string Map = "";
			GoW2ChapterEntry Chapter = (GoW2ChapterEntry)ComboBox_Game_GoW2SPMapName.SelectedItem;

			if(CheckBox_Game_GoW2OverrideSPMap.Checked && Chapter != null && ComboBox_Game.Text == "GearGame")
			{
				Map = "geargame_p?chapter=" + Chapter.Index.ToString();
			}
			else
			{
				if(CheckBox_Game_UseCookedMap.Checked)
				{
					Map = ComboBox_Cooking_MapsToCook.Text;
					// split up multiple maps
					if(Map != "")
					{
						string[] Maps = ComboBox_Cooking_MapsToCook.Text.Split(' ');
						Map = Maps[0];
					}
				}
				else
				{
					Map = ComboBox_MapToPlay.Text;
				}
			}

			return Map;
		}

		/// <summary>
		/// Gets the options for a single player game on the current platform.
		/// </summary>
		/// <param name="GameOptions">Receives the game options URL.</param>
		/// <param name="EngineOptions">Receives the engine options URL.</param>
		private void GetSPURLOptions(out string GameOptions, out string EngineOptions)
		{
			string ExtraGameOptions;
			string ExtraEngineOptions;
			ParseURLOptions(ECurrentConfig.SP, out ExtraGameOptions, out ExtraEngineOptions);

			// put together with URL options
			GameOptions = GetSelectedMapSP() + ExtraGameOptions;

			// just use the extra options to start with
			EngineOptions = ExtraEngineOptions;

			EngineOptions = EngineOptions.Trim();

			// possibly run the PC with cooked data
			if(ComboBox_Platform.Text == "PC" && CheckBox_Game_UseCookedMap.Checked)
			{
				EngineOptions = EngineOptions + " -seekfreeloading";
			}

			Resolution Res;
			if(ComboBox_Platform.Text == "PC" && Resolution.TryParse(ComboBox_Game_Resolution.Text, out Res))
			{
				EngineOptions += string.Format(" -resx={0} -resy={1}", Res.Width, Res.Height);
			}
		}

		/// <summary>
		/// Handles loading a SP map on PC or console
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnLoadMapClick(object sender, EventArgs e)
		{
			UpdateMapToPlayCollection();
			UpdateExtraOptionsCollection();

			// determine if the systray menu was used to select the map
			MenuItem Item = sender as MenuItem;
			// if so, browse for a map
			if(Item != null)
			{
				string MapName = Item.Text;
				// check for the browse option
				if(MapName == "Browse...")
				{
					if(!BrowseForMap(out MapName))
					{
						// aborted browse, don't launch
						return;
					}

				}
				// set the selected map to this one
				ComboBox_MapToPlay.Text = MapName;
			}


			string GameOptions;
			string EngineOptions;
			GetSPURLOptions(out GameOptions, out EngineOptions);

			// run on PC or console passing additional engine (-) options
			LaunchApp(GameOptions, EngineOptions, ECurrentConfig.SP, false);
		}

		/// <summary>
		/// Updates the collection of extra game options.
		/// </summary>
		private void UpdateExtraOptionsCollection()
		{
			if(ComboBox_Game_ExtraOptions.Text.Length > 0 && !ComboBox_Game_ExtraOptions.Items.Contains(ComboBox_Game_ExtraOptions.Text))
			{
				ComboBox_Game_ExtraOptions.Items.Add(ComboBox_Game_ExtraOptions.Text);
			}
		}

		/// <summary>
		/// Updates the collection of maps to play.
		/// </summary>
		private void UpdateMapToPlayCollection()
		{
			if(ComboBox_MapToPlay.Text.Length > 0 && !ComboBox_MapToPlay.Items.Contains(ComboBox_MapToPlay.Text))
			{
				ComboBox_MapToPlay.Items.Add(ComboBox_MapToPlay.Text);
			}
		}

		/// <summary>
		/// Starts the editor for the currently selected game.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnEditorClick(object sender, EventArgs e)
		{
			UpdateMapToPlayCollection();

			LaunchApp("editor " + GetSelectedMapSP(), "", ECurrentConfig.Global, true);
		}

		/// <summary>
		/// Opens a dialog box to search for a single player map.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnBrowseSPClick(object sender, EventArgs e)
		{
			string MapName = "";
			if(BrowseForMap(out MapName))
			{
				ComboBox_MapToPlay.Text = MapName;
			}
		}

		/// <summary>
		/// Event handler for using a cooked map to play.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnUseCookedMapChecked(object sender, EventArgs e)
		{
			ComboBox_MapToPlay.Enabled = !CheckBox_Game_UseCookedMap.Checked;
			ButtonBrowseMap.Enabled = !CheckBox_Game_UseCookedMap.Checked;

			UpdateInfoText();
		}

		#endregion

		#region Mutiplayer tab

		/// <summary>
		/// Retrieves the multiplayer game options.
		/// </summary>
		/// <param name="GameOptions">Receives the game options URL.</param>
		/// <param name="EngineOptions">Receives the engine options URL.</param>
		private void GetMPURLOptions(out string GameOptions, out string EngineOptions)
		{
			string InitialURL = "";
			string Options = "";

			// build URL and options based on type
			if((ServerType)ComboBox_Game_ServerType.SelectedItem == ServerType.Listen)
			{
				int Spacing = Properties.Settings.Default.WindowSpacing;

				if(CheckBox_Game_UseCookedMap.Checked)
				{
					string[] CookedMaps = ComboBox_Cooking_MapsToCook.Text.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

					if(CookedMaps.Length > 0)
					{
						InitialURL = CookedMaps[0];
					}
					else
					{
						InitialURL = ComboBox_MapToPlay.Text;
					}
				}
				else
				{
					InitialURL = ComboBox_MapToPlay.Text;
				}

				InitialURL += "?listen";

				if(ComboBox_Platform.Text == "PC")
				{
					Options = " -posX=" + Spacing + " -posY=" + Spacing;
				}
			}
			else
			{
				InitialURL = "server " + ComboBox_MapToPlay.Text;
			}

			// get extra options
			string ExtraGameOptions;
			string ExtraEngineOptions;
			ParseURLOptions(ECurrentConfig.MP, out ExtraGameOptions, out ExtraEngineOptions);

			// tack on the shared settings
			GameOptions = InitialURL + ExtraGameOptions;
			EngineOptions = Options + ExtraEngineOptions;
		}

		/// <summary>
		/// Kills all game processes spawned by UFE.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnKillAllClick(object sender, EventArgs e)
		{
			foreach(Process newProcess in Processes)
			{
				if(newProcess != null && !newProcess.HasExited)
				{
					try
					{
						newProcess.Kill();
					}
					catch(Exception)
					{
					}
				}
			}
			Processes.Clear();
		}

		/// <summary>
		/// Creates a PC client and connects it to a server running on localhost.
		/// </summary>
		/// <param name="ResX">The horizontal resoultion of the client window.</param>
		/// <param name="ResY">The vertical resolution of the client window.</param>
		/// <param name="Spacing">The spacing between windows.</param>
		/// <param name="URLOptions">The URL to use when starting the clients.</param>
		private void AddPCClient(int ResX, int ResY, int Spacing, string URLOptions)
		{
			int PosX = -1, PosY = -1;
			if(NumClients == 0)
			{
				PosX = Spacing;
				PosY = Spacing + ResY + Spacing;
			}
			else if(NumClients == 1)
			{
				PosX = Spacing + ResX + Spacing;
				PosY = Spacing;
			}
			else if(NumClients == 2)
			{
				PosX = Spacing + ResX + Spacing;
				PosY = Spacing + ResY + Spacing;
			}
			LaunchApp("127.0.0.1" + URLOptions, string.Format(" -posX={0} -posY={1} -resx={2} -resy={3}", PosX, PosY, ResX, ResY), ECurrentConfig.MP, false);
			NumClients++;
		}

		/// <summary>
		/// Generates the extra game options string.
		/// </summary>
		/// <returns>A string containing the URL for the extra game options.</returns>
		public string BuildExtraGameOptions()
		{
			return ComboBox_Game_ExtraOptions.Text.Trim();
		}

		/// <summary>
		/// Adds clients to a server running on localhost.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnClientClick(object sender, EventArgs e)
		{
			UpdateExtraOptionsCollection();

			string URLOptions = BuildExtraGameOptions();

			ConsoleInterface.Platform CurPlatform = (ConsoleInterface.Platform)ComboBox_Platform.SelectedItem;

			if(CurPlatform.Type == ConsoleInterface.PlatformType.PC)
			{
				int Spacing = Properties.Settings.Default.WindowSpacing;

				Resolution Res;
				Resolution.TryParse(ComboBox_Game_Resolution.Text, out Res);

				AddPCClient(Res.Width, Res.Height, Spacing, URLOptions);
			}
		}

		/// <summary>
		/// Launches a PC server on localhost or a server on a console.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnServerClientClick(object sender, EventArgs e)
		{
			UpdateMapToPlayCollection();
			UpdateExtraOptionsCollection();

			ConsoleInterface.Platform CurPlatform = (ConsoleInterface.Platform)ComboBox_Platform.SelectedItem;

			switch(CurPlatform.Type)
			{
				case ConsoleInterface.PlatformType.PC:
					{
						LaunchMPGamePC();
						break;
					}
				case ConsoleInterface.PlatformType.PS3:
				case ConsoleInterface.PlatformType.Xbox360:
					{
						LaunchMPGameConsole();
						break;
					}
			}
		}

		/// <summary>
		/// Gets the list of selected targets.
		/// </summary>
		/// <returns>An array of active targets.</returns>
		private ConsoleInterface.PlatformTarget[] GetActiveTargets()
		{
			List<ConsoleInterface.PlatformTarget> Targets = new List<ConsoleInterface.PlatformTarget>();

			foreach(TargetListViewItem CurItem in ListBox_Targets.Items)
			{
				if(CurItem.Checked)
				{
					Targets.Add(CurItem.Target);
				}
			}

			return Targets.ToArray();
		}

		/// <summary>
		/// Updates the history of base directories with the current base directory.
		/// </summary>
		private void UpdateConsoleBaseDirectoryHistory()
		{
			if(ComboBox_ConsoleTargets_ConsoleBaseDir.Text.Length > 0 && !ComboBox_ConsoleTargets_ConsoleBaseDir.Items.Contains(ComboBox_ConsoleTargets_ConsoleBaseDir.Text))
			{
				ComboBox_ConsoleTargets_ConsoleBaseDir.Items.Add(ComboBox_ConsoleTargets_ConsoleBaseDir.Text);
			}
		}

		/// <summary>
		/// Launchers a multiplayer game on a console.
		/// </summary>
		private void LaunchMPGameConsole()
		{
			UpdateConsoleBaseDirectoryHistory();

			ConsoleInterface.PlatformTarget[] Targets = GetActiveTargets();

			if(Targets.Length > 0)
			{
				string GameOptions;
				string EngineOptions;
				GetMPURLOptions(out GameOptions, out EngineOptions);

				string CmdLine = GetFinalURL(GameOptions, EngineOptions, ECurrentConfig.MP, false);
				string ConfigStr = GetConsoleConfigurationString((Configuration)ComboBox_Configuration.SelectedItem);
				string URLOptions = BuildExtraGameOptions();
				bool bIsConnected = Targets[0].IsConnected;

				if(!bIsConnected)
				{
					if(!Targets[0].Connect())
					{
						AddLine(Color.Red, "Failed to create server on target \'{0}\'!", Targets[0].Name);
						return;
					}
				}

				if(!Targets[0].RebootAndRun(ConfigStr, ComboBox_ConsoleTargets_ConsoleBaseDir.Text, CheckBox_Game_OverrideExecutable.Checked ? TextBox_Game_ExecutableNameOverride.Text : ComboBox_Game.Text, CmdLine, CheckBox_Game_OverrideExecutable.Checked))
				{
					AddLine(Color.Red, "Failed to create server on target \'{0}\'!", Targets[0].Name);
					return;
				}
				else
				{
					AddLine(null, "Created server on target \'{0}\'!", Targets[0].Name);
					LaunchUnrealConsole(Targets[0]);
				}

				if(!bIsConnected)
				{
					Targets[0].Disconnect();
				}

				int NumClients = Convert.ToInt32(NumericUpDown_Game_NumClients.Value);
				CmdLine = string.Format("{0} {1}", Targets[0].IPAddress.ToString(), URLOptions).Trim();
				CmdLine = GetFinalURL(CmdLine, null, ECurrentConfig.MP, false);

				for(int i = 1; i <= NumClients && i < Targets.Length; ++i)
				{
					bIsConnected = Targets[i].IsConnected;

					if(!bIsConnected)
					{
						if(!Targets[i].Connect())
						{
							AddLine(Color.Red, "Failed to create client \'{0}\'!", Targets[i].Name);
							continue;
						}
					}

					if(Targets[i].RebootAndRun(ConfigStr, ComboBox_ConsoleTargets_ConsoleBaseDir.Text, CheckBox_Game_OverrideExecutable.Checked ? TextBox_Game_ExecutableNameOverride.Text : ComboBox_Game.Text, CmdLine, CheckBox_Game_OverrideExecutable.Checked))
					{
						AddLine(null, "Created client \'{0}\'.", Targets[i].Name);
						LaunchUnrealConsole(Targets[i]);
					}
					else
					{
						AddLine(Color.Red, "Failed to create client \'{0}\'!", Targets[i].Name);
					}

					if(!bIsConnected)
					{
						Targets[i].Disconnect();
					}
				}
			}
		}

		/// <summary>
		/// Launches a multiplayer game on the PC.
		/// </summary>
		private void LaunchMPGamePC()
		{
			Resolution Res;
			Resolution.TryParse(ComboBox_Game_Resolution.Text, out Res);

			string URLOptions = BuildExtraGameOptions();

			int Spacing = Properties.Settings.Default.WindowSpacing;

			if((ServerType)ComboBox_Game_ServerType.SelectedItem == ServerType.Listen)
			{
				// launch the server
				LaunchApp(string.Format("{0}?listen{1}", this.ComboBox_MapToPlay.Text, URLOptions), string.Format(" -resx={0} -resy={1}", Res.Width, Res.Height), ECurrentConfig.MP, false);
			}
			else
			{
				LaunchApp(string.Format("server {0} {1}", this.ComboBox_MapToPlay.Text, URLOptions), "", ECurrentConfig.MP, false);
			}

			// stall a bit to prevent thrashing
			Thread.Sleep(10000);

			int DesiredClients = Convert.ToInt32(NumericUpDown_Game_NumClients.Value);
			NumClients = 0;
			while(NumClients < DesiredClients)
			{
				AddPCClient(Res.Width, Res.Height, Spacing, URLOptions);
				// stall a bit to prevent thrashing
				Thread.Sleep(10000);
			}
		}

		#endregion

		#region Cooking tab

		/// <summary>
		/// Generates the cooking commandline.
		/// </summary>
		/// <returns>The cooking commandline for the current platform, game, and configuration.</returns>
		private string GetCookingCommandLine(CommandletAction Action, CookingUserData UserData)
		{
			// Base command
			string CommandLine = "CookPackages -platform=" + ComboBox_Platform.Text;

			if(Action == CommandletAction.Cooking_IniIntsOnly)
			{
				CommandLine += " -inisOnly";
			}
			else
			{
				if(ComboBox_Cooking_MapsToCook.Text.Length > 0)
				{
					// Add in map name
					CommandLine += " " + ComboBox_Cooking_MapsToCook.Text;
				}

				if(CheckBox_Cooking_ForceSeekFreeRecook.Checked)
				{
					CommandLine += " -recookseekfree";
				}

				if(CheckBox_Cooking_Game_ForceSeekFreeRecookGameTypes.Checked)
				{
					CommandLine += " -recookseekfreegametypes";
				}

				if(CheckBox_Cooking_Game_ForceSeekFreeRecookGUDS.Checked)
				{
					CommandLine += " -recookseekfreeguds";
				}
			}

			// @todo: Make this default for shipped version of this app
			if(CheckBox_Cooking_CookMod.Checked || ComboBox_Platform.Text == "PS3 Mod")
			{
				ConsoleInterface.Platform CurPlateform = (ConsoleInterface.Platform)ComboBox_Platform.SelectedItem;

				CommandLine += " -user";

				if(CurPlateform.Type == ConsoleInterface.PlatformType.Xbox360 || CurPlateform.Type == ConsoleInterface.PlatformType.PS3)
				{
					CommandLine += " -usermodname=" + ModNameDialog.String;
				}

				if(ComboBox_Platform.Text == "PS3 Mod")
				{
					CommandLine += " -installed";
				}
			}

			// Add in the final release option if necessary
			if(CheckBox_Cooking_CookFinalReleaseScript.Checked)
			{
				CommandLine += " -final_release";
			}

			// Add in the final release option if necessary
			if(Action == CommandletAction.Cooking_FullRecook)
			{
				CommandLine += " -full";
			}

			// Whether to force recook sounds
			if(CheckBox_Cooking_ForceSoundRecook.Checked)
			{
				CommandLine += " -forcesoundrecook";
			}

			// Add in the final release option if necessary
			if(Action == CommandletAction.Cooking_AllMaps)
			{
				CommandLine += " -cookallmaps";
			}

			if(UserData.LanguageList[UserData.CurrentLanguage] != "INT")
			{
				CommandLine += " -languageforcooking=" + UserData.LanguageList[UserData.CurrentLanguage];
			}

			if(CheckBox_Cooking_DisablePackageCompression.Checked)
			{
				CommandLine += " -nopackagecompression";
			}

			if(ComboBox_Cooking_AdditionalOptions.Text.Length > 0)
			{
				CommandLine += " " + ComboBox_Cooking_AdditionalOptions.Text;
			}

			// we always want to have the latest .inis (this stops one from firing off the cooker and then coming back and seeing the "update .inis" message 
			// and then committing seppuku)
			CommandLine += " -updateInisAuto";

			return CommandLine;
		}

		/// <summary>
		/// Imports a list of maps from a text file.
		/// </summary>
		/// <param name="filename">The file to import the maps from.</param>
		private void ImportMapList(string filename)
		{
			System.IO.StreamReader streamIn = new System.IO.StreamReader(filename);
			ComboBox_Cooking_MapsToCook.Text = streamIn.ReadToEnd().Replace("\r\n", " ");
			streamIn.Close();
		}

		/// <summary>
		/// Event handler for importing a list of maps.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnImportMapListClick(object sender, EventArgs e)
		{
			OpenFileDialog ofd;
			ofd = new OpenFileDialog();
			ofd.Filter = "TXT Files (*.txt)|*.txt";
			if(ofd.ShowDialog() == DialogResult.OK)
			{
				ImportMapList(ofd.FileName);
			}
		}

		/// <summary>
		/// Event handler for starting cooking.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnStartCookingClick(object sender, System.EventArgs e)
		{
			HandleCookAction(CommandletAction.Cooking);
		}

		/// <summary>
		/// Performs common pre-cook tasks.
		/// </summary>
		/// <param name="Action">The type of cooking to be performed.</param>
		private void HandleCookAction(CommandletAction Action)
		{
			string MapsToCook = ComboBox_Cooking_MapsToCook.Text.Trim();

			if(MapsToCook.Length > 0 && !ComboBox_Cooking_MapsToCook.Items.Contains(MapsToCook))
			{
				ComboBox_Cooking_MapsToCook.Items.Add(MapsToCook);
			}

			if(ComboBox_Cooking_AdditionalOptions.Text.Length > 0 && !ComboBox_Cooking_AdditionalOptions.Items.Contains(ComboBox_Cooking_AdditionalOptions.Text))
			{
				ComboBox_Cooking_AdditionalOptions.Items.Add(ComboBox_Cooking_AdditionalOptions.Text);
			}

			ConsoleInterface.Platform CurPlatform = (ConsoleInterface.Platform)ComboBox_Platform.SelectedItem;

			if(CurPlatform == null)
			{
				return;
			}

			// handle files (TextureFileCache especially) being open on PC when trying to cook
			if(CurPlatform.Type == ConsoleInterface.PlatformType.PS3 && Action != CommandletAction.Cooking_IniIntsOnly)
			{
				// reboot any running PS3s
				foreach(TargetListViewItem CurItem in ListBox_Targets.Items)
				{
					if(CurItem.Checked)
					{
						CurItem.Target.Reboot();
					}
				}
			}

			// if no running commandlet, then start it
			if(CommandletProcess == null || CommandletProcess.HasExited)
			{
				if(CheckBox_Cooking_CookMod.Checked && (CurPlatform.Type == ConsoleInterface.PlatformType.PS3 || CurPlatform.Type == ConsoleInterface.PlatformType.Xbox360))
				{
					if(ModNameDialog.ShowDialog() != DialogResult.OK)
					{
						return;
					}
				}

				CookingUserData UserData = new CookingUserData(CheckBox_Cooking_SkipScriptCompile.Checked ? CommandletAction.Cooking : Action, GetLanguagesToCookAndSync());

				StartCommandlet(CommandletCategory.Cooking, CheckBox_Cooking_SkipScriptCompile.Checked ? Action : CommandletAction.Cooking_CompileScript, UserData);
			}
			else
			{
				CheckedStopCommandlet();
			}
		}

		#endregion

		#region PC setup tab

		/// <summary>
		/// Generates the command line for compiling scripts.
		/// </summary>
		/// <returns>The commandline for compiling scripts.</returns>
		public string BuildPCCommandLine()
		{
			StringBuilder Bldr = new StringBuilder();

			switch((Configuration)ComboBox_Configuration.SelectedItem)
			{
				case Configuration.Debug:
					{
						Bldr.Append("-debug ");
						break;
					}
				case Configuration.FinalRelease:
					{
						Bldr.Append("-final_release ");
						break;
					}
			}

			if(CheckBox_Compiling_FullRecompile.Checked)
			{
				Bldr.Append("-full ");
			}

			if(CheckBox_Compiling_AutoMode.Checked)
			{
				Bldr.Append("-auto -unattended -updateinisauto ");
			}

			if(CheckBox_Compiling_VerboseMode.Checked)
			{
				Bldr.Append("-verbose ");
			}

			return Bldr.ToString().Trim();
		}

		/// <summary>
		/// Generates the commandline for compiling scripts.
		/// </summary>
		/// <returns>The commandline for compiling scripts.</returns>
		private string GetCompileScriptCommandLine()
		{
			// plain make commandline
			string CommandLine = BuildPCCommandLine();

			if(CommandLine.Length > 0)
			{
				CommandLine = "make " + CommandLine;
			}
			else
			{
				CommandLine = "make";
			}

			return CommandLine;
		}

		/// <summary>
		/// Event handler for compiling scripts or for PS3.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnCompileClick(object sender, EventArgs e)
		{
			try
			{
				// if its not running, start it
				if(CommandletProcess == null || CommandletProcess.HasExited)
				{
					StartCommandlet(CommandletCategory.Compiling, CommandletAction.CompilingScript, null);
				}
				else
				{
					CheckedStopCommandlet();
				}
			}
			catch(Exception ex)
			{
				AddLine(Color.Red, ex.ToString());
			}
		}

		#endregion

		#region Console setup tab

		/// <summary>
		/// Sets the UI state for copying.
		/// </summary>
		/// <param name="On">True if files are being copied.</param>
		private void SetCopyingState(bool On)
		{
			if(On)
			{
				ToolStripButton_Cook.Enabled = false;
				ToolStripButton_Sync.Enabled = false;
				ToolStripButton_Reboot.Enabled = false;
				TabControl_Main.Enabled = false;
			}
			else
			{
				ConsoleInterface.Platform Platform = ComboBox_Platform.SelectedItem as ConsoleInterface.Platform;

				ToolStripButton_Cook.Enabled = true;
				ToolStripButton_Sync.Enabled = Platform.NeedsToSync;
				ToolStripButton_Reboot.Enabled = true;
				TabControl_Main.Enabled = true;
			}
		}

		/// <summary>
		/// Event handler for syncing to a target.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnCopyToTargetsClick(object sender, System.EventArgs e)
		{
			if(CommandletProcess != null && !CommandletProcess.HasExited)
			{
				CheckedStopCommandlet();
			}
			else
			{
				// Copy data to console.
				StartCommandlet(CommandletCategory.Syncing, CommandletAction.Syncing, "CookerSync.exe", null);
			}
		}

		/// <summary>
		/// Generates the TOC for the current platform.
		/// </summary>
		/// <returns>The TOC for the current platform.</returns>
		ConsoleInterface.TOCSettings CreateTOCSettings()
		{
			UpdateConsoleBaseDirectoryHistory();

			ConsoleInterface.TOCSettings BuildSettings = new ConsoleInterface.TOCSettings(new ConsoleInterface.OutputHandlerDelegate(OutputHandler));

			foreach(TargetListViewItem CurItem in ListBox_Targets.Items)
			{
				if(CurItem.Checked)
				{
					BuildSettings.TargetsToSync.Add(CurItem.Target.Name);
				}
			}

			BuildSettings.GameName = ComboBox_Game.Text;
			BuildSettings.TargetBaseDirectory = ComboBox_ConsoleTargets_ConsoleBaseDir.Text;
			BuildSettings.Languages = GetLanguagesToCookAndSync();

			return BuildSettings;
		}

		/// <summary>
		/// Output handler for commandlets.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		void OutputHandler(object sender, ConsoleInterface.OutputEventArgs e)
		{
			if(e.Message.EndsWith(Environment.NewLine))
			{
				AddLine(e.TextColor, e.Message.Substring(0, e.Message.Length - Environment.NewLine.Length));
			}
		}

		/// <summary>
		/// Sync the currently selected build to the prop-01 server so UnrealProp can prop it
		/// </summary>
		private void OnUnrealPropClick(object sender, EventArgs e)
		{
			// Copy to the \\prop-01 server - emulate the following command line
			// CookerSync.exe Gear -b UnrealEngine3 -p Xenon \\prop-01\Builds\Gear\Xenon\Gear_Xenon_[2008-02-13_02.00]

			SetCopyingState(true);

			// Set up CookerTools to copy to UnrealProp
			string GameName = ComboBox_Game.Text.Replace("Game", "");
			string PlatformName = ComboBox_Platform.Text;

			if(PlatformName.Equals(ConsoleInterface.PlatformType.Xbox360.ToString(), StringComparison.OrdinalIgnoreCase))
			{
				PlatformName = "Xenon";
			}

			DateTime Timestamp = DateTime.Now;
			string TimeStampString = Timestamp.Year + "-"
									+ Timestamp.Month.ToString("00") + "-"
									+ Timestamp.Day.ToString("00") + "_"
									+ Timestamp.Hour.ToString("00") + "."
									+ Timestamp.Minute.ToString("00");

			DirectoryInfo Branch = Directory.GetParent(Directory.GetCurrentDirectory());
			string DestPath = string.Format( "\\\\prop-01\\Builds\\{0}User\\{1}\\{0}_{1}_[{2}]_[{3}]", GameName, PlatformName, TimeStampString, Environment.UserName.ToUpper() );

			List<string> DestPathList = new List<string>();
			DestPathList.Add(DestPath);

			ConsoleInterface.TOCSettings BuildSettings = CreateTOCSettings();

			// we don't want to sync to a console so clear targets
			BuildSettings.TargetsToSync.Clear();
			BuildSettings.Force = false;
			BuildSettings.NoSync = false;
			BuildSettings.ComputeCRC = false;
			BuildSettings.MergeExistingCRC = false;
			BuildSettings.IsForShip = false;
			BuildSettings.VerifyCopy = false;
			BuildSettings.SleepDelay = 25;
			BuildSettings.TargetBaseDirectory = BuildSettings.TargetBaseDirectory.Replace(" ", "_");
			BuildSettings.DestinationPaths = DestPathList;

			// for PC sync it shouldn't matter which platform we use
			ConsoleInterface.Platform CurPlatform = (ConsoleInterface.Platform)ComboBox_Platform.SelectedItem;
			CurPlatform.TargetSync(BuildSettings, "CompleteBuild", false, false);

			SetCopyingState(false);
		}

		private void OnRebootTargetsClick(object sender, System.EventArgs e)
		{
			CheckedReboot(false, null);
		}

		#endregion

		#region Help tab

		/// <summary>
		/// Shows help.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void OnTextBoxHelpClicked(object sender, LinkClickedEventArgs e)
		{
			System.Diagnostics.Process.Start(e.LinkText);
		}

		#endregion

		#endregion

		/// <summary>
		/// Copies the single player URL to the clipboard.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void MenuMain_Edit_CopyToClipboard_SPURL_Click(object sender, EventArgs e)
		{
			string GameOptions, EngineOptions;
			GetSPURLOptions(out GameOptions, out EngineOptions);

			Clipboard.SetText(GetFinalURL(GameOptions, EngineOptions, ECurrentConfig.SP, false), TextDataFormat.UnicodeText);
		}

		/// <summary>
		/// Copies the multiplayer URL to the clipboard.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void MenuMain_Edit_CopyToClipboard_MPURL_Click(object sender, EventArgs e)
		{
			string GameOptions, EngineOptions;
			GetMPURLOptions(out GameOptions, out EngineOptions);

			Clipboard.SetText(GetFinalURL(GameOptions, EngineOptions, ECurrentConfig.MP, false), TextDataFormat.UnicodeText);
		}

		/// <summary>
		/// Copies the compile URL to the clipboard (context sensitive).
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void MenuMain_Edit_CopyToClipboard_Compile_Click(object sender, EventArgs e)
		{
			Clipboard.SetText(GetCompileScriptCommandLine(), TextDataFormat.UnicodeText);
		}

		/// <summary>
		/// Copies the cooking commandline to the clipboard.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void MenuMain_Edit_CopyToClipboard_Cook_Click(object sender, EventArgs e)
		{
			Clipboard.SetText(GetCookingCommandLine(CommandletAction.Cooking, new CookingUserData(CommandletAction.Cooking, GetLanguagesToCookAndSync())), TextDataFormat.UnicodeText);
		}

		/// <summary>
		/// Event handler for exiting UFE.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void MenuMain_File_Exit_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		/// <summary>
		/// Event handler for when the game tab has been selected.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Tab_Game_Enter(object sender, EventArgs e)
		{
			ToolStripButton_Launch.ForeColor = SystemColors.HotTrack;
			ToolStripButton_Server.ForeColor = SystemColors.HotTrack;
			ToolStripButton_Editor.ForeColor = SystemColors.HotTrack;
		}

		/// <summary>
		/// Event handler for when the game tab has been deselected.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Tab_Game_Leave(object sender, EventArgs e)
		{
			ToolStripButton_Launch.ForeColor = SystemColors.ControlText;
			ToolStripButton_Server.ForeColor = SystemColors.ControlText;
			ToolStripButton_Editor.ForeColor = SystemColors.ControlText;
		}

		/// <summary>
		/// Event handler for when the cooking tab has been selected.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Tab_Cooking_Enter(object sender, EventArgs e)
		{
			ToolStripButton_Cook.ForeColor = SystemColors.HotTrack;
		}

		/// <summary>
		/// Event handler for when the cooking tab has been deselected.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Tab_Cooking_Leave(object sender, EventArgs e)
		{
			ToolStripButton_Cook.ForeColor = SystemColors.ControlText;
		}

		/// <summary>
		/// Event handler for when the compiling tab has been selected.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Tab_Compiling_Enter(object sender, EventArgs e)
		{
			ToolStripButton_CompileScript.ForeColor = SystemColors.HotTrack;
		}

		/// <summary>
		/// Event handler for when the compiling tab has been deselected.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Tab_Compiling_Leave(object sender, EventArgs e)
		{
			ToolStripButton_CompileScript.ForeColor = SystemColors.ControlText;
		}

		/// <summary>
		/// Event handler for when a key has been pressed in the base dir text box.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void TextBox_Game_ConsoleBaseDir_KeyDown(object sender, KeyEventArgs e)
		{
			char[] InvalidPathChars = Path.GetInvalidPathChars();

			foreach(char InvalidChar in InvalidPathChars)
			{
				if(e.KeyValue == InvalidChar && e.KeyValue != '\b')
				{
					e.SuppressKeyPress = true;
					break;
				}
			}

			// for some reason '/' seems to be returned as 191 for text boxes
			if(e.KeyValue == '/' || e.KeyValue == 191)
			{
				e.SuppressKeyPress = true;
			}
		}

		/// <summary>
		/// Event handler for toggling executable name overrides.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void CheckBox_Game_OverrideExecutable_CheckedChanged(object sender, EventArgs e)
		{
			this.OverrideExecutable = CheckBox_Game_OverrideExecutable.Checked;
			TextBox_Game_ExecutableNameOverride.ReadOnly = !CheckBox_Game_OverrideExecutable.Checked;
		}

		/// <summary>
		/// Event handler for when the name of the executable override has changed.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void TextBox_Game_ExecutableNameOverride_TextChanged(object sender, EventArgs e)
		{
			this.ExecutableNameOverride = TextBox_Game_ExecutableNameOverride.Text;
		}

		/// <summary>
		/// Event handler for the maps to cook tooltip timer.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void Cooking_MapToCookToolTipTimer_Tick(object sender, EventArgs e)
		{
			Cooking_MapToCookToolTipTimer.Stop();

			if(ComboBox_Cooking_MapsToCook.Focused)
			{
				string[] Filters = ComboBox_Cooking_MapsToCook.Text.Trim().Split(' ');

				string Msg = BuildCookMapToolTip(Filters);

				Msg += "\r\nHit the \'Enter\' key to automatically fill in the list of maps.\r\nMultiple search strings can be separated by a space.";

				CookMapToolTip.Show(Msg, ComboBox_Cooking_MapsToCook, ComboBox_Cooking_MapsToCook.Width, ComboBox_Cooking_MapsToCook.Height, 10000);
			}
		}

		/// <summary>
		/// Event handler for cooking global shaders only.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void ToolStripButton_CookGlobalShaders_Click(object sender, EventArgs e)
		{
			bool bContinue = true;
			ConsoleInterface.Platform CurPlatform = (ConsoleInterface.Platform)ComboBox_Platform.SelectedItem;

			if(CurPlatform == null)
			{
				return;
			}

			if(CommandletProcess != null && !CommandletProcess.HasExited)
			{
				bContinue = CheckedStopCommandlet();
			}

			if(bContinue)
			{
				StartCommandlet(CommandletCategory.Cooking, CommandletAction.Cooking_CompileScript, CommandletAction.Cooking_GlobalShadersOnly);
			}
		}

		/// <summary>
		/// Event handler for showing all target information.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void CheckBox_ConsoleTargets_ShowAllInfo_CheckedChanged(object sender, EventArgs e)
		{
			UpdateConsoleTargetsTab(true);
		}

		/// <summary>
		/// Event handler for performing a full recook.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void ToolStripButton_FullRecook_Click(object sender, EventArgs e)
		{
			if(MessageBox.Show(this, "Performing a full recook will delete all cached data from a previous cook. Are you sure you want to do this?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.Yes)
			{
				HandleCookAction(CommandletAction.Cooking_FullRecook);
			}
		}

		/// <summary>
		/// Event handler for cooking .ini/.int's only.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void ToolStripButton_CookIntsOnly_Click(object sender, EventArgs e)
		{
			HandleCookAction(CommandletAction.Cooking_IniIntsOnly);
		}

		/// <summary>
		/// Event handler for cooking all maps.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void ToolStripButton_CookAllMaps_Click(object sender, EventArgs e)
		{
			HandleCookAction(CommandletAction.Cooking_AllMaps);
		}

		/// <summary>
		/// Retrieves the list of languages to use for cooking and syncing.
		/// </summary>
		/// <returns>An array of languages to use to for cooking and syncing.</returns>
		private string[] GetLanguagesToCookAndSync()
		{
			List<string> Languages = new List<string>();
			bool bHasInt = false;

			foreach(ListViewItem CurItem in ListView_Cooking_Languages.Items)
			{
				if(CurItem.Checked)
				{
					if(CurItem.Text.Equals("INT", StringComparison.OrdinalIgnoreCase))
					{
						bHasInt = true;
					}
					else
					{
						Languages.Add(CurItem.Text);
					}
				}
			}

			// INT must be last in the list of languages 
			if(Languages.Count == 0)
			{
				Languages.Add("INT");
			}
			else if(bHasInt)
			{
				// NOTE: There is currently a bug in the cooker when cooking multiple languages that requires you to cook INT first and last if you want to
				// cook INT + 1 or more other languages.
				Languages.Insert(0, "INT");
				Languages.Add("INT");
			}

			return Languages.ToArray();
		}

		/// <summary>
		/// Event handler for logging output.
		/// </summary>
		/// <param name="State">Context information.</param>
		private void OnOutputTimer(object State)
		{
			string Buf;

			lock(mOutputBuffer)
			{
				Buf = mOutputBuffer.ToString();
				mOutputBuffer.Length = 0;
			}

			try
			{
				if(Buf.Length > 0)
				{
					string LogName = string.Format("{0}{1}.txt", LOG_DIR, DateTime.Now.ToString("M-dd-yyyy"));

					if(!Directory.Exists(LOG_DIR))
					{
						Directory.CreateDirectory(LOG_DIR);
					}

					File.AppendAllText(LogName, Buf, Encoding.UTF8);
				}
			}
			catch(Exception ex)
			{
				System.Diagnostics.Debug.WriteLine(ex.ToString());
			}
		}

		/// <summary>
		/// Event handler for when the balloon tool tip is closed.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void NotifyIcon_CommandletFinished_BalloonTipClosed(object sender, EventArgs e)
		{
			NotifyIcon_CommandletFinished.Visible = false;
		}

		/// <summary>
		/// Event handler for when the balloon tool tip is clicked.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void NotifyIcon_CommandletFinished_BalloonTipClicked(object sender, EventArgs e)
		{
			NotifyIcon_CommandletFinished.Visible = false;
		}
	}
}