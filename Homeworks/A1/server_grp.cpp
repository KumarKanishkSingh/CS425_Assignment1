// // Client-side implementation in C++ for a chat server with private messages and group messaging

// #include <iostream>
// #include <string>
// #include <thread>
// #include <mutex>
// #include <unordered_map>
// #include <unordered_set>
// #include <vector>
// #include <sstream>
// #include <cstring>
// #include <cstdlib>
// #include <unistd.h>
// #include <arpa/inet.h>

// #define BUFFER_SIZE 1024

// std::mutex cout_mutex;

// void handle_server_messages(int client_socket) {
//     char buffer[BUFFER_SIZE];
//     while (true) {
//         memset(buffer, 0, BUFFER_SIZE);
//         int bytes_received = send(client_socket, buffer, BUFFER_SIZE, 0);
//         if (bytes_received <= 0) {
//             std::lock_guard<std::mutex> lock(cout_mutex);
//             std::cout << "Disconnected from server." << std::endl;
//             close(client_socket);
//             exit(0);
//         }
//         std::lock_guard<std::mutex> lock(cout_mutex);
//         std::cout << buffer << std::endl;
//     }
// }

// int main() {
//     int server_socket;
//     sockaddr_in server_address{};

//     server_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_socket < 0) {
//         std::cerr << "Error creating socket." << std::endl;
//         return 1;
//     }

//     server_address.sin_family = AF_INET;
//     server_address.sin_port = htons(12345);
//     server_address.sin_addr.s_addr = INADDR_ANY;

//     // if (connect(server_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
//     //     std::cerr << "Error connecting to server." << std::endl;
//     //     return 1;
//     // }

//     bind(server_socket, (struct sockaddr*)&server_address,
//          sizeof(server_address));

//     // listening to the assigned socket
//     listen(server_socket, 5);    

//     int client_socket
//         = accept(server_socket, nullptr, nullptr);

//     std::cout << "Connected to the server." << std::endl;

//     // Authentication
//     // std::string username, password;
//     // char username[BUFFER_SIZE], password[BUFFER_SIZE];
//     char buffer[BUFFER_SIZE];
//     std::strcpy(buffer,"Enter the username");

//     // memset(buffer, 0, BUFFER_SIZE);
//     send(client_socket, buffer, BUFFER_SIZE, 0); // Receive the message "Enter the user name" for the server
//     // You should have a line like this in the server.cpp code: send_message(client_socket, "Enter username: ");
 
//     // std::cout << buffer;\
//     // std::getline(std::cin, username);
//     // char buffer[BUFFER_SIZE];

//     recv(client_socket, buffer, BUFFER_SIZE, 0);
//     std::cout<<buffer<<std::endl;
//     // char username[]=buffer;
//     std::string username(buffer);
//     // std::strcpy(username, buffer);
//     // memset(buffer, 0, BUFFER_SIZE);
//     // char buffer[]="Enter the password";
//     std::strcpy(buffer,"Enter the password");
//     send(client_socket, buffer, BUFFER_SIZE, 0); // Receive the message "Enter the password" for the server
//     // std::cout << buffer;
//     // std::getline(std::cin, password);
//     memset(buffer, 0, BUFFER_SIZE);
//     recv(client_socket, buffer, BUFFER_SIZE, 0);
//     std::cout<<buffer<<std::endl;
//     std::string password(buffer);
//     // std::strcpy(password, buffer);
//     memset(buffer, 0, BUFFER_SIZE);
//     int check=1;
//     // password check code here
//     if(check)
//     {
//         std::strcpy(buffer,"Welcome to the server");
//         send(client_socket, buffer, BUFFER_SIZE, 0);
//     }
//     else
//     {
//         std::strcpy(buffer,"Authentication Failed");
//         send(client_socket, buffer, BUFFER_SIZE, 0);
//     }
//     // Depending on whether the authentication passes or not, receive the message "Authentication Failed" or "Welcome to the server"
//     // recv(server_socket, buffer, BUFFER_SIZE, 0); 
//     // std::cout << buffer << std::endl;

//     // if (std::string(buffer).find("Authentication failed") != std::string::npos) {
//     //     close(server_socket);
//     //     return 1;
//     // }

//     // Start thread for receiving messages from server
//     std::thread receive_thread(handle_server_messages, client_socket);
//     // We use detach because we want this thread to run in the background while the main thread continues running
//     receive_thread.detach();

//     // Send messages to the server
//     while (true) {
//         std::string message;
//         std::getline(std::cin, message);

//         if (message.empty()) continue;

//         recv(client_socket, message.c_str(), message.size(), 0);

//         if (message == "/exit") {
//             close(server_socket);
//             break;
//         }
//     }

//     return 0;
// }




// Start here

// Server-side implementation in C++ for a chat server with private messages and group messaging

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <algorithm> // Added for std::remove_if

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 2
#define PORT 12345

std::mutex cout_mutex;
std::mutex clients_mutex;

// Structure to hold client information
struct ClientInfo {
    int socket;
    std::string username;
};

// List of connected clients
std::vector<ClientInfo> clients;

// Function to broadcast a message to all clients except the sender
void broadcast_message(const std::string& message, int sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& client : clients) {
        if (client.socket != sender_socket) {
            if (send(client.socket, message.c_str(), message.size(), 0) < 0) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cerr << "Error sending message to client " << client.username << std::endl;
            }
        }
    }
}

// Function to handle communication with a single client
void handle_client(int client_socket){
    // , sockaddr_in client_address) {
    char buffer[BUFFER_SIZE];
    std::string username;
    std::string password;

    // Authentication Process
    // 1. Ask for username
    std::string prompt_username = "Enter the username: ";
    if (send(client_socket, prompt_username.c_str(), prompt_username.size(), 0) < 0) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "Failed to send username prompt to client." << std::endl;
        close(client_socket);
        return;
    }

    // Receive username
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "Failed to receive username from client." << std::endl;
        close(client_socket);
        return;
    }
    username = std::string(buffer);
    username.erase(username.find_last_not_of("\n\r") + 1); // Trim newline characters

    // 2. Ask for password
    std::string prompt_password = "Enter the password: ";
    if (send(client_socket, prompt_password.c_str(), prompt_password.size(), 0) < 0) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "Failed to send password prompt to client." << std::endl;
        close(client_socket);
        return;
    }

    // Receive password
    memset(buffer, 0, BUFFER_SIZE);
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "Failed to receive password from client." << std::endl;
        close(client_socket);
        return;
    }
    password = std::string(buffer);
    password.erase(password.find_last_not_of("\n\r") + 1); // Trim newline characters

    // Simple authentication check (accept any username/password)
    bool is_authenticated = true; // Replace with actual authentication logic

    if (is_authenticated) {
        std::string welcome_msg = "Welcome to the server, " + username + "!\n";
        if (send(client_socket, welcome_msg.c_str(), welcome_msg.size(), 0) < 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Failed to send welcome message to client." << std::endl;
            close(client_socket);
            return;
        }

        // Add client to the list of connected clients
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(ClientInfo{client_socket, username});
        }

        // Notify other clients about the new connection
        std::string join_msg = username + " has joined the chat.\n";
        broadcast_message(join_msg, client_socket);

        // Start receiving messages from the client
        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            int recv_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (recv_bytes <= 0) {
                // Client disconnected
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << username << " disconnected." << std::endl;
                close(client_socket);

                // Remove client from the list
                {
                    std::lock_guard<std::mutex> lock(clients_mutex);
                    clients.erase(std::remove_if(clients.begin(), clients.end(),
                        [&](const ClientInfo& c) { return c.socket == client_socket; }),
                        clients.end());
                }

                // Notify other clients about the disconnection
                std::string leave_msg = username + " has left the chat.\n";
                broadcast_message(leave_msg, client_socket);
                break;
            }

            std::string message = std::string(buffer);
            message.erase(message.find_last_not_of("\n\r") + 1); // Trim newline characters

            if (message == "/exit") {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << username << " has exited the chat." << std::endl;
                close(client_socket);

                // Remove client from the list
                {
                    std::lock_guard<std::mutex> lock(clients_mutex);
                    clients.erase(std::remove_if(clients.begin(), clients.end(),
                        [&](const ClientInfo& c) { return c.socket == client_socket; }),
                        clients.end());
                }

                // Notify other clients about the disconnection
                std::string exit_msg = username + " has left the chat.\n";
                broadcast_message(exit_msg, client_socket);
                break;
            }

            // Prepend username to the message
            std::string formatted_message = username + ": " + message + "\n";
            broadcast_message(formatted_message, client_socket);
        }
    } else {
        std::string fail_msg = "Authentication Failed\n";
        send(client_socket, fail_msg.c_str(), fail_msg.size(), 0);
        close(client_socket);
    }
}

int main() {
    int server_socket;
    sockaddr_in server_address{};

    // Create a TCP socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    // Allow address reuse
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt(SO_REUSEADDR) failed." << std::endl;
        close(server_socket);
        return 1;
    }

    // Define server address
    // server_address.sin_family = AF_INET;
    // server_address.sin_port = htons(12345);
    // server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345);
    server_address.sin_addr.s_addr = INADDR_ANY;


    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
    std::cerr << "Error binding socket." << std::endl;
    std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
    close(server_socket);
    return 1; 
 }

    //  bind(server_socket, (struct sockaddr*)&server_address,
    //      sizeof(server_address));

    // Start listening for incoming connections
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        std::cerr << "Error listening on socket." << std::endl;
        close(server_socket);
        return 1;
    }

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Server is listening on port " << PORT << "..." << std::endl;
    }

    // Vector to hold client threads
    std::vector<std::thread> client_threads;

    while (true) {
        sockaddr_in client_address{};
        socklen_t client_len = sizeof(client_address);

        // Accept a new client connection
        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len);
        if (client_socket < 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Error accepting client connection." << std::endl;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "New connection accepted." << std::endl;
        }

        // Create a thread to handle the new client
        client_threads.emplace_back(std::thread(handle_client, client_socket));
        // , client_address));

        // Optional: Limit the number of clients (e.g., 2)
        if (client_threads.size() >= MAX_CLIENTS) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Maximum client limit reached. No longer accepting new connections." << std::endl;
            break;
        }
    }

    // Join all client threads before exiting
    for (auto& th : client_threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    // Close the server socket
    close(server_socket);
    return 0;
}

