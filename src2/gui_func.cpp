#include "core2.cpp"
#include "gui.cpp"

void b0_click(Button& self) {
    std::cout << "check -> click\n";
}

void b0_draw(Button& self) {
    std::cout << core.screen_size.x << " " << core.screen_size.y << "\n";
    std::cout << self.hitbox.position.x << " " << self.hitbox.position.y << " " << self.hitbox.size.x << " " << self.hitbox.size.y << "\n";
}