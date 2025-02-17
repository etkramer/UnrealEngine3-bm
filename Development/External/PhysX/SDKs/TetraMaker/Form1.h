#pragma once

#include "glRenderer.h"
#include "SceneManager.h"
#include "dlgAbout.h"

static const int subdivisionResolution = 1;

// ---------------------------------------------------------
void String2CString(System::String *s, char *c_string, int n)
// ---------------------------------------------------------
{
	int len = s->Length;
	if (len > n-1) len = n-1;
	for (int i = 0; i < len; i++)
		c_string[i] = (char)s->Chars[i];
	c_string[s->Length] = '\0';
}

// ---------------------------------------------------------

namespace TetraMaker
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Threading;

	/// <summary> 
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the 
	///          'Resource File Name' property for the managed resource compiler tool 
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public __gc class Form1 : public System::Windows::Forms::Form
	{	
	public:
		Form1(void)
		{
			InitializeComponent();
		}
  
	protected:
		void Dispose(Boolean disposing)
		{
			if (disposing && components)
			{
				components->Dispose();
			}
			__super::Dispose(disposing);
		}
	private: System::Windows::Forms::Panel *  toolPanel;
	private: System::Windows::Forms::Panel *  viewPanel;
	private: System::Windows::Forms::MainMenu *  mainMenu1;
	private: System::Windows::Forms::MenuItem *  menuItem1;

	private: System::Windows::Forms::MenuItem *  menuItem3;
	private: System::Windows::Forms::MenuItem *  menuItem4;
	private: System::Windows::Forms::StatusBar *  statusBar;


	private: System::Windows::Forms::MenuItem *  menuItem7;
	private: System::Windows::Forms::MenuItem *  menuFileLoad;

	private: System::Windows::Forms::OpenFileDialog *  openFileDialog;
	private: System::Windows::Forms::SaveFileDialog *  saveFileDialog;
	private: System::Windows::Forms::MenuItem *  menuFileQuit;
	private: System::Windows::Forms::Panel *  surfacePanel;
	private: System::Windows::Forms::Label *  label1;
	private: System::Windows::Forms::CheckBox *  checkSurfaceVisible;
	private: System::Windows::Forms::CheckBox *  checkSurfaceWireframe;
	private: System::Windows::Forms::Panel *  panel1;
	private: System::Windows::Forms::CheckBox *  checkOriginalWireframe;
	private: System::Windows::Forms::CheckBox *  checkOriginalVisible;
	private: System::Windows::Forms::Label *  label2;

	private: System::Windows::Forms::Button *  buttonSurfaceSimplify;
	private: System::Windows::Forms::Panel *  panel2;
	private: System::Windows::Forms::Label *  label3;
	private: System::Windows::Forms::Button *  buttonVolumeGenerate;
	private: System::Windows::Forms::CheckBox *  checkVolumeWireframe;
	private: System::Windows::Forms::CheckBox *  checkVolumeVisible;
	private: System::Windows::Forms::Panel *  panel3;
	private: System::Windows::Forms::Label *  label4;
	private: System::Windows::Forms::HScrollBar *  scrollSubdivision;
	private: System::Windows::Forms::Label *  label5;
	private: System::Windows::Forms::Label *  labelSubdivision;
	private: System::Windows::Forms::Button *  buttonOriginalLoad;
	private: System::Windows::Forms::Button *  buttonSurfaceCopy;
	private: System::Windows::Forms::Button *  buttonSurfaceIso;
	private: System::Windows::Forms::Label *  label6;
	private: System::Windows::Forms::HScrollBar *  scrollXClipping;
	private: System::Windows::Forms::MenuItem *  menuLoadTetras;
	private: System::Windows::Forms::MenuItem *  menuSaveTetras;
	private: System::Windows::Forms::MenuItem *  menuItem6;
	private: System::Windows::Forms::MenuItem *  menuEdit;
	private: System::Windows::Forms::MenuItem *  menuUndo;
	private: System::Windows::Forms::CheckBox *  checkIsoSingle;

	private: System::Windows::Forms::Timer *  timer;
	private: System::Windows::Forms::ProgressBar *  progressBar;
	private: System::Windows::Forms::CheckBox *  checkCloseSurface;
	private: System::Windows::Forms::CheckBox *  checkSubdivideSurface;
	private: System::Windows::Forms::MenuItem *  menuItem2;
	private: System::Windows::Forms::MenuItem *  menuSaveSurface;
	private: System::Windows::Forms::Button *  buttonStuffer;
	private: System::Windows::Forms::MenuItem *  menuFileSave;
	private: System::Windows::Forms::MenuItem *  menuFileFlipOrientation;
	private: System::Windows::Forms::MenuItem *  menuFileMerge;




	private: System::ComponentModel::IContainer *  components;





	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = new System::ComponentModel::Container();
			this->toolPanel = new System::Windows::Forms::Panel();
			this->panel3 = new System::Windows::Forms::Panel();
			this->checkSubdivideSurface = new System::Windows::Forms::CheckBox();
			this->checkCloseSurface = new System::Windows::Forms::CheckBox();
			this->checkIsoSingle = new System::Windows::Forms::CheckBox();
			this->labelSubdivision = new System::Windows::Forms::Label();
			this->label5 = new System::Windows::Forms::Label();
			this->scrollSubdivision = new System::Windows::Forms::HScrollBar();
			this->label4 = new System::Windows::Forms::Label();
			this->panel2 = new System::Windows::Forms::Panel();
			this->progressBar = new System::Windows::Forms::ProgressBar();
			this->label6 = new System::Windows::Forms::Label();
			this->scrollXClipping = new System::Windows::Forms::HScrollBar();
			this->buttonVolumeGenerate = new System::Windows::Forms::Button();
			this->checkVolumeWireframe = new System::Windows::Forms::CheckBox();
			this->checkVolumeVisible = new System::Windows::Forms::CheckBox();
			this->label3 = new System::Windows::Forms::Label();
			this->buttonStuffer = new System::Windows::Forms::Button();
			this->panel1 = new System::Windows::Forms::Panel();
			this->buttonOriginalLoad = new System::Windows::Forms::Button();
			this->checkOriginalWireframe = new System::Windows::Forms::CheckBox();
			this->checkOriginalVisible = new System::Windows::Forms::CheckBox();
			this->label2 = new System::Windows::Forms::Label();
			this->surfacePanel = new System::Windows::Forms::Panel();
			this->buttonSurfaceIso = new System::Windows::Forms::Button();
			this->buttonSurfaceSimplify = new System::Windows::Forms::Button();
			this->buttonSurfaceCopy = new System::Windows::Forms::Button();
			this->checkSurfaceWireframe = new System::Windows::Forms::CheckBox();
			this->checkSurfaceVisible = new System::Windows::Forms::CheckBox();
			this->label1 = new System::Windows::Forms::Label();
			this->viewPanel = new System::Windows::Forms::Panel();
			this->mainMenu1 = new System::Windows::Forms::MainMenu();
			this->menuItem1 = new System::Windows::Forms::MenuItem();
			this->menuFileLoad = new System::Windows::Forms::MenuItem();
			this->menuFileMerge = new System::Windows::Forms::MenuItem();
			this->menuFileFlipOrientation = new System::Windows::Forms::MenuItem();
			this->menuFileSave = new System::Windows::Forms::MenuItem();
			this->menuItem7 = new System::Windows::Forms::MenuItem();
			this->menuSaveSurface = new System::Windows::Forms::MenuItem();
			this->menuItem2 = new System::Windows::Forms::MenuItem();
			this->menuLoadTetras = new System::Windows::Forms::MenuItem();
			this->menuSaveTetras = new System::Windows::Forms::MenuItem();
			this->menuItem6 = new System::Windows::Forms::MenuItem();
			this->menuFileQuit = new System::Windows::Forms::MenuItem();
			this->menuEdit = new System::Windows::Forms::MenuItem();
			this->menuUndo = new System::Windows::Forms::MenuItem();
			this->menuItem3 = new System::Windows::Forms::MenuItem();
			this->menuItem4 = new System::Windows::Forms::MenuItem();
			this->statusBar = new System::Windows::Forms::StatusBar();
			this->openFileDialog = new System::Windows::Forms::OpenFileDialog();
			this->saveFileDialog = new System::Windows::Forms::SaveFileDialog();
			this->timer = new System::Windows::Forms::Timer(this->components);
			this->toolPanel->SuspendLayout();
			this->panel3->SuspendLayout();
			this->panel2->SuspendLayout();
			this->panel1->SuspendLayout();
			this->surfacePanel->SuspendLayout();
			this->SuspendLayout();
			// 
			// toolPanel
			// 
			this->toolPanel->Anchor = (System::Windows::Forms::AnchorStyles)((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left);
			this->toolPanel->Controls->Add(this->panel3);
			this->toolPanel->Controls->Add(this->panel2);
			this->toolPanel->Controls->Add(this->panel1);
			this->toolPanel->Controls->Add(this->surfacePanel);
			this->toolPanel->Location = System::Drawing::Point(0, 8);
			this->toolPanel->Name = S"toolPanel";
			this->toolPanel->Size = System::Drawing::Size(168, 576);
			this->toolPanel->TabIndex = 0;
			// 
			// panel3
			// 
			this->panel3->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->panel3->Controls->Add(this->checkSubdivideSurface);
			this->panel3->Controls->Add(this->checkCloseSurface);
			this->panel3->Controls->Add(this->checkIsoSingle);
			this->panel3->Controls->Add(this->labelSubdivision);
			this->panel3->Controls->Add(this->label5);
			this->panel3->Controls->Add(this->scrollSubdivision);
			this->panel3->Controls->Add(this->label4);
			this->panel3->Location = System::Drawing::Point(8, 408);
			this->panel3->Name = S"panel3";
			this->panel3->Size = System::Drawing::Size(152, 152);
			this->panel3->TabIndex = 3;
			// 
			// checkSubdivideSurface
			// 
			this->checkSubdivideSurface->Location = System::Drawing::Point(8, 120);
			this->checkSubdivideSurface->Name = S"checkSubdivideSurface";
			this->checkSubdivideSurface->Size = System::Drawing::Size(128, 24);
			this->checkSubdivideSurface->TabIndex = 6;
			this->checkSubdivideSurface->Text = S"Subdivide surface";
			this->checkSubdivideSurface->CheckedChanged += new System::EventHandler(this, &TetraMaker::Form1::checkSubdivideSurface_CheckedChanged);
			// 
			// checkCloseSurface
			// 
			this->checkCloseSurface->Location = System::Drawing::Point(8, 96);
			this->checkCloseSurface->Name = S"checkCloseSurface";
			this->checkCloseSurface->Size = System::Drawing::Size(120, 24);
			this->checkCloseSurface->TabIndex = 5;
			this->checkCloseSurface->Text = S"Close surface";
			this->checkCloseSurface->CheckedChanged += new System::EventHandler(this, &TetraMaker::Form1::checkCloseSurface_CheckedChanged);
			// 
			// checkIsoSingle
			// 
			this->checkIsoSingle->Location = System::Drawing::Point(8, 72);
			this->checkIsoSingle->Name = S"checkIsoSingle";
			this->checkIsoSingle->Size = System::Drawing::Size(136, 24);
			this->checkIsoSingle->TabIndex = 4;
			this->checkIsoSingle->Text = S"Iso single surface";
			this->checkIsoSingle->CheckedChanged += new System::EventHandler(this, &TetraMaker::Form1::checkIsoSingle_CheckedChanged);
			// 
			// labelSubdivision
			// 
			this->labelSubdivision->Location = System::Drawing::Point(72, 32);
			this->labelSubdivision->Name = S"labelSubdivision";
			this->labelSubdivision->Size = System::Drawing::Size(40, 16);
			this->labelSubdivision->TabIndex = 3;
			this->labelSubdivision->Text = S"10";
			// 
			// label5
			// 
			this->label5->Location = System::Drawing::Point(8, 32);
			this->label5->Name = S"label5";
			this->label5->Size = System::Drawing::Size(72, 16);
			this->label5->TabIndex = 2;
			this->label5->Text = S"Subdivision:";
			// 
			// scrollSubdivision
			// 
			this->scrollSubdivision->LargeChange = 2;
			this->scrollSubdivision->Location = System::Drawing::Point(8, 48);
			this->scrollSubdivision->Minimum = 1;
			this->scrollSubdivision->Name = S"scrollSubdivision";
			this->scrollSubdivision->Size = System::Drawing::Size(136, 16);
			this->scrollSubdivision->TabIndex = 1;
			this->scrollSubdivision->Value = 1;
			this->scrollSubdivision->ValueChanged += new System::EventHandler(this, &TetraMaker::Form1::scrollSubdivision_ValueChanged);
			// 
			// label4
			// 
			this->label4->Font = new System::Drawing::Font(S"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, (System::Byte)0);
			this->label4->Location = System::Drawing::Point(8, 8);
			this->label4->Name = S"label4";
			this->label4->Size = System::Drawing::Size(100, 16);
			this->label4->TabIndex = 0;
			this->label4->Text = S"Parameters";
			// 
			// panel2
			// 
			this->panel2->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->panel2->Controls->Add(this->progressBar);
			this->panel2->Controls->Add(this->label6);
			this->panel2->Controls->Add(this->scrollXClipping);
			this->panel2->Controls->Add(this->buttonVolumeGenerate);
			this->panel2->Controls->Add(this->checkVolumeWireframe);
			this->panel2->Controls->Add(this->checkVolumeVisible);
			this->panel2->Controls->Add(this->label3);
			this->panel2->Controls->Add(this->buttonStuffer);
			this->panel2->Location = System::Drawing::Point(8, 232);
			this->panel2->Name = S"panel2";
			this->panel2->Size = System::Drawing::Size(152, 160);
			this->panel2->TabIndex = 2;
			// 
			// progressBar
			// 
			this->progressBar->Location = System::Drawing::Point(8, 64);
			this->progressBar->Name = S"progressBar";
			this->progressBar->Size = System::Drawing::Size(136, 16);
			this->progressBar->TabIndex = 9;
			// 
			// label6
			// 
			this->label6->Location = System::Drawing::Point(8, 112);
			this->label6->Name = S"label6";
			this->label6->Size = System::Drawing::Size(72, 16);
			this->label6->TabIndex = 8;
			this->label6->Text = S"X-Clipping:";
			// 
			// scrollXClipping
			// 
			this->scrollXClipping->Location = System::Drawing::Point(8, 128);
			this->scrollXClipping->Minimum = 1;
			this->scrollXClipping->Name = S"scrollXClipping";
			this->scrollXClipping->Size = System::Drawing::Size(136, 16);
			this->scrollXClipping->TabIndex = 7;
			this->scrollXClipping->Value = 1;
			this->scrollXClipping->ValueChanged += new System::EventHandler(this, &TetraMaker::Form1::scrollXClipping_ValueChanged);
			// 
			// buttonVolumeGenerate
			// 
			this->buttonVolumeGenerate->Location = System::Drawing::Point(8, 32);
			this->buttonVolumeGenerate->Name = S"buttonVolumeGenerate";
			this->buttonVolumeGenerate->Size = System::Drawing::Size(64, 23);
			this->buttonVolumeGenerate->TabIndex = 6;
			this->buttonVolumeGenerate->Text = S"Generate";
			this->buttonVolumeGenerate->Click += new System::EventHandler(this, &TetraMaker::Form1::buttonVolumeGenerate_Click);
			// 
			// checkVolumeWireframe
			// 
			this->checkVolumeWireframe->Location = System::Drawing::Point(64, 88);
			this->checkVolumeWireframe->Name = S"checkVolumeWireframe";
			this->checkVolumeWireframe->Size = System::Drawing::Size(56, 24);
			this->checkVolumeWireframe->TabIndex = 5;
			this->checkVolumeWireframe->Text = S"Wires";
			this->checkVolumeWireframe->CheckedChanged += new System::EventHandler(this, &TetraMaker::Form1::checkVolumeWireframe_CheckedChanged);
			// 
			// checkVolumeVisible
			// 
			this->checkVolumeVisible->Checked = true;
			this->checkVolumeVisible->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkVolumeVisible->Location = System::Drawing::Point(8, 88);
			this->checkVolumeVisible->Name = S"checkVolumeVisible";
			this->checkVolumeVisible->Size = System::Drawing::Size(64, 24);
			this->checkVolumeVisible->TabIndex = 4;
			this->checkVolumeVisible->Text = S"Visible";
			this->checkVolumeVisible->CheckedChanged += new System::EventHandler(this, &TetraMaker::Form1::checkVolumeVisible_CheckedChanged);
			// 
			// label3
			// 
			this->label3->Font = new System::Drawing::Font(S"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, (System::Byte)0);
			this->label3->Location = System::Drawing::Point(8, 8);
			this->label3->Name = S"label3";
			this->label3->Size = System::Drawing::Size(100, 16);
			this->label3->TabIndex = 1;
			this->label3->Text = S"Volume";
			// 
			// buttonStuffer
			// 
			this->buttonStuffer->Location = System::Drawing::Point(80, 32);
			this->buttonStuffer->Name = S"buttonStuffer";
			this->buttonStuffer->Size = System::Drawing::Size(64, 23);
			this->buttonStuffer->TabIndex = 3;
			this->buttonStuffer->Text = S"Stuffer";
			this->buttonStuffer->Click += new System::EventHandler(this, &TetraMaker::Form1::buttonStuffer_Click);
			// 
			// panel1
			// 
			this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->panel1->Controls->Add(this->buttonOriginalLoad);
			this->panel1->Controls->Add(this->checkOriginalWireframe);
			this->panel1->Controls->Add(this->checkOriginalVisible);
			this->panel1->Controls->Add(this->label2);
			this->panel1->Location = System::Drawing::Point(8, 8);
			this->panel1->Name = S"panel1";
			this->panel1->Size = System::Drawing::Size(152, 88);
			this->panel1->TabIndex = 1;
			// 
			// buttonOriginalLoad
			// 
			this->buttonOriginalLoad->Location = System::Drawing::Point(8, 32);
			this->buttonOriginalLoad->Name = S"buttonOriginalLoad";
			this->buttonOriginalLoad->Size = System::Drawing::Size(136, 24);
			this->buttonOriginalLoad->TabIndex = 3;
			this->buttonOriginalLoad->Text = S"Load";
			this->buttonOriginalLoad->Click += new System::EventHandler(this, &TetraMaker::Form1::buttonOriginalLoad_Click);
			// 
			// checkOriginalWireframe
			// 
			this->checkOriginalWireframe->Location = System::Drawing::Point(64, 56);
			this->checkOriginalWireframe->Name = S"checkOriginalWireframe";
			this->checkOriginalWireframe->Size = System::Drawing::Size(56, 24);
			this->checkOriginalWireframe->TabIndex = 2;
			this->checkOriginalWireframe->Text = S"Wires";
			this->checkOriginalWireframe->CheckedChanged += new System::EventHandler(this, &TetraMaker::Form1::checkOriginalWireframe_CheckedChanged);
			// 
			// checkOriginalVisible
			// 
			this->checkOriginalVisible->Checked = true;
			this->checkOriginalVisible->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkOriginalVisible->Location = System::Drawing::Point(8, 56);
			this->checkOriginalVisible->Name = S"checkOriginalVisible";
			this->checkOriginalVisible->Size = System::Drawing::Size(64, 24);
			this->checkOriginalVisible->TabIndex = 1;
			this->checkOriginalVisible->Text = S"Visible";
			this->checkOriginalVisible->CheckedChanged += new System::EventHandler(this, &TetraMaker::Form1::checkOriginalVisible_CheckedChanged);
			// 
			// label2
			// 
			this->label2->Font = new System::Drawing::Font(S"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, (System::Byte)0);
			this->label2->Location = System::Drawing::Point(8, 8);
			this->label2->Name = S"label2";
			this->label2->Size = System::Drawing::Size(100, 16);
			this->label2->TabIndex = 0;
			this->label2->Text = S"Original";
			// 
			// surfacePanel
			// 
			this->surfacePanel->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->surfacePanel->Controls->Add(this->buttonSurfaceIso);
			this->surfacePanel->Controls->Add(this->buttonSurfaceSimplify);
			this->surfacePanel->Controls->Add(this->buttonSurfaceCopy);
			this->surfacePanel->Controls->Add(this->checkSurfaceWireframe);
			this->surfacePanel->Controls->Add(this->checkSurfaceVisible);
			this->surfacePanel->Controls->Add(this->label1);
			this->surfacePanel->Location = System::Drawing::Point(8, 104);
			this->surfacePanel->Name = S"surfacePanel";
			this->surfacePanel->Size = System::Drawing::Size(152, 120);
			this->surfacePanel->TabIndex = 0;
			// 
			// buttonSurfaceIso
			// 
			this->buttonSurfaceIso->Location = System::Drawing::Point(80, 32);
			this->buttonSurfaceIso->Name = S"buttonSurfaceIso";
			this->buttonSurfaceIso->Size = System::Drawing::Size(64, 24);
			this->buttonSurfaceIso->TabIndex = 5;
			this->buttonSurfaceIso->Text = S"Iso";
			this->buttonSurfaceIso->Click += new System::EventHandler(this, &TetraMaker::Form1::buttonSurfaceIso_Click);
			// 
			// buttonSurfaceSimplify
			// 
			this->buttonSurfaceSimplify->Location = System::Drawing::Point(8, 64);
			this->buttonSurfaceSimplify->Name = S"buttonSurfaceSimplify";
			this->buttonSurfaceSimplify->Size = System::Drawing::Size(136, 23);
			this->buttonSurfaceSimplify->TabIndex = 4;
			this->buttonSurfaceSimplify->Text = S"Simplify";
			this->buttonSurfaceSimplify->Click += new System::EventHandler(this, &TetraMaker::Form1::buttonSurfaceSimplify_Click);
			// 
			// buttonSurfaceCopy
			// 
			this->buttonSurfaceCopy->Location = System::Drawing::Point(8, 32);
			this->buttonSurfaceCopy->Name = S"buttonSurfaceCopy";
			this->buttonSurfaceCopy->Size = System::Drawing::Size(64, 23);
			this->buttonSurfaceCopy->TabIndex = 3;
			this->buttonSurfaceCopy->Text = S"Copy";
			this->buttonSurfaceCopy->Click += new System::EventHandler(this, &TetraMaker::Form1::buttonSurfaceCopy_Click);
			// 
			// checkSurfaceWireframe
			// 
			this->checkSurfaceWireframe->Location = System::Drawing::Point(64, 88);
			this->checkSurfaceWireframe->Name = S"checkSurfaceWireframe";
			this->checkSurfaceWireframe->Size = System::Drawing::Size(56, 24);
			this->checkSurfaceWireframe->TabIndex = 2;
			this->checkSurfaceWireframe->Text = S"Wires";
			this->checkSurfaceWireframe->CheckedChanged += new System::EventHandler(this, &TetraMaker::Form1::checkSurfaceWireframe_CheckedChanged);
			// 
			// checkSurfaceVisible
			// 
			this->checkSurfaceVisible->Checked = true;
			this->checkSurfaceVisible->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkSurfaceVisible->Location = System::Drawing::Point(8, 88);
			this->checkSurfaceVisible->Name = S"checkSurfaceVisible";
			this->checkSurfaceVisible->Size = System::Drawing::Size(64, 24);
			this->checkSurfaceVisible->TabIndex = 1;
			this->checkSurfaceVisible->Text = S"Visible";
			this->checkSurfaceVisible->CheckedChanged += new System::EventHandler(this, &TetraMaker::Form1::checkSurfaceVisible_CheckedChanged);
			// 
			// label1
			// 
			this->label1->Font = new System::Drawing::Font(S"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, (System::Byte)0);
			this->label1->Location = System::Drawing::Point(8, 8);
			this->label1->Name = S"label1";
			this->label1->Size = System::Drawing::Size(100, 16);
			this->label1->TabIndex = 0;
			this->label1->Text = S"Surface";
			// 
			// viewPanel
			// 
			this->viewPanel->AllowDrop = true;
			this->viewPanel->Anchor = (System::Windows::Forms::AnchorStyles)(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right);
			this->viewPanel->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->viewPanel->Location = System::Drawing::Point(168, 8);
			this->viewPanel->Name = S"viewPanel";
			this->viewPanel->Size = System::Drawing::Size(696, 576);
			this->viewPanel->TabIndex = 1;
			this->viewPanel->Resize += new System::EventHandler(this, &TetraMaker::Form1::viewPanel_Resize);
			this->viewPanel->DragEnter += new System::Windows::Forms::DragEventHandler(this, &TetraMaker::Form1::viewPanel_DragEnter);
			this->viewPanel->MouseUp += new System::Windows::Forms::MouseEventHandler(this, &TetraMaker::Form1::viewPanel_MouseUp);
			this->viewPanel->Paint += new System::Windows::Forms::PaintEventHandler(this, &TetraMaker::Form1::viewPanel_Paint);
			this->viewPanel->DragDrop += new System::Windows::Forms::DragEventHandler(this, &TetraMaker::Form1::viewPanel_DragDrop);
			this->viewPanel->MouseMove += new System::Windows::Forms::MouseEventHandler(this, &TetraMaker::Form1::viewPanel_MouseMove);
			this->viewPanel->MouseDown += new System::Windows::Forms::MouseEventHandler(this, &TetraMaker::Form1::viewPanel_MouseDown);
			// 
			// mainMenu1
			// 
			System::Windows::Forms::MenuItem* __mcTemp__1[] = new System::Windows::Forms::MenuItem*[3];
			__mcTemp__1[0] = this->menuItem1;
			__mcTemp__1[1] = this->menuEdit;
			__mcTemp__1[2] = this->menuItem3;
			this->mainMenu1->MenuItems->AddRange(__mcTemp__1);
			// 
			// menuItem1
			// 
			this->menuItem1->Index = 0;
			System::Windows::Forms::MenuItem* __mcTemp__2[] = new System::Windows::Forms::MenuItem*[11];
			__mcTemp__2[0] = this->menuFileLoad;
			__mcTemp__2[1] = this->menuFileMerge;
			__mcTemp__2[2] = this->menuFileFlipOrientation;
			__mcTemp__2[3] = this->menuFileSave;
			__mcTemp__2[4] = this->menuItem7;
			__mcTemp__2[5] = this->menuSaveSurface;
			__mcTemp__2[6] = this->menuItem2;
			__mcTemp__2[7] = this->menuLoadTetras;
			__mcTemp__2[8] = this->menuSaveTetras;
			__mcTemp__2[9] = this->menuItem6;
			__mcTemp__2[10] = this->menuFileQuit;
			this->menuItem1->MenuItems->AddRange(__mcTemp__2);
			this->menuItem1->Text = S"&File";
			// 
			// menuFileLoad
			// 
			this->menuFileLoad->Index = 0;
			this->menuFileLoad->Text = S"&Load Original Mesh...";
			this->menuFileLoad->Click += new System::EventHandler(this, &TetraMaker::Form1::menuLoadMesh_Click);
			// 
			// menuFileMerge
			// 
			this->menuFileMerge->Index = 1;
			this->menuFileMerge->Text = S"&Weld Mesh";
			this->menuFileMerge->Click += new System::EventHandler(this, &TetraMaker::Form1::menuFileMerge_Click);
			// 
			// menuFileFlipOrientation
			// 
			this->menuFileFlipOrientation->Index = 2;
			this->menuFileFlipOrientation->Text = S"&Flip Mesh";
			this->menuFileFlipOrientation->Click += new System::EventHandler(this, &TetraMaker::Form1::menuFileFlipOrientation_Click);
			// 
			// menuFileSave
			// 
			this->menuFileSave->Index = 3;
			this->menuFileSave->Text = S"&Save Original Mesh...";
			this->menuFileSave->Click += new System::EventHandler(this, &TetraMaker::Form1::menuFileSave_Click);
			// 
			// menuItem7
			// 
			this->menuItem7->Index = 4;
			this->menuItem7->Text = S"-";
			// 
			// menuSaveSurface
			// 
			this->menuSaveSurface->Index = 5;
			this->menuSaveSurface->Text = S"Save Surface Mesh...";
			this->menuSaveSurface->Click += new System::EventHandler(this, &TetraMaker::Form1::menuSaveSurface_Click);
			// 
			// menuItem2
			// 
			this->menuItem2->Index = 6;
			this->menuItem2->Text = S"-";
			// 
			// menuLoadTetras
			// 
			this->menuLoadTetras->Index = 7;
			this->menuLoadTetras->Text = S"Load Tetrahedral Mesh...";
			this->menuLoadTetras->Click += new System::EventHandler(this, &TetraMaker::Form1::menuLoadTetras_Click);
			// 
			// menuSaveTetras
			// 
			this->menuSaveTetras->Index = 8;
			this->menuSaveTetras->Text = S"Save &Tetrahedral Mesh...";
			this->menuSaveTetras->Click += new System::EventHandler(this, &TetraMaker::Form1::menuSaveTetras_Click);
			// 
			// menuItem6
			// 
			this->menuItem6->Index = 9;
			this->menuItem6->Text = S"-";
			// 
			// menuFileQuit
			// 
			this->menuFileQuit->Index = 10;
			this->menuFileQuit->Text = S"&Quit";
			this->menuFileQuit->Click += new System::EventHandler(this, &TetraMaker::Form1::menuFileQuit_Click);
			// 
			// menuEdit
			// 
			this->menuEdit->Index = 1;
			System::Windows::Forms::MenuItem* __mcTemp__3[] = new System::Windows::Forms::MenuItem*[1];
			__mcTemp__3[0] = this->menuUndo;
			this->menuEdit->MenuItems->AddRange(__mcTemp__3);
			this->menuEdit->Text = S"&Edit";
			// 
			// menuUndo
			// 
			this->menuUndo->Index = 0;
			this->menuUndo->Shortcut = System::Windows::Forms::Shortcut::CtrlZ;
			this->menuUndo->Text = S"Undo";
			this->menuUndo->Click += new System::EventHandler(this, &TetraMaker::Form1::menuUndo_Click);
			// 
			// menuItem3
			// 
			this->menuItem3->Index = 2;
			System::Windows::Forms::MenuItem* __mcTemp__4[] = new System::Windows::Forms::MenuItem*[1];
			__mcTemp__4[0] = this->menuItem4;
			this->menuItem3->MenuItems->AddRange(__mcTemp__4);
			this->menuItem3->Text = S"&Help";
			// 
			// menuItem4
			// 
			this->menuItem4->Index = 0;
			this->menuItem4->Text = S"&About";
			this->menuItem4->Click += new System::EventHandler(this, &TetraMaker::Form1::menuItem4_Click);
			// 
			// statusBar
			// 
			this->statusBar->Location = System::Drawing::Point(0, 593);
			this->statusBar->Name = S"statusBar";
			this->statusBar->Size = System::Drawing::Size(872, 16);
			this->statusBar->TabIndex = 2;
			this->statusBar->Text = S"Ready";
			// 
			// timer
			// 
			this->timer->Tick += new System::EventHandler(this, &TetraMaker::Form1::timer_Tick);
			// 
			// Form1
			// 
			this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
			this->ClientSize = System::Drawing::Size(872, 609);
			this->Controls->Add(this->statusBar);
			this->Controls->Add(this->viewPanel);
			this->Controls->Add(this->toolPanel);
			this->KeyPreview = true;
			this->Menu = this->mainMenu1;
			this->Name = S"Form1";
			this->Text = S"Tetra Maker";
			this->KeyDown += new System::Windows::Forms::KeyEventHandler(this, &TetraMaker::Form1::Form1_KeyDown);
			this->Closing += new System::ComponentModel::CancelEventHandler(this, &TetraMaker::Form1::Form1_Closing);
			this->Load += new System::EventHandler(this, &TetraMaker::Form1::Form1_Load);
			this->KeyUp += new System::Windows::Forms::KeyEventHandler(this, &TetraMaker::Form1::Form1_KeyUp);
			this->toolPanel->ResumeLayout(false);
			this->panel3->ResumeLayout(false);
			this->panel2->ResumeLayout(false);
			this->panel1->ResumeLayout(false);
			this->surfacePanel->ResumeLayout(false);
			this->ResumeLayout(false);

		}	
	private: System::Void Form1_Load(System::Object *  sender, System::EventArgs *  e)
			 {
 				 Renderer *renderer = Renderer::getInstance();
				 renderer->init((HWND)(viewPanel->Handle.ToInt32()));
				 renderer->resize(viewPanel->Width, viewPanel->Height);
				 SceneManager::getInstance()->resize(viewPanel->Width, viewPanel->Height);
				 updateInterface();
			 }

   private: void updateInterface() 
			{
				SceneManager *sceneManager = SceneManager::getInstance();
				statusBar->Text = "Ready";
				checkOriginalVisible->Checked = sceneManager->viewOriginalVisible();
				checkOriginalWireframe->Checked = sceneManager->viewOriginalWireframe();
				checkSurfaceVisible->Checked = sceneManager->viewSurfaceVisible();
				checkSurfaceWireframe->Checked = sceneManager->viewSurfaceWireframe();
				checkVolumeVisible->Checked = sceneManager->viewVolumeVisible();
				checkVolumeWireframe->Checked = sceneManager->viewVolumeWireframe();
				int value = sceneManager->getSubdivision();
				scrollSubdivision->Value = value / subdivisionResolution;
				labelSubdivision->Text = value.ToString();
				checkIsoSingle->Checked = sceneManager->getIsoSingle();
				checkCloseSurface->Checked = sceneManager->getCloseSurface();
				checkSubdivideSurface->Checked = sceneManager->getSubdivideSurface();

				sceneManager->draw();
			}
	private: void setProcessing(bool on)
			 {
				if (on) {
					this->Cursor = Cursors::WaitCursor;
					statusBar->Text = "Processing..";
				}
				else {
					this->Cursor = Cursors::Default;
					statusBar->Text = "Ready";
				}
			 }

private: System::Void viewPanel_Paint(System::Object *  sender, System::Windows::Forms::PaintEventArgs *  e)
		 {
			 SceneManager *sceneManager = SceneManager::getInstance();
			 sceneManager->draw();
		 }

private: System::Void viewPanel_Resize(System::Object *  sender, System::EventArgs *  e)
		 {
			 Renderer *renderer = Renderer::getInstance();
			 renderer->resize(viewPanel->Width, viewPanel->Height);
			 SceneManager::getInstance()->resize(viewPanel->Width, viewPanel->Height);
		 }

private: System::Void viewPanel_MouseDown(System::Object *  sender, System::Windows::Forms::MouseEventArgs *  e)
		 {
			 SceneManager *sceneManager = SceneManager::getInstance();
			 SceneManagerMouseButton b = smbNone;
			 switch (e->Button) {
				 case MouseButtons::Left : b = smbLeft; break;
				 case MouseButtons::Middle : b = smbMiddle; break;
				 case MouseButtons::Right : b = smbRight; break;
			 }
			 sceneManager->mouseDown(e->X, e->Y, b);
			 const char *msg = sceneManager->getMouseMessage();
			 if (strlen(msg) > 0) {
				 ::MessageBox((HWND)Handle.ToInt32(), msg, "Mouse Message", MB_OK);
			 }
		 }

private: System::Void viewPanel_MouseMove(System::Object *  sender, System::Windows::Forms::MouseEventArgs *  e)
		 {
			 SceneManager *sceneManager = SceneManager::getInstance();
			 SceneManagerMouseButton b = smbNone;
			 switch (e->Button) {
				 case MouseButtons::Left : b = smbLeft; break;
				 case MouseButtons::Middle : b = smbMiddle; break;
				 case MouseButtons::Right : b = smbRight; break;
			 }
			 sceneManager->mouseMove(e->X, e->Y, b);
		 }

private: System::Void viewPanel_MouseUp(System::Object *  sender, System::Windows::Forms::MouseEventArgs *  e)
		 {
			 SceneManager *sceneManager = SceneManager::getInstance();
			 SceneManagerMouseButton b = smbNone;
			 switch (e->Button) {
				 case MouseButtons::Left : b = smbLeft; break;
				 case MouseButtons::Middle : b = smbMiddle; break;
				 case MouseButtons::Right : b = smbRight; break;
			 }
			 sceneManager->mouseUp(e->X, e->Y, b);
		 }

private: System::Void Form1_Closing(System::Object *  sender, System::ComponentModel::CancelEventArgs *  e)
		 {
			 SceneManager::destroyInstance();
		 }

private: System::Void viewPanel_DragDrop(System::Object *  sender, System::Windows::Forms::DragEventArgs *  e)
		 {
			if (e->Data->GetDataPresent(DataFormats::FileDrop)) {
				// Assign the file names to a String* array, in
				// case the user has selected multiple files.
				String* files[] = (String*[])e->Data->GetData(DataFormats::FileDrop);
				char filename[256], errorMessage[256];
				String2CString(files[0], filename, 256);
 				if (!SceneManager::getInstance()->loadMesh(filename, errorMessage, 256)) 
					::MessageBox((HWND)Handle.ToInt32(), errorMessage, "Load Error", MB_OK);
				else {
					updateInterface();
					char s[256];
					sprintf(s, "Loaded %i mesh triangles", 
						SceneManager::getInstance()->getNumOriginalTriangles());
					statusBar->Text = s; 
					sprintf(s, "Tetra Maker: %s", filename);
					this->Text = s;
				}
			}
		 }

private: System::Void viewPanel_DragEnter(System::Object *  sender, System::Windows::Forms::DragEventArgs *  e)
		 {
			if (e->Data->GetDataPresent(DataFormats::FileDrop)) 
				e->Effect = DragDropEffects::Copy;
			else 
				e->Effect = DragDropEffects::None;
		 }

private: System::Void Form1_KeyDown(System::Object *  sender, System::Windows::Forms::KeyEventArgs *  e)
		 {
			 // only works if KeyPreview is set to true in the form!
			 SceneManager *sceneManager = SceneManager::getInstance();
			 sceneManager->keyDown((unsigned char)e->KeyCode);
		 }

private: System::Void Form1_KeyUp(System::Object *  sender, System::Windows::Forms::KeyEventArgs *  e)
		 {
			 // only works if KeyPreview is set to true in the form!
			 SceneManager *sceneManager = SceneManager::getInstance();
			 sceneManager->keyUp((unsigned char)e->KeyCode);
		 }

private: System::Void menuLoadMesh_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 openFileDialog->Filter = "Wavefront meshes (*.obj)|*.obj|All files (*.*)|*.*";
			 if (openFileDialog->ShowDialog() != DialogResult::OK) return;
			 char filename[256], errorMessage[256];
			 String2CString(openFileDialog->FileName, filename, 256);
 			 if (!SceneManager::getInstance()->loadMesh(filename, errorMessage, 256)) 
				::MessageBox((HWND)Handle.ToInt32(), errorMessage, "Load Error", MB_OK);
			 else 
				updateInterface();

			char s[256];
			sprintf(s, "Loaded %i mesh triangles", 
				SceneManager::getInstance()->getNumOriginalTriangles());
			statusBar->Text = s; 
			sprintf(s, "Tetra Maker: %s", filename);
			this->Text = s;
		 }

private: System::Void menuFileQuit_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 Close();
		 }


private: System::Void checkSurfaceVisible_CheckedChanged(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager::getInstance()->setSurfaceVisible(checkSurfaceVisible->Checked);
		 }

private: System::Void checkSurfaceWireframe_CheckedChanged(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager::getInstance()->setSurfaceWireframe(checkSurfaceWireframe->Checked);
		 }

private: System::Void checkOriginalVisible_CheckedChanged(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager::getInstance()->setOriginalVisible(checkOriginalVisible->Checked);
		 }

private: System::Void checkOriginalWireframe_CheckedChanged(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager::getInstance()->setOriginalWireframe(checkOriginalWireframe->Checked);
		 }

private: System::Void buttonSurfaceSimplify_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 setProcessing(true);
			 SceneManager::getInstance()->simplifySurface();
			 setProcessing(false);
			char s[256];
			sprintf(s, "Generated %i surface triangles", SceneManager::getInstance()->getNumSurfaceTriangles());
			statusBar->Text = s; 
		 }

private: System::Void checkVolumeVisible_CheckedChanged(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager::getInstance()->setVolumeVisible(checkVolumeVisible->Checked);
		 }

private: System::Void checkVolumeWireframe_CheckedChanged(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager::getInstance()->setVolumeWireframe(checkVolumeWireframe->Checked);
		 }

public: System::Void ThreadProc(Object* stateInfo) 
{
	 SceneManager *sceneManager = SceneManager::getInstance();
	 sceneManager->generateVolume();
	 sceneManager->draw();
}


private: System::Void buttonVolumeGenerate_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager *sceneManager = SceneManager::getInstance();
			 if (timer->Enabled) {
				 sceneManager->cancelOperation();
				 buttonVolumeGenerate->Text = "Generate";
				 progressBar->Value = 0;
				 timer->Enabled = false;
			 }
			 else {
//				 if (SceneManager::getInstance()->getNumSurfaceTriangles() == 0) {
//					::MessageBox((HWND)Handle.ToInt32(), "Generate surface mesh first", "Volume Generation Error", MB_OK);
//					return;
//				}
				timer->Enabled = true;
				 buttonVolumeGenerate->Text = "Cancel";
				ThreadPool::QueueUserWorkItem(new WaitCallback(this, &TetraMaker::Form1::ThreadProc));
			 }
		 }

private: System::Void menuItem4_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 dlgAbout *about = new dlgAbout();
			 about->ShowDialog();
		 }

private: System::Void buttonOriginalLoad_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 menuLoadMesh_Click(NULL, NULL);
		 }

private: System::Void scrollSubdivision_ValueChanged(System::Object *  sender, System::EventArgs *  e)
		 {
			 int value = subdivisionResolution * scrollSubdivision->Value;
			 labelSubdivision->Text = value.ToString();
			 SceneManager::getInstance()->setSubdivision(value);
		 }

private: System::Void buttonSurfaceCopy_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager *sceneManager = SceneManager::getInstance();

			if (!sceneManager->copySurface()) {
				::MessageBox((HWND)Handle.ToInt32(), "Load original mesh first", "Surface Generation Error", MB_OK);
			}
			 updateInterface();
			char s[256];
			sprintf(s, "Generated %i surface triangles", sceneManager->getNumSurfaceTriangles());
			statusBar->Text = s; 
		 }

private: System::Void buttonSurfaceIso_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager *sceneManager = SceneManager::getInstance();

			 setProcessing(true);
			if (!SceneManager::getInstance()->isoSurface()) {
				::MessageBox((HWND)Handle.ToInt32(), "Load original mesh first", "Surface Generation Error", MB_OK);
			}
			 setProcessing(false);
			 updateInterface();
			char s[256];
			sprintf(s, "Generated %i surface triangles", sceneManager->getNumSurfaceTriangles());
			statusBar->Text = s; 
		 }

private: System::Void scrollXClipping_ValueChanged(System::Object *  sender, System::EventArgs *  e)
		 {
			 if (scrollXClipping->Maximum == 0) return;
			 float s = -0.05f + 1.1f * scrollXClipping->Value / scrollXClipping->Maximum;
			 SceneManager::getInstance()->setXClipping(s);
		 }

private: System::Void menuLoadTetras_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 openFileDialog->Filter = "Tetra meshes (*.tet)|*.tet|All files (*.*)|*.*";
			 if (openFileDialog->ShowDialog() != DialogResult::OK) return;
			 char filename[256], errorMessage[256];
			 String2CString(openFileDialog->FileName, filename, 256);
 			 if (!SceneManager::getInstance()->loadTetraMesh(filename, errorMessage, 256)) 
				::MessageBox((HWND)Handle.ToInt32(), errorMessage, "Load Error", MB_OK);
			 else 
				updateInterface();

			char s[256];
			sprintf(s, "Loaded %i mesh tetrahedra", 
				SceneManager::getInstance()->getNumTetrahedra());
			statusBar->Text = s; 
			sprintf(s, "Tetra Maker: %s", filename);
			this->Text = s;

		 }

private: System::Void menuSaveTetras_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 saveFileDialog->Filter = "Tetra meshes (*.tet)|*.tet|All files (*.*)|*.*";
			 saveFileDialog->AddExtension = true;
			 if (saveFileDialog->ShowDialog() != DialogResult::OK) return;
			 char filename[256], errorMessage[256];
			 String2CString(saveFileDialog->FileName, filename, 256);
 			 if (!SceneManager::getInstance()->saveTetraMesh(filename, errorMessage, 256))
				::MessageBox((HWND)Handle.ToInt32(), errorMessage, "Save Error", MB_OK);
		 }

private: System::Void menuUndo_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager::getInstance()->undo();
		 }

private: System::Void checkIsoSingle_CheckedChanged(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager::getInstance()->setIsoSingle(checkIsoSingle->Checked);
		 }

private: System::Void checkCloseSurface_CheckedChanged(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager::getInstance()->setCloseSurface(checkCloseSurface->Checked);
		 }

private: System::Void checkSubdivideSurface_CheckedChanged(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager::getInstance()->setSubdivideSurface(checkSubdivideSurface->Checked);
		 }

private: System::Void timer_Tick(System::Object *  sender, System::EventArgs *  e)
		 {	
			 SceneManager *sceneManager = SceneManager::getInstance();
			 int percentage = sceneManager->getPercentage();
			 progressBar->Value = percentage;
			 if (percentage == 100) {
				 timer->Enabled = false;
				 buttonVolumeGenerate->Text = "Generate";
				 updateInterface();
				char s[256];
				sprintf(s, "Generated %i tetrahedra", sceneManager->getNumTetrahedra());
				statusBar->Text = s; 
			 }
		 }

private: System::Void menuSaveSurface_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 saveFileDialog->Filter = "Wavefront meshes (*.obj)|*.obj|All files (*.*)|*.*";
			 saveFileDialog->AddExtension = true;
			 if (saveFileDialog->ShowDialog() != DialogResult::OK) return;
			 char filename[256], errorMessage[256];
			 String2CString(saveFileDialog->FileName, filename, 256);
 			 if (!SceneManager::getInstance()->saveSurface(filename, errorMessage, 256)) 
				::MessageBox((HWND)Handle.ToInt32(), errorMessage, "Save Error", MB_OK);
		 }

/*
private: System::Void buttonSurfaceHull_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 SceneManager *sceneManager = SceneManager::getInstance();

			 setProcessing(true);
			if (!SceneManager::getInstance()->hullSurface()) {
				::MessageBox((HWND)Handle.ToInt32(), "Load original mesh first", "Surface Generation Error", MB_OK);
			}
			 setProcessing(false);
			 updateInterface();
			char s[256];
			sprintf(s, "Generated %i surface triangles", sceneManager->getNumSurfaceTriangles());
			statusBar->Text = s; 
		 }
*/
private: System::Void buttonStuffer_Click(System::Object *  sender, System::EventArgs *  e)
		 {
 			 if (!SceneManager::getInstance()->stuffVolume()) 
				::MessageBox((HWND)Handle.ToInt32(), "Load a mesh first", "Stuffer error", MB_OK);
			 else 
				updateInterface();
			char s[256];
			sprintf(s, "Generated %i tetrahedra", SceneManager::getInstance()->getNumTetrahedra());
			statusBar->Text = s; 
		 }

private: System::Void menuFileSave_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 saveFileDialog->Filter = "Wavefront meshes (*.obj)|*.obj|All files (*.*)|*.*";
			 saveFileDialog->AddExtension = true;
			 if (saveFileDialog->ShowDialog() != DialogResult::OK) return;
			 char filename[256], errorMessage[256];
			 String2CString(saveFileDialog->FileName, filename, 256);
 			 if (!SceneManager::getInstance()->saveMesh(filename, errorMessage, 256)) 
				::MessageBox((HWND)Handle.ToInt32(), errorMessage, "Save Error", MB_OK);
			 else
				 updateInterface();
		 }

private: System::Void menuFileMerge_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 char errorMessage[256];
 			 if (!SceneManager::getInstance()->mergeMesh(errorMessage, 256)) 
				::MessageBox((HWND)Handle.ToInt32(), errorMessage, "Error", MB_OK);
			 else
				 updateInterface();
		 }

private: System::Void menuFileFlipOrientation_Click(System::Object *  sender, System::EventArgs *  e)
		 {
			 char errorMessage[256];
 			 if (!SceneManager::getInstance()->flipMesh(errorMessage, 256)) 
				::MessageBox((HWND)Handle.ToInt32(), errorMessage, "Flip Error", MB_OK);
			 else
				 updateInterface();
		 }

};
}


