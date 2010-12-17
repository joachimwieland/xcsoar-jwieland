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

package org.xcsoar;

import android.os.Handler;
import android.os.Bundle;
import android.content.Context;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;

/**
 * Code to support the internal GPS receiver via #LocationManager.
 */
public class InternalGPS
  implements LocationListener, Runnable {
  private static Handler handler;

  /**
   * Global initialization of the class.  Must be called from the main
   * event thread, because the Handler object must be bound to that
   * thread.
   */
  public static void Initialize() {
    handler = new Handler();
  }

  /** the name of the currently selected location provider */
  String locationProvider = LocationManager.GPS_PROVIDER;
  //String locationProvider = LocationManager.NETWORK_PROVIDER;

  LocationManager locationManager;

  InternalGPS(Context context) {
    locationManager = (LocationManager)context.getSystemService(Context.LOCATION_SERVICE);
    update();
  }

  /**
   * Called by the #Handler, indirectly by update().  Updates the
   * LocationManager subscription inside the main thread.
   */
  @Override public void run() {
    locationManager.removeUpdates(this);

    if (locationProvider != null) {
      locationManager.requestLocationUpdates(locationProvider,
                                             1000, 0, this);
      setConnected(1); // waiting for fix
    } else
      setConnected(0); // not connected
  }

  /**
   * Update the LocationManager subscription.  May be called from any
   * thread.
   */
  private void update() {
    handler.post(this);
  }

  public void setLocationProvider(String _locationProvider) {
    locationProvider = _locationProvider;
    update();
  }

  private native void setConnected(int connected);
  private native void setLocation(long time, int n_satellites,
                                  double longitude, double latitude,
                                  boolean hasAltitude, double altitude,
                                  boolean hasBearing, double bearing,
                                  boolean hasSpeed, double speed);

  private void sendLocation(Location location) {
    Bundle extras = location.getExtras();

    setLocation(location.getTime(),
                extras != null ? extras.getInt("satellites", 1) : 1,
                location.getLongitude(), location.getLatitude(),
                location.hasAltitude(), location.getAltitude(),
                location.hasBearing(), location.getBearing(),
                location.hasSpeed(), location.getSpeed());
  }

  /** from LocationListener */
  @Override public void onLocationChanged(Location newLocation) {
    setConnected(2); // fix found

    sendLocation(newLocation);
  }

  /** from LocationListener */
  @Override public void onProviderDisabled(String provider) {
    setConnected(0); // not connected
  }

  /** from LocationListener */
  @Override public void onProviderEnabled(String provider) {
    setConnected(1); // waiting for fix
  }

  /** from LocationListener */
  @Override public void onStatusChanged(String provider, int status,
                                        Bundle extras) {
    switch (status) {
    case LocationProvider.OUT_OF_SERVICE:
    case LocationProvider.TEMPORARILY_UNAVAILABLE:
      setConnected(0); // not connected
      break;

    case LocationProvider.AVAILABLE:
      break;
    }
  }
}