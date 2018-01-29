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

static int getAlphaInterpolationFactor(int b1,int b2,int b3,int b4,int b5,int b6,int triNum) {
    if (triNum==0) {
        return b1>>5;
    } else if (triNum==1) {
        return (b1>>2)&0x7;
    } else if (triNum==2) {
        return ((b1&0x2)<<1) | (b2>>7);
    } else if (triNum==3) {
        return (b2>>4)&0x7;
    } else if (triNum==4) {
        return (b2>>1)&0x7;
    } else if (triNum==5) {
        return ((b2&0x1)<<2) | (b3>>6);
    } else if (triNum==6) {
        return (b3>>3)&0x7;
    } else if (triNum==7) {
        return b3&0x7;
    } else if (triNum==8) {
        return b4>>5;
    } else if (triNum==9) {
        return (b4>>2)&0x7;
    } else if (triNum==10) {
        return ((b4&0x2)<<1) | (b5>>7);
    } else if (triNum==11) {
        return (b5>>4)&0x7;
    } else if (triNum==12) {
        return (b5>>1)&0x7;
    } else if (triNum==13) {
        return ((b5&0x1)<<2) | (b6>>6);
    } else if (triNum==14) {
        return (b6>>3)&0x7;
    } else if (triNum==15) {
        return b6&0x7;
    }
    return 0;
}

static int interpolate2bit(int c1,int c2,int interp) {
    //TODO: improve accuracy?
    c1 *= interp; c1 /= 3;
    c2 *= 3-interp; c2 /= 3;
    return c1+c2;
}

static int interpolate3bit(int c1,int c2,int interp) {
    //TODO: improve accuracy?
    c1 *= interp; c1 /= 7;
    c2 *= 7-interp; c2 /= 7;
    return c1+c2;
}

int main() {
    std::string filename;
    std::cout<<"Enter the filename of the SCB file to extract a block from: ";
    std::cin>>filename;

    std::vector<char> tBuf; //temporary buffer containing raw block data
    tBuf.clear();
    char* buf = new char[28];

    std::ifstream sourceFile; sourceFile.open(filename.c_str(),std::ios_base::in|std::ios_base::binary);
    if (sourceFile.bad()) {
        std::cout<<"Failed to load \""<<filename<<"\"";
        return -1;
    }

    //HEADER
    sourceFile.read(buf,4); //magic number
    if (buf[0]!='.' || buf[1]!='S' || buf[2]!='C' || buf[3]!='B') {
        std::cout<<"Header magic number is not \".SCB\"";
        return -1;
    }
    sourceFile.read(buf,4); //TODO: figure out what this part of the header means
    
    sourceFile.read(buf,4); int width = *((int*)((void*)buf));
    sourceFile.read(buf,4); int height = *((int*)((void*)buf));
    std::cout<<"Dimensions: "<<width<<" x "<<height<<"\n";

    //TODO: verify validity of this!
    int dxtVariant = 0;
    sourceFile.read(buf,4); int compressionFlag = *((int*)((void*)buf));
    if (compressionFlag==0x0) {
        std::cout<<"Compression method is DXT1\n";
        dxtVariant = 1; 
    } else if (compressionFlag==0x1) {
        std::cout<<"Compression method is DXT3\n";
        dxtVariant = 3;
    } else if (compressionFlag==0x2) {
        std::cout<<"Compression method is DXT5\n";
        dxtVariant = 5;
    } else {
        std::cout<<"Unknown compression flag: "<<compressionFlag;
        return -1;
    }

    sourceFile.read(buf,4); int blockCount = *((int*)((void*)buf));
    std::cout<<"Block count: "<<blockCount<<"\n";

    sourceFile.read(buf,4); //TODO: figure out what this part of the header means

    sourceFile.read(buf,4); //block size
    while (!sourceFile.eof()) {
        int blockSize = *(int*)((void*)buf);
        int writeIndex = tBuf.size();
        tBuf.resize(tBuf.size()+blockSize);
        sourceFile.read(&(tBuf[writeIndex]),blockSize); int bytesRead = sourceFile.gcount();
        if (bytesRead!=blockSize) {
            std::cout<<"PREMATURE EOF: "<<blockSize<<" "<<bytesRead<<"\n";
            return -1;
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
        int tInd;
        if (dxtVariant==1) {
            tInd = i*8;
        } else if (dxtVariant==3 || dxtVariant==5) {
            tInd = i*16+8;
        }
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

        if (dxtVariant==3) {
            //alpha data
            int alpha[8];
            alpha[0] = ((unsigned char)tBuf[tInd-8]);
            alpha[1] = ((unsigned char)tBuf[tInd-7]);
            alpha[2] = ((unsigned char)tBuf[tInd-6]);
            alpha[3] = ((unsigned char)tBuf[tInd-5]);
            alpha[4] = ((unsigned char)tBuf[tInd-4]);
            alpha[5] = ((unsigned char)tBuf[tInd-3]);
            alpha[6] = ((unsigned char)tBuf[tInd-2]);
            alpha[7] = ((unsigned char)tBuf[tInd-1]);
            for (int x=0;x<4;x++) {
                for (int y=0;y<4;y++) {
                    int a;
                    if (x%2==0) {
                        a = alpha[(x/2)]>>4;
                    } else {
                        a = alpha[(x/2)]&0xf;
                    }
                    realBuf[rX*4+(rY*256*4)+3+x*4+y*256*4] = a<<4;
                }
            }
        }

        if (dxtVariant==5) {
            //alpha + interpolation data
            int alpha[2];
            alpha[0] = ((unsigned char)tBuf[tInd-8]);
            alpha[1] = ((unsigned char)tBuf[tInd-7]);

            int alphaInt[6];
            alphaInt[0] = ((unsigned char)tBuf[tInd-6]);
            alphaInt[1] = ((unsigned char)tBuf[tInd-5]);
            alphaInt[2] = ((unsigned char)tBuf[tInd-4]);
            alphaInt[3] = ((unsigned char)tBuf[tInd-3]);
            alphaInt[4] = ((unsigned char)tBuf[tInd-2]);
            alphaInt[5] = ((unsigned char)tBuf[tInd-1]);
            for (int x=0;x<4;x++) {
                for (int y=0;y<4;y++) {
                    //get the interpolation bits
                    int a = getAlphaInterpolationFactor(alphaInt[0],alphaInt[1],alphaInt[2],alphaInt[3],alphaInt[4],alphaInt[5],swizzle[x+y*4]);

                    realBuf[rX*4+(rY*256*4)+3+x*4+y*256*4] = interpolate3bit(alpha[0],alpha[1],a);
                }
            }
        }
    }

    FIBITMAP* fiBitmap = FreeImage_ConvertFromRawBits((unsigned char*)((void*)realBuf.data()),256,256,256*4,32,0xFF0000,0x00FF00,0x0000FF,true);
    FreeImage_Save(FIF_PNG,fiBitmap,"block1.png",0);
    FreeImage_Unload(fiBitmap);

    std::cout<<"Wrote first block to block1.png\n";
    return 0;
}
