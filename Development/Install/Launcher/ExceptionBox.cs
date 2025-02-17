using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace EpicGames
{
	public partial class ExceptionBox : Form
	{
		public ExceptionBox()
		{
			InitializeComponent();
		}

		public ExceptionBox(string message)
		{
			InitializeComponent();
			textException.Text = message;
		}

		private void buttonOK_Click(object sender, EventArgs e)
		{
			this.Close();
		}
	}
}