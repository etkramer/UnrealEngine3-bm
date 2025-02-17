//=============================================================================
//	Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
//	Game Framework Wizard for UE3
//=============================================================================

using System;
using System.IO;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;

namespace WindowsApplication55
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
		private Divelements.WizardFramework.WizardPage pageCustomizeGame;
		private System.Windows.Forms.CheckBox checkBox2;
		private System.Windows.Forms.CheckBox checkBox3;
		private Divelements.WizardFramework.FinishPage finishPage1;
		private Divelements.WizardFramework.IntroductionPage introductionPage1;
		private Divelements.WizardFramework.InformationBox informationBox1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private Divelements.WizardFramework.WizardPage wizardPage1;
		private System.Windows.Forms.Label label10;
		private Divelements.WizardFramework.Wizard wizard1;
		private System.Windows.Forms.TextBox projectName;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.TextBox projectPrefix;
		private System.Windows.Forms.CheckBox updateLauncherEntries;
		private System.Windows.Forms.TextBox editorName;
		private System.Windows.Forms.Label label4;
		private System.ComponentModel.IContainer components;

		public Form1()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.pageCustomizeGame = new Divelements.WizardFramework.WizardPage();
            this.checkBox2 = new System.Windows.Forms.CheckBox();
            this.checkBox3 = new System.Windows.Forms.CheckBox();
            this.finishPage1 = new Divelements.WizardFramework.FinishPage();
            this.introductionPage1 = new Divelements.WizardFramework.IntroductionPage();
            this.informationBox1 = new Divelements.WizardFramework.InformationBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.wizardPage1 = new Divelements.WizardFramework.WizardPage();
            this.label4 = new System.Windows.Forms.Label();
            this.editorName = new System.Windows.Forms.TextBox();
            this.updateLauncherEntries = new System.Windows.Forms.CheckBox();
            this.label3 = new System.Windows.Forms.Label();
            this.projectPrefix = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.projectName = new System.Windows.Forms.TextBox();
            this.wizard1 = new Divelements.WizardFramework.Wizard();
            this.pageCustomizeGame.SuspendLayout();
            this.introductionPage1.SuspendLayout();
            this.wizardPage1.SuspendLayout();
            this.wizard1.SuspendLayout();
            this.SuspendLayout();
            // 
            // pageCustomizeGame
            // 
            this.pageCustomizeGame.Controls.Add(this.checkBox2);
            this.pageCustomizeGame.Controls.Add(this.checkBox3);
            this.pageCustomizeGame.Description = "Framework classes will be customized based on the options you choose below";
            this.pageCustomizeGame.Location = new System.Drawing.Point(21, 79);
            this.pageCustomizeGame.Name = "pageCustomizeGame";
            this.pageCustomizeGame.NextPage = this.finishPage1;
            this.pageCustomizeGame.PreviousPage = this.wizardPage1;
            this.pageCustomizeGame.Size = new System.Drawing.Size(455, 216);
            this.pageCustomizeGame.TabIndex = 1004;
            this.pageCustomizeGame.Text = "Customize your new game framework";
            this.pageCustomizeGame.BeforeDisplay += new System.EventHandler(this.pageCustomizeGame_BeforeDisplay);
            // 
            // checkBox2
            // 
            this.checkBox2.Location = new System.Drawing.Point(48, 17);
            this.checkBox2.Name = "checkBox2";
            this.checkBox2.Size = new System.Drawing.Size(125, 26);
            this.checkBox2.TabIndex = 0;
            this.checkBox2.Text = "Main Menu GUI";
            // 
            // checkBox3
            // 
            this.checkBox3.Location = new System.Drawing.Point(48, 43);
            this.checkBox3.Name = "checkBox3";
            this.checkBox3.Size = new System.Drawing.Size(125, 26);
            this.checkBox3.TabIndex = 0;
            this.checkBox3.Text = "In-Game HUD";
            // 
            // finishPage1
            // 
            this.finishPage1.AllowMovePrevious = false;
            this.finishPage1.FinishText = "Your new project has been created. To use your project launch Development\\Src\\Unr" +
                "ealEngine3.sln and compile the PCLaunch-MyNewGame.vcproj.";
            this.finishPage1.Location = new System.Drawing.Point(178, 68);
            this.finishPage1.Name = "finishPage1";
            this.finishPage1.PreviousPage = this.introductionPage1;
            this.finishPage1.Size = new System.Drawing.Size(305, 227);
            this.finishPage1.TabIndex = 1005;
            this.finishPage1.Text = "Completed Game Framework Wizard";
            this.finishPage1.BeforeDisplay += new System.EventHandler(this.finishPage1_BeforeDisplay);
            // 
            // introductionPage1
            // 
            this.introductionPage1.Controls.Add(this.informationBox1);
            this.introductionPage1.Controls.Add(this.label1);
            this.introductionPage1.Controls.Add(this.label2);
            this.introductionPage1.IntroductionText = "This wizard helps you:";
            this.introductionPage1.Location = new System.Drawing.Point(178, 68);
            this.introductionPage1.Name = "introductionPage1";
            this.introductionPage1.NextPage = this.wizardPage1;
            this.introductionPage1.Size = new System.Drawing.Size(305, 227);
            this.introductionPage1.TabIndex = 1003;
            this.introductionPage1.Text = "Welcome to the UnrealEngine 3 Game Framework Wizard";
            this.introductionPage1.BeforeDisplay += new System.EventHandler(this.introductionPage1_BeforeDisplay);
            // 
            // informationBox1
            // 
            this.informationBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.informationBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.informationBox1.Location = new System.Drawing.Point(31, 138);
            this.informationBox1.Name = "informationBox1";
            this.informationBox1.Size = new System.Drawing.Size(243, 82);
            this.informationBox1.TabIndex = 1;
            this.informationBox1.Text = "You can visit http://udn.epicgames.com for more information on this wizard";
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.label1.Location = new System.Drawing.Point(38, 39);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(239, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "?Create a new game framework";
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.label2.Location = new System.Drawing.Point(38, 60);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(239, 18);
            this.label2.TabIndex = 0;
            this.label2.Text = "?Customize the framework for your game";
            // 
            // wizardPage1
            // 
            this.wizardPage1.Controls.Add(this.label4);
            this.wizardPage1.Controls.Add(this.editorName);
            this.wizardPage1.Controls.Add(this.updateLauncherEntries);
            this.wizardPage1.Controls.Add(this.label3);
            this.wizardPage1.Controls.Add(this.projectPrefix);
            this.wizardPage1.Controls.Add(this.label10);
            this.wizardPage1.Controls.Add(this.projectName);
            this.wizardPage1.Description = "Choose a name for your new project.";
            this.wizardPage1.Location = new System.Drawing.Point(21, 79);
            this.wizardPage1.Name = "wizardPage1";
            this.wizardPage1.NextPage = this.finishPage1;
            this.wizardPage1.PreviousPage = this.introductionPage1;
            this.wizardPage1.Size = new System.Drawing.Size(455, 216);
            this.wizardPage1.TabIndex = 1008;
            this.wizardPage1.Text = "Project Name";
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(29, 86);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(96, 17);
            this.label4.TabIndex = 9;
            this.label4.Text = "Editor Name:";
            // 
            // editorName
            // 
            this.editorName.Location = new System.Drawing.Point(125, 86);
            this.editorName.Name = "editorName";
            this.editorName.Size = new System.Drawing.Size(144, 21);
            this.editorName.TabIndex = 6;
            this.editorName.Text = "MyNewEditor";
            // 
            // updateLauncherEntries
            // 
            this.updateLauncherEntries.Checked = true;
            this.updateLauncherEntries.CheckState = System.Windows.Forms.CheckState.Checked;
            this.updateLauncherEntries.Location = new System.Drawing.Point(125, 146);
            this.updateLauncherEntries.Name = "updateLauncherEntries";
            this.updateLauncherEntries.Size = new System.Drawing.Size(307, 44);
            this.updateLauncherEntries.TabIndex = 7;
            this.updateLauncherEntries.Text = "Add entries to launcher cpp/h files. Warning: Overwrites ExampleGame entries - ca" +
                "n only be done once!";
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(29, 52);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(96, 17);
            this.label3.TabIndex = 6;
            this.label3.Text = "Project prefix:";
            // 
            // projectPrefix
            // 
            this.projectPrefix.Location = new System.Drawing.Point(125, 52);
            this.projectPrefix.Name = "projectPrefix";
            this.projectPrefix.Size = new System.Drawing.Size(144, 21);
            this.projectPrefix.TabIndex = 5;
            this.projectPrefix.Text = "My";
            // 
            // label10
            // 
            this.label10.Location = new System.Drawing.Point(29, 17);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(96, 17);
            this.label10.TabIndex = 4;
            this.label10.Text = "Project Name:";
            // 
            // projectName
            // 
            this.projectName.Location = new System.Drawing.Point(125, 17);
            this.projectName.Name = "projectName";
            this.projectName.Size = new System.Drawing.Size(144, 21);
            this.projectName.TabIndex = 3;
            this.projectName.Text = "MyNewGame";
            // 
            // wizard1
            // 
            this.wizard1.BannerImage = ((System.Drawing.Image)(resources.GetObject("wizard1.BannerImage")));
            this.wizard1.Controls.Add(this.introductionPage1);
            this.wizard1.Controls.Add(this.wizardPage1);
            this.wizard1.Controls.Add(this.pageCustomizeGame);
            this.wizard1.Controls.Add(this.finishPage1);
            this.wizard1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.wizard1.ForeColor = System.Drawing.SystemColors.ControlText;
            this.wizard1.Location = new System.Drawing.Point(0, 0);
            this.wizard1.MarginImage = ((System.Drawing.Image)(resources.GetObject("wizard1.MarginImage")));
            this.wizard1.Name = "wizard1";
            this.wizard1.SelectedPage = this.introductionPage1;
            this.wizard1.Size = new System.Drawing.Size(497, 360);
            this.wizard1.TabIndex = 0;
            this.wizard1.Text = "wizard1";
            this.wizard1.Finish += new System.EventHandler(this.wizard1_Finish);
            this.wizard1.Cancel += new System.EventHandler(this.wizard1_Cancel);
            // 
            // Form1
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(6, 14);
            this.ClientSize = new System.Drawing.Size(497, 360);
            this.Controls.Add(this.wizard1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "Form1";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "UnrealEngine 3 Game Framework Wizard";
            this.pageCustomizeGame.ResumeLayout(false);
            this.introductionPage1.ResumeLayout(false);
            this.wizardPage1.ResumeLayout(false);
            this.wizardPage1.PerformLayout();
            this.wizard1.ResumeLayout(false);
            this.ResumeLayout(false);

		}
		#endregion

		public sealed class DirectoryUtils 
		{ 
			private DirectoryUtils() 
			{ 
			} 

			/// <summary> 
			///    Copies a directory to a new location. 
			/// </summary> 
			/// <param name="src">Source directory path</param> 
			/// <param name="dest">Destination directory path</param> 
			public static void CopyDirectory(String src, String dest) 
			{ 
				DirectoryInfo di = new DirectoryInfo(src); 

				foreach(FileSystemInfo fsi in di.GetFileSystemInfos()) 
				{ 
					String destName = Path.Combine(dest, fsi.Name); 
					if (fsi is FileInfo) 
					{
						// If dest file exists, clear its attributes so it can be overwritten
						if(File.Exists(destName))
							File.SetAttributes(destName,System.IO.FileAttributes.Normal);
						File.Copy(fsi.FullName, destName,true); 
						File.SetAttributes(destName,System.IO.FileAttributes.Normal);
					}
					else 
					{ 
						Directory.CreateDirectory(destName); 
						CopyDirectory(fsi.FullName, destName); 
					} 
				} 
			} 

			/// <summary>
			/// Recursive file gathering function - gathers files in nested dirs
			/// </summary>
			/// <param name="files"></param>
			/// <param name="dir"></param>
			public static void GetFilesRecursive(ArrayList files, string dir)
			{
				DirectoryInfo dirInfo = new DirectoryInfo(dir);
				foreach( DirectoryInfo di in dirInfo.GetDirectories() ) 
					GetFilesRecursive( files, di.FullName ); 
				foreach( FileInfo fi in dirInfo.GetFiles(  ) ) 
					files.Add(fi.FullName);
			}
		} 



		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new Form1());
		}

		/// <summary>
		/// Find/Replace in file
		/// </summary>
		/// <param name="file"></param>
		/// <param name="find"></param>
		/// <param name="replace"></param>
		private void FindReplace(string file,string find, string  replace)
		{
			StreamReader sr = new StreamReader(file); 
			string content = sr.ReadToEnd(); 
			sr.Close(); 

			try
			{
				StreamWriter sw = new StreamWriter(file); 
				// Replace Default
				content = content.Replace(find,replace);
				// Replace UPPERCASE
				content = content.Replace(find.ToUpper(),replace.ToUpper());
				// Replace lowercase
				content = content.Replace(find.ToLower(),replace.ToLower());

				sw.WriteLine(content); 
				sw.Close(); 
			}
			catch(Exception e)
			{
				if(DialogResult.Retry == System.Windows.Forms.MessageBox.Show(file+" is read-only. May need checking out of Perforce. Retry?",
					"File cannot be written",System.Windows.Forms.MessageBoxButtons.RetryCancel))
					{
						FindReplace(file,find,replace);
					}
			}
		}

		/// <summary>
		/// Renames files/classes to new project name
		/// </summary>
		/// <param name="files"></param>
		private void RenameFiles(ArrayList files)
		{
			foreach(string file in files)
			{
				// Find/replace contents in certain file types
				if(file.IndexOf(".cpp") != -1 || file.IndexOf(".h") != -1  || file.IndexOf(".vcproj") != -1 || file.IndexOf(".rc") != -1 )
					FindReplace(file,basePrefix,projectPrefix.Text);

				string newFile = file;
				int index = newFile.IndexOf(basePrefix);
				if(index!=-1)
					newFile = newFile.Substring(0,index)+projectPrefix.Text + newFile.Substring(index+basePrefix.Length);
				
				if(newFile != file)
				{
					if(System.IO.File.Exists(newFile))
						System.IO.File.Delete(newFile);
					System.IO.File.Move(file,newFile);
				}
			}
		}


		/// <summary>
		/// Base Project - Used as template for new projects
		/// </summary>
		string baseProject = "ExampleGame";
		string baseEditor  = "ExampleEditor";
		string basePrefix  = "Example";
		/// <summary>
		/// Creates a new project from the baseProject
		/// </summary>
		/// <param name="name"></param>
		private void CreateProject(string name)
		{
			// Confirm we are being run from the correct dir
			if(System.IO.Directory.Exists("..\\Binaries"))
				System.IO.Directory.SetCurrentDirectory("..");
			if(!System.IO.Directory.Exists("Binaries"))
			{
				System.Windows.Forms.MessageBox.Show("Can't find Binaries directory. Please ensure"+
					" wizard is being run from UnrealEngine3 or UnrealEngine3\\Binaries");
				return;
			}

			string srcDir = "Development\\Src\\";
			// Rename all project files/contents
			ArrayList files = new ArrayList();

			// Copy/rename base project
			DirectoryUtils.CopyDirectory(baseProject,projectName.Text);
			// Copy/rename source tree
			DirectoryUtils.CopyDirectory(srcDir+baseProject,srcDir+projectName.Text);
			DirectoryUtils.CopyDirectory(srcDir+baseEditor,srcDir+editorName.Text);

			string newProject = srcDir+"Windows\\"+projectName.Text+" Win32.vcproj";
			// Copy/rename launch project
			if(File.Exists(newProject))
				File.SetAttributes(newProject,System.IO.FileAttributes.Normal);
			System.IO.File.Copy(srcDir + "Windows\\" + baseProject + " Win32.vcproj", newProject, true);
			// Remove read-only flag if inherited
			File.SetAttributes(newProject,System.IO.FileAttributes.Normal);
			files.Add(srcDir + "Windows\\" + projectName.Text + " Win32.vcproj");

			// Copy/rename the com file for the game
            string newProjCom = "Binaries\\Win32\\" + projectName.Text + ".com";
			if(File.Exists(newProjCom))
				File.SetAttributes(newProjCom,System.IO.FileAttributes.Normal);
            System.IO.File.Copy("Binaries\\Win32\\" + baseProject + ".com", newProjCom, true);
			// Remove read-only flag if inherited
			File.SetAttributes(newProjCom,System.IO.FileAttributes.Normal);

			// Copy/rename the com file for the debug version of the game
            string newEditCom = "Binaries\\Win32\\DEBUG-" + projectName.Text + ".com";
			if(File.Exists(newEditCom))
				File.SetAttributes(newEditCom,System.IO.FileAttributes.Normal);
            System.IO.File.Copy("Binaries\\Win32\\DEBUG-" + baseProject + ".com", newEditCom, true);
			// Remove read-only flag if inherited
			File.SetAttributes(newEditCom,System.IO.FileAttributes.Normal);

			// copy the project and editor folders
			DirectoryUtils.GetFilesRecursive(files,srcDir+projectName.Text);
			DirectoryUtils.GetFilesRecursive(files,srcDir+editorName.Text);
			// Rename launcher entries
			if(updateLauncherEntries.Checked)
			{
				DirectoryUtils.GetFilesRecursive(files,srcDir+"Launch");
			}
			// Rename config files
			DirectoryUtils.GetFilesRecursive(files,projectName.Text+"\\Config");
			
			RenameFiles(files);

			// If they chose to rename the launcher entries then fix the solution for them too.
            if (updateLauncherEntries.Checked)
            {
                string solutionName = "Development\\Src\\xkx_UE3.sln";
                FindReplace(solutionName, baseProject + " Win32", projectName.Text + " Win32");
                FindReplace(solutionName, baseProject, projectName.Text);
                FindReplace(solutionName, baseEditor, editorName.Text);
            }
		}

		private void wizard1_Cancel(object sender, System.EventArgs e)
		{
			Close();
		}

		private void wizard1_Finish(object sender, System.EventArgs e)
		{
			Close();
		}

		private void finishPage1_BeforeDisplay(object sender, System.EventArgs e)
		{
			CreateProject(projectName.Text);
		}

		private void introductionPage1_BeforeDisplay(object sender, System.EventArgs e)
		{
		
		}

		private void pageCustomizeGame_BeforeDisplay(object sender, System.EventArgs e)
		{
		
		}

	}
}
