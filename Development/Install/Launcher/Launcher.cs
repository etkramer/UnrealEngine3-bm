using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Security;
using System.Security.Permissions;
using Microsoft.Win32;

namespace EpicGames
{
	[Flags]
	public enum EButtonFlags
	{
		None = 0,
		Launch = 1,
		Install = 1 << 1,
		Uninstall = 1 << 2,
		All = Launch | Install | Uninstall,
	}

    public partial class Launcher : Form
    {
		static readonly Color BUTTON_FOREGROUND = Color.White;
		static readonly Color BUTTON_DISABLED = Color.Black;
		//static readonly string MSI_INSTALLATION_KEY = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UserData\S-1-5-18\Products\90209AFBFFA76BD4E8B45E375057A17D";
		//static readonly string MSI_INSTALLATION_PROPERTIES_KEY = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UserData\S-1-5-18\Products\90209AFBFFA76BD4E8B45E375057A17D\InstallProperties";
		static readonly string INSTALLATION_KEY_x64 = @"SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\InstallShield_{BFA90209-7AFF-4DB6-8E4B-E57305751AD7}";
		static readonly string INSTALLATION_KEY_x86 = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\InstallShield_{BFA90209-7AFF-4DB6-8E4B-E57305751AD7}";

		EButtonFlags buttonFlags = EButtonFlags.All;

		[DllImport("Kernel32.dll")]
		[return:MarshalAs(UnmanagedType.Bool)]
		static extern bool IsWow64Process(IntPtr hProcess, [MarshalAs(UnmanagedType.Bool)]ref bool Wow64Process);

		static bool Is64BitOS
		{
			get
			{
				bool bRet = false;

				if(IntPtr.Size == 8)
				{
					bRet = true;
				}
				else
				{
					IsWow64Process(Process.GetCurrentProcess().Handle, ref bRet);
				}

				return bRet;
			}
		}

		static string InstallationKey
		{
			get
			{
				if(Is64BitOS)
				{
					return INSTALLATION_KEY_x64;
				}


				return INSTALLATION_KEY_x86;
			}
		}

        public Launcher()
        {
            InitializeComponent();

			// Handle the escape key for closing the form
			this.KeyDown += new KeyEventHandler(Launcher_KeyDown);

            // Set event handlers for changing button backgrounds
            ButtonLaunch.MouseEnter += new EventHandler(ButtonLaunch_MouseEnter);
            ButtonLaunch.MouseLeave += new EventHandler(ButtonLaunch_MouseLeave);

            ButtonInstall.MouseEnter += new EventHandler(ButtonInstall_MouseEnter);
            ButtonInstall.MouseLeave += new EventHandler(ButtonInstall_MouseLeave);

            ButtonUninstall.MouseEnter += new EventHandler(ButtonUninstall_MouseEnter);
            ButtonUninstall.MouseLeave += new EventHandler(ButtonUninstall_MouseLeave);

            Environment.CurrentDirectory = Application.StartupPath;

			UpdateInstallationStatus();

			buttonUpdateTimer.Tick += new EventHandler(buttonUpdateTimer_Tick);
			buttonUpdateTimer.Enabled = true;
        }

		static void OutputDebug(string message)
		{
			System.Diagnostics.Debug.WriteLine(message, "UT3Launcher");
		}

		void buttonUpdateTimer_Tick(object sender, EventArgs e)
		{
			UpdateInstallationStatus();
		}

		void UpdateInstallationStatus()
		{
			try
			{
				//NOTE: The GUID must be updated every time the GUID in the installer changes. That means for patches this program needs to be patched as well w/ the updated GUID
				RegistryKey key = Registry.LocalMachine.OpenSubKey(Launcher.InstallationKey, RegistryKeyPermissionCheck.Default, System.Security.AccessControl.RegistryRights.ReadKey);

				if(key == null)
				{
					buttonFlags = EButtonFlags.Install;
				}
				else
				{
					key.Close();
					buttonFlags = EButtonFlags.Launch | EButtonFlags.Uninstall;
				}
			}
			catch(Exception)
			{
				buttonFlags = EButtonFlags.All;
			}

			if((buttonFlags & EButtonFlags.Install) == EButtonFlags.Install)
			{
				ButtonInstall.ForeColor = BUTTON_FOREGROUND;
			}
			else
			{
				ButtonInstall.ForeColor = BUTTON_DISABLED;
			}

			if((buttonFlags & EButtonFlags.Launch) == EButtonFlags.Launch)
			{
				ButtonLaunch.ForeColor = BUTTON_FOREGROUND;
			}
			else
			{
				ButtonLaunch.ForeColor = BUTTON_DISABLED;
			}

			if((buttonFlags & EButtonFlags.Uninstall) == EButtonFlags.Uninstall)
			{
				ButtonUninstall.ForeColor = BUTTON_FOREGROUND;
			}
			else
			{
				ButtonUninstall.ForeColor = BUTTON_DISABLED;
			}
		}

		void Launcher_KeyDown(object sender, KeyEventArgs e)
		{
			if(e.KeyCode == Keys.Escape)
			{
				Application.Exit();
			}
		}

        private void Launch_Click( object sender, EventArgs e )
        {
			UpdateInstallationStatus();

			if((buttonFlags & EButtonFlags.Launch) == EButtonFlags.Launch)
			{
				try
				{
					RegistryKey key = Registry.LocalMachine.OpenSubKey(Launcher.InstallationKey, RegistryKeyPermissionCheck.Default, System.Security.AccessControl.RegistryRights.ReadKey);

					if(key != null)
					{
						string installDir = key.GetValue("InstallLocation", null, RegistryValueOptions.None) as string;
						key.Close();

						if(installDir != null)
						{
							Process UT3 = new Process();
#if DEBUG
							UT3.StartInfo.FileName = string.Format("{0}Binaries\\UTGame.exe", installDir);
							UT3.StartInfo.Arguments = "-seekfreeloading";
#else
							UT3.StartInfo.FileName = string.Format("{0}Binaries\\UT3.exe", installDir);
#endif
							UT3.StartInfo.UseShellExecute = true;
							UT3.Start();

							Application.Exit();
						}
					}
				}
				catch(Exception ex)
				{
					OutputDebug(ex.ToString());
				}
			}
        }
		void Install_Click(object sender, EventArgs e)
		{
			UpdateInstallationStatus();

			if((buttonFlags & EButtonFlags.Install) == EButtonFlags.Install)
			{
				try
				{
					Process setup = new Process();
					setup.StartInfo.FileName = "SetupUT3.exe";
					setup.StartInfo.UseShellExecute = true; //must be true for UAC to work properly

					buttonUpdateTimer.Enabled = false;

					Cursor currentCursor = this.Cursor;
					this.Cursor = Cursors.WaitCursor;

					setup.Start();
					setup.WaitForExit();

					this.Cursor = currentCursor;
				}
				catch(Exception ex)
				{
					OutputDebug(ex.ToString());
				}
				finally
				{
					buttonUpdateTimer.Enabled = true;
				}

				UpdateInstallationStatus();
			}
		}

		void Uninstall_Click(object sender, EventArgs e)
		{
			UpdateInstallationStatus();

			if((buttonFlags & EButtonFlags.Uninstall) == EButtonFlags.Uninstall)
			{
				try
				{
					RegistryKey key = Registry.LocalMachine.OpenSubKey(Launcher.InstallationKey, RegistryKeyPermissionCheck.Default, System.Security.AccessControl.RegistryRights.ReadKey);

					if(key != null)
					{
						string uninstallString = key.GetValue("UninstallString", null, RegistryValueOptions.None) as string;
						key.Close();

						if(uninstallString != null)
						{
							int endOfExePath = uninstallString.LastIndexOf('\"');

							Process msiexec = new Process();
							msiexec.StartInfo.FileName = uninstallString.Substring(1, endOfExePath - 1); //should be SetupUT3.exe

							string[] args = uninstallString.Substring(endOfExePath + 1).Split(' ');
							StringBuilder argBldr = new StringBuilder();
							foreach(string arg in args)
							{
								if(arg.Length > 0)
								{
									argBldr.AppendFormat("{0} ", arg);
								}
							}

							if(argBldr.Length > 0)
							{
								argBldr.Length = argBldr.Length - 1; //remove trailing space
								msiexec.StartInfo.Arguments = argBldr.ToString();
							}

							buttonUpdateTimer.Enabled = false;

							Cursor currentCursor = this.Cursor;
							this.Cursor = Cursors.WaitCursor;
							
							msiexec.StartInfo.UseShellExecute = true; //must be true for UAC to work properly
							msiexec.Start();
							msiexec.WaitForExit();
							
							this.Cursor = currentCursor;
						}
					}
				}
				catch(Exception ex)
				{
					OutputDebug(ex.ToString());
				}
				finally
				{
					buttonUpdateTimer.Enabled = true;
				}

				UpdateInstallationStatus();
			}
		}

        void ButtonUninstall_MouseLeave(object sender, EventArgs e)
        {
            ButtonUninstall.Image = null;
        }

        void ButtonUninstall_MouseEnter(object sender, EventArgs e)
        {
			if((buttonFlags & EButtonFlags.Uninstall) == EButtonFlags.Uninstall)
			{
				ButtonUninstall.Image = Properties.Resources.ut3_installbutton3;
			}
        }

        void ButtonInstall_MouseLeave(object sender, EventArgs e)
        {
            ButtonInstall.Image = null;
        }

        void ButtonInstall_MouseEnter(object sender, EventArgs e)
        {
			if((buttonFlags & EButtonFlags.Install) == EButtonFlags.Install)
			{
				ButtonInstall.Image = Properties.Resources.ut3_installbutton2;
			}
        }

        private void ButtonLaunch_MouseLeave(object sender, EventArgs e)
        {
            ButtonLaunch.Image = null;
        }

        private void ButtonLaunch_MouseEnter(object sender, EventArgs e)
        {
			if((buttonFlags & EButtonFlags.Launch) == EButtonFlags.Launch)
			{
				ButtonLaunch.Image = Properties.Resources.ut3_installbutton1;
			}
        }
    }
}