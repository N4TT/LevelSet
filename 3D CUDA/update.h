extern float image[HEIGHT][WIDTH][DEPTH]; //image to be segmented
extern float phi[HEIGHT][WIDTH][DEPTH]; //representation of the zero level set interface
extern int label[HEIGHT][WIDTH][DEPTH];//contains only integer values between -3 and 3

__global__ void prepareUpdates1(float *phiD, int *layerD, float *imageD);
__global__ void prepareUpdates2(float *phiD, int *layerD);
__global__ void prepareUpdates3(float *phiD, int *layerD, int *labelD);
__global__ void prepareUpdates4(float *phiD, int *layerD, int *labelD);
__global__ void prepareUpdates5(float *phiD, int *layerD, int *labelD);
__global__ void updateLevelSets1(float *phiD, int *layerD, int *labelD);
__global__ void updateLevelSets2(int *layerD, int *labelD);
__global__ void setVariablesInDevice(float threshold, float epsilon, float alpha);
