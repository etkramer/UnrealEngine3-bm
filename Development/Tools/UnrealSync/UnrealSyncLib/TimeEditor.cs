using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing.Design;
using System.Windows.Forms.Design;
using System.Windows.Forms;

namespace UnrealSync.Service
{
	/// <summary>
	/// Custom editor for time values in a property grid.
	/// </summary>
	public class TimeEditor : UITypeEditor
	{
		public const string DT_FORMAT = "h:mm tt";

		private IWindowsFormsEditorService edSvc = null;

		public override object EditValue(System.ComponentModel.ITypeDescriptorContext context, IServiceProvider provider, object value)
		{
			object retVal = value;

			using(DateTimePicker dtPicker = new DateTimePicker())
			{
				dtPicker.Format = DateTimePickerFormat.Custom;
				dtPicker.CustomFormat = DT_FORMAT;
				dtPicker.ShowUpDown = true;

				if(context != null && context.Instance != null && provider != null)
				{
					edSvc = (IWindowsFormsEditorService)provider.GetService(typeof(IWindowsFormsEditorService));

					if(edSvc != null)
					{
						edSvc.DropDownControl(dtPicker);

						retVal = dtPicker.Value.ToString(DT_FORMAT);
					}
				}
			}

			return retVal;

		}

		public override UITypeEditorEditStyle GetEditStyle(System.ComponentModel.ITypeDescriptorContext context)
		{
			if(context != null && context.Instance != null)
			{
				return UITypeEditorEditStyle.DropDown;
			}

			return base.GetEditStyle(context);
		}
	}
}
