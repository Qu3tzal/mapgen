#include <ctime>
#include <random>
#include <iostream>

#include <SFML/Graphics.hpp>

const float VIEW_MOVING_SPEED = 200.f;
const unsigned int MAP_WIDTH = 1000;
const unsigned int MAP_HEIGHT = 1000;
const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 800;
const unsigned int TICKS_PER_SECOND = 60;
const sf::Time TICK_TIME = sf::seconds(1.f / TICKS_PER_SECOND);

class Map : public sf::Transformable, public sf::Drawable
{
    public:
        // Constructor.
        explicit Map()
            : _vertexArray(sf::PrimitiveType::Quads, MAP_WIDTH * MAP_HEIGHT * 4) // Ouch, that makes a lot.
            , _playerPosition(1, 1)
        {
            for(unsigned int x(0) ; x < MAP_WIDTH ; ++x)
            {
                for(unsigned int y(0) ; y < MAP_HEIGHT ; ++y)
                {
                    unsigned int offset = (y * MAP_HEIGHT + x) * 4;

                    _vertexArray[offset].position = sf::Vector2f(x, y);
                    _vertexArray[offset + 1].position = sf::Vector2f(x + 1, y);
                    _vertexArray[offset + 2].position = sf::Vector2f(x + 1, y + 1);
                    _vertexArray[offset + 3].position = sf::Vector2f(x, y + 1);

                    _vertexArray[offset].color = sf::Color::White;
                    _vertexArray[offset + 1].color = sf::Color::White;
                    _vertexArray[offset + 2].color = sf::Color::White;
                    _vertexArray[offset + 3].color = sf::Color::White;
                }
            }
        }

        // Destructor.
        virtual ~Map(){}

        // Generate the map.
        void generate(int seed)
        {
            // Create the RNG.
            std::mt19937 mt(seed);
            std::uniform_real_distribution<double> dist(0.0, 1.0);

            // Create the map's data.
            std::vector<std::vector<Tile>> mapDataPreErode(MAP_WIDTH, std::vector<Tile>(MAP_HEIGHT, Tile::EMPTY));

            // Fill with random data.
            for(unsigned int x(0) ; x < MAP_WIDTH ; ++x)
            {
                for(unsigned int y(0) ; y < MAP_HEIGHT ; ++y)
                {
                    double value = dist(mt);

                    if(value >= 0.15)
                        mapDataPreErode[x][y] = Tile::WALL;
                }
            }

            // We need a new array because we want to work with the data pre-erode.
            _mapData = mapDataPreErode;

            // Erode.
            for(unsigned int x(0) ; x < MAP_WIDTH ; ++x)
            {
                for(unsigned int y(0) ; y < MAP_HEIGHT ; ++y)
                {
                    // Make sure the borders are walls.
                    if(x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1)
                    {
                        _mapData[x][y] = Tile::WALL;
                    }
                    else
                    {
                        // Only keep the dot as black if all the dots around are black too.
                        // All the dots around in a 4-connected graph.
                        if(mapDataPreErode[x][y] == Tile::WALL && mapDataPreErode[x - 1][y] == Tile::WALL && mapDataPreErode[x + 1][y] == Tile::WALL && mapDataPreErode[x][y - 1] == Tile::WALL && mapDataPreErode[x][y + 1] == Tile::WALL)
                            _mapData[x][y] = Tile::WALL;
                        else
                            _mapData[x][y] = Tile::EMPTY;
                    }
                }
            }

            // Set the player initial position.
            _mapData[1][1] = Tile::PLAYER;
            updateVertexArray();
        }

        // Returns the player position.
        sf::Vector2i getPlayerPosition() const
        {
            return _playerPosition;
        }

        // Destroy the wall.
        void destroyWall(sf::Vector2i wallPosition)
        {
            // Update the map data and the vertex array.
            _mapData[wallPosition.x][wallPosition.y] = Tile::EMPTY;
            setTileColor(wallPosition.x, wallPosition.y, sf::Color::White);
        }

        // Move the player.
        void movePlayer(sf::Vector2i movement)
        {
            if(movement.x == 0 && movement.y == 0)
                return;

            sf::Vector2i targetPosition = _playerPosition + movement;

            // Check the target position is not a wall.
            if(_mapData[targetPosition.x][targetPosition.y] != Tile::WALL)
            {
                // Set the new position as player.
                _mapData[targetPosition.x][targetPosition.y] = Tile::PLAYER;

                // Set the previous position as empty.
                _mapData[targetPosition.x][targetPosition.y] = Tile::EMPTY;

                // Update the vertex array.
                setTileColor(targetPosition.x, targetPosition.y, sf::Color::Red);
                setTileColor(_playerPosition.x, _playerPosition.y, sf::Color::White);

                // Update the player position.
                _playerPosition = targetPosition;
            }
        }

    private:
        // Tile values enum.
        enum class Tile
        {
            EMPTY,
            WALL,
            PLAYER
        };

        // The draw overload of sf::Drawable::draw
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
        {
            states.transform *= getTransform();
            target.draw(_vertexArray);
        }

        // Update the vertex array data from the map data.
        void updateVertexArray()
        {
            for(unsigned int x(0) ; x < MAP_WIDTH ; ++x)
            {
                for(unsigned int y(0) ; y < MAP_HEIGHT ; ++y)
                {
                    switch(_mapData[x][y])
                    {
                        case Tile::PLAYER: // Player
                            setTileColor(x, y, sf::Color::Red);
                            break;
                        case Tile::WALL: // Wall
                            setTileColor(x, y, sf::Color::Black);
                            break;
                        case Tile::EMPTY: // Empty
                        default:
                            setTileColor(x, y, sf::Color::White);
                            break;
                    }
                }
            }
        }

        // Set the color for one tile.
        void setTileColor(unsigned int x, unsigned int y, sf::Color color)
        {
            unsigned int offset = (y * MAP_HEIGHT + x) * 4;
            _vertexArray[offset].color = color;
            _vertexArray[offset + 1].color = color;
            _vertexArray[offset + 2].color = color;
            _vertexArray[offset + 3].color = color;
        }

    private:
        // The map data.
        std::vector<std::vector<Tile>> _mapData;

        // The vertex array.
        sf::VertexArray _vertexArray;

        // The player position.
        sf::Vector2i _playerPosition;
};

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
    Map map;
    map.generate(seed);

    // Create the window.
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "mapplay 1.0");

    // Set a zoomed view.
    sf::View view = window.getView();
    view.setSize(WINDOW_WIDTH * 0.1f, WINDOW_HEIGHT * 0.1f);
    window.setView(view);

    // Main loop.
    sf::Clock clock;
    sf::Time elapsed(sf::Time::Zero);

    while(window.isOpen())
    {
        // Get the elapsed time.
        sf::Time dt = clock.restart();
        elapsed += dt;

        /// Event handling.
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            else if(event.type == sf::Event::KeyReleased)
            {
                if(event.key.code == sf::Keyboard::Space) // Destroy the walls around the player.
                {
                    sf::Vector2i playerPosition = map.getPlayerPosition();

                    map.destroyWall(playerPosition + sf::Vector2i(1, 0));
                    map.destroyWall(playerPosition + sf::Vector2i(-1, 0));
                    map.destroyWall(playerPosition + sf::Vector2i(0, 1));
                    map.destroyWall(playerPosition + sf::Vector2i(0, -1));
                }
            }
        }

        // Update.
        if(elapsed >= TICK_TIME)
        {
            elapsed -= TICK_TIME;

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
                map.movePlayer(sf::Vector2i(-1, 0));
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                map.movePlayer(sf::Vector2i(1, 0));
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
                map.movePlayer(sf::Vector2i(0, -1));
            else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                map.movePlayer(sf::Vector2i(0, 1));
        }

        // Make sure the view is always centered on the player position.
        sf::View view = window.getView();
        view.setCenter(sf::Vector2f(map.getPlayerPosition().x, map.getPlayerPosition().y));
        window.setView(view);

        // Rendering.
        window.clear(sf::Color::White);
        window.draw(map);
        window.display();
    }
}
