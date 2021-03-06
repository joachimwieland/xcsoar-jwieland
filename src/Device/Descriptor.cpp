/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Device/Descriptor.hpp"
#include "Device/Driver.hpp"
#include "Device/Parser.hpp"
#include "Device/FLARM.hpp"
#include "DeviceBlackboard.hpp"
#include "NMEA/Info.hpp"
#include "Thread/Mutex.hpp"
#include "StringUtil.hpp"
#include "Logger/NMEALogger.hpp"
#include "Language/Language.hpp"
#include "Operation.hpp"
#include "OS/Clock.hpp"

#ifdef ANDROID
#include "Android/InternalGPS.hpp"
#endif

#include <assert.h>

DeviceDescriptor::DeviceDescriptor()
  :Com(NULL), pDevPipeTo(NULL),
   Driver(NULL), device(NULL),
#ifdef ANDROID
   internal_gps(NULL),
#endif
   ticker(false), busy(false)
{
}

DeviceDescriptor::~DeviceDescriptor()
{
  assert(!busy);
}

bool
DeviceDescriptor::Open(const DeviceConfig &config, Port *_port,
                       const struct DeviceRegister *_driver)
{
  assert(_port != NULL);
  assert(_driver != NULL);
  assert(Com == NULL);
  assert(device == NULL);
  assert(!ticker);

  device_blackboard.mutex.Lock();
  device_blackboard.SetRealState(index).Reset();
  device_blackboard.ScheduleMerge();
  device_blackboard.mutex.Unlock();

  settings_sent.Clear();
  settings_received.Clear();

  Com = _port;
  Driver = _driver;

  assert(Driver->CreateOnPort != NULL || Driver->IsNMEAOut());
  if (Driver->CreateOnPort == NULL)
    return true;

  parser.Reset();
  parser.SetReal(_tcscmp(Driver->name, _T("Condor")) != 0);

  device = Driver->CreateOnPort(config, Com);
  QuietOperationEnvironment env;
  if (!device->Open(env) || !Com->StartRxThread()) {
    delete device;
    device = NULL;
    Com = NULL;
    return false;
  }

  return true;
}

void
DeviceDescriptor::Close()
{
#ifdef ANDROID
  delete internal_gps;
  internal_gps = NULL;
#endif

  delete device;
  device = NULL;

  Port *OldCom = Com;
  Com = NULL;

  delete OldCom;

  Driver = NULL;
  pDevPipeTo = NULL;
  ticker = false;

  device_blackboard.mutex.Lock();
  device_blackboard.SetRealState(index).Reset();
  device_blackboard.ScheduleMerge();
  device_blackboard.mutex.Unlock();

  settings_sent.Clear();
  settings_received.Clear();
}

const TCHAR *
DeviceDescriptor::GetDisplayName() const
{
  return Driver != NULL ? Driver->display_name : NULL;
}

bool
DeviceDescriptor::IsDriver(const TCHAR *name) const
{
  return Driver != NULL
    ? _tcscmp(Driver->name, name) == 0
    : false;
}

bool
DeviceDescriptor::CanDeclare() const
{
  return Driver != NULL &&
    (Driver->CanDeclare() ||
     parser.isFlarm);
}

bool
DeviceDescriptor::IsLogger() const
{
  return Driver != NULL && Driver->IsLogger();
}

bool
DeviceDescriptor::ParseNMEA(const char *line, NMEA_INFO &info)
{
  assert(line != NULL);

  /* restore the driver's ExternalSettings */
  const ExternalSettings old_settings = info.settings;
  info.settings = settings_received;

  if (device != NULL && device->ParseNMEA(line, info)) {
    info.Connected.Update(fixed(MonotonicClockMS()) / 1000);

    /* clear the settings when the values are the same that we already
       sent to the device */
    const ExternalSettings old_received = settings_received;
    settings_received = info.settings;
    info.settings.EliminateRedundant(settings_sent, old_received);

    return true;
  }

  /* no change - restore the ExternalSettings that we returned last
     time */
  info.settings = old_settings;

  // Additional "if" to find GPS strings
  if (parser.ParseNMEAString_Internal(line, info)) {
    info.Connected.Update(fixed(MonotonicClockMS()) / 1000);
    return true;
  }

  return false;
}

bool
DeviceDescriptor::PutMacCready(fixed value)
{
  if (device == NULL || settings_sent.CompareMacCready(value))
    return true;

  if (!device->PutMacCready(value))
    return false;

  ScopeLock protect(device_blackboard.mutex);
  NMEA_INFO &basic = device_blackboard.SetRealState(index);
  settings_sent.mac_cready = value;
  settings_sent.mac_cready_available.Update(basic.clock);

  return true;
}

bool
DeviceDescriptor::PutBugs(fixed value)
{
  if (device == NULL || settings_sent.CompareBugs(value))
    return true;

  if (!device->PutBugs(value))
    return false;

  ScopeLock protect(device_blackboard.mutex);
  NMEA_INFO &basic = device_blackboard.SetRealState(index);
  settings_sent.bugs = value;
  settings_sent.bugs_available.Update(basic.clock);

  return true;
}

bool
DeviceDescriptor::PutBallast(fixed value)
{
  if (device == NULL || settings_sent.CompareBallastFraction(value))
    return true;

  if (!device->PutBallast(value))
    return false;

  ScopeLock protect(device_blackboard.mutex);
  NMEA_INFO &basic = device_blackboard.SetRealState(index);
  settings_sent.ballast_fraction = value;
  settings_sent.ballast_fraction_available.Update(basic.clock);

  return true;
}

bool
DeviceDescriptor::PutVolume(int volume)
{
  return device != NULL ? device->PutVolume(volume) : true;
}

bool
DeviceDescriptor::PutActiveFrequency(RadioFrequency frequency)
{
  return device != NULL ? device->PutActiveFrequency(frequency) : true;
}

bool
DeviceDescriptor::PutStandbyFrequency(RadioFrequency frequency)
{
  return device != NULL ? device->PutStandbyFrequency(frequency) : true;
}

bool
DeviceDescriptor::PutQNH(const AtmosphericPressure &value,
                         const DERIVED_INFO &calculated)
{
  if (device == NULL || settings_sent.CompareQNH(value.get_QNH()))
    return true;

  if (!device->PutQNH(value, calculated))
    return false;

  ScopeLock protect(device_blackboard.mutex);
  NMEA_INFO &basic = device_blackboard.SetRealState(index);
  settings_sent.qnh = value;
  settings_sent.qnh_available.Update(basic.clock);

  return true;
}

bool
DeviceDescriptor::PutVoice(const TCHAR *sentence)
{
  return device != NULL ? device->PutVoice(sentence) : true;
}

void
DeviceDescriptor::LinkTimeout()
{
  assert(!busy);

  if (device != NULL)
    device->LinkTimeout();
}

bool
DeviceDescriptor::Declare(const struct Declaration &declaration,
                          OperationEnvironment &env)
{
  if (Com == NULL)
    return false;

  SetBusy(true);

  TCHAR text[60];

  _stprintf(text, _("Declaring to %s"), Driver->display_name);
  env.SetText(text);

  Com->StopRxThread();

  bool result = device != NULL && device->Declare(declaration, env);

  if (parser.isFlarm) {
    _stprintf(text, _("Declaring to %s"), _T("FLARM"));
    env.SetText(text);
    result = FlarmDeclare(Com, declaration, env) || result;
  }

  Com->StartRxThread();

  SetBusy(false);
  return result;
}

bool
DeviceDescriptor::ReadFlightList(RecordedFlightList &flight_list,
                                 OperationEnvironment &env)
{
  if (Com == NULL || Driver == NULL || device == NULL)
    return false;

  TCHAR text[60];
  _stprintf(text, _("Reading flight list from %s"), Driver->display_name);
  env.SetText(text);

  Com->StopRxThread();
  bool result = device->ReadFlightList(flight_list, env);
  Com->StartRxThread();
  return result;
}

bool
DeviceDescriptor::DownloadFlight(const RecordedFlightInfo &flight,
                                 const TCHAR *path,
                                 OperationEnvironment &env)
{
  if (Driver == NULL || device == NULL)
    return false;

  TCHAR text[60];
  _stprintf(text, _("Downloading flight from %s"), Driver->display_name);
  env.SetText(text);

  Com->StopRxThread();
  bool result = device->DownloadFlight(flight, path, env);
  Com->StartRxThread();
  return result;
}

void
DeviceDescriptor::OnSysTicker(const NMEA_INFO &basic,
                              const DERIVED_INFO &calculated)
{
  if (device == NULL || IsBusy())
    return;

  ticker = !ticker;
  if (ticker)
    // write settings to vario every second
    device->OnSysTicker(basic, calculated);
}

void
DeviceDescriptor::LineReceived(const char *line)
{
  LogNMEA(line);

  if (pDevPipeTo && pDevPipeTo->Com) {
    // stream pipe, pass nmea to other device (NmeaOut)
    // TODO code: check TX buffer usage and skip it if buffer is full (outbaudrate < inbaudrate)
    pDevPipeTo->Com->Write(line);
    pDevPipeTo->Com->Write("\r\n");
  }

  ScopeLock protect(device_blackboard.mutex);
  NMEA_INFO &basic = device_blackboard.SetRealState(index);
  basic.UpdateClock();
  if (ParseNMEA(line, basic))
    device_blackboard.ScheduleMerge();
}
