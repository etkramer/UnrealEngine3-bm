using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Win32;

namespace UnrealSync
{
	/// <summary>
	/// Handles the registry entries for a sync job.
	/// </summary>
    public static class AppSettings
    {
		/// <summary>
		/// Gets the root sync job registry key.
		/// </summary>
		/// <returns>A <see cref="RegistryKey"/> pointing to the root sync job directory.</returns>
        private static RegistryKey GetUnrealSyncKey()
        {
            return Registry.CurrentUser.OpenSubKey("SOFTWARE", true).CreateSubKey("Epic Games").CreateSubKey("UnrealSync");
        }

		/// <summary>
		/// Retrieves an array of all sync jobs.
		/// </summary>
		/// <returns>An array of all sync jobs.</returns>
        public static SyncJob[] GetSyncJobs()
        {
			List<SyncJob> syncJobs = new List<SyncJob>();

			using(RegistryKey UnrealSyncKey = GetUnrealSyncKey())
			{
				// If the key exists, continue reading the jobs.
				if(UnrealSyncKey != null)
				{
					string[] projects = UnrealSyncKey.GetSubKeyNames();
					for(int i = 0; i < projects.Length; i++)
					{
						syncJobs.Add(GetSyncJobByName(projects[i]));
					}
				}
			}

            return syncJobs.ToArray();
        }

		/// <summary>
		/// Writes the registry entries for a sync job.
		/// </summary>
		/// <param name="jobToAdd">The job to be created.</param>
        public static void AddSyncJob(SyncJob jobToAdd)
        {
			using(RegistryKey UnrealSyncKey = GetUnrealSyncKey())
			{
				UnrealSyncKey.CreateSubKey(jobToAdd.Name);
				UnrealSyncKey.Close();
			}

            UpdateSyncJob(jobToAdd);
        }

		/// <summary>
		/// Updates the registry entries for a sync job.
		/// </summary>
		/// <param name="job">The job to be updated.</param>
        public static void UpdateSyncJob(SyncJob job)
        {
			using(RegistryKey jobRegKey = GetUnrealSyncKey().OpenSubKey(job.Name, true))
			{
				jobRegKey.SetValue("Enabled", job.Enabled);
				jobRegKey.SetValue("StartTime", job.StartTime);
				jobRegKey.SetValue("PerforceClientSpec", job.PerforceClientSpec);
				jobRegKey.SetValue("BatchFilePath", job.BatchFilePath);
				jobRegKey.SetValue("PostBatchPath", job.PostBatchPath);
				jobRegKey.SetValue("Label", job.Label);
				jobRegKey.SetValue("GameProcessName", job.GameProcessName);
				jobRegKey.SetValue("KillGameProcess", job.KillGameProcess);
				jobRegKey.SetValue("SendEmail", job.SendEmail);
				jobRegKey.Close();
			}
        }

		/// <summary>
		/// Removes the registry entries for a sync job.
		/// </summary>
		/// <param name="jobName">The name of the job to delete.</param>
		/// <returns>The job that was deleted.</returns>
        public static SyncJob RemoveSyncJobByName(String jobName)
        {
            SyncJob removedSyncJob = GetSyncJobByName(jobName);

			using(RegistryKey key = GetUnrealSyncKey())
			{
				key.DeleteSubKey(jobName);
			}

            return removedSyncJob;
        }

		/// <summary>
		/// Gets a sync job with the specified name of it exists.
		/// </summary>
		/// <param name="jobName">The name of the job to retrieve.</param>
		/// <returns>A new instance of <see cref="SyncJob"/> that is empty if a job with the specified name couldn't be found.</returns>
        public static SyncJob GetSyncJobByName(string jobName)
        {
			SyncJob retrievedJob = new SyncJob(jobName);

			using(RegistryKey currentKey = GetUnrealSyncKey().OpenSubKey(jobName))
			{
				bool temp;

				if(bool.TryParse((string)currentKey.GetValue("Enabled"), out temp))
				{
					retrievedJob.Enabled = temp;
				}

				if(bool.TryParse((string)currentKey.GetValue("SendEmail"), out temp))
				{
					retrievedJob.SendEmail = temp;
				}

				if(bool.TryParse((string)currentKey.GetValue("KillGameProcess"), out temp))
				{
					retrievedJob.KillGameProcess = temp;
				}

				retrievedJob.StartTime = (string)currentKey.GetValue("StartTime");
				retrievedJob.PerforceClientSpec = (string)currentKey.GetValue("PerforceClientSpec");
				retrievedJob.BatchFilePath = (string)currentKey.GetValue("BatchFilePath");
				retrievedJob.PostBatchPath = (string)currentKey.GetValue("PostBatchPath");
				retrievedJob.Label = (string)currentKey.GetValue("Label");
				retrievedJob.GameProcessName = (string)currentKey.GetValue("GameProcessName");
			}
            return retrievedJob;
        }
    }
}