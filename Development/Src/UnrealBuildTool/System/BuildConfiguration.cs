/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
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

		/** Whether PDB files should be used for Visual C++ builds. */
        public static bool bUsePDBFiles = GetEnvironmentVariable("ue3.bUsePDBFiles", false);
 
		/** Whether PCH files should be used. */
        public static bool bUsePCHFiles = false;//GetEnvironmentVariable("ue3.bUsePCHFiles", true);

		/** The minimum number of files that must use a precompiled header before it will be created and used. */
		public static int MinFilesUsingPrecompiledHeader = 4;

		/** Whether debug info should be written to the console. */
        public static bool bPrintDebugInfo = GetEnvironmentVariable("ue3.bPrintDebugInfo", false);

		/** Whether XGE may be used. */
        public static bool bAllowXGE = GetEnvironmentVariable("ue3.bAllowXGE", true);

		/** Whether to display the XGE build monitor. */
        public static bool bShowXGEMonitor = GetEnvironmentVariable("ue3.bShowXGEMonitor", true);

		/** The path to the intermediate folder, relative to Development/Src. */
		public static string BaseIntermediatePath = "../Intermediate";

		/** The path to the targets folder, relative to Development/Src. */
		public static string BaseTargetsPath = "Targets";

        /** Whether or not to delete outdated produced items. */
        public static bool bShouldDeleteAllOutdatedProducedItems = GetEnvironmentVariable("ue3.bShouldDeleteAllOutdatedProducedItems", true);

        /// <summary>
        /// Reads the specified environment variable
        /// </summary>
        /// <param name="VarName">the environment variable to read</param>
        /// <param name="bDefault">the default value to use if missing</param>
        /// <returns>the value of the environment variable if found and the default value if missing</returns>
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
