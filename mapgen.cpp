#include <ctime>
#include <random>
#include <iostream>

#include <SFML/Graphics.hpp>

const float VIEW_MOVING_SPEED = 200.f;
const unsigned int MAP_WIDTH = 1000;
const unsigned int MAP_HEIGHT = 1000;
const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 800;

sf::Image generateMap(int seed)
{
    // Create the RNG.
    std::mt19937 mt(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // Create the map's data.
    std::vector<std::vector<unsigned char>> mapDataPreErode(MAP_WIDTH, std::vector<unsigned char>(MAP_HEIGHT, 0));


    // Fill with random data.
    for(unsigned int x(0) ; x < MAP_WIDTH ; ++x)
    {
        for(unsigned int y(0) ; y < MAP_HEIGHT ; ++y)
        {
            double value = dist(mt);

            if(value >= 0.15)
                mapDataPreErode[x][y] = 1;
        }
    }

    // We need a new array because we want to work with the data pre-erode.
    std::vector<std::vector<unsigned char>> mapData(mapDataPreErode);

    // Erode.
    for(unsigned int x(0) ; x < MAP_WIDTH ; ++x)
    {
        for(unsigned int y(0) ; y < MAP_HEIGHT ; ++y)
        {
            // Make sure the borders are walls.
            if(x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1)
            {
                mapData[x][y] = 1;
            }
            else
            {
                // Only keep the dot as black if all the dots around are black too.
                // All the dots around in a 4-connected graph.
                if(mapDataPreErode[x][y] == 1 && mapDataPreErode[x - 1][y] == 1 && mapDataPreErode[x + 1][y] == 1 && mapDataPreErode[x][y - 1] == 1 && mapDataPreErode[x][y + 1] == 1)
                    mapData[x][y] = 1;
                else
                    mapData[x][y] = 0;
            }
        }
    }

    // Create the image.
    sf::Image map;
    map.create(MAP_WIDTH, MAP_HEIGHT, sf::Color::White);

    // Set the data as colored pixels on the image.
    for(unsigned int x(0) ; x < map.getSize().x ; ++x)
    {
        for(unsigned int y(0) ; y < map.getSize().y ; ++y)
        {
            switch(mapData[x][y])
            {
                case 1: // Walls
                    map.setPixel(x, y, sf::Color::Black);
                    break;
                case 0: // Empty
                default:
                    map.setPixel(x, y, sf::Color::White);
                    break;
            }
        }
    }

    return map;
}

int main(int argc, char *argv[])
{
    // Init the seed.
    int seed = 0;

    // Check if we are given a specific seed to use.
    if(argc == 2)
    {
        seed = atoi(argv[1]);
    }
    else
    {
        std::random_device device;
        seed = device();
    }

    // Generate the map.
    sf::Image mapImage = generateMap(seed);

    sf::Texture mapTexture;
    mapTexture.loadFromImage(mapImage);

    sf::Sprite mapSprite;
    mapSprite.setTexture(mapTexture);

    // Create the window.
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "mapgen 1.0");

    // View mode.
    bool zoomed(false);

    // Main loop.
    sf::Clock clock;
    while(window.isOpen())
    {
        // Get the elapsed time.
        sf::Time dt = clock.restart();

        /// Event handling.
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            else if(event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Space)
            {
                if(zoomed)
                {
                    sf::View view = window.getView();
                    view.setSize(WINDOW_WIDTH / 10.f, WINDOW_HEIGHT / 10.f);
                    window.setView(view);
                }
                else
                {
                    sf::View view = window.getView();
                    view.setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
                    window.setView(view);
                }

                zoomed = !zoomed;
            }
        }

        // User input/Logic update.
        bool hasPressedKey(false);
        sf::Vector2f viewMovement(0.f, 0.f);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        {
            hasPressedKey = true;
            viewMovement += sf::Vector2f(-VIEW_MOVING_SPEED, 0.f);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            hasPressedKey = true;
            viewMovement += sf::Vector2f(VIEW_MOVING_SPEED, 0.f);
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
        {
            hasPressedKey = true;
            viewMovement += sf::Vector2f(0.f, -VIEW_MOVING_SPEED);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            hasPressedKey = true;
            viewMovement += sf::Vector2f(0.f, VIEW_MOVING_SPEED);
        }

        if(hasPressedKey)
        {
            sf::View view = window.getView();
            view.setCenter(view.getCenter() + viewMovement * dt.asSeconds());
            window.setView(view);
        }

        // Rendering.
        window.clear(sf::Color::White);
        window.draw(mapSprite);
        window.display();
    }
}
