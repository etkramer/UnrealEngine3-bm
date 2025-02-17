// Guids.cs
// MUST match guids.h
using System;

namespace Epic.UnrealDebugger2005
{
    static class GuidList
    {
        public const string guidUCDebuggerPkgPkgString = "949b49e2-9051-424d-a3bf-44c7ea2bc347";
        public const string guidUCDebuggerPkgCmdSetString = "71139bf3-0043-4e3c-be8e-7690f957c4b7";
        public const string guidToolWindowPersistanceString = "58f08ec2-9154-47be-9be7-2932ce386f21";

        public static readonly Guid guidUCDebuggerPkgPkg = new Guid(guidUCDebuggerPkgPkgString);
        public static readonly Guid guidUCDebuggerPkgCmdSet = new Guid(guidUCDebuggerPkgCmdSetString);
        public static readonly Guid guidToolWindowPersistance = new Guid(guidToolWindowPersistanceString);
    };
}