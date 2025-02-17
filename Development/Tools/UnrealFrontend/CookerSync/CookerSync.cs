using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using System.IO;
using System.Xml.Serialization;
using System.Xml;

namespace CookerSync
{
	//class ConsoleLogger : IOutputHandler
	//{
	//    // IOutputHandler interface
	//    void CookerTools.IOutputHandler.OutputText(string Text)
	//    {
	//        Console.WriteLine(Text);
	//    }

	//    void CookerTools.IOutputHandler.OutputText(string Text, Color OutputColor)
	//    {
	//        // ignore the color
	//        Console.WriteLine(Text);
	//    }
	//};

	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class CookerSyncApp
	{
		const int ERR_CODE = 1;

		static void ShowUsage()
		{
			Console.WriteLine("Copies game content to one or more locations");
			Console.WriteLine("");
			Console.WriteLine("CookerSync <Name> [-Options] [-b <Dir>] [-p <Platform>] [-x <TagSet>] [Destinations...]");
			Console.WriteLine("");
			Console.WriteLine("  Name           : Name of the game to be synchronized");
			Console.WriteLine("  Destinations   : One or more targets (Xbox only) or UNC paths (any platform)");
			Console.WriteLine("                 : Destination required for PC or PS3. If Xbox360, default Xbox is used.");
			Console.WriteLine("");
			Console.WriteLine("Options:");
			Console.WriteLine("  -b, -base      : Specify base directory <Dir> to be used [Xbox only]");
			Console.WriteLine("                   If not specified defaults to: UnrealEngine3");
			Console.WriteLine("  -c, -crc       : Generate CRC when creating the TOC");
			Console.WriteLine("  -d, -demo      : Only include files with the IsForDemo flag set");
			Console.WriteLine("  -f, -force     : Force copying of all files regardless of time stamp");
			Console.WriteLine("  -h, -help      : This help text");
			Console.WriteLine("  -l, -log       : More verbose logging output");
			Console.WriteLine("  -m, -merge     : Use existing TOC file for CRCs");
			Console.WriteLine("  -n, -nosync    : Do not sync files, preview only");
			Console.WriteLine("  -nd, -nodest   : Ignore all destinations (useful for just generating TOCs");
			Console.WriteLine("  -p, -platform  : Specify platform <Platform> to be used");
			Console.WriteLine("                   Can be one of: Xbox360, Xenon, PS3 or PC [Default is Xbox360]");
			Console.WriteLine("  -r, -region    : Three letter code determining packages to copy [Default is INT]");
			Console.WriteLine("                 : Used for copying loc packages, all packages of the type _LOC.upk");
			Console.WriteLine("  -s, -sleep     : Specifies sleep (in ms) between each copy and verify to ease network hogging");
			Console.WriteLine("                 : Default is 25ms");
			Console.WriteLine("  -t, -target    : Only copy files with the 'IsForTarget' flag set");
			Console.WriteLine("  -v, -verify    : Verify that the CRCs match between source and destination");
			Console.WriteLine("                   Use in combination with -merge and/or -crc [copying to PC only]");
			Console.WriteLine("  -x, -tagset    : Only sync files in tag set <TagSet>. See CookerSync.xml for the tag sets.");
			Console.WriteLine("  -y, -synchost  : Connect to CookerSyncHost service for copying to a UNC share (\\\\server\\share\\UE3). Disables -v");
			Console.WriteLine("  -o, -reboot    : Reboot all targets before copying.");
			Console.WriteLine("  -allownotarget : Disable the error that is given by default when running without any valid targets.");
			Console.WriteLine("  -ni, -noninter : Non interactive mode; don't prompt the user for input.");
			Console.WriteLine("  -notoc         : Tells CookerSync to not save the Table of Contents (TOC) to disk.");

			Console.WriteLine("");
			Console.WriteLine("Examples:");
			Console.WriteLine("  Copy ExampleGame to the default xbox:");
			Console.WriteLine("\tCookerSync Example");
			Console.WriteLine("");
			Console.WriteLine("  Copy ExampleGame to the xbox named tango and xbox named bravo:");
			Console.WriteLine("\tCookerSync Example tango bravo");
			Console.WriteLine("");
			Console.WriteLine("  Copy ExampleGame for PS3 to the to PC path \\\\Share\\");
			Console.WriteLine("\tCookerSync Example -p PS3 \\\\Share\\");
			Console.WriteLine("");
			Console.WriteLine("  Copy ExampleGame to the xbox named tango and used UnrealEngine3-Demo as");
			Console.WriteLine("  the Xbox base directory:");
			Console.WriteLine("\tCookerSync Example tango -base UnrealEngine3-Demo");
			Console.WriteLine("");
			Console.WriteLine("  Copy ExampleGame to the PC Path C:\\DVD\\ generate CRCs for the Table of");
			Console.WriteLine("  contents and verify the CRC on the destination side:");
			Console.WriteLine("\tCookerSync Example -crc -v C:\\DVD\\");
			Console.WriteLine("");
			Console.WriteLine("  Preview the copy of ExampleGame to xbox named tango and PC path \\\\Share\\:");
			Console.WriteLine("\tCookerSync Example -n tango \\\\Share\\");
			Console.WriteLine("");
			Console.WriteLine("  Verify the copy of ExampleGame at C:\\DVD\\ without performing any copying:");
			Console.WriteLine("\tCookerSync Example -n -f -crc -v C:\\DVD\\");
			Console.WriteLine("");
			Console.WriteLine("  Verify the copy of ExampleGame at C:\\DVD\\ using the existing TOC file:");
			Console.WriteLine("\tCookerSync Example -v -m -crc -n -f C:\\DVD\\");
			Console.WriteLine("");
			Console.WriteLine("  Only generate the TOC with CRCs for ExampleGame:");
			Console.WriteLine("\tCookerSync Example -crc -nd");
			Console.WriteLine("");
			Console.WriteLine("  Merge CRCs from existing TOC file and generate CRCs for any missing files");
			Console.WriteLine("  or files that have mismatched lengths.  Write the resulting TOC to disk");
			Console.WriteLine("  without doing anything else:");
			Console.WriteLine("\tCookerSync Example -m -crc -n -nd");
		}

		static void Output(object sender, ConsoleInterface.OutputEventArgs e)
		{
			Console.Write(e.Message);
			System.Diagnostics.Debug.Write(e.Message);
		}

		/// <summary>
		/// Main entry point
		/// </summary>
		/// <param name="Args">Command line arguments</param>
		/// <returns></returns>
		[STAThread]
		static int Main(string[] Args)
		{
			// default to successful
			bool bWasSuccessful = true;

			Application.EnableVisualStyles();

			if(Args.Length == 0)
			{
				ShowUsage();
				return 1;
			}

			if(Args[0].StartsWith("-"))
			{
				ShowUsage();
				return 1;
			}
			string CWD = Environment.CurrentDirectory.ToLower();
			if(!CWD.EndsWith("binaries"))
			{
				Console.WriteLine("Error: CookerSync must be run from the 'Binaries' folder");
				return 1;
			}

			ConsoleInterface.TOCSettings PlatSettings = new ConsoleInterface.TOCSettings(new ConsoleInterface.OutputHandlerDelegate(Output));
			string PlatformToSyncName = "";
			string SyncTagSet = null;
			List<string> Languages = new List<string>();
			bool IgnoreDest = false;
			bool bRebootBeforeCopy = false;
			bool bNoTargetIsFailureCondition = true;
			bool bNonInteractive = false;

			// if any params were specified, parse them
			if(Args.Length > 1)
			{
				PlatSettings.GameName = Args[0];

				for(int ArgIndex = 1; ArgIndex < Args.Length; ArgIndex++)
				{
					if((String.Compare(Args[ArgIndex], "-b", true) == 0) || (String.Compare(Args[ArgIndex], "-base", true) == 0))
					{
						// make sure there is another param after this one
						if(Args.Length > ArgIndex + 1)
						{
							// the next param is the base directory
							PlatSettings.TargetBaseDirectory = Args[ArgIndex + 1];

							// skip over it
							ArgIndex++;
						}
						else
						{
							Console.WriteLine("Error: No base specified (use -h to see usage).");
							return ERR_CODE;
						}
					}
					else if((String.Compare(Args[ArgIndex], "-p", true) == 0) || (String.Compare(Args[ArgIndex], "-platform", true) == 0))
					{
						// make sure there is another param after this one
						if(Args.Length > ArgIndex + 1)
						{
							// the next param is the base directory
							PlatformToSyncName = Args[ArgIndex + 1];

							if(PlatformToSyncName.Equals("xenon", StringComparison.OrdinalIgnoreCase))
							{
								PlatformToSyncName = ConsoleInterface.PlatformType.Xbox360.ToString();
							}

							// skip over it
							ArgIndex++;
						}
						else
						{
							Console.WriteLine("Error: No platform specified (use -h to see usage).");
							return ERR_CODE;
						}
					}
					else if((String.Compare(Args[ArgIndex], "-r", true) == 0) || (String.Compare(Args[ArgIndex], "-region", true) == 0))
					{
						// make sure there is another param after this one
						if(Args.Length > ArgIndex + 1)
						{
							// the next param is the base directory
							Languages.Add(Args[ArgIndex + 1]);

							// skip over it
							ArgIndex++;
						}
						else
						{
							Console.WriteLine("Error: No region specified (use -h to see usage).");
							return ERR_CODE;
						}
					}
					else if((String.Compare(Args[ArgIndex], "-s", true) == 0) || (String.Compare(Args[ArgIndex], "-sleep", true) == 0))
					{
						// make sure there is another param after this one
						if(Args.Length > ArgIndex + 1)
						{
							try
							{
								PlatSettings.SleepDelay = Int32.Parse(Args[ArgIndex + 1]);
							}
							catch
							{
							}

							// skip over it
							ArgIndex++;
						}
						else
						{
							Console.WriteLine("Error: No sleep delay specified (use -h to see usage).");
							return ERR_CODE;
						}
					}
					else if((String.Compare(Args[ArgIndex], "-x", true) == 0) || (String.Compare(Args[ArgIndex], "-tagset", true) == 0))
					{
						// make sure there is another param after this one
						if(Args.Length > ArgIndex + 1)
						{
							// next param is the tagset
							SyncTagSet = Args[ArgIndex + 1];

							// skip over it
							ArgIndex++;
						}
						else
						{
							Console.WriteLine("Error: No sleep delay specified (use -h to see usage).");
							return ERR_CODE;
						}
					}
					else if((String.Compare(Args[ArgIndex], "-var", true) == 0))
					{
						// make sure there is another param after this one
						if(Args.Length > ArgIndex + 1)
						{
							string[] Tokens = Args[ArgIndex + 1].Split("=".ToCharArray());

							// make sure we split properly 
							if(Tokens.Length == 2)
							{
								PlatSettings.GenericVars.Add(Tokens[0], Tokens[1]);
							}

							// skip over it
							ArgIndex++;
						}
						else
						{
							Console.WriteLine("Error: No sleep delay specified (use -h to see usage).");
							return ERR_CODE;
						}
					}
					else if((String.Compare(Args[ArgIndex], "-c", true) == 0) || (String.Compare(Args[ArgIndex], "-crc", true) == 0))
					{
						PlatSettings.ComputeCRC = true;
					}
					else if((String.Compare(Args[ArgIndex], "-f", true) == 0) || (String.Compare(Args[ArgIndex], "-force", true) == 0))
					{
						PlatSettings.Force = true;
					}
					else if((String.Compare(Args[ArgIndex], "-h", true) == 0) || (String.Compare(Args[ArgIndex], "-help", true) == 0))
					{
						ShowUsage();
						return 0;
					}
					else if((String.Compare(Args[ArgIndex], "-l", true) == 0) || (String.Compare(Args[ArgIndex], "-log", true) == 0))
					{
						//VerboseOutput = true;
					}
					else if((String.Compare(Args[ArgIndex], "-m", true) == 0) || (String.Compare(Args[ArgIndex], "-merge", true) == 0))
					{
						PlatSettings.MergeExistingCRC = true;
					}
					else if((String.Compare(Args[ArgIndex], "-n", true) == 0) || (String.Compare(Args[ArgIndex], "-nosync", true) == 0))
					{
						PlatSettings.NoSync = true;
					}
					else if((String.Compare(Args[ArgIndex], "-nd", true) == 0) || (String.Compare(Args[ArgIndex], "-nodest", true) == 0))
					{
						IgnoreDest = true;
					}
					else if((String.Compare(Args[ArgIndex], "-t", true) == 0) || (String.Compare(Args[ArgIndex], "-target", true) == 0))
					{
						PlatSettings.IsForShip = true;
					}
					else if((String.Compare(Args[ArgIndex], "-v", true) == 0) || (String.Compare(Args[ArgIndex], "-verify", true) == 0))
					{
						PlatSettings.VerifyCopy = true;
					}
					else if((String.Compare(Args[ArgIndex], "-y", true) == 0) || (String.Compare(Args[ArgIndex], "-synchost", true) == 0))
					{
						PlatSettings.SyncToHost = true;
					}
					else if(string.Compare(Args[ArgIndex], "-o", true) == 0 || string.Compare(Args[ArgIndex], "-reboot", true) == 0)
					{
						bRebootBeforeCopy = true;
					}
					else if(string.Compare(Args[ArgIndex], "-allownotarget", true) == 0)
					{
						bNoTargetIsFailureCondition = false;
					}
					else if(string.Compare(Args[ArgIndex], "-ni", true) == 0 || string.Compare(Args[ArgIndex], "-noninter", true) == 0)
					{
						bNonInteractive = true;
					}
					else if("-notoc".Equals(Args[ArgIndex], StringComparison.OrdinalIgnoreCase))
					{
						PlatSettings.GenerateTOC = false;
					}
					else if(Args[ArgIndex].StartsWith("-"))
					{
						Console.WriteLine("Error: '" + Args[ArgIndex] + "' is not a valid option (use -h to see usage).");
						return ERR_CODE;
					}
					else
					{
						// is this a PC destination path?
						if(IsDirectory(Args[ArgIndex]))
						{
							PlatSettings.DestinationPaths.Add(Args[ArgIndex]);
						}
						else
						{
							PlatSettings.TargetsToSync.Add(Args[ArgIndex]);
							bWasSuccessful = true;
						}
					}
				}
			}

			if(Languages.Count > 0)
			{
				PlatSettings.Languages = Languages.ToArray();
			}
			else
			{
				PlatSettings.Languages = new string[] { "INT" };
			}

			ICollection<ConsoleInterface.Platform> Platforms = ConsoleInterface.DLLInterface.Platforms;
			ConsoleInterface.Platform PlatformToSync = null;

			if(PlatformToSyncName != null)
			{
				for(uint i = 1, CurPlatformBit = 1; i < (uint)ConsoleInterface.PlatformType.All; i = (i << 1) & 1, CurPlatformBit <<= 1)
				{
					ConsoleInterface.PlatformType CurPlatform = (ConsoleInterface.PlatformType)CurPlatformBit;

					if(CurPlatform.ToString().Equals(PlatformToSyncName, StringComparison.OrdinalIgnoreCase))
					{
						if(ConsoleInterface.DLLInterface.LoadPlatforms(CurPlatform) != CurPlatform || !ConsoleInterface.DLLInterface.TryGetPlatform(CurPlatform, ref PlatformToSync))
						{
							Console.WriteLine("Error: Could not load target platform \'{0}\'", PlatformToSyncName);
							bWasSuccessful = false;
						}

						break;
					}
				}

				if(PlatformToSync == null)
				{
					Console.WriteLine("Error: Platform \'{0}\' is not a valid platform!", PlatformToSyncName);
					bWasSuccessful = false;
				}
			}
			else
			{
				Console.WriteLine("Error: No platform name supplied!");
				bWasSuccessful = false;
			}

			// the only platform at the moment that needs to sync is xbox 360
			if(bWasSuccessful && PlatformToSync.NeedsToSync)
			{
				if(PlatformToSync.EnumerateAvailableTargets() == 0)
				{
					Console.WriteLine("Warning: The platform \'{0}\' does not have any targets.", PlatformToSyncName);
				}
			}

			// process handling platforms that sync, use the default target if needed
			if(bWasSuccessful && PlatformToSync.NeedsToSync && PlatSettings.TargetsToSync.Count == 0 && PlatSettings.DestinationPaths.Count == 0 && !IgnoreDest)
			{
				ConsoleInterface.PlatformTarget DefaultTarget = PlatformToSync.DefaultTarget;
				// is there a default console?
				if(DefaultTarget != null)
				{
					PlatSettings.TargetsToSync.Add(DefaultTarget.Name);
				}
				else if (bNoTargetIsFailureCondition)
				{
					Console.WriteLine("Error: No default target has been specified in the {0} target manager.", PlatformToSyncName);
					bWasSuccessful = false;
				}
			}

			// validate that we have a destination if needed
			if(PlatSettings.TargetsToSync.Count == 0 && PlatSettings.DestinationPaths.Count == 0 && !IgnoreDest && bNoTargetIsFailureCondition)
			{
				Console.WriteLine("Error: No destination found. Use -h for help.");
				bWasSuccessful = false;
			}

			if(bWasSuccessful)
			{
				// I hate doing this but I guess I should unless we want to change the cmd line all of the
				// build machines are using? (or at least the cmd line QA is using :p)
				if(!PlatSettings.GameName.EndsWith("Game", StringComparison.OrdinalIgnoreCase))
				{
					PlatSettings.GameName += "Game";
				}

				try
				{
					XmlSerializer Serializer = new XmlSerializer(typeof(ConsoleInterface.GameSettings));

					string EntryAssemblyLocation = System.Reflection.Assembly.GetEntryAssembly().Location;
					string EntryAssemblyDirectory = Path.GetDirectoryName( EntryAssemblyLocation );
					using (XmlReader Reader = XmlReader.Create( Path.Combine( EntryAssemblyDirectory, string.Format( "..\\{0}\\Build\\CookerSync_game.xml", PlatSettings.GameName ) ) ))
					{
						PlatSettings.GameOptions = (ConsoleInterface.GameSettings)Serializer.Deserialize(Reader);
					}
				}
				catch(Exception e)
				{
					Console.WriteLine(e.ToString());
					return ERR_CODE;
				}

				if(!Directory.Exists(Path.Combine(Directory.GetCurrentDirectory(), "..\\" + PlatSettings.GameName)))
				{
					Console.WriteLine("Error: '" + Args[0] + "' is not a valid game name.");
					bWasSuccessful = false;
				}
				else
				{
					try
					{
						// try to sync to the given xbox
						bWasSuccessful = PlatformToSync.TargetSync(PlatSettings, SyncTagSet, bRebootBeforeCopy, bNonInteractive);
					}
					catch(System.Exception e)
					{
						bWasSuccessful = false;
						Console.WriteLine(e.ToString());
					}
				}
			}

			// did we succeed?
			return bWasSuccessful ? 0 : ERR_CODE;
		}

		static bool IsDirectory(String Directory)
		{
			if(Directory.IndexOf('\\') == -1)
			{
				return false;
			}

			return true;
		}
	}
}
