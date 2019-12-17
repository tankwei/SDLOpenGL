#ifndef GSOBJMODEL_H
#define GSOBJMODEL_H

#include <vector> 
#include <map> 
#include "GS3dModelBase.h"

//http://www.zwqxin.com/archives/opengl/obj-model-format-import-and-render-2.html
//https://blog.csdn.net/ooffrankdura/article/details/97509417
//https://blog.csdn.net/xyh930929/article/details/82260581
//https://blog.csdn.net/xuexiaokkk/article/details/49533309
//https://blog.csdn.net/shenshen211/article/details/51740988  3D中的OBJ文件格式详解


#define MAX_LINE   255
#define VERTEX_OF_FACE     3

class GSOBJModel : public GS3dModelBase
{
    public:
        GSOBJModel();
        ~GSOBJModel();

        enum{
		    MAP_DIFFUSE,
		    MAP_SPECULAR,
		    MAP_BUMP
        };
	    typedef struct gsVertInfo{
		    gsVertInfo(const Vector3 &pos, const TexCoord &tc, const Vector3 &norm){
                vPos = pos; vTexcoord = tc; vNormal = norm;
            }
		    Vector3  vPos;
		    TexCoord vTexcoord;
		    Vector3  vNormal;
	    }gsVertInfo;
        
        // 材质信息结构体
        typedef struct gs_textureMaterialInfo{
            char      strName[MAX_NAME];// 纹理名称
            GLfloat   crAmbient[4];
            GLfloat   crDiffuse[4];
            GLfloat   crSpecular[4];
            GLfloat   fShiness;
            GLuint    nDiffuseMap;
            GLuint    nSpecularMap;
            GLuint    nBumpMap;
            GLuint    TexObjDiffuseMap;
            GLuint    TexObjSpecularMap;
            GLuint    TexObjBumpMap;
        }gs_textureMaterialInfo;
	
        //对象信息
        typedef struct gs_3DEntity{
            int                         nMaterialID;       // 纹理ID
            bool                        bHasTexture;       // 是否具有纹理映射
            bool                        bHasNormal;        // 是否具有法线
            std::vector<Vector3>        PosVerts;          // 对象的顶点
            std::vector<Vector3>        Normals;           // 对象的法向量
            std::vector<TexCoord>       Texcoords;         // 纹理UV坐标
            std::vector<unsigned short> Indexes;           // 对象的顶点索引
            unsigned int                nNumIndexes;       // 索引数目
            unsigned int                gs_ShaderProgram;
            GLuint                      gs_VAO;
            GLuint                      nPosVBO;
            GLuint                      nNormVBO;
            GLuint                      nTexcoordVBO;
            GLuint                      nIndexVBO;
        }gs_3DEntity;

        //模型信息结构体
        typedef struct gs3DModel{
            bool  bIsTextured;//是否使用纹理
            std::vector<gs_textureMaterialInfo> tMaterialInfoCon;   // 材质信息
            std::vector<gs_3DEntity> gs3DEntityCon;// 模型中对象信息
        }gs3DModel;

        virtual bool ImportModel(wchar_t *modelFileNameStr, GLuint usage);
    
    protected:
        virtual void DrawModel(glm::mat4 &view);
        
    private:
        void ProcessFileInfo();
        void ProcessVertexInfo(char chKeyword);
        void ProcessFaceInfo();
        int FindMtlID(char *szMtlName);
        void ComputeNormals(gs_3DEntity *t3DObj);
        FILE* m_FilePointer;
        gs3DModel m_3DModel;
        int  m_nCurObjectCount;
        int  m_nCurMaterialCount;
        std::vector<Vector3>   m_VPositionVec;
        std::vector<TexCoord>  m_VTexcoordVec;
        std::vector<Vector3>   m_VNormalVec;
        std::vector<Vector3>   m_VColorVec;
        std::map<gsVertInfo, unsigned short> m_VObjectIndexMap;
};


#endif