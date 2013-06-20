extern float image[HEIGHT][WIDTH]; //image to be segmented
extern float phi[HEIGHT][WIDTH]; //representation of the zero level set interface
extern int label[HEIGHT][WIDTH];//contains only integer values between -3 and 3

void __global__ prepareUpdates1(float *phiD, int *layerD, float *imageD);
void __global__ prepareUpdates2(float *phiD, int *layerD, int *labelD);
void __global__ prepareUpdates3(float *phiD, int *layerD, int *labelD);
void __global__ prepareUpdates4(float *phiD, int *layerD, int *labelD);

void __global__ updateLevelSets1(float *phiD, int *layerD, int *labelD);
void __global__ updateLevelSets2(int *layerD, int *labelD);
void __global__ setVariablesInDevice(float threshold, float epsilon, float alpha, float image[HEIGHT][WIDTH]);