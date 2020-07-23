// Drawing a fullscreen image on screen with glBegin/glEnd

#include <vitaGL.h>
#include <vita2d.h>
#include <stdlib.h>
#include <stdio.h>

GLenum texture_format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
GLuint texture = 0;

typedef struct PVRTextureHeader {
	uint32_t version; // Should be 0x50565203
	uint32_t flags;
	union {
		struct
		{
			uint32_t value;
			uint32_t empty; // If this is not equal to 0, use the other struct
		};
		struct
		{
			uint32_t channels;
			uint32_t bitrates;
		};
	} pixelFormat;
	uint32_t colorSpace;
	uint32_t channelType;
	uint32_t height;
	uint32_t width;
	uint32_t depth;
	uint32_t surfaceCount;
	uint32_t faceCount;
	uint32_t mipmapCount;
	uint32_t metadataSize;
} PVRTextureHeader;

void* loadTexture(uint16_t* width, uint16_t* height)
{
	FILE *fd = fopen("app0:texture.pvr", "rb");
	fseek(fd, 0, SEEK_END);
	size_t size = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	void *buf = malloc(size);
	fread(buf, size, 1, fd);
	fclose(fd);

	PVRTextureHeader *header = (PVRTextureHeader*)buf;
	*height = header->height;
	*width = header->width;
	
	void *textureData = buf + (sizeof(PVRTextureHeader) + header->metadataSize);
	size_t tex_size = size - (sizeof(PVRTextureHeader) + header->metadataSize);
	void* data = (unsigned char *)malloc(tex_size);
	memcpy(data, textureData, tex_size);
	free(buf);
	return data;
}

int main(){
	
	// Initializing graphics device
	vglInit(0x800000);
	
	uint16_t w, h;
	void* buffer = loadTexture(&w, &h);

	glClearColor(0.50, 0, 0, 0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 960, 544, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Initializing openGL texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, texture_format, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (;;){
		vglStartRendering();
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_QUADS);
		
		// Note: BMP images are vertically flipped
		glTexCoord2i(0, 1);
		glVertex3f(0, 0, 0);
		glTexCoord2i(1, 1);
		glVertex3f(544, 0, 0);
		glTexCoord2i(1, 0);
		glVertex3f(544, 544, 0);
		glTexCoord2i(0, 0);
		glVertex3f(0, 544, 0);
		
		glEnd();
		vglStopRendering();
		glLoadIdentity();
	}
	
	vglEnd();
	
}