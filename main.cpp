#include <iostream>
#include <sstream>
#include <regex>
#include <cmath>
#include <thread>

using std::literals::chrono_literals::operator ""s;

std::string exec(const std::string s){
    auto file = popen(s.c_str(),"r");
    if(file==nullptr){
        std::cout<<"popen failed"<<std::endl;
        return "";
    }
    std::stringstream ss;
    
    char buf[1024];
    while(fgets(buf,sizeof(buf),file)!= nullptr){
        ss << buf;
    }
    return ss.str();
}

bool is_number(const std::string& s)
{
    char* end = nullptr;
    double val = strtod(s.c_str(), &end);
    return end != s.c_str() && *end == '\0' && val != HUGE_VAL;
}

int get_current_temp(){
    auto temp_str = exec("vcgencmd measure_temp");
    std::cout<<temp_str<<std::endl;
    std::regex pattern("temp=(.+)'C");
    std::match_results<std::string::iterator> match_result;
    std::regex_match(temp_str.begin(),temp_str.end(),match_result,pattern);
    if(match_result.size()<2){
        std::cout<<"match failed"<<std::endl;
        return -1;
    }
    auto temp_num_str = match_result[1].str();
    if(is_number(temp_num_str)){
        return std::round(std::stod(temp_num_str));
    }
            std::cout<<"failed"<<std::endl;
    return -1;
}


int main(){
    
    for(int i=0;i<100;i++){
        std::cout<<"temp is "<<get_current_temp()<<std::endl;
        std::this_thread::sleep_for(1s);
    }

    
    return 0;
}