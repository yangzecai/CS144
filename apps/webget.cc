#include "socket.hh"
#include "util.hh"

#include <cstdlib>
#include <iostream>

using namespace std;

void get_URL(const string &host, const string &path) {
    TCPSocket client_socket;
    Address server_addr(host, std::string("http"));
    client_socket.connect(server_addr);
    std::string send_content;
    send_content.append("GET ");
    send_content.append(path);
    send_content.append(" HTTP/1.1\r\n");
    send_content.append("Host: ");
    send_content.append(host);
    send_content.append("\r\n");
    send_content.append("Connection: close\r\n\r\n");
    client_socket.write(send_content);

    while (!client_socket.eof()) {
      std::cout << client_socket.read();
    }
}

int main(int argc, char *argv[]) {
    try {
        if (argc <= 0) {
            abort();  // For sticklers: don't try to access argv[0] if argc <= 0.
        }

        // The program takes two command-line arguments: the hostname and "path" part of the URL.
        // Print the usage message unless there are these two arguments (plus the program name
        // itself, so arg count = 3 in total).
        if (argc != 3) {
            cerr << "Usage: " << argv[0] << " HOST PATH\n";
            cerr << "\tExample: " << argv[0] << " stanford.edu /class/cs144\n";
            return EXIT_FAILURE;
        }

        // Get the command-line arguments.
        const string host = argv[1];
        const string path = argv[2];

        // Call the student-written function.
        get_URL(host, path);
    } catch (const exception &e) {
        cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
