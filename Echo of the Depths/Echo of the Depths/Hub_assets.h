#pragma once
#include "SFML/Graphics.hpp"
#include "Chunk.h"
#include "enums.h"

class MainPlayer;
class SwordSlashEffect;
enum Locations;

using namespace sf;
using std::string;
using std::vector;

class Hub_assets {
private :
	float window_half_width, window_half_height;
	float map_width = 5000.f, map_height = 5000.f;
public:
	std::vector<std::vector<Texture>> chunk_textures;
	std::vector <std::vector<Chunk>> chunks;
	std::vector<std::vector<sf::Image>> chunk_mask_imgs;
	Texture crystal_left_texture, crystal_right_texture, crystal_top_texture, crystal_down_texture, mid_crystal_top_texture, 
		mid_crystal_bot_texture;
	Sprite mid_crystal_top_sprite;
	Locations new_location = Locations::hub;

	// tp rectangle fading
	RectangleShape last_rect;
	int last_rect_alpha = 0;
	bool last_rect_active = false;
	Clock last_rect_clock;
	enum FS { Non, FaIn, Wait, FaOut };
	bool enter_clicked = false;
	// tp rectangle fading

	struct Crystal {
		Texture texture;
		RectangleShape rect;
		Locations location;

		Crystal(const Vector2f& size, const Vector2f& position, const Locations& current_location) {
			rect.setSize(size);
			rect.setPosition(position);
			location = current_location;
		}

		bool load_texture(const string& filename) {
			if (!texture.loadFromFile(filename)) {
				return false;
			}
			return true;
		}

		void crystal_set_texture(Sprite& sprite) {
			sprite.setTexture(texture);
		}
	};

	// crystals & world description
	Crystal wastelands_of_ash, cold_memories, forests_of_echo, broken_lands;
	vector<Crystal> crystals;
	bool draw_sprite = false;
	RectangleShape black_rect;
	Sprite world_story_sprite;

	sf::View cam; // camera

	// Text
	Text pressF;
	Text pressEnter;
	// Text

	bool all_is_locked = false;

	Hub_assets(RenderWindow& window, Font& font);
		
	bool load_hub_masks();
	bool load_hub_textures();
		
	void show_world_description(Event& event, MainPlayer& player);
	void draw_world_description(RenderWindow& window);

	void update_and_draw_all(MainPlayer &player, RenderWindow& window, SwordSlashEffect &swordEffect);
		
	Chunk& getChunk(int cx, int cy);

	bool update_collisions_mask(MainPlayer& player, sf::Vector2f& move);
	bool update_collisions_lvl(MainPlayer& player);
	void chunk_setTextures();

	void update_camera(RenderWindow& window, MainPlayer& player);

	void update_chunks_visible(MainPlayer& player);

	void draw_visible_chunks(RenderWindow& window);
};