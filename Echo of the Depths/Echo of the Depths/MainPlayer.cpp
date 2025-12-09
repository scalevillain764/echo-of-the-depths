#include "SFML/Graphics.hpp"
#include "IneventoryTypes.h"
#include "Item_buffs.h"
#include "chest.h"
#include "MainPlayer.h"
#include "Chunk.h"
#include "Hub_assets.h"

using namespace sf;

void MainPlayer:: upd_item_ptr() {
	activeItem = inventory[selectedSlot].item.get();
}

void MainPlayer:: fill_textures_map() {
	back.insert({ "left",  {back_left0,  back_left1} });
	back.insert({ "right", { back_right0, back_right1 } });
	front.insert({ "left", { front_left0, front_left1 } });
	front.insert({ "right", { front_right0, front_right1 } });
	left.insert({ "left", { left_left_leg0, left_left_leg1 } });
	left.insert({ "right", {left_right_leg0, left_right_leg1} });
	right.insert({ "left", { right_left_leg0, right_left_leg1 } });
	right.insert({ "right", { right_right_leg0, right_right_leg1 } });
	menu_inventory_player_texture.push_back(player_collvl0);
	menu_inventory_player_texture.push_back(player_collvl1);
}

bool MainPlayer:: loadTextures() {
	if (!front_left0.loadFromFile("textures/main_player/main_char_front_left.png") || !front_right0.loadFromFile("textures/main_player/main_char_front_right.png")
		|| !back_right0.loadFromFile("textures/main_player/main_char_back_right.png") || !back_left0.loadFromFile("textures/main_player/main_char_back_left.png")
		|| !left_left_leg0.loadFromFile("textures/main_player/main_char_left_left.png") || !left_right_leg0.loadFromFile("textures/main_player/main_char_left_right.png")
		|| !right_left_leg0.loadFromFile("textures/main_player/main_char_right_left.png") || !right_right_leg0.loadFromFile("textures/main_player/main_char_right_right.png")
		|| !unitTex.loadFromFile("textures/main_player/inventory/unit.png") || !invslots_nums_texture.loadFromFile("textures/main_player/inventory/inventory_slots_nums.png")
		|| !activeSlotTex.loadFromFile("textures/main_player/inventory/active_unit.png") || !armor_slot_texture.loadFromFile("textures/main_player/inventory/Armor_slot.png")
		|| !menu_texture.loadFromFile("textures/main_player/inventory/menu.png") || !player_collvl0.loadFromFile("textures/main_player/inventory/main_player_blue.png")
		|| !player_collvl1.loadFromFile("textures/main_player/inventory/main_player_blue_1.png") || !front_left1.loadFromFile("textures/main_player/mp_col_1lvl/main_char_front_left.png") || !front_right1.loadFromFile("textures/main_player/mp_col_1lvl/main_char_front_right.png")
		|| !back_right1.loadFromFile("textures/main_player/mp_col_1lvl/main_char_back_right.png") || !back_left1.loadFromFile("textures/main_player/mp_col_1lvl/main_char_back_left.png")
		|| !left_left_leg1.loadFromFile("textures/main_player/mp_col_1lvl/main_char_left_left.png") || !left_right_leg1.loadFromFile("textures/main_player/mp_col_1lvl/main_char_left_right.png")
		|| !right_left_leg1.loadFromFile("textures/main_player/mp_col_1lvl/main_char_right_left.png") || !right_right_leg1.loadFromFile("textures/main_player/mp_col_1lvl/main_char_right_right.png")) {
		return false;
	}
	fill_textures_map();
	return true;
}

void MainPlayer:: setUpUI() {
	col_description = " - Resistance to slowing (";

	colTEXT.setFillColor(Color::Black);
	colTEXT.setCharacterSize(22);
	colTEXT.setPosition(835.f, 509.f);
	colTEXT.setString(chains_of_light_name + ": " + std::to_string(col_lvl) + " lvl" + col_description + std::to_string(static_cast<int>(100 * col_resistance_multiplier)) + "%)");

	health_text.setCharacterSize(24);
	health_text.setPosition(973.f, 995.f);
	health_text.setFillColor(sf::Color::White);

	tab_inventory.setPosition(575.f, 1030.f);
	tab_inventory.setCharacterSize(20);
	tab_inventory.setString("[TAB] - inventory.");
	tab_inventory.setFillColor(Color::White);

	space_use_item.setPosition(575.f, 1050.f);
	space_use_item.setCharacterSize(20);
	space_use_item.setString("[SPACE] - use item.");
	space_use_item.setFillColor(Color::White);

	// name text
	name_text.setFillColor(Color::White);
	name_text.setCharacterSize(24);
	name_text.setPosition(667.f, 300.f);
	name_text.setString(name);
	// name text

	// damage text
	damage_text.setCharacterSize(24);
	// damage text

	// armor text
	armor_text.setCharacterSize(24);
	// armor text

	inventory.resize(9);
	menu_inventory.resize(4);
	for (size_t i = 0; i < menu_inventory.size(); i++) {
		menu_inventory[i].resize(9);
	}

	health_bar.setPosition(733.f, 1000.f);
	health_bar.setFillColor(health_color);
	health_back_bar.setSize(sf::Vector2f(505.f, 21.f));
	health_back_bar.setPosition(733.f, 1000.f);
	health_back_bar.setFillColor(back_color);

	menu_sprite.setTexture(menu_texture);
	if (col_lvl <= menu_inventory_player_texture.size())
		player_icon_sprite.setTexture(menu_inventory_player_texture[col_lvl]);

	menu_sprite.setPosition(557.f, 256.f);
	player_icon_sprite.setPosition(621.f, 356.f);

	menu_black_rect.setFillColor(Color(0, 0, 0, 170));
	menu_black_rect.setPosition(0.f, 0.f);
	menu_black_rect.setSize(Vector2f(1920.f, 1080.f));
}

// saving
void MainPlayer:: copy_items_ifdead() {
	inventory_copy_for_this.resize(9);
	menu_inventory_copy.resize(4);
	for (size_t i = 0; i < menu_inventory_copy.size(); i++) {
		menu_inventory_copy[i].resize(9);
	}
	for (size_t i = 0; i < menu_inventory_copy.size(); i++) {
		for (size_t j = 0; j < menu_inventory_copy[i].size(); j++) {
			if (menu_inventory[i][j].item && menu_inventory[i][j].item->should_save)
				menu_inventory_copy[i][j].item = menu_inventory[i][j].item->clone();
		}
	}
	for (size_t i = 0; i < inventory.size(); i++) {
		if (inventory[i].item && inventory[i].item->should_save)
			inventory_copy_for_this[i].item = inventory[i].item->clone();
	}
	if (armor_slot.armor) {
		armor_slot_copy.armor = armor_slot.armor->clone();
	}
}

void MainPlayer:: copy_items_for_newlvl() {
	inventory_copy_for_this.resize(9);
	menu_inventory_copy.resize(4);
	for (size_t i = 0; i < menu_inventory_copy.size(); i++) {
		menu_inventory_copy[i].resize(9);
	}
	for (size_t i = 0; i < menu_inventory_copy.size(); i++) {
		for (size_t j = 0; j < menu_inventory_copy[i].size(); j++) {
			if (menu_inventory[i][j].item)
				menu_inventory_copy[i][j].item = menu_inventory[i][j].item->clone();
		}
	}
	for (size_t i = 0; i < inventory.size(); i++) {
		if (inventory[i].item)
			inventory_copy_for_this[i].item = inventory[i].item->clone();
	}
	if (armor_slot.armor) {
		armor_slot_copy.armor = armor_slot.armor->clone();
	}
}

void MainPlayer:: restore_items_from_copy() {
	inventory.resize(9);
	menu_inventory.resize(4);
	for (size_t i = 0; i < menu_inventory_copy.size(); i++) {
		menu_inventory[i].resize(menu_inventory_copy[i].size());
	}
	for (size_t i = 0; i < menu_inventory_copy.size(); i++) {
		for (size_t j = 0; j < menu_inventory_copy[i].size(); j++) {
			if (menu_inventory_copy[i][j].item)
				menu_inventory[i][j].item = menu_inventory_copy[i][j].item->clone();
			else {
				menu_inventory[i][j].item.reset();
			}
		}
	}
	for (size_t i = 0; i < inventory_copy_for_this.size(); i++) {
		if (inventory_copy_for_this[i].item) {
			inventory[i].item = inventory_copy_for_this[i].item->clone();
		}
		else {
			inventory[i].item.reset();
		}
	}
	if (armor_slot_copy.armor) {
		armor_slot.armor = armor_slot_copy.armor->clone();
	}
	else {
		armor_slot.armor.reset();
	}
}
// saving

// drawing inventory & cooldowns
void MainPlayer:: draw_cooldowns(RenderWindow& window) {
	for (auto& slot : inventory) {
		if (slot.item != nullptr) {
			if (!slot.item->cooldown_ready && slot.item->cooldown_clock.getElapsedTime().asSeconds() < slot.item->cooldown) {
				int elapsed_time = slot.item->cooldown_clock.getElapsedTime().asSeconds();
				int remaining_time = std::ceil(slot.item->cooldown - elapsed_time);
				float height = (float(slot.item->cooldown - elapsed_time) / slot.item->cooldown) * 41;
				float offsetY = 41 - height;

				slot.item->cd_text.setString(std::to_string(remaining_time));

				slot.item->cd_rect.setSize(Vector2f(41, height));
				slot.item->cd_rect.setPosition(slot.hitbox.getPosition().x + 4.f, slot.hitbox.getPosition().y + 4.f + offsetY);

				sf::FloatRect bounds = slot.item->cd_text.getLocalBounds();
				slot.item->cd_text.setOrigin(bounds.left + bounds.width / 2.f,
					bounds.top + bounds.height / 2.f);
				slot.item->cd_text.setPosition(slot.hitbox.getPosition().x + slot.hitbox.getGlobalBounds().width / 2.f,
					slot.hitbox.getPosition().y + slot.hitbox.getGlobalBounds().height / 2.f);
				if (remaining_time > 0) {
					window.draw(slot.item->cd_text);
					window.draw(slot.item->cd_rect);
				}
			}
			else {
				slot.item->cooldown_ready = true;
				slot.item->cd_rect.setSize(Vector2f(0.f, 0.f));
			}
		}
	}
	if (menu_is_active) {
		for (size_t raw = 0; raw < menu_inventory.size(); raw++) {
			for (size_t column = 0; column < menu_inventory[raw].size(); column++) {
				if (menu_inventory[raw][column].item) {
					if (!menu_inventory[raw][column].item->cooldown_ready && menu_inventory[raw][column].item->cooldown_clock.getElapsedTime().asSeconds() < menu_inventory[raw][column].item->cooldown) {
						int elapsed_time = menu_inventory[raw][column].item->cooldown_clock.getElapsedTime().asSeconds();
						int remaining_time = std::ceil(menu_inventory[raw][column].item->cooldown - elapsed_time);
						float height = (float(menu_inventory[raw][column].item->cooldown - elapsed_time) / menu_inventory[raw][column].item->cooldown) * 41;
						float offsetY = 41 - height;

						menu_inventory[raw][column].item->cd_text.setString(std::to_string(remaining_time));

						menu_inventory[raw][column].item->cd_rect.setSize(Vector2f(41, height));
						menu_inventory[raw][column].item->cd_rect.setPosition(menu_inventory[raw][column].hitbox.getPosition().x + 4.f, menu_inventory[raw][column].hitbox.getPosition().y + 4.f + offsetY);

						sf::FloatRect bounds = menu_inventory[raw][column].item->cd_text.getLocalBounds();
						menu_inventory[raw][column].item->cd_text.setOrigin(bounds.left + bounds.width / 2.f,
							bounds.top + bounds.height / 2.f);
						menu_inventory[raw][column].item->cd_text.setPosition(menu_inventory[raw][column].hitbox.getPosition().x + menu_inventory[raw][column].hitbox.getGlobalBounds().width / 2.f,
							menu_inventory[raw][column].hitbox.getPosition().y + menu_inventory[raw][column].hitbox.getGlobalBounds().height / 2.f);

						if (remaining_time > 0) {
							window.draw(menu_inventory[raw][column].item->cd_text);
							window.draw(menu_inventory[raw][column].item->cd_rect);
						}
					}
					else {
						menu_inventory[raw][column].item->cooldown_ready = true;
						menu_inventory[raw][column].item->cd_rect.setSize(Vector2f(0.f, 0.f));
					}
				}
			}
		}
	}
}

void MainPlayer:: draw_inventory(RenderWindow& window) {
	if (menu_is_active) {
		window.draw(menu_black_rect);
		window.draw(menu_sprite);
		window.draw(player_icon_sprite);
		window.draw(name_text);
		player_icon_sprite.setTexture(menu_inventory_player_texture[col_lvl]);
		colTEXT.setString(chains_of_light_name + ": " + std::to_string(col_lvl) + " lvl" + col_description + std::to_string(static_cast<int>(100 * col_resistance_multiplier)) + "%)");
		if (activeItem) {
			Sword* sword_ptr = dynamic_cast<Sword*>(activeItem);
			if (sword_ptr) {
				if (sword_ptr->damage > sword_ptr->base_damage) {
					damage_text.setFillColor(Color::Green);
				}
				else if (sword_ptr->damage < sword_ptr->base_damage) {
					damage_text.setFillColor(Color::Red);
				}
				else {
					damage_text.setFillColor(Color::White);
				}
				damage_text.setPosition(623.f, 569.f);
				damage_text.setString(std::to_string(sword_ptr->damage));
			}
			else {
				damage_text.setFillColor(Color::White);
				damage_text.setPosition(628.f, 569.f);
				damage_text.setString("0");
			}
		}
		else {
			damage_text.setFillColor(Color::White);
			damage_text.setPosition(628.f, 569.f);
			damage_text.setString("0");
		}
		window.draw(damage_text);

		if (armor_slot.armor != nullptr) {
			armor_text.setPosition(725.f, 569.f);
			armor_text.setString(std::to_string(armor_slot.armor->armor));
			if (armor_slot.armor->armor > armor_slot.armor->base_armor) {
				armor_text.setFillColor(Color::Green);
			}
			else if (armor_slot.armor->armor < armor_slot.armor->base_armor) {
				armor_text.setFillColor(Color::Red);
			}
			else {
				armor_text.setFillColor(Color::White);
			}
		}
		else {
			armor_text.setPosition(735.f, 569.f);
			armor_text.setFillColor(Color::White);
			armor_text.setString("0");
		}
		window.draw(armor_text);
		window.draw(colTEXT);

		for (size_t i = 0; i < menu_inventory.size(); i++) {
			for (size_t j = 0; j < menu_inventory[i].size(); j++) {
				window.draw(menu_inventory[i][j].hitbox);
				if (menu_inventory[i][j].item) {
					menu_inventory[i][j].item->sprite.setPosition(menu_inventory[i][j].hitbox.getPosition() + sf::Vector2f(4.f, 4.f));
					window.draw(menu_inventory[i][j].item->sprite);
				}
			}
		}
	}
	for (auto& slot : inventory) {
		window.draw(slot.hitbox);
	}
	activeSlot.setPosition(inventory[selectedSlot].hitbox.getPosition().x - 3.f, inventory[selectedSlot].hitbox.getPosition().y - 3.5f);
	window.draw(activeSlot);
	for (auto& slot : inventory) {
		if (slot.item != nullptr) {
			slot.item->sprite.setPosition(slot.hitbox.getPosition() + sf::Vector2f(4.f, 4.f));
			window.draw(slot.item->sprite);
		}
	}
	window.draw(invslots_nums_sprite);
	window.draw(armor_slot.hitbox);
	if (armor_slot.armor != nullptr) {
		armor_slot.armor->sprite.setPosition(armor_slot.hitbox.getPosition() + Vector2f(6.f, 10.f));
		window.draw(armor_slot.armor->sprite);
	}
	window.draw(space_use_item);
	window.draw(tab_inventory);
}
// drawing inventory & cooldowns

// activate & disable menu
void MainPlayer:: activateMenu(bool& all_is_locked) {
	all_is_locked = true;
	menu_is_active = true;
}
void MainPlayer:: disableMenu(bool& all_is_locked) {
	all_is_locked = false;
	menu_is_active = false;
}
// activate & disable menu

// settings

void MainPlayer:: setTextureFrontLeft() { sprite.setTexture(front["left"][col_lvl]); }
void MainPlayer:: setTextureFrontRight() { sprite.setTexture(front["right"][col_lvl]); }
void MainPlayer:: setTextureBackLeft() { sprite.setTexture(back["left"][col_lvl]); }
void MainPlayer:: setTextureBackRight() { sprite.setTexture(back["right"][col_lvl]); }
void MainPlayer:: setTextureLeftLeft() { sprite.setTexture(left["left"][col_lvl]); }
void MainPlayer:: setTextureLeftRight() { sprite.setTexture(left["right"][col_lvl]); }
void MainPlayer:: setTextureRightLeft() { sprite.setTexture(right["left"][col_lvl]); }
void MainPlayer:: setTextureRightRight() { sprite.setTexture(right["right"][col_lvl]); }

void MainPlayer:: setTextsettings(Font& font) { // привязал текст к MainPlayer
	health_text.setFont(font);
	space_use_item.setFont(font);
	name_text.setFont(font);
	tab_inventory.setFont(font);
	damage_text.setFont(font);
	armor_text.setFont(font);
	colTEXT.setFont(font);
}

void MainPlayer:: make_inventory() {
	invslots_nums_sprite.setTexture(invslots_nums_texture);
	invslots_nums_sprite.setPosition(771.f, 1057.f);
	for (size_t i = 0; i < menu_inventory.size(); i++) {
		for (size_t j = 0; j < menu_inventory[i].size(); j++) {
			menu_inventory[i][j].hitbox.setTexture(unitTex);
			menu_inventory[i][j].hitbox.setPosition(StartMenuX + j * 57, StartMenuY);
		}
		StartMenuX = 824;
		StartMenuY += 57;
	}
	activeSlot.setTexture(activeSlotTex);
	for (int i = 0; i < 9; i++) {
		inventory[i].hitbox.setPosition(733 + i * 57, 1025);
		inventory[i].hitbox.setTexture(unitTex);
	}
	armor_slot.hitbox.setTexture(armor_slot_texture);
	armor_slot.hitbox.setPosition(1290.f, 1004.f);
}
// settings

// drawing other
void MainPlayer:: draw_status_bar(Vector2i& mouse_pos, RenderWindow& window) {
	int index = 0;
	for (auto& buff : active_item_buffs) {
		buff->icon.setPosition(startX - index * offset, startY);
		window.draw(buff->icon);
		buff->update_status_description(mouse_pos, window);
		index++;
	}
	for (auto& eff : active_effects) {
		if (eff->isActive) {
			eff->status_sprite.setPosition(startX - index * offset, startY);
			window.draw(eff->status_sprite);
			eff->update_status_description(mouse_pos, window);
			index++;
		}
	}
}

void MainPlayer:: drawEffectsVisuals(RenderWindow& window, MainPlayer& player) {
	for (auto& eff : active_effects) {
		eff->draw(window, player);
	}
}

void MainPlayer:: draw_active_item(RenderWindow& window) {
	if (!activeItem) return;

	Sprite spriteCopy = activeItem->sprite;

	if (!spriteCopy.getTexture()) return;

	if (is_watching_back) {
		spriteCopy.setPosition(sprite.getPosition() + activeItem->front_offset);
		spriteCopy.setScale(activeItem->left_front_scale);
	}
	else if (is_watching_left) {
		spriteCopy.setPosition(sprite.getPosition() + activeItem->left_offset);
		spriteCopy.setScale(activeItem->left_front_scale);
	}
	else if (is_watching_right) {
		spriteCopy.setPosition(sprite.getPosition() + activeItem->right_offset);
		spriteCopy.setScale(activeItem->right_scale);
	}

	window.draw(spriteCopy);
}

void MainPlayer:: draw_health(RenderWindow& window) {
	window.draw(health_back_bar);
	if (current_health > 0) {
		health_bar.setSize(sf::Vector2f(505.f * (float(current_health) / maxHP), 21.f));
		health_text.setString(std::to_string(current_health));
		window.draw(health_bar);
		window.draw(health_text);
	}
	else if (current_health <= 0) {
		health_text.setString("0");
		isDead = true;
		window.draw(health_text);
	}
}
// drawing other

// update 
void MainPlayer::updateAllEffects(sf::Event* event, bool* isSeen, bool* all_is_locked) { //
	for (auto& eff : active_effects) {
		if (auto* scroll = dynamic_cast<Open_Scroll*>(eff.get())) {
			if (event) scroll->update(*event, *isSeen, *all_is_locked);
		}
		if (auto* heal = dynamic_cast<Heal*>(eff.get())) {
			heal->update(*this);
		}
		if (auto* ice_shield = dynamic_cast<Ice_Shield*>(eff.get())) {
			ice_shield->update(*this);
		}
		if (auto* reflect = dynamic_cast<Reflect_damage*>(eff.get())) {
			reflect->update(*this);
		}
	}
	active_effects.erase(
		std::remove_if(active_effects.begin(), active_effects.end(),
			[](const std::unique_ptr<Ability>& eff) { return eff->shouldRemove; }),
		active_effects.end()
	);
}

void MainPlayer:: updateAllBuffs(std::vector<InventorySlot>& inventory) {
	for (auto it = active_item_buffs.begin(); it != active_item_buffs.end();) {
		auto& buff = *it;
		if (!buff) { it = active_item_buffs.erase(it); continue; }
		buff->update(inventory);
		if (buff->shouldRemove) it = active_item_buffs.erase(it);
		else ++it;
	}
}

void MainPlayer:: update_all_(Font& font) {
	make_inventory();
	setTextsettings(font);
	restore_items_from_copy();

	sprite.setScale(0.48f, 0.48f);
	//sprite.getGlobalBounds().width = 139 * 0.48f;

	StartMenuY = 275;
	isDead = false;
	right_leg = true;
	is_watching_front = false; // стоит к нам спиной
	is_watching_back = true; // стоит к нам лицом
	is_watching_right = false;
	is_watching_left = true;
	moving = false;
	setTextureFrontRight();
}
// update

// items
void MainPlayer:: removeDeletedItems() {
	for (auto& slot : inventory) {
		if (slot.item && slot.item->toDelete) {
			slot.item.reset();
		}
	}
	if (activeItem && activeItem->toDelete) {
		activeItem = nullptr;
	}
}

void MainPlayer:: useActiveItem(const sf::Event& event, bool& all_is_locked, sf::Font& font, std::vector<std::vector<Chunk>>& chunks, Hub_assets& hub)
{
	int cX = sprite.getPosition().x / SIZE;
	int cY = sprite.getPosition().y / SIZE;
	Chunk& current_chunk = hub.getChunk(cX, cY);
	if (activeItem && activeItem->ability) {
		auto* read_book = dynamic_cast<ReadBook*>(activeItem->ability.get());
		if (read_book) {
			if (read_book->isActive) {
				read_book->update_event(event, all_is_locked);
			}
		}
	}
	if (!all_is_locked) {
		if (event.key.code == Keyboard::Space) {
			if (activeItem && activeItem->ability != nullptr) {
				activeItem->init_current_cd(font);
				if (Heal* heal = dynamic_cast<Heal*>(activeItem->ability.get())) {
					auto abilityPtr = std::move(activeItem->ability); // перенос
					abilityPtr->use(*this, all_is_locked);
					active_effects.push_back(std::move(abilityPtr));
					activeItem->toDeleteEnable();
				}
				else if (auto* open_scroll = dynamic_cast<Open_Scroll*>(activeItem->ability.get())) {
					auto abilityPtr = std::move(activeItem->ability); // перенос
					abilityPtr->use(*this, all_is_locked);
					active_effects.push_back(std::move(abilityPtr));
					all_is_locked = true;
					activeItem->toDeleteEnable();
				}
				else if (auto* tp_ability = dynamic_cast<Teleport*>(activeItem->ability.get())) {
					if (activeItem->cooldown_ready) {
						auto abilityPtr = tp_ability->clone();
						abilityPtr->use(*this, all_is_locked);
						if (auto* tp = dynamic_cast<Teleport*>(abilityPtr.get())) {
							Vector2f r_pos = tp->get_result_distance();
							TryToMove(r_pos, chunks, hub);
						}
						active_effects.push_back(std::move(abilityPtr));
						activeItem->cooldown_ready = false;
						activeItem->cooldown_clock.restart();
					}
				}
				else if (auto* read_book = dynamic_cast<ReadBook*>(activeItem->ability.get())) {
					if (!read_book->isActive) {
						read_book->use(*this, all_is_locked);
					}
				}
				else {
					if (activeItem->cooldown_ready) {
						auto abilityPtr = activeItem->ability->clone();
						abilityPtr->use(*this, all_is_locked);
						active_effects.push_back(std::move(abilityPtr));
						activeItem->cooldown_ready = false;
						activeItem->cooldown_clock.restart();
					}
				}
			}
		}
	}
}

void MainPlayer::useActiveItem(const sf::Event& event, bool& all_is_locked, Font& font, vector<RectangleShape>& walls) {
	if (activeItem && activeItem->ability) {
		auto* read_book = dynamic_cast<ReadBook*>(activeItem->ability.get());
		if (read_book) {
			if (read_book->isActive) {
				read_book->update_event(event, all_is_locked);
			}
		}
	}
	if (!all_is_locked) {
		if (event.key.code == Keyboard::Space) {
			if (activeItem && activeItem->ability != nullptr) {
				activeItem->init_current_cd(font);
				if (Heal* heal = dynamic_cast<Heal*>(activeItem->ability.get())) {
					auto abilityPtr = std::move(activeItem->ability); // перенос
					abilityPtr->use(*this, all_is_locked);
					active_effects.push_back(std::move(abilityPtr));
					activeItem->toDeleteEnable();
				}
				else if (auto* open_scroll = dynamic_cast<Open_Scroll*>(activeItem->ability.get())) {
					auto abilityPtr = std::move(activeItem->ability); // перенос
					abilityPtr->use(*this, all_is_locked);
					active_effects.push_back(std::move(abilityPtr));
					all_is_locked = true;
					activeItem->toDeleteEnable();
				}
				else if (auto* tp_ability = dynamic_cast<Teleport*>(activeItem->ability.get())) {
					if (activeItem->cooldown_ready) {
						auto abilityPtr = tp_ability->clone();
						abilityPtr->use(*this, all_is_locked);
						if (auto* tp = dynamic_cast<Teleport*>(abilityPtr.get())) {
							Vector2f r_pos = tp->get_result_distance();
							TryToMove(r_pos, walls);
						}
						active_effects.push_back(std::move(abilityPtr));
						activeItem->cooldown_ready = false;
						activeItem->cooldown_clock.restart();
					}
				}
				else if (auto* read_book = dynamic_cast<ReadBook*>(activeItem->ability.get())) {
					if (!read_book->isActive) {
						read_book->use(*this, all_is_locked);
					}
				}
				else {
					if (activeItem->cooldown_ready) {
						auto abilityPtr = activeItem->ability->clone();
						abilityPtr->use(*this, all_is_locked);
						active_effects.push_back(std::move(abilityPtr));
						activeItem->cooldown_ready = false;
						activeItem->cooldown_clock.restart();
					}
				}
			}
		}
	}
}
// items

// moving
void MainPlayer:: handleMovementInput(bool& all_is_locked, Vector2f& move, float& dt) {
	if (!all_is_locked) {
		if (Keyboard::isKeyPressed(Keyboard::W)) {
			is_watching_front = true;
			is_watching_back = false;
			is_watching_right = false;
			is_watching_left = false;
			moving = true;
			move.y -= 1;
		}
		if (Keyboard::isKeyPressed(Keyboard::S)) {
			is_watching_front = false;
			is_watching_back = true;
			is_watching_right = false;
			is_watching_left = false;
			moving = true;
			move.y += 1;
		}
		if (Keyboard::isKeyPressed(Keyboard::A)) {
			is_watching_front = false;
			is_watching_back = false;
			is_watching_right = false;
			is_watching_left = true;
			moving = true;
			move.x -= 1;
		}
		if (Keyboard::isKeyPressed(Keyboard::D)) {
			is_watching_front = false;
			is_watching_back = false;
			is_watching_right = true;
			is_watching_left = false;
			moving = true;
			move.x += 1;
		}
	}

	float length = std::sqrt(move.x * move.x + move.y * move.y);
	if (length > 0) {
		move /= length;
		move *= speed * dt;
	}

	if (moving) {
		if (animClock.getElapsedTime().asSeconds() > switchTime) {
			if (is_watching_back) {
				if (right_leg) setTextureFrontLeft();
				else setTextureFrontRight();
				right_leg = !right_leg;
				animClock.restart();
			}
			if (is_watching_front) {
				if (right_leg) setTextureBackLeft();
				else setTextureBackRight();
				right_leg = !right_leg;
				animClock.restart();
			}
			if (is_watching_left) {
				if (right_leg) setTextureLeftRight();
				else setTextureLeftLeft();
				right_leg = !right_leg;
				animClock.restart();
			}
			if (is_watching_right) {
				if (right_leg) setTextureRightRight();
				else setTextureRightLeft();
				right_leg = !right_leg;
				animClock.restart();
			}
		}
	}
}

void MainPlayer:: TryToMove(Vector2f& result_position, std::vector<RectangleShape>& walls, vector<sf::RectangleShape>& chest_zones, RectangleShape& boiler_zone) {
	Vector2f old_position = sprite.getPosition();
	sprite.setPosition(result_position);
	Vector2f delta = result_position - old_position;
	Vector2f normalized;

	float length = sqrt(pow(delta.x, 2) + pow(delta.y, 2));
	if (length != 0) {
		normalized.x = delta.x / length;
		normalized.y = delta.y / length;
	}
	for (auto& wall : walls) {
		while (wall.getGlobalBounds().intersects(sprite.getGlobalBounds())) {
			sprite.setPosition(sprite.getPosition() - normalized);
		}
	}
	for (auto& chest_zone : chest_zones) {
		while (chest_zone.getGlobalBounds().intersects(sprite.getGlobalBounds())) {
			sprite.setPosition(sprite.getPosition() - normalized);
		}
	}

	while (boiler_zone.getGlobalBounds().intersects(sprite.getGlobalBounds())) {
		sprite.setPosition(sprite.getPosition() - normalized);
	}
}

void MainPlayer:: TryToMove(Vector2f& result_position, std::vector<RectangleShape>& walls) {
	Vector2f old_position = sprite.getPosition();
	sprite.setPosition(result_position);
	Vector2f delta = result_position - old_position;
	Vector2f normalized;

	float length = sqrt(pow(delta.x, 2) + pow(delta.y, 2));
	if (length != 0) {
		normalized.x = delta.x / length;
		normalized.y = delta.y / length;
	}

	for (auto& wall : walls) {
		while (wall.getGlobalBounds().intersects(sprite.getGlobalBounds())) {
			sprite.setPosition(sprite.getPosition() - normalized);
		}
	}
}

void MainPlayer::TryToMove(Vector2f& result_position, vector<vector<Chunk>>& chunks, Hub_assets& hub)
{
	Vector2f old_position = sprite.getPosition();
	Vector2f delta = result_position - old_position;

	float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
	if (length == 0) return;

	Vector2f dir = delta / length; 

	sprite.setPosition(result_position);

	int cx = result_position.x / SIZE;
	int cy = result_position.y / SIZE;

	if (cx < 0 || cx >= chunks.size()) return;
	if (cy < 0 || cy >= chunks[cx].size()) return;

	Chunk& cur_chunk = hub.getChunk(cx, cy);

	auto checkCollision = [&](Vector2f pos)
	{
		sprite.setPosition(pos);
		sf::FloatRect bounds = sprite.getGlobalBounds();

		std::vector<Vector2f> points = {
			{bounds.left, bounds.top},
			{bounds.left + bounds.width / 2, bounds.top},
			{bounds.left + bounds.width, bounds.top},

			{bounds.left, bounds.top + bounds.height / 2},
			{bounds.left + bounds.width, bounds.top + bounds.height / 2},

			{bounds.left, bounds.top + bounds.height},
			{bounds.left + bounds.width / 2, bounds.top + bounds.height},
			{bounds.left + bounds.width, bounds.top + bounds.height}
		};

		for (auto& point : points)
		{
			int lx = int(point.x - cur_chunk.X);
			int ly = int(point.y - cur_chunk.Y);

			if (lx < 0 || ly < 0) continue;
			if (lx >= cur_chunk.chunk_mask.getSize().x) continue;
			if (ly >= cur_chunk.chunk_mask.getSize().y) continue;

			sf::Color px = cur_chunk.chunk_mask.getPixel(lx, ly);
			if (px.r <= 128)
				return true; 

			for (auto& wall : cur_chunk.collisions) {
				while (wall.getGlobalBounds().intersects(sprite.getGlobalBounds())) {
					return true;
				}
			}
		}
		return false;
	};

	if (!checkCollision(result_position))
		return;

	const int maxSteps = 64;  
	for (int i = 0; i < maxSteps; i++)

	{
		result_position -= dir; 
		if (!checkCollision(result_position))
		{
			sprite.setPosition(result_position);
			return;
		}
	}
	sprite.setPosition(old_position);
}
// moving

// drag & drop
void MainPlayer:: drag_item(RenderWindow& window, Vector2i& mouse_pos, bool& inventoryClicked, bool& all_is_locked, vector<std::reference_wrapper<Chest>>& chests, unique_ptr<Armor>& temp_armor,
	unique_ptr<Item>& temp_holder, Vector2f& back_pos, InventorySlot*& oldSlot, ArmorSlot*& oldArmor) {
	for (auto& chest_reference : chests) {
		auto& chest = chest_reference.get();
		if (chest.isOpened) {
			for (auto& chest_armor_slot : chest.armor) {
				if (chest_armor_slot.armor && chest_armor_slot.armor->sprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mouse_pos))) {
					oldArmor = &chest_armor_slot;
					back_pos = chest_armor_slot.armor->sprite.getPosition();
					temp_armor = std::move(chest_armor_slot.armor);
					temp_armor->isDraggin = true;
					temp_armor->offset = temp_armor->sprite.getPosition() - window.mapPixelToCoords(sf::Mouse::getPosition(window));
					break;
				}
			}
			for (auto& chest_slot : chest.items) {
				if (chest_slot.item && chest_slot.item->sprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mouse_pos))) {
					oldSlot = &chest_slot;
					back_pos = chest_slot.item->sprite.getPosition();
					temp_holder = std::move(chest_slot.item);
					temp_holder->isDraggin = true;
					temp_holder->offset = temp_holder->sprite.getPosition() - window.mapPixelToCoords(sf::Mouse::getPosition(window));
					break;
				}
			}
			if (chest.exit_button_sprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mouse_pos))) {
				chest.close_chest(all_is_locked);
			}
		}
	}

	for (auto& slot : inventory) {
		if (slot.hitbox.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) { // проверка что нажали на инвентарь
			inventoryClicked = true;
			break;
		}
	}

	for (auto& slot : inventory) { // инвентарь игрока
		if (slot.item && slot.item->sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
			oldSlot = &slot;
			temp_holder = std::move(slot.item);
			back_pos = temp_holder->sprite.getPosition();
			temp_holder->isDraggin = true;
			temp_holder->offset = temp_holder->sprite.getPosition() - window.mapPixelToCoords(sf::Mouse::getPosition(window));
			break;
		}
	}

	if (menu_is_active) {
		for (size_t raw = 0; raw < menu_inventory.size(); raw++) {
			for (size_t column = 0; column < menu_inventory[raw].size(); column++) {
				if (menu_inventory[raw][column].item && menu_inventory[raw][column].item->sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
					oldSlot = &menu_inventory[raw][column];
					temp_holder = std::move(menu_inventory[raw][column].item);
					back_pos = temp_holder->sprite.getPosition();
					temp_holder->isDraggin = true;
					temp_holder->offset = temp_holder->sprite.getPosition() - window.mapPixelToCoords(sf::Mouse::getPosition(window));
					break;
				}
			}
		}
	}


	if (armor_slot.armor && armor_slot.armor->sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) { // слот брони
		oldArmor = &armor_slot;
		temp_armor = std::move(armor_slot.armor);
		back_pos = temp_armor->sprite.getPosition();
		temp_armor->isDraggin = true;
		temp_armor->offset = temp_armor->sprite.getPosition() - window.mapPixelToCoords(sf::Mouse::getPosition(window));
	}

}

void MainPlayer:: drag_item(RenderWindow& window, Vector2f& mouse_pos_UI, bool& inventoryClicked, bool& all_is_locked, vector<std::reference_wrapper<Chest>>& chests, unique_ptr<Armor>& temp_armor,
	unique_ptr<Item>& temp_holder, Vector2f& back_pos, InventorySlot*& oldSlot, ArmorSlot*& oldArmor) {
	for (auto& chest_reference : chests) {
		auto& chest = chest_reference.get();
		if (chest.isOpened) {
			for (auto& chest_armor_slot : chest.armor) {
				if (chest_armor_slot.armor && chest_armor_slot.armor->sprite.getGlobalBounds().contains(mouse_pos_UI)) {
					oldArmor = &chest_armor_slot;
					back_pos = chest_armor_slot.armor->sprite.getPosition();
					temp_armor = std::move(chest_armor_slot.armor);
					temp_armor->isDraggin = true;
					temp_armor->offset = temp_armor->sprite.getPosition() - window.mapPixelToCoords(sf::Mouse::getPosition(window));
					break;
				}
			}
			for (auto& chest_slot : chest.items) {
				if (chest_slot.item && chest_slot.item->sprite.getGlobalBounds().contains(mouse_pos_UI)) {
					oldSlot = &chest_slot;
					back_pos = chest_slot.item->sprite.getPosition();
					temp_holder = std::move(chest_slot.item);
					temp_holder->isDraggin = true;
					temp_holder->offset = temp_holder->sprite.getPosition() - window.mapPixelToCoords(sf::Mouse::getPosition(window));
					break;
				}
			}
			if (chest.exit_button_sprite.getGlobalBounds().contains(mouse_pos_UI)) {
				chest.close_chest(all_is_locked);
			}
		}
	}

	for (auto& slot : inventory) {
		if (slot.hitbox.getGlobalBounds().contains(mouse_pos_UI)) { // проверка что нажали на инвентарь
			inventoryClicked = true;
			break;
		}
	}

	for (auto& slot : inventory) { // инвентарь игрока
		if (slot.item && slot.item->sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
			oldSlot = &slot;
			temp_holder = std::move(slot.item);
			back_pos = temp_holder->sprite.getPosition();
			temp_holder->isDraggin = true;
			temp_holder->offset = temp_holder->sprite.getPosition() - window.mapPixelToCoords(sf::Mouse::getPosition(window));
			break;
		}
	}

	if (menu_is_active) {
		for (size_t raw = 0; raw < menu_inventory.size(); raw++) {
			for (size_t column = 0; column < menu_inventory[raw].size(); column++) {
				if (menu_inventory[raw][column].item && menu_inventory[raw][column].item->sprite.getGlobalBounds().contains(mouse_pos_UI)) {
					oldSlot = &menu_inventory[raw][column];
					temp_holder = std::move(menu_inventory[raw][column].item);
					back_pos = temp_holder->sprite.getPosition();
					temp_holder->isDraggin = true;
					temp_holder->offset = temp_holder->sprite.getPosition() - window.mapPixelToCoords(sf::Mouse::getPosition(window));
					break;
				}
			}
		}
	}

	if (armor_slot.armor && armor_slot.armor->sprite.getGlobalBounds().contains(mouse_pos_UI)) { // слот брони
		oldArmor = &armor_slot;
		temp_armor = std::move(armor_slot.armor);
		back_pos = temp_armor->sprite.getPosition();
		temp_armor->isDraggin = true;
		temp_armor->offset = temp_armor->sprite.getPosition() - window.mapPixelToCoords(sf::Mouse::getPosition(window));
	}
}


void MainPlayer:: drop_item(const Event& event, vector<std::reference_wrapper<Chest>>& chests, unique_ptr<Armor>& temp_armor, unique_ptr<Item>& temp_holder, Vector2f& back_pos, InventorySlot*& oldSlot, ArmorSlot*& oldArmor) {
	if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
		// armor
		if (temp_armor) {
			bool placed = false;
			for (auto& chest_z : chests) {
				auto& chest = chest_z.get();
				if (chest.isOpened) {
					for (auto& chest_slot : chest.armor) {
						if (chest_slot.hitbox.getGlobalBounds().intersects(temp_armor->sprite.getGlobalBounds())) {
							temp_armor->isDraggin = false;
							if (chest_slot.armor) { // слот занят
								std::swap(chest_slot.armor, temp_armor);
								chest_slot.armor->sprite.setPosition(chest_slot.hitbox.getPosition() + sf::Vector2f(10.f, 10.f));
								chest_slot.armor->sprite.setScale(1.f, 1.f);
								if (oldArmor && temp_armor) {
									oldArmor->armor = std::move(temp_armor);
									oldArmor->armor->sprite.setPosition(back_pos);
									oldArmor->armor->sprite.setScale(1.f, 1.f);
								}
							}
							else { // слот пуст
								chest_slot.armor = std::move(temp_armor);
								chest_slot.armor->sprite.setPosition(chest_slot.hitbox.getPosition() + sf::Vector2f(10.f, 10.f));
								chest_slot.armor->sprite.setScale(1.f, 1.f);
							}
							placed = true;
							break;
						}
					}
					if (placed) break;
				}
			}
			if (!placed) {
				if (armor_slot.hitbox.getGlobalBounds().intersects(temp_armor->sprite.getGlobalBounds())) {
					temp_armor->isDraggin = false;
					if (armor_slot.armor) { // слот занят
						std::swap(armor_slot.armor, temp_armor);
						armor_slot.armor->sprite.setPosition(armor_slot.hitbox.getPosition() + Vector2f(6.f, 10.f));
						armor_slot.armor->sprite.setScale(1.212f, 1.275f);

						if (oldArmor && temp_armor) {
							oldArmor->armor = std::move(temp_armor);
							oldArmor->armor->sprite.setPosition(back_pos);
							oldArmor->armor->sprite.setScale(1.f, 1.f);
						}
					}
					else { // слот пуст
						armor_slot.armor = std::move(temp_armor);
						armor_slot.armor->sprite.setPosition(armor_slot.hitbox.getPosition() + sf::Vector2f(6.f, 10.f));
						armor_slot.armor->sprite.setScale(1.212f, 1.275f);
					}
					placed = true;
					/*break;*/
				}
			}
			if (!placed && oldArmor) {
				oldArmor->armor = std::move(temp_armor);
				oldArmor->armor->sprite.setPosition(back_pos);
				oldArmor->armor->isDraggin = false;
			}
			oldArmor = nullptr;
		}
		// armor

		// items
		if (temp_holder) {
			bool placed = false;

			InventorySlot* savedOldSlot = oldSlot; // чтобы не крашнуть и работать с этой ячейкой (на всякий)
			sf::Vector2f savedBackPos = back_pos; // чтобы не крашнуть

			for (auto& chest_ref : chests) {
				auto& chest = chest_ref.get();
				if (chest.isOpened) {
					for (auto& chest_slot : chest.items) {
						if (!temp_holder) break;
						if (chest_slot.hitbox.getGlobalBounds().intersects(temp_holder->sprite.getGlobalBounds())) {
							temp_holder->isDraggin = false;
							if (chest_slot.item) { // слот занят
								std::swap(chest_slot.item, temp_holder);
								if (chest_slot.item) chest_slot.item->sprite.setPosition(chest_slot.hitbox.getPosition() + sf::Vector2f(10.f, 10.f));

								if (savedOldSlot && temp_holder) {
									savedOldSlot->item = std::move(temp_holder);
									if (savedOldSlot->item) savedOldSlot->item->sprite.setPosition(savedBackPos);
								}
							}
							else { // слот пуст
								chest_slot.item = std::move(temp_holder);
								if (chest_slot.item) chest_slot.item->sprite.setPosition(chest_slot.hitbox.getPosition() + sf::Vector2f(10.f, 10.f));
							}

							placed = true;
							break;
						}
					}
				}
			}

			if (!placed) {
				if (menu_is_active) {
					for (size_t raw = 0; raw < menu_inventory.size() && !placed; ++raw) {
						for (size_t column = 0; column < menu_inventory[raw].size() && !placed; ++column) {
							if (!temp_holder) break;
							InventorySlot& target = menu_inventory[raw][column];
							if (target.hitbox.getGlobalBounds().intersects(temp_holder->sprite.getGlobalBounds())) {
								temp_holder->isDraggin = false;

								if (target.item) {
									std::swap(target.item, temp_holder);
									if (target.item) target.item->sprite.setPosition(target.hitbox.getPosition() + sf::Vector2f(10.f, 10.f));

									if (savedOldSlot && temp_holder) {
										savedOldSlot->item = std::move(temp_holder);
										if (savedOldSlot->item) savedOldSlot->item->sprite.setPosition(savedBackPos);
									}
								}
								else {
									target.item = std::move(temp_holder);
									if (target.item) target.item->sprite.setPosition(target.hitbox.getPosition() + sf::Vector2f(10.f, 10.f));
								}

								placed = true;
								break;
							}
						}
					}
				}
			}

			if (!placed) {
				for (auto& slot : inventory) {
					if (!temp_holder) break;
					if (slot.hitbox.getGlobalBounds().intersects(temp_holder->sprite.getGlobalBounds())) {
						temp_holder->isDraggin = false;

						if (slot.item) {
							std::swap(slot.item, temp_holder);
							if (slot.item) slot.item->sprite.setPosition(slot.hitbox.getPosition() + sf::Vector2f(10.f, 10.f));

							if (savedOldSlot && temp_holder) {
								savedOldSlot->item = std::move(temp_holder);
								if (savedOldSlot->item) savedOldSlot->item->sprite.setPosition(savedBackPos);
							}
						}
						else {
							slot.item = std::move(temp_holder);
							if (slot.item) slot.item->sprite.setPosition(slot.hitbox.getPosition() + sf::Vector2f(10.f, 10.f));
						}

						placed = true;
						break;
					}
				}
			}

			if (!placed && savedOldSlot) {
				savedOldSlot->item = std::move(temp_holder);
				if (savedOldSlot->item) {
					savedOldSlot->item->sprite.setPosition(savedBackPos);
					savedOldSlot->item->isDraggin = false;
				}
			}
			oldSlot = nullptr;
		}
	}
}
// drag & drop