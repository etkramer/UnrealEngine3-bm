using System;
using System.Collections.Generic;
using System.ServiceProcess;
using System.Text;
using Microsoft.Win32;
using UnrealSync;
using System.Timers;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;

namespace UnrealSync.Service
{
    public class UnrealSyncService : System.ServiceProcess.ServiceBase
    {
        private Timer runTimer;
        private ServiceHelper helper;

        public UnrealSyncService()
        {
            this.ServiceName = "UnrealSync";
            this.CanStop = true;
            this.CanPauseAndContinue = true;
            this.AutoLog = true;
            runTimer = new Timer();
            helper = new ServiceHelper();
            runTimer.Interval = 60000;
            runTimer.Elapsed += new ElapsedEventHandler(CheckForSyncJobAndRun);
        }

        private void CheckForSyncJobAndRun(Object sender, ElapsedEventArgs e)
        {
			try
			{
				helper.CheckForSyncJobAndRun(false);
			}
			catch(Exception ex)
			{
				System.Diagnostics.EventLog.WriteEntry("UnrealSyncService.exe", ex.ToString(), EventLogEntryType.Error);
			}
        }

        [STAThread]
        static void Main()
        {
            System.ServiceProcess.ServiceBase[] ServicesToRun;
            ServicesToRun = new System.ServiceProcess.ServiceBase[] { new UnrealSyncService() };
            System.ServiceProcess.ServiceBase.Run(ServicesToRun);
        }

        protected override void OnStart(string[] args)
        {
			ServiceHelper.CreateListenSocket();

            runTimer.AutoReset = true;
            runTimer.Enabled = true;
        }

        protected override void OnStop()
        {
            runTimer.AutoReset = false;
            runTimer.Enabled = false;

			ServiceHelper.CloseListenSocket();
        }

    }
}