//------------------------------------------------------------------------------
// Wrappers for various wxWindows things.  Mostly needed so that Unreal doesn't
// pitch a fit.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxWxWrappers_H__
#define FxWxWrappers_H__

//@note: Always keep these macros up-to-date with the current version of
//       wxWindows (see wx/object.h).
#ifdef __UNREAL__
	#define WX_DECLARE_DYNAMIC_CLASS(name)          \
			public:                                     \
			static wxClassInfo ms_classInfo;            \
			static wxObject* wxCreateObject();          \
			virtual wxClassInfo *GetClassInfo() const   \
			{ return &name::ms_classInfo; }
	#define WX_DECLARE_ABSTRACT_CLASS(name) WX_DECLARE_DYNAMIC_CLASS(name)
	#define WX_DECLARE_CLASS(name) WX_DECLARE_DYNAMIC_CLASS(name)
	#define WX_IMPLEMENT_DYNAMIC_CLASS(name, basename)  \
		wxObject* name::wxCreateObject()                \
			{ return new name; }                            \
			wxClassInfo name::ms_classInfo(wxT(#name),      \
			&basename::ms_classInfo, NULL,                  \
			(int) sizeof(name),                             \
			(wxObjectConstructorFn) name::wxCreateObject);
	#define WX_IMPLEMENT_ABSTRACT_CLASS(name, basename) \
		wxClassInfo name::ms_classInfo(wxT(#name),      \
		&basename::ms_classInfo, NULL,                  \
		(int) sizeof(name), (wxObjectConstructorFn) 0);
	#define WX_IMPLEMENT_CLASS(name, basename) WX_IMPLEMENT_ABSTRACT_CLASS(name, basename)
#else
	#define WX_DECLARE_ABSTRACT_CLASS DECLARE_ABSTRACT_CLASS
	#define WX_DECLARE_CLASS DECLARE_CLASS
	#define WX_DECLARE_DYNAMIC_CLASS DECLARE_DYNAMIC_CLASS
	#define WX_IMPLEMENT_ABSTRACT_CLASS IMPLEMENT_ABSTRACT_CLASS
	#define WX_IMPLEMENT_CLASS IMPLEMENT_CLASS
	#define WX_IMPLEMENT_DYNAMIC_CLASS IMPLEMENT_DYNAMIC_CLASS
#endif

#endif
