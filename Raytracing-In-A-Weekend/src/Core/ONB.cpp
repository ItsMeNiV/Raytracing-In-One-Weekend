#include "Core/ONB.h"

void ONB::buildFromW(const glm::vec3& n)
{
	axis[2] = glm::normalize(n);
	glm::vec3 a = (fabs(w().x) > 0.9) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
	axis[1] = glm::normalize(glm::cross(w(), a));
	axis[0] = glm::cross(w(), v());
}
