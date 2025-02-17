/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.IO;

namespace UnrealBuildTool
{
	class LocalExecutor
	{
		/** Regex that matches environment variables in $(Variable) format. */
		static Regex EnvironmentVariableRegex = new Regex("\\$\\(([\\d\\w]+)\\)");

		/** Replaces the environment variables references in a string with their values. */
		public static string ExpandEnvironmentVariables(string Text)
		{
			foreach(Match EnvironmentVariableMatch in EnvironmentVariableRegex.Matches(Text))
			{
				string VariableValue = Environment.GetEnvironmentVariable(EnvironmentVariableMatch.Groups[1].Value);
				Text = Text.Replace(EnvironmentVariableMatch.Value, VariableValue);
			}
			return Text;
		}

		/**
		 * Executes the specified actions locally.
		 * @return True if all the tasks succesfully executed, or false if any of them failed.
		 */
		public static bool ExecuteActions(List<Action> Actions)
		{
			Dictionary<Action,Process> ActionProcessDictionary = new Dictionary<Action,Process>();
			while(true)
			{
				// Count the number of unexecuted and still executing actions.
				int NumUnexecutedActions = 0;
				int NumExecutingActions = 0;
				foreach (Action Action in Actions)
				{
					Process ActionProcess = null;
					bool bFoundActionProcess = ActionProcessDictionary.TryGetValue(Action, out ActionProcess);
					if (bFoundActionProcess == false)
					{
						NumUnexecutedActions++;
					}
					else if (ActionProcess != null && ActionProcess.HasExited == false)
					{
						NumUnexecutedActions++;
						NumExecutingActions++;
					}
				}

				// If there aren't any unexecuted actions left, we're done executing.
				if (NumUnexecutedActions == 0)
				{
					break;
				}

				// If there are fewer actions executing than the maximum, look for unexecuted actions that don't have any outdated
				// prerequisites.
				foreach (Action Action in Actions)
				{
					Process ActionProcess = null;
					bool bFoundActionProcess = ActionProcessDictionary.TryGetValue(Action, out ActionProcess);
					if (bFoundActionProcess == false)
					{
						if (NumExecutingActions < Math.Max(1,System.Environment.ProcessorCount * BuildConfiguration.ProcessorCountMultiplier) )
						{
							// Determine whether there are any prerequisites of the action that are outdated.
							bool bHasOutdatedPrerequisites = false;
							bool bHasFailedPrerequisites = false;
							foreach (FileItem PrerequisiteItem in Action.PrerequisiteItems)
							{
								if (PrerequisiteItem.ProducingAction != null && Actions.Contains(PrerequisiteItem.ProducingAction))
								{
									Process PrerequisiteProcess = null;
									bool bFoundPrerequisiteProcess = ActionProcessDictionary.TryGetValue( PrerequisiteItem.ProducingAction, out PrerequisiteProcess );
									if (bFoundPrerequisiteProcess == true)
									{
										if (PrerequisiteProcess == null)
										{
											bHasFailedPrerequisites = true;
										}
										else if (PrerequisiteProcess.HasExited == false)
										{
											bHasOutdatedPrerequisites = true;
										}
										else if (PrerequisiteProcess.ExitCode != 0)
										{
											bHasFailedPrerequisites = true;
										}
									}
									else
									{
										bHasOutdatedPrerequisites = true;
									}
								}
							}

							// If there are any failed prerequisites of this action, don't execute it.
							if (bHasFailedPrerequisites)
							{
								// Add a null entry in the dictionary for this action.
								ActionProcessDictionary.Add( Action, null );
							}
							// If there aren't any outdated prerequisites of this action, execute it.
							else if (!bHasOutdatedPrerequisites)
							{
								// Create the action's process.
								ProcessStartInfo ActionStartInfo = new ProcessStartInfo();
								ActionStartInfo.WorkingDirectory = ExpandEnvironmentVariables(Action.WorkingDirectory);
								ActionStartInfo.FileName = ExpandEnvironmentVariables(Action.CommandPath);
								ActionStartInfo.Arguments = ExpandEnvironmentVariables(Action.CommandArguments);
								ActionStartInfo.UseShellExecute = false;

								// Log command-line used to execute task if debug info printing is enabled.
								if( BuildConfiguration.bPrintDebugInfo )
								{
									Console.WriteLine("Executing: {0} {1}",ActionStartInfo.FileName,ActionStartInfo.Arguments);
								}
								// Log summary if wanted.
								else if( Action.bShouldLogIfExecutedLocally )
								{
									Console.WriteLine("{0} {1}",Path.GetFileName(ActionStartInfo.FileName),Action.StatusDescription);
								}

								// Try to launch the action's process, and produce a friendly error message if it fails.
								try
								{
									ActionProcess = Process.Start(ActionStartInfo);
								}
								catch (Exception)
								{
									throw new BuildException("Failed to start local process for action: {0} {1}", Action.CommandPath, Action.CommandArguments);
								}

								// Add the action's process to the dictionary.
								ActionProcessDictionary.Add(Action, ActionProcess);

								NumExecutingActions++;
							}
						}
					}
				}

				System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(100));
			}

            if( BuildConfiguration.bLogDetailedActionStats )
            {
                Console.WriteLine("^CPU time^Tool^Task^Description");
            }
			double TotalCPUTime = 0;

			// Check whether any of the tasks failed and log action stats if wanted.
			bool bSuccess = true;
			foreach (KeyValuePair<Action, Process> ActionProcess in ActionProcessDictionary)
			{
				// Check for unexecuted actions, preemptive failure
				if (ActionProcess.Value == null)
				{
					bSuccess = false;
					continue;
				}
				// Check for executed action but general failure
				if (ActionProcess.Value.ExitCode != 0)
				{
					bSuccess = false;
				}
                // Log CPU time, tool and task.
				if (BuildConfiguration.bLogDetailedActionStats)
				{
                    Console.WriteLine( "^{0}^{1}^{2}^{3}", 
                        ActionProcess.Value.TotalProcessorTime.TotalSeconds, 
						Path.GetFileName(ActionProcess.Key.CommandPath), 
                        ActionProcess.Key.StatusDescription,
                        ActionProcess.Key.StatusDetailedDescription);
                }
				// Keep track of total CPU seconds spent on tasks.
				TotalCPUTime += ActionProcess.Value.TotalProcessorTime.TotalSeconds;
			}

			// Log total CPU seconds and numbers of processors involved in tasks.
			if( BuildConfiguration.bLogDetailedActionStats || BuildConfiguration.bPrintDebugInfo )
			{
				Console.WriteLine("Total CPU seconds: {0}  Processors: {1}", TotalCPUTime, System.Environment.ProcessorCount);
			}

			return bSuccess;
		}
	};
}
