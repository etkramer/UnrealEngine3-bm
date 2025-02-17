class ContentTagIndex extends Object
	native
	config(Editor);

/** Which engine version this index was built from */
var const int VersionInfo;

/** String references to assets, only needed for lack of TMap in script and lack of array<array<>> in script */
struct native AssetReferences
{
	var const array<string> AssetReference;
};

struct native TagInfo
{
	/** Asset type for this group of tags */
	var const config string AssetTypeName;
	var class<Object> AssetType;
	/** List of tags used by this asset type */
	var const config array<Name> Tags;
	/** List of content references with matching index to the entry in Tags array */
	var const array<AssetReferences> Assets;
};

/** Default list of tags read from config file for editor */
var const config array<TagInfo> DefaultTags;

/** Array of tags contained in this index */
var const array<TagInfo> Tags;

cpptext
{
	typedef TMap<UClass*,TMap<FString,TArray<FName> > > AssetMapType;

	/** Searches through DefaultsTags to find the matching entry based on AssetType */
	FTagInfo* FindDefaultTagInfoForClass(UClass *ClassToFind);

	/** Searches through Tags to find the matching entry based on AssetType, creating a new entry if none was found. */
	FTagInfo& GetTagInfo(UClass *ClassToFind);

	/** Adds a new content reference to the matching tags inside of the supplied TagInfo. */
	void AddContentReference(FTagInfo &TagInfo, TArray<FName> &AssetTags, FString AssetReference);

	/** Builds a map of asset types to assets to tags for parsing/merging/etc. */
	void BuildAssetTypeToAssetMap(AssetMapType &OutAssetTypeToAssetMap);

	/** Loads the reference content tag index and merges the results into this index */
	void MergeFromRefContentTagIndex();

	/** 
	 *  Fills in the content tags for the specified object.  This is used to get around having no
	 *  common parent (other than UObject) for the various asset types.
	 */
	static void ApplyTagsToObjects(TArray<UObject*> &Objects, UClass* ObjectsClass, TArray<FName> &TagsToApply);

	/** Gets the cumulative set of tags from the supplied objects. */
	static void GetTagsFromObjects(TArray<UObject*> &Objects, UClass* ObjectsClass, TArray<FName> &OutTags);

	/** Gets/creates the local content tag index. */
	static UContentTagIndex* GetLocalContentTagIndex();

	/** Saves the local index */
	static void SaveLocalContentTagIndex();
};