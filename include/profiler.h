#pragma once
#include <unordered_map>
#include <string>
#include <chrono>
#include <mutex>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
using namespace std;

struct Profiler {
    mutex mu;
    unordered_map<string, long long> counters;
    unordered_map<string, chrono::steady_clock::time_point> tstarts;
    unordered_map<string, long long> tms;

    void incr(const string &k, long long v=1){
        lock_guard<mutex> lk(mu);
        counters[k]+=v;
    }

    long long get_counter(const string &k){
        lock_guard<mutex> lk(mu);
        if(counters.count(k)) return counters[k];
        return 0;
    }

    void start(const string &k){
        lock_guard<mutex> lk(mu);
        tstarts[k] = chrono::steady_clock::now();
    }

    void stop(const string &k){
        // safer: compute t2 inside the lock
        lock_guard<mutex> lk(mu);
        auto it = tstarts.find(k);
        if(it != tstarts.end()) {
            auto t2 = chrono::steady_clock::now();
            auto dt = chrono::duration_cast<chrono::milliseconds>(t2 - it->second).count();
            if(dt > 0) tms[k] += dt;
            tstarts.erase(it);
    }
    }

    long long get_time_ms(const string &k){
        lock_guard<mutex> lk(mu);
        if(tms.count(k)) return tms[k];
        return 0;
    }

    void report_csv(ostream &out, const vector<string> &keys){
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
