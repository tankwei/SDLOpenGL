#ifndef RENTITY_H
#define RENTITY_H

#include "Transform.h"
#include "md2Model.h"
#include "Texture.h"
#include "Entity.h"
#include "BoundingBox.h"

enum AnimationState {
	IDLE,
	RUN,
	ATTACK,
	DEATH1
};

class AnimatedEntity : public Entity
{

public:
	AnimatedEntity();
	~AnimatedEntity();

	float& getInterpol() { return interpol; }

	virtual void setAnimationState(AnimationState state) { m_AnimState = state; }
	virtual AnimationState getState() { return m_AnimState; }
	virtual void animate(int start, int end, float percent);

	void render();

	int getCurrentFrame() { return currentFrame; }

protected:
	// for animated models
	int currentFrame;
	int nextFrame;
	float interpol;
	bool update;

	// this models own VAO, VBOs
	GLuint VAO;
	GLuint currentVBO;
	GLuint nextVBO;
	GLuint currentNormalsVBO;
	GLuint nextNormalsVBO;
	GLuint textureVBO;

	// check what animation is active
	int m_Start;
	int m_End;

	AnimationState m_AnimState;
};

#endif

