#version 400
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 ourColor;
in vec2 TexCoord;
in vec3 pos;
in vec3 Normal;  

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;

// texture sampler
uniform sampler2D texture1;

void main()
{
	// ambient
    float ambientStrength = 3;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    //vec3 lightDir = normalize(lightPos - pos);
    //directional light
    vec3 lightDir = normalize(lightPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.3;
    vec3 viewDir = normalize(viewPos - pos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = ambient + diffuse + specular;

	
    
   float brightness = dot(result, vec3(1.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    FragColor = texture(texture1, TexCoord)*vec4(result, 1.0f);
    }