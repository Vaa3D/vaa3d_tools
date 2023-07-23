//
// Created by SEU on 2023/3/19.
//

#include "VEPServer.h"

VEPServer::VEPServer(int port, int num_threads) {

}

void VEPServer::start() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_);

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(sockfd, 5) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int newsockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
        if (newsockfd < 0) {
            throw std::runtime_error("Failed to accept connection");
        }

        thread_pool_.enqueue([this, newsockfd] {
            handle_client(newsockfd);
        });
    }
}
