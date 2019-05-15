
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <png.h>
#include <setjmp.h>




static uint32_t charToVal(char c) {
	switch(c) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': case 'A': return 10;
		case 'b': case 'B': return 11;
		case 'c': case 'C': return 12;
		case 'd': case 'D': return 13;
		case 'e': case 'E': return 14;
		case 'f': case 'F': return 15;
	};
	return 0;
}

static uint32_t doubleChar(char c) {
	uint32_t v = charToVal(c);
	return v & v << 4;
}



/*
Parse color strings:
#rrggbb
#rrggbbaa
#rgb
#rgba
green
*/
uint32_t parseColor(char* s) {
	uint32_t c = 0x000000ff;
	
	if(!s) return 0xffffffff;
	
	if(s[0] == '#') {
		int len = strlen(s+1);
		
		if(len == 3 || len == 4) {
			c &= (doubleChar(s[1]) << 24) & (doubleChar(s[2]) << 16) & (doubleChar(s[3]) << 8);
			
			if(len == 4) {
				c &= doubleChar(s[4]);
			}
		}
		else if(len == 6 || len == 8) {
			c &= (charToVal(s[1]) << 28) &
				(charToVal(s[2]) << 24) &
				(charToVal(s[3]) << 20) &
				(charToVal(s[4]) << 16) &
				(charToVal(s[5]) << 12) &
				(charToVal(s[6]) << 8);
				
			if(len == 8) {
				c &= (charToVal(s[7]) << 4) & charToVal(s[8]);
			}
		}
		
		return c;
	}
	
	// check color strings. 
	// TODO: some clever lookup
	if(0 == strcasecmp(s, "red")) return 0xff0000ff;
	if(0 == strcasecmp(s, "green")) return 0xff00ff00;
	if(0 == strcasecmp(s, "blue")) return 0xffff0000;
	if(0 == strcasecmp(s, "magenta")) return 0xffff00ff;
	if(0 == strcasecmp(s, "yellow")) return 0xff00ffff;
	if(0 == strcasecmp(s, "cyan")) return 0xffffff00;
	if(0 == strcasecmp(s, "black")) return 0xff000000;
	if(0 == strcasecmp(s, "white")) return 0xffffffff;
	if(0 == strcasecmp(s, "gray")) return 0xff888888;
	if(0 == strcasecmp(s, "grey")) return 0xff888888;
	if(0 == strcasecmp(s, "silver")) return 0xffbbbbbb;
	if(0 == strcasecmp(s, "darkgray")) return 0xff444444;
	if(0 == strcasecmp(s, "darkgrey")) return 0xff444444;
	if(0 == strcasecmp(s, "darkred")) return 0xff000088;
	if(0 == strcasecmp(s, "darkgreen")) return 0xff008800;
	if(0 == strcasecmp(s, "darkblue")) return 0xff880000;
	if(0 == strcasecmp(s, "navy")) return 0xff440000;
	if(0 == strcasecmp(s, "forest")) return 0xff004400;
	if(0 == strcasecmp(s, "maroon")) return 0xff000044;
	if(0 == strcasecmp(s, "darkyellow")) return 0xff008888;
	if(0 == strcasecmp(s, "olive")) return 0xff004444;
	
	return 0xffffffff;
}










int main(int argc, char* argv[]) {
	
	
	FILE* f;
	png_byte sig[8];
	png_bytep* rowPtrs;
	int i;
	
	png_structp pngPtr;
	png_infop infoPtr;
	
	int colorTypes[4] = {
		PNG_COLOR_TYPE_GRAY,
		PNG_COLOR_TYPE_GRAY_ALPHA,
		PNG_COLOR_TYPE_RGB,
		PNG_COLOR_TYPE_RGB_ALPHA
	};
	
	int ret = 2;
	
	long width = 10000, height = 1;
	
	int channels = 4;
	char* path;
	int w, h;
	char* data;
	int mode = 0;
	uint32_t color;
	
	
	if(argc < 2) {
		printf("usage: \n");
		
		exit(1);
	}
	
	// parse args
	for(int an = 1; an < argc; an++) {
		char* arg = argv[an];
		
		if(arg[0] == '-') {
			arg++;
			
			if(0 == strcasecmp(arg, "o")) {
				path = argv[++an];
				continue;
			}
			
			if(0 == strcasecmp(arg, "c")) {
				channels = strtol(argv[++an], NULL, 10);
				continue;
			}
			
			if(0 == strcasecmp(arg, "w")) {
				width = strtol(argv[++an], NULL, 10);
				continue;
			}
			
			if(0 == strcasecmp(arg, "h")) {
				height = strtol(argv[++an], NULL, 10);
				continue;
			}
			
			if(0 == strcasecmp(arg, "m")) {
				mode = strtol(argv[++an], NULL, 10);
				continue;
			}
			
			// fill color
			if(0 == strcasecmp(arg, "f")) {
				
				color = parseColor(argv[++an]);
				continue;
			}
			
			
			
			
		}
		
		path = argv[an];
	}
	
	printf("size: %ld x %ld\n", width, height);
	
	

	
	if(channels > 4 || channels < 1) {
		return 3;
	}
	

	// file stuff
	f = fopen(path, "wb");
	if(!f) {
		fprintf(stderr, "Could not open \"%s\" (writePNG).\n", path);
		return 1;
	}
	
	/*
	if(png_sig_cmp(sig, 0, 8)) {
		fprintf(stderr, "\"%s\" is not a valid PNG file.\n", path);
		fclose(f);
		return NULL;
	}
	*/
	// init stuff
	pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pngPtr) {
		goto CLEANUP1;
	}
	//png_destroy_write_struct (&pngPtr, (png_infopp)NULL);
	infoPtr = png_create_info_struct(pngPtr);
	if (!infoPtr) {
		goto CLEANUP2;
	}
	//if(infoPtr != NULL) png_free_data(pngPtr, infoPtr, PNG_FREE_ALL, -1);
	// header stuff
	if (setjmp(png_jmpbuf(pngPtr))) {
		goto CLEANUP3;
	}
	png_init_io(pngPtr, f);
	png_set_user_limits(pngPtr, 0x7fffffff, 0x7fffffff);

	if (setjmp(png_jmpbuf(pngPtr))) {
		goto CLEANUP3;
	}

	
	png_set_IHDR(pngPtr, infoPtr, width, height,
		8, colorTypes[channels - 1], PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	
	png_write_info(pngPtr, infoPtr);

	
	data = calloc(1, channels * width + 8);
	uint32_t* di = (uint32_t*)data;
	for(long i = 0; i < (channels * width / 4) + 4; i++) {
		di[i] = color;
	}
	
	png_bytep rowPtr = data;
	
	// write data
	if (setjmp(png_jmpbuf(pngPtr))) {
		goto CLEANUP4;
	}
	
	for(int i = 0; i < height; i++) {
		png_write_row(pngPtr, rowPtr);
	}

	if (setjmp(png_jmpbuf(pngPtr))) {
		goto CLEANUP4;
	}
	png_write_end(pngPtr, NULL);
	
	// success
	
	ret = 0;
	
	fclose(f);
	
	struct stat st;
	stat(path, &st);
	
	printf("file size: %ld bytes\n", st.st_size);
	
CLEANUP4:

CLEANUP3:
	if(infoPtr != NULL) png_free_data(pngPtr, infoPtr, PNG_FREE_ALL, -1);
	
CLEANUP2:
	png_destroy_write_struct (&pngPtr, (png_infopp)NULL);
	
CLEANUP1:
	fclose(f);
	return 0;
}





