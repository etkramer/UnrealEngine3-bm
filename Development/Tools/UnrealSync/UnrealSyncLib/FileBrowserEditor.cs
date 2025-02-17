using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing.Design;
using System.Windows.Forms;

namespace UnrealSync.Service
{
	/// <summary>
	/// Custom editor for file path's in a property grid.
	/// </summary>
	public class FileBrowserEditor : UITypeEditor
	{
		public override UITypeEditorEditStyle GetEditStyle(System.ComponentModel.ITypeDescriptorContext context)
		{
			return UITypeEditorEditStyle.Modal;
		}

		public override object EditValue(System.ComponentModel.ITypeDescriptorContext context, IServiceProvider provider, object value)
		{
			object returnVal = value;

			using(OpenFileDialog FileDlg = new OpenFileDialog())
			{
				FileDlg.Title = "Please locate your sync batch file";
				FileDlg.AddExtension = true;
				FileDlg.CheckFileExists = true;
				FileDlg.CheckPathExists = true;
				FileDlg.DefaultExt = "bat";
				FileDlg.FileName = "sync.bat";
				FileDlg.Filter = "Batch File|*.bat";
				FileDlg.FilterIndex = 0;
				FileDlg.InitialDirectory = Properties.Settings.Default.LastBuildDirectory;
				FileDlg.Multiselect = false;
				FileDlg.RestoreDirectory = true;
				FileDlg.SupportMultiDottedExtensions = true;
				FileDlg.ValidateNames = true;

				if(FileDlg.ShowDialog() == DialogResult.OK)
				{
					returnVal = FileDlg.FileName;
				}

				Properties.Settings.Default.LastBuildDirectory = FileDlg.FileName;
				Properties.Settings.Default.Save();
			}

			return returnVal;
		}
	}
}
