#include <stdio.h>
#include <stdlib.h>

#include <aparse.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image_write.h>

#define debug aparse_prog_debug
#define info aparse_prog_info
#define warn aparse_prog_warn
#define error aparse_prog_error


static void glfw_error_callback(
        int err, 
        const char *desc)
{
    aparse_log("glfw", APARSE__ERROR_LABEL, "error code: %d (\"%s\")", err, desc);
}

int main(int argc, char **argv)
{
    static const int width = 1920;
    static const int height = 1080;

    static const float vertices[] =
    {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f
    };

    static const char *vertex_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec2 position;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position, 0.0, 1.0);\n"
        "}\n";

    static const char *fragment_shader_source =
        "#version 330 core\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "    color = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";
    
    aparse_arg main_args[] =
    {
        aparse_arg_end_marker
    };

    GLFWwindow *window = NULL;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    GLuint program = 0;
    unsigned char *pixels = NULL;

    if(aparse_parse(argc, argv, 
                    main_args, NULL,
                    "softrast reference implementation") != APARSE_STATUS_OK)
        return 1;


    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        error("failed to initialize glfw");
        return 1;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(
            width, height, 
            "softrast OpenGL reference", 
            NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        error("failed to create window");
        return 1;
    }

    glfwMakeContextCurrent(window);

    if(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        error("failed to initialize GLAD");
        return 1;
    }

    glViewport(0, 0, width, height);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        NULL
    );

    glEnableVertexAttribArray(0);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    pixels = malloc((size_t)width * height * 4);
    if (!pixels)
    {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(
        0, 0,
        width, height,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels
    );

    stbi_flip_vertically_on_write(1);

    stbi_write_png(
        "reference.png",
        width, height,
        4,
        pixels,
        width * 4
    );

    free(pixels);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}