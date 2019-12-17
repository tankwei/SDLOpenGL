#ifndef MAN_H
#define MAN_H

#include <util/camera.h>
#include <util/shader.h>

class Gun;

class Man
{
public:
	Man();
	virtual ~Man() {}
	void Init();
	void Update(GLfloat deltatime, GLboolean *keys);
	void Render(Camera &camera, glm::mat4 &projection, glm::mat4 &view, Gun &gun);
	void SetShoot(bool ifshoot);
	void SetState(bool ifmove);
	void DeduceLife(float life);
};

#endif
