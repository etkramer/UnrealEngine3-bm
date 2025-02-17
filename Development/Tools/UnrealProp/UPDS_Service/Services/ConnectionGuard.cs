using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.ServiceModel;
using System.Net.NetworkInformation;

namespace UProp
{
    // Working thread for guarding connection to UPMS
    public static class ConnectionGuard
    {
        static DS2MS_ConnectorClient ds2msConnectorClient;
        static InstanceContext instanceContext;

        static Thread threadGuard;
        static Random rnd = new Random(666);

        public static bool IsConnected
        {
            get
            {                
                return Connector==null?false:true;
            }
        }

        public static DS2MS_ConnectorClient Connector
        {
            get {
                TryToConnect();                
                return ds2msConnectorClient;
            }
        }

        public static DS2MS_ConnectorClient UPMS
        {
            get
            {
                TryToConnect();
                return ds2msConnectorClient;
            }
        }

        public static void Init()
        {
            instanceContext = new InstanceContext(new DS2MS_ConnectorCallback());
            TryToConnect();

            threadGuard = new Thread(new ThreadStart(GuardProc));
            threadGuard.Start();
        }

        public static void Release()
        {
            ds2msConnectorClient.Close();
            ds2msConnectorClient.Abort();
            threadGuard.Abort();
            threadGuard = null;
            instanceContext = null;
        }

        static void TryToConnect()
        {
            Mutex m = new Mutex(false);
            m.WaitOne();
            if (ds2msConnectorClient == null || ds2msConnectorClient.State != CommunicationState.Opened)
            {
                ds2msConnectorClient = null;
                try
                {
                    string myName = IPGlobalProperties.GetIPGlobalProperties().HostName + "." +
                        IPGlobalProperties.GetIPGlobalProperties().DomainName;

                    
                    ds2msConnectorClient = new DS2MS_ConnectorClient(instanceContext);
                    ds2msConnectorClient.Open();

                    ds2msConnectorClient.InnerDuplexChannel.Faulted += delegate(object sender, EventArgs e)
                    {
                        TaskExecutor.StopAllTasks();
                    };
                    

                    //while (ds2msConnectorClient.State == CommunicationState.Opening);
                    
                    ds2msConnectorClient.DistibutionServer_Register(myName);
                    Log.WriteLine("UPDS CONNECTION GUARD", Log.LogType.Important, "Connected and registered to UPMS!");
                }
                catch (Exception)
                {
                    Log.WriteLine("UPDS CONNECTION GUARD", Log.LogType.Important, "Cannot find UPMS!");
                    ds2msConnectorClient = null;
                }
            }
            m.ReleaseMutex();
        }
        

        // thread proc
        static void GuardProc()
        {
            while (true)
            {
                TryToConnect();
                Thread.Sleep(rnd.Next(25000, 35000)); // 25-35 sec interval
            }
        }

    }
}
