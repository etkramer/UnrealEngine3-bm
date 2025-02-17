namespace UnrealSync.Manager
{
    partial class MgrForm
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
			this.components = new System.ComponentModel.Container();
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MgrForm));
			this.tabControl1 = new System.Windows.Forms.TabControl();
			this.tabJobs = new System.Windows.Forms.TabPage();
			this.syncJobsGrid = new System.Windows.Forms.PropertyGrid();
			this.btnRun = new System.Windows.Forms.Button();
			this.lstbxSyncJobs = new System.Windows.Forms.ListBox();
			this.btnAdd = new System.Windows.Forms.Button();
			this.btnDelete = new System.Windows.Forms.Button();
			this.tabOutput = new System.Windows.Forms.TabPage();
			this.txtJobOutput = new System.Windows.Forms.TextBox();
			this.timerCleanJobOutput = new System.Windows.Forms.Timer(this.components);
			this.menuMain = new System.Windows.Forms.MenuStrip();
			this.menuMain_File = new System.Windows.Forms.ToolStripMenuItem();
			this.menuMain_File_Exit = new System.Windows.Forms.ToolStripMenuItem();
			this.menuMain_Output = new System.Windows.Forms.ToolStripMenuItem();
			this.menuMain_Output_Clear = new System.Windows.Forms.ToolStripMenuItem();
			this.tabControl1.SuspendLayout();
			this.tabJobs.SuspendLayout();
			this.tabOutput.SuspendLayout();
			this.menuMain.SuspendLayout();
			this.SuspendLayout();
			// 
			// tabControl1
			// 
			this.tabControl1.Controls.Add(this.tabJobs);
			this.tabControl1.Controls.Add(this.tabOutput);
			this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabControl1.Location = new System.Drawing.Point(0, 24);
			this.tabControl1.Name = "tabControl1";
			this.tabControl1.SelectedIndex = 0;
			this.tabControl1.Size = new System.Drawing.Size(689, 500);
			this.tabControl1.TabIndex = 0;
			// 
			// tabJobs
			// 
			this.tabJobs.Controls.Add(this.syncJobsGrid);
			this.tabJobs.Controls.Add(this.btnRun);
			this.tabJobs.Controls.Add(this.lstbxSyncJobs);
			this.tabJobs.Controls.Add(this.btnAdd);
			this.tabJobs.Controls.Add(this.btnDelete);
			this.tabJobs.Location = new System.Drawing.Point(4, 22);
			this.tabJobs.Name = "tabJobs";
			this.tabJobs.Padding = new System.Windows.Forms.Padding(3);
			this.tabJobs.Size = new System.Drawing.Size(681, 474);
			this.tabJobs.TabIndex = 0;
			this.tabJobs.Text = "Jobs";
			this.tabJobs.UseVisualStyleBackColor = true;
			// 
			// syncJobsGrid
			// 
			this.syncJobsGrid.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.syncJobsGrid.Location = new System.Drawing.Point(190, 12);
			this.syncJobsGrid.Name = "syncJobsGrid";
			this.syncJobsGrid.Size = new System.Drawing.Size(483, 454);
			this.syncJobsGrid.TabIndex = 97;
			this.syncJobsGrid.PropertyValueChanged += new System.Windows.Forms.PropertyValueChangedEventHandler(this.OnPropertyValueChanged);
			// 
			// btnRun
			// 
			this.btnRun.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.btnRun.Location = new System.Drawing.Point(8, 443);
			this.btnRun.Name = "btnRun";
			this.btnRun.Size = new System.Drawing.Size(55, 23);
			this.btnRun.TabIndex = 94;
			this.btnRun.Text = "Run";
			this.btnRun.UseVisualStyleBackColor = true;
			this.btnRun.Click += new System.EventHandler(this.btnRun_Click);
			// 
			// lstbxSyncJobs
			// 
			this.lstbxSyncJobs.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)));
			this.lstbxSyncJobs.FormattingEnabled = true;
			this.lstbxSyncJobs.Location = new System.Drawing.Point(8, 12);
			this.lstbxSyncJobs.Name = "lstbxSyncJobs";
			this.lstbxSyncJobs.Size = new System.Drawing.Size(176, 420);
			this.lstbxSyncJobs.TabIndex = 93;
			this.lstbxSyncJobs.SelectedIndexChanged += new System.EventHandler(this.lstbxSyncJobs_SelectedIndexChanged);
			// 
			// btnAdd
			// 
			this.btnAdd.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.btnAdd.Location = new System.Drawing.Point(69, 443);
			this.btnAdd.Name = "btnAdd";
			this.btnAdd.Size = new System.Drawing.Size(55, 23);
			this.btnAdd.TabIndex = 95;
			this.btnAdd.Text = "Add";
			this.btnAdd.UseVisualStyleBackColor = true;
			this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
			// 
			// btnDelete
			// 
			this.btnDelete.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.btnDelete.Location = new System.Drawing.Point(129, 443);
			this.btnDelete.Name = "btnDelete";
			this.btnDelete.Size = new System.Drawing.Size(55, 23);
			this.btnDelete.TabIndex = 96;
			this.btnDelete.Text = "Delete";
			this.btnDelete.UseVisualStyleBackColor = true;
			this.btnDelete.Click += new System.EventHandler(this.btnDelete_Click);
			// 
			// tabOutput
			// 
			this.tabOutput.Controls.Add(this.txtJobOutput);
			this.tabOutput.Location = new System.Drawing.Point(4, 22);
			this.tabOutput.Name = "tabOutput";
			this.tabOutput.Padding = new System.Windows.Forms.Padding(3);
			this.tabOutput.Size = new System.Drawing.Size(681, 498);
			this.tabOutput.TabIndex = 1;
			this.tabOutput.Text = "Job Output";
			this.tabOutput.UseVisualStyleBackColor = true;
			// 
			// txtJobOutput
			// 
			this.txtJobOutput.BackColor = System.Drawing.SystemColors.Window;
			this.txtJobOutput.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.txtJobOutput.Dock = System.Windows.Forms.DockStyle.Fill;
			this.txtJobOutput.Location = new System.Drawing.Point(3, 3);
			this.txtJobOutput.Multiline = true;
			this.txtJobOutput.Name = "txtJobOutput";
			this.txtJobOutput.ReadOnly = true;
			this.txtJobOutput.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.txtJobOutput.Size = new System.Drawing.Size(675, 492);
			this.txtJobOutput.TabIndex = 0;
			// 
			// timerCleanJobOutput
			// 
			this.timerCleanJobOutput.Enabled = true;
			this.timerCleanJobOutput.Interval = 3600000;
			this.timerCleanJobOutput.Tick += new System.EventHandler(this.OnCleanUpJobOutput);
			// 
			// menuMain
			// 
			this.menuMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuMain_File,
            this.menuMain_Output});
			this.menuMain.Location = new System.Drawing.Point(0, 0);
			this.menuMain.Name = "menuMain";
			this.menuMain.Size = new System.Drawing.Size(689, 24);
			this.menuMain.TabIndex = 1;
			this.menuMain.Text = "menuMain";
			// 
			// menuMain_File
			// 
			this.menuMain_File.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuMain_File_Exit});
			this.menuMain_File.Name = "menuMain_File";
			this.menuMain_File.Size = new System.Drawing.Size(37, 20);
			this.menuMain_File.Text = "&File";
			// 
			// menuMain_File_Exit
			// 
			this.menuMain_File_Exit.Name = "menuMain_File_Exit";
			this.menuMain_File_Exit.Size = new System.Drawing.Size(152, 22);
			this.menuMain_File_Exit.Text = "E&xit";
			this.menuMain_File_Exit.Click += new System.EventHandler(this.menuMain_Exit_Click);
			// 
			// menuMain_Output
			// 
			this.menuMain_Output.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuMain_Output_Clear});
			this.menuMain_Output.Name = "menuMain_Output";
			this.menuMain_Output.Size = new System.Drawing.Size(57, 20);
			this.menuMain_Output.Text = "&Output";
			// 
			// menuMain_Output_Clear
			// 
			this.menuMain_Output_Clear.Name = "menuMain_Output_Clear";
			this.menuMain_Output_Clear.Size = new System.Drawing.Size(152, 22);
			this.menuMain_Output_Clear.Text = "&Clear";
			this.menuMain_Output_Clear.Click += new System.EventHandler(this.menuMain_Output_Clear_Click);
			// 
			// MgrForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(689, 524);
			this.Controls.Add(this.tabControl1);
			this.Controls.Add(this.menuMain);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MainMenuStrip = this.menuMain;
			this.Name = "MgrForm";
			this.Text = "UnrealSync Manager";
			this.tabControl1.ResumeLayout(false);
			this.tabJobs.ResumeLayout(false);
			this.tabOutput.ResumeLayout(false);
			this.tabOutput.PerformLayout();
			this.menuMain.ResumeLayout(false);
			this.menuMain.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

        }

        #endregion

		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabJobs;
		private System.Windows.Forms.PropertyGrid syncJobsGrid;
		private System.Windows.Forms.Button btnRun;
		private System.Windows.Forms.ListBox lstbxSyncJobs;
		private System.Windows.Forms.Button btnAdd;
		private System.Windows.Forms.Button btnDelete;
		private System.Windows.Forms.TabPage tabOutput;
		private System.Windows.Forms.TextBox txtJobOutput;
		private System.Windows.Forms.Timer timerCleanJobOutput;
		private System.Windows.Forms.MenuStrip menuMain;
		private System.Windows.Forms.ToolStripMenuItem menuMain_File;
		private System.Windows.Forms.ToolStripMenuItem menuMain_File_Exit;
		private System.Windows.Forms.ToolStripMenuItem menuMain_Output;
		private System.Windows.Forms.ToolStripMenuItem menuMain_Output_Clear;

	}
}