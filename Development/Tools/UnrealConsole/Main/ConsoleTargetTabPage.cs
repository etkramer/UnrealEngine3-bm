using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.IO;
using UnrealControls;

namespace UnrealConsole.Main
{
	/// <summary>
	/// A tab page that represents a console target.
	/// </summary>
	public class ConsoleTargetTabPage : UnrealControls.DynamicTabPage
	{
		/// <summary>
		/// Delegate for marshaling append text calls into the UI thread.
		/// </summary>
		/// <param name="Txt">The text to be appended.</param>
		delegate void AppendTextDelegate(string Txt);

		/// <summary>
		/// Delegate for marshaling crashes to the UI thread.
		/// </summary>
		/// <param name="CallStack">The call stack.</param>
		/// <param name="MiniDumpLocation">The path to the mini-dump file.</param>
		delegate void CrashHandlerDelegate(string CallStack, string MiniDumpLocation);

		private static readonly string LOG_DIR = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), "Unreal Console Logs\\");
		
		private Button mBtnSend;
		private ConsoleInterface.PlatformTarget mTarget;
		private System.Threading.Timer mOutputTimer;
		private StringBuilder mOutputBuffer = new StringBuilder();
		private int mCmdIndex = -1;
		private int mLastCrashReport;
		private ContextMenuStrip mTTYCtxMenu;
		private System.ComponentModel.IContainer components;
		private ToolStripMenuItem mSelectAllToolStripMenuItem;
		private ToolStripMenuItem mCopyToolStripMenuItem;
		private TextBox mCommand;
		private bool mSendCommandToAll;
		private UnrealControls.OutputWindowView mOutputWindow;
		private volatile bool mLogOutput;
		private UnrealControls.OutputWindowDocument mDocument = new UnrealControls.OutputWindowDocument();

		/// <summary>
		/// Gets the target that belongs to the tab page.
		/// </summary>
		public ConsoleInterface.PlatformTarget Target
		{
			get { return mTarget; }
		}

		/// <summary>
		/// Gets the TTY output text box associated with the target.
		/// </summary>
		public UnrealControls.OutputWindowView TTYText
		{
			get { return mOutputWindow; }
		}

		/// <summary>
		/// Gets/Sets whether or not TTY output should be logged to disk.
		/// </summary>
		public bool LogOutput
		{
			get { return mLogOutput; }
			set
			{
				mLogOutput = value;

				if(value)
				{
					LogText(string.Format("\r\n*********************************** Starting new log session at {0} ***********************************\r\n\r\n", DateTime.Now.ToString("H:m:s")));
				}
			}
		}

		/// <summary>
		/// Gets the log file's name on disk.
		/// </summary>
		public string LogFilename
		{
			get { return string.Format("{0}_{1}_{2}.txt", mTarget.Name, mTarget.ParentPlatform.Name, DateTime.Today.ToString("M-dd-yyyy")); }
		}

		/// <summary>
		/// Gets/Sets whether commands are sent to all targets.
		/// </summary>
		public bool SendCommandsToAll
		{
			get { return mSendCommandToAll; }
			set
			{
				mSendCommandToAll = value;

				if(value)
				{
					mBtnSend.Text = "Send All";
				}
				else
				{
					mBtnSend.Text = "Send";
				}
			}
		}

		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="Target">The target to be associated with the tab page.</param>
		/// <param name="bLogOutput">True if the tab page is to log all TTY output to disk.</param>
		/// <param name="SendCommandsToAll">True if commands are to be sent to all targets.</param>
		public ConsoleTargetTabPage(ConsoleInterface.PlatformTarget Target, bool bLogOutput, bool SendCommandsToAll)
			: base(string.Format("{0} <{1}>", Target.Name, Target.ParentPlatform.Name))
		{
			InitializeComponent();

			mTarget = Target;
			mSendCommandToAll = SendCommandsToAll;

			// use the property so the time stamp is logged
			this.LogOutput = bLogOutput;

			mTarget.SetTTYCallback(new ConsoleInterface.TTYOutputDelegate(this.OnTTYEventCallback));
			mTarget.SetCrashCallback(new ConsoleInterface.CrashCallbackDelegate(this.OnCrash));
			mTarget.CrashFilter = Properties.Settings.Default.CrashFilter;

			if(!mTarget.Connect())
			{
				mDocument.AppendLine(Color.Red, "Could not connect to target!");
			}

			if(Properties.Settings.Default.CommandHistory == null)
			{
				Properties.Settings.Default.CommandHistory = new AutoCompleteStringCollection();
			}

			if(Properties.Settings.Default.TargetDumpTypes == null)
			{
				Properties.Settings.Default.TargetDumpTypes = new TargetDumpTypeCollection();
			}

			ConsoleInterface.DumpType DumpType;
			if(Properties.Settings.Default.TargetDumpTypes.TryGetValue(mTarget.TargetManagerName, out DumpType))
			{
				mTarget.CrashDumpType = DumpType;
			}

			mCommand.AutoCompleteCustomSource = Properties.Settings.Default.CommandHistory;
			mCommand.Focus();

			mOutputTimer = new System.Threading.Timer(new System.Threading.TimerCallback(OnOutputTimer), null, 250, 250);

			mOutputWindow.AutoScroll = true;
			mOutputWindow.Document = mDocument;
			mOutputWindow.KeyPress += new KeyPressEventHandler(mOutputWindow_KeyPress);
		}

		/// <summary>
		/// Event handler for when the user starts typing in the output window.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		void mOutputWindow_KeyPress(object sender, KeyPressEventArgs e)
		{
			if(char.IsLetterOrDigit(e.KeyChar) || char.IsPunctuation(e.KeyChar) || char.IsSymbol(e.KeyChar) || char.IsSeparator(e.KeyChar))
			{
				mCommand.AppendText("" + e.KeyChar);
				mCommand.Focus();
			}
		}

		/// <summary>
		/// Logs a message to disk.
		/// </summary>
		/// <param name="Message">The message to be appended to the log file.</param>
		void LogText(string Message)
		{
			if(Message.Length > 0)
			{
				try
				{
					if(!Directory.Exists(LOG_DIR))
					{
						Directory.CreateDirectory(LOG_DIR);
					}

					File.AppendAllText(Path.Combine(LOG_DIR, LogFilename), Message);
				}
				catch(Exception ex)
				{
					System.Diagnostics.Debug.WriteLine(ex.ToString());
				}
			}
		}

		/// <summary>
		/// Timer callback for batch appending TTY output.
		/// </summary>
		/// <param name="State">State passed to the callback.</param>
		void OnOutputTimer(object State)
		{
			string Buf = string.Empty;

			lock(mOutputBuffer)
			{
				if(mOutputBuffer.Length > 0)
				{
					Buf = mOutputBuffer.ToString();
					mOutputBuffer.Length = 0;
				}
			}

			if(Buf.Length > 0)
			{
				Print(Buf);

				if(mLogOutput)
				{
					LogText(Buf);
				}
			}
		}

		/// <summary>
		/// This function handles a TTY console ouput event.
		/// </summary>
		/// <param name="Txt">A pointer to a wide character string</param>
		unsafe void OnTTYEventCallback(IntPtr Txt)
		{
			char* TxtPtr = (char*)Txt.ToPointer();

			if(*TxtPtr != '\0')
			{
				lock(mOutputBuffer)
				{
					mOutputBuffer.Append(new string(TxtPtr));
				}
			}
		}

		/// <summary>
		/// Appends TTY output text.
		/// </summary>
		/// <param name="Str">The text to be appended.</param>
		void AppendText(string Str)
		{
			if(Str == null || Str.Length == 0)
			{
				return;
			}

			System.Diagnostics.Debug.WriteLine(Str);

			DynamicTabControl Owner = (DynamicTabControl)this.Parent;

			if(Owner != null && Owner.SelectedTab != this)
			{
				this.TabForegroundColor = Brushes.Red;
			}

			mDocument.AppendText(null, Str);
		}

		/// <summary>
		/// Marshals a print command into the UI thread.
		/// </summary>
		/// <param name="Str">The message to print.</param>
		void Print(string Str)
		{
			if(Str.Length > 0)
			{
				BeginInvoke(new AppendTextDelegate(this.AppendText), Str);
			}
		}

		/// <summary>
		/// Handles crashes.
		/// </summary>
		/// <param name="Data">A pointer to the callstack.</param>
		unsafe void OnCrash(IntPtr CallStackPtr, IntPtr MiniDumpLocationPtr)
		{
			string CallStack = new string((char*)CallStackPtr.ToPointer());
			string MiniDumpLoc = new string((char*)MiniDumpLocationPtr.ToPointer());

			if(CallStack.Length > 0 || MiniDumpLoc.Length > 0)
			{
				BeginInvoke(new CrashHandlerDelegate(UIThreadOnCrash), CallStack, MiniDumpLoc);
			}
		}

		/// <summary>
		/// The crash handler that runs on the UI thread.
		/// </summary>
		/// <param name="CallStack">The crash callstack.</param>
		void UIThreadOnCrash(string CallStack, string MiniDumpLocation)
		{
			if(File.Exists("EpicInternal.txt") && Environment.TickCount - mLastCrashReport >= 10000)
			{
				CrashReporter Reporter = new CrashReporter();
				//fire up the autoreporter
				string CrashResult = Reporter.SendCrashReport(CallStack, TTYText.Text, mTarget.ParentPlatform.Name, MiniDumpLocation);

				AppendText(Environment.NewLine + CallStack + Environment.NewLine);
				AppendText(Environment.NewLine + CrashResult + Environment.NewLine);

				mLastCrashReport = Environment.TickCount;
			}
			else
			{
				AppendText(Environment.NewLine + CallStack);
			}
		}

		#region Windows Forms generated code

		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			this.mBtnSend = new System.Windows.Forms.Button();
			this.mTTYCtxMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mSelectAllToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mCopyToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.mOutputWindow = new UnrealControls.OutputWindowView();
			mCommand = new System.Windows.Forms.TextBox();
			this.mTTYCtxMenu.SuspendLayout();
			this.SuspendLayout();
			// 
			// mBtnSend
			// 
			this.mBtnSend.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.mBtnSend.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.mBtnSend.Location = new System.Drawing.Point(362, 306);
			this.mBtnSend.Name = "mBtnSend";
			this.mBtnSend.Size = new System.Drawing.Size(75, 23);
			this.mBtnSend.TabIndex = 1;
			this.mBtnSend.Text = "&Send";
			this.mBtnSend.UseVisualStyleBackColor = true;
			this.mBtnSend.Click += new System.EventHandler(this.mBtnSend_Click);
			// 
			// mTTYCtxMenu
			// 
			this.mTTYCtxMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mSelectAllToolStripMenuItem,
            this.mCopyToolStripMenuItem});
			this.mTTYCtxMenu.Name = "mTTYCtxMenu";
			this.mTTYCtxMenu.Size = new System.Drawing.Size(123, 48);
			// 
			// mSelectAllToolStripMenuItem
			// 
			this.mSelectAllToolStripMenuItem.Name = "mSelectAllToolStripMenuItem";
			this.mSelectAllToolStripMenuItem.Size = new System.Drawing.Size(122, 22);
			this.mSelectAllToolStripMenuItem.Text = "Select All";
			this.mSelectAllToolStripMenuItem.Click += new System.EventHandler(this.mSelectAllToolStripMenuItem_Click);
			// 
			// mCopyToolStripMenuItem
			// 
			this.mCopyToolStripMenuItem.Name = "mCopyToolStripMenuItem";
			this.mCopyToolStripMenuItem.Size = new System.Drawing.Size(122, 22);
			this.mCopyToolStripMenuItem.Text = "Copy";
			this.mCopyToolStripMenuItem.Click += new System.EventHandler(this.mCopyToolStripMenuItem_Click);
			// 
			// mOutputWindow
			// 
			this.mOutputWindow.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.mOutputWindow.BackColor = System.Drawing.SystemColors.Window;
			this.mOutputWindow.Document = null;
			this.mOutputWindow.Location = new System.Drawing.Point(0, 0);
			this.mOutputWindow.Name = "mOutputWindow";
			this.mOutputWindow.Size = new System.Drawing.Size(440, 300);
			this.mOutputWindow.TabIndex = 2;
			// 
			// mCommand
			// 
			mCommand.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			mCommand.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
			mCommand.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.CustomSource;
			mCommand.Location = new System.Drawing.Point(3, 308);
			mCommand.Name = "mCommand";
			mCommand.Size = new System.Drawing.Size(353, 20);
			mCommand.TabIndex = 0;
			mCommand.WordWrap = false;
			mCommand.KeyDown += new System.Windows.Forms.KeyEventHandler(this.mCommand_KeyDown);
			// 
			// ConsoleTargetTabPage
			// 
			this.Controls.Add(mCommand);
			this.Controls.Add(this.mOutputWindow);
			this.Controls.Add(this.mBtnSend);
			this.Name = "ConsoleTargetTabPage";
			this.Size = new System.Drawing.Size(440, 332);
			this.mTTYCtxMenu.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		/// <summary>
		/// Event handler.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void mTTYText_KeyPress(object sender, KeyPressEventArgs e)
		{
			e.Handled = true;
			mCommand.Text += e.KeyChar;
			mCommand.Focus();
			mCommand.SelectionStart = mCommand.TextLength;
		}

		/// <summary>
		/// Event handler.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void mCommand_KeyDown(object sender, KeyEventArgs e)
		{
			AutoCompleteStringCollection CmdHistory = Properties.Settings.Default.CommandHistory;

			switch(e.KeyCode)
			{
				case Keys.Up:
					{
						e.Handled = true;

						if(CmdHistory != null && CmdHistory.Count > 0)
						{
							++mCmdIndex;

							if(mCmdIndex >= CmdHistory.Count)
							{
								mCmdIndex = 0;
							}

							LoadCurrentCommand();
						}

						break;
					}
				case Keys.Down:
					{
						e.Handled = true;

						if(CmdHistory != null)
						{
							--mCmdIndex;

							if(mCmdIndex < 0)
							{
								mCmdIndex = CmdHistory.Count - 1;
							}

							LoadCurrentCommand();

							
						}

						break;
					}
				case Keys.Enter:
					{
						e.Handled = true;
						ExecuteCommand();

						break;
					}
			}
		}

		/// <summary>
		/// Executes a command on the tab page target.
		/// </summary>
		private void ExecuteCommand()
		{
			AutoCompleteStringCollection CmdHistory = Properties.Settings.Default.CommandHistory;

			string Cmd = mCommand.Text;
			mCommand.Clear();

			if(Cmd.Length > 0)
			{
				if(CmdHistory == null)
				{
					CmdHistory = new AutoCompleteStringCollection();
					mCommand.AutoCompleteCustomSource = CmdHistory;
					Properties.Settings.Default.CommandHistory = CmdHistory;
				}

				if(!CmdHistory.Contains(Cmd))
				{
					CmdHistory.Insert(0, Cmd);
				}
				else
				{
					CmdHistory.Remove(Cmd);
					CmdHistory.Insert(0, Cmd);
				}

				DynamicTabControl Owner = this.Parent as DynamicTabControl;

				if(Owner != null)
				{
					foreach(ConsoleTargetTabPage CurPage in Owner.TabPages)
					{
						CurPage.ClearCommandHistoryState();
					}
				}

				if(mSendCommandToAll && Owner != null)
				{
					foreach(ConsoleTargetTabPage CurPage in Owner.TabPages)
					{
						CurPage.Target.SendCommand(Cmd);
					}
				}
				else
				{
					mTarget.SendCommand(Cmd);
				}
			}
		}

		/// <summary>
		/// Clears the command history state for the tab page.
		/// </summary>
		public void ClearCommandHistoryState()
		{
			mCmdIndex = -1;
		}

		/// <summary>
		/// Loads the currently selected command in the command history.
		/// </summary>
		private void LoadCurrentCommand()
		{
			AutoCompleteStringCollection CmdHistory = Properties.Settings.Default.CommandHistory;
			if(CmdHistory != null)
			{
				if(mCmdIndex >= 0 && mCmdIndex < CmdHistory.Count)
				{
					mCommand.Text = CmdHistory[mCmdIndex];
					mCommand.SelectAll();
				}
			}
		}

		/// <summary>
		/// Event handler.
		/// </summary>
		/// <param name="e">Information about the event.</param>
		protected override void OnTabSelected(EventArgs e)
		{
			base.OnTabSelected(e);
			mCommand.Focus();

			this.TabForegroundColor = SystemBrushes.WindowText;
		}

		/// <summary>
		/// Event handler.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void mBtnSend_Click(object sender, EventArgs e)
		{
			ExecuteCommand();
		}

		/// <summary>
		/// Cleans up resources when the control is being destroyed.
		/// </summary>
		/// <param name="disposing">True if disposing managed resources.</param>
		protected override void Dispose(bool disposing)
		{
			if(disposing)
			{
				mOutputTimer.Dispose();
			}

			base.Dispose(disposing);
		}

		/// <summary>
		/// Event handler for selecting all TTY output text.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void mSelectAllToolStripMenuItem_Click(object sender, EventArgs e)
		{
			mOutputWindow.SelectAll();
		}

		/// <summary>
		/// Event handler for copying selected TTY output text to the clipboard.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void mCopyToolStripMenuItem_Click(object sender, EventArgs e)
		{
			mOutputWindow.CopySelectedText();
		}

		/// <summary>
		/// Saves the dump type to the user settings.
		/// </summary>
		public void SaveDumpType()
		{
			if(Properties.Settings.Default.TargetDumpTypes == null)
			{
				Properties.Settings.Default.TargetDumpTypes = new TargetDumpTypeCollection();
			}

			Properties.Settings.Default.TargetDumpTypes.SetValue(mTarget.TargetManagerName, mTarget.CrashDumpType);
		}
	}
}
