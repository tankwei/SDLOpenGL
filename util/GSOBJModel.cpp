#include "util/wtypes.h"
extern "C"
{
	#include "FileUtil.h"
}
#include "GSOBJModel.h"
#include <util/shader.h>

const char* modelVertexShaderSource = "#version 320 es\n"
"layout (location = 0) in mediump vec3 aPos;\n"
"layout (location = 1) in mediump vec3 aNormal;\n"
"layout (location = 2) in mediump vec2 aTexCoords;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out mediump vec2 TexCoord;\n"
"void main(){\n"
"gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
"TexCoord=aTexCoords;\n"
"}";
const char* modelFragmentShaderSource = "#version 320 es\n"
"out mediump vec4 FragColor;\n"
"in mediump vec2 TexCoord;\n"
"void main(){\n"
"FragColor = vec4(0.3f, 0.6f, 0.2f, 1.0f);\n"
"}";

GSOBJModel::GSOBJModel() : m_FilePointer(NULL), m_nCurObjectCount(0), m_nCurMaterialCount(0){

}

GSOBJModel::~GSOBJModel(){

}

bool operator<(const GSOBJModel::gsVertInfo &vertInfo1, const GSOBJModel::gsVertInfo &vertInfo2)
{
	if(vertInfo1.vPos.x != vertInfo2.vPos.x){
		return vertInfo1.vPos.x - vertInfo2.vPos.x < 0;
	}
	else if(vertInfo1.vPos.y != vertInfo2.vPos.y){
		return vertInfo1.vPos.y < vertInfo2.vPos.y;
	}
	else if(vertInfo1.vPos.z != vertInfo2.vPos.z){
		return vertInfo1.vPos.z < vertInfo2.vPos.z;
	}
	else if(vertInfo1.vTexcoord.x != vertInfo2.vTexcoord.x){//u
		return vertInfo1.vTexcoord.x - vertInfo2.vTexcoord.x < 0;
	}
	else if(vertInfo1.vTexcoord.y != vertInfo2.vTexcoord.y){//v
		return vertInfo1.vTexcoord.y - vertInfo2.vTexcoord.y < 0;
	}
	if(vertInfo1.vNormal.x != vertInfo2.vNormal.x){
		return vertInfo1.vNormal.x < vertInfo2.vNormal.x;
	}
	else if(vertInfo1.vNormal.y != vertInfo2.vNormal.y){
		return vertInfo1.vNormal.y - vertInfo2.vNormal.y < 0;
	}
	else{
		return vertInfo1.vNormal.z - vertInfo2.vNormal.z < 0;
	}
}

bool GSOBJModel::ImportModel(wchar_t *modelFileNameStr, GLuint usage){
    wchar_t filePathStr[MAX_PATH] = {0};
    if(this->m_ResourceDirectory){
        wcsncat(filePathStr,this->m_ResourceDirectory,sizeof(filePathStr) / sizeof(wchar_t));
    }
    wcsncat(filePathStr,modelFileNameStr,sizeof(filePathStr) / sizeof(wchar_t));
    this->m_FilePointer = wfopen(filePathStr,L"rb");
    if(!this->m_FilePointer){
        return false;
    }
    this->m_nCurObjectCount = 0;
    this->m_nCurMaterialCount = 0;
    this->ProcessFileInfo();
    fclose(this->m_FilePointer);
    this->m_FilePointer = NULL;
    this->m_VPositionVec.clear();
	this->m_VNormalVec.clear();
	this->m_VTexcoordVec.clear();
	this->m_VObjectIndexMap.clear();

    //绑定
    Shader shader;
	for(unsigned int i = 0; i < this->m_3DModel.gs3DEntityCon.size(); i++) {
        this->m_3DModel.gs3DEntityCon[i].gs_ShaderProgram = shader.LoadShaders(modelVertexShaderSource, modelFragmentShaderSource);
		if(!this->m_3DModel.gs3DEntityCon[i].PosVerts.empty()){
            glGenVertexArrays(1, &(this->m_3DModel.gs3DEntityCon[i].gs_VAO));
	        glBindVertexArray(this->m_3DModel.gs3DEntityCon[i].gs_VAO);
			glGenBuffers(1, &this->m_3DModel.gs3DEntityCon[i].nPosVBO);
			glBindBuffer(GL_ARRAY_BUFFER, this->m_3DModel.gs3DEntityCon[i].nPosVBO);
			glBufferData(GL_ARRAY_BUFFER, this->m_3DModel.gs3DEntityCon[i].PosVerts.size() * sizeof(Vector3), 
				(GLvoid*)&this->m_3DModel.gs3DEntityCon[i].PosVerts[0], usage);
		}
		if(!this->m_3DModel.gs3DEntityCon[i].bHasNormal){
			// 计算顶点的法向量
			ComputeNormals(&this->m_3DModel.gs3DEntityCon[i]);
			this->m_3DModel.gs3DEntityCon[i].bHasNormal = true;
		}
		if(!this->m_3DModel.gs3DEntityCon[i].Normals.empty()){
			glGenBuffers(1, &this->m_3DModel.gs3DEntityCon[i].nNormVBO);
			glBindBuffer(GL_ARRAY_BUFFER, this->m_3DModel.gs3DEntityCon[i].nNormVBO);
			glBufferData(GL_ARRAY_BUFFER, this->m_3DModel.gs3DEntityCon[i].Normals.size() * sizeof(Vector3), 
				(GLvoid*)&this->m_3DModel.gs3DEntityCon[i].Normals[0], usage);
		}
		if(this->m_3DModel.gs3DEntityCon[i].bHasTexture && !this->m_3DModel.gs3DEntityCon[i].Texcoords.empty()){
			glGenBuffers(1, &this->m_3DModel.gs3DEntityCon[i].nTexcoordVBO);
			glBindBuffer(GL_ARRAY_BUFFER, this->m_3DModel.gs3DEntityCon[i].nTexcoordVBO);
			glBufferData(GL_ARRAY_BUFFER, this->m_3DModel.gs3DEntityCon[i].Texcoords.size() * sizeof(TexCoord), 
				(GLvoid*)&this->m_3DModel.gs3DEntityCon[i].Texcoords[0], usage);
		}
		if(!this->m_3DModel.gs3DEntityCon[i].Indexes.empty()){
			glGenBuffers(1, &this->m_3DModel.gs3DEntityCon[i].nIndexVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_3DModel.gs3DEntityCon[i].nIndexVBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->m_3DModel.gs3DEntityCon[i].Indexes.size() * sizeof(unsigned short), 
				(GLvoid*)&this->m_3DModel.gs3DEntityCon[i].Indexes[0], usage);
			this->m_3DModel.gs3DEntityCon[i].nNumIndexes = this->m_3DModel.gs3DEntityCon[i].Indexes.size();
		}
	}
    return true;
}

void GSOBJModel::DrawModel(glm::mat4 &view)
{
    glm::mat4 anim = glm::mat4(1.0f);
    anim = glm::translate(anim, glm::vec3(0.0f, -100.0f, 0.0f)); // translate it down so it's at the center of the scene
    anim = glm::scale(anim, glm::vec3(100.0f, 100.0f, 100.0f));	// it's a bit too big for our scene, so scale it down
    glm::mat4 projection = glm::perspective(glm::radians(6.0f), 1.0f * 1280 / 720, 0.1f, 1000.0f);
    //glm::mat4 mvp = projection * view * anim;
	for(unsigned int i = 0; i < this->m_3DModel.gs3DEntityCon.size(); i++) {
		gs_3DEntity *t3DObj = &(this->m_3DModel.gs3DEntityCon[i]);
        glUseProgram(t3DObj->gs_ShaderProgram);
		GLuint transformLoc = glGetUniformLocation(t3DObj->gs_ShaderProgram, "projection");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection));
        transformLoc = glGetUniformLocation(t3DObj->gs_ShaderProgram, "view");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(view));
        transformLoc = glGetUniformLocation(t3DObj->gs_ShaderProgram, "model");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(anim));
		glBindVertexArray(t3DObj->gs_VAO);
        glDrawElements(GL_TRIANGLES, t3DObj->nNumIndexes, GL_UNSIGNED_SHORT, NULL);
    }
}

void GSOBJModel::ProcessFileInfo(){
    char strBuff[MAX_LINE] = {0};
    char chKeyWord = 0;
    while(EOF != fscanf(this->m_FilePointer,"%s",strBuff,MAX_LINE)){
        chKeyWord = strBuff[0];
        switch (chKeyWord){
            case 'm':
            {
                if(0 == strcmp(strBuff,"mtllib")){
                    this->m_3DModel.bIsTextured = true;
                }
                fgets(strBuff, MAX_LINE, m_FilePointer);
                break;
            }
            case 'u':
            {
                if(0 == strcmp(strBuff,"usemtl")){
                    gs_3DEntity gs_3dObject = {0};
                    gs_3dObject.bHasTexture = false;
                    gs_3dObject.bHasNormal = false;
                    fscanf(this->m_FilePointer,"%s",strBuff,MAX_LINE);
                    gs_3dObject.nMaterialID = this->FindMtlID(strBuff);
                    this->m_3DModel.gs3DEntityCon.push_back(gs_3dObject);
                    ++this->m_nCurObjectCount;
                    this->m_VObjectIndexMap.clear();
                }
                fgets(strBuff, MAX_LINE, this->m_FilePointer);
                break;
            }
            case 'v':
            {
                this->ProcessVertexInfo(strBuff[1]);
            }
            break;
            case 'f':
            {
                if(0 == this->m_nCurObjectCount){
                    gs_3DEntity gs_3dObject = {0};
                    this->m_3DModel.gs3DEntityCon.push_back(gs_3dObject);
                }
                this->ProcessFaceInfo();
            }
            break;
        default:
            fgets(strBuff, MAX_LINE, this->m_FilePointer);
            break;
        }
    }
}

void GSOBJModel::ProcessVertexInfo(char chKeyword){
	char strBuff[MAX_LINE]	= {0};
	if('\0' == chKeyword){// 如果是\0，则必是顶点("v")
		Vector3  v_Vertex;
		// 读入顶点坐标，格式是"v x y z"
		fscanf(m_FilePointer, "%f %f %f", &v_Vertex.x, &v_Vertex.y, &v_Vertex.z);
		// 添加一个新的顶点到顶点列表中
		m_VPositionVec.push_back(v_Vertex);
        fgets(strBuff, MAX_LINE, m_FilePointer);
        float r = 0,g = 0,b = 0;
        short count = sscanf(strBuff, "%f %f %f", &r, &g, &b);
        if(3 == count){
            Vector3 v_Color;
            m_VColorVec.push_back(v_Color);
        }
        return;
	}
	else if('t' == chKeyword){// 如果是't'，则必定是纹理坐标("vt")
		TexCoord vNewTexCoord;
		// 读入纹理坐标，格式是"vt u v"
		fscanf(m_FilePointer, "%f %f", &vNewTexCoord.x, &vNewTexCoord.y);
        //http://www.cplusplus.com/reference/cstdio/fscanf/
        //https://blog.csdn.net/frankeyshuo/article/details/7604782
		// 添加一个新的纹理坐标到列表中
		m_VTexcoordVec.push_back(vNewTexCoord);
	}
	else if('n' == chKeyword){	// 否则可能是法向量("vn")
		Vector3  vNewNormal;
		// 读入法线，格式是"vn x y z"
		fscanf(m_FilePointer, "%f %f %f", &vNewNormal.x, &vNewNormal.y, &vNewNormal.z);
		// 添加一个新的纹理坐标到列表中
		m_VNormalVec.push_back(vNewNormal);
	}
    fgets(strBuff, MAX_LINE, m_FilePointer);// 读入该行余下的内容，则文件指针指向下一行
}

void GSOBJModel::ProcessFaceInfo(){
    gs_3DEntity &pCurObj = this->m_3DModel.gs3DEntityCon[this->m_nCurObjectCount - 1];
    char strBuff[MAX_LINE] = {0};
    unsigned int vIdx = 0,tIdx = 0,nIdx = 0;
    Vector3 vPosVert;
    TexCoord vTextcoord;
    Vector3 vNormal(0,1,0);
    size_t nDistance = 0;
    fscanf(this->m_FilePointer,"%s",strBuff,MAX_LINE);
    if(3 == sscanf(strBuff,"%d/%d/%d",&vIdx,&tIdx,&nIdx)){
        if(!pCurObj.bHasTexture){
            pCurObj.bHasTexture = true;
        }
        if(!pCurObj.bHasNormal){
            pCurObj.bHasNormal = true;
        }
    }
    std::map<gsVertInfo,unsigned short>::iterator vObjectPos;
    for(;;){
        gsVertInfo vertInfo(this->m_VPositionVec[vIdx - 1],
        this->m_VTexcoordVec[tIdx - 1],
        this->m_VNormalVec[nIdx - 1]);
        vObjectPos = this->m_VObjectIndexMap.find(vertInfo);
        if(vObjectPos != this->m_VObjectIndexMap.end()){
            pCurObj.Indexes.push_back(vObjectPos->second);
        }
        else{
            pCurObj.PosVerts.push_back(this->m_VPositionVec[vIdx - 1]);
            pCurObj.Texcoords.push_back(this->m_VTexcoordVec[tIdx - 1]);
            pCurObj.Indexes.push_back(pCurObj.PosVerts.size() - 1);
            this->m_VObjectIndexMap.insert(std::pair<gsVertInfo,unsigned>(vertInfo,pCurObj.PosVerts.size() - 1));
        }
        if(3 != fscanf(this->m_FilePointer, "%d/%d/%d", &vIdx, &tIdx, &nIdx)){
            break;
        }
    }
}

int GSOBJModel::FindMtlID(char *szMtlName)
{
	for(unsigned int j = 0; j < this->m_3DModel.tMaterialInfoCon.size(); ++j){
		if(0 == strcmp(this->m_3DModel.tMaterialInfoCon[j].strName, szMtlName)){
			return j;
		}
	}
	return -1;
}

// 下面的函数用于计算对象的顶点法向量, 顶点的法向量主要用来计算光照
void GSOBJModel::ComputeNormals(gs_3DEntity *t3DObj)
{
	Vector3 vVector1, vVector2, vPoly[3];

	// 分配需要的存储空间
	Vector3 *pTempNormals  = new Vector3 [t3DObj->Indexes.size() / VERTEX_OF_FACE];
	memset(pTempNormals,     0, t3DObj->Indexes.size() * sizeof(Vector3) / VERTEX_OF_FACE);

	// 遍历对象的所有面
	for(unsigned int j = 0; j < t3DObj->Indexes.size(); j += VERTEX_OF_FACE){                        
		vPoly[0] = t3DObj->PosVerts[t3DObj->Indexes[j]];
		vPoly[1] = t3DObj->PosVerts[t3DObj->Indexes[j + 1]];
		vPoly[2] = t3DObj->PosVerts[t3DObj->Indexes[j + 2]];

		// 计算面的法向量

		vVector1 = (vPoly[0] - vPoly[2]);    // 获得多边形的矢量
		vVector2 = (vPoly[2] - vPoly[1]);    // 获得多边形的第二个矢量
		pTempNormals[j / VERTEX_OF_FACE] = glm::cross(vVector1,vVector2);
		//vVector1.CrossProd(vVector2);  // 保存面的非规范化法向量
	}
	// 下面求顶点法向量
	Vector3 vSum(0);
	int shared=0;
	// 遍历所有的顶点
	for (unsigned int i = 0; i < t3DObj->PosVerts.size(); ++i)     
	{
		shared = 0;
		vSum = glm::vec3(0,0,0);//set(0, 0, 0)
		for (unsigned int j = 0; j < t3DObj->Indexes.size(); j += VERTEX_OF_FACE){// 遍历所有的三角形面                                                // 判断该点是否与其它的面共享
			if (t3DObj->Indexes[j    ] == i || 
				t3DObj->Indexes[j + 1] == i || 
				t3DObj->Indexes[j + 2] == i){
				vSum = (vSum + pTempNormals[j / VERTEX_OF_FACE]); //指示索引j所在的面法向量
				shared++;                
			}
		} 
		if(shared){
			t3DObj->Normals[i] = (vSum / GLfloat(-shared));
		}
		// 规范化最后的顶点法向
		//(t3DObj->Normals[i]).normalize();
		t3DObj->Normals[i] = glm::normalize(t3DObj->Normals[i]);           
	}
	delete [] pTempNormals;
}