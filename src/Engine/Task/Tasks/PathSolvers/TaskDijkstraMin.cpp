/* Copyright_License {

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

#include "TaskDijkstraMin.hpp"
#include "Task/Tasks/OrderedTask.hpp"

TaskDijkstraMin::TaskDijkstraMin(OrderedTask& _task, const bool do_reserve) :
  TaskDijkstra(_task, true, do_reserve)
{
}

bool
TaskDijkstraMin::distance_min(const SearchPoint &currentLocation)
{
  if (!refresh_task())
    return false;

  const ScanTaskPoint start(max(1, (int)active_stage) - 1, 0);

  dijkstra.restart(start);
  if (active_stage)
    add_start_edges(currentLocation);
  return run();
}


void
TaskDijkstraMin::save()
{
  for (unsigned j = active_stage; j != num_stages; ++j)
    task.set_tp_search_min(j, solution[j]);
}

