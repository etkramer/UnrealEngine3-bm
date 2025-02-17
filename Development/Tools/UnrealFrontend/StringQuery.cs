using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace UnrealFrontend
{
	public partial class StringQuery : Form
	{
		public StringQuery()
		{
			InitializeComponent();
		}

		public string Caption
		{
			set { Label_Caption.Text = value; }
			get { return Label_Caption.Text; }
		}

		public string Title
		{
			set { Text = value; }
			get { return Text; }
		}

		public string String
		{
			set { TextBox_String.Text = value; }
			get { return TextBox_String.Text; }
		}
	}
}