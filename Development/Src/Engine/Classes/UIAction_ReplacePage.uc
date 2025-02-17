/**
 * Replaces a tab page with a different tab page.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_ReplacePage extends UIAction_TabControl;

/** the class to use for the new page class */
var()	class<UITabPage>	TabPageClass;

/** reference to the page that should be removed */
var		UITabPage			PageToRemove;

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

	if ( (TabControl != None && PageToRemove != None)
	&&	(TabPageClass != None || PageToInsert != None) )
	{
		NewPage = PageToInsert;
		if ( NewPage == None )
		{
			// if we're using a prefab, we must use the same class as the prefab as well
			if ( PagePrefab != None )
			{
				TabPageClass = PagePrefab.Class;
			}

			NewPage = TabControl.CreateTabPage( TabPageClass, PagePrefab );
		}

		if ( NewPage != None )
		{
			bSuccess = TabControl.ReplacePage(PageToRemove, NewPage, PlayerIndex, bFocusPage);

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
	bFocusPage=true

	ObjName="Replace Page"
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Page To Remove",PropertyName=PageToRemove))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Page To Insert",PropertyName=PageToInsert,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Page Prefab",PropertyName=PagePrefab,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="New Page",bWriteable=true))
}
