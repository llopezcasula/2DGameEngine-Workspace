#version 330 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec4 u_Color;
uniform int u_UseTexture;

void main()
{
    if (u_UseTexture == 1)
    {
        FragColor = texture(u_Texture, v_TexCoord) * u_Color;
    }
    else
    {
        FragColor = u_Color;
    }
}


