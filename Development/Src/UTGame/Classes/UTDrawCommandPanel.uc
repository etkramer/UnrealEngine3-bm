/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTDrawCommandPanel extends UTDrawPanel;


var transient UTHud MyHud;
var transient UTUIScene_Hud UTHudScene;
var transient UTPlayerController MyPlayerOwner;
var transient float LastRenderTime;
var transient float TransitionTimer;

var transient string AllCaption;
var transient string MoreCaption;


var() Color TextColor;
var() Color SelectedColor;
var() Color DisabledColor;

var() texture2D SelTexture;
var() TextureCoordinates SelTexCoords;
var() color SelTexColor;

var() texture2D BkgTexture;
var() TextureCoordinates BkgTexCoords;
var() color BkgTexColor;


var() float TransitionTime;


enum EListType
{
	ELT_Main,
	ELT_Ack,
	ELT_FriendlyFire,
	ELT_Order,
	ELT_Taunt,
	ELT_PlayerSelect,
};

struct ECmdData
{
	var bool bDisabled;
	var name EmoteTag;
	var UTPlayerReplicationInfo PRI;
	var string Caption;
};

struct ECmdListData
{
	var int Selection;
	var int Top;
	var array<ECmdData> Values;
};

/** The 3 lists */

var transient ECmdListData MyLists[3];

/** Which list has focus */
var int FocusedList;
var int LastFocusedList;

/*** LOCALIZE ME */
struct CategoryInfo
{
	var name Category;
	var localized string FriendlyName;
};

var array<CategoryInfo> CategoryNames;

var transient array<name> Lookup;

function class<UTFamilyInfo> MyFamilyInfo()
{
	local UTPlayerReplicationInfo PRI;

	if (MyPlayerOwner != none)
	{
		PRI = UTPlayerReplicationInfo(MyPlayerOwner.PlayerReplicationInfo);
		return PRI.CharClassInfo;
	}
	return none;
}

function UTPawn MyPawn()
{
	if ( MyPlayerOwner != none && MyPlayerOwner.Pawn != none )
	{
		if ( Vehicle(MyPlayerOwner.Pawn) != none )
		{
			return UTPawn(Vehicle(MyPlayerOwner.Pawn).Driver);
		}

		return UTPawn(MyPlayerOwner.Pawn);
	}
	return none;
}

event PostInitialize()
{
	Super.PostInitialize();

	UTHudScene = UTUIScene_Hud( GetScene() );
	MyHud = UTHudScene.GetPlayerHud();
	MyPlayerOwner = UTHudScene.GetUTPlayerOwner();
/*
	if ( MyPawn() == None )
	{
		return;
	}
*/
	OnProcessInputKey=ProcessInputKey;

	// Initialize the main groups
	InitGroupList();
	InitPRIList();
	FillEmoteList( MyLists[0].Values[0].EmoteTag );
}

function NotifyGameSessionEnded()
{
	MyHud = none;
}

function InitGroupList()
{
	local int i;
	local class<UTFamilyInfo> MyFInfo;

	MyFInfo = MyFamilyInfo();

	if ( MyFInfo != none )
	{
		MyLists[0].Values.Length = CategoryNames.Length;
		for (i=0;i<CategoryNames.Length;i++)
		{
			MyLists[0].Values[i].Caption = CategoryNames[i].FriendlyName;
			MyLists[0].Values[i].EmoteTag = CategoryNames[i].Category;
			MyLists[0].Values[i].bDisabled = MyFInFo.static.GetEmoteGroupCnt(CategoryNames[i].Category) <= 0;
		}
	}
}

function InitPRIList()
{
	local int i,cnt,per;
	local GameReplicationInfo GRI;

	GRI = UTHudScene.GetWorldInfo().GRI;

	Cnt = 0;
	Per = 0;
	for (i=0;i<GRI.PRIArray.Length;i++)
	{
		if ( GRI.PRIArray[i] != none && !GRI.PRIArray[i].bOnlySpectator && GRI.PRIArray[i] != MyPlayerOwner.PlayerReplicationInfo &&
			GRI.OnSameTeam(GRI.PRIArray[i], MyPlayerOwner) )
		{
			MyLists[2].Values.Length = Cnt+1;
			MyLists[2].Values[Cnt].Caption = GRI.PRIArray[i].GetPlayerAlias();
			MyLists[2].Values[Cnt].PRI = UTPlayerReplicationInfo(GRI.PRIArray[i]);
			MyLists[2].Values[Cnt].bDisabled = false;

			per++;
			cnt++;
			if (per >= 8)
			{
				MyLists[2].Values[cnt].Caption = MoreCaption;
				MyLists[2].Values[cnt].EmoteTag = 'More';
				per = 0;
				cnt++;
			}
		}
	}

	if (Cnt > 9)	// If we have more than 9 entries, add a final "more" box.
	{
		MyLists[2].Values[cnt].Caption = MoreCaption;
		MyLists[2].Values[cnt].EmoteTag = 'More';
	}

	// Add the ALL

	MyLists[2].Values.Insert(0,1);
	MyLists[2].Values[0].Caption = AllCaption;
	MyLists[2].Values[0].EmoteTag = 'All';
	MyLists[2].Values[0].bDisabled = false;

}

function FillEmoteList(name Category)
{
	local int i,cnt,per;
	local class<UTFamilyInfo> MyFInfo;
	local array<string> Captions;
	local array<name> EmoteTags;

	// Clear the old one

	MyLists[1].Selection = 0;
	if ( MyLists[1].Values.Length > 0)
	{
		MyLists[1].Values.Remove(0,MyLists[1].Values.Length);
	}

	MyFInfo = MyFamilyInfo();

	if ( MyFInfo != none )
	{
		Cnt = 0;
		Per = 0;
		MyFInfo.static.GetEmotes(Category, Captions, EmoteTags);
		for (i=0;i<Captions.Length;i++)
		{
			MyLists[1].Values.Length = cnt + 1;
			MyLists[1].Values[Cnt].Caption = Captions[i];
			MyLists[1].Values[Cnt].EmoteTag = EmoteTags[i];
			MyLists[1].Values[Cnt].bDisabled= false;

			per++;
			cnt++;
			if (per>9)
			{
				MyLists[1].Values.Length = cnt + 1;
				per=0;
				MyLists[1].Values[Cnt].Caption = MoreCaption;
				MyLists[1].Values[Cnt].EmoteTag = EmoteTags[i];
				cnt++;
			}

		}

		if (Cnt > 9)
		{
				MyLists[1].Values.Length = cnt + 1;
				MyLists[1].Values[Cnt].Caption = MoreCaption;
				MyLists[1].Values[Cnt].EmoteTag = EmoteTags[i];
		}
	}
}


function float DrawList(float XPos, float PercX, float Alpha, bool bShowSelection, int Idx)
{
	local int i, cnt;
	local float XL,YL, Width, Height, YPos;
	local string s;

	if ( MyLists[Idx].Values.Length <= 0 )
	{
		return 0;
	}

	Cnt = MyLists[Idx].Values.Length;

	if ( Cnt>9 )
	{
		Cnt = Clamp(Cnt - MyLists[Idx].Top,0,10);
	}

	// Size the list

	for (i=0;i<Cnt;i++)
	{

		s = (i<10 ? string(i+1) : "0")$". "$MyLists[Idx].Values[MyLists[Idx].Top+i].Caption;
		Canvas.StrLen(s,xl,yl);
		if (xl > Width)
		{
			Width = XL;
		}
		Height += YL;
	}

	YPos = (Canvas.ClipY * 0.5) - (Height * 0.5);

	Canvas.DrawColor = BkgTexColor;
	Canvas.SetPos(XPos - (Width * PercX), YPos);
	Canvas.DrawTile(BkgTexture,Width,Height,BkgTexCoords.U,BkgTexCoords.V,BkgTexCoords.UL,BkgTexCoords.VL);


	// Draw it.

	for (i=0;i<Cnt;i++)
	{
		s = (i<10 ? string(i+1) : "0")$". "$MyLists[Idx].Values[MyLists[Idx].Top+i].Caption;

		if ( bShowSelection && MyLists[Idx].Selection == i )
		{
			Canvas.SetPos(XPos - (Width * PercX), YPos);
			Canvas.DrawColor = SelTexColor;
			Canvas.DrawTile(SelTexture,Width,YL,SelTexCoords.U,SelTexCoords.V,SelTexCoords.UL,SelTexCoords.VL);
		}

		if ( MyLists[Idx].Values[MyLists[Idx].Top+i].bDisabled )
		{
			Canvas.DrawColor = DisabledColor;
		}
		else if ( bShowSelection && MyLists[Idx].Selection == i )
		{
			Canvas.DrawColor = SelectedColor;
		}
		else
		{
			Canvas.DrawColor = TextColor;
		}

		Canvas.DrawColor.A = Alpha;
		Canvas.SetPos(XPos - (Width * PercX), YPos);
		Canvas.DrawText(s);
		YPos += YL;
	}

	Canvas.StrLen("WW",XL,YL);
	Width += XL;

	return Width;
}



event DrawPanel()
{
	local float XPos,MaxWidth, Perc, DeltaTime;
	local float A;

	// Hijack Perc for a sec while calculating DeltaTime
	Perc = 	UTHudScene.GetWorldInfo().TimeSeconds;
	DeltaTime = Perc - LastRenderTime;
	LastRenderTime = Perc;
/*
	// If we no longer have a pawn, all the data is invalid, exit!
	if ( MyPawn() == none )
	{
		UTHudScene.CloseScene();
		return;
	}
*/

	Canvas.Font = class'UTHud'.default.HudFonts[1];
	XPos = 0;

	if ( MyHud != none )
	{
		// Update the Transition Timer
		if (TransitionTimer > 0)
		{
			TransitionTimer = FClamp(TransitionTimer - DeltaTime, 0.0 , TransitionTime);
		}

		Perc = 1.0 - (TransitionTimer / TransitionTime);

		// First, try to draw the group stack.

		if (FocusedList > 0)
		{
			// If Perc is >= 1.0 we have fully transitioned.

			if (Perc < 1.0)
			{

				if (LastFocusedList < FocusedList)
				{
					MaxWidth = DrawList(XPos, Perc, 255 * (1.0 - Perc), false, FocusedList - 1);
					XPos = MaxWidth * (1.0 - Perc);
				}
			}
		}


		// Draw the Center Stack....

		if (LastFocusedList > FocusedList && Perc < 1.0)
		{
			MaxWidth = DrawList(XPos, (1.0-Perc), Max( (255 * Perc), 64), true, FocusedList);
			XPos += MaxWidth * Perc;
		}
		else
		{
			MaxWidth = DrawList(XPos, 0.0, Max( (255 * Perc), 64), true, FocusedList);
			XPos += MaxWidth;
		}

		// Draw the Right List...

		if ( FocusedList <1 || NeedsPRIList() )
		{
			if ( FocusedList > LastFocusedList )
			{
				A = 64 * Perc;
			}
			else
			{
				A = int(255.0 * (1.0 - Perc));
				A = Clamp(A,64,255);
			}
			Canvas.DrawColor.A = A;
			DrawList(XPos, 0.0, A, false, FocusedList+1);
		}
	}
}

function bool NeedsPRIList()
{
	local class<UTFamilyInfo> MyFInfo;
	local int i;
	local name EmoteTag;

	if (FocusedList>1)
	{
		return false;
	}

	MyFInfo = MyFamilyInfo();
	if ( MyFInfo != none )
	{
		EmoteTag = MyLists[FocusedList].Values[ MyLists[FocusedList].Selection].EmoteTag;
		for (i=0;i<MyFInfo.default.FamilyEmotes.length;i++)
		{
			if (MyFInfo.default.FamilyEmotes[i].EmoteTag == EmoteTag )
			{
				return MyFInfo.default.FamilyEmotes[i].bRequiresPlayer;
			}
		}
	}

	return false;
}

/**
 * Setup Input subscriptions
 */
event GetSupportedUIActionKeyNames(out array<Name> out_KeyNames )
{
	out_KeyNames[out_KeyNames.Length] = 'SelectionUp';
	out_KeyNames[out_KeyNames.Length] = 'SelectionDown';
	out_KeyNames[out_KeyNames.Length] = 'Select';
	out_KeyNames[out_KeyNames.Length] = 'Forward';
	out_KeyNames[out_KeyNames.Length] = 'Back';
	out_KeyNames[out_KeyNames.Length] = 'One';
	out_KeyNames[out_KeyNames.Length] = 'Two';
	out_KeyNames[out_KeyNames.Length] = 'Three';
	out_KeyNames[out_KeyNames.Length] = 'Four';
	out_KeyNames[out_KeyNames.Length] = 'Five';
	out_KeyNames[out_KeyNames.Length] = 'Six';
	out_KeyNames[out_KeyNames.Length] = 'Seven';
	out_KeyNames[out_KeyNames.Length] = 'Eight';
	out_KeyNames[out_KeyNames.Length] = 'Nine';
	out_KeyNames[out_KeyNames.Length] = 'Zero';
}


function AdjustListSelection(int Modification, int List)
{

	local int Sel;

	Sel = MyLists[List].Selection + MyLists[List].Top;
	if (MyLists[List].Values[Sel].EmoteTag == 'More' && Modification > 0)
	{
		MyLists[List].Top += MyLists[List].Selection+1;
		MyLists[List].Selection = 0;

		if (MyLists[List].Top >= MyLists[List].Values.Length)
		{
			MyLists[List].Top = 0;
		}
		return;
	}

	MyLists[List].Selection = Clamp( MyLists[List].Selection + Modification, 0, MyLists[List].Values.Length-1);
	if (List == 0)
	{
		MyLists[2].Top = 0;
		MyLists[2].Selection = 0;
		FillEmoteList( MyLists[0].Values[MyLists[0].Selection].EmoteTag );
	}

}


function bool ProcessInputKey( const out SubscribedInputEventParameters EventParms )
{

	local bool bSelect;
	local int idx, ActualIdx;

	if ( MyHud != none && EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
	{
		Idx = Lookup.Find(EventParms.InputAliasName);
		if (Idx > INDEX_None)
		{
			ActualIdx = Idx + MyLists[FocusedList].Top;
			if (ActualIdx < MyLists[FocusedList].Values.Length)
			{
				MyLists[FocusedList].Selection = Idx;
				if (FocusedList == 0)
				{
					FillEmoteList( MyLists[0].Values[MyLists[0].Selection].EmoteTag );
				}
				bSelect = true;
			}
		}

	 	if ( EventParms.InputAliasName == 'SelectionUp' )
	 	{
	 		AdjustListSelection(-1,FocusedList);
	 		return true;
		}
	 	else if ( EventParms.InputAliasName == 'SelectionDown' )
	 	{
	 		AdjustListSelection(+1,FocusedList);
	 		return true;
		}
	 	else if ( bSelect || EventParms.InputAliasName == 'Select' || EventParms.InputAliasName == 'Forward')
	 	{
	 		if (!MyLists[FocusedList].Values[MyLists[FocusedList].Selection].bDisabled)
	 		{
		 		// Before moving forward, actually try and select entry.  If we can,
		 		// perform any commands and then close the scene.

		 		if ( bSelect || EventParms.InputAliasName == 'Select' )
		 		{
		 			if ( FocusedList == 2 || EndOfChain() )
		 			{
						if ( PerformCommand() )
						{
							UTHudScene.CloseScene(UTHudScene);
						}
						return true;
		 			}
		 		}

				// We aren't ready to leave, attempt to move forward in the stack.

	 			if ( FocusedList<1 || NeedsPRIList() )
	 			{
					LastFocusedList = FocusedList;
					FocusedList = Clamp(FocusedList+1,0,2);
					TransitionTimer = TransitionTime;
				}
			}
	 		return true;

		}
		else if ( EventParms.InputAliasName == 'Back' )
		{
			// Move backwards in the stack or if at the front,
			// close the scene.

			if (FocusedList > 0)
			{
				LastFocusedList = FocusedList;
				FocusedList = Clamp(FocusedList-1,0,2);
				TransitionTimer = TransitionTime;
			}
			else
			{
				UTHudScene.CloseScene(UTHudScene);
			}
	 		return true;
		}
	}

	return false;
}

/**
 * What this function does depends on the list in play.  If it's the Emote list, then
 * it will look to see if there the emote requires a player id.  If this is the player list
 * then it always returns true since there is no where else to go.
 */
function bool EndOfChain()
{
	local class<UTFamilyInfo> MyFInfo;
	local int ActualIndex, EmoteIndex;

	if ( FocusedList > 0 )
	{
		if (FocusedList < 2)
		{

			ActualIndex = MyLists[FocusedList].Top + MyLists[FocusedList].Selection;

			MyFInfo = MyFamilyInfo();
			if ( MyFInfo != none )
			{
				EmoteIndex = MyFInfo.static.GetEmoteIndex( MyLists[FocusedList].Values[ActualIndex].EmoteTag );
				if ( EmoteIndex != INDEX_None )
				{
					if ( !MyFInfo.default.FamilyEmotes[EmoteIndex].bRequiresPlayer )
					{
						return true;
					}
				}
			}

			return false;
		}
		else
		{
			return true;
		}
	}

	return false;
}

function bool PerformCommand()
{
	local UTPawn P;
	local class<UTFamilyInfo> MyFInfo;
	local int BListIndex, EListIndex, EmoteIndex;;
	local int PlayerID;

	P = MyPawn();
	MyFInfo = MyFamilyInfo();
	if ( MyFInfo != none )
	{
		// Grab the indices
		EListIndex = MyLists[1].Top + MyLists[1].Selection;
		BListIndex = MyLists[2].Top + MyLists[2].Selection;
		EmoteIndex = MyFInfo.static.GetEmoteIndex( MyLists[1].Values[EListIndex].EmoteTag );

		// If we have a valid emote, perform it's command
		if (EmoteIndex != INDEX_None )
		{
			if ( MyFinfo.default.FamilyEmotes[EmoteIndex].bRequiresPlayer )
			{
				if ( MyLists[2].Values[BListIndex].EmoteTag == 'All' )
				{
					PlayerID = 255;
				}
				else if ( MyLists[2].Values[BListIndex].PRI != none )
				{
					PlayerID = MyLists[2].Values[BListIndex].PRI.PlayerID;
				}
				else
				{
					PlayerID = -1;
				}
			}
			else
			{
				PlayerID = 255;
			}

			if (MyFinfo.default.FamilyEmotes[EmoteIndex].CategoryName == 'Class' && MyPlayerOwner != none)
			{
				MyPlayerOwner.ChangeClass(string(MyFinfo.default.FamilyEmotes[EmoteIndex].EmoteTag));
			}
			else if (P != None)
			{
				P.PlayEmote(MyLists[1].Values[EListIndex].EmoteTag,PlayerID);
			}

			return true;
		}

	}

	return false;
}

defaultproperties
{
	FocusedList=0
	LastFocusedList=50000;

	TextColor=(R=255,G=255,B=255,A=255)
	SelectedColor=(R=255,G=255,B=0,A=255)
	DisabledColor=(R=128,G=128,B=128,A=255)

	TransitionTime=0.2

	CategoryNames(0)=(Category="Order")
	CategoryNames(1)=(Category="Taunt")
	CategoryNames(2)=(Category="Status")
	CategoryNames(3)=(Category="Class")

	AllCaption="[All]"
	MoreCaption="[More]"

	SelTexture=Texture2D'UI_HUD.HUD.UI_HUD_BaseC'
	SelTexCoords=(U=622,V=312,UL=274,VL=63)
	SelTexColor=(R=0,G=51,B=0,A=255)

	BkgTexture=Texture2D'UI_HUD.HUD.UI_HUD_BaseD'
	BkgTexCoords=(U=610,V=374,UL=164,VL=126)
	BkgTexColor=(R=0,G=20,B=20,A=200)

	Lookup(0)="One"
	Lookup(1)="Two"
	Lookup(2)="Three"
	Lookup(3)="Four"
	Lookup(4)="Five"
	Lookup(5)="Six"
	Lookup(6)="Seven"
	Lookup(7)="Eight"
	Lookup(8)="Nine"
	Lookup(9)="Zero"
}
