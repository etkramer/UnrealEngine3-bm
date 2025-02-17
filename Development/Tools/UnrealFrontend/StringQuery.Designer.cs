namespace UnrealFrontend
{
	partial class StringQuery
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
			System.Windows.Forms.Button button1;
			System.Windows.Forms.Button button2;
			this.TextBox_String = new System.Windows.Forms.TextBox();
			this.Label_Caption = new System.Windows.Forms.Label();
			button1 = new System.Windows.Forms.Button();
			button2 = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// button1
			// 
			button1.DialogResult = System.Windows.Forms.DialogResult.OK;
			button1.Location = new System.Drawing.Point(217, 55);
			button1.Name = "button1";
			button1.Size = new System.Drawing.Size(75, 23);
			button1.TabIndex = 0;
			button1.Text = "OK";
			button1.UseVisualStyleBackColor = true;
			// 
			// button2
			// 
			button2.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			button2.Location = new System.Drawing.Point(298, 55);
			button2.Name = "button2";
			button2.Size = new System.Drawing.Size(75, 23);
			button2.TabIndex = 1;
			button2.Text = "Cancel";
			button2.UseVisualStyleBackColor = true;
			// 
			// TextBox_String
			// 
			this.TextBox_String.Location = new System.Drawing.Point(16, 29);
			this.TextBox_String.Name = "TextBox_String";
			this.TextBox_String.Size = new System.Drawing.Size(563, 20);
			this.TextBox_String.TabIndex = 2;
			// 
			// Label_Caption
			// 
			this.Label_Caption.Location = new System.Drawing.Point(13, 13);
			this.Label_Caption.Name = "Label_Caption";
			this.Label_Caption.Size = new System.Drawing.Size(566, 16);
			this.Label_Caption.TabIndex = 3;
			this.Label_Caption.Text = "Please enter the name of your mod/downloadable content:";
			this.Label_Caption.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// StringQuery
			// 
			this.AcceptButton = button1;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(591, 84);
			this.ControlBox = false;
			this.Controls.Add(this.Label_Caption);
			this.Controls.Add(this.TextBox_String);
			this.Controls.Add(button2);
			this.Controls.Add(button1);
			this.Name = "StringQuery";
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Mod/DLC Name";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		public System.Windows.Forms.TextBox TextBox_String;
		private System.Windows.Forms.Label Label_Caption;
	}
}