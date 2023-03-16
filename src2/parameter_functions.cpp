#include "func.cpp"

void Text::draw() {
    glm::vec2 translate = glm::vec2{box.position.x < 0 ? core.screen_size.x + box.position.x : box.position.x, box.position.y < 0 ? core.screen_size.y + box.position.y : box.position.y};
    glm::mat3 trans_mat = glm::translate(identity_matrix, translate);

    core.gui_core.shaders[s_text_col].use();

    vertex_buf.bind();
    core.gui_core.textures[t_text].bind(0);
    glUniformMatrix3fv(0, 1, false, &core.screen_matrix[0][0]);
    glUniformMatrix3fv(1, 1, false, &trans_mat[0][0]);
    glUniform4fv(2, 1, &color[0]);

    glDrawArrays(GL_TRIANGLES, 0, vertex_buf.vertices);
}

inline void Text_box::update_text() {
    text.load_buffers(core.gui_core.font, str);
}

void tb0_calc(Text_box& self) {
    if(self.box.contains(core.cursor_pos)) {
        self.hovered = true;
    } else {
        self.hovered = false;
    }

    for(Event& e : core.gui_core.events) {
        switch(e.index()) {
            case 0: {
                Key_event& a = std::get<0>(e);
                switch(a.key) {
                    case GLFW_KEY_LEFT: {
                        if(a.action == GLFW_PRESS || a.action == GLFW_REPEAT) {
                            if(self.selected) {
                                if(self.pos != 0) {
                                    --self.pos;
                                    self.pos_coord.x -= (((self.pos > 0) ? core.gui_core.font.glyph_map[self.str[self.pos]].advance1 : 0) + core.gui_core.font.glyph_map[self.str[self.pos]].advance2 + core.gui_core.font.glyph_map[self.str[self.pos]].tex_width) * self.text.size;
                                }
                            }
                        }
                        break;
                    }
                    case GLFW_KEY_RIGHT: {
                        if(self.selected) {
                            if(a.action == GLFW_PRESS || a.action == GLFW_REPEAT) {
                                if(self.pos != self.str.size()) {
                                    self.pos_coord.x += (((self.pos > 0) ? core.gui_core.font.glyph_map[self.str[self.pos]].advance1 : 0) + core.gui_core.font.glyph_map[self.str[self.pos]].advance2 + core.gui_core.font.glyph_map[self.str[self.pos]].tex_width) * self.text.size;
                                    ++self.pos;
                                }
                            }
                        }
                        break;
                    }
                    case GLFW_KEY_BACKSPACE: {
                        if(a.action == GLFW_PRESS || a.action == GLFW_REPEAT) {
                            if(self.selected) {
                                if(self.pos != 0) {
                                    --self.pos;
                                    int change = (((self.pos > 0) ? core.gui_core.font.glyph_map[self.str[self.pos]].advance1 : 0) + core.gui_core.font.glyph_map[self.str[self.pos]].advance2 + core.gui_core.font.glyph_map[self.str[self.pos]].tex_width) * self.text.size;
                                    self.box.size.x -= change;
                                    self.pos_coord.x -= change;

                                    self.str.erase(self.str.begin() + self.pos);
                                    self.update_text();
                                }
                            }
                        }
                        break;
                    }
                }
                break;
            }
            case 1: {
                Mouse_button_event& a = std::get<1>(e);
                if(a.button == GLFW_MOUSE_BUTTON_LEFT) {
                    if(a.action == GLFW_PRESS) {
                        if(self.hovered) {
                            if(!self.selected) {
                                self.selected = true;
                            }

                            int new_pos = 0;
                            int loc = 0;
                            int prev_loc = 0;

                            while(true) {
                                if(loc >= core.cursor_pos.x - self.text.box.position.x) {
                                    self.pos_coord.x = prev_loc;
                                    --new_pos;
                                    break;
                                } else if(new_pos >= self.str.size()) {
                                    self.pos_coord.x = loc;
                                    --new_pos;
                                    break;
                                }
                                prev_loc = loc;
                                loc += (((new_pos > 0) ? core.gui_core.font.glyph_map[self.str[new_pos]].advance1 : 0) + core.gui_core.font.glyph_map[self.str[new_pos]].advance2 + core.gui_core.font.glyph_map[self.str[new_pos]].tex_width) * self.text.size;
                                ++new_pos;
                            }

                            self.pos = std::max(new_pos, 0);
                        } else {
                            self.selected = false;
                        }
                    }
                }
                break;
            }
            case 2: {
                Char_event& a = std::get<2>(e);
                if(self.selected) {
                    uint8_t c;
                    if(core.key_map[GLFW_KEY_RIGHT_SHIFT] && a.c >= 'A' && a.c <= 'F') {
                        c = a.c - 'A' + 0x80;
                    } else {
                        c = a.c;
                    }
                    int change = (((self.pos > 0) ? core.gui_core.font.glyph_map[c].advance1 : 0) + core.gui_core.font.glyph_map[c].advance2 + core.gui_core.font.glyph_map[c].tex_width) * self.text.size;
                    self.box.size.x += change;
                    self.pos_coord.x += change;

                    if(self.pos == self.str.size()) self.str += c;
                    else self.str.insert(self.str.begin() + self.pos, c);
                    ++self.pos; 
                    self.update_text();
                }

                break;
            }
        }
    }
}

void tb0_draw(Text_box& self) {
    if(self.selected) {
        core.gui_core.buffers[b_rect].bind();
        core.gui_core.shaders[s_flat].use();
        
        glm::vec4 select_color = {self.text.color.rgb(), 0.25};
        glm::mat3 trans_mat = glm::scale(glm::translate(identity_matrix, glm::vec2{self.box.position}), glm::vec2{self.box.size});

        glUniformMatrix3fv(0, 1, false, &trans_mat[0][0]);
        glUniformMatrix3fv(1, 1, false, &core.screen_matrix[0][0]);
        glUniform4fv(2, 1, &select_color[0]);

        glDrawArrays(GL_TRIANGLES, 0, core.gui_core.buffers[b_rect].vertices);

        trans_mat = glm::scale(glm::translate(identity_matrix, glm::vec2{self.text.box.position + self.pos_coord}), glm::vec2{self.text.size, 9 * self.text.size});

        glUniformMatrix3fv(0, 1, false, &trans_mat[0][0]);
        glUniformMatrix3fv(1, 1, false, &core.screen_matrix[0][0]);
        glUniform4fv(2, 1, &self.text.color[0]);

        glDrawArrays(GL_TRIANGLES, 0, core.gui_core.buffers[b_rect].vertices);
    } else if(self.hovered) {
        core.gui_core.buffers[b_rect].bind();
        core.gui_core.shaders[s_flat].use();
        
        glm::vec4 hover_color = {self.text.color.rgb(), 0.125};
        glm::mat3 trans_mat = glm::scale(glm::translate(identity_matrix, glm::vec2{self.box.position}), glm::vec2{self.box.size});

        glUniformMatrix3fv(0, 1, false, &trans_mat[0][0]);
        glUniformMatrix3fv(1, 1, false, &core.screen_matrix[0][0]);
        glUniform4fv(2, 1, &hover_color[0]);

        glDrawArrays(GL_TRIANGLES, 0, core.gui_core.buffers[b_rect].vertices);
    }

    self.text.draw();
}