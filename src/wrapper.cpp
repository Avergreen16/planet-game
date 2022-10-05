#pragma once

#include "global.cpp"

struct Vertex {
    glm::vec3 position;
    glm::vec2 tex_coord;
    glm::vec2 limit;

    Vertex() = default;

    Vertex(glm::vec3 position, glm::vec2 tex_coord, glm::vec2 limit) {
        this->position = position;
        this->tex_coord = tex_coord;
        this->limit = limit;
    }
    
    Vertex(float x, float y, float z, float u, float v, int lu, int lv) {
        position = {x, y, z};
        tex_coord = {u, v};
        limit = {lu, lv};
    }
};

struct Buffer {
    GLuint vertex_array;
    GLuint vertex_buffer;

    void init() {
        glGenVertexArrays(1, &vertex_array);
        glBindVertexArray(vertex_array);

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    }

    void set_data(void* ptr, size_t size_bytes, GLenum usage = GL_STATIC_DRAW) {
        glBufferData(GL_ARRAY_BUFFER, size_bytes, ptr, usage);
    }

    void set_attrib(int id, int values, int stride_bytes, int offset_bytes, GLuint type = GL_FLOAT) {
        glVertexAttribPointer(id, values, type, GL_FALSE, stride_bytes, (void*)offset_bytes);
        glEnableVertexAttribArray(id);
    }

    void bind() {
        glBindVertexArray(vertex_array);
    }
};

struct Shader {
    GLuint id;

    bool compile(const char* vertex_shader_src, const char* fragment_shader_src) {
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_src, 0);
        glCompileShader(vertex_shader);
        
        // check if vertex shader compiled correctly
        GLint compile_check;
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_check);
        if(compile_check == GL_FALSE) {
            std::cout << "ERROR: Vertex shader failed to compile.\n";
            return false;
        }

        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_src, 0);
        glCompileShader(fragment_shader);
        
        // check if fragment shader compiled correctly
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_check);
        if(compile_check == GL_FALSE) {
            std::cout << "ERROR: Fragment shader failed to compile.\n";
            glDeleteShader(vertex_shader);
            return false;
        }
        
        // attach and link vertex/fragment shaders
        id = glCreateProgram();
        glAttachShader(id, vertex_shader);
        glAttachShader(id, fragment_shader);
        glLinkProgram(id);

        // delete vertex/fragment shaders after compilation
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }

    void use() {
        glUseProgram(id);
    }
};

struct Texture {
    GLuint id;

    bool load(const char* path) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        int width, height, num_channels;
        uint8_t* data = stbi_load(path, &width, &height, &num_channels, 0);

        if(!data) {
            std::cout << "ERROR: Texture failed to load.\n";
            return false;
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            return true;
        }
    }

    void bind() {
        glBindTexture(GL_TEXTURE_2D, id);
    }
};