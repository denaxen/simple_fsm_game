#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <list>

using namespace std;
const int window_width = 1200;
const int window_height = 900;

struct rect
{
	sf::Vector2f a, b;
};

struct platf
{
    sf::Vector2f a, b;
};

class Animation
{
private:
	sf::IntRect texture_rect;

	int number_of_frames;
	float animation_speed;

	float time;

public:
	Animation() {}

	Animation(sf::IntRect texture_rect, int number_of_frames, float animation_speed) :
			  number_of_frames(number_of_frames), animation_speed(animation_speed), texture_rect(texture_rect),
			  time(0)
	{
	}

	void update(float dt)
	{
		time += dt;
		if (time > number_of_frames / animation_speed)
			time -= number_of_frames / animation_speed;
	}

	void set_sprite(sf::Sprite& sprite, bool is_faced_right)
	{
		int current_frame = (int)(animation_speed * time) % number_of_frames;
		if (is_faced_right)
		{
			texture_rect.left = current_frame * texture_rect.width;
			sprite.setTextureRect(texture_rect);
		}
		else
		{
			texture_rect.left = (current_frame + 1) * texture_rect.width;
			texture_rect.width *= -1;
			sprite.setTextureRect(texture_rect);
			texture_rect.width *= -1;
		}
	}
};


class Player;

class PlayerState
{
protected:
	Animation animation;
public:
	PlayerState()
	{
	}
	virtual void start_running(Player* sm) = 0;
	virtual void stop(Player* sm) = 0;
	virtual void jump(Player* sm) = 0;
	virtual void start_sliding(Player* sm) = 0;
	virtual void hook(Player* sm) = 0;
	void update(float dt)
	{
		animation.update(dt);
	}
	void set_sprite(sf::Sprite& sprite, int direction)
	{
		animation.set_sprite(sprite, direction);
	}
	virtual ~PlayerState() {};
};

class Idle : public PlayerState
{
public:
	Idle()
	{
		animation = Animation({0, 0, 19, 34}, 12, 12);
		cout << "Creating Idle state" << endl;
	}
	void start_running(Player* player);
	void stop(Player* player);
	void jump(Player* sm);
	void start_sliding(Player* sm);
	void hook(Player* sm);
};

class Running : public PlayerState
{
public:
	Running()
	{
		animation = Animation({0, 64, 21, 34}, 8, 12);
		cout << "Creating Running state" << endl;
	}
	void start_running(Player* player);
	void stop(Player* player);
	void jump(Player* sm);
	void start_sliding(Player* sm);
	void hook(Player* sm);
};

class Jumping : public PlayerState
{
public:
	Jumping()
	{
		animation = Animation({0, 229, 20, 36}, 2, 1);
		cout << "Creating Jumping state" << endl;
	}
	void start_running(Player* player);
	void stop(Player* player);
	void jump(Player* sm);
	void start_sliding(Player* sm);
	void hook(Player* sm);
};

class Sliding : public PlayerState
{
public:
	Sliding()
	{
		animation = Animation({ 0, 190, 19, 36}, 2, 2);
		cout << "Creating Sliding state" << endl;
	}
	void jump(Player* sm);
	void start_running(Player* sm);
	void stop(Player* sm);
	void start_sliding(Player* sm);
	void hook(Player* sm);
};

class Hooked : public PlayerState
{
public:
	Hooked()
	{
		animation = Animation({ 0, 118, 20, 50 }, 6, 6);
		cout << "Creating Hooked state" << endl;
	}
	void jump(Player* sm);
	void start_running(Player* sm){}
	void stop(Player* sm){}
	void start_sliding(Player* sm){}
	void hook(Player* sm) {}
};

class World
{
private:
	sf::Texture texture_background;
	sf::Sprite sprite_background;
	list<rect> ground;
	list<platf> platforms;
public:
	World()
	{
		if (!texture_background.loadFromFile("./background.png"))
    	{
        	std::cout << "Can't load image ./background.jpg" << std::endl;
        	exit(1);
    	}
    	sprite_background.setTexture(texture_background);
    	sprite_background.setPosition(0,0);
    	sprite_background.setScale(float(window_width) / float(texture_background.getSize().x), 
    		float(window_height) / float(texture_background.getSize().y));
        platforms.push_back({{230, 140}, {580, 140}});
        platforms.push_back({{180, 459}, {310, 459}});
        ground.push_back({{-10, 460}, {181, window_height}});
        ground.push_back({{180, 700}, {441, window_height}});
        ground.push_back({{440, 770}, {636, window_height}});
        ground.push_back({{635, 700}, {window_width, window_height}});
        ground.push_back({{380, 142}, {560, 350}});


    	
	}
	void draw(sf::RenderWindow& window)
	{
		
		window.draw(sprite_background);
	}
	friend class Player;
};


class Player
{
private:
	PlayerState* state;
	float sliding_timer;
	
	sf::Texture texture;
	sf::Sprite sprite;

	bool is_faced_right;
	bool on_ground;
	bool able_to_jump;
	bool on_platform;
	sf::Vector2f velocity;
	sf::Vector2f position;
	float gravity_force;
	float speed;

	void set_state(PlayerState* new_state)
	{
		delete state;
		state = new_state;
	}

	enum States
	{
		idle, jumping, running, sliding, hooked
	};
	States State;
public:

	Player(sf::Vector2f new_position)
	{
		if (!texture.loadFromFile("./hero.png"))
		{
			std::cout << "Can't load image ./hero.png for Player class" << std::endl;
			exit(1);
		}
		sprite.setTexture(texture);
		sprite.setPosition(new_position);
		sprite.setScale(3, 3);
		state = new Idle();
		State = States::idle;
		position = new_position;
		//cout << position.x << " " << position.y << " " << new_position.x << endl;
		gravity_force = 25;
		speed = 300.0;
		on_ground = false;
		sliding_timer = 0;
		able_to_jump = false;
		on_platform = false;
	}

	void gravitation (World& world)
    {
    	for (std::list<platf>::iterator it = world.platforms.begin(); it != world.platforms.end(); it++)
        {
            if (position.y > it->a.y && position.y < it->b.y + 20 && position.x < it->b.x && position.x > it->a.x && velocity.y > 0)
            {
                velocity.y = 0;
                position.y = it->a.y;
                //cout << on_ground << endl;
                on_ground = true;
                //cout << on_ground << endl;
                able_to_jump = true;
                on_platform = true;
                if (velocity.x == 0)
                    state->stop(this);
                else
                    state->start_running(this);
            }
            
            
        }
    	for (std::list<rect>::iterator it = world.ground.begin(); it != world.ground.end(); it++)
        {
	        if (position.y > it->a.y && position.y < it->b.y && position.x < it->b.x && position.x > it->a.x)
	        {
                velocity.y = 0;
                position.y = it->a.y;
                on_ground = true;
                able_to_jump = true;
                if (velocity.x == 0)
                    state->stop(this);
                else
                    state->start_running(this);
                break;
            }
            if (position.y < it->a.y && position.x < it->b.x && position.x > it->a.x && State != States::hooked)
            {
                velocity.y += gravity_force;
                //on_ground = false;
            }
        }
        
	    

    }

	void handle_ground (World& world, float dt)
	{
        position += velocity * dt;
		for (std::list<rect>::iterator it = world.ground.begin(); it != world.ground.end(); it++)
		{
		    if (position.x <= it->b.x && position.x >= it->a.x && position.y > it->a.y && position.y < it->b.y)
            {
                if (position.x <= it->b.x && position.x >= it->a.x)
                {
                	//cout << position.y << " " << it->a.y << endl;
                    position.x -= velocity.x * dt;
                    //break;
                }
                if (position.y < it->b.y)
                {
                    //position.y += velocity.y * dt;
                }
            }
		}
	}

	void handle_hook (World& world)
	{
		if (State == States::jumping)
		{
			if (position.x <= 345 && position.x >= 315 && position.y <= 535 && position.y >= 515 && !is_faced_right)
			{
				state->hook(this);
				velocity.y = 0;
				position = {330, 525};
			}
			if (position.x <= 230 && position.x >= 200 && position.y <= 240 && position.y >= 225 && is_faced_right)
			{
				state->hook(this);
				velocity.y = 0;
				position = {220, 205};
			}
		}
	}

	void update(float dt, World& world)
	{
		state->update(dt);
		if (State == States::sliding)
		{
			if (sliding_timer > 1)
				if (velocity.x != 0)
					state->start_running(this);
			if (velocity.x == 0)
			{
				state->start_running(this);
				state->stop(this);
			}
		}
		//cout << able_to_jump << endl;
		
		//cout << on_ground << endl;
		//cout << State << endl;
		cout << position.x << " " << position.y << endl;
		velocity.x = 0;
		if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) && State != States::hooked)
		{
			velocity.x -= speed;
			is_faced_right = false;
		}
		if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) && State != States::hooked)
		{
			velocity.x += speed;
			is_faced_right = true;
		}
		handle_hook(world);
		gravitation(world);
        handle_ground(world, dt);
		if (position.x > window_width - 70)
			position.x = window_width - 70;
		if (position.x < 0)
			position.x = 0;

		sprite.setPosition(position);
		sliding_timer += dt;
	}

	void draw(sf::RenderWindow& window)
	{
		state->set_sprite(sprite, is_faced_right);
		window.draw(sprite);
	}


	void handle_events(const sf::Event& event, World& world) 
	{
		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::A || event.key.code == sf::Keyboard::D)
			{
				if (State == States::idle)
					state->start_running(this);
			}

			if ((event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::Down ||
				 event.key.code == sf::Keyboard::A || event.key.code == sf::Keyboard::D || event.key.code == sf::Keyboard::S) && State == States::hooked)
			{
				if (position.y == 525)
					position.y = 536;
				if (position.y == 205)
					position.y = 241;
				state->jump(this);
			}
			if ((event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) && State == States::hooked)
			{
				if (position.y == 525)
					position = {290, 450};
				if (position.y == 205)
					position = {255, 130};
				state->jump(this);
			}

			if ((event.key.code == sf::Keyboard::W || event.key.code == sf::Keyboard::Up) && on_ground)
			{
				state->jump(this);
				position.y -= 30;
                velocity.y = -800;
                on_ground = false;
			}

			if (event.key.code == sf::Keyboard::LShift || event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
			{
				state->start_sliding(this);
				sliding_timer = 0;
			}

		}
		if (event.type == sf::Event::KeyReleased)
		{
			if ((event.key.code == sf::Keyboard::Left && !sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) || 
				(event.key.code == sf::Keyboard::A && !sf::Keyboard::isKeyPressed(sf::Keyboard::D)))
			{
				if (State != States::jumping)
					state->stop(this);
			}
			if ((event.key.code == sf::Keyboard::Right && !sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) ||
				(event.key.code == sf::Keyboard::D && !sf::Keyboard::isKeyPressed(sf::Keyboard::A)))
			{
				if (State != States::jumping)
					state->stop(this);
			}
		}
	}

	~Player()
	{
		delete state;
	}

	friend class PlayerState;
	friend class Idle;
	friend class Running;
	friend class Jumping;
	friend class Hooked;
	friend class Sliding;
};


void Idle::start_running(Player* player)
{
	player->set_state(new Running());
	player->State = player->States::running;
}

void Idle::jump(Player* player)
{
	player->set_state(new Jumping());
	player->State = player->States::jumping;
}

void Idle::stop(Player* player)
{
}

void Idle::hook(Player* player)
{
}

void Idle::start_sliding(Player* player)
{
}

void Running::start_running(Player* player)
{
}

void Running::jump(Player* player)
{
	player->set_state(new Jumping());
	player->State = player->States::jumping;
}

void Running::stop(Player* player)
{
	player->set_state(new Idle());
	player->State = player->States::idle;
}

void Running::hook(Player* player)
{
}

void Running::start_sliding(Player* player)
{
	player->set_state(new Sliding());
	player->State = player->States::sliding;
}

void Jumping::start_running(Player* player)
{

	player->set_state(new Running());
	player->State = player->States::running;
}

void Jumping::jump(Player* player)
{
}

void Jumping::stop(Player* player)
{
	player->set_state(new Idle());
	player->State = player->States::idle;
}

void Jumping::hook(Player* player)
{
	player->set_state(new Hooked());
	player->State = player->States::hooked;
}

void Jumping::start_sliding(Player* player)
{
}

void Sliding::start_sliding(Player* player)
{
}
void Sliding::jump(Player* player)
{
	player->set_state(new Jumping());
	player->State = player->States::jumping;
}
void Sliding::stop(Player* player)
{
}
void Sliding::start_running(Player* player)
{
	player->set_state(new Running());
	player->State = player->States::running;
}
void Sliding::hook(Player* player)
{}
void Hooked::jump(Player* player)
{
	player->set_state(new Jumping());
	player->State = player->States::jumping;
}


int main () 
{
	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Player states");
	window.setFramerateLimit(60);

	float time = 0;
	float dt = 1.0 / 60;

	Player player({400, 400});
	World world;

	while (window.isOpen()) 
	{
		sf::Event event;
		while(window.pollEvent(event)) 
		{
			if(event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) 
			{
				window.close();
			}
			player.handle_events(event, world);
		}
		window.clear(sf::Color::Black);

		player.update(dt, world);
		world.draw(window);
		player.draw(window);

		window.display();

		time += 1.0 / 60;
	}

	return EXIT_SUCCESS;
}



/*
	Задания:
		1) Добавить остальные состояния. Они должны правильно отрисововаться.
		   При этом понадобится внести изменения и в класс Player.
		   Например, для состояния Jumping нужно будет добавить гравитацию
		2) Добавить background.png на экран. 
		   Персонаж должен правильно взаимодействовать с окружением.
		   Это проще всего сделать введя новый класс World или Game, который будет содержать
		   					экземпляр Player, а так же остальные параметры мира.

*/