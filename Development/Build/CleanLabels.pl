#!/usr/bin/perl.exe

###############################################################################
#
# This script deletes all unlocked Perforce labels that point to 0 files.
#
################################################################################

$emptyLabelLine = 'no such file\(s\)\.';		# The "p4 files" output indicating 0 files.

# Get a list of all labels in the repository.
open( LABELLIST, "p4 labels |");
chomp( @labelList = <LABELLIST> );
close( LABELLIST );

# Delete all labels that point to 0 files.
foreach $labelDesc ( @labelList )
{
	# Grab the label name.  Perforce output from "p4 labels" is of the form
	# "Label labelname 2004/02/25 'description goes here'"
	if ( $labelDesc =~ /^Label ([\w\-\[\]\.]+) / )
	{
		$undecoratedLabel = $1;						# The label name.
		$decoratedLabel = "@".$undecoratedLabel;	# The label name prepended with a @.

		#######################################################################
		# Get a list of files for the label with a call to "p4 files".
		# If the label is empty, output will be of the form "labelname - no such files(s)."

		print "Checking $undecoratedLabel . . . ";
		if ( open( LABELFILES, "p4 files $decoratedLabel 2>&1 |") )
		{
			chomp( @labelFileList = <LABELFILES> );
			close( LABELFILES );

			if ( $labelFileList[0] =~ /$emptyLabelLine/ )
			{
				print "empty.  Deleting . . . ";
				system( "p4 label -d $undecoratedLabel" );
			}
			else
			{
				print "non-empty.  Leaving.\n";
			}
		}
	}
}
