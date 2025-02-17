/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
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
	}
}
