#include <iostream>
#include <sstream>
#include <regex>
#include <cmath>
#include <thread>
#include <vector>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <chrono>

using std::literals::chrono_literals::operator""s;

std::vector<ftxui::Color> colors;

void init_colors()
{
    colors.reserve(800);
    for (int i = 66; i <= 245; i++)
    {
        colors.push_back(ftxui::Color(66, i, 245));
    }
    for (int i = 245; i >= 66; i--)
    {
        colors.push_back(ftxui::Color(66, 245, i));
    }
    for (int i = 66; i <= 245; i++)
    {
        colors.push_back(ftxui::Color(i, 245, 66));
    }
    for (int i = 245; i >= 66; i--)
    {
        colors.push_back(ftxui::Color(245, i, 66));
    }
}

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

ftxui::Color get_text_color(int cur, int range)
{
   int index = colors.size()*(static_cast<double>(cur)/range);
   if(index>=colors.size()){
       index = colors.size()-1;
   }
   return colors[index];
}

ftxui::Color get_current_text_color()
{
    auto temp = get_current_temp();
    if (temp == -1)
    {
        ftxui::Color(ftxui::Color::Palette1::Default);
    }
    if(temp>=100){
        temp==99;
    }
    return colors[static_cast<int>(colors.size()*temp/100.0)];
}

int main()
{
    setenv("TERM", "xterm-256color", 1);
    setenv("COLORTERM", "truecolor", 1);
    init_colors();

    auto screen = ftxui::ScreenInteractive::TerminalOutput();

    auto render = ftxui::Renderer([&]
                                  {
                                      auto text = ftxui::text(L"CPU温度:" + std::to_wstring(get_current_temp()) + L"°C");

                                      return ftxui::vbox({text | ftxui::color(get_current_text_color()) | ftxui::center,
                                                          ftxui::separator()});
                                  });

    auto bar = ftxui::Renderer([&]
                               {
                                   ftxui::Elements elements;
                                   auto temp = get_current_temp();
                                   auto width = screen.dimx();
                                   int should_width = std::round(temp / 100.0 * width);
                                   for (int i = 0; i < should_width; i++)
                                   {
                                       elements.push_back(ftxui::text(L" ") | ftxui::bgcolor(get_text_color(i,width)));
                                   }
                                   return ftxui::hbox(elements);
                               });

    std::thread update([&]()
                       {
                           for (;;)
                           {
                               using namespace std::chrono_literals;
                               std::this_thread::sleep_for(0.1s);
                               screen.PostEvent(ftxui::Event::Custom);
                           }
                       });

    auto components = ftxui::Container::Vertical({render, bar});

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
