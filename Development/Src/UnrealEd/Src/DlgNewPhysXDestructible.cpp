/*=============================================================================
	DlgNewPhysXDestructible.cpp: Destructible Vertical Component.
	Copyright 2007-2008 AGEIA Technologies.
=============================================================================*/

#include "UnrealEd.h"
#include "Properties.h"
#include "UnFracturedStaticMesh.h"
#include "DlgNewPhysXDestructible.h"

IMPLEMENT_CLASS(UPhysXFractureOptions);

/*-----------------------------------------------------------------------------
WxDlgNewPhysXDestructible.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE(WxDlgNewPhysXDestructible, wxDialog)
EVT_BUTTON( wxID_OK, WxDlgNewPhysXDestructible::OnOK )
EVT_BUTTON( wxID_CANCEL, WxDlgNewPhysXDestructible::OnCancel )
EVT_CLOSE( WxDlgNewPhysXDestructible::OnClose )
END_EVENT_TABLE()

WxDlgNewPhysXDestructible::WxDlgNewPhysXDestructible()
{
	wxDialog::Create( NULL, wxID_ANY, TEXT("CreateNewPhysXDestructible"), wxDefaultPosition, wxDefaultSize );

	wxBoxSizer* HorizontalSizer = new wxBoxSizer( wxHORIZONTAL );
	{
		PropertyWindowSizer = new wxBoxSizer(wxVERTICAL);
		{
			PropertyWindow = new WxPropertyWindow;
			PropertyWindow->Create( this, GUnrealEd );
			PhysXFractureOptions = ConstructObject<UPhysXFractureOptions>( UPhysXFractureOptions::StaticClass() );
			PropertyWindow->SetObject( PhysXFractureOptions, 1, 0, 1 );
			PropertyWindow->SetMinSize( wxSize( 400, 360 ) );
			PropertyWindowSizer->Add( PropertyWindow, 1, wxALL|wxEXPAND, 5 );
		}
		HorizontalSizer->Add( PropertyWindowSizer, 1, wxALL|wxEXPAND, 5 );

		wxBoxSizer* ButtonSizer = new wxBoxSizer( wxVERTICAL );
		{
			wxButton* ButtonOK = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
			ButtonOK->SetDefault();
			ButtonSizer->Add( ButtonOK, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

			wxButton* ButtonCancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
			ButtonSizer->Add(ButtonCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
		}
		
		HorizontalSizer->Add(ButtonSizer, 0, wxALL, 5);
	}
	SetSizer(HorizontalSizer);

	FWindowUtil::LoadPosSize( TEXT("DlgNewPhysXDestructible"), this );
	GetSizer()->Fit(this);

	FLocalizeWindow( this );
}

WxDlgNewPhysXDestructible::~WxDlgNewPhysXDestructible()
{
	FWindowUtil::SavePosSize( TEXT("DlgNewPhysXDestructible"), this );
	delete PropertyWindow;
}

UBOOL WxDlgNewPhysXDestructible::Show( UFracturedStaticMesh* InFracturedStaticMesh, UPackage* InPackage, FString InFracturedMeshName)
{
	FracturedStaticMesh = InFracturedStaticMesh;
	Package = InPackage;
	FracturedMeshName = InFracturedMeshName;
	return wxDialog::Show( 1 );
}

void WxDlgNewPhysXDestructible::OnOK( wxCommandEvent& In )
{
	// Estimate number of generated pieces
	INT PieceCountEstimate = FracturedStaticMesh->GetNumFragments();
	for( INT Depth = 0; Depth < PhysXFractureOptions->SlicingLevels.Num(); ++Depth )
	{
		FPhysXSlicingParameters & Parameters = PhysXFractureOptions->SlicingLevels( Depth );
		PieceCountEstimate *= (Max( Parameters.SlicesInX, 0 )+1)*(Max( Parameters.SlicesInY, 0 )+1)*(Max( Parameters.SlicesInZ, 0 )+1);
	}
	if( PieceCountEstimate >= 1000 )
	{
		const FLOAT Log10Count = 0.4342944819f*appLoge( (FLOAT)PieceCountEstimate );
		const INT IntLog10Count = (INT)Log10Count;
		const INT Mult = Clamp( (INT)appPow( 10.0f, Log10Count-(FLOAT)IntLog10Count ), 0, 9 );
		PieceCountEstimate = (INT)(Mult*appPow( 10, IntLog10Count ));
		const char * WarningLargePieceCount_Key = PieceCountEstimate < 60000 ? "WarningLargePieceCount" : "WarningLargePieceCount64k";
		wxMessageDialog LargePieceCountWarning( this,
			*FString::Printf(LocalizeSecure(LocalizeUnrealEd(WarningLargePieceCount_Key), PieceCountEstimate)),
			wxString(*LocalizeUnrealEd("WarningLargePieceCount_Title")),
			wxOK|wxCANCEL|wxICON_EXCLAMATION
			);
		if( wxID_CANCEL == LargePieceCountWarning.ShowModal())
		{
			wxDialog::SetAffirmativeId( In.GetId() ); // not certain if need this
			wxDialog::AcceptAndClose();
			return;
		}
	}

	extern void PhysXFractureMesh( UFracturedStaticMesh* FracturedStaticMesh, 
								   UPhysXFractureOptions* FractureOptions,
								   UObject* Outer = INVALID_OBJECT, 
								   FName Name = NAME_None,
								   EObjectFlags SetFlags = 0);

	PropertyWindow->FlushLastFocused();
	Package->SetDirtyFlag(TRUE);

	PhysXFractureMesh( FracturedStaticMesh, PhysXFractureOptions, Package, FName( *FracturedMeshName ), RF_Standalone | RF_Public );

	wxDialog::SetAffirmativeId( In.GetId() ); // not certain if need this
	wxDialog::AcceptAndClose();
}

void WxDlgNewPhysXDestructible::OnCancel( wxCommandEvent& In )
{
	wxDialog::Destroy();
}

void WxDlgNewPhysXDestructible::OnClose( wxCloseEvent& In )
{
	wxDialog::Destroy();
}
