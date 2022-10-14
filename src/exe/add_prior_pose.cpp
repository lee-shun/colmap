#include <fstream>
#include <iostream>
#include <string>

#include "base/database.h"

/**
 * locate the file to the line number
 * */
inline std::ifstream& SeekToLine(std::ifstream& in, const uint16_t line_nbr) {
  int i;
  char buf[1024];
  // locate to begin of the file
  in.seekg(0, std::ios::beg);
  for (i = 0; i < line_nbr; i++) {
    in.getline(buf, sizeof(buf));
  }
  return in;
}

bool GetGPSbyIdx(int id, std::string gps_path, Eigen::Vector3d& gps) {
  std::ifstream pose_fin_;
  pose_fin_.open(gps_path);
  if (!pose_fin_) {
    std::cout << "can not open in given path, no such file or directory! "
              << gps_path.c_str() << std::endl;
    return false;
  }
  std::string pose_tmp;
  std::vector<double> t_elements;
  SeekToLine(pose_fin_, id + 1);  // line num = id +1
  // read each id, lon, lat, alt, everytime
  for (int i = 0; i < 4; ++i) {
    if (!getline(pose_fin_, pose_tmp, ',')) {
      std::cout << "pose reading error! at index " << id << std::endl;
      return false;
    }
    t_elements.push_back(std::stold(pose_tmp));
  }
  gps = Eigen::Vector3d(t_elements[1], t_elements[2], t_elements[3]);
  return true;
}

int main(int argc, char** argv) {
  // useage add_prior_pose path_to_database path_to_gpspose
  if (argc != 3) {
    std::cout << "Usage: add_prior_pose path_to_database path_to_gpspose"
              << std::endl;
    return -1;
  }
  std::string datebase_path = argv[1];
  std::string gps_path = argv[2];

  std::cout << "Use database from: " << datebase_path << std::endl;
  std::cout << "Use gps from: " << gps_path << std::endl;

  colmap::Database database(datebase_path);
  std::vector<colmap::Image> images = database.ReadAllImages();
  for (int i = 0; i < images.size(); ++i) {
    colmap::Image& img = images[i];
    Eigen::Vector3d t;
    if (!GetGPSbyIdx(i, gps_path, t)) return -1;
    img.SetTvecPrior(t);
    database.UpdateImage(img);
  }

  database.Close();
  return 0;
}
