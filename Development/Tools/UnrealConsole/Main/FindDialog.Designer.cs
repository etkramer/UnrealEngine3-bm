namespace UnrealConsole
{
	partial class FindDialog
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
			if(TxtBox != null)
			{
				DisconnectEvents();
			}

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
			this.Combo_SearchString = new System.Windows.Forms.ComboBox();
			this.Button_Close = new System.Windows.Forms.Button();
			this.Button_FindNext = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.CheckBox_MatchCase = new System.Windows.Forms.CheckBox();
			this.Group_Direction = new System.Windows.Forms.GroupBox();
			this.Radio_Down = new System.Windows.Forms.RadioButton();
			this.Radio_Up = new System.Windows.Forms.RadioButton();
			this.CheckBox_MatchWord = new System.Windows.Forms.CheckBox();
			this.Group_Direction.SuspendLayout();
			this.SuspendLayout();
			// 
			// Combo_SearchString
			// 
			this.Combo_SearchString.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.Combo_SearchString.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
			this.Combo_SearchString.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.Combo_SearchString.Location = new System.Drawing.Point(77, 6);
			this.Combo_SearchString.Name = "Combo_SearchString";
			this.Combo_SearchString.Size = new System.Drawing.Size(347, 21);
			this.Combo_SearchString.TabIndex = 0;
			// 
			// Button_Close
			// 
			this.Button_Close.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.Button_Close.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.Button_Close.Location = new System.Drawing.Point(349, 54);
			this.Button_Close.Name = "Button_Close";
			this.Button_Close.Size = new System.Drawing.Size(75, 23);
			this.Button_Close.TabIndex = 2;
			this.Button_Close.Text = "&Close";
			this.Button_Close.UseVisualStyleBackColor = true;
			this.Button_Close.Click += new System.EventHandler(this.Button_Close_Click);
			// 
			// Button_FindNext
			// 
			this.Button_FindNext.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.Button_FindNext.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.Button_FindNext.Location = new System.Drawing.Point(268, 54);
			this.Button_FindNext.Name = "Button_FindNext";
			this.Button_FindNext.Size = new System.Drawing.Size(75, 23);
			this.Button_FindNext.TabIndex = 1;
			this.Button_FindNext.Text = "&Find Next";
			this.Button_FindNext.UseVisualStyleBackColor = true;
			this.Button_FindNext.Click += new System.EventHandler(this.Button_FindNext_Click);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(12, 9);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(59, 13);
			this.label1.TabIndex = 3;
			this.label1.Text = "Find What:";
			// 
			// CheckBox_MatchCase
			// 
			this.CheckBox_MatchCase.AutoSize = true;
			this.CheckBox_MatchCase.Location = new System.Drawing.Point(12, 33);
			this.CheckBox_MatchCase.Name = "CheckBox_MatchCase";
			this.CheckBox_MatchCase.Size = new System.Drawing.Size(82, 17);
			this.CheckBox_MatchCase.TabIndex = 3;
			this.CheckBox_MatchCase.Text = "Match case";
			this.CheckBox_MatchCase.UseVisualStyleBackColor = true;
			// 
			// Group_Direction
			// 
			this.Group_Direction.Controls.Add(this.Radio_Down);
			this.Group_Direction.Controls.Add(this.Radio_Up);
			this.Group_Direction.Location = new System.Drawing.Point(142, 33);
			this.Group_Direction.Name = "Group_Direction";
			this.Group_Direction.Size = new System.Drawing.Size(106, 44);
			this.Group_Direction.TabIndex = 6;
			this.Group_Direction.TabStop = false;
			this.Group_Direction.Text = "Direction";
			// 
			// Radio_Down
			// 
			this.Radio_Down.AutoSize = true;
			this.Radio_Down.Checked = true;
			this.Radio_Down.Location = new System.Drawing.Point(51, 19);
			this.Radio_Down.Name = "Radio_Down";
			this.Radio_Down.Size = new System.Drawing.Size(53, 17);
			this.Radio_Down.TabIndex = 6;
			this.Radio_Down.TabStop = true;
			this.Radio_Down.Text = "Down";
			this.Radio_Down.UseVisualStyleBackColor = true;
			// 
			// Radio_Up
			// 
			this.Radio_Up.AutoSize = true;
			this.Radio_Up.Location = new System.Drawing.Point(6, 19);
			this.Radio_Up.Name = "Radio_Up";
			this.Radio_Up.Size = new System.Drawing.Size(39, 17);
			this.Radio_Up.TabIndex = 5;
			this.Radio_Up.TabStop = true;
			this.Radio_Up.Text = "Up";
			this.Radio_Up.UseVisualStyleBackColor = true;
			// 
			// CheckBox_MatchWord
			// 
			this.CheckBox_MatchWord.AutoSize = true;
			this.CheckBox_MatchWord.Location = new System.Drawing.Point(12, 59);
			this.CheckBox_MatchWord.Name = "CheckBox_MatchWord";
			this.CheckBox_MatchWord.Size = new System.Drawing.Size(113, 17);
			this.CheckBox_MatchWord.TabIndex = 4;
			this.CheckBox_MatchWord.Text = "Match whole word";
			this.CheckBox_MatchWord.UseVisualStyleBackColor = true;
			// 
			// FindDialog
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(436, 86);
			this.Controls.Add(this.CheckBox_MatchWord);
			this.Controls.Add(this.Group_Direction);
			this.Controls.Add(this.CheckBox_MatchCase);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.Button_FindNext);
			this.Controls.Add(this.Button_Close);
			this.Controls.Add(this.Combo_SearchString);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.MinimumSize = new System.Drawing.Size(452, 120);
			this.Name = "FindDialog";
			this.ShowInTaskbar = false;
			this.Text = "Find";
			this.Group_Direction.ResumeLayout(false);
			this.Group_Direction.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ComboBox Combo_SearchString;
		private System.Windows.Forms.Button Button_Close;
		private System.Windows.Forms.Button Button_FindNext;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.CheckBox CheckBox_MatchCase;
		private System.Windows.Forms.GroupBox Group_Direction;
		private System.Windows.Forms.RadioButton Radio_Down;
		private System.Windows.Forms.RadioButton Radio_Up;
		private System.Windows.Forms.CheckBox CheckBox_MatchWord;
	}
}