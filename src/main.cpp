#include <stdio.h>
#include "libfswatch/c++/monitor.hpp"
#include "libfswatch/c++/monitor_factory.hpp"
#include "simple-websocket-server/client_ws.hpp"
#include "simple-websocket-server/server_ws.hpp"

using namespace std;

using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;

static void process_events(const vector<fsw::event>& events, void* user)
{
  shared_ptr<WsClient::Connection> connection = *((shared_ptr<WsClient::Connection>*) user);

  for (fsw::event event : events) {
    cout << event.get_path() << " ";
    for (fsw_event_flag flag : event.get_flags()) {
      
      cout << fsw::event::get_event_flag_name(flag) << " ";
    }
    cout << endl;
  }
}

int main(int argc, char* argv[]) {
  if (argc > 2) {
    string watch_dir = argv[1];
    string websocket_host_port_path = argv[2];

    // WebSocket client code adapted from the following example:
    // https://gitlab.com/eidheim/Simple-WebSocket-Server/-/blob/v2.0.2/ws_examples.cpp?ref_type=tags#L109-146
    WsClient client(websocket_host_port_path);


    client.on_open = [&websocket_host_port_path, &watch_dir](shared_ptr<WsClient::Connection> connection) {
      cout << "Connected to ws://" << websocket_host_port_path << endl;
      vector<string> paths = {watch_dir};
      fsw::monitor *active_monitor =
        fsw::monitor_factory::create_monitor(fsw_monitor_type::system_default_monitor_type,
                                             paths,
                                             process_events,
                                             &connection);
      
      active_monitor->set_event_type_filters({{Created}, {Updated}, {Removed}, {Renamed}});
      active_monitor->set_recursive(true);
      cout << "Watching " + watch_dir + " for changes." << endl;
      active_monitor->start();
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

