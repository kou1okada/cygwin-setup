/*
 * Copyright (c) 2000, Red Hat, Inc.
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     A copy of the GNU General Public License can be found at
 *     http://www.gnu.org/
 *
 * Written by Andrej Borsenkow <Andrej.Borsenkow@mow.siemens.ru>
 * based on work and suggestions of DJ Delorie
 *
 */

/* The purpose of this file is to ask the user where they want the
   root of the installation to be, and to ask whether the user prefers
   text or binary mounts. */

static char *cvsid =
  "\n%%% $Id$\n";

#include "win32.h"
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "dialog.h"
#include "resource.h"
#include "state.h"
#include "msg.h"
#include "mount.h"
#include "concat.h"
#include "log.h"
#include "mkdir.h"

#include "port.h"

static void
get_root_dir_now ()
{
  int istext;
  int issystem;
  if (get_root_dir ())
    return;
  read_mounts ();
}

void
save_local_dir ()
{
  get_root_dir_now ();

  mkdir_p (1, local_dir);

  FILE *f;
  if (!get_root_dir ())
    f = fopen ("last-cache", "wb");
  else
    {
      f = fopen (cygpath ("/etc/setup/last-cache", 0), "wb");
      if ( _access ( "last-cache" , 0 ) == 0 && get_root_dir() )
        remove ("last-cache");
    }
  if (!f)
    return;
  fprintf (f, "%s", local_dir);
  fclose (f);
}


static void
check_if_enable_next (HWND h)
{
  EnableWindow (GetDlgItem (h, IDOK), local_dir != 0);
}

static void
load_dialog (HWND h)
{
  eset (h, IDC_LOCAL_DIR, local_dir);
  check_if_enable_next (h);
}

static void
save_dialog (HWND h)
{
  local_dir = eget (h, IDC_LOCAL_DIR, local_dir);
}


static int CALLBACK
browse_cb (HWND h, UINT msg, LPARAM lp, LPARAM data)
{
  switch (msg)
    {
    case BFFM_INITIALIZED:
      if (local_dir)
	SendMessage (h, BFFM_SETSELECTION, TRUE, (LPARAM) local_dir);
      break;
    }
  return 0;
}

static void
browse (HWND h)
{
  BROWSEINFO bi;
  CHAR name[MAX_PATH];
  LPITEMIDLIST pidl;
  memset (&bi, 0, sizeof (bi));
  bi.hwndOwner = h;
  bi.pszDisplayName = name;
  bi.lpszTitle = "Select download directory";
  bi.ulFlags = BIF_RETURNONLYFSDIRS;
  bi.lpfn = browse_cb;
  pidl = SHBrowseForFolder (&bi);
  if (pidl)
    {
      if (SHGetPathFromIDList (pidl, name))
	eset (h, IDC_LOCAL_DIR, name);
    }
}


static BOOL
dialog_cmd (HWND h, int id, HWND hwndctl, UINT code)
{
  switch (id)
    {

    case IDC_LOCAL_DIR:
      save_dialog (h);
      check_if_enable_next (h);
      break;

    case IDC_LOCAL_DIR_BROWSE:
      browse (h);
      break;

    case IDOK:
      save_dialog (h);
      save_local_dir ();
      if (SetCurrentDirectoryA (local_dir))
	{
	  switch (source)
	    {
	    case IDC_SOURCE_DOWNLOAD:
	    case IDC_SOURCE_NETINST:
	      NEXT (IDD_NET);
	      break;
	    case IDC_SOURCE_CWD:
	      NEXT (IDD_S_FROM_CWD);
	      break;
	    default:
	      msg ("source is default? %d\n", source);
	      NEXT (0);
	      break;
	    }
	}
      else
	note (IDS_ERR_CHDIR, local_dir);

      break;

    case IDC_BACK:
      save_dialog (h);
      switch (source)
	{
	case IDC_SOURCE_NETINST:
	case IDC_SOURCE_CWD:
	  NEXT (IDD_ROOT);
	  break;
	case IDC_SOURCE_DOWNLOAD:
	  NEXT (IDD_SOURCE);
	  break;
	default:
	  msg ("source is default? %d\n", source);
	  NEXT (0);
	}
      break;

    case IDCANCEL:
      NEXT (0);
      break;
    }
}

static BOOL CALLBACK
dialog_proc (HWND h, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
    {
    case WM_INITDIALOG:
      load_dialog (h);
      return FALSE;
    case WM_COMMAND:
      return HANDLE_WM_COMMAND (h, wParam, lParam, dialog_cmd);
    }
  return FALSE;
}

extern char cwd[_MAX_PATH];

void
do_local_dir (HINSTANCE h)
{
  static int inited = 0;
  if (!inited)
    {
      FILE *f;
      get_root_dir_now ();
      if (_access ( "last-cache" , 0 ) != 0 && get_root_dir() )
       	f = fopen (cygpath ("/etc/setup/last-cache", 0), "rt");
      else
	f = fopen ("last-cache", "rt");
      if (f)
	{
	  char localdir[1000];
	  localdir[0] = '\0';
	  char *fg_ret = fgets (localdir, 1000, f);
	  fclose (f);
	  if (fg_ret)
	    {
	      free (local_dir);
	      local_dir = strdup (localdir);
	    }
	}
      inited = 1;
    }

  int rv = 0;
  rv = DialogBox (h, MAKEINTRESOURCE (IDD_LOCAL_DIR), 0, dialog_proc);
  if (rv == -1)
    fatal (IDS_DIALOG_FAILED);

  log (0, "Selected local directory: %s", local_dir);
}
