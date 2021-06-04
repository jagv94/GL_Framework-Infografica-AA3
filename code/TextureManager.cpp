#include "TextureManager.h"

TextureManager::TextureManager() {}
TextureManager::TextureManager(const char* _texturePath, bool _fliped)
{
	glGenTextures(1, &img); //Cargamos en gráfica y accedemos a la variable que contendrá la textura
	glBindTexture(GL_TEXTURE_2D, img); //Bindeamos la textura (empezamos a acceder a la información de la textura)

	stbi_set_flip_vertically_on_load(_fliped); //Giramos la imagen si la vemos al reves

	if (Textures::imgLocation.find(_texturePath) == Textures::imgLocation.end()) {
		data = stbi_load(_texturePath, &x, &y, &n, 4); //Cargamos la textura
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			Textures::imgLocation[_texturePath] = data;
		}
		else printf("Failed to load texture\n");
	}
	else data = Textures::imgLocation.at(_texturePath);

	//Filtros de la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Liberamos memoria de la imagen de la textura aquí y en el cleanup
	stbi_image_free(data);
}

const char* TextureManager::GetTexturePath()
{
	return texturePath;
}

unsigned int TextureManager::GetImg()
{
	return img;
}
