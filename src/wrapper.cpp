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
    bool initialized = false;

    void init() {
        if(initialized == true) {
            glDeleteBuffers(1, &vertex_buffer);
            glDeleteBuffers(1, &vertex_array);
        } else initialized = true;

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

    ~Buffer() {
        if(initialized == true) {
            glDeleteBuffers(1, &vertex_buffer);
            glDeleteBuffers(1, &vertex_array);

            std::cout << "Buffer deleted\n";
        }
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

            int log_size = 0;
            glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_size);

            std::vector<char> error_log(log_size);
	        glGetShaderInfoLog(fragment_shader, log_size, &log_size, &error_log[0]);
            std::cout << "ERROR: Fragment shader failed to compile:\n" << error_log.data() << "\n";

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
    glm::ivec2 size;
    int num_channels;
    GLenum format;

    bool load(const char* path) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        uint8_t* data = stbi_load(path, &size.x, &size.y, &num_channels, 0);

        if(!data) {
            std::cout << "ERROR: Texture failed to load.\n";
            return false;
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);

            format = GL_RGBA;

            return true;
        }
    }

    void bind(int tex_id, int uniform) {
        glActiveTexture(GL_TEXTURE0 + tex_id);
        glBindTexture(GL_TEXTURE_2D, id);
        glUniform1i(uniform, tex_id);
    }

    void bind() {
        glBindTexture(GL_TEXTURE_2D, id);
    }
};

template<size_t num_color_buffers>
struct Framebuffer {
    GLuint id;
    Texture color_tex[num_color_buffers];
    //Texture depth_stencil_tex;
    GLuint depth_buffer;
    bool initialized = false;

    void init(int width, int height) {
        initialized = true;
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        // allocate and bind color texture
        for(int i = 0; i < num_color_buffers; i++) {
            glGenTextures(1, &color_tex[i].id);
            glBindTexture(GL_TEXTURE_2D, color_tex[i].id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_tex[i].id, 0);

            color_tex[i].size.x = width;
            color_tex[i].size.y = height;
            color_tex[i].num_channels = 3;
            color_tex[i].format = GL_RGB;
        }

        // allocate and bind depth texture
        /*glGenTextures(1, &depth_stencil_tex.id);
        glBindTexture(GL_TEXTURE_2D, depth_stencil_tex.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_stencil_tex.id, 0);*/

        // renderbuffer for depth
        glGenRenderbuffers(1, &depth_buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

        if(glCheckFramebufferStatus(id) == GL_FRAMEBUFFER_COMPLETE) std::cout << "framebuffer complete\n";
        else std::cout << "Framebuffer NOT complete\n";
    }
    
    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    void resize(glm::ivec2 new_size) {
        bind();
        glViewport(0, 0, new_size.x, new_size.y);

        for(int i = 0; i < num_color_buffers; i++) {
            color_tex[i].bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, new_size.x, new_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            color_tex[i].size = new_size;
        }

        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, new_size.x, new_size.y);
    }

    ~Framebuffer() {
        if(initialized) {
            glDeleteFramebuffers(1, &id);
        }
    }
};

void save_texture(Texture& texture, const char* save_path) {
    std::vector<uint8_t> vector(texture.size.x * texture.size.y * texture.num_channels);
    texture.bind();
    glGetTexImage(GL_TEXTURE_2D, 0, texture.format, GL_UNSIGNED_BYTE, vector.data());

    stbi__flip_vertically_on_write = true;
    stbi_write_png(save_path, texture.size.x, texture.size.y, texture.num_channels, vector.data(), texture.size.x * texture.num_channels);
}