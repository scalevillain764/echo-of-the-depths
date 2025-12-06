#include "SFML/Graphics.hpp"
#include <vector>
#include <iostream>
#include <filesystem>
#include "SwordEffect.h"
#include "Evil.h"
#include "MainPlayer.h"
#include "IneventoryTypes.h"
#include "item_descriptions.h"
#include "buttons.h"
#include "Pause.h"
#include "try_again.h"
#include "chest.h"
#include "Item_manager.h"
#include <tuple>
#include "little_box.h"
#include "Item_Buffs.h"
#include <algorithm>
#include "Messages.h"
#include "Chunk.h"
#include "Hub_assets.h"
#include "enums.h"
#include "worlds.h"

using std::vector;
using sf::RenderWindow;
using sf::VideoMode;
using sf::Event;
using sf::Texture;
using sf::Sprite;
using sf::Keyboard;
using sf::Vector2f;
using sf::Vector2i;
using sf::Mouse;
using std::tuple;

const Vector2f one_line = { 10.f, 1045.f };
const Vector2f two_line = { 5.f, 1020.f };

const Vector2f PORTAL_RIGHT_SCALE = { -0.62f, 0.62f };
const Vector2f PORTAL_LEFT_SCALE = { 0.62f, 0.62f };

void goToNextLevel(Locations& loc) {
	int next = static_cast<int>(loc) + 1;
	if (next < static_cast<int>(Locations::count)) {
		loc = static_cast<Locations>(next);
	}
}

class Portal {
public :
	// portal
	bool portal_should_activate = false;
	Sprite portal_sprite;
	sf::Texture portal_texts[9];
	int currentFrame = 0;
	sf::Clock portalClock;
	sf::Text enter;
	// portal

	bool load() {
		if (!portal_texts[0].loadFromFile("textures/portal/1.png") ||
			!portal_texts[1].loadFromFile("textures/portal/2.png") ||
			!portal_texts[2].loadFromFile("textures/portal/3.png") ||
			!portal_texts[3].loadFromFile("textures/portal/4.png") ||
			!portal_texts[4].loadFromFile("textures/portal/5.png") ||
			!portal_texts[5].loadFromFile("textures/portal/6.png") ||
			!portal_texts[6].loadFromFile("textures/portal/7.png") ||
			!portal_texts[7].loadFromFile("textures/portal/8.png") ||
			!portal_texts[8].loadFromFile("textures/portal/9.png")) {
			return false;
		}
		return true;
	}

	void make_portal(Font& font, Vector2f pos, Vector2f portal_scale) {
		enter.setFont(font);
		enter.setCharacterSize(25); 
		enter.setFillColor(sf::Color::White);
		enter.setString("Press [ENTER] to enter the portal");
		portal_sprite.setPosition(pos);
		portal_sprite.setScale(portal_scale);
	}

	void draw(MainPlayer &player, RenderWindow &window) {
		enter.setPosition(player.sprite.getPosition().x - 105.f, player.sprite.getPosition().y - 45.f);
		if (portalClock.getElapsedTime().asMilliseconds() > 100) {
			currentFrame = (currentFrame + 1) % 9;
			portal_sprite.setTexture(portal_texts[currentFrame]);
			portalClock.restart();
		}
		window.draw(portal_sprite);
		if (player.sprite.getGlobalBounds().intersects(portal_sprite.getGlobalBounds())) {
			window.draw(enter);
		}
	}

};

class SecondLocationAssets {
public:
	bool LevelCompleted = false;
	bool got_seal = false;
	
	// end of level rectangle fading
	RectangleShape last_rect;
	int last_rect_alpha = 0;
	bool last_rect_active = false;
	Clock last_rect_clock;
	enum FS {Non, FaIn, Wait, FaOut};
	// end of level rectangle fading

	Texture bgTex, unitTex;
	Sprite bgSprite;
	
	// statue
	Texture statue_texture, statue_texture2;
	Sprite statue_sprite;
	Texture statue_texts_activated[5];
	int current_st_frame = 0;
	bool statue_forward = true;
	bool statue_active = false;
	Clock statue_clock;

	Texture bg_texts_activated[5];
	// statue

	Chest chest_one, chest_two, stone_chest; // chests

	// golems
	Stone_Golem golem1;
	Stone_Golem golem2;
	bool golemFight = false;
	// golems

	bool statue_got_leaf = false;

	vector<sf::RectangleShape> chest_zones;
	sf::RectangleShape chest_one_zone;
	sf::RectangleShape chest_two_zone;
	sf::RectangleShape stone_chest_zone;
	
	// fade rect
	enum FadeRectState { None, FadingIn, Waiting, FadingOut };
	RectangleShape fade_rect;
	int fade_rect_alpha = 0;
	Clock fade_rect_clock;
	bool fade_rect_active = false;
	bool fade_rect_used = false;
	// fade rect

	// torches
	bool torchSequenceActive = false;
	bool isTorchScrollSeen = false;
	enum TorchState { FadeIN, Hold, FadeOUT };

	struct Torch {
		sf::RectangleShape zone;
		bool activated = false;
		bool shouldDraw = false;
		int torch_alpha = 0;
		sf::Clock torch_aplha_clock;
		sf::Clock torch_live_clock;

		TorchState ts = FadeIN;
	};

	unordered_map<int, tuple<Vector2f, Torch, Texture, Sprite>> torch_zones; // зоны активации факелов
	vector<int> torchOrder = { 1, 4, 3, 5, 2 };

	Texture torchline_one;
	Texture torchline_two;
	Texture torchline_three;
	Texture torchline_four;
	Texture torchline_five;

	Sprite torch_line_sprite_one;
	Sprite torch_line_sprite_two;
	Sprite torch_line_sprite_three;
	Sprite torch_line_sprite_four;
	Sprite torch_line_sprite_five;

	Torch torch1;
	Torch torch2;
	Torch torch3;
	Torch torch4;
	Torch torch5; 
	 
	int currentTorch = 0;
	int currentTorchIndex = 0;

	int torch_counter = 0;
	// torches

	vector<sf::RectangleShape> walls;
	vector<sf::RectangleShape> walls_for_evil;
	vector<std::reference_wrapper<Chest>> chests;

	Text OpenChest;

	Text pressE;
	Text pressG; // to activate torch
	bool show_pressE_ONE = false;
	bool show_pressE_TWO = false;
	bool show_pressE_THREE = false;

	// tips	
	Tip find_scroll;
	Tip talk_to_statue;
	Tip statue_text1;
	Tip statue_text2;
	Tip statue_text3;

		// hint #1
		bool find_scroll_activated = false;
		// hint #1

		// hint #2
		bool alpha_changed = false;
		bool alpha_changed2 = false;
		// hint #2

		// hint #3
		bool statue_text_2_activated = false;
		// hint #3

		// hint #4
		bool statue_text3_activated = false;
		// hint #4
	// tips

	bool all_is_locked = false;
	bool isInChestZone = false;
	bool torch_activated = false; // все факелы активны?
	bool got_fern = false;
	bool statue_got_stone = false;

	// boiler
	RectangleShape boiler_zone;
	Sprite boiler_sprite;
	Texture boiler_textures[5];
	Clock boiler_timer;
	int current_boiler_frame = 0;
	bool forward = true;
	bool boiler_active = false; // to change boiler frame
	bool boiler_full = false;
	bool boiler_alpha_schange = true;
	Error boiler_error;
	// boiler(котел)

	// bloody screen
	Texture bloody_screen_texture;
	Sprite bloody_screen_sprite;
	int bs_alpha = 255;
	int bs_active = false;
	Clock bs_clock;
	// bloody screen

	SecondLocationAssets(Font &font) : golem1(30, 80, 38.f, 2.5f), golem2(30, 80, 38.f, 2.5f),
		find_scroll("find_scroll", "Tip: Find scroll in chests", font, 24, Color::White, one_line, 255),
		statue_text1("statue_text1", "Statue: Who walks the path of light\nwill find the ancient greenery.", font, 24, Color::White, two_line, 255),
		statue_text2("statue_text2", "Statue: REMEMBER, so that the BLOOD\ndoes not blind the SPROUT", font, 24, Color::White, two_line, 255),
		statue_text3("statue_text3", "Statue: Get a reward for saving.", font, 24, Color::White, one_line, 255),
		talk_to_statue("talk_to_statue", "Tip: Talk to statue", font, 24, Color::White, one_line, 255),
		boiler_error("boiler_error", "Error : You need at least 35% of health.", font, 24, Color::Red, one_line, 255) {};

	bool load(Font& font) {
		if (!bgTex.loadFromFile("textures/chapone/locationtwo/etc/map_texts/second_location.png") ||
			!statue_texture.loadFromFile("textures/chapone/locationtwo/etc/statue.png") || !chest_one.load("textures/chapone/locationtwo/chests/chestone.png") ||
			!chest_two.load("textures/chapone/locationtwo/chests/chesttwo.png") || !stone_chest.load("textures/chapone/locationtwo/chests/stone_chest.png") || 
			!torchline_one.loadFromFile("textures/chapone/locationtwo/etc/Новая папка/one.png") || !torchline_two.loadFromFile("textures/chapone/locationtwo/etc/Новая папка/two.png") || 
			!torchline_three.loadFromFile("textures/chapone/locationtwo/etc/Новая папка/three.png") || !torchline_four.loadFromFile("textures/chapone/locationtwo/etc/Новая папка/four.png")
			|| !golem1.loadTexture(font) 
			|| !golem2.loadTexture(font)
			|| !statue_texture2.loadFromFile("textures/chapone/locationtwo/etc/statue2.png") || 
			!boiler_textures[0].loadFromFile("textures/chapone/locationtwo/etc/boiler/empty boiler.png") || 
			!boiler_textures[1].loadFromFile("textures/chapone/locationtwo/etc/boiler/blood boiler1.png") || 
			!boiler_textures[2].loadFromFile("textures/chapone/locationtwo/etc/boiler/full boiler1.png") || 
			!boiler_textures[3].loadFromFile("textures/chapone/locationtwo/etc/boiler/full boiler2.png") ||
			!boiler_textures[4].loadFromFile("textures/chapone/locationtwo/etc/boiler/full boiler3.png") ||
			!bloody_screen_texture.loadFromFile("textures/chapone/locationtwo/etc/boiler/bloody_screen.png") ||
			!statue_texts_activated[0].loadFromFile("textures/chapone/locationtwo/etc/statue_full_activated5.png") || 
			!statue_texts_activated[1].loadFromFile("textures/chapone/locationtwo/etc/statue_full_activated4.png") ||
			!statue_texts_activated[2].loadFromFile("textures/chapone/locationtwo/etc/statue_full_activated3.png") || 
			!statue_texts_activated[3].loadFromFile("textures/chapone/locationtwo/etc/statue_full_activated2.png") ||
			!statue_texts_activated[4].loadFromFile("textures/chapone/locationtwo/etc/statue_full_activated.png") ||
			!bg_texts_activated[0].loadFromFile("textures/chapone/locationtwo/etc/map_texts/second_location_activated5.png") || 
			!bg_texts_activated[1].loadFromFile("textures/chapone/locationtwo/etc/map_texts/second_location_activated4.png") || 
			!bg_texts_activated[2].loadFromFile("textures/chapone/locationtwo/etc/map_texts/second_location_activated3.png") ||
			!bg_texts_activated[3].loadFromFile("textures/chapone/locationtwo/etc/map_texts/second_location_activated2.png") || 
			!bg_texts_activated[4].loadFromFile("textures/chapone/locationtwo/etc/map_texts/second_location_activated.png")) {
			// 0 - empty boiler
			// 1 half of boiler
			// 2 full_one
			// 3 full_two
			// 3 full_three
			return false;
		}
		return true;
	}

	void update() {
		bgSprite.setTexture(bgTex); bgSprite.setPosition(0.f, 0.f);
		statue_sprite.setTexture(statue_texture); statue_sprite.setPosition(853.f, 54.f);
		chest_one.setTexture(); chest_one.sprite.setPosition(328.f, 169.f); chests.push_back(chest_one);
		chest_two.setTexture(); chest_two.sprite.setPosition(1450.f, 107.f); chests.push_back(chest_two);
		stone_chest.setTexture(); stone_chest.sprite.setPosition(1431.f, 714.f); chests.push_back(stone_chest);

		last_rect.setPosition(0.f, 0.f);
		last_rect.setSize(Vector2f(1920.f, 1080.f));

		find_scroll.emitTip();

		fade_rect.setPosition(0.f, 0.f);
		fade_rect.setSize(Vector2f(1920.f, 1080.f));

		bloody_screen_sprite.setTexture(bloody_screen_texture); 

		boiler_sprite.setPosition(1121.f, 207.f);
		boiler_sprite.setTexture(boiler_textures[0]);

		boiler_zone.setSize(sf::Vector2f(67.f, 15.f));
		boiler_zone.setPosition(boiler_sprite.getPosition());

		pressG.setString("Press [G] to activate");
		pressG.setCharacterSize(24);
		pressG.setFillColor(Color::White); 

		torch1.zone.setPosition(683.f, 398.f);
		torch2.zone.setPosition(1030.f, 474.f);
		torch3.zone.setPosition(1248.f, 214.f);
		torch4.zone.setPosition(1331.f, 570.f);
		torch5.zone.setPosition(1049.f, 774.f); 

		torch_zones.emplace(1, std::make_tuple(Vector2f(667.f, 462.f), torch1, torchline_one, torch_line_sprite_one));
		torch_zones.emplace(4, std::make_tuple(Vector2f(1239.f, 271.f), torch4, torchline_two, torch_line_sprite_two));
		torch_zones.emplace(3, std::make_tuple(Vector2f(1057.f, 292.f), torch3, torchline_three, torch_line_sprite_three));
		torch_zones.emplace(5, std::make_tuple(Vector2f(938.f, 546.f), torch5, torchline_four, torch_line_sprite_four));
		torch_zones.emplace(2, std::make_tuple(Vector2f(0.f, 0.f), torch2, torchline_five, torch_line_sprite_five));

		for (auto& torch_zone : torch_zones) {
			std::get<1>(torch_zone.second).zone.setSize(Vector2f(26.f, 89.f));
		}

		pressE.setString("Press [E] to talk to statue");
		pressE.setPosition(810.f, 25.f);
		pressE.setCharacterSize(24);
		pressE.setFillColor(Color::White);

		chest_one_zone.setPosition(308.f, 200.f);
		chest_one_zone.setSize(Vector2f(151.f, 110.f));
		chest_zones.push_back(chest_one_zone);

		chest_two_zone.setPosition(1443.f, 96.f);
		chest_two_zone.setSize(Vector2f(127.f, 89.f));
		chest_zones.push_back(chest_two_zone);

		stone_chest_zone.setPosition(1427.f, 698.f);
		stone_chest_zone.setSize(Vector2f(155.f, 114.f));
		chest_zones.push_back(stone_chest_zone);

		auto addWall = [this](float x, float y, float w, float h) {
			sf::RectangleShape wall(sf::Vector2f(w, h));
			wall.setPosition(x, y);
			wall.setFillColor(sf::Color(255, 255, 255, 255));
			walls.push_back(wall);
		}; 

		auto addWall2 = [this](float x, float y, float w, float h) {
			sf::RectangleShape wall(sf::Vector2f(w, h));
			wall.setPosition(x, y);
			wall.setFillColor(sf::Color(255, 255, 255, 255));
			walls_for_evil.push_back(wall);
		};

		// usual walls for player
		addWall(218, 868, 176, 28); // 1
		addWall(52, 471, 170, 425); // 2
		addWall(199, 473, 248, 35); // 3
		addWall(146, 111, 155, 360); // 4
		addWall(291, 125, 254, 21); // 5
		addWall(437, 0, 110, 135); // 6
		addWall(540, 0, 1145, 16); // 7
		addWall(1665, 0, 115, 309); // 8
		addWall(1459, 307, 228, 28); // 9
		addWall(1665, 326, 115, 567); // 10
		addWall(532, 872, 1153, 34); // 11
		addWall(702, 752, 33, 124); // 12
		addWall(394, 902, 138, 26); // 13

		// statue
		addWall(856, 210, 101, 1); // 14
		// statue

		addWall(543, 341, 207, 30); // 15
		addWall(813, 508, 91, 37); // 16
		addWall(979, 420, 148, 55); // 17
		addWall(1087, 166, 219, 35); // 18
		addWall(1070, 166, 21, 90); // 19
		addWall(1301, 511, 153, 35); // 20
		addWall(999, 743, 184, 20); // 21
		// usual walls for player

		// walls for evil
		addWall2(218, 868, 176, 28); // 1
		addWall2(52, 471, 170, 425); // 2
		addWall2(199, 473, 248, 35); // 3
		addWall2(110, 111, 155, 360); // 4
		addWall2(291, 125, 254, 21); // 5
		addWall2(437, 0, 110, 135); // 6
		addWall2(540, 0, 1145, 16); // 7
		addWall2(1665, 0, 115, 309); // 8
		addWall2(1459, 307, 228, 28); // 9
		addWall2(1665, 326, 115, 567); // 10
		addWall2(532, 872, 1153, 34); // 11
		addWall2(702, 752, 33, 124); // 12
		addWall2(394, 902, 138, 26); // 13

		//// statue
		//addWall(856, 210, 101, 1); // 14
		//// statue

		addWall2(543, 341, 207, 13); // 15
		addWall2(813, 508, 91, 37); // 16
		addWall2(979, 420, 148, 55); // 17
		addWall2(1087, 166, 219, 35); // 18
		addWall2(1070, 166, 21, 90); // 19
		addWall2(1301, 511, 153, 35); // 20
		addWall2(999, 743, 184, 20); // 21
		// walls for evil
	}

	void apply_font(Font& font) {
		pressE.setFont(font);
		pressG.setFont(font);
	}

};

class FirstLocationAssets {
public:
	Texture mapTex, tableTex, bgTex;
	Sprite mapSprite, tableSprite, bgSprite;
	vector<unique_ptr<Item>> skulls;
	vector<sf::RectangleShape> skull_zones;
	vector<sf::RectangleShape> walls;
	sf::RectangleShape background_forMAP;

	vector<std::reference_wrapper<Chest>> chests;

	//END OF THE LEVEL
	bool draw_Loading = false;
	sf::RectangleShape fade_rect;
	sf::Clock fade_clock;
	/*bool IS_LEVEL_COMPLETED = false;*/
	int last_alpha = 0;
	bool all_is_locked = false;
	Text esc;
	// END OF THE LEVEL

	Sprite penta_sprite;
	Texture penta_texture;

	// bowl
	Texture bowl_texture, bowl_oneskull, bowl_twoskulls, bowl_threeskulls;
	Sprite bowl;
	int bowl_skull_counter = 0;
	bool isInBowlZone = false;
	sf::Text pressG;
	// bowl

	bool isInTableZone = false;
	sf::RectangleShape table_zone;

	// for skulls
	sf::Text skulls_counter_text;
	int skull_counter = 0;
	bool isInSkullZone = false;
	sf::Text E;
	// for skulls

	// Tips
	Tip find_table;
	Tip enter_the_portal;
	// Tips

	sf::Text map_task; // задание под картой
	sf::Text pressF; // открыть карту
	bool draw_skulls = false;
	bool showingmap = false;

	bool LevelCompleted = false;

	FirstLocationAssets(Font& font) : find_table("find_table", "Tip: Find table to open the map", font, 24, Color::White, one_line, 255),  
		enter_the_portal("enter_the_portal", "Tip: Enter the portal", font, 24, Color::White, one_line, 255) {}

	bool load() { 
		if (!mapTex.loadFromFile("textures/chapone/etc/map_.png") ||
			!tableTex.loadFromFile("textures/chapone/walls/table1.png") ||
			!bgTex.loadFromFile("textures/chapone/walls/first_location_one.png") ||
			!bowl_texture.loadFromFile("textures/chapone/bowl/bowl.png") ||
			!bowl_oneskull.loadFromFile("textures/chapone/bowl/bowl1.png") ||
			!bowl_twoskulls.loadFromFile("textures/chapone/bowl/bowl2.png") ||
			!bowl_threeskulls.loadFromFile("textures/chapone/bowl/bowl3.png") ||
			!penta_texture.loadFromFile("textures/chapone/bowl/pentagramm.png")) 
			return false;
		return true;
	}

	void makeSkulls(ItemManager& item_manager) {
		unique_ptr<Item> skull_one = item_manager.items[9]->clone();
		unique_ptr<Item> skull_two = item_manager.items[9]->clone();
		unique_ptr<Item> skull_three = item_manager.items[9]->clone();
		//skull
		skulls.resize(3);
		skulls[0] = std::move(skull_one);
		skulls[1] = std::move(skull_two);
		skulls[2] = std::move(skull_three);

		skulls[0]->sprite.setPosition(1367, 194);
		skulls[1]->sprite.setPosition(617, 609);
		skulls[2]->sprite.setPosition(1071, 967);

		Vector2f skull_zone_size(31.f, 31.f);
		skull_zones.resize(3);
		for (auto& skull_zone : skull_zones) {
			skull_zone.setFillColor(sf::Color(255, 255, 255, 127));
			skull_zone.setSize(skull_zone_size);
		}
		skull_zones[0].setPosition(1364.f, 192.f);
		skull_zones[1].setPosition(614.f, 606.f);
		skull_zones[2].setPosition(1068.f, 967.f);
		//skulls
	}

	void update() {
		mapSprite.setTexture(mapTex); mapSprite.setPosition(600, 27); // карта
		tableSprite.setTexture(tableTex); tableSprite.setPosition(998, 850); // стол
		bgSprite.setTexture(bgTex); bgSprite.setPosition(0.f, 0.f);
		bowl.setTexture(bowl_texture); bowl.setPosition(1251, 853);
		penta_sprite.setTexture(penta_texture); penta_sprite.setPosition(1191.f, 838.f);
		fade_rect.setFillColor(sf::Color(0, 0, 0, 0)); fade_rect.setPosition(0.f, 0.f); fade_rect.setSize(sf::Vector2f(1920.f, 1080.f));

		background_forMAP.setSize({ 1920.f, 1080.f });
		background_forMAP.setFillColor(sf::Color(0, 0, 0, 170));

		find_table.emitTip();

		// press esc
		esc.setString("[ESC] - close");
		esc.setCharacterSize(24);
		esc.setFillColor(sf::Color::White);
		esc.setPosition(8.f, 60.f);
		// press esc

		auto addWall = [this](float x, float y, float w, float h) {
			sf::RectangleShape wall(sf::Vector2f(w, h));
			wall.setPosition(x, y);
			wall.setFillColor(sf::Color(255, 255, 255, 255));
			walls.push_back(wall);
			};

		addWall(509, 79, 18, 120); // 1
		addWall(545, 79, 167, 18);// 2
		addWall(692, 79, 18, 47); // 3
		addWall(692, 108, 325, 18); //4
		addWall(1001, 108, 18, 157); // 5
		addWall(1001, 247, 193, 18); // 6
		addWall(1176, 138, 18, 127); // 7
		addWall(1176, 138, 113, 18); // 8
		addWall(1271, 138, 18.f, 54.f); // 9
		addWall(1271.f, 174.f, 193.f, 18.f);// 10
		addWall(1446.f, 174.f, 18.f, 284.f); // 11
		addWall(1129.f, 437.f, 335.f, 18.f); // 12
		addWall(1001.f, 449.f, 131.f, 18.f); // 13
		addWall(1001.f, 449.f, 18.f, 55.f); // 14
		addWall(1001.f, 486.f, 141.f, 18.f); // 15
		addWall(1119.f, 486.f, 18.f, 228.f); // 16
		addWall(1119.f, 696.f, 353.f, 18.f); // 17
		addWall(1450.f, 696.f, 18.f, 301.f); // 18
		addWall(869.f, 985.f, 603.f, 18.f); // 19
		addWall(869.f, 717.f, 18.f, 295.f); // 20
		addWall(530.f, 710.f, 357.f, 18.f); // 21
		addWall(530.f, 476.f, 8.f, 250.f); // 23
		addWall(530.f, 476.f, 115.f, 18.f); // 24
		addWall(627.f, 476.f, 18.f, 104.f); // 25
		addWall(627.f, 562.f, 89.f, 18.f); // 26
		addWall(698.f, 476.f, 18.f, 104.f); // 27
		addWall(698.f, 486.f, 134.f, 18.f); // 28
		addWall(812.f, 292.f, 18.f, 200.f); // 29
		addWall(568.f, 292.f, 262.f, 18.f); // 30
		addWall(529.f, 167.f, 59.f, 129.f); // 31
	}
};

class ThirdLocationAssets {
public:
	Texture bgTex;
	Sprite bgSprite;
	bool LevelCompleted = false;

	// end of level rectangle fading
	RectangleShape last_rect;
	int last_rect_alpha = 0;
	bool last_rect_active = false;
	Clock last_rect_clock;
	enum FS { Non, FaIn, Wait, FaOut };
	// end of level rectangle fading

	// chests
	Chest chest_for_sword;
	Text OpenChest;
	vector<std::reference_wrapper<Chest>> chests;
	// chests

	Tip kill_the_boss;

	// evil
	Mush_monster ms1;
	Mush_monster ms2;
	Mush_monster ms3;
	Mush_monster ms4;
	Vector2f l_pos;
	Skeleton skelet;
	Wizard wizard_boss;
	bool draw_wizard_boss = false; // [DEBUG]
	// evil

	Tip look_mirror;

	// bed zone
	RectangleShape bed_zone;
	bool bed_zone_active = false;
	Tip looking_the_bed; // осматриваю кровать
	Text pressF_bed;
	Clock bed_zone_clock;
	bool pressFbed_clicked = false;
	Error take_key_again;
	bool GOT_KEY = false;
	// bed zone

	bool torch_activated = false;
	RectangleShape activating_torch_zone;
	Text pressEnter;

	// barrier
	bool portal_book_grabbed = false;
	Text press_e_takeabook;
	RectangleShape barrier_book_zone;
	CircleShape barrier_hitbox;
	bool barrier_active = true;
	Sprite barrier_sprite;
	Texture barrier_texture;
	bool forward = true;
	int cur_frame = 0;
	int frameHeight = 662, frameWidth = 282;
	Clock barrier_clock;
	Clock barrier_alpha_clock;
	int barrier_alpha = 255;
	// barrier
   
	Error ancient_seal_error;

	// box_one
	Texture box_one_texture;
	Sprite box_one_sprite;

	int box_one_cur_frame = 0;
	int box_one_frameHeight = 183, box_one_frameWidth = 165;
	int hit_counter = 0;
	// box_one

	// bloody screen
	Texture bloody_screen_texture;
	Sprite bloody_screen_sprite;
	int bs_alpha = 255;
	int bs_active = false;
	Clock bs_clock;
	// bloody screen

	// lib event
	bool first_time_event = true;

	Clock lib_event_timer; // 40s
	int lib_event = 0;
	Text lib_event_txt; // press Q 

	Tip result_text;
	Error lib_event_error;
	Error lib_sword_error;
	// lib event

	// wardrope
	Sprite wardrope_sprite;
	Texture wardrope_opened_texture_wb, wardrope_opened_texture_nb; // nb - no book!
	bool wardrope_is_opened = false;
	Text E;
	Text G; // take a book
	bool book_grabbed = false;
	Error wardrope_error;
	// wardrope

	// mirror
	Texture mirror_texture, broken_mirror;
	Sprite mirror_sprite;
	bool mirror_is_broken = false;
	Text pressF; // mirror text
	Tip mirror_tip;
	bool mir_is_already_broken = false;
	// mirror

	RectangleShape lib_event_zone;
	map<int, string> lib_event_texts = { {1,  "UNKNOWN: OOOPPPSS....."}, {2, "UNKNOWN: Your weapon got +30% damage"}, {3, "UNKNOWN: You got heal potion"}, {4, "UNKNOWN: SUPRPISE!!!"}};

	Vector2f skel_pos[3] = { {111.f, 311.f}, {1680.f, 635.f}, {865.f, 730.f} };
	int skelet_rand_pos = 0;
	// 1. -15% hp просто так, 2. +30% к урону оружия на 30 сек, 3. зелье хила, 4. выпрыгивает скелет
	// lib event

	vector<RectangleShape> walls;
	vector<RectangleShape> walls_for_mushrooms;

	bool all_is_locked = false;

		ThirdLocationAssets(Font& font) : ms1(10, 50, 60, 1, { 111.f, 517.f }), ms2(10, 50, 60, 1, { 1460.f, 475.f }), skelet(34, 70, 50.f, 3.f, { 880.f, 720.f }),
			result_text("rez_txt", "", font, 24, Color::White, one_line, 255), 
			lib_event_error("lib_event_error", "AAAAAA", font, 24, Color::White, one_line, 255), 
			mirror_tip("mirror_tip", "Tip: Check the book", font, 24, Color::White, one_line, 255),
			lib_sword_error("lib_sword_error", "UNKNOWN: Get a weapon.", font, 24, Color::White, one_line, 255),
			wardrope_error("wardrope_error", "Find an old key.", font, 24, Color::Red, one_line, 255),
			looking_the_bed("looking_bed", "I'm examining the bed.", font, 24, Color::White, one_line, 255),
			take_key_again("take_key_again", "Failed to take a key. Try again", font, 24, Color::White, one_line, 255),
			ancient_seal_error("ancient_seal_err", "Find an ancient seal", font, 24, Color::Red, one_line, 255),
			ms3(38, 100, 70, 1.5, {503.f, 123.f}), ms4(38, 100, 70, 1.5, { 832.f, 637.f }),
			look_mirror("look_mirr", "Something is calling from the mirror... Come closer.", font, 24, Color::White, one_line, 255),
			wizard_boss(0, 500, 50, 385.f, 0, 250.f, "textures/chapone/evil/wizard/fb.png", { 618.f, 235.f }),
			kill_the_boss("kill the boss", "KILL THE BOSS!!", font, 24, Color::White, one_line, 255) {}

		bool load(Font& font) {
			if (!bgTex.loadFromFile("textures/chapone/locationthree/map_texts/location_three.png") || !chest_for_sword.load("textures/chapone/locationthree/chest/chest__one.png")
				|| !barrier_texture.loadFromFile("textures/chapone/locationthree/etc/barrier.png") || !ms1.load(font) || !ms2.load(font)
				|| !skelet.loadTexture("textures/chapone/evil/skeleton_guard/skelfront_right.png", "textures/chapone/evil/skeleton_guard/skelfront_left.png", "textures/chapone/evil/skeleton_guard/skelback_right.png",
					"textures/chapone/evil/skeleton_guard/skelback_left.png", "textures/chapone/evil/skeleton_guard/skelleft_left.png", "textures/chapone/evil/skeleton_guard/skelleft_right.png",
					"textures/chapone/evil/skeleton_guard/skelright_right.png", "textures/chapone/evil/skeleton_guard/skelright_left.png", font) ||
				!box_one_texture.loadFromFile("textures/chapone/locationthree/box_one/box.png") || !bloody_screen_texture.loadFromFile("textures/chapone/locationtwo/etc/boiler/bloody_screen.png") || 
				!wardrope_opened_texture_wb.loadFromFile("textures/chapone/locationthree/map_texts/wardrope_opened_with_book.png") ||
				!wardrope_opened_texture_nb.loadFromFile("textures/chapone/locationthree/map_texts/wardrope_opened_without_book.png") || 
				!mirror_texture.loadFromFile("textures/chapone/locationthree/etc/mirror.png") || !broken_mirror.loadFromFile("textures/chapone/locationthree/etc/mirror_broken.png") ||
				!ms3.load(font) || !ms4.load(font) || !wizard_boss.load(font)  || !wizard_boss.load_tornado(
					"textures/chapone/evil/wizard/death anim/one.png", "textures/chapone/evil/wizard/death anim/two.png",
					"textures/chapone/evil/wizard/death anim/three.png", "textures/chapone/evil/wizard/death anim/four.png",
					"textures/chapone/evil/wizard/death anim/five.png", "textures/chapone/evil/wizard/death anim/six.png",
					"textures/chapone/evil/wizard/death anim/seven.png", "textures/chapone/evil/wizard/death anim/eight.png",
					"textures/chapone/evil/wizard/death anim/nine.png", "textures/chapone/evil/wizard/death anim/ten.png",
					"textures/chapone/evil/wizard/death anim/eleven.png", "textures/chapone/evil/wizard/death anim/twelve.png",
					"textures/chapone/evil/wizard/death anim/thirteen.png", "textures/chapone/evil/wizard/death anim/fourteen.png",
					"textures/chapone/evil/wizard/death anim/fifteen.png", "textures/chapone/evil/wizard/death anim/sixteen.png",
					"textures/chapone/evil/wizard/death anim/seventeen.png", "textures/chapone/evil/wizard/death anim/eighteen.png")) {
				return false;
			}

			auto addWall = [this](float x, float y, float w, float h) {
				sf::RectangleShape wall(sf::Vector2f(w, h));
				wall.setPosition(x, y);
				wall.setFillColor(sf::Color(255, 255, 255, 255));
				walls.push_back(wall);
			};

			addWall(0, 0, 109, 822); // 1
			addWall(42, 0, 1878, 211); // 2
			addWall(132, 206, 166, 29); // wardrope
			addWall(1025, 170, 137, 93); // 3
			addWall(1808, 206, 112, 577); // 4
			addWall(1534, 772, 385, 200); // 5.1
			addWall(1306, 912, 310, 161); // 5.2
			addWall(1031, 772, 371, 200); // 5.3
			addWall(755, 868, 310, 161); // 6
			addWall(29, 771, 770, 184); // 7
			addWall(812, 462, 137, 29); // book (8.1)
			addWall(827, 439, 107, 30); // book (8.2)
			addWall(380, 458, 117, 40); // table (9)
			addWall(357, 266, 157, 27); // box (10.1)
			addWall(402, 293, 64, 17); // box (10.2)
			addWall(1640, 235, 112, 10); // chest (11)

			// bed
			addWall(1626, 465, 14, 75); // 12
			addWall(1740, 465, 14, 75); // 13
			addWall(1626, 482, 128, 122); // 14
			// bed

			addWall(1273, 389, 32, 65); // 15

			auto addWallMushRoom = [this](float x, float y, float w, float h) {
				sf::RectangleShape wall(sf::Vector2f(w, h));
				wall.setPosition(x, y);
				wall.setFillColor(sf::Color(255, 255, 255, 255));
				walls_for_mushrooms.push_back(wall);
			};

			addWallMushRoom(0, 0, 109, 822); // 1
			addWallMushRoom(42, 0, 1878, 205); // 2
			addWallMushRoom(132, 206, 166, 29); // wardrope(3)
			addWallMushRoom(357, 206, 157, 27); // box(4)
			addWallMushRoom(1808, 206, 112, 577); // 5
			addWallMushRoom(1534, 772, 385, 200); // 6.1
			addWallMushRoom(1306, 912, 310, 161); // 6.2
			addWallMushRoom(1031, 772, 371, 200); // 6.3
			addWallMushRoom(755, 868, 310, 161); // 7
			addWallMushRoom(29, 771, 770, 184); // 8
			addWallMushRoom(380, 458, 117, 1); // table (9)
			addWallMushRoom(1273, 389, 32, 65); // 10

			// bed
			addWall(1626, 465, 14, 75); // 11
			addWall(1740, 465, 14, 75); // 12
			addWall(1626, 482, 128, 64); // 13
			// bed

			return true;
		}

		void update() {
			bgSprite.setPosition(0.f, 0.f);	bgSprite.setTexture(bgTex);

			activating_torch_zone.setPosition(673.f, 212.f), activating_torch_zone.setSize(Vector2f(20.f, 15.f));

			chest_for_sword.sprite.setPosition(1622, 243); chest_for_sword.setTexture();
			chests.push_back(chest_for_sword);

			last_rect.setFillColor(Color::Black);
			last_rect.setPosition(0.f, 0.f);
			last_rect.setSize({ 1920.f, 1080.f });

			barrier_sprite.setPosition(747.f, 0.f); barrier_sprite.setTexture(barrier_texture);
			barrier_hitbox.setRadius(88.f); barrier_hitbox.setPosition(793.f, 426.f);
			barrier_hitbox.setFillColor(Color::White);
			barrier_book_zone.setPosition(817.f, 444.f); barrier_book_zone.setSize(Vector2f(132.f, 129.f));

			skelet.setFrontLeftTexture();
			
			box_one_sprite.setTexture(box_one_texture);
			box_one_sprite.setTextureRect(IntRect(0, 0, box_one_frameWidth, box_one_frameHeight));
			box_one_sprite.setPosition(353.f, 242.f);

			lib_event_zone.setSize(Vector2f(137.f, 97.f));
			lib_event_zone.setPosition(1025.f, 263.f);
			lib_event_txt.setString("Press [Q] to get a reward");
			lib_event_txt.setCharacterSize(24);
			lib_event_txt.setFillColor(Color::White);

			bloody_screen_sprite.setTexture(bloody_screen_texture);

			mirror_sprite.setPosition(1250.f, 105.f);
			mirror_sprite.setTexture(mirror_texture);

			E.setCharacterSize(24);
			E.setString("[E]");
			E.setFillColor(Color::White);

			G.setCharacterSize(24);
			G.setString("[G] - grab a book");
			G.setFillColor(Color::White);
			G.setPosition(8.f, 61.f);

			bed_zone.setPosition(1628.f, 633.f);
			bed_zone.setSize(Vector2f(131.f, 57.f));
			pressF_bed.setCharacterSize(24);
			pressF_bed.setString("Press [F] to inspect the bed");
			pressF_bed.setFillColor(Color::White);

			pressF.setString("Press [F] to look at the mirror");
			pressF.setFillColor(Color::White);
			pressF.setCharacterSize(24);

			pressEnter.setString("Press [ENTER] to ac tivate");
			pressEnter.setFillColor(Color::White);
			pressEnter.setCharacterSize(24);

			OpenChest.setFillColor(sf::Color::White);
			OpenChest.setCharacterSize(24);
			OpenChest.setString("Press [E] to open the chest");

			press_e_takeabook.setFillColor(Color::White);
			press_e_takeabook.setCharacterSize(24);
			press_e_takeabook.setString("Press [E] to get a book");

			wardrope_sprite.setTexture(wardrope_opened_texture_wb);
			wardrope_sprite.setPosition(120.f, 143.f);
		}

		void update_barrier(RenderWindow& window) {
			if (barrier_active) {
				if (barrier_clock.getElapsedTime().asSeconds() > 0.09) {
					if (forward) {
						cur_frame++;
						if (cur_frame >= 5) {
							cur_frame = 5;
							forward = false;
						}
					}
					else {
						cur_frame--;
						if (cur_frame <= 0) {
							cur_frame = 0;
							forward = true;
						}
					}
					barrier_sprite.setTextureRect({ cur_frame * frameWidth, 0, frameWidth, frameHeight });
					barrier_clock.restart();
				}
			}
			window.draw(barrier_sprite);
		}

		void check_if_the_book_grabbed() {
			if (!book_grabbed) {
				wardrope_sprite.setTexture(wardrope_opened_texture_wb);
			}
			else {
				wardrope_sprite.setTexture(wardrope_opened_texture_nb);
			}
		}

		void apply_font(Font& font) {
			lib_event_txt.setFont(font);
			pressF.setFont(font);
			pressF_bed.setFont(font);
			// wardrope
			G.setFont(font);
			E.setFont(font);
			// wardrope
			pressEnter.setFont(font);
			OpenChest.setFont(font);
			press_e_takeabook.setFont(font);
		}

		void init_library_event(MainPlayer &player, Sword *sword, Item *activeItem, ItemManager &item_manager, unique_ptr<Skeleton>& new_skelet, Font &font) {
			if (lib_event_zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
				if (lib_event_timer.getElapsedTime().asSeconds() >= 40 || first_time_event) {
					lib_event = (rand() % 3) + 1;
					skelet_rand_pos = (rand() % 2) + 0;
					first_time_event = false; 
					result_text.emitTip();
					switch (lib_event) {
					case 1:
					{
						int cur_dmg = player.maxHP * 0.15f;
						if (player.current_health >= player.maxHP * 0.4f) {
							player.current_health -= cur_dmg;
							bs_active = true;
						}
						break;
					}
					case 2: {
						if (activeItem && activeItem == sword) {
							std::unique_ptr<DamageBoost> dmg_boost = std::make_unique<DamageBoost>("textures/buffs/damage_buff.png", 0.3f, 30, "textures/buffs/buffs_description/dmg_boost_description.png");
							dmg_boost->use(player.inventory);
							player.active_item_buffs.push_back(std::move(dmg_boost));
						}
						else {
							lib_sword_error.emitError(); 
						}
						break;
					}
					case 3: {
						unique_ptr<Item> inter_heal_potion = item_manager.items[3]->clone();
							if (inter_heal_potion) {
								for (auto& slot : player.inventory) {
									if (slot.item == nullptr) {
										slot.item = std::move(inter_heal_potion);
										break;
									}
								}
							}
						break;
					}
					case 4: {
						 new_skelet = std::make_unique<Skeleton>(44, 100, 50, 3.5f, skel_pos[skelet_rand_pos]);
						 new_skelet->loadTexture("textures/chapone/evil/skeleton_guard/skelfront_right.png", "textures/chapone/evil/skeleton_guard/skelfront_left.png", "textures/chapone/evil/skeleton_guard/skelback_right.png",
							 "textures/chapone/evil/skeleton_guard/skelback_left.png", "textures/chapone/evil/skeleton_guard/skelleft_left.png", "textures/chapone/evil/skeleton_guard/skelleft_right.png",
							 "textures/chapone/evil/skeleton_guard/skelright_right.png", "textures/chapone/evil/skeleton_guard/skelright_left.png", font);
						 break;
						}
					}
						
					if (!lib_sword_error.isEnabled() && !lib_event_error.isEnabled()) {
						auto found_text = lib_event_texts.find(lib_event);
						if (found_text != lib_event_texts.end()) {
							result_text.setTipText(found_text->second);
						}
						lib_event_timer.restart();
					}
				}
				else {
					if (!lib_event_error.isEnabled())
						lib_event_error.emitError();
				}
			}
		}
};

GameState forests_of_echo_lvl(RenderWindow& window, MainPlayer& player, sf::Font& font) {
	/*player.sprite.setPosition(559.f, 4820.f);*/
	player.sprite.setPosition(3177.f, 4346.f);
	player.maxHP = 115;
	player.current_health = 115;
	player.update_all_(font);
	
	SwordSlashEffect swordEffect;

	Forests_of_echo_assets assets(window);
	if (assets.load_chunk_textures() && assets.load_chunk_mask()) {
		assets.chunk_setTextures();
	}
	else {
		return Exit_gs;
	}

	while (window.isOpen()) {
		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::KeyPressed) {
				if (event.key.code == Keyboard::Escape) {
					window.close();
				}
			}
		}

	/*	if (!pause.isPaused) {*/
			Vector2f move(0.f, 0.f);
			float dt = assets.delta_clock.restart().asSeconds();
			player.moving = false;

			player.handleMovementInput(assets.all_is_locked, move, dt);

			sf::Vector2f moveX(move.x, 0.f);
			player.sprite.move(moveX);
			if (!assets.update_collisions_mask(player, moveX) || !assets.update_collisions_lvl(player)) {
				player.sprite.move(-moveX);
			}

			sf::Vector2f moveY(0.f, move.y);
			player.sprite.move(moveY);
			if (!assets.update_collisions_mask(player, moveY) || !assets.update_collisions_lvl(player)) {
				player.sprite.move(-moveY);
			}

			assets.update_chunks_visible(player);
			assets.update_camera(window, player);

		/*	if (temp_holder && temp_holder->isDraggin) {
				temp_holder->sprite.setPosition(mouse_posui + temp_holder->offset);
			}
			if (temp_armor && temp_armor->isDraggin) {
				temp_armor->sprite.setPosition(mouse_posui + temp_armor->offset);
			}*/

			window.clear();

			window.setView(assets.cam);
			assets.draw_visible_chunks(window);
			player.drawEffectsVisuals(window, player);
			assets.update_and_draw_all(player, window, swordEffect);

			window.setView(window.getDefaultView());
			player.draw_inventory(window);
			/*window.draw(pause.pause_button_sprite);*/

			if (player.activeItem && player.activeItem->ability) {
				if (auto* read_book = dynamic_cast<ReadBook*>(player.activeItem->ability.get())) {
					read_book->draw(window, player);
				}
			}
			player.updateAllEffects(nullptr, nullptr, nullptr);
			player.draw_cooldowns(window);

			player.draw_health(window);
			/*player.draw_status_bar(mouse_pos, window);

			if (temp_holder) {
				window.draw(temp_holder->sprite);
			}
			if (temp_armor) {
				window.draw(temp_armor->sprite);
			}*/
		/*}*/
			window.display();
	}

}

GameState Hub(RenderWindow& window, MainPlayer& player, Locations& current_location, GameState state, sf::Font& font, Pause &pause, ItemManager& item_manager) {
	player.sprite.setPosition(2455.f, 2858.f);
	player.maxHP = 115;
	player.current_health = 115;
	player.update_all_(font);

	Clock delta;
	SwordSlashEffect swordEffect;

	Hub_assets assets(window, font);
	if (assets.load_hub_textures() && assets.load_hub_masks()) {
		assets.chunk_setTextures();
	}
	else {
		return Exit_gs;
	}
	Hub_assets::FS last_rect_state = Hub_assets::FS::Non;

	// Items
	unique_ptr<Item> sword = item_manager.items[10]->clone();
	player.inventory[1].item = std::move(sword);

	unique_ptr<Item> fog_mask = item_manager.items[5]->clone();
	//assets.chest_two.items[1].item = std::move(fog_mask);
	player.inventory[1].item = std::move(fog_mask);

	Armor* showingArmor = nullptr;
	unique_ptr<Armor> temp_armor = nullptr;
	ArmorSlot* oldArmor = nullptr;

	Item* showingItem = nullptr; // предмет, чьё описание показывается
	unique_ptr<Item> temp_holder = nullptr; // временно
	sf::Vector2f back_pos; // позиция предмета, который мы переместил
	InventorySlot* oldSlot = nullptr;

	std::vector<std::reference_wrapper<Chest>> chests;
	
	while (window.isOpen()) {
		Event event;
		Vector2i mouse_pos = Mouse::getPosition(window);
		Vector2f mouse_posui = window.mapPixelToCoords(mouse_pos, window.getDefaultView());

		player.upd_item_ptr();

		static Item* lastSword = nullptr;
		if (player.activeItem != lastSword && player.activeItem) {
			Sword* sword = dynamic_cast<Sword*>(player.activeItem);
			if (sword) {
				swordEffect.loadFromItem(sword);
				lastSword = player.activeItem;
			}
		}

		while (window.pollEvent(event)) {
			assets.show_world_description(event, player);
			if (event.type == Event::Closed)
				window.close();
			if (event.type == Event::KeyPressed) {
				if(event.key.code == Keyboard::Tab) 
					player.activateMenu(assets.all_is_locked);
				if (event.key.code >= Keyboard::Num1 && event.key.code <= Keyboard::Num9)
					player.selectedSlot = event.key.code - Keyboard::Num1;
				if (event.key.code == Keyboard::Escape) {
					if (player.menu_is_active) {
						player.disableMenu(assets.all_is_locked);
					}
				}
				if (event.key.code == Keyboard::Enter && !assets.enter_clicked) {
					for (auto& crystal : assets.crystals) {
						if (crystal.rect.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
							assets.all_is_locked = true;
							assets.new_location = crystal.location;
							last_rect_state = Hub_assets::FS::Non;
							assets.last_rect_active = true;
							assets.enter_clicked = true;
						}
					}
				}
				if (event.key.code == Keyboard::Space) {
					player.useActiveItem(event, assets.all_is_locked, font, assets.chunks, assets);
				}
			}
			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
				bool inventoryClicked = false;
				player.drag_item(window, mouse_posui, inventoryClicked, assets.all_is_locked, chests, temp_armor, temp_holder, back_pos, oldSlot, oldArmor);
				if (!inventoryClicked) {
					if (!assets.all_is_locked && !pause.isPaused) {
						Sword* sword = dynamic_cast<Sword*>(player.activeItem);
						if (player.activeItem && sword) {
							if (swordEffect.attack_delay.getElapsedTime().asSeconds() >= sword->attack_cooldown) {
								swordEffect.activate(player);
								swordEffect.attack_delay.restart();
							}
						}
					}
				}
			}
			player.drop_item(event, chests, temp_armor, temp_holder, back_pos, oldSlot, oldArmor);
		}

		if (!pause.isPaused) {
			Vector2f move(0.f, 0.f);
			float dt = delta.restart().asSeconds();
			player.moving = false;

			player.handleMovementInput(assets.all_is_locked, move, dt);

			sf::Vector2f moveX(move.x, 0.f);
			player.sprite.move(moveX);
			if (!assets.update_collisions_mask(player, moveX) || !assets.update_collisions_lvl(player)) {
				player.sprite.move(-moveX);
			}

			sf::Vector2f moveY(0.f, move.y);
			player.sprite.move(moveY);
			if (!assets.update_collisions_mask(player, moveY) || !assets.update_collisions_lvl(player)) {
				player.sprite.move(-moveY);
			}

			assets.update_chunks_visible(player);
			assets.update_camera(window, player);

			if (temp_holder && temp_holder->isDraggin) {
				temp_holder->sprite.setPosition(mouse_posui + temp_holder->offset);
			}
			if (temp_armor && temp_armor->isDraggin) {
				temp_armor->sprite.setPosition(mouse_posui + temp_armor->offset);
			}

			window.clear();

			window.setView(assets.cam);
			assets.draw_visible_chunks(window);
			player.drawEffectsVisuals(window, player);
			assets.update_and_draw_all(player, window, swordEffect);

			for (auto& crystal : assets.crystals) {
				if (player.sprite.getGlobalBounds().intersects(crystal.rect.getGlobalBounds())) {
					assets.pressF.setPosition(player.sprite.getPosition().x - 60.f, player.sprite.getPosition().y - 45.f);
					assets.pressEnter.setPosition(player.sprite.getPosition().x - 70.f, player.sprite.getPosition().y - 70.f);
					window.draw(assets.pressEnter);
					window.draw(assets.pressF);
				}
			}

			window.setView(window.getDefaultView());
			player.draw_inventory(window);
			window.draw(pause.pause_button_sprite);
			assets.draw_world_description(window);
			if (player.activeItem && player.activeItem->ability) {
				if (auto* read_book = dynamic_cast<ReadBook*>(player.activeItem->ability.get())) {
					read_book->draw(window, player);
				}
			}
			player.updateAllEffects(nullptr, nullptr, nullptr);
			player.draw_cooldowns(window);

			player.draw_health(window);
			player.draw_status_bar(mouse_pos, window);

			if (temp_holder) {
				window.draw(temp_holder->sprite);
			}
			if (temp_armor) {
				window.draw(temp_armor->sprite);
			}

			// last_rect
			if (assets.last_rect_active) {
				assets.last_rect.setPosition(0.f, 0.f);
				if (assets.last_rect_clock.getElapsedTime().asSeconds() >= 1.3) {
					last_rect_state = Hub_assets::FS::FaIn;
					assets.last_rect_clock.restart();
				}
				if (last_rect_state == Hub_assets::FS::FaIn) {
					if (assets.last_rect_clock.getElapsedTime().asSeconds() >= 0.011 && assets.last_rect_alpha < 255) {
						assets.last_rect_alpha++;
						assets.last_rect.setFillColor(sf::Color(0, 0, 0, assets.last_rect_alpha));
						assets.last_rect_clock.restart();
					}
					if (assets.last_rect_alpha >= 255) {
						last_rect_state = Hub_assets::FS::Wait;
						assets.last_rect_clock.restart();
					}
				}
				else if (last_rect_state == Hub_assets::FS::Wait) {
					if (assets.last_rect_clock.getElapsedTime().asSeconds() > 0.2f) {
						assets.last_rect_alpha = 255;
						current_location = assets.new_location;
						assets.all_is_locked = false;
						return GameState::Playing;
						last_rect_state = Hub_assets::FS::FaOut;
					}
				}
				else if (last_rect_state == Hub_assets::FS::FaOut) {
					assets.last_rect_active = false;
					assets.last_rect_alpha = 0;
					assets.last_rect.setFillColor(sf::Color(0, 0, 0, 0));
					assets.enter_clicked = false;
				}
			}
			if (assets.last_rect_active) {
				window.draw(assets.last_rect);
			}
			// last_rec

		}
		else {
			window.draw(pause.pause_screen_sprite);
			window.draw(pause.resume.button_sprite);
			window.draw(pause.exit.button_sprite);
			window.draw(pause.settings.button_sprite);
		}
		window.display();
	}
	return state;
}

GameState third_location(RenderWindow& window, MainPlayer& player, GameState state, sf::Font& font,
	Locations& current_location, ItemDescription& item_description, Pause& pause, TryAgainMenu& tryagain, 
	std::map<std::string, Texture>& allItemTextures, ItemManager& item_manager) {

	player.sprite.setPosition(514.f, 227.f);
	player.maxHP = 115;
	player.current_health = 115;
	player.update_all_(font);

	ThirdLocationAssets assets(font);
	SwordSlashEffect swordEffect;

	if (!assets.load(font)) {
		return Exit_gs;
	}
	assets.update();
	assets.apply_font(font);

	assets.chest_for_sword.set_text_settings(font);
	assets.chest_for_sword.initArmor();
	assets.chest_for_sword.init_items_if_needed();
	assets.chest_for_sword.update();

	// portal
	Portal portal;
	if (!portal.load()) {
		return Exit_gs;
	}
	else {
		portal.make_portal(font, { 1175.f,  385.f }, { PORTAL_LEFT_SCALE });
	}
	ThirdLocationAssets::FS last_rect_state = ThirdLocationAssets::FS::Non;
	// portal

	std::unique_ptr<Skeleton> new_skelet = nullptr;

	// items
	unique_ptr<Item> mysterious_apple = item_manager.items[16]->clone();
	
	unique_ptr<Item> vein_of_earth = item_manager.items[10]->clone();
	/*assets.chest_for_sword.items[4].item = std::move(vein_of_earth);*/
	player.menu_inventory[1][2].item = std::move(vein_of_earth);

	unique_ptr<Item> heal_potion_one = item_manager.items[3]->clone();
	heal_potion_one->sprite.setPosition(522.f, 317.f);

	unique_ptr<Item> heal_potion_two = item_manager.items[3]->clone();
	assets.chest_for_sword.items[1].item = std::move(heal_potion_two);

	unique_ptr<Item> wkey = item_manager.items[13]->clone();

	unique_ptr<Item> cursed_diary = item_manager.items[12]->clone();
	
	unique_ptr<Item> portal_book = item_manager.items[15]->clone();

	unique_ptr<Item> cursed_mirror = item_manager.items[14]->clone();
	//player.inventory[7].item = std::move(cursed_mirror);
	// items

	// Items
	Armor* showingArmor = nullptr;
	unique_ptr<Armor> temp_armor = nullptr;
	ArmorSlot* oldArmor = nullptr;

	Item* showingItem = nullptr; // предмет, чьё описание показывается
	unique_ptr<Item> temp_holder = nullptr; // временно
	sf::Vector2f back_pos; // позиция предмета, который мы переместил
	InventorySlot* oldSlot = nullptr;

	// chest text

	// chest text

	Clock deltaClock;
	while (window.isOpen()) {
		Event event;
		player.upd_item_ptr();

		static Item* lastSword = nullptr;
		if (player.activeItem != lastSword && player.activeItem) {
			Sword* sword = dynamic_cast<Sword*>(player.activeItem);
			if (sword) {
				swordEffect.loadFromItem(sword);
				lastSword = player.activeItem;
			}
		}

		Sword* sword = dynamic_cast<Sword*>(player.activeItem); // для замедления

		sf::Vector2i mouse_pos = Mouse::getPosition(window); // !!!!!
		while (window.pollEvent(event)) {
			if (event.type == Event::KeyPressed) {
				if (event.key.code == Keyboard::F) {
					if (!assets.mirror_is_broken && assets.mirror_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds()))
					{
						bool found_book = false;
						for (auto& slot : player.inventory) {
							if (slot.item && slot.item->ability) {
								if (auto* read_book = dynamic_cast<ReadBook*>(slot.item->ability.get())) {
									read_book->add_page("textures/books/cursed_diary(third_loc)/P3.png");
									found_book = true;
									break;
								}
							}
						}
						if (found_book) {
							assets.mir_is_already_broken = true;
							assets.mirror_is_broken = true;
							assets.mirror_sprite.setTexture(assets.broken_mirror);
							assets.mirror_tip.emitTip();
						}
					}
					if (!assets.GOT_KEY && assets.bed_zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
						assets.bed_zone_active = true;
						assets.pressFbed_clicked = true;
						assets.bed_zone_clock.restart();
						assets.looking_the_bed.emitTip();
					}
				}
				if (event.key.code >= Keyboard::Num1 && event.key.code <= Keyboard::Num9) {
					player.selectedSlot = event.key.code - Keyboard::Num1;
				}
				if (event.key.code == Keyboard::Z) {
					assets.wizard_boss.spawn();
				}
				if (event.key.code == Keyboard::Enter && portal.portal_should_activate && portal.portal_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
					assets.all_is_locked = true;
					assets.last_rect_active = true;
					assets.LevelCompleted = true;
					player.cur_health_copy = player.current_health;
					player.copy_items_for_newlvl();
				}
				player.useActiveItem(event, assets.all_is_locked, font, assets.walls);
				if (!assets.all_is_locked) {
					if (event.key.code == Keyboard::Space && player.activeItem && player.activeItem->ability == nullptr) {
						if (player.activeItem->name == "mysterious_apple") {
							/*player.removeItem();*/
							player.col_lvl = 1;
							player.col_resistance_multiplier = 0.15f;
							portal.portal_should_activate = true;
						}
					}
				}
				if (event.key.code == Keyboard::Tab) {
					player.activateMenu(assets.all_is_locked);
				}
				if (event.key.code == Keyboard::Escape) {
					if (player.menu_is_active) {
						player.disableMenu(assets.all_is_locked);
					}
					if (assets.chest_for_sword.isOpened) {
						assets.chest_for_sword.close_chest(assets.all_is_locked);
					}
				}
				if (event.key.code == Keyboard::E) {
					if (assets.chest_for_sword.sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
					/*	if (activeItem && activeItem->name == "ancient_seal1") {
							assets.chest_for_sword.open_chest(assets.all_is_locked);
						}
						else {
							assets.ancient_seal_error.emitError();
						}*/
						assets.chest_for_sword.open_chest(assets.all_is_locked);
					}
					if (assets.barrier_book_zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) &&
						assets.wizard_boss.isKilled && assets.barrier_alpha <= 0 && !assets.portal_book_grabbed) {
						for (auto& slot : player.inventory) { 
							if (slot.item == nullptr) {
								slot.item = std::move(portal_book);
								assets.portal_book_grabbed = true;								
								break;
							}
						}
					}
					if (assets.wardrope_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
						if (player.activeItem) {
							auto* key_ptr = dynamic_cast<Key*>(player.activeItem);
							if (key_ptr && key_ptr->name == "third_loc_wardrope_key") {
								assets.wardrope_is_opened = !assets.wardrope_is_opened;
							}
							else {
								assets.wardrope_error.emitError();
							}
						}
						else {
							assets.wardrope_error.emitError();
						}
					}
				}
				if (event.key.code == Keyboard::Q) {
					assets.init_library_event(player, sword, player.activeItem, item_manager, new_skelet, font);
				}
				if (event.key.code == Keyboard::G) {
					if (assets.wardrope_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && assets.wardrope_is_opened) {
						assets.book_grabbed = true;
						assets.look_mirror.emitTip();
						for (auto& slot : player.inventory) {
							if (!slot.item) {
								slot.item = std::move(cursed_diary);
								break;
							}
						}
					}
				}
				if (event.key.code == Keyboard::Enter) {
					if(assets.mir_is_already_broken) {
						if (assets.activating_torch_zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
							assets.torch_activated = true;
							assets.ms3.health = 100;
							assets.ms4.health = 100;
						}
					}
				}
			}
			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
				bool inventoryClicked = false;				
				player.drag_item(window, mouse_pos, inventoryClicked, assets.all_is_locked, assets.chests, temp_armor, temp_holder, back_pos, oldSlot, oldArmor);
				if (!inventoryClicked) {
					if (!assets.all_is_locked && !pause.isPaused) {
						Sword* sword = dynamic_cast<Sword*>(player.activeItem);
						if (player.activeItem && sword) {
							if (swordEffect.attack_delay.getElapsedTime().asSeconds() >= sword->attack_cooldown) {
								swordEffect.activate(player);
								swordEffect.attack_delay.restart();
								if (swordEffect.isActive()) {
									assets.ms1.get_damage(swordEffect, sword, player);
									assets.ms2.get_damage(swordEffect, sword, player);
									assets.ms3.get_damage(swordEffect, sword, player);
									assets.ms4.get_damage(swordEffect, sword, player);
									if (assets.box_one_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
										assets.hit_counter++;
									}
									if (new_skelet != nullptr) {
										new_skelet->get_damage(swordEffect, sword, player);
									}
									assets.skelet.get_damage(swordEffect, sword, player);
									assets.wizard_boss.get_damage(swordEffect, sword, player);
								}
							}
						}
					}				
				}

				if (pause.pause_button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && !pause.isPaused
					&& !player.isDead && !inventoryClicked) {
					pause.isPaused = true;
				}
				if (pause.resume.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.resume.to_true();
					pause.resume.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (pause.settings.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.settings.to_true();
					pause.settings.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (pause.exit.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.exit.to_true();
					pause.exit.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && tryagain.retry.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) &&
					tryagain.isActive) {
					tryagain.retry.to_true();
					tryagain.retry.setClickedTexture();
					tryagain.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && tryagain.menu.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) &&
					tryagain.isActive) {
					tryagain.menu.to_true();
					tryagain.menu.setClickedTexture();
					tryagain.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && pause.resume.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.resume.to_true();
					pause.resume.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && pause.settings.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.settings.to_true();
					pause.settings.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && pause.exit.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.exit.to_true();
					pause.exit.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && pause.pause_button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && !pause.isPaused
					&& !player.isDead) {
					pause.isPaused = true;
				}
			}
			player.drop_item(event, assets.chests, temp_armor, temp_holder, back_pos, oldSlot, oldArmor);	
		}

		if (tryagain.retry.get_clicked() && tryagain.button_clock.getElapsedTime().asSeconds() > 0.15) {
			tryagain.retry.setTexture();
			tryagain.retry.to_false();
			// bool reset
			tryagain.isActive = false;
			if (player.isDead) {
				player.copy_items_ifdead();
				player.current_health = player.cur_health_copy;
			}
			// bool reset
			return Playing;
		}
		if (tryagain.menu.get_clicked() && tryagain.button_clock.getElapsedTime().asSeconds() > 0.15) {
			tryagain.menu.setTexture();
			tryagain.menu.to_false();
			tryagain.isActive = false;
			pause.isPaused = false;
			return Menu_gs;
		}
		if (pause.resume.get_clicked() && pause.button_clock.getElapsedTime().asSeconds() > 0.15) {
			pause.resume.setTexture();
			pause.resume.to_false();
			pause.isPaused = false;
		}
		if (pause.settings.get_clicked() && pause.button_clock.getElapsedTime().asSeconds() > 0.15) {
			pause.settings.setTexture();
			pause.settings.to_false();
		}
		if (pause.exit.get_clicked() && pause.button_clock.getElapsedTime().asSeconds() > 0.15) {
			pause.exit.setTexture();
			pause.exit.to_false();
			tryagain.isActive = false;
			pause.isPaused = false;
			return Menu_gs;
		}

		assets.press_e_takeabook.setPosition(player.sprite.getPosition().x - 60.f, player.sprite.getPosition().y - 45.f);
		assets.OpenChest.setPosition(player.sprite.getPosition().x - 60.f, player.sprite.getPosition().y - 45.f);
		assets.lib_event_txt.setPosition(player.sprite.getPosition().x - 60.f, player.sprite.getPosition().y - 45.f);

		if (temp_holder && temp_holder->isDraggin) {
			temp_holder->sprite.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)) + temp_holder->offset);
		}
		if (temp_armor && temp_armor->isDraggin) {
			temp_armor->sprite.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)) + temp_armor->offset);
		}

		float dt = deltaClock.restart().asSeconds(); // !!!!!!!!!

		if (!pause.isPaused) {
			player.moving = false;
			sf::Vector2f move(0.f, 0.f);
			player.handleMovementInput(assets.all_is_locked, move, dt);

			// коллизии
			player.sprite.move(move.x, 0.f); // x
			for (auto& wall : assets.walls) {
				if (player.sprite.getGlobalBounds().intersects(wall.getGlobalBounds())) {
					player.sprite.move(-move.x, 0.f);
					break;
				}
			}
			if (assets.barrier_active) {
				if (player.sprite.getGlobalBounds().intersects(assets.barrier_hitbox.getGlobalBounds())) {
					player.sprite.move(-move.x, 0.f);
				}
			}

			player.sprite.move(0.f, move.y); // y
			for (auto& wall : assets.walls) {
				if (player.sprite.getGlobalBounds().intersects(wall.getGlobalBounds())) {
					player.sprite.move(0.f, -move.y);
					break;
				}
			}
			if (assets.barrier_active) {
				if (player.sprite.getGlobalBounds().intersects(assets.barrier_hitbox.getGlobalBounds())) {
					player.sprite.move(0.f, -move.y);
				}
			}
			// коллизии

				window.clear();
				window.draw(assets.bgSprite);

				if (portal.portal_should_activate) portal.draw(player, window);

				window.draw(assets.mirror_sprite); // mirror
				
				if (assets.mirror_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && !assets.mirror_is_broken) {
					assets.pressF.setPosition(player.sprite.getPosition().x - 70.f, player.sprite.getPosition().y - 45.f);
					window.draw(assets.pressF);
				}

				if (assets.activating_torch_zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && assets.mir_is_already_broken) {
					assets.pressEnter.setPosition(player.sprite.getPosition().x - 70.f, player.sprite.getPosition().y - 45.f);
					window.draw(assets.pressEnter);
				}

				if (assets.bed_zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && !assets.pressFbed_clicked) {
					assets.pressF_bed.setPosition(player.sprite.getPosition().x - 70.f, player.sprite.getPosition().y - 45.f);
					window.draw(assets.pressF_bed);
				}

				if (assets.bed_zone_active && !assets.GOT_KEY) {
					if (assets.bed_zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
						if (assets.bed_zone_clock.getElapsedTime().asSeconds() < 1.5) {
							assets.looking_the_bed.renderTip(window);
						}
						if (assets.bed_zone_clock.getElapsedTime().asSeconds() >= 1.5) {
							assets.looking_the_bed.disableTip();
							for (auto& slot : player.inventory) {
								if (!slot.item) {
									slot.item = std::move(wkey);
									assets.GOT_KEY = true;
									break;
								}
							}
						}
					}
					else {
						assets.bed_zone_active = false;
						assets.pressFbed_clicked = false; 
						assets.take_key_again.emitError();
						assets.bed_zone_clock.restart();
					}
				}

				assets.take_key_again.tickError();
				assets.take_key_again.renderError(window);
				
				assets.mirror_tip.tickTip();
				assets.mirror_tip.renderTip(window);

				// wardope_opened
				if (assets.wardrope_is_opened) {
					assets.check_if_the_book_grabbed();
					window.draw(assets.wardrope_sprite); 
				}
				if (assets.wardrope_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
					assets.E.setPosition(player.sprite.getPosition().x + 30.f, player.sprite.getPosition().y - 45.f);
					window.draw(assets.E);
					if (assets.wardrope_is_opened)
					window.draw(assets.G);
				}
				// wardrope_opened

				if (assets.portal_book_grabbed) {
					if (mysterious_apple) {
						mysterious_apple->hover({ 423.f, 483.f });
						window.draw(mysterious_apple->sprite);
						if (player.sprite.getGlobalBounds().intersects(mysterious_apple->sprite.getGlobalBounds())) {
							for (auto& slot : player.inventory) {
								if (!slot.item) {
									slot.item = std::move(mysterious_apple);
									break;
								}
							}
						}
					}
				}

				// box
				if (assets.hit_counter >= 4) {
					assets.box_one_cur_frame = 4;
					if (heal_potion_one != nullptr) {
						if (player.sprite.getGlobalBounds().intersects(heal_potion_one->sprite.getGlobalBounds())) {
							for (auto& slot : player.inventory) {
								if (slot.item == nullptr) {						
									slot.item = std::move(heal_potion_one);
									break;
								}
							}
						}
						else {
							heal_potion_one->hover({522.f, 317.f});
							window.draw(heal_potion_one->sprite);
						}
					}					
				}
				else {
					assets.box_one_cur_frame = assets.hit_counter;
				}
				assets.box_one_sprite.setTextureRect(IntRect(assets.box_one_cur_frame * assets.box_one_frameWidth, 0, assets.box_one_frameWidth,
					assets.box_one_frameHeight));
				window.draw(assets.box_one_sprite);
				// box

				// chest for sword
				assets.chest_for_sword.update_animation(134, 110, 4);
				window.draw(assets.chest_for_sword.sprite);
				// chest for sword

				window.draw(player.sprite);
				player.draw_health(window);

				// active Item		
				if(player.activeItem)
				player.draw_active_item(window);
				// active Item

				// skelet from library
				if (new_skelet) {
					if (new_skelet->health <= 0) {
						new_skelet->isKilled = true;
					}
					if (new_skelet->isKilled) {
						new_skelet.reset(); 
					}
					else {
						new_skelet->update(dt, player.sprite.getPosition(), player, assets.walls, sword, 100);
						new_skelet->attack(player);
						if (new_skelet->active)
							window.draw(new_skelet->slash_sprite);

						if (new_skelet->trail.isActive)
							window.draw(new_skelet->trail.sprite);

						window.draw(new_skelet->sprite);
						window.draw(new_skelet->health_back_bar);
						window.draw(new_skelet->health_bar);
						window.draw(new_skelet->health_text);
					}
				}		
				// skelet from library

				// skelet from box
				if (assets.hit_counter >= 4) {
					if (assets.skelet.health <= 0) {
						assets.skelet.isKilled = true;
					}
					if (!assets.skelet.isKilled) {
						assets.skelet.update(dt, player.sprite.getPosition(), player, assets.walls, sword, 70);
						assets.skelet.attack(player);
						if (assets.skelet.active) {
							window.draw(assets.skelet.slash_sprite);
						}
						if (assets.skelet.trail.isActive) {
							window.draw(assets.skelet.trail.sprite);
						}
						window.draw(assets.skelet.sprite);
						window.draw(assets.skelet.health_back_bar);
						window.draw(assets.skelet.health_bar);
						window.draw(assets.skelet.health_text);
					}
				}
				// skelet from box

				// mush monsters
				if (!assets.all_is_locked) {
					if (assets.ms1.health > 0) {
						assets.ms1.update(dt, player.sprite.getPosition(), player, assets.walls_for_mushrooms, sword, 50.f);					
					}
					else if (assets.ms1.health <= 0) {
						assets.ms1.health = 0;
						assets.ms1.isKilled = true;
					}

					if (assets.ms2.health > 0) {
						assets.ms2.update(dt, player.sprite.getPosition(), player, assets.walls_for_mushrooms, sword, 50.f);
					}
					else if (assets.ms2.health <= 0) {
						assets.ms2.health = 0;
						assets.ms2.isKilled = true;
					}

					if (assets.torch_activated) {
						if (assets.ms3.health > 0) {
							assets.ms3.update(dt, player.sprite.getPosition(), player, assets.walls_for_mushrooms, sword, 100.f);
						}
						else if (assets.ms3.health <= 0) {
							assets.ms3.health = 0;
							assets.ms3.isKilled = true;
						}

						if (assets.ms4.health > 0) {
							assets.ms4.update(dt, player.sprite.getPosition(), player, assets.walls_for_mushrooms, sword, 100.f);
							assets.l_pos = assets.ms4.sprite.getPosition();
						}
						else if (assets.ms4.health <= 0) {
							assets.ms4.health = 0;
							assets.ms4.isKilled = true;
							if (cursed_mirror != nullptr) {
								if (cursed_mirror->sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
									for (auto& slot : player.inventory) {
										if (!slot.item) {
											slot.item = std::move(cursed_mirror);
											assets.kill_the_boss.emitTip();
											assets.wizard_boss.spawn();
											break;
										}
									}
								}
								else {
									cursed_mirror->hover(assets.l_pos);
									window.draw(cursed_mirror->sprite); // c_mirror upd
								}
							}						
						}
					}
				}		
				if (assets.ms1.health > 0) {
					if (assets.ms1.trail.isActive) {
						window.draw(assets.ms1.trail.sprite);
					}
					window.draw(assets.ms1.sprite);
				}
				if (assets.ms2.health > 0) {
					if (assets.ms2.trail.isActive) {
						window.draw(assets.ms2.trail.sprite);
					}
					window.draw(assets.ms2.sprite);
				}
				if (assets.torch_activated) {
					if (assets.ms3.health > 0) {
						if (assets.ms3.trail.isActive) {
							window.draw(assets.ms3.trail.sprite);
						}
						window.draw(assets.ms3.sprite);
					}
					if (assets.ms4.health > 0) {
						if (assets.ms4.trail.isActive) {
							window.draw(assets.ms4.trail.sprite);
						}
						window.draw(assets.ms4.sprite);
					}
				}
				// mush monsters
				
				if (!assets.all_is_locked) {
					if (assets.wizard_boss.health > 0) {
						assets.wizard_boss.update(dt, player.sprite.getPosition(), player, assets.walls, sword, 500.f);
					}
				}
			
				if (assets.wizard_boss.aaIsActive && !assets.wizard_boss.spawned) {
					window.draw(assets.wizard_boss.aa_sprite);
				}
				if (assets.wizard_boss.health > 0 && assets.wizard_boss.spawned) {	
					if (assets.wizard_boss.trail.isActive) {
						window.draw(assets.wizard_boss.trail.sprite);
					}
					assets.wizard_boss.draw_health(window);
					window.draw(assets.wizard_boss.sprite);
					if(assets.wizard_boss.light_strikeActive)
					window.draw(assets.wizard_boss.ls_sprite);
					if (assets.wizard_boss.fireball != nullptr)
						assets.wizard_boss.fireball->draw_fireball(window);
				}
				else if (assets.wizard_boss.health <= 0 && assets.wizard_boss.spawned) {
					/*assets.wizard_boss.make_tornado(assets.wizard_boss.sprite.getPosition());*/
					assets.wizard_boss.tornado_active = true;
					assets.wizard_boss.isKilled = true;
				}

				if (assets.wizard_boss.tornado_active && !assets.wizard_boss.tornado_was_made) {
					assets.wizard_boss.make_tornado(assets.wizard_boss.sprite.getPosition());
					assets.wizard_boss.tornado_was_made = true;
				}

				if (assets.wizard_boss.tornado_active && assets.wizard_boss.tornado_was_made && !assets.wizard_boss.stop_drawing_tornado) {
					assets.wizard_boss.update_tornado();
					window.draw(assets.wizard_boss.tornado_sprite);
				}

				if (assets.wizard_boss.isKilled) {
					Color barrier_cur_color = assets.barrier_sprite.getColor();
					if (assets.barrier_alpha_clock.getElapsedTime().asSeconds() > 0.02) {
						assets.barrier_alpha--;
						barrier_cur_color.a = assets.barrier_alpha;
						assets.barrier_sprite.setColor(barrier_cur_color);
						assets.barrier_alpha_clock.restart();
					}
					if (assets.barrier_alpha <= 0) {
						assets.barrier_active = false;
					}
				}

				// barrier
				if (assets.barrier_active) {
					assets.update_barrier(window);
				}
				// barrier
				
				// ms health
				if (!assets.ms1.isKilled)
					assets.ms1.draw_health(window);
				if (!assets.ms2.isKilled)
					assets.ms2.draw_health(window);
				if (assets.torch_activated) {
					if (!assets.ms3.isKilled)
						assets.ms3.draw_health(window);
					if (!assets.ms4.isKilled)
						assets.ms4.draw_health(window);
				}				
				// ms health

				// lib event
				if (assets.lib_event_zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
					window.draw(assets.lib_event_txt);
				}
				// lib event

				// bloody screen
				if (assets.bs_active) {
					if (assets.bs_clock.getElapsedTime().asSeconds() > 0.015 && assets.bs_alpha > 0) {
						assets.bs_alpha -= 1;
						sf::Color cur_color = assets.bloody_screen_sprite.getColor();
						cur_color.a = assets.bs_alpha;
						assets.bloody_screen_sprite.setColor(cur_color);
						assets.bs_clock.restart();
					}
					if (assets.bs_alpha <= 0) {
						assets.bs_active = false;
						assets.bs_alpha = 255;
					}
					window.draw(assets.bloody_screen_sprite);
				}
				// bloody screen

				swordEffect.update(player);
				if (swordEffect.isActive()) {
					window.draw(swordEffect.sprite);
				}

				player.updateAllEffects();

				// иконки бафов
				player.updateAllBuffs(player.inventory);
				player.draw_status_bar(mouse_pos, window);
				player.drawEffectsVisuals(window, player);

				if (assets.chest_for_sword.sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && !assets.chest_for_sword.isOpened) {
					window.draw(assets.OpenChest);
				}

				if (assets.barrier_book_zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) &&
					assets.wizard_boss.isKilled && assets.barrier_alpha <= 0 && !assets.portal_book_grabbed) {
					window.draw(assets.press_e_takeabook);
				}

				// chest
				if (assets.chest_for_sword.isOpened) {
					assets.chest_for_sword.draw_items(window);
					item_description.draw_descriptions(assets.chest_for_sword.items, window, mouse_pos, showingItem);
					item_description.draw_armor_chest(assets.chest_for_sword.armor, window, mouse_pos, showingArmor);
				}	

				if (!assets.chest_for_sword.isOpened) {
					assets.ancient_seal_error.tickError();
					assets.ancient_seal_error.renderError(window);
				}
				// chest

				// if book grabbed tip
				assets.look_mirror.tickTip();
				assets.look_mirror.renderTip(window);
				
				// wardrope error
				assets.wardrope_error.tickError();
				assets.wardrope_error.renderError(window);
				// wardrope error
				
				// lib event error
				assets.lib_event_error.tickError();
				assets.lib_event_error.setErrorText("It's not time yet: " + std::to_string(static_cast<int>(assets.lib_event_timer.getElapsedTime().asSeconds())) + "sec / 40sec");
				assets.lib_event_error.renderError(window);
				// lib event error

				// sword error
				assets.lib_sword_error.tickError();
				assets.lib_sword_error.renderError(window);
				// sword error

				assets.kill_the_boss.tickTip();
				assets.kill_the_boss.renderTip(window);

				assets.result_text.tickTip(); // result text
				assets.result_text.renderTip(window);

				if (player.activeItem && player.activeItem->ability) {
					if (auto* read_book = dynamic_cast<ReadBook*>(player.activeItem->ability.get())) {
						read_book->draw(window, player);
					}
				}

				player.updateAllEffects(nullptr, nullptr, nullptr);
				window.draw(pause.pause_button_sprite); // pause button

				// player inventory
				player.draw_inventory(window);
				if (player.armor_slot.hitbox.getGlobalBounds().contains(static_cast<sf::Vector2f>(mouse_pos))) { // если слот брони пустой
					item_description.check_if_slot_empty(player.armor_slot, window);
				}
				item_description.draw_descriptions(player.inventory, window, mouse_pos, showingItem);
				item_description.draw_armor_player(player.armor_slot, window, mouse_pos, showingArmor); // player
				player.draw_cooldowns(window);

				if (temp_holder) {
					window.draw(temp_holder->sprite);
				}
				if (temp_armor) {
					window.draw(temp_armor->sprite);

				}
				// player inventory
				if (player.isDead) {
					tryagain.isActive = true;
					window.draw(tryagain.rectangle);
					window.draw(tryagain.menu_screen_sprite);
					window.draw(tryagain.menu.button_sprite);
					window.draw(tryagain.retry.button_sprite);
				}

				// last_rect
				if (assets.last_rect_active) {
					if (assets.last_rect_clock.getElapsedTime().asSeconds() >= 1.3) {
						last_rect_state = ThirdLocationAssets::FS::FaIn;
						assets.last_rect_clock.restart();
					}
					if (last_rect_state == ThirdLocationAssets::FS::FaIn) {
						if (assets.last_rect_clock.getElapsedTime().asSeconds() >= 0.011 && assets.last_rect_alpha < 255) {
							assets.last_rect_alpha++;
							assets.last_rect.setFillColor(sf::Color(0, 0, 0, assets.last_rect_alpha));
							assets.last_rect_clock.restart();
						}
						if (assets.last_rect_alpha >= 255) {
							last_rect_state = ThirdLocationAssets::FS::Wait;
							assets.last_rect_clock.restart();
						}
					}
					else if (last_rect_state == ThirdLocationAssets::FS::Wait) {
						if (assets.last_rect_clock.getElapsedTime().asSeconds() > 0.2f) {
							assets.last_rect_alpha = 255;
							goToNextLevel(current_location);
							return GameState::Playing;
							last_rect_state = ThirdLocationAssets::FS::FaOut;
						}
					}
					else if (last_rect_state == ThirdLocationAssets::FS::FaOut) {
						assets.last_rect_active = false;
						assets.last_rect_alpha = 0;
						assets.last_rect.setFillColor(sf::Color(0, 0, 0, 0));
					}
				}
				if (assets.last_rect_active) {
					window.draw(assets.last_rect);
				}
				// last_rect
		}
		else {
			window.draw(pause.pause_screen_sprite);
			window.draw(pause.resume.button_sprite);
			window.draw(pause.exit.button_sprite);
			window.draw(pause.settings.button_sprite);
		}
		player.removeDeletedItems();
		window.display();
	}
	return state;
}

GameState second_location(RenderWindow& window, MainPlayer& player, GameState state, sf::Font& font, Locations& current_location, ItemDescription& item_description, Pause& pause, TryAgainMenu& tryagain, const std::map<std::string, Texture>& allItemTextures, ItemManager& item_manager) {
	player.sprite.setPosition(271.f, 604.f);
	player.update_all_(font);
	player.current_health = 100;
	player.maxHP = 115;
	player.restore_items_from_copy();

	SecondLocationAssets assets(font);
	if (!assets.load(font)) {
		return Exit_gs;
	}
	assets.update();
	
	SecondLocationAssets::FS last_rect_state = SecondLocationAssets::Non;
	SecondLocationAssets::FadeRectState fade_rect_state = SecondLocationAssets::None;

	// portal
	Portal portal;
	if (!portal.load()) {
		return Exit_gs;
	}
	else {
		portal.make_portal(font, {335.f,  517.f}, {PORTAL_RIGHT_SCALE}); 
	}
	// portal

	// debug
	// debug

	// golems
	assets.golem1.position = { 1465.f, 375.f };
	assets.golem2.position = { 565.f, 100.f };
	// golems

	// little box
	LittleBox sword_box;
	if (!sword_box.load("textures/chapone/locationtwo/etc/box for sword/little_chest_green.png", "textures/chapone/locationtwo/etc/box for sword/lit_chest_light1.png",
		"textures/chapone/locationtwo/etc/box for sword/lit_chest_light2.png", "textures/chapone/locationtwo/etc/box for sword/lit_chest_light3.png",
		"textures/chapone/locationtwo/etc/box for sword/lit_chest_light4.png") ||
		!sword_box.load_tornado("textures/chapone/locationtwo/etc/box for sword/appearing/one.png", "textures/chapone/locationtwo/etc/box for sword/appearing/two.png",
			"textures/chapone/locationtwo/etc/box for sword/appearing/three.png", "textures/chapone/locationtwo/etc/box for sword/appearing/four.png",
			"textures/chapone/locationtwo/etc/box for sword/appearing/five.png", "textures/chapone/locationtwo/etc/box for sword/appearing/six.png",
			"textures/chapone/locationtwo/etc/box for sword/appearing/seven.png", "textures/chapone/locationtwo/etc/box for sword/appearing/eight.png",
			"textures/chapone/locationtwo/etc/box for sword/appearing/nine.png", "textures/chapone/locationtwo/etc/box for sword/appearing/ten.png",
			"textures/chapone/locationtwo/etc/box for sword/appearing/eleven.png", "textures/chapone/locationtwo/etc/box for sword/appearing/twelve.png",
			"textures/chapone/locationtwo/etc/box for sword/appearing/thirteen.png", "textures/chapone/locationtwo/etc/box for sword/appearing/fourteen.png",
			"textures/chapone/locationtwo/etc/box for sword/appearing/fifteen.png", "textures/chapone/locationtwo/etc/box for sword/appearing/sixteen.png",
			"textures/chapone/locationtwo/etc/box for sword/appearing/seventeen.png", "textures/chapone/locationtwo/etc/box for sword/appearing/eighteen.png")) {
		return Exit_gs;
	}
	sword_box.make_box({ 585.f, 143.f });

	LittleBox fern_box;
	if (!fern_box.load("textures/chapone/locationtwo/etc/little_chest/little_chest.png", "textures/chapone/locationtwo/etc/little_chest/lit_chest_light1.png",
		"textures/chapone/locationtwo/etc/little_chest/lit_chest_light2.png", "textures/chapone/locationtwo/etc/little_chest/lit_chest_light3.png",
		"textures/chapone/locationtwo/etc/little_chest/lit_chest_light4.png")) {
		return Exit_gs;
	}
	fern_box.make_box({ 344.f, 584.f });
	// litlle box

	// fonts
	assets.apply_font(font);
	// fonts

	for (auto& chest : assets.chests) {
		chest.get().initArmor();
		chest.get().init_items_if_needed();
		chest.get().update();
	}
	
	// Items
	
	// fern_leaf
	unique_ptr<Item> fern_leaf = item_manager.items[8]->clone();
	// fern_leaf

	unique_ptr<Item> ancient_seal = item_manager.items[11]->clone();

	unique_ptr<Armor> shadow_armor = item_manager.armor[0]->clone();
	assets.chest_two.armor[4].armor = std::move(shadow_armor);

	// bloody crystal
	unique_ptr<Item> bloody_crsytal = item_manager.items[1]->clone();
	assets.chest_one.items[0].item = std::move(bloody_crsytal);
	// bloody crystal 

	// edge of daarkness
	unique_ptr<Item> edge_of_darkness = item_manager.items[2]->clone();
	assets.chest_one.items[1].item = std::move(edge_of_darkness);
	// edge of darkness

	// heal
	unique_ptr<Item> heal_potion = item_manager.items[3]->clone();
	assets.chest_one.items[2].item = std::move(heal_potion);

	unique_ptr<Item> heal_potion2 = item_manager.items[3]->clone();
	assets.stone_chest.items[5].item = std::move(heal_potion2);
	// heal

	// memory_sotone
	unique_ptr<Item> memory_stone = item_manager.items[4]->clone();
	assets.chest_two.items[6].item = std::move(memory_stone);
	// heal

	// fog mask
	unique_ptr<Item> fog_mask = item_manager.items[5]->clone();
	//assets.chest_two.items[1].item = std::move(fog_mask);
	player.inventory[1].item = std::move(fog_mask);
	// fog mask

	// ice amulet
	unique_ptr<Item> ice_amulet = item_manager.items[6]->clone();
	assets.chest_two.items[3].item = std::move(ice_amulet);
	// ice amulet

	// torch scroll
	unique_ptr<Item> torch_scroll = item_manager.items[7]->clone();
	assets.stone_chest.items[1].item = std::move(torch_scroll);
	// torch scroll

	// Items
	Armor* showingArmor = nullptr;
	unique_ptr<Armor> temp_armor = nullptr;
	ArmorSlot* oldArmor = nullptr;

	Item* showingItem = nullptr; // предмет, чьё описание показывается
	unique_ptr<Item> temp_holder = nullptr; // временно
	sf::Vector2f back_pos; // позиция предмета, который мы переместил
	InventorySlot* oldSlot = nullptr;

	// open Chest text
	assets.OpenChest.setFont(font);
	assets.OpenChest.setFillColor(sf::Color::White);
	assets.OpenChest.setCharacterSize(25);
	assets.OpenChest.setString("Press [E] to open the chest");
	// open Chest text

	// chest text
	for (auto& chest : assets.chests) {
		chest.get().set_text_settings(font);
	}
	// chest text

	// sword effect
	SwordSlashEffect swordEffect; 
	// sword effect

	sf::Clock deltaClock; // for moving

	while (window.isOpen()) {
		Event event;
		player.upd_item_ptr();

		assets.show_pressE_ONE = false;
		assets.show_pressE_TWO = false;
		assets.show_pressE_THREE = false;

		// Загружаем эффект меча только при смене активного меча
		static Item* lastSword = nullptr; // статическая перемененая
		if (player.activeItem != lastSword && player.activeItem) {
			Sword* sword = dynamic_cast<Sword*>(player.activeItem);
			if (sword) {
				swordEffect.loadFromItem(sword);
				lastSword = player.activeItem;
			}
		}
		
		Sword* sword = dynamic_cast<Sword*>(player.activeItem);

		sf::Vector2i mouse_pos = Mouse::getPosition(window); // !!!!!
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) {
				window.close();
			}
			if (event.type == Event::KeyPressed) {
				for (auto& chest : assets.chests) {
					if (event.key.code == Keyboard::Escape) {
						if (player.menu_is_active) {
							player.disableMenu(assets.all_is_locked);
						}
						if (chest.get().isOpened) {
							chest.get().close_chest(assets.all_is_locked);
						}
					}
				}
				if (event.key.code == Keyboard::Enter && portal.portal_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
					assets.all_is_locked = true;
					assets.last_rect_active = true;
					assets.LevelCompleted = true;
					player.cur_health_copy = player.current_health;
					player.copy_items_for_newlvl();
				}
				if (event.key.code == Keyboard::Z) {
					std::cout << player.activeItem << std::endl;
				}
				if (event.key.code == Keyboard::X) {
					for (auto& slot : player.inventory) {
						std::cout << slot.item << std::endl;
					}
				}
				player.useActiveItem(event, assets.all_is_locked, font, assets.walls);
				if (!assets.all_is_locked) { 
					if (event.key.code == Keyboard::Space && player.activeItem && !player.activeItem->ability) {
						if (player.activeItem->name == "memory_stone" && assets.statue_text_2_activated
							&& assets.statue_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
							assets.statue_sprite.setTexture(assets.statue_texture2);
							assets.statue_got_stone = true;
							player.activeItem->toDeleteEnable();
						}
						if (player.activeItem->name == "bloody_crystal" && assets.statue_got_stone 
							&& !assets.boiler_active && assets.boiler_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
							int current_player_hp = (float(player.current_health) / player.maxHP) * 100;
							int cur_dmg = player.maxHP * 0.2; // -20% health
							if (current_player_hp >= 35) {
								player.current_health -= cur_dmg;
								assets.boiler_active = true;
								assets.bs_active = true;
								player.activeItem->toDeleteEnable();
							}
							else {
								assets.boiler_error.emitError();
							}
						}
						if (player.activeItem->name == "fern_leaf" && assets.statue_got_stone
							&& assets.statue_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && assets.boiler_active) {
							player.activeItem->toDeleteEnable();
							assets.statue_got_leaf = true;
							assets.got_fern = false;
						}
					}
				}
				if (event.key.code >= Keyboard::Num1 && event.key.code <= Keyboard::Num9) {
					player.selectedSlot = event.key.code - Keyboard::Num1; 
				}
				if (event.key.code == Keyboard::Tab) {
					player.activateMenu(assets.all_is_locked);
				}
				if (event.key.code == Keyboard::E) {
					for (size_t i = 0; i < assets.chest_zones.size(); ++i) {
						if (player.sprite.getGlobalBounds().intersects(assets.chest_zones[i].getGlobalBounds())) {
							assets.chests[i].get().open_chest(assets.all_is_locked);
							break;
						}
					}
				}
				if (event.key.code == Keyboard::E) {
					if (!assets.find_scroll_activated && assets.statue_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())
						&& assets.isTorchScrollSeen) {
						assets.find_scroll_activated = true;
					}
					if (assets.alpha_changed && !assets.statue_text_2_activated && assets.statue_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && !assets.alpha_changed2) {
						assets.statue_text_2_activated = true;
					}
					if (!assets.statue_text3_activated && assets.alpha_changed2 && assets.statue_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && assets.fade_rect_alpha <= 0) {
						assets.statue_text3_activated = true;
						assets.statue_text3.emitTip();
					}
				}

				if (event.key.code == Keyboard::G && assets.find_scroll_activated && !assets.golemFight) {
					for (auto& it : assets.torch_zones) {
						if (std::get<1>(it.second).zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && std::get<1>(it.second).activated == false) {
							assets.currentTorch = it.first;
							// если ещё не начинали цепочку, то запускаем
							if (!assets.torchSequenceActive) {
								assets.torchSequenceActive = true;
							}

							if (assets.currentTorchIndex < assets.torchOrder.size() &&
								assets.currentTorch == assets.torchOrder[assets.currentTorchIndex])
							{
								int torchKey = assets.torchOrder[assets.currentTorchIndex];
								auto& current_tuple = assets.torch_zones[torchKey];

								std::get<1>(current_tuple).activated = true;
								std::get<1>(current_tuple).shouldDraw = true;
								std::get<3>(current_tuple).setTexture(std::get<2>(current_tuple));
								std::get<3>(current_tuple).setPosition(std::get<0>(current_tuple));

								std::get<1>(current_tuple).ts = SecondLocationAssets::TorchState::FadeIN;
								std::get<1>(current_tuple).torch_alpha = 0;

								assets.currentTorchIndex++;

								// проверка на завершение
								if (assets.currentTorchIndex == assets.torchOrder.size()) {
									assets.torchSequenceActive = false;

								}
							}
							else {
								for (auto& zone : assets.torch_zones) {
									std::get<1>(zone.second).activated = false;
									std::get<1>(zone.second).shouldDraw = false;
								}
								assets.currentTorchIndex = 0;
								assets.torchSequenceActive = false;
							}
							break; // важный break, чтобы один факел нажатия G не триггерил все сразу
						}
					}
				}
			}

			player.updateAllEffects(&event, &assets.isTorchScrollSeen, &assets.all_is_locked); // для свитков и карт и тд!!

			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
				bool inventoryClicked = false;
				player.drag_item(window, mouse_pos, inventoryClicked, assets.all_is_locked, assets.chests, temp_armor, temp_holder, back_pos, oldSlot, oldArmor);
				if (!assets.all_is_locked && !inventoryClicked) {					
					Sword* sword = dynamic_cast<Sword*>(player.activeItem);
					if (player.activeItem && sword) {
						if (swordEffect.attack_delay.getElapsedTime().asSeconds() >= sword->attack_cooldown) {
							swordEffect.activate(player);
							swordEffect.attack_delay.restart();
							if (swordEffect.isActive()) {	
								assets.golem1.get_damage(swordEffect, sword, player);
								assets.golem2.get_damage(swordEffect, sword, player);
							}
						}
					}
				}

				if (pause.pause_button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && !pause.isPaused
					&& !player.isDead && !inventoryClicked) {
					pause.isPaused = true;
				}
				if (pause.resume.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.resume.to_true();
					pause.resume.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (pause.settings.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.settings.to_true();
					pause.settings.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (pause.exit.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.exit.to_true();
					pause.exit.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && tryagain.retry.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) &&
					tryagain.isActive) {
					tryagain.retry.to_true();
					tryagain.retry.setClickedTexture();
					tryagain.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && tryagain.menu.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) &&
					tryagain.isActive) {
					tryagain.menu.to_true();
					tryagain.menu.setClickedTexture();
					tryagain.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && pause.resume.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.resume.to_true();
					pause.resume.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && pause.settings.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.settings.to_true();
					pause.settings.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && pause.exit.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.exit.to_true();
					pause.exit.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && pause.pause_button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && !pause.isPaused
					&& !player.isDead) {;
					pause.isPaused = true;
				}
			}
			player.drop_item(event, assets.chests, temp_armor, temp_holder, back_pos, oldSlot, oldArmor);
		}

		if (tryagain.retry.get_clicked() && tryagain.button_clock.getElapsedTime().asSeconds() > 0.15) {
			tryagain.retry.setTexture();
			tryagain.retry.to_false();
			// bool reset
			tryagain.isActive = false;
			if (player.isDead) {
				player.current_health = player.cur_health_copy;
				player.copy_items_ifdead();
			}
			// bool reset
			return Playing;
		}
		if (tryagain.menu.get_clicked() && tryagain.button_clock.getElapsedTime().asSeconds() > 0.15) {
			tryagain.menu.setTexture();
			tryagain.menu.to_false();
			tryagain.isActive = false;
			pause.isPaused = false;
			return Menu_gs;
		}
		if (pause.resume.get_clicked() && pause.button_clock.getElapsedTime().asSeconds() > 0.15) {
			pause.resume.setTexture();
			pause.resume.to_false();
			pause.isPaused = false;
		}
		if (pause.settings.get_clicked() && pause.button_clock.getElapsedTime().asSeconds() > 0.15) {
			pause.settings.setTexture();
			pause.settings.to_false();
		}
		if (pause.exit.get_clicked() && pause.button_clock.getElapsedTime().asSeconds() > 0.15) {
			pause.exit.setTexture();
			pause.exit.to_false();
			tryagain.isActive = false;
			pause.isPaused = false;
			return Menu_gs;
		}

		assets.OpenChest.setPosition(player.sprite.getPosition().x - 30.f, player.sprite.getPosition().y - 45.f);
		assets.pressG.setPosition(player.sprite.getPosition().x - 60.f, player.sprite.getPosition().y - 45.f);

		if (temp_holder && temp_holder->isDraggin) {
			temp_holder->sprite.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)) + temp_holder->offset);
		}
		if (temp_armor && temp_armor->isDraggin) {
			temp_armor->sprite.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)) + temp_armor->offset);
		}

		float dt = deltaClock.restart().asSeconds();

		if (!pause.isPaused) {
			player.moving = false;
			sf::Vector2f move(0.f, 0.f);
			player.handleMovementInput(assets.all_is_locked, move, dt);

			// коллизии 
			player.sprite.move(move.x, 0.f); // x
			for (auto& wall : assets.walls) {
				if (player.sprite.getGlobalBounds().intersects(wall.getGlobalBounds())) {
					player.sprite.move(-move.x, 0.f);
					break;
				}
			}
			for (auto& chest : assets.chests) {
				if (player.sprite.getGlobalBounds().intersects(chest.get().sprite.getGlobalBounds())) {
					player.sprite.move(-move.x, 0.f);
					break;
				}
			}
			if (player.sprite.getGlobalBounds().intersects(assets.boiler_zone.getGlobalBounds()) && assets.statue_got_stone) {
				player.sprite.move(-move.x, 0.f);
			}

			player.sprite.move(0.f, move.y); // y
			for (auto& wall : assets.walls) {
				if (player.sprite.getGlobalBounds().intersects(wall.getGlobalBounds())) {
					player.sprite.move(0.f, -move.y);
					break;
				}
			}
			for (auto& chest : assets.chests) {
				if (player.sprite.getGlobalBounds().intersects(chest.get().sprite.getGlobalBounds())) {
					player.sprite.move(0.f, -move.y);
					break;
				}
			}
			if (player.sprite.getGlobalBounds().intersects(assets.boiler_zone.getGlobalBounds()) && assets.statue_got_stone) {
				player.sprite.move(0.f, -move.y);
			}
			// коллизии

			assets.golem1.update_animation(player);
			assets.golem2.update_animation(player);
			// sword_effect
			swordEffect.update(player);
			// sword_effect
			window.clear();
			window.draw(assets.bgSprite);

			if (portal.portal_should_activate) portal.draw(player, window);
;
			for (auto& torch : assets.torch_zones) { // линии для факелов
				auto& current_torch = torch.second;
				auto color = std::get<3>(current_torch).getColor();
				if (std::get<1>(current_torch).shouldDraw && std::get<1>(current_torch).activated) {
					if (std::get<1>(current_torch).ts == SecondLocationAssets::TorchState::FadeIN) {
						if (std::get<1>(current_torch).torch_alpha < 255 && std::get<1>(current_torch).torch_aplha_clock.getElapsedTime().asSeconds() > 0.002) {
							color.a = std::get<1>(current_torch).torch_alpha;
							std::get<3>(current_torch).setColor(color);
							std::get<1>(current_torch).torch_alpha += 2;
							std::get<1>(current_torch).torch_aplha_clock.restart();
						}
						if (std::get<1>(current_torch).torch_alpha >= 255) {
							std::get<1>(current_torch).ts = SecondLocationAssets::TorchState::Hold;
							std::get<1>(current_torch).torch_live_clock.restart();
						}
					}
					if (std::get<1>(current_torch).ts == SecondLocationAssets::TorchState::Hold) {
						if (std::get<1>(current_torch).torch_live_clock.getElapsedTime().asSeconds() >= 1) {
							std::get<1>(current_torch).ts = SecondLocationAssets::TorchState::FadeOUT;
							std::get<1>(current_torch).torch_aplha_clock.restart();
						}
					}
					if (std::get<1>(current_torch).ts == SecondLocationAssets::TorchState::FadeOUT) {
						if (std::get<1>(current_torch).torch_alpha > 0 && std::get<1>(current_torch).torch_aplha_clock.getElapsedTime().asSeconds() > 0.002) {
							color.a = std::get<1>(current_torch).torch_alpha;
							std::get<3>(current_torch).setColor(color);
							std::get<1>(current_torch).torch_alpha -= 2;
							std::get<1>(current_torch).torch_aplha_clock.restart();
						}
					}
					window.draw(std::get<3>(torch.second));
				}
			}

			if (assets.golem1.health <= 0 && assets.golem2.health <= 0) {
				assets.golemFight = false;
			}

			// box
			if (assets.golem1.health <= 0 && assets.golem2.health <= 0) {
				fern_box.update();
				fern_box.draw(window);
				if (fern_box.box_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
					// убрать сундук
					fern_box.remove();
					// найти свободный слот
					for (size_t i = 0; i < player.inventory.size(); i++) {
						if (player.inventory[i].item == nullptr) {
							player.inventory[i].item = std::move(fern_leaf); 
							assets.got_fern = true;
							break;
						}
					}
				}
			}

			// boiler error
			assets.boiler_error.tickError();
			assets.boiler_error.renderError(window);
			// boiler error

			// boiler
			if (assets.statue_got_stone && assets.boiler_active) {
				if (assets.boiler_timer.getElapsedTime().asSeconds() > 0.15) {
					if (assets.forward) {
						assets.current_boiler_frame++;
						if (assets.current_boiler_frame >= 4) {
							assets.current_boiler_frame = 4;
							assets.forward = false;
						}
					}
					else {
						assets.current_boiler_frame--;
						if (assets.current_boiler_frame <= 2) {
							assets.current_boiler_frame = 2;
							assets.forward = true;
						}
					}

					assets.boiler_sprite.setTexture(assets.boiler_textures[assets.current_boiler_frame]);
					assets.boiler_timer.restart();
				}
			}
			if (assets.statue_got_stone) {
				window.draw(assets.boiler_sprite);
			}
			// boiler

			 // sword_box
			if (assets.statue_text3_activated && assets.statue_text3.tip_alpha <= 0) {
				sword_box.tornado_active = true;
			}

			if (sword_box.tornado_active && !sword_box.tornado_was_made) {
				sword_box.make_tornado({ 541.f, 96.f });
				sword_box.tornado_was_made = true;
			}

			if (sword_box.tornado_active && sword_box.tornado_was_made && !sword_box.stop_drawing_tornado) {
				sword_box.update_tornado();
				sword_box.draw_tornado(window);
			}

			if (sword_box.stop_drawing_tornado) {
				sword_box.update();
				sword_box.draw(window);

				if (sword_box.box_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
						// убрать сундук
					sword_box.remove();
						// найти свободный слот
					for (size_t i = 0; i < player.inventory.size(); i++) {
						if (player.inventory[i].item == nullptr) {
							player.inventory[i].item = std::move(ancient_seal);
							assets.got_seal = true;
							portal.portal_should_activate = true;
							break;
						}
					}
				}
			}
			
			// sword_box
			window.draw(player.sprite);
			player.draw_health(window);

			for (auto& chest : assets.chests) {
				window.draw(chest.get().sprite);
			}

			// active slot		
			if (player.activeItem) {
				player.draw_active_item(window);
			}
			// active slot

			if (!assets.golemFight) {
				if (player.sprite.getPosition().y < assets.statue_sprite.getPosition().y + 75.f) {
					if (swordEffect.isActive()) {
						window.draw(swordEffect.sprite);
					}
					window.draw(assets.statue_sprite); // statue
				}
				else {
					window.draw(assets.statue_sprite); // statue
					if (swordEffect.isActive()) {
						window.draw(swordEffect.sprite);
					}
				}
			}
			else if (assets.golemFight) {
				window.draw(assets.statue_sprite);
				if (assets.golem1.health > 0) {			
					// обновление логики
					if (!assets.all_is_locked) {
						assets.golem1.update(dt, sf::Vector2f(player.sprite.getPosition()), player, assets.walls_for_evil, sword, 80.f);
						assets.golem1.attack(player);
						if (assets.golem1.active) {
							window.draw(assets.golem1.slash_sprite);
						}
					}
					window.draw(assets.golem1.sprite);
					window.draw(assets.golem1.health_back_bar);
					window.draw(assets.golem1.health_bar);
					window.draw(assets.golem1.health_text);
				}
				else if (assets.golem1.health <= 0) {
					assets.golem1.health = 0;
					assets.golem1.isKilled = true;
				}

				if (assets.golem2.health > 0) {
					if (!assets.all_is_locked) {
						assets.golem2.update(dt, sf::Vector2f(player.sprite.getPosition()), player, assets.walls_for_evil, sword, 80.f);
						assets.golem2.attack(player);
						if (assets.golem2.active) {
							window.draw(assets.golem2.slash_sprite);
						}
					}
					window.draw(assets.golem2.sprite);
					window.draw(assets.golem2.health_back_bar);
					window.draw(assets.golem2.health_bar);
					window.draw(assets.golem2.health_text);
				}
				else if (assets.golem2.health <= 0) {
					assets.golem2.health = 0;
					assets.golem2.isKilled = true;
				}
				if (swordEffect.isActive()) {
					window.draw(swordEffect.sprite);
				}
			}

			// chests
			for (int i = 0; i < 3; i++) {
				if (player.sprite.getGlobalBounds().intersects(assets.chest_zones[i].getGlobalBounds()) && !assets.chests[i].get().isOpened) {
					window.draw(assets.OpenChest);
				}
			}

			for (auto& chest : assets.chests) {
				if (chest.get().isOpened) {
					chest.get().draw_items(window);
				}
			}
			// chests

			player.draw_status_bar(mouse_pos, window);
			player.drawEffectsVisuals(window, player); // рисуем эффекты до мини-описания предмета

			
			for (auto& chest : assets.chests) {
				if (chest.get().isOpened) {
					item_description.draw_descriptions(chest.get().items, window, mouse_pos, showingItem);
					item_description.draw_armor_chest(chest.get().armor, window, mouse_pos, showingArmor);
				}
			}

			if (assets.find_scroll_activated && !assets.golemFight) {
				for (auto& torch : assets.torch_zones) { // текст в зоне факела
					if (std::get<1>(torch.second).zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && !std::get<1>(torch.second).activated) {
						window.draw(assets.pressG);
					}
				}
			}

			if (assets.currentTorchIndex == 5 && assets.isTorchScrollSeen) {
				for (auto& zone : assets.torch_zones) {
					std::get<1>(zone.second).activated = true;
					std::get<1>(zone.second).shouldDraw = false;
				}
				assets.isTorchScrollSeen = false;
				assets.golemFight = true;
			}
			if (assets.statue_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && !assets.torch_activated && assets.find_scroll_activated
				&& !assets.golem1.isKilled && !assets.golem2.isKilled) {
				/*window.draw(assets.statue_text1);*/
				window.draw(assets.statue_text1.tip_text);
			}
			else if (assets.statue_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && !assets.torch_activated && !assets.find_scroll_activated && assets.isTorchScrollSeen) {
				assets.show_pressE_ONE = true;
			}
			player.updateAllEffects(nullptr, &assets.isTorchScrollSeen, &assets.all_is_locked);
			window.draw(pause.pause_button_sprite); // pause button
			
			// tips		
			 // tip #1
			assets.find_scroll.tickTip();
			if (assets.find_scroll.tip_alpha <= 0 && assets.find_scroll_activated) {
				assets.show_pressE_ONE = false;
			}
			assets.find_scroll.renderTip(window);
			// tip #1

			// tip #2
			if (assets.isTorchScrollSeen && !assets.find_scroll_activated) {
				assets.talk_to_statue.tickTip();
				assets.talk_to_statue.renderTip(window);
			}
			// tip #2

			// tip #3
			if (assets.got_fern && !assets.alpha_changed) {
				assets.talk_to_statue.resetAlpha();
				assets.alpha_changed = true;
			}
			if (assets.got_fern && assets.alpha_changed) {
				assets.talk_to_statue.tickTip();
				assets.talk_to_statue.renderTip(window);
			}
			if (assets.statue_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && assets.got_fern && assets.alpha_changed && !assets.statue_text_2_activated &&
				!assets.statue_text3_activated && !assets.statue_got_stone) {
				assets.show_pressE_TWO = true;
			}
			else {
				assets.show_pressE_TWO = false;
			}
			if (assets.statue_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && assets.statue_text_2_activated && !assets.boiler_error.isEnabled()) {
				window.draw(assets.statue_text2.tip_text);
			}
			// tip #3

			// tip #4 
			if (assets.fade_rect_used && assets.alpha_changed2) {
				assets.talk_to_statue.tickTip();
				assets.talk_to_statue.renderTip(window);
			}
			if (assets.statue_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds()) && !assets.got_seal && assets.alpha_changed2 && !assets.statue_text3_activated && assets.talk_to_statue.tip_alpha <= 0) {
				assets.show_pressE_THREE = true;
			}
			if (assets.statue_text3_activated && assets.alpha_changed2) {
				assets.statue_text3.tickTip();
				assets.statue_text3.renderTip(window);
			}
			// tip #4

			 if (assets.show_pressE_ONE || assets.show_pressE_TWO || assets.show_pressE_THREE) {
				 window.draw(assets.pressE);
			 }
		    // tips
			 
			 // player inventory
			 player.draw_inventory(window);
			 if (player.armor_slot.hitbox.getGlobalBounds().contains(static_cast<sf::Vector2f>(mouse_pos))) { // если слот брони пустой
				 item_description.check_if_slot_empty(player.armor_slot, window);
			 }
			 item_description.draw_descriptions(player.inventory, window, mouse_pos, showingItem);
			 item_description.draw_armor_player(player.armor_slot, window, mouse_pos, showingArmor); // player
			 player.draw_cooldowns(window);

			 if (temp_holder) {
				 window.draw(temp_holder->sprite);
			 }
			 if (temp_armor) {
				 window.draw(temp_armor->sprite);

			 }
			 // player inventory

			 // bloody screen
			 if (assets.bs_active) {
				 if (assets.bs_clock.getElapsedTime().asSeconds() > 0.015 && assets.bs_alpha > 0) {
					 assets.bs_alpha -= 1;
					 sf::Color cur_color = assets.bloody_screen_sprite.getColor();
					 cur_color.a = assets.bs_alpha;
					 assets.bloody_screen_sprite.setColor(cur_color);
					 assets.bs_clock.restart();
				 }
				 if (assets.bs_alpha <= 0) {
					 assets.bs_active = false;
				 }
				 window.draw(assets.bloody_screen_sprite);
			 }
			 // bloody screen

			 // last_rect
			 if (assets.last_rect_active) {
				 if (assets.last_rect_clock.getElapsedTime().asSeconds() >= 1.3) {
					 last_rect_state = SecondLocationAssets::FaIn;
					 assets.last_rect_clock.restart();
				 }
				 if (last_rect_state == SecondLocationAssets::FaIn) {
					 if (assets.last_rect_clock.getElapsedTime().asSeconds() >= 0.011 && assets.last_rect_alpha < 255) {
						 assets.last_rect_alpha++;
						 assets.last_rect.setFillColor(sf::Color(0, 0, 0, assets.last_rect_alpha));
						 assets.last_rect_clock.restart();
					 }
					 if (assets.last_rect_alpha >= 255) {
						 last_rect_state = SecondLocationAssets::Wait;
						 assets.last_rect_clock.restart();
					 }
				 }
				 else if (last_rect_state == SecondLocationAssets::Wait) {
					 if (assets.last_rect_clock.getElapsedTime().asSeconds() > 0.2f) {
						 assets.last_rect_alpha = 255;
						 goToNextLevel(current_location);
						 return GameState::Playing;
						 last_rect_state = SecondLocationAssets::FaOut;
					 }
				 }
				 else if (last_rect_state == SecondLocationAssets::FaOut) {
					 assets.last_rect_active = false;
					 assets.last_rect_alpha = 0;
					 assets.last_rect.setFillColor(sf::Color(0, 0, 0, 0));
				 }
			 }
			 if (assets.last_rect_active) {
				 window.draw(assets.last_rect);
			 }
			 // last_rect

			 // fade rect
			 if (assets.statue_got_leaf && !assets.fade_rect_active) {
				 if (assets.fade_rect_clock.getElapsedTime().asSeconds() >= 1.3) {
					 fade_rect_state = SecondLocationAssets::FadingIn;
					 assets.fade_rect_active = true;
					 assets.fade_rect_clock.restart();
				 }
			 }
			 if (fade_rect_state == SecondLocationAssets::FadingIn) {
				 if (assets.fade_rect_clock.getElapsedTime().asSeconds() >= 0.011 && assets.fade_rect_alpha < 255) {
					 assets.fade_rect_alpha++;
					 assets.fade_rect.setFillColor(sf::Color(25, 110, 4, assets.fade_rect_alpha));
					 assets.fade_rect_clock.restart();
				 }
				 if (assets.fade_rect_alpha >= 255) {
					 fade_rect_state = SecondLocationAssets::Waiting;
					 assets.statue_active = true;
					 assets.boiler_active = false;
					 assets.statue_got_stone = false;
					 assets.fade_rect_clock.restart(); 
				 }
			 }
			 if (fade_rect_state == SecondLocationAssets::Waiting) {
				 assets.fade_rect_alpha = 255;	
				 assets.statue_text2.disableTip();
				 assets.statue_text_2_activated = false;
				 if (assets.fade_rect_clock.getElapsedTime().asSeconds() >= 1.3) {
					 fade_rect_state = SecondLocationAssets::FadingOut;
				 }
			 }
			 if (fade_rect_state == SecondLocationAssets::FadingOut) {
				 if (assets.fade_rect_clock.getElapsedTime().asSeconds() >= 0.011) {
					 assets.fade_rect_alpha--;
					 assets.fade_rect.setFillColor(sf::Color(25, 110, 4, assets.fade_rect_alpha));
					 assets.fade_rect_clock.restart();
				 }
				 if (assets.fade_rect_alpha <= 0) {
					 assets.fade_rect_active = false;
					 assets.statue_got_leaf = false;
					 assets.fade_rect_used = true;
				 }
				 if (assets.fade_rect_alpha <= 200 && assets.fade_rect_alpha > 150) {
					 if (!assets.alpha_changed2) {
						 assets.talk_to_statue.tip_alpha = 255;
						 assets.talk_to_statue.emitTip();
						 assets.alpha_changed2 = true;
					 }
				 }
			 }

			 if (assets.fade_rect_active) {
				 window.draw(assets.fade_rect); 
			 }
			 // fade rect

			 // statue
			 if (assets.statue_active) {
				 assets.statue_sprite.setPosition(856.f, 58.f);
				 if (assets.statue_clock.getElapsedTime().asSeconds() > 0.13) {
					 if (assets.statue_forward) {
						 assets.current_st_frame++;
						 if (assets.current_st_frame >= 4) {
							 assets.current_st_frame = 4;
							 assets.statue_forward = false;
						 }
					 }
					 else {
						 assets.current_st_frame--;
						 if (assets.current_st_frame <= 0) {
							 assets.current_st_frame = 0;
							 assets.statue_forward = true;
						 }
					 }
					 assets.bgSprite.setTexture(assets.bg_texts_activated[assets.current_st_frame]);
					 assets.statue_sprite.setTexture(assets.statue_texts_activated[assets.current_st_frame], true);
					 assets.statue_clock.restart();
				 }
			 }
			 // statue

			 // crits
			 swordEffect.draw_crits(window);
			 // crits
			 
			// for debug
		
			// for debug

			if (player.isDead) {
				tryagain.isActive = true;
				window.draw(tryagain.rectangle);
				window.draw(tryagain.menu_screen_sprite);
				window.draw(tryagain.menu.button_sprite);
				window.draw(tryagain.retry.button_sprite);
			}
		}
		else {
			window.draw(pause.pause_screen_sprite);
			window.draw(pause.resume.button_sprite);
			window.draw(pause.exit.button_sprite);
			window.draw(pause.settings.button_sprite);
		}
		player.removeDeletedItems();
		window.display();
	}
	return state;
}

GameState first_location(RenderWindow& window, MainPlayer& player, GameState state, sf::Font& font, Locations &current_location, ItemDescription& item_description, Pause& pause, TryAgainMenu& tryagain, const std::map<std::string, Texture>& allItemTextures, ItemManager &item_manager) {
	FirstLocationAssets assets(font); // first level assets
	if (!assets.load()) {
		return Exit_gs;
	}
	assets.update();
	assets.makeSkulls(item_manager);
	assets.esc.setFont(font);

	player.sprite.setPosition(850, 440);
	player.update_all_(font);
	player.restore_items_from_copy();

	player.maxHP = 100;
	player.current_health = 100;
	pause.isPaused = false;

	// portal
	Portal portal;
	if (!portal.load()) {
		return Exit_gs;
	}
	else {
		portal.make_portal(font, { 1302.f, 233.f }, { PORTAL_LEFT_SCALE });
	}
	// portal


	// Items
	Armor* showingArmor = nullptr;
	unique_ptr<Armor> temp_armor = nullptr;
	ArmorSlot* oldArmor = nullptr;

	Item* showingItem = nullptr; // предмет, чьё описание показывается
	unique_ptr<Item> temp_holder = nullptr; // временно
	sf::Vector2f back_pos; // позиция предмета, который мы переместил
	InventorySlot* oldSlot = nullptr;

	// health player
	player.current_health = 100;
	// health player

	// skelet
	Skeleton skelet(21, 50, 50.f, 2.5f, { 640.f, 145.f }); // dmg, health, speed, attack delay
	skelet.loadTexture("textures/chapone/evil/skeleton/skelfront_right.png", "textures/chapone/evil/skeleton/skelfront_left.png", "textures/chapone/evil/skeleton/skelback_right.png",
		"textures/chapone/evil/skeleton/skelback_left.png", "textures/chapone/evil/skeleton/skelleft_left.png", "textures/chapone/evil/skeleton/skelleft_right.png", 
		"textures/chapone/evil/skeleton/skelright_right.png", "textures/chapone/evil/skeleton/skelright_left.png", font);
	skelet.setFrontLeftTexture();
	skelet.sprite.setPosition(skelet.position);
	// skelet

	// bowl
	assets.pressG.setFont(font);
	assets.pressG.setString("Press [G] to put the skull");
	assets.pressG.setFillColor(Color::White);
	assets.pressG.setCharacterSize(24);
	assets.bowl_skull_counter = 0;
	sf::RectangleShape bowl_zone(sf::Vector2f(86.f, 70.f));
	bowl_zone.setPosition(1244, 848);
	// bowl

	//sword slash
	SwordSlashEffect swordEffect;
	//sword slash

	// skulls & counter
	assets.skull_counter = 0;
	assets.skulls_counter_text.setFont(font);
	assets.skulls_counter_text.setStyle(sf::Text::Bold);
	assets.skulls_counter_text.setPosition(1660.f, 125.f);
	assets.skulls_counter_text.setCharacterSize(64);

	assets.E.setFont(font);
	assets.E.setString("[E]");
	assets.E.setCharacterSize(24);
	assets.E.setFillColor(sf::Color::White);
	// skulls & counter

	// map & table
	sf::Vector2f table_zone_size(170.f, 170.f);
	assets.table_zone.setSize(table_zone_size);
	assets.table_zone.setFillColor(sf::Color(255.f, 255.f, 255.f, 0.f));
	assets.table_zone.setPosition(935.f, 822.f);

	assets.map_task.setFont(font);
	assets.map_task.setString("Find 3 skulls to unlock the door");
	assets.map_task.setPosition(590.f, 900.f);
	assets.map_task.setCharacterSize(64);
	assets.map_task.setFillColor(sf::Color::White);

	assets.pressF.setFont(font);
	assets.pressF.setString("Press [F] to open the map.");
	assets.pressF.setCharacterSize(24);
	assets.pressF.setFillColor(sf::Color::White);

	assets.showingmap = false;
	assets.mapSprite.setPosition(600.f, 27.f);
	assets.draw_skulls = false;
	// map & table

	unique_ptr<Item> sword = item_manager.items[0]->clone();
	player.inventory[0].item = std::move(sword);

	sf::Clock deltaClock;
	while (window.isOpen() && state == Playing) {

		Event event;
		Vector2i mouse_pos = sf::Mouse::getPosition(window); // !!!
		player.upd_item_ptr();
		Sword* sword = dynamic_cast<Sword*>(player.activeItem);

		static Item* lastSword = nullptr;
		if (player.activeItem != lastSword && player.activeItem) {
			Sword* sword = dynamic_cast<Sword*>(player.activeItem);
			if (sword) {
				swordEffect.loadFromItem(sword);
				lastSword = player.activeItem;
			}
		}

		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) return Exit_gs;
			if (event.type == Event::KeyPressed) {
				player.useActiveItem(event, assets.all_is_locked, font, assets.walls);
				if (event.key.code >= Keyboard::Num1 && event.key.code <= Keyboard::Num9)
					player.selectedSlot = event.key.code - Keyboard::Num1;
				if (event.key.code == Keyboard::Escape) {
					if (player.menu_is_active) {
						player.disableMenu(assets.all_is_locked);
					}
					if (assets.showingmap) {
						assets.showingmap = false;
						assets.all_is_locked = false;
					}
				}
				if (event.key.code == Keyboard::Tab) {
					player.activateMenu(assets.all_is_locked);
				}
				if (assets.isInTableZone && event.key.code == Keyboard::F && !assets.LevelCompleted) {
					assets.showingmap = true;
					assets.draw_skulls = true;
					assets.all_is_locked = true;
				}
				if (event.key.code == Keyboard::E) {
					if (assets.isInSkullZone && assets.draw_skulls) {
						for (auto& skull : assets.skulls) {
							if (skull && !skull->name.empty() && player.sprite.getGlobalBounds().intersects(skull->sprite.getGlobalBounds())) {
								for (size_t i = 0; i < player.inventory.size(); i++) {
									if (player.inventory[i].item == nullptr) {
										player.inventory[i].item = std::move(skull);
									/*	player.inventory[i].item->name = "skull";*/
										player.inventory[i].item->sprite.setScale(0.9f, 0.9f);
										player.inventory[i].item->sprite.setPosition(player.inventory[i].hitbox.getPosition() + Vector2f(10.f, 10.f));
										assets.skull_counter++;
										break;
									}
								}
								break;
							}
						}
					}
				}
				if (event.key.code == Keyboard::G) {
					if (assets.isInBowlZone) {
						if (player.activeItem && player.activeItem->name == "skull") {
							player.activeItem->toDeleteEnable();
							assets.bowl_skull_counter++;
						}
					}
				}
				if (event.key.code == Keyboard::Enter && portal.portal_sprite.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
					assets.draw_Loading = true;
					assets.all_is_locked = true;
					assets.LevelCompleted = true;
					player.cur_health_copy = player.current_health;	
					player.copy_items_for_newlvl();
				}
			}
			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
				bool inventoryClicked = false;
				player.drag_item(window, mouse_pos, inventoryClicked, assets.all_is_locked, assets.chests, temp_armor, temp_holder, back_pos, oldSlot, oldArmor);
				for (auto& slot : player.inventory) {
					if (slot.hitbox.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
						inventoryClicked = true;
						break;
					}
				}
				if (!inventoryClicked) {
					Item* activeItem = player.inventory[player.selectedSlot].item.get();		
					Sword* sword = dynamic_cast<Sword*>(activeItem);
					if (activeItem && sword) {
						if (swordEffect.attack_delay.getElapsedTime().asSeconds() >= sword->attack_cooldown) {
							swordEffect.activate(player);
							swordEffect.attack_delay.restart();
							if (swordEffect.isActive()) {
								skelet.get_damage(swordEffect, sword, player); 
							}
						}
					}
				}
				if (event.mouseButton.button == sf::Mouse::Left && pause.resume.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.resume.to_true();
					pause.resume.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && pause.settings.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.settings.to_true();
					pause.settings.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && pause.exit.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && pause.isPaused) {
					pause.exit.to_true();
					pause.exit.setClickedTexture();
					pause.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && pause.pause_button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && !pause.isPaused
					&& !player.isDead) {
					pause.isPaused = true;
				}
				if (event.mouseButton.button == sf::Mouse::Left && tryagain.retry.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) &&
					tryagain.isActive) {
					tryagain.retry.to_true();
					tryagain.retry.setClickedTexture();
					tryagain.button_clock.restart().asSeconds();
				}
				if (event.mouseButton.button == sf::Mouse::Left && tryagain.menu.button_sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) &&
					tryagain.isActive) {
					tryagain.menu.to_true();
					tryagain.menu.setClickedTexture();
					tryagain.button_clock.restart().asSeconds();
				}
			}
			player.drop_item(event, assets.chests, temp_armor, temp_holder, back_pos, oldSlot, oldArmor);
		}
		if (tryagain.retry.get_clicked() && tryagain.button_clock.getElapsedTime().asSeconds() > 0.15) {
			tryagain.retry.setTexture();
			tryagain.retry.to_false();
			// bool reset
			tryagain.isActive = false;
			if (player.isDead) {
				player.copy_items_ifdead();
			}
			// bool reset
			return Playing;
		}
		if (tryagain.menu.get_clicked() && tryagain.button_clock.getElapsedTime().asSeconds() > 0.15) {
			tryagain.menu.setTexture();
			tryagain.menu.to_false();
			tryagain.isActive = false;
			pause.isPaused = false;
			return Menu_gs;
		}
		if (pause.resume.get_clicked() && pause.button_clock.getElapsedTime().asSeconds() > 0.15) {
			pause.resume.setTexture();
			pause.resume.to_false();
			pause.isPaused = false;
		}
		if (pause.settings.get_clicked() && pause.button_clock.getElapsedTime().asSeconds() > 0.15) {
			pause.settings.setTexture();
			pause.settings.to_false();
		}
		if (pause.exit.get_clicked() && pause.button_clock.getElapsedTime().asSeconds() > 0.15) {
			pause.exit.setTexture();
			pause.exit.to_false();
			tryagain.isActive = false;
			pause.isPaused = false;
			return Menu_gs;
		}

		// весь цикл
		if (!pause.isPaused) {
			if (temp_holder && temp_holder->isDraggin) {
				temp_holder->sprite.setPosition(
					window.mapPixelToCoords(sf::Mouse::getPosition(window)) + temp_holder->offset);
			}
			swordEffect.update(player);

			float dt = deltaClock.restart().asSeconds();
			Vector2f move(0, 0);
			player.moving = false;
			player.handleMovementInput(assets.all_is_locked, move, dt);

			if (assets.table_zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
				assets.isInTableZone = true;
				assets.pressF.setPosition(player.sprite.getPosition().x - 80.f, player.sprite.getPosition().y - 45.f);
			}
			else {
				assets.isInTableZone = false; // не в зоне, чтобы отобрадать текст
			}

			assets.isInSkullZone = false;
			for (auto& skull_zone : assets.skull_zones) {
				if (skull_zone.getGlobalBounds().intersects(player.sprite.getGlobalBounds())) {
					assets.isInSkullZone = true;
					break; // достаточно одного совпадения
				}
			}

			// also skelet
			skelet.update_animation(player);
			skelet.updateDamageVisuals();
			// also skelet

			// коллизии
			player.sprite.move(move.x, 0.f); // x
			for (auto& wall : assets.walls) {
				if (player.sprite.getGlobalBounds().intersects(wall.getGlobalBounds())) {
					player.sprite.move(-move.x, 0.f);
					break;
				}
			}
			if (player.sprite.getGlobalBounds().intersects(assets.tableSprite.getGlobalBounds())) {
				player.sprite.move(-move.x, 0.f);
			}
			if (player.sprite.getGlobalBounds().intersects(assets.bowl.getGlobalBounds())) {
				player.sprite.move(-move.x, 0.f);
			}

			player.sprite.move(0.f, move.y); // y
			for (auto& wall : assets.walls) {
				if (player.sprite.getGlobalBounds().intersects(wall.getGlobalBounds())) {
					player.sprite.move(0.f, -move.y);
					break;
				}
			}
			if (player.sprite.getGlobalBounds().intersects(assets.tableSprite.getGlobalBounds())) {
				player.sprite.move(0.f, -move.y);
			}
			if (player.sprite.getGlobalBounds().intersects(assets.bowl.getGlobalBounds())) {
				player.sprite.move(0.f, -move.y);
			}
			// коллизии

			// BOWL
			assets.isInBowlZone = false;
			if (player.sprite.getGlobalBounds().intersects(bowl_zone.getGlobalBounds())) {
				assets.isInBowlZone = true;
			}

			// BOWL
			window.clear(sf::Color::White);
			window.draw(assets.bgSprite);

			// bowl_pressG
			if (assets.isInBowlZone && !assets.LevelCompleted) {
				assets.pressG.setPosition(player.sprite.getPosition().x - 60.f, player.sprite.getPosition().y - 45.f);
				window.draw(assets.pressG);
			}
			// bowl_pressG

			// skulls
			assets.skulls_counter_text.setString(std::to_string(assets.skull_counter) + "/3");
			if (assets.draw_skulls) {
				for (auto& skull : assets.skulls) {
					if (skull) {
						window.draw(skull->sprite);
					}
				}
				window.draw(assets.skulls_counter_text);
			}
			player.draw_health(window);
			if (assets.isInSkullZone && assets.draw_skulls) {
				assets.E.setPosition(player.sprite.getPosition().x + 30.f, player.sprite.getPosition().y - 45.f);
				window.draw(assets.E);
			}
			if (assets.bowl_skull_counter == 1) {
				assets.bowl.setTexture(assets.bowl_oneskull);
			}
			else if (assets.bowl_skull_counter == 2) {
				assets.bowl.setTexture(assets.bowl_twoskulls);
			}
			else if (assets.bowl_skull_counter == 3 && !skelet.isKilled) {
				assets.bowl.setTexture(assets.bowl_threeskulls); 
			}
			else if (assets.bowl_skull_counter == 3 && skelet.isKilled) {
				assets.draw_skulls = false;
				assets.bowl.setTexture(assets.bowl_texture);
				window.draw(assets.penta_sprite);
				portal.portal_should_activate = true;
			}
			if(portal.portal_should_activate) portal.draw(player, window);
			window.draw(assets.bowl);
			// skulls

			window.draw(player.sprite);
			if (assets.isInTableZone && !assets.isInSkullZone && !assets.LevelCompleted) {
				window.draw(assets.pressF);
			}

			// активный слот
			if (player.activeItem) {
				player.draw_active_item(window);
			}
			// активный слот

			// skelet
			if (assets.skull_counter == 3) {
				if (skelet.health > 0) {
					// обновление логики
					skelet.update(dt, sf::Vector2f(player.sprite.getPosition()), player, assets.walls, sword, 50.f);
					skelet.update_attack_rects();
					skelet.attack(player);
					skelet.slash_update();
					if (skelet.trail.isActive) {
						window.draw(skelet.trail.sprite);
					}
					window.draw(skelet.sprite);
					if (skelet.active) {
						window.draw(skelet.slash_sprite);
					}
					window.draw(skelet.health_back_bar);
					window.draw(skelet.health_bar);
					window.draw(skelet.health_text);
				}
				else if (skelet.health <= 0) {
					skelet.health = 0;
					skelet.isKilled = true;
				}
			}
			// skelet

			if (swordEffect.isActive()) {
				window.draw(swordEffect.sprite);
			}
			if (assets.showingmap) {
				window.draw(assets.background_forMAP);
				window.draw(assets.esc);
				window.draw(assets.mapSprite);
				window.draw(assets.map_task);
			}
			
			assets.find_table.tickTip();
			assets.find_table.renderTip(window);

			if (assets.LevelCompleted) {
				assets.enter_the_portal.emitTip();
				assets.enter_the_portal.renderTip(window);
			}
			
			sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

			window.draw(pause.pause_button_sprite);

			// player inventory
			player.draw_inventory(window);
			if (player.armor_slot.hitbox.getGlobalBounds().contains(static_cast<sf::Vector2f>(mouse_pos))) { // если слот брони пустой
				item_description.check_if_slot_empty(player.armor_slot, window);
			}
			item_description.draw_descriptions(player.inventory, window, mouse_pos, showingItem);
			item_description.draw_armor_player(player.armor_slot, window, mouse_pos, showingArmor); // player
			player.draw_cooldowns(window);
			if (temp_holder) {
				window.draw(temp_holder->sprite);
			}
			// player inventory

			if (player.isDead) {
				tryagain.isActive = true;
				window.draw(tryagain.rectangle);
				window.draw(tryagain.menu_screen_sprite);
				window.draw(tryagain.menu.button_sprite);
				window.draw(tryagain.retry.button_sprite);
			}
			if (assets.draw_Loading) {
				if (assets.fade_clock.getElapsedTime().asSeconds() > 0.02) {
					assets.last_alpha += 1;
					assets.fade_rect.setFillColor(sf::Color(0, 0, 0, assets.last_alpha));
					if (assets.last_alpha >= 255) {
						goToNextLevel(current_location);
						return GameState::Playing;
					}
					assets.fade_clock.restart().asSeconds();
				}
				window.draw(assets.fade_rect);
			}
		}
		else {
			window.draw(pause.pause_screen_sprite);
			window.draw(pause.resume.button_sprite);
			window.draw(pause.settings.button_sprite);
			window.draw(pause.exit.button_sprite);
		}
		player.removeDeletedItems();
		window.display();
	}
	return state;
}


MenuState Menu(RenderWindow& window, Locations& current_location) {
	sf::RectangleShape black_desktop(Vector2f(1920.f, 1080.f));
	black_desktop.setFillColor(sf::Color::Black);

	sf::Clock fadeClock; // for fading
	int alpha = 0;
	const float FADE_SPEED = 1.0f;
	FadeStage fadeStage = NoneFade;

	Texture story_texture; // мини-история
	int story_alpha = 0;
	sf::Sprite story_sprite;
	if (story_texture.loadFromFile("textures/chapone/story.png")) {
		story_sprite.setTexture(story_texture);
		story_sprite.setPosition(0.f, 0.f);
		story_sprite.setColor(sf::Color(255, 255, 255, 0));
	}

	// buttons
	sf::Clock button_clock;
	Button button_play;
	if (button_play.load_texture("textures/buttons/Button_play.png", "textures/buttons/Button_play_clicked.png")) {
		button_play.setTexture();
	}

	button_play.button_sprite.setPosition(820.f, 500.f);

	Button button_exit;
	if (!button_exit.load_texture("textures/buttons/Button_exit.png", "textures/buttons/Button_exit_clicked.png")) {
		std::cerr << "Ошибка загрузки текстур кнопки Exit!" << std::endl;
	}
	else {
		button_exit.setTexture();
	}
	button_exit.button_sprite.setPosition(820.f, 620.f);
	// buttons

	Texture label_texture;
	sf::Sprite label;
	if (label_texture.loadFromFile("textures/buttons/LABEL.png")) {
		label.setTexture(label_texture);
		label.setScale(2.5f, 2.5f);
		label.setPosition(400.f, 100.f);
	}

	Texture back_texture;
	sf::Sprite background_sprite;
	if (back_texture.loadFromFile("textures/background.jpg")) {
		background_sprite.setTexture(back_texture);
	}
	background_sprite.setPosition(0.f, 0.f);

	bool isPlayClicked = false;
	bool isExitClicked = false;
	bool isRightArrowClicked = false;

	Texture first_chap_text;
	sf::Sprite first_chap;
	if (first_chap_text.loadFromFile("textures/chapone/chapone_label.png")) {
		first_chap.setTexture(first_chap_text);
		first_chap.setPosition(336.f, 374.f);
		first_chap.setScale(2.5f, 2.5f);
		first_chap.setColor(sf::Color(255, 255, 255, 0)); // изначально прозрачный
	}

	while (window.isOpen()) {
		sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
		Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left && button_exit.button_sprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mouse_pos)) && !isExitClicked) {
					button_exit.setClickedTexture();
					button_exit.to_true();
					button_clock.restart().asSeconds();
					isExitClicked = true;
				}
				if (event.mouseButton.button == sf::Mouse::Left && button_play.button_sprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mouse_pos)) && !isPlayClicked) {
					button_play.to_true();
					isPlayClicked = true;
					button_play.setClickedTexture();
					button_clock.restart().asSeconds();
				}
			}
			if (event.type == Event::KeyPressed) {
				if (fadeStage == ShowStory) {
					if (event.key.code == sf::Keyboard::Right && !isRightArrowClicked) {
						isRightArrowClicked = true;
						fadeStage = ShowLabel;
						fadeClock.restart();
					}
				}
			}
		}
		if (button_play.get_clicked() && button_clock.getElapsedTime().asSeconds() > 0.15) {
			button_play.setTexture();
			button_play.to_false();
			fadeStage = FadeToBlack;
			fadeClock.restart().asSeconds();
			
		}
		if (button_exit.get_clicked() && button_clock.getElapsedTime().asSeconds() > 0.15) {
			return Exit;
		}
		if (fadeStage == FadeToBlack) {
			if (fadeClock.getElapsedTime().asSeconds() > (FADE_SPEED / 255) && alpha < 255) {
				fadeClock.restart();
				alpha++;
				sf::Color c = black_desktop.getFillColor();
				c.a = alpha;
				black_desktop.setFillColor(c);
			}
			if (alpha >= 255) {
				if (current_location == first) {
					fadeStage = ShowStory;
					fadeClock.restart();
					story_sprite.setColor(sf::Color(255, 255, 255, 255));
				}
				else {
					return Play; 
				}
			}
		}
		else if (fadeStage == ShowLabel) {
			if (fadeClock.getElapsedTime().asSeconds() > 0.5f) { // пауза перед показом
				alpha = 255;
				first_chap.setColor(sf::Color(255, 255, 255, 255));
				fadeStage = FadeOutLabel;
				fadeClock.restart();
			}
		}
		else if (fadeStage == FadeOutLabel) {
			if (fadeClock.getElapsedTime().asSeconds() > 0.015f) {
				fadeClock.restart();
				alpha--;
				sf::Color c = first_chap.getColor();
				c.a = alpha;
				first_chap.setColor(c);
				if (alpha <= 0) {
					isPlayClicked = false;
					isExitClicked = false;
					isRightArrowClicked = false;
					return Play;
				}
			}
		}
		window.clear();

		if (fadeStage == NoneFade || fadeStage == FadeToBlack) {
			window.clear(sf::Color::White);
			window.draw(background_sprite);
			window.draw(label);
			window.draw(button_exit.button_sprite);
			window.draw(button_play.button_sprite);
			if (fadeStage == FadeToBlack)
				window.draw(black_desktop);
		}

		if (current_location == first) {
			if (fadeStage == ShowStory) {
				window.clear(sf::Color::Black);
				window.draw(story_sprite);
			}
			if (fadeStage == ShowLabel || fadeStage == FadeOutLabel) {
				window.clear(sf::Color::Black);
				window.draw(first_chap);
			}
		}
		window.display();
	}
	return None;
}

int main() {
	setlocale(LC_ALL, "RU");
	srand(time(NULL));

	RenderWindow window(VideoMode::getDesktopMode(), sf::String("Echo of the Depths"), sf::Style::Fullscreen);

	sf::Font pixel_game;
	if (!pixel_game.loadFromFile("fonts/Pixel_Game.otf")) {
		return -1;
	}

	std::map<std::string, Texture> allItemTextures;
	allItemTextures["skull"].loadFromFile("textures/chapone/skull.png"); // skull
	allItemTextures["sword"].loadFromFile("textures/chapone/weapons/sword_right.png"); // usual sword
	allItemTextures["edge_of_dark"].loadFromFile("textures/chapone/locationtwo/weapons/edge_of_darkness/edgeOfDarkness_right.png"); // edge of darkness
	allItemTextures["bloody_crystal"].loadFromFile("textures/chapone/locationtwo/activating/bloody_crystal.png"); // bloody crystal
	allItemTextures["heal_potion"].loadFromFile("textures/Potions/heal/heal_potion.png"); // heal potion
	allItemTextures["memory_stone"].loadFromFile("textures/chapone/locationtwo/activating/memory_stone.png"); // memory stone
	allItemTextures["shadow_armor"].loadFromFile("textures/chapone/locationtwo/Armor/shadow_armor.png"); // shadow armor
	allItemTextures["fog mask"].loadFromFile("textures/chapone/locationtwo/Items/fog_mask/fog_mask.png"); // fog mask
	allItemTextures["ice amulet"].loadFromFile("textures/chapone/locationtwo/Items/ice_amulet/ice_amulet.png"); // ice amulet
	allItemTextures["torch_scroll_activated"].loadFromFile("textures/chapone/locationtwo/Items/torch_spell.png"); // большой свиток
	allItemTextures["torch_scroll"].loadFromFile("textures/chapone/locationtwo/Items/torch_scroll.png"); // свиток для активации факелов в инветнаре
	allItemTextures["lim_scroll_alert"].loadFromFile("textures/limited_scrolls/attention.png"); // текст сверху свитка(если свиток на раз) 
	allItemTextures["fern_leaf"].loadFromFile("textures/chapone/locationtwo/Items/fern_leaf.png"); // fearn leaf
	allItemTextures["skull"].loadFromFile("textures/chapone/skull.png"); // skull
	allItemTextures["vein_of_earth"].loadFromFile("textures/chapone/locationthree/weapons/vein_of_earth/vein_of_earth.png"); // vein of earth
	allItemTextures["ancient_seal1"].loadFromFile("textures/chapone/locationtwo/Items/ancient_seal.png"); // ancient seal 1
	allItemTextures["cursed_diary"].loadFromFile("textures/books/cursed_diary(third_loc)/book_item.png"); // book(third_location)
	allItemTextures["third_loc_wardrope_key"].loadFromFile("textures/chapone/locationthree/Items/wkey.png"); // wardrope key
	allItemTextures["cursed_mirror"].loadFromFile("textures/chapone/locationthree/Items/cursed_mirror/cursed_mirror.png"); // cursed_mirror
	allItemTextures["portal_book"].loadFromFile("textures/books/portal_book(third_loc)/icon.png"); // portal book
	allItemTextures["apple"].loadFromFile("textures/chapone/locationthree/Items/apple.png");

	ItemManager item_manager;
	if (!item_manager.loadAllItems(allItemTextures, pixel_game) || !item_manager.loadArmor(allItemTextures)) {
		return -1;
	}

	Pause pause;
	if (!pause.load()) {
		return -1;
	}

	ItemDescription item_description;
	if (!item_description.load()) {
		return -1;
	}

	TryAgainMenu tryagain;
	if (!tryagain.load()) {
		return -1;
	}

	// main char loading
	MainPlayer player;
	if (!player.loadTextures()) {
		return -1;
	}
	else {
		player.setUpUI();
	}

	GameState gs = GameState::Menu_gs;
	Locations current_location = Locations::forests_of_echo;

	while (window.isOpen()) {
		if (gs == GameState::Playing) {
			switch (current_location) {
			case Locations::first:
				gs = first_location(window, player, gs, pixel_game, current_location, item_description, pause, tryagain, allItemTextures, item_manager);
				break;

			case Locations::second:
				gs = second_location(window, player, gs, pixel_game, current_location, item_description, pause, tryagain, allItemTextures, item_manager);
				break;

			case Locations::third:
				gs = third_location(window, player, gs, pixel_game, current_location, item_description, pause, tryagain, allItemTextures, item_manager);
				break;
			
			case Locations::hub:
				gs = Hub(window, player, current_location, gs, pixel_game, pause, item_manager);
				break;
				
			case Locations::forests_of_echo:
				gs = forests_of_echo_lvl(window, player, pixel_game);
				break;
			}
		}
		else if (gs == GameState::Menu_gs) {
			MenuState result = Menu(window, current_location);
			if (result == Exit)
				window.close();
			else if (result == Play)
				gs = GameState::Playing;
		}
	}
	return 0;
} 