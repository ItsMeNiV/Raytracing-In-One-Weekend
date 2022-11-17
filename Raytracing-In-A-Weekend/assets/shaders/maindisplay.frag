#version 460 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D ImageTexture;

void main()
{
	FragColor = texture(ImageTexture, TexCoord);
}