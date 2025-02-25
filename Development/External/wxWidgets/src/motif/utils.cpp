/////////////////////////////////////////////////////////////////////////////
// Name:        src/motif/utils.cpp
// Purpose:     Various utilities
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// RCS-ID:      $Id: utils.cpp 42787 2006-10-30 22:58:02Z VZ $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __VMS
#define XtDisplay XTDISPLAY
#endif

#include "wx/utils.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/dcmemory.h"
    #include "wx/bitmap.h"
#endif

#include "wx/apptrait.h"
#include "wx/evtloop.h"

#include <string.h>

#if (defined(__SUNCC__) || defined(__CLCC__))
    #include <sysent.h>
#endif

#ifdef __VMS__
#pragma message disable nosimpint
#endif

#include "wx/unix/execute.h"

#include <Xm/Xm.h>
#include <Xm/Frame.h>

#include "wx/motif/private.h"

#if wxUSE_RESOURCES
#include "X11/Xresource.h"
#endif

#include "X11/Xutil.h"

#ifdef __VMS__
#pragma message enable nosimpint
#endif

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

// Yuck this is really BOTH site and platform dependent
// so we should use some other strategy!
#ifdef sun
    #define DEFAULT_XRESOURCE_DIR "/usr/openwin/lib/app-defaults"
#else
    #define DEFAULT_XRESOURCE_DIR "/usr/lib/X11/app-defaults"
#endif

#if wxUSE_RESOURCES
static char *GetIniFile (char *dest, const char *filename);
#endif

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// async event processing
// ----------------------------------------------------------------------------

// Consume all events until no more left
void wxFlushEvents(WXDisplay* wxdisplay)
{
    Display *display = (Display*)wxdisplay;
    wxEventLoop evtLoop;

    XSync (display, False);

    while (evtLoop.Pending())
    {
        XFlush (display);
        evtLoop.Dispatch();
    }
}

// ----------------------------------------------------------------------------
// wxExecute stuff
// ----------------------------------------------------------------------------

static void xt_notify_end_process(XtPointer data, int *WXUNUSED(fid),
                                  XtInputId *id)
{
    wxEndProcessData *proc_data = (wxEndProcessData *)data;

    wxHandleProcessTermination(proc_data);

    // VZ: I think they should be the same...
    wxASSERT( (int)*id == proc_data->tag );

    XtRemoveInput(*id);
}

int wxAddProcessCallback(wxEndProcessData *proc_data, int fd)
{
    XtInputId id = XtAppAddInput((XtAppContext) wxTheApp->GetAppContext(),
                                 fd,
                                 (XtPointer *) XtInputReadMask,
                                 (XtInputCallbackProc) xt_notify_end_process,
                                 (XtPointer) proc_data);

    return (int)id;
}

// ----------------------------------------------------------------------------
// misc
// ----------------------------------------------------------------------------

// Emit a beeeeeep
#ifndef __EMX__
// on OS/2, we use the wxBell from wxBase library (src/os2/utils.cpp)
void wxBell()
{
    // Use current setting for the bell
    XBell (wxGlobalDisplay(), 0);
}
#endif

wxPortId wxGUIAppTraits::GetToolkitVersion(int *verMaj, int *verMin) const
{
    // XmVERSION and XmREVISION are defined in Xm/Xm.h
    if ( verMaj )
        *verMaj = XmVERSION;
    if ( verMin )
        *verMin = XmREVISION;

    return wxPORT_MOTIF;
}


// ----------------------------------------------------------------------------
// Reading and writing resources (eg WIN.INI, .Xdefaults)
// ----------------------------------------------------------------------------

#if wxUSE_RESOURCES

// Read $HOME for what it says is home, if not
// read $USER or $LOGNAME for user name else determine
// the Real User, then determine the Real home dir.
static char * GetIniFile (char *dest, const char *filename)
{
    char *home = NULL;
    if (filename && wxIsAbsolutePath(filename))
    {
        strcpy(dest, filename);
    }
    else if ((home = wxGetUserHome()) != NULL)
    {
        strcpy(dest, home);
        if (dest[strlen(dest) - 1] != '/')
            strcat (dest, "/");
        if (filename == NULL)
        {
            if ((filename = getenv ("XENVIRONMENT")) == NULL)
                filename = ".Xdefaults";
        }
        else if (*filename != '.')
            strcat (dest, ".");
        strcat (dest, filename);
    } else
    {
        dest[0] = '\0';
    }
    return dest;
}

static char *GetResourcePath(char *buf, const char *name, bool create = false)
{
    if (create && wxFileExists (name) ) {
        strcpy(buf, name);
        return buf; // Exists so ...
    }

    if (*name == '/')
        strcpy(buf, name);
    else {
        // Put in standard place for resource files if not absolute
        strcpy (buf, DEFAULT_XRESOURCE_DIR);
        strcat (buf, "/");
        strcat (buf, wxFileNameFromPath (name).c_str());
    }

    if (create) {
        // Touch the file to create it
        FILE *fd = fopen (buf, "w");
        if (fd) fclose (fd);
    }
    return buf;
}

/*
* We have a cache for writing different resource files,
* which will only get flushed when we call wxFlushResources().
* Build up a list of resource databases waiting to be written.
*
*/

wxList wxResourceCache (wxKEY_STRING);

void
wxFlushResources (void)
{
    char nameBuffer[512];

    wxNode *node = wxResourceCache.First ();
    while (node)
    {
        const char *file = node->GetKeyString();
        // If file doesn't exist, create it first.
        (void)GetResourcePath(nameBuffer, file, true);

        XrmDatabase database = (XrmDatabase) node->Data ();
        XrmPutFileDatabase (database, nameBuffer);
        XrmDestroyDatabase (database);
        wxNode *next = node->Next ();
        delete node;
        node = next;
    }
}

static XrmDatabase wxResourceDatabase = 0;

void wxXMergeDatabases (wxApp * theApp, Display * display);

bool wxWriteResource(const wxString& section, const wxString& entry, const wxString& value, const wxString& file)
{
    char buffer[500];

    (void) GetIniFile (buffer, file);

    XrmDatabase database;
    wxNode *node = wxResourceCache.Find (buffer);
    if (node)
        database = (XrmDatabase) node->Data ();
    else
    {
        database = XrmGetFileDatabase (buffer);
        wxResourceCache.Append (buffer, (wxObject *) database);
    }

    char resName[300];
    strcpy (resName, section.c_str());
    strcat (resName, ".");
    strcat (resName, entry.c_str());

    XrmPutStringResource (&database, resName, value);
    return true;
}

bool wxWriteResource(const wxString& section, const wxString& entry, float value, const wxString& file)
{
    char buf[50];
    sprintf(buf, "%.4f", value);
    return wxWriteResource(section, entry, buf, file);
}

bool wxWriteResource(const wxString& section, const wxString& entry, long value, const wxString& file)
{
    char buf[50];
    sprintf(buf, "%ld", value);
    return wxWriteResource(section, entry, buf, file);
}

bool wxWriteResource(const wxString& section, const wxString& entry, int value, const wxString& file)
{
    char buf[50];
    sprintf(buf, "%d", value);
    return wxWriteResource(section, entry, buf, file);
}

bool wxGetResource(const wxString& section, const wxString& entry, char **value, const wxString& file)
{
    if (!wxResourceDatabase)
    {
        Display *display = wxGlobalDisplay();
        wxXMergeDatabases (wxTheApp, display);
    }

    XrmDatabase database;

    if (!file.empty())
    {
        char buffer[500];

        // Is this right? Trying to get it to look in the user's
        // home directory instead of current directory -- JACS
        (void) GetIniFile (buffer, file);

        wxNode *node = wxResourceCache.Find (buffer);
        if (node)
            database = (XrmDatabase) node->Data ();
        else
        {
            database = XrmGetFileDatabase (buffer);
            wxResourceCache.Append (buffer, (wxObject *) database);
        }
    }
    else
        database = wxResourceDatabase;

    XrmValue xvalue;
    char *str_type[20];
    char buf[150];
    strcpy (buf, section);
    strcat (buf, ".");
    strcat (buf, entry);

    Bool success = XrmGetResource (database, buf, "*", str_type,
        &xvalue);
    // Try different combinations of upper/lower case, just in case...
    if (!success)
    {
        buf[0] = (isupper (buf[0]) ? tolower (buf[0]) : toupper (buf[0]));
        success = XrmGetResource (database, buf, "*", str_type,
            &xvalue);
    }
    if (success)
    {
        if (*value)
            delete[] *value;

        *value = new char[xvalue.size + 1];
        strncpy (*value, xvalue.addr, (int) xvalue.size);
        return true;
    }
    return false;
}

bool wxGetResource(const wxString& section, const wxString& entry, float *value, const wxString& file)
{
    char *s = NULL;
    bool succ = wxGetResource(section, entry, (char **)&s, file);
    if (succ)
    {
        *value = (float)strtod(s, NULL);
        delete[] s;
        return true;
    }
    else return false;
}

bool wxGetResource(const wxString& section, const wxString& entry, long *value, const wxString& file)
{
    char *s = NULL;
    bool succ = wxGetResource(section, entry, (char **)&s, file);
    if (succ)
    {
        *value = strtol(s, NULL, 10);
        delete[] s;
        return true;
    }
    else return false;
}

bool wxGetResource(const wxString& section, const wxString& entry, int *value, const wxString& file)
{
    char *s = NULL;
    bool succ = wxGetResource(section, entry, (char **)&s, file);
    if (succ)
    {
        // Handle True, False here
        // True, Yes, Enables, Set or  Activated
        if (*s == 'T' || *s == 'Y' || *s == 'E' || *s == 'S' || *s == 'A')
            *value = true;
        // False, No, Disabled, Reset, Cleared, Deactivated
        else if (*s == 'F' || *s == 'N' || *s == 'D' || *s == 'R' || *s == 'C')
            *value = false;
        // Handle as Integer
        else
            *value = (int) strtol (s, NULL, 10);
        delete[] s;
        return true;
    }
    else
        return false;
}

void wxXMergeDatabases (wxApp * theApp, Display * display)
{
    XrmDatabase homeDB, serverDB, applicationDB;
    char filenamebuf[1024];

    char *filename = &filenamebuf[0];
    char *environment;
    wxString classname = theApp->GetClassName();
    char name[256];
    (void) strcpy (name, "/usr/lib/X11/app-defaults/");
    (void) strcat (name, classname.c_str());

    /* Get application defaults file, if any */
    applicationDB = XrmGetFileDatabase (name);
    (void) XrmMergeDatabases (applicationDB, &wxResourceDatabase);

    /* Merge server defaults, created by xrdb, loaded as a property of the root
    * window when the server initializes and loaded into the display
    * structure on XOpenDisplay;
    * if not defined, use .Xdefaults
    */

    if (XResourceManagerString (display) != NULL)
    {
        serverDB = XrmGetStringDatabase (XResourceManagerString (display));
    }
    else
    {
        (void) GetIniFile (filename, NULL);
        serverDB = XrmGetFileDatabase (filename);
    }
    XrmMergeDatabases (serverDB, &wxResourceDatabase);

    /* Open XENVIRONMENT file, or if not defined, the .Xdefaults,
    * and merge into existing database
    */

    if ((environment = getenv ("XENVIRONMENT")) == NULL)
    {
        size_t len;
        environment = GetIniFile (filename, NULL);
        len = strlen (environment);
        wxString hostname = wxGetHostName();
        if ( !hostname.empty() )
            strncat(environment, hostname, 1024 - len);
    }
    homeDB = XrmGetFileDatabase (environment);
    XrmMergeDatabases (homeDB, &wxResourceDatabase);
}

#if 0

/*
* Not yet used but may be useful.
*
*/
void
wxSetDefaultResources (const Widget w, const char **resourceSpec, const char *name)
{
    int i;
    Display *dpy = XtDisplay (w);    // Retrieve the display pointer

    XrmDatabase rdb = NULL;    // A resource data base

    // Create an empty resource database
    rdb = XrmGetStringDatabase ("");

    // Add the Component resources, prepending the name of the component

    i = 0;
    while (resourceSpec[i] != NULL)
    {
        char buf[1000];

        sprintf (buf, "*%s%s", name, resourceSpec[i++]);
        XrmPutLineResource (&rdb, buf);
    }

    // Merge them into the Xt database, with lowest precendence

    if (rdb)
    {
#if (XlibSpecificationRelease>=5)
        XrmDatabase db = XtDatabase (dpy);
        XrmCombineDatabase (rdb, &db, False);
#else
        XrmMergeDatabases (dpy->db, &rdb);
        dpy->db = rdb;
#endif
    }
}
#endif
// 0

#endif // wxUSE_RESOURCES

// ----------------------------------------------------------------------------
// display info
// ----------------------------------------------------------------------------

void wxGetMousePosition( int* x, int* y )
{
#if wxUSE_NANOX
    // TODO
    *x = 0;
    *y = 0;
#else
    XMotionEvent xev;
    Window root, child;
    XQueryPointer(wxGlobalDisplay(),
                  DefaultRootWindow(wxGlobalDisplay()),
                  &root, &child,
                  &(xev.x_root), &(xev.y_root),
                  &(xev.x),      &(xev.y),
                  &(xev.state));
    *x = xev.x_root;
    *y = xev.y_root;
#endif
}

// Return true if we have a colour display
bool wxColourDisplay()
{
    return wxDisplayDepth() > 1;
}

// Returns depth of screen
int wxDisplayDepth()
{
    Display *dpy = wxGlobalDisplay();

    return DefaultDepth (dpy, DefaultScreen (dpy));
}

// Get size of display
void wxDisplaySize(int *width, int *height)
{
    Display *dpy = wxGlobalDisplay();

    if ( width )
        *width = DisplayWidth (dpy, DefaultScreen (dpy));
    if ( height )
        *height = DisplayHeight (dpy, DefaultScreen (dpy));
}

void wxDisplaySizeMM(int *width, int *height)
{
    Display *dpy = wxGlobalDisplay();

    if ( width )
        *width = DisplayWidthMM(dpy, DefaultScreen (dpy));
    if ( height )
        *height = DisplayHeightMM(dpy, DefaultScreen (dpy));
}

void wxClientDisplayRect(int *x, int *y, int *width, int *height)
{
    // This is supposed to return desktop dimensions minus any window
    // manager panels, menus, taskbars, etc.  If there is a way to do that
    // for this platform please fix this function, otherwise it defaults
    // to the entire desktop.
    if (x) *x = 0;
    if (y) *y = 0;
    wxDisplaySize(width, height);
}


// Configurable display in wxX11 and wxMotif
static WXDisplay *gs_currentDisplay = NULL;
static wxString gs_displayName;

WXDisplay *wxGetDisplay()
{
    if (gs_currentDisplay)
        return gs_currentDisplay;
    else if (wxTheApp)
        return wxTheApp->GetInitialDisplay();
    return NULL;
}

bool wxSetDisplay(const wxString& display_name)
{
    gs_displayName = display_name;

    if ( display_name.empty() )
    {
        gs_currentDisplay = NULL;

        return true;
    }
    else
    {
        Cardinal argc = 0;

        Display *display = XtOpenDisplay((XtAppContext) wxTheApp->GetAppContext(),
            display_name.c_str(),
            wxTheApp->GetAppName().c_str(),
            wxTheApp->GetClassName().c_str(),
            NULL,
#if XtSpecificationRelease < 5
            0, &argc,
#else
            0, (int *)&argc,
#endif
            NULL);

        if (display)
        {
            gs_currentDisplay = (WXDisplay*) display;
            return true;
        }
        else
            return false;
    }
}

wxString wxGetDisplayName()
{
    return gs_displayName;
}

wxWindow* wxFindWindowAtPoint(const wxPoint& pt)
{
    return wxGenericFindWindowAtPoint(pt);
}

// ----------------------------------------------------------------------------
// Some colour manipulation routines
// ----------------------------------------------------------------------------

void wxHSVToXColor(wxHSV *hsv,XColor *rgb)
{
    int h = hsv->h;
    int s = hsv->s;
    int v = hsv->v;
    int r = 0, g = 0, b = 0;
    int i, f;
    int p, q, t;
    s = (s * wxMAX_RGB) / wxMAX_SV;
    v = (v * wxMAX_RGB) / wxMAX_SV;
    if (h == 360) h = 0;
    if (s == 0) { h = 0; r = g = b = v; }
    i = h / 60;
    f = h % 60;
    p = v * (wxMAX_RGB - s) / wxMAX_RGB;
    q = v * (wxMAX_RGB - s * f / 60) / wxMAX_RGB;
    t = v * (wxMAX_RGB - s * (60 - f) / 60) / wxMAX_RGB;
    switch (i)
    {
    case 0: r = v, g = t, b = p; break;
    case 1: r = q, g = v, b = p; break;
    case 2: r = p, g = v, b = t; break;
    case 3: r = p, g = q, b = v; break;
    case 4: r = t, g = p, b = v; break;
    case 5: r = v, g = p, b = q; break;
    }
    rgb->red = (unsigned short)(r << 8);
    rgb->green = (unsigned short)(g << 8);
    rgb->blue = (unsigned short)(b << 8);
}

void wxXColorToHSV(wxHSV *hsv,XColor *rgb)
{
    int r = rgb->red >> 8;
    int g = rgb->green >> 8;
    int b = rgb->blue >> 8;
    int maxv = wxMax3(r, g, b);
    int minv = wxMin3(r, g, b);
    int h = 0, s, v;
    v = maxv;
    if (maxv) s = (maxv - minv) * wxMAX_RGB / maxv;
    else s = 0;
    if (s == 0) h = 0;
    else
    {
        int rc, gc, bc, hex = 0;
        rc = (maxv - r) * wxMAX_RGB / (maxv - minv);
        gc = (maxv - g) * wxMAX_RGB / (maxv - minv);
        bc = (maxv - b) * wxMAX_RGB / (maxv - minv);
        if (r == maxv) { h = bc - gc, hex = 0; }
        else if (g == maxv) { h = rc - bc, hex = 2; }
        else if (b == maxv) { h = gc - rc, hex = 4; }
        h = hex * 60 + (h * 60 / wxMAX_RGB);
        if (h < 0) h += 360;
    }
    hsv->h = h;
    hsv->s = (s * wxMAX_SV) / wxMAX_RGB;
    hsv->v = (v * wxMAX_SV) / wxMAX_RGB;
}

void wxAllocNearestColor(Display *d,Colormap cmp,XColor *xc)
{
#if !wxUSE_NANOX
    int llp;

    int screen = DefaultScreen(d);
    int num_colors = DisplayCells(d,screen);

    XColor *color_defs = new XColor[num_colors];
    for(llp = 0;llp < num_colors;llp++) color_defs[llp].pixel = llp;
    XQueryColors(d,cmp,color_defs,num_colors);

    wxHSV hsv_defs, hsv;
    wxXColorToHSV(&hsv,xc);

    int diff, min_diff = 0, pixel = 0;

    for(llp = 0;llp < num_colors;llp++)
    {
        wxXColorToHSV(&hsv_defs,&color_defs[llp]);
        diff = wxSIGN(wxH_WEIGHT * (hsv.h - hsv_defs.h)) +
            wxSIGN(wxS_WEIGHT * (hsv.s - hsv_defs.s)) +
            wxSIGN(wxV_WEIGHT * (hsv.v - hsv_defs.v));
        if (llp == 0) min_diff = diff;
        if (min_diff > diff) { min_diff = diff; pixel = llp; }
        if (min_diff == 0) break;
    }

    xc -> red = color_defs[pixel].red;
    xc -> green = color_defs[pixel].green;
    xc -> blue = color_defs[pixel].blue;
    xc -> flags = DoRed | DoGreen | DoBlue;

/*  FIXME, TODO
    if (!XAllocColor(d,cmp,xc))
        cout << "wxAllocNearestColor : Warning : Cannot find nearest color !\n";
*/

    delete[] color_defs;
#endif
}

void wxAllocColor(Display *d,Colormap cmp,XColor *xc)
{
    if (!XAllocColor(d,cmp,xc))
    {
        //          cout << "wxAllocColor : Warning : Can not allocate color, attempt find nearest !\n";
        wxAllocNearestColor(d,cmp,xc);
    }
}

#ifdef __WXDEBUG__
wxString wxGetXEventName(XEvent& event)
{
#if wxUSE_NANOX
    wxString str(wxT("(some event)"));
    return str;
#else
    int type = event.xany.type;
    static char* event_name[] = {
        wxMOTIF_STR(""), wxMOTIF_STR("unknown(-)"),                                         // 0-1
        wxMOTIF_STR("KeyPress"), wxMOTIF_STR("KeyRelease"), wxMOTIF_STR("ButtonPress"), wxMOTIF_STR("ButtonRelease"), // 2-5
        wxMOTIF_STR("MotionNotify"), wxMOTIF_STR("EnterNotify"), wxMOTIF_STR("LeaveNotify"), wxMOTIF_STR("FocusIn"),  // 6-9
        wxMOTIF_STR("FocusOut"), wxMOTIF_STR("KeymapNotify"), wxMOTIF_STR("Expose"), wxMOTIF_STR("GraphicsExpose"),   // 10-13
        wxMOTIF_STR("NoExpose"), wxMOTIF_STR("VisibilityNotify"), wxMOTIF_STR("CreateNotify"),           // 14-16
        wxMOTIF_STR("DestroyNotify"), wxMOTIF_STR("UnmapNotify"), wxMOTIF_STR("MapNotify"), wxMOTIF_STR("MapRequest"),// 17-20
        wxMOTIF_STR("ReparentNotify"), wxMOTIF_STR("ConfigureNotify"), wxMOTIF_STR("ConfigureRequest"),  // 21-23
        wxMOTIF_STR("GravityNotify"), wxMOTIF_STR("ResizeRequest"), wxMOTIF_STR("CirculateNotify"),      // 24-26
        wxMOTIF_STR("CirculateRequest"), wxMOTIF_STR("PropertyNotify"), wxMOTIF_STR("SelectionClear"),   // 27-29
        wxMOTIF_STR("SelectionRequest"), wxMOTIF_STR("SelectionNotify"), wxMOTIF_STR("ColormapNotify"),  // 30-32
        wxMOTIF_STR("ClientMessage"), wxMOTIF_STR("MappingNotify"),                         // 33-34
        wxMOTIF_STR("unknown(+)")};                                            // 35
    type = wxMin(35, type); type = wxMax(1, type);
    wxString str(event_name[type]);
    return str;
#endif
}
#endif

// ----------------------------------------------------------------------------
// accelerators
// ----------------------------------------------------------------------------

// Find the letter corresponding to the mnemonic, for Motif
char wxFindMnemonic (const char *s)
{
    char mnem = 0;
    int len = strlen (s);
    int i;

    for (i = 0; i < len; i++)
    {
        if (s[i] == '&')
        {
            // Carefully handle &&
            if ((i + 1) <= len && s[i + 1] == '&')
                i++;
            else
            {
                mnem = s[i + 1];
                break;
            }
        }
    }
    return mnem;
}

char* wxFindAccelerator( const char *s )
{
#if 1
    wxUnusedVar(s);
    // VZ: this function returns incorrect keysym which completely breaks kbd
    //     handling
    return NULL;
#else
    // The accelerator text is after the \t char.
    s = strchr( s, '\t' );

    if( !s ) return NULL;

    /*
    Now we need to format it as X standard:

      input            output

        F7           --> <Key>F7
        Ctrl+N       --> Ctrl<Key>N
        Alt+k        --> Meta<Key>k
        Ctrl+Shift+A --> Ctrl Shift<Key>A

        and handle Ctrl-N & similia
    */

    static char buf[256];

    buf[0] = '\0';
    wxString tmp = s + 1; // skip TAB
    size_t index = 0;

    while( index < tmp.length() )
    {
        size_t plus  = tmp.find( '+', index );
        size_t minus = tmp.find( '-', index );

        // neither '+' nor '-', add <Key>
        if( plus == wxString::npos && minus == wxString::npos )
        {
            strcat( buf, "<Key>" );
            strcat( buf, tmp.c_str() + index );

            return buf;
        }

        // OK: npos is big and positive
        size_t sep = wxMin( plus, minus );
        wxString mod = tmp.substr( index, sep - index );

        // Ctrl  -> Ctrl
        // Shift -> Shift
        // Alt   -> Meta
        if( mod == "Alt" )
            mod = "Meta";

        if( buf[0] )
            strcat( buf, " " );

        strcat( buf, mod.c_str() );

        index = sep + 1;
    }

    return NULL;
#endif
}

XmString wxFindAcceleratorText (const char *s)
{
#if 1
    wxUnusedVar(s);
    // VZ: this function returns incorrect keysym which completely breaks kbd
    //     handling
    return NULL;
#else
    // The accelerator text is after the \t char.
    s = strchr( s, '\t' );

    if( !s ) return NULL;

    return wxStringToXmString( s + 1 ); // skip TAB!
#endif
}

// Change a widget's foreground and background colours.
void wxDoChangeForegroundColour(WXWidget widget, wxColour& foregroundColour)
{
    // When should we specify the foreground, if it's calculated
    // by wxComputeColours?
    // Solution: say we start with the default (computed) foreground colour.
    // If we call SetForegroundColour explicitly for a control or window,
    // then the foreground is changed.
    // Therefore SetBackgroundColour computes the foreground colour, and
    // SetForegroundColour changes the foreground colour. The ordering is
    // important.

    XtVaSetValues ((Widget) widget,
        XmNforeground, foregroundColour.AllocColour(XtDisplay((Widget) widget)),
        NULL);
}

void wxDoChangeBackgroundColour(WXWidget widget, const wxColour& backgroundColour, bool changeArmColour)
{
    wxComputeColours (XtDisplay((Widget) widget), & backgroundColour,
        (wxColour*) NULL);

    XtVaSetValues ((Widget) widget,
        XmNbackground, g_itemColors[wxBACK_INDEX].pixel,
        XmNtopShadowColor, g_itemColors[wxTOPS_INDEX].pixel,
        XmNbottomShadowColor, g_itemColors[wxBOTS_INDEX].pixel,
        XmNforeground, g_itemColors[wxFORE_INDEX].pixel,
        NULL);

    if (changeArmColour)
        XtVaSetValues ((Widget) widget,
        XmNarmColor, g_itemColors[wxSELE_INDEX].pixel,
        NULL);
}

extern void wxDoChangeFont(WXWidget widget, const wxFont& font)
{
    // Lesstif 0.87 hangs here, but 0.93 does not; MBN: sometimes it does
#if !wxCHECK_LESSTIF() // || wxCHECK_LESSTIF_VERSION( 0, 93 )
    Widget w = (Widget)widget;
    XtVaSetValues( w,
                   wxFont::GetFontTag(), font.GetFontTypeC( XtDisplay(w) ),
                   NULL );
#else
    wxUnusedVar(widget);
    wxUnusedVar(font);
#endif

}

wxString wxXmStringToString( const XmString& xmString )
{
    char *txt;
    if( XmStringGetLtoR( xmString, XmSTRING_DEFAULT_CHARSET, &txt ) )
    {
        wxString str(txt);
        XtFree (txt);
        return str;
    }

    return wxEmptyString;
}

XmString wxStringToXmString( const wxString& str )
{
    return XmStringCreateLtoR((char *)str.c_str(), XmSTRING_DEFAULT_CHARSET);
}

XmString wxStringToXmString( const char* str )
{
    return XmStringCreateLtoR((char *)str, XmSTRING_DEFAULT_CHARSET);
}

// ----------------------------------------------------------------------------
// wxBitmap utility functions
// ----------------------------------------------------------------------------

// Creates a bitmap with transparent areas drawn in
// the given colour.
wxBitmap wxCreateMaskedBitmap(const wxBitmap& bitmap, const wxColour& colour)
{
    wxBitmap newBitmap(bitmap.GetWidth(),
                       bitmap.GetHeight(),
                       bitmap.GetDepth());
    wxMemoryDC destDC;
    wxMemoryDC srcDC;

    srcDC.SelectObjectAsSource(bitmap);
    destDC.SelectObject(newBitmap);

    wxBrush brush(colour, wxSOLID);
    destDC.SetBackground(brush);
    destDC.Clear();
    destDC.Blit(0, 0, bitmap.GetWidth(), bitmap.GetHeight(),
                &srcDC, 0, 0, wxCOPY, true);

    return newBitmap;
}

// ----------------------------------------------------------------------------
// Miscellaneous functions
// ----------------------------------------------------------------------------

WXWidget wxCreateBorderWidget( WXWidget parent, long style )
{
    Widget borderWidget = (Widget)NULL, parentWidget = (Widget)parent;

    if (style & wxSIMPLE_BORDER)
    {
        borderWidget = XtVaCreateManagedWidget
                                   (
                                    "simpleBorder",
                                    xmFrameWidgetClass, parentWidget,
                                    XmNshadowType, XmSHADOW_ETCHED_IN,
                                    XmNshadowThickness, 1,
                                    NULL
                                   );
    }
    else if (style & wxSUNKEN_BORDER)
    {
        borderWidget = XtVaCreateManagedWidget
                                   (
                                    "sunkenBorder",
                                    xmFrameWidgetClass, parentWidget,
                                    XmNshadowType, XmSHADOW_IN,
                                    NULL
                                   );
    }
    else if (style & wxRAISED_BORDER)
    {
        borderWidget = XtVaCreateManagedWidget
                                   (
                                    "raisedBorder",
                                    xmFrameWidgetClass, parentWidget,
                                    XmNshadowType, XmSHADOW_OUT,
                                    NULL
                                   );
    }

    return borderWidget;
}
