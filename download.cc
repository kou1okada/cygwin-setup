/*
 * Copyright (c) 2000, 2001, Red Hat, Inc.
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     A copy of the GNU General Public License can be found at
 *     http://www.gnu.org/
 *
 * Written by DJ Delorie <dj@cygnus.com>
 *
 */

/* The purpose of this file is to download all the files we need to
   do the installation. */

#if 0
static const char *cvsid =
  "\n%%% $Id$\n";
#endif

#include "download.h"
  
#include "win32.h"

#include <stdio.h>
#include <unistd.h>
#include <process.h>

#include "resource.h"
#include "msg.h"
#include "dialog.h"
#include "String++.h"
#include "geturl.h"
#include "state.h"
#include "LogSingleton.h"
#include "filemanip.h"

#include "io_stream.h"

#include "package_db.h"
#include "package_meta.h"
#include "package_version.h"
#include "package_source.h"

#include "rfc1738.h"

#include "threebar.h"

#include "libmd5-rfc/md5.h"

#include "Exception.h"

#include "getopt++/BoolOption.h"

using namespace std;

extern ThreeBarProgressPage Progress;

static BoolOption NoMD5Option (false, '5', "no-md5", "Suppress MD5 checksum verification");

static bool
validateCachedPackage (String const &fullname, packagesource & pkgsource)
{
  DWORD size = get_file_size(fullname);
  if (size != pkgsource.size)
  {
    log (LOG_BABBLE) << "INVALID PACKAGE: " << fullname
      << " - Size mismatch: Ini-file: " << pkgsource.size
      << " != On-disk: " << size << endLog;
    return false;
  }
  if (pkgsource.md5.isSet() && !NoMD5Option)
    {
      // check the MD5 sum of the cached file here
      io_stream *thefile = io_stream::open (fullname, "rb");
      if (!thefile)
	return 0;
      md5_state_t pns;
      md5_init (&pns);

      log (LOG_BABBLE) << "Checking MD5 for " << fullname << endLog;

      Progress.SetText1 ((String ("Checking MD5 for ") + pkgsource.Base()).cstr_oneuse());
      Progress.SetText4 ("Progress:");
      Progress.SetBar1 (0);
      
      unsigned char buffer[16384];
      ssize_t count;
      while ((count = thefile->read (buffer, 16384)) > 0)
	{
	  md5_append (&pns, buffer, count);
	  Progress.SetBar1 (thefile->tell(), thefile->get_size());
	}
      delete thefile;
      if (count < 0)
        throw new Exception (TOSTRING(__LINE__) " " __FILE__,
                             String ("IO Error reading ") + fullname,
                             APPERR_IO_ERROR);
      
      md5_byte_t tempdigest[16];
      md5_finish(&pns, tempdigest);
      md5 tempMD5;
      tempMD5.set (tempdigest);
      
      if (pkgsource.md5 != tempMD5)
      {
        log (LOG_BABBLE) << "INVALID PACKAGE: " << fullname
          << " - MD5 mismatch: Ini-file: " << pkgsource.md5.print() 
          << " != On-disk: " << tempMD5.print() << endLog;
        return false;
      }

      log (LOG_BABBLE) << "MD5 verified OK: " << fullname
        << pkgsource.md5.print() << endLog;
    }
  return true;
}

/* 0 on failure
 */
int
check_for_cached (packagesource & pkgsource)
{
  /* search algo:
     1) is there a legacy version in the cache dir available.
     (Note that the cache dir is represented by a mirror site of
     file://local_dir
   */

  // Already found one.
  if (pkgsource.Cached())
    return 1;
  
  String prefix = String ("file://") + local_dir +  "/";
  String fullname = prefix + pkgsource.Canonical();
  if (io_stream::exists(fullname))
  {
    if (get_file_size(fullname) != pkgsource.size)
    {
      log (LOG_PLAIN) << "Silently skipping wrong-sized package " << fullname
        << endLog;
    }
    else
    {
      if (validateCachedPackage (fullname, pkgsource))
        pkgsource.set_cached (fullname);
      else
        throw new Exception (TOSTRING(__LINE__) " " __FILE__,
                             String ("Package validation failure for ")
                             + fullname,
                             APPERR_CORRUPT_PACKAGE);
      return 1;
    }
  }

  /*
     2) is there a version from one of the selected mirror sites available ?
     */
  for (packagesource::sitestype::const_iterator n = pkgsource.sites.begin();
       n != pkgsource.sites.end(); ++n)
  {
    String fullname = prefix + rfc1738_escape_part (n->key) + "/" +
      pkgsource.Canonical ();
    if (io_stream::exists(fullname))
    {
      if (get_file_size(fullname) != pkgsource.size)
      {
        log (LOG_PLAIN) << "Silently skipping wrong-sized package " << fullname
          << endLog;
      }
      else
      {
        if (validateCachedPackage (fullname, pkgsource))
          pkgsource.set_cached (fullname);
        else
          throw new Exception (TOSTRING(__LINE__) " " __FILE__,
                               String ("Package validation failure for ")
                               + fullname,
                               APPERR_CORRUPT_PACKAGE);
        return 1;
      }
    }
  }
  return 0;
}

/* download a file from a mirror site to the local cache. */
static int
download_one (packagesource & pkgsource, HWND owner)
{
  try
    {
      if (check_for_cached (pkgsource))
        return 0;
    }
  catch (Exception * e)
    {
      // We know what to do with these..
      if (e->errNo() == APPERR_CORRUPT_PACKAGE)
	{
	  fatal (owner, IDS_CORRUPT_PACKAGE, pkgsource.Canonical());
    	  return 1;
	}
      // Unexpected exception.
      throw e;
    }
  /* try the download sites one after another */

  int success = 0;
  for (packagesource::sitestype::const_iterator n = pkgsource.sites.begin();
       n != pkgsource.sites.end() && !success; ++n)
    {
      String const local = local_dir + "/" +
				  rfc1738_escape_part (n->key) + "/" +
				  pkgsource.Canonical ();
      io_stream::mkpath_p (PATH_TO_FILE, String ("file://") + local);

      if (get_url_to_file(n->key +  "/" + pkgsource.Canonical (),
			  local + ".tmp", pkgsource.size, owner))
	{
	  /* FIXME: note new source ? */
	  continue;
	}
      else
	{
	  size_t size = get_file_size (String("file://") + local + ".tmp");
	  if (size == pkgsource.size)
	    {
	      log (LOG_PLAIN) << "Downloaded " << local << endLog;
	      if (_access (local.cstr_oneuse(), 0) == 0)
		remove (local.cstr_oneuse());
	      rename ((local + ".tmp").cstr_oneuse(), local.cstr_oneuse());
	      success = 1;
	      pkgsource.set_cached (String ("file://") + local);
	      // FIXME: move the downloaded file to the 
	      //  original locations - without the mirror site dir in the way
	      continue;
	    }
	  else
	    {
	      log (LOG_PLAIN) << "Download " << local << " wrong size (" <<
		size << " actual vs " << pkgsource.size << " expected)" << 
		endLog;
	      remove ((local + ".tmp").cstr_oneuse());
	      continue;
	    }
	}
    }
  if (success)
    return 0;
  /* FIXME: Do we want to note this? if so how? */
  return 1;
}

static int
do_download_thread (HINSTANCE h, HWND owner)
{
  int errors = 0;
  total_download_bytes = 0;
  total_download_bytes_sofar = 0;

  packagedb db;
  /* calculate the amount needed */
  for (vector <packagemeta *>::iterator i = db.packages.begin ();
       i != db.packages.end (); ++i)
    {
      packagemeta & pkg = **i;
      if (pkg.desired.changeRequested())
	{
	  packageversion version = pkg.desired;
	  packageversion sourceversion = version.sourcePackage();
	  try 
	    {
    	      if (version.picked())
		{
		  for (vector<packagesource>::iterator i = 
		       version.sources ()->begin(); 
		       i != version.sources ()->end(); ++i)
		    if (!check_for_cached (*i))
      		      total_download_bytes += i->size;
		}
    	      if (sourceversion.picked ())
		{
		  for (vector<packagesource>::iterator i =
		       sourceversion.sources ()->begin();
		       i != sourceversion.sources ()->end(); ++i)
		    if (!check_for_cached (*i))
		      total_download_bytes += i->size;
		}
	    }
	  catch (Exception * e)
	    {
	      // We know what to do with these..
	      if (e->errNo() == APPERR_CORRUPT_PACKAGE)
		fatal (owner, IDS_CORRUPT_PACKAGE, pkg.name.cstr_oneuse());
	      // Unexpected exception.
	      throw e;
	    }
	}
    }

  /* and do the download. FIXME: This here we assign a new name for the cached version
   * and check that above.
   */
  for (vector <packagemeta *>::iterator i = db.packages.begin ();
       i != db.packages.end (); ++i)
    {
      packagemeta & pkg = **i;
      if (pkg.desired.changeRequested())
	{
	  int e = 0;
	  packageversion version = pkg.desired;
	  packageversion sourceversion = version.sourcePackage();
	  if (version.picked())
	    {
	      for (vector<packagesource>::iterator i =
   		   version.sources ()->begin();
		   i != version.sources ()->end(); ++i)
    		e += download_one (*i, owner);
	    }
	  if (sourceversion && sourceversion.picked())
	    {
	      for (vector<packagesource>::iterator i =
   		   sourceversion.sources ()->begin();
		   i != sourceversion.sources ()->end(); ++i)
    		e += download_one (*i, owner);
	    }
	  errors += e;
#if 0
	  if (e)
	    pkg->action = ACTION_ERROR;
#endif
	}
    }

  if (errors)
    {
      if (yesno (owner, IDS_DOWNLOAD_INCOMPLETE) == IDYES)
	{
	  return IDD_SITE;
	}
    }

  if (source == IDC_SOURCE_DOWNLOAD)
    {
      if (errors)
	exit_msg = IDS_DOWNLOAD_INCOMPLETE;
      else if (!unattended_mode)
	exit_msg = IDS_DOWNLOAD_COMPLETE;
      return 0;
    }
  else
    return IDD_S_INSTALL;
}

static DWORD WINAPI
do_download_reflector (void *p)
{
  HANDLE *context;
  context = (HANDLE *) p;

  try
  {
    int next_dialog =
      do_download_thread ((HINSTANCE) context[0], (HWND) context[1]);

    // Tell the progress page that we're done downloading
    Progress.PostMessage (WM_APP_DOWNLOAD_THREAD_COMPLETE, 0, next_dialog);
  }
  TOPLEVEL_CATCH("download");

  ExitThread(0);
}

static HANDLE context[2];

void
do_download (HINSTANCE h, HWND owner)
{
  context[0] = h;
  context[1] = owner;

  DWORD threadID;
  CreateThread (NULL, 0, do_download_reflector, context, 0, &threadID);
}
