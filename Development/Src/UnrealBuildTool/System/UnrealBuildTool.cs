/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Threading;

namespace UnrealBuildTool
{
    partial class UnrealBuildTool
	{
		/** Builds a list of actions that need to be executed to produce the specified output items. */
		static List<Action> GetActionsToExecute(IEnumerable<FileItem> OutputItems)
		{
			// Link producing actions to the items they produce.
			LinkActionsAndItems();

			// Detect cycles in the action graph.
			DetectActionGraphCycles();

			// Build a set of all actions needed for this target.
			Dictionary<Action, bool> ActionsNeededForThisTarget = new Dictionary<Action, bool>();

			// For now simply treat all object files as the root target.
			foreach (FileItem OutputItem in OutputItems)
			{
				GatherPrerequisiteActions(OutputItem, ref ActionsNeededForThisTarget);
			}

			// Build a set of all actions that are outdated.
			Dictionary<Action, bool> OutdatedActionDictionary = GatherAllOutdatedActions();

			// Delete produced items that are outdated.
            if( BuildConfiguration.bShouldDeleteAllOutdatedProducedItems )
            {
                DeleteAllOutdatedProducedItems( OutdatedActionDictionary );
            }

			// Create directories for the outdated produced items.
			CreateDirectoriesForProducedItems(OutdatedActionDictionary);

			// Build a list of actions that are both needed for this target and outdated.
			List<Action> ActionsToExecute = new List<Action>();
			foreach (Action Action in AllActions)
			{
				if (ActionsNeededForThisTarget.ContainsKey(Action) && OutdatedActionDictionary[Action] && Action.CommandPath != null)
				{
					ActionsToExecute.Add(Action);
				}
			}

			return ActionsToExecute;
		}

		/** Executes a list of actions. */
		static bool ExecuteActions(List<Action> ActionsToExecute, out string ExecutorName )
		{
			if (ActionsToExecute.Count > 0)
			{
				if (BuildConfiguration.bAllowXGE)
				{
					// Write the actions to execute to a XGE task file.
					string XGETaskFilePath = Path.Combine(BuildConfiguration.BaseIntermediatePath, "XGETasks.xml");
					XGE.WriteTaskFile(ActionsToExecute, XGETaskFilePath);

					// Try to execute the XGE tasks, and if XGE is available, skip the local execution fallback.
					XGE.ExecutionResult XGEResult = XGE.ExecuteTaskFile(XGETaskFilePath);
					if (XGEResult != XGE.ExecutionResult.Unavailable)
					{
						ExecutorName = "XGE";
						return XGEResult == XGE.ExecutionResult.TasksSucceeded;
					}
				}

				// If XGE is disallowed or unavailable, execute the commands locally.
				ExecutorName = "Local";
				return LocalExecutor.ExecuteActions(ActionsToExecute);
			}
			// Nothing to execute.
			else
			{
				ExecutorName = "NoActionsToExecute";
				Console.WriteLine("Target is up to date.");
			}

			return true;
		}

		static int Main(string[] Arguments)
		{
			// Helpers used for stats tracking.
			TimeSpan StartTime = new TimeSpan(System.DateTime.Now.Ticks);
			Target Target =  null;
			int NumExecutedActions = 0;
			double MutexWaitTime = 0;
			string ExecutorName = "Unknown";

			// Don't allow simultaneous execution of Unreal Built Tool. Multi-selection in the UI e.g. causes this and you want serial
			// execution in this case to avoid contention issues with shared produced items.
			bool bSuccess = true;
			bool bCreatedMutex = false;
			using (Mutex SingleInstanceMutex = new Mutex(true, "Global\\UnrealBuildTool_Mutex", out bCreatedMutex))
			{
				try
                {
                    // Log command-line arguments.
                    if (BuildConfiguration.bPrintDebugInfo)
                    {
                        Console.Write("Command-line arguments: ");
                        foreach (string Argument in Arguments)
                        {
                            Console.Write("{0} ", Argument);
                        }
                        Console.WriteLine("");
                    }

                    if (!bCreatedMutex)
                    {
                        // If this instance didn't create the mutex, wait for the existing mutex to be released by the mutex's creator.
                        TimeSpan MutexWaitStartTime = new TimeSpan(System.DateTime.Now.Ticks);
                        SingleInstanceMutex.WaitOne();
                        MutexWaitTime = new TimeSpan(System.DateTime.Now.Ticks).Subtract(MutexWaitStartTime).TotalSeconds;
                    }

                    // Parse optional command-line flags.
                    if (Utils.ParseCommandLineFlag(Arguments, "-Verbose"))
                    {
                        BuildConfiguration.bPrintDebugInfo = true;
                    }

                    if (Utils.ParseCommandLineFlag(Arguments, "-NoXGE"))
                    {
                        BuildConfiguration.bAllowXGE = false;
                    }

                    if (Utils.ParseCommandLineFlag(Arguments, "-NoXGEMonitor"))
                    {
                        BuildConfiguration.bShowXGEMonitor = false;
                    }

                    if (Utils.ParseCommandLineFlag(Arguments, "-StressTestUnity"))
                    {
                        BuildConfiguration.bStressTestUnity = true;
                    }

                    if (Utils.ParseCommandLineFlag(Arguments, "-DisableUnity"))
                    {
                        BuildConfiguration.bUseUnityBuild = false;
                    }

                    if (Utils.ParseCommandLineFlag(Arguments, "-NoPCH"))
                    {
                        BuildConfiguration.bUsePCHFiles = false;
                    }

                    StripHeadersUtil StripHeaders = null;
                    if (BuildConfiguration.bRemoveUnusedHeaders)
                    {
                        StripHeaders = new StripHeadersUtil();
                    }

                    while(true)
                    {
                        // Clear cache of file entries to refresh any files that mayb have been modified 
                        FileItem.ResetFileMapCache();

                        // Configure the build actions and items.
                        Target = new UE3BuildTarget();
                        IEnumerable<FileItem> TargetOutputItems = Target.Build(Arguments);

                        // Plan the actions to execute for the build.
                        List<Action> ActionsToExecute = GetActionsToExecute(TargetOutputItems);
                        NumExecutedActions = ActionsToExecute.Count;

                        // Display some stats to the user.
                        if (BuildConfiguration.bPrintDebugInfo)
                        {
                            Console.WriteLine(
                                "{0} actions, {1} outdated and requested actions",
                                AllActions.Count,
                                ActionsToExecute.Count
                                );
                        }

                        // Execute the actions.
                        bSuccess = ExecuteActions(ActionsToExecute, out ExecutorName);
                        
                        // Iterate compilation if we're stripping headers from source files
                        if (StripHeaders == null || 
							!StripHeaders.ProcessNext(ActionsToExecute,Target))
                        {
                            break;
                        }
                    }
                }
				catch (Exception Exception)
				{
					Console.WriteLine("{0}", Exception);
					bSuccess = false;
				}

				// Release the mutex.
				SingleInstanceMutex.ReleaseMutex();
			}

			// Figure out how long we took to execute and update stats DB if there is a valid target.
			double BuildDuration = new TimeSpan(System.DateTime.Now.Ticks).Subtract(StartTime).TotalSeconds - MutexWaitTime;
			if( Target != null )
			{
				PerfDataBase.SendBuildSummary( BuildDuration, Target, bSuccess, AllActions.Count, NumOutdatedActions, NumExecutedActions, ExecutorName );
			}

			// Update duration to include time taken to talk to the database and log it to the console
			double BuildAndSqlDuration = new TimeSpan(System.DateTime.Now.Ticks).Subtract(StartTime).TotalSeconds - MutexWaitTime;
			Console.WriteLine("UBT execution time: {0:0.00} seconds", BuildAndSqlDuration);

			// Warn is connecting to the DB took too long.
			if( BuildAndSqlDuration - BuildDuration > 1 )
			{
				Console.WriteLine("Warning: Communicating with the Database took {0} seconds.",BuildAndSqlDuration - BuildDuration);
			}

			return bSuccess ? 0 : 1;
		}
	}
}

