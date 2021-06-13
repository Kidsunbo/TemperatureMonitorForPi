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
    return !s.empty() && std::find_if(s.begin(), 
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

int get_current_temp(){
    auto temp_str = exec("vcgencmd measure_temp");
    std::regex pattern("temp=([0-9]+)'C");
    std::match_results<std::string::iterator> match_result;
    std::regex_match(temp_str.begin(),temp_str.end(),match_result,pattern);
    if(match_result.begin()==match_result.end()){
        return -1;
    }
    auto temp_num_str = match_result.begin()->str();
    if(is_number(temp_num_str)){
        return std::round(std::stod(temp_num_str));
    }
    return -1;
}


int main(){
    
    for(int i=0;i<100;i++){
        std::cout<<"temp is "<<get_current_temp()<<std::endl;
        std::this_thread::sleep_for(1s);
    }
    
    return 0;
}