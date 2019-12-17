#ifndef GS3DMODELBASE_H
#define GS3DMODELBASE_H


#include "glad/glad.h"
#include "game/Texture.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MAX_NAME     255

#ifndef MAX_PATH
#define MAX_PATH 260
#endif


class GS3dModelBase
{
    public:
    GS3dModelBase();
    virtual ~GS3dModelBase() = 0;
    
    typedef glm::vec2 TexCoord;
    typedef glm::vec3 Vector3;
    virtual bool ImportModel(wchar_t *strFileName, GLuint usage = GL_STREAM_DRAW) = 0;
    void RenderModel(glm::mat4 &view);
    protected:
        virtual void DrawModel(glm::mat4 &view) = 0;
        wchar_t *m_ResourceDirectory;
	    Vector3 m_ModelSize;
};


#endif
