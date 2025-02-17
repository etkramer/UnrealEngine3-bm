/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTDrawMapVotePanel extends UTDrawPanel;

var transient UTVoteReplicationInfo VoteRI;
var transient int TopIndex, Index;

var() font 	TextFont;
var() color TextColor;
var() color SelectionTextColor;

event PostInitialize()
{
	Super.PostInitialize();
	OnProcessInputKey=ProcessInputKey;
}


function InitializeVRI(UTVoteReplicationInfo NewVoteRI)
{
	VoteRI = NewVoteRI;
}

event DrawPanel()
{
	local float XL,YL, XPos, YPos, FScaleX, FScaleY;
	local float ItemsPerPage;
	local Vector2D ViewportSize;

	local int NoMaps;
	local int RenderIndex;

	local string s;

	if ( VoteRI == none )
	{
		return;
	}

	// Figure out the sizing


	// -- First, the size of a standard line of text

	Canvas.Font = TextFont;
	Canvas.StrLen("WQ",XL,YL);

	GetViewportSize(ViewportSize);

	FScaleY = ViewportSize.Y / 768;
	FScaleX = FScaleY * (1024/768);

	XL *= FScaleX;
	YL *= FScaleY;


	ItemsPerPage = Canvas.ClipY / YL;


	NoMaps = VoteRI.Maps.Length;

	Index = Clamp(Index, 0,NoMaps-1);

	if (NoMaps > ItemsPerPage)
	{
		// Make sure we can see the selection in case of a
		// resolution switch.

		if (Index - TopIndex > ItemsPerPage)
		{
			TopIndex = Index - (ItemsPerPage / 2); // Center it
		}
	}

	// Render the Entries

	RenderIndex = TopIndex;
	XPos = 0;
	YPos = YPos;
	while (YPos < Canvas.ClipY && RenderIndex < VoteRI.Maps.Length)
	{
		Canvas.SetPos(XPos,YPos);
		Canvas.DrawColor = RenderIndex == Index ? SelectionTextColor : TextColor;

		S = (VoteRI.Maps[RenderIndex].NoVotes > 0) ? string(VoteRI.Maps[RenderIndex].NoVotes) : "--";
		S = S @ VoteRI.Maps[RenderIndex].Map;

		Canvas.DrawText(S,,FScaleX,FScaleY);

		YPos += YL;
		RenderIndex++;
	}

}

event GetSupportedUIActionKeyNames(out array<Name> out_KeyNames )
{
	out_KeyNames[out_KeyNames.Length] = 'SelectionUp';
	out_KeyNames[out_KeyNames.Length] = 'SelectionDown';
	out_KeyNames[out_KeyNames.Length] = 'Select';
	out_KeyNames[out_KeyNames.Length] = 'Back';
	out_keyNames[out_KeyNames.Length] = 'Click';
}


function bool ProcessInputKey( const out SubscribedInputEventParameters EventParms )
{
//	`log("### ProcessInputKey:"@EventParms.EventType@EventParms.InputAliasName);

    if ( VoteRI != none )
    {
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
		 	if ( EventParms.InputAliasName == 'SelectionUp' )
		 	{
				Index--;
			}
		 	else if ( EventParms.InputAliasName == 'SelectionDown' )
		 	{
		 		Index++;
			}
			else if ( EventParms.InputAliasName == 'Click' )
			{
				FindItemUnderCursor();
			}

		}
		else if (EventPArms.EventType == IE_Released )
		{
			if ( EventPArms.InputAliasName == 'Select' )
			{
				VoteRI.ServerRecordVoteFor( VoteRI.Maps[Index].MapID );
			}

			if ( EventParms.InputAliasName == 'Back' )
			{
//				`log("### Back");
				GetScene().CloseScene();
			}
		}
	}

	return true;
}

function FindItemUnderCursor()
{
	local int cX, cY;
	local float mX, mY;
	local float H,W,S;
	local int NewIndex;
	local vector2D ViewportSize;

	H = TextFont.GetMaxCharHeight();
	class'UIRoot'.static.GetCursorPosition( cX, cY );
	mX = cX - GetPosition(UIFACE_Left, EVALPOS_PixelViewport, true);
	mY = cY - GetPosition(UIFACE_Top, EVALPOS_PixelViewport, true);

   	GetViewportSize(ViewportSize);

	S = ViewportSize.Y / 768;

	W = GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport);

	if ( mX > 0 && mX < W && mY > 0 )
	{
		NewIndex = int( mY / (H * S));
		if ( NewIndex >0 && NewIndex < VoteRI.Maps.Length )
		{
			Index = NewIndex;
			VoteRI.ServerRecordVoteFor( VoteRI.Maps[Index].MapID );
		}
	}
}


defaultproperties
{
	TextFont=font'UI_Fonts_Final.Menus.Fonts_Positec'
	TextColor=(R=255,G=255,B=255,A=255)
	SelectionTextColor=(R=255,G=255,B=0,A=255)
}
