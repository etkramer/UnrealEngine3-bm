using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.ComponentModel;

namespace UnrealSync.Manager
{
	/// <summary>
	/// Custom application context for handling an invisible window on app startup.
	/// </summary>
	public class UnrealSyncApplicationContext : ApplicationContext
	{
		const string TT_SHOWMGR = "Show UnrealSync &Manager";
		const string TT_HIDEMGR = "Hide UnrealSync &Manager";

		MgrForm form = new MgrForm();
		IContainer components;
		NotifyIcon notifyIcon;
		ContextMenuStrip ctxTrayIcon;
		ToolStripMenuItem exitToolStripMenuItem;
		ToolStripSeparator seperator;
		ToolStripMenuItem showWindowToolStripMenuItem;

		public UnrealSyncApplicationContext(bool bStartVisible)
		{
			InitializeComponent();

			if(bStartVisible)
			{
				this.MainForm = this.form;
			}
		}

		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MgrForm));
			this.components = new Container();
			this.notifyIcon = new NotifyIcon(components);
			this.ctxTrayIcon = new ContextMenuStrip(this.components);
			this.exitToolStripMenuItem = new ToolStripMenuItem();
			this.seperator = new ToolStripSeparator();
			this.showWindowToolStripMenuItem = new ToolStripMenuItem();
			this.ctxTrayIcon.SuspendLayout();

			// 
			// seperator
			// 
			this.seperator.Name = "seperator";
			this.seperator.Size = new System.Drawing.Size(149, 6);
			// 
			// showWindowToolStripMenuItem
			// 
			this.showWindowToolStripMenuItem.Name = "showWindowToolStripMenuItem";
			this.showWindowToolStripMenuItem.Size = new System.Drawing.Size(92, 22);
			this.showWindowToolStripMenuItem.Text = TT_SHOWMGR;
			this.showWindowToolStripMenuItem.Click += new EventHandler(showWindowToolStripMenuItem_Click);
			// 
			// exitToolStripMenuItem
			// 
			this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
			this.exitToolStripMenuItem.Size = new System.Drawing.Size(92, 22);
			this.exitToolStripMenuItem.Text = "E&xit";
			this.exitToolStripMenuItem.Click += new EventHandler(exitToolStripMenuItem_Click);
			// 
			// ctxTrayIcon
			// 
			this.ctxTrayIcon.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
			this.showWindowToolStripMenuItem,
			this.seperator,
            this.exitToolStripMenuItem});
			this.ctxTrayIcon.Name = "ctxTrayIcon";
			this.ctxTrayIcon.Size = new System.Drawing.Size(93, 26);
			this.ctxTrayIcon.Opening += new CancelEventHandler(ctxTrayIcon_Opening);
			// 
			// notifyIcon
			// 
			this.notifyIcon.ContextMenuStrip = this.ctxTrayIcon;
			this.notifyIcon.Text = "UnrealSync";
			this.notifyIcon.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.notifyIcon.Visible = true;
			this.notifyIcon.MouseDoubleClick += new MouseEventHandler(notifyIcon_MouseDoubleClick);

			this.ctxTrayIcon.ResumeLayout(false);
		}

		void ctxTrayIcon_Opening(object sender, CancelEventArgs e)
		{
			if(this.MainForm == null || !this.MainForm.Visible)
			{
				this.showWindowToolStripMenuItem.Text = TT_SHOWMGR;
			}
			else
			{
				this.showWindowToolStripMenuItem.Text = TT_HIDEMGR;
			}
		}

		void showWindowToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if(this.MainForm == null || !this.MainForm.Visible)
			{
				ShowMainWindow();
			}
			else
			{
				this.MainForm.Hide();
			}
		}

		public void ShowMainWindow()
		{
			notifyIcon_MouseDoubleClick(notifyIcon, new MouseEventArgs(MouseButtons.Left, 2, 0, 0, 0));
		}

		protected override void Dispose(bool disposing)
		{
			if(disposing)
			{
				if(components != null)
				{
					components.Dispose();
					components = null;
				}

				if(this.MainForm == null && this.form != null)
				{
					this.form.Dispose();
				}

				this.form = null;
			}

			base.Dispose(disposing);
		}

		void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			ExitThread();
		}

		void notifyIcon_MouseDoubleClick(object sender, MouseEventArgs e)
		{
			if(this.MainForm == null)
			{
				this.MainForm = this.form;
				this.MainForm.Show();
			}
			else
			{
				if(this.MainForm.WindowState == FormWindowState.Minimized)
				{
					MgrForm tempForm = this.MainForm as MgrForm;

					if(tempForm != null)
					{
						this.MainForm.WindowState = tempForm.PreviousWindowState;
					}
					else
					{
						this.MainForm.WindowState = FormWindowState.Normal;
					}
				}

				if(!this.MainForm.Visible)
				{
					this.MainForm.Show();
				}
				else
				{
					this.MainForm.Activate();
				}
			}
		}
	}
}
