using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Net;

namespace UnrealDatabaseProxy
{
	/// <summary>
	/// This class represents the server and handles incoming connections.
	/// </summary>
	public class Server
	{
		const int PORT = 10500;

		Socket mListenSocket;
		SocketAsyncEventArgs mAcceptArgs = new SocketAsyncEventArgs();

		/// <summary>
		/// Constructor.
		/// </summary>
		public Server()
		{
			mAcceptArgs.DisconnectReuseSocket = false;
			mAcceptArgs.Completed += new EventHandler<SocketAsyncEventArgs>(OnAccept);
		}

		/// <summary>
		/// Callback for accepting incoming connections.
		/// </summary>
		/// <param name="sender">The object that initiated the event.</param>
		/// <param name="e">Information about the event.</param>
		void OnAccept(object sender, SocketAsyncEventArgs e)
		{
            //Console.WriteLine( "Trying to aceept a connection" );
			ClientConnection newClient = null;
			try
			{
				newClient = new ClientConnection(e.AcceptSocket);
				e.AcceptSocket = null;

				mListenSocket.AcceptAsync(e);

				newClient.BeginRecv();
			}
			catch(Exception ex)
			{
				if(newClient != null)
				{
					newClient.Dispose();
				}

				System.Diagnostics.Debug.WriteLine(ex.ToString());
				System.Diagnostics.EventLog.WriteEntry("UnrealDatabaseProxy", ex.ToString(), System.Diagnostics.EventLogEntryType.Error);
			}
		}

		/// <summary>
		/// Starts the server and begins listening for incoming connections.
		/// </summary>
		public void Start()
		{
			try
			{
				if(mListenSocket != null && mListenSocket.Connected)
				{
					mListenSocket.Close();
					mListenSocket = null;
				}

				mListenSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

				mListenSocket.Bind(new IPEndPoint(IPAddress.Any, PORT));
				mListenSocket.Listen(100);
				mListenSocket.AcceptAsync(mAcceptArgs);

                //Console.WriteLine("server started up");
			}
			catch(Exception ex)
			{
				System.Diagnostics.Debug.WriteLine(ex.ToString());
				System.Diagnostics.EventLog.WriteEntry("UnrealDatabaseProxy", ex.ToString(), System.Diagnostics.EventLogEntryType.Error);

				throw ex;
			}
		}

		/// <summary>
		/// Stops the server from accepting incoming connections.
		/// </summary>
		public void Stop()
		{
			try
			{
				mListenSocket.Close();
				mListenSocket = null;
			}
			catch(Exception ex)
			{
				System.Diagnostics.Debug.WriteLine(ex.ToString());
				System.Diagnostics.EventLog.WriteEntry("UnrealDatabaseProxy", ex.ToString(), System.Diagnostics.EventLogEntryType.Error);

				throw ex;
			}
		}
	}
}
