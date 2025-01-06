#include <stdio.h>
#include "efsw/efsw.hpp"
#include "simple-websocket-server/client_ws.hpp"
#include "simple-websocket-server/server_ws.hpp"

using namespace std;

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;

class UpdateListener : public efsw::FileWatchListener {
  private:
    shared_ptr<WsClient::Connection> connection;

  public:
    UpdateListener(shared_ptr<WsClient::Connection> _connection) {
      connection = _connection;
    }

    void handleFileAction(efsw::WatchID watchid, const std::string& dir,
                          const std::string& filename, efsw::Action action,
                          std::string oldFilename) override {
      string oldFilepath = dir + filename;
      string msg = "{\"filepath\":\"" + dir + filename + "\",";
      switch (action) {
        case efsw::Actions::Add:
          msg.append("\"action\":\"ADD\"}");
          break;
        case efsw::Actions::Delete:
          msg.append("\"action\":\"DELETE\"}");
          break;
        case efsw::Actions::Modified:
          msg.append("\"action\":\"MODIFIED\"}");
          break;
        case efsw::Actions::Moved:
          msg.append("\"oldFilepath\":\"" + oldFilepath + "\",\"action\":\"MOVE\"}");
          break;
        default:
          std::cout << "Unrecognized action" << std::endl;
      }
      connection->send(msg);
      cout << msg << endl;
    }
};

int main(int argc, char* argv[]) {
  if (argc > 2) {
    string watch_dir = argv[1];
    string websocket_host_port_path = argv[2];

    // WebSocket client code adapted from the following example:
    // https://gitlab.com/eidheim/Simple-WebSocket-Server/-/blob/v2.0.2/ws_examples.cpp?ref_type=tags#L109-146
    WsClient client(websocket_host_port_path);

    client.on_open = [&websocket_host_port_path, &watch_dir](shared_ptr<WsClient::Connection> connection) {
      cout << "Connected to ws://" << websocket_host_port_path << endl;
      efsw::FileWatcher* fileWatcher = new efsw::FileWatcher();
      UpdateListener* listener = new UpdateListener(connection);
      bool recursive = true;
      efsw::WatchID watchID = fileWatcher->addWatch(watch_dir, listener, recursive);
      if (watchID < 0) {
        cout << "Error " << watchID <<  " watching directory " << watch_dir << endl;
        cout << "See https://github.com/SpartanJ/efsw/blob/1.4.1/include/efsw/efsw.h#L76-L85 for error code." << endl;
        exit(1);
      }
      fileWatcher->watch();
      cout << "[Watch #" << watchID  << "] Watching " + watch_dir + " for changes." << endl;
    };

    client.on_close = [&websocket_host_port_path](shared_ptr<WsClient::Connection> /*connection*/, int status, const string & /*reason*/) {
      cout << "Closed connection to ws://" << websocket_host_port_path << " with status code " << status << endl;
    };

    // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
    client.on_error = [](shared_ptr<WsClient::Connection> /*connection*/, const SimpleWeb::error_code &ec) {
      cout << "Error: " << ec << ", error message: " << ec.message() << endl;
    };
    cout << "Connecting to ws://" << websocket_host_port_path << endl;
    client.start();
  } else {
    cout << "Usage: <watch_dir> <websocket_host_port_path>" << endl;
    cout << "    watch_dir - directory to recursively watch for changes" << endl;
    cout << "    websocket_host_port_path - host, port and path of WebSocket server to write changes to." << endl;
    cout << "    Example: ./src localhost:8080" << endl;
  }
  
}

