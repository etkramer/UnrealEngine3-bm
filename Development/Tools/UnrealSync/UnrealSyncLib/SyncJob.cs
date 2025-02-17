using System;
using System.Collections.Generic;
using System.Text;
using System.ComponentModel;
using System.Drawing.Design;
using UnrealSync.Service;

namespace UnrealSync
{
	/// <summary>
	/// This class represents a sync job.
	/// </summary>
    public class SyncJob
    {
        private string name;
        private bool enabled = true;
        private bool killGameProcess = true;
		private bool sendEmail = true;
        private string startTime = "2:00 AM";
        private string perforceClientSpec = "";
        private string batchFilePath = "";
        private string gameProcessName = "";
        private string label = "";
        private string postBatchPath = "";

		[DisplayName("Send Email")]
		[Description("Set this to True if you want to be sent an email for every job that finishes.")]
		public bool SendEmail
		{
			get { return sendEmail; }
			set { sendEmail = value; }
		}

		[DisplayName("Kill Game Process")]
		[Description("Set this to True if you want to kill the game process associated with the job when the job is run.")]
        public bool KillGameProcess
        {
            get { return killGameProcess; }
			set { killGameProcess = value; }
        }

		[DisplayName("Post Sync Batch File Path")]
		[Description("The path to a batch file that will be executed after the sync job completes.")]
		[Editor(typeof(FileBrowserEditor), typeof(UITypeEditor))]
        public string PostBatchPath
        {
            get { return postBatchPath; }
			set { postBatchPath = value; }
        }

		[DisplayName("Sync to Label")]
		[Description("A label you want to sync the client to. If the sync batch file path and the sync label are empty then the sync operation will be performed to head.\n\nNote:\nUsing %D% will substitute the date in YYYY-MM-DD format.")]
        public string Label
        {
            get { return label; }
			set { label = value; }
        }

        public SyncJob(String name)
        {
            this.name = name;
        }

		[Description("The name of the sync job.")]
        public string Name
        {
            get { return name; }
			set { name = value; }
        }

		[Description("Set this to true if you want the job to be run at Start Time. Otherwise the job will not be run.")]
        public bool Enabled
        {
            get { return enabled; }
			set { enabled = value; }
        }

		[DisplayName("Start Time")]
		[Description("The time the sync job will be run every day. The proper format for this is xx:xx xx. For example: 12:32 AM or 3:55 PM")]
		[Editor(typeof(TimeEditor), typeof(UITypeEditor))]
        public string StartTime
        {
            get { return startTime; }
			set { DateTime.ParseExact(value, TimeEditor.DT_FORMAT, System.Threading.Thread.CurrentThread.CurrentUICulture); startTime = value; }
        }

		[DisplayName("Perforce Client Spec")]
		[Description("The client spec in perforce you wish to sync with.")]
        public string PerforceClientSpec
        {
            get { return perforceClientSpec; }
			set { perforceClientSpec = value; }
        }

		[DisplayName("Batch File Path")]
		[Description("The path to a batch file you want to run for the sync operation. If the sync batch file path and the sync label are empty then the sync operation will be performed to head.")]
		[Editor(typeof(FileBrowserEditor), typeof(UITypeEditor))]
        public string BatchFilePath
        {
            get { return batchFilePath; }
			set { batchFilePath = value; }
        }

		[DisplayName("Game Process Name")]
		[Description("The name of the game process this job is associated with.")]
        public string GameProcessName
        {
            get { return gameProcessName; }
			set { gameProcessName = value; }
        }

        public DateTime GetStartDateTime()
        {
			return DateTime.ParseExact(startTime, TimeEditor.DT_FORMAT, System.Threading.Thread.CurrentThread.CurrentUICulture);
        }

		public override string ToString()
		{
 			 return this.name;
		}
    }
}
