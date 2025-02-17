/*=============================================================================
	UnUIExporter.cpp: UI exporter and factory class implementations.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes
#include "UnrealEd.h"
#include "EngineUIPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

/* ==========================================================================================================
	UUISceneFactory
========================================================================================================== */
void UUISceneFactory::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
	new(GetClass(),TEXT("UISceneClass"),	RF_Public) UClassProperty (CPP_PROPERTY(NewSceneClass), TEXT("UI"), CPF_Edit, UUIScene::StaticClass());
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UUISceneFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UUIScene::StaticClass();
	Description			= TEXT("UIScene");

	NewSceneClass = UUIScene::StaticClass();
}


/**
 * Provides the factory with a reference to the global scene manager.
 */
void UUISceneFactory::SetSceneManager( class UUISceneManager* InSceneManager )
{
	UISceneManager = InSceneManager;
}

/* ==========================================================================================================
	UUISceneFactoryNew
========================================================================================================== */

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UUISceneFactoryNew::InitializeIntrinsicPropertyValues()
{
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
}

/**
 * Creates a new UIScene instance
 * 
 * @param	Class		the class to use for creating the scene
 * @param	InOuter		the object to create the new object within
 * @param	InName		the name to give the new object
 * @param	Flags		object flags
 * @param	Context		@todo
 * @param	Warn		@todo
 *
 * @return	a pointer to an instance of a UIScene object
 */

UObject* UUISceneFactoryNew::FactoryCreateNew( UClass* Class, UObject* InOuter, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(UISceneManager);

	UUIScene* SceneTemplate = NewSceneClass->GetDefaultObject<UUIScene>();
	UUIScene* Scene = UISceneManager->CreateScene(SceneTemplate, InOuter, InName);
	if ( Scene != NULL )
	{
		Scene->SetFlags(Flags);
	}

	return Scene;
}

/**
 * This class is used for clearing references to widgets outside of the scene after pasting widgets into a UIScene.
 */
class FArchiveClearExternalSceneReferences : public FArchive
{
public:
	FArchiveClearExternalSceneReferences( class UUIScene* InOwnerScene, const TArray<class UUIObject*>& SerializeSet )
	: OwnerScene(InOwnerScene)
	{
		// optimize the serialization path to ignore properties which don't potentially contain object references
		ArIsObjectReferenceCollector = TRUE;

		// don't serialize the archetype property
		ArIgnoreArchetypeRef = TRUE;

		// we're serializing from the top-down, so don't serialize the Outer property
		ArIgnoreOuterRef = TRUE;

		for ( INT Index = 0; Index < SerializeSet.Num(); Index++ )
		{
			UUIObject* Widget = SerializeSet(Index);

			if ( !OriginalObjects.Contains(Widget) )
			{
				TArray<UObject*> Subobjects;
				FArchiveObjectReferenceCollector SubobjectCollector(&Subobjects, Widget, FALSE, TRUE, TRUE, FALSE);
				Widget->Serialize(SubobjectCollector);

				OriginalObjects.Add(Widget);
				for ( INT ObjIndex = 0; ObjIndex < Subobjects.Num(); ObjIndex++ )
				{
					OriginalObjects.Add(Subobjects(ObjIndex));
				}
			}

			if ( !SerializedObjects.Contains(Widget) )
			{
				SerializedObjects.Add(Widget);
				Widget->Serialize(*this);
			}
		}
	}

protected:

	/** 
	 * UObject serialize operator implementation
	 *
	 * @param Object	reference to Object reference
	 * @return reference to instance of this class
	 */
	FArchive& operator<<( UObject*& Object )
	{
		if ( Object != NULL && Object->IsIn(OwnerScene->GetOutermost()) )
		{
			if ( OriginalObjects.Contains(Object) )
			{
				if ( !SerializedObjects.Contains(Object) )
				{
					SerializedObjects.Add(Object);
					Object->Serialize(*this);
				}
			}
			// references to objects in other packages will automatically be cleared from UObjectProperty::ImportText, so
			// we only need to worry about references to objects contained by other scenes in the same package
			else if ( ExternalObjects.Contains(Object) ||
				(!Object->HasAnyFlags(RF_Public) && Object != OwnerScene && !Object->IsIn(OwnerScene)) )
			{
				ExternalObjects.Add(Object);
				Object = NULL;
			}
		}
		
		return *this;
	}

	/**
	 * The scene that contains the objects to clear references from.
	 */
	UUIScene* OwnerScene;

	/**
	 * Prevents serializing the same objects twice
	 */
	TSet<UObject*> SerializedObjects;

	/**
	 * The objects which were passed to this archive
	 */
	TSet<UObject*> OriginalObjects;

	/**
	 * Objects which we've established to be external to this scene and non-referenceable
	 */
	TSet<UObject*> ExternalObjects;
};


/* ==========================================================================================================
	UUISceneFactoryText
========================================================================================================== */
/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UUISceneFactoryText::InitializeIntrinsicPropertyValues()
{
	// this factory can only create UIScene and widgets that are being pasted to UIScenes
	SupportedClass = UUIScene::StaticClass();

	// we cannot create new UIScenes from scratch using this factory
	bCreateNew = FALSE;

	// we can create new UIScenes from text using this factory
	bText = TRUE;

	// this controls whether this factory appears in the Generic Browser's "Import" dialog
	//@todo hmmmm, should we set this to TRUE?
//	bEditorImport = TRUE;

	// indicate that this factory is capable of importing entire scenes using T3D format
	new(Formats) FString(TEXT("t3d;UI Scene"));
}

/**
 * Create a new UI Scene and initialize it from a text buffer.
 *
 * @param	Class			the class to use for creating the UIScene
 * @param	InParent		@todo
 * @param	Name			the name to use for the new UIScene
 * @param	Flags			the object flags to use for the new UIScene
 * @param	Context			unused
 * @param	Type			indicates the type of import.  supported values are: paste, t3d
 * @param	Buffer			the text buffer to use for initializing the new scene
 * @param	BufferEnd		marks the end of the text buffer
 * @param	Warn			output device to use for any warnings/errors encountered during import
 */
UObject* UUISceneFactoryText::FactoryCreateText
(
	UClass*				Class,
	UObject*			InParent, 
	FName				Name, 
	EObjectFlags		Flags, 
	UObject*			Context, 
	const TCHAR*		Type, 
	const TCHAR*&		Buffer, 
	const TCHAR*		BufferEnd, 
	FFeedbackContext*	Warn
)
{
	// clear any leading whitespace
	ParseNext(&Buffer);

	// make sure that this is a valid T3D block for importing UIScene/UIObjects
	if ( !GetBEGIN(&Buffer, TEXT("SCENE")) )
	{
		//@todo - what should we do here....
		return NULL;
	}

	// unselect all widgets
	//@todo

	// if the type isn't "paste", then we are importing text from a t3d file.
	// setting this causes all properties to be imported using the PPF_AttemptNonQualifiedSearch flag
	UBOOL bPreviousImportingT3d = GEditor->IsImportingT3D;
	GEditor->IsImportingT3D = appStricmp(Type, TEXT("paste")) != 0;
	GIsImportingT3D = GEditor->IsImportingT3D;

	// In order to ensure that all object references are resolved correctly, we import the object in two passes:
	// - first we create or look up any object references encountered
	// - next we import the property values for all objects we created
	TMap<UObject*,FString> ImportedObjectText;

	/**
	 * keep a mapping of new widgets that we create to their intended parents, so that after we import all property values,
	 * we can add the new widgets to their parent's Children array.
	 */
	TMultiMap<UUIScreenObject*,UUIObject*> ParentChildMap;

	// keep track of the parents in a separate array to make it easier to lookup the children from the multimap
	TArray<UUIScreenObject*> ParentList;

	// keeps track of nested subobjects - when a subobject definition is encountered while parsing property text,
	// the newly created object corresponding to that subobject is added to this stack.  When the end object line
	// is parsed, we pop the object off the stack so that any additional property value text is associated with the
	// correct object
	TArray<UObject*> ImportStack;

	// the CurrentImportObject is the object that should be associated with property value text that we encounter
	UObject* CurrentImportObject=InParent;
	ImportStack.Push(InParent);

	// first pass

	// this will contain the text from each line as it's parsed
	FString CurrentLine;

	while ( ParseLine(&Buffer,CurrentLine) )
	{
		const TCHAR* Str = *CurrentLine;
		if ( GetEND(&Str,TEXT("SCENE")) )
		{
			// make sure our begin/end lines matched up
			checkSlow(ImportStack.Num()==1);

			// we've reached the end of the text
			break;
		}
		else if ( GetEND(&Str,TEXT("WIDGET")) || GetEND(&Str,TEXT("SEQUENCE")) || GetEND(&Str,TEXT("OBJECT")) )
		{
			// we've reached the end of the current widget's property values
			// pop the widget from the import stack and reset the CurrentImportObject
			UObject* ImportedObject = ImportStack.Pop();
			CurrentImportObject = ImportStack.Last();

			//@todo - any special handling for certain object types
		}
		else if ( GetBEGIN(&Str,TEXT("WIDGET")) )
		{
			UUIScreenObject* WidgetParent = CastChecked<UUIScreenObject>(CurrentImportObject);

			// importing a widget
			UClass* WidgetClass=NULL;

			// parse the class from the object definition
			if ( ParseObject<UClass>(Str, TEXT("CLASS="), WidgetClass, ANY_PACKAGE) )
			{
				// parse the name for the widget
				FName WidgetName(NAME_None);
				Parse(Str, TEXT("NAME="), WidgetName);

				// if the parent already has a widget that has this name, rename the existing widget
				if ( WidgetName != NAME_None )
				{
					UUIObject* ExistingWidget = FindObject<UUIObject>(WidgetParent, *WidgetName.ToString());
					if ( ExistingWidget != NULL )
					{
						ExistingWidget->Modify();
						ExistingWidget->Rename(NULL, NULL, REN_ForceNoResetLoaders);
					}
				}

				// if an archetype was specified in the Begin Object block, use that as the template for the ConstructObject call.
				// @todo: Generalize this whole system into using Archetype= for all Object importing
				FString ArchetypeName;
				UObject* WidgetArchetype = NULL;
				if (Parse(Str, TEXT("Archetype="), ArchetypeName))
				{
					// if given a name, break it up along the ' so separate the class from the name
					TArray<FString> Refs;
					ArchetypeName.ParseIntoArray(&Refs, TEXT("'"), TRUE);

					// find the class
					UClass* ArchetypeClass = (UClass*)UObject::StaticFindObject(UClass::StaticClass(), ANY_PACKAGE, *Refs(0));
					if (ArchetypeClass)
					{
						// if we had the class, find the archetype
						// @todo: this _may_ need StaticLoadObject, but there is currently a bug in StaticLoadObject that it can't take a non-package pathname properly
						WidgetArchetype = UObject::StaticFindObject(ArchetypeClass, ANY_PACKAGE, *Refs(1));
					}
				}


				// now we're ready to create the widget
				UUIObject* NewWidget = WidgetParent->CreateWidget(WidgetParent, WidgetClass, WidgetArchetype, WidgetName);
				if ( NewWidget != NULL )
				{
					ParentList.AddUniqueItem(WidgetParent);
					ParentChildMap.Add(WidgetParent, NewWidget);

					ImportStack.Push(NewWidget);
					CurrentImportObject = NewWidget;
					ImportedObjectText.Set(NewWidget, TEXT(""));
				}
			}
		}
		else if ( GetBEGIN(&Str,TEXT("SEQUENCE")) )
		{
			// start importing the sequence for a widget

			// find the parent sequence for this sequence

			UClass* SeqObjClass=NULL;

			// parse the class from the object definition
			if ( ParseObject<UClass>(Str, TEXT("CLASS="), SeqObjClass, ANY_PACKAGE) )
			{
				if ( !SeqObjClass->IsChildOf(USequenceObject::StaticClass()) )
				{
					continue;
				}

				// parse the name for the sequence
				FName SeqObjName(NAME_None);
				Parse( Str, TEXT("NAME="), SeqObjName );

				UUIComp_Event* MainSequenceContainer = Cast<UUIComp_Event>(CurrentImportObject);

				// now we're ready to create the new sequence
				UUISequence* NewSequence = NULL;

				// if the parent already has a widget that has this name, rename the existing widget
				if ( SeqObjName != NAME_None )
				{
					// when Created was called on the owning widget, it created an event component which created the global sequence 
					// using the correct archetype.  So if the CurrentImportObject is the event component we will find an existing sequence,
					// but if CurrentImportObject is a UIState, there shouldn't be an existing sequence.
					UUISequence* ExistingSequence = NewSequence = FindObject<UUISequence>(CurrentImportObject, *SeqObjName.ToString());
					check(ExistingSequence == NULL || MainSequenceContainer != NULL);
				}

				// if an archetype was specified in the Begin Object block, use that as the template for the ConstructObject call.
				// @todo: Generalize this whole system into using Archetype= for all Object importing
				FString ArchetypeName;
				UObject* SequenceArchetype = NULL;
				if (Parse(Str, TEXT("Archetype="), ArchetypeName))
				{
					// if given a name, break it up along the ' so separate the class from the name
					TArray<FString> Refs;
					ArchetypeName.ParseIntoArray(&Refs, TEXT("'"), TRUE);

					// find the class
					UClass* ArchetypeClass = (UClass*)UObject::StaticFindObject(UClass::StaticClass(), ANY_PACKAGE, *Refs(0));
					if (ArchetypeClass)
					{
						// if we had the class, find the archetype
						// @todo: this _may_ need StaticLoadObject, but there is currently a bug in StaticLoadObject that it can't take a non-package pathname properly
						SequenceArchetype = UObject::StaticFindObject(ArchetypeClass, ANY_PACKAGE, *Refs(1));
					}
				}


				if ( MainSequenceContainer == NULL )
				{
					UUIState* StateSequenceContainer = Cast<UUIState>(CurrentImportObject);
					if ( StateSequenceContainer != NULL )
					{
						StateSequenceContainer->CreateStateSequence(SeqObjName);
						NewSequence = StateSequenceContainer->StateSequence;
					}
				}
				else
				{
					if ( NewSequence != NULL )
					{
						MainSequenceContainer->EventContainer = NewSequence;
					}
					else
					{
						NewSequence = MainSequenceContainer->EventContainer = MainSequenceContainer->CreateEventContainer(SeqObjName);
					}
				}

				check(NewSequence);

				NewSequence->SetFlags(Flags);
				ImportStack.Push(NewSequence);
				CurrentImportObject = NewSequence;
				ImportedObjectText.Set(NewSequence, TEXT(""));
			}
		}
		else if ( GetBEGIN(&Str,TEXT("OBJECT")) )
		{
			// importing some other object type - i.e. sequence objects, components, etc.
			UObject* ObjectOuter = CurrentImportObject;

			// parse the class from the object definition
			UClass* ObjectClass=NULL;
			if ( ParseObject<UClass>(Str, TEXT("CLASS="), ObjectClass, ANY_PACKAGE) )
			{
				// parse the name for the widget
				FName ObjectName(NAME_None);
				Parse(Str, TEXT("NAME="), ObjectName);

				// the object name for components will actually be the component's TemplateName and the actual object name should
				// be specified by ObjName=, so if we have that, use that instead
				FName InstancedObjectName(NAME_None);
				if ( Parse(Str,TEXT("ObjName="), InstancedObjectName) )
				{
					ObjectName = InstancedObjectName;
				}

				// if the parent already has a widget that has this name, rename the existing widget
				if ( ObjectName != NAME_None )
				{
					UObject* ExistingObject = FindObject<UObject>(ObjectOuter, *ObjectName.ToString());
					if ( ExistingObject != NULL )
					{
						ExistingObject->Modify();
						ExistingObject->Rename(NULL, NULL, REN_ForceNoResetLoaders);
					}
				}

				// if an archetype was specified in the Begin Object block, use that as the template for the ConstructObject call.
				// @todo: Generalize this whole system into using Archetype= for all Object importing
				FString ArchetypeName;
				UObject* ObjArchetype = NULL;
				if (Parse(Str, TEXT("Archetype="), ArchetypeName))
				{
					// if given a name, break it up along the ' so separate the class from the name
					TArray<FString> Refs;
					ArchetypeName.ParseIntoArray(&Refs, TEXT("'"), TRUE);

					// find the class
					UClass* ArchetypeClass = (UClass*)UObject::StaticFindObject(UClass::StaticClass(), ANY_PACKAGE, *Refs(0));
					if (ArchetypeClass)
					{
						// if we had the class, find the archetype
						// @todo: this _may_ need StaticLoadObject, but there is currently a bug in StaticLoadObject that it can't take a non-package pathname properly
						ObjArchetype = UObject::StaticFindObject(ArchetypeClass, ANY_PACKAGE, *Refs(1));
					}
				}

				// now we're ready to create the subobject
				UObject* NewObject = ConstructObject<UObject>( ObjectClass, ObjectOuter, ObjectName, Flags, ObjArchetype );
				if ( NewObject != NULL )
				{
					ImportStack.Push(NewObject);
					CurrentImportObject = NewObject;
					ImportedObjectText.Set(NewObject, TEXT(""));
				}
			}
		}
		else if ( CurrentImportObject != NULL )
		{
			// this line contains an actual property value
			FString* WidgetPropertyText = ImportedObjectText.Find(CurrentImportObject);
			if ( WidgetPropertyText == NULL )
			{
				WidgetPropertyText = &ImportedObjectText.Set(CurrentImportObject, TEXT(""));
			}

			(*WidgetPropertyText) += CurrentLine + LINE_TERMINATOR;
		}
	}


	// second pass - import all property values for the newly created objects
	for ( TMap<UObject*,FString>::TIterator It(ImportedObjectText); It; ++It )
	{
		UObject* ImportObject = It.Key();
		FString& ImportText = It.Value();

		ImportObjectProperties((BYTE*)ImportObject,*ImportText, ImportObject->GetClass(), ImportObject, ImportObject, GWarn, 0);
	}

	TArray<UUIObject*> SelectionSet;

	// for each of the widgets that has new children, add those widgets to the parent's Children array
	for ( INT ParentIndex = 0; ParentIndex < ParentList.Num(); ParentIndex++ )
	{
		UUIScreenObject* WidgetParent = ParentList(ParentIndex);
		if ( Cast<UUIObject>(WidgetParent) != NULL )
		{
			SelectionSet.AddUniqueItem( Cast<UUIObject>(WidgetParent) );
		}

		// lookup the list of new children associated with this parent widget
		TArray<UUIObject*> NewChildren;
		ParentChildMap.MultiFind(WidgetParent, NewChildren);

		WidgetParent->Modify(TRUE);

		// iterate the list backwards since MultiFind returns the array in the opposite order from which
		// they were added
		for ( INT ChildIndex = NewChildren.Num() - 1; ChildIndex >= 0; ChildIndex-- )
		{
			UUIObject* Child = NewChildren(ChildIndex);

			WidgetParent->InsertChild(Child);

			// select this widget as well
			SelectionSet.AddUniqueItem(Child);
		}
	}

	FArchiveClearExternalSceneReferences ClearExternalReferences(CastChecked<UUIScreenObject>(InParent)->GetScene(), SelectionSet);

	// change the scene's selected widgets to the newly created widgets
	UUISceneManager* SceneManager = GUnrealEd->GetBrowserManager()->UISceneManager;
	SceneManager->SetSelectedSceneWidgets(CastChecked<UUIScreenObject>(InParent)->GetScene(), SelectionSet);

	// Restore the previous value of the t3d import flag
	GEditor->IsImportingT3D = bPreviousImportingT3d;
	GIsImportingT3D = GEditor->IsImportingT3D;

	return InParent;
}

/* ==========================================================================================================
	UUISceneExporter
========================================================================================================== */
/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UUISceneExporter::InitializeIntrinsicPropertyValues()
{
	//we export UIScenes
	SupportedClass = UUIScene::StaticClass();

	// we export UIScene data as text
	bText = TRUE;

	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("COPY")) );
	new(FormatDescription)FString(TEXT("UI Scene Text"));
}

struct FUISequenceExportHelper
{
	USequence*					Sequence;
	TArray<USequenceObject*>	CurrentSequenceObjects;
	TArray<USequence*>			CurrentNestedSequences;

	FUISequenceExportHelper( UUISequence* InSequence )
	: Sequence(InSequence)
	{
		// store the existing values in local storage
		CurrentSequenceObjects = Sequence->SequenceObjects;
		CurrentNestedSequences = Sequence->NestedSequences;

		if ( Sequence->SequenceObjects.Num() > 0 )
		{
			TArray<USequenceObject*> Subsequences;		
			// remove all subsequences from the sequence's main SequenceObjects list
			if ( ContainsObjectOfClass<USequenceObject>(Sequence->SequenceObjects, UUISequence::StaticClass(), FALSE, &Subsequences) )
			{
				for ( INT SubseqIndex = 0; SubseqIndex < Subsequences.Num(); SubseqIndex++ )
				{
					Sequence->SequenceObjects.RemoveItem(Subsequences(SubseqIndex));
				}
			}
		}

		if ( Sequence->NestedSequences.Num() > 0 )
		{
			TArray<USequence*> Subsequences;
			// now remove all subsequences from the sequence's NestedSequences array
			if ( ContainsObjectOfClass(Sequence->NestedSequences, UUISequence::StaticClass(), FALSE, &Subsequences) )
			{
				for ( INT SubseqIndex = 0; SubseqIndex < CurrentNestedSequences.Num(); SubseqIndex++ )
				{
					Sequence->NestedSequences.RemoveItem(CurrentNestedSequences(SubseqIndex));
				}
			}
		}
	}

	~FUISequenceExportHelper()
	{
		// reset the sequence's object list and nested sequences to their original contents
		Sequence->SequenceObjects = CurrentSequenceObjects;
		Sequence->NestedSequences = CurrentNestedSequences;
	}
};

/**
 * Exports the property values for the specified UISequence.
 *
 * @param	SequenceToExport	the sequence to export property values for
 * @param	Ar					the archive to export to
 * @param	PortFlags			modifies the behavior of the property value exporting
 */
void UUISceneExporter::ExportUISequence( UUISequence* SequenceToExport, FOutputDevice& Ar, DWORD PortFlags )
{
	// SequenceToExport will sometimes be NULL for widgets in the editor
	if ( SequenceToExport != NULL )
	{
		SequenceToExport->SetFlags(RF_TagImp);

		UUISequence* SequenceArchetype = CastChecked<UUISequence>(SequenceToExport->GetArchetype());
		Ar.Logf( TEXT("%sBegin Sequence Class=%s Name=%s"), appSpc(TextIndent), *SequenceToExport->GetClass()->GetName(), *SequenceToExport->GetName());
		if ( !SequenceArchetype->HasAnyFlags(RF_ClassDefaultObject) )
		{
			Ar.Logf(TEXT(" Archetype=%s'%s'"), *SequenceArchetype->GetClass()->GetName(), *SequenceArchetype->GetPathName() );
		}
		Ar.Log(LINE_TERMINATOR);
		TextIndent += 3;

		// remove any sequences from this sequence's SequenceObjecs array which belong to other
		// widgets, so that those sequences don't get exported as a subobject of this widget's sequence
		FUISequenceExportHelper ExportHelper(SequenceToExport);

		// export any components contained by this sequence
		TArray<UComponent*> SequenceComponents;
		BuildComponentMap(SequenceToExport, SequenceComponents);
		ExportComponentDefinitions(NULL, SequenceComponents, Ar, PortFlags);

		ExportProperties( NULL, Ar, SequenceToExport->GetClass(), (BYTE*)SequenceToExport, TextIndent, SequenceArchetype->GetClass(), (BYTE*)SequenceArchetype, SequenceToExport, PortFlags );

		TextIndent -= 3;
		Ar.Logf( TEXT("%sEnd Sequence	// %s%s"), appSpc(TextIndent), *SequenceToExport->GetName(), LINE_TERMINATOR );
	}
}

/**
 * Exports the property values for the specified widget menu state along with the state's sequence.
 *
 * @param	StateToExport	the widget menu state to export property values for
 * @param	Ar				the archive to export to
 * @param	PortFlags		modifies the behavior of the property value exporting
 */
void UUISceneExporter::ExportWidgetStates( UUIState* StateToExport, FOutputDevice& Ar, DWORD PortFlags )
{
	StateToExport->SetFlags(RF_TagImp);

	Ar.Logf(TEXT("%sBegin Object Class=%s Name=%s%s"),
		appSpc(TextIndent),
		*StateToExport->GetClass()->GetName(),
		*StateToExport->GetName(),
		LINE_TERMINATOR
		);

	TextIndent += 3;
	ExportUISequence(StateToExport->StateSequence, Ar, PortFlags);

	// export any components contained by this state
	TArray<UComponent*> StateComponents;
	BuildComponentMap(StateToExport, StateComponents);
	ExportComponentDefinitions(NULL, StateComponents, Ar, PortFlags);

	// export the properties for the menu state
	ExportProperties(
		NULL,
		Ar,
		StateToExport->GetClass(),
		(BYTE*)StateToExport,
		TextIndent,
		StateToExport->GetArchetype()->GetClass(),
		(BYTE*)StateToExport->GetArchetype(),
		Cast<UUIObject>(StateToExport->GetOwner()),
		PortFlags
		);

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd Object	// %s%s"), appSpc(TextIndent), *StateToExport->GetName(), LINE_TERMINATOR);
}

/**
 * Exports the property values for the specified widget as well as all its child widgets.
 *
 * @param	WidgetToExport	the widget to export property values for
 * @param	Ar				the archive to export to
 * @param	PortFlags		modifies the behavior of the property value exporting
 */
void UUISceneExporter::ExportWidgetProperties( UUIObject* WidgetToExport, FOutputDevice& Ar, DWORD PortFlags )
{
	// prevent this widget from being exported again
	WidgetToExport->SetFlags(RF_TagImp);

	UUIObject* WidgetArchetype = Cast<UUIObject>(WidgetToExport->GetArchetype());

	// start the subobject definition for the widget
	Ar.Logf(TEXT("%sBegin Widget Class=%s Name=%s"),
		appSpc(TextIndent), 
		*WidgetToExport->GetClass()->GetName(),
		*WidgetToExport->GetName()
		);

	if ( !WidgetArchetype->HasAnyFlags(RF_ClassDefaultObject) )
	{
		Ar.Logf(TEXT(" Archetype=%s'%s'"),
			*WidgetArchetype->GetClass()->GetName(),
			*WidgetArchetype->GetPathName());
	}

	Ar.Log(LINE_TERMINATOR);
	TextIndent += 3;


	// first, export the sequence for this widget
	if ( WidgetToExport->EventProvider != NULL && WidgetToExport->EventProvider->EventContainer != NULL )
	{
		UUIComp_Event* EventProvider = WidgetToExport->EventProvider;
		UUIComp_Event* EventProviderArchetype = CastChecked<UUIComp_Event>(EventProvider->GetArchetype());
		EventProvider->SetFlags(RF_TagImp);

		// export the beginning of the subobject definition for the event provider component
		Ar.Logf(TEXT("%sBegin Object Class=%s Name=%s"), appSpc(TextIndent), *EventProvider->GetClass()->GetName(), *EventProvider->GetName() );
		if ( !EventProviderArchetype->HasAnyFlags(RF_ClassDefaultObject) )
		{
			Ar.Logf(TEXT(" Archetype=%s'%s'"), 	*EventProviderArchetype->GetClass()->GetName(), *EventProviderArchetype->GetPathName());
		}
		Ar.Log(LINE_TERMINATOR);

		TextIndent += 3;

		ExportUISequence(EventProvider->EventContainer, Ar, PortFlags);

		// export the properties for the event provider component
		ExportProperties(
			NULL,
			Ar,
			EventProvider->GetClass(),
			(BYTE*)EventProvider,
			TextIndent,
			EventProviderArchetype->GetClass(),
			(BYTE*)EventProviderArchetype,
			WidgetToExport,
			PortFlags
			);

		TextIndent -= 3;
		Ar.Logf(TEXT("%sEnd Object	// %s%s"), appSpc(TextIndent), *EventProvider->GetName(), LINE_TERMINATOR);
	}

	// first, export the subobjects and components
	TArray<UComponent*> WidgetComponents;
	BuildComponentMap(WidgetToExport, WidgetComponents);
	ExportComponentDefinitions(NULL, WidgetComponents, Ar, PortFlags);

	for ( INT StateIndex = 0; StateIndex < WidgetToExport->InactiveStates.Num(); StateIndex++ )
	{
		ExportWidgetStates(WidgetToExport->InactiveStates(StateIndex), Ar, PortFlags);
	}

	// next, export the widget children
	TArray<UUIObject*> ChildWidgets = WidgetToExport->GetChildren();
	for ( INT ChildIndex = 0; ChildIndex < ChildWidgets.Num(); ChildIndex++ )
	{
		ExportWidgetProperties(ChildWidgets(ChildIndex), Ar, PortFlags);
	}

	// now export the property values for this widget
	ExportProperties(
		NULL,
		Ar,								// output device
		WidgetToExport->GetClass(),		// object class
		(BYTE*)WidgetToExport,			// data
		TextIndent,						// indent
		WidgetArchetype->GetClass(),	// default class
		(BYTE*)WidgetArchetype,			// default data
		WidgetToExport,					// subobject root
		PortFlags						// export modifiers
		);

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd Widget	// %s%s"), appSpc(TextIndent), *WidgetToExport->GetName(), LINE_TERMINATOR);
}

UBOOL UUISceneExporter::ExportText
(
	const FExportObjectInnerContext*	Context,
	UObject*							Object,
	const TCHAR*						Type,
	FOutputDevice&						Ar,
	FFeedbackContext*					Warn,
	DWORD								PortFlags/*=0*/ 
)
{
	// The owning scene should always be passed in as the RootObject
	UUIScene* Scene = CastChecked<UUIScene>( Object );

	// Iterate over all objects making sure they import/export flags are unset. 
	// These are used for ensuring we export each object only once etc.
	for( FObjectIterator It; It; ++It )
	{
		It->ClearFlags( RF_TagImp | RF_TagExp );
	}

	Ar.Logf(TEXT("%sBegin Scene%s"), appSpc(TextIndent), LINE_TERMINATOR);
	TextIndent += 3;

	// if we're exporting to the copy/paste buffer, only export the selected widgets
	bSelectedOnly = bSelectedOnly || appStricmp(Type,TEXT("COPY")) == 0;

	TArray<UUIObject*> WidgetsToExport;

	UBOOL bExportWidgets = FALSE;

	// If only the selected widgets should be exported, we'll need to figure out what the selected widgets are
	if( bSelectedOnly )
	{
		UUISceneManager* SceneManager = GUnrealEd->GetBrowserManager()->UISceneManager;
		bExportWidgets = SceneManager->GetSelectedSceneWidgets(Scene, WidgetsToExport);
	}
	else
	{
		bExportWidgets = TRUE;
		WidgetsToExport = Scene->GetChildren();
	}

	if ( bExportWidgets )
	{
		for ( INT WidgetIndex = 0; WidgetIndex < WidgetsToExport.Num(); WidgetIndex++ )
		{
			UUIObject* Widget = WidgetsToExport(WidgetIndex);
			ExportWidgetProperties( Widget, Ar, PortFlags );
		}
	}
	
	if ( !bSelectedOnly )
	{
		// If exporting everything - export the properties for the scene as well

		// first export the components
		TArray<UComponent*> Components;
		BuildComponentMap(Scene, Components);
		ExportComponentDefinitions(Context, Components, Ar, PortFlags);

		ExportProperties( Context, Ar, Scene->GetClass(), (BYTE*)Scene, TextIndent, Scene->GetArchetype()->GetClass(), (BYTE*)Scene->GetArchetype(), Scene, PortFlags );
	}

	TextIndent -= 3;
	Ar.Logf(TEXT("%sEnd Scene%s"), appSpc(TextIndent), LINE_TERMINATOR);

	return TRUE;
}

/**
 * Builds a map containing the components owned directly by ComponentOwner.
 *
 * @param	ComponentOwner		the object to search for components in
 * @param	out_ComponentMap	will be filled in with the components that are contained by ComponentOwner
 */
void UUISceneExporter::BuildComponentMap( UObject* ComponentOwner, TArray<UComponent*>& out_Components )
{
	check(ComponentOwner);
	if ( ComponentOwner->GetClass()->HasAnyClassFlags(CLASS_HasComponents) )
	{
		TArray<UComponent*> ComponentReferences;
		TArchiveObjectReferenceCollector<UComponent> ComponentCollector(
			&ComponentReferences,		//	ObjectArray
			ComponentOwner,				//	LimitOuter
			TRUE,						//	bRequireDirectOuter
			TRUE,						//	bShouldIgnoreArchetypes
			TRUE,						//	bSerializeRecursively
			TRUE);						//	bShouldIgnoreTransients

		ComponentOwner->Serialize(ComponentCollector);

		out_Components.Empty();
		for ( INT ComponentIndex = 0; ComponentIndex < ComponentReferences.Num(); ComponentIndex++ )
		{
			UComponent* Component = ComponentReferences(ComponentIndex);
			out_Components.AddItem(Component);
		}
	}
}

