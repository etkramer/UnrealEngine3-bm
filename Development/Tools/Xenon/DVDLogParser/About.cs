using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace DVDLogParser
{
	/// <summary>
	/// Summary description for About.
	/// </summary>
	public class About : System.Windows.Forms.Form
	{
		private System.Windows.Forms.PictureBox AboutImage;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public About()
		{
			// Required for Windows Form Designer support
			InitializeComponent();

			this.Show();
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(About));
			this.AboutImage = new System.Windows.Forms.PictureBox();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// AboutImage
			// 
			this.AboutImage.AccessibleName = "AboutImage";
			this.AboutImage.Image = ((System.Drawing.Image)(resources.GetObject("AboutImage.Image")));
			this.AboutImage.Location = new System.Drawing.Point(16, 16);
			this.AboutImage.Name = "AboutImage";
			this.AboutImage.Size = new System.Drawing.Size(56, 56);
			this.AboutImage.TabIndex = 0;
			this.AboutImage.TabStop = false;
			// 
			// label1
			// 
			this.label1.Font = new System.Drawing.Font("Courier New", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label1.Location = new System.Drawing.Point(104, 16);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(288, 24);
			this.label1.TabIndex = 1;
			this.label1.Text = "DVD Log Parser";
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(104, 64);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(288, 40);
			this.label2.TabIndex = 2;
			this.label2.Text = "Tool to parse the log file of the Xbox 360 DVD authoring tool in realtime.";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(104, 128);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(288, 32);
			this.label3.TabIndex = 3;
			this.label3.Text = "(c) Epic Games, 2006";
			// 
			// About
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(400, 166);
			this.Controls.Add(this.label3);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.AboutImage);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "About";
			this.ShowInTaskbar = false;
			this.Text = "About";
			this.ResumeLayout(false);

		}
		#endregion
	}
}
