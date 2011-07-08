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

#ifndef XCSOAR_DIALOGS_WAYPOINT_HPP
#define XCSOAR_DIALOGS_WAYPOINT_HPP

#include <stddef.h>

struct GeoPoint;
class Waypoint;
class Waypoints;
class SingleWindow;
class OrderedTask;

const Waypoint *
dlgWaypointSelect(SingleWindow &parent,
                  const GeoPoint &location,
                  OrderedTask *ordered_task = NULL,
                  const unsigned ordered_task_index = 0);

void
dlgWaypointSelectAddToLastUsed(const Waypoint &wp);

void
dlgConfigWaypointsShowModal();

bool
dlgWaypointEditShowModal(Waypoint &way_point);

void
dlgWaypointDetailsShowModal(SingleWindow &parent, const Waypoint& waypoint,
                            bool allow_navigation = true);

bool
PopupNearestWaypointDetails(const Waypoints &way_points,
                            const GeoPoint &location,
                            double range, bool scalefilter = false);

#endif