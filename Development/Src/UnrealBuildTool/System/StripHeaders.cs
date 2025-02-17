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
    /** Project and all of its CPP source file entries */
    class ProjectEntry
    {
        /** Relative path of project file */
        public string Path = null;

        /** All the CPP source files referenced by the project file */
        public List<SourceFileEntry> SourceFiles = new List<SourceFileEntry>();
    }

    /** CPP source file that is being iterated on to remove headers */
    class SourceFileEntry
    {
        /** Last copy of source file that was built */
        public FileItem CPPFile = null;
        
        /** Path to backup copy of last version of the source file that build successfully */
        public string BackupFilePath = null;
        
        /** Last header include line that was removed from the source file */
        public int LastLineRemoved = -1;
        
        /** Whether the last compilation was successful */
        public bool bLastCompileSucceeded = false;
        
        /** Flag header stripping iteration as completed for a given source file */
        public bool bFinishedIterating = false;
    }

    /** Utility to remove unused header files from CPP source files */
    class StripHeadersUtil
    {
        /** Projects and CPP source files to iterate on */
        List<ProjectEntry> Projects = new List<ProjectEntry>();
        
        /** Current number of global iterations */
        int NumIterations = 0;
        
        /** Path to UE3 source directory */
        static string SourceBasePath = "\\UnrealEngine3\\Development\\Src";
        
        /** Path to intermediate directory for creating source file backups */
        static string BackupBasePath = "\\StripHeaders\\Src";

        /** All the headers in this list are ignored when stripping */
        static string[] HeadersToIgnore = 
            {
                "UMemoryDefines.h",
                "DebugDefines.h",
                "UnFaceFXSupport.h",
                "UnNovodexSupport.h",
				"SourceControlIntegration.h"
            };

        /** 
         * Initialize project entries. This is only done before the first iteration.
         * 
         * @param ProjectPaths All the projects that were used when building
         **/
        void Initialize(List<string> ProjectPaths)
        {
            // Init projects and cpp files used
            Projects.Clear();
            foreach (string ProjectPath in ProjectPaths)
            {
                string ProjectDirectory = Path.GetDirectoryName(ProjectPath);
                List<string> ProjectFilePaths = VCProject.GetProjectFiles(ProjectPath);

                // Add a new entry for each project
                ProjectEntry Project = new ProjectEntry();
                Project.Path = ProjectPath;
                Projects.Add(Project);

                // Gather a list of CPP files in the project.
                foreach (string ProjectFilePath in ProjectFilePaths)
                {
                    string Extension = Path.GetExtension(ProjectFilePath).ToUpperInvariant();
                    if (Extension == ".CPP")
                    {
                        FileItem CPPFileItem = FileItem.GetExistingItemByPath(Path.Combine(ProjectDirectory, ProjectFilePath));
                        // make sure the file exists, is writable, and is in the Src path
                        if (CPPFileItem != null &&
                            CPPFileItem.bExists &&
                            CPPFileItem.Info != null &&
                            !CPPFileItem.Info.IsReadOnly &&
                            CPPFileItem.AbsolutePathUpperInvariant.Contains(SourceBasePath.ToUpperInvariant()) )
                        {
                            SourceFileEntry SourceFileEntry = new SourceFileEntry();
                            SourceFileEntry.CPPFile = CPPFileItem;
                            Project.SourceFiles.Add(SourceFileEntry);
                        }
                    }
                }
            }
        }

        /**
         * Find the first action that has the source file as a prerequisite item
         * 
         * @param Actions List of all compile actions used during the last build
         * @param SourceFile CPP file to find action for
         * 
         * @return Action that is used to compile the given source file
         **/
        Action FindCompileAction(List<Action> Actions, SourceFileEntry SourceFile)
        {
            if (SourceFile.CPPFile != null)
            {
                foreach (Action CompileAction in Actions)
                {
                    foreach (FileItem PrerequisiteFile in CompileAction.PrerequisiteItems)
                    {
                        // match based on the prerequisite list of the action
                        if (PrerequisiteFile.bExists &&
                            PrerequisiteFile.AbsolutePath == SourceFile.CPPFile.AbsolutePath)
                        {
                            return CompileAction;
                        }
                    }
                }
            }
            return null;
        }

        /**
         * Iterate over all the executed actions and update the file infos of
         * their produced items.  This is needed in order to update dependency checking
         * to determine outdated object files.
         * 
         * @param ActionsExecuted List of all compile actions used during the last build
         **/
        void UpdateProducedItemInfos(List<Action> ActionsExecuted)
        {
            foreach (Action CompileAction in ActionsExecuted)
            {
                // update file info and write times of all the produced items (obj files)
                for (int Idx = 0; Idx < CompileAction.ProducedItems.Count; Idx++)
                {
                    CompileAction.ProducedItems[Idx] = new FileItem(CompileAction.ProducedItems[Idx].AbsolutePath);
                }
            }
        }

        /**
         * Determine which source files compiled successfully and flag them as such.
         * 
         * @param ActionsExecuted List of all compile actions used during the last build         
         **/
        void UpdateCompileResults(List<Action> ActionsExecuted)
        {
            // Make sure to update file times for obj files generated 
            UpdateProducedItemInfos(ActionsExecuted);

            // Cache results of outdated actions
            Dictionary<Action, bool> OutdatedActionDictionary = new Dictionary<Action, bool>();
            foreach (ProjectEntry Project in Projects)
            {
                // Find the compile action for each source file and determine if it updated successfully
                foreach (SourceFileEntry SourceFile in Project.SourceFiles)
                {
                    Action CompileAction = FindCompileAction(ActionsExecuted, SourceFile);
                    if( CompileAction != null )
                    {
                        bool bIsOutdated = UnrealBuildTool.IsActionOutdated(CompileAction, ref OutdatedActionDictionary);
                        SourceFile.bLastCompileSucceeded = !bIsOutdated;
                    }
                    else
                    {
                        // Missing actions treated as successful since a compile was not needed for it
                        SourceFile.bLastCompileSucceeded = true;
                    }
                }
            }
        }

        /**
         * Iterate over all source files and determine if all compiles succeeded
         * 
         * @return true if all compiles succeeded
         **/
        bool AllCompilesSucceeded()
        {
            foreach (ProjectEntry Project in Projects)
            {
                foreach (SourceFileEntry SourceFile in Project.SourceFiles)
                {
                    if (!SourceFile.bLastCompileSucceeded)
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        /**
         * @return full path to backup file for the given source path
         **/
        string ConvertSourceToBackupPath(string AbsoluteSourcePath)
        {
            int BaseIdx = AbsoluteSourcePath.ToUpperInvariant().IndexOf(SourceBasePath.ToUpperInvariant());
            if( BaseIdx != -1 )
            {
                string SrcRelativePath = AbsoluteSourcePath.Substring(BaseIdx + SourceBasePath.Length);
                return Path.GetFullPath(BuildConfiguration.BaseIntermediatePath) + BackupBasePath + SrcRelativePath;
            }
            else
            {
                return null;
            }
        }

        /**
         * Copy all files that succeeded compile so that we can restore 
         * source files using these backups.  If a file did not succeed
         * its last compile then it is not backed up.  This guarantees that
         * all backed up source files compile.
         * */
        void UpdateSourceBackups()
        {
            foreach (ProjectEntry Project in Projects)
            {
                foreach (SourceFileEntry SourceFile in Project.SourceFiles)
                {
                    // Only backup a file if it compiled, and also skip if we are done iterating on the source file
                    if (SourceFile.CPPFile != null &&
                        SourceFile.CPPFile.Info != null &&
                        SourceFile.bLastCompileSucceeded &&
                        !SourceFile.bFinishedIterating )
                    {
                        SourceFile.BackupFilePath = ConvertSourceToBackupPath(SourceFile.CPPFile.AbsolutePath);
                        if (SourceFile.BackupFilePath != null)
                        {
                            // Copy file to backup folder
                            if (!Directory.Exists(Path.GetDirectoryName(SourceFile.BackupFilePath)))
                            {
                                Directory.CreateDirectory(Path.GetDirectoryName(SourceFile.BackupFilePath));
                            }
                            SourceFile.CPPFile.Info.CopyTo(SourceFile.BackupFilePath, true);
                        }
                    }
                }
            }
        }

        /**
         * Try to restore all original source files from their backups if they exist
         **/
        void RestoreAllFromBackups()
        {
            foreach (ProjectEntry Project in Projects)
            {
                foreach (SourceFileEntry SourceFile in Project.SourceFiles)
                {
                    if (SourceFile.CPPFile != null)
                    {
                        string BackupFilePath = ConvertSourceToBackupPath(SourceFile.CPPFile.AbsolutePath);
                        if (BackupFilePath != null)
                        {
                            FileInfo BackupFile = new FileInfo(BackupFilePath);
                            if (BackupFile.Exists)
                            {
                                BackupFile.CopyTo(SourceFile.CPPFile.AbsolutePath, true);
                            }
                        }
                    }
                }
            }       
        }

        /** 
         * Determine if a header should be skipped when stripping source files.
         * 
         * @param HeaderString String containing the header name
         * @return true if the given header should be ignored from stripping
         **/
        bool IsHeaderIgnored(string HeaderString)
        {
            foreach (string Ignore in HeadersToIgnore)
            {
                if( HeaderString.Contains(Ignore) )
                {
                    return true;
                }
            }
            return false;
        }

        /**
         * Determine if the header line is a pch header 
         * 
         * @param HeaderString String containing the header name
         * @SourceFile CPP source file being processed
         * @return true if the header line is used for pch
         **/
        bool IsPCHHeader(string HeaderString,SourceFileEntry SourceFile)
        {
            if( SourceFile.CPPFile != null && 
                SourceFile.CPPFile.PrecompiledHeaderIncludeFilename.Length > 2 )
            {
                return HeaderString.Contains(SourceFile.CPPFile.PrecompiledHeaderIncludeFilename);
            }
            return false;
        }

        /**
         * Try to remove the next header include line from the source file.
         * Header lines are removed bottom-up to not invalidate header dependencies.
         * 
         * @param SourceFile CPP file to process
         * @return true if we were able to remove a header line from the source file 
         **/
        bool RemoveNextHeaderLine(SourceFileEntry SourceFile)
        {
            bool bSuccess = false;
            // If the last line removed from the file is 0 then we can't iterate on it anymore
            if (SourceFile.LastLineRemoved != 0)
            {
                int LineToRemove = 0;
                string[] SourceLines = File.ReadAllLines(SourceFile.CPPFile.AbsolutePath);
                if (SourceFile.LastLineRemoved == -1)
                {
                    // -1 represents the first iteration to initialize to the end of the source file
                    LineToRemove = SourceLines.Length - 1;
                }
                else
                {
                    // Move up from the last line we removed (or tried to remove)
                    LineToRemove = SourceFile.LastLineRemoved - 1;
                }

                // Move up the file until we get to the next include line
                while (LineToRemove >= 0 && LineToRemove < SourceLines.Length)
                {
                    string SourceLine = SourceLines[LineToRemove];
                    if (SourceLine.TrimStart().StartsWith("#include") &&
                        !IsHeaderIgnored(SourceLine) )
                    {
                        break;
                    }
                    LineToRemove--;
                }

                // Update the source file by writing all but the single removed line.
                // And also keep track of what was removed.
                if (LineToRemove >= 0 && 
                    LineToRemove != SourceFile.LastLineRemoved)
                {
                    StreamWriter CPPFileStream = new StreamWriter(SourceFile.CPPFile.AbsolutePath,false);
                    for (int LineIdx = 0; LineIdx < SourceLines.Length; LineIdx++)
                    {
                        if (LineIdx != LineToRemove)
                        {
                            CPPFileStream.WriteLine(SourceLines[LineIdx]);
                        }
                    }
                    CPPFileStream.Close();
                    SourceFile.LastLineRemoved = LineToRemove;
                    bSuccess = true;
                }
            }
            return bSuccess;
        }

        /**
         * Iterate over all CPP source files and process their header removal.
         * If a source file failed its last iteration then backup from the last successful compile.
         * If it succeeded its last iteration then try removing another header line.
         * 
         * @return true if there is anything left to iterate on and another build is required
         **/
        bool RemoveHeaders()
        {
            bool bContinue = false;
            foreach (ProjectEntry Project in Projects)
            {
                foreach (SourceFileEntry SourceFile in Project.SourceFiles)
                {
                    // Missing entry, finish iterating on it
                    if( SourceFile.CPPFile == null )
                    {
                        SourceFile.bFinishedIterating = true;
                    }

                    // Only iterate on the source files that can still have headers removed
                    if (!SourceFile.bFinishedIterating)
                    {
                        // If the compile failed then we need to restore the backup file before iterating on headers
                        // The backup version is guaranteed to compile since we only save off the last version that was successful
                        if (!SourceFile.bLastCompileSucceeded)
                        {
                            bool bRestoreSucceeded = false;
                            if( SourceFile.BackupFilePath != null )
                            {
                                FileInfo BackupFile = new FileInfo(SourceFile.BackupFilePath);
                                if (BackupFile.Exists)
                                {
                                    BackupFile.CopyTo(SourceFile.CPPFile.AbsolutePath, true);
                                    bRestoreSucceeded = true;
                                }
                            }
                            if (!bRestoreSucceeded)
                            {
                                Console.WriteLine("Strip headers: missing backup for source file {0}", SourceFile.CPPFile.AbsolutePath);
                                SourceFile.bFinishedIterating = true;
                            }
                        }
                        // Try removing the next header line from the source file. 
                        // If we can't remove any more then stop iterating on the source file
                        if( !RemoveNextHeaderLine(SourceFile) )
                        {
                            SourceFile.bFinishedIterating = true;
                        }
                    }

                    // If any source file still needs a pass then need to continue iterating on bulds
                    if (!SourceFile.bFinishedIterating)
                    {
                        bContinue = true;
                    }
                }
            }
            return bContinue;
        }

        /**
         * Iterate over the project cpp files and determine what compiled successfully.
         * Then try to strip some more headers and see if another build pass is required.
         * 
         * @param AxtionsExecuted List of all compile actions used during the last build         
         * @param Target Current target used during the last build
         * @return true if header stripping requires another build iteration
         **/
        public bool ProcessNext(List<Action> ActionsExecuted, Target Target)
        {
            // Initialize projects and their cpp file entries before first iteration
            if (NumIterations == 0)
            {
                Initialize(Target.GetProjectPaths());
            }

            // flag cpp files that succeeded/failed compiling from last iteration
            UpdateCompileResults(ActionsExecuted); 

            // Make sure that the source files actually compiled before the first iteration
            if (NumIterations == 0 && 
                !AllCompilesSucceeded() )
            {
                Console.WriteLine("Strip headers: either the projects doesn't compile or the strip headers process was cancelled.");
                Console.WriteLine("Strip headers: attempting to restore source files from last good backup!");
                RestoreAllFromBackups();
                return false;
            }

            // Generate backup files for CPPs that compiled successfully
            UpdateSourceBackups();

            // Remove next set of headers from CPPs
            if( !RemoveHeaders() )
            {
                Console.WriteLine("Strip headers: finished build iteration");
                return false;
            }

            // Keep track of total iterations
            NumIterations++;
            Console.WriteLine("Strip headers: build iteration #{0} starting...", NumIterations);

            return true;
        }
    }
}

