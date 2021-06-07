#version 330
    layout (points) in;
    layout (triangle_strip,max_vertices = 4) out;
    out vec4 vert_Normal;
    out vec2 TexCoord;

    vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position + vec4(10, -10, 0, 0)) - vec3(gl_in[0].gl_Position + vec4(10, 10, 0, 0));
   vec3 b = vec3(gl_in[0].gl_Position + vec4(-10, -10, 0, 0)) - vec3(gl_in[0].gl_Position + vec4(-10, 10, 0, 0));
   return normalize(cross(a, b));
} 

    vec4 newNormal = vec4(GetNormal(), 0.f);
    void main() {
        
        gl_Position = gl_in[0].gl_Position + vec4(100, -100, 0, 0);
        vert_Normal = newNormal;
        TexCoord = vec2(1, 0);
        EmitVertex();
        gl_Position = gl_in[0].gl_Position + vec4(100, 100, 0, 0);
        vert_Normal = newNormal;
        TexCoord = vec2(1, 1);
        EmitVertex();
        gl_Position = gl_in[0].gl_Position + vec4(-100, -100, 0, 0);
        vert_Normal = newNormal;
        TexCoord = vec2(0, 0);
        EmitVertex();
        gl_Position = gl_in[0].gl_Position + vec4(-100, 100, 0, 0);
        vert_Normal = newNormal;
        TexCoord = vec2(0, 1);
        EmitVertex();
        EndPrimitive();
    }