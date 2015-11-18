/**
* @brief Test raytracer for Apocrita
* @file main.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 7/01/2015
*
*/
//__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <limits.h>
#include <stdlib.h> 
#include <getopt.h>
#include <string_utils.hpp>

#include <random>
#include <pthread.h>
#include <simplex.hpp>

#include <omp.h>

#ifdef _USE_MPI
#include <mpi.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "main.hpp"
#include "geometry.hpp"
#include "tracer.hpp"


using namespace std;
using namespace s9;

// Naughty Globals ><

RaytraceOptions options;

#ifdef _USE_MPI
MPI_Status stat;
#endif

Scene scene;

// X Display stuff
Display *display;
Window window;
int xid;
GC gc;
bool window_running = false;
XImage* render_image;
char* render_image_data; // sadly we need another buffer ><


// Pre-define for now - makes the file a bit neater
void writeBitmap (std::vector< std::vector< glm::vec3 > > &bitmap);


#ifdef _USE_MPI

// The server process basically listens for messages
// and pulls in pixel values. TBF we could actually use
// the MPI map and fold functions?

void runServerProcess(int num_mpi_procs) {

  std::vector< std::vector< glm::vec3 > > bitmap;

  // Set the background to black
  for (int i = 0; i < options.height; ++i){
    bitmap.push_back( std::vector< glm::vec3 >() );
    for (int j = 0; j < options.width; ++j){
      bitmap[i].push_back( glm::vec3(0,0,0) );
    }
  }

  // Now listen to the clients and fill our pixel buffer

  long int pixel_count = 0;

  // TODO - This -3 is a bug. Despite checking for divisions with
  // remainders in the main function, it still fails. V odd!
  while (pixel_count < (options.height * options.width)) {
    MPI_Status status;
    int source;
    int tag = 999;
    MPIPixel pixels[options.mpi_item_buffer];

    for (source = 1; source < num_mpi_procs; source++ ){
      MPI_Recv(&pixels,options.mpi_item_buffer,pixelType,source,tag,MPI_COMM_WORLD,&status);

      for (int i = 0; i < options.mpi_item_buffer; ++i){

        MPIPixel pixel = pixels[i];
        // Watch out for padding pixels
        if (!(pixel.x == -1 && pixel.y == -1)){
          unsigned long int colour;
          colour = static_cast<unsigned long> (255 * pixel.r) << 16 |
          static_cast<char> (255 * pixel.g) << 8 |
          static_cast<char> (255 * pixel.b);

          if (options.live){
            XPutPixel(render_image, pixel.x, pixel.y, colour);
          }

          bitmap[pixel.y][pixel.x] = glm::vec3(pixel.r,pixel.g,pixel.b);
          pixel_count++;
        } 

      }
  
    }
    if (options.live){
      // XPutImage seems to crash if put inside the loop :S
      XPutImage(display, window, gc, render_image, 0, 0, 0, 0, options.width, options.height);
    }

  }

  // We've got all we need so write out 
  writeBitmap(bitmap);

}


// Run a client process - return the colour data to our server with 
// the pixel we've coloured in.

void runClientProcess(long int offset, long int range) {

  std::vector<MPIPixel> pixel_buffer;
    
  for (long int i=0; i < range; ++i) {
    int x = (offset + i) % options.width;
    int y = (offset + i) / options.width;

    glm::vec3 colour = fireRays(x,y,
      options.width, options.height,
      options.perspective,
      options.near_plane,
      options.far_plane,
      scene,
      options.num_bounces,
      options.num_rays_per_pixel);

    int dest = 0;
    int tag = 999;

    MPIPixel pixel;
    pixel.x = x;
    pixel.y = y;
    pixel.r = colour.x;
    pixel.g = colour.y;
    pixel.b = colour.z;
  
    pixel_buffer.push_back(pixel);

    if (pixel_buffer.size() == options.mpi_item_buffer || i+1 == range){
      // Pad out the data if needed
      
      while (pixel_buffer.size() < options.mpi_item_buffer){
        MPIPixel padding;
        padding.x = -1;
        padding.y = -1;
        pixel_buffer.push_back(padding);
      }

      MPI_Send(&pixel_buffer[0], options.mpi_item_buffer,pixelType,dest,tag,MPI_COMM_WORLD);
      pixel_buffer.clear();

    }
  } 
}

#endif

// OpenMP Version of the same code

void runProcess() {
   std::vector< std::vector< glm::vec3 > > bitmap;

  // Set the background to black
  for (int i = 0; i < options.height; ++i){
    bitmap.push_back( std::vector< glm::vec3 >() );
    for (int j = 0; j < options.width; ++j){
      bitmap[i].push_back( glm::vec3(0,0,0) );
    }
  }
  
  int offset = 0;

  // Main rendering section goes here
  for (int i = 0; i < options.height; ++i ){
    for (int j = 0; j < options.width; ++j ) {
      int x = j;
      int y = i;

      glm::vec3 ray_colour = fireRays(x,y,
        options.width, options.height,
        options.perspective,
        options.near_plane,
        options.far_plane,
        scene,
        options.num_bounces,
        options.num_rays_per_pixel);


      unsigned long int colour;
      colour = static_cast<unsigned long> (255 * ray_colour.x) << 16 |
      static_cast<char> (255 * ray_colour.y) << 8 |
      static_cast<char> (255 * ray_colour.z);

      if (options.live){
        XPutPixel(render_image, x, y, colour);
      }

      bitmap[y][x] = ray_colour;
    }

    if (options.live){
    // XPutImage seems to crash if put inside the loop :S
    XPutImage(display, window, gc, render_image, 0, 0, 0, 0, options.width, options.height);
    }
  } 
  
  // We've got all we need so write out 
  writeBitmap(bitmap);
}


// Command line opttions jobby

void parseCommandOptions (int argc, const char * argv[]) {
  int c;
  int digit_optind = 0;
  static struct option long_options[] = {
      {"width", 1, 0, 0},
      {"height", 0, 0, 0},
      {NULL, 0, NULL, 0}
  };
  int option_index = 0;

  while ((c = getopt_long(argc, (char **)argv, "w:h:f:n:b:p:?x", long_options, &option_index)) != -1) {
  	int this_option_optind = optind ? optind : 1;
  	switch (c) {
      case 0 :
        break;
      case 'w' :
        options.width = FromStringS9<unsigned int>( std::string(optarg) );
        break;

      case 'f' :
        options.filename = std::string(optarg);
        break;

      case 'n' :
        options.frame = FromStringS9<unsigned int>( std::string(optarg) );
        break;

      case 'b' :
        options.num_bounces = FromStringS9<unsigned int>( std::string(optarg) );
        break;

      case 'x' :
        options.live = true;
        break;

      case 'h' :
        options.height = FromStringS9<unsigned int>( std::string(optarg) );
        break;

      case 'p' :
        options.mpi_item_buffer = FromStringS9<unsigned int>( std::string(optarg) );
        break;

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


// Write the bitmap out as an actual file
void writeBitmap (std::vector< std::vector< glm::vec3 > > &bitmap) {

  // TODO - Proper C++ style casts please!

  // Bitmap header
  struct __attribute__ ((packed)) bmp24_file_header
  {
    char        magic1;       // 'B'
    char        magic2;       // 'M'
    int         size;         // 0
    short int   reserved1;    // 0
    short int   reserved2;    // 0
    int         offbytes;      // 14 + 40
  };

  // Bitmap info header
  struct __attribute__ ((packed)) bmp24_info_header
  {
    int        size;                // 40
    int        width;               // pic.width
    int        height;              // pic.height
    short int   planes;             // 1
    short int   bit_count;          // 24
    int        compression;         // 0
    int        size_image;          // (pic.width * 3 + extra_bytes) * pic.height
    int        x_pels_per_meter;    // 2952
    int        y_pels_per_meter;    // 2952
    int        clr_used;            // 0
    int        clr_important;       // 0
  };


  ofstream myfile(options.filename,  ios::out | std::ios::binary);

  //std::cout << sizeof(bmp24_file_header) << "," << sizeof(bmp24_info_header) << std::endl;

  bmp24_file_header header;
  bmp24_info_header info;

  int extra_bytes = (4 - (options.width * 3) % 4) % 4;

  // Write the header to the bmp
  header.magic1 = 'B';
  header.magic2 = 'M';
  header.size = 0; // (options.width * 3 + extra_bytes) * options.height + 14 + 40;
  header.reserved1 = 0;
  header.reserved2 = 0;
  header.offbytes =  sizeof(bmp24_file_header) + sizeof(bmp24_info_header);

  // Write the header

  info.size = sizeof(bmp24_info_header);
  info.width = static_cast<long>(options.width);
  info.height = static_cast<long>(options.height);
  info.planes = 1;
  info.bit_count = 24;
  info.compression = 0;
  info.size_image = (options.width * 3 + extra_bytes) * options.height;
  info.x_pels_per_meter = 2952;
  info.y_pels_per_meter = 2952;
  info.clr_used = 0;
  info.clr_important = 0;

  myfile.write( reinterpret_cast<char *>(&header), sizeof(header));
  myfile.write( reinterpret_cast<char *>(&info), sizeof(info));

  for (int y = options.height - 1; y >= 0; --y ){

    for (int x = 0; x < options.width; ++x){
      glm::vec3 colour = bitmap[y][x];
      char r = char( floor( colour.x * 255));
      char g = char( floor( colour.y * 255));
      char b = char( floor( colour.z * 255));
      myfile << r << g << b;
    }
    for (int b = 0; b < extra_bytes; ++b){
      myfile << 'a';
    }
  }
  
  myfile.close();
}


// Create some test geometry for our scene
void setScene(){

  // Test Spheres

  Sphere s0(glm::vec3(0.5f, 0.8f, 2.5f), 1.0f);
  Sphere s1(glm::vec3(1.3f, 0.1f, 3.5f), 0.75f);
  Sphere s2(glm::vec3(-1.2f, 0.2f, 2.5f), 0.75f);
  Sphere s3(glm::vec3(0.0f, -0.1f, 2.0f), 0.75f);

  s0.material.shiny = 0.3;
  s1.material.shiny = 0.1;
  s3.material.shiny = 0.9;

  s0.material.colour = glm::vec3(0.0f,0.0f,1.0f);
  s1.material.colour = glm::vec3(1.0f,0.0f,0.0f);
  s2.material.colour = glm::vec3(0.0f,1.0f,1.0f);
  s3.material.colour = glm::vec3(1.0f,1.0f,1.0f);

  scene.spheres.push_back(s0);
  scene.spheres.push_back(s1);
  scene.spheres.push_back(s2);
  scene.spheres.push_back(s3);

  // Lights

  LightPoint l0;
  l0.pos = glm::vec3(1.0f,5.0f,5.0f);
  l0.colour = glm::vec3(0.1f,0.1f,0.1f);
  scene.lights.push_back(l0);

  LightPoint l1;
  l1.pos = glm::vec3(1.0f,0.0f,-1.0f);
  l1.colour = glm::vec3(0.1f,0.1f,0.1f);
  scene.lights.push_back(l1);


  LightPoint l2;
  l1.pos = glm::vec3(-5.0f,5.0f,15.0f);
  l1.colour = glm::vec3(0.1f,0.0f,0.1f);
  scene.lights.push_back(l2);
}

void* windowThread(void* id) {
  XEvent e;
  while (1) {
    XNextEvent(display, &e);
    if (e.type == Expose) {

    }
   
    if (e.type == KeyPress)
      break;
  }
    
  XCloseDisplay(display);
}


void createWindow() {

  std::cout << "Creating live view window..." << std::endl;

  // Allocate XImage Buffer

  XInitThreads();

  display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
  }

  xid = DefaultScreen(display);

  window = XCreateSimpleWindow(display, RootWindow(display, xid), 10, 10, options.width, options.height, 1,
                         BlackPixel(display, xid), WhitePixel(display, xid));

  XGCValues values;
  /* which values in 'values' to check when creating the GC. */

  values.cap_style = CapButt;
  values.join_style = JoinBevel;

  unsigned long valuemask = GCCapStyle | GCJoinStyle;
  
  gc = XCreateGC(display,window,valuemask, &values);
  
  int depth = DefaultDepth(display, DefaultScreen(display));

  XSelectInput(display, window, ExposureMask | KeyPressMask);
  XMapWindow(display, window);

  Visual *visual=DefaultVisual(display, 0);

  render_image_data = new char[ 4 * options.width * options.height];

  render_image = XCreateImage(display, visual, depth, ZPixmap, 0, render_image_data,options.width,options.height, 32,0);

  pthread_t thread;
  int rc = pthread_create(&thread, NULL, windowThread,nullptr);
  if(rc){
    std::cout<< "ERROR; return code from pthread_create() is " << rc << std::endl;
  }
  window_running = true;
}



// Our main function - sets up MPI and similar

int main (int argc, const char * argv[]) {

#ifdef _USE_MPI
  // Naughty! Stripping const which is a tad bad
  MPI_Init(&argc, const_cast<char***>(&argv));

  int numprocs;
  int myid;
  int length_name;
  char name[200];

  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);
  
  MPI_Get_processor_name(name, &length_name);

  std::cout << "MPI NumProcs: " << numprocs << ", id: " << myid << ", name: " << name << std::endl;  
#endif


  // Defaults
  options.width = 320;
  options.height = 240;
  options.num_bounces = 3;
  options.live = false;
  options.num_rays_per_pixel = 1;
  options.near_plane = 0.1f;
  options.far_plane = 100.0f;
  options.mpi_item_buffer = 100;
  options.filename = "teapot.obj";

  parseCommandOptions(argc,argv);

  std::cout << "Rendering size " << options.width << ", " << options.height <<  " for file: " << options.filename << std::endl;

  options.perspective = glm::perspective(35.0f, static_cast<float>(options.width) / static_cast<float>(options.height), 
    options.near_plane, options.far_plane);

  setScene();

#ifdef _USE_MPI
  // Setup MPI Type
  // Setup description of the 3 MPI_FLOAT fields x, y, z,
  MPI_Aint  offsets[2], extent; 
  MPI_Datatype oldtypes[2];
  int blockcounts[2]; 

  offsets[0] = 0; 
  oldtypes[0] = MPI_FLOAT; 
  blockcounts[0] = 3; 
  // Setup description of the 2 MPI_INT fields x, y 
  // Need to first figure offset by getting size of MPI_FLOAT 
  MPI_Type_extent(MPI_FLOAT, &extent); 
  offsets[1] = 3 * extent; 
  oldtypes[1] = MPI_INT; 
  blockcounts[1] = 2; 

  MPI_Type_struct(2, blockcounts, offsets, oldtypes, &pixelType); 
  MPI_Type_commit(&pixelType); 

  // Make some decisions about client and server processes

  // We need to divide up the workload on our machines
  if (numprocs > 1){
    if (myid == 0 ) { 

      // Main process - create our window
      if (options.live){
        createWindow();
      } 
      
      runServerProcess(numprocs);

    } else {
      // Divide up our image amongst the processes
      long int range = options.width * options.height / (numprocs-1);

      // We may have leftovers though so the last id can deal with these
      if (myid == numprocs-1){
        int extra =  (options.width * options.height) % (numprocs-1); 
        runClientProcess(range * (myid-1), range + extra);
      } else{
        runClientProcess(range * (myid-1), range);
      }
    }
  }

  MPI_Finalize();

  std::cout << "Rendered all pixels." << std::endl;
#else
  // Main process - create our window
  if (options.live){
    createWindow();
  }
  runProcess();

#endif

  if (options.live){
    while(window_running){

    }
  }

}
