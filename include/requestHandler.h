# ifndef REQUEST_HANDLER_H
# define REQUEST_HANDLER_H

#include <string>
#include <vector>

using namespace std;

void* accept_request(int client_socket);
void upload_file(int client_socket, const string& filename);

# endif // REQUEST_HANDLER_H