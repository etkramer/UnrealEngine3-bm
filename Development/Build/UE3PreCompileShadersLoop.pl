#!/usr/bin/perl.exe

$buildflag				= '\\\\build-server\\BuildFlags\\buildUE3.txt';
$buildflagQA				= '\\\\build-server\\BuildFlags\\buildUE3QA.txt';
$buildflagPCOnly			= '\\\\build-server\\BuildFlags\\buildUE3PCOnly.txt';
$buildflagPCLTCG			= '\\\\build-server\\BuildFlags\\buildUE3PCLTCG.txt';
$buildflagPCAndXenon			= '\\\\build-server\\BuildFlags\\buildUE3PCAndXenon.txt';

$buildflag			= '\\\\build-server\\BuildFlags\\buildUE3.txt';
$buildflagPCS_Full			= '\\\\build-server-01\\BuildFlags\\buildUE3PCS_Full.txt';
$buildflagPCS_All			= '\\\\build-server-01\\BuildFlags\\buildUE3PCS_ALL.txt';
$buildflagPCS_Example		= '\\\\build-server-01\\BuildFlags\\buildUE3PCS_Example.txt';
$buildflagPCS_UT			= '\\\\build-server-01\\BuildFlags\\buildUE3PCS_UT.txt';
$buildflagPCS_Warfare		= '\\\\build-server-01\\BuildFlags\\buildUE3PCS_Warfare.txt';


$buildflagGoWE3 			= '\\\\build-server\\BuildFlags\\buildUE3GoWE3.txt';
$buildflagGoWE3LTCG 			= '\\\\build-server\\BuildFlags\\buildUE3GoWE3LTCG.txt';

$buildBeingVetted		      	= '\\\\build-server\\BuildFlags\\QABuildIsBeingVetted.txt';
$buildQASemaphore		      	= '\\\\build-server\\BuildFlags\\OnlyUseQABuildSemaphore.txt';




$buildscript		     = 'C:\\Build_UE3\\UnrealEngine3\\Development\\Build\\Build.pl';
$buildscriptPreCompileShaders	     = 'C:\\Build_UE3\\UnrealEngine3\\Development\\Build\\BuildPreCompiledShaders.pl';
$buildscriptGoWE3	     = 'C:\\Build_UE3\\UnrealEngine3\\Development\\Build\\BuildGoWE3.pl';

$clientspec       = 'REFSHADER-FARM_UE3';

# Make sure everything is cleaned up from a halted build.
system( "p4 -c \"$clientspec\" revert -c default \/\/\.\.\. 2>&1" );
system( "del $buildflag /f" );

while(1)
{
	print "Checking for UE3 precompile shaders...\n";



	if( ( -e $buildflagPCS_Full ) )
	{
		print "Doing a PCS Full build!\n";
		system( "del $buildflagPCS_Full /f" );
		system( "$buildscriptPreCompileShaders doBuildPreCompiledShaders_Full" );

	}
	elsif( ( -e $buildflagPCS_All ) )
	{
		print "Doing a PCS All build!\n";
		system( "del $buildflagPCS_All /f" );
		system( "$buildscriptPreCompileShaders doBuildPreCompiledShaders_All" );

	}
	elsif( ( -e $buildflagPCS_Example ) )
	{
		print "Doing a PCS Example build!\n";
		system( "del $buildflagPCS_Example /f" );
		system( "$buildscriptPreCompileShaders doBuildPreCompiledShaders_Example" );

	}
	elsif( ( -e $buildflagPCS_UT ) )
	{
		print "Doing a PCS UT build!\n";
		system( "del $buildflagPCS_UT /f" );
		system( "$buildscriptPreCompileShaders doBuildPreCompiledShaders_UT" );

	}
	elsif( ( -e $buildflagPCS_Warfare ) )
	{
		print "Doing a PCS Warfare build!\n";
		system( "del $buildflagPCS_Warfare /f" );
		system( "$buildscriptPreCompileShaders doBuildPreCompiledShaders_Warfare" );

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

