/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;

namespace UnrealBuildTool
{
	class Unity
	{
		/**
		 * Given a set of C++ files, generates another set of C++ files that #include all the original
		 * files, the goal being to compile the same code in fewer translation units.
		 * The "unity" files are written to the CompileEnvironment's OutputDirectory.
		 * @param CPPFiles - The C++ files to #include.
		 * @param CompileEnvironment - The environment that is used to compile the C++ files.
		 * @return The "unity" C++ files.
		 */
		public static List<FileItem> GenerateUnityCPPs(
			List<FileItem> CPPFiles, 
			CPPEnvironment CompileEnvironment
			)	
		{
			// Create a set of CPP files that combine smaller CPP files into larger compilation units, along with the corresponding 
			// actions to compile them.
			int InputFileIndex = 0;
			List<FileItem> UnityCPPFiles = new List<FileItem>();
			while (InputFileIndex < CPPFiles.Count)
			{
				StringWriter OutputUnityCPPWriter = new StringWriter();

				OutputUnityCPPWriter.WriteLine("// This file is automatically generated at compile-time to include some subset of the user-created cpp files.");

				// Explicitly include the precompiled header first, since Visual C++ expects the first top-level #include to be the header file
				// that was used to create the PCH.
				if (CompileEnvironment.PrecompiledHeaderIncludeFilename != null)
				{
					OutputUnityCPPWriter.WriteLine("#include \"{0}\"", CompileEnvironment.PrecompiledHeaderIncludeFilename);
				}

				// Add source files to the unity file until the number of included bytes crosses a threshold.
				long NumIncludedBytesInThisOutputFile = 0;
				while(	InputFileIndex < CPPFiles.Count &&
						(BuildConfiguration.bStressTestUnity ||
						NumIncludedBytesInThisOutputFile < BuildConfiguration.NumIncludedBytesPerUnityCPP))
				{
					FileItem CPPFile = CPPFiles[InputFileIndex];
					OutputUnityCPPWriter.WriteLine("#include \"{0}\"", CPPFile.AbsolutePath);
					NumIncludedBytesInThisOutputFile += CPPFile.Info.Length;
					InputFileIndex++;
				}

				// Write the unity file to the intermediate folder.
				string UnityCPPFilePath = Path.Combine(
					CompileEnvironment.OutputDirectory,
					string.Format("Unity_{0}EtAl.cpp",Path.GetFileNameWithoutExtension(CPPFiles[InputFileIndex - 1].AbsolutePath))
					);
				FileItem UnityCPPFile = FileItem.CreateIntermediateTextFile(UnityCPPFilePath, OutputUnityCPPWriter.ToString());
				UnityCPPFiles.Add(UnityCPPFile);
			}

			return UnityCPPFiles;
		}
	}
}