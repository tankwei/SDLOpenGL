#include "GS3dModelBase.h"

GS3dModelBase::GS3dModelBase() : m_ModelSize(Vector3(1.0)),m_ResourceDirectory(NULL){
    
}

GS3dModelBase::~GS3dModelBase(){

}

void GS3dModelBase::RenderModel(glm::mat4 &view)
{
    this->DrawModel(view);
}