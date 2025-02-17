using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace DVDLogParser
{
	/// <summary>
	/// Summary description for Preferences.
	/// </summary>
	public class Preferences : System.Windows.Forms.Form
	{
		private System.Windows.Forms.NumericUpDown Preferences_SecondsHistory;
		private System.Windows.Forms.Button Preferences_OK;
		private System.Windows.Forms.Label Preferences_SecondsHistoryText;
		private System.ComponentModel.Container components = null;
		private System.Windows.Forms.Label Preferences_UpdateRateText;
		private System.Windows.Forms.NumericUpDown Preferences_UpdateRate;

		private LogParserDisplay Display;

		private void Preferences_HistorySecondsChanged( object sender, System.EventArgs e )
		{
			try
			{
				System.Windows.Forms.NumericUpDown Number = ( System.Windows.Forms.NumericUpDown )sender;
				Display.SetHistorySeconds( Convert.ToInt32( Number.Value ) );
			}
			catch( System.Exception )
			{
			}
		}

		private void Preferences_UpdateRateChanged(object sender, System.EventArgs e)
		{
			try
			{
				System.Windows.Forms.NumericUpDown Number = ( System.Windows.Forms.NumericUpDown )sender;
				Display.SetUpdateRateSeconds( Convert.ToSingle( Number.Value ) );
			}
			catch( System.Exception )
			{
			}		
		}

		private void Preferences_ClickOK( object sender, System.EventArgs e )
		{
			Dispose();
		}

		public Preferences( LogParserDisplay Parent, int HistorySeconds, int UpdateRateDeciseconds )
		{
			Display = Parent;

			// Required for Windows Form Designer support
			InitializeComponent();

			Preferences_SecondsHistory.Value = new System.Decimal( new int[] { HistorySeconds, 0, 0, 0 } );		
			Preferences_UpdateRate.Value = new System.Decimal( new int[] { UpdateRateDeciseconds, 0, 0, 65536 } );
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
			this.Preferences_SecondsHistory = new System.Windows.Forms.NumericUpDown();
			this.Preferences_SecondsHistoryText = new System.Windows.Forms.Label();
			this.Preferences_OK = new System.Windows.Forms.Button();
			this.Preferences_UpdateRateText = new System.Windows.Forms.Label();
			this.Preferences_UpdateRate = new System.Windows.Forms.NumericUpDown();
			((System.ComponentModel.ISupportInitialize)(this.Preferences_SecondsHistory)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.Preferences_UpdateRate)).BeginInit();
			this.SuspendLayout();
			// 
			// Preferences_SecondsHistory
			// 
			this.Preferences_SecondsHistory.Location = new System.Drawing.Point(272, 24);
			this.Preferences_SecondsHistory.Maximum = new System.Decimal(new int[] {
																					   1000,
																					   0,
																					   0,
																					   0});
			this.Preferences_SecondsHistory.Minimum = new System.Decimal(new int[] {
																					   10,
																					   0,
																					   0,
																					   0});
			this.Preferences_SecondsHistory.Name = "Preferences_SecondsHistory";
			this.Preferences_SecondsHistory.Size = new System.Drawing.Size(104, 20);
			this.Preferences_SecondsHistory.TabIndex = 0;
			this.Preferences_SecondsHistory.Value = new System.Decimal(new int[] {
																					 20,
																					 0,
																					 0,
																					 0});
			this.Preferences_SecondsHistory.ValueChanged += new System.EventHandler(this.Preferences_HistorySecondsChanged);
			// 
			// Preferences_SecondsHistoryText
			// 
			this.Preferences_SecondsHistoryText.Location = new System.Drawing.Point(24, 24);
			this.Preferences_SecondsHistoryText.Name = "Preferences_SecondsHistoryText";
			this.Preferences_SecondsHistoryText.Size = new System.Drawing.Size(232, 23);
			this.Preferences_SecondsHistoryText.TabIndex = 1;
			this.Preferences_SecondsHistoryText.Text = "Number of seconds to maintain history";
			// 
			// Preferences_OK
			// 
			this.Preferences_OK.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.Preferences_OK.Location = new System.Drawing.Point(280, 136);
			this.Preferences_OK.Name = "Preferences_OK";
			this.Preferences_OK.Size = new System.Drawing.Size(96, 32);
			this.Preferences_OK.TabIndex = 2;
			this.Preferences_OK.Text = "OK";
			this.Preferences_OK.Click += new System.EventHandler(this.Preferences_ClickOK);
			// 
			// Preferences_UpdateRateText
			// 
			this.Preferences_UpdateRateText.Location = new System.Drawing.Point(24, 64);
			this.Preferences_UpdateRateText.Name = "Preferences_UpdateRateText";
			this.Preferences_UpdateRateText.Size = new System.Drawing.Size(232, 23);
			this.Preferences_UpdateRateText.TabIndex = 3;
			this.Preferences_UpdateRateText.Text = "Update rate";
			// 
			// Preferences_UpdateRate
			// 
			this.Preferences_UpdateRate.DecimalPlaces = 1;
			this.Preferences_UpdateRate.Increment = new System.Decimal(new int[] {
																					 1,
																					 0,
																					 0,
																					 65536});
			this.Preferences_UpdateRate.Location = new System.Drawing.Point(272, 64);
			this.Preferences_UpdateRate.Maximum = new System.Decimal(new int[] {
																				   10,
																				   0,
																				   0,
																				   0});
			this.Preferences_UpdateRate.Minimum = new System.Decimal(new int[] {
																				   2,
																				   0,
																				   0,
																				   65536});
			this.Preferences_UpdateRate.Name = "Preferences_UpdateRate";
			this.Preferences_UpdateRate.Size = new System.Drawing.Size(104, 20);
			this.Preferences_UpdateRate.TabIndex = 4;
			this.Preferences_UpdateRate.Value = new System.Decimal(new int[] {
																				 1,
																				 0,
																				 0,
																				 0});
			this.Preferences_UpdateRate.ValueChanged += new System.EventHandler(this.Preferences_UpdateRateChanged);
			// 
			// Preferences
			// 
			this.AcceptButton = this.Preferences_OK;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.Preferences_OK;
			this.ClientSize = new System.Drawing.Size(392, 180);
			this.Controls.Add(this.Preferences_UpdateRate);
			this.Controls.Add(this.Preferences_UpdateRateText);
			this.Controls.Add(this.Preferences_OK);
			this.Controls.Add(this.Preferences_SecondsHistoryText);
			this.Controls.Add(this.Preferences_SecondsHistory);
			this.DockPadding.All = 16;
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "Preferences";
			this.ShowInTaskbar = false;
			this.Text = "Preferences";
			((System.ComponentModel.ISupportInitialize)(this.Preferences_SecondsHistory)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.Preferences_UpdateRate)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion
	}
}
