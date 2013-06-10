#define N (2048*2048) //number of threads
#define M 512 //number of threads per block
int main( void ) {
    int *a, *b, *c; // host copies of a, b, c
    int *dev_a, *dev_b, *dev_c; // device copies of a, b, c
    int arraySize = N * sizeof( int ); // need space for N integers

    // allocate device copies of a, b, c
    cudaMalloc( (void**)&dev_a, arraySize );
    cudaMalloc( (void**)&dev_b, arraySize );
    cudaMalloc( (void**)&dev_c, arraySize );

    a = (int*)malloc( arraySize ); 
    b = (int*)malloc( arraySize );
    c = (int*)malloc( arraySize );

	//fill the a and b arrays with randon integers
    random_ints( a, N ); 
    random_ints( b, N );
    
    // copy inputs to device
    cudaMemcpy( dev_a, a, arraySize, cudaMemcpyHostToDevice );
    cudaMemcpy( dev_b, b, arraySize, cudaMemcpyHostToDevice );
	
    // launch add() kernel with blocks and threads
    add<<< N/M, M >>>( dev_a, dev_b, dev_c );
	
    // copy device result back to host copy of c
    cudaMemcpy( c, dev_c, arraySize, cudaMemcpyDeviceToHost );
	
    free( a ); free( b ); free( c );
    cudaFree( dev_a );
    cudaFree( dev_b );
    cudaFree( dev_c );
    return 0;
}
    
__global__ void add( int *a, int *b, int *c ) {
    int threadId = threadIdx.x + blockDim.x*blockIdx.x;
	if(threadId < arraySize){
	    c[threadId] = a[threadId] + b[threadId];
	}
}
    