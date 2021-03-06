/***********************************************************************
**
**   vla_support.cpp
**
**   This file is part of libkfrgcs.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht,
**                           Garrecht Ingenieurgesellschaft
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef VLA_SUPPORT
#define VLA_SUPPORT

#include "vlapityp.h"
#include "vlapierr.h"

#include <stddef.h>

class Port;
class OperationEnvironment;

/*
	VLA_SYS contains target system dependent primitives upon which
	the subclasses rely.
	Implement this functions according to	your target system.
	A sample implementation for Win32(R) can be found in file
		"VLAPI2SYS_WIN32.CPP"
*/
class VLA_SYS {
protected:
  //
  Port *port;

 VLA_SYS(Port &_port):port(&_port) {}
};


/*
	This class contains target system independent base functions
	for communication with the VOLKSLOGGER.
	These functions are critical and normally don't need to be changed
	by the user. If you think that something has to be changed here,
	contact GARRECHT for comments on your intended changes.
*/
class VLA_XFR : protected VLA_SYS {
protected:
  OperationEnvironment &env;

  int32 databaud; // Baudrate as integer (e.g. 115200)

  VLA_XFR(Port &port, OperationEnvironment &env);
  void set_databaud(int32 db);

	// establish connection with VL within specified time
  VLA_ERROR connect(int32 waittime);

	VLA_ERROR readinfo(lpb buffer, int32 buffersize);

  // write binary DATABASE-block to VL
  VLA_ERROR dbbput(lpb dbbbuffer, int32 dbbsize);
  // read binary DATABASE from VL
  VLA_ERROR dbbget(lpb dbbbuffer, int32 dbbsize);
  // read all binary flight logs from VL
  VLA_ERROR all_logsget(lpb dbbbuffer, int32 dbbsize);
  // read one binary flight log from VL
  int32 flightget(lpb buffer, int32 buffersize, int16 flightnr, int16 secmode);
  VLA_ERROR readdir(lpb buffer, int32 buffersize);
};

#endif
