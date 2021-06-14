#include <iostream>
#include <sstream>
#include <regex>
#include <cmath>
#include <thread>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using std::literals::chrono_literals::operator""s;

std::string exec(const std::string s)
{
    auto file = popen(s.c_str(), "r");
    if (file == nullptr)
    {
        std::cout << "popen failed" << std::endl;
        return "";
    }
    std::stringstream ss;

    char buf[1024];
    while (fgets(buf, sizeof(buf), file) != nullptr)
    {
        ss << buf;
    }
    pclose(file);
    return ss.str();
}

bool is_number(const std::string &s)
{
    char *end = nullptr;
    double val = strtod(s.c_str(), &end);
    return end != s.c_str() && *end == '\0' && val != HUGE_VAL;
}

int get_current_temp()
{
    auto temp_str = exec("vcgencmd measure_temp 2> /dev/null");
    std::regex pattern("temp=(.+)'C");
    std::match_results<std::string::iterator> match_result;
    std::regex_search(temp_str.begin(), temp_str.end(), match_result, pattern);
    if (match_result.size() < 2)
    {
        return -1;
    }
    auto temp_num_str = match_result[1].str();
    if (is_number(temp_num_str))
    {
        return std::round(std::stod(temp_num_str));
    }
    return -1;
}

ftxui::Color get_text_color(int temp)
{
    if (temp < 25)
    {
        return ftxui::Color(66, (245 - 66) / 25 * temp + 66, 245);
    }
    else if (temp < 50)
    {
        return ftxui::Color(66, 245, 66 + (255 - 66) / 25 * (49 - temp));
    }
    else if (temp < 75)
    {
        return ftxui::Color((245 - 66) / 25 * (temp - 49) + 66, 245, 66);
    }
    else
    {
        return ftxui::Color(245, 66 + (245 - 66) / 25 * (99 - temp), 66);
    }
}

ftxui::Color get_current_text_color()
{
    auto temp = get_current_temp();
    if (temp == -1)
    {
        ftxui::Color(ftxui::Color::Palette1::Default);
    }
    return get_text_color(temp);
}

int main()
{
    auto screen = ftxui::ScreenInteractive::TerminalOutput();

    auto text = ftxui::text(L"CPU温度:" + std::to_wstring(get_current_temp()) + L"°C");
    auto render = ftxui::Renderer([&]
                                  { return ftxui::vbox({
                                      text | ftxui::color(get_current_text_color()) | ftxui::center, 
                                      ftxui::separator()}); });

    auto bar = ftxui::Renderer([&]{
        ftxui::Elements elements;
        auto temp = get_current_temp();
        auto width = screen.dimx();
        int should_width = std::round(temp/100.0*width);
        for(int i=0;i<should_width;i++){
            elements.push_back(ftxui::text(L" ")|ftxui::bgcolor(get_text_color(i)));
        }
        return ftxui::hbox(elements,ftxui::text(std::to_wstring(width)));
    });

    std::thread update([&]()
                       {
                           for (;;)
                           {
                               using namespace std::chrono_literals;
                               std::this_thread::sleep_for(0.5s);
                               screen.PostEvent(ftxui::Event::Custom);
                           }
                       });

    auto components = ftxui::Container::Vertical({render,bar});

    components = ftxui::CatchEvent(components, [&](ftxui::Event event)
                                   {
                                       if (event == ftxui::Event::Character('q'))
                                       {
                                           screen.ExitLoopClosure()();
                                           return true;
                                       }
                                       return false;
                                   });
    update.detach();
    screen.Loop(components);

    return 0;
}
