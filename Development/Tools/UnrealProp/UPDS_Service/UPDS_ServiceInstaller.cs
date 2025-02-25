using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration.Install;
using System.ServiceProcess;
using System.Xml;

namespace UnrealProp
{
    [RunInstaller( true )]
    public partial class UPDS_ServiceInstaller : Installer
    {
        private ServiceInstaller Installer;
        private ServiceProcessInstaller ProcessInstaller;

        public UPDS_ServiceInstaller()
        {
            InitializeComponent();
            Installer = new ServiceInstaller();
            Installer.StartType = System.ServiceProcess.ServiceStartMode.Automatic;
            Installer.ServiceName = "UPDS_Service";
            Installer.DisplayName = "UPDS_Service";
            Installer.Description = "UnrealProp Distribution Service";
            Installers.Add( Installer );

            ProcessInstaller = new ServiceProcessInstaller();
            ProcessInstaller.Account = ServiceAccount.User;
            ProcessInstaller.Username = "EPICGAMES\\UnrealProp";
            Installers.Add( ProcessInstaller );
        }

        // Must exists, because installation will fail without it.
        public override void Install( System.Collections.IDictionary stateSaver )
        {
            base.Install( stateSaver );
        }
    }
}