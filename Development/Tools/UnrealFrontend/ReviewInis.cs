using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace UnrealFrontend
{
	public partial class ReviewInis : Form
	{
		public ReviewInis()
		{
			InitializeComponent();
		}

		private void LinkLabel_ModDir_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
		{
			// open the directory
			System.Diagnostics.Process.Start(LinkLabel_ModDir.Text);
		}
	}
}