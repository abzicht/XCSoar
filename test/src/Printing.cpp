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
#include "Printing.hpp"
#include <fstream>

#include "Engine/Task/TaskManager.hpp"
#include "Task/Tasks/AbortTask.hpp"
#include "Task/Tasks/GotoTask.hpp"
#include "Task/Tasks/OrderedTask.hpp"
#include "Task/Tasks/AbstractTask.hpp"
#include "Task/Tasks/BaseTask/TaskPoint.hpp"
#include "Task/Tasks/BaseTask/SampledTaskPoint.hpp"
#include "Task/Tasks/BaseTask/OrderedTaskPoint.hpp"
#include "Navigation/SearchPointVector.hpp"
#include "Trace/Trace.hpp"

#ifdef FIXED_MATH
std::ostream& operator<<(std::ostream& os,fixed const& value)
{
  return os<<value.as_double();
}
#endif

#include "Waypoint/Waypoint.hpp"

std::ostream& operator<< (std::ostream& f, 
                          const Waypoint& wp)
{
  f << wp.location.longitude << " " << wp.location.latitude << "\n";
  return f;
}

#include "Navigation/Flat/FlatBoundingBox.hpp"

/*
void 
FlatBoundingBox::print(std::ostream &f, const TaskProjection &task_projection) const {
  FlatGeoPoint ll(bb_ll.Longitude,bb_ll.Latitude);
  FlatGeoPoint lr(bb_ur.Longitude,bb_ll.Latitude);
  FlatGeoPoint ur(bb_ur.Longitude,bb_ur.Latitude);
  FlatGeoPoint ul(bb_ll.Longitude,bb_ur.Latitude);
  GeoPoint gll = task_projection.unproject(ll);
  GeoPoint glr = task_projection.unproject(lr);
  GeoPoint gur = task_projection.unproject(ur);
  GeoPoint gul = task_projection.unproject(ul);
  
  f << gll.Longitude << " " << gll.Latitude << "\n";
  f << glr.Longitude << " " << glr.Latitude << "\n";
  f << gur.Longitude << " " << gur.Latitude << "\n";
  f << gul.Longitude << " " << gul.Latitude << "\n";
  f << gll.Longitude << " " << gll.Latitude << "\n";
  f << "\n";
}
*/

/*
void
TaskMacCready::print(std::ostream &f, const AIRCRAFT_STATE &aircraft) const
{
  AIRCRAFT_STATE aircraft_start = get_aircraft_start(aircraft);
  AIRCRAFT_STATE aircraft_predict = aircraft;
  aircraft_predict.Altitude = aircraft_start.Altitude;
  f << "#  i alt  min  elev\n";
  f << start-0.5 << " " << aircraft_start.Altitude << " " <<
    minHs[start] << " " <<
    task_points[start]->get_elevation() << "\n";
  for (int i=start; i<=end; i++) {
    aircraft_predict.Altitude -= gs[i].HeightGlide;
    f << i << " " << aircraft_predict.Altitude << " " << minHs[i]
      << " " << task_points[i]->get_elevation() << "\n";
  }
  f << "\n";
}
*/

static void
PrintTracePoint(const TracePoint &point, std::ofstream& fs)
{
  fs << point.time
     << " " << point.get_location().longitude
     << " " << point.get_location().latitude
     << " " << point.GetAltitude()
     << " " << point.GetVario()
     << "\n";
}

void
PrintHelper::trace_print(const Trace& trace, const GeoPoint &loc)
{
  std::ofstream fs("results/res-trace.txt");

  for (Trace::const_iterator it = trace.begin(); it != trace.end(); ++it)
    PrintTracePoint(*it, fs);
}


#include "Math/Angle.hpp"

std::ostream& operator<< (std::ostream& o, const Angle& a)
{
  o << a.Degrees();
  return o;
} 


void write_point(const SearchPoint& sp, const FlatGeoPoint& p, const char* name)
{
  printf("%g %g %d %d # %s\n",
         (double)sp.get_location().longitude.Degrees(),
         (double)sp.get_location().latitude.Degrees(),
         p.Longitude,
         p.Latitude,
         name);
  fflush(stdout);
}

void write_spv (const SearchPointVector& spv)
{
  for (std::vector<SearchPoint>::const_iterator v = spv.begin();
       v != spv.end(); ++v) {
    write_point(*v, v->get_flatLocation(), "spv");
  }
  printf("spv\n");
  fflush(stdout);
}

#include "Route/AirspaceRoute.hpp"

void PrintHelper::print_route(RoutePlanner& r)
{
  for (Route::const_iterator i = r.solution_route.begin();
       i!= r.solution_route.end(); ++i) {
    printf("%.6g %.6g %d # solution\n",
           (double)i->longitude.Degrees(),
           (double)i->latitude.Degrees(),
           0);
  }
  printf("# solution\n");
  for (Route::const_iterator i = r.solution_route.begin();
       i!= r.solution_route.end(); ++i) {
    printf("%.6g %.6g %d # solution\n",
           (double)i->longitude.Degrees(),
           (double)i->latitude.Degrees(),
           (int)i->altitude);
  }
  printf("# solution\n");
  printf("# solution\n");
  printf("# stats:\n");
  printf("#   dijkstra links %d\n", (int)r.count_dij);
  printf("#   unique links %d\n", (int)r.count_unique);
  printf("#   airspace queries %d\n", (int)r.count_airspace);
  printf("#   terrain queries %d\n", (int)r.count_terrain);
  printf("#   supressed %d\n", (int)r.count_supressed);
}

#include "Route/ReachFan.hpp"

void
PrintHelper::print_reach_tree(const RoutePlanner& r)
{
  print(r.reach);
}

void
PrintHelper::print(const ReachFan& r)
{
  print(r.root);
}

void
PrintHelper::print(const FlatTriangleFanTree& r) {
  print((const FlatTriangleFan&)r, r.depth);

  for (FlatTriangleFanTree::LeafVector::const_iterator it = r.children.begin();
       it != r.children.end(); ++it) {
    print(*it);
  }
};

void
PrintHelper::print(const FlatTriangleFan& r, const unsigned depth) {
  if (r.vs.size()<3)
    return;

  if (depth) {
    printf("%d %d # fcorner\n", r.vs[0].Longitude, r.vs[0].Latitude);
  }

  for (FlatTriangleFan::VertexVector::const_iterator it = r.vs.begin();
       it != r.vs.end(); ++it) {
    const FlatGeoPoint p = (*it);
    printf("%d %d # ftri\n", p.Longitude, p.Latitude);
  }
  printf("%d %d # ftri\n", r.vs[0].Longitude, r.vs[0].Latitude);
  printf("# ftri\n");
}

