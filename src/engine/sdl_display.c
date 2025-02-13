#include "../../include/sdl_display.h"
#include "../../include/utils/gameplay_utils.h"
#include "../../include/utils/timer.h"

void setup_cells(Sdl_Data *sdl_data);
void render_button(SDL_Renderer *renderer, Button *button, int clip);
void render_object(SDL_Renderer *renderer, Object *object, int clip);
void render_board(Sdl_Data *sdl_data);
void render_menu(Sdl_Data *sdl_data);
void render_playfield(Sdl_Data *sdl_data);
void free_txt(SDL_Texture **texture);
SDL_Texture *load_from_text(Sdl_Data *sdl_data, SDL_Rect *content_rect, const char *text);

static const int buffer_size = 32;
static bool high_res = 0;

void stop_game(Sdl_Data *sdl_data)
{
	Object *timer = sdl_data->playfield->timer;
	char buffer[buffer_size];

	switch(sdl_data->game->status)
	{
		case X_WON:
			sprintf(buffer, "X WON!");
			break;
		case O_WON:
			sprintf(buffer, "O WON!");
			break;
		default:
			sprintf(buffer, "DRAW!");
	}

	free_txt(&timer->content_txt);
	free_txt(&sdl_data->playfield->forfeit->content_txt);
	timer->content_txt = load_from_text(sdl_data, &timer->content_rect, buffer);
	sdl_data->playfield->forfeit->content_txt = load_from_text(sdl_data, &sdl_data->playfield->forfeit->content_rect, "EXIT");
	render_object(sdl_data->renderer, timer, 0);
	render_button(sdl_data->renderer, sdl_data->playfield->forfeit, 0);

	sdl_data->select_x = -1;
	sdl_data->select_y = -1;
	sdl_data->select_board = -1;
	render_playfield(sdl_data);
}

void f_mode(Sdl_Data *sdl_data)
{
	sdl_data->super_mode = sdl_data->super_mode ? 0 : 1;
	sdl_data->game->board_size = sdl_data->super_mode == 1 ? 3 : 1;
	render_button(sdl_data->renderer, sdl_data->menu->buttons[mode], (int)sdl_data->super_mode + 1);
}

void f_play(Sdl_Data *sdl_data)
{
	char buffer[buffer_size];
	sdl_data->game->bot_turn = 0;
	sdl_data->select_x = -1;
	sdl_data->select_y = -1;
	sdl_data->game->status = IN_PROGRESS;
	sdl_data->game->moves_count = 0;

	if(sdl_data->super_mode)
	{
		sdl_data->select_board = -1;
		for (int i = 0; i < 9; i++) {
			sdl_data->game->board[i]->moves_count = 0;
		}
	}
	else
	{
		sdl_data->select_board = 0;
		sdl_data->game->board[0]->moves_count = 0;
	}

	setup_cells(sdl_data);
	free_txt(&sdl_data->playfield->timer->content_txt);
	free_txt(&sdl_data->playfield->forfeit->content_txt);
	if(sdl_data->game->round_time % 60 == 0)
	{
		sprintf(buffer, "%d:00", sdl_data->game->round_time / 60);
	}
	else
	{
		sprintf(buffer, "%d:%2d", sdl_data->game->round_time / 60, sdl_data->game->round_time % 60);
	}
	sdl_data->playfield->timer->content_txt = load_from_text(sdl_data, &sdl_data->playfield->timer->content_rect, buffer);
	sdl_data->playfield->forfeit->content_txt = load_from_text(sdl_data, &sdl_data->playfield->forfeit->content_rect, "FORFEIT");
	set_timer(&sdl_data->game->timer, sdl_data->game->round_time);
	sdl_data->last_time = seconds_left(sdl_data->game->timer) + 1;
	sdl_data->in_game = 1;
	render_playfield(sdl_data);
}
void f_add_id(Sdl_Data *sdl_data)
{
	char buffer[buffer_size];
	if (sdl_data->on_lan)
	{
		sdl_data->game_id = (sdl_data->game_id + 1) % max_id;
		sdl_data->game->turn = sdl_data->game->turn == X ? O : X;
		sprintf(buffer, "START: %c", sdl_data->game->turn == X ? 'X' : 'O');
	}
	else
	{
		sdl_data->bot_difficulty = (sdl_data->bot_difficulty + 1) % max_diff;
		switch(sdl_data->bot_difficulty)
		{
			case 2:
				sprintf(buffer, "HARD");
				break;
			case 1:
				sprintf(buffer, "MEDIUM");
				break;
			default:
				sprintf(buffer, "EASY");
		}
	}
	free_txt(&sdl_data->menu->game_id->content_txt);
	sdl_data->menu->game_id->content_txt = load_from_text(sdl_data, &sdl_data->menu->game_id->content_rect, buffer);
	render_object(sdl_data->renderer, sdl_data->menu->game_id, 0);
}
void f_sub_id(Sdl_Data *sdl_data)
{
	char buffer[buffer_size];
	if (sdl_data->on_lan)
	{
		sdl_data->game_id = (sdl_data->game_id - 1) % max_id < 0 ? max_id - 1 : (sdl_data->game_id - 1) % max_id;
		sdl_data->game->turn = sdl_data->game->turn == X ? O : X;
		sprintf(buffer, "START: %c", sdl_data->game->turn == X ? 'X' : 'O');
	}
	else
	{
		sdl_data->bot_difficulty = (sdl_data->bot_difficulty - 1) % max_diff < 0 ? max_diff - 1 : (sdl_data->bot_difficulty - 1) % max_diff;
		switch(sdl_data->bot_difficulty)
		{
			case 2:
				sprintf(buffer, "HARD");
				break;
			case 1:
				sprintf(buffer, "MEDIUM");
				break;
			default:
				sprintf(buffer, "EASY");
		}
	}
	free_txt(&sdl_data->menu->game_id->content_txt);
	sdl_data->menu->game_id->content_txt = load_from_text(sdl_data, &sdl_data->menu->game_id->content_rect, buffer);
	render_object(sdl_data->renderer, sdl_data->menu->game_id, 0);
}
void f_add_mult_id(Sdl_Data *sdl_data)
{
	return;
	if (!sdl_data->on_lan)
		return;
	char buffer[buffer_size];
	sdl_data->game_id = (sdl_data->game_id + 10) % max_id;
	sprintf(buffer, "%d", sdl_data->game_id);
	free_txt(&sdl_data->menu->game_id->content_txt);
	sdl_data->menu->game_id->content_txt = load_from_text(sdl_data, &sdl_data->menu->game_id->content_rect, buffer);
	render_object(sdl_data->renderer, sdl_data->menu->game_id, 0);
}
void f_sub_mult_id(Sdl_Data *sdl_data)
{
	return;
	if (!sdl_data->on_lan)
		return;
	char buffer[buffer_size];
	sdl_data->game_id = (sdl_data->game_id - 10) % max_id < 0 ? max_id - 10 + sdl_data->game_id : (sdl_data->game_id - 10) % max_id;
	sprintf(buffer, "%d", sdl_data->game_id);
	free_txt(&sdl_data->menu->game_id->content_txt);
	sdl_data->menu->game_id->content_txt = load_from_text(sdl_data, &sdl_data->menu->game_id->content_rect, buffer);
	render_object(sdl_data->renderer, sdl_data->menu->game_id, 0);
}
void f_next_x(Sdl_Data *sdl_data)
{
	sdl_data->pallete->sprite_x = (sdl_data->pallete->sprite_x + 1) % sign_sprites;
	render_object(sdl_data->renderer, sdl_data->menu->sign_x, (int)sdl_data->pallete->sprite_x + 1);
}
void f_prev_x(Sdl_Data *sdl_data)
{
	sdl_data->pallete->sprite_x = (sdl_data->pallete->sprite_x - 1) % sign_sprites < 0 ? sign_sprites - 1 : (sdl_data->pallete->sprite_x - 1) % sign_sprites;
	render_object(sdl_data->renderer, sdl_data->menu->sign_x, (int)sdl_data->pallete->sprite_x + 1);
}
void f_next_o(Sdl_Data *sdl_data)
{
	sdl_data->pallete->sprite_o = (sdl_data->pallete->sprite_o + 1) % sign_sprites;
	render_object(sdl_data->renderer, sdl_data->menu->sign_o, (int)sdl_data->pallete->sprite_o + 1);
}
void f_prev_o(Sdl_Data *sdl_data)
{
	sdl_data->pallete->sprite_o = (sdl_data->pallete->sprite_o - 1) % sign_sprites < 0 ? sign_sprites - 1 : (sdl_data->pallete->sprite_o - 1) % sign_sprites;
	render_object(sdl_data->renderer, sdl_data->menu->sign_o, (int)sdl_data->pallete->sprite_o + 1);
}
void f_switch_online(Sdl_Data *sdl_data)
{
	char buffer[buffer_size];
	sdl_data->on_lan = sdl_data->on_lan ? 0 : 1;
	free_txt(&sdl_data->menu->game_id->content_txt);
	free_txt(&sdl_data->menu->buttons[switch_online]->content_txt);

	if (sdl_data->on_lan)
	{
		sprintf(buffer, "START: %c", sdl_data->game->turn == X ? 'X' : 'O');
		sdl_data->menu->game_id->content_txt = load_from_text(sdl_data, &sdl_data->menu->game_id->content_rect, buffer);
		sdl_data->menu->buttons[switch_online]->content_txt = load_from_text(sdl_data, &sdl_data->menu->buttons[switch_online]->content_rect, "LOCAL");
	}
	else
	{
		switch(sdl_data->bot_difficulty)
		{
			case 2:
				sprintf(buffer, "HARD");
				break;
			case 1:
				sprintf(buffer, "MEDIUM");
				break;
			default:
				sprintf(buffer, "EASY");
		}
		sdl_data->menu->game_id->content_txt = load_from_text(sdl_data, &sdl_data->menu->game_id->content_rect, buffer);
		sdl_data->menu->buttons[switch_online]->content_txt = load_from_text(sdl_data, &sdl_data->menu->buttons[switch_online]->content_rect, "vs BOT");
	}

	render_object(sdl_data->renderer, sdl_data->menu->game_id, 0);
	render_button(sdl_data->renderer, sdl_data->menu->buttons[switch_online], 0);
}

void f_select_cell(Sdl_Data *sdl_data, int x, int y)
{
	if(sdl_data->game->status != IN_PROGRESS)
	{
		return;
	}

	int select_x = (x - sdl_data->playfield->background->background_rect.x) / (board_size * (high_res ? 2 : 1) / (sdl_data->super_mode ? 9 : 3));
	int select_y = (y - sdl_data->playfield->background->background_rect.y) / (board_size * (high_res ? 2 : 1) / (sdl_data->super_mode ? 9 : 3));
	int select_board = select_x / 3 + select_y / 3 * 3;

	if(sdl_data->game->board[select_board]->value[select_x % 3][select_y % 3] != EMPTY)
	{
		return;
	}

	if((sdl_data->select_board == -1 || sdl_data->select_board == select_board) && sdl_data->game->board[select_board]->status == IN_PROGRESS)
	{
		sdl_data->select_x = select_x;
		sdl_data->select_y = select_y;
	}
}

void f_put_sign(Sdl_Data *sdl_data)
{
	if(sdl_data->select_x == -1)
	{
		return;
	}

	if(sdl_data->select_board == -1)
	{
		sdl_data->select_board = sdl_data->select_x / 3 + sdl_data->select_y / 3 * 3;
	}

	sdl_data->select_x %= 3;
	sdl_data->select_y %= 3;
	gameplay(sdl_data);

	if(sdl_data->super_mode)
	{	
		sdl_data->select_board = sdl_data->select_x + sdl_data->select_y * 3;
		if(sdl_data->game->board[sdl_data->select_board]->status != IN_PROGRESS)
		{
			sdl_data->select_board = -1;
		}
	}

	if(sdl_data->game->status != IN_PROGRESS)
	{
		stop_game(sdl_data);
	}

	if(!sdl_data->on_lan)
	{
		gameplay(sdl_data);
		if(sdl_data->game->status != IN_PROGRESS)
		{
			stop_game(sdl_data);
		}
	}

	sdl_data->select_x = -1;
	sdl_data->select_y = -1;
	render_playfield(sdl_data);
}

void f_forfeit(Sdl_Data *sdl_data)
{
	if(sdl_data->game->status == IN_PROGRESS)
	{	
		if(sdl_data->game->turn == X)
		{
			sdl_data->game->status = O_WON;
		}
		else
		{
			sdl_data->game->status = X_WON;
		}
		stop_game(sdl_data);

		return;
	}

	char buffer[10];
	sdl_data->in_game = 0;
	reset_Game(sdl_data->game);
	sprintf(buffer, "START: %c", sdl_data->game->turn == X ? 'X' : 'O');
	free_txt(&sdl_data->menu->game_id->content_txt);
	sdl_data->menu->game_id->content_txt = load_from_text(sdl_data, &sdl_data->menu->game_id->content_rect, buffer);
	render_menu(sdl_data);
}

void free_txt(SDL_Texture **texture)
{
	if (*texture != NULL)
	{
		SDL_DestroyTexture(*texture);
		texture = NULL;
	}
}

SDL_Texture *load_from_file(SDL_Renderer *renderer, const char *path)
{
	SDL_Surface *loaded_surface = IMG_Load(path);
	if (loaded_surface == NULL)
	{
		fprintf(stderr, "Unable to load image %s! Error: %s\n", path, IMG_GetError());
		return NULL;
	}

	SDL_Texture *new_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
	if (new_texture == NULL)
	{
		fprintf(stderr, "Unable to create texture from %s! Error: %s\n", path, SDL_GetError());
	}

	SDL_FreeSurface(loaded_surface);

	return new_texture;
}

SDL_Texture *load_from_text(Sdl_Data *sdl_data, SDL_Rect *content_rect, const char *text)
{
	SDL_Surface *text_surface = TTF_RenderText_Solid(sdl_data->pallete->font, text, sdl_data->pallete->text);
	if (text_surface == NULL)
	{
		return NULL;
	}

	SDL_Texture *new_texture = SDL_CreateTextureFromSurface(sdl_data->renderer, text_surface);
	if (new_texture == NULL)
	{
		fprintf(stderr, "Unable to create texture from %s! Error: %s\n", text, SDL_GetError());
	}
	else
	{
		content_rect->w = text_surface->w;
		content_rect->h = text_surface->h;
	}

	SDL_FreeSurface(text_surface);

	return new_texture;
}

void close_sdl(Sdl_Data *sdl_data)
{
	SDL_DestroyRenderer(sdl_data->renderer);
	SDL_DestroyWindow(sdl_data->window);
	IMG_Quit();
	SDL_Quit();
}

void set_color(SDL_Color *color, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	color->r = r;
	color->g = g;
	color->b = b;
	color->a = a;
}

void set_pos(SDL_Rect *rect, int x, int y, int w, int h)
{
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
}

bool init_sdl(Sdl_Data **sdl_data_ptr)
{
	*sdl_data_ptr = malloc(sizeof(Sdl_Data));
	Sdl_Data *sdl_data = *sdl_data_ptr;
	if(sdl_data == NULL)
	{
		fprintf(stderr, "Unable to allocate memory\n");
		return 0;
	}

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		return 0;
	}

	if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Warning: Linear texture filtering disabled\n");
	}

	if(!SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1"))
	{
		printf("Warning: Vsync disabled\n");
	}

	if(!SDL_SetHint(SDL_HINT_VIDEO_DOUBLE_BUFFER, "1"))
	{
		printf("Warning: Double buffering disabled\n");
	}

	SDL_DisplayMode dm;
	int w, h;

	if(SDL_GetDesktopDisplayMode(0, &dm) != 0)
	{
		high_res = 0;
	}
	else
	{
		w = dm.w;
		h = dm.h;

		if(w > window_width * 2 || h > window_height * 2)
		{
			high_res = 1;
		}
	}

	sdl_data->window = SDL_CreateWindow("Super TicTacToe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width * (high_res ? 2 : 1), window_height * (high_res ? 2 : 1), SDL_WINDOW_SHOWN);
	if (sdl_data->window == NULL)
	{
		fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
		return 0;
	}

	sdl_data->renderer = SDL_CreateRenderer(sdl_data->window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
	if (sdl_data->renderer == NULL)
	{
		fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
		return 0;
	}

	SDL_SetRenderDrawColor(sdl_data->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(sdl_data->renderer);
	SDL_RenderPresent(sdl_data->renderer);

	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags))
	{
		fprintf(stderr, "SDL_image could not initialize: %s\n", IMG_GetError());
		return 0;
	}

	if (TTF_Init() == -1)
	{
		fprintf(stderr, "SDL_ttf could not initialize: %s\n", TTF_GetError());
		return 0;
	}

	return 1;
}

bool init_pallete(Sdl_Data *sdl_data)
{
	sdl_data->pallete = malloc(sizeof(Pallete));
	Pallete *pallete = sdl_data->pallete;
	if (pallete == NULL)
	{
		fprintf(stderr, "Unable to allocate memory\n");
		return 0;
	}

	sdl_data->pallete->font = TTF_OpenFont(path_font, font_size);
	if (sdl_data->pallete->font == NULL)
	{
		fprintf(stderr, "Unable to open font %s! Error: %s\n", path_font, TTF_GetError());
		return 0;
	}

	pallete->sprite_x = 0;
	pallete->sprite_o = 0;
	set_color(&pallete->background, 0, 0, 0, 35);
	set_color(&pallete->board_lines, 0, 0, 0, 100);
	set_color(&pallete->spot_highlight, 0, 0, 0, 35);
	set_color(&pallete->text, 0, 0, 0, 0);

	return 1;
}

bool init_textures(Sdl_Data *sdl_data)
{
	SDL_Renderer *renderer = sdl_data->renderer;
	sdl_data->textures = malloc(sizeof(Textures));
	Textures *textures = sdl_data->textures;
	if (textures == NULL)
	{
		return 0;
	}

	textures->single_arrow = load_from_file(renderer, path_single_arrow);
	if (textures->single_arrow == NULL)
	{
		return 0;
	}

	textures->double_arrow = load_from_file(renderer, path_double_arrow);
	if (textures->double_arrow == NULL)
	{
		return 0;
	}

	textures->sign_x = load_from_file(renderer, path_x);
	if (textures->sign_x == NULL)
	{
		return 0;
	}

	textures->sign_o = load_from_file(renderer, path_o);
	if (textures->sign_o == NULL)
	{
		return 0;
	}

	textures->modes = load_from_file(renderer, path_modes);
	if (textures->modes == NULL)
	{
		return 0;
	}

	textures->field_background = load_from_file(renderer, path_field_background);
	if (textures->field_background == NULL)
	{
		return 0;
	}

	textures->logo = load_from_file(renderer, path_logo);
	if (textures->logo == NULL)
	{
		return 0;
	}

	return 1;
}

bool init_menu(Sdl_Data *sdl_data)
{
	sdl_data->menu = malloc(sizeof(Menu));
	Menu *menu = sdl_data->menu;
	if (menu == NULL)
	{
		fprintf(stderr, "Unable to allocate memory\n");
		return 0;
	}

	sdl_data->menu_functions = malloc(menu_buttons * sizeof(void (*)(Sdl_Data *)));
	void (**menu_functions)(Sdl_Data *) = sdl_data->menu_functions;
	if (menu_functions == NULL)
	{
		fprintf(stderr, "Unable to allocate memory\n");
		return 0;
	}

	Textures *textures = sdl_data->textures;

	menu->logo = malloc(sizeof(Object));
	menu->game_id = malloc(sizeof(Object));
	menu->sign_x = malloc(sizeof(Object));
	menu->sign_o = malloc(sizeof(Object));
	if (menu->logo == NULL || menu->game_id == NULL || menu->sign_x == NULL || menu->sign_x == NULL)
	{
		fprintf(stderr, "Unable to allocate memory\n");
		return 0;
	}

	int mult = high_res ? 2 : 1;

	set_pos(&menu->logo->background_rect, (window_width - 512) / (high_res ? 1 : 2), 40 * mult, 512 * mult, 128 * mult);
	menu->logo->content_rect = menu->logo->background_rect;
	menu->logo->background_txt = NULL;
	menu->logo->content_txt = textures->logo;

	set_pos(&menu->game_id->background_rect, (window_width - 256) / (high_res ? 1 : 2), 368 * mult, 256 * mult, 64 * mult);
	menu->game_id->background_txt = textures->field_background;
	menu->game_id->content_rect = menu->game_id->background_rect;
	menu->game_id->content_txt = load_from_text(sdl_data, &menu->game_id->content_rect, "EASY");

	set_pos(&menu->sign_x->background_rect, (window_width - 128) / (high_res ? 1 : 2) - 250 * mult, 520 * mult, 128 * mult, 128 * mult);
	menu->sign_x->content_rect = menu->sign_x->background_rect;
	menu->sign_x->background_txt = textures->field_background;
	menu->sign_x->content_txt = textures->sign_x;

	set_pos(&menu->sign_o->background_rect, (window_width - 128) / (high_res ? 1 : 2) + 250 * mult, 520 * mult, 128 * mult, 128 * mult);
	menu->sign_o->content_rect = menu->sign_o->background_rect;
	menu->sign_o->background_txt = textures->field_background;
	menu->sign_o->content_txt = textures->sign_o;

	menu->buttons = malloc(menu_buttons * sizeof(Button *));
	if (menu->buttons == NULL)
	{
		fprintf(stderr, "Unable to allocate memory\n");
		return 0;
	}

	Button **buttons = menu->buttons;

	for (int i = 0; i < menu_buttons; i++)
	{
		buttons[i] = malloc(sizeof(Button));
		if (buttons[i] == NULL)
		{
			fprintf(stderr, "Unable to allocate memory\n");
			return 0;
		}
		buttons[i]->flip = SDL_FLIP_NONE;
		buttons[i]->background_txt = textures->field_background;
	}

	set_pos(&buttons[mode]->background_rect, (window_width - 128) / (high_res ? 1 : 2), 150 * mult, 128 * mult, 128 * mult);
	buttons[mode]->content_rect = buttons[mode]->background_rect;
	buttons[mode]->content_txt = textures->modes;
	menu_functions[mode] = f_mode;

	set_pos(&buttons[play]->background_rect, (window_width - 256) / (high_res ? 1 : 2), 300 * mult, 256 * mult, 64 * mult);
	buttons[play]->content_rect = buttons[play]->background_rect;
	buttons[play]->content_txt = load_from_text(sdl_data, &buttons[play]->content_rect, "PLAY");
	menu_functions[play] = f_play;

	set_pos(&buttons[add_id]->background_rect, (window_width - 64) / (high_res ? 1 : 2) + 180 * mult, 368 * mult, 64 * mult, 64 * mult);
	buttons[add_id]->content_rect = buttons[add_id]->background_rect;
	buttons[add_id]->content_txt = textures->single_arrow;
	menu_functions[add_id] = f_add_id;

	set_pos(&buttons[sub_id]->background_rect, (window_width - 64) / (high_res ? 1 : 2) - 180 * mult, 368 * mult, 64 * mult, 64 * mult);
	buttons[sub_id]->flip = SDL_FLIP_HORIZONTAL;
	buttons[sub_id]->content_rect = buttons[sub_id]->background_rect;
	buttons[sub_id]->content_txt = textures->single_arrow;
	menu_functions[sub_id] = f_sub_id;

	set_pos(&buttons[add_mult_id]->background_rect, (window_width - 64) / (high_res ? 1 : 2) + 260 * mult, 368 * mult, 64 * mult, 64 * mult);
	buttons[add_mult_id]->content_rect = buttons[add_mult_id]->background_rect;
	buttons[add_mult_id]->content_txt = textures->double_arrow;
	menu_functions[add_mult_id] = f_add_mult_id;

	set_pos(&buttons[sub_mult_id]->background_rect, (window_width - 64) / (high_res ? 1 : 2) - 260 * mult, 368 * mult, 64 * mult, 64 * mult);
	buttons[sub_mult_id]->flip = SDL_FLIP_HORIZONTAL;
	buttons[sub_mult_id]->content_rect = buttons[sub_mult_id]->background_rect;
	buttons[sub_mult_id]->content_txt = textures->double_arrow;
	menu_functions[sub_mult_id] = f_sub_mult_id;

	set_pos(&buttons[next_x]->background_rect, (window_width - 128) / (high_res ? 1 : 2) - 100 * mult, 552 * mult, 64 * mult, 64 * mult);
	buttons[next_x]->content_rect = buttons[next_x]->background_rect;
	buttons[next_x]->content_txt = textures->single_arrow;
	menu_functions[next_x] = f_next_x;

	set_pos(&buttons[prev_x]->background_rect, (window_width - 128) / (high_res ? 1 : 2) - 336 * mult, 552 * mult, 64 * mult, 64 * mult);
	buttons[prev_x]->flip = SDL_FLIP_HORIZONTAL;
	buttons[prev_x]->content_rect = buttons[prev_x]->background_rect;
	buttons[prev_x]->content_txt = textures->single_arrow;
	menu_functions[prev_x] = f_prev_x;

	set_pos(&buttons[next_o]->background_rect, (window_width - 128) / (high_res ? 1 : 2) + 400 * mult, 552 * mult, 64 * mult, 64 * mult);
	buttons[next_o]->content_rect = buttons[next_o]->background_rect;
	buttons[next_o]->content_txt = textures->single_arrow;
	menu_functions[next_o] = f_next_o;

	set_pos(&buttons[prev_o]->background_rect, (window_width - 128) / (high_res ? 1 : 2) + 164 * mult, 552 * mult, 64 * mult, 64 * mult);
	buttons[prev_o]->flip = SDL_FLIP_HORIZONTAL;
	buttons[prev_o]->content_rect = buttons[prev_o]->background_rect;
	buttons[prev_o]->content_txt = textures->single_arrow;
	menu_functions[prev_o] = f_prev_o;

	set_pos(&buttons[switch_online]->background_rect, (window_width - 256) / (high_res ? 1 : 2), 436 * mult, 256 * mult, 64 * mult);
	buttons[switch_online]->content_rect = buttons[switch_online]->background_rect;
	buttons[switch_online]->content_txt = load_from_text(sdl_data, &buttons[switch_online]->content_rect, "vs BOT");
	menu_functions[switch_online] = f_switch_online;

	return 1;
}

bool init_playfield(Sdl_Data *sdl_data)
{
	sdl_data->playfield = malloc(sizeof(Playfield));
	Playfield *playfield = sdl_data->playfield;
	if (playfield == NULL)
	{
		fprintf(stderr, "Unable to allocate memory\n");
		return 0;
	}

	playfield->normal_cells = malloc(max_normal_cells * sizeof(Cell *));
	if (playfield->normal_cells == NULL)
	{
		fprintf(stderr, "Unable to allocate memory\n");
		return 0;
	}

	for (int i = 0; i < max_normal_cells; i++)
	{
		playfield->normal_cells[i] = malloc(sizeof(Cell));
		if (playfield->normal_cells[i] == NULL)
		{
			fprintf(stderr, "Unable to allocate memory\n");
			return 0;
		}
	}

	playfield->super_cells = malloc(max_super_cells * sizeof(Cell *));
	if (playfield->super_cells == NULL)
	{
		fprintf(stderr, "Unable to allocate memory\n");
		return 0;
	}

	for (int i = 0; i < max_super_cells; i++)
	{
		playfield->super_cells[i] = malloc(sizeof(Cell));
		if (playfield->super_cells[i] == NULL)
		{
			fprintf(stderr, "Unable to allocate memory\n");
			return 0;
		}
	}

	Textures *textures = sdl_data->textures;

	playfield->background = malloc(sizeof(Object));
	playfield->timer = malloc(sizeof(Object));
	playfield->put_sign = malloc(sizeof(Button));
	playfield->forfeit = malloc(sizeof(Button));
	if (playfield->background == NULL || playfield->timer == NULL || playfield->put_sign == NULL || playfield->forfeit == NULL)
	{
		fprintf(stderr, "Unable to allocate memory\n");
		return 0;
	}

	int mult = high_res ? 2 : 1;

	set_pos(&playfield->background->background_rect, 100 * mult, 100 * mult, board_size * mult, board_size * mult);
	playfield->background->content_rect = playfield->background->background_rect;
	playfield->background->background_txt = sdl_data->textures->field_background;
	playfield->background->content_txt = NULL;

	set_pos(&playfield->timer->background_rect, 650 * mult, 100 * mult, 256 * mult, 128 * mult);
	playfield->timer->content_rect = playfield->timer->background_rect;
	playfield->timer->background_txt = sdl_data->textures->field_background;
	playfield->timer->content_txt = NULL;

	set_pos(&playfield->put_sign->background_rect, 650 * mult, 286 * mult, 256 * mult, 128 * mult);
	playfield->put_sign->flip = SDL_FLIP_NONE;
	playfield->put_sign->background_txt = textures->field_background;
	playfield->put_sign->content_rect = playfield->put_sign->background_rect;
	playfield->put_sign->content_txt = load_from_text(sdl_data, &playfield->put_sign->content_rect, "PUT SIGN");
	sdl_data->put_sign = f_put_sign;

	set_pos(&playfield->forfeit->background_rect, 650 * mult, 472 * mult, 256 * mult, 128 * mult);
	playfield->forfeit->flip = SDL_FLIP_NONE;
	playfield->forfeit->background_txt = textures->field_background;
	playfield->forfeit->content_rect = playfield->forfeit->background_rect;
	playfield->forfeit->content_txt = NULL;
	sdl_data->forfeit = f_forfeit;

	sdl_data->select_cell = f_select_cell;

	return 1;
}

void setup_cells(Sdl_Data *sdl_data)
{
	int n_size = board_size * (high_res ? 2 : 1) / 3, s_size = board_size * (high_res ? 2 : 1) / 9;
	int x = sdl_data->playfield->background->background_rect.x;
	int y = sdl_data->playfield->background->background_rect.y;
	Cell *cell;

	for (int i = 0; i < max_super_cells; i++)
	{
		cell = sdl_data->playfield->super_cells[i];
		set_pos(&cell->rect, x + i % 9 * s_size, y + i / 9 * s_size, s_size, s_size);
	}

	for (int i = 0; i < max_normal_cells; i++)
	{
		cell = sdl_data->playfield->normal_cells[i];
		set_pos(&cell->rect, x + i % 3 * n_size, y + i / 3 * n_size, n_size, n_size);
	}

	sdl_data->select_x = -1;
	sdl_data->select_y = -1;
}

void highlight(Sdl_Data *sdl_data, SDL_Rect *rect)
{
	SDL_BlendMode blend_mode;
	SDL_Color *color = &sdl_data->pallete->spot_highlight;
    SDL_GetRenderDrawBlendMode(sdl_data->renderer, &blend_mode);
	SDL_SetRenderDrawBlendMode(sdl_data->renderer, SDL_BLENDMODE_MUL);
    SDL_SetRenderDrawColor(sdl_data->renderer, color->r, color->g, color->b, color->a);
    SDL_RenderFillRect(sdl_data->renderer, rect);
    SDL_SetRenderDrawBlendMode(sdl_data->renderer, blend_mode);
	SDL_RenderPresent(sdl_data->renderer);
}

void render_cell(Sdl_Data *sdl_data, Cell *cell, int sign)
{
	if (sign == EMPTY)
	{
		return;
	}

	if(sign == DRAW)
	{
		SDL_Rect rect;
		SDL_Texture *txt = load_from_text(sdl_data, &rect, "DRAW");

		double w_mult = (double)rect.w / (double)cell->rect.w;
		double h_mult = (double)rect.h / (double)cell->rect.h;
		double mult = fmax(w_mult, h_mult) * 1.05f;

		rect.w = (double)rect.w / mult;
		rect.h = (double)rect.h / mult;
		rect.x = (cell->rect.w - rect.w) / 2 + cell->rect.x;
		rect.y = (cell->rect.h - rect.h) / 2 + cell->rect.y;

		SDL_RenderCopy(sdl_data->renderer, txt, NULL, &rect);
		SDL_DestroyTexture(txt);
		return;
	}

	SDL_Texture *txt = sign == X ? sdl_data->textures->sign_x : sdl_data->textures->sign_o;
	int clip = sign == X ? sdl_data->pallete->sprite_x : sdl_data->pallete->sprite_o;
	SDL_Rect sprite_clip;
	sprite_clip.x = 128 * (clip);
	sprite_clip.y = 0;
	sprite_clip.w = 128;
	sprite_clip.h = 128;

	SDL_RenderCopy(sdl_data->renderer, txt, &sprite_clip, &cell->rect);
}

void render_object(SDL_Renderer *renderer, Object *object, int clip)
{
	SDL_RenderCopy(renderer, object->background_txt, NULL, &object->background_rect);

	if (clip == 0)
	{
		double w_mult = (double)object->content_rect.w / (double)object->background_rect.w;
		double h_mult = (double)object->content_rect.h / (double)object->background_rect.h;
		double mult = fmax(w_mult, h_mult) * 1.05f;

		SDL_Rect centered_content;
		centered_content.w = (double)object->content_rect.w / mult;
		centered_content.h = (double)object->content_rect.h / mult;
		centered_content.x = (object->background_rect.w - centered_content.w) / 2 + object->background_rect.x;
		centered_content.y = (object->background_rect.h - centered_content.h) / 2 + object->background_rect.y;

		SDL_RenderCopy(renderer, object->content_txt, NULL, &centered_content);
	}
	else
	{
		SDL_Rect sprite_clip;
		sprite_clip.x = 128 * (clip - 1);
		sprite_clip.y = 0;
		sprite_clip.w = 128;
		sprite_clip.h = 128;
		SDL_RenderCopyEx(renderer, object->content_txt, &sprite_clip, &object->content_rect, 0, NULL, SDL_FLIP_NONE);
	}
}

void render_button(SDL_Renderer *renderer, Button *button, int clip)
{
	SDL_RenderCopy(renderer, button->background_txt, NULL, &button->background_rect);

	if (clip == 0)
	{
		double w_mult = (double)button->content_rect.w / (double)button->background_rect.w;
		double h_mult = (double)button->content_rect.h / (double)button->background_rect.h;
		double mult = fmax(w_mult, h_mult) * 1.05f;

		SDL_Rect centered_content;
		centered_content.w = (double)button->content_rect.w / mult;
		centered_content.h = (double)button->content_rect.h / mult;
		centered_content.x = (button->background_rect.w - centered_content.w) / 2 + button->background_rect.x;
		centered_content.y = (button->background_rect.h - centered_content.h) / 2 + button->background_rect.y;

		SDL_RenderCopyEx(renderer, button->content_txt, NULL, &centered_content, 0, NULL, button->flip);
	}
	else
	{
		SDL_Rect sprite_clip;
		sprite_clip.x = 512 * (clip - 1);
		sprite_clip.y = 0;
		sprite_clip.w = 512;
		sprite_clip.h = 512;
		SDL_RenderCopyEx(renderer, button->content_txt, &sprite_clip, &button->content_rect, 0, NULL, button->flip);
	}
}

void render_menu(Sdl_Data *sdl_data)
{
	Menu *menu = sdl_data->menu;
	SDL_Renderer *renderer = sdl_data->renderer;

	SDL_SetRenderDrawColor(sdl_data->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(sdl_data->renderer);

	render_object(renderer, menu->logo, 0);
	render_object(renderer, menu->sign_x, sdl_data->pallete->sprite_x + 1);
	render_object(renderer, menu->sign_o, sdl_data->pallete->sprite_o + 1);
	render_object(renderer, menu->game_id, 0);

	for (int i = 0; i < menu_buttons; i++)
	{
		if(i == sub_mult_id || i == add_mult_id)
		{
			continue;
		}
		render_button(renderer, menu->buttons[i], i == mode ? (int)sdl_data->super_mode + 1 : 0);
	}

	SDL_RenderPresent(renderer);
}

void render_board(Sdl_Data *sdl_data)
{
	Playfield *playfield = sdl_data->playfield;
	SDL_Renderer *renderer = sdl_data->renderer;
	int side = sdl_data->super_mode ? 9 : 3;
	int size_bold = board_size * (high_res ? 2 : 1) / 3, size = board_size * (high_res ? 2 : 1) / 9, x, y;
	SDL_Rect *board_rect = &sdl_data->playfield->background->background_rect;
	Board **board = sdl_data->game->board;

	render_object(renderer, playfield->background, 0);

	if (sdl_data->super_mode)
	{
		if(sdl_data->select_board != -1)
		{
			SDL_Rect rect;
			rect.x = board_rect->x + sdl_data->select_board % 3 * size_bold;
			rect.y = board_rect->y + sdl_data->select_board / 3 * size_bold;
			rect.w = size_bold;
			rect.h = size_bold;
			highlight(sdl_data, &rect);
		}

		for(int i = 0; i < max_super_cells; i++)
		{
			render_cell(sdl_data, sdl_data->playfield->super_cells[i], board[(i % 9) / 3 + (i / 27) * 3]->value[i % 3][(i % 27) / 9]);
		}

		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

		for (int i = 1; i < side; i++)
		{
			if (i % 3 == 0)
			{
				continue;
			}
			x = board_rect->x;
			y = board_rect->y;
			SDL_RenderDrawLine(sdl_data->renderer, x + size * i, y, x + size * i, y + board_size * (high_res ? 2 : 1));
			SDL_RenderDrawLine(sdl_data->renderer, x, y + size * i, x + board_size * (high_res ? 2 : 1), y + size * i);
		}
	}

	for(int i = 0; i < max_normal_cells; i++)
	{
		render_cell(sdl_data, sdl_data->playfield->normal_cells[i], sdl_data->super_mode ? board[i]->status : board[0]->value[i % 3][i / 3]);
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0x40);

	for (int i = 1; i < 3; i++)
	{
		x = board_rect->x;
		y = board_rect->y;
		SDL_RenderDrawLine(sdl_data->renderer, x + size_bold * i, y, x + size_bold * i, y + board_size * (high_res ? 2 : 1));
		SDL_RenderDrawLine(sdl_data->renderer, x, y + size_bold * i, x + board_size * (high_res ? 2 : 1), y + size_bold * i);
	}
}

void render_playfield(Sdl_Data *sdl_data)
{
	Playfield *playfield = sdl_data->playfield;
	SDL_Renderer *renderer = sdl_data->renderer;

	SDL_SetRenderDrawColor(sdl_data->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(sdl_data->renderer);

	render_object(renderer, playfield->timer, 0);
	render_button(renderer, playfield->put_sign, 0);
	render_button(renderer, playfield->forfeit, 0);

	render_board(sdl_data);
	SDL_RenderPresent(renderer);
}

bool load_media(Sdl_Data *sdl_data)
{
	sdl_data->game_id = 0;
	sdl_data->in_game = 0;
	sdl_data->super_mode = 0;
	sdl_data->bot_difficulty = 0;
	sdl_data->on_lan = 0;

	if (!init_textures(sdl_data) || !init_pallete(sdl_data) || !init_menu(sdl_data) || !init_playfield(sdl_data))
	{
		return 0;
	}

	render_menu(sdl_data);

	return 1;
}

void handle_menu_event(Sdl_Data *sdl_data, SDL_Event event)
{
	Menu *menu = sdl_data->menu;

	if (event.type == SDL_MOUSEBUTTONDOWN)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		SDL_Rect *hitbox;

		for (int i = 0; i < menu_buttons; i++)
		{
			if(i == sub_mult_id || i == add_mult_id)
			{
				continue;
			}
			hitbox = &menu->buttons[i]->background_rect;
			if (x > hitbox->x && y > hitbox->y && x < hitbox->x + hitbox->w && y < hitbox->y + hitbox->h)
			{
				sdl_data->menu_functions[i](sdl_data);
				if(i != play)
				{
					highlight(sdl_data, &menu->buttons[i]->background_rect);
				}
				break;
			}
		}
		SDL_RenderPresent(sdl_data->renderer);
	}
	else if (event.type == SDL_MOUSEBUTTONUP)
	{
		render_menu(sdl_data);
	}
}

void handle_ingame_event(Sdl_Data *sdl_data, SDL_Event event)
{
	Playfield *playfield = sdl_data->playfield;

	if(event.type == SDL_MOUSEBUTTONDOWN)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		SDL_Rect *hitbox;

		hitbox = &playfield->put_sign->background_rect;
		if (x > hitbox->x && y > hitbox->y && x < hitbox->x + hitbox->w && y < hitbox->y + hitbox->h)
		{
			sdl_data->put_sign(sdl_data);
			highlight(sdl_data, hitbox);
			SDL_RenderPresent(sdl_data->renderer);
			return;
		}

		hitbox = &playfield->forfeit->background_rect;
		if (x > hitbox->x && y > hitbox->y && x < hitbox->x + hitbox->w && y < hitbox->y + hitbox->h)
		{
			sdl_data->forfeit(sdl_data);
			highlight(sdl_data, hitbox);
			SDL_RenderPresent(sdl_data->renderer);
			return;
		}

		if(sdl_data->super_mode)
		{
			for (int i = 0; i < max_super_cells; i++)
			{
				hitbox = &playfield->super_cells[i]->rect;
				if (x > hitbox->x && y > hitbox->y && x < hitbox->x + hitbox->w && y < hitbox->y + hitbox->h)
				{
					sdl_data->select_cell(sdl_data, x, y);
					highlight(sdl_data, hitbox);
					SDL_RenderPresent(sdl_data->renderer);
					return;
				}
			}
		}
		else
		{
			for(int i = 0; i < max_normal_cells; i++)
			{
				hitbox = &playfield->normal_cells[i]->rect;
				if (x > hitbox->x && y > hitbox->y && x < hitbox->x + hitbox->w && y < hitbox->y + hitbox->h)
				{
					sdl_data->select_cell(sdl_data, x, y);
					highlight(sdl_data, hitbox);
					SDL_RenderPresent(sdl_data->renderer);
					return;
				}
			}
		}	
	}
	else if(event.type == SDL_MOUSEBUTTONUP)
	{
		render_playfield(sdl_data);
	}
}

void frame_events(Sdl_Data *sdl_data, bool *quit)
{
	SDL_Event event;

	while(SDL_PollEvent(&event) != 0)
	{
		if(event.type == SDL_QUIT)
		{
			*quit = 1;
		}

		if(sdl_data->in_game)
		{
			handle_ingame_event(sdl_data, event);
		}
		else
		{
			handle_menu_event(sdl_data, event);
		}
	}

	int s_left = seconds_left(sdl_data->game->timer);
	if(sdl_data->game->status == IN_PROGRESS && sdl_data->in_game && s_left != sdl_data->last_time)
	{
		sdl_data->last_time = s_left;
		if(s_left == 0)
		{
			sdl_data->game->status = DRAW;
			stop_game(sdl_data);
		}
		else
		{
			char buffer[buffer_size];
			if(s_left % 60 == 0)
			{
				sprintf(buffer, "%d:00", s_left / 60);
			}
			else
			{
				sprintf(buffer, "%d:%2d", s_left / 60, s_left % 60);
			}
			free_txt(&sdl_data->playfield->timer->content_txt);
			sdl_data->playfield->timer->content_txt = load_from_text(sdl_data, &sdl_data->playfield->timer->content_rect, buffer);
			render_object(sdl_data->renderer, sdl_data->playfield->timer, 0);
			SDL_RenderPresent(sdl_data->renderer);
		}
	}
}
