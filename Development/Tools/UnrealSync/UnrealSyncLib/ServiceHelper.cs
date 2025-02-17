using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Net.Mail;
using System.Reflection;
using System.Threading;
using System.Runtime.InteropServices;
using System.Net.Sockets;
using System.Net;
using System.Reflection.Emit;

namespace UnrealSync
{
	/// <summary>
	/// The execution status of a job.
	/// </summary>
	public enum ExecuteStatus
	{
		STATUS_OK,
		STATUS_GAME_RUNNING,
		STATUS_EXECUTION_PROBLEM
	}

	/// <summary>
	/// Helper class for the unreal sync service. Handles running jobs and distributing job output.
	/// </summary>
    public class ServiceHelper
    {
		class JobState
		{
			public Process Process;
			public SyncJob Job;
			public UnrealSyncLogger Logger;
			public DataReceivedEventHandler OutputHandler;
		}

		delegate DataReceivedEventArgs GetDataReceivedEventArgsDelegate(string message);

		/// <summary>
		/// The port the unreal sync service listens for incoming connections on.
		/// </summary>
		public const int PORT = 10300;
		public const string OUTPUTFORMAT = "[{0}] {1}: {2}{3}";

        const string LOG_SEPARATOR = "============================================================================================";

		private static GetDataReceivedEventArgsDelegate getDataReceivedEventArgs;
		private static ReaderWriterLock socketLock = new ReaderWriterLock();
		private static List<Socket> connectedSockets = new List<Socket>();
		private static Socket listenSocket;
		private static long jobCount;

		private Dictionary<Process, JobState> jobHash = new Dictionary<Process, JobState>();

		/// <summary>
		/// See <see href="http://msdn2.microsoft.com/en-us/library/ms724211.aspx"/> for the MSDN docs.
		/// </summary>
		/// <param name="hObject">The handle to close.</param>
		/// <returns>True if the function succeeded.</returns>
		[DllImport("kernel32.dll")]
		[return: MarshalAs(UnmanagedType.Bool)]
		public static extern bool CloseHandle(IntPtr hObject);

		static ServiceHelper()
		{
			CreateGetDataReceivedEventArgs();
		}

		/// <summary>
		/// Constructor.
		/// </summary>
        public ServiceHelper()
        {
        }

		/// <summary>
		/// <see cref="DataReceivedEventArgs"/> has an internal constructor so this function generates a dynamic method for creating <see cref="DataReceivedEventArgs"/> instances.
		/// </summary>
		static void CreateGetDataReceivedEventArgs()
		{
			ConstructorInfo ctorInfo = typeof(DataReceivedEventArgs).GetConstructor(BindingFlags.NonPublic | BindingFlags.Instance, Type.DefaultBinder, new Type[] { typeof(string) }, null);

			// DataReceivedEventArgs GetDataReceivedEventArgs(string message)
			// {
			//		return new DataReceivedEventArgs(message);
			// }
			DynamicMethod method = new DynamicMethod("GetDataReceivedEventArgs", typeof(DataReceivedEventArgs), new Type[] { typeof(string) }, typeof(DataReceivedEventArgs));
			method.DefineParameter(0, ParameterAttributes.In, "message");

			ILGenerator ilgen = method.GetILGenerator();

			ilgen.Emit(OpCodes.Ldarg_0);
			ilgen.Emit(OpCodes.Newobj, ctorInfo);
			ilgen.Emit(OpCodes.Ret);

			getDataReceivedEventArgs = method.CreateDelegate(typeof(GetDataReceivedEventArgsDelegate)) as GetDataReceivedEventArgsDelegate;
		}

		/// <summary>
		/// Retrieves the logger for a specific process.
		/// </summary>
		/// <param name="proc">The process to get the logger for.</param>
		/// <param name="logger">The resulting process logger.</param>
		/// <returns>True if a logger exists for the specified process.</returns>
		public bool TryGetLogger(Process proc, out UnrealSyncLogger logger)
		{
			JobState state;

			lock(jobHash)
			{
				jobHash.TryGetValue(proc, out state);
			}

			if(state != null)
			{
				logger = state.Logger;
			}
			else
			{
				logger = null;
			}

			return state != null;
		}

		/// <summary>
		/// Creates the socket that listens for incoming output connections.
		/// </summary>
		public static void CreateListenSocket()
		{
			if(listenSocket != null)
			{
				listenSocket.Close();
			}

			listenSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
			listenSocket.Bind(new IPEndPoint(IPAddress.Any, PORT));
			listenSocket.Listen(100);
			listenSocket.BeginAccept(OnAccept, null);
		}

		/// <summary>
		/// Called when an incoming connection is to be accepted.
		/// </summary>
		/// <param name="result">The result of the async operation.</param>
		static void OnAccept(IAsyncResult result)
		{
			try
			{
				Socket newSock = listenSocket.EndAccept(result);

				socketLock.AcquireWriterLock(5000);

				try
				{
					connectedSockets.Add(newSock);
					
					byte[] recvBuf = new byte[8];
					newSock.BeginReceive(recvBuf, 0, recvBuf.Length, SocketFlags.None, null, recvBuf);
				}
				finally
				{
					socketLock.ReleaseWriterLock();
				}

				listenSocket.BeginAccept(OnAccept, null);
			}
			catch(Exception)
			{
			}
		}

		/// <summary>
		/// Closes the listen socket and disconnects all clients.
		/// </summary>
		public static void CloseListenSocket()
		{
			listenSocket.Close();
			listenSocket = null;

			socketLock.AcquireWriterLock(5000);

			try
			{
				foreach(Socket sock in connectedSockets)
				{
					try
					{
						sock.Shutdown(SocketShutdown.Both);
						sock.Close();
					}
					catch(Exception)
					{
					}
				}

				connectedSockets.Clear();
			}
			finally
			{
				socketLock.ReleaseWriterLock();
			}
		}

		/// <summary>
		/// Writes job output to all connected clients.
		/// </summary>
		/// <param name="job">The job that generated the output.</param>
		/// <param name="message">The message to be written.</param>
		/// <param name="parms">Message parameters.</param>
		public static void WriteOutput(SyncJob job, string message, params object[] parms)
		{
			if(job != null)
			{
				string str = string.Format(OUTPUTFORMAT, DateTime.Now.ToString("m/d/y h:mm:ss tt"), job.Name, string.Format(message, parms), Environment.NewLine);
				byte[] bytesToWrite = Encoding.Unicode.GetBytes(str);
				List<Socket> socketsToRemove = null;

				socketLock.AcquireReaderLock(500);

				try
				{
					foreach(Socket sock in connectedSockets)
					{
						try
						{
							if(sock.Send(bytesToWrite) == 0)
							{
								throw new Exception();
							}
						}
						catch(Exception)
						{
							if(socketsToRemove == null)
							{
								socketsToRemove = new List<Socket>();
							}

							socketsToRemove.Add(sock);
						}
					}
				}
				finally
				{
					socketLock.ReleaseReaderLock();
				}

				if(socketsToRemove != null)
				{
					socketLock.AcquireWriterLock(500);

					try
					{
						foreach(Socket sock in socketsToRemove)
						{
							try
							{
								sock.Close();
							}
							catch(Exception)
							{
							}

							connectedSockets.Remove(sock);
						}
					}
					finally
					{
						socketLock.ReleaseWriterLock();
					}
				}
			}
		}

        /// <summary>
        /// Checks for any jobs that need to be run.
        /// </summary>
        /// <param name="debug">True if being run in debug mode which forces execution for all jobs.</param>
        public void CheckForSyncJobAndRun(bool debug)
        {
            DateTime currentTime = DateTime.Now;            // current time
            int hour = currentTime.Hour;                    // current hour
            int minute = currentTime.Minute;                // current minute
            SyncJob[] jobs = AppSettings.GetSyncJobs();     // list of sync jobs
            DateTime jobTime;                               // current job time

            // Iterate through the job list to see if one should run in this minute
            foreach(SyncJob job in jobs)
            {
				try
				{
					// Get the current time - getting this before iterating through the list in the event that this takes a while.
					jobTime = job.GetStartDateTime();

					if(job.Enabled && (debug || (jobTime.Hour == hour && jobTime.Minute == minute)))
					{
						ExecuteJob(job, null);
					}
				}
				catch(Exception ex)
				{
					EventLog.WriteEntry(GetAppName(), ex.ToString(), EventLogEntryType.Warning);
				}
            }
        }

		/// <summary>
		/// Gets the logger for a job.
		/// </summary>
		/// <param name="jobName">The name of the job requesting its log name.</param>
		/// <returns>A new instance of <see cref="UnrealSyncLogger"/>.</returns>
		private static UnrealSyncLogger GetSyncJobLog(string jobName)
		{
			return new UnrealSyncLogger(Path.GetTempPath() + "\\UnrealSync_" + jobName + "_" + DateTime.Now.ToFileTime().ToString() + ".txt");
		}

		/// <summary>
		/// Executes a sync job.
		/// </summary>
		/// <param name="job">The job to be executed.</param>
		/// <returns>The current status of the job.</returns>
		public ExecuteStatus ExecuteJob(SyncJob job, DataReceivedEventHandler outputCallback)
		{
			if(outputCallback == null)
			{
				outputCallback = new DataReceivedEventHandler(PrintLog);
			}

			Process[] gameProcesses = null;                 // holds a list of processes that match a game associated with a sync job
			ExecuteStatus returnCode = ExecuteStatus.STATUS_OK;

			// Get a list of processes that have the process name associated with this sync job.  If
			// no process is defined, skip.  (i.e. there is no process checking)
			if(job.GameProcessName != null && job.GameProcessName.Length > 0)
			{
				gameProcesses = Process.GetProcessesByName(Path.GetFileNameWithoutExtension(job.GameProcessName));
			}

			// Open the log for the sync job
			UnrealSyncLogger logWriter = GetSyncJobLog(job.Name);

			// If the game isn't running, sync.
			if(gameProcesses == null || gameProcesses.Length == 0 || job.KillGameProcess)
			{
				if(gameProcesses != null)
				{
					if(job.KillGameProcess)
					{
						foreach(Process curProcess in gameProcesses)
						{
							try
							{
								curProcess.Kill();
							}
							catch(Exception ex)
							{
								logWriter.WriteLine(ex.ToString());
							}
						}
					}

					foreach(Process curProcess in gameProcesses)
					{
						curProcess.Dispose();
					}
				}

				Process newProcess = new Process();                      // sync job;

				EventLog.WriteEntry(GetAppName(), job.Name + " sync job starting", EventLogEntryType.Information);

				// Redirect the output.
				newProcess.StartInfo.UseShellExecute = false;
				newProcess.StartInfo.RedirectStandardOutput = true;
				newProcess.StartInfo.RedirectStandardError = true;
				newProcess.StartInfo.CreateNoWindow = true;

				// If a batch file is defined, run it
				if(job.BatchFilePath != null && job.BatchFilePath.Length > 0)
				{
					newProcess.StartInfo.FileName = job.BatchFilePath;
					newProcess.StartInfo.WorkingDirectory = Path.GetDirectoryName(Path.GetFullPath(job.BatchFilePath));
					newProcess.StartInfo.Arguments = job.PerforceClientSpec;
				}
				// If a batch file is not defined, sync to head
				else
				{
					// If the client spec is defined, use it
					newProcess.StartInfo.FileName = "p4";
					newProcess.StartInfo.Arguments = "";

					if(job.PerforceClientSpec != null && job.PerforceClientSpec.Length > 0)
					{
						newProcess.StartInfo.Arguments += "-c " + job.PerforceClientSpec;
					}

					newProcess.StartInfo.Arguments += " sync";
					
					if(job.Label != null && job.Label.Length > 0)
					{
						String label = job.Label;
						label = label.Replace("%D%", DateTime.Now.ToString("yyyy-MM-dd"));
						newProcess.StartInfo.Arguments += " @" + label;
					}
				}

				JobState state = new JobState();

				state.Process = newProcess;
				state.Job = job;
				state.Logger = logWriter;
				state.OutputHandler = outputCallback;

				lock(jobHash)
				{
					jobHash.Add(newProcess, state);
				}


				// Spawn the process - try to start the process, handling thrown exceptions as a failure.
				try
				{
					newProcess.EnableRaisingEvents = true;
					newProcess.OutputDataReceived += outputCallback;
					newProcess.ErrorDataReceived += outputCallback;

					newProcess.Exited += new EventHandler(ProcessExit);

					// Write log header
					GenerateProcessOutput(state, LOG_SEPARATOR);
					GenerateProcessOutput(state, "JOB: {0}", job.Name);
					GenerateProcessOutput(state, "START DATE/TIME: " + DateTime.Now);
					GenerateProcessOutput(state, "STARTING: {0} {1}", newProcess.StartInfo.FileName, newProcess.StartInfo.Arguments);
					GenerateProcessOutput(state, LOG_SEPARATOR);

					newProcess.Start();

					newProcess.BeginOutputReadLine();

					Interlocked.Increment(ref jobCount);
				}
				catch(Exception ex0)
				{
					string errStr = string.Format("Job \'{0}\' failed: {1}",  job.Name, ex0.ToString());
					
					EventLog.WriteEntry(GetAppName(), errStr, EventLogEntryType.Warning);

					try
					{
						GenerateProcessOutput(state, errStr);
					}
					catch(Exception)
					{
					}
					
					returnCode = ExecuteStatus.STATUS_EXECUTION_PROBLEM;

					lock(jobHash)
					{
						jobHash.Remove(newProcess);
					}

					newProcess.Dispose();
					logWriter.Dispose();
				}
			}
			// If the game is running output an error in the log.
			else
			{
				logWriter.WriteLine(LOG_SEPARATOR);
				logWriter.WriteLine("CANNOT SYNC JOB \'{0}\'", job.Name);
				logWriter.WriteLine(job.GameProcessName + " is running");
				logWriter.WriteLine(LOG_SEPARATOR);
				logWriter.Dispose();

				returnCode = ExecuteStatus.STATUS_GAME_RUNNING;
			}

			return returnCode;
		}

		/// <summary>
		/// Gets the name of the executing application.
		/// </summary>
		/// <returns>The name of the executing application.</returns>
		private static string GetAppName()
		{
			return Path.GetFileName(Assembly.GetEntryAssembly().Location);
		}

		/// <summary>
		/// Logs output.
		/// </summary>
		/// <param name="Sender">The object that generated the event.</param>
		/// <param name="e">Information about the event.</param>
        private void PrintLog(object Sender, DataReceivedEventArgs e)
        {
			Process process = Sender as Process;

			if(process != null)
			{
				try
				{
					JobState state;

					lock(jobHash)
					{
						jobHash.TryGetValue(process, out state);
					}
					
					string Line = e.Data;

					if(Line != null)
					{
						WriteOutput(state.Job, Line);

						state.Logger.WriteLine(Line);
					}
				}
				catch(Exception ex)
				{
					EventLog.WriteEntry(GetAppName(), ex.ToString(), EventLogEntryType.Warning);
				}
			}
        }

		/// <summary>
		/// Simulates an output event for a process.
		/// </summary>
		/// <param name="state">The job state.</param>
		/// <param name="message">The generated message.</param>
		/// <param name="parms">Parameters for the generated message.</param>
		void GenerateProcessOutput(JobState state, string message, params object[] parms)
		{
			if(state != null && getDataReceivedEventArgs != null)
			{
				DataReceivedEventArgs args = getDataReceivedEventArgs(string.Format(message, parms));
				state.OutputHandler(state.Process, args);
			}
		}

		/// <summary>
		/// Called when a job process exits.
		/// </summary>
		/// <param name="Sender">The process that generated the event.</param>
		/// <param name="e">Information baout the event.</param>
        private void ProcessExit(object Sender, System.EventArgs e)
        {
			Process process = Sender as Process;

			if(process != null)
			{
				JobState state;

				lock(jobHash)
				{
					jobHash.TryGetValue(process, out state);
				}

				try
				{
					GenerateProcessOutput(state, LOG_SEPARATOR);
					GenerateProcessOutput(state, "JOB: {0}", state.Job.Name);
					GenerateProcessOutput(state, "END DATE/TIME: " + DateTime.Now);
					GenerateProcessOutput(state, LOG_SEPARATOR);

					if(null != state.Job.PostBatchPath && state.Job.PostBatchPath.Length > 0)
					{
						Process postProcess = new Process();
						
						JobState postState = new JobState();
						postState.Job = state.Job;
						postState.Logger = state.Logger;
						postState.OutputHandler = state.OutputHandler;
						postState.Process = postProcess;

						postProcess.StartInfo.FileName = state.Job.PostBatchPath;
						postProcess.StartInfo.UseShellExecute = false;
						postProcess.StartInfo.RedirectStandardOutput = true;
						postProcess.StartInfo.RedirectStandardError = true;
						postProcess.StartInfo.CreateNoWindow = false;
						postProcess.EnableRaisingEvents = true;
						postProcess.OutputDataReceived += postState.OutputHandler;
						postProcess.ErrorDataReceived += postState.OutputHandler;
						postProcess.Exited += new EventHandler(PostProcessExit);

						lock(jobHash)
						{
							jobHash.Add(postProcess, postState);
						}

						try
						{
							// Write log header
							GenerateProcessOutput(postState, LOG_SEPARATOR);
							GenerateProcessOutput(postState, "JOB: {0}", postState.Job.Name);
							GenerateProcessOutput(postState, "POST-SYNC START DATE/TIME: " + DateTime.Now);
							GenerateProcessOutput(postState, "STARTING: {0}", postState.Job.PostBatchPath);
							GenerateProcessOutput(postState, LOG_SEPARATOR);

							postProcess.Start();
						}
						catch(Exception ex)
						{
							lock(jobHash)
							{
								jobHash.Remove(postProcess);
							}

							throw ex;
						}

						Interlocked.Increment(ref jobCount);

						postProcess.BeginOutputReadLine();
					}
					else
					{
						EmailLog(state.Logger, state.Job);
						state.Logger.Dispose();
					}
				}
				catch(Exception ex)
				{
					string errStr;

					if(state != null)
					{
						errStr = string.Format("Post sync batch file for job \'{0}\' failed: {1}", state.Job.Name, ex.ToString());
					}
					else
					{
						errStr = string.Format("Post sync batch file failed: {1}", ex.ToString());
					}

					EventLog.WriteEntry(GetAppName(), errStr, EventLogEntryType.Warning);

					if(state != null)
					{
						try
						{
							GenerateProcessOutput(state, errStr);

							EmailLog(state.Logger, state.Job);

							state.Logger.Dispose();
						}
						catch(Exception)
						{
						}
					}
				}
				finally
				{
					lock(jobHash)
					{
						jobHash.Remove(process);
					}

					if(state != null)
					{
						process.OutputDataReceived -= state.OutputHandler;
						process.ErrorDataReceived -= state.OutputHandler;
						process.Exited -= new EventHandler(ProcessExit);
					}

					process.Dispose();

					Interlocked.Decrement(ref jobCount);
				}
			}
        }

		/// <summary>
		/// Emails the log of a job to the current user.
		/// </summary>
		/// <param name="logWriter">The job's log.</param>
		/// <param name="job">The job that has finished running.</param>
		private static void EmailLog(UnrealSyncLogger logWriter, SyncJob job)
        {
            try
            {
				if(!job.SendEmail)
				{
					return;
				}

				logWriter.Flush();

                MailAddress currentUserEmail = new MailAddress(System.Security.Principal.WindowsIdentity.GetCurrent().Name.ToString().Split('\\')[1] + "@epicgames.net");
				SmtpClient smtpClient = new SmtpClient("mail-01.epicgames.net");

				using(MailMessage logEmail = new MailMessage(currentUserEmail, currentUserEmail))
				{
					string body;

					// Don't call Dispose() on reader because it'll close logWriter's stream
					lock(logWriter.SyncObject)
					{
						StreamReader reader = new StreamReader(logWriter.BaseStream);
						reader.BaseStream.Seek(0, SeekOrigin.Begin);
						body = reader.ReadToEnd();
					}

					logEmail.BodyEncoding = Encoding.ASCII;
					logEmail.Subject = string.Format("[UNREALSYNC] Job \'{0}\'", job.Name);
					logEmail.Priority = MailPriority.Normal;
					logEmail.Body = body;
					smtpClient.Send(logEmail);
				}
            }
            catch(Exception ex)
			{
				EventLog.WriteEntry(GetAppName(), ex.ToString(), EventLogEntryType.Warning);
			}
        }

		/// <summary>
		/// Called when a post-job process has exited.
		/// </summary>
		/// <param name="Sender">The process that generated the event.</param>
		/// <param name="e">Information about the event.</param>
        private void PostProcessExit(object Sender, System.EventArgs e) 
		{
			Process process = Sender as Process;

			if(process != null)
			{
				JobState state;

				lock(jobHash)
				{
					jobHash.TryGetValue(process, out state);
				}

				try
				{
					GenerateProcessOutput(state, LOG_SEPARATOR);
					GenerateProcessOutput(state, "JOB: {0}", state.Job.Name);
					GenerateProcessOutput(state, "POST-SYNC END DATE/TIME: " + DateTime.Now);
					GenerateProcessOutput(state, LOG_SEPARATOR);

					EmailLog(state.Logger, state.Job);
				}
				catch(Exception ex)
				{
					EventLog.WriteEntry(GetAppName(), ex.ToString(), EventLogEntryType.Warning);
				}
				finally
				{
					if(state != null)
					{
						state.Logger.Dispose();
						process.OutputDataReceived -= state.OutputHandler;
						process.ErrorDataReceived -= state.OutputHandler;
						process.Exited -= new EventHandler(ProcessExit);
					}

					lock(jobHash)
					{
						jobHash.Remove(process);
					}

					process.Dispose();

					Interlocked.Decrement(ref jobCount);
				}
			}
        }

    }
}
