#pragma once
#include <unordered_map>
#include <string>
#include <chrono>
#include <mutex>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>

struct Profiler {
    std::mutex mu;
    std::unordered_map<std::string, long long> counters;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> tstarts;
    std::unordered_map<std::string, long long> tms;

    void incr(const std::string &k, long long v=1){
        std::lock_guard<std::mutex> lk(mu);
        counters[k]+=v;
    }

    long long get_counter(const std::string &k){
        std::lock_guard<std::mutex> lk(mu);
        if(counters.count(k)) return counters[k];
        return 0;
    }

    void start(const std::string &k){
        std::lock_guard<std::mutex> lk(mu);
        tstarts[k] = std::chrono::steady_clock::now();
    }

    void stop(const std::string &k){
        // safer: compute t2 inside the lock
        std::lock_guard<std::mutex> lk(mu);
        auto it = tstarts.find(k);
        if(it != tstarts.end()) {
            auto t2 = std::chrono::steady_clock::now();
            auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - it->second).count();
            if(dt > 0) tms[k] += dt;
            tstarts.erase(it);
    }
    }

    long long get_time_ms(const std::string &k){
        std::lock_guard<std::mutex> lk(mu);
        if(tms.count(k)) return tms[k];
        return 0;
    }
    
    void report_csv(std::ostream &out, const std::vector<std::string> &keys){
        // prints selected counters and times in CSV order
        for(size_t i=0;i<keys.size();++i){
            auto &k = keys[i];
            if(i) out << ",";
            if(k.rfind("time:",0)==0){
                out << get_time_ms(k.substr(5));
            } else {
                out << get_counter(k);
            }
        }
        out << "\n";
    }
};
