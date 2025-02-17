/**
 * Gears2 specific version of achievements data provider.  Needed only so that enums from GearTypes can be used for AchievementId in the
 * list of icons.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearDataProvider_PlayerAchievements extends UIDataProvider_PlayerAchievements
	Config(Game);

/**
 * A markup string for an achievement icon texture.
 */
struct AchievementIconReference
{
	/** the id of the achievement */
	var		const	EGearAchievement	AchievementId;
	/** the markup string for the icon texture */
	var		const	string				AchievementIconMarkup;
	/** the markup string for the locked icon texture */
	var		const	string				LockedAchievementIconMarkup;
};

var	config		array<AchievementIconReference>				AchievementIcons;

/**
 * Loads the achievement icons from the .ini and applies them to the list of achievements.
 */
function PopulateAchievementIcons()
{
	local int IconIndex, AchievementIndex;
	local UIProviderFieldValue FieldValue;
	local string IconMarkup;

	`log(`location @ `showvar(AchievementIcons.Length,NumIcons) @ `showvar(Achievements.Length,NumAchievements),,'DevDataStore');
	for ( IconIndex = 0; IconIndex < AchievementIcons.Length; IconIndex++ )
	{
		AchievementIndex = Achievements.Find('Id', AchievementIcons[IconIndex].AchievementId);
		if ( AchievementIndex != INDEX_NONE )
		{
			if ( Achievements[AchievementIndex].Image == None )
			{
				IconMarkup = (Achievements[AchievementIndex].bWasAchievedOnline || Achievements[AchievementIndex].bWasAchievedOffline)
					? AchievementIcons[IconIndex].AchievementIconMarkup
					: AchievementIcons[IconIndex].LockedAchievementIconMarkup;

				if ( GetDataStoreFieldValue(IconMarkup, FieldValue) )
				{
					Achievements[AchievementIndex].Image = FieldValue.ImageValue;
				}
				else
				{
					`log(`location @ "failed to resolve icon markup for AchievementIcon" @ IconIndex $ ":"
						@ `showvar((Achievements[AchievementIndex].bWasAchievedOnline || Achievements[AchievementIndex].bWasAchievedOffline),Completed)
						@ `showvar(AchievementIcons[IconIndex].AchievementId,AchievementId)
						@ ((Achievements[AchievementIndex].bWasAchievedOnline || Achievements[AchievementIndex].bWasAchievedOffline)
							? `showvar(AchievementIcons[IconIndex].AchievementIconMarkup,IconMarkup)
							: `showvar(AchievementIcons[IconIndex].LockedAchievementIconMarkup,LockedIconMarkup))
					);
				}
			}
		}
		else
		{
			`log(`location @ "couldn't find matching achievement for AchievementIcon" @ IconIndex $ ":"
				@ `showvar(AchievementIcons[IconIndex].AchievementId,AchievementId) @ `showvar(AchievementIcons[IconIndex].AchievementIconMarkup,IconMarkup));
		}
	}
}

/**
 * Wrapper for retrieving the image markup for an achievement's icon.
 */
function string GetAchievementIconPathName( int AchievementIdx, optional bool bReturnLockedIcon )
{
	local int IconIndex;
	local EGearAchievement AchievementId;
	local string Result;

	AchievementId = EGearAchievement(byte(AchievementIdx));
	IconIndex = AchievementIcons.Find('AchievementId', AchievementId);
	if ( IconIndex != INDEX_NONE )
	{
		Result = bReturnLockedIcon
			? AchievementIcons[IconIndex].LockedAchievementIconMarkup
			: AchievementIcons[IconIndex].AchievementIconMarkup;
	}

	return Result;
}

DefaultProperties
{

}
