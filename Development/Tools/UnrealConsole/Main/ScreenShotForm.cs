using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Drawing.Imaging;

namespace UnrealConsole.Main
{
	public partial class ScreenShotForm : Form
	{
		public ScreenShotForm(string Title, Image Img)
		{
			InitializeComponent();

			this.Text = Title;
			this.pictMain.Image = Img;
			this.pictMain.Width = Img.Width;
			this.pictMain.Height = Img.Height;

			Screen Primary = Screen.PrimaryScreen;

			if(Img.Width < Primary.Bounds.Width && (Img.Height + pictMain.Location.Y) < Primary.Bounds.Height)
			{
				this.SetClientSizeCore(Img.Width, Img.Height + pictMain.Location.Y);
			}
		}

		private void menuItemFile_Exit_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		private void menuItemFile_SaveAs_Click(object sender, EventArgs e)
		{
			if(saveImageDlg.FileName != null && saveImageDlg.FileName.Length > 0)
			{
				saveImageDlg.FileName = Path.GetFileName(saveImageDlg.FileName);
			}

			if(saveImageDlg.ShowDialog(this) == DialogResult.OK)
			{
				ImageFormat ImgFormat = ImageFormat.Png;

				switch(Path.GetExtension(saveImageDlg.FileName))
				{
					case "png":
						{
							ImgFormat = ImageFormat.Png;
							break;
						}
					case "bmp":
						{
							ImgFormat = ImageFormat.Bmp;
							break;
						}
					case "jpg":
						{
							ImgFormat = ImageFormat.Jpeg;
							break;
						}
					case "gif":
						{
							ImgFormat = ImageFormat.Gif;
							break;
						}
				}

				pictMain.Image.Save(saveImageDlg.FileName, ImgFormat);
			}
		}

		protected override void OnClosed(EventArgs e)
		{
			base.OnClosed(e);

			this.Dispose();
		}
	}
}