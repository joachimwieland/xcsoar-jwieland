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

#include "WayPoint/WaypointWriter.hpp"

#include "Engine/Waypoint/Waypoints.hpp"
#include "IO/TextWriter.hpp"
#include "Units/Units.hpp"

void
WaypointWriter::Save(TextWriter &writer)
{
  // Iterate through the waypoint list and save each waypoint with
  // the right file number to the TextWriter
  /// @todo JMW: iteration ordered by ID would be preferred
  for (Waypoints::const_iterator it = waypoints.begin();
       it != waypoints.end(); it++) {
    const Waypoint& wp = it->get_waypoint();
    if (wp.FileNum == file_number)
      WriteWaypoint(writer, wp);
  }
}

void
WaypointWriter::WriteWaypoint(TextWriter &writer, const Waypoint& wp)
{
  // Write the waypoint id
  writer.printf("%u,", wp.original_id > 0 ? wp.original_id : wp.id);

  // Write the latitude
  WriteAngle(writer, wp.Location.Latitude, true);
  writer.write(',');

  // Write the longitude id
  WriteAngle(writer, wp.Location.Longitude, false);
  writer.write(',');

  // Write the altitude id
  WriteAltitude(writer, wp.Altitude);
  writer.write(',');

  // Write the waypoint flags
  WriteFlags(writer, wp);
  writer.write(',');

  // Write the waypoint name
  writer.write(wp.Name.c_str());
  writer.write(',');

  // Write the waypoint description
  writer.writeln(wp.Comment.c_str());
}

void
WaypointWriter::WriteAngle(TextWriter &writer, const Angle &angle,
                           bool is_latitude)
{
  // Calculate degrees, minutes and seconds
  int deg, min, sec;
  bool is_positive;
  if (is_latitude)
    Units::LatitudeToDMS(angle, &deg, &min, &sec, &is_positive);
  else
    Units::LongitudeToDMS(angle, &deg, &min, &sec, &is_positive);

  // Save them into the buffer string
  writer.printf(is_latitude ? "%02d:%02d:%02d" : "%03d:%02d:%02d", deg, min, sec);

  // Attach the buffer string to the output
  if (is_latitude)
    writer.write(is_positive ? "N" : "S");
  else
    writer.write(is_positive ? "E" : "W");
}

void
WaypointWriter::WriteAltitude(TextWriter &writer, fixed altitude)
{
  writer.printf("%dM", (int)altitude);
}

void
WaypointWriter::WriteFlags(TextWriter &writer, const Waypoint &wp)
{
  if (wp.IsAirport())
    writer.write('A');
  if (wp.Flags.TurnPoint)
    writer.write('T');
  if (wp.IsLandable())
    writer.write('L');
  if (wp.Flags.Home)
    writer.write('H');
  if (wp.Flags.StartPoint)
    writer.write('S');
  if (wp.Flags.FinishPoint)
    writer.write('F');

  // set as turnpoint by default if nothing else
  if (!wp.Flags.TurnPoint &&
      !wp.IsLandable() &&
      !wp.Flags.Home &&
      !wp.Flags.StartPoint &&
      !wp.Flags.FinishPoint)
    writer.write('T');
}

