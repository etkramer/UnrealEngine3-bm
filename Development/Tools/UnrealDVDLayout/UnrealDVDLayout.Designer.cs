namespace UnrealDVDLayout
{
    partial class UnrealDVDLayout
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose( bool disposing )
        {
            if( disposing && ( components != null ) )
            {
                components.Dispose();
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
            this.components = new System.ComponentModel.Container();
            this.MainMenu = new System.Windows.Forms.MenuStrip();
            this.FileMainMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ImportTOCsFileMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.SaveXGDMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.RecentMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.Recent0MenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.Recent1MenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.Recent2MenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.Recent3MenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.Recent4MenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.QuitFileMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolsMainMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.OptionsToolsMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.MainToolStrip = new System.Windows.Forms.ToolStrip();
            this.GameToolStripComboBox = new System.Windows.Forms.ToolStripComboBox();
            this.ImportTOCsToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.SaveXGDToolStripButton = new System.Windows.Forms.ToolStripButton();
            this.GenericOpenFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.MainLogWindow = new System.Windows.Forms.RichTextBox();
            this.GenericSaveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.FileListView = new System.Windows.Forms.ListView();
            this.FileListViewPathHeader = new System.Windows.Forms.ColumnHeader();
            this.FileListViewMenuStrip = new System.Windows.Forms.ContextMenuStrip( this.components );
            this.NewGroupToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.AddToGroupToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.MakeFromRegExpMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.GroupListView = new System.Windows.Forms.ListView();
            this.ListViewNameHeader = new System.Windows.Forms.ColumnHeader();
            this.ListViewSizeHeader = new System.Windows.Forms.ColumnHeader();
            this.GroupListViewMenuStrip = new System.Windows.Forms.ContextMenuStrip( this.components );
            this.AddToLayer0MenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.AddToLayer1MenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.removeFromDiscToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.Layer0ListView = new System.Windows.Forms.ListView();
            this.Layer0GroupHeader = new System.Windows.Forms.ColumnHeader();
            this.Layer0SizeHeader = new System.Windows.Forms.ColumnHeader();
            this.Layer1ListView = new System.Windows.Forms.ListView();
            this.Layer1GroupHeader = new System.Windows.Forms.ColumnHeader();
            this.Layer1SizeHeader = new System.Windows.Forms.ColumnHeader();
            this.Layer0MoveUpButton = new System.Windows.Forms.Button();
            this.Layer0MoveDownButton = new System.Windows.Forms.Button();
            this.Layer1MoveDownButton = new System.Windows.Forms.Button();
            this.Layer1MoveUpButton = new System.Windows.Forms.Button();
            this.Layer0Label = new System.Windows.Forms.Label();
            this.Layer1Label = new System.Windows.Forms.Label();
            this.GroupMoveUpButton = new System.Windows.Forms.Button();
            this.GroupMoveDownButton = new System.Windows.Forms.Button();
            this.MainMenu.SuspendLayout();
            this.MainToolStrip.SuspendLayout();
            this.FileListViewMenuStrip.SuspendLayout();
            this.GroupListViewMenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // MainMenu
            // 
            this.MainMenu.Items.AddRange( new System.Windows.Forms.ToolStripItem[] {
            this.FileMainMenuItem,
            this.ToolsMainMenuItem} );
            this.MainMenu.Location = new System.Drawing.Point( 0, 0 );
            this.MainMenu.Name = "MainMenu";
            this.MainMenu.Size = new System.Drawing.Size( 1366, 24 );
            this.MainMenu.TabIndex = 0;
            this.MainMenu.Text = "menuStrip1";
            // 
            // FileMainMenuItem
            // 
            this.FileMainMenuItem.DropDownItems.AddRange( new System.Windows.Forms.ToolStripItem[] {
            this.ImportTOCsFileMenuItem,
            this.SaveXGDMenuItem,
            this.toolStripSeparator1,
            this.RecentMenuItem,
            this.toolStripSeparator2,
            this.QuitFileMenuItem} );
            this.FileMainMenuItem.Name = "FileMainMenuItem";
            this.FileMainMenuItem.Size = new System.Drawing.Size( 37, 20 );
            this.FileMainMenuItem.Text = "File";
            // 
            // ImportTOCsFileMenuItem
            // 
            this.ImportTOCsFileMenuItem.Name = "ImportTOCsFileMenuItem";
            this.ImportTOCsFileMenuItem.Size = new System.Drawing.Size( 142, 22 );
            this.ImportTOCsFileMenuItem.Text = "Import TOCs";
            this.ImportTOCsFileMenuItem.Click += new System.EventHandler( this.ImportTOCMenu_Click );
            // 
            // SaveXGDMenuItem
            // 
            this.SaveXGDMenuItem.Name = "SaveXGDMenuItem";
            this.SaveXGDMenuItem.Size = new System.Drawing.Size( 142, 22 );
            this.SaveXGDMenuItem.Text = "Save XGDs";
            this.SaveXGDMenuItem.Click += new System.EventHandler( this.SaveXGDMenu_Click );
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size( 139, 6 );
            // 
            // RecentMenuItem
            // 
            this.RecentMenuItem.DropDownItems.AddRange( new System.Windows.Forms.ToolStripItem[] {
            this.Recent0MenuItem,
            this.Recent1MenuItem,
            this.Recent2MenuItem,
            this.Recent3MenuItem,
            this.Recent4MenuItem} );
            this.RecentMenuItem.Name = "RecentMenuItem";
            this.RecentMenuItem.Size = new System.Drawing.Size( 142, 22 );
            this.RecentMenuItem.Text = "Recent";
            this.RecentMenuItem.ToolTipText = "Recently loaded files";
            // 
            // Recent0MenuItem
            // 
            this.Recent0MenuItem.Name = "Recent0MenuItem";
            this.Recent0MenuItem.Size = new System.Drawing.Size( 67, 22 );
            this.Recent0MenuItem.Click += new System.EventHandler( this.RecentMenuItem_Click );
            // 
            // Recent1MenuItem
            // 
            this.Recent1MenuItem.Name = "Recent1MenuItem";
            this.Recent1MenuItem.Size = new System.Drawing.Size( 67, 22 );
            this.Recent1MenuItem.Click += new System.EventHandler( this.RecentMenuItem_Click );
            // 
            // Recent2MenuItem
            // 
            this.Recent2MenuItem.Name = "Recent2MenuItem";
            this.Recent2MenuItem.Size = new System.Drawing.Size( 67, 22 );
            this.Recent2MenuItem.Click += new System.EventHandler( this.RecentMenuItem_Click );
            // 
            // Recent3MenuItem
            // 
            this.Recent3MenuItem.Name = "Recent3MenuItem";
            this.Recent3MenuItem.Size = new System.Drawing.Size( 67, 22 );
            this.Recent3MenuItem.Click += new System.EventHandler( this.RecentMenuItem_Click );
            // 
            // Recent4MenuItem
            // 
            this.Recent4MenuItem.Name = "Recent4MenuItem";
            this.Recent4MenuItem.Size = new System.Drawing.Size( 67, 22 );
            this.Recent4MenuItem.Click += new System.EventHandler( this.RecentMenuItem_Click );
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size( 139, 6 );
            // 
            // QuitFileMenuItem
            // 
            this.QuitFileMenuItem.Name = "QuitFileMenuItem";
            this.QuitFileMenuItem.Size = new System.Drawing.Size( 142, 22 );
            this.QuitFileMenuItem.Text = "Quit";
            this.QuitFileMenuItem.Click += new System.EventHandler( this.QuitMenuItem_Click );
            // 
            // ToolsMainMenuItem
            // 
            this.ToolsMainMenuItem.DropDownItems.AddRange( new System.Windows.Forms.ToolStripItem[] {
            this.OptionsToolsMenuItem} );
            this.ToolsMainMenuItem.Name = "ToolsMainMenuItem";
            this.ToolsMainMenuItem.Size = new System.Drawing.Size( 48, 20 );
            this.ToolsMainMenuItem.Text = "Tools";
            // 
            // OptionsToolsMenuItem
            // 
            this.OptionsToolsMenuItem.Name = "OptionsToolsMenuItem";
            this.OptionsToolsMenuItem.Size = new System.Drawing.Size( 116, 22 );
            this.OptionsToolsMenuItem.Text = "Options";
            this.OptionsToolsMenuItem.Click += new System.EventHandler( this.OptionsMenuItem_Click );
            // 
            // MainToolStrip
            // 
            this.MainToolStrip.Items.AddRange( new System.Windows.Forms.ToolStripItem[] {
            this.GameToolStripComboBox,
            this.ImportTOCsToolStripButton,
            this.SaveXGDToolStripButton} );
            this.MainToolStrip.Location = new System.Drawing.Point( 0, 24 );
            this.MainToolStrip.Name = "MainToolStrip";
            this.MainToolStrip.Size = new System.Drawing.Size( 1366, 25 );
            this.MainToolStrip.TabIndex = 1;
            // 
            // GameToolStripComboBox
            // 
            this.GameToolStripComboBox.Items.AddRange( new object[] {
            "Example",
            "Gear",
            "UT"} );
            this.GameToolStripComboBox.Name = "GameToolStripComboBox";
            this.GameToolStripComboBox.Size = new System.Drawing.Size( 121, 25 );
            this.GameToolStripComboBox.ToolTipText = "Game to make layout for";
            this.GameToolStripComboBox.SelectedIndexChanged += new System.EventHandler( this.GameToolStripComboBox_Changed );
            // 
            // ImportTOCsToolStripButton
            // 
            this.ImportTOCsToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.ImportTOCsToolStripButton.Image = global::UnrealDVDLayout.Properties.Resources.Btn_FolderOpen;
            this.ImportTOCsToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.ImportTOCsToolStripButton.Name = "ImportTOCsToolStripButton";
            this.ImportTOCsToolStripButton.Size = new System.Drawing.Size( 23, 22 );
            this.ImportTOCsToolStripButton.ToolTipText = "Import TOC Files";
            this.ImportTOCsToolStripButton.Click += new System.EventHandler( this.ImportTOCMenu_Click );
            // 
            // SaveXGDToolStripButton
            // 
            this.SaveXGDToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.SaveXGDToolStripButton.Image = global::UnrealDVDLayout.Properties.Resources.Save;
            this.SaveXGDToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.SaveXGDToolStripButton.Name = "SaveXGDToolStripButton";
            this.SaveXGDToolStripButton.Size = new System.Drawing.Size( 23, 22 );
            this.SaveXGDToolStripButton.ToolTipText = "Save XGD File";
            this.SaveXGDToolStripButton.Click += new System.EventHandler( this.SaveXGDMenu_Click );
            // 
            // GenericOpenFileDialog
            // 
            this.GenericOpenFileDialog.DefaultExt = "*.txt";
            this.GenericOpenFileDialog.FileName = "Xbox360TOC.txt";
            this.GenericOpenFileDialog.Filter = "TOC Files (*.txt)|*.txt|XGD Files (*.xgd)|*.xgd";
            this.GenericOpenFileDialog.Multiselect = true;
            this.GenericOpenFileDialog.ReadOnlyChecked = true;
            this.GenericOpenFileDialog.RestoreDirectory = true;
            this.GenericOpenFileDialog.ShowHelp = true;
            this.GenericOpenFileDialog.SupportMultiDottedExtensions = true;
            // 
            // MainLogWindow
            // 
            this.MainLogWindow.Anchor = ( ( System.Windows.Forms.AnchorStyles )( ( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom )
                        | System.Windows.Forms.AnchorStyles.Left )
                        | System.Windows.Forms.AnchorStyles.Right ) ) );
            this.MainLogWindow.Font = new System.Drawing.Font( "Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ( ( byte )( 0 ) ) );
            this.MainLogWindow.Location = new System.Drawing.Point( 12, 891 );
            this.MainLogWindow.Name = "MainLogWindow";
            this.MainLogWindow.ReadOnly = true;
            this.MainLogWindow.Size = new System.Drawing.Size( 1342, 312 );
            this.MainLogWindow.TabIndex = 2;
            this.MainLogWindow.Text = "";
            // 
            // GenericSaveFileDialog
            // 
            this.GenericSaveFileDialog.DefaultExt = "*.XGD";
            this.GenericSaveFileDialog.Filter = "XGD files (*.XGD)|*.XGD";
            this.GenericSaveFileDialog.RestoreDirectory = true;
            this.GenericSaveFileDialog.ShowHelp = true;
            this.GenericSaveFileDialog.SupportMultiDottedExtensions = true;
            // 
            // FileListView
            // 
            this.FileListView.Columns.AddRange( new System.Windows.Forms.ColumnHeader[] {
            this.FileListViewPathHeader} );
            this.FileListView.ContextMenuStrip = this.FileListViewMenuStrip;
            this.FileListView.HideSelection = false;
            this.FileListView.Location = new System.Drawing.Point( 12, 52 );
            this.FileListView.Name = "FileListView";
            this.FileListView.Size = new System.Drawing.Size( 350, 833 );
            this.FileListView.TabIndex = 3;
            this.FileListView.UseCompatibleStateImageBehavior = false;
            this.FileListView.View = System.Windows.Forms.View.Details;
            // 
            // FileListViewPathHeader
            // 
            this.FileListViewPathHeader.Text = "File path";
            this.FileListViewPathHeader.Width = 344;
            // 
            // FileListViewMenuStrip
            // 
            this.FileListViewMenuStrip.Items.AddRange( new System.Windows.Forms.ToolStripItem[] {
            this.NewGroupToolStripMenuItem,
            this.AddToGroupToolStripMenuItem,
            this.MakeFromRegExpMenuItem} );
            this.FileListViewMenuStrip.Name = "FileListBoxMenuStrip";
            this.FileListViewMenuStrip.Size = new System.Drawing.Size( 174, 70 );
            this.FileListViewMenuStrip.Opening += new System.ComponentModel.CancelEventHandler( this.FileListBoxMenuStrip_Opening );
            // 
            // NewGroupToolStripMenuItem
            // 
            this.NewGroupToolStripMenuItem.Name = "NewGroupToolStripMenuItem";
            this.NewGroupToolStripMenuItem.Size = new System.Drawing.Size( 173, 22 );
            this.NewGroupToolStripMenuItem.Text = "New Group";
            // 
            // AddToGroupToolStripMenuItem
            // 
            this.AddToGroupToolStripMenuItem.DoubleClickEnabled = true;
            this.AddToGroupToolStripMenuItem.Name = "AddToGroupToolStripMenuItem";
            this.AddToGroupToolStripMenuItem.Size = new System.Drawing.Size( 173, 22 );
            this.AddToGroupToolStripMenuItem.Text = "Add to Group";
            // 
            // MakeFromRegExpMenuItem
            // 
            this.MakeFromRegExpMenuItem.Name = "MakeFromRegExpMenuItem";
            this.MakeFromRegExpMenuItem.Size = new System.Drawing.Size( 173, 22 );
            this.MakeFromRegExpMenuItem.Text = "Make from RegExp";
            this.MakeFromRegExpMenuItem.Click += new System.EventHandler( this.MakeFromRegExpMenuItem_Click );
            // 
            // GroupListView
            // 
            this.GroupListView.Columns.AddRange( new System.Windows.Forms.ColumnHeader[] {
            this.ListViewNameHeader,
            this.ListViewSizeHeader} );
            this.GroupListView.ContextMenuStrip = this.GroupListViewMenuStrip;
            this.GroupListView.FullRowSelect = true;
            this.GroupListView.HideSelection = false;
            this.GroupListView.Location = new System.Drawing.Point( 392, 52 );
            this.GroupListView.Name = "GroupListView";
            this.GroupListView.Size = new System.Drawing.Size( 250, 833 );
            this.GroupListView.TabIndex = 4;
            this.GroupListView.UseCompatibleStateImageBehavior = false;
            this.GroupListView.View = System.Windows.Forms.View.Details;
            this.GroupListView.DoubleClick += new System.EventHandler( this.GroupListView_DoubleClick );
            this.GroupListView.KeyDown += new System.Windows.Forms.KeyEventHandler( this.GroupListView_KeyPress );
            // 
            // ListViewNameHeader
            // 
            this.ListViewNameHeader.Text = "Group Name";
            this.ListViewNameHeader.Width = 176;
            // 
            // ListViewSizeHeader
            // 
            this.ListViewSizeHeader.Text = "Size (MB)";
            this.ListViewSizeHeader.Width = 70;
            // 
            // GroupListViewMenuStrip
            // 
            this.GroupListViewMenuStrip.Items.AddRange( new System.Windows.Forms.ToolStripItem[] {
            this.AddToLayer0MenuItem,
            this.AddToLayer1MenuItem,
            this.removeFromDiscToolStripMenuItem} );
            this.GroupListViewMenuStrip.Name = "GroupListViewMenuStrip";
            this.GroupListViewMenuStrip.Size = new System.Drawing.Size( 172, 70 );
            // 
            // AddToLayer0MenuItem
            // 
            this.AddToLayer0MenuItem.Name = "AddToLayer0MenuItem";
            this.AddToLayer0MenuItem.Size = new System.Drawing.Size( 171, 22 );
            this.AddToLayer0MenuItem.Text = "Add to Layer 0";
            this.AddToLayer0MenuItem.Click += new System.EventHandler( this.AddToLayer0MenuItem_Click );
            // 
            // AddToLayer1MenuItem
            // 
            this.AddToLayer1MenuItem.Name = "AddToLayer1MenuItem";
            this.AddToLayer1MenuItem.Size = new System.Drawing.Size( 171, 22 );
            this.AddToLayer1MenuItem.Text = "Add to Layer 1";
            this.AddToLayer1MenuItem.Click += new System.EventHandler( this.AddToLayer1MenuItem_Click );
            // 
            // removeFromDiscToolStripMenuItem
            // 
            this.removeFromDiscToolStripMenuItem.Name = "removeFromDiscToolStripMenuItem";
            this.removeFromDiscToolStripMenuItem.Size = new System.Drawing.Size( 171, 22 );
            this.removeFromDiscToolStripMenuItem.Text = "Remove from Disc";
            this.removeFromDiscToolStripMenuItem.Click += new System.EventHandler( this.RemoveFromDiscMenuItem_Click );
            // 
            // Layer0ListView
            // 
            this.Layer0ListView.Columns.AddRange( new System.Windows.Forms.ColumnHeader[] {
            this.Layer0GroupHeader,
            this.Layer0SizeHeader} );
            this.Layer0ListView.HideSelection = false;
            this.Layer0ListView.Location = new System.Drawing.Point( 726, 136 );
            this.Layer0ListView.Name = "Layer0ListView";
            this.Layer0ListView.Size = new System.Drawing.Size( 250, 689 );
            this.Layer0ListView.TabIndex = 5;
            this.Layer0ListView.UseCompatibleStateImageBehavior = false;
            this.Layer0ListView.View = System.Windows.Forms.View.Details;
            this.Layer0ListView.KeyDown += new System.Windows.Forms.KeyEventHandler( this.Layer0ListView_KeyPress );
            // 
            // Layer0GroupHeader
            // 
            this.Layer0GroupHeader.Text = "Group Name";
            this.Layer0GroupHeader.Width = 186;
            // 
            // Layer0SizeHeader
            // 
            this.Layer0SizeHeader.Text = "Size (MB)";
            // 
            // Layer1ListView
            // 
            this.Layer1ListView.Columns.AddRange( new System.Windows.Forms.ColumnHeader[] {
            this.Layer1GroupHeader,
            this.Layer1SizeHeader} );
            this.Layer1ListView.HideSelection = false;
            this.Layer1ListView.Location = new System.Drawing.Point( 1049, 136 );
            this.Layer1ListView.Name = "Layer1ListView";
            this.Layer1ListView.Size = new System.Drawing.Size( 250, 689 );
            this.Layer1ListView.TabIndex = 6;
            this.Layer1ListView.UseCompatibleStateImageBehavior = false;
            this.Layer1ListView.View = System.Windows.Forms.View.Details;
            // 
            // Layer1GroupHeader
            // 
            this.Layer1GroupHeader.Text = "Group Name";
            this.Layer1GroupHeader.Width = 186;
            // 
            // Layer1SizeHeader
            // 
            this.Layer1SizeHeader.Text = "Size (MB)";
            // 
            // Layer0MoveUpButton
            // 
            this.Layer0MoveUpButton.Location = new System.Drawing.Point( 982, 430 );
            this.Layer0MoveUpButton.Name = "Layer0MoveUpButton";
            this.Layer0MoveUpButton.Size = new System.Drawing.Size( 50, 40 );
            this.Layer0MoveUpButton.TabIndex = 7;
            this.Layer0MoveUpButton.Text = "Up";
            this.Layer0MoveUpButton.UseVisualStyleBackColor = true;
            this.Layer0MoveUpButton.Click += new System.EventHandler( this.Layer0MoveUpButton_Click );
            // 
            // Layer0MoveDownButton
            // 
            this.Layer0MoveDownButton.Location = new System.Drawing.Point( 982, 476 );
            this.Layer0MoveDownButton.Name = "Layer0MoveDownButton";
            this.Layer0MoveDownButton.Size = new System.Drawing.Size( 50, 40 );
            this.Layer0MoveDownButton.TabIndex = 8;
            this.Layer0MoveDownButton.Text = "Down";
            this.Layer0MoveDownButton.UseVisualStyleBackColor = true;
            this.Layer0MoveDownButton.Click += new System.EventHandler( this.Layer0MoveDownButton_Click );
            // 
            // Layer1MoveDownButton
            // 
            this.Layer1MoveDownButton.Location = new System.Drawing.Point( 1305, 476 );
            this.Layer1MoveDownButton.Name = "Layer1MoveDownButton";
            this.Layer1MoveDownButton.Size = new System.Drawing.Size( 50, 40 );
            this.Layer1MoveDownButton.TabIndex = 9;
            this.Layer1MoveDownButton.Text = "Down";
            this.Layer1MoveDownButton.UseVisualStyleBackColor = true;
            this.Layer1MoveDownButton.Click += new System.EventHandler( this.Layer1MoveDownButton_Click );
            // 
            // Layer1MoveUpButton
            // 
            this.Layer1MoveUpButton.Location = new System.Drawing.Point( 1305, 430 );
            this.Layer1MoveUpButton.Name = "Layer1MoveUpButton";
            this.Layer1MoveUpButton.Size = new System.Drawing.Size( 50, 40 );
            this.Layer1MoveUpButton.TabIndex = 10;
            this.Layer1MoveUpButton.Text = "Up";
            this.Layer1MoveUpButton.UseVisualStyleBackColor = true;
            this.Layer1MoveUpButton.Click += new System.EventHandler( this.Layer1MoveUpButton_Click );
            // 
            // Layer0Label
            // 
            this.Layer0Label.AutoSize = true;
            this.Layer0Label.Font = new System.Drawing.Font( "Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ( ( byte )( 0 ) ) );
            this.Layer0Label.Location = new System.Drawing.Point( 723, 116 );
            this.Layer0Label.Name = "Layer0Label";
            this.Layer0Label.Size = new System.Drawing.Size( 99, 16 );
            this.Layer0Label.TabIndex = 12;
            this.Layer0Label.Text = "Layer 0 : 0% full";
            // 
            // Layer1Label
            // 
            this.Layer1Label.AutoSize = true;
            this.Layer1Label.Font = new System.Drawing.Font( "Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ( ( byte )( 0 ) ) );
            this.Layer1Label.Location = new System.Drawing.Point( 1046, 116 );
            this.Layer1Label.Name = "Layer1Label";
            this.Layer1Label.Size = new System.Drawing.Size( 99, 16 );
            this.Layer1Label.TabIndex = 13;
            this.Layer1Label.Text = "Layer 1 : 0% full";
            // 
            // GroupMoveUpButton
            // 
            this.GroupMoveUpButton.Location = new System.Drawing.Point( 648, 430 );
            this.GroupMoveUpButton.Name = "GroupMoveUpButton";
            this.GroupMoveUpButton.Size = new System.Drawing.Size( 50, 40 );
            this.GroupMoveUpButton.TabIndex = 14;
            this.GroupMoveUpButton.Text = "Up";
            this.GroupMoveUpButton.UseVisualStyleBackColor = true;
            this.GroupMoveUpButton.Click += new System.EventHandler( this.GroupMoveUpButton_Click );
            // 
            // GroupMoveDownButton
            // 
            this.GroupMoveDownButton.Location = new System.Drawing.Point( 648, 476 );
            this.GroupMoveDownButton.Name = "GroupMoveDownButton";
            this.GroupMoveDownButton.Size = new System.Drawing.Size( 50, 40 );
            this.GroupMoveDownButton.TabIndex = 15;
            this.GroupMoveDownButton.Text = "Down";
            this.GroupMoveDownButton.UseVisualStyleBackColor = true;
            this.GroupMoveDownButton.Click += new System.EventHandler( this.GroupMoveDownButton_Click );
            // 
            // UnrealDVDLayout
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF( 6F, 13F );
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size( 1366, 1215 );
            this.Controls.Add( this.GroupMoveDownButton );
            this.Controls.Add( this.GroupMoveUpButton );
            this.Controls.Add( this.Layer0Label );
            this.Controls.Add( this.GroupListView );
            this.Controls.Add( this.Layer1Label );
            this.Controls.Add( this.Layer0MoveDownButton );
            this.Controls.Add( this.Layer1MoveUpButton );
            this.Controls.Add( this.Layer0MoveUpButton );
            this.Controls.Add( this.Layer0ListView );
            this.Controls.Add( this.Layer1MoveDownButton );
            this.Controls.Add( this.FileListView );
            this.Controls.Add( this.MainToolStrip );
            this.Controls.Add( this.MainMenu );
            this.Controls.Add( this.MainLogWindow );
            this.Controls.Add( this.Layer1ListView );
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.MainMenuStrip = this.MainMenu;
            this.Name = "UnrealDVDLayout";
            this.Text = "Unreal DVD Layout Tool";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler( this.UnrealDVDLayout_FormClosed );
            this.MainMenu.ResumeLayout( false );
            this.MainMenu.PerformLayout();
            this.MainToolStrip.ResumeLayout( false );
            this.MainToolStrip.PerformLayout();
            this.FileListViewMenuStrip.ResumeLayout( false );
            this.GroupListViewMenuStrip.ResumeLayout( false );
            this.ResumeLayout( false );
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip MainMenu;
        private System.Windows.Forms.ToolStrip MainToolStrip;
        private System.Windows.Forms.OpenFileDialog GenericOpenFileDialog;
        private System.Windows.Forms.ToolStripMenuItem FileMainMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ImportTOCsFileMenuItem;
        private System.Windows.Forms.ToolStripMenuItem QuitFileMenuItem;
        private System.Windows.Forms.ToolStripButton ImportTOCsToolStripButton;
        private System.Windows.Forms.ToolStripMenuItem ToolsMainMenuItem;
        private System.Windows.Forms.ToolStripMenuItem OptionsToolsMenuItem;
        private System.Windows.Forms.RichTextBox MainLogWindow;
        private System.Windows.Forms.ToolStripButton SaveXGDToolStripButton;
        private System.Windows.Forms.ToolStripMenuItem SaveXGDMenuItem;
        private System.Windows.Forms.SaveFileDialog GenericSaveFileDialog;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem RecentMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripMenuItem Recent0MenuItem;
        private System.Windows.Forms.ToolStripMenuItem Recent1MenuItem;
        private System.Windows.Forms.ToolStripMenuItem Recent2MenuItem;
        private System.Windows.Forms.ToolStripMenuItem Recent3MenuItem;
        private System.Windows.Forms.ToolStripMenuItem Recent4MenuItem;
        private System.Windows.Forms.ListView FileListView;
        private System.Windows.Forms.ContextMenuStrip FileListViewMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem AddToGroupToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem MakeFromRegExpMenuItem;
        private System.Windows.Forms.ToolStripComboBox GameToolStripComboBox;
        private System.Windows.Forms.ListView GroupListView;
        private System.Windows.Forms.ColumnHeader ListViewNameHeader;
        private System.Windows.Forms.ColumnHeader ListViewSizeHeader;
        private System.Windows.Forms.ToolStripMenuItem NewGroupToolStripMenuItem;
        private System.Windows.Forms.ColumnHeader FileListViewPathHeader;
        private System.Windows.Forms.ListView Layer0ListView;
        private System.Windows.Forms.ListView Layer1ListView;
        private System.Windows.Forms.ColumnHeader Layer0GroupHeader;
        private System.Windows.Forms.ColumnHeader Layer0SizeHeader;
        private System.Windows.Forms.ColumnHeader Layer1GroupHeader;
        private System.Windows.Forms.ColumnHeader Layer1SizeHeader;
        private System.Windows.Forms.Button Layer0MoveUpButton;
        private System.Windows.Forms.Button Layer0MoveDownButton;
        private System.Windows.Forms.Button Layer1MoveDownButton;
        private System.Windows.Forms.Button Layer1MoveUpButton;
        private System.Windows.Forms.ContextMenuStrip GroupListViewMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem AddToLayer0MenuItem;
        private System.Windows.Forms.ToolStripMenuItem AddToLayer1MenuItem;
        private System.Windows.Forms.Label Layer0Label;
        private System.Windows.Forms.Label Layer1Label;
        private System.Windows.Forms.ToolStripMenuItem removeFromDiscToolStripMenuItem;
        private System.Windows.Forms.Button GroupMoveUpButton;
        private System.Windows.Forms.Button GroupMoveDownButton;
    }
}

