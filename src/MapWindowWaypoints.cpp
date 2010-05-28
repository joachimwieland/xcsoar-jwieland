/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

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

#include "MapWindow.hpp"
#include "MapWindowLabels.hpp"
#include "Screen/Graphics.hpp"
#include "Waypoint/WaypointVisitor.hpp"
#include "GlideSolvers/GlidePolar.hpp"
#include "Task/Tasks/TaskSolvers/TaskSolution.hpp"
#include "Task/Tasks/BaseTask/UnorderedTaskPoint.hpp"
#include "Task/Visitors/TaskPointVisitor.hpp"
#include "Task/Visitors/TaskVisitor.hpp"
#include "TaskClientMap.hpp"

#include <assert.h>
#include <stdio.h>

class WaypointVisitorMap: 
  public WaypointVisitor, 
  public TaskPointConstVisitor,
  public TaskVisitor
{
public:
  WaypointVisitorMap(MapWindow &_map, Canvas &_canvas, const GlidePolar &polar):
    map(_map),
    aircraft_state(ToAircraftState(map.Basic())),
    canvas(_canvas),
    glide_polar(polar)
  {
    // if pan mode, show full names
    pDisplayTextType = map.SettingsMap().DisplayTextType;
    if (map.SettingsMap().EnablePan)
      pDisplayTextType = DISPLAYNAME;

    _tcscpy(sAltUnit, Units::GetAltitudeName());
  }

  void
  DrawWaypoint(const Waypoint& way_point, bool in_task = false)
  {
    POINT sc;
    if (!map.LonLat2ScreenIfVisible(way_point.Location, &sc))
      return;

    TextInBoxMode_t TextDisplayMode;
    bool irange = false;
    bool islandable = false;
    bool dowrite = in_task || (map.SettingsMap().DeclutterLabels < 2);

    TextDisplayMode.AsInt = 0;

    irange = map.WaypointInScaleFilter(way_point);

    const MaskedIcon *icon = &MapGfx.SmallIcon;

    bool draw_alt = false;
    int AltArrivalAGL = 0;

    if (way_point.is_landable()) {
      islandable = true; // so we can always draw them

      const UnorderedTaskPoint t(way_point, map.SettingsComputer());
      const GlideResult r =
        TaskSolution::glide_solution_remaining(t, aircraft_state, glide_polar);
      bool reachable = r.glide_reachable();

      if ((map.SettingsMap().DeclutterLabels < 1) || in_task) {
        if (reachable)
          AltArrivalAGL = (int)Units::ToUserUnit(r.AltitudeDifference,
                                                 Units::AltitudeUnit);
        draw_alt = reachable;
      }

      if (reachable) {
        TextDisplayMode.AsFlag.Reachable = 1;

        if ((map.SettingsMap().DeclutterLabels < 2) || in_task) {
          if (in_task || (map.SettingsMap().DeclutterLabels < 1))
            TextDisplayMode.AsFlag.Border = 1;

          // show all reachable landing fields unless we want a decluttered
          // screen.
          dowrite = true;
        }

        if (way_point.Flags.Airport)
          icon = &MapGfx.AirportReachableIcon;
        else
          icon = &MapGfx.FieldReachableIcon;
      } else {
        if (way_point.Flags.Airport)
          icon = &MapGfx.AirportUnreachableIcon;
        else
          icon = &MapGfx.FieldUnreachableIcon;
      }
    } else {
      if (map.GetMapScaleKM() > fixed_four)
        icon = &MapGfx.SmallIcon;
      else
        icon = &MapGfx.TurnPointIcon;
    }

    if (in_task)
      TextDisplayMode.AsFlag.WhiteBold = 1;

    if (irange || in_task || dowrite || islandable)
      icon->draw(canvas, map.get_bitmap_canvas(), sc.x, sc.y);

    if (pDisplayTextType == DISPLAYNAMEIFINTASK) {
      if (!in_task)
        return;

      dowrite = true;
    }

    if (!dowrite)
      return;

    TCHAR Buffer[32];

    switch (pDisplayTextType) {
    case DISPLAYNAMEIFINTASK:
      if (in_task)
        _stprintf(Buffer, _T("%s"), way_point.Name.c_str());
      break;

    case DISPLAYNAME:
      _stprintf(Buffer, _T("%s"), way_point.Name.c_str());
      break;

    case DISPLAYNUMBER:
      _stprintf(Buffer, _T("%d"), way_point.id);
      break;

    case DISPLAYFIRSTFIVE:
      _tcsncpy(Buffer, way_point.Name.c_str(), 5);
      Buffer[5] = '\0';
      break;

    case DISPLAYFIRSTTHREE:
      _tcsncpy(Buffer, way_point.Name.c_str(), 3);
      Buffer[3] = '\0';
      break;

    case DISPLAYNONE:
      Buffer[0] = '\0';
      break;

    case DISPLAYUNTILSPACE:
      _stprintf(Buffer, _T("%s"), way_point.Name.c_str());
      TCHAR *tmp;
      tmp = _tcsstr(Buffer, _T(" "));
      if (tmp != NULL)
        tmp[0] = '\0';

      break;

    default:
      assert(0);
      break;
    }

    if (draw_alt) {
      size_t length = _tcslen(Buffer);
      if (length > 0)
        Buffer[length++] = _T(':');

      _stprintf(Buffer + length, _T("%d%s"), AltArrivalAGL, sAltUnit);
    }

    MapWaypointLabelAdd(Buffer, sc.x + 5, sc.y, TextDisplayMode, AltArrivalAGL,
                        in_task, false, false, false, map.GetMapRect());
  }

  void
  Visit(const AbortTask& task)
  {
    task.tp_CAccept(*this);
  }

  void
  Visit(const OrderedTask& task)
  {
    task.tp_CAccept(*this);
  }

  void
  Visit(const GotoTask& task)
  {
    task.tp_CAccept(*this);
  }

  void
  Visit(const Waypoint& way_point)
  {
    DrawWaypoint(way_point, false);
  }

  void
  Visit(const UnorderedTaskPoint& tp)
  {
    DrawWaypoint(tp.get_waypoint(), true);
  }

  void
  Visit(const StartPoint& tp)
  {
    DrawWaypoint(tp.get_waypoint(), true);
  }

  void
  Visit(const FinishPoint& tp)
  {
    DrawWaypoint(tp.get_waypoint(), true);
  }

  void
  Visit(const AATPoint& tp)
  {
    DrawWaypoint(tp.get_waypoint(), true);
  }

  void
  Visit(const ASTPoint& tp)
  {
    DrawWaypoint(tp.get_waypoint(), true);
  }

private:
  MapWindow &map;
  const AIRCRAFT_STATE aircraft_state;
  Canvas &canvas;
  int pDisplayTextType;
  TCHAR sAltUnit[4];
  const GlidePolar glide_polar;
};

void
MapWindow::DrawWaypoints(Canvas &canvas)
{
  if (!task || task->is_waypoints_empty()) 
    return;

  MapWaypointLabelClear();

  WaypointVisitorMap v(*this, canvas, get_glide_polar());
  task->waypoints_visit_within_range(PanLocation,
                                     fixed(GetScreenDistanceMeters()), v);
  if (SettingsMap().DisplayTextType == DISPLAYNAMEIFINTASK)
    task->CAccept(v);

  MapWaypointLabelSortAndRender(canvas);
}
