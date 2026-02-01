#version 330 core
out vec4 FragColor;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform int u_UseTexture;
uniform vec4 u_Color;

void main()
{
    vec4 col = u_Color;

    if (u_UseTexture == 1)
    {
        vec4 tex = texture(u_Texture, v_TexCoord);
        col *= tex;               // keeps tex.a
        // Optional: discard fully transparent pixels (prevents fringes)
        // if (col.a < 0.01) discard;
    }

    FragColor = col;
}
