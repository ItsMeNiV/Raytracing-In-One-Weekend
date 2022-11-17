#include "Raytracer.h"

GPURaytracer::GPURaytracer(std::shared_ptr<std::vector<GLubyte>> imageTextureData, Camera& camera, HittableList& world, const Vec3& background, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth, const int originalScreenWidth, const int originalScreenHeight)
    : Raytracer(imageTextureData, camera, world, background, imageHeight, imageWidth, samplesPerPixel, maxDepth),
	originalScreenWidth(originalScreenWidth), originalScreenHeight(originalScreenHeight)
{
    raytraceShader = std::make_unique<Shader>("assets/shaders/maindisplay.vert", "assets/shaders/raytracer.frag");
}

void GPURaytracer::Run()
{
	float fullscreenQuadVerts[] = {
		-1.0f, -1.0f, 1.0f,
		 1.0f, -1.0f, 1.0f,
		 1.0f,  1.0f, 1.0f,
		-1.0f,  1.0f, 1.0f
	};

	uint32_t fullscreenQuadIndices[] = {
		0, 1, 2,
		0, 2, 3
	};

	uint32_t renderTexture, frameBuffer, renderBuffer;

	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mImageWidth, mImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenRenderbuffers(1, &renderBuffer);
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);
	{
		auto glstatus = glGetError();
		if (glstatus != GL_NO_ERROR)
			std::cout << "Error in GL call: " << glstatus << std::endl;
	}
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mImageWidth, mImageHeight);
	{
		auto glstatus = glGetError();
		if (glstatus != GL_NO_ERROR)
			std::cout << "Error in GL call: " << glstatus << std::endl;
	}
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
	{
		auto glstatus = glGetError();
		if (glstatus != GL_NO_ERROR)
			std::cout << "Error in GL call: " << glstatus << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	uint32_t vertexBuffer, indexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenQuadVerts), fullscreenQuadVerts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fullscreenQuadIndices), fullscreenQuadIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	raytraceShader->use();
	glViewport(0, 0, mImageWidth, mImageHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, mImageWidth, mImageHeight, GL_RGBA, GL_UNSIGNED_BYTE, mImageTextureData->data());

	glViewport(0, 0, originalScreenWidth, originalScreenHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &frameBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glDeleteRenderbuffers(1, &renderBuffer);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glBindTexture(GL_TEXTURE_2D, 0);
}
