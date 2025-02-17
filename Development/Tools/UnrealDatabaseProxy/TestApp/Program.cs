using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Net;

namespace TestApp
{
	class Program
	{
		static void Main(string[] args)
		{
			Console.WriteLine("Creating socket...");
			Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

			Console.WriteLine("Connecting to 127.0.0.1:10500...");
			sock.Connect(new IPEndPoint(IPAddress.Parse("127.0.0.1"), 10500));

			Console.WriteLine("Sending data...");
			
			Console.WriteLine("{0} bytes sent!", sock.Send(Encoding.Unicode.GetBytes("exec dbo.[TestProc] @SomeStr='Hello World!', @SomeVal=100\0")));

			Console.WriteLine("Closing socket...");
			sock.Close();
		}
	}
}
