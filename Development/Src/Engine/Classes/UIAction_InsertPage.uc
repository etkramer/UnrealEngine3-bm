/**
 * Adds a new page to a tab control at the specified location.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_InsertPage extends UIAction_TabControl;

var()	class<UITabPage>	TabPageClass;

/**
 * The position [into the tab control's Pages array] to insert the new tab page.  If not specified (value of INDEX_NONE),
 * will be added to the end of the list.
 */
var()	int					InsertIndex;

/**
 * Specifies whether the page should also be given focus once it's successfully added to the tab control.
 */
var()	bool				bFocusPage;

/**
 * Reference to an existing page instance.  If non-NULL, this page will be inserted rather than creating a new one.
 */
var		UITabPage			PageToInsert;

/**
 * Optional UIPrefab to use as the template for the new page.  If specified, this will be used instead of the value of TabPageClass.
 */
var		UITabPage			PagePrefab;

/* == Events == */
/**
 * Called when this event is activated.  Performs the logic for this action.
 */
event Activated()
{
	local bool bSuccess;
	local UITabPage NewPage;
	local SeqVar_Object ObjVar;

	Super.Activated();

	if ( TabControl != None
	&&	(TabPageClass != None || PageToInsert != None) )
	{
		NewPage = PageToInsert;
		if ( NewPage == None )
		{
			if ( PagePrefab != None )
			{
				TabPageClass = PagePrefab.Class;
			}

			NewPage = TabControl.CreateTabPage( TabPageClass, PagePrefab );
		}

		if ( NewPage != None )
		{
			bSuccess = TabControl.InsertPage(NewPage, PlayerIndex, InsertIndex, bFocusPage);
			foreach LinkedVariables( class'SeqVar_Object', ObjVar, "New Page" )
			{
				ObjVar.SetObjectValue(NewPage);
			}
		}
	}

	if ( bSuccess )
	{
		if ( !OutputLinks[0].bDisabled )
		{
			OutputLinks[0].bHasImpulse = true;
		}
	}
	else
	{
		if ( !OutputLinks[1].bDisabled )
		{
			OutputLinks[1].bHasImpulse = true;
		}
	}
}

DefaultProperties
{
	TabPageClass=class'Engine.UITabPage'
	InsertIndex=INDEX_NONE
	bFocusPage=true


	ObjName="Add Page"

	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Position",PropertyName=InsertIndex,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Page To Insert",PropertyName=PageToInsert,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Page Prefab",PropertyName=PagePrefab,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="New Page",bWriteable=true))
}
