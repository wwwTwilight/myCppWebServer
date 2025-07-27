# ifndef REQUEST_HANDLER_H
# define REQUEST_HANDLER_H

#include <string>
#include <vector>

using namespace std;
string get_mine_type(const string& filename);
void exec_cgi(int client_socket, const string& method, const string& _path, const string& query);
void open_http_file(int& client_socket, const string& filename);
void* accept_request(int client_socket);

# endif // REQUEST_HANDLER_H