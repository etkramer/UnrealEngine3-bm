/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace UnrealBuildTool
{
	class Xbox360ToolChain
	{
		/** Checks that the Xbox 360 SDK is installed, and if so returns the path to its binaries directory. */
		public static string GetBinDirectory()
		{
			string MoreInfoString =
				"See https://udn.epicgames.com/Three/GettingStartedPS3 for help setting up the UE3 PS3 compilation environment";

			// Read the root directory of the PS3 SDK from the environment.
			string XEDKEnvironmentVariable = Environment.GetEnvironmentVariable("XEDK");

			// Check that the environment variable is defined
			if (XEDKEnvironmentVariable == null)
			{
				throw new BuildException(
					"XEDK environment variable isn't set; you must properly install the Xbox 360 SDK before building UE3 for Xbox 360.\n" +
					MoreInfoString
					);
			}

			// Check that the environment variable references a valid directory.
			if (!Directory.Exists(XEDKEnvironmentVariable))
			{
				throw new BuildException(
					string.Format(
						"XEDK environment variable is set to a non-existant directory: {0}\n",
						XEDKEnvironmentVariable
						) +
					MoreInfoString
					);
			}

			return Path.Combine(
				XEDKEnvironmentVariable,
				"bin/win32"
				);
		}

		/** Creates an XEX file from a PE EXE file. */
		public static FileItem CreateXEXFromEXE(FileItem EXEFile,string XEXFilePath,FileItem XEXConfigFile)
		{
			Action ImageXEXAction = new Action();

			ImageXEXAction.WorkingDirectory = Path.GetFullPath(".");
			ImageXEXAction.CommandPath = Path.Combine(GetBinDirectory(),"imagexex.exe");
			ImageXEXAction.CommandArguments = string.Format("/out:\"{0}\" /nologo \"{1}\"",XEXFilePath,EXEFile.AbsolutePath);
			ImageXEXAction.StatusDescription = string.Format("{0}", Path.GetFileName(XEXFilePath));
			ImageXEXAction.bCanExecuteRemotely = false;

			// If a XEX config file was specified, use it.
			if (XEXConfigFile != null)
			{
				ImageXEXAction.CommandArguments += string.Format(" /XEXCONFIG:\"{0}\"", XEXConfigFile.AbsolutePath);
				ImageXEXAction.PrerequisiteItems.Add(XEXConfigFile);
			}

			// Add the EXE file as a prerequisite of the action.
			ImageXEXAction.PrerequisiteItems.Add(EXEFile);

			// Add the XEX file as a production of the action.
			FileItem XEXFile = FileItem.GetItemByPath(XEXFilePath);
			ImageXEXAction.ProducedItems.Add(XEXFile);

			return XEXFile;
		}
	}
}
