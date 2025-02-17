// VsPkg.cs : Implementation of UCDebuggerPkg
//

using System;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.ComponentModel.Design;
using System.IO;
using Microsoft.Win32;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.VCProject;
using Microsoft.VisualStudio.VCProjectEngine;
using EnvDTE;

namespace Epic.UnrealDebugger2005
{
    /// <summary>
    /// This is the class that implements the package exposed by this assembly.
    ///
    /// The minimum requirement for a class to be considered a valid package for Visual Studio
    /// is to implement the IVsPackage interface and register itself with the shell.
    /// This package uses the helper classes defined inside the Managed Package Framework (MPF)
    /// to do it: it derives from the Package class that provides the implementation of the 
    /// IVsPackage interface and uses the registration attributes defined in the framework to 
    /// register itself and its components with the shell.
    /// </summary>
    // This attribute tells the registration utility (regpkg.exe) that this class needs
    // to be registered as package.
    [PackageRegistration(UseManagedResourcesOnly = true)]
    // A Visual Studio component can be registered under different regitry roots; for instance
    // when you debug your package you want to register it in the experimental hive. This
    // attribute specifies the registry root to use if no one is provided to regpkg.exe with
    // the /root switch.
    [DefaultRegistryRoot("Software\\Microsoft\\VisualStudio\\8.0")]
    // This attribute is used to register the informations needed to show the this package
    // in the Help/About dialog of Visual Studio.
    [InstalledProductRegistration(false, "#110", "#112", "1.0", IconResourceID = 400)]
    // In order be loaded inside Visual Studio in a machine that has not the VS SDK installed, 
    // package needs to have a valid load key (it can be requested at 
    // http://msdn.microsoft.com/vstudio/extend/). This attributes tells the shell that this 
    // package has a load key embedded in its resources.
    [ProvideLoadKey("Standard", "1", "UnrealDebugger2005", "Epic Games", 105)]
    // This attribute is needed to let the shell know that this package exposes some menus.
    [ProvideMenuResource(1000, 1)]
    // This attribute registers a tool window exposed by this package.
    [ProvideToolWindow(typeof(UnrealWatchPane))]
    // Load when a sln opens
    [ProvideAutoLoad(UIContextGuids.SolutionExists)]

    [Guid(GuidList.guidUCDebuggerPkgPkgString)]
    public sealed class UCDebuggerPkg : Package, IVsSolutionEvents, IVsDebuggerEvents
    {
        const int bitmapResourceID = 300;

        /// <summary>
        /// Default constructor of the package.
        /// Inside this method you can place any initialization code that does not require 
        /// any Visual Studio service because at this point the package object is created but 
        /// not sited yet inside Visual Studio environment. The place to do all the other 
        /// initialization is the Initialize method.
        /// </summary>
        public UCDebuggerPkg()
        {
            Trace.WriteLine(string.Format(CultureInfo.CurrentCulture, "Entering constructor for: {0}", this.ToString()));
        }

        /// <summary>
        /// This function is called when the user clicks the menu item that shows the 
        /// tool window. See the Initialize method to see how the menu item is associated to 
        /// this function using the OleMenuCommandService service and the MenuCommand class.
        /// </summary>
        private void ShowToolWindow(object sender, EventArgs e)
        {
            ShowToolWindow();
        }
        private void ShowToolWindow()
        {
            // Get the instance number 0 of this tool window. This window is single instance so this instance
            // is actually the only one.
            // The last flag is set to true so that if the tool window does not exists it will be created.
            ToolWindowPane window = this.FindToolWindow(typeof(UnrealWatchPane), 0, true);
            if ((null == window) || (null == window.Frame))
            {
                throw new COMException(Resources.CanNotCreateWindow);
            }
            IVsWindowFrame windowFrame = (IVsWindowFrame)window.Frame;
            Microsoft.VisualStudio.ErrorHandler.ThrowOnFailure(windowFrame.Show());
        }

        //////////////////////////////////////////////////////////////////////////
        private uint cookie;
        IVsDebugger _debugger;
        EnvDTE.DTE _dte;
        FileSystemWatcher fileWatcher;


        /// <summary>
        /// This method recursively searches through a list of solution/project items for a
        /// specific project file based on the project name.
        /// </summary>
        /// <param name="ProjectName">the name of the project to search for (i.e. PCLaunch-ExampleGame)</param>
        /// <param name="ProjectList">the list of solution or project items to search through</param>
        /// <returns>a reference to the project with the specified name, or null if it isn't found</returns>
        public Project FindStartupProject(string ProjectName, ProjectItems ProjectList)
        {
            if (ProjectList != null)
            {
                foreach (ProjectItem proj in ProjectList)
                {
                    if (proj.SubProject != null)
                    {
                        if (proj.SubProject.Name == ProjectName)
                        {
                            return proj.SubProject;
                        }

                        Project NestedProj = FindStartupProject(ProjectName, proj.SubProject.ProjectItems);
                        if (NestedProj != null)
                        {
                            return NestedProj;
                        }
                    }
                }
            }

            return null;
        }

        /// <summary>
        /// Finds a specific project file in the current solution using the project's name.
        /// </summary>
        /// <param name="ProjectName">the name of the project to search for</param>
        /// <returns>a reference to the project with the specified name, or null if it isn't found</returns>
        public Project FindStartupProject(string ProjectName)
        {
            if (_dte.Solution.Projects != null)
            {
                foreach (Project proj in _dte.Solution.Projects)
                {
                    if (proj.Name == ProjectName)
                    {
                        return proj;
                    }

                    // search this project's subitems
                    Project NestedProj = FindStartupProject(ProjectName, proj.ProjectItems);
                    if (NestedProj != null)
                    {
                        return NestedProj;
                    }
                }
            }

            return null;
        }

        public bool GetGameTargetInfo()
        {
            try
            {
                EnvDTE.SolutionBuild sb = _dte.Solution.SolutionBuild;
                Array ar = (Array)sb.StartupProjects;
				if (ar != null)
				{
					string StartupProjectPath = (string)ar.GetValue(0);
					string StartupProjectName = Path.GetFileNameWithoutExtension(StartupProjectPath);
					Project StartupProj = FindStartupProject(StartupProjectName);
					if (StartupProj == null)
					{
						throw new Exception("Invalid startup project selected:" + StartupProjectPath);
					}

					Configuration conf = StartupProj.ConfigurationManager.ActiveConfiguration;
					EnvDTE.Property cmd = conf.Properties.Item("Command");
					EnvDTE.Property args = conf.Properties.Item("CommandArguments");
					EnvDTE.Property remoteMachine = conf.Properties.Item("RemoteMachine");
					// currently using localhost only
					//                     if (remoteMachine.Value != null)
					//                         WTGlobals.WT_TARGET = remoteMachine.Value.ToString();
					//                     else
					//                         WTGlobals.WT_TARGET = "localhost";

					VCProject vcprj = (VCProject)StartupProj.Object;
					IVCCollection vcconfigs = (IVCCollection)vcprj.Configurations;
					VCConfiguration vcconf = (VCConfiguration)vcconfigs.Item(conf.ConfigurationName);
					if (args.Value != null)
					{
						WTGlobals.WT_COMMANDLINEARGS = args.Value.ToString();

						// if the user didn't specify -vadebug in the command line arguments, add that now since it's required.
						string ls = args.Value.ToString().ToLower();
						if (ls.IndexOf("-vadebug") == -1)
						{
							WTGlobals.WT_COMMANDLINEARGS = "-vadebug " + WTGlobals.WT_COMMANDLINEARGS;
						}
					}
					else
					{
						// if the user didn't specify -vadebug in the command line arguments, add that now since it's required.
						WTGlobals.WT_COMMANDLINEARGS = "-vadebug";
					}

					if (cmd.Value != null)
					{
						string exe = vcconf.Evaluate(cmd.Value.ToString());
						WTGlobals.SetGameExe(exe);
						return true;
					}
					else
					{
						throw new Exception("The startup project has no path set for the game's .exe");
					}
				}
				else
				{
					throw new Exception("No startup project selected!");
				}
            }
            catch (System.Exception e)
            {
                Alert("Error", e.ToString());
                return false;
            }
        }

        public void LaunchUCDebugger()
        {
            // Let VS Launch the game
            if (!GetGameTargetInfo())
                return;

            System.Diagnostics.Process game = new System.Diagnostics.Process();
            game.StartInfo.FileName = WTGlobals.WT_GAMEPATH;
			game.StartInfo.Arguments = WTGlobals.WT_COMMANDLINEARGS;

            if (game.Start())
            {
                AttachToGame();
            }
        }
        public void AttachToGame()
        {
            // Launch our debugger
            if (WTGlobals.gAttached)
                return;
            
            WTGlobals.gAttached = true;
            
            VsDebugTargetInfo tinfo = new VsDebugTargetInfo();
            tinfo.cbSize = (uint)Marshal.SizeOf(tinfo);
            tinfo.bstrExe = WTGlobals.WT_SDK_DLL;
            tinfo.dlo = DEBUG_LAUNCH_OPERATION.DLO_CreateProcess;
            
            Guid guid = new Guid("{9195946B-D25F-4001-8C20-B22A743013C7}"); // GUID of our debug engine
            IntPtr pguid = Marshal.AllocCoTaskMem(Marshal.SizeOf(guid));
            Marshal.StructureToPtr(guid, pguid, false);
            tinfo.pClsidList = pguid;
            tinfo.dwClsidCount = 1;
            tinfo.fSendStdoutToOutputWindow = 0;

            IntPtr ptr = Marshal.AllocCoTaskMem((int)tinfo.cbSize);
            Marshal.StructureToPtr(tinfo, ptr, false);

            try
            {
                int i = _debugger.LaunchDebugTargets(1, ptr);
				ShowToolWindow();
            }
            catch (Exception e)
            {
                Alert("Error", e.ToString());
            }
        }
        public int OnModeChange(DBGMODE dbgmodeNew)
        {
            if (dbgmodeNew == DBGMODE.DBGMODE_Design)
			{
				WTGlobals.gAttached = false;
			}
            return 0;
        }

        /////////////////////////////////////////////////////////////////////////////
        // Overriden Package Implementation
        #region Package Members

        /// <summary>
        /// Initialization of the package; this method is called right after the package is sited, so this is the place
        /// where you can put all the initilaization code that rely on services provided by VisualStudio.
        /// </summary>
        protected override void Initialize()
        {
            Trace.WriteLine(string.Format(CultureInfo.CurrentCulture, "Entering Initialize() of: {0}", this.ToString()));
            base.Initialize();
            // Add our command handlers for menu (commands must exist in the .ctc file)
            OleMenuCommandService mcs = GetService(typeof(IMenuCommandService)) as OleMenuCommandService;
            if (null != mcs)
            {
                // Create the command for the menu item.
                CommandID menuCommandID = new CommandID(GuidList.guidUCDebuggerPkgCmdSet, (int)PkgCmdIDList.cmdidLaunchDebugger);
                MenuCommand menuItem = new MenuCommand(new EventHandler(LaunchUCDebuggerCallback), menuCommandID);
                mcs.AddCommand(menuItem);
                // Create the command for the tool window
                CommandID toolwndCommandID = new CommandID(GuidList.guidUCDebuggerPkgCmdSet, (int)PkgCmdIDList.cmdidUnrealWatch);
                MenuCommand menuToolWin = new MenuCommand(new EventHandler(ShowToolWindow), toolwndCommandID);
                mcs.AddCommand(menuToolWin);
                // Create the command for the menu item.
                CommandID menuCommandAttachID = new CommandID(GuidList.guidUCDebuggerPkgCmdSet, (int)PkgCmdIDList.cmdidAttachDebugger);
                MenuCommand menuAttachItem = new MenuCommand(new EventHandler(AttachToGameCallback), menuCommandAttachID);
                mcs.AddCommand(menuAttachItem);
            }
            _dte = (EnvDTE.DTE)GetService(typeof(EnvDTE.DTE));
            _debugger = (IVsDebugger)GetService(typeof(IVsDebugger));
            _debugger.AdviseDebuggerEvents(this, out cookie);

            IVsSolution sol = (IVsSolution)GetService(typeof(IVsSolution));
            sol.AdviseSolutionEvents(this, out cookie);
            WTGlobals.gUCDebuggerPkg = this;
            fileWatcher = new FileSystemWatcher();
            //fileWatcher.SynchronizingObject = this;
            fileWatcher.Filter = "attach.txt";
            fileWatcher.Changed += new FileSystemEventHandler(OnWatchFileChange);
            myTimer.Tick += new EventHandler(TimerEventProcessor);
            myTimer.Interval = 500;
        }


        private void WatchForGameAttach()
        {
            try
            {
                if (_dte.Solution.IsOpen)
                {
                    string sln_path = Path.GetDirectoryName(_dte.Solution.FileName);
                    if (sln_path.ToLower().Contains("unrealengine"))
                    {
                        String iniPath = System.IO.Path.GetTempPath() + "\\UCDebugger";
                        System.IO.Directory.CreateDirectory(iniPath);
                        string gamepath = Path.GetFullPath(sln_path + "\\..\\..\\Binaries\\game.exe");
                        WTGlobals.SetGameExe(gamepath);
                        fileWatcher.Path = iniPath;
                        fileWatcher.EnableRaisingEvents = true;

                        if (File.Exists(WTGlobals.WT_ATTACHFILE))
                        {
							//@fixme ronp - disable for now; doesn't work reliably enough to make it worth it
							//@{
							// Cause event to fire
                            //myTimer.Stop();
                            //myTimer.Start();
							//@}
							try
							{
								File.Delete(WTGlobals.WT_ATTACHFILE);
							}
							catch (Exception ee)
							{
								Alert("Error", ee.ToString());
							}
						}
                    }
                }
            }
            catch (Exception e)
            {
                Alert("Error", e.ToString());
                throw e;
            }
        }
        void Alert(string title, string txt)
        {
            // Show a Message Box to prove we were here
            IVsUIShell uiShell = (IVsUIShell)GetService(typeof(SVsUIShell));
            Guid clsid = Guid.Empty;
            int result;
            uiShell.ShowMessageBox(
                       0,
                       ref clsid,
                       title,
                       txt,
                       string.Empty,
                       0,
                       OLEMSGBUTTON.OLEMSGBUTTON_OK,
                       OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST,
                       OLEMSGICON.OLEMSGICON_INFO,
                       0,        // false
                       out result);

        }
        private void OnAttach()
        {
            try
            {
                if (!WTGlobals.gAttached)
                {
                    if (File.Exists(WTGlobals.WT_ATTACHFILE))
                    {
                        try
                        {
                            File.Delete(WTGlobals.WT_ATTACHFILE);
                        }
                        catch (Exception /*ee*/)
                        {
                            /*Alert("Error", ee.ToString());*/
                        }
                        _dte.ExecuteCommand("Tools.UnrealAttach", "");
                    }
                }
            }
            catch (Exception ee)
            {
                Alert("Error", ee.ToString());
            }
        }
        // This is the method to run when the timer is raised.
        private void TimerEventProcessor(Object myObject, EventArgs myEventArgs)
        {
            myTimer.Stop();
            OnAttach();
         }

        private void OnWatchFileChange(object sender, FileSystemEventArgs e)
        {
            myTimer.Stop();
            OnAttach();
        }
        #endregion
        /// <summary>
        /// This function is the callback used to execute a command when the a menu item is clicked.
        /// See the Initialize method to see how the menu item is associated to this function using
        /// the OleMenuCommandService service and the MenuCommand class.
        /// </summary>
        private void LaunchUCDebuggerCallback(object sender, EventArgs e)
        {
            try
            {
                LaunchUCDebugger();
            }
            catch (System.Exception ee)
            {
                Alert("Error", ee.ToString());
            }
        }

        /// <summary>
        /// This function is the callback used to execute a command when the a menu item is clicked.
        /// See the Initialize method to see how the menu item is associated to this function using
        /// the OleMenuCommandService service and the MenuCommand class.
        /// </summary>
        private void AttachToGameCallback(object sender, EventArgs e)
        {
                try
                {
                    AttachToGame();
                }
                catch (System.Exception ee)
                {
                    Alert("Error", ee.ToString());
                }
            }


        #region IVsSolutionEvents Members

        public int OnAfterCloseSolution(object pUnkReserved)
        {
			if (fileWatcher != null)
			{
				fileWatcher.EnableRaisingEvents = false;
			}
            return 0;
        }

        public int OnAfterLoadProject(IVsHierarchy pStubHierarchy, IVsHierarchy pRealHierarchy)
        {
            return 1;
        }

        public int OnAfterOpenProject(IVsHierarchy pHierarchy, int fAdded)
        {
            return 1;
        }

        public int OnAfterOpenSolution(object pUnkReserved, int fNewSolution)
        {
            try
            {
                WatchForGameAttach();
            }
            catch (System.Exception e)
            {
                Alert("Error", e.ToString());
            }
            return 0;
        }

        public int OnBeforeCloseProject(IVsHierarchy pHierarchy, int fRemoved)
        {
            return 1;
        }

        public int OnBeforeCloseSolution(object pUnkReserved)
        {
            return 1;
        }

        public int OnBeforeUnloadProject(IVsHierarchy pRealHierarchy, IVsHierarchy pStubHierarchy)
        {
            return 1;
        }

        public int OnQueryCloseProject(IVsHierarchy pHierarchy, int fRemoving, ref int pfCancel)
        {
            //throw new Exception("The method or operation is not implemented.");
			return 1;
        }

        public int OnQueryCloseSolution(object pUnkReserved, ref int pfCancel)
        {
            //throw new Exception("The method or operation is not implemented.");
			return 1;
        }

        public int OnQueryUnloadProject(IVsHierarchy pRealHierarchy, ref int pfCancel)
        {
            //throw new Exception("The method or operation is not implemented.");
			return 1;
        }

        #endregion

        static System.Windows.Forms.Timer myTimer = new System.Windows.Forms.Timer();

    }
}