/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"

#include "EngineMeshClasses.h"
#include "GenericBrowser.h"
#include "MeshUtils.h"
#include "MeshSimplificationWindow.h"
#include "StaticMeshEditor.h"


/**
 * TODO:
 *
 *		- List box support for 'Details...' windows and 'Assignment Complete' message boxes
 *		- Add 'Don't show this again' check boxes to notification message boxes
 *				- Better yet, use a balloon pop up for non-interactive messages
 *		- Add 'Help' button that links to https://udn.epicgames.com/Three/MeshSimplificationTool
 *		- When user is placing new content, check for simplified version already in level
 *				- either swap these automatically at build time
 *				- or swap at placement time automatically
 *				- or prompt at placement time
 *		- Robust Undo/Redo support
 *		- Fractured object support (refracture after simplify)
 *		- Add map check warnings about high res meshes placed in level when a simplified mesh is available to use
 *		- Support interacting with *all loaded levels*?  (List box GUI?)
 *				- it's probably better than we don't support this
 *		- Use asset import timestamps instead of (fake) CRCs to detect high res mesh changes?
 *		- Skeletal Mesh support
 *		- Support per-vertex weighting of simplification (important for skeletal meshes)
 */



/**
 * WxMeshSimplificationWindow
 *
 * The Mesh Simplification tool allows users to create and maintain reduced-polycount versions of mesh
 * assets on a per-level basis.
 *
 * If you change functionality here, be sure to update the UDN help page:
 *      https://udn.epicgames.com/Three/MeshSimplificationTool
 */

BEGIN_EVENT_TABLE( WxMeshSimplificationWindow, wxPanel )
	EVT_COMMAND_SCROLL( ID_MeshSimp_TargetTriangleCount, WxMeshSimplificationWindow::OnTargetTriangleCountSlider )
	EVT_BUTTON( ID_MeshSimp_HighResActorDetails, WxMeshSimplificationWindow::OnHighResActorDetailsButton )
	EVT_BUTTON( ID_MeshSimp_SimplifiedActorDetails, WxMeshSimplificationWindow::OnSimplifiedActorDetailsButton )
	EVT_BUTTON( ID_MeshSimp_CreateOrUpdateSimplifiedMesh, WxMeshSimplificationWindow::OnCreateOrUpdateSimplifiedMeshButton )
	EVT_BUTTON( ID_MeshSimp_ApplySimplifiedToCurrentLevel, WxMeshSimplificationWindow::OnApplySimplifiedToCurrentLevelButton )
	EVT_BUTTON( ID_MeshSimp_ApplyHighResToCurrentLevel, WxMeshSimplificationWindow::OnApplyHighResToCurrentLevelButton )
	EVT_BUTTON( ID_MeshSimp_UnbindFromHighResMesh, WxMeshSimplificationWindow::OnUnbindFromHighResMeshButton )
END_EVENT_TABLE()


/** Constructor that takes a pointer to the owner object */
WxMeshSimplificationWindow::WxMeshSimplificationWindow( WxStaticMeshEditor* InStaticMeshEditor )
	: wxPanel( InStaticMeshEditor )	// Parent window
{
	// Store owner object
	StaticMeshEditor = InStaticMeshEditor;

	HighResSourceStaticMesh = NULL;

	// Register for 'current level has changed' events.  Most of the features in this editor pertain the 
	// level that the user is currently working in, so we need to know when the user switches levels
	GCallbackEvent->Register( CALLBACK_NewCurrentLevel, this );


	// @todo: Should be a global constant for the editor
	const INT BorderSize = 5;


	// Setup controls
	wxBoxSizer* TopVSizer = new wxBoxSizer( wxVERTICAL );
	{
		// Vertical space
		TopVSizer->AddSpacer( BorderSize );


		// High res mesh group
		wxStaticBoxSizer* HighResMeshGroupVSizer =
			new wxStaticBoxSizer( wxVERTICAL, this, *LocalizeUnrealEd( "MeshSimp_HighResMeshGroupLabel" ) );
		{
			// Vertical space
			HighResMeshGroupVSizer->AddSpacer( BorderSize );

			// High res mesh name label
			{
				HighResMeshNameLabel =
					new wxStaticText(
						this,								// Parent
						-1,									// ID
						wxString() );						// Text

				const INT SizerItemFlags =
					wxALL;							// Use border spacing on ALL sides

				HighResMeshGroupVSizer->Add(
					HighResMeshNameLabel,			// Sizer or control
					0,								// Sizing proportion
					SizerItemFlags,   				// Flags
					BorderSize );					// Border size
			}


			// High res mesh package label
			{
				HighResMeshPackageLabel =
					new wxStaticText(
						this,								// Parent
						-1,									// ID
						wxString() );						// Text

				const INT SizerItemFlags =
					wxALL;							// Use border spacing on ALL sides

				HighResMeshGroupVSizer->Add(
					HighResMeshPackageLabel,		// Sizer or control
					0,								// Sizing proportion
					SizerItemFlags,   				// Flags
					BorderSize );					// Border size
			}


			// High res mesh geometry label
			{
				HighResMeshGeometryLabel =
					new wxStaticText(
						this,								// Parent
						-1,									// ID
						wxString() );						// Text

				const INT SizerItemFlags =
					wxALL;							// Use border spacing on ALL sides

				HighResMeshGroupVSizer->Add(
					HighResMeshGeometryLabel,		// Sizer or control
					0,								// Sizing proportion
					SizerItemFlags,   				// Flags
					BorderSize );					// Border size
			}


			wxBoxSizer* HSizer = new wxBoxSizer( wxHORIZONTAL );
			{
				// High res actor count label
				{
					HighResActorCountLabel =
						new wxStaticText(
							this,								// Parent
							-1,									// ID
							wxString() );						// Text

					const INT SizerItemFlags =
						wxALL;							// Use border spacing on ALL sides

					HSizer->Add(
						HighResActorCountLabel,				// Control window
						0,									// Sizing proportion
						SizerItemFlags,						// Flags
						BorderSize );						// Border spacing amount
				}

				// Horizontal space
				HSizer->AddSpacer( BorderSize );

				// High res actor 'Details...' button
				{
					HighResActorDetailsButton =
						new wxButton(
							this,								// Parent
							ID_MeshSimp_HighResActorDetails,	// ID
							*LocalizeUnrealEd( "MeshSimp_HighResActorDetailsButton" ) );	// Text

					const INT SizerItemFlags =
						wxALL;							// Use border spacing on ALL sides

					HSizer->Add(
						HighResActorDetailsButton,			// Control window
						0,									// Sizing proportion
						SizerItemFlags,						// Flags
						BorderSize );						// Border spacing amount
				}

				const INT SizerItemFlags =
					wxALL;							// Use border spacing on ALL sides

				HighResMeshGroupVSizer->Add(
					HSizer,							// Sizer or control
					0,								// Sizing proportion
					SizerItemFlags,   				// Flags
					0 );							// Border size
			}


			// Vertical space
			HighResMeshGroupVSizer->AddSpacer( BorderSize );


			// 'Apply High Res to Current Level' button
			{
				ApplyHighResToCurrentLevelButton =
					new wxButton(
						this,									// Parent
						ID_MeshSimp_ApplyHighResToCurrentLevel,		// Event ID
						*LocalizeUnrealEd( "MeshSimp_ApplyHighResToCurrentLevel" ) );	// Caption

				const INT SizerItemFlags =
					wxALIGN_CENTER_HORIZONTAL |		// Center horizontally
					wxALL;							// Use border spacing on ALL sides

				HighResMeshGroupVSizer->Add(
					ApplyHighResToCurrentLevelButton,		// Sizer or control
					0,								// Sizing proportion
					SizerItemFlags,   				// Flags
					BorderSize );					// Border size
			}


			// Vertical space
			HighResMeshGroupVSizer->AddSpacer( BorderSize );


			const INT SizerItemFlags =
				wxEXPAND |						// Expand to fill space
				wxALL;							// Use border spacing on ALL sides

			TopVSizer->Add(
				HighResMeshGroupVSizer,			// Sizer or control
				0,								// Sizing proportion
				SizerItemFlags,   				// Flags
				BorderSize );					// Border size
		}


		// Vertical space
		TopVSizer->AddSpacer( BorderSize * 2 );


		// Simplification group
		wxStaticBoxSizer* SimplificationGroupVSizer =
			new wxStaticBoxSizer( wxVERTICAL, this, *LocalizeUnrealEd( "MeshSimp_SimplificationGroupLabel" ) );
		{
			// Vertical space
			SimplificationGroupVSizer->AddSpacer( BorderSize );

			wxBoxSizer* HSizer = new wxBoxSizer( wxHORIZONTAL );
			{
				// Target triangle count label
				{
					wxStaticText* StaticText =
						new wxStaticText(
							this,								// Parent window
							wxID_STATIC,						// Window ID
							*LocalizeUnrealEd( "MeshSimp_TargetTriangleCount" ),	// Label text
							wxDefaultPosition,					// Position
							wxDefaultSize );					// Size

					const INT SizerItemFlags =
						wxALL;							// Use border spacing on ALL sides

					HSizer->Add(
						StaticText,							// Control window
						0,									// Sizing proportion
						SizerItemFlags,						// Flags
						BorderSize );						// Border spacing amount
				}


				// Target triangle count
				{
					const INT SliderFlags =
						wxSL_HORIZONTAL	|		// Horizontal slider
						wxSL_LABELS;			// Displays min/max/current text labels

					// NOTE: Proper min/max/value settings will be filled in later in UpdateControls()
					TargetTriangleCountSlider =
						new wxSlider(
							this,									// Parent window
							ID_MeshSimp_TargetTriangleCount,		// Event ID
							1,										// Current value
							1,										// Min value
							1,										// Max value
							wxDefaultPosition,						// Position
							wxDefaultSize,							// Size
							SliderFlags );							// Flags

					const INT SizerItemFlags =
						wxEXPAND |						// Expand to fill space
						wxALL;							// Use border spacing on ALL sides

					HSizer->Add(
						TargetTriangleCountSlider,			// Control window
						1,									// Sizing proportion
						SizerItemFlags,						// Flags
						BorderSize );						// Border spacing amount
				}


				const INT SizerItemFlags =
					wxEXPAND |						// Expand to fill space
					wxALL;							// Use border spacing on ALL sides

				SimplificationGroupVSizer->Add(
					HSizer,							// Sizer or control
					0,								// Sizing proportion
					SizerItemFlags,   				// Flags
					0 );							// Border size
			}
				


			// 'Create or Update Simplified Mesh' button
			{
				CreateOrUpdateSimplifiedMeshButton =
					new wxButton(
						this,										// Parent
						ID_MeshSimp_CreateOrUpdateSimplifiedMesh,	// Event ID
						wxString() );	// Caption

				const INT SizerItemFlags =
					wxALIGN_CENTER_HORIZONTAL |		// Center horizontally
					wxALL;							// Use border spacing on ALL sides

				SimplificationGroupVSizer->Add(
					CreateOrUpdateSimplifiedMeshButton,	// Sizer or control
					0,									// Sizing proportion
					SizerItemFlags,   					// Flags
					BorderSize );						// Border size
			}


			// Vertical space
			SimplificationGroupVSizer->AddSpacer( BorderSize );


			const INT SizerItemFlags =
				wxEXPAND |						// Expand to fill space
				wxALL;							// Use border spacing on ALL sides

			TopVSizer->Add(
				SimplificationGroupVSizer,		// Sizer or control
				0,								// Sizing proportion
				SizerItemFlags,   				// Flags
				BorderSize );					// Border size
		}

		
		// Vertical space
		TopVSizer->AddSpacer( BorderSize * 2 );


		// Simplified mesh group
		wxStaticBoxSizer* SimplifiedMeshGroupVSizer =
			new wxStaticBoxSizer( wxVERTICAL, this, *LocalizeUnrealEd( "MeshSimp_SimplifiedMeshGroupLabel" ) );
		{
			// Vertical space
			SimplifiedMeshGroupVSizer->AddSpacer( BorderSize );

			// Simplified mesh name label
			{
				SimplifiedMeshNameLabel =
					new wxStaticText(
						this,								// Parent
						-1,									// ID
						wxString() );						// Text

				const INT SizerItemFlags =
					wxALL;							// Use border spacing on ALL sides

				SimplifiedMeshGroupVSizer->Add(
					SimplifiedMeshNameLabel,			// Sizer or control
					0,								// Sizing proportion
					SizerItemFlags,   				// Flags
					BorderSize );					// Border size
			}


			// Simplified mesh package label
			{
				SimplifiedMeshPackageLabel =
					new wxStaticText(
						this,								// Parent
						-1,									// ID
						wxString() );						// Text

				const INT SizerItemFlags =
					wxALL;							// Use border spacing on ALL sides

				SimplifiedMeshGroupVSizer->Add(
					SimplifiedMeshPackageLabel,		// Sizer or control
					0,								// Sizing proportion
					SizerItemFlags,   				// Flags
					BorderSize );					// Border size
			}


			// Simplified mesh geometry label
			{
				SimplifiedMeshGeometryLabel =
					new wxStaticText(
						this,								// Parent
						-1,									// ID
						wxString() );						// Text

				const INT SizerItemFlags =
					wxALL;							// Use border spacing on ALL sides

				SimplifiedMeshGroupVSizer->Add(
					SimplifiedMeshGeometryLabel,			// Sizer or control
					0,								// Sizing proportion
					SizerItemFlags,   				// Flags
					BorderSize );					// Border size
			}


			wxBoxSizer* HSizer = new wxBoxSizer( wxHORIZONTAL );
			{
				// Simplified actor count label
				{
					SimplifiedActorCountLabel =
						new wxStaticText(
							this,								// Parent
							-1,									// ID
							wxString() );						// Text

					const INT SizerItemFlags =
						wxALL;							// Use border spacing on ALL sides

					HSizer->Add(
						SimplifiedActorCountLabel,		// Sizer or control
						0,								// Sizing proportion
						SizerItemFlags,   				// Flags
						BorderSize );					// Border size
				}

				// Horizontal space
				HSizer->AddSpacer( BorderSize );

				// Simplified actor 'Details...' button
				{
					SimplifiedActorDetailsButton =
						new wxButton(
							this,								// Parent
							ID_MeshSimp_SimplifiedActorDetails,	// ID
							*LocalizeUnrealEd( "MeshSimp_SimplifiedActorDetailsButton" ) );	// Text

					const INT SizerItemFlags =
						wxALL;							// Use border spacing on ALL sides

					HSizer->Add(
						SimplifiedActorDetailsButton,		// Control window
						0,									// Sizing proportion
						SizerItemFlags,						// Flags
						BorderSize );						// Border spacing amount
				}

				const INT SizerItemFlags =
					wxALL;							// Use border spacing on ALL sides

				SimplifiedMeshGroupVSizer->Add(
					HSizer,							// Sizer or control
					0,								// Sizing proportion
					SizerItemFlags,   				// Flags
					0 );							// Border size
			}


			// Vertical space
			SimplifiedMeshGroupVSizer->AddSpacer( BorderSize );


			// 'Apply Simplified to Current Level' button
			{
				ApplySimplifiedToCurrentLevelButton =
					new wxButton(
						this,									// Parent
						ID_MeshSimp_ApplySimplifiedToCurrentLevel,		// Event ID
						*LocalizeUnrealEd( "MeshSimp_ApplySimplifiedToCurrentLevel" ) );	// Caption

				const INT SizerItemFlags =
					wxALIGN_CENTER_HORIZONTAL |		// Center horizontally
					wxALL;							// Use border spacing on ALL sides

				SimplifiedMeshGroupVSizer->Add(
					ApplySimplifiedToCurrentLevelButton,		// Sizer or control
					0,								// Sizing proportion
					SizerItemFlags,   				// Flags
					BorderSize );					// Border size
			}


			// 'Unbind From High Res Mesh' button
			// @todo: Consider exposing this button to the users (currently we hide it because it's somewhat dangerous.)
			UnbindFromHighResMeshButton = NULL;
			if( 0 )	
			{
				UnbindFromHighResMeshButton =
					new wxButton(
						this,									// Parent
						ID_MeshSimp_UnbindFromHighResMesh,		// Event ID
						*LocalizeUnrealEd( "MeshSimp_UnbindFromHighResMesh" ) );	// Caption

				const INT SizerItemFlags =
					wxALIGN_CENTER_HORIZONTAL |		// Center horizontally
					wxALL;							// Use border spacing on ALL sides

				SimplifiedMeshGroupVSizer->Add(
					UnbindFromHighResMeshButton,	// Sizer or control
					0,								// Sizing proportion
					SizerItemFlags,   				// Flags
					BorderSize );					// Border size
			}


			// Vertical space
			SimplifiedMeshGroupVSizer->AddSpacer( BorderSize );



			const INT SizerItemFlags =
				wxEXPAND |						// Expand to fill space
				wxALL;							// Use border spacing on ALL sides

			TopVSizer->Add(
				SimplifiedMeshGroupVSizer,		// Sizer or control
				0,								// Sizing proportion
				SizerItemFlags,   				// Flags
				BorderSize );					// Border size
		}


		// Vertical space
		TopVSizer->AddSpacer( BorderSize * 2 );
	}




	
	// Assign the master sizer to the window
	SetSizer( TopVSizer );


	// Load the high res source mesh if we need to
	if( !CacheHighResSourceStaticMesh( StaticMeshEditor->bSimplifyMode ? TRUE : FALSE ) )	// Warn user?
	{
		// Unable to cache high res source mesh.  Oh well, we'll deal with that later.
	}


	// Update controls
	UpdateControls();
}



WxMeshSimplificationWindow::~WxMeshSimplificationWindow()
{
	// Unregister for events
	GCallbackEvent->Unregister( CALLBACK_NewCurrentLevel, this );
}



/** Updates the controls in this panel */
void WxMeshSimplificationWindow::UpdateControls()
{
	// Cache the high res source mesh if we need to
	if( !CacheHighResSourceStaticMesh( FALSE ) )		// Warn user?
	{
		// Unable to cache high res source mesh.  Oh well, we'll deal with that later.
	}

	// Find the high res and simplified mesh so we can gather statistics
	UStaticMesh* HighResMesh = StaticMeshEditor->StaticMesh;
	UStaticMesh* SimplifiedMesh = NULL;
	const UBOOL bIsAlreadySimplified = ( StaticMeshEditor->StaticMesh->HighResSourceMeshName.Len() > 0 );
	if( bIsAlreadySimplified )
	{
		SimplifiedMesh = StaticMeshEditor->StaticMesh;

		// Mesh in editor is already simplified.  We'll need to track down the original source mesh.
		if( HighResSourceStaticMesh != NULL )
		{
			// Use the high res source mesh as our simplification source model
			HighResMesh = HighResSourceStaticMesh;
		}
		else
		{
			HighResMesh = NULL;
		}
	}


	// Triangle count
	{
		// Check triangle count of mesh
		INT CurMeshTriangles = 0;
		{
			// Grab triangle count of mesh's highest-resolution LOD
			FStaticMeshRenderData& BaseLOD = StaticMeshEditor->StaticMesh->LODModels( 0 );
			CurMeshTriangles = Max( BaseLOD.IndexBuffer.Indices.Num() / 3, 1 );
		}

		// Select a maximum triangle count
		INT MaxTriangles = 1;
		const UBOOL bIsAlreadySimplified =
			( StaticMeshEditor->StaticMesh->HighResSourceMeshName.Len() > 0 );
		if( bIsAlreadySimplified && HighResSourceStaticMesh != NULL )
		{
			// Use the high res source mesh's triangle count as the maximum
			FStaticMeshRenderData& BaseLOD = HighResSourceStaticMesh->LODModels( 0 );
			MaxTriangles = Max( BaseLOD.IndexBuffer.Indices.Num() / 3, 1 );
		}
		else if( StaticMeshEditor->StaticMesh != NULL )
		{
			// Use the editor's mesh's triangle count as the maximum
			MaxTriangles = CurMeshTriangles;
		}


		// If the mesh is already simplified, then we'll set the target triangle count to match
		// it's current triangle count.  Otherwise, we'll just choose use a percentage of the high res
		// mesh's triangle count.
		INT InitialTargetTriangleCount = CurMeshTriangles;
		if( !bIsAlreadySimplified )
		{
			// Default to two thirds of the high res triangle count
			InitialTargetTriangleCount = Max( ( MaxTriangles * 2 ) / 3, 1 );
		}


		// Update triangle count
		TargetTriangleCountSlider->SetRange( 1, MaxTriangles );
		TargetTriangleCountSlider->SetValue( InitialTargetTriangleCount );
	}



	// "Create or Update Simplified Mesh" button
	{
		if( SimplifiedMesh != NULL )
		{
			CreateOrUpdateSimplifiedMeshButton->SetLabel(
				*LocalizeUnrealEd( "MeshSimp_UpdateSimplifiedMesh" ) );
		}
		else
		{
			CreateOrUpdateSimplifiedMeshButton->SetLabel(
				*LocalizeUnrealEd( "MeshSimp_CreateSimplifiedMesh" ) );
		}
	}

	
	// Find all instances of high-res mesh
	TArray< AActor* > HighResActors;
	if( HighResMesh != NULL )
	{
		FindAllInstancesOfMesh(
			HighResMesh,		// Mesh to search for
			HighResActors );	// Out: Actors referencing this mesh
	}

	// Find all instances of simplified mesh
	TArray< AActor* > SimplifiedActors;
	if( SimplifiedMesh != NULL )
	{
		FindAllInstancesOfMesh(
			SimplifiedMesh,		// Mesh to search for
			SimplifiedActors );	// Out: Actors referencing this mesh
	}


	// "High Res Actor Details" button
	{
		// Only show the button if we have a high res mesh
		HighResActorDetailsButton->Show( HighResMesh != NULL );

		// Only enable the button if there is at least one actor using the mesh
		HighResActorDetailsButton->Enable( HighResActors.Num() > 0 );
	}


	// "Simplified Actor Details" button
	{
		// Only show the button if we have a simplified mesh
		SimplifiedActorDetailsButton->Show( SimplifiedMesh != NULL );

		// Only enable the button if there is at least one actor using the mesh
		SimplifiedActorDetailsButton->Enable( SimplifiedActors.Num() > 0 );
	}


	// Labels
	{
		// Count geometry
		INT HighResTriangleCount = 0;
		INT HighResVertexCount = 0;
		INT SimplifiedTriangleCount = 0;
		INT SimplifiedVertexCount = 0;
		INT SimplifiedTrianglePct = 100;
		INT SimplifiedVertexPct = 100;
		{
			if( HighResMesh != NULL )
			{
				if( HighResMesh->LODModels.Num() > 0 )
				{
					HighResTriangleCount = HighResMesh->LODModels( 0 ).IndexBuffer.Indices.Num() / 3;
					HighResVertexCount = HighResMesh->LODModels( 0 ).NumVertices;
				}
			}
			if( SimplifiedMesh != NULL )
			{
				if( SimplifiedMesh->LODModels.Num() > 0 )
				{
					SimplifiedTriangleCount = SimplifiedMesh->LODModels( 0 ).IndexBuffer.Indices.Num() / 3;
					SimplifiedVertexCount = SimplifiedMesh->LODModels( 0 ).NumVertices;
				}
			}

			if( HighResTriangleCount > 0 )
			{
				SimplifiedTrianglePct = SimplifiedTriangleCount * 100 / HighResTriangleCount;
			}

			if( HighResVertexCount > 0 )
			{
				SimplifiedVertexPct = SimplifiedVertexCount * 100 / HighResVertexCount;
			}
		}


		if( HighResMesh != NULL )
		{
			HighResMeshNameLabel->SetLabel(
				*FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_HighResMeshNameLabel_F" ), *HighResMesh->GetName() ) ) );

			HighResMeshPackageLabel->SetLabel(
				*FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_HighResMeshPackageLabel_F" ), *HighResMesh->GetOutermost()->GetName() ) ) );

			HighResMeshGeometryLabel->SetLabel(
				*FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_HighResMeshGeometryLabel_F" ),
					HighResTriangleCount, HighResVertexCount ) ) );

			HighResActorCountLabel->SetLabel(
				*FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_HighResActorCountLabel_F" ), HighResActors.Num() ) ) );
		}
		else
		{
			// High res mesh not found
			HighResMeshNameLabel->SetLabel( wxString() );
			HighResMeshPackageLabel->SetLabel( wxString() );
			HighResMeshGeometryLabel->SetLabel( wxString() );
			HighResActorCountLabel->SetLabel(
				*LocalizeUnrealEd( "MeshSimp_HighResActorCountLabel_NotAvailable" ) );
		}


		if( SimplifiedMesh != NULL )
		{
			SimplifiedMeshNameLabel->SetLabel(
				*FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_SimplifiedMeshNameLabel_F" ), *SimplifiedMesh->GetName() ) ) );

			SimplifiedMeshPackageLabel->SetLabel(
				*FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_SimplifiedMeshPackageLabel_F" ), *SimplifiedMesh->GetOutermost()->GetName() ) ) );

			SimplifiedMeshGeometryLabel->SetLabel(
				*FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_SimplifiedMeshGeometryLabel_F" ),
					SimplifiedTriangleCount, SimplifiedTrianglePct,
					SimplifiedVertexCount, SimplifiedVertexPct ) ) );

			SimplifiedActorCountLabel->SetLabel(
				*FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_SimplifiedActorCountLabel_F" ), SimplifiedActors.Num() ) ) );
		}
		else
		{
			// Unknown or non-existent simplified mesh
			SimplifiedMeshNameLabel->SetLabel( *LocalizeUnrealEd( "MeshSimp_SimplifiedMeshNameLabel_None" ) );
			SimplifiedMeshPackageLabel->SetLabel( wxString() );
			SimplifiedMeshGeometryLabel->SetLabel( wxString() );
			SimplifiedActorCountLabel->SetLabel( wxString() );
		}
	}


	// 'Apply Simplified to Current Level' button
	{
		// We only enable the button if we have both a simplified and high res mesh, and we have at least
		// one actor in the level that's still has the high res mesh assigned to it.
		UBOOL bCanApplyToLevel =
			( SimplifiedMesh != NULL && HighResMesh != NULL ) &&
			( HighResActors.Num() > 0 );
		ApplySimplifiedToCurrentLevelButton->Enable( bCanApplyToLevel ? true : false );
	}

	
	// 'Apply High Res to Current Level' button
	{
		// We only enable the button if we have both a simplified and high res mesh, and we have at least
		// one actor in the level that's still has the simplified mesh assigned to it.
		UBOOL bCanApplyToLevel =
			( SimplifiedMesh != NULL && HighResMesh != NULL ) &&
			( SimplifiedActors.Num() > 0 );
		ApplyHighResToCurrentLevelButton->Enable( bCanApplyToLevel ? true : false );
	}

	// 'Unbind From High Res Mesh' button
	if( UnbindFromHighResMeshButton != NULL )
	{
		// This only works if we have both a simplified and high res mesh
		UBOOL bCanUnbind =
			( SimplifiedMesh != NULL && HighResMesh != NULL );
		UnbindFromHighResMeshButton->Enable( bCanUnbind ? true : false );
	}


	// Control sizes may have changed, so update the window layout
	Layout();
}



/**
 *  Checks for a copy of the editor's mesh that's already simplified and stored in the level's package.  If
 *  one is found, we prompt the user to switch to viewing (resimplifying) the mesh we found instead.
 */
void WxMeshSimplificationWindow::CheckForAlreadySimplifiedDuplicate()
{
	if( StaticMeshEditor == NULL || StaticMeshEditor->StaticMesh == NULL )
	{
		// Need a static mesh in the editor to proceed
		return;
	}


	// If the mesh in the editor is already simplified, then we don't need to look for anything
	const UBOOL bIsAlreadySimplified =
		( StaticMeshEditor->StaticMesh->HighResSourceMeshName.Len() > 0 );
	if( !bIsAlreadySimplified )
	{
		UStaticMesh* ExistingSimplifiedMesh = NULL;

		for( FObjectIterator ObjectIt; ObjectIt != NULL; ++ObjectIt )
		{
			UObject* Object = *ObjectIt;

			// Is this a static mesh?
			UStaticMesh* StaticMesh = Cast<UStaticMesh>( Object );
			if( StaticMesh != NULL )
			{
				// Is it sitting in our level's package?
				if( StaticMesh->GetOutermost() == GWorld->CurrentLevel->GetOutermost() )
				{
					// Does it have a high res source mesh?
					if( StaticMesh->HighResSourceMeshName.Len() > 0 )
					{
						// Does the high res source mesh look like our editor's current mesh?
						if( StaticMesh->HighResSourceMeshName == StaticMeshEditor->StaticMesh->GetPathName() )
						{
							// We have a match!
							ExistingSimplifiedMesh = StaticMesh;
							break;
						}
					}
				}
			}
		}


		if( ExistingSimplifiedMesh != NULL )
		{
			UBOOL bShouldProceed =
				appMsgf( AMT_YesNo, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_SimplifiedCopyFoundInLevel_F" ), *StaticMeshEditor->StaticMesh->GetName(), *ExistingSimplifiedMesh->GetName(), *GWorld->CurrentLevel->GetOutermost()->GetName() ) ) );
			if( bShouldProceed )
			{
				// Switch static mesh editor to view the simplified mesh now.  This makes it easier to make
				// additional adjustments to the triangle count
				StaticMeshEditor->SetEditorMesh( ExistingSimplifiedMesh );

				// We may have modified the mesh we're looking at, so invalidate the viewport!
				StaticMeshEditor->InvalidateViewport();

				// Update mesh simplification window controls since actor instance count, etc may have changed
				UpdateControls();
			}
		}
	}
}



/**
 * Caches the high resolution source static mesh associated with the mesh in the editor (if there is one)
 *
 * @param bShouldWarnUser TRUE if we should warn the user about any problems encountered.
 * 
 * @return Returns TRUE If everything went OK.
 */
UBOOL WxMeshSimplificationWindow::CacheHighResSourceStaticMesh( BOOL bShouldWarnUser )
{
	UBOOL bRetVal = TRUE;
	UBOOL bShouldUnbindFromHighResMesh = FALSE;

	if( HighResSourceStaticMesh == NULL )
	{
		if( StaticMeshEditor != NULL && StaticMeshEditor->StaticMesh != NULL )
		{
			UStaticMesh* StaticMesh = StaticMeshEditor->StaticMesh;
			const UBOOL bIsAlreadySimplified = ( StaticMesh->HighResSourceMeshName.Len() > 0 );
			if( bIsAlreadySimplified )
			{
				// Start a 'slow task'
				const UBOOL bShowProgressWindow = FALSE;
				GWarn->BeginSlowTask( *LocalizeUnrealEd( "MeshSimp_LoadingHighResSourceMesh" ), bShowProgressWindow );

				// Mesh in editor is already simplified.  We'll need to track down the original source mesh.
				HighResSourceStaticMesh = StaticMesh->LoadHighResSourceMesh();


				if( HighResSourceStaticMesh != NULL )
				{
					if( bShouldWarnUser )
					{
						// Compute CRC of high resolution mesh
						const DWORD HighResMeshCRC = HighResSourceStaticMesh->ComputeSimplifiedCRCForMesh();

						if( HighResMeshCRC != StaticMesh->HighResSourceMeshCRC )
						{
							// It looks like the high res mesh has changed since we were generated.  We'll need
							// to let the user know about this.
							appMsgf( AMT_OK, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_HighResMeshHashChanged_F" ), *HighResSourceStaticMesh->GetName(), *StaticMeshEditor->StaticMesh->GetName() ) ) );
						}
					}
				}
				else
				{
					if( bShouldWarnUser )
					{
						bShouldUnbindFromHighResMesh =
							appMsgf( AMT_YesNo, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_ErrorLoadingSourceMesh_F" ), *StaticMesh->HighResSourceMeshName, *StaticMesh->GetName() ) ) );
					}
					bRetVal = FALSE;
				}



				// Finish 'slow task'
				GWarn->EndSlowTask();
			}
		}
	}


	if( bShouldUnbindFromHighResMesh )
	{
		// Prompt again to make sure this is really what they want to do
		PromptToUnbindFromHighResMesh();
		
		// If it looks like we're unbound now, then return TRUE
		if( StaticMeshEditor->StaticMesh->HighResSourceMeshName.Len() == 0 )
		{
			// Rescued by the unbind!
			bRetVal = TRUE;
		}
	}

	return bRetVal;
}



/** Invalidates the cached high res source static mesh.  Called when switching viewed meshes in the editor. */
void WxMeshSimplificationWindow::InvalidateHighResSourceStaticMesh()
{
	// Kill our reference to this
	HighResSourceStaticMesh = NULL;

	if( !CacheHighResSourceStaticMesh( StaticMeshEditor->bSimplifyMode ? TRUE : FALSE ) )
	{
		// Unable to cache high res source mesh.  Oh well, we'll deal with that later.
	}

	// Update controls (triangle counts, etc)
	UpdateControls();
}



/**
 * Prompts the user for a package/name for a new mesh, then creates a copy of the specified mesh.
 * If the target mesh already exists and appears to be compatible, we'll use that instead.
 *
 * @param InStaticMesh The mesh to duplicate
 *
 * @return The mesh that we found or created, or NULL if a problem was encountered.  Warnings reported internally.
 */
UStaticMesh* WxMeshSimplificationWindow::FindOrCreateCopyOfStaticMesh( UStaticMesh* InStaticMesh )
{
	// Allow the user to override the name and package for the new mesh
	// NOTE: We don't bother retaining the Group Name from the original mesh's package
	FString DefaultPackage = GWorld->CurrentLevel->GetOutermost()->GetName();
	FString DefaultGroup = TEXT("");
	FString DefaultName = InStaticMesh->GetName() + TEXT("_SIMPLIFIED");

	// Confirm the package and object name for the new simplified mesh
	WxDlgPackageGroupName dlg;
	if( dlg.ShowModal( DefaultPackage, DefaultGroup, DefaultName ) == wxID_OK )
	{
		// Validate the package and object name
		UPackage* NewPackage = NULL;
		FString NewObjName;
		const UBOOL bAllowCollisions = TRUE;
		UBOOL bValidPackage = dlg.ProcessNewAssetDlg( &NewPackage, &NewObjName, bAllowCollisions, UStaticMesh::StaticClass());
	
		if( bValidPackage && NewObjName.Len() > 0 )
		{
			// Check to see if the mesh already exists.  If it does, and its parent matches this object, then
			// we can offer to replace it.
			UStaticMesh* ExistingMesh = ( UStaticMesh* )UObject::StaticFindObject( UObject::StaticClass(), NewPackage, *NewObjName );
			if( ExistingMesh != NULL )
			{
				// OK, the mesh already exists.  If the current mesh is already the high res source mesh for the
				// existing mesh, then we'll just report back with the existing mesh so that it can be updated in place
				if( ExistingMesh->HighResSourceMeshName == InStaticMesh->GetPathName() )
				{
					// We're allowed to overwrite our own child
					return ExistingMesh;
				}
				else
				{
					// Destination mesh exists already
					appMsgf( AMT_OK, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_DestinationMeshAlreadyExists_F" ), *NewObjName, *dlg.GetPackage(), *InStaticMesh->GetName() ) ) );
				}
			}
			else
			{
				// Mesh doesn't exist so we'll create it now by duplicating the original asset 
				TMap< UObject*, UObject* > DuplicatedObjectList;

				// Duplicate the source mesh
				FObjectDuplicationParameters DupParams( InStaticMesh, NewPackage );
				{
					DupParams.DestName = FName( *NewObjName, FNAME_Add, TRUE );
					DupParams.CreatedObjects = &DuplicatedObjectList;

					// Check to see if we're duplicating into a level package or not.  If we're going to a level
					// package, we want to remove the 'standalone' flag from the mesh
					if( NewPackage->ContainsMap() )
					{
						// Remove the 'standalone' flag for the duplicated mesh, since we're copying it into a level
						// package and we don't want to keep it around if it never ends up being referenced by any actors
						// in the level.
						DupParams.FlagMask &= ~RF_Standalone;
					}
				}
				UStaticMesh* NewStaticMesh =
					CastChecked<UStaticMesh>( UObject::StaticDuplicateObjectEx( DupParams ) );
				if( NewStaticMesh != NULL )
				{
					// NOTE: It's possible that sub-objects were duplicated along with the static mesh.  This is
					//   often the case for URB_BodySetup objects.  The DuplicatedObjectList array will contain
					//   the static mesh as well as any other objects that were copied.

					if( !NewPackage->ContainsMap() )
					{
						// Looks like we're being created in a non-level package, so make sure RF_Standalone is set
						// so the mesh will actually be saved out
						NewStaticMesh->SetFlags( RF_Standalone );
					}

					// Done!  Return the new mesh.
					return NewStaticMesh;
				}
				else
				{
					// Unable to duplicate the source mesh
					appMsgf( AMT_OK, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_ErrorDuplicatingSourceMesh_F" ), *InStaticMesh->GetName() ) ) );
				}
			}
		}
	}

	// Cancelled or failed
	return NULL;
}



/**
 * Finds all actors that reference the specified static mesh
 *
 * @param InStaticMeshToSearchFor The static mesh we're looking for
 * @param OutActorsReferencingMesh (Out) List of actors that reference the specified mesh
 */
void WxMeshSimplificationWindow::FindAllInstancesOfMesh( const UStaticMesh* InStaticMeshToSearchFor,
														 TArray< AActor* >& OutActorsReferencingMesh )
{
	checkSlow( InStaticMeshToSearchFor != NULL );
	OutActorsReferencingMesh.Reset();


	TArray< AActor* > ActorsNotUpdated;

	// Locate actors that use this mesh
	ReplaceStaticMeshReferences(
		InStaticMeshToSearchFor,	// Mesh to find
		NULL,  						// New mesh
		OutActorsReferencingMesh,	// Out: Actors updated
		ActorsNotUpdated,			// Out: Actors not updated
		NULL );						// Feedback context for progress
}



/**
 * Replaces all actor references to the specified mesh with a new mesh
 *
 * @param InStaticMeshToSearchFor The mesh we're looking for
 * @param InNewStaticMesh The mesh we should replace with.  If NULL, we'll only search for meshes.
 * @param OutActorsUpdated (Out) Actors that were updated successfully
 * @param OutActorsNotUpdated (Out) Actors that we failed to update
 * @param FeedbackContext Optional context for reporting progress (can be NULL)
 */
void WxMeshSimplificationWindow::ReplaceStaticMeshReferences( const UStaticMesh* InStaticMeshToSearchFor,
															  UStaticMesh* InNewStaticMesh,
															  TArray< AActor* >& OutActorsUpdated,
															  TArray< AActor* >& OutActorsNotUpdated,
															  FFeedbackContext* FeedbackContext )
{
	checkSlow( InStaticMeshToSearchFor != NULL );
	checkSlow( InStaticMeshToSearchFor != InNewStaticMesh );

	OutActorsUpdated.Reset();
	OutActorsNotUpdated.Reset();

	FStaticMeshComponentReattachContext* ComponentReattachContext = NULL;
	if( InNewStaticMesh != NULL )
	{
		// We'll use a reattach context to detach this static mesh from any components using it before
		// swapping it out with a new mesh.
		ComponentReattachContext =
			new FStaticMeshComponentReattachContext( const_cast< UStaticMesh* >( InStaticMeshToSearchFor ) );
	}


	// First, count all of the meshes
	TArray< AActor* > ActorsToReplace;
	if( InNewStaticMesh != NULL )
	{
		FindAllInstancesOfMesh( InStaticMeshToSearchFor, ActorsToReplace );
	}


	// Iterate over all actors
	INT CurActorIndex = 0;
	for( FActorIterator It; It; ++It, ++CurActorIndex )
	{
		AActor* CurActor = *It;
		if( CurActor != NULL )
		{
			// We only care about actors in the current level
			if( CurActor->IsInLevel( GWorld->CurrentLevel ) )
			{
				if( InNewStaticMesh != NULL && FeedbackContext != NULL )
				{
					FeedbackContext->StatusUpdatef( CurActorIndex, ActorsToReplace.Num(), *LocalizeUnrealEd( "MeshSimp_UpdatingActorReferences" ) );
				}


				// StaticMeshActor
				AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>( CurActor );
				if( StaticMeshActor != NULL && StaticMeshActor->StaticMeshComponent != NULL )
				{
					// Make sure we have a static mesh
					UStaticMesh* StaticMesh = StaticMeshActor->StaticMeshComponent->StaticMesh;
					if( StaticMesh != NULL )
					{
						// Do the meshes match?
						if( StaticMesh == InStaticMeshToSearchFor )
						{
							// Are we replacing?
							if( InNewStaticMesh != NULL )
							{
								// Replace the mesh reference!
								StaticMeshActor->StaticMeshComponent->Modify( TRUE );
								StaticMeshActor->StaticMeshComponent->StaticMesh = InNewStaticMesh;

								// We swapped the mesh, so invalidate lighting
								CurActor->InvalidateLightingCache();
							}

							OutActorsUpdated.AddItem( CurActor );
						}
					}
					else
					{
						// No mesh associated with this actor's static mesh component!
					}
				}


				// DynamicSMActor
				ADynamicSMActor* DynamicSMActor = Cast<ADynamicSMActor>( CurActor );
				if( DynamicSMActor != NULL && DynamicSMActor->StaticMeshComponent != NULL )
				{
					// Make sure we have a static mesh
					UStaticMesh* StaticMesh = DynamicSMActor->StaticMeshComponent->StaticMesh;
					if( StaticMesh != NULL )
					{
						// Do the meshes match?
						if( StaticMesh == InStaticMeshToSearchFor )
						{
							// Are we replacing?
							if( InNewStaticMesh != NULL )
							{
								// Replace the mesh reference!
								DynamicSMActor->StaticMeshComponent->Modify( TRUE );
								DynamicSMActor->StaticMeshComponent->StaticMesh = InNewStaticMesh;
								
								// @todo: What about DynamicSMActor->ReplicatedMesh?  Should that be updated too?

								// We swapped the mesh, so invalidate lighting
								CurActor->InvalidateLightingCache();
							}

							OutActorsUpdated.AddItem( CurActor );
						}
					}
					else
					{
						// No mesh associated with this actor's static mesh component!
					}
				}
			}
		}
	}


	// Clean up reattach context if we used one
	if( ComponentReattachContext != NULL )
	{
		delete ComponentReattachContext;
		ComponentReattachContext = NULL;
	}

	if( ActorsToReplace.Num() > 0 && InNewStaticMesh != NULL && FeedbackContext != NULL )
	{
		FeedbackContext->StatusUpdatef( ActorsToReplace.Num(), ActorsToReplace.Num(), *LocalizeUnrealEd( "MeshSimp_UpdatingActorReferences" ) );
	}
}



/** Serialize this object to track references to UObject dependencies */
void WxMeshSimplificationWindow::Serialize( FArchive& Ar )
{
	// NOTE: May be NULL
	Ar << HighResSourceStaticMesh;
}




/**
 * Updates all actors to use either the simplified or high res mesh
 *
 * @param bReplaceWithSimplified TRUE to update actors to use the simplified mesh, or FALSE to use the high res mesh
 */
void WxMeshSimplificationWindow::UpdateActorReferences( UBOOL bReplaceWithSimplified )
{
	if( StaticMeshEditor != NULL && StaticMeshEditor->StaticMesh != NULL )
	{
		// Cache the high res source mesh if we need to
		if( !CacheHighResSourceStaticMesh( TRUE ) )		// Warn user?
		{
			// Unable to cache high res source mesh.  Warnings reported internally.
			return;
		}

		// Figure out what our source mesh is.  If the mesh in the editor is already simplified, then we'll
		// use its high resolution source mesh.
		const UBOOL bIsAlreadySimplified = ( StaticMeshEditor->StaticMesh->HighResSourceMeshName.Len() > 0 );
		if( bIsAlreadySimplified && HighResSourceStaticMesh != NULL )
		{
			UStaticMesh* SourceStaticMesh = bReplaceWithSimplified ? HighResSourceStaticMesh : StaticMeshEditor->StaticMesh;
			UStaticMesh* DestStaticMesh = bReplaceWithSimplified ? StaticMeshEditor->StaticMesh : HighResSourceStaticMesh;

			// Start a 'slow task'
			// NOTE: ReplaceStaticMeshReferences will be responsible for updating progress values
			const UBOOL bShowProgressWindow = TRUE;
			GWarn->BeginSlowTask( *LocalizeUnrealEd( "MeshSimp_UpdatingActorReferences" ), bShowProgressWindow );




			TArray< AActor* > ActorsUpdated;
			TArray< AActor* > ActorsNotUpdated;

			// Update any actors that use the mesh to point to the new simplified mesh
			ReplaceStaticMeshReferences(
				SourceStaticMesh,			// Mesh to replace
				DestStaticMesh,  			// New mesh
				ActorsUpdated,				// Out: Actors updated
				ActorsNotUpdated,			// Out: Actors not updated
				GWarn );					// Feedback context for progress


			// Finish 'slow task'
			GWarn->EndSlowTask();


			if( ActorsUpdated.Num() > 0 || ActorsNotUpdated.Num() > 0 )
			{
				// Make a list of actors that updated OK
				FString UpdatedActorsString;
				for( INT CurActorIndex = 0; CurActorIndex < ActorsUpdated.Num(); ++CurActorIndex )
				{
					UpdatedActorsString += ActorsUpdated( CurActorIndex )->GetName();
					UpdatedActorsString += TEXT( "\n" );
				}

				// Make a list of actors that failed to update
				FString FailedActorsString;
				for( INT CurActorIndex = 0; CurActorIndex < ActorsNotUpdated.Num(); ++CurActorIndex )
				{
					FailedActorsString += ActorsNotUpdated( CurActorIndex )->GetName();
					FailedActorsString += TEXT( "\n" );
				}

				if( ActorsNotUpdated.Num() == 0 )
				{
					// All actors updated OK!
					appMsgf( AMT_OK, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_AllActorsUpdatedOK_F" ), ActorsUpdated.Num(), *UpdatedActorsString ) ) );
				}
				else
				{
					// Some actors failed to update
					appMsgf( AMT_OK, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_ActorsFailedToUpdate_F" ), ActorsUpdated.Num(), ActorsNotUpdated.Num(), *FailedActorsString, *UpdatedActorsString ) ) );
				}
			}
		}


		// We changed in-level assets, so make sure the level editor viewports get refreshed
		GEditor->RedrawLevelEditingViewports();

		// Package dirtiness may have changed, so update the generic browser
		WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT( "GenericBrowser" ) );
		check( GenericBrowser != NULL );
		GenericBrowser->Update();

		// Update mesh simplification window controls since actor instance count, etc may have changed
		UpdateControls();
	}
}



/**
 * Asks the user if they really want to unbind from the high res mesh, then does so if the user chooses to.
 */
void WxMeshSimplificationWindow::PromptToUnbindFromHighResMesh()
{
	if( StaticMeshEditor != NULL && StaticMeshEditor->StaticMesh != NULL )
	{
		// Figure out what our source mesh is.  If the mesh in the editor is already simplified, then we'll
		// use its high resolution source mesh.
		const UBOOL bIsAlreadySimplified = ( StaticMeshEditor->StaticMesh->HighResSourceMeshName.Len() > 0 );
		if( bIsAlreadySimplified )
		{
			UStaticMesh* SimplifiedMesh = StaticMeshEditor->StaticMesh;

			UBOOL bShouldProceed =
				appMsgf( AMT_YesNo, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_ProceedWithUnbindFromHighResMesh_F" ), *StaticMeshEditor->StaticMesh->GetName(), *SimplifiedMesh->HighResSourceMeshName, *StaticMeshEditor->StaticMesh->GetName() ) ) );
			if( bShouldProceed )
			{
				// OK, time to unbind!

				// Remove references to source mesh
				SimplifiedMesh->Modify( TRUE );
				SimplifiedMesh->HighResSourceMeshName.Empty();
				SimplifiedMesh->HighResSourceMeshCRC = 0;

				// Invalidate cached high res mesh
				InvalidateHighResSourceStaticMesh();

				// Package dirtiness may have changed, so update the generic browser
				WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT( "GenericBrowser" ) );
				check( GenericBrowser != NULL );
				GenericBrowser->Update();

				// Update mesh simplification window controls since actor instance count, etc may have changed
				UpdateControls();
			}
		}
	}
}



/** Called when the triangle count slider is interacted with */
void WxMeshSimplificationWindow::OnTargetTriangleCountSlider( wxScrollEvent& In )
{
}



/** Called when 'High Res Actor Details' button is pressed */
void WxMeshSimplificationWindow::OnHighResActorDetailsButton( wxCommandEvent& In )
{
	// Cache the high res source mesh if we need to
	if( !CacheHighResSourceStaticMesh( FALSE ) )		// Warn user?
	{
		// Unable to cache high res source mesh.  Oh well, we'll deal with that later.
	}

	// Find the high res and simplified mesh so we can gather statistics
	UStaticMesh* HighResMesh = StaticMeshEditor->StaticMesh;
	const UBOOL bIsAlreadySimplified = ( StaticMeshEditor->StaticMesh->HighResSourceMeshName.Len() > 0 );
	if( bIsAlreadySimplified && HighResSourceStaticMesh != NULL )
	{
		// Use the high res source mesh as our simplification source model
		HighResMesh = HighResSourceStaticMesh;
	}

	// Find all instances of high-res mesh
	TArray< AActor* > HighResActors;
	if( HighResMesh != NULL )
	{
		FindAllInstancesOfMesh(
			HighResMesh,		// Mesh to search for
			HighResActors );	// Out: Actors referencing this mesh
	}


	if( HighResActors.Num() > 0 )
	{
		// Make a list of actors
		FString ActorsString;
		for( INT CurActorIndex = 0; CurActorIndex < HighResActors.Num(); ++CurActorIndex )
		{
			ActorsString += HighResActors( CurActorIndex )->GetName();
			ActorsString += TEXT( "\n" );
		}

		// Display the list
		appMsgf( AMT_OK, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_HighResActorDetails_F" ), HighResActors.Num(), *HighResMesh->GetName(), *ActorsString ) ) );
	}
}



/** Called when 'Simplified Actor Details' button is pressed */
void WxMeshSimplificationWindow::OnSimplifiedActorDetailsButton( wxCommandEvent& In )
{
	// Find the high res and simplified mesh so we can gather statistics
	UStaticMesh* SimplifiedMesh = NULL;
	const UBOOL bIsAlreadySimplified = ( StaticMeshEditor->StaticMesh->HighResSourceMeshName.Len() > 0 );
	if( bIsAlreadySimplified )
	{
		SimplifiedMesh = StaticMeshEditor->StaticMesh;
	}

	// Find all instances of simplified mesh
	TArray< AActor* > SimplifiedActors;
	if( SimplifiedMesh != NULL )
	{
		FindAllInstancesOfMesh(
			SimplifiedMesh,		// Mesh to search for
			SimplifiedActors );	// Out: Actors referencing this mesh
	}


	if( SimplifiedActors.Num() > 0 )
	{
		// Make a list of actors
		FString ActorsString;
		for( INT CurActorIndex = 0; CurActorIndex < SimplifiedActors.Num(); ++CurActorIndex )
		{
			ActorsString += SimplifiedActors( CurActorIndex )->GetName();
			ActorsString += TEXT( "\n" );
		}

		// Display the list
		appMsgf( AMT_OK, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_SimplifiedActorDetails_F" ), SimplifiedActors.Num(), *SimplifiedMesh->GetName(), *ActorsString ) ) );
	}
}



/** Called when 'Create or Update Simplified Mesh' button is pressed */
void WxMeshSimplificationWindow::OnCreateOrUpdateSimplifiedMeshButton( wxCommandEvent& In )
{
	if( StaticMeshEditor == NULL || StaticMeshEditor->StaticMesh == NULL )
	{
		// No static mesh in parent editor.  Should never happen?
		return;
	}


	// Cache the high res source mesh if we need to
	if( !CacheHighResSourceStaticMesh( TRUE ) )		// Warn user?
	{
		// Unable to cache high res source mesh.  Warnings reported internally.
		return;
	}



	UBOOL bSlowTaskStarted = FALSE;


	// Figure out what our source mesh is.  If the mesh in the editor is already simplified, then we'll
	// use its high resolution source mesh.
	UStaticMesh* SourceStaticMesh = NULL;
	UStaticMesh* DestStaticMesh = NULL;
	const UBOOL bIsAlreadySimplified = ( StaticMeshEditor->StaticMesh->HighResSourceMeshName.Len() > 0 );
	if( bIsAlreadySimplified && HighResSourceStaticMesh != NULL )
	{
		// Use the high res source mesh as our simplification source model
		SourceStaticMesh = HighResSourceStaticMesh;
		DestStaticMesh = StaticMeshEditor->StaticMesh;
	}
	else
	{
		// Source mesh is in the editor, but we don't have a simplified mesh yet
		SourceStaticMesh = StaticMeshEditor->StaticMesh;
		DestStaticMesh = NULL;
	}


	// Disallow simplification of meshes that are COOKED
	if( ( SourceStaticMesh->GetOutermost()->PackageFlags & PKG_Cooked ) ||
		( StaticMeshEditor->StaticMesh->GetOutermost()->PackageFlags & PKG_Cooked ) )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd( "Error_OperationDisallowedOnCookedContent" ) );
		return;
	}


	if( bIsAlreadySimplified )
	{
		// We'll update the existing simplified mesh

		// Start a 'slow task'
		const UBOOL bShowProgressWindow = TRUE;
		GWarn->BeginSlowTask( *LocalizeUnrealEd( "MeshSimp_UpdatingSimplifiedMesh" ), bShowProgressWindow );
		bSlowTaskStarted = TRUE;
	}
	else
	{
		// Prompt the user to make sure they want to proceed with creating a new mesh
		UBOOL bShouldProceed =
			appMsgf( AMT_YesNo, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_PromptToCreateNewMesh" ), *SourceStaticMesh->GetName() ) ) );
		if( bShouldProceed )
		{
			// Warn the user if the source mesh exists in the level's package file.  If the mesh is in the level's
			// package, it partly defeats the purpose of creating a simplified copy, since we may now need to load both
			// meshes into memory!
			if( SourceStaticMesh->GetOutermost() == GWorld->CurrentLevel->GetOutermost() )
			{
				bShouldProceed = appMsgf( AMT_YesNo, *LocalizeUnrealEd( "MeshSimp_SourceMeshInLevelPackageWarning" ) );
			}
			if( bShouldProceed )
			{
				// Start a 'slow task'
				const UBOOL bShowProgressWindow = TRUE;
				GWarn->BeginSlowTask( *LocalizeUnrealEd( "MeshSimp_CreatingSimplifiedMesh" ), bShowProgressWindow );
				bSlowTaskStarted = TRUE;

				// Find or create a destination mesh by copying the source mesh (if necessary)
				DestStaticMesh = FindOrCreateCopyOfStaticMesh( SourceStaticMesh );
			}
		}
	}


	if( SourceStaticMesh != NULL && DestStaticMesh != NULL )
	{
		// Simplify the mesh!
		{
			// Detach all instances of the static mesh while we're working
			FStaticMeshComponentReattachContext	ComponentReattachContext( DestStaticMesh );
			DestStaticMesh->ReleaseResources();
			DestStaticMesh->ReleaseResourcesFence.Wait();

			// Remove existing LODs
			{
				check( DestStaticMesh->LODInfo.Num() == DestStaticMesh->LODModels.Num() );
				while( DestStaticMesh->LODModels.Num() > 0 )
				{
					INT LODToRemove = DestStaticMesh->LODModels.Num() - 1;
					DestStaticMesh->LODModels.Remove( LODToRemove );
					DestStaticMesh->LODInfo.Remove( LODToRemove );
				}
			}

			// Generate new LOD
			{
				// Make sure our target triangle count is valid
				FStaticMeshRenderData& SourceMeshLOD = SourceStaticMesh->LODModels( 0 );
				INT SourceMeshTriCount = SourceMeshLOD.IndexBuffer.Indices.Num() / 3;
				INT TargetTriCount = Clamp( TargetTriangleCountSlider->GetValue(), 1, SourceMeshTriCount );

				// Simplify it!
				const INT DestLODIndex = 0;
				if( !GenerateLOD( SourceStaticMesh, DestStaticMesh, DestLODIndex, TargetTriCount ) )
				{
					// LOD generation failed!  We're in bad shape right now because we've already purged
					// the new mesh of LODs.  Warn the user.
					appMsgf( AMT_OK, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_GenerateLODFailed_F" ), *DestStaticMesh->GetName() ) ) );
				}
			}


			// Restore static mesh state
			DestStaticMesh->InitResources();
		}


		// Store fully qualified path and name of the mesh we were duplicated from
		DestStaticMesh->Modify( TRUE );
		DestStaticMesh->HighResSourceMeshName = SourceStaticMesh->GetPathName();

		// Generate and store the CRC of the high res mesh so that we'll know if we become outdated.
		DestStaticMesh->HighResSourceMeshCRC = SourceStaticMesh->ComputeSimplifiedCRCForMesh();


		// Check to see if the simplified mesh is what we're looking at in the editor
		if( StaticMeshEditor->StaticMesh != DestStaticMesh )
		{
			// Let the user know that we're going to switch the static mesh editor to view the simplified
			// mesh.  This provides more intuitive controls for re-simplifying the mesh later.
			appMsgf( AMT_OK, *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "MeshSimp_SwitchingEditorToSimplifedMesh_F" ), *DestStaticMesh->GetName() ) ) );

			// Switch static mesh editor to view the simplified mesh now.  This makes it easier to make
			// additional adjustments to the triangle count
			StaticMeshEditor->SetEditorMesh( DestStaticMesh );
		}
	}
	else
	{
		// Unable to find/duplicate the source mesh
		// NOTE: Warnings reported already by this point
	}


	if( bSlowTaskStarted )
	{
		// Finish 'slow task'
		GWarn->EndSlowTask();
	}


	// We may have created a new object, so update the generic browser
	WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT( "GenericBrowser" ) );
	check( GenericBrowser != NULL );
	GenericBrowser->Update();

	// The triangle/vertex counts for the currently-viewed mesh may have changed; UpdateToolbars re-caches this
	StaticMeshEditor->UpdateToolbars();

	// We may have modified the mesh we're looking at, so invalidate the viewport!
	StaticMeshEditor->InvalidateViewport();

	// We changed an asset that was probably referenced by in-level assets, so make sure the level editor
	// viewports get refreshed
	GEditor->RedrawLevelEditingViewports();

	// Update mesh simplification window controls since actor instance count, etc may have changed
	UpdateControls();
}



/** Called when 'Apply Simplified to Current Level' button is pressed */
void WxMeshSimplificationWindow::OnApplySimplifiedToCurrentLevelButton( wxCommandEvent& In )
{
	const UBOOL bReplaceWithSimplified = TRUE;
	UpdateActorReferences( bReplaceWithSimplified );
}



/** Called when 'Apply High Res to Current Level' button is pressed */
void WxMeshSimplificationWindow::OnApplyHighResToCurrentLevelButton( wxCommandEvent& In )
{
	const UBOOL bReplaceWithSimplified = FALSE;
	UpdateActorReferences( bReplaceWithSimplified );

	// @todo: Provide an option to delete the simplified mesh after replacing with high res?
}



/** Called when 'Unbind From High Res Mesh' button is pressed */
void WxMeshSimplificationWindow::OnUnbindFromHighResMeshButton( wxCommandEvent& In )
{
	PromptToUnbindFromHighResMesh();
}



/**
 * FCallbackEventDevice interface
 */
void WxMeshSimplificationWindow::Send( ECallbackEventType Event )
{
	if( Event == CALLBACK_NewCurrentLevel )
	{
		// OK the loaded level may have changed, so update our dialog window
		UpdateControls();
	}
}



/**
 * FCallbackEventDevice interface
 */
void WxMeshSimplificationWindow::Send( ECallbackEventType Event, DWORD Param )
{
	if( Event == CALLBACK_MapChange && ( Param & MapChangeEventFlags::NewMap ) )
	{
		// OK the loaded level may have changed, so update our dialog window
		UpdateControls();
	}
}
