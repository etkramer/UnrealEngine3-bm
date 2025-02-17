/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

// GenericBrowserType
//
// This class provides a generic interface for extending the generic browsers
// base list of resource types.

class GenericBrowserType
	extends Object
	abstract
	hidecategories(Object,GenericBrowserType)
	native;

// A human readable name for this modifier
var string Description;

struct native GenericBrowserTypeInfo
{
	/** the class associated with this browser type */
	var const class				Class;

	/** the color to use for rendering objects of this type */
	var const color				BorderColor;

	/** if specified, only objects that have these flags will be considered */
	var native const qword		RequiredFlags;

	/** Pointer to a context menu object */
	var native const pointer	ContextMenu{class WxMBGenericBrowserContextBase};

	/** Pointer to the GenericBrowserType that should be called to handle events for this type. */
	var GenericBrowserType		BrowserType;

	/** Callback used to determine whether object is Supported*/
	var native pointer			IsSupportedCallback;

structcpptext
{
	typedef UBOOL (*GenericBrowserSupportCallback)(UObject* Object);

	FGenericBrowserTypeInfo(
		UClass* InClass,
		const FColor& InBorderColor,
		class WxMBGenericBrowserContextBase* InContextMenu,
		QWORD InRequiredFlags = 0,
		UGenericBrowserType* InBrowserType = NULL,
		GenericBrowserSupportCallback InIsSupportedCallback = NULL
	)
	:	Class(InClass)
	,	ContextMenu(InContextMenu)
	,	RequiredFlags(InRequiredFlags)
	,	BorderColor(InBorderColor)
	,	BrowserType(InBrowserType)
	,	IsSupportedCallback(InIsSupportedCallback)
	{}

	UBOOL Supports( UObject* Object ) const
	{
		UBOOL bResult = FALSE;
		if ( Object->IsA(Class) )
		{
			bResult = TRUE;
			if ( RequiredFlags != 0 )
			{
				bResult = Object->HasAllFlags(RequiredFlags);
			}
			if( bResult && IsSupportedCallback )
			{
				GenericBrowserSupportCallback Callback = (GenericBrowserSupportCallback) IsSupportedCallback;
				bResult = Callback( Object );
			}
		}
		return bResult;
	}

	inline UBOOL operator==( const FGenericBrowserTypeInfo& Other ) const
	{
		return ( Class == Other.Class && RequiredFlags == Other.RequiredFlags );
	}
}
};

// A list of information that this type supports.
var native array<GenericBrowserTypeInfo> SupportInfo;

// The color of the border drawn around this type in the browser.
var color BorderColor;

cpptext
{
	/**
	 * @return Returns the browser type description string.
	 */
	const FString& GetBrowserTypeDescription() const
	{
		return Description;
	}

	FColor GetBorderColor( UObject* InObject );

	/**
	 * Does any initial set up that the type requires.
	 */
	virtual void Init() {}

	/**
	 * Checks to see if the specified class is handled by this type.
	 *
	 * @param	InObject	The object we need to check if we support
	 */
	UBOOL Supports( UObject* InObject );

	/**
	 * Creates a context menu specific to the type of objects that are selected.
	 *
	 * @param	Selection	The selected object set.
	 */
	class WxMBGenericBrowserContextBase* GetContextMenu( USelection* Selection );

	/**
	 * Invokes the editor for an object.  The default behaviour is to
	 * open a property window for the object.  Dervied classes can override
	 * this with eg an editor which is specialized for the object's class.
	 *
	 * @param	InObject	The object to invoke the editor for.
	 */
	virtual UBOOL ShowObjectEditor( UObject* InObject )
	{
		return ShowObjectProperties( InObject );
	}

	/**
	 * Opens a property window for the specified object.  By default, GEditor's
	 * notify hook is used on the property window.  Derived classes can override
	 * this method in order to eg provide their own notify hook.
	 *
	 * @param	InObject	The object to invoke the property window for.
	 */
	virtual UBOOL ShowObjectProperties( UObject* InObject );

	/**
	 * Opens a property window for the specified objects.  By default, GEditor's
	 * notify hook is used on the property window.  Derived classes can override
	 * this method in order to eg provide their own notify hook.
	 *
	 * @param	InObjects	The objects to invoke the property window for.
	 */
	virtual UBOOL ShowObjectProperties( const TArray<UObject*>& InObjects );

	/**
	 * Invokes the editor for all selected objects.
	 */
	virtual UBOOL ShowObjectEditor();

	/**
	 * Displays the object properties window for all selected objects that this
	 * GenericBrowserType supports.
	 */
	UBOOL ShowObjectProperties();

	/**
	 * Invokes a custom menu item command for every selected object
	 * of a supported class.
	 *
	 * @param InCommand		The command to execute
	 */

	virtual void InvokeCustomCommand( INT InCommand );

	/**
	 * Invokes a custom menu item command.
	 *
	 * @param InCommand		The command to execute
	 * @param InObject		The object to invoke the command against
	 */

	virtual void InvokeCustomCommand( INT InCommand, UObject* InObject ) {}

	/**
	 * Calls the virtual "DoubleClick" function for each object
	 * of a supported class.
	 */

	virtual void DoubleClick();

	/**
	 * Allows each type to handle double clicking as they see fit.
	 */

	virtual void DoubleClick( UObject* InObject );

	/**
	 * Retrieves a list of objects supported by this browser type which
	 * are currently selected in the generic browser.
	 */
	void GetSelectedObjects( TArray<UObject*>& Objects );

	/**
	 * Determines whether the specified package is allowed to be saved.
	 */
	virtual UBOOL IsSavePackageAllowed( UPackage* PackageToSave );

protected:
	/**
	 * Determines whether the specified package is allowed to be saved.
	 *
	 * @param	PackageToSave		the package that is about to be saved
	 * @param	StandaloneObjects	a list of objects from PackageToSave which were marked RF_Standalone
	 */
	virtual UBOOL IsSavePackageAllowed( UPackage* PackageToSave, TArray<UObject*>& StandaloneObjects ) { return TRUE; }

public:
	/**
	 * Called when the user chooses to delete objects from the generic browser.  Gives the resource type the opportunity
	 * to perform any special logic prior to the delete.
	 *
	 * @param	ObjectToDelete	the object about to be deleted.
	 *
	 * @return	TRUE to allow the object to be deleted, FALSE to prevent the object from being deleted.
	 */
	virtual UBOOL NotifyPreDeleteObject( UObject* ObjectToDelete ) { return TRUE; }

	/**
	 * Called when the user chooses to delete objects from the generic browser, after the object has been checked for referencers.
	 * Gives the resource type the opportunity to perform any special logic after the delete.
	 *
	 * @param	ObjectToDelete		the object that was deleted.
	 * @param	bDeleteSuccessful	TRUE if the object wasn't referenced and was successfully marked for deletion.
	 */
	virtual void NotifyPostDeleteObject( UObject* ObjectToDelete, UBOOL bDeleteSuccessful ) {}
}

defaultproperties
{
}
