#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <utility>
#include <arpa/inet.h>
#include <thread>
#include <mutex>
//#include "server.hpp"

typedef std::pair<int, int> location;

const int VELO = 5;
std::mutex map_mutex;

std::vector<std::thread> readingThreads;
bool alive = true;
bool updated = false;
void error(const char* msg) {
    perror(msg);
    alive = false;
}

void waiting() {
    std::this_thread::sleep_for(std::chrono::minutes(4));
    alive = false;
}

void broadcast(const std::unordered_map<int, location> &socks) {
    std::string str;
    std::string temp;
    double num;
    map_mutex.lock();
    for (auto&[sock, local] : socks) {
        num = local.first * 10000 + local.second;
        temp = std::to_string((int)num);
        while(temp.size() < 8) {
            temp = "0" + temp;
        }
        str += temp;
    }
    for (auto& i : socks) {
        int numb;
        if ((numb = send(i.first, str.c_str(), str.size(), 0)) < 0) {
            error("ERROR SENDING");
        } else {
            std::cout << "sent " << numb << " of "<< str << " to " << i.first << "\n";
        }
    }
    map_mutex.unlock();
}

void reading(std::unordered_map<int, std::pair<int,int>>& socks,const int& sock) {
    char buf[256];
    if (read(sock, buf, 255) < 0) {
        error("ERROR READING");
    }
    switch(buf[0]) {
        case 'w':
            socks[sock].second -= VELO;
            break;
        case 'a':
            socks[sock].first -= VELO;
            break;
        case 's':
            socks[sock].second += VELO;
            break;
        case 'd':
            socks[sock].first += VELO;
            break;
        /*
        default:
            std::cout << buf[0] << std::endl;
            break;*/
    } 
    updated = true; 
}

void accepting(std::unordered_map<int, location>& socks, int& serverSock) {
    struct sockaddr_in cli_addr;
    socklen_t clilen;
    clilen = sizeof(cli_addr);
    int clisock = accept(serverSock, (sockaddr *)&cli_addr, &clilen);
    if (clisock < 0) {
        error("ERROR ON ACCEPT");
    } else {
        std::cout << "Accepted client\n";
    }
    socks[clisock] = std::make_pair(0,0);
    std::cout << "client is at (" << socks[clisock].first << ", " << socks[clisock].second << ")\n";
    readingThreads.push_back(std::thread([&](){while(alive){reading(socks, clisock); std::this_thread::sleep_for(std::chrono::milliseconds(5));}}));
    updated = true;
}

int main(int argc, char *argv[]) {
    int sockfd, port;
    //std::vector<int> newsockfd;
    std::unordered_map<int, std::pair<int, int>> newsockfd;
    socklen_t clilen;
    char buf[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    int max_so_far = 0;
    fd_set current_sockets, temp;

    if (argc < 2) {
        fprintf(stderr, "ERROR, NO PORT PROVIDED\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error("ERROR OPENING SOCKET");
    }

    memset(&serv_addr,0, sizeof(serv_addr));
    
    port = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    FD_ZERO(&current_sockets);
    FD_SET(sockfd, &current_sockets);
    max_so_far = sockfd;

    if (bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {error("ERROR ON BINDING");}

    listen(sockfd, 5);
    std::thread acceptingThread([&]() {while(alive){accepting(newsockfd, sockfd);}});
    //std::thread readingThread(newsockfd, 3);
    std::thread waitingThread([](){while(alive){waiting();}});
    while(alive) {
        //temp = current_sockets;

        /*if (select(FD_SETSIZE, &temp, NULL,NULL,NULL) < 0) {
            error("SELECT ERROR");
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &temp)) {
                if (i == sockfd) {
                    FD_SET(accepting(newsockfd, sockfd), &current_sockets);
                } else {
                    reading(newsockfd, i);
                }
            }
        }*/
        updated = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (updated) {
            broadcast(newsockfd);
        }
    }
    /*
    clilen = sizeof(cli_addr);

    newsockfd.emplace_back(accept(sockfd, (sockaddr *) &cli_addr, &clilen));

    if (newsockfd.back() < 0) {
        error("ERROR ON ACCEPT");
    }

    std::cout << "server: got connection from " << inet_ntoa(cli_addr.sin_addr) << " port " << ntohs(cli_addr.sin_port) << "\n";

    send(newsockfd, defaultMessage.c_str(), defaultMessage.length(), 0);

    memset(buf, 0,256);

    n = read(newsockfd, buf, 255);
    if (n < 0) {
        error("ERROR READING FROM SOCKET");
    } else {
        std::cout << "Here is the message from client: " << buf << "\n";
    }
    */
    waitingThread.join();
    acceptingThread.join();
    for (auto& t : readingThreads) {
        t.join();
    }
    for (auto& i : newsockfd) close(i.first);
    close(sockfd);
    return 0;
}

