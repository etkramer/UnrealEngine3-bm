/*=============================================================================
	MeshSimplificationWindow.h: Static mesh editor's Mesh Simplification tool
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


#ifndef __MeshSimplificationWindow_h__
#define __MeshSimplificationWindow_h__


class WxStaticMeshEditor;


/**
 * WxMeshSimplificationWindow
 *
 * The Mesh Simplification tool allows users to create and maintain reduced-polycount versions of mesh
 * assets on a per-level basis.
 *
 * If you change functionality here, be sure to update the UDN help page:
 *      https://udn.epicgames.com/Three/MeshSimplificationTool
 */
class WxMeshSimplificationWindow
	: public wxPanel,
	  public FSerializableObject,
	  public FCallbackEventDevice
{

public:

	/** Constructor that takes a pointer to the owner object */
	WxMeshSimplificationWindow( WxStaticMeshEditor* InStaticMeshEditor );

	/** Destructor */
	virtual ~WxMeshSimplificationWindow();

	/** Invalidates the cached high res source static mesh.  Called when switching viewed meshes in the editor. */
	void InvalidateHighResSourceStaticMesh();

	/** Serialize this object to track references to UObject dependencies */
	virtual void Serialize( FArchive& Ar );

	/**
	 *  Checks for a copy of the editor's mesh that's already simplified and stored in the level's package.  If
	 *  one is found, we prompt the user to switch to viewing (resimplifying) the mesh we found instead.
	 */
	void CheckForAlreadySimplifiedDuplicate();


protected:

	/** Updates the controls in this panel */
	void UpdateControls();


	/**
	 * Prompts the user for a package/name for a new mesh, then creates a copy of the specified mesh.
	 * If the target mesh already exists and appears to be compatible, we'll use that instead.
	 *
	 * @param InStaticMesh The mesh to duplicate
	 *
	 * @return The mesh that we found or created, or NULL if a problem was encountered.  Warnings reported internally.
	 */
	static UStaticMesh* FindOrCreateCopyOfStaticMesh( UStaticMesh* InStaticMesh );

	/**
	 * Caches the high resolution source static mesh associated with the mesh in the editor (if there is one)
	 *
	 * @param bShouldWarnUser TRUE if we should warn the user about any problems encountered.
	 * 
	 * @return Returns TRUE If everything went OK.
	 */
	UBOOL CacheHighResSourceStaticMesh( BOOL bShouldWarnUser );

	/**
	 * Finds all actors that reference the specified static mesh
	 *
	 * @param InStaticMeshToSearchFor The static mesh we're looking for
	 * @param OutActorsReferencingMesh (Out) List of actors that reference the specified mesh
	 */
	static void FindAllInstancesOfMesh( const UStaticMesh* InStaticMeshToSearchFor,
									    TArray< AActor* >& OutActorsReferencingMesh );

	/**
	 * Replaces all actor references to the specified mesh with a new mesh
	 *
	 * @param InStaticMeshToSearchFor The mesh we're looking for
	 * @param InNewStaticMesh The mesh we should replace with.  If NULL, we'll only search for meshes.
	 * @param OutActorsUpdated (Out) Actors that were updated successfully
	 * @param OutActorsNotUpdated (Out) Actors that we failed to update
	 * @param FeedbackContext Optional context for reporting progress (can be NULL)
	 */
	static void ReplaceStaticMeshReferences( const UStaticMesh* InStaticMeshToSearchFor,
											 UStaticMesh* InNewStaticMesh,
										     TArray< AActor* >& OutActorsUpdated,
										     TArray< AActor* >& OutActorsNotUpdated,
											 FFeedbackContext* FeedbackContext );


	/**
	 * Updates all actors to use either the simplified or high res mesh
	 *
	 * @param bReplaceWithSimplified TRUE to update actors to use the simplified mesh, or FALSE to use the high res mesh
	 */
	void UpdateActorReferences( UBOOL bReplaceWithSimplified );

	/**
	 * Asks the user if they really want to unbind from the high res mesh, then does so if the user chooses to.
	 */
	void PromptToUnbindFromHighResMesh();

	/** Called when the triangle count slider is interacted with */
	void OnTargetTriangleCountSlider( wxScrollEvent& In );

	/** Called when 'High Res Actor Details' button is pressed */
	void OnHighResActorDetailsButton( wxCommandEvent& In );

	/** Called when 'Simplified Actor Details' button is pressed */
	void OnSimplifiedActorDetailsButton( wxCommandEvent& In );

	/** Called when 'Create or Update Simplified Mesh' button is pressed */
	void OnCreateOrUpdateSimplifiedMeshButton( wxCommandEvent& In );

	/** Called when 'Apply Simplified to Current Level' button is pressed */
	void OnApplySimplifiedToCurrentLevelButton( wxCommandEvent& In );

	/** Called when 'Apply High Res to Current Level' button is pressed */
	void OnApplyHighResToCurrentLevelButton( wxCommandEvent& In );

	/** Called when 'Unbind From High Res Mesh' button is pressed */
	void OnUnbindFromHighResMeshButton( wxCommandEvent& In );

	/**
	 * FCallbackEventDevice interface
	 */
	virtual void Send( ECallbackEventType Event );
	virtual void Send( ECallbackEventType Event, DWORD Param );


protected:

	/** Editor that owns us */
	WxStaticMeshEditor* StaticMeshEditor;

	/** Target triangle count for simplified mesh */
	wxSlider* TargetTriangleCountSlider;

	/** High res actor details button */
	wxButton* HighResActorDetailsButton;

	/** Simplified actor details button */
	wxButton* SimplifiedActorDetailsButton;

	/** Creates or updates an existing simplified mesh */
	wxButton* CreateOrUpdateSimplifiedMeshButton;

	/** Applies the simplified mesh to all instances in the current level */
	wxButton* ApplySimplifiedToCurrentLevelButton;

	/** Applies the high res mesh to all instances in the current level */
	wxButton* ApplyHighResToCurrentLevelButton;

	/** Unbinds the simplified mesh from the high res mesh */
	wxButton* UnbindFromHighResMeshButton;

	/** Label for high res mesh name */
	wxStaticText* HighResMeshNameLabel;

	/** Label for simplified mesh name */
	wxStaticText* SimplifiedMeshNameLabel;

	/** Label for high res mesh package */
	wxStaticText* HighResMeshPackageLabel;

	/** Label for simplified mesh package */
	wxStaticText* SimplifiedMeshPackageLabel;

	/** Label for high res mesh geometry */
	wxStaticText* HighResMeshGeometryLabel;

	/** Label for simplified mesh geometry */
	wxStaticText* SimplifiedMeshGeometryLabel;

	/** Label for number of actors in level using high res mesh */
	wxStaticText* HighResActorCountLabel;

	/** Label for number of actors in level using simplified mesh */
	wxStaticText* SimplifiedActorCountLabel;

	/** High resolution source mesh.  NULL unless we're processing an already-simplified mesh. */
	UStaticMesh* HighResSourceStaticMesh;


	DECLARE_EVENT_TABLE()
};



#endif // __MeshSimplificationWindow_h__
