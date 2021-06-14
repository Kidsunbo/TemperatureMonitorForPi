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

ftxui::Color get_current_text_color()
{
    auto temp = get_current_temp();
    if (temp == -1)
    {
        ftxui::Color(ftxui::Color::Palette1::Default);
    }
    return ftxui::Color(155 + temp, 0, 255 - temp);
}

int main()
{

    auto screen = ftxui::ScreenInteractive::TerminalOutput();

    auto text = ftxui::text(L"CPU温度:" + std::to_wstring(get_current_temp()) + L"'C");
    auto render = ftxui::Renderer([&]
                                  { return ftxui::vbox({text | ftxui::color(get_current_text_color()) | ftxui::align_right}); });

    auto components = ftxui::Container::Vertical({render});

    components = ftxui::CatchEvent(components, [&](ftxui::Event event)
                                   {
                                       if (event.character() == L'q')
                                       {
                                           screen.ExitLoopClosure()();
                                           return true;
                                       }
                                       return false;
                                   });
    screen.Loop(components);

    return 0;
}
