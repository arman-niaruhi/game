 #version 400
  out vec4 FragColor;
  
  in vec3 pos;
  in vec3 Normal;  
  in vec2 TexCoord;
  
  
  uniform vec3 viewPos; 
  uniform bool shooting; 
  
  
  // texture sampler
  uniform sampler2D text;
  uniform samplerCube enviroMap;
  
  const vec3 lightDirection = normalize(vec3(0.2,-1,0.3));
  float ambient = .8f;
  

  
  void main()
  {       
      if (shooting){
           ambient = 10.8;
          }
      float brightness = max(dot(-lightDirection,normalize(Normal)),0.0)+ ambient;
      FragColor = texture(text,TexCoord) * brightness;
      vec3 I = normalize(pos - viewPos);
      vec3 R = reflect(I, normalize(Normal));
      vec4 reflectedColor = texture(enviroMap, R);
      FragColor = mix(FragColor, reflectedColor, 0.95);
  }
    
  