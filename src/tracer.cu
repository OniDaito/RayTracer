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
  Ray r;
  return r;
}


// Our actual kernel that runs over the screen and spits out a colour
__global__ void RenderKernel(RaytraceOptions options, float3 *output) {

  // assign a CUDA thread to every pixel (x,y) 
  // blockIdx, blockDim and threadIdx are CUDA specific keywords
  // replaces nested outer loops in CPU code looping over image rows and image columns 
  unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;   
  unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;

  unsigned int i = y * options.width + x; // index of current pixel (calculated using thread index)

  if (i < options.height * options.width){
    output[i].x = 1.0f; 
    output[i].y = 1.0f;
    output[i].z = 1.0f;
  }

}

void RaytraceKernelCUDA(RaytraceBitmap &bitmap, const RaytraceOptions &options, const Scene &scene)  {

  float3* output_h = new float3[options.width*options.height];  // pointer to memory for image on the host (system RAM)
	float3* output_d;                                             // pointer to memory for image on the device (GPU VRAM)

  // set output to black
  for (int i=0; i < options.width * options.height; ++i){
    output_h[i].x = output_h[i].y = output_h[i].z = 0.0f;
  } 

	// allocate memory on the CUDA device (GPU VRAM)
	cudaMalloc(&output_d, options.width * options.height * sizeof(float3));
  
	// dim3 is CUDA specific type, block and grid are required to schedule CUDA threads over streaming multiprocessors
	dim3 block(8, 8, 1);
	dim3 grid(options.width / block.x, options.height / block.y, 1);

	cout << "CUDA initialised. Start rendering..." << endl;
  cout << cudaGetLastError() << endl;
	
  // schedule threads on device and launch CUDA kernel from host
	RenderKernel <<< grid, block >>>(options, output_d);  
  cout << cudaGetErrorString(cudaGetLastError()) << endl;
  cudaDeviceSynchronize(); // Probably dont need this now

	// copy results of computation from device back to host
	// TODO - remove this double copy and go straight to bitmap if we can
  cudaMemcpy(output_h, output_d, options.width * options.height * sizeof(float3), cudaMemcpyDeviceToHost);  
 
	for (int x = 0; x < options.width; ++x) {
	  for (int y = 0; y < options.height; ++y) {
      float3 colour = output_h[ options.width * y + x] ; 
      bitmap.SetRGB(x,y,colour.x,colour.y,colour.z);
    } 
  }

  // free CUDA memory
	cudaFree(output_d);  

  delete[] output_h;


}
