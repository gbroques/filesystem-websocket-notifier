#include <stdio.h>
#include "simple-websocket-server/client_ws.hpp"
#include "simple-websocket-server/server_ws.hpp"
#define DMON_IMPL
#include "dmon.h"

using namespace std;

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;

static void watch_callback(dmon_watch_id watch_id, dmon_action action, const char* rootdir,
                           const char* filepath, const char* oldfilepath, void* user)
{
    shared_ptr<WsClient::Connection> connection = *((shared_ptr<WsClient::Connection>*) user);
    (void)(user);
    (void)(watch_id);
    string rootdirstr = rootdir;
    string filepathstr = rootdirstr + filepath;
    string oldfilepathstr;
    if (oldfilepath != NULL) {
      oldfilepathstr = rootdirstr + oldfilepath;
    }
    string msg = "{\"filepath\":\"" + filepathstr + "\",";

    switch (action) {
    case DMON_ACTION_CREATE:
      msg.append("\"action\":\"CREATE\"}");
      break;
    case DMON_ACTION_DELETE:
      msg.append("\"action\":\"DELETE\"}");
      break;
    case DMON_ACTION_MODIFY:
      msg.append("\"action\":\"MODIFY\"}");
      break;
    case DMON_ACTION_MOVE:
      msg.append("\"oldfilepath\":\"" + oldfilepathstr + "\",\"action\":\"MOVE\"}");
      break;
    }
    connection->send(msg);
    cout << msg << endl;
}

int main(int argc, char* argv[]) {
  if (argc > 2) {
    string watch_dir = argv[1];
    string websocket_host_port_path = argv[2];

    // WebSocket client code adapted from the following example:
    // https://gitlab.com/eidheim/Simple-WebSocket-Server/-/blob/v2.0.2/ws_examples.cpp?ref_type=tags#L109-146
    WsClient client(websocket_host_port_path);

    shared_ptr<WsClient::Connection> connection_pointer;

    client.on_open = [&connection_pointer, &websocket_host_port_path, &watch_dir](shared_ptr<WsClient::Connection> connection) {
      cout << "Connected to ws://" << websocket_host_port_path << endl;
      connection_pointer = connection;
      dmon_init();
      cout << "Watching " + watch_dir + " for changes." << endl;
    };

    client.on_close = [&websocket_host_port_path](shared_ptr<WsClient::Connection> /*connection*/, int status, const string & /*reason*/) {
      cout << "Closed connection to ws://" << websocket_host_port_path << " with status code " << status << endl;
    };

    // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
    client.on_error = [](shared_ptr<WsClient::Connection> /*connection*/, const SimpleWeb::error_code &ec) {
      cout << "Error: " << ec << ", error message: " << ec.message() << endl;
    };

    // dmon code adapted from the following example:
    // https://github.com/septag/dmon/blob/a56fdb90e787fa2acecb4a956ec7afd400d1715c/test.c
    dmon_watch(argv[1], watch_callback, DMON_WATCHFLAGS_RECURSIVE, &connection_pointer);
    cout << "Connecting to ws://" << websocket_host_port_path << endl;
    client.start();
    dmon_deinit();
  } else {
    cout << "Usage: <watch_dir> <websocket_host_port_path>" << endl;
    cout << "    watch_dir - directory to recursively watch for changes" << endl;
    cout << "    websocket_host_port_path - host, port and path of WebSocket server to write changes to." << endl;
    cout << "    Example: ./src localhost:8080" << endl;
  }
  
}

