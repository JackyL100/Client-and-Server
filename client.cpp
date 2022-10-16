#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
//#include "client.hpp"
#include "functionalClient.hpp"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "usage: HOSTNAME PORT\n";
        exit(0);
    }
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Client", SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, 860, 640,SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL) {
        std::cout << "could not create window: " << SDL_GetError() << "\n";
        return 1;
    }
    renderTarget = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderTarget == nullptr) {
        std::cout << "could not create renderer:" << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Texture *texture = nullptr;
    texture = createTexture("bigBg.jpg");
    yourTexture = createTexture("DeoxysSheet.png");
    otherTexture = createTexture("RayquazaSheet.png");
    SDL_Event windowEvent;
    const Uint8* keyState;
    bool isRunning = true;
    int sockfd, port;
    std::string received;
    struct sockaddr_in serv_addr;

    port = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR OPENING SOCKET");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
    auto move = [&](const Uint8 *keyState) {
        if (keyState[keys[0]]) {
            sending(sockfd, "w");
            you.y -= 5;
        } else if (keyState[keys[1]]) {
            sending(sockfd, "s");
            you.y += 5;
        } else if (keyState[keys[2]]) {
            sending(sockfd, "a");
            you.x -= 5;
        } else if (keyState[keys[3]]) {
            sending(sockfd, "d");
            you.x += 5;
        }
    };
    std::vector<std::future<void>> myFuture;
    received = "";
    //client bear(atoi(argv[2]), gethostbyname(argv[1]));
    if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) error("ERROR CONNECTING");
    SDL_SetRenderDrawColor(renderTarget, 255,0,0,255);

    std::thread receivingThread([&]() {while(isRunning) {
        receive(sockfd, received, buf);
        SDL_RenderClear(renderTarget);
        SDL_RenderCopy(renderTarget, texture, NULL, NULL);
        renderAll(received, myFuture);
        SDL_RenderPresent(renderTarget);
    }});

    while(isRunning) {
        if (SDL_PollEvent(&windowEvent)) {
            if (windowEvent.type == SDL_QUIT) {
                isRunning = false;
            } else if (windowEvent.type == SDL_KEYDOWN) {
                keyState = SDL_GetKeyboardState(NULL);
                //bear.move(keyState);
                move(keyState);
                
            }
        }
        /*
        //receive(sockfd, received, buf);
        //bear.receive();
        SDL_RenderClear(renderTarget);
        //SDL_SetRenderTarget(renderTarget, texture);
        SDL_RenderCopy(renderTarget, texture, NULL, NULL);
        //bear.renderAll();
        renderAll(received, myFuture);
        SDL_RenderPresent(renderTarget);*/
    }
    /*
    memset(buf, 0, 256);
    std::cout << "PLEASE ENTER THE MESSAGE: ";
    std::cin >> buf;

    n = write(sockfd, buf, strlen(buf));

    if (n < 0) {
        error("ERROR WRITING TO SOCKET");
    }

    memset(buf, 0, 256);

    n = read(sockfd, buf, 255);
    
    if (n < 0) {
        error("ERROR READING FROM SOCKET");
    }

    std::cout << buf;
    */
    close(sockfd);
    receivingThread.join();
    SDL_DestroyRenderer(renderTarget);
    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(yourTexture);
    SDL_DestroyTexture(otherTexture);
    renderTarget = nullptr;
    window = nullptr;
    texture = nullptr;
    yourTexture = nullptr;
    otherTexture = nullptr;
    SDL_Quit();
    return EXIT_SUCCESS;
}
