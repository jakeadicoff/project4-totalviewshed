#ifndef __grid_h
#define	__grid_h

#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <string>
#include <time.h>
#include <math.h>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iomanip>

using namespace std;

class Grid{
 public:
  Grid(string file_name, int vp_row_, int vp_col_);

  //Outputs viewshed data to specified file
  void output_viewshed_file(string file_name, int num_threads);
  void debug();

 private:
  vector <double> row_major;
  vector <int> viewshed;

  int num_rows, num_cols, vp_row, vp_col;
  int nodata_val;
  string header_string;

  //reads file and populates class variables
  void read_file(string file_name);

  //calculate visibility of every single point in grid
  void generate_viewshed(int num_threads);

  //calculates if r,c is visible from viewpoint
  int is_visible(int r, int c);

  //row_major accessor function
  double get_height(int r, int c) {return row_major[r*num_cols + c];};

  //distance formula, from (r,c) to viewpoint
  double distance_to_viewpoint(double r, double c) { return sqrt(pow(r-vp_row,2)+pow(c-vp_col,2)); };

  template <class T>
    void print_grid(vector<T> vec);
};

#endif
