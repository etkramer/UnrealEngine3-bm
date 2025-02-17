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
	/** The platforms that may be compilation targets for C++ files. */
	enum CPPTargetPlatform
	{
		Win32,
		Xbox360
	}

	/** The optimization level that may be compilation targets for C++ files. */
	enum CPPTargetConfiguration
	{
		Debug,
		Release,
		ReleaseLTCG
	}

	/** The possible interactions between a precompiled header and a C++ file being compiled. */
	enum PrecompiledHeaderAction
	{
		None,
		Include,
		Create
	}

	/** Encapsulates the compilation output of compiling a set of C++ files. */
	class CPPOutput
	{
		public List<FileItem> ObjectFiles = new List<FileItem>();
		public List<FileItem> DebugDataFiles = new List<FileItem>();
		public FileItem PrecompiledHeaderFile = null;
	}

	/** Encapsulates the environment that a C++ file is compiled in. */
	partial class CPPEnvironment
	{
		/** The directory to put the output object/debug files in. */
		public string OutputDirectory = null;

		/** The file containing the precompiled header data. */
		public FileItem PrecompiledHeaderFile = null;

		/** The name of the header file which is precompiled. */
		public string PrecompiledHeaderIncludeFilename = null;

		/** Whether the compilation should create, use, or do nothing with the precompiled header. */
		public PrecompiledHeaderAction PrecompiledHeaderAction = PrecompiledHeaderAction.None;

		/** The platform to be compiled for. */
		public CPPTargetPlatform TargetPlatform;

		/** The configuration to be compiled for. */
		public CPPTargetConfiguration TargetConfiguration;

		/** True if debug info should be created. */
		public bool bCreateDebugInfo = true;

		/** The include paths to look for included files in. */
		public List<string> IncludePaths = new List<string>();

		/**
		 * The include paths where changes to contained files won't cause dependent C++ source files to
		 * be recompiled, unless BuildConfiguration.bCheckSystemHeadersForModification==TRUE.
		 */
		public List<string> SystemIncludePaths = new List<string>();

		/** The C++ preprocessor definitions to use. */
		public List<string> Definitions = new List<string>();

		/** Additional arguments to pass to the compiler. */
		public string AdditionalArguments = "";

		/** Default constructor. */
		public CPPEnvironment()
		{}

		/** Copy constructor. */
		public CPPEnvironment(CPPEnvironment InCopyEnvironment)
		{
			OutputDirectory = InCopyEnvironment.OutputDirectory;
			PrecompiledHeaderFile = InCopyEnvironment.PrecompiledHeaderFile;
			PrecompiledHeaderIncludeFilename = InCopyEnvironment.PrecompiledHeaderIncludeFilename;
			PrecompiledHeaderAction = InCopyEnvironment.PrecompiledHeaderAction;
			TargetPlatform = InCopyEnvironment.TargetPlatform;
			TargetConfiguration = InCopyEnvironment.TargetConfiguration;
			bCreateDebugInfo = InCopyEnvironment.bCreateDebugInfo;
			IncludePaths.AddRange(InCopyEnvironment.IncludePaths);
			SystemIncludePaths.AddRange(InCopyEnvironment.SystemIncludePaths);
			Definitions.AddRange(InCopyEnvironment.Definitions);
			AdditionalArguments = InCopyEnvironment.AdditionalArguments;
		}

		/**
		 * Creates actions to compile a set of C++ source files.
		 * @param CPPFiles - The C++ source files to compile.
		 * @return The object files produced by the actions.
		 */
		public CPPOutput CompileFiles(IEnumerable<FileItem> CPPFiles)
		{
			if (TargetPlatform == CPPTargetPlatform.Win32 || TargetPlatform == CPPTargetPlatform.Xbox360)
			{
				return VCToolChain.CompileCPPFiles(this, CPPFiles);
			}
			else
			{
				Debug.Fail("Unrecognized C++ target platform.");
				return new CPPOutput();
			}
		}

		/**
		 * Creates actions to compile a set of Windows resource script files.
		 * @param RCFiles - The resource script files to compile.
		 * @return The compiled resource (.res) files produced by the actions.
		 */
		public CPPOutput CompileRCFiles(IEnumerable<FileItem> RCFiles)
		{
			if (TargetPlatform == CPPTargetPlatform.Win32)
			{
				return VCToolChain.CompileRCFiles(this, RCFiles);
			}
			else
			{
				return new CPPOutput();
			}
		}
	};
}
