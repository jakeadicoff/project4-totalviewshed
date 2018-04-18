#include "main.h"

using namespace std;

int main(int argc, char** argv) {

  if(argc < 5) {
    cout << "Not enough arguments:\n" <<
      "arg 1: input file name\n" <<
      "arg 2: output file name\n" <<
      "arg 3: row of viewpoint\n" <<
      "arg 4: col of viewpoint\n" <<
      "arg 5: number of threads to use" << endl;
    exit(0);
  }

  string input_file_name = argv[1];
  string output_file_name = argv[2];
  int vp_row = atoi(argv[3]);
  int vp_col = atoi(argv[4]);
  int num_threads = atoi(argv[5]);

  Grid my_grid(input_file_name, vp_row, vp_col);
  my_grid.output_viewshed_file(output_file_name, num_threads);

  //  my_grid.debug();
  return 0;
}
