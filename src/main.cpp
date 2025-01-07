#include <stdio.h>
#include "efsw/efsw.hpp"
#include "simple-websocket-server/client_ws.hpp"
#include "simple-websocket-server/server_ws.hpp"
#include "boost/json.hpp"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;

std::string map_action_to_string(efsw::Action action) {
  switch (action) {
    case efsw::Actions::Add:
      return "ADD";
    case efsw::Actions::Delete:
      return "DELETE";
    case efsw::Actions::Modified:
      return "MODIFIED";
    case efsw::Actions::Moved:
      return "MOVE";
    default:
      return "UNRECOGNIZED";
  }
}

class UpdateListener : public efsw::FileWatchListener {
  private:
    std::shared_ptr<WsClient::Connection> connection;

  public:
    UpdateListener(std::shared_ptr<WsClient::Connection> _connection) {
      connection = _connection;
    }

    void handleFileAction(efsw::WatchID watchid, const std::string& dir,
                          const std::string& filename, efsw::Action action,
                          std::string oldFilename) override {
      std::string oldFilepath = dir + filename;
      std::string action_label = map_action_to_string(action);
      boost::json::object obj({{"filepath", dir + filename}, {"action", action_label}});
      if (!oldFilename.empty()) {
        obj.emplace("oldFilepath", oldFilepath);
      }
      std::string json = boost::json::serialize(obj);
      connection->send(json);
      std::cout << json << std::endl;
    }
};

int main(int argc, char* argv[]) {
  if (argc > 2) {
    std::string watch_dir = argv[1];
    std::string websocket_host_port_path = argv[2];

    // WebSocket client code adapted from the following example:
    // https://gitlab.com/eidheim/Simple-WebSocket-Server/-/blob/v2.0.2/ws_examples.cpp?ref_type=tags#L109-146
    WsClient client(websocket_host_port_path);

    client.on_open = [&websocket_host_port_path, &watch_dir](std::shared_ptr<WsClient::Connection> connection) {
      std::cout << "Connected to ws://" << websocket_host_port_path << std::endl;
      efsw::FileWatcher* fileWatcher = new efsw::FileWatcher();
      UpdateListener* listener = new UpdateListener(connection);
      bool recursive = true;
      efsw::WatchID watchID = fileWatcher->addWatch(watch_dir, listener, recursive);
      if (watchID < 0) {
        std::cout << "Error " << watchID <<  " watching directory " << watch_dir << std::endl;
        std::cout << "See https://github.com/SpartanJ/efsw/blob/1.4.1/include/efsw/efsw.h#L76-L85 for error code." << std::endl;
        exit(1);
      }
      fileWatcher->watch();
      std::cout << "[Watch #" << watchID  << "] Watching " + watch_dir + " for changes." << std::endl;
    };

    client.on_close = [&websocket_host_port_path](std::shared_ptr<WsClient::Connection> /*connection*/, int status, const std::string & /*reason*/) {
      std::cout << "Closed connection to ws://" << websocket_host_port_path << " with status code " << status << std::endl;
    };

    // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
    client.on_error = [](std::shared_ptr<WsClient::Connection> /*connection*/, const SimpleWeb::error_code &ec) {
      std::cout << "Error: " << ec << ", error message: " << ec.message() << std::endl;
    };
    std::cout << "Connecting to ws://" << websocket_host_port_path << std::endl;
    client.start();
  } else {
    std::cout << "Usage: <watch_dir> <websocket_host_port_path>" << std::endl;
    std::cout << "    watch_dir - directory to recursively watch for changes" << std::endl;
    std::cout << "    websocket_host_port_path - host, port and path of WebSocket server to write changes to." << std::endl;
    std::cout << "    Example: ./src localhost:8080" << std::endl;
  }
  
}

