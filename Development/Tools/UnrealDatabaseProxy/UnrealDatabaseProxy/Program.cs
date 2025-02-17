using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading;

namespace UnrealDatabaseProxy
{
	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		static void Main()
		{
			ServiceBase[] ServicesToRun;
			ServicesToRun = new ServiceBase[] 
			{ 
				new Service() 
			};
			ServiceBase.Run(ServicesToRun);
		}
/*
		// test main so you can see the log spam in Visual Studio's output window
        static void Main()
        {
            //ServiceBase[] ServicesToRun;
            //ServicesToRun = new ServiceBase[] 
            //{ 
            //	new Service() 
            //};
            //ServiceBase.Run(ServicesToRun);


                UnrealDatabaseProxy.Server foo = new UnrealDatabaseProxy.Server();
                foo.Start();
                while (true)
                {
                    System.Threading.Thread.Sleep(1);
            }
        }
*/

	}
}
