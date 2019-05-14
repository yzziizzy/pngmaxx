
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <png.h>
#include <setjmp.h>





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

	
	data = calloc(1, channels * width);
	
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





