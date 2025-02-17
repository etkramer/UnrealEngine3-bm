namespace UnrealFrontend
{
	partial class ModSelector
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
			this.RadioButton_ModMap = new System.Windows.Forms.RadioButton();
			this.RadioButton_ModMutator = new System.Windows.Forms.RadioButton();
			this.RadioButton_ModGameType = new System.Windows.Forms.RadioButton();
			this.RadioButton_ModTC = new System.Windows.Forms.RadioButton();
			this.RadioButton_ModCharacter = new System.Windows.Forms.RadioButton();
			this.RadioButton_ModNone = new System.Windows.Forms.RadioButton();
			this.Button_OK = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(13, 13);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(308, 28);
			this.label1.TabIndex = 0;
			this.label1.Text = "It looks like you have made a new mod. Select the type of mod below to use a temp" +
				"late .ini to enable the mod in-game:";
			// 
			// RadioButton_ModMap
			// 
			this.RadioButton_ModMap.AutoSize = true;
			this.RadioButton_ModMap.Location = new System.Drawing.Point(41, 44);
			this.RadioButton_ModMap.Name = "RadioButton_ModMap";
			this.RadioButton_ModMap.Size = new System.Drawing.Size(46, 17);
			this.RadioButton_ModMap.TabIndex = 1;
			this.RadioButton_ModMap.TabStop = true;
			this.RadioButton_ModMap.Tag = "Map";
			this.RadioButton_ModMap.Text = "Map";
			this.RadioButton_ModMap.UseVisualStyleBackColor = true;
			// 
			// RadioButton_ModMutator
			// 
			this.RadioButton_ModMutator.AutoSize = true;
			this.RadioButton_ModMutator.Location = new System.Drawing.Point(41, 67);
			this.RadioButton_ModMutator.Name = "RadioButton_ModMutator";
			this.RadioButton_ModMutator.Size = new System.Drawing.Size(61, 17);
			this.RadioButton_ModMutator.TabIndex = 2;
			this.RadioButton_ModMutator.TabStop = true;
			this.RadioButton_ModMutator.Tag = "Mutator";
			this.RadioButton_ModMutator.Text = "Mutator";
			this.RadioButton_ModMutator.UseVisualStyleBackColor = true;
			// 
			// RadioButton_ModGameType
			// 
			this.RadioButton_ModGameType.AutoSize = true;
			this.RadioButton_ModGameType.Location = new System.Drawing.Point(41, 90);
			this.RadioButton_ModGameType.Name = "RadioButton_ModGameType";
			this.RadioButton_ModGameType.Size = new System.Drawing.Size(80, 17);
			this.RadioButton_ModGameType.TabIndex = 4;
			this.RadioButton_ModGameType.TabStop = true;
			this.RadioButton_ModGameType.Tag = "GameType";
			this.RadioButton_ModGameType.Text = "Game Type";
			this.RadioButton_ModGameType.UseVisualStyleBackColor = true;
			// 
			// RadioButton_ModTC
			// 
			this.RadioButton_ModTC.AutoSize = true;
			this.RadioButton_ModTC.Location = new System.Drawing.Point(41, 136);
			this.RadioButton_ModTC.Name = "RadioButton_ModTC";
			this.RadioButton_ModTC.Size = new System.Drawing.Size(105, 17);
			this.RadioButton_ModTC.TabIndex = 5;
			this.RadioButton_ModTC.TabStop = true;
			this.RadioButton_ModTC.Tag = "TotalConversion";
			this.RadioButton_ModTC.Text = "Total Conversion";
			this.RadioButton_ModTC.UseVisualStyleBackColor = true;
			// 
			// RadioButton_ModCharacter
			// 
			this.RadioButton_ModCharacter.AutoSize = true;
			this.RadioButton_ModCharacter.Location = new System.Drawing.Point(41, 113);
			this.RadioButton_ModCharacter.Name = "RadioButton_ModCharacter";
			this.RadioButton_ModCharacter.Size = new System.Drawing.Size(106, 17);
			this.RadioButton_ModCharacter.TabIndex = 6;
			this.RadioButton_ModCharacter.TabStop = true;
			this.RadioButton_ModCharacter.Tag = "CharacterPieces";
			this.RadioButton_ModCharacter.Text = "Character Pieces";
			this.RadioButton_ModCharacter.UseVisualStyleBackColor = true;
			// 
			// RadioButton_ModNone
			// 
			this.RadioButton_ModNone.AutoSize = true;
			this.RadioButton_ModNone.Location = new System.Drawing.Point(41, 159);
			this.RadioButton_ModNone.Name = "RadioButton_ModNone";
			this.RadioButton_ModNone.Size = new System.Drawing.Size(182, 17);
			this.RadioButton_ModNone.TabIndex = 7;
			this.RadioButton_ModNone.TabStop = true;
			this.RadioButton_ModNone.Text = "None of the above / No template";
			this.RadioButton_ModNone.UseVisualStyleBackColor = true;
			// 
			// Button_OK
			// 
			this.Button_OK.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.Button_OK.Location = new System.Drawing.Point(129, 198);
			this.Button_OK.Name = "Button_OK";
			this.Button_OK.Size = new System.Drawing.Size(75, 23);
			this.Button_OK.TabIndex = 8;
			this.Button_OK.Text = "OK";
			this.Button_OK.UseVisualStyleBackColor = true;
			// 
			// ModSelector
			// 
			this.AcceptButton = this.Button_OK;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(333, 233);
			this.ControlBox = false;
			this.Controls.Add(this.Button_OK);
			this.Controls.Add(this.RadioButton_ModNone);
			this.Controls.Add(this.RadioButton_ModCharacter);
			this.Controls.Add(this.RadioButton_ModTC);
			this.Controls.Add(this.RadioButton_ModGameType);
			this.Controls.Add(this.RadioButton_ModMutator);
			this.Controls.Add(this.RadioButton_ModMap);
			this.Controls.Add(this.label1);
			this.Name = "ModSelector";
			this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "New Mod Assistant";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.RadioButton RadioButton_ModMap;
		private System.Windows.Forms.RadioButton RadioButton_ModMutator;
		private System.Windows.Forms.RadioButton RadioButton_ModGameType;
		private System.Windows.Forms.RadioButton RadioButton_ModTC;
		private System.Windows.Forms.RadioButton RadioButton_ModCharacter;
		private System.Windows.Forms.Button Button_OK;
		public System.Windows.Forms.RadioButton RadioButton_ModNone;
	}
}