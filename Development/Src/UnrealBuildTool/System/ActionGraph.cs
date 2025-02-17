/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Diagnostics;

namespace UnrealBuildTool
{
	/** A build action. */
	class Action
	{
		public List<FileItem> PrerequisiteItems = new List<FileItem>();
		public List<FileItem> ProducedItems = new List<FileItem>();

		public string WorkingDirectory = null;
		public string CommandPath = null;
		public string CommandArguments = null;
		public string StatusDescription = "...";
        public string StatusDetailedDescription = "";
		public bool bCanExecuteRemotely = false;
		public bool bIsVCCompiler = false;
		public bool bIsGCCCompiler = false;
		/**
		 * Whether we should log this action if executed by the local executor. This is useful for actions that take time
		 * but invoke tools without any console output.
		 */
		public bool bShouldLogIfExecutedLocally = true;

		public Action()
		{
			UnrealBuildTool.AllActions.Add(this);
		}
	};

	partial class UnrealBuildTool
	{
		public static List<Action> AllActions = new List<Action>();

		/** Number of outdated actions encountered. We know we are doing a full rebuild If outdated actions == AllActions.Count. */
		public static int NumOutdatedActions = 0;


		/** Links actions with their prerequisite and produced items into an action graph. */
		static void LinkActionsAndItems()
		{
			foreach (Action Action in AllActions)
			{
				foreach (FileItem ProducedItem in Action.ProducedItems)
				{
					ProducedItem.ProducingAction = Action;
				}
			}
		}

		/** Checks for cycles in the action graph. */
		static void DetectActionGraphCycles()
		{
			// Starting with actions that only depend on non-produced items, iteratively expand a set of actions that are only dependent on
			// non-cyclical actions.
			Dictionary<Action, bool> ActionIsNonCyclical = new Dictionary<Action, bool>();
			while (true)
			{
				bool bFoundNewNonCyclicalAction = false;

				foreach (Action Action in AllActions)
				{
					if (!ActionIsNonCyclical.ContainsKey(Action))
					{
						// Determine if the action depends on only actions that are already known to be non-cyclical.
						bool bActionOnlyDependsOnNonCyclicalActions = true;
						foreach (FileItem PrerequisiteItem in Action.PrerequisiteItems)
						{
							if (PrerequisiteItem.ProducingAction != null)
							{
								if (!ActionIsNonCyclical.ContainsKey(PrerequisiteItem.ProducingAction))
								{
									bActionOnlyDependsOnNonCyclicalActions = false;
								}
							}
						}

						// If the action only depends on known non-cyclical actions, then add it to the set of known non-cyclical actions.
						if (bActionOnlyDependsOnNonCyclicalActions)
						{
							ActionIsNonCyclical.Add(Action, true);
							bFoundNewNonCyclicalAction = true;
						}
					}
				}

				// If this iteration has visited all actions without finding a new non-cyclical action, then all non-cyclical actions have
				// been found.
				if (!bFoundNewNonCyclicalAction)
				{
					break;
				}
			}

			// If there are any cyclical actions, throw an exception.
			if (ActionIsNonCyclical.Count < AllActions.Count)
			{
				// Describe the cyclical actions.
				string CycleDescription = "";
				foreach (Action Action in AllActions)
				{
					if (!ActionIsNonCyclical.ContainsKey(Action))
					{
						CycleDescription += string.Format("Action: {0}\r\n", Action.CommandPath);
						CycleDescription += string.Format("\twith arguments: {0}\r\n", Action.CommandArguments);
						foreach (FileItem PrerequisiteItem in Action.PrerequisiteItems)
						{
							CycleDescription += string.Format("\tdepends on: {0}\r\n", PrerequisiteItem.AbsolutePath);
						}
						foreach (FileItem ProducedItem in Action.ProducedItems)
						{
							CycleDescription += string.Format("\tproduces:   {0}\r\n", ProducedItem.AbsolutePath);
						}
						CycleDescription += "\r\n\r\n";
					}
				}

				throw new BuildException("Action graph contains cycle!\r\n\r\n{0}", CycleDescription);
			}
		}

		/**
		 * Determines the full set of actions that must be built to produce an item.
		 * @param OutputItem - The item to be built.
		 * @param PrerequisiteActions - The actions that must be built and the root action are 
		 */
		static void GatherPrerequisiteActions(
			FileItem OutputItem,
			ref Dictionary<Action, bool> PrerequisiteActions
			)
		{
			if (OutputItem.ProducingAction != null)
			{
				if (!PrerequisiteActions.ContainsKey(OutputItem.ProducingAction))
				{
					PrerequisiteActions.Add(OutputItem.ProducingAction, true);
					foreach (FileItem PrerequisiteItem in OutputItem.ProducingAction.PrerequisiteItems)
					{
						GatherPrerequisiteActions(PrerequisiteItem, ref PrerequisiteActions);
					}
				}
			}
		}

		/**
		 * Determines whether an action is outdated based on the modification times for its prerequisite
		 * and produced items.
		 * @param RootAction - The action being considered.
		 * @param OutdatedActionDictionary - 
         * @return true if outdated
		 */
        static public bool IsActionOutdated(Action RootAction, ref Dictionary<Action, bool> OutdatedActionDictionary)
		{
			// Only compute the outdated-ness for actions that don't aren't cached in the outdated action dictionary.
			bool bIsOutdated = false;
			if (!OutdatedActionDictionary.TryGetValue(RootAction, out bIsOutdated))
			{
				// Determine the last time the action was run based on the write times of its produced files.
				DateTime LastExecutionTime = DateTime.MaxValue;
				bool bAllProducedItemsExist = true;
				foreach (FileItem ProducedItem in RootAction.ProducedItems)
				{
					// If the produced file doesn't exist or has zero size, consider it outdated.  The zero size check is to detect cases
					// where aborting an earlier compile produced invalid zero-sized obj files, but that may cause actions where that's
					// legitimate output to always be considered outdated.
					if (ProducedItem.bExists && ProducedItem.Info.Length > 0)
					{
						// Use the oldest produced item's time as the last execution time.
						bool bItemWriteTimeIsOlderThanLastExecution = ProducedItem.LastWriteTime.CompareTo(LastExecutionTime) < 0;
						if (bItemWriteTimeIsOlderThanLastExecution)
						{
							LastExecutionTime = ProducedItem.LastWriteTime;
						}
					}
					else
					{
						// If any of the produced items doesn't exist, the action is outdated.
						if (BuildConfiguration.bPrintDebugInfo)
						{
							Console.WriteLine(
								"{0}: Produced item \"{1}\" doesn't exist.",
								RootAction.StatusDescription,
								Path.GetFileName(ProducedItem.AbsolutePath)
								);
						}
						bAllProducedItemsExist = false;
						bIsOutdated = true;
						break;
					}
				}

				if(bAllProducedItemsExist)
				{
					// Check if any of the prerequisite items are produced by outdated actions, or have changed more recently than
					// the oldest produced item.
					foreach (FileItem PrerequisiteItem in RootAction.PrerequisiteItems)
					{
						if (PrerequisiteItem.ProducingAction != null)
						{
							if(IsActionOutdated(PrerequisiteItem.ProducingAction,ref OutdatedActionDictionary))
							{
								if (BuildConfiguration.bPrintDebugInfo)
								{
									Console.WriteLine(
										"{0}: Prerequisite {1} is produced by outdated action.",
										RootAction.StatusDescription,
										Path.GetFileName(PrerequisiteItem.AbsolutePath)
										);
								}
								bIsOutdated = true;
							}
						}

						if (PrerequisiteItem.bExists)
						{
							bool bPrerequisiteItemIsNewerThanLastExecution = PrerequisiteItem.LastWriteTime.CompareTo(LastExecutionTime) > 0;
							if (bPrerequisiteItemIsNewerThanLastExecution)
							{
								if (BuildConfiguration.bPrintDebugInfo)
								{
									Console.WriteLine(
										"{0}: Prerequisite {1} is newer than the last execution of the action: {2} vs {3}",
										RootAction.StatusDescription,
										Path.GetFileName(PrerequisiteItem.AbsolutePath),
										PrerequisiteItem.LastWriteTime,
										LastExecutionTime
										);
								}
								bIsOutdated = true;
							}
						}

						// GatherAllOutdatedActions will ensure all actions are checked for outdated-ness, so we don't need to recurse with
						// all this action's prerequisites once we've determined it's outdated.
						if (bIsOutdated)
						{
							break;
						}
					}
				}

				// Cache the outdated-ness of this action.
				OutdatedActionDictionary.Add(RootAction, bIsOutdated);

				// Keep track of how many outdated actions there are. Used to determine whether this was a full rebuild or not.
				if( bIsOutdated )
				{
					NumOutdatedActions++;
				}
			}

			return bIsOutdated;
		}

		/**
		 * Builds a dictionary containing the actions from AllActions that are outdated by calling
		 * IsActionOutdated.
		 */
		static Dictionary<Action,bool> GatherAllOutdatedActions()
		{
			Dictionary<Action, bool> OutdatedActionDictionary = new Dictionary<Action, bool>();

			foreach (Action Action in AllActions)
			{
				IsActionOutdated(Action, ref OutdatedActionDictionary);
			}

			return OutdatedActionDictionary;
		}

		/** Deletes all the items produced by actions in the provided outdated action dictionary. */
		static void DeleteAllOutdatedProducedItems(Dictionary<Action, bool> OutdatedActionDictionary)
		{
			foreach (KeyValuePair<Action,bool> OutdatedActionInfo in OutdatedActionDictionary)
			{
				if (OutdatedActionInfo.Value)
				{
					foreach (FileItem ProducedItem in OutdatedActionInfo.Key.ProducedItems)
					{
						if (ProducedItem.bExists)
						{
							if (BuildConfiguration.bPrintDebugInfo)
							{
								Console.WriteLine("Deleting outdated item: {0}", ProducedItem.AbsolutePath);
							}
							ProducedItem.Delete();
						}
					}
				}
			}
		}

		/**
		 * Creates directories for all the items produced by actions in the provided outdated action
		 * dictionary.
		 */
		static void CreateDirectoriesForProducedItems(Dictionary<Action, bool> OutdatedActionDictionary)
		{
			foreach (KeyValuePair<Action, bool> OutdatedActionInfo in OutdatedActionDictionary)
			{
				if (OutdatedActionInfo.Value)
				{
					foreach (FileItem ProducedItem in OutdatedActionInfo.Key.ProducedItems)
					{
						string DirectoryPath = Path.GetDirectoryName(ProducedItem.AbsolutePath);
						if(!Directory.Exists(DirectoryPath))
						{
							if (BuildConfiguration.bPrintDebugInfo)
							{
								Console.WriteLine("Creating directory for produced item: {0}", DirectoryPath);
							}
							Directory.CreateDirectory(DirectoryPath);
						}
					}
				}
			}
		}
	};
}
