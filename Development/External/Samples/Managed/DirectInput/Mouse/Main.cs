using System;
using System.Threading;
using Microsoft.DirectX.DirectInput;
using Microsoft.DirectX;

namespace Mouse
{
	public class cMain : IDisposable
	{
		// These two fields hold common data that
		// different threads will have to access
		public static MouseState	g_dims			= new MouseState();
		public static bool			g_bRunning		= true;

		InputObject	m_di			= null;
		Device		m_dev			= null;
		Thread		m_threadInput	= null;				
		frmUI		m_UI			= null;

		public cMain()
		{
			// Create an instance of the UI form.
			m_UI = new frmUI();
			// Create a DirectInputObject object.
			m_di = new InputObject( DXHelp.AppInstance );
			// Create the device.
			m_dev = new Device(SystemGuid.Mouse, m_di);
			// Set the cooperative level for the device.
			m_dev.SetCooperativeLevel(m_UI.Handle, CooperativeLevelFlags.Exclusive | CooperativeLevelFlags.Foreground);
			// Set the data format to the mouse2 pre-defined format.
			m_dev.SetDataFormat(DeviceDataFormat.Mouse);
			// Must allocate memory for arrays inside
			// the C# app, since no fixed-length arrays
			// are permitted inside of structures.			
			g_dims.Buttons = new byte[8];
			// Create the thread that the app will use
			// to get state information from the device.
			m_threadInput = new Thread(new ThreadStart(DIThread) );
			// Name it something so we can see in the output
			// window when it has exited.
			m_threadInput.Name = "DIThread";
		}
		public void Start()
		{
			// Start the thread.
			m_threadInput.Start();
			// Show the UI form.
			m_UI.ShowDialog();
			// Wait until the input thread exits.
			m_threadInput.Join();
			// Call the function that destroys all the objects.
			this.Dispose();
		}
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[MTAThread]
		static void Main()
		{
			cMain main = new cMain();
			main.Start();
		}
		public void Dispose()
		{
			// Unacquire and destroy all Dinput objects.
			m_dev.Unacquire();
			m_dev.Dispose();
			m_di.Dispose();
		}
		public void DIThread()
		{
			// The actual input loop runs in this thread.

			// Bool flag that is set when it's ok
			// to get device state information.
			bool bOk = false;

			// Make sure there is a valid device.
			if (m_dev != null)
			{				
				// Keep looping.
				while (g_bRunning)
				{
					try
					{
						// Don't really need to poll a mouse, but
						// this is a good way to check if the app
						// can get the device state.
						m_dev.Poll();
					}
					catch(DirectXException ex)
					{
						// Check to see if either the app
						// needs to acquire the device, or
						// if the app lost the mouse to another
						// process.
						if ( (ex.ErrorCode == (int)ErrorCode.NotAcquired) || (ex.ErrorCode ==(int)ErrorCode.InputLost) )
						{
							try
							{
								// Acquire the device.
								m_dev.Acquire();
								// Set the flag for now.
								bOk = true;
							}
							catch(DirectXException ex2)
							{								
								if ( ex2.ErrorCode != (int)ErrorCode.OtherAppHasPrio )
								{	// Something very odd happened.
									throw new Exception("An unknown error has occcurred. This app won't be able to process device info.");
								}
								// Failed to aquire the device.
								// This could be because the app
								// doesn't have focus.
								bOk = false;
							}
						}
					}
					if (bOk == true)
					{
						// Lock the UI class so it can't overwrite
						// the g_dims structure during a race condition.
						lock(m_UI)
						{
							// Get the state of the device
							try	{ g_dims = m_dev.CurrentMouseState; }
							// Catch any exceptions. None will be handled here, 
							// any device re-aquisition will be handled above.	
							catch(DirectXException){}
						}
						// Call the function in the other thread that updates the UI.
						m_UI.UpdateUI();
					}
				} //while (g_bRunning)
			} //if (m_dev != null)
		}
	}
}
