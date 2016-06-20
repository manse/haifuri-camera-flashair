#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <glob.h>
#include <dirent.h>

const int IMAGE_LINE = 683;
const int IMAGE_WIDTH = 384;
const int BYTES_PER_LINE = 48;
const int CHANNEL = 3;
const unsigned int BAUDRATE = 38400;
const char IMAGE_DIRECTORY[] = "./upload/";

int main(int argc, char *argv[]) {
	while (1) {
		DIR *dir = opendir(IMAGE_DIRECTORY);
		struct dirent *dp;
		char* path = NULL;
		while ((dp = readdir(dir)) != NULL) {
			if (strstr(dp->d_name, ".bmp")) {
				char result[256];
				strcpy(result, IMAGE_DIRECTORY);
				strcat(result, dp->d_name);
				path = result;
			}
		}
		closedir(dir);

		if (path) {
			int i, j, k, fd;
			if ((fd = serialOpen("/dev/ttyAMA0", BAUDRATE)) < 0) {
				printf("Can not open serial\n");
				return 0;
			}

			int offset = 0;
			char oBuf[4], bBuf[IMAGE_LINE * IMAGE_WIDTH * CHANNEL];
			FILE *fp = fopen(path, "rb");
			fseek(fp, 10, SEEK_SET);
			fread(oBuf, sizeof(char), 4, fp);
			for (i = 3; i >= 0; i--) {
				offset = (offset << 8) | oBuf[i];
			}
			fseek(fp, offset, SEEK_SET);
			fread(bBuf, sizeof(char), IMAGE_LINE * IMAGE_WIDTH * CHANNEL, fp);
			fclose(fp);
			remove(path);

			char *bBufPtr = bBuf - 1;
			for (i = 0; i < IMAGE_LINE; i ++) {
				serialPutchar(fd, 0x1c);
				serialPutchar(fd, 0x2a);
				serialPutchar(fd, 0x63);
				serialPutchar(fd, 0x00);
				serialPutchar(fd, 0x01);
				bBufPtr += IMAGE_WIDTH * CHANNEL;
				for (j = 0; j < BYTES_PER_LINE; j++) {
					char buf = 0;
					for (k = 7; k >= 0; k--, bBufPtr -= CHANNEL) {
						buf = buf | (*bBufPtr < 128 ? 1 << k : 0);
					}
					serialPutchar(fd, buf);
				}
				bBufPtr += IMAGE_WIDTH * CHANNEL;
			}
			serialPrintf(fd, "\r\r");
		}
		sleep(1);
	}
}

