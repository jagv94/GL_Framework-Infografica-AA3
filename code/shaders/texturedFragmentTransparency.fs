#version 330
	out vec4 FragColor;
	in vec4 vert_Normal;
	in vec4 fragPos;
	in vec2 TexCoord;
	out vec4 out_Color;
	uniform mat4 mv_Mat;
	uniform vec4 objectColor; //Color del objeto
	uniform vec4 directional_light; //Direccion de la luz direccional
	vec4 lightDirection; //Direccion de la luz (dependiendo de si se usa para la direccional o la point)
	uniform vec4 pointLight_pos; //Posicion de la PointLight
	uniform vec4 ambientColor; //Color de la luz ambiente
	uniform vec4 ambientIntensity; //Intensidad de la luz ambiente
	uniform vec4 difuseColor; //Color de la luz difusa
	uniform vec4 difuseIntensity; //Intensidad de la luz difusa
	uniform vec4 specularColor; //Color de la luz especular
	uniform vec4 specularIntensity; //Intensidad de la luz especular
	uniform float specularDensity; //Densidad especular
	uniform int lightSelection; //Selector del tipo de iluminacion (direccional o point)
	uniform sampler2D ourTexture;
	vec4 posCamera; //Posición de la camara
	vec4 resultOfAll;
	vec3 result; //Resultado de las operaciones
	void main() {
		switch(lightSelection){ //Switch para la selección de iluminación
			case 0:
				lightDirection = directional_light; //Asignamos la dirección de la luz como de directional light
				break;
			case 1:
				lightDirection = normalize(pointLight_pos - fragPos); //Asignamos la dirección de la luz como la de la point light
				break;
			default:
				lightDirection = directional_light; //Asignamos la dirección de la luz como de directional light
				break;
		}

		posCamera = inverse(mv_Mat)[3]; //Obtenemos la posición de la cámara de la inversa de matrix view
		//luz ambiente (color ambiente * intensidad ambiente)
		vec4 ambient = (ambientColor * ambientIntensity);

		//luz difusa (color luz difusa * intensidas luz difusa * dot de (normal objeto y direccion de la luz)
		vec4 diffuse = (difuseColor * difuseIntensity * max(dot(vert_Normal, normalize(lightDirection)), 0.0));

		//specular (dot de direccion de la camara respecto la cara del objeto)
		vec4 specular = ((pow(max(dot(normalize(posCamera - fragPos), reflect(-lightDirection, vert_Normal)), 0.0), specularDensity)) * specularIntensity * specularColor);

		//Sumamos las anteriores operaciones y finalmente multiplicamos por el color original del objeto
		result = vec3((ambient + diffuse + specular) * objectColor);
		out_Color = vec4(result, 1.0);
		resultOfAll = texture(ourTexture, TexCoord) * out_Color;
		
		FragColor = vec4(resultOfAll.x,resultOfAll.y, resultOfAll.z, 0.5f);
	}