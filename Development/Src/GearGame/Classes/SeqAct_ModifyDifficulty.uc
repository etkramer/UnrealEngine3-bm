/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ModifyDifficulty extends SequenceAction
	config(Game)
	native;


// add acquisition time
// Response_MinEnemySeenTime
// Response_MinEnemyHearTime

/**
* We need a struct to wrap our array of values as the ImportText code path is whack.
**/
struct native AttributeRatingStruct
{
	var() Array<float> Values;
};


/**
* This array stores the 'rating' to 'percentage to multiply by' values for Accuracy.
* Currently, -2 is the 0th entry in this array.
* The default value is 1.0f (or 100%)
**/
var config AttributeRatingStruct AcurracyMultiplierValues;


/**
* This array stores the 'rating' to 'percentage to multiply by' values for Health.
* Currently, -2 is the 0th entry in this array.
* The default value is 1.0f (or 100%)
**/
var config AttributeRatingStruct HealthMultiplierValues;


/**
* This array stores the 'rating' to 'percentage to multiply by' values for AcquisitionTime.
* Currently, -2 is the 0th entry in this array.
* The default value is 1.0f (or 100%)
**/
var config AttributeRatingStruct AcquisitionTimeMultiplierValues;



/** This will adjust the over all health of the mob spawned **/
var() int HealthRating;

/** This will adjust the over all accuracy of the mob spawned **/
var() int AccuracyRating;

/** This will adjust the over all acquisition of the mob spawned **/
var() int AcquisitionTimeRating;

cpptext
{
	void Activated();

	void DeActivated();

	virtual void PostEditChange( UProperty* PropertyThatChanged );

	/**
	* This will determine how much we multiply the Accuracy values of the mobs spawned by this factory.
	* The factory will look at the rating value it has for accuracy and then look up in the
	* AcurracyMultiplierValues to get the percentage to multiply the base value by.
	* Separate from the Accuracy as we may want a different number of ratings
	**/
	FLOAT DetermineAccuracyMultiplier() const;


	/**
	* This will determine how much we multiply the Health values of the mobs spawned by this factory.
	* The factory will look at the rating value it has for accuracy and then look up in the
	* AcurracyMultiplierValues to get the percentage to multiply the base value by.
	* Separate from the Accuracy as we may want a different number of ratings
	**/
	FLOAT DetermineHealthMultiplier() const;


	FLOAT DetermineAcquisitionTimeMultiplier() const;

};


/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 0;
}


defaultproperties
{
	ObjName="Modify Difficulty"
	ObjCategory="Gear"



}
