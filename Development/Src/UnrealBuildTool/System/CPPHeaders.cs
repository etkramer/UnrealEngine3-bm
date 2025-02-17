/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Text.RegularExpressions;
using System.IO;

namespace UnrealBuildTool
{
	partial class CPPEnvironment
	{
		/** Contains a mapping from filename to the full path of the header in this environment. */
		Dictionary<string, FileItem> IncludeFileSearchDictionary = new Dictionary<string, FileItem>();

		/** Finds the header file that is referred to by a partial include filename. */
		public FileItem FindIncludedFile(string RelativeIncludePath)
		{
			// Only search for the include file if the result hasn't been cached.
			FileItem Result = null;
			if (!IncludeFileSearchDictionary.TryGetValue(RelativeIncludePath, out Result))
			{
				// Build a single list of include paths to search.
				List<string> IncludePathsToSearch = new List<string>();
				IncludePathsToSearch.AddRange(IncludePaths);

				// Optionally search for the file in the system include paths.
				if (BuildConfiguration.bCheckSystemHeadersForModification)
				{
					IncludePathsToSearch.AddRange(SystemIncludePaths);
				}

				// Find the first include path that the included file exists in.
				foreach (string IncludePath in IncludePathsToSearch)
				{
					if (Result == null)
					{
						Result = FileItem.GetExistingItemByPath(
							Path.Combine(
								IncludePath,
								RelativeIncludePath
								)
							);
					}
					else
					{
						break;
					}
				}

				// Cache the result of the include path search.
				IncludeFileSearchDictionary.Add(RelativeIncludePath, Result);
			}

			if (BuildConfiguration.bPrintDebugInfo)
			{
				if (Result != null)
				{
					Console.WriteLine("Resolved included file \"{0}\" to: {1}", RelativeIncludePath, Result.AbsolutePath);
				}
				else
				{
					Console.WriteLine("Couldn't resolve included file \"{0}\"", RelativeIncludePath);
				}
			}

			return Result;
		}
		
		/** A cache of the list of other files that are directly or indirectly included by a C++ file. */
		static Dictionary<FileItem, List<FileItem>> IncludedFilesMap = new Dictionary<FileItem, List<FileItem>>();

		/** Finds the files directly or indirectly included by the given C++ file. */
		void GetIncludeDependencies(FileItem CPPFile, ref Dictionary<FileItem,bool> Result)
		{
			if (!IncludedFilesMap.ContainsKey(CPPFile))
			{
				// Add a dummy entry for the include file to avoid infinitely recursing on include file loops.
				IncludedFilesMap.Add(CPPFile, new List<FileItem>());

				// Gather a list of names of files directly included by this C++ file.
				List<string> DirectlyIncludedFileNames = GetDirectIncludeDependencies(CPPFile);

				// Build a list of the unique set of files that are included by this file.
				Dictionary<FileItem, bool> IncludedFileDictionary = new Dictionary<FileItem, bool>();
				foreach (string DirectlyIncludedFileName in DirectlyIncludedFileNames)
				{
					// Resolve the included file name to an actual file.
					FileItem IncludedFile = FindIncludedFile(DirectlyIncludedFileName);
					if (IncludedFile != null)
					{
						if (!IncludedFileDictionary.ContainsKey(IncludedFile))
						{
							IncludedFileDictionary.Add(IncludedFile,true);
						}
					}
				}

				// Convert the dictionary of files included by this file into a list.
				List<FileItem> IncludedFileList = new List<FileItem>();
				foreach (KeyValuePair<FileItem, bool> IncludedFile in IncludedFileDictionary)
				{
					IncludedFileList.Add(IncludedFile.Key);
				}

				// Add the set of files included by this file to the cache.
				IncludedFilesMap.Remove(CPPFile);
				IncludedFilesMap.Add(CPPFile, IncludedFileList);
			}

			// Copy the list of files included by this file into the result list.
			foreach (FileItem IncludedFile in IncludedFilesMap[CPPFile])
			{
				if (!Result.ContainsKey(IncludedFile))
				{
					// If the result list doesn't contain this file yet, add the file and the files it includes.
					Result.Add(IncludedFile,true);
					GetIncludeDependencies(IncludedFile, ref Result);
				}
			}
		}

		/** @return The list of files which are directly or indirectly included by a C++ file. */
		public List<FileItem> GetIncludeDependencies(FileItem CPPFile)
		{
			// Find the dependencies of the file.
			Dictionary<FileItem, bool> IncludedFileDictionary = new Dictionary<FileItem, bool>();
			GetIncludeDependencies(CPPFile, ref IncludedFileDictionary);

			// Convert the dependency dictionary into a list.
			List<FileItem> Result = new List<FileItem>();
			foreach (KeyValuePair<FileItem, bool> IncludedFile in IncludedFileDictionary)
			{
				Result.Add(IncludedFile.Key);
			}

			return Result;
		}

		/** Regex that matches #include statements. */
		static Regex CPPHeaderRegex = new Regex("[ \t]*#[ \t]*include[ \t]*[<\"]([^\">]*)[\">].*", RegexOptions.Compiled);

		/** Finds the names of files directly included by the given C++ file. */
		public List<string> GetDirectIncludeDependencies(FileItem CPPFile)
		{
			// Read lines from the C++ file.
			using (FileStream CPPFileStream = new FileStream(CPPFile.AbsolutePath, FileMode.Open, FileAccess.Read))
			{
				using (StreamReader CPPFileReader = new StreamReader(CPPFileStream))
				{
					List<string> Result = new List<string>();
					while(!CPPFileReader.EndOfStream)
					{
						string Line = CPPFileReader.ReadLine();

						// Check whether the first non-whitespace character of the line is a #.
						bool bFirstCharacterIsPound = false;
						for (int CharacterIndex = 0; CharacterIndex < Line.Length; CharacterIndex++)
						{
							if (Line[CharacterIndex] == '#')
							{
								bFirstCharacterIsPound = true;
								break;
							}
							else if (Line[CharacterIndex] != ' ' && Line[CharacterIndex] != '\t')
							{
								break;
							}
						}

						// If the line starts with a #, check whether it's an #include statement.
						if (bFirstCharacterIsPound)
						{
							Match IncludeMatch = CPPHeaderRegex.Match(Line);
							if (IncludeMatch.Success)
							{
								Result.Add(IncludeMatch.Groups[1].Value);
							}
						}
					}

					return Result;
				}
			}
		}
	}
}