/*
 * Copyright (c) 2002, Robert Collins.
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     A copy of the GNU General Public License can be found at
 *     http://www.gnu.org/
 *
 * Written by Robert Collins  <rbtcollins@hotmail.com>
 *
 */

#ifndef _INISTATE_H_
#define _INISTATE_H_

/* To parse this */
#include "String++.h"

class IniState
{
public:
  IniState() : timestamp (0), version() {}
  unsigned int timestamp;
  String version;
};

#endif /* _INISTATE_H_ */
