// A place for the bascis of what make a mesh

#ifndef MESH_BUFFER_H_
#define MESH_BUFFER_H_

#include <vector>
#include "glm/glm.hpp"

class MeshBuffer
{
public:
    MeshBuffer();
    virtual ~MeshBuffer();

    MeshBuffer(const MeshBuffer & ref);
    MeshBuffer& operator=(const MeshBuffer & ref);

    void loadFile(const char * fileName);

    void setVerts(unsigned int count, const float* verts);
    void setNorms(unsigned int count, const float* normals);
    void setTexCoords(unsigned int layer, unsigned int count, const float* coords);
    void setIndices(unsigned int count, const unsigned int * indices);

    const std::vector<glm::vec3>& getVerts();
    const std::vector<glm::vec3>& getNorms();
    const std::vector<glm::vec2>& getTexCoords(unsigned int layer);
    const unsigned int * getIndices() const;

    unsigned int getVertCnt() const;
    unsigned int getIdxCnt() const;

    void generateFaceNormals();

    bool UsesNormals;
    bool UsesUVs;
    bool UsesIndices;

private:

    void cleanUp();

    unsigned int VertCnt;
    unsigned int IdxCnt;

    std::vector<glm::vec3> Verts;
    std::vector<glm::vec3> Norms;
    std::vector<glm::vec2> TexCoords;
    unsigned int * Indices;
};

#endif // MESH_BUFFER_H_
