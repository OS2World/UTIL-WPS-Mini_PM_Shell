// create.c

// called from WM_CREATE in main app's client wm_create;
// allocates storage for globals and starts filling them in;
// starts thread 2 for the object window to perform lengthy tasks;

#define INCL_DOS
#define INCL_PM
#include <os2.h>

// c includes
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <process.h>

// app includes
#include "def.h"
#include "mshell.h"

#ifdef DEBUG
#include "pmassert.h"
#endif


// -------------------------------------------------------------------------
// called from WM_CREATE in client win proc

PGLOBALS Create( HWND hwnd )
{
    BOOL      bOK;
    BOOL      bNeedDefaults;
    HAB       hab;
    PGLOBALS  pg;
    int       idThread;
    ULONG     rc;
    ULONG     ul;


    // query hab
    hab = WinQueryAnchorBlock( hwnd );

    // allocate global memory; see GLOBALS struct in MSHELL.H
    pg = malloc( sizeof( GLOBALS ));

#ifdef DEBUG
    pmassert( hab, pg );
#endif

    memset( pg, 0, sizeof( GLOBALS ));

    // set globals pointer into window words
    bOK = WinSetWindowULong( hwnd, QWL_USER, (ULONG) pg );

#ifdef DEBUG
    pmassert( hab, bOK  );
#endif

    // store window handles etc in globals
    pg->hwndClient   = hwnd;
    pg->hab          = hab;
    pg->hwndFrame    = WinQueryWindow( hwnd, QW_PARENT );

#ifdef DEBUG
    pmassert( hab, pg->hwndFrame );
#endif

    pg->hwndTitlebar = WinWindowFromID( pg->hwndFrame, FID_TITLEBAR );

#ifdef DEBUG
    pmassert( hab, pg->hwndTitlebar );
#endif

    pg->hwndMenubar  = WinWindowFromID( pg->hwndFrame, FID_MENU );

#ifdef DEBUG
    pmassert( hab, pg->hwndMenubar );
#endif

    // set text into title bar
    bOK = WinSetWindowText( pg->hwndTitlebar, CAPTION );

#ifdef DEBUG
    pmassert( hab, bOK   );
#endif

    // query size of desktop
    WinQueryWindowRect( HWND_DESKTOP, &(pg->rectlDesktop) );

    // disable client and menubar until application completely initializes.
    // these windows will reenable when object window is done with its WM_CREATE
    WinSendMsg( hwnd, WM_USER_DISABLE_CLIENT, 0, 0 );

    // create a listbox, child of the client window
    pg->hwndListbox = WinCreateWindow( hwnd,
                                       WC_LISTBOX,
                                       "",
                                       WS_VISIBLE ,
                                       0,0,0,0,
                                       hwnd,
                                       HWND_TOP,
                                       ID_LISTBOX,
                                       NULL,
                                       NULL );
#ifdef DEBUG
    pmassert( hab, pg->hwndListbox );
#endif

    // listbox must have focus (else doesn't work on mouseless systems)
    WinSetFocus( HWND_DESKTOP, pg->hwndListbox );

    // create a modeless dialog for the spooler interface dialog
    pg->hwndSpooler = (HWND) WinLoadDlg( HWND_DESKTOP, HWND_DESKTOP, (PFNWP) SpoolerDlgProc, (HMODULE)0,  IDD_SPOOLER, (PVOID) pg );

#ifdef DEBUG
    pmassert( hab, pg->hwndSpooler );
#endif

    // read the saved profile for size and position
    bOK = PrfQueryProfileSize( HINI_PROFILE, INI_APP, INIKEY_PROFILE, &ul );
    bNeedDefaults = FALSE;

#ifdef DEBUG
    dbprintf(( "Profile size is %d\n", ul ));
#endif

    if( !bOK  ||  ul != sizeof( PROFILE )) {
        // set up profile defaults
        bNeedDefaults = TRUE;
    } else {
        ul = sizeof( PROFILE );
        bOK = PrfQueryProfileData( HINI_PROFILE, INI_APP, INIKEY_PROFILE, (PVOID)&(pg->profile), &ul );

#ifdef DEBUG
        pmassert( hab, bOK );
#endif

        // sanity checks on profile data
        bOK   = ( 0 < pg->profile.swp.x   &&   pg->profile.swp.x < pg->rectlDesktop.xRight  );
        bOK  &= ( 0 < pg->profile.swp.y   &&   pg->profile.swp.y < pg->rectlDesktop.yTop    );
        bOK  &= ( pg->profile.swp.x + pg->profile.swp.cx  < pg->rectlDesktop.xRight  );
        bOK  &= ( pg->profile.swp.y + pg->profile.swp.cy  < pg->rectlDesktop.yTop    );
        bNeedDefaults = !bOK;

#ifdef DEBUG
        if( !bOK  ) {
            dbprintf(( "sanity checks failed\n" ));
        }
#endif

    }

    if( bNeedDefaults ) {

#ifdef DEBUG
        dbprintf(( "Using defaults from WinQueryTaskSizePos\n" ));
#endif

        rc = WinQueryTaskSizePos( pg->hab, 0, &(pg->profile.swp) );

#ifdef DEBUG
        pmassert( hab, rc == 0 );
#endif

    }

    // create the invisible object window operated by thread 2
    // perform any lengthy tasks on thread 2 by posting a work msg to object window
    // see WM_CREATE case in OBJECT.C which continues initialization
    idThread = _beginthread( threadmain, NULL, LEN_STACK, (void*)pg );

#ifdef DEBUG
    pmassert( hab, idThread );
#endif

    // return the pointer to globals
    return pg;
}

