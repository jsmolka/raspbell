#define _WIN32_WINNT 0x0601

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <SDL2/SDL.h>

namespace beast = boost::beast;

using tcp = boost::asio::ip::tcp;

void fail(boost::system::error_code ec, const char* what)
{
    std::cerr << what << ": " << ec.message() << std::endl;
}

class Window
{
public:
    bool isActive() const
    {
        return window && renderer;
    }

    void show()
    {
        SDL_DisplayMode display;
        SDL_GetCurrentDisplayMode(0, &display);

        window = SDL_CreateWindow(
            "Raspbell Client",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            #ifdef NDEBUG
            display.w, display.h,
            SDL_WINDOW_FULLSCREEN
            | SDL_WINDOW_ALWAYS_ON_TOP
            | SDL_WINDOW_INPUT_GRABBED
            | SDL_WINDOW_MOUSE_CAPTURE
            #else
            100, 100,
            0
            #endif
        );
        
        renderer = SDL_CreateRenderer(window, -1, 0);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    void processEvents()
    {
        bool close = false;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                close = true;
                break;

            case SDL_KEYDOWN:
                close |= event.key.keysym.scancode == SDL_SCANCODE_ESCAPE;
                break;
            }
        }

        if (close)
        {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);

            window = nullptr;
            renderer = nullptr;
        }
    }

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
};

class Session : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(boost::asio::io_context& ioc)
        : resolver(ioc)
        , ws(ioc)
        , timer(ws.get_executor())
    {
        beast::websocket::stream_base::timeout option;
        option.handshake_timeout = std::chrono::seconds(30);
        option.idle_timeout = beast::websocket::stream_base::none();
        option.keep_alive_pings = false;

        ws.set_option(option);
    }

    void run(const char* host, const char* port)
    {
        this->host = host;
        this->port = port;

        resolver.async_resolve(host, port, std::bind(
            &Session::onResolve,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2
        ));

        timer.expires_after(std::chrono::milliseconds(100));
        timer.async_wait(std::bind(
            &Session::onTimer,
            shared_from_this(),
            std::placeholders::_1
        ));
    }

private:
    void onTimer(boost::system::error_code ec)
    {
        if (ec) return fail(ec, "Timer");

        if (window.isActive())
            window.processEvents();

        timer.expires_after(std::chrono::milliseconds(100));
        timer.async_wait(std::bind(
            &Session::onTimer,
            shared_from_this(),
            std::placeholders::_1
        ));
    }

    void onResolve(boost::system::error_code ec, tcp::resolver::results_type results)
    {
        if (ec) return fail(ec, "Resolve");

        boost::asio::async_connect(ws.next_layer(), results.begin(), results.end(), std::bind(
            &Session::onConnect,
            shared_from_this(),
            std::placeholders::_1
        ));
    }

    void onConnect(boost::system::error_code ec)
    {
        if (ec) return fail(ec, "Connect");

        ws.async_handshake(host, "/", std::bind(
            &Session::onHandshake,
            shared_from_this(),
            std::placeholders::_1
        ));
    }

    void onHandshake(boost::system::error_code ec)
    {
        if (ec) return fail(ec, "Handshake");

        ws.async_read(buffer, std::bind(
            &Session::onRead,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2
        ));
    }

    void onRead(boost::system::error_code ec, std::size_t transferred)
    {
        boost::ignore_unused(transferred);

        if (ec) return fail(ec, "Read");

        if (!window.isActive())
            window.show();

        buffer.clear();

        ws.async_read(buffer, std::bind(
            &Session::onRead,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2
        ));
    }

    Window window;
    tcp::resolver resolver;
    beast::multi_buffer buffer;
    beast::websocket::stream<tcp::socket> ws;
    boost::asio::steady_timer timer;
    std::string host;
    std::string port;
};

int main(int argc, char* argv[])
{
    try
    {
        SDL_InitSubSystem(SDL_INIT_VIDEO);

        if (argc != 3) throw std::runtime_error("Usage: client host port");

        const char* host = argv[1];
        const char* port = argv[2];

        boost::asio::io_context ioc;
        std::make_shared<Session>(ioc)->run(host, port);
        ioc.run();
    }
    catch (const std::exception& ex)
    {
        SDL_ShowSimpleMessageBox(0, "Raspbell Client Error", ex.what(), nullptr);
    }

    SDL_Quit();

    return 0;
}
