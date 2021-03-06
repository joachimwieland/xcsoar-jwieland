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

#include "FAITriangleTaskFactory.hpp"
#include "Task/Tasks/OrderedTask.hpp"

FAITriangleTaskFactory::FAITriangleTaskFactory(OrderedTask& _task,
                               const TaskBehaviour &tb):
  FAITaskFactory(_task, tb)
{
}

bool 
FAITriangleTaskFactory::validate()
{

  bool valid = FAITaskFactory::validate();

  return valid && AbstractTaskFactory::TestFAITriangle();
}

void 
FAITriangleTaskFactory::update_ordered_task_behaviour(OrderedTaskBehaviour& to)
{
  FAITaskFactory::update_ordered_task_behaviour(to);
  to.min_points = 4;
  to.max_points = 4;
}
