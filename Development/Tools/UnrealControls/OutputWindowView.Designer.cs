namespace UnrealControls
{
	partial class OutputWindowView
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			this.mHScrollBar = new System.Windows.Forms.HScrollBar();
			this.mVScrollBar = new System.Windows.Forms.VScrollBar();
			this.mDefaultCtxMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.mCtxDefault_Copy = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.mCtxDefault_ScrollToEnd = new System.Windows.Forms.ToolStripMenuItem();
			this.mCtxDefault_ScrollToHome = new System.Windows.Forms.ToolStripMenuItem();
			this.mScrollSelectedTextTimer = new System.Windows.Forms.Timer(this.components);
			this.mDefaultCtxMenu.SuspendLayout();
			this.SuspendLayout();
			// 
			// mHScrollBar
			// 
			this.mHScrollBar.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.mHScrollBar.Cursor = System.Windows.Forms.Cursors.Arrow;
			this.mHScrollBar.Location = new System.Drawing.Point(0, 250);
			this.mHScrollBar.Name = "mHScrollBar";
			this.mHScrollBar.Size = new System.Drawing.Size(325, 17);
			this.mHScrollBar.TabIndex = 0;
			this.mHScrollBar.ValueChanged += new System.EventHandler(this.mHScrollBar_ValueChanged);
			// 
			// mVScrollBar
			// 
			this.mVScrollBar.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.mVScrollBar.Cursor = System.Windows.Forms.Cursors.Arrow;
			this.mVScrollBar.Location = new System.Drawing.Point(325, 0);
			this.mVScrollBar.Name = "mVScrollBar";
			this.mVScrollBar.Size = new System.Drawing.Size(17, 250);
			this.mVScrollBar.TabIndex = 1;
			this.mVScrollBar.ValueChanged += new System.EventHandler(this.mVScrollBar_ValueChanged);
			// 
			// mDefaultCtxMenu
			// 
			this.mDefaultCtxMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mCtxDefault_Copy,
            this.toolStripSeparator1,
            this.mCtxDefault_ScrollToEnd,
            this.mCtxDefault_ScrollToHome});
			this.mDefaultCtxMenu.Name = "mDefaultCtxMenu";
			this.mDefaultCtxMenu.Size = new System.Drawing.Size(157, 76);
			// 
			// mCtxDefault_Copy
			// 
			this.mCtxDefault_Copy.Name = "mCtxDefault_Copy";
			this.mCtxDefault_Copy.Size = new System.Drawing.Size(156, 22);
			this.mCtxDefault_Copy.Text = "Copy";
			this.mCtxDefault_Copy.Click += new System.EventHandler(this.mCtxDefault_Copy_Click);
			// 
			// toolStripSeparator1
			// 
			this.toolStripSeparator1.Name = "toolStripSeparator1";
			this.toolStripSeparator1.Size = new System.Drawing.Size(153, 6);
			// 
			// mCtxDefault_ScrollToEnd
			// 
			this.mCtxDefault_ScrollToEnd.Name = "mCtxDefault_ScrollToEnd";
			this.mCtxDefault_ScrollToEnd.Size = new System.Drawing.Size(156, 22);
			this.mCtxDefault_ScrollToEnd.Text = "Scroll To End";
			this.mCtxDefault_ScrollToEnd.Click += new System.EventHandler(this.mCtxDefault_ScrollToEnd_Click);
			// 
			// mCtxDefault_ScrollToHome
			// 
			this.mCtxDefault_ScrollToHome.Name = "mCtxDefault_ScrollToHome";
			this.mCtxDefault_ScrollToHome.Size = new System.Drawing.Size(156, 22);
			this.mCtxDefault_ScrollToHome.Text = "Scroll To Home";
			this.mCtxDefault_ScrollToHome.Click += new System.EventHandler(this.mCtxDefault_ScrollToHome_Click);
			// 
			// mScrollSelectedTextTimer
			// 
			this.mScrollSelectedTextTimer.Interval = 20;
			this.mScrollSelectedTextTimer.Tick += new System.EventHandler(this.mScrollSelectedTextTimer_Tick);
			// 
			// OutputWindowView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.SystemColors.Window;
			this.ContextMenuStrip = this.mDefaultCtxMenu;
			this.Controls.Add(this.mVScrollBar);
			this.Controls.Add(this.mHScrollBar);
			this.Cursor = System.Windows.Forms.Cursors.IBeam;
			this.DoubleBuffered = true;
			this.Font = new System.Drawing.Font("Courier New", 9F);
			this.Name = "OutputWindowView";
			this.Size = new System.Drawing.Size(342, 267);
			this.mDefaultCtxMenu.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.HScrollBar mHScrollBar;
		private System.Windows.Forms.VScrollBar mVScrollBar;
		private System.Windows.Forms.ContextMenuStrip mDefaultCtxMenu;
		private System.Windows.Forms.ToolStripMenuItem mCtxDefault_Copy;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
		private System.Windows.Forms.ToolStripMenuItem mCtxDefault_ScrollToEnd;
		private System.Windows.Forms.ToolStripMenuItem mCtxDefault_ScrollToHome;
		private System.Windows.Forms.Timer mScrollSelectedTextTimer;
	}
}
