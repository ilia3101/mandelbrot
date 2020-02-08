#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// void writebmp(unsigned char * data, int width, int height, char * filename) {
//     int rowbytes = width*3+(4-(width*3%4))%4, imagesize = rowbytes*height, y;
//     unsigned short header[] = {0x4D42,0,0,0,0,26,0,12,0,width,height,1,24};
//     *((unsigned int *)(header+1)) = 26 + imagesize-((4-(width*3%4))%4);
//     FILE * file = fopen(filename, "wb");
//     fwrite(header, 1, 26, file);
//     for (y = 0; y < height; ++y) fwrite(data+(y*width*3), rowbytes, 1, file);
//     fwrite(data, width*3, 1, file);
//     fclose(file);
// }

/* Black and white! */
void writebmp(unsigned char * data, int width, int height, char * filename) {
    int rowbytes = width+(4-(width%4))%4, imagesize = rowbytes*height, y;
    unsigned short header[] = {0x4D42,0,0,0,0,26+256*3,0,12,0,width,height,1,8};
    *((unsigned int *)(header+1)) = 26 + imagesize-((4-(width%4))%4);

    FILE * file = fopen(filename, "wb");
    fwrite(header, 1, 26, file);

    uint8_t pallette[256*3]; for (int i = 0; i < 256; ++i) {
        pallette[i*3] = i;
        pallette[i*3+1] = i;
        pallette[i*3+2] = i;
    }
    fwrite(pallette, 1, 256*3, file);

    for (y = 0; y < height; ++y) fwrite(data+(y*width), rowbytes, 1, file);
    fwrite(data, width, 1, file);
    fclose(file);
}

#define FloatType double

int main()
{
    int samples = 1;
    int resultwidth = 1024;
    int resultheight = 1024;
    int width = resultwidth*samples;
    int height = resultheight*samples;

    uint8_t * image = calloc(width * height, sizeof(uint8_t));

    FloatType start_x = -0.72, end_x = -0.32, start_y = 0.37, end_y = 0.77;
    // FloatType start_x = -2, end_x = 2, start_y = -2, end_y = 2;

    FloatType c = 1.0;

    int max_iterations = 1200;

    for (int y = 0; y < height; ++y)
    {
        uint8_t * row = image + (y*width);

        if ((y % 100) == 0)
            printf("row %i\n", y);

        for (int x = 0; x < width; ++x)
        {
            uint8_t * pix = row + x;

            FloatType c_real = ((((FloatType)x + 0.5l) / (FloatType)width) * (end_x - start_x) + start_x);
            FloatType c_imaginary = ((((FloatType)y + 0.5l) / (FloatType)height) * (end_y - start_y) + start_y);
            FloatType z_real = 0.0l;
            FloatType z_imaginary = 0.0l;

            int i;

            for (i = 0; i < max_iterations; ++i)
            {
                /* Square it */
                FloatType z_real_2 = z_real*z_real - z_imaginary*z_imaginary + c_real;
                z_imaginary = 2*z_imaginary*z_real + c_imaginary;

                /* if magintude is larger than 2 */
                if ((z_real_2*z_real_2 + z_imaginary*z_imaginary) > 4.0l) break;
                z_real = z_real_2;
            }

            /* set pixel colour */
            if (i < max_iterations)
            {
                uint16_t pix_value = powl(((FloatType)i)/1000l, 0.66l) * 420.0l;
                if (pix_value > 255) pix_value = 255;
                *pix = pix_value;
            }
        }
    }

    /* Downsample */
    if (samples > 1)
    {
        uint8_t * image2 = calloc(resultwidth*resultheight,1);

        uint16_t * row_sums = alloca(width * sizeof(uint16_t));
        for (int y = 0; y < resultheight; ++y)
        {
            uint8_t * row_out = image2 + resultwidth*y;
            uint8_t * row = image + width * samples * y;

            /* Sum vertically */
            for (int x = 0; x < width; ++x) row_sums[x] = row[x];
            for (int y2 = y+1; y2 < y+samples; ++y2) {
                row += width;
                for (int x = 0; x < width; ++x) row_sums[x] += row[x];
            }

            /* sum horizontally and set output */
            for (int x = 0; x < resultwidth; ++x)
            {
                uint16_t * pix = row_sums + samples*x;
                uint16_t result = 0;
                for (int i = 0; i < samples; ++i) result += pix[i];

                result = result / (samples*samples);

                row_out[x] = result;
            }
        }
        writebmp(image2, resultwidth, resultheight, "maneblbrot.bmp");
    }
    else
    {
        writebmp(image, resultwidth, resultheight, "maneblbrot.bmp");
    }
    


    return 0;
}
