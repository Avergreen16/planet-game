#include "core2.cpp"
#include "gui.cpp"

void Text::draw() {
    glm::mat3 trans_mat = glm::translate(identity_matrix, glm::vec2(box.position));

    core.shaders[_text_col].use();

    vertex_buf.bind();
    core.textures[_text].bind(0);
    glUniformMatrix3fv(0, 1, false, &core.view_matrix[0][0]);
    glUniformMatrix3fv(1, 1, false, &trans_mat[0][0]);
    glUniform4fv(2, 1, &color[0]);

    glDrawArrays(GL_TRIANGLES, 0, vertex_buf.vertices);
}

void b0_click(Button& self) {
    std::cout << "check -> click\n";
}

void b0_draw(Button& self) {
    //std::cout << core.screen_size.x << " " << core.screen_size.y << "\n";
    //std::cout << self.hitbox.position.x << " " << self.hitbox.position.y << " " << self.hitbox.size.x << " " << self.hitbox.size.y << "\n";
}