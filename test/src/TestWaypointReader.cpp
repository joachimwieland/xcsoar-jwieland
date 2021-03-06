/* Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2010 The XCSoar Project
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

#include "Waypoint/WaypointReader.hpp"
#include "Waypoint/WaypointReaderBase.hpp"
#include "Engine/Waypoint/Waypoints.hpp"
#include "Terrain/RasterMap.hpp"
#include "Units/Units.hpp"
#include "TestUtil.hpp"
#include "tstring.hpp"
#include "Operation.hpp"

#include <vector>

static void
TestExtractParameters()
{
  TCHAR buffer[1024];
  const TCHAR *params[64];
  unsigned n;

  // test basic functionality

  n = WaypointReaderBase::ExtractParameters(_T(""), buffer, params, 64);
  ok1(n == 1);
  ok1(_tcscmp(params[0], _T("")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T("foo"), buffer, params, 64);
  ok1(n == 1);
  ok1(_tcscmp(params[0], _T("foo")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T("foo,bar"), buffer, params, 64);
  ok1(n == 2);
  ok1(_tcscmp(params[0], _T("foo")) == 0);
  ok1(_tcscmp(params[1], _T("bar")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T("foo,bar"), buffer, params, 1);
  ok1(n == 1);
  ok1(_tcscmp(params[0], _T("foo")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T("foo,bar,"), buffer, params, 64);
  ok1(n == 3);
  ok1(_tcscmp(params[0], _T("foo")) == 0);
  ok1(_tcscmp(params[1], _T("bar")) == 0);
  ok1(_tcscmp(params[2], _T("")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T("foo,bar,,"), buffer, params, 64);
  ok1(n == 4);
  ok1(_tcscmp(params[0], _T("foo")) == 0);
  ok1(_tcscmp(params[1], _T("bar")) == 0);
  ok1(_tcscmp(params[2], _T("")) == 0);
  ok1(_tcscmp(params[3], _T("")) == 0);


  // with qoutes but no quote handling

  n = WaypointReaderBase::ExtractParameters(_T("\"foo,comma\",\"bar\""),
                                      buffer, params, 64);
  ok1(n == 3);
  ok1(_tcscmp(params[0], _T("\"foo")) == 0);
  ok1(_tcscmp(params[1], _T("comma\"")) == 0);
  ok1(_tcscmp(params[2], _T("\"bar\"")) == 0);


  // quote handling

  n = WaypointReaderBase::ExtractParameters(_T("\"\""),
                                      buffer, params, 64, false, _T('"'));
  ok1(n == 1);
  ok1(_tcscmp(params[0], _T("")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T("\"\"\""),
                                      buffer, params, 64, false, _T('"'));
  ok1(n == 1);
  ok1(_tcscmp(params[0], _T("\"")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T("\"\"\"\""),
                                      buffer, params, 64, false, _T('"'));
  ok1(n == 1);
  ok1(_tcscmp(params[0], _T("\"")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T("\"foo,comma\",\"bar\""),
                                      buffer, params, 64, false, _T('"'));
  ok1(n == 2);
  ok1(_tcscmp(params[0], _T("foo,comma")) == 0);
  ok1(_tcscmp(params[1], _T("bar")) == 0);


  // no quotes, whitespace removal

  n = WaypointReaderBase::ExtractParameters(_T("foo bar"),
                                      buffer, params, 64, true);
  ok1(n == 1);
  ok1(_tcscmp(params[0], _T("foo bar")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T("foo , bar, baz"),
                                      buffer, params, 64, true);
  ok1(n == 3);
  ok1(_tcscmp(params[0], _T("foo")) == 0);
  ok1(_tcscmp(params[1], _T("bar")) == 0);
  ok1(_tcscmp(params[2], _T("baz")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T(" foo  ,  bar  , baz "),
                                      buffer, params, 64, true);
  ok1(n == 3);
  ok1(_tcscmp(params[0], _T("foo")) == 0);
  ok1(_tcscmp(params[1], _T("bar")) == 0);
  ok1(_tcscmp(params[2], _T("baz")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T(" foo\"  , \" bar \"  , \"baz "),
                                      buffer, params, 64, true);
  ok1(n == 3);
  ok1(_tcscmp(params[0], _T("foo\"")) == 0);
  ok1(_tcscmp(params[1], _T("\" bar \"")) == 0);
  ok1(_tcscmp(params[2], _T("\"baz")) == 0);

  // quote handling, whitespace removal

  n = WaypointReaderBase::ExtractParameters(_T("\"foo \" , \" bar\", \" baz\""),
                                      buffer, params, 64, true, _T('"'));
  ok1(n == 3);
  ok1(_tcscmp(params[0], _T("foo ")) == 0);
  ok1(_tcscmp(params[1], _T(" bar")) == 0);
  ok1(_tcscmp(params[2], _T(" baz")) == 0);

  n = WaypointReaderBase::ExtractParameters(
                            _T(" \" foo  \"  ,  \"  bar  \"  , \" baz \" "),
                            buffer, params, 64, true, _T('"'));
  ok1(n == 3);
  ok1(_tcscmp(params[0], _T(" foo  ")) == 0);
  ok1(_tcscmp(params[1], _T("  bar  ")) == 0);
  ok1(_tcscmp(params[2], _T(" baz ")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T("\"foo\",\"\",\"bar\""),
                                      buffer, params, 64, true, _T('"'));
  ok1(n == 3);
  ok1(_tcscmp(params[0], _T("foo")) == 0);
  ok1(_tcscmp(params[1], _T("")) == 0);
  ok1(_tcscmp(params[2], _T("bar")) == 0);

  // missing end quote
  n = WaypointReaderBase::ExtractParameters(_T("\"foo, bar"),
                                      buffer, params, 64, true, _T('"'));
  ok1(n == 1);
  ok1(_tcscmp(params[0], _T("foo, bar")) == 0);

  // embedded quotes and commas
  n = WaypointReaderBase::ExtractParameters(_T("\"foo, \"bar\"\""),
                                      buffer, params, 64, true, _T('"'));
  ok1(n == 1);
  ok1(_tcscmp(params[0], _T("foo, \"bar\"")) == 0);

  n = WaypointReaderBase::ExtractParameters(_T("\"foo, \"\"bar\"\"\""),
                                      buffer, params, 64, true, _T('"'));
  ok1(n == 1);
  ok1(_tcscmp(params[0], _T("foo, \"bar\"")) == 0);
}

typedef std::vector<Waypoint> wp_vector;

static bool
TestWaypointFile(const TCHAR* filename, Waypoints &way_points, unsigned num_wps)
{
  WaypointReader f(filename, 0);
  if (!ok1(!f.Error())) {
    skip(3, 0, "opening waypoint file failed");
    return false;
  }

  NullOperationEnvironment operation;
  if(!ok1(f.Parse(way_points, operation))) {
    skip(2, 0, "parsing waypoint file failed");
    return false;
  }

  way_points.optimise();

  ok1(!way_points.empty());
  ok1(way_points.size() == num_wps);

  return true;
}

static const Waypoint*
GetWaypoint(const Waypoint org_wp, const Waypoints &way_points)
{
  const Waypoint *wp = way_points.lookup_name(org_wp.Name);
  if (!ok1(wp != NULL)) {
    skip(2, 0, "waypoint not found");
    return NULL;
  }
  if(!ok1(wp->Location.distance(org_wp.Location) <= fixed(1000)))
    printf("%f %f\n", (double)wp->Location.Latitude.value_degrees(), (double)wp->Location.Longitude.value_degrees());
  ok1(fabs(wp->Altitude - org_wp.Altitude) < fixed_half);

  return wp;
}

static void
TestWinPilotWaypoint(const Waypoint org_wp, const Waypoint *wp)
{
  if (wp == NULL) {
    skip(7, 0, "waypoint not found");
    return;
  }

  ok1(wp->Type == ((!org_wp.IsLandable()) ? Waypoint::wtNormal : org_wp.Type));
  ok1(wp->Flags.TurnPoint == org_wp.Flags.TurnPoint);
  ok1(wp->Flags.Home == org_wp.Flags.Home);
  ok1(wp->Flags.StartPoint == org_wp.Flags.StartPoint);
  ok1(wp->Flags.FinishPoint == org_wp.Flags.FinishPoint);
  ok1(wp->runway.IsDirectionDefined() == org_wp.runway.IsDirectionDefined() &&
      (!wp->runway.IsDirectionDefined() ||
       wp->runway.GetDirectionDegrees() == org_wp.runway.GetDirectionDegrees()));
}

static void
TestWinPilot(wp_vector org_wp)
{
  Waypoints way_points;
  if (!TestWaypointFile(_T("test/data/waypoints.dat"), way_points,
                        org_wp.size())) {
    skip(10 * org_wp.size(), 0, "opening waypoint file failed");
    return;
  }

  wp_vector::iterator it;
  for (it = org_wp.begin(); it < org_wp.end(); it++) {
    const Waypoint *wp = GetWaypoint(*it, way_points);
    TestWinPilotWaypoint(*it, wp);
  }
}

static void
TestSeeYouWaypoint(const Waypoint org_wp, const Waypoint *wp)
{
  if (wp == NULL) {
    skip(6, 0, "waypoint not found");
    return;
  }

  ok1(wp->Type == org_wp.Type);
  ok1(wp->Flags.TurnPoint == org_wp.Flags.TurnPoint);
  // No home waypoints in a SeeYou file
  //ok1(wp->Flags.Home == org_wp.Flags.Home);
  ok1(wp->Flags.StartPoint == org_wp.Flags.StartPoint);
  ok1(wp->Flags.FinishPoint == org_wp.Flags.FinishPoint);
  ok1(wp->Comment == org_wp.Comment);
  ok1(wp->runway.IsDirectionDefined() == org_wp.runway.IsDirectionDefined() &&
      (!wp->runway.IsDirectionDefined() ||
       wp->runway.GetDirectionDegrees() == org_wp.runway.GetDirectionDegrees()));
  ok1(wp->runway.IsLengthDefined() == org_wp.runway.IsLengthDefined() &&
      (!wp->runway.IsLengthDefined() ||
       wp->runway.GetLength() == org_wp.runway.GetLength()));
}

static void
TestSeeYou(wp_vector org_wp)
{
  Waypoints way_points;
  if (!TestWaypointFile(_T("test/data/waypoints.cup"), way_points,
                        org_wp.size())) {
    skip(9 * org_wp.size(), 0, "opening waypoint file failed");
    return;
  }

  wp_vector::iterator it;
  for (it = org_wp.begin(); it < org_wp.end(); it++) {
    const Waypoint *wp = GetWaypoint(*it, way_points);
    TestSeeYouWaypoint(*it, wp);
  }
}

static void
TestZanderWaypoint(const Waypoint org_wp, const Waypoint *wp)
{
  if (wp == NULL) {
    skip(7, 0, "waypoint not found");
    return;
  }

  ok1(wp->Type == ((!org_wp.IsLandable()) ? Waypoint::wtNormal : org_wp.Type));
  ok1(wp->Flags.TurnPoint == org_wp.Flags.TurnPoint);
  ok1(wp->Flags.Home == org_wp.Flags.Home);
  ok1(wp->Flags.StartPoint == org_wp.Flags.StartPoint);
  ok1(wp->Flags.FinishPoint == org_wp.Flags.FinishPoint);
}

static void
TestZander(wp_vector org_wp)
{
  Waypoints way_points;
  if (!TestWaypointFile(_T("test/data/waypoints.wpz"), way_points,
                        org_wp.size())) {
    skip(10 * org_wp.size(), 0, "opening waypoint file failed");
    return;
  }

  wp_vector::iterator it;
  for (it = org_wp.begin(); it < org_wp.end(); it++) {
    if (it->Name.length() > 12)
      it->Name = it->Name.erase(12);
    trim_inplace(it->Name);
    const Waypoint *wp = GetWaypoint(*it, way_points);
    TestZanderWaypoint(*it, wp);
  }
}

static void
TestFS(wp_vector org_wp)
{
  Waypoints way_points;
  if (!TestWaypointFile(_T("test/data/waypoints_geo.wpt"), way_points,
                        org_wp.size())) {
    skip(3 * org_wp.size(), 0, "opening waypoint file failed");
    return;
  }

  wp_vector::iterator it;
  for (it = org_wp.begin(); it < org_wp.end(); it++) {
    if (it->Name.length() > 8)
      it->Name = it->Name.erase(8);
    trim_inplace(it->Name);
    GetWaypoint(*it, way_points);
  }
}

static void
TestFS_UTM(wp_vector org_wp)
{
  Waypoints way_points;
  if (!TestWaypointFile(_T("test/data/waypoints_utm.wpt"), way_points,
                        org_wp.size())) {
    skip(3 * org_wp.size(), 0, "opening waypoint file failed");
    return;
  }

  wp_vector::iterator it;
  for (it = org_wp.begin(); it < org_wp.end(); it++) {
    if (it->Name.length() > 8)
      it->Name = it->Name.erase(8);
    trim_inplace(it->Name);
    GetWaypoint(*it, way_points);
  }
}

static void
TestOzi(wp_vector org_wp)
{
  Waypoints way_points;
  if (!TestWaypointFile(_T("test/data/waypoints_ozi.wpt"), way_points,
                        org_wp.size())) {
    skip(3 * org_wp.size(), 0, "opening waypoint file failed");
    return;
  }

  wp_vector::iterator it;
  for (it = org_wp.begin(); it < org_wp.end(); it++) {
    trim_inplace(it->Name);
    GetWaypoint(*it, way_points);
  }
}

static wp_vector
CreateOriginalWaypoints()
{
  wp_vector org_wp;

  GeoPoint loc;

  // Bergneustadt
  loc.Latitude = Angle::degrees(fixed(51.051944444444445));
  loc.Longitude = Angle::degrees(fixed(7.7061111111111114));

  Waypoint wp(loc);
  wp.Altitude = fixed(488);
  wp.Name = _T("Bergneustadt");
  wp.Comment = _T("Rabbit holes, 20\" ditch south end of rwy");
  wp.runway.SetDirection(Angle::degrees(fixed(40)));
  wp.runway.SetLength(590);

  wp.Type = Waypoint::wtAirfield;
  wp.Flags.TurnPoint = true;
  wp.Flags.Home = true;
  wp.Flags.StartPoint = false;
  wp.Flags.FinishPoint = false;
  org_wp.push_back(wp);

  // Aconcagua
  loc.Latitude = Angle::dms(fixed(32), fixed(39), fixed(12)).flipped();
  loc.Longitude = Angle::dms(fixed(70), fixed(0), fixed(42)).flipped();

  Waypoint wp2(loc);
  wp2.Altitude = fixed(6962);
  wp2.Name = _T("Aconcagua");
  wp2.Comment = _T("Highest mountain in south-america");

  wp2.Type = Waypoint::wtMountainTop;
  wp2.Flags.TurnPoint = true;
  wp2.Flags.Home = false;
  wp2.Flags.StartPoint = false;
  wp2.Flags.FinishPoint = false;
  org_wp.push_back(wp2);

  // Golden Gate Bridge
  loc.Latitude = Angle::dms(fixed(37), fixed(49), fixed(3));
  loc.Longitude = Angle::dms(fixed(122), fixed(28), fixed(42)).flipped();

  Waypoint wp3(loc);
  wp3.Altitude = fixed(227);
  wp3.Name = _T("Golden Gate Bridge");
  wp3.Comment = _T("");

  wp3.Type = Waypoint::wtBridge;
  wp3.Flags.TurnPoint = true;
  wp3.Flags.Home = false;
  wp3.Flags.StartPoint = false;
  wp3.Flags.FinishPoint = false;
  org_wp.push_back(wp3);

  // Red Square
  loc.Latitude = Angle::dms(fixed(55), fixed(45), fixed(15));
  loc.Longitude = Angle::dms(fixed(37), fixed(37), fixed(12));

  Waypoint wp4(loc);
  wp4.Altitude = fixed(123);
  wp4.Name = _T("Red Square");
  wp4.runway.SetDirection(Angle::degrees(fixed(90)));
  wp4.runway.SetLength(Units::ToSysUnit(fixed(0.01), unStatuteMiles));

  wp4.Type = Waypoint::wtOutlanding;
  wp4.Flags.TurnPoint = true;
  wp4.Flags.Home = false;
  wp4.Flags.StartPoint = false;
  wp4.Flags.FinishPoint = false;
  org_wp.push_back(wp4);

  // Sydney Opera
  loc.Latitude = Angle::dms(fixed(33), fixed(51), fixed(25)).flipped();
  loc.Longitude = Angle::dms(fixed(151), fixed(12), fixed(55));

  Waypoint wp5(loc);
  wp5.Altitude = fixed(5);
  wp5.Name = _T("Sydney Opera");
  wp5.Comment = _T("");

  wp5.Type = Waypoint::wtNormal;
  wp5.Flags.TurnPoint = true;
  wp5.Flags.Home = false;
  wp5.Flags.StartPoint = false;
  wp5.Flags.FinishPoint = false;
  org_wp.push_back(wp5);

  return org_wp;
}

int main(int argc, char **argv)
{
  wp_vector org_wp = CreateOriginalWaypoints();

  plan_tests(63 + 6 * 4 + (9 + 10 + 8 + 3 + 3 + 3) * org_wp.size());

  TestExtractParameters();

  TestWinPilot(org_wp);
  TestSeeYou(org_wp);
  TestZander(org_wp);
  TestFS(org_wp);
  TestFS_UTM(org_wp);
  TestOzi(org_wp);

  return exit_status();
}
