using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using Pipes;
using System.Threading;
using System.IO;

namespace UnrealFrontend
{
	public enum CommandletCategory
	{
		Unknown = 0,
		Cooking,
		Compiling,
		Syncing,
	}

	public enum CommandletAction
	{
		Unknown = 0,
		Cooking,
		Cooking_GlobalShadersOnly,
		Cooking_CompileScript,
		Cooking_IniIntsOnly,
		Cooking_FullRecook,
		Cooking_AllMaps,
		CompilingScript,
		Syncing,
	}

	public class CommandletOutputEventArgs : EventArgs
	{
		string mMsg = string.Empty;

		public string Message
		{
			get { return mMsg; }
		}

		public CommandletOutputEventArgs(string Msg)
		{
			if(Msg == null)
			{
				throw new ArgumentNullException("Msg");
			}

			mMsg = Msg;
		}
	}

	public class Commandlet : IDisposable
	{
		Process mCmdletProc;
		string mExecutablePath;
		string mCmdLine;
		ConsoleInterface.Platform mPlatform;
		NamedPipe mLogPipe;
		Thread mOutputThread;
		CommandletCategory mCategory;
		CommandletAction mAction;
		object mUserData;

		EventHandler<EventArgs> mOnExited;
		public event EventHandler<EventArgs> Exited
		{
			add { mOnExited += value; }
			remove { mOnExited -= value; }
		}

		EventHandler<CommandletOutputEventArgs> mOnOutput;
		public event EventHandler<CommandletOutputEventArgs> Output
		{
			add { mOnOutput += value; }
			remove { mOnOutput -= value; }
		}

		public int ExitCode
		{
			get
			{
				if(mCmdletProc != null)
				{
					return mCmdletProc.ExitCode;
				}

				return 0;
			}
		}

		public bool HasExited
		{
			get
			{
				if(mCmdletProc != null)
				{
					return mCmdletProc.HasExited;
				}

				return true;
			}
		}

		public string ExecutablePath
		{
			get { return mExecutablePath; }
		}

		public string CommandLine
		{
			get { return mCmdLine; }
		}

		public ConsoleInterface.Platform Platform
		{
			get { return mPlatform; }
		}

		public CommandletCategory Category
		{
			get { return mCategory; }
		}

		public CommandletAction Action
		{
			get { return mAction; }
		}

		public object UserData
		{
			get { return mUserData; }
			set { mUserData = value; }
		}

		public Commandlet(string ExecutablePath, ConsoleInterface.Platform Platform)
		{
			if(ExecutablePath == null)
			{
				throw new ArgumentNullException("ExecutablePath");
			}

			if(Platform == null)
			{
				throw new ArgumentNullException("Platform");
			}

			this.mExecutablePath = ExecutablePath;
			this.mPlatform = Platform;
		}

		public void Start(CommandletCategory Category, CommandletAction Action, string CmdLine)
		{
			Start(Category, Action, CmdLine, true);
		}

		public bool Start(CommandletCategory Category, CommandletAction Action, string CmdLine, bool bCreateNoWindow)
		{
			if(mCmdletProc != null && !mCmdletProc.HasExited)
			{
				return false;
			}

			this.mCmdLine = CmdLine;
			this.mCategory = Category;
			this.mAction = Action;

			bool bIsGameExe = Path.GetFileName(mExecutablePath).EndsWith("Game.exe", StringComparison.OrdinalIgnoreCase);

			ProcessStartInfo Info = new ProcessStartInfo(mExecutablePath, mCmdLine);
			Info.CreateNoWindow = bCreateNoWindow;
			Info.UseShellExecute = false;
			Info.RedirectStandardError = !bIsGameExe;
			Info.RedirectStandardOutput = !bIsGameExe;

			mCmdletProc = new Process();
			mCmdletProc.EnableRaisingEvents = true;
			mCmdletProc.StartInfo = Info;
			mCmdletProc.Exited += new EventHandler(mCmdletProc_Exited);
			mCmdletProc.OutputDataReceived += new DataReceivedEventHandler(mCmdletProc_OutputDataReceived);

			bool bProcessStarted = mCmdletProc.Start();

			if(!bIsGameExe)
			{
				mCmdletProc.BeginOutputReadLine();
			}
			else
			{
				mLogPipe = new NamedPipe();

				if(mLogPipe.Connect(mCmdletProc))
				{
					if(mOutputThread != null && mOutputThread.IsAlive)
					{
						mOutputThread.Abort();
					}

					mOutputThread = new Thread(new ParameterizedThreadStart(PollOutput));
					mOutputThread.Start(this);
				}
			}

			return bProcessStarted;
		}

		void mCmdletProc_OutputDataReceived(object sender, DataReceivedEventArgs e)
		{
			try
			{
				//if(e.Data != null)
				//{
				//    System.Diagnostics.Debug.WriteLine(e.Data);
				//}

				if(mOnOutput != null && e.Data != null)
				{
					mOnOutput(this, new CommandletOutputEventArgs(e.Data));
				}
			}
			catch(Exception ex)
			{
				System.Diagnostics.Debug.WriteLine(ex.ToString());
			}
		}

		void PollOutput(object State)
		{
			try
			{
				while(mCmdletProc != null && !mCmdletProc.HasExited)
				{
					string Msg = mLogPipe.Read();

					if(mOnOutput != null && Msg.Length > 0 && !Msg.StartsWith(UnrealFrontendWindow.UNI_COLOR_MAGIC))
					{
						mOnOutput(this, new CommandletOutputEventArgs(Msg.Replace("\r\n", "")));
					}
				}
			}
			catch(ThreadAbortException)
			{
			}
			catch(Exception ex)
			{
				System.Diagnostics.Debug.WriteLine(ex.ToString());
			}
		}

		void mCmdletProc_Exited(object sender, EventArgs e)
		{
			if(mOnExited != null)
			{
				mOnExited(this, e);
			}

			if(mOutputThread != null && mOutputThread.IsAlive)
			{
				mOutputThread.Abort();
			}
		}

		public void Kill()
		{
			if(mCmdletProc != null)
			{
				mCmdletProc.Kill();
			}

			if(mOutputThread != null && mOutputThread.IsAlive)
			{
				mOutputThread.Abort();
			}
		}

		#region IDisposable Members

		public void Dispose()
		{
			if(mCmdletProc != null)
			{
				mCmdletProc.Exited -= new EventHandler(mCmdletProc_Exited);
				mCmdletProc.OutputDataReceived -= new DataReceivedEventHandler(mCmdletProc_OutputDataReceived);
				mCmdletProc.Dispose();
				mCmdletProc = null;
			}

			if(mOutputThread != null && mOutputThread.IsAlive)
			{
				mOutputThread.Abort();
			}
		}

		#endregion
	}
}
