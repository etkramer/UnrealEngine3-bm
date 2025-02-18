/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
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
				if (ActionsNeededForThisTarget.ContainsKey(Action) && OutdatedActionDictionary[Action])
				{
					ActionsToExecute.Add(Action);
				}
			}

			return ActionsToExecute;
		}

		/** Executes a list of actions. */
		static bool ExecuteActions(List<Action> ActionsToExecute)
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
						return XGEResult == XGE.ExecutionResult.TasksSucceeded;
					}
				}

				// If XGE is disallowed or unavailable, execute the commands locally.
				return LocalExecutor.ExecuteActions(ActionsToExecute);
			}
			else
			{
				Console.WriteLine("Target is up to date.");
			}

			return true;
		}

		static int Main(string[] Arguments)
		{
			bool bSuccess = true;
		
			bool bCreatedMutex = false;
			using (Mutex SingleInstanceMutex = new Mutex(true, "Global\\UnrealBuildTool_Mutex", out bCreatedMutex))
			{
				try
				{
					if (!bCreatedMutex)
					{
						// If this instance didn't create the mutex, wait for the existing mutex to be released by the mutex's creator.
						SingleInstanceMutex.WaitOne();
					}

					// Parse optional command-line flags.
					BuildConfiguration.bPrintDebugInfo = Utils.ParseCommandLineFlag(Arguments, "-Verbose");

					if (Utils.ParseCommandLineFlag(Arguments, "-NoXGE"))
					{
						BuildConfiguration.bAllowXGE = false;
					}

					if (Utils.ParseCommandLineFlag(Arguments, "-StressTestUnity"))
					{
						BuildConfiguration.bStressTestUnity = true;
					}

					if (Utils.ParseCommandLineFlag(Arguments, "-DisableUnity"))
					{
						BuildConfiguration.bUseUnityBuild = false;
					}

					// Configure the build actions and items.
					Target Target = new UE3BuildTarget();
					IEnumerable<FileItem> TargetOutputItems = Target.Build(Arguments);

					// Plan the actions to execute for the build.
					List<Action> ActionsToExecute = GetActionsToExecute(TargetOutputItems);

					// Display some stats to the user.
#if false
					if (BuildConfiguration.bPrintDebugInfo)
#endif
					{
						Console.WriteLine(
							"{0} actions, {1} outdated and requested actions",
							AllActions.Count,
							ActionsToExecute.Count
							);
					}

					// Execute the actions.
					bSuccess = ExecuteActions(ActionsToExecute);
				}
				catch (Exception Exception)
				{
					Console.WriteLine("{0}", Exception);
					bSuccess = false;
				}

				// Release the mutex.
				SingleInstanceMutex.ReleaseMutex();
			}

			return bSuccess ? 0 : 1;
		}
	}
}

