/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
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
					ActionProcessDictionary.TryGetValue(Action, out ActionProcess);

					if(ActionProcess == null)
					{
						NumUnexecutedActions++;
					}
					else if (ActionProcess.HasExited == false)
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
					ActionProcessDictionary.TryGetValue(Action, out ActionProcess);
					if (ActionProcess == null)
					{
						if (NumExecutingActions < System.Environment.ProcessorCount)
						{
							// Determine whether there are any prerequisites of the action that are outdated.
							bool bHasOutdatedPrerequisites = false;
							foreach (FileItem PrerequisiteItem in Action.PrerequisiteItems)
							{
								if (PrerequisiteItem.ProducingAction != null && Actions.Contains(PrerequisiteItem.ProducingAction))
								{
									Process PrerequisiteProcess = null;
									ActionProcessDictionary.TryGetValue(PrerequisiteItem.ProducingAction, out PrerequisiteProcess);
									if (PrerequisiteProcess == null || PrerequisiteProcess.HasExited == false)
									{
										bHasOutdatedPrerequisites = true;
									}
								}
							}

							// If there aren't any outdated prerequisites of this action, execute it.
							if (!bHasOutdatedPrerequisites)
							{
								// Create the action's process.
								ProcessStartInfo ActionStartInfo = new ProcessStartInfo();
								ActionStartInfo.WorkingDirectory = ExpandEnvironmentVariables(Action.WorkingDirectory);
								ActionStartInfo.FileName = ExpandEnvironmentVariables(Action.CommandPath);
								ActionStartInfo.Arguments = ExpandEnvironmentVariables(Action.CommandArguments);
								ActionStartInfo.UseShellExecute = false;

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

			// Check whether any of the tasks failed.
			bool bSuccess = true;
			foreach (KeyValuePair<Action, Process> ActionProcess in ActionProcessDictionary)
			{
				if (ActionProcess.Value.ExitCode != 0)
				{
					bSuccess = false;
				}
			}

			return bSuccess;
		}
	};
}
