/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __LOCALIZATIONEXPORT_H__
#define __LOCALIZATIONEXPORT_H__

/**
 * A set of static methods for exporting localized properties.
 */
class FLocalizationExport
{
public:
	/**
	 * @param	Package						The package to export.
	 * @param	IntName						Name of the .int name.
	 * @param	bCompareAgainstDefaults		If TRUE, don't export the property unless it differs from defaults.
	 * @param	bDumpEmptyProperties		If TRUE, export properties with zero length/empty strings.
	 */
	static void ExportPackage(UPackage* Package, const TCHAR* IntName, UBOOL bCompareAgainstDefaults, UBOOL bDumpEmptyProperties);

	/**
	 * @param	Class						Class of the object being exported.
	 * @param	SuperClass					Superclass of the object being exported.
	 * @param	OuterClass
	 * @param	Struct						Struct property to export.
	 * @param	IntName						Name of the .int file.
	 * @param	SectionName					Config section to export to.
	 * @param	KeyPrefix					Config key (property name) to export to.
	 * @param	DataBase					Base data address for the data.
	 * @param	DataOffset					Offset from the base address.
	 * @param	bAtRoot
	 * @param	bCompareAgainstDefaults		If TRUE, don't export the property unless it differs from defaults.
	 * @param	bDumpEmptyProperties		If TRUE, export properties with zero length/empty strings.
	 */
	static void ExportStruct(UClass*			Class,
							 UClass*			SuperClass,
							 UClass*			OuterClass,
							 UStruct*		Struct,
							 const TCHAR*	IntName,
							 const TCHAR*	SectionName,
							 const TCHAR*	KeyPrefix,
							 BYTE*			DataBase,
							 INT				DataOffset,
							 UBOOL			bAtRoot,
							 UBOOL			bCompareAgainstDefaults,
							 UBOOL			bDumpEmptyProperties);

	/**
	 * @param	Class						Class of the object being exported.
	 * @param	SuperClass					Superclass of the object being exported.
	 * @param	OuterClass
	 * @param	Prop						Property to export.
	 * @param	IntName						Name of the .int file.
	 * @param	SectionName					Config section to export to.
	 * @param	KeyPrefix					Config key (property name) to export to.
	 * @param	DataBase					Base data address for the data.
	 * @param	DataOffset					Offset from the base address.
	 * @param	bCompareAgainstDefaults		If TRUE, don't export the property unless it differs from defaults.
	 * @param	bDumpEmptyProperties		If TRUE, export properties with zero length/empty strings.
	 */
	static void ExportProp(UClass*		Class, 
						   UClass*		SuperClass, 
						   UClass*		OuterClass, 
						   UProperty*	Prop, 
						   const TCHAR*	IntName, 
						   const TCHAR*	SectionName, 
						   const TCHAR*	KeyPrefix, 
						   BYTE*			DataBase, 
						   INT			DataOffset,
						   UBOOL			bCompareAgainstDefaults,
						   UBOOL			bDumpEmptyProperties);

	/**
	 * @param	Class						Class of the object being exported.
	 * @param	SuperClass					Superclass of the object being exported.
	 * @param	OuterClass
	 * @param	Prop						Array property to export.
	 * @param	IntName						Name of the .int file.
	 * @param	SectionName					Config section to export to.
	 * @param	KeyPrefix					Config key (property name) to export to.
	 * @param	DataBase					Base data address for the data.
	 * @param	DataOffset					Offset from the base address.
	 * @param	bAtRoot
	 * @param	bCompareAgainstDefaults		If TRUE, don't export the property unless it differs from defaults.
	 */
	static void ExportDynamicArray(UClass*			Class,
								   UClass*			SuperClass, 
								   UClass*			OuterClass, 
								   UArrayProperty*	Prop, 
								   const TCHAR*		IntName, 
								   const TCHAR*		SectionName, 
								   const TCHAR*		KeyPrefix, 
								   BYTE*				DataBase, 
								   INT				DataOffset, 
								   UBOOL				bAtRoot,
								   UBOOL				bCompareAgainstDefaults);

	/**
	 * Generates an .int name from the package name.
	 */
	static void GenerateIntNameFromPackageName(const FString &PackageName, FString &OutIntName);
};

#endif // __LOCALIZATIONEXPORT_H__
