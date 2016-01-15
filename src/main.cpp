/**
* @brief Test raytracer for Apocrita
* @file main.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 7/01/2015
*
*/

#include <limits.h>
#include <stdlib.h> 
#include <getopt.h>
#include <string_utils.hpp>
#include <signal.h>

#include <random>
#include <pthread.h>
#include <simplex.hpp>
#include <iostream>
#include <fstream>

#include <omp.h>

#ifdef _USE_MPI
#include "mpi.hpp"
#endif

#include "main.hpp"
#include "geometry.hpp"
#include "tracer.hpp"
#include "bmp.hpp"

using namespace std;
using namespace s9;

int getRandomNumber() {
  return 4; // Chosen by fair dice roll
            // Guaranteed to be random
}
// Command line opttions jobby

void ParseCommandOptions (RaytraceOptions &options, int argc, const char * argv[]) {
  int c;
  int digit_optind = 0;
  static struct option long_options[] = {
      {"width", 1, 0, 0},
      {"height", 0, 0, 0},
      {NULL, 0, NULL, 0}
  };
  int option_index = 0;

  while ((c = getopt_long(argc, (char **)argv, "w:h:f:n:b:s:p:?x", long_options, &option_index)) != -1) {
  	int this_option_optind = optind ? optind : 1;
  	switch (c) {
      case 0 :
        break;
      case 'w' :
        options.width = FromStringS9<unsigned int>( std::string(optarg) );
        break;

      case 'f' :
        options.output_filename = std::string(optarg);
        break;

      case 's' :
        options.scene_filename = std::string(optarg);
        break;

      case 'n' :
        options.frame = FromStringS9<unsigned int>( std::string(optarg) );
        break;

      case 'b' :
        options.max_bounces = FromStringS9<unsigned int>( std::string(optarg) );
        break;

      case 'x' :
        options.live = true;
        break;

      case 'h' :
        options.height = FromStringS9<unsigned int>( std::string(optarg) );
        break;

      /*case 'p' :
        options.mpi_item_buffer = FromStringS9<unsigned int>( std::string(optarg) );
        break;*/

      case '?':
        std::cout << "Basic raytracer. Use -w and -h for size of output" << std::endl;
        break;

      default:
        std::cout << "?? getopt returned character code" << c << std::endl;
      }
  }
  
  if (optind < argc) {
      std::cout << "non-option ARGV-elements: " << std::endl;
      while (optind < argc)
          std::cout << argv[optind++];
      std::cout << std::endl;
  }

}


// Needs a bit more cleanup

void signal_callback_handler(int signum) {
  printf("Caught signal %d\n",signum);
  //window_running = false;
  exit(signum); 
}

// Our main function - sets up MPI and similar

int main (int argc, const char * argv[]) {
  // Register signal and signal handler
  signal(SIGINT, signal_callback_handler);

  // Default options setup
  RaytraceOptions options;

  options.width = 320;
  options.height = 240;
  options.max_bounces = 3;
  options.live = false;
  options.num_rays_per_pixel = 5;
  options.output_filename = "test.bmp";
  options.scene_filename = "none";

  ParseCommandOptions(options, argc, argv);

  std::cout << "Rendering size " << options.width << ", " << options.height <<  " for file: " << options.scene_filename << std::endl;

  Scene scene = CreateScene(options);

  // Create the main buffer for our frame
  
  RaytraceBitmap bitmap;
  
  // Set the background to black
  
  for (int i = 0; i < options.height; ++i){
    bitmap.push_back( std::vector< glm::vec3 >() );
    for (int j = 0; j < options.width; ++j){
      bitmap[i].push_back( glm::vec3(0,0,0) );
    }
  }

  // Main process - create our window as well if we want?
  
  RaytraceKernel(bitmap, options, scene);
 
  // Write out the bitmap
  
  WriteBitmap(bitmap, options);

}
