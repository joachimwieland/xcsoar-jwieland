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

#ifndef XCSOAR_NMEA_DERIVED_H
#define XCSOAR_NMEA_DERIVED_H

#include "Math/fixed.hpp"
#include "Navigation/GeoPoint.hpp"
#include "Navigation/SpeedVector.hpp"
#include "Task/TaskStats/TaskStats.hpp"
#include "Task/TaskStats/CommonStats.hpp"
#include "Task/TaskStats/ContestStatistics.hpp"
#include "NMEA/VarioInfo.hpp"
#include "NMEA/ClimbInfo.hpp"
#include "NMEA/CirclingInfo.hpp"
#include "NMEA/ThermalBand.hpp"
#include "NMEA/ThermalLocator.hpp"
#include "NMEA/Validity.hpp"
#include "NMEA/ClimbHistory.hpp"
#include "TeamCodeCalculation.hpp"
#include "Engine/Navigation/Aircraft.hpp"
#include "Engine/Navigation/TraceHistory.hpp"
#include "DateTime.hpp"
#include "Engine/GlideSolvers/GlidePolar.hpp"
#include "Engine/Atmosphere/Pressure.hpp"

#include <tchar.h>

/**
 * Derived terrain altitude information, including glide range
 * 
 */
struct TERRAIN_ALT_INFO
{
  /** True if terrain is valid, False otherwise */
  bool TerrainValid;

  /**
   * Does the attribute #TerrainBase have a valid value?
   */
  bool terrain_base_valid;

  /**
   * Does the attribute #AltitudeAGL have a valid value?
   */
  bool AltitudeAGLValid;

  bool TerrainWarning;

  /** Terrain altitude */
  fixed TerrainAlt;

  /** Lowest height within glide range */
  fixed TerrainBase;

  /** Altitude over terrain */
  fixed AltitudeAGL;

  GeoPoint TerrainWarningLocation;

  void Clear();

  /**
   * Returns the terrain base, and falls back for terrain altitude if
   * the base is not known.
   */
  fixed GetTerrainBaseFallback() const {
    return terrain_base_valid
      ? TerrainBase
      : TerrainAlt;
  }
};

/**
 * Derived team code information
 */
struct TEAMCODE_INFO
{
  /**
   * Are #teammate_vector and #TeammateLocation available?
   */
  bool teammate_available;

  /**
   * Is #flarm_teammate_code available?
   */
  bool flarm_teammate_code_available;

  /**
   * is #flarm_teammate_code current or did we lose him?
   */
  bool flarm_teammate_code_current;

  /** Team code */
  TeamCode OwnTeamCode;

  /** Vector to the chosen team mate */
  GeoVector teammate_vector;

  /** Position of the chosen team mate */
  GeoPoint TeammateLocation;

  /**
   * The team code of the FLARM teammate.
   */
  TeamCode flarm_teammate_code;

  void Clear();
};

struct AirspaceWarningsInfo {
  /**
   * The time stamp of the most recent airspace warning.  Check if
   * this value gets increased to see if there's a new warning.
   */
  Validity latest;

  void Clear();
};

/**
 * A struct that holds all the calculated values derived from the data in the
 * NMEA_INFO struct
 */
struct DERIVED_INFO: 
  public VARIO_INFO,
  public CLIMB_INFO,
  public CIRCLING_INFO,
  public TERRAIN_ALT_INFO,
  public TEAMCODE_INFO
{
  /** GPS date and time (local) */
  BrokenDateTime local_date_time;

  fixed V_stf; /**< Speed to fly block/dolphin (m/s) */

  /** Bearing including wind factor */
  Angle Heading;

  /**
   * Auto QNH calculation result.
   */
  AtmosphericPressure pressure;
  Validity pressure_available;

  ClimbHistory climb_history;

  /**
   * Does #estimated_wind have a meaningful value?
   */
  Validity estimated_wind_available;

  SpeedVector estimated_wind;   /**< Wind speed, direction */

  /**
   * Is the wind available?
   */
  Validity wind_available;

  /**
   * The effective wind vector; depending on the settings, this is
   * either ExternalWind, calculated wind or manual wind.
   */
  SpeedVector wind;

  fixed AutoZoomDistance; /**< Distance to zoom to for autozoom */

  fixed TimeSunset; /**< Local time of sunset */
  /** Sun's azimuth at the current location and time */
  Angle SunAzimuth;

  TaskStats task_stats; /**< Copy of task statistics data for active task */
  CommonStats common_stats; /**< Copy of common task statistics data */
  ContestStatistics contest_stats; /**< Copy of contest statistics data */

  FLYING_STATE flight;

  ThermalBandInfo thermal_band;

  THERMAL_LOCATOR_INFO thermal_locator;

  unsigned time_process_gps; /**< Time (ms) to process main computer functions */
  unsigned time_process_idle; /**< Time (ms) to process idle computer functions */

  TraceHistory trace_history; /**< Store of short term history of variables */

  Validity auto_mac_cready_available;
  fixed auto_mac_cready;

  GlidePolar glide_polar_reach; /**< Glide polar used for reach calculations */
  GlidePolar glide_polar_safety; /**< Glide polar used for safety calculations */

  AirspaceWarningsInfo airspace_warnings;

  /**
   * @todo Reset to cleared state
   */
  void Reset();

  void ResetFlight(bool full);

  void Expire(fixed Time);
};

#endif

