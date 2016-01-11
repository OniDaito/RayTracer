

#include "mpi.hpp"

// The server process basically listens for messages
// and pulls in pixel values. TBF we could actually use
// the MPI map and fold functions?

void runServerProcess(int num_mpi_procs) {

  RaytraceBitmap bitmap;

  // Set the background to black
  for (int i = 0; i < options.height; ++i){
    bitmap.push_back( std::vector< glm::vec3 >() );
    for (int j = 0; j < options.width; ++j){
      bitmap[i].push_back( glm::vec3(0,0,0) );
    }
  }

  // Now listen to the clients and fill our pixel buffer

  long int pixel_count = 0;

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

    /*if (options.live){
      // XPutImage seems to crash if put inside the loop :S
      XPutImage(display, window, gc, render_image, 0, 0, 0, 0, options.width, options.height);
    }*/

  }

  // We've got all we need so write out 
  //writeBitmap(bitmap);

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


void MPISetup() {

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

}
