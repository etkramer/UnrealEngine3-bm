/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System.Collections.Generic;
using System.IO;
using System.Diagnostics;

namespace UnrealBuildTool
{
	abstract class Utils
	{
		/** Searches for a flag in a set of command-line arguments. */
		public static bool ParseCommandLineFlag(string[] Arguments, string FlagName)
		{
			// Find an argument with the given name.
			for (int ArgumentIndex = 0; ArgumentIndex < Arguments.Length; ArgumentIndex++)
			{
				string Argument = Arguments[ArgumentIndex].ToUpperInvariant();
				if (Argument == FlagName.ToUpperInvariant())
				{
					return true;
				}
			}
			return false;
		}

		/**
		 * Compiles a set of projects.
		 * @param CompileEnvironment - The environment the C/C++ files in the project are compiled with.
		 * @param LinkEnvironment - The object files produced by compiling the project will be added to the
		 *							LinkEnvironment's InputFiles list.
		 * @param ProjectPaths - A list of paths to Visual C++ .vcproj files containing the files to compile.
		 */
		public static void CompileProjects(
			CPPEnvironment CompileEnvironment,
			LinkEnvironment LinkEnvironment,
			IEnumerable<string> ProjectPaths
			)
		{
			foreach (string ProjectPath in ProjectPaths)
			{
				CPPEnvironment ProjectCPPEnvironment = new CPPEnvironment(CompileEnvironment);
				ProjectCPPEnvironment.OutputDirectory = Path.Combine(
					CompileEnvironment.OutputDirectory,
					Path.GetFileNameWithoutExtension(ProjectPath)
					);
			
				// Allow files to be included from the project's source directory.
				ProjectCPPEnvironment.IncludePaths.Add(Path.Combine(Path.GetDirectoryName(ProjectPath), "Src"));

				// If the project file doesn't exist, display a friendly error.
				if (!File.Exists(ProjectPath))
				{
					throw new BuildException("Project file \"{0}\" doesn't exist.", ProjectPath);
				}

				// Load the list of files from the specified project file.
				string ProjectDirectory = Path.GetDirectoryName(ProjectPath);
				List<string> ProjectFilePaths = VCProject.GetProjectFiles(ProjectPath);

				// Gather a list of CPP files in the project.
				List<FileItem> CPPFiles = new List<FileItem>();
				List<FileItem> CFiles = new List<FileItem>();
				List<FileItem> CAndCPPFiles = new List<FileItem>();
				List<FileItem> RCFiles = new List<FileItem>();
				foreach (string ProjectFilePath in ProjectFilePaths)
				{
					string Extension = Path.GetExtension(ProjectFilePath).ToUpperInvariant();
					FileItem FileItem = FileItem.GetExistingItemByPath(Path.Combine(ProjectDirectory, ProjectFilePath));
					if (FileItem != null)
					{
						if (Extension == ".CPP")
						{
							CPPFiles.Add(FileItem);
							CAndCPPFiles.Add(FileItem);
						}
						else if (Extension == ".C")
						{
							CFiles.Add(FileItem);
							CAndCPPFiles.Add(FileItem);
						}
						else if (Extension == ".RC")
						{
							RCFiles.Add(FileItem);
						}
					}
				}

				// Use precompiled headers if requested.
				CPPOutput PCHOutput = null;
				if (BuildConfiguration.bUsePCHFiles)
				{
					// Check if all of the C++ files include the same file as their first header.
					string PrecompiledHeaderIncludeFilename = null;
					foreach (FileItem CPPFile in CAndCPPFiles)
					{
						List<string> DirectIncludeFilenames = ProjectCPPEnvironment.GetDirectIncludeDependencies(CPPFile);

						if (DirectIncludeFilenames.Count > 0)
						{
							string PotentialPrecompiledHeaderIncludeFilename = DirectIncludeFilenames[0];
							if (PrecompiledHeaderIncludeFilename == null)
							{
								PrecompiledHeaderIncludeFilename = PotentialPrecompiledHeaderIncludeFilename;
							}
							else if (PrecompiledHeaderIncludeFilename != PotentialPrecompiledHeaderIncludeFilename)
							{
								PrecompiledHeaderIncludeFilename = null;
								break;
							}
						}
					}

					// If there was one header that was included first by all C++ files, use it as the precompiled header.
					if (PrecompiledHeaderIncludeFilename != null)
					{
						// Set the environment's precompiled header include filename.
						ProjectCPPEnvironment.PrecompiledHeaderIncludeFilename = PrecompiledHeaderIncludeFilename;

						// Find the header file to be precompiled.
						FileItem PrecompiledHeaderIncludeFile = ProjectCPPEnvironment.FindIncludedFile(PrecompiledHeaderIncludeFilename);
						if (PrecompiledHeaderIncludeFile != null)
						{
							// Create a new C++ environment that is used to create the PCH.
							CPPEnvironment ProjectPCHEnvironment = new CPPEnvironment(ProjectCPPEnvironment);
							ProjectPCHEnvironment.PrecompiledHeaderAction = PrecompiledHeaderAction.Create;

							// Create the action to compile the PCH file.
							PCHOutput = ProjectPCHEnvironment.CompileFiles(new FileItem[] { PrecompiledHeaderIncludeFile });
						}
					}
				}

				// Generate unity C++ files if requested.
				if (BuildConfiguration.bUseUnityBuild)
				{
					CPPFiles = Unity.GenerateUnityCPPs(CPPFiles, ProjectCPPEnvironment);
				}

				// Only use precompiled headers for projects with enough files to make the PCH creation worthwhile.
				if (PCHOutput != null &&
					(CPPFiles.Count + CFiles.Count) >= BuildConfiguration.MinFilesUsingPrecompiledHeader)
				{
					ProjectCPPEnvironment.PrecompiledHeaderAction = PrecompiledHeaderAction.Include;
					ProjectCPPEnvironment.PrecompiledHeaderFile = PCHOutput.PrecompiledHeaderFile;

					// Link in the object files produced by creating the precompiled header.
					LinkEnvironment.InputFiles.AddRange(PCHOutput.ObjectFiles);
				}

				// Compile either the C++ source files or the unity C++ files.
 				LinkEnvironment.InputFiles.AddRange(ProjectCPPEnvironment.CompileFiles(CPPFiles).ObjectFiles);

				// Compile C files directly.
				LinkEnvironment.InputFiles.AddRange(ProjectCPPEnvironment.CompileFiles(CFiles).ObjectFiles);

				// Compile RC files.
				LinkEnvironment.InputFiles.AddRange(ProjectCPPEnvironment.CompileRCFiles(RCFiles).ObjectFiles);
			}
		}
	}
}