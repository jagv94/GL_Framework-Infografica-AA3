#include "CubeMap.h"

//Acceso externo a loadOBJ, que nos ayudará a cargar nuestro modelo 3D posteriormente.
extern bool loadOBJ(const char* path, std::vector < glm::vec3 >& out_vertices,
    std::vector < glm::vec2 >& out_uvs, std::vector < glm::vec3 >& out_normals);

extern void linkProgram(GLuint program);
extern glm::vec3 randomize(float _min, float _max);

unsigned int CubeMap::Start(std::vector<std::string> faces)
{
    unsigned int _textureID;
    //Generate texture
    glGenTextures(1, &_textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _textureID);

    //Bind texture each face
    int width, height, nrChannels;
    unsigned char* data;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    //Texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return _textureID;

}

CubeMap::CubeMap(const char* _modelPath, unsigned int _texture, Shader _shader) : modelPath(_modelPath), texture(_texture), ourShader(_shader) {
    //Cargamos el objeto
    bool res = loadOBJ(modelPath, vertices, uvs, normals);

    //Seteamos vertices, normales y UVs
    glGenVertexArrays(1, &objectVao); //Obtenemos ide del objeto
    glBindVertexArray(objectVao); //Bindeamos el objeto (empezamos a acceder a la información del objeto)
    glGenBuffers(3, objectVbo); //Cargamos los datos en GPU

    //Parametrizamos el modelo
    glBindBuffer(GL_ARRAY_BUFFER, objectVbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, objectVbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, objectVbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvs.size(), &uvs[0], GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    //Desbindeamos el objeto de memoria
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindAttribLocation(ourShader.ID, 0, "in_Position");
    
    linkProgram(ourShader.ID);
}

void CubeMap::Draw(glm::mat4 cameraView, glm::mat4 projection)
{
    glDepthMask(GL_FALSE);
    ourShader.use();
    // ... set view and projection matrix
    glBindVertexArray(objectVao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
    // ... draw rest of the scene

    //Activamos el shader antes de configurar los uniforms
    glm::mat4 objMat = glm::mat4(1.f);
    glm::mat4 view = glm::mat4(glm::mat3(cameraView));
    glm::mat4 s = glm::scale(glm::mat4(), glm::vec3(2000, 2000, 2000));
    objMat = view * s;

    ourShader.setMat4("view", objMat);
    ourShader.setMat4("projection", projection);

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glUseProgram(0);
    glBindVertexArray(0);
}
