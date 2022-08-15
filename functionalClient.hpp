#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <utility>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <string_view>
#include <vector>
#include <future>
#include <cstring>
#include <charconv>
#include <algorithm>

SDL_Renderer *renderTarget = nullptr;
std::vector<char> buf;
SDL_Rect you{0,0,100,100};


void error(const char* msg) {
    perror(msg);
    exit(1);
}

SDL_Texture* createTexture(std::string filePath) {
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = nullptr;
    surface = IMG_Load(filePath.c_str());
    if (surface == nullptr) {
        std::cout << "couldn't load surface " << SDL_GetError() << "\n";
    } else {
        texture = SDL_CreateTextureFromSurface(renderTarget, surface);
        SDL_FreeSurface(surface);
        surface = nullptr;
        if (texture == nullptr) {
            std::cout << "couldn't load texture " << SDL_GetError() << "\n";
        } else {
            return texture;
        }
    }
    return nullptr;
}

SDL_Texture* yourTexture = nullptr;
const SDL_Rect yourTextRect{0,0,66,68};
SDL_Texture* otherTexture = nullptr;
const SDL_Rect otherTextRect{0,0,66,66};

const SDL_Scancode keys[4] = {SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D};

void sending(int& sockfd, std::string&& str) {
    /*
    memset(buf, 0, 256);
    strcpy(buf, str.data());
    if (write(sockfd, buf, strlen(buf)) < 0) error("ERROR SENDING");*/
    if (send(sockfd, str.c_str(), str.size(), 0) < 0) error("ERROR SENDING");
}

void receive(int& sockfd, std::string& str, std::vector<char>& buff) {
    char buf[256];
    int n;
    str = "";
    do {
        n = recv(sockfd, buf, sizeof(buf) - 1, 0);
        if (n < 0) {
            error("ERROR RECEIVING");
        } else {
            str += buf;
        }
    } while(n == 256);
    std::cout << n << "\n";
    std::cout << str << "\n";
    /*
    for (auto& c : buf) {
        std::cout << c;
    }
    std::cout << "\n";
    //buf.shrink_to_fit();

    //std::transform(buf.begin(), buf.end(), std::back_inserter(str), [](char c) {return c;});
    for (char c : buf) {
        str += c;
    }
    std::cout << str << "\n";*/

}

void renderAll(const std::string&str, std::vector<std::future<void>>& futures) {
    futures.clear();
    int x, y;
    int end = str.size() - 7;
    for (int i = 0; i < end; i+=8) { 
        std::cout << str.size() << "\n";
        //std::from_chars(str.data() + i, str.data() + i + 4, x);
        //std::from_chars(str.data() + i + 4, str.data() + i + 8, y);
        x = std::stoi(str.substr(i, 4));
        y = std::stoi(str.substr(i + 4, 4));
        /*futures.push_back(std::async(std::launch::async, [](int x, int y) {
            SDL_Rect r{x,y,40,40};
            SDL_RenderDrawRect(renderTarget, &r);
        }, x, y));*/
        SDL_Rect r;
        r.x = x; r.y = y; r.w = 100; r.h = 100;
        if (r.x == you.x && r.y == you.y) {
            SDL_RenderCopy(renderTarget, yourTexture, &yourTextRect, &r);
        } else {
            SDL_RenderCopy(renderTarget, otherTexture, &otherTextRect, &r);
        }
        std::cout << you.x << " " << you.y << "\n";
        std::cout << "drew a rectangle\n";
    }
}

/*
class client {
    public:
        int sockfd, port;
        std::string_view received;

        client(int port, hostent* name) {
            this->port = port;
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) error("ERROR OPENING SOCKET");
            server = name;
            if (server == NULL) {
                fprintf(stderr, "ERROR, NO SUCH HOST\n");
                exit(0);
            }
            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            memcpy(server->h_addr_list, &serv_addr.sin_addr.s_addr, server->h_length);
            serv_addr.sin_port = htons(port);
            if (connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {error("ERROR CONNECTING");} else {std::cout << "Successfully connected\n";}
            keys[0] = SDL_SCANCODE_W;
            keys[1] = SDL_SCANCODE_S;
            keys[2] = SDL_SCANCODE_A;
            keys[3] = SDL_SCANCODE_D;
        }

        void yeet(std::string_view str) {
            memset(buf, 0, 256);
            strcpy(buf, str.data());
            if (write(sockfd,buf, strlen(buf)) < 0) error("ERROR WRITING TO SOCKET");
        }

        void receive() {
            memset(buf, 0, 256);
            int numb;
            if ((numb = recv(sockfd, buf, 255, 0)) == -1) {
                error("ERROR RECV-ING");
            }
            buf[numb] = '\0';
            received = buf;
        }
        
        void renderAll() {
            futures.clear();
            int x, y;
            for (int i = 0; i < received.size(); i+=8) {
                std::from_chars(received.data() + i, received.data() + i + 4, x);
                std::from_chars(received.data() + i + 4, received.data() + i + 8,y);
                futures.push_back(std::async(std::launch::async,[](int x, int y) {
                    SDL_Rect r{x,y,40,40};
                    SDL_RenderDrawRect(renderTarget, &r);
                }, x, y));
            }
        }
        void move(const Uint8 *keyState) {
            if (keyState[keys[0]]) {
                yeet("w");
            } else if (keyState[keys[1]]) {
                yeet("s");
            } else if (keyState[keys[2]]) {
                yeet("a");
            } else if (keyState[keys[3]]) {
                yeet("d");
            }
        }
        
        void render(int x, int y) {
            SDL_Rect r{x,y,40,40};
            SDL_RenderDrawRect(renderTarget, &r);
        }
        ~client() {
            close(sockfd);
        }

    private:
        struct sockaddr_in serv_addr;
        struct hostent *server;
        char buf[256];
        std::vector<std::future<void>> futures;
        SDL_Scancode keys[4];
};*/