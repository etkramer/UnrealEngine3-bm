/**
 * This action allows designers to search for an existing open scene by scene tag.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class UIAction_FindScene extends UIAction_Scene
	HideCategories(UIAction_Scene);// don't show the editable variables from UIAction_Scene since they are output for this action.


/** the scene tag to search for */
var		transient	name	SceneTagToFind;

var		transient	int		IndexOfScene;

/* === SequenceOp interface === */
event Activated()
{
	local GameUISceneClient SceneClient;
	local LocalPlayer PlayerOwner;
	local int OutputLinkToActivate;

	Super.Activated();

	Scene = None;
	IndexOfScene=INDEX_NONE;
	if ( SceneTagToFind != 'None' )
	{
		SceneClient = class'UIRoot'.static.GetSceneClient();
		if ( SceneClient != None )
		{
			// GamePlayers is actually a member of Engine, but since GameUISceneClient is declared within UIInteraction,
			// UIInteraction is declared within GameViewportClient, and GameViewportClient is declared within Engine, we can
			// access the GamePlayers array easily from script
			if ( PlayerIndex >= 0 && PlayerIndex < SceneClient.GamePlayers.Length )
			{
				PlayerOwner = SceneClient.GamePlayers[PlayerIndex];
			}

			IndexOfScene = SceneClient.FindSceneIndexByTag(SceneTagToFind, PlayerOwner);
			Scene = SceneClient.FindSceneByTag(SceneTagToFind, PlayerOwner);
		}
	}

	OutputLinkToActivate = (Scene != None) ? ACTION_SUCCESS_INDEX : ACTION_FAILURE_INDEX;
	OutputLinks[OutputLinkToActivate].bHasImpulse = true;
}

DefaultProperties
{
	ObjName="Find Scene"
	IndexOfScene=INDEX_NONE

	// override the Scene variable link to make it writeable, since it's an output value in this action
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Scene",PropertyName=Scene,bWriteable=true)

	// remove the Override Priority link, since we're not manipulating the scene stack.
	VariableLinks.Remove((ExpectedType=class'SeqVar_Byte',LinkDesc="Override Priority",PropertyName=ForcedScenePriority,bHidden=true))

	// add a link for the search tag, an input value
	VariableLinks.Add((ExpectedType=class'SeqVar_Name',LinkDesc="Search Tag",PropertyName=SceneTagToFind))

	// add a link for the scene's locatio in the scene stack, an output value
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Stack Index",PropertyName=IndexOfScene,bWriteable=true))
}
