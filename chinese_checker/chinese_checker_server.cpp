#include <cstdlib>   // atoi
#include <exception> // exception
#include <iostream>  // cout, cerr
#include <queue>     // queue

#include <boost/asio.hpp>

#include "common.hpp"
#include "control_source.hpp"

constexpr char KEY_MAP[] = "dbcazfgknjhluiopqrstmvxwye";

using boost::asio::ip::tcp;

class room: public blocking_queue<true> {
    public:
        room() {
            for (int i = 0; i < NUM_PLAYER; i++) {
                has_player[i] = false;
            }
        }

        bool player_login(int i) {
            if (i < 0 || i >= NUM_PLAYER) {
                return false;
            }
            lock();
            if (has_player[i]) {
                unlock();
                return false;
            } else {
                has_player[i] = true;
                unlock();
                return true;
            }
        }

        bool player_logout(int i) {
            if (i < 0 || i >= NUM_PLAYER) {
                return false;
            }
            lock();
            if (!has_player[i]) {
                unlock();
                return false;
            } else {
                has_player[i] = false;
                unlock();
                return true;
            }
        }
    private:
        bool has_player[NUM_PLAYER];
} q;

class session: public std::enable_shared_from_this<session> {
    public:
        session(tcp::socket socket): socket_(std::move(socket)), logged_in(false) {}

        void start() {
            do_read();
        }
    private:
        void do_read() {
            auto self(shared_from_this());
            socket_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                [this, self](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        if (length == 2 && this->data_[0] == commands::PUT) {
                            if (logged_in) {
                                q.put(this->data_[1]);
                            }
                            do_read();
                        } else if (length == 1 && this->data_[0] == commands::GET) {
                            if (logged_in) {
                                if (!q.has_next()) {
                                    data_[0] = CHAR_CONT;
                                } else {
                                    char c = q.get();
                                    if (c >= 'a' && c <= 'z') {
                                        c = KEY_MAP[c - 'a'];
                                    }
                                    data_[0] = c;
                                }
                                do_write(1);
                            } else {
                                do_read();
                            }
                        } else if (length > 1 && this->data_[0] == commands::IN) {
                            if (!this->logged_in) {
                                player = (int)this->data_[1];
                                if (q.player_login(player)) {
                                    this->logged_in = true;
                                    data_[0] = success_fail::SUCCESS;
                                } else {
                                    data_[0] = success_fail::FAIL;
                                }
                            } else {
                                data_[0] = success_fail::FAIL;
                            }
                            do_write(1);
                        }
                    } else {
                        if (this->logged_in) {
                            this->logged_in = false;
                            q.player_logout(player);
                        }
                    }
                });
        }

        void do_write(std::size_t length) {
            auto self(shared_from_this());
            boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                    if (!ec) {
                        do_read();
                    } else {
                        if (this->logged_in) {
                            this->logged_in = false;
                            q.player_logout(player);
                        }
                    }
                });
        }

        tcp::socket socket_;
        bool logged_in;
        int player;
        static constexpr int MAX_LENGTH = 2;
        char data_[MAX_LENGTH];
};

class server {
    public:
        server(boost::asio::io_context& io_context, short port): acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
            do_accept();
        }
    private:
        void do_accept() {
            acceptor_.async_accept(
                [this](boost::system::error_code ec, tcp::socket socket) {
                    if (!ec) {
                        std::make_shared<session>(std::move(socket))->start();
                    }

                    do_accept();
                });
        }

        tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <port>\n";
            return 1;
        }

        std::cout << "Server started\n";

        boost::asio::io_context io_context;

        server s(io_context, std::atoi(argv[1]));

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    std::cout << "Server stopped\n";

    return 0;
}
