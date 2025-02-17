/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;

namespace UnrealBuildTool
{
	/** Encapsulates the environment that is used to link object files. */
	class LinkEnvironment
	{
		/** The directory to put the non-executable files in (PDBs, import library, etc) */
		public string OutputDirectory;

		/** The file path for the executable file that is output by the linker. */
		public string OutputFilePath;

		/** The platform that is being linked for. */
		public CPPTargetPlatform TargetPlatform;

		/** The configuration that is being linked for. */
		public CPPTargetConfiguration TargetConfiguration;

		/** A list of the paths used to find libraries. */
		public List<string> LibraryPaths = new List<string>();

		/** A list of libraries to exclude from linking. */
		public List<string> ExcludedLibraries = new List<string>();

		/** A list of additional libraries to link in. */
		public List<string> AdditionalLibraries = new List<string>();

		/**
		 * A list of the dynamically linked libraries that shouldn't be loaded until they are first called
		 * into.
		 */
		public List<string> DelayLoadDLLs = new List<string>();

		/** A list of the object files to be linked. */
		public List<FileItem> InputFiles = new List<FileItem>();

		/** Additional arguments to pass to the linker. */
		public string AdditionalArguments = "";

		/** Whether the produced binary is going to ship to consumers. */
		public bool bIsShippingBinary = false;

		/** Default constructor. */
		public LinkEnvironment()
		{
		}

		/** Copy constructor. */
		public LinkEnvironment(LinkEnvironment InCopyEnvironment)
		{
			OutputDirectory = InCopyEnvironment.OutputDirectory;
			OutputFilePath = InCopyEnvironment.OutputFilePath;
			TargetPlatform = InCopyEnvironment.TargetPlatform;
			TargetConfiguration = InCopyEnvironment.TargetConfiguration;
			LibraryPaths.AddRange(InCopyEnvironment.LibraryPaths);
			ExcludedLibraries.AddRange(InCopyEnvironment.ExcludedLibraries);
			AdditionalLibraries.AddRange(InCopyEnvironment.AdditionalLibraries);
			DelayLoadDLLs.AddRange(InCopyEnvironment.DelayLoadDLLs);
			InputFiles.AddRange(InCopyEnvironment.InputFiles);
			AdditionalArguments = InCopyEnvironment.AdditionalArguments;
			bIsShippingBinary = InCopyEnvironment.bIsShippingBinary;
		}

		/** Links the input files into an executable. */
		public FileItem LinkExecutable()
		{
			if (TargetPlatform == CPPTargetPlatform.Win32)
			{
                if (BuildConfiguration.bUseIntelCompiler)
                {
                    return IntelToolChain.LinkFiles(this);
                }
                else
                {
                    return VCToolChain.LinkFiles(this);
                }
			}
            else if (TargetPlatform == CPPTargetPlatform.Xbox360)
            {
                return VCToolChain.LinkFiles(this);
            }
            else if (TargetPlatform == CPPTargetPlatform.PS3_PPU)
            {
                return PS3ToolChain.LinkFiles(this);
            }
            else
            {
                return null;
            }
		}
	}
}
