#include <fstream>
#include <string>
#include <stdlib.h>
#include <cmath>
#include "Types.hpp"

namespace SIPL {
template <class T>
class Volume{
    public:
       // Volume(std::string filename); // for reading mhd files
        Volume(const char * filename, int width, int height, int depth); // for reading raw files
        //Volume(int width, int height, int depth);
        Volume(int3 size);
        template <class U>
       // Volume(Volume<U> * otherVolume);
        //T get(int x, int y, int z) const;
		int get(int x, int y, int z) const {
			//if(!this->inBounds(x,y,z))
			//  throw OutOfBoundsException(x, y, z, this->width, this->height, this->depth, __LINE__, __FILE__);
			return (int)this->data[x+y*this->width+z*this->width*this->height];
		}
        //T get(int3 pos) const;
        //T get(int i) const;
        //void set(int x, int y, int z, T value);
        //void set(int3 pos, T v);
        //void set(int i, T v);
        //void set(Region region, T value);
        int getDepth() const;
		int getWidth() const;
        int getHeight() const;
        int3 getSize() const;
		void set(int3 pos, T v);
		 void save(const char * filepath);
       // void save(const char * filepath);
       
        //template <class U>
        //Volume<T> & operator=(const Volume<U> &otherVolume);
        //bool inBounds(int x, int y, int z) const;
        //bool inBounds(int3 pos) const;
        //bool inBounds(int i) const;
        //template <class U>
        //void convert(Volume<U> * otherImage) ;
        //int getTotalSize() const;
		T * data;
    private:
		
        int width, height;
		int depth;
};

template <class T>
Volume<T>::Volume(const char * filename, int width, int height, int depth) {
    // Read raw file
    this->data = new T[width*height*depth];
    FILE * file = fopen(filename, "rb");
    if(file == NULL)
        printf("file not found");//throw FileNotFoundException(filename, __LINE__, __FILE__);
    fread(this->data, sizeof(T), width*height*depth, file);
    fclose(file);
    this->width = width;
    this->height = height;
    this->depth = depth;
}

template <class T>
Volume<T>::Volume(int3 size) {
    this->data = new T[size.x*size.y*size.z];
    this->width = size.x;
    this->height = size.y;
    this->depth = size.z;
}

template <class T>
int Volume<T>::getWidth() const {
    return this->width;
}

template <class T>
int Volume<T>::getHeight() const {
    return this->height;
}

template <class T>
int Volume<T>::getDepth() const {
    return this->depth;
}

template <class T>
void Volume<T>::save(const char * filepath) {
    // This might not work for the defined struct types?
    FILE * file = fopen(filepath, "wb");
    //if(file == NULL)
      //  throw IOException(filepath, __LINE__, __FILE__);

    fwrite(this->data, sizeof(T), this->width*this->height*this->depth, file);
    fclose(file);
}
template <class T>
void Volume<T>::set(int3 pos, T value) {
    //this->set(pos.x, pos.y, pos.z, value);
	this->data[pos.x+pos.y*this->width+pos.z*this->width*this->height] = value;
}

template <class T>
int3 Volume<T>::getSize() const {
    int3 size;
    size.x = this->width;
    size.y = this->height;
    size.z = this->depth;
    return size;
}

}