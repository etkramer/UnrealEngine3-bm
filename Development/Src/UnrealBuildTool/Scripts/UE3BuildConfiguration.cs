/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.IO;

namespace UnrealBuildTool
{
	enum PS3GCMType
	{
		Release,
		Debug,
		HUD
	}

	class UE3BuildConfiguration
	{
		/** Whether to compile with GCM HUD support on PS3. */
		public static PS3GCMType PS3GCMType = PS3GCMType.Release;

		/** Whether to deploy the executable after compilation on platforms that require deployment. */
		public static bool bDeployAfterCompile = false;

		/** Whether to globally disable debug info generation; see DebugInfoHeuristics.cs for per-config and per-platform options. */
		public static bool bDisableDebugInfo = false;
    }
}
