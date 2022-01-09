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

Event event;
Texture *stickman_texture;
Sprite* player_sprite;
RenderWindow *window;
Sound *shooting_sound, *red_light_green_light_sound;
class Starter {
public:
	RectangleShape sky;

	Font font;
	Text text;

	SoundBuffer soundbuffer[2];
	Starter() {
		font.loadFromFile("resources/AGENCYR.ttf"); // Text
		text.setFont(font);
		text.setFillColor(Color::Green);
		text.setCharacterSize(55);
		text.setPosition(200, 380);
		text.setString("Press Enter to start");

		window = new RenderWindow(VideoMode(800, 900), "Red light, green light");
		window->setFramerateLimit(60);
		window->clear(Color::Black);
		window->draw(text);
		window->display();

		sky.setSize(Vector2f(800, 100)); // Sky
		sky.setFillColor(Color::Cyan);

		shooting_sound = new Sound();
		red_light_green_light_sound = new Sound();
		soundbuffer[0].loadFromFile("resources/Red light green light sound.wav"); // Sound
		red_light_green_light_sound->setBuffer(soundbuffer[0]);
		soundbuffer[1].loadFromFile("resources/shooting_sound.wav");
		shooting_sound->setBuffer(soundbuffer[1]);

		stickman_texture = new Texture();
		stickman_texture->loadFromFile("resources/stickman.png");
		player_sprite = new Sprite();
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
public:
	Sprite npc;
	NPC(int y) {
		
		npc.setTexture(*stickman_texture);
		npc.scale(0.2f, 0.2f);
		npc.setPosition(11, y);
	}

	void draw_npc(RenderWindow& window) {
		window.draw(npc);
	}
	
	void NPC_action() {
		while (true) {
			Sleep(300);
			int moving_interval = random_integer(80, 200);
			printf("%d\n", moving_interval);
			/*bool times_up = false;
			thread thread1(alarm, 1000, times_up);
			thread1.detach();*/
			while (red_light_green_light_sound->getStatus() == Sound::Playing) {
				npc.move(3, 0);
				Sleep(moving_interval);
				if (npc.getPosition().x > 800) // NPC winning the game
					return;
			}
			while (red_light_green_light_sound->getStatus() != Sound::Playing);
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
			threads.push_back(thread(&NPC::NPC_action, npcs[i]));
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

int remaining_seconds = 60;
bool is_game_done = false;

class Robot {
	Font font;
	Text remaining_time;

	Texture red_light, green_light; // Robot
	Sprite robot;

public:
	Robot() {
		font.loadFromFile("resources/AGENCYR.ttf"); // remaining_time
		remaining_time.setFont(font);
		remaining_time.setFillColor(Color::Black);
		remaining_time.setCharacterSize(44);
		remaining_time.setPosition(11, 11);

		red_light.loadFromFile("resources/red_light.png"); // Robot
		green_light.loadFromFile("resources/green_light.png");
		robot.setPosition(350, 110);
	}

	void timer() {
		while (true) {
			remaining_time.setString("Remaining time : " + to_string(remaining_seconds));
			Sleep(1000);
			remaining_seconds--;
		}
	}

	void draw_remaining_time() {
		window->draw(remaining_time);
	}

	void robot_killer() {
		while (true) {
			if (is_game_done == true)
				return;

			robot.setTexture(green_light); // Green light
			red_light_green_light_sound->play();
			while (red_light_green_light_sound->getStatus() == Sound::Playing == true); // Wait until the sound ends
			robot.setTexture(red_light); // Red light
			Sleep(2888);
		}
	}

	void draw_robot() {
		window->draw(robot);
	}
};

class Game_done {
	Font font;
	Text text;
public:
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

	void restart_game(NPC_set& npc_set, Robot& robot, Game_done& game_done) {
		text.setFillColor(Color::Black);
		text.setCharacterSize(44);
		text.setPosition(11, 11);

		player_sprite->setPosition(11, 900 - 80);
		int y = 800 - 80;
		npc_set.recover_position();

		Sleep(3333);
		is_game_done = false;

		remaining_seconds = 61;
		thread thread1(&Robot::robot_killer, &robot);
		thread1.detach();
	}
};

class Player {
	bool right_key_pressed = false;
public:
	Player() {
		player_sprite->setTexture(*stickman_texture);
		player_sprite->setPosition(11, 900 - 80);
		player_sprite->scale(0.2f, 0.2f);
	}

	void draw_plyaer() {
		window->draw(*player_sprite);
	}

	void move_right(Game_done& game_done) {
		player_sprite->move(3, 0);
		if (red_light_green_light_sound->getStatus() == Sound::Playing == false || seconds <= 0) { // Kill and game over if moving when the robot is watching or time's up
			is_game_done = true;
			shooting_sound->play();
			red_light_green_light_sound->stop();
			game_done.game_over();
			return;
		}
		if (player_sprite->getPosition().x >= 800) { // Game clear
			is_game_done = true;
			red_light_green_light_sound->stop();
			game_done.game_clear();
			return;
		}
	}

	inline void events(Game_done& game_done) {
		if (Keyboard::isKeyPressed(Keyboard::Right) == false) // To prevent holding the right key
			right_key_pressed = false;

		while (window->pollEvent(event)) {
			switch (event.type) {
			case Event::Closed:
				window->clear();
				break;
			case Event::KeyPressed:
				if (Keyboard::isKeyPressed(Keyboard::Right) == true && right_key_pressed == false) { // Move right
					right_key_pressed = true;
					move_right(game_done);
				}
				break;
			}
		}
	}
};

int main() {
	Starter starter;
	Robot robot;
	Player player;
	NPC_set npc_set;
	Game_done game_done;

	while (true) { // Wait until Enter is pressed to start
		if (window->pollEvent(event))
			if (Keyboard::isKeyPressed(Keyboard::Enter) == true)
				break;
	}

	thread thread1(&Robot::robot_killer, &robot);
	thread1.detach();
	thread thread2(&Robot::timer, &robot);
	thread2.detach();

	npc_set.NPCs_action(starter);

	while (window->isOpen()) {
		window->clear(Color::White);

		player.events(game_done); // To move player
		window->draw(starter.sky);
		player.draw_plyaer();
		robot.draw_remaining_time();
		robot.draw_robot();
		npc_set.draw_npcs(*window);

		window->display();

		if (is_game_done == true) { // When the game is done
			window->clear(Color::Black);
			robot.draw_remaining_time();
			window->display();

			while (true) // Restart when Enter is pressed
				if (window->pollEvent(event))
					if (Keyboard::isKeyPressed(Keyboard::Enter) == true)
						break;

			game_done.restart_game(npc_set, robot, game_done);
		}
	}
}