/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_Gate extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated()
	{
		UBOOL bWasOpen = bOpen;
		// first look for an open/close impulse
		if (InputLinks(1).bHasImpulse)
		{
			// open the gate
			bOpen = TRUE;
		}
		else
		if (InputLinks(2).bHasImpulse)
		{
			// close the gate
			bOpen = FALSE;
		}
		else
		if (InputLinks(3).bHasImpulse)
		{
			// toggle the gate
			bOpen = !bOpen;
		}
		KISMET_LOG(TEXT("- Gate status: %s (was: %s)"),bOpen?TEXT("Open"):TEXT("Closed"),bWasOpen?TEXT("Open"):TEXT("Closed"));
		// next check for an activation impulse
		if (bOpen && InputLinks(0).bHasImpulse)
		{
			if (!OutputLinks(0).bDisabled && 
				!(OutputLinks(0).bDisabledPIE && GIsEditor))
			{
				OutputLinks(0).bHasImpulse = TRUE;
			}
			if (AutoCloseCount > 0 && ActivateCount >= AutoCloseCount)
			{
				bOpen = FALSE;
			}
		}
	}
}

/** Is this gate currently open? */
var() bool bOpen<autocomment=true>;

/** Auto close after this many activations */
var() int AutoCloseCount<autocomment=true>;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

defaultproperties
{
	ObjName="Gate"
	ObjCategory="Misc"

	bOpen=TRUE
	bAutoActivateOutputLinks=false

	InputLinks(0)=(LinkDesc="In")
	InputLinks(1)=(LinkDesc="Open")
	InputLinks(2)=(LinkDesc="Close")
	InputLinks(3)=(LinkDesc="Toggle")

	VariableLinks.Empty
}
