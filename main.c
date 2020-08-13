// Drawing a fullscreen image on screen with glBegin/glEnd

#include <vitaGL.h>
#include <stdlib.h>
#include <stdio.h>

GLuint texture = 0;
GLuint buffers[2] = {0, 0};

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

void* loadTexture(uint16_t* width, uint16_t* height, uint32_t* imagesize)
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
	*imagesize = tex_size;
	void* data = (unsigned char *)malloc(tex_size);
	memcpy(data, textureData, tex_size);
	free(buf);
	return data;
}

float vertices[] = 
{
	0.0f, 0.0f, 0.0f,
	544.0f, 0.0f, 0.0f,
	544.0f, 544.0f, 0.0f,
	0.0f, 544.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f
};

uint16_t indices[] = 
{
	0, 1, 3,
	1, 2, 3
};

int main() {
	// Initializing graphics device
	vglInit(0x800000);

	glClearColor(0.50, 0, 0, 0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 960, 544, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// Generate buffers
	glGenBuffers(2, buffers);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 20, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

	// Loading texture from PVR file. 
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	uint16_t w, h;
	uint32_t size;
	void *buffer = loadTexture(&w, &h, &size);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG, w, h, 0, size, buffer);
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (;;){
		vglStartRendering();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D, texture);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, 0);
		glTexCoordPointer(2, GL_FLOAT, 0, 12 * sizeof(float));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		vglStopRendering();
		glLoadIdentity();
	}
	
	vglEnd();
	
}