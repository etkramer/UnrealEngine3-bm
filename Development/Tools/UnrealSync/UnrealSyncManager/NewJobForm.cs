using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace UnrealSync.Manager
{
	public partial class NewJobForm : Form
	{
		public string SyncJobName
		{
			get { return txtJobName.Text; }
		}

		public NewJobForm()
		{
			InitializeComponent();
		}

		private void btnOK_Click(object sender, EventArgs e)
		{
			if(txtJobName.Text == null || txtJobName.Text.Length == 0)
			{
				MessageBox.Show(this, "Invalid job name!", "Error", MessageBoxButtons.OK);
			}
			else
			{
				this.DialogResult = DialogResult.OK;
				this.Close();
			}
		}

		private void btnCancel_Click(object sender, EventArgs e)
		{
			this.DialogResult = DialogResult.Cancel;
			this.Close();
		}
	}
}