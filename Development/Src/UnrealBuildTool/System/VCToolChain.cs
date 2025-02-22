/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

namespace UnrealBuildTool
{
	class VCToolChain
	{
		/** Accesses the bin directory for the VC toolchain for the specified platform. */
		public static string GetVCBinDirectory(CPPTargetPlatform Platform)
		{
			if (Platform == CPPTargetPlatform.Xbox360)
			{
				return Xbox360ToolChain.GetBinDirectory();
			}
			else
			{
				return Path.Combine(
					Environment.GetEnvironmentVariable("VS80COMNTOOLS"),
					"../../VC/bin"
					);
			}
		}

		static string GetCLArguments_Global()
		{
			string Result = "";

			// Prevents the compiler from displaying its logo for each invocation.
			Result += " /nologo";

			// Allow inline method expansion.
			Result += " /Ob2";

			// Favor code speed.
			Result += " /Ot";

			// Enable intrinsic functions.
			Result += " /Oi";

			// Pack struct members on 4-byte boundaries.
			Result += " /Zp4";

			// Separate functions for linker.
			Result += " /Gy";

			// Relaxes floating point precision semantics to allow more optimization.
			Result += " /fp:fast";

			// Compile into an .obj file, and skip linking.
			Result += " /c";

			// Allow 1000% of the default memory allocation limit.
			Result += " /Zm1000";

			// Allow large object files to avoid hitting the 2^16 section limit when running with -StressTestUnity.
			Result += " /bigobj";

			return Result;
		}

		static string GetCLArguments_CPP()
		{
			string Result = "";

			// Explicitly compile the file as C++.
			Result += " /TP";

			// Disable C++ RTTI.
			Result += " /GR-";

			// Level 4 warnings.
			Result += " /W4";

			// Treat warnings as errors.
			Result += " /WX";

			return Result;
		}

		static string GetCLArguments_C()
		{
			string Result = "";

			// Explicitly compile the file as C.
			Result += " /TC";

			// Level 0 warnings.  Needed for external C projects that produce warnings at higher warning levels.
			Result += " /W0";

			return Result;
		}

		static string GetCLArguments_Debug()
		{
			string Result = "";

			// Disable compiler optimization.
			Result += " /Od";

			return Result;
		}

		static string GetCLArguments_Release()
		{
			string Result = "";

			// Maximum optimizations.
			Result += " /Ox";

			return Result;
		}

		static string GetCLArguments_ReleaseLTCG()
		{
			string Result = "";

			// Maximum optimizations.
			Result += " /Ox";

			// Link-time code generation.
			Result += " /GL";

			return Result;
		}

		static string GetCLArguments_Win32()
		{
			string Result = "";

			// Allow the compiler to generate SSE instructions.
			Result += " /arch:SSE";

			// Prompt the user before reporting internal errors to Microsoft.
			Result += " /errorReport:prompt";

			// Enable C++ exception handling, but not C exceptions.
			Result += " /EHsc";

			return Result;
		}

		static string GetCLArguments_Xbox360()
		{
			string Result = "";

			// Pool strings as read-only.
			Result += " /GF";

			return Result;
		}

		static string GetLinkArguments_Global()
		{
			string Result = "";

			// Don't create a side-by-side manifest file for the executable.
			Result += " /MANIFEST:NO";

			// Prevents the linker from displaying its logo for each invocation.
			Result += " /NOLOGO";

			// Output debug info for the linked executable.
			Result += " /DEBUG";

			// Prompt the user before reporting internal errors to Microsoft.
			Result += " /errorReport:prompt";

			return Result;
		}

		static string GetLinkArguments_Win32()
		{
			string Result = "";

			// Link for win32.
			Result += " /SUBSYSTEM:WINDOWS";

			// Allow the OS to load the EXE at different base addresses than its preferred base address.
			Result += " /FIXED:No";

			// Allow delay-loaded DLLs to be explicitly unloaded.
			Result += " /DELAY:UNLOAD";

			// Disables the 2GB address space limit on 64-bit Windows and 32-bit Windows with /3GB specified in boot.ini
			Result += " /LARGEADDRESSAWARE";

			// Explicitly declare that the executable is compatible with Data Execution Prevention.
			Result += " /NXCOMPAT";

			// Set the default stack size.
			Result += " /STACK:5000000,5000000";

			// Generates a table of Safe Exception Handlers.  Documentation isn't clear whether they actually mean
			// Structured Exception Handlers.
			Result += " /SAFESEH";

			return Result;
		}

		static string GetLinkArguments_Xbox360()
		{
			string Result = "";

			// Don't produce the final XEX directly.
			Result += " /XEX:NO";

			// Set the default stack size.
			Result += " /STACK:262144,262144";

			return Result;
		}

		static string GetLinkArguments_Debug()
		{
			string Result = "";

			// Keep symbols that are unreferenced.
			Result += " /OPT:NOREF";

			// Disable identical COMDAT folding.
			Result += " /OPT:NOICF";

			if (BuildConfiguration.bUsePDBFiles)
			{
				// Allow minimal rebuild.
				Result += " /Gm";
			}

			return Result;
		}

		static string GetLinkArguments_Release()
		{
			string Result = "";

			// Keep symbols that are unreferenced.
			Result += " /OPT:NOREF";

			// Disable identical COMDAT folding.
			Result += " /OPT:NOICF";

			return Result;
		}

		static string GetLinkArguments_ReleaseLTCG()
		{
			string Result = "";

			// Eliminate unreferenced symbols.
			Result += " /OPT:REF";

			// Remove redundant COMDATs.
			Result += " /OPT:ICF";

			// Use link-time code generation.
			Result += " /ltcg";

			return Result;
		}

		public static CPPOutput CompileCPPFiles(CPPEnvironment CompileEnvironment, IEnumerable<FileItem> SourceFiles)
		{
			string Arguments = GetCLArguments_Global();

			if (CompileEnvironment.bCreateDebugInfo)
			{
				if (BuildConfiguration.bUsePDBFiles)
				{
					// Store debug info a .pdb file.
					Arguments += " /Zi";
				}
				else
				{
					// Store C7-format debug info in the .obj files.
					Arguments += " /Z7";
				}
			}

			// Add platform-specific compiler options.
			bool bUseStaticRuntimeLibrary = false;
			if(CompileEnvironment.TargetPlatform == CPPTargetPlatform.Win32)
			{
				Arguments += GetCLArguments_Win32();

				if(CompileEnvironment.TargetConfiguration == CPPTargetConfiguration.Debug)
				{
					// Enable runtime stack checking for Win32 debug builds.
					Arguments += " /RTCs";
				}
			}
			else
			{
				bUseStaticRuntimeLibrary = true;

				Arguments += GetCLArguments_Xbox360();

				if(CompileEnvironment.TargetConfiguration == CPPTargetConfiguration.ReleaseLTCG)
				{
					// Disable generation of trap instructions around integer divides.
					Arguments += " /Oc";

					// Perform an additional code scheduling pass before the register allocation phase.
					Arguments += " /Ou";

					// Enable inline assembly optimization.
					Arguments += " /Oz";
				}
			}
			
			// Add configuration specific arguments.
			bool bUseDebugRuntimeLibrary = false;
			switch (CompileEnvironment.TargetConfiguration)
			{
				case CPPTargetConfiguration.Debug:
					bUseDebugRuntimeLibrary = true;
					Arguments += GetCLArguments_Debug();
					break;
				case CPPTargetConfiguration.Release:
					Arguments += GetCLArguments_Release();
					break;
				case CPPTargetConfiguration.ReleaseLTCG:
					Arguments += GetCLArguments_ReleaseLTCG();
					break;
			};

			// Specify the appropriate runtime library based on the platform and config.
			Arguments += string.Format(
				" /M{0}{1}",
				bUseStaticRuntimeLibrary ? 'T' : 'D',
				bUseDebugRuntimeLibrary ? "d" : ""
				);

			// Add include paths to the argument list.
			foreach (string IncludePath in CompileEnvironment.IncludePaths)
			{
				Arguments += string.Format(" /I \"{0}\"", IncludePath);
			}
			foreach (string IncludePath in CompileEnvironment.SystemIncludePaths)
			{
				Arguments += string.Format(" /I \"{0}\"", IncludePath);
			}

			// Add preprocessor definitions to the argument list.
			foreach (string Definition in CompileEnvironment.Definitions)
			{
				Arguments += string.Format(" /D \"{0}\"", Definition);
			}

			// Create a compile action for each source file.
			CPPOutput Result = new CPPOutput();
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
					// Generate a CPP File that just includes the precompiled header.
					string PCHCPPFilename = Path.GetFileName(CompileEnvironment.PrecompiledHeaderIncludeFilename) + ".PCH.cpp";
					string PCHCPPPath = Path.Combine(CompileEnvironment.OutputDirectory, PCHCPPFilename);
					FileItem PCHCPPFile = FileItem.CreateIntermediateTextFile(
						PCHCPPPath,
						string.Format("#include \"{0}\"\r\n", CompileEnvironment.PrecompiledHeaderIncludeFilename)
						);

					// Add the precompiled header file to the produced items list.
					FileItem PrecompiledHeaderFile = FileItem.GetItemByPath(
						Path.Combine(
							CompileEnvironment.OutputDirectory,
							Path.GetFileName(SourceFile.AbsolutePath) + ".pch"
							)
						);
					CompileAction.ProducedItems.Add(PrecompiledHeaderFile);
					Result.PrecompiledHeaderFile = PrecompiledHeaderFile;

					// Add the parameters needed to compile the precompiled header file to the command-line.
					FileArguments += string.Format(" /Yc\"{0}\"", CompileEnvironment.PrecompiledHeaderIncludeFilename);
					FileArguments += string.Format(" /Fp\"{0}\"", PrecompiledHeaderFile.AbsolutePath);
					FileArguments += string.Format(" {0}", PCHCPPFile.AbsolutePath);
				}
				else
				{
					if (CompileEnvironment.PrecompiledHeaderAction == PrecompiledHeaderAction.Include)
					{
						CompileAction.PrerequisiteItems.Add(CompileEnvironment.PrecompiledHeaderFile);
						FileArguments += string.Format(" /Yu\"{0}\"", CompileEnvironment.PrecompiledHeaderIncludeFilename);
						FileArguments += string.Format(" /Fp\"{0}\"", CompileEnvironment.PrecompiledHeaderFile.AbsolutePath);

					}
					
					// Add the source file path to the command-line.
					FileArguments += string.Format(" \"{0}\"", SourceFile.AbsolutePath);
				}

				// Add the object file to the produced item list.
				FileItem ObjectFile = FileItem.GetItemByPath(
					Path.Combine(
						CompileEnvironment.OutputDirectory,
						Path.GetFileName(SourceFile.AbsolutePath) + ".obj"
						)
					);
				CompileAction.ProducedItems.Add(ObjectFile);
				Result.ObjectFiles.Add(ObjectFile);
				FileArguments += string.Format(" /Fo\"{0}\"", ObjectFile.AbsolutePath);

				if (BuildConfiguration.bUsePDBFiles)
				{
					// Specify the PDB file that the compiler should write to.
					FileItem PDBFile = FileItem.GetItemByPath(
							Path.Combine(
								CompileEnvironment.OutputDirectory,
								Path.GetFileName(SourceFile.AbsolutePath) + ".pdb"
								)
							);
					FileArguments += string.Format(" /Fd\"{0}\"", PDBFile.AbsolutePath);
					CompileAction.ProducedItems.Add(PDBFile);
					Result.DebugDataFiles.Add(PDBFile);
				}

				// Add C or C++ specific compiler arguments.
				if (Path.GetExtension(SourceFile.AbsolutePath).ToUpperInvariant() == ".C")
				{
					FileArguments += GetCLArguments_C();
				}
				else
				{
					FileArguments += GetCLArguments_CPP();
				}

				CompileAction.WorkingDirectory = Path.GetFullPath(".");
				CompileAction.CommandPath = Path.Combine(GetVCBinDirectory(CompileEnvironment.TargetPlatform), "cl.exe");
				CompileAction.CommandArguments = Arguments + FileArguments + CompileEnvironment.AdditionalArguments;
				CompileAction.StatusDescription = string.Format("{0}",Path.GetFileName(SourceFile.AbsolutePath));

				// Allow compiles to execute remotely unless they create a precompiled header.  Force those to run locally to avoid the
				// need to copy the PCH back to the build instigator before distribution to other remote build agents.
				// Also, distributing compiles that use precompiled headers doesn't work for an unknown reason.
				CompileAction.bCanExecuteRemotely = CompileEnvironment.PrecompiledHeaderAction == PrecompiledHeaderAction.None;
			}
			return Result;
		}

		public static CPPOutput CompileRCFiles(CPPEnvironment Environment, IEnumerable<FileItem> RCFiles)
		{
			CPPOutput Result = new CPPOutput();

			foreach (FileItem RCFile in RCFiles)
			{
				Action CompileAction = new Action();
				CompileAction.WorkingDirectory = Path.GetFullPath(".");
				CompileAction.CommandPath = Path.Combine(GetVCBinDirectory(Environment.TargetPlatform), "rc.exe");
				CompileAction.StatusDescription = string.Format("{0}", Path.GetFileName(RCFile.AbsolutePath));

				// Language
				CompileAction.CommandArguments += " /l 0x409";

				// Include paths.
				foreach (string IncludePath in Environment.IncludePaths)
				{
					CompileAction.CommandArguments += string.Format(" /i \"{0}\"", IncludePath);
				}

				// Preprocessor definitions.
				foreach (string Definition in Environment.Definitions)
				{
					CompileAction.CommandArguments += string.Format(" /d \"{0}\"", Definition);
				}

				// Add the RES file to the produced item list.
				FileItem CompiledResourceFile = FileItem.GetItemByPath(
					Path.Combine(
						Environment.OutputDirectory,
						Path.GetFileName(RCFile.AbsolutePath) + ".res"
						)
					);
				CompileAction.ProducedItems.Add(CompiledResourceFile);
				CompileAction.CommandArguments += string.Format(" /fo \"{0}\"", CompiledResourceFile.AbsolutePath);
				Result.ObjectFiles.Add(CompiledResourceFile);

				// Add the RC file as a prerequisite of the action.
				CompileAction.PrerequisiteItems.Add(RCFile);
				CompileAction.CommandArguments += string.Format(" \"{0}\"", RCFile.AbsolutePath);

				// Add the files included by the RC file as prerequisites of the action.
				foreach (FileItem IncludedFile in Environment.GetIncludeDependencies(RCFile))
				{
					CompileAction.PrerequisiteItems.Add(IncludedFile);
				}
			}

			return Result;
		}

		public static FileItem LinkFiles(LinkEnvironment LinkEnvironment)
		{
			// Create an action that invokes the linker.
			Action LinkAction = new Action();
			LinkAction.WorkingDirectory = Path.GetFullPath(".");
			LinkAction.CommandPath = Path.Combine(GetVCBinDirectory(LinkEnvironment.TargetPlatform), "link.exe");

			LinkAction.CommandArguments = GetLinkArguments_Global();

			// Add platform-specific arguments.
			if (LinkEnvironment.TargetPlatform == CPPTargetPlatform.Win32)
			{
				LinkAction.CommandArguments += GetLinkArguments_Win32();

				// Delay-load these DLLs.
				foreach (string DelayLoadDLL in LinkEnvironment.DelayLoadDLLs)
				{
					LinkAction.CommandArguments += string.Format(" /DELAYLOAD:\"{0}\"", DelayLoadDLL);
				}
			}
			else
			{
				LinkAction.CommandArguments += GetLinkArguments_Xbox360();

				if (LinkEnvironment.TargetConfiguration != CPPTargetConfiguration.Debug)
				{
					// Generate an EXE checksum for release binaries.
					LinkAction.CommandArguments += " /RELEASE";
				}
			}

			// Add the configuration-specific arguments.
			switch (LinkEnvironment.TargetConfiguration)
			{
				case CPPTargetConfiguration.Debug:
					LinkAction.CommandArguments += GetLinkArguments_Debug();
					break;
				case CPPTargetConfiguration.Release:
					LinkAction.CommandArguments += GetLinkArguments_Release();
					break;
				case CPPTargetConfiguration.ReleaseLTCG:
					LinkAction.CommandArguments += GetLinkArguments_ReleaseLTCG();
					break;
			}

			// Add the library paths to the argument list.
			foreach (string LibraryPath in LinkEnvironment.LibraryPaths)
			{
				LinkAction.CommandArguments += string.Format(" /LIBPATH:\"{0}\"", LibraryPath);
			}

			// Add the excluded default libraries to the argument list.
			foreach (string ExcludedLibrary in LinkEnvironment.ExcludedLibraries)
			{
				LinkAction.CommandArguments += string.Format(" /NODEFAULTLIB:\"{0}\"", ExcludedLibrary);
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
			foreach (string AdditionalLibrary in LinkEnvironment.AdditionalLibraries)
			{
				InputFileNames.Add(string.Format("\"{0}\"", AdditionalLibrary));
			}
			LinkAction.CommandArguments += string.Format(" @\"{0}\"", ResponseFile.Create(InputFileNames));

			// Add the output file to the command-line.
			LinkAction.CommandArguments += string.Format(" /OUT:\"{0}\"", OutputFile.AbsolutePath);

			// Write the import library to the output directory.
			string ImportLibraryFilePath = Path.Combine(
				LinkEnvironment.OutputDirectory,
				Path.GetFileNameWithoutExtension(OutputFile.AbsolutePath) + ".lib"
				);
			FileItem ImportLibraryFile = FileItem.GetItemByPath(ImportLibraryFilePath);
			LinkAction.CommandArguments += string.Format(" /IMPLIB:\"{0}\"",ImportLibraryFilePath);
			LinkAction.ProducedItems.Add(ImportLibraryFile);

			// An export file is written to the output directory implicitly; add it to the produced items list.
			string ExportFilePath = Path.ChangeExtension(ImportLibraryFilePath, ".exp");
			FileItem ExportFile = FileItem.GetItemByPath(ExportFilePath);
			LinkAction.ProducedItems.Add(ExportFile);

			// Write the PDB file to the output directory.
			string PDBFilePath = Path.Combine(LinkEnvironment.OutputDirectory,Path.GetFileNameWithoutExtension(OutputFile.AbsolutePath) + ".pdb");
			FileItem PDBFile = FileItem.GetItemByPath(PDBFilePath);
			LinkAction.CommandArguments += string.Format(" /PDB:\"{0}\"",PDBFilePath);
			LinkAction.ProducedItems.Add(PDBFile);

			// Add the additional arguments specified by the environment.
			LinkAction.CommandArguments += LinkEnvironment.AdditionalArguments;

			// Only execute linking on the local PC.
			LinkAction.bCanExecuteRemotely = false;

			return OutputFile;
		}
	};
}