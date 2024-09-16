#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <cstdlib> 
#include <cmath>   
#include <string>  

// g++ T4.cpp -o t4 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
// ./T4.exe

// Mapa
char mapa[15][17] = {
    "9333333563333339",
    "1000000000000002",
    "1088800880088802",
    "1080008888000882",
    "1000808000080002",
    "1088800008088802",
    "1000008888000002",
    "0088800000088800",
    "1080808888080802",
    "1000800000080702",
    "1880008080000882",
    "1000888088880002",
    "1080800000080802",
    "1080008008000802",
    "9444444564444449"
};

// Definir tipos de sprites
enum SpriteType { FLOOR_COIN, WALL1, WALL2, WALL3, WALL4, DOOR1, DOOR2, FLOOR, BARREL, DIRT, WARRIOR, FLOOR_PLAIN, SLIME, GHOST, CYCLOPS };

// Parâmetros de movimento e  tamanho da célula -> Variáveis de controle
int moedas = 117;
int pontos = 0;
int posx = 1, posy = 1;
int dirX = 0, dirY = 0;
int nextDirX = 0, nextDirY = 0;
const float cellSize = 30.0f;

// Dados dos inimigos
struct Enemy {
    int posx, posy;
    sf::Sprite sprite;
};

// Função para verificar se a nova posição é válida
bool canMove(int newX, int newY) {
    if (newX < 0 || newX >= 17 || newY < 0 || newY >= 15) return false;
    char tile = mapa[newY][newX];
    return tile != '1' && tile != '2' && tile != '3' && tile != '4' && tile != '8' && tile != '9';
}

// Função para lidar com o teleporte
void handleTeleport() {
    if (posx == 0 && posy == 7) { posx = 15; dirX = 1; dirY = 0; }
    else if (posx == 15 && posy == 7) { posx = 0; dirX = -1; dirY = 0; }
    else if (posx == 7 && posy == 0) { posx = 7; posy = 14; dirX = 0; dirY = 1; }
    else if (posx == 8 && posy == 0 ) { posx = 8; posy = 14; dirX = 0; dirY = 1; }
    else if (posx == 7 && posy == 14) { posx = 7; posy = 0; dirX = 0; dirY = -1; }
    else if (posx == 8 && posy == 14) { posx = 8; posy = 0; dirX = 0; dirY = -1; }
}

// Função pontos e moedas
void checkForCoin() {
    if (mapa[posy][posx] == '0') {
        mapa[posy][posx] = '7';  
        --moedas;
        pontos += 100; 
    }
}

// Função para mover inimigo em direção ao Pac-Man
void moveEnemy(Enemy& enemy, int pacManX, int pacManY) {
    int diffX = pacManX - enemy.posx;
    int diffY = pacManY - enemy.posy;

  
    if (std::abs(diffX) > std::abs(diffY)) {
        if (diffX > 0 && canMove(enemy.posx + 1, enemy.posy)) {
            enemy.posx += 1;
        } else if (diffX < 0 && canMove(enemy.posx - 1, enemy.posy)) {
            enemy.posx -= 1;
        } else if (diffY > 0 && canMove(enemy.posx, enemy.posy + 1)) {
            enemy.posy += 1;
        } else if (diffY < 0 && canMove(enemy.posx, enemy.posy - 1)) {
            enemy.posy -= 1;
        }
    } else {
        if (diffY > 0 && canMove(enemy.posx, enemy.posy + 1)) {
            enemy.posy += 1;
        } else if (diffY < 0 && canMove(enemy.posx, enemy.posy - 1)) {
            enemy.posy -= 1;
        } else if (diffX > 0 && canMove(enemy.posx + 1, enemy.posy)) {
            enemy.posx += 1;
        } else if (diffX < 0 && canMove(enemy.posx - 1, enemy.posy)) {
            enemy.posx -= 1;
        }
    }
}

// Função para verificar colisão com Pac-Man
bool checkCollisionWithPacMan(const Enemy& enemy) {
    return enemy.posx == posx && enemy.posy == posy;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1024, 768), "Pac-Man");

    // Carregar texturas
    sf::Texture textures[15];
    std::string textureFiles[15] = {
        "floorCoin.png", "wallSide1.png", "wallSide2.png", "wallUpper.png",
        "wallDown.png", "doorUpper1.png", "doorUpper2.png", "floor.png",
        "barrel.png", "dirt.png", "warrior.png", "floor.png",
        "slime.png", "ghost.png", "cyclops.png"
    };
    for (int i = 0; i < 15; ++i) {
        if (!textures[i].loadFromFile(textureFiles[i])) {
            std::cout << "Erro lendo imagem " << textureFiles[i] << "\n";
            return 0;
        }
    }
    sf::Sprite sprites[15];
    for (int i = 0; i < 15; ++i) {
        sprites[i].setTexture(textures[i]);
        sprites[i].setScale(cellSize / 16.0f, cellSize / 16.0f); 
    }


    // Inimigos nascem nas posições especificadas
    Enemy enemies[3] = {
        {7, 12, sprites[SLIME]},    
        {14, 13, sprites[GHOST]},   
        {1, 13, sprites[CYCLOPS]}  
    };

    // Offset para centralizar o labirinto
    int offsetX = (1024 - 17 * static_cast<int>(cellSize)) / 2;
    int offsetY = (768 - 15 * static_cast<int>(cellSize)) / 2;
    srand(static_cast<unsigned>(time(0)));  // Inicializar seed de números aleatórios

    // Carregar fonte e configurar o texto para o relógio
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "Erro ao carregar a fonte." << std::endl;
        return -1;
    }
    
    
    sf::Text clockText;
    clockText.setFont(font);
    clockText.setCharacterSize(24);
    clockText.setFillColor(sf::Color::White);
    clockText.setPosition(50, 50);

    float moveDelay = 0.2f; 
    float enemyMoveDelay = 0.2f;  

    sf::Clock clock;
    sf::Clock enemyClock; // Relógio separado para inimigos
    sf::Clock gameClock;
    sf::Color backgroundColor(118, 59, 54); // Cor #763b36
    sf::Text pointsText;
    sf::Music music;

    pointsText.setFont(font);
    pointsText.setCharacterSize(24);
    pointsText.setFillColor(sf::Color::White);
    pointsText.setPosition(50, 80);

    if (!music.openFromFile("music.ogg")) {
        std::cout << "Erro ao carregar a música." << std::endl;
        return -1;
    }

    music.setLoop(true); // Configura a música para tocar em loop
    music.play(); // Inicia a música

    while (window.isOpen()) {
        if (moedas == 0) {
            std::cout << "Você ganhou!" << std::endl;
            window.close();
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Left)  { nextDirX = -1; nextDirY = 0; }
                else if (event.key.code == sf::Keyboard::Right) { nextDirX = 1; nextDirY = 0; }
                else if (event.key.code == sf::Keyboard::Up)    { nextDirX = 0; nextDirY = -1; }
                else if (event.key.code == sf::Keyboard::Down)  { nextDirX = 0; nextDirY = 1; }
            }
        }

        // Atualizar o tempo do jogo
        sf::Time elapsed = gameClock.getElapsedTime();
        clockText.setString("TIME: " + std::to_string(static_cast<int>(elapsed.asSeconds())) + "s");
        pointsText.setString("POINTS: " + std::to_string(pontos));

        if (clock.getElapsedTime().asSeconds() > moveDelay) {
            if (canMove(posx + nextDirX, posy + nextDirY)) {
                dirX = nextDirX;
                dirY = nextDirY;
            }

            if (canMove(posx + dirX, posy + dirY)) {
                posx += dirX;
                posy += dirY;
            }

            handleTeleport();
            checkForCoin();

            clock.restart();
        }

        if (enemyClock.getElapsedTime().asSeconds() > enemyMoveDelay) {
            for (int i = 0; i < 3; ++i) {
                moveEnemy(enemies[i], posx, posy);
                if (checkCollisionWithPacMan(enemies[i])) {
                    std::cout << "Você foi pego!" << std::endl;
                    window.close();
                }
            }
            enemyClock.restart();
        }

        window.clear(backgroundColor);

        // Desenhar o mapa
        for (int y = 0; y < 15; ++y) {
            for (int x = 0; x < 17; ++x) {
                int spriteIndex;
                switch (mapa[y][x]) {
                    case '0': spriteIndex = FLOOR_COIN; break;
                    case '1': spriteIndex = WALL1; break;
                    case '2': spriteIndex = WALL2; break;
                    case '3': spriteIndex = WALL3; break;
                    case '4': spriteIndex = WALL4; break;
                    case '5': spriteIndex = DOOR1; break;
                    case '6': spriteIndex = DOOR2; break;
                    case '7': spriteIndex = FLOOR; break;
                    case '8': spriteIndex = BARREL; break;
                    case '9': spriteIndex = DIRT; break;
                    default:
                        sf::RectangleShape rect(sf::Vector2f(cellSize, cellSize));
                        rect.setFillColor(backgroundColor); // Usa a cor personalizada
                        rect.setPosition(offsetX + x * cellSize, offsetY + y * cellSize);
                        window.draw(rect);
                    continue;
                }

                sprites[spriteIndex].setPosition(offsetX + x * cellSize, offsetY + y * cellSize);
                window.draw(sprites[spriteIndex]);
            }
        }

        // Desenhar Pac-Man
        sprites[WARRIOR].setPosition(offsetX + posx * cellSize, offsetY + posy * cellSize);
        window.draw(sprites[WARRIOR]);

        // Desenhar inimigos
        for (int i = 0; i < 3; ++i) {
            enemies[i].sprite.setPosition(offsetX + enemies[i].posx * cellSize, offsetY + enemies[i].posy * cellSize);
            window.draw(enemies[i].sprite);
        }

        // Desenhar o relógio
        window.draw(clockText);
        window.draw(clockText);
        window.draw(pointsText);

        window.display();
    }

    return 0;

}
