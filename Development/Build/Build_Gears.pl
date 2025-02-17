#!/usr/bin/perl.exe

use File::DosGlob 'glob';
use File::DosGlob 'GLOBAL_glob';

############
# NOTE: This file is meant to be run using ActiveState perl.  The perl with cygwin doesn't work correctly 
###

########################################################################
# Added command-line functionality to enable/disable build behaviour from the command line.
#     - Default behaviour is to rebuild the engine and all games.
#     - Command-line args are case insensitive.
#     - Specifying "incbuild" controls incremental building instead of complete rebuilding.  Useful for quick builds.
#     - Specifying any game names will cause only those games to be built.  Game names are "war", "ut", "example".
#     - Example usage:
#          Build.pl                        Does a complete rebuild of everything (a la build machine).
#          Build.pl example incbuild       Does an incremental build of ExampleGame.  Great for engine dev testing.
#          Build.pl war                    Does a rebuild of WarGame (including dependencies, ie the engine).
#          Build.pl incbuild ut example    Does an incremental rebuild of ExampleGame and UTGame only.
########################################################################

########################################################################
# Developer settings

$useDeveloperSettings                    = '0';

#$developer_root_dir                      = 'E:\\Code\\UnrealEngine3\\';
#$developer_buildLogDir                   = 'E:\\Code\\BuildLogs\\';
#$developer_clientspec                    = 'DaveBurke_Code';
#$developer_owner                         = 'dave_burke';
#$developer_addrAnnounceChangeLogToCoders = 'dave.burke@epicgames.com';
#$developer_addrFail                      = 'dave.burke@epicgames.com';

$developer_root_dir                      = 'D:\\depot_epicgames\\UnrealEngine3\\';
$developer_buildLogDir                   = 'D:\\depot_epicgames\\UnrealEngine3\\BuildLogs\\';
$developer_clientspec                    = 'msweitzer-x-34-code';
$developer_owner                         = 'martin_sweitzer';
$developer_addrAnnounceChangeLogToCoders = 'msew@epicgames.com';
$developer_addrFail                      = 'msew@epicgames.com';


###############################################################################
# Build script debug flags for disabling various aspects of the build process.

$doSync              = 1;    # Sync to depot head revision?
$doCheckout          = 1;    # Perform actual file checkout?

$doBuild             = 1;    # Compile new build version?
$doBuildPC	     = 1;    # Build PC?  Effectively 0 if doBuild is 0.
$doBuildPCLTCG	     = 0;    # Build PC LTCG?  Effectively 0 if doBuild is 0.
$doBuildScript       = 1;    # Build script?  Effectively 0 if doBuild is 0.
$doBuildXenon	     = 1;    # Build Xenon?  Effectively 0 if doBuild is 0.
$doBuildXenonLTCG    = 1;    # build the xenon LTCG?  Effectively 0 if doBuild is 0.
$xenonXDKVersion     = 3529; # the version of the Xenon XDK

$doBuildStatsViewer  = 1;    # Build StatsViewer?  Effectively 0 if doBuild is 0.

$doMail                      = 1;  # Send success/fail emails?
$doMailToProjects            = 1;  # Send project-specific emails?  Effectively 0 if doMail is 0.
$doMailToTesters             = 1;  # Send email to testers?  Effectively 0 if doMail is 0.
$doAnnounceChangeLogToCoders = 1;  # Send email to $addrCoders with link to changelog?  Effectively 0 if do Mail is 0.

$doDocs              = 0;    # Generate documentation?
$doUE3Docs           = 0;    # Generate UE3 documentation?  Effectively 0 if doDocs is 0.
$doXenonDocs         = 0;    # Generate Xenon-specific documentation?  Effectively 0 if doDocs is 0.

$doDocNag            = 0;    # Do documentation nag?

$doSubmit            = 1;    # Submit back into depot?
$uploadSymbols	     = 1;    # Upload symbols to symbol server?

$doAQABuild          = 0;    # Whether we should do a QA build or not.  A QA build will do the following:  0) update the QA label 1) write out the version of the engine that was built to the QABuildInfo.ini 2) place the OnlyUseQABuildSemaphore.txt 

$doDelayedBuild      = 1;    # if we are doing delayedBuild we will not updated the latestUnrealEngine3 label and instead let a vetting process do that.

########################################################################
# Directory and program definitions.

# The root path on the build server is E:\Build_UE3\UnrealEngine3\.  If this
# is different than your path, then for testing you'll have to redefine
# $root_dir to your path name.

$root_dir					= 'C:\\Build_UE3\\UnrealEngine3-Gears\\';

$buildLogDir				= 'C:\\Build_UE3\\BuildLogs\\';                           # Dir to dump log files into
$buildLogLink               = '\\\\Build-Server-01\\BuildLogs\\';                        # Link to the log file dir.
$buildSyncBatchLink         = '\\\\Build-Server-01\\Builds\\SyncToUE3Builds\\';          # Link to the sync batch file dir.
$avarisBuildFile            = '\\\\Build-Server-01\\BuildFlags\\makeUC2build.txt';       # Link to the file that triggers Avaris builds.

##############################################################################
# Perforce options.

# $clientspec is the changelist client for all p4 operations performed by the
# build script.  Clients are only valid when running on their machine
# (e.g. 'UnrealEngine3Build' is only valid when the script is run on build-server).
# For testing, fill your own clientspec and cspecpath into the developer settings.

$clientspec       = 'UnrealEngine3Build-UberServer';
$buildclientspec  = 'UnrealEngine3Build-UberServer';        # The clientspec who's changes are searched for the last build.  Never changes, even in dev.
#$cspecpath        = 'C:\\Build_UE3\\UnrealEngine3';							
$owner            = 'build_machine';
$depot            = '//depot/UnrealEngine3-Gears/';

###############################################################################
# Mail options.

$blat                  = 'c:\\bin\\blat\\blat.exe';          # Command line SMTP mailer.
$mailserver            = "mercury.epicgames.net";

#NOTE:  -external lists include -internal lists 

$addrWarfareInternal   = 'QA@epicgames.com'; # 'warfare-list@epicgames.com';
$addrWarfareExternal   = 'wrbldnts@microsoft.com';  # 'warfare-external-list@epicgames.com';

$addrEnvyInternal      = 'QA@epicgames.com'; # 'envy-list@epicgames.com';
$addrEnvyExternal      = 'QA@epicgames.com'; # 'envy-external-list@epicgames.com';

$addrTesters                    = 'QA@epicgames.com';
#$addrTesters                    = 'msew@epicgames.com';
$addrAnnounceChangeLogToCoders  = 'coders-list@epicgames.com';
$addrFail                       = 'coders-list@epicgames.com';

$addrFrom              = 'build@epicgames.com';
$failSubject           = "GEARS BUILD FAILED";

$lineSpacer            = "--------------------------------------------------------------------------------\n";
$announceMailHeaderText;
$announceQAMailHeaderText;


###############################################################################
# Documentation options.

$doxygen             = 'c:\\bin\\doxygen\\doxygen.exe';          # Documentation generator.
$doxyfileUE3         = 'UnrealEngine3.dox';
$doxyfileXenon       = 'Xenon.dox';

$doxyUE3Logfile      = 'latestDoxygenOutput_UE3.log';
$doxyXenonLogfile    = 'latestDoxygenOutput_Xenon.log';

###############################################################################
# Build options

$devenv				 = 'c:\\program files\\microsoft visual studio .net 2003\\common7\\ide\\devenv.exe';

# Default to building all games.  buildGames and buildAllGames can be overridden on the command line.
@buildGames = ('War');	# List of games to build.  Consulted if buildAllGames = 0.
$buildAllGames = 0;								# Flag to indicate if all builds should be compiled.  Overrides buildGames list.
$incrementalBuild = 0;							# If 1 do incremental builds; if 0 (the default), do rebuilds.

###############################################################################
#

%categoryText =  ( "Code" => "CODE CHANGES",
                   "Art" => "ART CHANGES",
                   "Map" => "MAP CHANGES",
                   "ini" => "CONFIG CHANGES",
                   "int" => "LOCLALIZATION CHANGES",
                   "Sound" => "SOUND CHANGES",
                   "Documentation" => "DOCUMENTATION CHANGES",
                   "Misc" => "MISC CHANGES"
);

###############################################################################
# Categories for the changes.  Every file is matched to a specific category.
# Files not matched to an entry in file_cat will be assigned to the "Misc" category.

%file_cat = ("uc"      =>    "Code",
             "cpp"     =>    "Code",
             "h"       =>    "Code",
             "vcproj"  =>    "Code",
             "sln"     =>    "Code",
             "int"     =>    "int",
             "psh"     =>    "Code",
             "vsh"     =>    "Code",
             "xpu"     =>    "Code",
             "xvu"     =>    "Code",
             "pl"      =>    "Code",
             "bat"     =>    "Code",
             "hlsl"    =>    "Code",
             "usf"     =>    "Code",
             "ini"     =>    "ini",
             "ukx"     =>    "Art",
             "usx"     =>    "Art",
             "utx"     =>    "Art",
             "unr"     =>    "Art",
             "dds"     =>    "Art",
             "psd"     =>    "Art",
             "tga"     =>    "Art",
             "bmp"     =>    "Art",
             "pcx"     =>    "Art",
             "ase"     =>    "Art",
             "max"     =>    "Art",
             "raw"     =>    "Art",
             "xmv"     =>    "Art",
             "upk"     =>    "Art",
             "umap"    =>    "Map",
             "ut3"    =>     "Map",
             "war"     =>    "Map",
             "uax"     =>    "Sound",
             "xsb"     =>    "Sound",
             "xwb"     =>    "Sound",
             "xap"     =>    "Sound",
             "wav"     =>    "Sound",
             "wma"     =>    "Sound",
             "ogg"     =>    "Sound",
             "doc"     =>    "Documentation",
             "xls"     =>    "Documentation",
             "html"    =>    "Documentation",
);

%changelist;                              # This is where the changes will be assembled.
%changelistWarfare;
%changelistEnvy;
%changelistExampleGame;
%clarg;

$default_change                = 0;       # Did default.ini change?
$user_change                   = 0;       # Did defaultuser.ini change?

@checkedoutfiles;                         # List of all checked out files.

###############################################################################
# clock/unclock variables

$timer      = 0;
$timer_op   = "";

###############################################################################
# elapsedTime
# - Returns a string describing the elapsed time between to time() calls.
# - Input arg $_[0] is the difference between the time() calls.
###############################################################################

sub elapsedTime
{
	$seconds = $_[0];
	$minutes = $seconds / 60;
	$hours = int($minutes / 60);
	$minutes = int($minutes - ($hours * 60));
	$seconds = int($seconds - ($minutes * 60));
	return $hours." hrs ".$minutes." min ".$seconds." sec";
}

###############################################################################
# clock/unclock
# - Provides simple timing; a set of clock/unclock calls delimit a timing interval.
# - Both clock and unclock print start/end timing status to the output file handle.
# - clock/unclock blocks cannot be nested.  Unmatched or nested calls cause
#   warnings to be printed out.
###############################################################################

sub clock
{
	if ($timer_op ne "")
	{
		print "Warn: new clock interrupting previous clock for $timer_op!\n";
	}

	$timer_op = $_[0];
	$timer = time;
	print "Starting [$timer_op] at ".localtime($timer)."\n";
}


sub unclock
{
	if ($_[0] ne $timer_op)
	{
		print "Warn: unclock mismatch, $_[0] vs. $timer_op!\n";
	}
	else
	{
		print "Finished [$timer_op], elapsed: ".elapsedTime(time-$timer)."\n";
	}

	$timer_op = "";
}

###############################################################################
# incEngineVer
# - Increments the native ENGINE_VERSION define appearing in $unObjVercpp.
###############################################################################

sub incEngineVer()
{
    # Grab a copy of the current contents of $unObjVercpp,
	open( VERFILE, "$unObjVercpp" );
	@vertext = <VERFILE>;
	close( VERFILE );

    # Write $unObjVercpp back out, but with ENGINE_VERSION bumped up by one.
	open( VERFILE, ">$unObjVercpp" );
	foreach $verline ( @vertext )
	{
		$verline =~ s/\n//;   # Remove first newline occurance (hopefully at end of line...)
		if( $verline =~ m/^#define\sENGINE_VERSION\s([0-9]+)$/ )
		{
			$v = $1+1;
			$verline = "#define ENGINE_VERSION	$v";
		}
		print VERFILE "$verline\n";
	}

	close( VERFILE );


	open( VERFILE, "$unObjVercpp" );
	@vertext = <VERFILE>;
	close( VERFILE );

    # Write $unObjVercpp back out, but with BUILT_FROM_CHANGELIST set to the changelist that was used to build
	open( VERFILE, ">$unObjVercpp" );
	foreach $verline ( @vertext )
	{
		$verline =~ s/\n//;   # Remove first newline occurance (hopefully at end of line...)
		if( $verline =~ m/^#define\sBUILT_FROM_CHANGELIST\s([0-9]+)$/ )
		{
			$v = $builtFromChangelistNum;
			$verline = "#define BUILT_FROM_CHANGELIST $v";
		}
		print VERFILE "$verline\n";
	}

	close( VERFILE );
}


###############################################################################
# incr xboxlive build numbers
###############################################################################

sub incXboxLiveVer()
{
    # Grab a copy of the current contents of xex.xml,
	open( VERFILE, "$examplegameXexXml" );
	@vertext = <VERFILE>;
	close( VERFILE );

    # Write the file back out, but with build= bumped up by one.
	open( VERFILE, ">$examplegameXexXml" );
	foreach $verline ( @vertext )
	{
		$verline =~ s/\n//;   # Remove first newline occurance (hopefully at end of line...)
		#print "verline: $verline\n";

		if( $verline =~ m/build=\"([0-9]+)\"$/ )
		{
			$v = $1+1;
			$verline = "     build=\"$v\"";
		}
		print VERFILE "$verline\n";
	}

	close( VERFILE );


    # Grab a copy of the current contents of xex.xml,
	open( VERFILE, "$wargameXexXml" );
	@vertext = <VERFILE>;
	close( VERFILE );

    # Write the file back out, but with build= bumped up by one.
	open( VERFILE, ">$wargameXexXml" );
	foreach $verline ( @vertext )
	{
		$verline =~ s/\n//;   # Remove first newline occurance (hopefully at end of line...)
		#print "verline: $verline\n";

		if( $verline =~ m/build=\"([0-9]+)\"$/ )
		{
			$v = $1+1;
			$verline = "     build=\"$v\"";
		}
		print VERFILE "$verline\n";
	}

	close( VERFILE );
}


###############################################################################
# setVersion
# - Sets $version, $buildlog, and the various mail subjects.
###############################################################################

sub setVersion
{
	($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime();
        $versionTimeStamp = sprintf( "%04d-%02d-%02d_%02d.%02d", $year + 1900, $mon + 1, $mday, $hour, $min );
	$version = sprintf( "UnrealEngine3_Gears_Build_[$versionTimeStamp]" );

	$announceSubject = "New Gears of War version in P4: $version";
	$announceSubjectWarfare = $announceSubject;
	$announceSubjectEnvy = $announceSubject;
	$announceSubjectTesters = $announceSubject;
	$announceSubjectCoders = $announceSubject;


	$buildlogFilename = $version.".log";
	$buildlog = $buildLogDir.$buildlogFilename;
	$failedLogLink = $failedLogDirLink.$buildlogFilename;

	updateBVTTimestampFile()
}


###############################################################################
# updateBVTTimestampFile
# - Sets $version in the bvt timestamp .properties so we can have matching bvt runs
###############################################################################

sub updateBVTTimestampFile
{
    # Grab a copy of the current contents of xex.xml,
	open( VERFILE, "$bvtTimestampFile" );
	@vertext = <VERFILE>;
	close( VERFILE );

    # Write the file back out, but with build= bumped up by one.
	open( VERFILE, ">$bvtTimestampFile" );
	foreach $verline ( @vertext )
	{
		$verline =~ s/\n//;   # Remove first newline occurance (hopefully at end of line...)
		print "verline: $verline and $version\n";

		if( $verline =~ m/timestampForBVT=/ )
		{
			$v = $versionTimeStamp;
			$verline = "timestampForBVT=$v";
		}
		print VERFILE "$verline\n";
	}

	close( VERFILE );


    # Grab a copy of the current contents of build.properties,
	open( VERFILE, "$bvtTimestampFile" );
	@vertext = <VERFILE>;
	close( VERFILE );

    # Write the file back out, but with changelist this build was built from
	open( VERFILE, ">$bvtTimestampFile" );
	foreach $verline ( @vertext )
	{
		$verline =~ s/\n//;   # Remove first newline occurance (hopefully at end of line...)
		print "verline: $verline and $version\n";

		if( $verline =~ m/changelistBuiltFrom=/ )
		{
			$v = $builtFromChangelistNum;
			$verline = "changelistBuiltFrom=$v";
		}
		print VERFILE "$verline\n";
	}

	close( VERFILE );

}

###############################################################################
# setAnnounceMailHeaderText
# - Sets $announceMailHeaderText
###############################################################################
sub setAnnounceMailHeaderText
{

  if( $doAQABuild == 1 )
  {
    $announceQAMailHeaderText =

    "QA BUILD CANDIDATE  QA BUILD CANDIDATE  QA BUILD CANDIDATE  QA BUILD CANDIDATE\n\n".

    "This build is a QA Build Candidate.  Please sync to:  ".'@'."currentQABuildInTesting to test it\n\n".

    "QA BUILD CANDIDATE  QA BUILD CANDIDATE  QA BUILD CANDIDATE  QA BUILD CANDIDATE\n\n".

    "\n";

  }


  $announceMailHeaderText = 

    $announceQAMailHeaderText.
    "To use this build, sync to changelist:  $submittedChangeListNum\n".
    "\n$buildSyncBatchFile\n\n".
    "This build was labeled as: ".'@'."latestUnrealEngine3-Gears\n\n".
    "This build was built from changelist:  $builtFromChangelistNum\n\n".
    "The following platforms were built in this build:\n".
    "    PC:        $doBuildPC\n".
    "    PCLTCG:    $doBuildPCLTCG\n".
    "    Xenon:     $doBuildXenon\n".
    "    XenonLTCG: $doBuildXenonLTCG\n".
    "        Xenon XDK used: $xenonXDKVersion\n".
    "\n\nChanges since last version:\n\n";

}




###############################################################################
# getCheckoutList
# - Retrieves the checkout list from file and packs it in $checkoutFiles.
# - Returns 1 on success or 0 on fail.
###############################################################################

#add a docs .tx list shizzle
#	if( $doDocs == 1 && $doAQABuild == 1 )
#	{
#	    push @checkedoutfiles, $1;
#	    push @checkedoutfiles, $1;
#	}


sub getCheckoutList
{
	# Declare temporaries.
	my @checkoutPotentials;
	my @checkoutExcludePotentials;
	my @expandedPotentials;
	my @expandedExcludes;
	my $includedFile;
	my $excludedFile;
	my $bShouldBeExcluded;

	if ( open( CHECKOUTFILELIST, $checkoutfilelistfile ) )
	{
		# Grab the list of include paths.
		chomp( @checkoutPotentials = <CHECKOUTFILELIST> );     # Strip trailing newlines.
		close( CHECKOUTFILELIST );

		# Grab the list of exclude paths.
		if ( open( CHECKOUTFILEEXCLUDE, $checkoutfileexcludefile ) )
		{
			chomp( @checkoutExcludePotentials = <CHECKOUTFILEEXCLUDE> );
			close( CHECKOUTFILEEXCLUDE );
		}

		# Expand include paths.
		foreach $includedFile ( @checkoutPotentials )
		{
			#print "GLOBBING $root_dir$includedFile \n";
			push( @expandedPotentials, glob("$root_dir$includedFile") );
		}

		# Expand exclude paths.
		foreach $excludedFile ( @checkoutExcludePotentials )
		{
			#print "GLOBBING $root_dir$excludedFile \n";
			push( @expandedExcludes, glob("$root_dir$excludedFile") );
		}

		# Add non-excluded files to the global list.
		foreach $includedFile( @expandedPotentials )
		{
			$bShouldBeExcluded = 0;
			foreach $excludedFile ( @expandedExcludes )
			{
				if ( $includedFile eq $excludedFile )
				{
					$bShouldBeExcluded = 1;
					last;
				}
			}

			if ( !$bShouldBeExcluded )
			{
				#print "adding $includedFile \n";
				push @checkoutFiles, $includedFile;
			}
		}

		return 1;			# Report success.
	}

	return 0;				# Report failure.
}

###############################################################################
# revertAll
# - Discards all changes and reverts to the revisions from the last sync.
###############################################################################
# On successful revert, p4 output is:
# //depot/UnrealEngine3/Development/Build/dummy2.txt#1 - was edit, reverted
#
# On attempt to revert a file not open for edit, p4 output is:
# dummy.txt - file(s) not opened on this client.
#
sub revertAll
{
	# Revert and unlock if we had checked out files.
	if ( $doCheckout == 1 )
	{
		system( "p4 -c \"$clientspec\" revert -c default \/\/\.\.\. 2>&1" );
	}
}

###############################################################################
# checkoutAll
# - Opens the files in @checkoutFiles for editing, and lock them so that other
#   users can't submit until the build is finished.
# - Returns 1 on success or 0 on fail.
###############################################################################

sub checkoutAll
{
	foreach $file (@checkoutFiles)
	{
		#open( EDIT, "p4 -c \"$clientspec\" edit \"$cspecpath\\$file\" 2>&1 |");
		open( EDIT, "p4 -c \"$clientspec\" edit \"$file\" 2>&1 |");
		@edit = <EDIT>;
		close( EDIT );

		# If the file was successfully opened for edit, p4 output will be
		# //depot/UnrealEngine3/fileAndPath - opened for edit
		# If the file is already opened for edit by this client, p4 output will be 
		# //depot/UnrealEngine3/fileAndPath - currently opened for edit
		#
		# In both cases we succeed; so, match " opened for edit".  Previously, this check
		# matched "- opened for edit", which could potentially cause a build script
		# to fail if it was run after a previously failed build attempt.

		if( ! ($edit[0] =~ m/\ opened\ for\ edit$/) )
		{
			announceFailure("Perforce checkout","<no log file>");
			print "UE3 build failure: Checkout failed.\n\n$edit[0]";
			return 0;
		}
		else
		{
			# Open for edit was successful; add the file to the list of opened files.
			foreach $line (@edit)
			{
				if ( $line =~ /(.+)\#/ )
				{
					# Add file to the checked out list.
					print "checked out file:$1\n";  #TODO: comment this out again.
					push @checkedoutfiles, $1;
				}
			}
		}
	}

	# Try to lock all the files opened for edit.
	
	# !!! It's a bad idea to fail if locking doesn't succeed.  This is p4 considers a
	# lock attempt to have failed if the client already has a lock, which could
	# conceivably happen if the build script crashes in a bizarre way. !!!
	local $assertLockingSucceeds = 0;   # If 0, don't bother making sure that all files were locked.

	if ( $assertLockingSucceeds == 0 )
	{
		system( "p4 -c $clientspec lock 2>&1" );
	}
	else
	{
		open( LOCKSTATUS, "p4 -c $clientspec lock 2>&1 |" );
		local @output = <LOCKSTATUS>;
		close( LOCKSTATUS );

		# Count up the number of files that we tried to lock but couldn't.

		# If the file locks successfully, p4 output will be:
		# //depot/UnrealEngine3/fileAndPath - locking
		# If the file is already locked, p4 output will be:
		# //depot/UnrealEngine3/fileAndPath - already locked

		$numFilesAlreadyLocked = 0;

		foreach $line (@output)
		{
			if ( ! ($line =~ m/\-\ locking/) )
			{
				$numFilesAlreadyLocked++;
			}
		}
	    
		if ( $numFilesAlreadyLocked != 0 )
		{
			announceFailure( "Lock already exists on $numFilesAlreadyLocked file(s) in checkout list; Perforce checkout", "<no log file>" );
			print "Gears build failure: Checkout failed; $numFilesAlreadyLocked file(s) already locked\n\n";
			return 0;
		}
    }

	return 1;
}

###############################################################################
# submitAll
# - Unlocks all files listed in @checkedoutfiles and submits them to the depot.
# - Returns 1 on success or 0 on fail.
###############################################################################
# On successful submit, p4 output is:
# Submitting change 75545.
# Locking 1 files ...
# add //depot/UnrealEngine3/Development/Build/dummy2.txt#1
# Change 75545 submitted.
#
# Another successful submit:
# Submitting change 75605.
# Locking 2 files ...
# edit //depot/UnrealEngine3/Development/Build/dummy.txt#5
# delete //depot/UnrealEngine3/WarGame/Content/wargameContent.txt#2
# Change 75605 renamed change 75608 and submitted.
#
# On unsuccessful submit, p4 output is:
# Submitting change 75544.
# //depot/UnrealEngine3/Development/Build/dummy.txt - already locked
# File(s) couldn't be locked.
# Submit failed -- fix problems above then use 'p4 submit -c 75544'.

sub submitAll
{
	# Unlock the files for check in.  Even though this unlock looks redundant,
	# because your locks are released automatically on submission, if submit fails
	# then the locks won't be released, so we must manually unlock.
        # msew: this is bad as during the unlock I could edit and submit a file and cause a build failure
        #       if the build fails then we need to a) revert
	#system( "p4 -c $clientspec unlock 2>&1" );

	# Revert unchanged files.
	# No need to revert unchanged files.  Submitting unchanged files does no harm. 
        # msew: we want to revert unchanged files as binary files take up their compressed size
        #       we also want to revert unchanged files as it makes looking at the history of changes in the file impossible
        #     But we CAN NOT do this.  the @checkedoutfiles list contains the set of files we are
        #     expecting to check in.  This stops us from reverting unchanged files
	#system( "p4 -c $clientspec revert -a 2>&1" );

	open( SUBMITCMD, "|p4 -c \"$clientspec\" submit -i > output.txt 2>&1 " );

	print SUBMITCMD "Change:\tnew\n";
	print SUBMITCMD "Client:\t$clientspec\n";
	print SUBMITCMD "Status:\tnew\n";		
	print SUBMITCMD "Description:\t$version\n built from changelist:  $builtFromChangelistNum";

	if( $doAQABuild == 1 )
	{
	    print SUBMITCMD "  QA BUILD CANDIDATE";
	}

	print SUBMITCMD "\n";
		
	print SUBMITCMD "Files:\n";

	foreach $file (@checkedoutfiles)
	{
		print SUBMITCMD "\t$file#edit\n";
	}
	
	close( SUBMITCMD );

	# Was the submit successful?
	local $foundSuccess = 0;
	local $accumSubmitLog = "";
	if (open(OUTPUT,"output.txt") )
	{
		foreach $line (<OUTPUT>)
		{
			$accumSubmitLog .= $line;
			if ( $line =~ m/([0-9]+) submitted/i )
			{
			    $submittedChangeListNum = $1;
				$foundSuccess = 1;
				last;
			}
			elsif ( $line =~ m/([0-9]+) and submitted/i )		# In the case of Perforce renaming.
			{
			    $submittedChangeListNum = $1;
				$foundSuccess = 1;
				last;
			}
		}
		close( OUTPUT );
	}
	else
	{
		$foundSuccess = 1;
	}

	if ( $foundSuccess == 0 )
	{
		$failLogName = "submitFail_".$version.".log";
		open( FAILLOG, "> $failLogName" );
		print FAILLOG $accumSubmitLog;
		print FAILLOG "Probable cause: game/script compile error prevented the generation of a file that was flagged for submission.\n";
		print FAILLOG "See compile logs to verify.\n";
		close( FAILLOG );
		system("copy $failLogName $failedLogDir /y" );
		announceFailure( "Perforce submit (untrapped compile fail?)", $failedLogDirLink.$failLogName );
	}

	return $foundSuccess;
}

###############################################################################
# createLabels
# - Creates the latestUnrealEngine3 label.
###############################################################################

sub createLabels
{
	# Unique version labels are needed for the delayed LD build process
	if( 1 )
	{
	    #$labelVersion = sprintf( "$version\_$builtFromChangelistNum" );
	    $labelVersion = sprintf( "$version" );
	    # Create the version label.
	    system( "" );
	    open( LABELCMD, "|p4 -c \"$clientspec\" label -i" );
	    print LABELCMD "Label:\t$labelVersion\n";
	    print LABELCMD "Owner:\t$owner\n";
	    print LABELCMD "Description:\n\t$version Built from changelist:  $builtFromChangelistNum\n";		
	    print LABELCMD "Options:\tunlocked\n";
	    print LABELCMD "View:\n\t\"$depot\...\"\n";
	    close( LABELCMD );
	    system( "p4 -c \"$clientspec\" labelsync -l $labelVersion" );
	}


	if( $doAQABuild == 1 )
	{
	    # Create the currentQABuildInTesting label.
	    #system( " -c \"$clientspec\" label -d currentQABuildInTesting" );
	    open( LABELCMD, "p4 -c \"$clientspec\" label -i" );
	    print LABELCMD "Label:\tcurrentQABuildInTesting\n";
	    print LABELCMD "Owner:\t$owner\n";
	    print LABELCMD "Description:\n\t$version\n\nBuilt from changelist:  $builtFromChangelistNum\n";		
	    print LABELCMD "Options:\tunlocked\n";
	    print LABELCMD "View:\n\t\"$depot\...\"\n";
	    close( LABELCMD );
	    system( "p4 -c \"$clientspec\" labelsync -l currentQABuildInTesting" );
	}


	if( ( $doBuildPC == 1 )
	    && ( $doBuildXenon == 1 )
	  )
	{
	    # Create the latestUnrealEngine3-PCAndXenon-Gears label.
	    #system( " -c \"$clientspec\" label -d latestUnrealEngine3-PCAndXenon-Gears" );
	    open( LABELCMD, "p4 -c \"$clientspec\" label -i" );
	    print LABELCMD "Label:\tlatestUnrealEngine3-PCAndXenon-Gears\n";
	    print LABELCMD "Owner:\t$owner\n";
	    print LABELCMD "Description:\n\t$version\n\nBuilt from changelist:  $builtFromChangelistNum\n";		
	    print LABELCMD "Options:\tunlocked\n";
	    print LABELCMD "View:\n\t\"$depot\...\"\n";
	    close( LABELCMD );
	    system( "p4 -c \"$clientspec\" labelsync -l latestUnrealEngine3-PCAndXenon-Gears" );
	}

	if( $doDelayedBuild == 0 )
	{
	    # Create the latestUnrealEngine3-Gears label.
	    #system( " -c \"$clientspec\" label -d latestUnrealEngine3-Gears" );
	    open( LABELCMD, "p4 -c \"$clientspec\" label -i" );
	    print LABELCMD "Label:\tlatestUnrealEngine3-Gears\n";
	    print LABELCMD "Owner:\t$owner\n";
	    print LABELCMD "Description:\n\t$version\n\nBuilt from changelist:  $builtFromChangelistNum\n";		
	    print LABELCMD "Options:\tunlocked\n";
	    print LABELCMD "View:\n\t\"$depot\...\"\n";
	    close( LABELCMD );
	    system( "p4 -c \"$clientspec\" labelsync -l latestUnrealEngine3-Gears" );
	  }
}

###############################################################################
# getDepotChanges
# - Gets a list of all changes from perforce and packs them in @depotChanges.
###############################################################################
sub getDepotChanges
{
# TEST: undo this after testing!!!!!!!!!
if ( 1 ) {
	open( CHANGES,"p4 changes \"$depot...\"|" );
	@depotChanges = <CHANGES>;
	close( CHANGES );
	} else {
	push @depotChanges, "Change 77669 on 2005/01/12 by Ray_Davis@Epic_Ray-Pentium4_Code \'- added Object.Split() for spli\'";
	#push @depotChanges, "Change 77770 on 2005/01/13 by lee_dotson@LEEDOT-P4_Build '- replaced a texture that was a'";
	}
}

###############################################################################
# assembleChangesSinceLastBuild
# - Reads changes since last build and assembles them in @changelist
###############################################################################

sub assembleChangesSinceLastBuild
{
	###################################
	# Scan through the changes to find the last build change.

	$foundlastbuild = 0;    # Flag indicating whether or not we've found a last build.
	$lastbuildchange = 0;   # Where we'll store the changeNum of the last build change.
	$lastbuildtime = 0;     # Where we'll store the time of the last build change.

	# This p4 command uses buildclientspec instead of clientspec because the last
	# committed build will have been made by the build machine, not the developer
	# clientspec.
	open( CHANGES, "p4 changes -c \"$buildclientspec\" \"$depot...\"|" );
	foreach $line (<CHANGES>)
	{
		# Is this a build version change?
		if ( $line =~ /^Change ([0-9]+) on ([0-9]{4})\/([0-9]{2})\/([0-9]{2}).+UnrealEngine3_Gears_Build_/ )
		{
			$lastbuildchange = $1;
			print "Last known build on $2/$3/$4\n";

			# Read the actual build time; the format is UnrealEngine3_Build_[2004-07-30_15.56]
			open (BUILDDESC,"p4 describe $lastbuildchange|");
			foreach $line (<BUILDDESC>)
			{
				if ($line =~ m/UnrealEngine3_Build_\[([0-9]{4})-([0-9]{2})-([0-9]{2})_([0-9]{2})\.([0-9]{2})\]/)
				{
					$lastbuildtime = $1.$2.$3.$4.$5;
					print "Build time: $lastbuildtime\n";
					last;
				}
			}
			close (BUILDDESC);

			# We've found a build version change; stop searching.
			$foundlastbuild = 1;
			last;
		}
	}
	close(CHANGES);

	# Break out if there were no build changes.
	if ($foundlastbuild == 0)
	{
		print "ERROR: failed to find last build version!\n";
		return 0;
	}

	###################################
    # Open up the changes for this depot.

	#open (CHANGES,"p4 changes \"$depot...\"|");
	#foreach $change (<CHANGES>)
	getDepotChanges;

	$builtFromChangelistNum = 0;
	$submittedChangeListNum = 0;

	foreach $change ( @depotChanges )
	{
		#print "$change\n";
		# Grab the change number.
		if ($change =~ /Change ([0-9]+) on/)
		{
			$change_num = $1;
			if ($change_num == $lastbuildchange)
			{
				# Exclude the build from the changelist.
				skip;
			}
			elsif ($change_num < $lastbuildchange)
			{
				if ($lastbuildtime == 0)
				{
					last;
				}
				# Check the actual time to make sure it wasn't submitted in between build submissions.
				else
				{
					$changetime = 0;

					open (DESCRIBE,"p4 describe $change_num|");
					foreach $line (<DESCRIBE>)
					{
						# Format is "on 2004/07/30 16:04:51".
						if ($line =~ m/on ([0-9]{4})\/([0-9]{2})\/([0-9]{2}) ([0-9]{2})\:([0-9]{2})/)
						{
							$changetime = $1.$2.$3.$4.$5;
							# No more changes.
							last;
						}
					}
					close (DESCRIBE);

					# If the actual time is before the last buildtime, ignore the change.
					if ($changetime == 0 || $changetime < $lastbuildtime)
					{
						# No more changes.
						#if ( $change_num != 77669 )   # TEST: comment this out!!!!!!!
						#{
							last;
						#}
					}
				}
			}  # end elsif ($change_num < $lastbuildchange)


			# Store off the latest valid change number.  To be included in build mails in place of unqiue version labels.
			if ( $builtFromChangelistNum < $change_num )
			{
				$builtFromChangelistNum = $change_num;
			}

			# Store off this changeNum for when we do the documentation nag later.
			push @validChangeNums, $change_num;

			# We're going to build a list of categories, as specified in %file_cat, that this change belongs to.
			%belongs_in = ();

			# Parse the description of this change.
			$reading_desc = 1;            # Flag indicating whether we're still reading the description of this change.
			$change_header = "";          # Where non-engine changes will be accumulated.
			$desc = "";                   # Where engine changes will be accumulated.
			$project_desc = "";           # Project specific description.
			$reading_project = 0;         # 0 = engine (default), 1 = Warfare, 2 = Envy, 3 = ExampleGame.
			
			# It may be the case that a change description affects files of various types (content, game code, engine code,...).
			# If that's the case, it'll look something like:
			#        - engine code changes description
			#        # Warfare
			#        - warfare change descriptions go down here.
			# So, all description lines that we see above a # Gamename line are potential engine changes.  We'll know
			# for sure if we don't see any /Content/ lines amongst the affected files.
			$sawPotentialEngineLine = 0;  # Flag indicating whether or not there are potential engine change lines.

			# Flags indicating if the change has affected a file in project\Content.
			# The flag is set to 1 if the change affects the project or 0 otherwise.
			# $contentChange[0] = <never used>
			# $contentChange[1] = Warfare\Content
			# $contentChange[2] = UTGame\Content
			# $contentChange[3] = ExampleGame\Content
			@contentChange = ( 0, 0, 0, 0 );
			local $wasContentChange = 0;

			open( DESCRIBE, "p4 describe $change_num|" );
			foreach $line (<DESCRIBE>)
			{
				# Has the change affected a project?
				if ( ($line =~ /#\s*Warfare/i) || ($line =~ /#\s*Warefare/i) || ($line =~ /#\s*WarGame/i) )
				{
					#print "saw a line containing #Warfare\n";
					$reading_project = 1;
				}
				elsif ($line =~ /#\s*Envy/i)
				{
					#print "saw a line containing #Envy\n";
					$reading_project = 2;
				}
				elsif ($line =~ /#\s*ExampleGame/i)
				{
					#print "saw a line containing #ExampleGame\n";
					$reading_project = 4;
				}
				# If we're still reading the changes description, watch for the end of the desc.
				# We'll know the end has been hit when we transition to the "Affected files" block of p4's 'describe' output.
				elsif ($reading_desc == 1)
				{
					if ($line =~ /Affected files/)
					{
						# We've reached the end of the changes.
						$reading_desc = 0;
					}
					else
					{
						# Only append the line to the list of changes if it contains non-whitespace chars.
						if ($line =~ /\w/)
						{
							# If this isn't the first line of the change ("Change 123 by...") and we haven't
							# seen a #projectName tag yet, this comment could be an engine change . . .
							if ( !($line =~ /Change $change_num by/) && $reading_project == 0 )
							{
								#print "saw a line maybe for engine: $line";
								$sawPotentialEngineLine = 1;
							}

							if ($line =~ /Change\s*(\d+)/)
							{
								$change_header .= $line;
							}
							# Store non-engine changes in a separate list.
							elsif ($reading_project != 0)
							{
								$project_desc .= $line;
							}
							else
							{
								$desc .= $line;
							}
						}
					}
				} #end elsif ($reading_desc == 1)
				else
				{
					# Check to see if this line contains a file ref.
					if ($line =~ /\.(\w+)\#/)
					{
						local $fileExtension = $1;       # Save off the file extension.
						#print "fileExtention is $fileExtension \n";
						#print "line is $line";
						push @fileChanges, $line;

						# Scan the patch for project\Content changes and set the appropriate contentChange flag.
						if ( $line =~ /WarGame\/Content/i )
						{
							#print "saw WarGameContent change: $line";
							$sawPotentialEngineLine = 0;      # Content change disables engine flag.
							$contentChange[1] = 1;
							$wasContentChange = 1;
						}
						elsif( $line =~ /UTGame\/Content/i )
						{
							#print "saw UTGameContent change: $line";
							$sawPotentialEngineLine = 0;      # Content change disables engine flag.
							$contentChange[2] = 1;
							$wasContentChange = 1;
						}
						elsif( $line =~ /ExampleGame\/Content/i )
						{
							#print "saw ExampleGameContent change: $line";
							$sawPotentialEngineLine = 0;      # Content change disables engine flag.
							$contentChange[4] = 1;
							$wasContentChange = 1;
						}
	
						#print "extension is $fileExtension \n";

						# Associate the file extension with its category, or "Misc" if none was found.
						$category = $file_cat{lc($fileExtension)};
						if ($category eq "")
						{
							$category = "Misc";
						}

						# Mark this category as one we belong in.
						$belongs_in{$category} += 1;    # Some arbitrary value.

						# Check if it's a critical change.
						if ($default_change == 0 && $line =~ /Default\.ini/)
						{
							$default_change = 1;
						}
						elsif ($user_change == 0 && $line =~ /DefUser\.ini/)
						{
							$user_change = 1;
						}
					}
				}
			} #end foreach $line (<DESCRIBE>)
			close (DESCRIBE);

            # For each category the change belongs to, prepend the $change_header to the description.
            # Then, add the description followed by a newline to the changelist for that category.
			if ($desc ne "" )
			{
				$desc = $change_header.$desc;

				# If the change is going to be reported as a project\Content change, don't bother
				# repeating the description as belonging to a category.

				$best_cnt = 0;
				$best_cat = "Misc";
				foreach $cat (keys(%belongs_in))
				{
					if ($belongs_in{$cat} > $best_cnt)
					{
						$best_cnt = $belongs_in{$cat};
						$best_cat = $cat;
					}
				}

				if ( $wasContentChange == 1 )
				{
					# If it was a content change, prepend the category to the description.
					#$desc = $categoryText{$best_cat}.":\n\n".$desc;

					if ( $contentChange[1] == 1 )    {$changelistWarfare{$best_cat} .= $desc."\n";}
					elsif ( $contentChange[2] == 1 ) { $changelistEnvy{$best_cat} .= $desc."\n";}
					elsif ( $contentChange[4] == 1 ) { $changelistExampleGame{$best_cat} .= $desc."\n";}
				}
				else
				{
					# It wasn't a content change.  Add it to the bulk changelist for that category.
					$changelist{$best_cat} .= $desc."\n";
				}
			}

			if ($project_desc ne "")
			{
				$project_desc = $change_header.$project_desc;
				if ($reading_project == 1)
				{
      				$changelist{"Warfare"} .= $project_desc."\n";
				}
				elsif ($reading_project == 2)
				{
					$changelist{"Envy"} .= $project_desc."\n";
				}
				elsif ($reading_project == 3)
				{
					$changelist{"ExampleGame"} .= $project_desc."\n";
				}
			}

			# Append the project change to the project\content description.
			if ( $contentChange[1] == 1 )
			{
				if ( $desc ne "" ) { $changelist{"WarfareContent"} .= $desc."\n"; }
				#if ( $project_desc ne "" ) { $changelist{"WarfareContent"} .= $project_desc."\n"; }
			}

			if ( $contentChange[2] == 1 )
			{
				if ( $desc ne "" ) { $changelist{"EnvyContent"} .= $desc."\n"; }
				#if ( $project_desc ne "" ) { $changelist{"EnvyContent"} .= $project_desc."\n"; }
			}

			if ( $contentChange[3] == 1 )
			{
				if ( $desc ne "" ) { $changelist{"ExampleGameContent"} .= $desc."\n"; }
				#if ( $project_desc ne "" ) { $changelist{"ExampleGameContent"} .= $project_desc."\n"; }
			}

		} #end if ($change =~ /Change ([0-9]+) on/)
	} # end foreach $change (<CHANGES>)

	# All done!
	#close (CHANGES);
	return 1;
}

###############################################################################
# belongstodepot
# - Returns 1 if the change specified on arg $_[0] was from $depot, or 0 if not.
###############################################################################

#sub belongstodepot
#{
#	$changenum = $_[0];
#	$result = 0;
#	open(DESCRIBE, "p4 describe $changenum|");
#	foreach $line (<DESCRIBE>)
#	{
#		if ($line =~ /$depot/)
#		{
#			$result = 1;
#			last;
#		}
#	}
#	close(DESCRIBE);
#	return $result;
#}

###################################################################################################
#
# Subroutines for mailing and logging.
#
# The mailing procedures open a mail pipe which gets redirected to output.txt.
# output.txt is going to get parsed
#
###################################################################################################

sub announceToQAPeopleThatAQABuildIsBuilding
{
	$attempts = 0;
	$success = 0;
	while ($success == 0 && $attempts < 10)
	{
		# Open a mail pipe.  The pipe is redirected to output.txt so that we can
		# check for success by testing for the presence of output.txt
		open( SMTP, "|\"$blat\" - -t $addrTesters -f $addrFrom -s \"New QA Build has been triggered\" -server $mailserver > output.txt" );

		print SMTP "New QA Build has been triggered\n\n";

		close( SMTP );


		# Make sure we were able to send the mail.
		if (open(OUTPUT,"output.txt"))
		{
			$errors = 0;
			foreach $line (<OUTPUT>)
			{
				if ($line =~ m/Error:/i)
				{
					chomp($line);
					$errors++;
				}
			}
			$success = $errors == 0 ? 1 : 0;
			close(OUTPUT);
		}
		else
		{
			$success = 1;
		}

		# If we failed, delay a few seconds before attempting again.
		if ($success == 0)
		{
			print "Unable to send mail, retrying in 15 seconds...\n";
			sleep(15);
		}
		$attempts++;
	}

	# Notify that we weren't able to send the mail.
	if ($success == 0)
	{
		print "Failed to send mail after $attempts attempts!\n";
	}
}



sub announceToQAPeopleThatABuildIsBuilding
{
	$attempts = 0;
	$success = 0;
	while ($success == 0 && $attempts < 10)
	{
		# Open a mail pipe.  The pipe is redirected to output.txt so that we can
		# check for success by testing for the presence of output.txt
		open( SMTP, "|\"$blat\" - -t $addrTesters -f $addrFrom -s \"New Gears Build has been triggered\" -server $mailserver > output.txt" );

		print SMTP "New Gears Build has been triggered\n\n";

		close( SMTP );


		# Make sure we were able to send the mail.
		if (open(OUTPUT,"output.txt"))
		{
			$errors = 0;
			foreach $line (<OUTPUT>)
			{
				if ($line =~ m/Error:/i)
				{
					chomp($line);
					$errors++;
				}
			}
			$success = $errors == 0 ? 1 : 0;
			close(OUTPUT);
		}
		else
		{
			$success = 1;
		}

		# If we failed, delay a few seconds before attempting again.
		if ($success == 0)
		{
			print "Unable to send mail, retrying in 15 seconds...\n";
			sleep(15);
		}
		$attempts++;
	}

	# Notify that we weren't able to send the mail.
	if ($success == 0)
	{
		print "Failed to send mail after $attempts attempts!\n";
	}
}



##############################################################################
# announceToQAPeople
# - sends out email to the people that need
##############################################################################

sub announceToQAPeople
{
	$attempts = 0;
	$success = 0;
	while ($success == 0 && $attempts < 10)
	{
		# Open a mail pipe.  The pipe is redirected to output.txt so that we can
		# check for success by testing for the presence of output.txt
		open( SMTP, "|\"$blat\" - -t $addrTesters -f $addrFrom -s \"New QA Build has been built\" -server $mailserver > output.txt" );

		print SMTP "New QA Build has been built\n\n";
#		print SMTP "
#        ___________    ____ 
#      ______/   \__//   \__/____\ 
#    _/   \_/  :           //____\\ 
#   /|      :  :  ..      /        \ 
#  | |     ::     ::      \        / 
#  | |     :|     ||     \ \______/ 
#  | |     ||     ||      |\  /  | 
#   \|     ||     ||      |   / | \ 
#    |     ||     ||      |  / /_\ \ 
#    | ___ || ___ ||      | /  /    \ 
#     \_-_/  \_-_/ | ____ |/__/      \ 
#                  _\_--_/    \      / 
#                 /____             / 
#                /     \           / 
#                \______\_________/ 
#
#";

		print SMTP "IT IS GO TIME!";
		close( SMTP );


		# Make sure we were able to send the mail.
		if (open(OUTPUT,"output.txt"))
		{
			$errors = 0;
			foreach $line (<OUTPUT>)
			{
				if ($line =~ m/Error:/i)
				{
					chomp($line);
					$errors++;
				}
			}
			$success = $errors == 0 ? 1 : 0;
			close(OUTPUT);
		}
		else
		{
			$success = 1;
		}

		# If we failed, delay a few seconds before attempting again.
		if ($success == 0)
		{
			print "Unable to send mail, retrying in 15 seconds...\n";
			sleep(15);
		}
		$attempts++;
	}

	# Notify that we weren't able to send the mail.
	if ($success == 0)
	{
		print "Failed to send mail after $attempts attempts!\n";
	}

}


###############################################################################
# announceToTesters
# - Sends out changelist to the testers.
###############################################################################

sub announceToTesters
{
	$attempts = 0;
	$success = 0;
	while ($success == 0 && $attempts < 10)
	{
		# Open a mail pipe.  The pipe is redirected to output.txt so that we can
		# check for success by testing for the presence of output.txt
		open( SMTP, "|\"$blat\" - -t $addrTesters -f $addrFrom -s \"$announceSubjectTesters\" -server $mailserver > output.txt" );

		setAnnounceMailHeaderText;
		print SMTP "$announceMailHeaderText";


		# If there was a critical change, mark in the build notes.
		if ($default_change == 1)
		{
			print SMTP "NOTE: Default.ini has changed!\n\n";
		}
		if ($user_change == 1)
		{
			print SMTP "NOTE: DefUser.ini has changed!\n\n";
		}

		# Only dump code changes.
		if ($changelist{"Code"} ne "")
		{
			print SMTP "CODE CHANGES:\n\n";
			print SMTP $changelist{"Code"}."\n";
		}
		else
		{
			print SMTP "No code changes.\n";
		}
		close(SMTP);

		# Make sure we were able to send the mail.
		if (open(OUTPUT,"output.txt"))
		{
			$errors = 0;
			foreach $line (<OUTPUT>)
			{
				if ($line =~ m/Error:/i)
				{
					chomp($line);
					$errors++;
				}
			}
			$success = $errors == 0 ? 1 : 0;
			close(OUTPUT);
		}
		else
		{
			$success = 1;
		}

		# If we failed, delay a few seconds before attempting again.
		if ($success == 0)
		{
			print "Unable to send mail, retrying in 15 seconds...\n";
			sleep(15);
		}
		$attempts++;
	}

	# Notify that we weren't able to send the mail.
	if ($success == 0)
	{
		print "Failed to send mail after $attempts attempts!\n";
	}
}

###############################################################################
# assembleHashChangeLog
# - Accepts input hash from global hash named clarg.
# - Assembles the changes into $commonChangeLog.
# - The flag on $_[0] controls whether overrideSawPotentialEngineLine (0 - no, 1 - yes).
# - The flag on $_[1] controls whether ExampleGame changes are included (0 - no, 1 - yes).
###############################################################################

sub assembleHashChangeLog
{
	$commonChangeLog = "";
	my $overrideSawPotentialEngineLine = $_[0];
	my $includeExampleGame = $_[1];

	# If there was a critical change, mark in the build notes.
	if ($default_change == 1)
	{
		$commonChangeLog .= "NOTE: Please delete Unreal.ini, otherwise the game may not run correctly!\n\n";
	}
	if ($user_change == 1)
	{
		$commonChangeLog .= "NOTE: Please delete User.ini, otherwise the game may not run correctly!\n\n";
	}

	############################
	# Output common changes.

	if ($clarg{"Code"} ne "")
	{
		if ( $overrideSawPotentialEngineLine == 1 || $sawPotentialEngineLine == 1 )
		{
			$commonChangeLog .= "ENGINE CODE CHANGES:\n\n".$clarg{"Code"}."\n";
		}
	}

	if ($clarg{"Map"} ne "")
	{
		$commonChangeLog .= "MAP CHANGES:\n\n".$clarg{"Map"}."\n";
	}

	if ($clarg{"Art"} ne "")
	{
		$commonChangeLog .= "ART CHANGES:\n\n".$clarg{"Art"}."\n";
	}

	if ($clarg{"Sound"} ne "")
	{
		$commonChangeLog .= "SOUND CHANGES:\n\n".$clarg{"Sound"}."\n";
	}

	if ($clarg{"ini"} ne "")
	{
		$commonChangeLog .= "CONFIG CHANGES:\n\n".$clarg{"ini"}."\n";
	}

	if ($clarg{"int"} ne "")
	{
		$commonChangeLog .= "LOCALIZATION CHANGES:\n\n".$clarg{"int"}."\n";
	}

	if ($clarg{"Documentation"} ne "")
	{
		$commonChangeLog .= "DOCUMENTATION CHANGES:\n\n".$clarg{"Documentation"}."\n";
	}

	if ($clarg{"Misc"} ne "")
	{
		$commonChangeLog .= "MISC CHANGES:\n\n".$clarg{"Misc"}."\n";
	}


	############################
	# Output ExampleGame changes.

	if ( $includeExampleGame == 1 )
	{
		if ($clarg{"ExampleGame"} ne "")
		{
			$commonChangeLog .= $lineSpacer."EXAMPLEGAME CODE CHANGES:\n\n".$clarg{"ExampleGame"}."\n";
		}

		if ($clarg{"ExampleGameContent"} ne "")
		{
			$commonChangeLog .= $lineSpacer."EXAMPLEGAME CONTENT CHANGES:\n\n".$clarg{"ExampleGameContent"}."\n";
		}
	}

}

###############################################################################
# wasMailSendSuccessful
# - returns 1 if mail send was successful or 0 on fail.
###############################################################################

sub wasMailSendSuccessful
{
	local $success = 1;

	# Make sure we were able to send the mail.
	if (open(OUTPUT,"output.txt"))
	{
		$errors = 0;
		foreach $line (<OUTPUT>)
		{
			if ($line =~ m/Error:/i)
			{
				chomp($line);
				$errors++;
			}
		}
	
		$success = $errors == 0 ? 1 : 0;
		close(OUTPUT);
	}

	return $success;
}
		
###############################################################################
# announceWarfareChanges
# -
###############################################################################

sub announceWarfareChanges
{
	$attempts = 0;
	$success = 0;

	while ($success == 0 && $attempts < 10)
	{
		open( SMTP, "|\"$blat\" - -t $addrWarfareExternal,$addrWarfareInternal -f $addrFrom -s \"$announceSubjectWarfare\" -server $mailserver > output.txt" );

		setAnnounceMailHeaderText;
		print SMTP "$announceMailHeaderText";

		# Output Warfare code changes.
		if ($changelist{"Warfare"} ne "")
		{
			print SMTP "WARFARE CODE CHANGES:\n\n";
			print SMTP $changelist{"Warfare"}."\n";
			print SMTP $lineSpacer;
		}

		# Output Warfare content changes.
		%clarg = %changelistWarfare;
		assembleHashChangeLog( 0, 0 );
		if ( $commonChangeLog ne "" )
		{
			print SMTP "WARFARE CONTENT CHANGES:\n\n";
			print SMTP $commonChangeLog;
			print SMTP $lineSpacer;
		}

		# Output common changes.
		%clarg = %changelist;
		assembleHashChangeLog( 1, 0 );
		if ( $commonChangeLog ne "" )
		{
			print SMTP $commonChangeLog;
			print SMTP $lineSpacer;
		}

		close(SMTP);

		$success = wasMailSendSuccessful;
		# If mail send failed, delay a few seconds before trying again.
		if ($success == 0)
		{
			print "Unable to send mail, retrying in 15 seconds...\n";
			sleep(15);
		}
		
		$attempts++;
   }

	# Notify that we weren't able to send the mail.
	if ($success == 0)
	{
		print "Failed to send mail after $attempts attempts!\n";
	}
}

###############################################################################
# announceEnvyChanges
#
###############################################################################

sub announceEnvyChanges
{
	$attempts = 0;
	$success = 0;

	while ($success == 0 && $attempts < 10)
	{
		open( SMTP, "|\"$blat\" - -t $addrEnvyExternal,$addrEnvyInternal -f $addrFrom -s \"$announceSubjectEnvy\" -server $mailserver > output.txt" );

		setAnnounceMailHeaderText;
		print SMTP "$announceMailHeaderText";


		# Output Envy code changes.
		if ($changelist{"Envy"} ne "")
		{
			print SMTP "ENVY CODE CHANGES:\n\n";
			print SMTP $changelist{"Envy"}."\n";
			print SMTP $lineSpacer;
		}

		# Output Envy content changes.
		%clarg = %changelistEnvy;
		assembleHashChangeLog( 0, 0 );
		if ( $commonChangeLog ne "" )
		{
			print SMTP "ENVY CONTENT CHANGES:\n\n";
			print SMTP $commonChangeLog;
			print SMTP $lineSpacer;
		}

		# Output common changes.
		%clarg = %changelist;
		assembleHashChangeLog( 1, 0 );
		if ( $commonChangeLog ne "" )
		{
			print SMTP $commonChangeLog;
			print SMTP $lineSpacer;
		}

		close(SMTP);

		# If mail send failed, delay a few seconds before trying again.
		$success = wasMailSendSuccessful;
		if ($success == 0)
		{
			print "Unable to send mail, retrying in 15 seconds...\n";
			sleep(15);
		}

		$attempts++;
	}

	# Notify that we weren't able to send the mail.
	if ($success == 0)
	{
		print "Failed to send mail after $attempts attempts!\n";
	}
}

###############################################################################
# writeLocalChangeLog
# - Echoes the change announcement to local disk.
###############################################################################

sub writeLocalChangeLog
{
	if ( open(CHANGELOG,"> ".$buildLogDir."changes_$version.txt") )
	{
		setAnnounceMailHeaderText;
		print CHANGELOG "$announceMailHeaderText";

		# Output common changes.
		%clarg = %changelist;
		assembleHashChangeLog( 1, 1 );
		if ( $commonChangeLog ne "" )
		{
			print CHANGELOG $commonChangeLog;
			print CHANGELOG $lineSpacer;
		}

		# Output Warfare code changes.
		if ($changelist{"Warfare"} ne "")
		{
			print CHANGELOG "WARFARE CODE CHANGES:\n\n";
			print CHANGELOG $changelist{"Warfare"}."\n";
			print CHANGELOG $lineSpacer;
		}

		# Output Warfare content changes.
		%clarg = %changelistWarfare;
		assembleHashChangeLog( 0, 0 );
		if ( $commonChangeLog ne "" )
		{
			print CHANGELOG "WARFARE CONTENT CHANGES:\n\n";
			print CHANGELOG $commonChangeLog;
			print CHANGELOG $lineSpacer;
		}

		# Output Envy code changes.
		if ($changelist{"Envy"} ne "")
		{
			print CHANGELOG "ENVY CODE CHANGES:\n\n";
			print CHANGELOG $changelist{"Envy"}."\n";
			print CHANGELOG $lineSpacer;
		}

		# Output Envy content changes.
		%clarg = %changelistEnvy;
		assembleHashChangeLog( 0, 0 );
		if ( $commonChangeLog ne "" )
		{
			print CHANGELOG "ENVY CONTENT CHANGES:\n\n";
			print CHANGELOG $commonChangeLog;
			print CHANGELOG $lineSpacer;
		}
		
		if ( 0 ) {
		
		# Output common changes.
		if ( $commonChangeLog ne "" )
		{
			print CHANGELOG $commonChangeLog;
			print CHANGELOG $lineSpacer;
		}

		# Output Warfare code changes.
		if ($changelist{"Warfare"} ne "")
		{
			print CHANGELOG "WARFARE CODE CHANGES:\n\n";
			print CHANGELOG $changelist{"Warfare"}."\n";
			print CHANGELOG $lineSpacer;
		}

		# Output Warfare content changes.
		if ($changelist{"WarfareContent"} ne "")
		{
			print SMTP "WARFARE CONTENT CHANGES:\n\n";
			print SMTP $changelist{"WarfareContent"}."\n";
			print SMTP $lineSpacer;
		}

		# Output Envy code changes.
		if ($changelist{"Envy"} ne "")
		{
			print SMTP "ENVY CODE CHANGES:\n\n";
			print SMTP $changelist{"Envy"}."\n";
			print SMTP $lineSpacer;
		}

		#Output Envy content changes.
		if ($changelist{"EnvyContent"} ne "")
		{
			print SMTP "ENVY CONTENT CHANGES:\n\n";
			print SMTP $changelist{"EnvyContent"}."\n";
		}
		}

		close(CHANGELOG);
	}
	else
	{
		print "Couldn't open ".$buildLogDir."changes_".$version.".txt for writing\n";
	}
}

###############################################################################
# announceChangeLogToCoders
# - Sends a mail to $addrAnnounceChangeLogToCoders containing the contents of the local change log.
###############################################################################

sub announceChangeLogToCoders
{
	my @changeLogContents;

	# Read in the change log contents.
	if ( open(CHANGELOG, $buildLogDir."changes_$version.txt") )
	{
		@changeLogContents = <CHANGELOG>;
		close( CHANGELOG );
	}
	else
	{
		push @changeLogContents, "Couldn't open ".$buildLogDir."changes_".$version.".txt for reading\n";
	}

	$attempts = 0;
	$success = 0;

	while ($success == 0 && $attempts < 10)
	{
		open( SMTP, "|\"$blat\" - -t $addrAnnounceChangeLogToCoders -f $addrFrom -s \"$announceSubjectCoders\" -server $mailserver > output.txt" );
		print SMTP @changeLogContents;
		close(SMTP);

		# If mail send failed, delay a few seconds before trying again.
		$success = wasMailSendSuccessful;
		if ($success == 0)
		{
			print "Unable to send mail, retrying in 15 seconds...\n";
			sleep(15);
		}

		$attempts++;
	}

	# Notify that we weren't able to send the mail.
	if ($success == 0)
	{
		print "Failed to send mail after $attempts attempts!\n";
	}
}

###############################################################################
# createSyncBatchFile
# - Assembles and writes $buildSyncBatchFile, the server-side batch file for
#   sync'ing to the build.
###############################################################################

sub createSyncBatchFile
{
	$buildSyncBatchFile = $buildSyncBatchLink."syncTo$submittedChangeListNum.bat";

	open( SYNCFILE, "> $buildSyncBatchFile" );
	print SYNCFILE "p4 sync \@$submittedChangeListNum\n";
	close( SYNCFILE );
}

###############################################################################
# announceBuildSuccess
# - Announces build success to the mailing lists and writes a changes logfile.
###############################################################################

sub announceBuildSuccess
{
	createSyncBatchFile;

	if ($doMail == 1)
	{
		#assembleCommonChangeLog;

		if ( $doMailToProjects == 1 )
		{
			announceWarfareChanges;
			announceEnvyChanges;
		}

		if ( $doMailToTesters == 1 )
		{
			announceToTesters;
		}

		if ( $doAQABuild == 1 )
		{
			announceToQAPeople;
		}
	}

	# Changes are always saved locally.
	writeLocalChangeLog;
	if ( $doMail == 1 && $doAnnounceChangeLogToCoders == 1)
	{
		announceChangeLogToCoders;
	}
}

###############################################################################
# parseNativeLog
# - Scans through the specified file for failures and errors.
# - Returns 1 if none were found or 0 if failures/errors were found.
###############################################################################

sub parseNativeLog
{
	local $log_file = $_[0];
	#print "parseNativeLog: parsing $log_file\n";

	open(COMPILELOG, $log_file);
	while (<COMPILELOG>)
	{
		chomp;
		if ( /.*([0-9]+)\ failed/ )
		{
			if( $1 != 0 )
			{
				close(COMPILELOG);
				print "parseNativeLog -- found fails in $log_file\n";
				return 0;
			}
		}

		if ( /.*([0-9]+)\ error\(s\)/ )
		{
			if ( $1 != 0 )
			{
				close (COMPILELOG);
				print "parseNativeLog -- found errors in $log_file\n";
				return 0;
			}
		}
	}
	close(COMPILELOG);

	print "no fails or errors detected\n";
	return 1;
}

###############################################################################
# parseLogForXenonConnectFail
# - Scans through the specified file for the string "xbecopy: error X1001".
# - Returns 1 if it was found 0 otherwise.
###############################################################################

sub parseLogForXenonConnectFail
{
	local $log_file = $_[0];
	local $numMatches = 0;

	open(COMPILELOG, $log_file);
	while (<COMPILELOG>)
	{
		chomp;
		if ( /^(xbecopy: error X1001)/ )
		{
			$numMatches++;
		}
	}
	close(COMPILELOG);

	print "parseLogForXenonConnectFail -- found $numMatches matches\n";

	# If we couldn't find the "Could not connect" message, then there actually
	# was a valid Xbox compile error.
	if ( $numMatches == 0 )    # FIXME: Eventually this should be changed to match the number of errors in the log . . .
	{
		return 0;
	}

	return 1;
}

###############################################################################
# parseScriptLog
# - determines error/warning count for script compiles
###############################################################################

sub parseScriptLog
{
	local $log_file = $_[0];
	#print "parseScriptLog: parsing $log_file\n";

	$error_cnt = 0;
	$warning_cnt = 0;
	$header_cnt = 0;
	$last_class = "Unknown class";
	@errors;
	@warnings;

	# Try to open ucc.log.
	if (open(UCCLOG,$log_file))
	{
		local $last_header = "";
		local $last_class = "";
		
		# Parse contents of the log file.
		foreach $line (<UCCLOG>)
		{
			chomp($line);
			# generic error format
			if ($line =~ m/: Error/i)
			{
				push(@errors,$line);
				$error_cnt++;
			}
			# or a more specific error
			elsif ($line =~ m/Critical:/i)
			{
				push(@errors,$line);
				$error_cnt++;
			}
			# generic warning format
			elsif ($line =~ m/: Warning/i)
			{
					push(@warnings,$line);
			}
			# special check for import warnings
			elsif ($line =~ m/Unknown property/i)
			{
				push(@warnings,$last_class.": ".$line);
				$warning_cnt++;
			}
			# check for the class for the following warnings
			elsif ($line =~ m/Class: (\w+) extends/i)
			{
				$last_class = $1;
			}
			elsif ($line =~ m/Cannot export/i)
			{
				push(@errors,$line);
			}
		}

		# close the log file
		close(UCCLOG);
	}
	else
	{
		print "Failed to open $log_file!\n";
	}
	return ($error_cnt == 0);
}

###############################################################################
# announceFailure
#
###############################################################################

sub announceFailure
{
	local ($localpart) = $_[0];
	local ($localfile) = $_[1];

	if ( $doMail == 1 )
	{
		$attempts = 0;
		$success = 0;
		while ($success == 0 && $attempts < 10)
		{
			open( SMTP, "|\"$blat\" - -t $addrFail -f $addrFrom -s \"$failSubject -- $localpart failed.\" -server $mailserver > output.txt" );
			print SMTP "Gears build failure: $localpart failed.\n\nSee $localfile for the build log.";
			close( SMTP );

			# Make sure we were able to send the mail.
			if (open(OUTPUT,"output.txt"))
			{
				$errors = 0;
				foreach $line (<OUTPUT>)
				{
					if ($line =~ m/Error:/i)
					{
						chomp($line);
						$errors++;
					}
				}
				$success = $errors == 0 ? 1 : 0;
				close(OUTPUT);
			}
			else
			{
				$success = 1;
			}

			# If we failed, delay a few seconds before attempting again.
			if ($success == 0)
			{
				print "Unable to send mail, retrying in 15 seconds...\n";
				sleep(15);
			}
			$attempts++;
		}
	}
	print "Gears build failure: $localpart failed.\n\nSee $localfile for the build log.\n";
}

###############################################################################
# compilation for PC
###############################################################################

# Cleans a single game (and its dependancies) for the PC.
sub cleanGame
{
	my $buildGame = $_[0];
	print "- Cleaning $buildGame for PC\n";
	system("\"$devenv\" \"$solution\" /clean \"Release\" /project PCLaunch-".$buildGame."Game /out $buildlog");
}


# Cleans a single game (and its dependancies) for the PC.
sub cleanGameLTCG
{
	my $buildGame = $_[0];
	print "- Cleaning $buildGame for PC LTCG\n";
	system("\"$devenv\" \"$solution\" /clean \"ReleaseLTCG\" /project PCLaunch-".$buildGame."Game /out $buildlog");
}


# Builds (not rebuilds) a single game for the PC.
sub compilePC
{
	my $buildGame = $_[0];
	my $runString = "\"$devenv\" \"$solution\" /build Release /project PCLaunch-".$buildGame."Game /out $buildlog";
	print "- Building ".$buildGame."Game for PC\n";
	system( $runString );
	return $buildlog;
}

# Builds (not rebuilds) a single game for the PC.
sub compilePCLTCG
{
	my $buildGame = $_[0];
	my $runString = "\"$devenv\" \"$solution\" /build ReleaseLTCG /project PCLaunch-".$buildGame."Game /out $buildlog";
	print "- Building ".$buildGame."Game for PC LTCG\n";
	system( $runString );
	return $buildlog;
}


# Cleans all projects for the PC.
sub cleanPCAll
{
	print "- Cleaning all for PC\n";
	system("\"$devenv\" /clean \"Release\" \"$solution\" /out $buildlog");
	return $buildlog;
}


# Builds (not rebuilds) everything for the PC.
sub compilePCAll
{
	print "- Building all for PC\n";
	system("\"$devenv\" /build \"Release\" \"$solution\" /out $buildlog");
	return $buildlog;
}


# Cleans all projects for the PC.
sub cleanPCLTCGAll
{
	print "- Cleaning all for PC\n";
	system("\"$devenv\" /clean \"ReleaseLTCG\" \"$solution\" /out $buildlog");
	return $buildlog;
}


# Builds (not rebuilds) everything for the PC.
sub compilePCLTCGAll
{
	print "- Building all for PC\n";
	system("\"$devenv\" /build \"ReleaseLTCG\" \"$solution\" /out $buildlog");
	return $buildlog;
}


#############################################################
# compileXenon
# - Does a build for Xenon using the XeRelease configuration.
#############################################################

sub compileXenon
{
	#local $build_game = $_[0];
	print "- Building Xenon:XeRelease\n";
	system("\"$devenv\" \"$solution\" /rebuild \"XeRelease\" /out $buildlog");

	return $buildlog;
}

#############################################################
# compileXenonLTCG
# - Does a build for Xenon using the XeReleaseLTCG configuration.
#############################################################

sub compileXenonLTCG
{
	#local $build_game = $_[0];
	print "- Building Xenon:XeReleaseLTCG\n";
	system("\"$devenv\" \"$solution\" /rebuild \"XeReleaseLTCG\" /out $buildlog");

	return $buildlog;
}


sub compileXenonLTCGDebugConsole
{
	#local $build_game = $_[0];
	print "- Building Xenon:XeReleaseLTCGDebugConsole\n";
	system("\"$devenv\" \"$solution\" /rebuild \"XeReleaseLTCG-DebugConsole\" /out $buildlog");

	return $buildlog;
}


#############################################################
# compileScript
# - Compiles script for the specified game.
# - Returns the script-compile log's filename.
#############################################################

sub compileScript
{
	local $build_game = $_[0];
	print "compiling script for $build_game...\n";

	local $script_dir = $root_dir.$build_game."Game\\";
	
	# Delete all writeable .ini's.
	system("del ".$script_dir."Config\\*.ini");
	
	# Remove all existing .u's.
	system("del ".$script_dir."Script\\*.u /f");
	
	# Remove log file.
	system("del ".$script_dir."Logs\\Launch.log /f");
	
	# Compile.
	system( $systemdir."\\".$build_game."Game.com make -full -unattended LOG=Launch.log");

	# Wait for the log, indicating build finish.
	while (!(-e $script_dir."Logs\\Launch.log"))
	{
		#print "still waiting for ".$script_dir."Logs\\Launch.log . . .\n";
		sleep(10);
	}

	# Return the log filename, which will be checked for build errors.
	return $script_dir."Logs\\Launch.log";
}


sub compileScriptFinalRelease
{
	local $build_game = $_[0];
	print "compiling script -FINAL_RELEASE for $build_game...\n";

	local $script_dir = $root_dir.$build_game."Game\\";
	
	# Delete all writeable .ini's.
	system("del ".$script_dir."Config\\*.ini");
	
	# Remove all existing .u's.
	system("del ".$script_dir."ScriptFinalRelease\\*.u /f");
	
	# Remove log file.
	system("del ".$script_dir."Logs\\Launch.log /f");
	
	# Compile.
	system( $systemdir."\\".$build_game."Game.com make -full -unattended -FINAL_RELEASE LOG=Launch.log");

	# Wait for the log, indicating build finish.
	while (!(-e $script_dir."Logs\\Launch.log"))
	{
		#print "still waiting for ".$script_dir."Logs\\Launch.log . . .\n";
		sleep(10);
	}

	# Return the log filename, which will be checked for build errors.
	return $script_dir."Logs\\Launch.log";
}



#############################################################
# compileStatsViewer
# - Does a build for the StatsViewer
#############################################################

sub compileStatsViewer
{
	#local $build_game = $_[0];
	print "- Building StatsViewer\n";
	system("\"$devenv\" \"$root_dir\\Development\\Tools\\StatsViewer\\StatsViewer.sln\" /rebuild \"Release\" /out $buildlog");

	return $buildlog;
}


########################################################################
# grabChangeDescription
#
########################################################################

sub grabChangeDescription
{
	open( CHANGES, "p4 changes \"$depot...\"|" );
	foreach $change (<CHANGES>)
	{
	}
	close( CHANGES );
}

########################################################################
# setDeveloper
# - Overrides build settings with developer settings.  Must be the first thing called!
########################################################################

sub setDeveloper
{
	print $lineSpacer;
	print "Checking for developer settings...\n";

	if ( $useDeveloperSettings == 1 )
	{
		print "Using developer settings\n";

		local $old_root_dir = $root_dir;
		local $old_buildLogDir = $buildLogDir;
		local $old_clientspec = $clientspec;
		local $old_owner = $owner;
		local $old_addrAnnounceChangeLogToCoders = $addrAnnounceChangeLogToCoders;
		local $old_addrFail = $addrFail;

		$root_dir = $developer_root_dir;
		$buildLogDir = $developer_buildLogDir;
		$clientspec = $developer_clientspec;
		$owner = $developer_owner;
		$addrAnnounceChangeLogToCoders = $developer_addrAnnounceChangeLogToCoders;
		$addrFail = $developer_addrFail;

		print "root_dir was $old_root_dir, now $root_dir\n";
		print "buildLogDir was $old_buildLogDir, now $buildLogDir\n";
		print "clientspec was $old_clientspec, now $clientspec\n";
		print "owner was $old_owner, now $owner\n";
		print "addrAnnounceChangeLogToCoders was $old_addrAnnounceChangeLogToCoders, now $addrAnnounceChangeLogToCoders\n";
		print "addrFail was $old_addrFail, now $addrFail\n";
	}
	else
	{
		print "None found\n";
	}
	
	$systemdir				= $root_dir.'Binaries';
	$failedLogDir				= $buildLogDir."FailedBuilds\\";
	$failedLogDirLink			= $buildLogLink."FailedBuilds\\";
	$solution				= $root_dir.'Development\\Src\\UnrealEngine3.sln';			# Solution to compile in devenv
	$checkoutfilelistfile		= $root_dir.'Development\\Build\\BuildFileList.txt';		# List of files to delete and build - included in P4.
	$checkoutfileexcludefile	= $root_dir.'Development\\Build\\BuildFileExcludeList.txt';	# List of files to exclude from build - included in P4.
	$unObjVercpp				= $root_dir.'Development\\Src\\Core\\Src\\UnObjVer.cpp';	# C++ source file containing the defintion of ENGINE_VERSION.
	$qaBuildInfoFile			= '\\\\build-server-01\\BuildFlags\\QABuildInfoCandidateToBeSignedOffOn.ini';	# text .ini file containing the version QA version info

	$examplegameXexXml		    = $root_dir.'Development\\Src\\ExampleGame\\Live\\xex.xml';	# .xml with examplegame's live version
	$wargameXexXml				= $root_dir.'Development\\Src\\WarfareGame\\Live\\xex.xml';	# .xml with wargame's live version

	$bvtTimestampFile		    = $root_dir.'Development\\Src\\build.properties';	# .properties file with the key for th bvt timestamp


	print $lineSpacer;
}

sub assembleSortedChangePaths
{
	local @changePaths;
	#grabChangeDescription();

	#############
	foreach $blah (@fileChanges)
	{
		chomp( $blah );
		@fields = split /$depot/, $blah;
		if ( scalar(@fields) == 2 )
		{
			@moreFields = split /\#/, $fields[1];
			#print "$moreFields[0] and also $moreFields[1] \n";
			#print "$moreFields[0] \n";
			push @changePaths, $moreFields[0];
		}
	}

	return sort @changePaths;
}

################################################################################################################################################
################################################################################################################################################
#
# Documentation nagging system
#
################################################################################################################################################
################################################################################################################################################

########################################################################
# buildDoxygenConfigFileContents
# - Scans the file specified on $_[1] for the "INPUT =" line, replaces it with
#   "INPUT=bunch of text and stuff" (as specfied on input arg $_[0]) and
#   returns the result in a string.
########################################################################

sub buildDoxygenConfigFileContents
{
	local $inputFilename = $_[0];
	local $templateDoxFilename = $_[1];
	local @textbuf;

	if ( open (INFILE, "$templateDoxFilename" ) )
	{
		chomp( @textbuf = <INFILE> );
		close( INFILE );
	}
	else
	{
		print "buildDoxygenConfigFileContents - couldn't open $templateDoxFilename\n";
		return "";
	}

	local $newFileContents;
	foreach $line (@textbuf)
	{
		# Scan for input variable and fill it with the file we want.
		# We have to filter out comment lines first because the default config file automatically generated by
		# doxygen by default contains comment lines of description that contain the variable names.
		if ( !($line =~ /^\#/i ) )
		{
			if ( $line =~ /\s*INPUT\s/)
			{
				$newFileContents .= "INPUT = $inputFilename\n";
			}
			else
			{
				$newFileContents .= $line."\n";
			}
		}
	}

	return $newFileContents;
}

########################################################################
# writeDoxygenConfigFile
# - Reads in a doxygen config file $_[0] and writes a file named $_[1] where
#   the INPUT var is set to $_[2];
# - Returns 1 on success or 0 on fail.
########################################################################

sub writeDoxygenConfigFile
{
	local $oldDoxygenConfigFilename = $_[0];
	local $newDoxygenConfigFilename = $_[1];
	local $valueForInputVar = $_[2];

	$newFileContents = buildDoxygenConfigFileContents( $valueForInputVar, $oldDoxygenConfigFilename );
	if ( open( OUTFILE, "> $newDoxygenConfigFilename" ) )
	{
		print OUTFILE $newFileContents;
		close( OUTFILE );
		return 1;
	}
	return 0;
}

########################################################################
# compareFileContents
# - Inputs should be packed into @g_contentsYes and @g_contentsNo.
# - Returns 1 if the two lists match.
# - Documented type is specified on $_[0], and must be "class" or "struct".
########################################################################

sub compareFileContents
{
	local $docType = $_[0];

	if ( $docType ne "class" && $docType ne "struct" )
	{
		print "compareFileContents - received invalid type $docType\n";
	}

	local $numContentsYes = scalar( @g_contentsYes );
	local $numContentsNo = scalar( @g_contentsNo );

	if ( $numContentsYes != $numContentsNo )
	{
		return 0;
	}

	local $ctr = 0;
	while ( $ctr < $numContentsYes )
	{
		if ( 0 ) {  ####### !!! This can stay undcoumented as long as we're stripping link tags from the HTML.
		if ( $g_contentsYes[$ctr] =~ /The documentation for this $docType was generated from the following file/ )
		{
			print "Found footer\n";
			last;
		}
		}

		if ( $g_contentsYes[$ctr] ne $g_contentsNo[$ctr] )
		{
			# Check if the unmatch is in the navbar.
			local $line = $g_contentsYes[$ctr];
			if ( $line =~ /File\&nbsp\;List/ || $line =~ /File\&nbsp\;Members/ )
			{
				print "Found unmatch in navbar (File List or File Members), ignoring.\n";
			}
			else
			{
				print "$g_contentsYes[$ctr]\n";
				print "$g_contentsNo[$ctr]\n";
				print "----------\n";
				exit( 0 );
			}
		}
		$ctr++;
	}

	return 1;
}

########################################################################
# removeLinkTagsFromHTMLContents
# - Strips <a ></a> tags from an HTML buffer.
# - The input contents are in @g_inFile.
# - The output is written to the reference in $_[0].
########################################################################

sub removeLinkTagsFromHTMLContents
{
	local $outp = $_[0];
	foreach $line ( @g_inFile )
	{
		while( $line =~ s/<a.*?>// || $line =~ s/<\/a>// )
		{}
		push @$outp, $line;
	}
}
########################################################################
# compareDocumentationForType
# - Gets a directory listing of .html files for the specified datatype
# - The datatype to document is specified on $_[0] is 1, and must be "class" or "struct".
# - For each .html, files are read in and compared.
# - If the docs don't match, undocumented entities are added to @g_undocumentedEntities.
########################################################################

sub compareDocumentationForType
{
	local $docType = $_[0];

	if ( $docType ne "class" && $docType ne "struct" )
	{
		print "compareDocumentationForType - received invalid type $docType\n";
	}

	# Get a directory listing from the EXTRACT_ALL documentation and compare the files to the non-EXTRACT_ALL documentation.
	system("dir allIsYes\\".$docType."*.html > output.txt" );

	open( INFILE, "output.txt ");
	chomp( @dirContents = <INFILE> );
	close( INFILE );
	foreach $line (@dirContents)
	{
		# Extract list of files of the form "class_someNameHere.html".
		# Ignore files of the form "class_someNameHere-members.html".
		if ( $line =~ /\s(\w+)\.html/ && !($line =~ /-members.html/) )
		{
			local $basename = $1;
			local $filename = $basename.".html";

			# Strip off the docType from the front.
			$basename =~ /^$docType(\w+)/;
			local $entity = $1;
			local $addEntityToNagList = 0;

			#print "Filename:   $filename\n";
			#print "Basename:   $basename\n";
			#print "Entityname: $entity\n";

			if ( ( -e "allIsNo\\$filename" ) )
			{
				#print "allIsNo\\$filename exists!\n";
				
				local @g_inFile;
				local @g_contentsYes = ();
				local @g_contentsNo = ();

				open( INFILE, "allIsYes\\$filename" );
				chomp( @g_inFile = <INFILE> );
				close( INFILE );
				removeLinkTagsFromHTMLContents( "g_contentsYes" );
				
				#open( OUTFILE, "allIsYes\\$basename_NEW.html" );
				#foreach $line( @g_contentsYes ) { print OUTFILE "$line\n"; }
				#close( OUTFILE );
				
				@g_inFile = ();

				open( INFILE, "allIsNo\\$filename");
				chomp( @g_inFile = <INFILE> );
				close( INFILE );
				removeLinkTagsFromHTMLContents( "g_contentsNo" );

				#open( OUTFILE, "allIsNo\\$basename_NEW.html" );
				#foreach $line( @g_contentsNo ) { print OUTFILE "$line\n"; }
				#close( OUTFILE );

				local $contentsMatch = compareFileContents( $docType );
				if ( $contentsMatch == 0 )
				{
					print "$docType $entity has undocumented members!\n";
					$addEntityToNagList = 1;
				}
			}
			else
			{
				# The file doesn't exist for the EXTRACT_ALL=no documentation, meaning that nothing is documented.
				print "$docType $entity is missing $docType documentation!\n";
				$addEntityToNagList = 1;
			}
			
			# If the contents of the file don't match, add the entity to the naglist.
			if ( $addEntityToNagList == 1 )
			{
				push @g_undocumentedEntities, $docType." ".$entity;
			}
			else
			{
				print "$docType $entity is fully documented\n";
			}
		}
	}
}

########################################################################
# compareDocumentationForFile
# - Creates two doxygen config files, one with EXTRACT_ALL=yes and one with EXTRACT_ALL=no
# - doxygen is run to create two folders called 
# - All class and struct documentation
# - Returns 0 if comparison couldn't happen (file permissions, etc) or 1 otherwise.
########################################################################

sub compareDocumentationForFile
{
	local $changeFilePath = $_[0];

	if ( writeDoxygenConfigFile( "Template_allIsYes.dox", "temp_yes.dox", $changeFilePath ) == 0 )
	{
		print "writeDoxygenConfigFile failed for yes\n";
		return 0;
	}
	else
	{
		#print "writeDoxygenConfigFile succeeded for yes\n";
		system( "$doxygen temp_yes.dox" );
	}

	if ( writeDoxygenConfigFile( "Template_allIsNo.dox", "temp_no.dox", $changeFilePath ) == 0 )
	{
		print "writeDoxygenConfigFile failed for no\n";
		return 0;
	}
	else
	{
		#print "writeDoxygenConfigFile succeeded for no\n";
		system( "$doxygen temp_no.dox" );
	}

	compareDocumentationForType( "class" );
	compareDocumentationForType( "struct" );

if ( 0 ) {  # FIXME: enable this again!!!
	# Cleanup temporary files.
	system("del temp_yes.dox");
	system("del /q allIsYes\\*");
	system("rmdir allIsYes");

	system("del temp_no.dox");
	system("del /q allIsNo\\*");
	system("rmdir allIsNo");
	}

	return 1;
}

# FIXME: delete this!!!!
sub oldDocumentationNag
{
	############
	if ( 0 ) {
		# Grab a copy of the log and parse out undocumented members.
		if ( open( DOXYFILE, "latestDoxygenWarnings_UE3.log" ) )
		{
			chomp( @doxytext = <DOXYFILE> );
			close(DIXYFILE);
		}
		else
		{
			print "Couldn't open latestDoxygenWarnings_UE3.log\n";
		}

		print "parsing latestDoxygenWarnings_UE3.log...\n";
		open (OUTFILE, ">output.txt" );
		@sortedDoxytext = sort @doxytext;
		foreach $line (@sortedDoxytext)
		{
			if ( $line =~ /is not documented/ )
			{
				@fields = split / /, $line;
				$swappedRoot = 'E:/Build_UE3/UnrealEngine3-Gears/';
				@moreFields = split /$swappedRoot/, $fields[0];
				print OUTFILE "$moreFields[1]\n";
				#print "$fields[0]\n";
			}
		}
		close( OUTFILE );
	}
	################
}
################################################################################################################################################
################################################################################################################################################

########################################################################
# dobuild
#
########################################################################

sub dobuild
{
	$buildStartTime = time;

	###################################
	# Begin by syncing the client.

	if ($doSync == 1)
	{
		clock "SYNC";
		system("p4 -c \"$clientspec\" sync \"$depot...\"");
		unclock "SYNC";
	}

	if ($doCheckout == 1)
	{
		clock "CHECKOUT";

		# Retrieve the checkout list from file.
		if ( !getCheckoutList() )
		{
			announceFailure("Couldn't get checkout list; opening $checkoutfilelistfile for read", "<no log file>");
			print "Gears build failed: Couldn't read checkout list from $checkoutfilelistfile.\n\n";
			return;
		}

		# Mark checkout list files for edit and lock in P4.
		if( !checkoutAll() )
		{
			# Checkout failed for some reason; unlock.
			revertAll();
			return;
		}

		unclock "CHECKOUT";
	}

	clock "GET CHANGES";
	if ( !assembleChangesSinceLastBuild(0) )
	{
		announceFailure("Find last build version", "<no log file>");
		print "Gears build failed: Couldn't find last build version.\n\n";

		# Unlock if we had checked out files.
		revertAll();
		return;
	}

	unclock "GET CHANGES";

	setVersion;

	if ( $doBuild == 1 )
	{
		print "Compiling new build version: $version\n";

		if( $doAQABuild == 1 )
		{
		  announceToQAPeopleThatAQABuildIsBuilding;
		}
		else
		{
		  announceToQAPeopleThatABuildIsBuilding;
		}

		# Build StatsViewer
		if ( $doBuildStatsViewer == 1 )
		{
			clock "BUILD STATSVIEWER";

			$compilelog = compileStatsViewer;
			print "StatsViewer compilelog is $compilelog\n";

			if ( parseNativeLog( $compilelog ) == 0 )
			{
			  # StatsViewer compile failed.  Copy the log over the failed log directory.
			  system( "copy $compilelog $failedLogDir /y" );
			
			  # Unlock if we did a checkout.
			  revertAll();
			
			  announceFailure( "StatsViewer compile has failed", $failedLogDirLink  );
			  return;
			
			}
			unclock "BUILD STATSVIEWER";
   	       }


		# Increment the engine version.
		incEngineVer();
		incXboxLiveVer();

		# Build native for PC.
		if ( $doBuildPC == 1 )
		{
			clock "BUILD CPP FOR PC";

			# If all games are to be built, simply rebuild the entire .NET solution.
			# Otherwise, simply build (not rebuild) each game.
			if ( $buildAllGames == 1 )
			{
				if ( $incrementalBuild == 0 )
				{
					cleanPCAll;
				}
				
				local $compilelog = compilePCAll;
				print "PC rebuild compile log is $compilelog\n";

				if ( parseNativeLog( $compilelog ) == 0 )
				{
					system( "copy $compilelog $failedLogDir /y" );

					# Unlock if we did a checkout.
					revertAll();

					announceFailure( "PC native rebuild compile", $failedLogLink );
					return;
				}
			}
			else
			  {
			    if ( $incrementalBuild == 0 )
			      {
				cleanPCAll;
				#cleanGame( $game );
			      }

				foreach $game (@buildGames)
				{
					local $compilelog = compilePC( $game );
					print "PC build ($game) compile log is $compilelog\n";

					if ( parseNativeLog( $compilelog ) == 0 )
					{
						system( "copy $compilelog $failedLogDir /y" );

						# Unlock if we did a checkout.
						revertAll();

						announceFailure( "PC native build ($game) compile", $failedLogLink );
						return;
					}
				}
			} # endif buildAllGames == 1

			unclock "BUILD CPP FOR PC";
		}



		# Build native for PC LTCG.
		if ( $doBuildPCLTCG == 1 )
		{
			clock "BUILD CPP FOR PC LTCG";

			# If all games are to be built, simply rebuild the entire .NET solution.
			# Otherwise, simply build (not rebuild) each game.
			if ( $buildAllGames == 1 )
			{
				if ( $incrementalBuild == 0 )
				{
					cleanPCLTCGAll;
				}
				
				local $compilelog = compilePCLTCGAll;
				print "PC LTCG rebuild compile log is $compilelog\n";

				if ( parseNativeLog( $compilelog ) == 0 )
				{
					system( "copy $compilelog $failedLogDir /y" );

					# Unlock if we did a checkout.
					revertAll();

					announceFailure( "PC LTCG native rebuild compile", $failedLogLink );
					return;
				}
			}
			else
			{
			  if ( $incrementalBuild == 0 )
			    {
			      cleanPCLTCGAll;
			      #cleanGameLTCG( $game );
			    }
	   
				foreach $game (@buildGames)
				{

					local $compilelog = compilePCLTCG( $game );
					print "PC LTCG build ($game) compile log is $compilelog\n";

					if ( parseNativeLog( $compilelog ) == 0 )
					{
						system( "copy $compilelog $failedLogDir /y" );

						# Unlock if we did a checkout.
						revertAll();

						announceFailure( "PC LTCG native build ($game) compile", $failedLogLink );
						return;
					}
				}
			} # endif buildAllGames == 1

			unclock "BUILD CPP FOR PC LTCG";
		}


		# Build script.
		if ( $doBuildScript == 1 )
		{
			clock "BUILD SCRIPT";

			foreach $game (@buildGames)
			{
				# Compile script, grab the compile log filename and parse it for errors.
				local $compilelog = compileScript( $game );
				if ( parseScriptLog( $compilelog ) == 0 )
				{
					# Script compile failed.  Copy the log over the failed log directory.
					system( "copy $compilelog $failedLogDir /y" );

					# Unlock if we did a checkout.
					revertAll();

					announceFailure( "Script Rebuild ($game) compile", $failedLogDirLink."Launch.log" );
					return;
				}

				# Compile script, grab the compile log filename and parse it for errors.
				local $compilelog = compileScriptFinalRelease( $game );
				if ( parseScriptLog( $compilelog ) == 0 )
				{
					# Script compile failed.  Copy the log over the failed log directory.
					system( "copy $compilelog $failedLogDir /y" );

					# Unlock if we did a checkout.
					revertAll();

					announceFailure( "Script Rebuild ($game) compile", $failedLogDirLink."Launch.log" );
					return;
				}
			}

			unclock "BUILD SCRIPT";
		}


		# Build native for Xenon.
		if ( $doBuildXenon == 1 )
		{
			clock "BUILD CPP FOR XERELEASE";

			$compilelog = compileXenon;
			print "Xenon compilelog is $compilelog\n";
			if ( parseNativeLog( $compilelog ) == 0 )
			{
				# Check and make sure the error is not a "could not connect to Xbox" error.
				if ( parseLogForXenonConnectFail( $compilelog ) == 0 )
				{
					system( "copy $compilelog $failedLogDir /y" );
					revertAll();
					announceFailure( "Xenon:XeRelease native rebuild compile", $failedLogLink );
					return;
				}
			}
			#system("copy ".$root_dir."Development\\lib\\xerelease\\lib\\xerelease\\*.exe ".$root_dir."Binaries\\ /y");

			unclock "BUILD CPP FOR XERELEASE";
		}


		# Build native for Xenon.
		if ( $doBuildXenonLTCG == 1 )
		{
		    clock "BUILD CPP FOR XERELEASELTCG";						
		
		    $compilelog = compileXenonLTCG;
		    print "XenonLTCG compilelog is $compilelog\n";
		    if ( parseNativeLog( $compilelog ) == 0 )
		    {
		        # Check and make sure the error is not a "could not connect to Xbox" error.
		        if ( parseLogForXenonConnectFail( $compilelog ) == 0 )
			{
			    system( "copy $compilelog $failedLogDir /y" );
			    revertAll();
			    announceFailure( "Xenon:XeReleaseLTCG native rebuild compile", $failedLogLink );
			    return;
			  }
		    }
			
		    #system("copy ".$root_dir."Development\\lib\\xerelease\\lib\\xereleaseltcg\\*.exe ".$root_dir."Binaries\\ /y");

		    unclock "BUILD CPP FOR XERELEASELTCG";



		    clock "BUILD CPP FOR XERELEASELTCG-DEBUGCONSOLE";						
		
		    $compilelog = compileXenonLTCGDebugConsole;
		    print "XenonLTCGDebugConsole compilelog is $compilelog\n";
		    if ( parseNativeLog( $compilelog ) == 0 )
		    {
		        # Check and make sure the error is not a "could not connect to Xbox" error.
		        if ( parseLogForXenonConnectFail( $compilelog ) == 0 )
			{
			    system( "copy $compilelog $failedLogDir /y" );
			    revertAll();
			    announceFailure( "Xenon:XeReleaseLTCGDebugConsole native rebuild compile", $failedLogLink );
			    return;
			  }
		    }
			
		    #system("copy ".$root_dir."Development\\lib\\xerelease\\lib\\xereleaseltcg\\*.exe ".$root_dir."Binaries\\ /y");

		    unclock "BUILD CPP FOR XERELEASELTCG-DEBUGCONSOLE";



		}

	}

	###################################
	# Generate documentation.
	
	if( $doDocs == 1 && $doAQABuild == 1 )
	{
		clock "DOCUMENTATION";

		if ( $doUE3Docs == 1 )
		{
			print "Building UnrealEngine3 documentation . . .\n";
			system("$doxygen $doxyfileUE3 > $doxyUE3Logfile 2>&1");
			system("move /y $doxyUE3Logfile $buildLogDir");
		}

		if ( $doXenonDocs == 1 )
		{
			print "Building Xenon-specific documentation . . .\n";
			system("$doxygen $doxyfileXenon > $doxyXenonLogfile 2>&1");	
			system("move /y $doxyXenonLogfile $buildLogDir");
		}

		# Copy off the compiled html files and kill temps.
		system("copy ..\\Documentation\\html\\UnrealEngine3.chm ..\\Documentation\\");
		system("copy ..\\Documentation\\html\\Xenon.chm ..\\Documentation\\Xenon\\");
		#system("del /q ..\\Documentation\\html\\*");
		#system("rmdir ..\\Documentation\\html\\");

		unclock "DOCUMENTATION";
	}

	if ( $doDocNag == 1 )
	{
		clock "DOCUMENTATION_NAG";

		@sortedChangePaths = assembleSortedChangePaths;
		$prevChangeFile = "";
		foreach $changefile (@sortedChangePaths)
		{
			$ctr++;
			
			if ($changefile =~ /\.(\w+)/)
			{
				local $fileExtension = $1;
				print "fileExtension is $fileExtension\n";
				if ( $fileExtension eq "h" || $fileExtension eq "cpp" )
				{
					if ( $changefile ne $prevChangeFile &&
							$changefile =~ /Development\/Src/ )
					{
							print "File $ctr: $changefile\n";
							
							#$changeFilePath = "..\/..\/$changefile";
							$changeFilePath = "test.h";
							compareDocumentationForFile( $changeFilePath );
							$prevChangeFile = $changefile;
					}
				}
			}
		}
		unclock "DOCUMENTATION_NAG";
	}

	###################################
	# Submit build.

	if ($doSubmit == 1)
	{
		# Submit and label.
		print "START SUBMIT $version\n";

		clock "SUBMIT BUILD";
		if ( submitAll() == 0 )
		{
			print "UE3 build failed: Perforce submit failed.  Reverting.\n\n";
			revertAll();
			return;
		}
		createLabels();


       # this needs to be called only when there is a successful build else this will be out of date due to possible build failures
       # write the $qaBuildInfoFile back out with the version build version updated
       # NOTE:  this depends on incEngineVer() being called first and updating the $v var with the new Engine version
	if( $doAQABuild == 1 )
	{
	    # Grab a copy of the current contents of $unObjVercpp,
	    open( QA_BUILD_INFO_FILE, "$qaBuildInfoFile" );
	    @vertext = <QA_BUILD_INFO_FILE>;
	    close( QA_BUILD_INFO_FILE );

	    open( QA_BUILD_INFO_FILE, ">$qaBuildInfoFile" );
	    foreach $verline ( @vertext )
	    {
		$verline =~ s/\n//;   # Remove first newline occurance (hopefully at end of line...)
		if( $verline =~ m/EngineVersion=([0-9]*)$/ ) # EngineVersion=1099
		{
			$verline = "EngineVersion=$v";
			print "\n\n\ new QA build number: $verline \n\n";
		}
		print QA_BUILD_INFO_FILE "$verline\n";
	    }
	    close( QA_BUILD_INFO_FILE );


	    # since we have done a QA build we now need to create QABuildIsBeingVetted.txt file which will stop further builds
	    open( QA_BEING_VETTED_FILE, ">\\\\build-server-01\\BuildFlags\\QABuildIsBeingVetted.txt" );
	    close( QA_BEING_VETTED_FILE );
	}

		unclock "SUBMIT BUILD";
	}
	else
	{
		# If we did a checkout but aren't submitting, revert to the repository.
		revertAll();
	}

	###################################
	# Upload symbols to symbol server.

	if ($uploadSymbols == 1)
	{
                clock "UPDATE SYMBOL SERVER";
		system("store_symbols_gears $builtFromChangelistNum $version");
                unclock "UPDATE SYMBOL SERVER";
	}

	###################################
	# Complete; announce build success to mailing lists and logs.

	clock "ANNOUNCE";
	announceBuildSuccess();
	unclock "ANNOUNCE";

	print "BUILD COMPLETE $version\n";
	print "Total time: ".elapsedTime(time-$buildStartTime)."\n";
}

########################################################################
# parseCommandLineArgs
#
########################################################################

sub parseCommandLineArgs
{

	foreach $argnum (0 .. $ARGV )
	{
		$curArg = $ARGV[$argnum];
		#$curArg =~ tr/A-Z/a-z/;		# Convert to lowercase.  Ignores locale!

		#print "curArg $curArg\n";

		if( $curArg eq 'doQABuild' )
		{
		    $doAQABuild = 1;
		    print "   Doing a QABuild.\n";
		}
		elsif( $curArg eq 'doPCLTCGBuild' )
		{
		    $doBuildXenon = 0;
		    $doBuildXenonLTCG = 0;
		    print "   Doing a PCOnlyBuild.\n";
		}
		elsif( $curArg eq 'doPCOnlyBuild' )
		{
		    $doBuildPCLTCG = 0;
		    $doBuildXenon = 0;
		    $doBuildXenonLTCG = 0;
		    print "   Doing a PCOnlyBuild.\n";
		}
		elsif( $curArg eq 'doPCAndXenonBuild' )
		{
		    $doBuildPCLTCG = 0;
		    $doBuildXenon = 1;
		    $doBuildXenonLTCG = 1;
		    print "   Doing a PCAndXenonBuild.\n";
		}
		elsif( $curArg eq 'labeltest' )
		{
		  setVersion();
		  $builtFromChangelistNum = "666";
		  $labelVersion = sprintf( "$version\_$builtFromChangelistNum" );
		# Create the version label.
		#system( "p4 -c \"$developer_clientspec\" label -t latestUnrealEngine3 $version_$builtFromChangelistNum" );
		  system( "" );
		open( LABELCMD, "|p4 -c \"$clientspec\" label -i" );
		print LABELCMD "Label:\t$labelVersion\n";
		print LABELCMD "Owner:\t$owner\n";
		print LABELCMD "Description:\n\t$version Built from changelist:  $builtFromChangelistNum\n";		
		print LABELCMD "Options:\tunlocked\n";
		print LABELCMD "View:\n\t\"$depot\...\"\n";
		close( LABELCMD );
		system( "p4 -c \"$clientspec\" labelsync -l $labelVersion" );
		exit(0);

	    }
		elsif( $curArg eq 'amht' )
		{
	#	    $doAQABuild = 1;
		  setAnnounceMailHeaderText;
		  print "announceMailHeaderText: $announceMailHeaderText\n";
		}
		elsif( $curArg eq 'testQAEmail' )
		{
		  print "test qa email";
		    $doAQABuild = 1;
		  announceToQAPeople;
	#	  setAnnounceMailHeaderText;
		#  print "announceMailHeaderText: $announceMailHeaderText\n";
		}
		elsif( $curArg eq 'incrXboxLive' )
		{
		  print "incrincrXboxLive incrXboxLive incrXboxLive\n";
		  setDeveloper();
		  incXboxLiveVer();
		  exit( 0 );
		} 
		elsif( $curArg eq 'incrBVT' )
		{
		  print "incrincrBVT \n";
		  setDeveloper();
		  assembleChangesSinceLastBuild();

		 # setVersion();
		  incEngineVer();
		  exit( 0 );
		} 

		# do the normal build
		else
		{
		  print "   Doing a normal Build.";
		}


	}

	# Flags indicate whether or not we've already seen the game name on the command line.
	my $sawExample = 0;
	my $sawUT = 0;
	my $sawWar = 0;

	# We'll accumulate games to build here.
	my @gamesList;

	foreach $argnum (0 .. $#ARGV )
	{
		$curArg = $ARGV[$argnum];
		$curArg =~ tr/A-Z/a-z/;		# Convert to lowercase.  Ignores locale!

		if ( $curArg =~ 'example' && !$sawExample )
		{
			#print "saw example\n";
			$sawExample = 1;
			push @gamesList, 'Example';
		}
		elsif ( $curArg eq 'ut' && !$sawUT )
		{
			#print "saw ut\n";
			$sawUT = 1;
			push @gamesList, 'UT';
		}
		elsif ( $curArg eq 'war' && !$sawWar )
		{
			#print "saw war\n";
			$sawWar = 1;
			push @gamesList, 'War';
		      }
		elsif ( $curArg eq 'incbuild' && !$sawWar )
		{
			#print "saw incbuild\n";
			$incrementalBuild = 1;
		}
	}

	# If we saw any games games to build, overide the default of all games.
	if ( $sawExample || $sawUT || $sawWar )
	{
		@buildGames = @gamesList;
	}
	
	my $numGames = $#buildGames+1;
	if ( $numGames == 3)
	{
		$buildAllGames = 1;
	}
	else
	{
		$buildAllGames = 0;
	}

	print $lineSpacer;
	print "Games to build:";
	foreach $gameMode ( @buildGames )
	{
		print "  $gameMode";
	}

	print "\nBuild all games:  $buildAllGames\n";
	print "Incremental build:  $incrementalBuild\n";
}

########################################################################
# main

# Parse command line arguments.
parseCommandLineArgs;

# Adopt developer paths/settings if in developer mode.
setDeveloper;

# Make sure the compile log directory exists.
if( !(-e $buildLogDir) )
{
	system("mkdir $buildLogDir");
}

# Make sure the directory for failed builds exists.
if ( !(-e $failedLogDir) )
{
	system("mkdir $failedLogDir");
}

# Don't allow any UE3 builds if Avaris is in the middle of a build.
if ( -e $avarisBuildFile )
{
	print "Avaris build file \"$avarisBuildFile\" exists!\n";
	announceFailure( "Avaris build currently underway; build", "<no log file>" );
	print "Avaris build currently underway; build refused\n\n";
	exit( 0 );
}
else
{
	print "Can't see Avaris build file \"$avarisBuildFile\"\nProceeding with build\n";
	print $lineSpacer;
}

if ( 0 ) {
compareDocumentationForFile( "test.h" ); # baseclass.h" );
#compareDocumentationForFile( "..\\Src\\Core\\Inc\\Core.h" ); # ..\\Src\\Core\\Inc\\UnArc.h" );
print "UndocumentedList:\n";
foreach $line ( @g_undocumentedEntities )
{
	print "$line\n";
}
exit( 0 );
}

# Fire off the build.
dobuild;
