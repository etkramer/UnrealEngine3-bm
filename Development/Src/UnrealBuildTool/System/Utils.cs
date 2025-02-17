/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
 */
using System;
using System.Collections.Generic;
using System.IO;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace UnrealBuildTool
{
    class PrecompileHeaderEnvironment
    {
        /** Compile environment with pch usage */
        public CPPEnvironment CompileEnvPCH = null;
        /** List of source files with the same pch usage */
        public List<FileItem> SourceFiles = new List<FileItem>();

        /**
         * Creates a precompiled header action to generate a new pch file
         * 
         * @param PrecompiledHeaderIncludeFilename - Name of the header used for pch.
         * @param ProjectCPPEnvironment - The environment the C/C++ files in the project are compiled with.
         * @return the compilation output result of the created pch.
         */
        public static CPPOutput GeneratePCHCreationAction(string PrecompiledHeaderIncludeFilename, CPPEnvironment ProjectCPPEnvironment)
        {
            CPPOutput PCHOutput = null;
            // Find the header file to be precompiled. Don't skip external headers
            FileItem PrecompiledHeaderIncludeFile = ProjectCPPEnvironment.FindIncludedFile(PrecompiledHeaderIncludeFilename, false);
            if (PrecompiledHeaderIncludeFile != null)
            {
                // Create a new C++ environment that is used to create the PCH.
                CPPEnvironment ProjectPCHEnvironment = new CPPEnvironment(ProjectCPPEnvironment);
                ProjectPCHEnvironment.PrecompiledHeaderAction = PrecompiledHeaderAction.Create;
                ProjectPCHEnvironment.PrecompiledHeaderIncludeFilename = PrecompiledHeaderIncludeFilename;
                // Create the action to compile the PCH file.
                PCHOutput = ProjectPCHEnvironment.CompileFiles(new FileItem[] { PrecompiledHeaderIncludeFile });
            }
            else
            {
                Console.WriteLine("Couldn't find PCH file \"{0}\".",PrecompiledHeaderIncludeFilename);
            }
            return PCHOutput;
        }
    }

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
		 * @param Projects - A list of projects with paths to Visual C++ .vcproj files containing the files to compile.
		 */
		public static void CompileProjects(
			CPPEnvironment CompileEnvironment,
			LinkEnvironment LinkEnvironment,
			IEnumerable<UE3ProjectDesc> Projects
			)
		{
			foreach (UE3ProjectDesc ProjectDesc in Projects)
			{
				CPPEnvironment ProjectCPPEnvironment = new CPPEnvironment(CompileEnvironment);
				ProjectCPPEnvironment.OutputDirectory = Path.Combine(
					CompileEnvironment.OutputDirectory,
					Path.GetFileNameWithoutExtension(ProjectDesc.ProjectPath)
					);
			
				// Update project environment for the project CLR mode
				ProjectCPPEnvironment.CLRMode = ProjectDesc.CLRMode;
			
				// Allow files to be included from the project's source directory.
				ProjectCPPEnvironment.IncludePaths.Add( Path.Combine( Path.GetDirectoryName( ProjectDesc.ProjectPath ), "Src" ) );

				// If the project file doesn't exist, display a friendly error.
				if( !File.Exists( ProjectDesc.ProjectPath ) )
				{
					throw new BuildException( "Project file \"{0}\" doesn't exist.", ProjectDesc.ProjectPath );
				}

				// Load the list of files from the specified project file.
				string ProjectDirectory = Path.GetDirectoryName( ProjectDesc.ProjectPath );
				List<string> ProjectFilePaths = VCProject.GetProjectFiles( ProjectDesc.ProjectPath );

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

                // Map from pch header string to its compilation environment.
                Dictionary<string, PrecompileHeaderEnvironment> UsageMapPCH = new Dictionary<string, PrecompileHeaderEnvironment>();

                // Clear out any references to precompiled header usage for source files..
                foreach (FileItem CPPFile in CPPFiles)
                {
                    CPPFile.PrecompiledHeaderIncludeFilename = null;
                }

                // Determine what potential precompiled header is used by each source file.
                if (BuildConfiguration.bUsePCHFiles || BuildConfiguration.bRemoveUnusedHeaders)
				{
                    foreach (FileItem CPPFile in CPPFiles)
                    {
                        // Find headers used by the source file.
                        List<string> DirectIncludeFilenames = ProjectCPPEnvironment.GetDirectIncludeDependencies(CPPFile);
                        if (DirectIncludeFilenames.Count > 0)
                        {
                            // The pch header should always be the first line in the source file.
                            CPPFile.PrecompiledHeaderIncludeFilename = DirectIncludeFilenames[0];
                            // Create a new entry if not in the pch usage map
                            PrecompileHeaderEnvironment PCHEnvironment = null;
                            if (!UsageMapPCH.TryGetValue(CPPFile.PrecompiledHeaderIncludeFilename, out PCHEnvironment))
                            {
                                PCHEnvironment = new PrecompileHeaderEnvironment();
                                UsageMapPCH.Add(CPPFile.PrecompiledHeaderIncludeFilename, PCHEnvironment);
                            }
                            PCHEnvironment.SourceFiles.Add(CPPFile);
                        }
                    }
                }

				if (BuildConfiguration.bUsePCHFiles) 
                {
                    // List of source files that won't use pch
                    List<FileItem> NonPCHCPPFiles = new List<FileItem>();

                    foreach (FileItem CPPFile in CPPFiles)
                    {
                        bool bUsedPCH = false;
                        PrecompileHeaderEnvironment PCHEnvironment = null;
                        UsageMapPCH.TryGetValue(CPPFile.PrecompiledHeaderIncludeFilename, out PCHEnvironment);

                        // If there was one header that was included first by enough C++ files, use it as the precompiled header.
                        // Only use precompiled headers for projects with enough files to make the PCH creation worthwhile.
                        if (PCHEnvironment != null &&
                            PCHEnvironment.SourceFiles.Count >= BuildConfiguration.MinFilesUsingPrecompiledHeader)
                        {
                            // Create the pch compile environment if one doesn't already exist
                            if (PCHEnvironment.CompileEnvPCH == null)
                            {
                                // Create a new C++ environment that uses the PCH.
                                PCHEnvironment.CompileEnvPCH = new CPPEnvironment(ProjectCPPEnvironment);
                                PCHEnvironment.CompileEnvPCH.PrecompiledHeaderAction = PrecompiledHeaderAction.Include;
                                // Set the environment's precompiled header include filename.
                                PCHEnvironment.CompileEnvPCH.PrecompiledHeaderIncludeFilename = CPPFile.PrecompiledHeaderIncludeFilename;

                                if (BuildConfiguration.bPrintDebugInfo)
                                {
                                    Console.WriteLine("PCH file \"{0}\" generated for project \"{1}\" .",
                                        CPPFile.PrecompiledHeaderIncludeFilename,
                                        ProjectDesc.ProjectPath);
                                }
                                bUsedPCH = true;
                            }
                        }
                        else
                        {
                            // compile the source file without a precompiled header
                            NonPCHCPPFiles.Add(CPPFile);
                        }

                        if (BuildConfiguration.bPrintDebugInfo) 
                        {
                            if (!bUsedPCH)
                            {
                                Console.WriteLine("No PCH usage for file \"{0}\" .", CPPFile.AbsolutePath);
                            }
                        }
                    }

                    // Compile the C++ source or the unity C++ files that use a PCH environment.
                    foreach (KeyValuePair<string, PrecompileHeaderEnvironment> PCHUsage in UsageMapPCH)
                    {
                        string PrecompiledHeaderFileName = PCHUsage.Key;
                        PrecompileHeaderEnvironment PCHEnvironment = PCHUsage.Value;
                        if (PCHEnvironment != null &&
                            PCHEnvironment.CompileEnvPCH != null)
                        {
                            List<FileItem> PCHCPPFiles = PCHEnvironment.SourceFiles;
                            if (BuildConfiguration.bUseUnityBuild)
                            {
                                // unity files generated for only the set of files which share the same PCH environment
                                PCHCPPFiles = Unity.GenerateUnityCPPs(PCHCPPFiles, PCHEnvironment.CompileEnvPCH);
                            }

                            // Check if there are enough unity files to warrant pch generation
                            if (PCHCPPFiles.Count >= BuildConfiguration.MinFilesUsingPrecompiledHeader &&
                                PCHEnvironment.CompileEnvPCH.PrecompiledHeaderFile == null)
                            {
                                CPPOutput PCHOutput = PrecompileHeaderEnvironment.GeneratePCHCreationAction(PrecompiledHeaderFileName,ProjectCPPEnvironment);
                                if (PCHOutput != null)
                                {
                                    PCHEnvironment.CompileEnvPCH.PrecompiledHeaderFile = PCHOutput.PrecompiledHeaderFile;
                                    // Link in the object files produced by creating the precompiled header.
                                    LinkEnvironment.InputFiles.AddRange(PCHOutput.ObjectFiles);
                                }
                            }

                            if (PCHEnvironment.CompileEnvPCH.PrecompiledHeaderFile != null)
                            {
                                // if pch action was generated for the environment then use pch
                                LinkEnvironment.InputFiles.AddRange(PCHEnvironment.CompileEnvPCH.CompileFiles(PCHCPPFiles).ObjectFiles);
                            }
                            else
                            {
                                // otherwise, compile non-pch
                                LinkEnvironment.InputFiles.AddRange(ProjectCPPEnvironment.CompileFiles(PCHCPPFiles).ObjectFiles);                                
                            }
                        }
                    }

                    // Compile the C++ source or the unity C++ files that don't use PCH.
                    if (NonPCHCPPFiles.Count > 0)
                    {
                        if (BuildConfiguration.bUseUnityBuild)
                        {
                            NonPCHCPPFiles = Unity.GenerateUnityCPPs(NonPCHCPPFiles, ProjectCPPEnvironment);
                        }
                        LinkEnvironment.InputFiles.AddRange(ProjectCPPEnvironment.CompileFiles(NonPCHCPPFiles).ObjectFiles);
                    }
                }
                // no pch usage
                else
                {
                    // Generate unity C++ files if requested.
                    if (BuildConfiguration.bUseUnityBuild)
                    {
                        CPPFiles = Unity.GenerateUnityCPPs(CPPFiles, ProjectCPPEnvironment);
                    }

                    // Compile either the C++ source files or the unity C++ files.
                    LinkEnvironment.InputFiles.AddRange(ProjectCPPEnvironment.CompileFiles(CPPFiles).ObjectFiles);
                }
				
				// Compile C files directly.
				LinkEnvironment.InputFiles.AddRange(ProjectCPPEnvironment.CompileFiles(CFiles).ObjectFiles);

				// Compile RC files.
				LinkEnvironment.InputFiles.AddRange(ProjectCPPEnvironment.CompileRCFiles(RCFiles).ObjectFiles);
			}
		}
		
		/** Regular expression to match $(ENV) environment variables. */
		static Regex EnvironmentVariableRegex = new Regex( "\\$\\(.*?\\)", RegexOptions.None );

		/**
		 * Resolves $(ENV) to the value of the environment variable in the passed in string.
		 * 
		 * @param	InString	String to resolve environment variable in.
		 * @return	String with environment variable expanded/ resolved.
		 */
		public static string ResolveEnvironmentVariable( string InString )
		{
			string Result = InString;

			// Try to find $(ENV) substring.
			Match M = EnvironmentVariableRegex.Match( InString );
			// If found, get rid of $() before looking it up and replace match with resolved environment variable.
			if( M.Success )
			{
				// Look up environment variable and replace it.
				Result = Result.Replace( M.ToString(), Environment.GetEnvironmentVariable( M.ToString().Substring(2,M.ToString().Length-3)));
			}

			return Result;
		}

		/**
		 * Thie is a faster replacement of File.ReadAllText. Code snippet based on code 
		 * and analysis by Sam Allen
		 * 
		 * http://dotnetperls.com/Content/File-Handling.aspx
		 * 
		 * @param	SourceFile		Source file to fully read and convert to string
		 * @return	Textual representation of file.
		 */
		public static string ReadAllText( string SourceFile )
		{
			using( StreamReader Reader = new StreamReader( SourceFile, System.Text.Encoding.UTF8 ) )
			{
				return Reader.ReadToEnd();
			}
		}
	}
}