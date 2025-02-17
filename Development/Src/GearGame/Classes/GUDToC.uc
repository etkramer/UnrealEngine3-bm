/**
 * Class: GUDToC
 * GUD table of contents.
 * Used by the GUDManager to handle split banks for characters.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GUDToC extends Object
	dependson(GUDTypes)
	native(Sound);


struct native GUDToCEntry
{
	/** */
	var transient init native string	BankName;
	/** Approximate bank memory size, in bytes. */
	var transient init int				ApproxBankSize;

	structcpptext
	{
		UBOOL operator==(const FGUDToCEntry& Other) const
		{
			return BankName == Other.BankName;
		}

		virtual void Serialize(FArchive& Ar)
		{
			Ar << BankName;
			Ar << ApproxBankSize;
		}

		friend FArchive& operator<<(FArchive& Ar, FGUDToCEntry& Entry)
		{
			return Ar << Entry.BankName << Entry.ApproxBankSize;
		}
	}
};

/** 
 *	Struct that holds the information on the break-up/cooked GUDBank.
 */
struct native GUDCollection
{
	/** The root GUD bank. */
	var transient init native GUDToCEntry			RootGUDBank;
	/** The 'variety' GUD banks. */
	var transient init native array<GUDToCEntry>	GUDBanks;
	
	structcpptext
	{
		virtual void Serialize(FArchive& Ar)
		{
			Ar << RootGUDBank;
			Ar << GUDBanks;
		}

		friend FArchive& operator<<(FArchive& Ar, FGUDCollection& Collection)
		{
			return Ar << Collection.RootGUDBank << Collection.GUDBanks;
		}
	}
};

/**
 *	Maps the GUDBank name to the list of bank packages that it was broken up into.
 */ 
var transient native Map_Mirror		TableOfContents{TMap<FString,FGUDCollection>};

cpptext
{
public:
	static FString ToCPackageName;
	static FName ToCObjectName;

	virtual void AddReferencedObjects( TArray<UObject*>& ObjectArray );
	virtual void Serialize(FArchive& Ar);
}

defaultproperties
{
}
