/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.IO;

namespace UnrealBuildTool
{
	class BuildConfiguration
	{
		/** Whether to unify C++ code into larger files for faster compilation. */
		public static bool bUseUnityBuild = GetEnvironmentVariable("ue3.bUseUnityBuild", true);

		/** An approximate number of bytes of C++ code to target for inclusion in a single unified C++ file. */
		public static int NumIncludedBytesPerUnityCPP = 256 * 1024;

		/** Whether to stress test the C++ unity build robustness by including all C++ files files in a project from a single unified file. */
		public static bool bStressTestUnity = GetEnvironmentVariable("ue3.bStressTestUnity", false);

		/** Whether headers in system paths should be checked for modification when determining outdated actions. */
		public static bool bCheckSystemHeadersForModification = GetEnvironmentVariable("ue3.bCheckSystemHeadersForModification", false);

		/** Whether headers in the Development\Extrenal folder should be checked for modification when determining outdated actions. */
		public static bool bCheckExternalHeadersForModification = GetEnvironmentVariable("ue3.bCheckExternalHeadersForModification", false);

		/** Whether PDB files should be used for Visual C++ builds. */
		public static bool bUsePDBFiles = GetEnvironmentVariable("ue3.bUsePDBFiles", false);
 
		/** Whether PCH files should be used. */
		public static bool bUsePCHFiles = GetEnvironmentVariable("ue3.bUsePCHFiles", true);

		/** The minimum number of files that must use a precompiled header before it will be created and used. */
		public static int MinFilesUsingPrecompiledHeader = 4;

		/** Whether debug info should be written to the console. */
		public static bool bPrintDebugInfo = GetEnvironmentVariable("ue3.bPrintDebugInfo", false);

		/** Whether to log detailed action stats. This forces local execution. */
		public static bool bLogDetailedActionStats = GetEnvironmentVariable("ue3.bLogDetailedActionStats", false);

		/** Whether XGE may be used. */
		public static bool bAllowXGE = GetEnvironmentVariable("ue3.bAllowXGE", true);

		/** Whether to display the XGE build monitor. */
		public static bool bShowXGEMonitor = GetEnvironmentVariable("ue3.bShowXGEMonitor", true);

		/** Whether or not to delete outdated produced items. */
		public static bool bShouldDeleteAllOutdatedProducedItems = GetEnvironmentVariable("ue3.bShouldDeleteAllOutdatedProducedItems", false);

		/** Whether to use incremental linking or not. */
		public static bool bUseIncrementalLinking = GetEnvironmentVariable("ue3.bUseIncrementalLinking", false);
		
		/** Whether to support edit and continue. */
		public static bool bSupportEditAndContinue = GetEnvironmentVariable("ue3.bSupportEditAndContinue", false);

		/** Processor count multiplier for local execution. Can be below 1 to reserve CPU for other tasks. */
		public static double ProcessorCountMultiplier = 1.5;

		/** The path to the intermediate folder, relative to Development/Src. */
		public static string BaseIntermediatePath = "../Intermediate";

		/** The path to the targets folder, relative to Development/Src. */
		public static string BaseTargetsPath = "Targets";

		/** Name of performance database to talk to, clear out if you don't have one running */
		public static string PerfDatabaseName = "";

        /** Whether to use the Intel compiler for the Win32 build */
        public static bool bUseIntelCompiler = GetEnvironmentVariable("ue3.bUseIntelCompiler", false);

        /** If true then the compile output/errors are suppressed */
        public static bool bSilentCompileOutput = false;

		/** If true then we will iterate the build until each CPP source file includes only required headers */
        public static bool bRemoveUnusedHeaders = false;

		/** True if managed code (and all related functionality) will be enabled */
		public static bool bAllowManagedCode = false;

		/**
		 * Validates the configuration. E.g. some options are mutually exclusive whereof some imply others. Also
		 * some functionality is not available on all platforms.
		 * 
		 * @param	Configuration	Current configuration (e.g. release, debug, ...)
		 * @param	Platform		Current platform (e.g. Win32, PS3, ...)
		 * 
		 * @warning: the order of validation is important
		 */
		public static void ValidateConfiguration( CPPTargetConfiguration Configuration, CPPTargetPlatform Platform )
		{
			// E&C support.
			if( bSupportEditAndContinue )
			{
				// Only supported on PC in debug
				if( Platform == CPPTargetPlatform.Win32	&& Configuration == CPPTargetConfiguration.Debug )
				{
					// Relies on incremental linking.
					bUseIncrementalLinking = true;
				}
				// Disable.
				else
				{
					bUseIncrementalLinking = false;
				}
			}
			// Incremental linking.
			if( bUseIncrementalLinking )
			{
				// Only suported on PC.
				if( Platform == CPPTargetPlatform.Win32 )
				{
					bUsePDBFiles = true;
				}
				// Disable.
				else
				{
					bUseIncrementalLinking = false;
				}
			}
			// Detailed stats
			if( bLogDetailedActionStats )
			{
				// Force local execution as we only have stats for local actions.
				bAllowXGE = false;
			}
			// PDB
			if( bUsePDBFiles )
			{
				// Only supported on PC due to presumed mspdbsrv concurrency issue. We're not seeing this
				// causing issues with PC builds.
				//
				// http://social.msdn.microsoft.com/forums/en-US/vcgeneral/thread/a722c6b6-53ab-41a4-b6c1-5cb02b2623d4
				if( Platform == CPPTargetPlatform.Win32 )
				{
					// Force local execution as we have one PDB for all files using the same PCH.
					bAllowXGE = false;
				}
				else
				{
					bUsePDBFiles = false;
				}
			}
            // Intel compiler
            if(bUseIntelCompiler)
            {
				
                // Make sure ICPP_COMPILER10 environment variable exists.
                string Value = Environment.GetEnvironmentVariable("ICPP_COMPILER10");
                if (Value == null)
                {
                    Console.WriteLine("ICPP_COMPILER10 environment variable doesnt exist. Reverting to VC compiler.");
                    bUseIntelCompiler = false;           
                }
				// Intel compiler is only supported on Win 32.
				else if( Platform == CPPTargetPlatform.Win32 )
				{
					// Force local execution as we don't support distributing Intel compiler yet.
					bAllowXGE = false;
					// Disable use of precompiled headers as ICL massages output filenames.
					bUsePCHFiles = false;
				}
				// Non- Win32 platform, disable Intel compiler.
				else
				{
					bUseIntelCompiler = false;
				}
            }
			// PS3
			if( Platform == CPPTargetPlatform.PS3_PPU )
			{
				// Disable PCH if XGE is enabled to work around issue. We're not detecting presence of XGE
				// at this point so you need to disable XGE manually if you want to use PCH on PS3.
				if( bAllowXGE )
				{
					bUsePCHFiles = false;
				}
			}
            // Strip headers
            if (bRemoveUnusedHeaders)
            {
				// Disable unifying of CPPs and also any PCH usage
                bUseUnityBuild = false;
				bUsePCHFiles = false;
                // Don't skew build perf stats with header stripping 
                PerfDatabaseName = "";
                // Don't care about errors from intermediate compile iterations
                bSilentCompileOutput = true;
                bShowXGEMonitor = false;
            }                
                
		}


		/**
		 * Reads the specified environment variable
		 *
		 * @param	VarName		the environment variable to read
		 * @param	bDefault	the default value to use if missing
		 * @return	the value of the environment variable if found and the default value if missing
		 */
		private static bool GetEnvironmentVariable(string VarName, bool bDefault)
		{
			string Value = Environment.GetEnvironmentVariable(VarName);
			if (Value != null)
			{
				// Convert the string to its boolean value
				return Convert.ToBoolean(Value);
			}
			return bDefault;
		}
	}
}
