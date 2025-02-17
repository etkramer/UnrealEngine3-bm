/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Reflection;

namespace UnrealBuildTool
{
	/** The interface of a target; the granularity of building that is exposed to users. */
	interface Target
	{
		IEnumerable<FileItem> Build(string[] Arguments);

		/**
		 * @return Name of target, e.g. name of the game being built.
		 */
		string GetTargetName();
		/**
		 * @return Name of configuration, e.g. "Release"
		 */
		string GetConfigurationName();
		/**
		 * @return Name of platform, e.g. "Win32"
		 */
		string GetPlatformName();
		/**
		 * @return TRUE if debug information is created, FALSE otherwise.
		 */
		bool IsCreatingDebugInfo();

        /**
		* @return List of all projects being built for this Target configuration
		*/
        List<string> GetProjectPaths();
	}
}
