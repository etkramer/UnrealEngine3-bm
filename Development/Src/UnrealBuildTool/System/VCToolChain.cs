/**
 *
 * Copyright 1998-2009 Epic Games, Inc. All Rights Reserved.
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

		static string GetCLArguments_Global(CPPEnvironment CompileEnvironment)
		{
			string Result = "";

			// Prevents the compiler from displaying its logo for each invocation.
			Result += " /nologo";

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

			// Handle Common Language Runtime support (C++/CLI)
			if( CompileEnvironment.CLRMode == CPPCLRMode.CLREnabled )
			{
				Result += " /clr";
			}

			bool bUseDebugRuntimeLibrary = false;
			bool bUseStaticRuntimeLibrary = false;

			//
			//	Debug
			//
			if( CompileEnvironment.TargetConfiguration == CPPTargetConfiguration.Debug )
			{
				// Use debug runtime in debug configuration.
				bUseDebugRuntimeLibrary = true;

				// Disable compiler optimization.
				Result += " /Od";

				// Allow inline method expansion unless E&C support is requested.
				if( !BuildConfiguration.bSupportEditAndContinue )
				{
					Result += " /Ob2";
				}

				if( CompileEnvironment.TargetPlatform == CPPTargetPlatform.Win32 )
				{
					// Runtime stack checks are not allowed when compiling for CLR
					if( CompileEnvironment.CLRMode == CPPCLRMode.CLRDisabled )
					{
						Result += " /RTCs";
					}
				}
			}
			//
			//	Release and LTCG
			//
			else
			{
				// Maximum optimizations.
				Result += " /Ox";

				// Allow inline method expansion.			
				Result += " /Ob2";

				//
				// LTCG
				//
				if( CompileEnvironment.TargetConfiguration == CPPTargetConfiguration.ReleaseLTCG )
				{
					// Link-time code generation.
					Result += " /GL";
	
					// Xbox 360 specific LTCG settings
					if( CompileEnvironment.TargetPlatform == CPPTargetPlatform.Xbox360 )
					{
						// Disable generation of trap instructions around integer divides.
						Result += " /Oc";

						// Perform an additional code scheduling pass before the register allocation phase.
						Result += " /Ou";

						// Enable inline assembly optimization.
						Result += " /Oz";
					}
				}
			}

			//
			//	PC
			//
			if( CompileEnvironment.TargetPlatform == CPPTargetPlatform.Win32 )
			{

			    // SSE options are not allowed when using CLR compilation
			    if( CompileEnvironment.CLRMode == CPPCLRMode.CLRDisabled )
			    {
				    // Allow the compiler to generate SSE instructions.
				    Result += " /arch:SSE";
			    }
    
			    // Prompt the user before reporting internal errors to Microsoft.
			    Result += " /errorReport:prompt";
    
			    if( CompileEnvironment.CLRMode == CPPCLRMode.CLRDisabled )
			    {
				    // Enable C++ exception handling, but not C exceptions.
				    Result += " /EHsc";
			    }
			    else
			    {
				    // For C++/CLI all exceptions must be left enabled
				    Result += " /EHa";
			    }
			}
			//
			//	Xbox 360
			//
			else
			{
				// Use static runtime on Xbox 360.
				bUseStaticRuntimeLibrary = true;

				// Pool strings as read-only.
				Result += " /GF";
			}

			// If enabled, create debug information.
			if( CompileEnvironment.bCreateDebugInfo )
			{
				// Store debug info in .pdb files.
				if( BuildConfiguration.bUsePDBFiles )
				{
					// Create debug info suitable for E&C if wanted.
					if( BuildConfiguration.bSupportEditAndContinue
					// We only need to do this in debug as that's the only configuration that supports E&C.
					&& CompileEnvironment.TargetConfiguration == CPPTargetConfiguration.Debug )
					{
						Result += " /ZI";
					}
					// Regular PDB debug information.
					else
					{
						Result += " /Zi";
					}
				}
				// Store C7-format debug info in the .obj files, which is faster.
				else
				{
					Result += " /Z7";
				}
			}

			// Specify the appropriate runtime library based on the platform and config.
			Result += string.Format(
				" /M{0}{1}",
				bUseStaticRuntimeLibrary ? 'T' : 'D',
				bUseDebugRuntimeLibrary ? "d" : ""
				);

			return Result;
		}

		static string GetCLArguments_CPP( CPPEnvironment CompileEnvironment )
		{
			string Result = "";

			// Explicitly compile the file as C++.
			Result += " /TP";

			// C++/CLI requires that RTTI is left enabled
			if( CompileEnvironment.CLRMode == CPPCLRMode.CLRDisabled )
			{
				// Disable C++ RTTI.
// Atlas-Modif [Begin]
				//Result += " /GR-";
// Atlas-Modif [End]
			}

			// Treat warnings as errors.
			Result += " /WX";

// Atlas-Modif [Begin]
            Result += " /wd4819";    // to make it work under CJK windows
// Atlas-Modif [End]

			if( CompileEnvironment.CLRMode == CPPCLRMode.CLRDisabled )
			{
				// Level 4 warnings.
				Result += " /W4";
			}
			else
			{
				// CLR-specific warnings

				// @WPF: We can't enable Level 4 warnings for CLR projects because for some reason warning
				//   4339 cannot be suppressed with either #pragma warning or the /wd4339 compiler option.
				Result += " /W3";

				// // Suppress (level 4) warning about use of undefined types since this comes up with template classes
				// Result += " /wd4339"; // 'MyClass' : use of undefined type detected in CLR meta-data - use of this type may lead to a runtime exception
			}

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

		static string GetLinkArguments(LinkEnvironment LinkEnvironment)
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

			//
			//	PC
			//
			if( LinkEnvironment.TargetPlatform == CPPTargetPlatform.Win32 )
			{
				// Link for win32.
				Result += " /SUBSYSTEM:WINDOWS";

				// Allow the OS to load the EXE at different base addresses than its preferred base address.
				Result += " /FIXED:No";

				// Disables the 2GB address space limit on 64-bit Windows and 32-bit Windows with /3GB specified in boot.ini
				Result += " /LARGEADDRESSAWARE";

				// Explicitly declare that the executable is compatible with Data Execution Prevention.
				Result += " /NXCOMPAT";

				// Set the default stack size.
				Result += " /STACK:5000000,5000000";

				// E&C can't use /SAFESEH.
				if( !BuildConfiguration.bSupportEditAndContinue )
				{
					// Generates a table of Safe Exception Handlers.  Documentation isn't clear whether they actually mean
					// Structured Exception Handlers.
					Result += " /SAFESEH";
				}

				// Include definition file required for PixelMine's UnrealScript debugger.
				//Result += " /DEF:UnrealEngine3.def";

				// Allow delay-loaded DLLs to be explicitly unloaded.
				Result += " /DELAY:UNLOAD";
			}
			//
			//	Xbox 360
			//
			else
			{
				// Don't produce the final XEX directly.
				Result += " /XEX:NO";

				// Set the default stack size.
				Result += " /STACK:262144,262144";
			}

			//
			//	ReleaseLTCG
			//
			if( LinkEnvironment.TargetConfiguration == CPPTargetConfiguration.ReleaseLTCG )
			{
				// Use link-time code generation.
				Result += " /ltcg";
			}

			//
			//	Shipping binary
			//
			if( LinkEnvironment.bIsShippingBinary )
			{
				// Generate an EXE checksum.
				Result += " /RELEASE";

				// Eliminate unreferenced symbols.
				Result += " /OPT:REF";

				// Remove redundant COMDATs.
				Result += " /OPT:ICF";
			}
			//
			//	Regular development binary. 
			//
			else
			{
				// Keep symbols that are unreferenced.
				Result += " /OPT:NOREF";

				// Disable identical COMDAT folding.
				Result += " /OPT:NOICF";
			}

			// Enable incremental linking if wanted.
			if( BuildConfiguration.bUseIncrementalLinking )
			{
				Result += " /INCREMENTAL";
			}
			// Disabled by default as it can cause issues and forces local execution.
			else
			{
				Result += " /INCREMENTAL:NO";
			}

			return Result;
		}

		public static CPPOutput CompileCPPFiles(CPPEnvironment CompileEnvironment, IEnumerable<FileItem> SourceFiles)
		{
			string Arguments = GetCLArguments_Global(CompileEnvironment);

			// Add include paths to the argument list.
			foreach (string IncludePath in CompileEnvironment.IncludePaths)
			{
				Arguments += string.Format(" /I \"{0}\"", IncludePath);
			}
			foreach (string IncludePath in CompileEnvironment.SystemIncludePaths)
			{
				Arguments += string.Format(" /I \"{0}\"", IncludePath);
			}


			if( CompileEnvironment.CLRMode == CPPCLRMode.CLREnabled )
			{
				// Add .NET framework assembly paths.  This is needed so that C++/CLI projects
				// can reference assemblies with #using, without having to hard code a path in the
				// .cpp file to the assembly's location.				
				foreach (string AssemblyPath in CompileEnvironment.SystemDotNetAssemblyPaths)
				{
					Arguments += string.Format(" /AI \"{0}\"", AssemblyPath);
				}

				// Add explicit .NET framework assembly references				
				foreach( string AssemblyName in CompileEnvironment.FrameworkAssemblyDependencies )
				{
					Arguments += string.Format( " /FU \"{0}\"", AssemblyName );
				}

				// Add private assembly references				
				foreach( string AssemblyName in CompileEnvironment.PrivateAssemblyDependencies )
				{
					Arguments += string.Format( " /FU \"{0}\"", AssemblyName );
				}
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

				// If this is a CLR file then make sure our dependent assemblies are added as prerequisites
				if( CompileEnvironment.CLRMode == CPPCLRMode.CLREnabled )
				{
					foreach( string CurPrivateAssemblyDependency in CompileEnvironment.PrivateAssemblyDependencies )
					{
						FileItem AssemblyFile = FileItem.GetItemByPath( CurPrivateAssemblyDependency );
						CompileAction.PrerequisiteItems.Add( AssemblyFile );
					}
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
					// All files using the same PCH are required to share a PDB.
					string PDBFileName;
					if (CompileEnvironment.PrecompiledHeaderAction == PrecompiledHeaderAction.Include)
					{
						PDBFileName = Path.GetFileName(CompileEnvironment.PrecompiledHeaderIncludeFilename);
					}
					else
					{
						PDBFileName = Path.GetFileName(SourceFile.AbsolutePath);
					}

					// Specify the PDB file that the compiler should write to.
					FileItem PDBFile = FileItem.GetItemByPath(
							Path.Combine(
								CompileEnvironment.OutputDirectory,
								PDBFileName + ".pdb"
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
					FileArguments += GetCLArguments_CPP( CompileEnvironment );
				}

				CompileAction.WorkingDirectory = Path.GetFullPath(".");
				CompileAction.CommandPath = Path.Combine(GetVCBinDirectory(CompileEnvironment.TargetPlatform), "cl.exe");
				CompileAction.bIsVCCompiler = true;
				CompileAction.CommandArguments = Arguments + FileArguments + CompileEnvironment.AdditionalArguments;
				CompileAction.StatusDescription = string.Format("{0}",Path.GetFileName(SourceFile.AbsolutePath));
				CompileAction.StatusDetailedDescription = SourceFile.Description;
				CompileAction.bShouldLogIfExecutedLocally = false;

				// Only tasks that don't use precompiled headers can be distributed with the current version of XGE.
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
            if (!BuildConfiguration.bRemoveUnusedHeaders)
            {
                LinkAction.CommandPath = Path.Combine(GetVCBinDirectory(LinkEnvironment.TargetPlatform), "link.exe");
            }

			// Get link arguments.
			LinkAction.CommandArguments = GetLinkArguments(LinkEnvironment);

			// Add delay loaded DLLs.
			if (LinkEnvironment.TargetPlatform == CPPTargetPlatform.Win32)
			{
				// Delay-load these DLLs.
				foreach (string DelayLoadDLL in LinkEnvironment.DelayLoadDLLs)
				{
					LinkAction.CommandArguments += string.Format(" /DELAYLOAD:\"{0}\"", DelayLoadDLL);
				}
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