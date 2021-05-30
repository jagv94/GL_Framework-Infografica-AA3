#version 330
	in vec3 in_Position;
	in vec3 in_Normal;
	in vec2 aTexCoord;
	out vec4 _vert_Normal;
	out vec4 _fragPos;
	out vec3 ourColor;
	out vec2 _TexCoord;
	uniform mat4 objMat;
	uniform mat4 mv_Mat;
	uniform mat4 mvpMat;
	//mat4 normalMat;
	void main() {
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);
		_fragPos = vec4(objMat * vec4(in_Position, 1.0)); //Posición del fragmento
		_vert_Normal = vec4(mat3(transpose(inverse(objMat))) * in_Normal, 0.0); //Vertice normal. Ajustada para adaptarse a objetos reescalados
		_TexCoord = aTexCoord;
	}