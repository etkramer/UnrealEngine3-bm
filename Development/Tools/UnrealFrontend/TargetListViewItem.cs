using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Net;

namespace UnrealFrontend
{
	/// <summary>
	/// Represents a target list view item.
	/// </summary>
	public class TargetListViewItem : ListViewItem
	{
		ConsoleInterface.PlatformTarget mTarget;

		/// <summary>
		/// Gets the target associated with the list view item.
		/// </summary>
		public ConsoleInterface.PlatformTarget Target
		{
			get { return mTarget; }
		}

		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="Target">The target to be associated with the item.</param>
		/// <param name="bPullTargetInfo">True if all of the information for the target will be retrieved for the sub-items.</param>
		public TargetListViewItem(ConsoleInterface.PlatformTarget Target, bool bPullTargetInfo)
		{
			if(Target == null)
			{
				throw new ArgumentNullException("Target");
			}

			mTarget = Target;

			if(bPullTargetInfo)
			{
				this.Text = mTarget.Name;
				this.Tag = mTarget.TargetManagerName;
				this.SubItems.Add(mTarget.IPAddress.ToString());
				this.SubItems.Add(mTarget.DebugIPAddress.ToString());
				this.SubItems.Add(mTarget.ConsoleType.ToString());
			}
			else
			{
				this.Tag = this.Text = mTarget.TargetManagerName;
				this.SubItems.Add("n/a");
				this.SubItems.Add("n/a");
				this.SubItems.Add("n/a");
			}
		}
	}
}
