namespace UnrealConsole.Main
{
	partial class ScreenShotForm
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if(disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.menuMain = new System.Windows.Forms.MenuStrip();
			this.menuItemFile = new System.Windows.Forms.ToolStripMenuItem();
			this.menuItemFile_SaveAs = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.menuItemFile_Exit = new System.Windows.Forms.ToolStripMenuItem();
			this.saveImageDlg = new System.Windows.Forms.SaveFileDialog();
			this.pictMain = new System.Windows.Forms.PictureBox();
			this.menuMain.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.pictMain)).BeginInit();
			this.SuspendLayout();
			// 
			// menuMain
			// 
			this.menuMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuItemFile});
			this.menuMain.Location = new System.Drawing.Point(0, 0);
			this.menuMain.Name = "menuMain";
			this.menuMain.Size = new System.Drawing.Size(711, 24);
			this.menuMain.TabIndex = 1;
			this.menuMain.Text = "menuStrip1";
			// 
			// menuItemFile
			// 
			this.menuItemFile.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuItemFile_SaveAs,
            this.toolStripMenuItem1,
            this.menuItemFile_Exit});
			this.menuItemFile.Name = "menuItemFile";
			this.menuItemFile.Size = new System.Drawing.Size(37, 20);
			this.menuItemFile.Text = "&File";
			// 
			// menuItemFile_SaveAs
			// 
			this.menuItemFile_SaveAs.Name = "menuItemFile_SaveAs";
			this.menuItemFile_SaveAs.Size = new System.Drawing.Size(152, 22);
			this.menuItemFile_SaveAs.Text = "&Save As...";
			this.menuItemFile_SaveAs.Click += new System.EventHandler(this.menuItemFile_SaveAs_Click);
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(149, 6);
			// 
			// menuItemFile_Exit
			// 
			this.menuItemFile_Exit.Name = "menuItemFile_Exit";
			this.menuItemFile_Exit.Size = new System.Drawing.Size(152, 22);
			this.menuItemFile_Exit.Text = "E&xit";
			this.menuItemFile_Exit.Click += new System.EventHandler(this.menuItemFile_Exit_Click);
			// 
			// saveImageDlg
			// 
			this.saveImageDlg.DefaultExt = "png";
			this.saveImageDlg.Filter = "PNG (*.png)|*.png|JPEG (*.jpg)|*.jpg|Bitmap (*.bmp)|*.bmp|GIF (*.gif)|*.gif";
			this.saveImageDlg.RestoreDirectory = true;
			this.saveImageDlg.SupportMultiDottedExtensions = true;
			this.saveImageDlg.Title = "Save Image As...";
			// 
			// pictMain
			// 
			this.pictMain.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
			this.pictMain.Location = new System.Drawing.Point(0, 27);
			this.pictMain.Name = "pictMain";
			this.pictMain.Size = new System.Drawing.Size(209, 143);
			this.pictMain.TabIndex = 2;
			this.pictMain.TabStop = false;
			// 
			// ScreenShotForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.AutoScroll = true;
			this.ClientSize = new System.Drawing.Size(711, 495);
			this.Controls.Add(this.pictMain);
			this.Controls.Add(this.menuMain);
			this.MainMenuStrip = this.menuMain;
			this.Name = "ScreenShotForm";
			this.Text = "ScreenShotForm";
			this.menuMain.ResumeLayout(false);
			this.menuMain.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.pictMain)).EndInit();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.MenuStrip menuMain;
		private System.Windows.Forms.ToolStripMenuItem menuItemFile;
		private System.Windows.Forms.ToolStripMenuItem menuItemFile_SaveAs;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem menuItemFile_Exit;
		private System.Windows.Forms.SaveFileDialog saveImageDlg;
		private System.Windows.Forms.PictureBox pictMain;
	}
}