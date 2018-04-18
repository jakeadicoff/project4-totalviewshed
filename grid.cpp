#include <omp.h>
#include "grid.h"
using namespace std;

//constructor
Grid::Grid(string file_name, int vp_row_, int vp_col_) {
  this->num_cols = 0;
  this->num_rows = 0;
  this->vp_row = vp_row_;
  this->vp_col = vp_col_;

  read_file(file_name);

  vector<int> zero_int(num_cols*num_rows, 0);
  this->viewshed = zero_int;
}

//read in test file, populate row_major grid
void Grid::read_file(string file_name) {
  //open file
  ifstream file_stream;
  file_stream.open(file_name.c_str(), ios::in);

  if(!file_stream.good())
    cout << "Error: not able to open file" << endl;

  string line;

  //parse file for nrows, ncols, nodata, and data
  int i = 0;
  while(getline(file_stream,line)) {
    //keep track of metadata header
    if(i <= 5) header_string += line += "\n";

    //put grid data into row major grid
    if(i > 5) {
      //stackoverflow.com/questions/236129/most-elegant-way-to-split-a-string
      istringstream iss(line);

      vector<string> tokens{istream_iterator<string>{iss},
	  istream_iterator<string>{}};

      //stackoverflow.com/questions/20257582/convert-vectorstdstring-to-vectordouble
      vector<double> doubleVector(tokens.size());
      transform(tokens.begin(), tokens.end(), doubleVector.begin(),
		[](const std::string& val) { return std::stod(val); });

      //Append new line that was just read into file
      row_major.insert(row_major.end(), doubleVector.begin(), doubleVector.end());
    }

    //first line is num_cols
    else if(i == 0) sscanf(line.c_str(), "%*s %d", &num_cols);
    //second is num_rows
    else if(i == 1) sscanf(line.c_str(), "%*s %d", &num_rows);
    //second is num_rows
    else if(i == 5) sscanf(line.c_str(), "%*s %d", &nodata_val);

    i++; //increment line counter
  }

  // cout << "rows: " << num_rows << ", cols: " << num_cols
  //      << "; nodata value: " << nodata_val << endl;

  // cout << "Viewpoint is " << vp_row << "," << vp_col << endl;
}

//calculates if r,c is visible from viewpoint
int Grid::is_visible(int r, int c){
  //special cases
  if(viewshed[r*num_cols+c] == nodata_val) return nodata_val;
  else if(r == vp_row && c == vp_col) return 1;

  double vp_height = get_height(vp_row, vp_col);

  //elevation angle from viewpoint to target
  double target_elevation =
    (get_height(r, c)-vp_height)/distance_to_viewpoint(r, c);

  double azimuth = double(r - vp_row)/double(c - vp_col);

  //check for VERTICAL intersect: check values BETWEEN vp_col and c
  if(!std::isinf(azimuth)) {
    int increment = 0;
    int vert_range = 0;

    //establish range and direction of increment (low to high or high to low)
    if (vp_col < c) {
      increment = 1;
      vert_range = c - vp_col;
    }
    else{
      increment = -1;
      vert_range = vp_col - c;
    }

    //check every intersection height between viewpoint and target
    for(int i = 1; abs(i) < vert_range; i+=increment) {
      //calculate intersection x,y values
      double r_int_dub = vp_row + i*azimuth;
      if(r_int_dub < 0) r_int_dub = 0; //rounding error
      else if(r_int_dub > num_rows-1) r_int_dub = num_rows-1; //rounding error

      int r_int_floor = int(floor(r_int_dub));
      int r_int_ceil = int(ceil(r_int_dub));
      int c_int = vp_col + i;

      //nodata case: nodata automatically blocks visibility
      if(get_height(r_int_floor, c_int) == nodata_val ||
	 get_height(r_int_ceil, c_int) == nodata_val) return 0;

      //linear interpolation of intersection point
      double weight = r_int_dub - r_int_floor;
      double interp_height =
	get_height(r_int_floor, c_int) * (1-weight) +
	get_height(r_int_ceil, c_int) * weight;

      //elevation angle to intersection point
      double current_elevation =
	(interp_height - vp_height)/distance_to_viewpoint(r_int_dub, c_int);

      if(current_elevation > target_elevation) return 0;
    }
  }

  //check for HORIZONTAL intersect: check values BETWEEN vp_row and r
  if(azimuth != 0) {
    int increment = 0;
    int horiz_range = 0;

    //establish range and direction of increment (low to high or high to low)
    if (vp_row < r) {
      increment = 1;
      horiz_range = r - vp_row;
    }
    else{
      increment = -1;
      horiz_range = vp_row - r;
    }

    //check every intersection height between viewpoint and target
    for(int i = 1; abs(i) < horiz_range; i+=increment) {
      //calculate intersection x,y values
      double c_int_dub = vp_col + i*1/azimuth;
      if(c_int_dub < 0) c_int_dub = 0; //rounding error
      else if(c_int_dub > num_cols-1) c_int_dub = num_cols-1; //rounding error

      double c_int_floor = int(floor(c_int_dub));
      double c_int_ceil = int(ceil(c_int_dub));
      int r_int = vp_row + i;

      //nodata case: nodata automatically blocks visibility
      if(get_height(c_int_floor, r_int) == nodata_val ||
	 get_height(c_int_ceil, r_int) == nodata_val) return 0;

      //linear interpolation of intersection point
      double weight = c_int_dub - c_int_floor;
      double interp_height =
	get_height(r_int,c_int_floor) * (1-weight) +
	get_height(r_int,c_int_ceil) * weight;

      //elevation angle to intersection point
      double current_elevation =
	(interp_height - vp_height)/distance_to_viewpoint(c_int_dub, r_int);

      if(current_elevation > target_elevation) return 0;
    }
  }

  return 1;
}

//calculate visibility of every single point in grid
void Grid::generate_viewshed(int num_threads){
  omp_set_num_threads(num_threads);
#pragma omp parallel for collapse(2)
  for(int i = 0; i < num_rows; i++){
    for(int j = 0; j < num_cols; j++){
      viewshed[i*num_cols + j] = is_visible(i, j);
    }
  }
}

//write output asc file
void Grid::output_viewshed_file(string file_name, int num_threads) {
  double start_time = omp_get_wtime();
  generate_viewshed(num_threads);
  double time = omp_get_wtime() - start_time;

  cout << "Time (in seconds) to generate viewshed on " <<
    num_threads << " threads:\n" << time << endl;

  ofstream output_file;
  output_file.open(file_name);
  if(output_file.is_open()) {
    //write header string
    output_file << header_string;

    //write viewshed grid
    for(int i = 0; i < num_rows; i++){
      for(int j = 0; j < num_cols; j++){
	output_file << viewshed[i*num_cols + j] << " ";
      }
      output_file << "\n";
    }

    output_file.close();
    //    cout << "Viewshed written to file <" << file_name << ">" << endl;
  }
}

//DEBUGGING FUNCTIONS
template <class T>
void Grid::print_grid(vector<T> vec) {
  for(int i = 0; i < num_rows; i++){
    for(int j = 0; j < num_cols; j++){
      cout << vec[i*num_cols + j] << "\t";
    }
    cout << endl;
  }
}

void Grid::debug() {
  cout << "viewshed:" << endl;
  print_grid<int>(viewshed);

  cout << endl << "grid:" << endl;
  print_grid<double>(row_major);
}
