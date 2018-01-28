#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <FreeImage.h>

static int makeR5(int b1,int b2) {
    int r = b1>>3; r*=0xFF; r/=0x1F;
    return r;
}

static int makeG6(int b1,int b2) {
    int g = (b1&0x07)<<3 | (b2>>5); g*=0xFF; g/=0x3F;
    return g;
}

static int makeB5(int b1,int b2) {
    int b = b2&0x1F; b*=0xFF; b/=0x1F;
    return b;
}

static int getInterpolationFactor(int b1,int b2,int b3,int b4,int pairNum) {
    if (pairNum<=3) {
        return (b1>>((pairNum)*2))&0x3;
    } else if (pairNum<=7) {
        return (b2>>((pairNum-4)*2))&0x3;
    } else if (pairNum<=11) {
        return (b3>>((pairNum-8)*2))&0x3;
    } else if (pairNum<=15) {
        return (b4>>((pairNum-12)*2))&0x3;
    }
    return 0;
}

static int interpolate2bit(int c1,int c2,int interp) {
    //TODO: improve accuracy?
    c1 *= interp; c1 /= 3;
    c2 *= 3-interp; c2 /= 3;
    return c1+c2;
}

int main() {
    std::string filename;
    std::cout<<"Enter the filename of the SCB file to extract a block from: ";
    std::cin>>filename;

    std::vector<char> tBuf; //temporary buffer containing raw block data
    tBuf.clear();
    char* buf = new char[28];

    //TODO: add error checking
    std::ifstream sourceFile; sourceFile.open(filename.c_str(),std::ios_base::in|std::ios_base::binary);

    sourceFile.read(buf,28); //header (TODO: parse instead of skip)
    int blockCount = 0;


    sourceFile.read(buf,4); //block size
    while (!sourceFile.eof()) {
        blockCount++;
        int blockSize = *(int*)((void*)buf);
        int writeIndex = tBuf.size();
        tBuf.resize(tBuf.size()+blockSize);
        sourceFile.read(&(tBuf[writeIndex]),blockSize); int bytesRead = sourceFile.gcount();
        if (bytesRead!=blockSize) {
            std::cout<<"PREMATURE EOF: "<<blockSize<<" "<<bytesRead<<"\n";
        }
        sourceFile.read(buf,4); //block size
    }
    sourceFile.close(); delete[] buf;
    std::vector<char> realBuf; realBuf.clear();

    realBuf.resize(256*256*4);

    int swizzle[] = {
        0,1,2,3,
        4,5,6,7,
        8,9,10,11,
        12,13,14,15
    };

    //TODO: properly determine the dimensions of the first block,
    //(sqrt of block size?) and also load every block
    for (int i=0;i<64*64;i++) {
        //ASSUMING DXT1 (TODO: make this able to read all other compression formats)
        int tInd = i*8;
        int rX = (i%64)*4;
        int rY = (i/64)*4;

        //color data
        int b1a = ((unsigned char)tBuf[tInd+1]);
        int b1b = ((unsigned char)tBuf[tInd]);

        int b2a = ((unsigned char)tBuf[tInd+3]);
        int b2b = ((unsigned char)tBuf[tInd+2]);

        //interpolation data / color palette
        int int1 = tBuf[tInd+4];
        int int2 = tBuf[tInd+5];
        int int3 = tBuf[tInd+6];
        int int4 = tBuf[tInd+7];

        //convert r5g6b5 to r8g8b8
        int r1 = makeR5(b1a,b1b);
        int g1 = makeG6(b1a,b1b);
        int b1 = makeB5(b1a,b1b);
        int r2 = makeR5(b2a,b2b);
        int g2 = makeG6(b2a,b2b);
        int b2 = makeB5(b2a,b2b);

        for (int x=0;x<4;x++) {
            for (int y=0;y<4;y++) {
                //get the interpolation bits
                int intBits = getInterpolationFactor(int1,int2,int3,int4,swizzle[x+y*4]);
                //HACKS
                intBits+=3; intBits%=4;
                //fill the final buffer with interpolated color data
                realBuf[rX*4+(rY*256*4)+2+x*4+y*256*4] = interpolate2bit(r1,r2,intBits); //r
                realBuf[rX*4+(rY*256*4)+1+x*4+y*256*4] = interpolate2bit(g1,g2,intBits); //g
                realBuf[rX*4+(rY*256*4)+0+x*4+y*256*4] = interpolate2bit(b1,b2,intBits); //b
                realBuf[rX*4+(rY*256*4)+3+x*4+y*256*4] = 255; //a
            }
        }
    }

    FIBITMAP* fiBitmap = FreeImage_ConvertFromRawBits((unsigned char*)((void*)realBuf.data()),256,256,256*4,32,0xFF0000,0x00FF00,0x0000FF,true);
    FreeImage_Save(FIF_PNG,fiBitmap,"block1.png",0);
    FreeImage_Unload(fiBitmap);

    std::cout<<"Wrote first block to block1.png\n";
    return 0;
}
