#ifndef POINTCLOUD_IO_H
#define POINTCLOUD_IO_H

//#include "pointcloud/pointcloud.hpp"
#include "util/color.h"

#include <string>
#include <vector>
#include <iostream> // std::cout
#include <time.h>

//#define ECAL true
#ifdef ECAL
// proto
#include "pcl.pb.h"
// unpacker for Pointcloud2
#include "../include/PointCloudHandler.h"
#endif

namespace lloft {
  /*  purpose:  point cloud input / output
      date:     5/20/2023
      author:   w.schulz
        */

  enum io_type { ascii, binary };
/*
  class pointcloud_io : public pointcloud {
  public:
    pointcloud& p;
    pointcloud_io(pointcloud& pc) : p(pc) {
    }
#ifdef ECAL
    void ecal_callback(const pcl::PointCloud2& pc);
#endif
    void load_pcd(std::string filename, io_type iot);
    void load_ply();
    void write_pcd(std::string filename, io_type iot);
    void write_ply();
  };
*/
#ifdef ECAL
  // Callback for eCal message -> fill pointcloud
  void pointcloud_io::ecal_callback(const pcl::PointCloud2& pc)
  {
    //  ecalmsg = pointcloud_msg.numpoints();
    std::vector<double> pcl2_TMP_points;
    std::vector<std::string> fields = {
      "x", "y", "z" ,
      "Low Gain Intensity",
      "High Gain Intensity",
//      "Laser Shot Cols",
//      "Laser Shot Row",
//      "Horizontal Offset",
      "Azimuth",
      "Radius",
      "Elevation" };
    getPointFields(pc, pcl2_TMP_points, fields);
    if (pcl2_TMP_points.size() == 0) return; // most probably some field name spelled wrongly

    numpoints = pc.width();
    int pointCount = numpoints;
    int pointSize = pc.point_step();
    frameid = atoi(pc.header().frame_id().c_str());
    
    // process
    p.pts_raw.clear();
    p.pts_raw.clear();
    // OpenGL
    vertices.clear();
    colors[0].clear();
    colors[1].clear();
    colors[2].clear();

///    lloft::p_op.organize(); // uses pcl2_TMP_points
///    lloft::p_op::order(); // uses pcl2_TMP_points
    float r, g, b;

    // (iv) visualize point cloud
    for (int i = 0; i < numpoints; i++)
    {
      point pt;
      pt.x = pcl2_TMP_points[i * fields.size()];
      pt.y = pcl2_TMP_points[i * fields.size() + 1];
      pt.z = pcl2_TMP_points[i * fields.size() + 2];
      int ilow = pcl2_TMP_points[i * fields.size() + 3];
      int ihigh = pcl2_TMP_points[i * fields.size() + 4];
      /*    int col = points[i * fields.size() + 5];
          int row = points[i * fields.size() + 6];
          int hoffs = points[i * fields.size() + 7];
          int az = points[i * fields.size() + 8]; // need to convert
          float faz = (float)az * 10000.0f * 3.14159f / 180.0f;
          float radius = points[i * fields.size() + 9];
      */

      //    int intensity = ilow;
      int intensity = ihigh;
///      pt.inten = intensity;

#define COLOR_LOWGAIN_MIN 0
#define COLOR_LOWGAIN_MAX 5000
      //    double hue = (1.0F - std::max<float>(COLOR_LOWGAIN_MIN, std::min<float>(ilow, COLOR_LOWGAIN_MAX)) / COLOR_LOWGAIN_MAX);
      // intensity
      double hue = (1.0F - std::max<float>(intensityclamp[0], std::min<float>(intensity, intensityclamp[1])) / intensityclamp[1]);
      // order
      ///    double hue = (1.0F - std::max<float>(0, std::min<float>(i, ecal_numpoints)) / ecal_numpoints);
          // order
///      int max = highlight;// 65535;
      ///    double hue = (1.0F - std::max<float>(0, std::min<float>(col, max)) / max);
///      float r, g, b;
      hsv2rgb(hue, 1.0F, 1.0F, r, g, b);

      point_rgb ptcol;
      ptcol.r = b * 255;
      ptcol.g = r * 255;
      ptcol.b = g * 255;

      //    if (row == rowlookup[highlight]) { pt.r = 200; pt.g = pt.r; pt.b = pt.r; } else { pt.r = 20; pt.g = pt.r; pt.b = pt.r; }
      ///    if (col == collookup[178] || row == rowlookup[highlight]) { pt.r = 200; pt.g = pt.r; pt.b = pt.r; } else { pt.r = 20; pt.g = pt.r; pt.b = pt.r; }

///      m_measPoints[0].push_back(pt);
      p.pts_raw.push_back(pt);
      p.pts_rgb.push_back(ptcol);
      /*
      vertices.push_back(pt.x);
      vertices.push_back(pt.y);
      vertices.push_back(pt.z);
      colors[0].push_back((float)b);
      colors[0].push_back((float)g);
      colors[0].push_back((float)r);
      colors[1].push_back((float)b);
      colors[1].push_back((float)g);
      colors[1].push_back((float)r);
      colors[2].push_back((float)b);
      colors[2].push_back((float)g);
      colors[2].push_back((float)r);
      */
#if 0
      // do check for touch point
      if (row == rowlookup[78] && col == collookup[178]) {
        if (x > 0.0) touchpoint[0] = x;// / radius; // --> 79 / 178
      }
      if (row == rowlookup[1] && col == collookup[178]) {
        if (x > 0.0) touchpoint[1] = x;// / radius; // --> 79 / 178
      }
#endif
    }
    p.numpoints = p.pts_raw.size();

    // (V) upscale
    // 2do: (ii) order point cloud
///    lloft::p_op.order();

    // (vi) upscale
  //  lloft::p_op.upscale(nrows, ncols);
  //  lloft::p_op.upscale_cart(nrows, ncols);
/*
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
*/
// 2do: merge with pointcloud.hpp
//    cloud.clear();
/*    vertices.clear();
    colors[0].clear();
    colors[1].clear();
    colors[2].clear();

    // 2do: copy over, simplify later
    for (int i = 0; i < p.numpoints; i++) // this for loop can be spared! MeasurmentPoint3D is proprietary, replace with PointXYZ
    {
      // (a) vertices
//      PointXYZI pt;
//      pt = cloud[i];
      lloft::point pt = p.pts_raw[i];
      vertices.push_back(pt.x);
      vertices.push_back(pt.y);
      vertices.push_back(pt.z);
      
      // (b) colors, precalc multiple colorizations (height, dist, intensity, ...)
      //     this needs 3 buffer objects on the gpu, alternative: fragment shader 
 //     float r, g, b;
      double hue;
      float height = (float)pt.z;
      float absDist = std::sqrt(pt.x * pt.x + pt.y * pt.y);

      r = g = b = 1;
//      hue = (1.0F - std::max<float>(COLOR_AUTOSAR_HEIGHT_MIN, std::min<float>(height, COLOR_AUTOSAR_HEIGHT_MAX)) / COLOR_AUTOSAR_HEIGHT_MAX) * (4.0F / 6.0F);
//      hsv2rgb(hue, 1.0F, 1.0F, r, g, b);
      colors[0].push_back((float)b);
      colors[0].push_back((float)g);
      colors[0].push_back((float)r);

//      hue = (1.0F - std::max<float>(0.0f, std::min<float>(absDist, COLOR_DISTANCE_MAX)) / COLOR_DISTANCE_MAX);
//      hsv2rgb(hue, 1.0F, 1.0F, r, g, b);
      colors[1].push_back((float)b);
      colors[1].push_back((float)g);
      colors[1].push_back((float)r);

      int ilow = 200;// pt.intensity;
//#define COLOR_LOWGAIN_MIN 0
//#define COLOR_LOWGAIN_MAX 1000//5000
      //    double hue = (1.0F - std::max<float>(COLOR_LOWGAIN_MIN, std::min<float>(ilow, COLOR_LOWGAIN_MAX)) / COLOR_LOWGAIN_MAX);
//      hue = (1.0F - std::max<float>(intensityclamp[0], std::min<float>(ilow, intensityclamp[1])) / intensityclamp[1]);
//      hsv2rgb(hue, 1.0F, 1.0F, r, g, b);
      colors[2].push_back((float)b);
      colors[2].push_back((float)g);
      colors[2].push_back((float)r);

    }
    */
  }
#endif // ECAL

  // b) load from .pcd or .bin
/*  void pointcloud_io::load_visualize(std::string filename)
  {
    static const float COLOR_DISTANCE_MAX_INV = 1.0F / 250.0F;
    //  static const float COLOR_AUTOSAR_HEIGHT_MIN = 0.0F;
    static const float COLOR_AUTOSAR_HEIGHT_MIN = -1.2F;
    //  static const float COLOR_AUTOSAR_HEIGHT_MAX = 3.0F;
    static const float COLOR_AUTOSAR_HEIGHT_MAX = 1.5F;
    static const float FLOAT_TO_S16Q7 = 128.0F;
    static const float S16Q7_TO_FLOAT = 1.0F / FLOAT_TO_S16Q7;
    // convert coloring parameters from float to fixpoint
    static const float COLOR_AUTOSAR_HEIGHT_MIN_S16Q7 = COLOR_AUTOSAR_HEIGHT_MIN * FLOAT_TO_S16Q7;
    static const float COLOR_AUTOSAR_HEIGHT_MAX_S16Q7 = COLOR_AUTOSAR_HEIGHT_MAX * FLOAT_TO_S16Q7;
    static const float COLOR_DISTANCE_MAX_INV_S16Q7 = COLOR_DISTANCE_MAX_INV / FLOAT_TO_S16Q7;

    p_cloud->clear();
    float loadtime = loadPointCloud(filename, *p_cloud); // returns load time in [sec], is < 0, if something failed
    if (loadtime < 0) return point_cloud(0, -1);

    // 2do: copy over, simplify later
    for (int i = 0; i < p_cloud->size(); i++) // this for loop can be spared! MeasurmentPoint3D is proprietary, replace with PointXYZ
    {
      MeasurementPoint3D pt;

      PointXYZI pt2;
      pt2 = (*p_cloud)[i];

      pt.x = pt2.x;
      pt.y = pt2.y;
      pt.z = pt2.z;
      pt.r = pt2.intensity;
      pt.g = pt.r;
      pt.b = pt.r;
      m_measPoints[0].push_back(pt);

      float r;
      float g;
      float b;
      double hue;
      float height = pt.z;
      float absDist = std::sqrt(pt.x * pt.x + pt.y * pt.y);

      switch (colorcoding)
      {
      case 1:
        // height
  //      float height = pt.z;
        hue = (1.0F - std::max<float>(COLOR_AUTOSAR_HEIGHT_MIN, std::min<float>(height, COLOR_AUTOSAR_HEIGHT_MAX)) / COLOR_AUTOSAR_HEIGHT_MAX) * (4.0F / 6.0F);
        hsv2rgb(hue, 1.0F, 1.0F, r, g, b);
        break;
      case 2:
        // distance
  //      float absDist = std::sqrt(pt.x * pt.x + pt.y * pt.y);
        hue = std::max<float>(0.0F, std::min<float>(absDist * COLOR_DISTANCE_MAX_INV, 1.0F)) * (4.0F / 6.0F);
        hsv2rgb(hue, 1.0F, 1.0F, r, g, b);
        break;
      }
      pt.r = r * 255 - 0.5;
      pt.g = g * 255 - 0.5;
      pt.b = b * 255 - 0.5;
      m_measPoints[1].push_back(pt);
    }

    return point_cloud(m_measPoints->size(), loadtime);
  }
  */
  enum e_pointcloudfiletype { ft_unknown=0, ft_PCD=1, ft_BIN=2, ft_CSV=3, ft_XYZ=4 };
  // pointcloud file being loaded
  struct s_pointcloudfile
  {
    std::string filepath;  // c:/leckere/erdbeermarmelade.honig
    std::string pathname;  // c:/leckere/
    std::string filename;  // erdbeermarmelade.honig
    std::string file_ext;  // .honig
    e_pointcloudfiletype ft;
  };


  struct PointXYZI
  {
    float x;
    float y;
    float z;
    float intensity;
  };
  typedef std::vector<PointXYZI> PointCloudI;

  PointCloudI cloud;

  int loadPCDFile(const std::string &filename, PointCloudI &cloud) // this can only read ASCII content files!, takes 0.8sec to load a 5MB .PCD (release build)
  {
    FILE *f;
    char line[255];
    char tmp0[55];
    char tmp1[55];
    char tmp2[55];
    char tmp3[55];
    float coord[4];
    bool bheader = true;

    ///  fopen_s(&f, filename.c_str(), "r");
///    if (!file_exists(filename.c_str())) return 0;
    f = fopen(filename.c_str(), "r");
    while (fgets(line, sizeof(line), f))
    {
      if (bheader) sscanf(line, "%s %s %s %s", tmp0, tmp1, tmp2, tmp3);

      if (!bheader)
      {
        //      sscanf(line, "%f %f %f %f", &coord[0], &coord[1], &coord[2]);
        sscanf(line, "%f %f %f %f", &coord[0], &coord[1], &coord[2], &coord[3]);
        PointXYZI p;
        p.x = coord[0];
        p.y = coord[1];
        p.z = coord[2];
        p.intensity = coord[3];
        cloud.push_back(p);
      }

      if (bheader)
        if (std::string(tmp0) == "DATA")// && (std::string(tmp2) == "ascii")) // omitting the "ascii" seems considerably faster !?
          bheader = false;
    }
    fclose(f);
    std::cout << "done with loading " << filename << std::endl;
    return 0;
  }

  float loadPointCloud(const std::string &filename, PointCloudI &cloud) // return load time [sec]
  {
    std::cout << "loading" << filename << std::endl;
    s_pointcloudfile pf;

    clock_t begin_time = clock();
    //  begin_time = clock();

#ifdef _WIN32				// nur f?r Windows
/*  pf.filepath = filename;
  std::string s = PathFindFileName(filename.c_str());
  pf.pathname = filename.substr(0, filename.find(s));
  pf.filename = s.substr(0, s.find_last_of("."));        // e.g. "veloout"
  pf.file_ext = PathFindExtension(filename.c_str());     // e.g. ".pcd"
  if (pf.file_ext.compare(".bin") == 0) pf.ft = ft_BIN;
  if (pf.file_ext.compare(".pcd") == 0) pf.ft = ft_PCD;
  if (pf.file_ext.compare(".csv") == 0) pf.ft = ft_CSV;  // Veloview output
  if (pf.file_ext.compare(".xyz") == 0) pf.ft = ft_XYZ;  // e.g. Cirrus dataset
*/  pf.ft = ft_PCD; // 2do!!
#else
///  fsys::path p = fsys::path(filename.c_str()); // C++17 :-)
    pf.ft = ft_PCD; // 2do!!
#endif
//  pclfile = pf; // set global var!

/*    if (pf.ft == ft_BIN)
    {
      if (loadPCDbin(filename, cloud) == -1)
      {
        //      PCL_ERROR("Couldn't read file .bin\n");
        system("pause");
        return (-1);
      }
    }
*/    if (pf.ft == ft_PCD)
    {
      std::cout << "loading pcd: " << filename << std::endl;
      if (loadPCDFile(filename, cloud) == -1)
      {
        //      PCL_ERROR("Couldn't read file .pcd\n");
        system("pause");
        return (-1);
      }
    }
/*    if (pf.ft == ft_CSV)
    {
      // there is no autoselect which csv-loader to use
      if (loadCSV_from_AEye_Endor(filename, cloud) == -1)
        //    if (loadCSV_from_VeloView(filename, cloud) == -1)
      {
        //      PCL_ERROR("Couldn't read file .csv\n");
        system("pause");
        return (-1);
      }
    }
    if (pf.ft == ft_XYZ)
    {
      if (loadCSV_from_VeloView(filename, cloud) == -1)
      {
        //      PCL_ERROR("Couldn't read file .csv\n");
        system("pause");
        return (-1);
      }
    }
*/    std::cout << "Loaded " << cloud.size() << " data points from .pcd/.bin with the following fields: " << std::endl;
    //  std::cout << "Load__ = " << float(clock() - begin_time) / CLOCKS_PER_SEC << " [s]" << std::endl;
    return float(clock() - begin_time) / CLOCKS_PER_SEC;
  }
}
#endif // POINTCLOUD_IO_H
