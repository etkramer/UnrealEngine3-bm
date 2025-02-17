using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace DVDLogParser
{
	/// <summary>
	/// Summary description for ProgressBar.
	/// </summary>
	public class ProgressBar : System.Windows.Forms.Form
	{
		private System.Windows.Forms.ProgressBar ProgressBar_Bar;
		private System.Windows.Forms.TextBox ProgressBar_Text;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public ProgressBar( string Title, int MaxValue )
		{
			// Required for Windows Form Designer support
			InitializeComponent();

			ProgressBar_Text.Text = Title;
			ProgressBar_Bar.Maximum = MaxValue;

			this.Show();
		}

		public void SetValue( int CurrentValue )
		{
			ProgressBar_Bar.Value = CurrentValue;
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
			this.ProgressBar_Bar = new System.Windows.Forms.ProgressBar();
			this.ProgressBar_Text = new System.Windows.Forms.TextBox();
			this.SuspendLayout();
			// 
			// ProgressBar_Bar
			// 
			this.ProgressBar_Bar.Location = new System.Drawing.Point(24, 48);
			this.ProgressBar_Bar.Name = "ProgressBar_Bar";
			this.ProgressBar_Bar.Size = new System.Drawing.Size(496, 32);
			this.ProgressBar_Bar.Step = 2;
			this.ProgressBar_Bar.TabIndex = 0;
			// 
			// ProgressBar_Text
			// 
			this.ProgressBar_Text.BackColor = System.Drawing.SystemColors.Control;
			this.ProgressBar_Text.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.ProgressBar_Text.CausesValidation = false;
			this.ProgressBar_Text.Location = new System.Drawing.Point(24, 16);
			this.ProgressBar_Text.Name = "ProgressBar_Text";
			this.ProgressBar_Text.ReadOnly = true;
			this.ProgressBar_Text.Size = new System.Drawing.Size(504, 13);
			this.ProgressBar_Text.TabIndex = 1;
			this.ProgressBar_Text.Text = "";
			this.ProgressBar_Text.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// ProgressBar
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(7, 13);
			this.ClientSize = new System.Drawing.Size(544, 94);
			this.Controls.Add(this.ProgressBar_Text);
			this.Controls.Add(this.ProgressBar_Bar);
			this.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "ProgressBar";
			this.Text = "Parsing ...";
			this.TopMost = true;
			this.ResumeLayout(false);

		}
		#endregion
	}
}
