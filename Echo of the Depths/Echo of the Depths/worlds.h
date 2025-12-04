#pragma once
#include "SFML/Graphics.hpp"
#include "MainPlayer.h"
#include "Chunk.h"
#include "enums.h"
#include "SwordEffect.h"

using sf::Texture;
using sf::RenderWindow;

class world_assets {
public :
	// camera
	float window_half_width, window_half_height;
	float map_width = 5000.f, map_height = 5000.f;
	sf::View cam;

	// visuals : chunks & textures & etc
	std::vector<std::vector<Texture>> chunk_textures;
	std::vector <std::vector<Chunk>> chunks;
	std::vector<std::vector<sf::Image>> chunk_mask_imgs;
	Locations new_location = Locations::hub;
	sf::Clock delta_clock;

	bool all_is_locked = false;
	

	virtual bool load_chunk_textures() = 0;
	virtual bool load_chunk_mask() = 0;
	virtual void update_and_draw_all(MainPlayer& player, RenderWindow& window, SwordSlashEffect& swordEffect) const = 0;

	Chunk& getChunk(int cx, int cy) {
		return chunks[cy][cx];
	}

	bool update_collisions_mask(MainPlayer& player, sf::Vector2f& move) {
		Vector2f nextPos = player.sprite.getPosition() + move;
		int cx = nextPos.x / SIZE;
		int cy = nextPos.y / SIZE;

		if (cy < 0 || cy >= chunks.size()) return false;
		if (cx < 0 || cx >= chunks[cy].size()) return false;

		Chunk& ch = getChunk(cx, cy);

		sf::FloatRect bounds = player.sprite.getGlobalBounds();

		std::vector<Vector2f> points = {
		{ bounds.left,  bounds.top }, // 1
		{ bounds.left + bounds.width / 2.f, bounds.top }, // 2
		{ bounds.left + bounds.width, bounds.top }, // 3

		{ bounds.left,  bounds.top + bounds.height / 2.f }, // 4
		{ bounds.left + bounds.width, bounds.top + bounds.height / 2.f }, // 5

		{ bounds.left,bounds.top + bounds.height }, // 6
		{ bounds.left + bounds.width / 2.f, bounds.top + bounds.height }, // 7
		{ bounds.left + bounds.width, bounds.top + bounds.height } // 8
		};

		for (auto& point : points) {
			int lx = static_cast<int>(point.x - ch.X);
			int ly = static_cast<int>(point.y - ch.Y);
			if (lx < 0 || lx >= ch.chunk_mask.getSize().x) continue;
			if (ly < 0 || ly >= ch.chunk_mask.getSize().y) continue;

			sf::Color px = ch.chunk_mask.getPixel(lx, ly);
			if (px.r <= 128) {
				return false;
			}
		}
		return true;
	}

	bool update_collisions_lvl(MainPlayer& player) {
		for (auto& r : chunks) {
			for (auto& c : r) {
				if (c.loaded) {
					for (auto& collision : c.collisions) {
						if (player.sprite.getGlobalBounds().intersects(collision.getGlobalBounds())) {
							return false;
						}
					}
				}
			}
		}
		return true;
	}

	void chunk_setTextures() {
		for (int raw = 0; raw < chunk_textures.size(); raw++) {
			for (int column = 0; column < chunk_textures[raw].size(); column++) {
				chunks[raw][column].setTexture(&chunk_textures[raw][column]);
				chunks[raw][column].X = column * SIZE;
				chunks[raw][column].Y = raw * SIZE;
				chunks[raw][column].chunk_sprite.setPosition(chunks[raw][column].X, chunks[raw][column].Y);
			}
		}
		for (int r = 0; r < chunks.size(); r++) {
			for (int c = 0; c < chunks[r].size(); c++) {
				chunks[r][c].chunk_mask = chunk_mask_imgs[r][c];
			}
		}
	}

	void update_camera(RenderWindow& window, MainPlayer& player) {
		float camX = std::clamp(player.sprite.getPosition().x, window_half_width, map_width - window_half_width);
		float camY = std::clamp(player.sprite.getPosition().y, window_half_height, map_height - window_half_height);
		cam.setCenter(camX, camY);
	}

	void update_chunks_visible(MainPlayer& player) {
		int chunkX = floor(player.sprite.getPosition().x / SIZE);
		int chunkY = floor(player.sprite.getPosition().y / SIZE);

		chunkX = std::clamp(chunkX, 0, 4);
		chunkY = std::clamp(chunkY, 0, 4);

		for (int r = 0; r < chunks.size(); r++)
			for (int c = 0; c < chunks[r].size(); c++)
				chunks[r][c].loaded = false;

		auto safeLoad = [&](int y, int x)
		{
			if (y >= 0 && y < 5 && x >= 0 && x < 5)
				chunks[y][x].loaded = true;
		};

		safeLoad(chunkY, chunkX);
		safeLoad(chunkY, chunkX + 1);
		safeLoad(chunkY, chunkX - 1);
		safeLoad(chunkY + 1, chunkX);
		safeLoad(chunkY - 1, chunkX);
		safeLoad(chunkY + 1, chunkX - 1);
		safeLoad(chunkY - 1, chunkX - 1);
		safeLoad(chunkY - 1, chunkX + 1);
		safeLoad(chunkY + 1, chunkX + 1);
	}

	void draw_visible_chunks(RenderWindow& window) {
		for (int r = 0; r < chunks.size(); r++) {
			for (int c = 0; c < chunks[r].size(); c++) {
				if (chunks[r][c].loaded && chunks[r][c].chunk_texture != nullptr)
					window.draw(chunks[r][c].chunk_sprite);
			}
		}
	}
};

class Forests_of_echo_assets : public world_assets {
public :
	Forests_of_echo_assets(RenderWindow &window) {
		window_half_width = window.getSize().x / 2;
		window_half_height = window.getSize().y / 2;

		chunks.resize(5);
		chunk_textures.resize(5);
		chunk_mask_imgs.resize(5);
		for (int raw = 0; raw < chunk_textures.size(); raw++) {
			chunk_textures[raw].resize(5);
			chunks[raw].resize(5);
			chunk_mask_imgs[raw].resize(5);
		}
	}

	bool load_chunk_textures() override {
		for (int raw = 0; raw < chunk_textures.size(); raw++) {
			for (int column = 0; column < chunk_textures[raw].size(); column++) {
				if (!chunk_textures[raw][column].loadFromFile("textures/chaptwo/forests_of_echo/chunks/chunk" + std::to_string(raw + 1) + '_' + std::to_string(column + 1) + ".png")) {
					return false;
				}
			}
		}
		return true;
	}

	bool load_chunk_mask() override {
		return false;
	}

	void update_and_draw_all(MainPlayer& player, RenderWindow& window, SwordSlashEffect& swordEffect) const override {
		struct DrawItem {
			float footY;
			Sprite sprite;
		};

		std::vector<DrawItem> drawList;

		for (auto& raw_chunk : chunks) {
			for (auto& column_chunk : raw_chunk) {
				if (column_chunk.loaded) {
					for (auto& spr : column_chunk.sprites_to_draw) {
						float foot = spr.getPosition().y + spr.getGlobalBounds().height;
						drawList.push_back({ foot, spr });
					}
				}
			}
		}

		float foot = player.sprite.getPosition().y + player.sprite.getGlobalBounds().height;
		drawList.push_back({ foot, player.sprite });

		if (player.activeItem) {
			Sprite sprite_copy = player.activeItem->sprite;
			if (!sprite_copy.getTexture()) return;

			if (player.is_watching_back) {
				sprite_copy.setPosition(player.sprite.getPosition() + player.activeItem->front_offset);
				sprite_copy.setScale(player.activeItem->left_front_scale);
			}
			else if (player.is_watching_left) {
				sprite_copy.setPosition(player.sprite.getPosition() + player.activeItem->left_offset);
				sprite_copy.setScale(player.activeItem->left_front_scale);
			}
			else if (player.is_watching_right) {
				sprite_copy.setPosition(player.sprite.getPosition() + player.activeItem->right_offset);
				sprite_copy.setScale(player.activeItem->right_scale);
			}
			float foot = player.sprite.getPosition().y + player.sprite.getGlobalBounds().height;
			drawList.push_back({ foot, sprite_copy });
		}

		swordEffect.update(player);
		if (swordEffect.isActive()) {
			float foot = player.sprite.getPosition().y + player.sprite.getGlobalBounds().height;
			drawList.push_back({ foot, swordEffect.sprite });
		}

		std::sort(drawList.begin(), drawList.end(),
			[](const DrawItem& a, const DrawItem& b)
			{
				return a.footY < b.footY;
			}
		);

		for (auto& item : drawList)
			window.draw(item.sprite);
	}

};