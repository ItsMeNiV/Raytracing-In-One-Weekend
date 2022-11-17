#version 460 core
layout (location = 0) in vec3 aPos;

out vec2 TexCoord;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	TexCoord = 0.5 * gl_Position.xy + vec2(0.5);
}