using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using UnrealSync;
using System.IO;
using System.Diagnostics;
using System.Threading;
using System.Runtime.InteropServices;
using System.Net.Sockets;

namespace UnrealSync.Manager
{
    public partial class MgrForm : Form
    {
		[DllImport("User32.dll", CharSet = CharSet.Auto, EntryPoint = "SendMessage")]
		static extern IntPtr SendMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);

		const int WM_VSCROLL = 277;
		const int SB_BOTTOM = 7;





		const int MAX_JOBOUTPUT_LINES = 5000;

		delegate void UpdateJobOutputDelegate(string str);
		delegate void LoadJobOutputDelegate(List<string> buffer);

        private ServiceHelper helper;
		private Thread jobOutputThread;
		private long jobOutputLock;
		private long windowHasBeenShown;
		private FormWindowState previousWindowState;
		private FormWindowState currentWindowState;

        public MgrForm()
        {
            InitializeComponent();
            LoadJobs();
            helper = new ServiceHelper();

			jobOutputThread = new Thread(JobOutputThreadHandler);
			jobOutputThread.Name = "Job Output Thread";
			jobOutputThread.IsBackground = true;
			jobOutputThread.Start(this);

			previousWindowState = currentWindowState = base.WindowState;
        }

		public FormWindowState PreviousWindowState
		{
			get { return previousWindowState; }
		}

		public new FormWindowState WindowState
		{
			get { return base.WindowState; }
			set
			{
				previousWindowState = currentWindowState;
				currentWindowState = base.WindowState = value;
			}
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			base.OnFormClosing(e);

			if(e.CloseReason == CloseReason.UserClosing)
			{
				e.Cancel = true;
				this.Visible = false;
			}
		}

		void JobOutputThreadHandler(object state)
		{
			List<string> lineBuffer = new List<string>();
			byte[] readBuf = new byte[2048]; //1024 unicode characters
			int bytesRead;
			Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

			try
			{
				while(Interlocked.Read(ref jobOutputLock) == 0)
				{
					try
					{
						if(Interlocked.Read(ref windowHasBeenShown) > 0)
						{
							if(lineBuffer.Count > 0)
							{
								// Note: we have to copy lineBuffer so that we can call Clear() in a threadsafe manner
								BeginInvoke(new LoadJobOutputDelegate(LoadJobOutputBuffer), new List<string>(lineBuffer));
								lineBuffer.Clear();
							}
						}

						if(!sock.Connected)
						{
							sock.Connect("127.0.0.1", ServiceHelper.PORT);
						}
						else
						{
							if(sock.Poll(100, SelectMode.SelectRead))
							{
								bytesRead = sock.Receive(readBuf, SocketFlags.None);

								if(bytesRead == 0)
								{
									sock.Close();
									sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
								}
								else
								{
									string str = Encoding.Unicode.GetString(readBuf, 0, bytesRead);

									if(Interlocked.Read(ref windowHasBeenShown) > 0)
									{
										if(lineBuffer.Count > 0)
										{
											lineBuffer.Add(str);

											// Note: we have to copy lineBuffer so that we can call Clear() in a threadsafe manner
											BeginInvoke(new LoadJobOutputDelegate(LoadJobOutputBuffer), new List<string>(lineBuffer));
											lineBuffer.Clear();
										}
										else
										{
											BeginInvoke(new UpdateJobOutputDelegate(UpdateJobOutput), str);
										}
									}
									else
									{
										lineBuffer.Add(str);

										if(lineBuffer.Count > MAX_JOBOUTPUT_LINES)
										{
											lineBuffer.RemoveAt(0);
										}
									}
									
									// if we got data start over so we don't Sleep() since there is probably more pending data to read
									continue;
								}
							}
						}
					}
					catch(Exception ex)
					{
						System.Diagnostics.Debug.WriteLine(ex.ToString());
					}

					Thread.Sleep(1000);
				}	
			}
			catch(ThreadAbortException abortEx)
			{
				System.Diagnostics.Debug.WriteLine(abortEx.ToString());
			}
			finally
			{
				try
				{
					System.Diagnostics.Debug.WriteLine("Closing service socket");
					sock.Close();
				}
				catch(Exception)
				{
				}
			}
		}

		void LoadJobOutputBuffer(List<string> buffer)
		{
			foreach(string str in buffer)
			{
				txtJobOutput.AppendText(str);
			}

			IntPtr ptrWparam = new IntPtr(SB_BOTTOM);
			IntPtr ptrLparam = new IntPtr(0);
			SendMessage(txtJobOutput.Handle, WM_VSCROLL, ptrWparam, ptrLparam); 

		}

		void UpdateJobOutput(string str)
		{
			txtJobOutput.AppendText(str);

			IntPtr ptrWparam = new IntPtr(SB_BOTTOM);
			IntPtr ptrLparam = new IntPtr(0);
			SendMessage(txtJobOutput.Handle, WM_VSCROLL, ptrWparam, ptrLparam); 

		}

		void OnPropertyValueChanged(object s, PropertyValueChangedEventArgs e)
		{
			SyncJob job = syncJobsGrid.SelectedObject as SyncJob;

			if(job != null)
			{
				if(e.ChangedItem.PropertyDescriptor.Name == "Name")
				{
					AppSettings.RemoveSyncJobByName(e.OldValue.ToString());
					AppSettings.AddSyncJob(job);

					// on vista the only way to force a repaint is
					// to remove and re-add the item. Invalidate() doesn't work
					// for some unknown reason
					lstbxSyncJobs.Items.Remove(job);
					lstbxSyncJobs.Items.Add(job);
					lstbxSyncJobs.SelectedItem = job;
				}
				else
				{
					AppSettings.UpdateSyncJob(job);
				}
			}
		}

        private void LoadJobs() 
		{
            lstbxSyncJobs.Items.Clear();
            
			SyncJob[] jobs = AppSettings.GetSyncJobs();

			if(jobs.Length > 0)
            {
				lstbxSyncJobs.Items.AddRange(jobs);
                lstbxSyncJobs.SelectedIndex = 0;
            }
            else
            {
                btnDelete.Enabled = false;
                btnRun.Enabled = false;
            }
        }

        private void btnDelete_Click(object sender, EventArgs e)
        {
			if(lstbxSyncJobs.SelectedItem != null)
			{
				AppSettings.RemoveSyncJobByName(lstbxSyncJobs.SelectedItem.ToString());
				lstbxSyncJobs.Items.Remove(lstbxSyncJobs.SelectedItem);
			}

			if(lstbxSyncJobs.Items.Count == 0)
			{
				btnDelete.Enabled = false;
				btnRun.Enabled = false;
			}
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
			using(NewJobForm frm = new NewJobForm())
			{
				frm.ShowDialog(this);

				if(frm.DialogResult != DialogResult.OK)
				{
					return;
				}

				for(int i = 0; i < lstbxSyncJobs.Items.Count; ++i)
				{
					if(lstbxSyncJobs.Items[i].ToString() == frm.SyncJobName)
					{
						lstbxSyncJobs.SelectedIndex = i;
						return;
					}
				}

				SyncJob newJob = new SyncJob(frm.SyncJobName);

				ProcessStartInfo cmdStartInfo = new ProcessStartInfo();
				cmdStartInfo.CreateNoWindow = true;
				cmdStartInfo.FileName = "p4";
				cmdStartInfo.Arguments = "info";
				cmdStartInfo.RedirectStandardOutput = true;
				cmdStartInfo.Verb = "open";
				cmdStartInfo.UseShellExecute = false;

				try
				{
					using(Process cmdProc = Process.Start(cmdStartInfo))
					{
						while(!cmdProc.StandardOutput.EndOfStream)
						{
							string result = cmdProc.StandardOutput.ReadLine();

							if(result.StartsWith("Client name:"))
							{
								newJob.PerforceClientSpec = result.Substring("Client name:".Length).Trim();
							}
						}

						cmdProc.Close();
					}
				}
				catch(Exception)
				{
				}

				AppSettings.AddSyncJob(newJob);

				lstbxSyncJobs.Items.Add(newJob);
				lstbxSyncJobs.SelectedItem = newJob;
			}

			if(lstbxSyncJobs.Items.Count > 0)
			{
				btnDelete.Enabled = true;
				btnRun.Enabled = true;
			}
        }

        private void btnRun_Click(object sender, EventArgs e)
        {
            SyncJob job = lstbxSyncJobs.SelectedItem as SyncJob;

			if(job != null)
			{
				ExecuteStatus jobStatus = helper.ExecuteJob(job, new DataReceivedEventHandler(OnRunJobOutput));
				if(jobStatus == ExecuteStatus.STATUS_EXECUTION_PROBLEM)
				{
					MessageBox.Show("There was a problem executing your sync job.  Check the log file for details.");
				}
				else if(jobStatus == ExecuteStatus.STATUS_GAME_RUNNING)
				{
					MessageBox.Show("The game associated with this job is currently running.  Please close it and try again.");
				}
			}
        }

		private void OnRunJobOutput(object sender, DataReceivedEventArgs e)
		{
			if(e.Data != null)
			{
				string str = string.Format(ServiceHelper.OUTPUTFORMAT, DateTime.Now.ToString("m/d/y h:mm:ss tt"), "(running test)", e.Data, Environment.NewLine);

				UnrealSyncLogger logger;

				if(helper.TryGetLogger(sender as Process, out logger))
				{
					logger.WriteLine(e.Data);
				}

				BeginInvoke(new UpdateJobOutputDelegate(UpdateJobOutput), str);
			}
		}

		private void lstbxSyncJobs_SelectedIndexChanged(object sender, EventArgs e)
		{
			syncJobsGrid.SelectedObject = lstbxSyncJobs.SelectedItem;
		}

		void OnCleanUpJobOutput(object sender, EventArgs e)
		{
			List<string> lines = new List<string>(txtJobOutput.Lines);

			if(lines.Count > MAX_JOBOUTPUT_LINES)
			{
				lines.RemoveRange(0, lines.Count - MAX_JOBOUTPUT_LINES);

				txtJobOutput.Lines = lines.ToArray();
			}
		}

		protected override void OnResize(EventArgs e)
		{
			base.OnResize(e);

			if(currentWindowState != base.WindowState)
			{
				previousWindowState = currentWindowState;
				currentWindowState = base.WindowState;
			}
		}

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);

			Interlocked.Increment(ref windowHasBeenShown);
		}

		private void menuMain_Exit_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		private void menuMain_Output_Clear_Click(object sender, EventArgs e)
		{
			txtJobOutput.Clear();
		}
    }
}