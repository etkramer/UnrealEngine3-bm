/*=============================================================================
	UnrealEdFactories.h: UnrealEd factory and exporter class declarations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNREALEDFACTORIES_H__
#define __UNREALEDFACTORIES_H__

/**
 * Base class for factories which create UIScenes.
 */
class UUISceneFactory : public UFactory
{
	DECLARE_ABSTRACT_CLASS(UUISceneFactory,UFactory,CLASS_Intrinsic,UnrealEd)

	void StaticConstructor();

	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();

	/**
	 * Provides the factory with a reference to the global scene manager.
	 */
	void SetSceneManager( class UUISceneManager* InSceneManager );

	UClass* NewSceneClass;

protected:
	/**
	 * the manager responsible for tracking all UIScenes that are being edited.
	 * Set by UUISceneManager::Initialize()
	 */
	class UUISceneManager*	UISceneManager;
};

/**
 * This class is used to allow the user to create new UIScenes from within the Generic Browser.
 */
class UUISceneFactoryNew : public UUISceneFactory
{
	DECLARE_CLASS(UUISceneFactoryNew,UUISceneFactory,0,UnrealEd)

	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();

	/**
	 * Creates a new UIScene instance
	 * 
	 * @param	Class		the class to use for creating the scene
	 * @param	InOuter		the object to create the new object within
	 * @param	InName		the name to give the new object
	 * @param	Flags		object flags
	 *
	 * @return	a pointer to an instance of a UIScene object
	 */
	UObject* FactoryCreateNew( class UClass* Class, class UObject* InOuter, class FName InName, EObjectFlags Flags, class UObject* Context, class FFeedbackContext* Warn);
};

/**
 * This class is used to allow the user to create new UIScenes or copy/paste UIObjects by importing property values from a text file.
 */
class UUISceneFactoryText : public UUISceneFactory
{
	DECLARE_CLASS(UUISceneFactoryText,UUISceneFactory,0,UnrealEd);

	/**
	* Initializes property values for intrinsic classes.  It is called immediately after the class default object
	* is initialized against its archetype, but before any objects of this class are created.
	*/
	void InitializeIntrinsicPropertyValues();
	UObject* FactoryCreateText( UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn );
};

/**
 * This class handling exporting UIScene and UIObject data to text format, for storing in T3D format or supporting copy/paste.
 */
class UUISceneExporter : public UExporter
{
	DECLARE_CLASS(UUISceneExporter,UExporter,0,UnrealEd)

	/**
	* Initializes property values for intrinsic classes.  It is called immediately after the class default object
	* is initialized against its archetype, but before any objects of this class are created.
	*/
	void InitializeIntrinsicPropertyValues();

	/**
	 * Exports the property values for the specified widget as well as all its child widgets.
	 *
	 * @param	WidgetToExport	the widget to export property values for
	 * @param	Ar				the archive to export to
	 * @param	PortFlags		modifies the behavior of the property value exporting
	 */
	void ExportWidgetProperties( class UUIObject* WidgetToExport, class FOutputDevice& Ar, DWORD PortFlags );

	/**
	 * Exports the property values for the specified UISequence.
	 *
	 * @param	SequenceToExport	the sequence to export property values for
	 * @param	Ar					the archive to export to
	 * @param	PortFlags			modifies the behavior of the property value exporting
	 */
	void ExportUISequence( class UUISequence* SequenceToExport, class FOutputDevice& Ar, DWORD PortFlags );

	/**
	 * Exports the property values for the specified widget menu state along with the state's sequence.
	 *
	 * @param	StateToExport	the widget menu state to export property values for
	 * @param	Ar				the archive to export to
	 * @param	PortFlags		modifies the behavior of the property value exporting
	 */
	void ExportWidgetStates( class UUIState* StateToExport, class FOutputDevice& Ar, DWORD PortFlags );

	UBOOL ExportText( const FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, DWORD PortFlags=0 );

	/**
	 * Builds a map containing the components owned directly by ComponentOwner.
	 *
	 * @param	ComponentOwner		the object to search for components in
	 * @param	out_ComponentMap	will be filled in with the components that are contained by ComponentOwner
	 */
	void BuildComponentMap( UObject* ComponentOwner, TArray<UComponent*>& out_Components );
};

#endif	// __UNREALEDFACTORIES_H__
