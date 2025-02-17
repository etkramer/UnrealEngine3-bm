namespace UnrealFrontend
{
	partial class ReviewInis
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
			if (disposing && (components != null))
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
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.LinkLabel_ModDir = new System.Windows.Forms.LinkLabel();
			this.button1 = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(13, 13);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(142, 13);
			this.label1.TabIndex = 0;
			this.label1.Text = "Please review the .ini files in:";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(13, 59);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(533, 13);
			this.label2.TabIndex = 1;
			this.label2.Text = "and make any changes necessary. Note that you won\'t have make these changes next " +
				"time you cook this mod.";
			// 
			// LinkLabel_ModDir
			// 
			this.LinkLabel_ModDir.AutoSize = true;
			this.LinkLabel_ModDir.Location = new System.Drawing.Point(13, 36);
			this.LinkLabel_ModDir.Name = "LinkLabel_ModDir";
			this.LinkLabel_ModDir.Size = new System.Drawing.Size(41, 13);
			this.LinkLabel_ModDir.TabIndex = 2;
			this.LinkLabel_ModDir.TabStop = true;
			this.LinkLabel_ModDir.Text = "ModDir";
			this.LinkLabel_ModDir.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.LinkLabel_ModDir_LinkClicked);
			// 
			// button1
			// 
			this.button1.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.button1.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.button1.Location = new System.Drawing.Point(242, 91);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(75, 23);
			this.button1.TabIndex = 3;
			this.button1.Text = "OK";
			this.button1.UseVisualStyleBackColor = true;
			// 
			// ReviewInis
			// 
			this.AcceptButton = this.button1;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(558, 126);
			this.ControlBox = false;
			this.Controls.Add(this.button1);
			this.Controls.Add(this.LinkLabel_ModDir);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.Name = "ReviewInis";
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Review .ini Files";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Button button1;
		public System.Windows.Forms.LinkLabel LinkLabel_ModDir;
	}
}