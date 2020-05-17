#define _WIN32_WINNT 0x0601

#include <chrono>
#include <stdexcept>
#include <thread>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <SDL2/SDL.h>

namespace beast = boost::beast;

using tcp = boost::asio::ip::tcp;

constexpr auto kWindowW = 500;
constexpr auto kWindowH = 500;

static void alert(const std::string& message)
{
    auto window = SDL_CreateWindow(
        message.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        kWindowW,
        kWindowH,
        SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_MOUSE_CAPTURE
    );

    auto renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
        }
        SDL_RaiseWindow(window);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

int main(int argc, char* argv[])
{
    try
    {
        SDL_InitSubSystem(SDL_INIT_VIDEO);

        const char* host = "192.168.0.141";
        const char* port = "8844";

        boost::asio::io_context ioc;
        
        tcp::resolver resolver(ioc);
        beast::websocket::stream<tcp::socket> ws(ioc);

        const auto endpoints = resolver.resolve(host, port);
        boost::asio::connect(ws.next_layer(), endpoints.begin(), endpoints.end());

        ws.handshake(host, "/");

        beast::flat_buffer buffer;
        while (ws.is_open())
        {
            ws.read(buffer);

            if (buffer.size() > 0)
            {
                alert(beast::buffers_to_string(buffer.data()));
                buffer.clear();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        ws.close(beast::websocket::close_code::normal);
    }
    catch (const std::exception& ex)
    {
        SDL_ShowSimpleMessageBox(0, "Error", ex.what(), nullptr);
    }
    
    SDL_Quit();

    return 0;
}