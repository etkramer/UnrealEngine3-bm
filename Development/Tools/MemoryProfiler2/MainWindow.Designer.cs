namespace MemoryProfiler2
{
    partial class MainWindow
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainWindow));
            this.CallGraphTreeView = new System.Windows.Forms.TreeView();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportToCSVToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.toolStripLabel1 = new System.Windows.Forms.ToolStripLabel();
            this.DiffStartComboBox = new System.Windows.Forms.ToolStripComboBox();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripLabel2 = new System.Windows.Forms.ToolStripLabel();
            this.DiffEndComboBox = new System.Windows.Forms.ToolStripComboBox();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripLabel3 = new System.Windows.Forms.ToolStripLabel();
            this.SortCriteriaComboBox = new System.Windows.Forms.ToolStripComboBox();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.GoButton = new System.Windows.Forms.ToolStripButton();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.StatusStripLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.ExclusiveSingleCallStackView = new System.Windows.Forms.ListView();
            this.Function = new System.Windows.Forms.ColumnHeader();
            this.File = new System.Windows.Forms.ColumnHeader();
            this.Line = new System.Windows.Forms.ColumnHeader();
            this.ExclusiveListView = new System.Windows.Forms.ListView();
            this.SizeInKByte = new System.Windows.Forms.ColumnHeader();
            this.SizePercentage = new System.Windows.Forms.ColumnHeader();
            this.Count = new System.Windows.Forms.ColumnHeader();
            this.CountPercentage = new System.Windows.Forms.ColumnHeader();
            this.toolStripLabel4 = new System.Windows.Forms.ToolStripLabel();
            this.toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
            this.AllocationTypeComboBox = new System.Windows.Forms.ToolStripComboBox();
            this.menuStrip1.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.SuspendLayout();
            // 
            // CallGraphTreeView
            // 
            this.CallGraphTreeView.BorderStyle = System.Windows.Forms.BorderStyle.None;
            resources.ApplyResources(this.CallGraphTreeView, "CallGraphTreeView");
            this.CallGraphTreeView.Name = "CallGraphTreeView";
            this.CallGraphTreeView.NodeMouseClick += new System.Windows.Forms.TreeNodeMouseClickEventHandler(this.CallGraphTreeView_NodeMouseClick);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            resources.ApplyResources(this.menuStrip1, "menuStrip1");
            this.menuStrip1.Name = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openToolStripMenuItem,
            this.exportToCSVToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            resources.ApplyResources(this.fileToolStripMenuItem, "fileToolStripMenuItem");
            // 
            // openToolStripMenuItem
            // 
            this.openToolStripMenuItem.Name = "openToolStripMenuItem";
            resources.ApplyResources(this.openToolStripMenuItem, "openToolStripMenuItem");
            this.openToolStripMenuItem.Click += new System.EventHandler(this.openToolStripMenuItem_Click);
            // 
            // exportToCSVToolStripMenuItem
            // 
            this.exportToCSVToolStripMenuItem.Name = "exportToCSVToolStripMenuItem";
            resources.ApplyResources(this.exportToCSVToolStripMenuItem, "exportToCSVToolStripMenuItem");
            this.exportToCSVToolStripMenuItem.Click += new System.EventHandler(this.exportToCSVToolStripMenuItem_Click);
            // 
            // toolStrip1
            // 
            resources.ApplyResources(this.toolStrip1, "toolStrip1");
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripLabel1,
            this.DiffStartComboBox,
            this.toolStripSeparator1,
            this.toolStripLabel2,
            this.DiffEndComboBox,
            this.toolStripSeparator2,
            this.toolStripLabel3,
            this.SortCriteriaComboBox,
            this.toolStripSeparator3,
            this.toolStripLabel4,
            this.AllocationTypeComboBox,
            this.toolStripSeparator4,
            this.GoButton});
            this.toolStrip1.Name = "toolStrip1";
            // 
            // toolStripLabel1
            // 
            this.toolStripLabel1.Name = "toolStripLabel1";
            resources.ApplyResources(this.toolStripLabel1, "toolStripLabel1");
            // 
            // DiffStartComboBox
            // 
            this.DiffStartComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            resources.ApplyResources(this.DiffStartComboBox, "DiffStartComboBox");
            this.DiffStartComboBox.Name = "DiffStartComboBox";
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            resources.ApplyResources(this.toolStripSeparator1, "toolStripSeparator1");
            // 
            // toolStripLabel2
            // 
            this.toolStripLabel2.Name = "toolStripLabel2";
            resources.ApplyResources(this.toolStripLabel2, "toolStripLabel2");
            // 
            // DiffEndComboBox
            // 
            this.DiffEndComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            resources.ApplyResources(this.DiffEndComboBox, "DiffEndComboBox");
            this.DiffEndComboBox.Name = "DiffEndComboBox";
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            resources.ApplyResources(this.toolStripSeparator2, "toolStripSeparator2");
            // 
            // toolStripLabel3
            // 
            this.toolStripLabel3.Name = "toolStripLabel3";
            resources.ApplyResources(this.toolStripLabel3, "toolStripLabel3");
            // 
            // SortCriteriaComboBox
            // 
            this.SortCriteriaComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            resources.ApplyResources(this.SortCriteriaComboBox, "SortCriteriaComboBox");
            this.SortCriteriaComboBox.Items.AddRange(new object[] {
            resources.GetString("SortCriteriaComboBox.Items"),
            resources.GetString("SortCriteriaComboBox.Items1")});
            this.SortCriteriaComboBox.Name = "SortCriteriaComboBox";
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            resources.ApplyResources(this.toolStripSeparator3, "toolStripSeparator3");
            // 
            // GoButton
            // 
            this.GoButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            resources.ApplyResources(this.GoButton, "GoButton");
            this.GoButton.Name = "GoButton";
            this.GoButton.Click += new System.EventHandler(this.GoButton_Click);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.StatusStripLabel});
            resources.ApplyResources(this.statusStrip1, "statusStrip1");
            this.statusStrip1.Name = "statusStrip1";
            // 
            // StatusStripLabel
            // 
            this.StatusStripLabel.Name = "StatusStripLabel";
            resources.ApplyResources(this.StatusStripLabel, "StatusStripLabel");
            // 
            // tabControl1
            // 
            resources.ApplyResources(this.tabControl1, "tabControl1");
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.CallGraphTreeView);
            resources.ApplyResources(this.tabPage1, "tabPage1");
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.ExclusiveSingleCallStackView);
            this.tabPage2.Controls.Add(this.ExclusiveListView);
            resources.ApplyResources(this.tabPage2, "tabPage2");
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // ExclusiveSingleCallStackView
            // 
            resources.ApplyResources(this.ExclusiveSingleCallStackView, "ExclusiveSingleCallStackView");
            this.ExclusiveSingleCallStackView.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.ExclusiveSingleCallStackView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.Function,
            this.File,
            this.Line});
            this.ExclusiveSingleCallStackView.FullRowSelect = true;
            this.ExclusiveSingleCallStackView.GridLines = true;
            this.ExclusiveSingleCallStackView.Name = "ExclusiveSingleCallStackView";
            this.ExclusiveSingleCallStackView.UseCompatibleStateImageBehavior = false;
            this.ExclusiveSingleCallStackView.View = System.Windows.Forms.View.Details;
            // 
            // Function
            // 
            resources.ApplyResources(this.Function, "Function");
            // 
            // File
            // 
            resources.ApplyResources(this.File, "File");
            // 
            // Line
            // 
            resources.ApplyResources(this.Line, "Line");
            // 
            // ExclusiveListView
            // 
            resources.ApplyResources(this.ExclusiveListView, "ExclusiveListView");
            this.ExclusiveListView.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.ExclusiveListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.SizeInKByte,
            this.SizePercentage,
            this.Count,
            this.CountPercentage});
            this.ExclusiveListView.FullRowSelect = true;
            this.ExclusiveListView.GridLines = true;
            this.ExclusiveListView.Name = "ExclusiveListView";
            this.ExclusiveListView.UseCompatibleStateImageBehavior = false;
            this.ExclusiveListView.View = System.Windows.Forms.View.Details;
            this.ExclusiveListView.SelectedIndexChanged += new System.EventHandler(this.ExclusiveListView_SelectedIndexChanged);
            // 
            // SizeInKByte
            // 
            resources.ApplyResources(this.SizeInKByte, "SizeInKByte");
            // 
            // SizePercentage
            // 
            resources.ApplyResources(this.SizePercentage, "SizePercentage");
            // 
            // Count
            // 
            resources.ApplyResources(this.Count, "Count");
            // 
            // CountPercentage
            // 
            resources.ApplyResources(this.CountPercentage, "CountPercentage");
            // 
            // toolStripLabel4
            // 
            this.toolStripLabel4.Name = "toolStripLabel4";
            resources.ApplyResources(this.toolStripLabel4, "toolStripLabel4");
            // 
            // toolStripSeparator4
            // 
            this.toolStripSeparator4.Name = "toolStripSeparator4";
            resources.ApplyResources(this.toolStripSeparator4, "toolStripSeparator4");
            // 
            // AllocationTypeComboBox
            // 
            this.AllocationTypeComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            resources.ApplyResources(this.AllocationTypeComboBox, "AllocationTypeComboBox");
            this.AllocationTypeComboBox.Items.AddRange(new object[] {
            resources.GetString("AllocationTypeComboBox.Items"),
            resources.GetString("AllocationTypeComboBox.Items1")});
            this.AllocationTypeComboBox.Name = "AllocationTypeComboBox";
            // 
            // MainWindow
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.menuStrip1);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.toolStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "MainWindow";
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

		private System.Windows.Forms.TreeView CallGraphTreeView;
		private System.Windows.Forms.MenuStrip menuStrip1;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem exportToCSVToolStripMenuItem;
		private System.Windows.Forms.ToolStrip toolStrip1;
		private System.Windows.Forms.ToolStripLabel toolStripLabel1;
		private System.Windows.Forms.ToolStripComboBox DiffStartComboBox;
		private System.Windows.Forms.ToolStripLabel toolStripLabel2;
		private System.Windows.Forms.ToolStripComboBox DiffEndComboBox;
		private System.Windows.Forms.ToolStripButton GoButton;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
		private System.Windows.Forms.StatusStrip statusStrip1;
		private System.Windows.Forms.ToolStripStatusLabel StatusStripLabel;
		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TabPage tabPage2;
		private System.Windows.Forms.ListView ExclusiveListView;
		private System.Windows.Forms.ColumnHeader SizeInKByte;
		private System.Windows.Forms.ColumnHeader Count;
		private System.Windows.Forms.ColumnHeader SizePercentage;
		private System.Windows.Forms.ColumnHeader CountPercentage;
		private System.Windows.Forms.ListView ExclusiveSingleCallStackView;
		private System.Windows.Forms.ColumnHeader Function;
		private System.Windows.Forms.ColumnHeader File;
		private System.Windows.Forms.ColumnHeader Line;
		private System.Windows.Forms.ToolStripLabel toolStripLabel3;
		private System.Windows.Forms.ToolStripComboBox SortCriteriaComboBox;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripLabel toolStripLabel4;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
        private System.Windows.Forms.ToolStripComboBox AllocationTypeComboBox;
    }
}

