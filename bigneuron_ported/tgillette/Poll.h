//
//  Poll.h
//  BigNeuron Consensus Builder
//
//  Created by Todd Gillette on 5/29/15.
//  Copyright (c) 2015 Todd Gillette. All rights reserved.
//

#ifndef __ConsensusBuilder__Poll__
#define __ConsensusBuilder__Poll__

#include <stdio.h>
#include <map>
#include <set>

// Define an object Counter for determining the amount of reconstructions pointing to each object (e.g. branch, connection)
/*
template<class T> class Poll{
    std::map<T *,int> counter;
    int count;
public:
    void increment(T * object);
    void decrement(T * object);
    void remove(T * object);
    int get_count();
    int get_count(T * object);
    double get_contribution(T * object);
    double get_confidence(); // max contribution
};

template<class T> class WeightedPoll{
    std::map<T *,double> counter;
    double total;
public:
    void increment(T * object, double weight);
    void decrement(T * object, double weight);
    void remove(T * object);
    double get_total();
    double get_weighted_count(T * object);
    double get_contribution(T * object);
    typename std::set<T*> get_objects();
    double get_confidence(); // max contribution
};

 template<class T> class WeightedPoll{
 std::map<T *,double> counter;
 double total;
 public:
 void increment(T * object, double weight);
 void decrement(T * object, double weight);
 void remove(T * object);
 double get_total();
 double get_weighted_count(T * object);
 double get_contribution(T * object);
 typename std::set<T*> get_objects();
 double get_confidence(); // max contribution
 };
 */
class Connection;

class WeightedPoll{
    std::map<Connection *,double> counter;
    double total;
public:
    void increment(Connection * object, double weight);
    void decrement(Connection * object, double weight);
    void remove(Connection * object);
    double get_total();
    double get_weighted_count(Connection * object);
    double get_contribution(Connection * object);
    typename std::set<Connection*> get_objects();
    double get_confidence(); // max contribution
};


#endif /* defined(__ConsensusBuilder__Poll__) */
