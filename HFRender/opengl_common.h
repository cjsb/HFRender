#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>

const char* GLenumToString(GLenum value);

#define GL_CHECK_ERROR {\
	GLenum e = glGetError();\
	if (e != 0 ) {\
		printf("GL Error 0x%04x in %s at line %i in %s, %s", \
				e, __FUNCSIG__, __LINE__, __FILE__, GLenumToString(e));\
		assert(0 && "GL Error encountered!");\
	}\
}

