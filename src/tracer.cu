/**
* @brief Test raytracer for Apocrita
* @file tracer.cu
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 14/07/2016
*
*/


#include "tracer_cuda.hpp"

using namespace std;

__device__ Ray GenerateRay() {
}


// Our actual kernel that runs over the screen and spits out a colour
__global__ void RenderKernel(float3 *output) {

}

void RaytraceKernelCUDA(RaytraceBitmap &bitmap, const RaytraceOptions &options, const Scene &scene)  {

  float3* output_h = new float3[options.width*options.height];  // pointer to memory for image on the host (system RAM)
	float3* output_d;                                             // pointer to memory for image on the device (GPU VRAM)

	// allocate memory on the CUDA device (GPU VRAM)
	cudaMalloc(&output_d, options.width * options.height * sizeof(float3));
        
	// dim3 is CUDA specific type, block and grid are required to schedule CUDA threads over streaming multiprocessors
	dim3 block(8, 8, 1);   
	dim3 grid(options.width / block.x, options.height / block.y, 1);

	cout << "CUDA initialised. Start rendering..." << endl;
 
	// schedule threads on device and launch CUDA kernel from host
	RenderKernel <<< grid, block >>>(output_d);  

	// copy results of computation from device back to host
	cudaMemcpy(output_h, output_d, options.width * options.height * sizeof(float3), cudaMemcpyDeviceToHost);  
 
	// free CUDA memory
	cudaFree(output_d);  

	for (int x = 0; x < options.width; ++x) {
	  for (int y = 0; y < options.height; ++y) {
      float3 colour = output_h[ options.width * y  + x * 3] ; 
      bitmap[y][x][0] = static_cast<unsigned int>(floor(256.0 / colour.x)); 
      bitmap[y][x][1] = static_cast<unsigned int>(floor(256.0 / colour.y)); 
      bitmap[y][x][2] = static_cast<unsigned int>(floor(256.0 / colour.z)); 
    } 
  }
	
  delete[] output_h;


}
