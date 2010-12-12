/*
Copyright_License {

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

#include "Android/Timer.hpp"
#include "Java/Global.hpp"
#include "Screen/Window.hpp"
#include "Screen/SDL/Event.hpp"
#include "org_xcsoar_Timer.h"

#include <SDL_events.h>
#include <assert.h>

AndroidTimer::Bridge::Bridge(JNIEnv *env, jlong ptr, jint period)
{
  jclass cls = env->FindClass("org/xcsoar/Timer");
  assert(cls != NULL);

  jmethodID cid = env->GetMethodID(cls, "<init>", "(JI)V");
  assert(cid != NULL);

  jobject obj = env->NewObject(cls, cid, ptr, period);
  env->DeleteLocalRef(cls);

  set(obj);
  env->DeleteLocalRef(obj);
}

AndroidTimer::Bridge::~Bridge()
{
  uninstall();
}

AndroidTimer::AndroidTimer(Window &_window, unsigned ms)
  :window(_window), bridge(Java::GetEnv(), (jlong)this, ms),
   disabled(false), running(false)
{
}

static bool
match_timer(const SDL_Event &event, void *ctx)
{
  return event.type == Window::EVENT_TIMER && event.user.data1 == ctx;
}

AndroidTimer::~AndroidTimer()
{
  EventQueue::purge(SDL_EVENTMASK(Window::EVENT_TIMER),
                    match_timer, (void *)this);
}

void
AndroidTimer::disable()
{
  assert(!disabled);

  if (running)
    disabled = true;
  else
    delete this;
}

void
AndroidTimer::run()
{
  assert(!running);
  assert(!disabled);

  running = true;
  window.on_timer(this);
  running = false;

  if (disabled)
    delete this;
  else
    bridge.install();
}

JNIEXPORT void JNICALL
Java_org_xcsoar_Timer_run(JNIEnv *env, jobject obj, jlong ptr)
{
  AndroidTimer *timer = (AndroidTimer *)(void *)ptr;

  SDL_Event event;
  event.type = Window::EVENT_TIMER;
  event.user.code = 0;
  event.user.data1 = timer;
  event.user.data2 = NULL;
  ::SDL_PushEvent(&event);
}