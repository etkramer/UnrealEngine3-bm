#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace TetraMaker
{
	/// <summary> 
	/// Summary for dlgAbout
	///
	/// WARNING: If you change the name of this class, you will need to change the 
	///          'Resource File Name' property for the managed resource compiler tool 
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public __gc class dlgAbout : public System::Windows::Forms::Form
	{
	public: 
		dlgAbout(void)
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
	private: System::Windows::Forms::Button *  buttonOK;
	private: System::Windows::Forms::Panel *  panel1;
	private: System::Windows::Forms::Label *  label1;
	private: System::Windows::Forms::Label *  label2;
	private: System::Windows::Forms::Label *  label3;
	private: System::Windows::Forms::Label *  label4;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container* components;

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->buttonOK = new System::Windows::Forms::Button();
			this->panel1 = new System::Windows::Forms::Panel();
			this->label1 = new System::Windows::Forms::Label();
			this->label2 = new System::Windows::Forms::Label();
			this->label3 = new System::Windows::Forms::Label();
			this->label4 = new System::Windows::Forms::Label();
			this->panel1->SuspendLayout();
			this->SuspendLayout();
			// 
			// buttonOK
			// 
			this->buttonOK->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->buttonOK->Location = System::Drawing::Point(104, 88);
			this->buttonOK->Name = S"buttonOK";
			this->buttonOK->Size = System::Drawing::Size(128, 24);
			this->buttonOK->TabIndex = 0;
			this->buttonOK->Text = S"OK";
			// 
			// panel1
			// 
			this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->panel1->Controls->Add(this->label1);
			this->panel1->Controls->Add(this->label2);
			this->panel1->Controls->Add(this->label3);
			this->panel1->Controls->Add(this->label4);
			this->panel1->Location = System::Drawing::Point(8, 8);
			this->panel1->Name = S"panel1";
			this->panel1->Size = System::Drawing::Size(336, 64);
			this->panel1->TabIndex = 2;
			// 
			// label1
			// 
			this->label1->Font = new System::Drawing::Font(S"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, (System::Byte)0);
			this->label1->Location = System::Drawing::Point(10, 8);
			this->label1->Name = S"label1";
			this->label1->Size = System::Drawing::Size(80, 16);
			this->label1->TabIndex = 4;
			this->label1->Text = S"TetraMaker";
			// 
			// label2
			// 
			this->label2->Location = System::Drawing::Point(10, 32);
			this->label2->Name = S"label2";
			this->label2->Size = System::Drawing::Size(100, 16);
			this->label2->TabIndex = 5;
			this->label2->Text = S"Version 1.0";
			// 
			// label3
			// 
			this->label3->Location = System::Drawing::Point(170, 32);
			this->label3->Name = S"label3";
			this->label3->Size = System::Drawing::Size(144, 16);
			this->label3->TabIndex = 2;
			this->label3->Text = S"Copyright 2006 by AGEIA";
			// 
			// label4
			// 
			this->label4->Location = System::Drawing::Point(170, 8);
			this->label4->Name = S"label4";
			this->label4->Size = System::Drawing::Size(152, 16);
			this->label4->TabIndex = 3;
			this->label4->Text = S"by Matthias Müller-Fischer";
			// 
			// dlgAbout
			// 
			this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
			this->ClientSize = System::Drawing::Size(352, 126);
			this->Controls->Add(this->panel1);
			this->Controls->Add(this->buttonOK);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->Name = S"dlgAbout";
			this->Text = S"About TetraMaker";
			this->panel1->ResumeLayout(false);
			this->ResumeLayout(false);

		}		
	};
}
