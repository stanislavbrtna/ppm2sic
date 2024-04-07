#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

uint8_t svp_fread_u8(FILE *fp) {
  uint8_t x;

  fread(&x, sizeof(x), 1, fp);
  return x;
}

void svp_fwrite_u8(FILE *fp, uint8_t val) {
  if(fwrite(&val, sizeof(uint8_t), 1, fp) == 0){
    //printf("fwrite error! (1)\n");
  }
}

int main(int argc, char *argv[]) {

  FILE *fPointer;

	uint8_t ch[16];
	uint8_t ch2 = 0;
	uint16_t a, color;
	uint32_t fpos = 0;
	uint32_t fpos_line_begin;
	uint8_t r, g, b;
	uint16_t img_width = 0;
	uint16_t img_height = 0;
	int16_t xi = 0;
	int16_t yi = 0;
	uint8_t n, laneScaleCnt;
	uint8_t scale;
	uint8_t fnameBuffer[512];


	uint8_t * filenameIn, * filenameOut;
	uint8_t compression = 0;

  if (argc < 2) {
    printf("usage: %s [input.ppm] (output.sic)\n", argv[0]);
    return 0;
  }

	filenameIn = argv[1];

	if (argc > 2) {
	  filenameOut = argv[2];
  } else {
    int i=0;
    while((argv[1])[i] != 0){
      fnameBuffer[i] = (argv[1])[i];
      i++;
    }
    fnameBuffer[i] = 0;
    fnameBuffer[i - 3] = 's';
    fnameBuffer[i - 2] = 'i';
    fnameBuffer[i - 1] = 'c';
    printf("setting new fname: %s\n", fnameBuffer);
    filenameOut = fnameBuffer;
  }
  
  fPointer = fopen(filenameIn, "rb");

	if (!fPointer) {
	  printf("Error while opening file %s!\n", filenameIn);
	  return 1;
	}
  
	ch[0] = svp_fread_u8(fPointer);
	ch[1] = svp_fread_u8(fPointer);
	if ((ch[0] != 'P') && (ch[1] != '6')) {
		printf("draw_ppm: Error: wrong header\n");
		return 1;
	}
	fpos = 3;
	fseek(fPointer, sizeof(uint8_t) * (fpos), SEEK_SET);
  if(svp_fread_u8(fPointer) == '#') {	
	  while (ch2 != 10) {
	    fseek(fPointer, sizeof(uint8_t) * (fpos), SEEK_SET);
		  ch2 = svp_fread_u8(fPointer);
		  fpos++;
	  }
	}
  
  ch2 = 0;
  a = 0;
  while (ch2 != 10) {
	  fseek(fPointer, sizeof(uint8_t) * (fpos), SEEK_SET);
	  ch2 = svp_fread_u8(fPointer);
	  ch[a] = ch2;
	  fpos++;
	  a++;
  }
  ch[a] = 10;
	a = 0;
	while (ch[a] != ' ') {
		img_width *= 10;
		img_width += ch[a] - 48;
		a++;
	}
	a++;
	while (ch[a] != 10) {
		img_height *= 10;
		img_height += ch[a] - 48;
		a++;
	}

	ch2 = 0;

  printf("Image size: w: %u, h: %u \n", img_width, img_height);
  
  if (img_width != 32 || img_height != 32) {
    printf("Error: Only 32x32 images are supported");
  }

	while(ch2 != 10) {
	  fseek(fPointer, sizeof(uint8_t) * (fpos), SEEK_SET);
		ch2 = svp_fread_u8(fPointer);
		fpos++;
	}

	fpos_line_begin = fpos;
	laneScaleCnt = 0;

	uint8_t rgtmp[3];

	FILE *fOut;

	fOut = fopen(filenameOut, "wb");

  uint8_t bit_cnt = 0;
  uint8_t byte[8];
  
  uint16_t helper = 0;
	
	printf("C const:\nuint8_t array[] = {\n");
	
  if (compression == 0) {
    scale = 1;
	  while (1) {
		  fread(rgtmp, sizeof(rgtmp), 1, fPointer);
		  fpos += 3;

		  r = rgtmp[0];
		  g = rgtmp[1];
		  b = rgtmp[2];

		  if (xi == img_width - 1) {
		    laneScaleCnt++;
		    if (laneScaleCnt == scale) {
			    yi++;
			    xi = 0;
			    laneScaleCnt = 0;
			    fpos_line_begin = fpos;
		    } else {
		      xi = 0;
		      fpos = fpos_line_begin;
		    }
		  } else {
			  xi++;
		  }

      if (r == 0 && g == 0 && b == 0) {
        byte[bit_cnt] = 1;
      } else {
        byte[bit_cnt] = 0;
      }
      
      bit_cnt++;
      
      if (bit_cnt == 8) {
        uint8_t out = 0;
        
        out =  (byte[7] << 7) | (byte[6] << 6) | (byte[5] << 5) | (byte[4] << 4) | (byte[3] << 3) | (byte[2] << 2) | (byte[1] << 1) | byte[0];
        
        printf("0x%x,", out);
        fwrite(&out, sizeof(out), 1, fOut);
        helper++;
        bit_cnt = 0;
      }
      
      if (helper == 8) {
        printf("\n");
        helper = 0;
      }
      
      if (yi == img_height) {
			  break;
		  }
	  }
	}
	
	printf("\b};\nDone\n");

	fclose(fPointer);
  fclose(fOut);
	return 0;
}
