#pragma once

#include "includes.cpp"

const glm::mat3 identity_matrix_3 = glm::identity<glm::mat3>();
const glm::mat4 identity_matrix_4 = glm::identity<glm::mat4>();
const glm::mat4 rot_z_4 = {
    {1, 0, 0, 0},
    {0, -1, 0, 0},
    {0, 0, -1, 0},
    {0, 0, 0, 1}
};
const glm::mat4 inverse_z_4 = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, -1, 0},
    {0, 0, 0, 1}
};

std::string get_text_from_file(std::string path) {
    std::ifstream file;
    file.open(path);

    if(file.is_open()) {
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();

    } else {
        std::cout << "failed to open file " << path << std::endl;
    }

    file.close();
}

struct Buffer {
    GLuint vertex_array;
    GLuint vertex_buffer;
    int vertices = 0;
    bool initialized = false;

    Buffer(const Buffer& a) {
        vertex_array = a.vertex_array;
        vertex_buffer = a.vertex_buffer;
        vertices = a.vertices;
        initialized = a.initialized;
    }
    
    Buffer() = default;

    Buffer(Buffer&& a) {
        std::swap(vertex_array, a.vertex_array);
        std::swap(vertex_buffer, a.vertex_buffer);
        std::swap(vertices, a.vertices);
        std::swap(initialized, a.initialized);
    };

    Buffer& operator=(const Buffer& a) {
        vertex_array = a.vertex_array;
        vertex_buffer = a.vertex_buffer;
        vertices = a.vertices;
        initialized = a.initialized;

        return *this;
    };

    Buffer& operator=(Buffer&& a) {
        std::swap(vertex_array, a.vertex_array);
        std::swap(vertex_buffer, a.vertex_buffer);
        std::swap(vertices, a.vertices);
        std::swap(initialized, a.initialized);

        return *this;
    }

    void init() {
        initialized = true;

        glGenVertexArrays(1, &vertex_array);
        glBindVertexArray(vertex_array);

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    }

    void bind() {
        glBindVertexArray(vertex_array);
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

    bool compile(std::string vertex_shader_path, std::string fragment_shader_path) {
        std::string vertex_shader_src = get_text_from_file(vertex_shader_path);
        std::string fragment_shader_src = get_text_from_file(fragment_shader_path);
        const char* v_ptr = vertex_shader_src.data();
        const char* f_ptr = fragment_shader_src.data();
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &v_ptr, 0);
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
        glShaderSource(fragment_shader, 1, &f_ptr, 0);
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

        return true;
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

    bool load(const char* path, bool mipmap = false) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        uint8_t* data = stbi_load(path, &size.x, &size.y, &num_channels, 0);

        if(!data) {
            std::cout << "ERROR: Texture failed to load.\n";
            return false;
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);

            format = GL_RGBA;

            if(mipmap) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 3);
                //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 4.0);
                //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.25f);
                glGenerateMipmap(GL_TEXTURE_2D);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }

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
    
    Texture() = default;

    Texture(const Texture& t) = delete;

    Texture(Texture&& t) {
        id = t.id;
        size = t.size;
        num_channels = t.num_channels;
        format = t.format;
        t.id = 0;
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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_tex[i].id, 0);

            color_tex[i].size.x = width;
            color_tex[i].size.y = height;
            color_tex[i].num_channels = 3;
            color_tex[i].format = GL_RGBA;
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

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    void bind(int width, int height) {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        glViewport(0, 0, width, height);
    }

    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    void resize(glm::ivec2 new_size) {
        bind(new_size.x, new_size.y);

        for(int i = 0; i < num_color_buffers; i++) {
            color_tex[i].bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, new_size.x, new_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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