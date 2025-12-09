#include "Hub_assets.h"
#include "MainPlayer.h"
#include "Chunk.h"
#include "enums.h"
#include "SwordEffect.h"

using namespace sf;

Hub_assets::Hub_assets(RenderWindow& window, Font& font) :
	wastelands_of_ash(Vector2f(130.f, 130.f), Vector2f(1745.f, 1920.f), Locations::wastelands_of_ash),
	cold_memories(Vector2f(130.f, 130.f), Vector2f(3190.f, 1912.f), Locations::memoeris_of_cold),
	forests_of_echo(Vector2f(130.f, 130.f), Vector2f(3160.f, 2950.f), Locations::forests_of_echo),
	broken_lands(Vector2f(130.f, 130.f), Vector2f(1744.f, 2942.f), Locations::broken_lands)
{
	black_rect.setSize(sf::Vector2f(1920.f, 1080.f));
	black_rect.setFillColor(sf::Color(0, 0, 0, 170));

	window_half_height = window.getSize().x / 2;
	window_half_width = window.getSize().y / 2;

	wastelands_of_ash.load_texture("textures/chaptwo/wastelands_of_ash/description.png");  cold_memories.load_texture("textures/chaptwo/cold_memories/description.png");
	broken_lands.load_texture("textures/chaptwo/broken_lands/description.png"); forests_of_echo.load_texture("textures/chaptwo/forests_of_echo/description.png");

	crystal_left_texture.loadFromFile("textures/chaptwo/hub/sprites_to_draw/crystal2_2.png");
	crystal_top_texture.loadFromFile("textures/chaptwo/hub/sprites_to_draw/crystal2_4.png");
	crystal_right_texture.loadFromFile("textures/chaptwo/hub/sprites_to_draw/crystal3_4.png");
	crystal_down_texture.loadFromFile("textures/chaptwo/hub/sprites_to_draw/crystal3_2.png");
	mid_crystal_top_texture.loadFromFile("textures/chaptwo/hub/sprites_to_draw/mid_crystal_top.png");
	mid_crystal_bot_texture.loadFromFile("textures/chaptwo/hub/sprites_to_draw/mid_crystal_bot.png");

	crystals.push_back(wastelands_of_ash);
	crystals.push_back(cold_memories);
	crystals.push_back(forests_of_echo);
	crystals.push_back(broken_lands);

	cam.setSize(window.getSize().x, window.getSize().y);
	chunks.resize(5);
	chunk_textures.resize(5);
	chunk_mask_imgs.resize(5);
	for (int raw = 0; raw < chunk_textures.size(); raw++) {
		chunk_textures[raw].resize(5);
		chunks[raw].resize(5);
		chunk_mask_imgs[raw].resize(5);
	}

	pressEnter.setFont(font);
	pressEnter.setFillColor(Color::White);
	pressEnter.setCharacterSize(24);
	pressEnter.setString("Press [Enter] to enter the level");

	pressF.setFont(font);
	pressF.setFillColor(Color::White);
	pressF.setCharacterSize(24);
	pressF.setString("Press [F] to read about level");

	// collisions
	chunks[2][3].collisions.resize(3);
	chunks[2][3].collisions[0].setSize(Vector2f(60.f, 1.f));
	chunks[2][3].collisions[0].setPosition((3229.f - chunks[2][3].X), (2977.f - chunks[2][3].Y));

	chunks[2][3].collisions[1].setSize(Vector2f(47.f, 1.f));
	chunks[2][3].collisions[1].setPosition((3182.f - chunks[2][3].X), (2959.f - chunks[2][3].Y));

	chunks[2][3].collisions[2].setSize(Vector2f(1.f, 1.f));
	chunks[2][3].collisions[2].setPosition((3158.f - chunks[2][3].X), (2938.f - chunks[2][3].Y));

	chunks[1][3].collisions.resize(1);
	chunks[1][3].collisions[0].setSize(Vector2f(106.f, 1.f));
	chunks[1][3].collisions[0].setPosition((3196.f - chunks[1][3].X), (1940.f - chunks[1][3].Y));

	chunks[1][1].collisions.resize(1);
	chunks[1][1].collisions[0].setSize(Vector2f(105.f, 1.f));
	chunks[1][1].collisions[0].setPosition((1760.f - chunks[1][1].X), (1950.f - chunks[1][1].Y));

	chunks[2][1].collisions.resize(1);
	chunks[2][1].collisions[0].setSize(Vector2f(106.f, 1.f));
	chunks[2][1].collisions[0].setPosition((1753.f - chunks[2][1].X), (2927.f - chunks[2][1].Y));
	// collisions

	chunks[1][1].sprites_to_draw.resize(1);
	chunks[1][3].sprites_to_draw.resize(1);
	chunks[2][1].sprites_to_draw.resize(1);
	chunks[2][3].sprites_to_draw.resize(1);
	chunks[2][2].sprites_to_draw.resize(1);

	chunks[1][1].sprites_to_draw[0].setTexture(crystal_left_texture);
	chunks[1][3].sprites_to_draw[0].setTexture(crystal_top_texture);
	chunks[2][1].sprites_to_draw[0].setTexture(crystal_down_texture);
	chunks[2][3].sprites_to_draw[0].setTexture(crystal_right_texture);
	chunks[2][2].sprites_to_draw[0].setTexture(mid_crystal_bot_texture);
	mid_crystal_top_sprite.setTexture(mid_crystal_top_texture);

	mid_crystal_top_sprite.setPosition(2425.f - chunks[1][2].X, 1543.f - chunks[1][2].Y);
	chunks[1][1].sprites_to_draw[0].setPosition(1753.f - chunks[1][1].X, 1771.f - chunks[1][1].Y);
	chunks[1][3].sprites_to_draw[0].setPosition(3196.f - chunks[1][3].X, 1745.f - chunks[1][3].Y);
	chunks[2][1].sprites_to_draw[0].setPosition(1744.f - chunks[2][1].X, 2765.f - chunks[2][1].Y);
	chunks[2][3].sprites_to_draw[0].setPosition(3142.f - chunks[2][3].X, 2710.f - chunks[2][3].Y);
	chunks[2][2].sprites_to_draw[0].setPosition(2203.f - chunks[2][2].X, 2048.f - chunks[2][2].Y);

	last_rect.setSize(Vector2f(1920.f, 1080.f));
	last_rect.setFillColor(Color::Black);
}

void Hub_assets::update_and_draw_all(MainPlayer& player,
	RenderWindow& window,
	SwordSlashEffect& swordEffect)
{
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
		drawList.push_back({ foot, sprite_copy});
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
	
	window.draw(mid_crystal_top_sprite);
}

bool Hub_assets:: load_hub_masks() {
	for (int raw = 0; raw < chunk_mask_imgs.size(); raw++) {
		for (int column = 0; column < chunk_mask_imgs[raw].size(); column++) {
			if (!chunk_mask_imgs[raw][column].loadFromFile("textures/chaptwo/hub/mask_chunks/mask_chunk" + std::to_string(raw + 1) + "_" + std::to_string(column + 1) + ".png")) {
				return false;
			}
		}
	}
	return true;
}

bool Hub_assets:: load_hub_textures() {
	for (int raw = 0; raw < chunk_textures.size(); raw++) {
		for (int column = 0; column < chunk_textures[raw].size(); column++) {
			if (!chunk_textures[raw][column].loadFromFile("textures/chaptwo/hub/chunks/chunk" + std::to_string(raw + 1) + "_" + std::to_string(column + 1) + ".png")) {
				return false;
			}
		}
	}
	return true;
}

void Hub_assets::show_world_description(Event& event, MainPlayer& player) {
	if (event.type == Event::KeyPressed) {
		if (event.key.code == Keyboard::F) {
			bool found = false;
			for (auto& crystal : crystals) {
				if (player.sprite.getGlobalBounds().intersects(crystal.rect.getGlobalBounds())) {
					crystal.crystal_set_texture(world_story_sprite);
					found = true;
					break;
				}
			}
			draw_sprite = found;
			all_is_locked = found;
		}
		if (event.key.code == Keyboard::Escape) {
			draw_sprite = false;
			all_is_locked = false;
		}
	}
}

void Hub_assets:: draw_world_description(RenderWindow& window) {
	if (draw_sprite) {
		black_rect.setPosition(0.f, 0.f);
		world_story_sprite.setPosition(347.f, 185.f);

		window.draw(black_rect);
		window.draw(world_story_sprite);
	}
}

Chunk& Hub_assets::getChunk(int cx, int cy) {
	return chunks[cy][cx];
}

bool Hub_assets:: update_collisions_mask(MainPlayer& player, sf::Vector2f& move) {
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
		/*if (px.r <= 128) {
			return false;
		}*/
		/*if (px == Color::Black) {
			return false;
		}*/
		if (px.r < 5 && px.g < 5 && px.b < 5) {
			return false;
		}
	}
	return true;
}

bool Hub_assets:: update_collisions_lvl(MainPlayer& player) {
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

void Hub_assets::chunk_setTextures() {
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

void Hub_assets:: update_camera(RenderWindow& window, MainPlayer& player) {
	float camX = std::clamp(player.sprite.getPosition().x, window_half_width, map_width - window_half_width);
	float camY = std::clamp(player.sprite.getPosition().y, window_half_height, map_height - window_half_height );

	cam.setCenter(camX, camY);

	window.setView(cam);
}

void Hub_assets:: update_chunks_visible(MainPlayer& player) {
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

void Hub_assets:: draw_visible_chunks(RenderWindow& window) {
	for (int r = 0; r < chunks.size(); r++) {
		for (int c = 0; c < chunks[r].size(); c++) {
			if (chunks[r][c].loaded && chunks[r][c].chunk_texture != nullptr)
				window.draw(chunks[r][c].chunk_sprite);
		}
	}
}