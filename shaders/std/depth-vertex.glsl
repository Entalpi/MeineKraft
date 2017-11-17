uniform mat4 projection;
uniform mat4 camera_view;

in mat4 model;
in vec3 position;

void main() {
    gl_Position = projection * camera_view * model * vec4(position, 1.0f);
}