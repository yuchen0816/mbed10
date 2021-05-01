#include "mbed.h"

#include "wifi_helper.h"

#include "mbed-trace/mbed_trace.h"


class SocketDemo {

    static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 10;

    static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;

    static constexpr char*  HOST_IP="192.168.160.19";

    static constexpr size_t REMOTE_PORT = 80; // standard HTTP port


public:

    SocketDemo() : _net(NetworkInterface::get_default_instance()) {}


    ~SocketDemo()

    {

        if (_net)

            _net->disconnect();

    }


    void run()

    {

        if (!_net) {

            printf("Error! No network interface found.\r\n");

            return;

        }


        printf("Connecting to the network...\r\n");


        nsapi_size_or_error_t result = _net->connect();

        if (result != 0) {

            printf("Error! _net->connect() returned: %d\r\n", result);

            return;

        }


        print_network_info();


        result = _socket.open(_net);

        if (result != 0) {

            printf("Error! _socket.open() returned: %d\r\n", result);

            return;

        }


        SocketAddress address(HOST_IP, REMOTE_PORT); //Set IP and port directly


        //if (!resolve_hostname(address)) return;

        //address.set_port(REMOTE_PORT);


        printf("Opening connection to remote port %d\r\n", REMOTE_PORT);


        result = _socket.connect(address);

        if (result != 0) {

            printf("Error! _socket.connect() returned: %d\r\n", result);

            return;

        }


        if (!send_http_request() || !receive_http_response())

            return;


        printf("Demo concluded successfully \r\n");

    }


private:

    bool resolve_hostname(SocketAddress &address)

    {

        const char hostname[] = MBED_CONF_APP_HOSTNAME;


        printf("\nResolve hostname %s\r\n", hostname);

        nsapi_size_or_error_t result = _net->gethostbyname(hostname, &address);

        if (result != 0) {

            printf("Error! gethostbyname(%s) returned: %d\r\n", hostname, result);

            return false;

        }


        printf("%s address is %s\r\n", hostname, (address.get_ip_address() ? address.get_ip_address() : "None") );


        return true;

    }


    bool send_http_request()

    {

        /* loop until whole request sent */

        const char buffer[] = "GET / HTTP/1.1\r\n" "Host: 192.168.160.19\r\n" "Connection: close\r\n" "\r\n";


        nsapi_size_t bytes_to_send = strlen(buffer);

        nsapi_size_or_error_t bytes_sent = 0;


        printf("\r\nSending message: \r\n%s", buffer);


        while (bytes_to_send) {

            bytes_sent = _socket.send(buffer + bytes_sent, bytes_to_send);

            if (bytes_sent < 0) {

                printf("Error! _socket.send() returned: %d\r\n", bytes_sent);

                return false;

            } else

                printf("sent %d bytes\r\n", bytes_sent);


            bytes_to_send -= bytes_sent;

        }


        printf("Complete message sent\r\n");


        return true;

    }


    bool receive_http_response()

    {

        char buffer[MAX_MESSAGE_RECEIVED_LENGTH];

        int remaining_bytes = MAX_MESSAGE_RECEIVED_LENGTH;

        int received_bytes = 0;


        /* loop until there is nothing received or we've ran out of buffer space */

        nsapi_size_or_error_t result = remaining_bytes;

        while (result > 0 && remaining_bytes > 0) {

            nsapi_size_or_error_t result = _socket.recv(buffer + received_bytes, remaining_bytes);

            if (result < 0) {

                printf("Error! _socket.recv() returned: %d\r\n", result);

                return false;

            }


            received_bytes += result;

            remaining_bytes -= result;

        }


        printf("received %d bytes:\r\n%.*s\r\n\r\n", received_bytes, strstr(buffer, "\n") - buffer, buffer);


        return true;

    }


    void print_network_info()

    {

        SocketAddress a;

        _net->get_ip_address(&a);

        printf("IP address: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");

        _net->get_netmask(&a);

        printf("Netmask: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");

        _net->get_gateway(&a);

        printf("Gateway: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");

    }


private:

    NetworkInterface *_net;

    TCPSocket _socket;

};


int main() {

    printf("\r\nStarting socket demo\r\n\r\n");


#ifdef MBED_CONF_MBED_TRACE_ENABLE

    mbed_trace_init();

#endif


    SocketDemo *example = new SocketDemo();

    MBED_ASSERT(example);

    example->run();


    return 0;

}
