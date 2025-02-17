#!/usr/bin/perl.exe

$buildflag				= '\\\\build-server-01\\BuildFlags\\buildUE3.txt';
$buildflagQA				= '\\\\build-server-01\\BuildFlags\\buildUE3QA.txt';
$buildflagPCOnly			= '\\\\build-server-01\\BuildFlags\\buildUE3PCOnly.txt';
$buildflagPCLTCG			= '\\\\build-server-01\\BuildFlags\\buildUE3PCLTCG.txt';
$buildflagPCAndXenon			= '\\\\build-server-01\\BuildFlags\\buildUE3PCAndXenon.txt';

$buildflagGemini			= '\\\\build-server-01\\BuildFlags\\buildUE3Gemini.txt';
$buildflagGeminiPSC_All			= '\\\\build-server-01\\BuildFlags\\buildUE3GeminiPSC_ALL.txt';
$buildflagGeminiPSC_Example		= '\\\\build-server-01\\BuildFlags\\buildUE3GeminiPSC_Example.txt';
$buildflagGeminiPSC_UT			= '\\\\build-server-01\\BuildFlags\\buildUE3GeminiPSC_UT.txt';
$buildflagGeminiPSC_Warfare		= '\\\\build-server-01\\BuildFlags\\buildUE3GeminiPSC_Warfare.txt';


$buildflagGears 			= '\\\\build-server-01\\BuildFlags\\buildUE3Gears.txt';
$buildflagGoWE3LTCG 			= '\\\\build-server-01\\BuildFlags\\buildUE3GoWE3LTCG.txt';

$buildBeingVetted		      	= '\\\\build-server-01\\BuildFlags\\QABuildIsBeingVetted.txt';
$buildQASemaphore		      	= '\\\\build-server-01\\BuildFlags\\OnlyUseQABuildSemaphore.txt';




$buildscript		     = 'C:\\Build_UE3\\UnrealEngine3\\Development\\Build\\Build.pl';
$buildscriptGemini	     = 'C:\\Build_UE3\\UnrealEngine3\\Development\\Build\\BuildGemini.pl';
$buildscriptGears	     = 'C:\\Build_UE3\\UnrealEngine3-Gears\\Development\\Build\\Build_Gears.pl';

$clientspec       = 'UnrealEngine3Build-UberServer';

# Make sure everything is cleaned up from a halted build.
system( "p4 -c \"$clientspec\" revert -c default \/\/\.\.\. 2>&1" );
system( "del $buildflag /f" );

while(1)
{
	print "Checking for UE3 buildflag...\n";
	if( ( -e $buildflagPCOnly )
	    && ( !( -e $buildBeingVetted ) )
	  )
	{
		print "Doing a PC only build!\n";
		system( "$buildscript doPCOnlyBuild" );
		system( "del $buildflagPCOnly /f" );
	}
	elsif( ( -e $buildflagGears ) )
	{
		print "Doing a Gears build!\n";
		system( "$buildscriptGears" );
		system( "del $buildflagGears /f" );
	}
	elsif( ( -e $buildflagGoWE3LTCG ) )
	{
		print "Doing a GoW E3 LTCG build!\n";
		system( "$buildscriptGoWE3 LTCG" );
		system( "del $buildflagGoWE3LTCG /f" );
	}
	elsif( ( -e $buildflagPCAndXenon )
	    && ( !( -e $buildBeingVetted ) )
	  )
	{
		print "Doing a PC and Xenon build!\n";
		system( "$buildscript doPCAndXenonBuild" );
		system( "del $buildflagPCAndXenon /f" );
	}
	elsif( ( -e $buildflagPCLTCG )
	    && ( !( -e $buildBeingVetted ) )
	  )
	{
		print "Doing a PC LTCG build!\n";
		system( "$buildscript doPCLTCGBuild" );
		system( "del $buildflagPCLTCG /f" );
	}
	elsif( ( -e $buildflagQA )
	    && ( !( -e $buildBeingVetted ) )
	    && ( !( -e $buildQASemaphore ) )
	  )
	{
		print "Doing a QA build!\n";
		system( "$buildscript doQABuild" );
		system( "del $buildflagQA /f" );
	}
	elsif( ( -e $buildflag )
	    && ( !( -e $buildBeingVetted ) )
	  )
	{
		print "Doing a build!\n";
		system( "$buildscript" );
		system( "del $buildflag /f" );
	}
	elsif( ( -e $buildflagGemini ) )
	{
		print "Doing a Gemini build!\n";
		system( "$buildscriptGemini" );
		system( "del $buildflagGemini /f" );
	}
	else
	{
		sleep(10);
	}

	if( -e $buildBeingVetted )
	{
		print "A QA Build is being Vetted.  No builds will occur.\n";
	}

}

