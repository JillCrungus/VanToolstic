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
        return (b4>>((pairNum)*2))&0x3;
    } else if (pairNum<=7) {
        return (b3>>((pairNum-4)*2))&0x3;
    } else if (pairNum<=11) {
        return (b2>>((pairNum-8)*2))&0x3;
    } else if (pairNum<=15) {
        return (b1>>((pairNum-12)*2))&0x3;
    }
    return 0;
}

static int getAlphaInterpolationFactor(int b1,int b2,int b3,int b4,int b5,int b6,int triNum) {
    int t123 = (b6<<16) | (b5<<8) | b4;
    int t456 = (b3<<16) | (b2<<8) | b1;

    if (triNum<=7) {
        return (t123>>((triNum)*3))&0x7;
    } else if (triNum<=15) {
        return (t456>>((triNum-8)*3))&0x7;
    }
    return 0;
}

static int interpolate2bit(int c1,int c2,int interp) {
    c1 *= 3-interp; c1 /= 3;
    c2 *= interp; c2 /= 3;
    return c1+c2;
}

static int interpolateAlpha3bit(int c1,int c2,int interp) {
    if (interp==0) {
        return c1;
    }
    if (interp==7) {
        return c2;
    }
    if (c1>c2) {
        c1 *= 7-interp; c1 /= 7;
        c2 *= interp; c2 /= 7;
        return c1+c2;
    } else {
        if (interp==5) {
            return 0;
        } else if (interp==6) {
            return 255;
        } else {
            c1 *= 5-interp; c1 /= 5;
            c2 *= interp; c2 /= 5;
            return c1+c2;
        }
    }
}

static int getDimFromSize(int c) {
    int bShift = 0;
    while (1<<bShift < c) { bShift++; }
    return 1<<(bShift/2);
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
    } else if (compressionFlag==0x1 || compressionFlag==0x2) {
        std::cout<<"Compression method is DXT2/3 (flag is "<<compressionFlag<<")\n";
        dxtVariant = 3;
    } else if (compressionFlag==0x3 || compressionFlag==0x4) {
        //NOTE: I don't think this flag is ever used in Whacked!
        //so the code that handles it is commented out
        std::cout<<"Compression method is DXT4/5\n";
        dxtVariant = 5;
    } else {
        std::cout<<"Unknown compression flag: "<<compressionFlag;
        return -1;
    }

    sourceFile.read(buf,4); int blockCount = *((int*)((void*)buf));
    std::cout<<"Block count: "<<blockCount<<"\n";

    sourceFile.read(buf,4); //TODO: figure out what this part of the header means

    std::vector<char> realBuf;
    int blockNum = 0;
    sourceFile.read(buf,4); //block size
    while (!sourceFile.eof()) {
        int blockSize = *(int*)((void*)buf);
        tBuf.resize(blockSize);
        sourceFile.read(tBuf.data(),blockSize); int bytesRead = sourceFile.gcount();
        if (bytesRead!=blockSize) {
            std::cout<<"PREMATURE EOF: "<<blockSize<<" "<<bytesRead<<"\n";
            return -1;
        }

        realBuf.clear();

        int colorPaletteSw[] = {
            0,3,1,2
        };

        int alphaPaletteSw[] = {
            0,7,1,2,3,4,5,6
        };

        int blockDims;
        if (dxtVariant==1) {
            blockDims = blockSize/8;
        } else if (dxtVariant==3 || dxtVariant==5) {
            blockDims = blockSize/16;
        }
        
        blockDims = getDimFromSize(blockDims);
        std::cout<<"Dimensions of block "<<blockNum<<": "<<blockDims<<" x "<<blockDims<<"\n";

        realBuf.resize(blockDims*4*blockDims*4*4);

        for (int i=0;i<blockDims*blockDims;i++) {
            int tInd;
            if (dxtVariant==1) {
                tInd = i*8;
            } else if (dxtVariant==3 || dxtVariant==5) {
                tInd = i*16+8;
            }
            int rX = (i%blockDims)*4;
            int rY = (i/blockDims)*4;

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
                    int intBits = colorPaletteSw[getInterpolationFactor(int1,int2,int3,int4,x+(3-y)*4)];

                    //fill the final buffer with interpolated color data
                    realBuf[rX*4+(rY*blockDims*4*4)+2+x*4+y*blockDims*4*4] = interpolate2bit(r1,r2,intBits); //r
                    realBuf[rX*4+(rY*blockDims*4*4)+1+x*4+y*blockDims*4*4] = interpolate2bit(g1,g2,intBits); //g
                    realBuf[rX*4+(rY*blockDims*4*4)+0+x*4+y*blockDims*4*4] = interpolate2bit(b1,b2,intBits); //b
                    realBuf[rX*4+(rY*blockDims*4*4)+3+x*4+y*blockDims*4*4] = 255; //a
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
                        if (x%2==1) {
                            a = alpha[(x/2)+y*2]>>4;
                        } else {
                            a = alpha[(x/2)+y*2]&0xf;
                        }
                        realBuf[rX*4+(rY*blockDims*4*4)+3+x*4+y*blockDims*4*4] = a<<4;
                    }
                }
            }

            /*if (dxtVariant==5) {
                //alpha + interpolation data
                int alpha[2];
                alpha[0] = ((unsigned char)tBuf[tInd-7]);
                alpha[1] = ((unsigned char)tBuf[tInd-8]);

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
                        int a = alphaPaletteSw[getAlphaInterpolationFactor(alphaInt[0],alphaInt[1],alphaInt[2],alphaInt[3],alphaInt[4],alphaInt[5],x+(3-y)*4)];
                        
                        int val = interpolateAlpha3bit(alpha[0],alpha[1],a);

                        realBuf[rX*4+(rY*blockDims*4*4)+3+x*4+y*blockDims*4*4] = 255;//interpolate3bit(alpha[0],alpha[1],a);

                        realBuf[rX*4+(rY*blockDims*4*4)+2+x*4+y*blockDims*4*4] = val>=0 ? val : 255; //r
                        realBuf[rX*4+(rY*blockDims*4*4)+1+x*4+y*blockDims*4*4] = val>=0 ? val : 0; //g
                        realBuf[rX*4+(rY*blockDims*4*4)+0+x*4+y*blockDims*4*4] = val>=0 ? val : 0; //b
                    }
                }
            }*/
        }

        FIBITMAP* fiBitmap = FreeImage_ConvertFromRawBits((unsigned char*)((void*)realBuf.data()),blockDims*4,blockDims*4,blockDims*4*4,32,0xFF0000,0x00FF00,0x0000FF,true);
        std::string saveName = std::string("block")+std::to_string(blockNum)+".png";
        FreeImage_Save(FIF_PNG,fiBitmap,saveName.c_str(),0);
        FreeImage_Unload(fiBitmap);

        std::cout<<"Wrote first block to block"<<blockNum<<".png\n";

        blockNum++;
        sourceFile.read(buf,4); //block size
    }
    sourceFile.close(); delete[] buf;
    
    return 0;
}
