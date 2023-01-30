#pragma once

#include "global.cpp"

/*struct Vertex {
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
};*/

struct Buffer {
    GLuint vertex_array;
    GLuint vertex_buffer;
    int vertices = 0;
    bool initialized = false;

    Buffer(Buffer& a) {
        vertex_array = a.vertex_array;
        vertex_buffer = a.vertex_buffer;
        vertices = a.vertices;
        initialized = true;

        a.vertex_array = 0;
        a.vertex_buffer = 0;
        a.vertices = 0;
        a.initialized = false;
    }
    
    Buffer() = default;

    Buffer(Buffer&& a) {
        vertex_array = a.vertex_array;
        vertex_buffer = a.vertex_buffer;
        vertices = a.vertices;
        initialized = true;

        a.vertex_array = 0;
        a.vertex_buffer = 0;
        a.vertices = 0;
        a.initialized = false;
    };

    void init() {
        initialized = true;

        glGenVertexArrays(1, &vertex_array);
        glBindVertexArray(vertex_array);

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    }

    void set_data(void* ptr, int num_vertices, int size_per_vertex, GLenum usage = GL_STATIC_DRAW) {
        vertices = num_vertices;
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, num_vertices * size_per_vertex, ptr, usage);
    }

    void set_attrib(int id, int values, int stride_bytes, int offset_bytes, GLuint type = GL_FLOAT) {
        glBindVertexArray(vertex_array);
        glVertexAttribPointer(id, values, type, GL_FALSE, stride_bytes, (void*)offset_bytes);
        glEnableVertexAttribArray(id);
    }

    void bind() {
        glBindVertexArray(vertex_array);
    }

    void delete_buffers() {
        glDeleteBuffers(1, &vertex_buffer);
        glDeleteVertexArrays(1, &vertex_array);
        initialized = false;
    }

    ~Buffer() {
        if(initialized == true) {
            delete_buffers();
        }
    }
};

struct Storage_buffer {
    GLuint id;
    int size;
    bool initialized = false;

    void init() {
        initialized = true;
        glGenBuffers(1, &id);
    }

    void set_data(void* pointer, int size_bytes) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size_bytes, pointer, GL_DYNAMIC_DRAW);
        size = size_bytes;
    }

    void bind(int index) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, id);
    }

    void delete_buffer() {
        glDeleteBuffers(1, &id);
    }

    ~Storage_buffer() {
        if(initialized) {
            delete_buffer();
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
            int log_size = 0;
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_size);

            std::vector<char> error_log(log_size);
	        glGetShaderInfoLog(vertex_shader, log_size, &log_size, &error_log[0]);
            std::cout << "ERROR: Vertex shader failed to compile:\n" << error_log.data() << "\n";

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

    ~Shader() {
        glDeleteProgram(id);
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

    void bind(int binding) {
        glActiveTexture(GL_TEXTURE0 + binding);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    void bind() {
        glBindTexture(GL_TEXTURE_2D, id);
    }

    ~Texture() {
        glDeleteTextures(1, &id);
    }
};

template<size_t num_color_buffers>
struct Framebuffer {
    GLuint id;
    Texture color_tex[num_color_buffers];
    Texture depth_tex;
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
        glGenTextures(1, &depth_tex.id);
        glBindTexture(GL_TEXTURE_2D, depth_tex.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        depth_tex.size = {width, height};

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex.id, 0);


        GLenum buffers[num_color_buffers];
        for(int i = 0; i < num_color_buffers; ++i) {
            buffers[i] = GL_COLOR_ATTACHMENT0 + i;
        }
        glDrawBuffers(num_color_buffers, buffers);
    }
    
    void bind(int width, int height) {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        glViewport(0, 0, width, height);
    }

    void resize(glm::ivec2 new_size) {
        bind(new_size.x, new_size.y);

        for(int i = 0; i < num_color_buffers; i++) {
            color_tex[i].bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, new_size.x, new_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            color_tex[i].size = new_size;
        }

        depth_tex.bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, new_size.x, new_size.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
        depth_tex.size = new_size;
    }

    ~Framebuffer() {
        if(initialized) {
            glDeleteFramebuffers(1, &id);
        }
    }
};

struct Framebuffer_depth {
    GLuint id;
    Texture depth_tex;
    bool initialized = false;

    void init(int width, int height) {
        initialized = true;
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        // allocate and bind depth texture
        glGenTextures(1, &depth_tex.id);
        glBindTexture(GL_TEXTURE_2D, depth_tex.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        depth_tex.size = {width, height};

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex.id, 0);

        if(glCheckFramebufferStatus(id) == GL_FRAMEBUFFER_COMPLETE) std::cout << "framebuffer complete\n";
        else std::cout << "Framebuffer NOT complete\n";
    }
    
    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    void bind(int width, int height) {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        glViewport(0, 0, width, height);
    }

    void resize(glm::ivec2 new_size) {
        bind();

        depth_tex.bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, new_size.x, new_size.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
        depth_tex.size = new_size;
    }

    ~Framebuffer_depth() {
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