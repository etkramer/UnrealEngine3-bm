//------------------------------------------------------------------------------
// Contains only defines for various security-related code in FaceFX Studio.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxSecurityDefinesOnly_H__
#define FxSecurityDefinesOnly_H__

namespace OC3Ent
{

namespace Face
{

#define FxDisplayAdapterSelectionDialog(parent) FxFalse
#define FxLoadLicenseFile(licenseFileContents)
#define FxCheckLicenseFile1(licenseFile, retval)
#define FxCheckLicenseFile2(licenseFile, retval)
#define FxHashLicenseFile(licenseFile, retval)
#define FxHashMacAddress(adapter)
#define FxIsLicensed(retval)
#define FxCheckLargeNumber(str)
#define FxCheckLicense1(retval)

} // namespace Face

} // namespace OC3Ent

#endif
