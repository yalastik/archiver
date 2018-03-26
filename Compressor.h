#include <string>

using namespace std;

#ifndef KDZ_COMPRESSOR_H
#define KDZ_COMPRESSOR_H


class Compressor {
public:
//    Compressor(int histSize, int prevSize){}
    Compressor() = default;

    virtual void encode(const string &filename_in, const string &filename_out){}
};

class Decompressor {
public:
    virtual void decode(const string &filename_in, const string &filename_out){}
};


#endif //KDZ_COMPRESSOR_H
