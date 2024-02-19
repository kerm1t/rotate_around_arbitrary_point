#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include "math/linmath.h"
#include "math/mathbase.h"

#include <memory.h>
#include <vector>
#include <algorithm> // std::max

#include "pointcloud_io.hpp"

namespace lloft {
  // semantic
  // zenseact

  // OpenGL
  std::vector<float> vertices;
  std::vector<float> colors[3]; // height, dist, intensity

  struct point {
    float x;
    float y;
    float z;
  };
  typedef std::vector<point> points_raw;
/*    int inten;
    int az;   // RAD/10000
    int el;   // RAD/10000
    float d;
*/
  // the following vector has the same size as "points_raw" vector, it will be given to GPU
  struct point_rgb {
    unsigned char r;
    unsigned char g;
    unsigned char b;
  };
  typedef std::vector<point_rgb> points_rgb;

  enum point_class { road, below_road, above_road, lane_marker };
  struct point_sem {
    float x;
    float y;
    float z;
    int inten;
    int az;   // RAD/10000
    int el;   // RAD/10000
    float d;
    point_class pclass;
  };
  typedef std::vector<point> points_semantic;

  enum color_coding { cc_height, cc_distance }; // --> to visualization ?
  color_coding colorcoding = cc_height;
  int intensityclamp[2] = { 0, 10000 };

/* HRL specific
  ===============
  max col + row differ for scan patterns, e.g.
  
  Berlin 2492 x 1080,
  VW          x 810,
  Volvo  1700 x 1070

  however not all opf these col and rows are used
  also the acquisition is rather by oversampling a line.
  so point cloud is unordered (column wise) 
  next up: order/sort the unordered hrl point cloud
*/
#define MAXCOL 2500
#define MAXROW 1200

#define MAXUSEDROW 80
#define MAXUSEDCOL 1700

  int points_set[MAXROW + 1][MAXCOL + 1];
  int rows_set[MAXROW + 1];
  int cols_set[MAXCOL + 1];
  int image[MAXUSEDROW][MAXUSEDCOL];
  // whoooo big data coming
/*  struct point_ord {
    float x;
    float y;
    float z;
    float d;
    float az; // <- int
    float el; // <- int
    int inten;
  };
  point_ord points_ordered[MAXUSEDROW][MAXUSEDCOL]; 
  */

  class pointbase {
  };
  
  class pointcloud : public pointbase {
  public:
    int frameid;
    int numpoints;
    bool ordered;
    int numrows;
    int numcols;
    points_raw pts_raw;
    points_rgb pts_rgb;
    points_raw* pts_ord; // optional

    int size(); // = numpoints = width * height
    float loadtime();
    vec4 bbox;
  };
  
  class pointcloud_HRL : public pointcloud {
    void sort();
    // to_pointcloud2()
  };

  int pointcloud::size() {
    return this->numpoints;
  }

  float pointcloud::loadtime() {
    return 0; // replace dummy
  }

  // b) load from .pcd or .bin
//point_cloud pointcloud_load(std::string s)
  int pointcloud_load(std::string s)
  {
    static const float COLOR_DISTANCE_MAX_INV = 1.0F / 250.0F;
    //  static const float COLOR_AUTOSAR_HEIGHT_MIN = 0.0F;
    static const float COLOR_AUTOSAR_HEIGHT_MIN = -1.2F;
    //  static const float COLOR_AUTOSAR_HEIGHT_MAX = 3.0F;
    static const float COLOR_AUTOSAR_HEIGHT_MAX = 3.5F;
    static const float FLOAT_TO_S16Q7 = 128.0F;
    static const float S16Q7_TO_FLOAT = 1.0F / FLOAT_TO_S16Q7;
    // convert coloring parameters from float to fixpoint
    static const float COLOR_AUTOSAR_HEIGHT_MIN_S16Q7 = COLOR_AUTOSAR_HEIGHT_MIN * FLOAT_TO_S16Q7;
    static const float COLOR_AUTOSAR_HEIGHT_MAX_S16Q7 = COLOR_AUTOSAR_HEIGHT_MAX * FLOAT_TO_S16Q7;
    static const float COLOR_DISTANCE_MAX_INV_S16Q7 = COLOR_DISTANCE_MAX_INV / FLOAT_TO_S16Q7;
    static const float COLOR_DISTANCE_MAX = 50.0f;

    std::cout << "clearing..." << std::endl;
    ///  p_cloud->clear();
    cloud.clear();
    vertices.clear();
    colors[0].clear();
    colors[1].clear();
    colors[2].clear();
    std::cout << "cleared" << std::endl;

    float loadtime = loadPointCloud(s, cloud); // returns load time in [sec], is < 0, if something failed
  ///  if (loadtime < 0) return point_cloud(0,-1);

    // 2do: copy over, simplify later
    for (int i = 0; i < cloud.size(); i++) // this for loop can be spared! MeasurmentPoint3D is proprietary, replace with PointXYZ
    {
      // (a) vertices
      PointXYZI pt;
      pt = cloud[i];
      vertices.push_back(pt.x);
      vertices.push_back(pt.y);
      vertices.push_back(pt.z);

      // (b) colors, precalc multiple colorizations (height, dist, intensity, ...)
      //     this needs 3 buffer objects on the gpu, alternative: fragment shader 
      float r, g, b;
      double hue;
      float height = (float)pt.z;
      float absDist = std::sqrt(pt.x * pt.x + pt.y * pt.y);

      hue = (1.0F - std::max<float>(COLOR_AUTOSAR_HEIGHT_MIN, std::min<float>(height, COLOR_AUTOSAR_HEIGHT_MAX)) / COLOR_AUTOSAR_HEIGHT_MAX) * (4.0F / 6.0F);
      hsv2rgb(hue, 1.0F, 1.0F, r, g, b);
      colors[0].push_back((float)b);
      colors[0].push_back((float)g);
      colors[0].push_back((float)r);

      hue = (1.0F - std::max<float>(0.0f, std::min<float>(absDist, COLOR_DISTANCE_MAX)) / COLOR_DISTANCE_MAX);
      hsv2rgb(hue, 1.0F, 1.0F, r, g, b);
      colors[1].push_back((float)b);
      colors[1].push_back((float)g);
      colors[1].push_back((float)r);

      int ilow = pt.intensity;
#define COLOR_LOWGAIN_MIN 0
#define COLOR_LOWGAIN_MAX 1000//5000
      //    double hue = (1.0F - std::max<float>(COLOR_LOWGAIN_MIN, std::min<float>(ilow, COLOR_LOWGAIN_MAX)) / COLOR_LOWGAIN_MAX);
      hue = (1.0F - std::max<float>(intensityclamp[0], std::min<float>(ilow, intensityclamp[1])) / intensityclamp[1]);
      hsv2rgb(hue, 1.0F, 1.0F, r, g, b);
      colors[2].push_back((float)b);
      colors[2].push_back((float)g);
      colors[2].push_back((float)r);
    }

    int numpoints = cloud.size();
    return numpoints;

    //  return point_cloud(m_measPoints->size(), loadtime);
  }
}
#endif // POINTCLOUD_H
