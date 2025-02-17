#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Diagnostics;

 
namespace ConsoleInterface
{
	/// <summary> 
	/// Summary for DLLInterface
	/// </summary>
	__gc public class DLLInterface :  public System::ComponentModel::Component
	{
	public:
		DLLInterface(void)
		{
			InitializeComponent();
		}
		DLLInterface(System::ComponentModel::IContainer *container) : components(0)
		{
			/// <summary>
			/// Required for Windows.Forms Class Composition Designer support
			/// </summary>

			container->Add(this);
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
				
	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container *components;
		
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>		
		void InitializeComponent(void)
		{
			components = new System::ComponentModel::Container();
		}
	};
}