#include<SDL.h>
#include<SDL_image.h>
#include<SDL_mixer.h>
#include<iostream>
#include<map>
#include<string>
#include<cstdlib>

SDL_Window* window=nullptr;
SDL_Renderer* renderer=nullptr;
const int ret_w=6, ret_h=24;
const int window_w=300, window_h = 300;

struct Rect {
	int x;
	int y = 150;
	int vy = 0;
	SDL_Rect ret{ x - (ret_w / 2), y - (ret_h / 2), ret_w, ret_h };

	void updateret() {
		ret.x = x - (ret_w / 2);
		ret.y = y - (ret_h / 2);
	}
};
struct Ball {
	int x=150, y=150, vx=0, vy=0;
	int difficulty = 0; //determines the horizontal speed of the ball
};
struct Input {
	SDL_Event event;
	bool up=false, down=false;
	bool quit = false;
};

void init();
void gameloop();
void eventloop(Input& input);
void update(Rect&, Rect&, Ball&, const Input& input);
void render(const Rect&, const Rect&, const Ball&);
void close();

void init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_CreateWindowAndRenderer(window_w, window_h, SDL_WINDOW_SHOWN, &window, &renderer);
	SDL_SetWindowTitle(window, "Pong");
}

inline void gameloop() {
	Rect player, enemy;
	player.x = 3, enemy.x=300-3; //3 é metade da largura do rect
	player.updateret(), enemy.updateret();
	Ball ball;
	ball.vy = 1;
	ball.vx = -1;
	Input input;
	int32_t time_remaining, then;
	while (input.quit == false) {
		then = SDL_GetTicks();
		eventloop(input);
		update(player, enemy, ball, input);
		render(player, enemy, ball);
		time_remaining =16- (SDL_GetTicks()-then);
		if(time_remaining>0) SDL_Delay(time_remaining);
	}
}

inline void eventloop(Input& input) {
	while (SDL_PollEvent(&(input.event))) {
		switch (input.event.type) {
		case SDL_KEYDOWN:
			switch (input.event.key.keysym.sym) {
			case SDLK_UP:
				input.up = true;
				break;
			case SDLK_DOWN:
				input.down = true;
				break;
			}
			break;
		case SDL_KEYUP:
			switch (input.event.key.keysym.sym) {
			case SDLK_UP:
				input.up = false;
				break;
			case SDLK_DOWN:
				input.down = false;
				break;
			}
			break;
		case SDL_QUIT:
			input.quit = true;
			break;
		}
	}
}

enum class collision {none,top_wall,bottom_wall,player,enemy,player_win,enemy_win};
inline collision ball_colliding(const Rect& player, const Rect& enemy, const Ball& ball) {
	if (ball.y - 1 <= 0) return collision::top_wall;
	if (ball.y + 1 >= window_h) return collision::bottom_wall;
	if (ball.x-2 <= player.x + ret_w/2 && ball.x+2 >= player.x - ret_w/2 && ball.y <= player.y + ret_h/2 && ball.y >= player.y - ret_h/2) return collision::player;
	if (ball.x-2 <= enemy.x + ret_w/2 && ball.x+2 >= enemy.x - ret_w/2 && ball.y <= enemy.y + ret_h/2 && ball.y >= enemy.y - ret_h/2) return collision::enemy;
	if (ball.x + 1 >= window_w) return collision::player_win;
	if (ball.x - 1 <= 0) return collision::enemy_win;
	return collision::none;
}

inline void update(Rect& player, Rect& enemy, Ball& ball, const Input& input) {
	//update the player
	if (input.up == true && input.down==false && player.y > ret_h/2) {
		player.y -= 3;
		player.ret.y = (player.y) - (ret_h / 2);
	}
	if (input.down == true && input.up==false && player.y < 300 - ret_h/2) {
		player.y += 3;
		player.ret.y = (player.y) - (ret_h / 2);
	}
	//update the enemy
	if (enemy.y < ball.y && enemy.y < 300 - ret_h/2) {
		enemy.y += 3;
		enemy.ret.y = (enemy.y) - (ret_h / 2);
	}
	if (enemy.y > ball.y && enemy.y > ret_h / 2) {
		enemy.y -= 3;
		enemy.ret.y = (enemy.y) - (ret_h / 2);
	}

	switch (auto result=ball_colliding(player, enemy, ball)) {
	case collision::none:
		ball.x += ball.vx;
		ball.y += ball.vy;
		break;
	case collision::top_wall:
		ball.vy *= -1;
		ball.y += 4;
		break;
	case collision::bottom_wall:
		ball.vy *= -1;
		ball.y -= 4;
		break;
	case collision::player:
		ball.vx *= -1;
		if(ball.vx<4) ++(ball.vx);
		if (input.up == true && input.down == false && ball.vy>-4) ball.vy -= 1;
		if (input.up == false && input.down == true && ball.vy<4) ball.vy += 1;
		ball.x += 4;
		break;
	case collision::enemy:
		ball.vx *= -1;
		if(ball.vx>-4) --(ball.vx);
		ball.x -= 4;
		break;
	case collision::enemy_win:
	case collision::player_win:
		std::string say="Player ";
		say += (result == collision::enemy_win) ? "Lost!" : "Won!";
		std::cout << say << std::endl;
		system("pause");
		exit(EXIT_SUCCESS);
		break;
	}
}

inline void render(const Rect& player, const Rect& enemy, const Ball& ball) {
	//limpar a tela
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);
	//set up white color for drawing everything else
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	//desenhar jogadores
	SDL_RenderDrawRect(renderer, &(player.ret));
	SDL_RenderDrawRect(renderer, &(enemy.ret));
	//desenhar a bola (3x3 pixels)
	SDL_RenderDrawPoint(renderer, ball.x, ball.y);
	SDL_RenderDrawPoint(renderer, ball.x+1, ball.y);
	SDL_RenderDrawPoint(renderer, ball.x-1, ball.y);
	SDL_RenderDrawPoint(renderer, ball.x, ball.y+1);
	SDL_RenderDrawPoint(renderer, ball.x, ball.y-1);
	SDL_RenderDrawPoint(renderer, ball.x+1, ball.y+1);
	SDL_RenderDrawPoint(renderer, ball.x+1, ball.y-1);
	SDL_RenderDrawPoint(renderer, ball.x-1, ball.y+1);
	SDL_RenderDrawPoint(renderer, ball.x-1, ball.y-1);
	//e finalmente, fazer o desenho na tela
	SDL_RenderPresent(renderer);
}

void close() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char** argv) {
	std::atexit(close);
	init();
	gameloop();
	return 0;
}

