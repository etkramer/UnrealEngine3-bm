using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;

namespace UnrealConsole.Network
{
	/// <summary>
	/// A form for telling the user targets are being enumerated.
	/// </summary>
	public partial class EnumeratingTargetsForm : Form
	{
		ManualResetEvent mEvent;

		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="Event">The event that tells the form all targets have been enumerated.</param>
		public EnumeratingTargetsForm(ManualResetEvent Event)
		{
			InitializeComponent();

			mEvent = Event;
		}

		/// <summary>
		/// Timer callback for polling the lifetime event.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		private void mLifetimeTimer_Tick(object sender, EventArgs e)
		{
			if(mEvent.WaitOne(1, false))
			{
				this.Close();
			}
		}
	}
}