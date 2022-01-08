#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <list>
#include <windows.h> // for Sleep();
#include <thread>
#include <random>
#include <string>

using namespace std;
using namespace sf;

int random_integer(int min, int max) {
	random_device seed; // Generate a random seed
	mt19937_64 mersenne(seed());
	uniform_int_distribution<> random(min, max);
	return random(mersenne);
}

class Starter {
public:
	RenderWindow window;

	RectangleShape sky;

	Font font;
	Text text;

	SoundBuffer soundbuffer[2]; // Sound
	Sound shooting_sound;
	Sound red_light_green_light_sound;

	Starter() {
		window.create(VideoMode(800, 900), "Red light, green light"); // Window
		window.setFramerateLimit(60);
		window.clear(Color::Black);

		sky.setSize(Vector2f(800, 100)); // Sky
		sky.setFillColor(Color::Cyan);

		font.loadFromFile("resources/AGENCYR.ttf"); // Text
		text.setFont(font);
		text.setFillColor(Color::Green);
		text.setCharacterSize(55);
		text.setPosition(200, 380);
		text.setString("Press Enter to start");

		soundbuffer[0].loadFromFile("resources/Red light green light sound.wav"); // Sound
		red_light_green_light_sound.setBuffer(soundbuffer[0]);
		soundbuffer[1].loadFromFile("resources/shooting_sound.wav");
		shooting_sound.setBuffer(soundbuffer[1]);
	}
};

//void alarm(int target_time, bool times_up) {
//	int elapsed_time = 0;
//	while (true) {
//		Sleep(1);
//		elapsed_time++;
//		if (elapsed_time == target_time) {
//			times_up = true;
//			return;
//		}
//	}
//}

class NPC {
	Texture stickman_texture; // Player
public:
	Sprite npc;
	NPC(int y) {
		stickman_texture.loadFromFile("resources/stickman.png"); // Player
		npc.setTexture(stickman_texture);
		npc.scale(0.2f, 0.2f);
		npc.setPosition(11, y);
	}

	void draw_npc(RenderWindow& window) {
		window.draw(npc);
	}
	
	void NPC_action(Starter& starter) {
		while (true) {
			Sleep(300);
			int moving_interval = random_integer(80, 200);
			printf("%d\n", moving_interval);
			/*bool times_up = false;
			thread thread1(alarm, 1000, times_up);
			thread1.detach();*/
			while (starter.red_light_green_light_sound.getStatus() == Sound::Playing) {
				npc.move(3, 0);
				Sleep(moving_interval);
				if (npc.getPosition().x > 800) // NPC winning the game
					return;
			}
			while (starter.red_light_green_light_sound.getStatus() != Sound::Playing);
		}
	}
};


class NPC_set {
	vector<NPC*> npcs;
	Texture stickman_texture;
	vector<thread> threads;
public:
	NPC_set() {
		int y = 800 - 80;
		for (int i = 0; i < 3; i++) {
			npcs.push_back(new NPC(y));
			y -= 100;
		}
	}

	void recover_position() {
		int y = 800 - 80;
		for (int i = 0; i < 3; i++) {
			npcs[i]->npc.setPosition(11, y);
			y -= 100;
		}
	}

	void draw_npcs(RenderWindow& window) {
		for (int i = 0; i < 3; i++)
			npcs[i]->draw_npc(window);
	}

	void NPCs_action(Starter& starter) {
		for (int i = 0; i < 3; i++) {
			threads.push_back(thread(&NPC::NPC_action, npcs[i], ref(starter)));
			threads[i].detach();
		}

		//thread thread1(&NPC::NPC_action, npcs[0], ref(starter));
		//thread thread2(&NPC::NPC_action, npcs[1], ref(starter));
		//thread thread3(&NPC::NPC_action, npcs[2], ref(starter));
		//thread1.detach();
		//thread2.detach();
		//thread3.detach();
	}
};

class Main_game {
	int seconds = 60;
	Font font;
	Text text;

	bool right_key_pressed = false, game_done = false;

	Texture red_light, green_light; // Robot
	Sprite robot;
	Texture stickman_texture; // Player
	Sprite player;

public:
	Event event;

	Main_game() {
		font.loadFromFile("resources/AGENCYR.ttf"); // Score
		text.setFont(font);
		text.setFillColor(Color::Black);
		text.setCharacterSize(44);
		text.setPosition(11, 11);

		red_light.loadFromFile("resources/red_light.png"); // Robot
		green_light.loadFromFile("resources/green_light.png");
		robot.setPosition(350, 110);

		stickman_texture.loadFromFile("resources/stickman.png"); // Player
		player.setTexture(stickman_texture);
		player.setPosition(11, 900 - 80);
		player.scale(0.2f, 0.2f);
	}

	void draw_plyaer(RenderWindow& window) {
		window.draw(player);
	}

	void move_right(Sound& red_light_green_light_sound, Sound& shooting_sound) {
		player.move(3, 0);
		if (red_light_green_light_sound.getStatus() == Sound::Playing == false || seconds <= 0) { // Kill if the robot is watching or time's up
			game_done = true;
			shooting_sound.play();
			red_light_green_light_sound.stop();
			game_over();
			return;
		}
		if (player.getPosition().x >= 800) { // Game clear
			game_done = true;
			red_light_green_light_sound.stop();
			game_clear();
			return;
		}
	}

	inline void events(Sound& red_light_green_light_sound, Sound& shooting_sound, RenderWindow& window) {
		if (Keyboard::isKeyPressed(Keyboard::Right) == false) // To prevent holding the right key
			right_key_pressed = false;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case Event::Closed:
				window.clear();
				break;
			case Event::KeyPressed:
				if (Keyboard::isKeyPressed(Keyboard::Right) == true && right_key_pressed == false) { // Move right
					right_key_pressed = true;
					move_right(red_light_green_light_sound, shooting_sound);
				}
				break;
			}
		}
	}

	void timer() {
		while (true) {
			text.setString("Left time : " + to_string(seconds));
			Sleep(1000);
			seconds--;
		}
	}

	void draw_text(RenderWindow& window) {
		window.draw(text);
	}

	void robot_killer(Sound& red_light_green_light_sound) {
		while (true) {
			if (game_done == true)
				return;

			robot.setTexture(green_light); // Green light
			red_light_green_light_sound.play();
			while (red_light_green_light_sound.getStatus() == Sound::Playing == true); // Wait until the sound ends
			robot.setTexture(red_light); // Red light
			Sleep(2888);
		}
	}

	void draw_robot(RenderWindow& window) {
		window.draw(robot);
	}

	void game_over() {
		text.setFillColor(Color::Red);
		text.setCharacterSize(55);
		text.setPosition(270, 380);
		text.setString("GAME OVER\nEnter to retry");
	}

	void game_clear() {
		text.setFillColor(Color::Yellow);
		text.setCharacterSize(55);
		text.setPosition(270, 380);
		text.setString("GAME CLEAR\nEnter to retry");
	}

	bool is_game_done() {
		return game_done;
	}

	void restart_game(Sound& red_light_green_light_sound, NPC_set& npc_set) {
		text.setFillColor(Color::Black);
		text.setCharacterSize(44);
		text.setPosition(11, 11);

		player.setPosition(11, 900 - 80);
		int y = 800 - 80;
		npc_set.recover_position();

		Sleep(3333);
		game_done = false;

		seconds = 61;
		thread thread1(&Main_game::robot_killer, this, ref(red_light_green_light_sound));
		thread1.detach();
	}
};

int main() {
	Starter starter;
	starter.window.draw(starter.text);
	starter.window.display();

	Main_game main_game;
	NPC_set npc_set;

	while (true) { // Wait until Enter is pressed to start
		if (starter.window.pollEvent(main_game.event))
			if (Keyboard::isKeyPressed(Keyboard::Enter) == true)
				break;
	}

	thread thread1(&Main_game::robot_killer, &main_game, ref(starter.red_light_green_light_sound)); // Robot killer
	thread1.detach();
	thread thread2(&Main_game::timer, &main_game);
	thread2.detach();

	npc_set.NPCs_action(starter);

	while (starter.window.isOpen()) {
		starter.window.clear(Color::White);

		main_game.events(starter.red_light_green_light_sound, starter.shooting_sound, starter.window); // To move player
		starter.window.draw(starter.sky);
		main_game.draw_plyaer(starter.window);
		main_game.draw_text(starter.window);
		main_game.draw_robot(starter.window);
		npc_set.draw_npcs(starter.window);

		starter.window.display();

		if (main_game.is_game_done() == true) { // When the game is done
			starter.window.clear(Color::Black);
			main_game.draw_text(starter.window); // Game over or Game clear
			starter.window.display();

			while (true) // Restart when Enter is pressed
				if (starter.window.pollEvent(main_game.event))
					if (Keyboard::isKeyPressed(Keyboard::Enter) == true)
						break;

			main_game.restart_game(starter.red_light_green_light_sound, npc_set);
		}
	}
}