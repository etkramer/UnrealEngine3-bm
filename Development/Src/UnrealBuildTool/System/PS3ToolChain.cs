/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.IO;

namespace UnrealBuildTool
{
	class PS3ToolChain
	{
		/**
		 * Whether precompiled headers should be disabled on PS3; this is currently necessary to work around errors that GCC errors that occur
		 * when using PCHs in conjunction with XGE.
		 */
		static bool bDisablePrecompiledHeaders = true;

		static string GetCompileArguments_Global()
		{
			string Result = "";
			Result += " -fmessage-length=0";
			Result += " -fshort-wchar";
			Result += " -Wcomment";
			Result += " -Wmissing-braces";
			Result += " -Wparentheses";
			Result += " -Wredundant-decls";
			Result += " -Werror";
			Result += " -fno-exceptions";
			Result += " -Winvalid-pch";
			Result += " -pipe";
			Result += " -Wno-redundant-decls";
			Result += " -Wreturn-type";
			Result += " -Winit-self";
			Result += " -c";
			return Result;
		}

		static string GetCompileArguments_CPP()
		{
			string Result = "";
			Result += " -Wreorder";
			Result += " -Wno-non-virtual-dtor";
			Result += " -fcheck-new";
			Result += " -fno-rtti";
			return Result;
		}

		static string GetCompileArguments_C()
		{
			string Result = "";
			Result += " -x c";
			Result += " -ffunction-sections";
			Result += " -fdata-sections";
			return Result;
		}

		static string GetCompileArguments_Debug()
		{
			string Result = "";
			return Result;
		}

		static string GetCompileArguments_Release()
		{
			string Result = "";
			Result += " -fno-strict-aliasing";
			Result += " -O3";
			Result += " -fforce-addr";
			Result += " -ffast-math";
			Result += " -funroll-loops";
			Result += " -Wuninitialized";
			return Result;
		}

		static string GetCompileArguments_ReleaseLTCG()
		{
			string Result = "";
			Result += " -fno-strict-aliasing";
			Result += " -O3";
			Result += " -fforce-addr";
			Result += " -ffast-math";
			Result += " -funroll-loops";
			Result += " -Wuninitialized";
			return Result;
		}

		static string GetLinkArguments_Global()
		{
			string Result = "";

			// Use a wrapper function for these symbols.
			Result += " -Wl,--wrap,malloc";
			Result += " -Wl,--wrap,free";
			Result += " -Wl,--wrap,memalign";
			Result += " -Wl,--wrap,calloc";
			Result += " -Wl,--wrap,realloc";
			Result += " -Wl,--wrap,reallocalign";

			return Result;
		}

		static string GetObjCopyArguments_Global()
		{
			string Result = "";

			Result += " -I binary";
			Result += " -O elf64-powerpc-celloslv2";
			Result += " -B powerpc";
			Result += " --set-section-align .data=7";
			Result += " --set-section-pad .data=128";

			return Result;
		}

		/** Returns a valid PS3 SDK root directory path, or throws a BuildException. */
		static string GetSCEPS3Root()
		{
			string MoreInfoString =
				"See https://udn.epicgames.com/Three/GettingStartedPS3 for help setting up the UE3 PS3 compilation environment";

			// Read the root directory of the PS3 SDK from the environment.
			string SCEPS3RootEnvironmentVariable = Environment.GetEnvironmentVariable("SCE_PS3_ROOT");

			// Check that the environment variable is defined
			if (SCEPS3RootEnvironmentVariable == null)
			{
				throw new BuildException(
					"SCE_PS3_ROOT environment variable isn't set; you must properly install the PS3 SDK before building UE3 for PS3.\n" +
					MoreInfoString
					);
			}

			// Check that the environment variable references a valid directory.
			if (!Directory.Exists(SCEPS3RootEnvironmentVariable))
			{
				throw new BuildException(
					string.Format(
						"SCE_PS3_ROOT environment variable is set to a non-existant directory: {0}\n",
						SCEPS3RootEnvironmentVariable
						) +
					MoreInfoString
					);
			}

			return SCEPS3RootEnvironmentVariable;
		}

		public static CPPOutput CompileCPPFiles(CPPEnvironment CompileEnvironment,IEnumerable<FileItem> SourceFiles)
		{
			// Optionally disable precompiled headers by not creating the .gch file.
			if (CompileEnvironment.PrecompiledHeaderAction == PrecompiledHeaderAction.Create && bDisablePrecompiledHeaders)
			{
				return null;
			}

			string Arguments = GetCompileArguments_Global();

			switch (CompileEnvironment.TargetConfiguration)
			{
				case CPPTargetConfiguration.Debug:
					Arguments += GetCompileArguments_Debug();
					break;
				case CPPTargetConfiguration.Release:
					Arguments += GetCompileArguments_Release();
					break;
				case CPPTargetConfiguration.ReleaseLTCG:
					Arguments += GetCompileArguments_ReleaseLTCG();
					break;
			}

			if (CompileEnvironment.bCreateDebugInfo)
			{
				// Create GDB format debug data.
				Arguments += " -g";
				Arguments += " -ggdb";
			}

			if (CompileEnvironment.PrecompiledHeaderAction == PrecompiledHeaderAction.Include)
			{
				// Add the precompiled header file's path to the include path so GCC can find it.
				// This needs to be before the other include paths to ensure GCC uses it instead of the source header file.
				Arguments += string.Format(
					" -I\"{0}\"",
					Path.GetDirectoryName(CompileEnvironment.PrecompiledHeaderFile.AbsolutePath)
					);
			}

			// Add include paths to the argument list.
			foreach (string IncludePath in CompileEnvironment.IncludePaths)
			{
				Arguments += string.Format(" -I\"{0}\"", IncludePath);
			}
			foreach (string IncludePath in CompileEnvironment.SystemIncludePaths)
			{
				Arguments += string.Format(" -I\"{0}\"", IncludePath);
			}

			foreach (string Definition in CompileEnvironment.Definitions)
			{
				Arguments += string.Format(" -D\"{0}\"", Definition);
			}

			CPPOutput Result = new CPPOutput();
			// Create a compile action for each source file.
			foreach (FileItem SourceFile in SourceFiles)
			{
				Action CompileAction = new Action();
				string FileArguments = "";

				// Add the C++ source file and its included files to the prerequisite item list.
				CompileAction.PrerequisiteItems.Add(SourceFile);
				foreach (FileItem IncludedFile in CompileEnvironment.GetIncludeDependencies(SourceFile))
				{
					CompileAction.PrerequisiteItems.Add(IncludedFile);
				}

				if (CompileEnvironment.PrecompiledHeaderAction == PrecompiledHeaderAction.Create)
				{
					// Add the precompiled header file to the produced item list.
					FileItem PrecompiledHeaderFile = FileItem.GetItemByPath(
						Path.Combine(
							CompileEnvironment.OutputDirectory,
							Path.GetFileName(SourceFile.AbsolutePath) + ".gch"
							)
						);
					CompileAction.ProducedItems.Add(PrecompiledHeaderFile);
					Result.PrecompiledHeaderFile = PrecompiledHeaderFile;

					// Add the parameters needed to compile the precompiled header file to the command-line.
					FileArguments += string.Format(" -o \"{0}\"", PrecompiledHeaderFile.AbsolutePath);
				}
				else
				{
					if (CompileEnvironment.PrecompiledHeaderAction == PrecompiledHeaderAction.Include)
					{
						CompileAction.PrerequisiteItems.Add(CompileEnvironment.PrecompiledHeaderFile);
					}

					// Add the object file to the produced item list.
					FileItem ObjectFile = FileItem.GetItemByPath(
						Path.Combine(
							CompileEnvironment.OutputDirectory,
							Path.GetFileName(SourceFile.AbsolutePath) + ".o"
							)
						);
					CompileAction.ProducedItems.Add(ObjectFile);
					Result.ObjectFiles.Add(ObjectFile);
					FileArguments += string.Format(" -o \"{0}\"", ObjectFile.AbsolutePath);
				}

				if (Path.GetExtension(SourceFile.AbsolutePath).ToUpperInvariant() == ".C")
				{
					// Compile the file as C code.
					FileArguments += GetCompileArguments_C();
				}
				else
				{
					// Compile the file as C++ code.
					FileArguments += GetCompileArguments_CPP();
				}

				// Add the source file path to the command-line.
				FileArguments += string.Format(" {0}", SourceFile.AbsolutePath);

				CompileAction.WorkingDirectory = Path.GetFullPath(".");
				CompileAction.CommandPath = Path.Combine(GetSCEPS3Root(), "host-win32/ppu/bin/ppu-lv2-g++.exe");
				CompileAction.CommandArguments = Arguments + FileArguments + CompileEnvironment.AdditionalArguments;
				CompileAction.StatusDescription = string.Format("{0}", Path.GetFileName(SourceFile.AbsolutePath));

				// Allow compiles to execute remotely unless they create a precompiled header.  Force those to run locally to avoid the
				// need to copy the PCH back to the build instigator before distribution to other remote build agents.
				// For now compiles that use a precompiled header are also run locally, since GCC crashes when those compiles are distributed.
				CompileAction.bCanExecuteRemotely = CompileEnvironment.PrecompiledHeaderAction == PrecompiledHeaderAction.None;
			}
			return Result;
		}

		public static FileItem LinkFiles(LinkEnvironment LinkEnvironment)
		{
			// Create an action that invokes the linker.
			Action LinkAction = new Action();
			LinkAction.WorkingDirectory = Path.GetFullPath(".");
			LinkAction.CommandPath = Path.Combine(GetSCEPS3Root(), "host-win32/ppu/bin/ppu-lv2-g++.exe");

			LinkAction.CommandArguments = GetLinkArguments_Global();

			// Add the library paths to the argument list.
			foreach (string LibraryPath in LinkEnvironment.LibraryPaths)
			{
				LinkAction.CommandArguments += string.Format(" -L\"{0}\"", LibraryPath);
			}

			// Add the additional libraries to the argument list.
			foreach (string AdditionalLibrary in LinkEnvironment.AdditionalLibraries)
			{
				LinkAction.CommandArguments += string.Format(" -l\"{0}\"", AdditionalLibrary);
			}

			// Add the output file as a production of the link action.
			FileItem OutputFile = FileItem.GetItemByPath(LinkEnvironment.OutputFilePath);
			LinkAction.ProducedItems.Add(OutputFile);
			LinkAction.StatusDescription = string.Format("{0}", Path.GetFileName(OutputFile.AbsolutePath));

			// Add the input files to a response file, and pass the response file on the command-line.
			List<string> InputFileNames = new List<string>();
			foreach (FileItem InputFile in LinkEnvironment.InputFiles)
			{
				InputFileNames.Add(string.Format("\"{0}\"", InputFile.AbsolutePath));
				LinkAction.PrerequisiteItems.Add(InputFile);
			}

			// Write the list of input files to a response file.
			LinkAction.CommandArguments += string.Format(" @\"{0}\"", ResponseFile.Create(InputFileNames));

			// Add the output file to the command-line.
			LinkAction.CommandArguments += string.Format(" -o \"{0}\"", OutputFile.AbsolutePath);

			// Add the additional arguments specified by the environment.
			LinkAction.CommandArguments += LinkEnvironment.AdditionalArguments;

			// Only execute linking on the local PC.
			LinkAction.bCanExecuteRemotely = false;

			return OutputFile;
		}

		public static FileItem CreateObjectFileFromDataFile(FileItem InputDataFile, string OutputObjectFilePath, string SectionName)
		{
			// Create an action that invokes ppu-lv2-objcopy.
			// The action's working directory is the directory containing the input data file, so the directory path isn't included
			// in the mangled symbol name that is used to refer to the data.
			Action ObjCopyAction = new Action();
			ObjCopyAction.WorkingDirectory = Path.GetDirectoryName(InputDataFile.AbsolutePath);
			ObjCopyAction.CommandPath = Path.Combine(GetSCEPS3Root(), "host-win32/ppu/bin/ppu-lv2-objcopy.exe");
			ObjCopyAction.StatusDescription = string.Format("{0}", Path.GetFileName(OutputObjectFilePath));
			ObjCopyAction.bCanExecuteRemotely = false;

			ObjCopyAction.CommandArguments = GetObjCopyArguments_Global();
			
			// Rename the data section to the specified name.
			ObjCopyAction.CommandArguments += string.Format(" --rename-section .data={0}", SectionName);

			// Specify the input file.
			ObjCopyAction.CommandArguments += string.Format(" {0}", Path.GetFileName(InputDataFile.AbsolutePath));
			ObjCopyAction.PrerequisiteItems.Add(InputDataFile);

			// Specify the output object file.
			FileItem OutputObjectFile = FileItem.GetItemByPath(OutputObjectFilePath);
			ObjCopyAction.CommandArguments += string.Format(" {0}", OutputObjectFile.AbsolutePath);
			ObjCopyAction.ProducedItems.Add(OutputObjectFile);

			return OutputObjectFile;
		}

		public static FileItem StripDebugInfoFromExecutable(FileItem InputFile,string OutputFilePath)
		{
			// Create an action that invokes ppu-lv2-strip.
			Action StripAction = new Action();
			StripAction.WorkingDirectory = Path.GetFullPath(".");
			StripAction.CommandPath = Path.Combine(GetSCEPS3Root(), "host-win32/ppu/bin/ppu-lv2-strip.exe");
			StripAction.StatusDescription = string.Format("{0}", Path.GetFileName(OutputFilePath));
			StripAction.bCanExecuteRemotely = false;

			// Strip all debug info and symbols.
			StripAction.CommandArguments = " -s";

			// Specify the output object file.
			FileItem OutputFile = FileItem.GetItemByPath(OutputFilePath);
			StripAction.CommandArguments += string.Format(" -o {0}", OutputFile.AbsolutePath);
			StripAction.ProducedItems.Add(OutputFile);

			// Specify the input file.
			StripAction.CommandArguments += string.Format(" {0}", InputFile.AbsolutePath);
			StripAction.PrerequisiteItems.Add(InputFile);

			return OutputFile;
		}

		public static FileItem EncryptExecutable(FileItem InputFile, string OutputFilePath)
		{
			// Create an action that invokes make_fself.
			Action EncryptAction = new Action();
			EncryptAction.WorkingDirectory = Path.GetFullPath(".");
			EncryptAction.CommandPath = Path.Combine(GetSCEPS3Root(), "host-win32/bin/make_fself.exe");
			EncryptAction.StatusDescription = string.Format("{0}", Path.GetFileName(OutputFilePath));
			EncryptAction.bCanExecuteRemotely = false;

			// Specify the input file.
			EncryptAction.CommandArguments += string.Format(" {0}", InputFile.AbsolutePath);
			EncryptAction.PrerequisiteItems.Add(InputFile);

			// Specify the output object file.
			FileItem OutputFile = FileItem.GetItemByPath(OutputFilePath);
			EncryptAction.CommandArguments += string.Format(" {0}", OutputFile.AbsolutePath);
			EncryptAction.ProducedItems.Add(OutputFile);

			return OutputFile;
		}
	};
}
