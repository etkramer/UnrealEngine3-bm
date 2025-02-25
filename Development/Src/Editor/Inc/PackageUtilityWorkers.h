/*=============================================================================
	PackageUtilityWorkers.cpp: Declarations for structs and classes used by package commandlets.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __PACKAGEUTILITYWORKERS_H__
#define __PACKAGEUTILITYWORKERS_H__

/**
 * These bit flag values represent the different types of information that can be reported about a package
 */
enum EPackageInfoFlags
{
	PKGINFO_None		=0x00,
	PKGINFO_Names		=0x01,
	PKGINFO_Imports		=0x02,
	PKGINFO_Exports		=0x04,
	PKGINFO_Compact		=0x08,
	PKGINFO_Chunks		=0x10,
	PKGINFO_Depends		=0x20,
	PKGINFO_Paths		=0x40,

	PKGINFO_All			= PKGINFO_Names|PKGINFO_Imports|PKGINFO_Exports|PKGINFO_Chunks|PKGINFO_Depends|PKGINFO_Paths,
};

/**
 * Base for classes which generate output for the PkgInfo commandlet
 */
struct FPkgInfoReporter
{
	/** Constructors */
	FPkgInfoReporter() 
	: InfoFlags(PKGINFO_None), bHideOffsets(FALSE), Linker(NULL), PackageCount(0)
	{}
	FPkgInfoReporter( DWORD InInfoFlags, UBOOL bInHideOffsets, ULinkerLoad* InLinker=NULL )
	: InfoFlags(InInfoFlags), bHideOffsets(bInHideOffsets), Linker(InLinker), PackageCount(0)
	{}
	FPkgInfoReporter( const FPkgInfoReporter& Other )
	: InfoFlags(Other.InfoFlags), bHideOffsets(Other.bHideOffsets), Linker(Other.Linker), PackageCount(Other.PackageCount)
	{}

	/** Destructor */
	virtual ~FPkgInfoReporter() {}

	/**
	 * Performs the actual work - generates a report containing information about the linker.
	 *
	 * @param	InLinker	if specified, changes this reporter's Linker before generating the report.
	 */
	virtual void GeneratePackageReport( class ULinkerLoad* InLinker=NULL )=0;

	/**
	 * Changes the target linker for this reporter.  Useful when generating reports for multiple packages.
	 */
	void SetLinker( class ULinkerLoad* NewLinker )
	{
		Linker = NewLinker;
	}

protected:
	/**
	 * A bitmask of PKGINFO_ flags that determine the information that is included in the report.
	 */
	DWORD InfoFlags;

	/**
	 * Determines whether FObjectExport::SerialOffset will be included in the output; useful when generating
	 * a report for comparison against another version of the same package.
	 */
	UBOOL bHideOffsets;

	/**
	 * The linker of the package to generate the report for
	 */
	class ULinkerLoad* Linker;

	/**
	 * The number of packages evaluated by this reporter so far.  Must be incremented by child classes.
	 */
	INT PackageCount;
};

struct FPkgInfoReporter_Log : public FPkgInfoReporter
{
	FPkgInfoReporter_Log( DWORD InInfoFlags, UBOOL bInHideOffsets, ULinkerLoad* InLinker=NULL )
	: FPkgInfoReporter(InInfoFlags, bInHideOffsets, InLinker)
	{}
	FPkgInfoReporter_Log( const FPkgInfoReporter_Log& Other )
	: FPkgInfoReporter( Other )
	{}
	/**
	 * Writes information about the linker to the log.
	 *
	 * @param	InLinker	if specified, changes this reporter's Linker before generating the report.
	 */
	virtual void GeneratePackageReport( class ULinkerLoad* InLinker=NULL );
};

#endif


//EOF








