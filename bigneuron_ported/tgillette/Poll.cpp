//
//  Poll.cpp
//  BigNeuron Consensus Builder
//
//  Created by Todd Gillette on 5/29/15.
//  Copyright (c) 2015 Todd Gillette. All rights reserved.
//

#include "Poll.h"

/* Class Poll */
// Define an object Poll for determining the amount of reconstructions pointing to each object (e.g. branch, connection)
/*
template<class T>
void Poll<T>::increment(T * object){
    count++;
    if (counter.count(object) == 0){
        counter.insert(object, 1);
    }else{
        counter.insert(object, counter.at(object)+1);
    }
};
template<class T> void Poll<T>::decrement(T * object){
    int count = counter.at(object);
    counter.remove(object);
    if (count > 1){
        counter.insert(object,count-1);
    }
}
template<class T> void Poll<T>::remove(T * object){
    counter.remove(object);
}


template<class T>
int Poll<T>::get_count(){
    return count;
};

template<class T>
int Poll<T>::get_count(T * object){
    return counter.at(object);
};

template<class T>
double Poll<T>::get_contribution(T * object){
    if (counter.find(object) != counter.end()){
        return ((double)counter.at(object))/count;
    }
    return 0;
};

template<class T>
double Poll<T>::get_confidence(){
    double max_confidence = 0;
    for (T key : counter){
        if (counter.get(key) > max_confidence){
            max_confidence = counter.getKey();
        }
    }
    return max_confidence;
}
*/

/* Class WeightedPoll */
/*
template<class T>
void WeightedPoll<T>::increment(T * object, double weight){
    total += weight;
    if (counter.count(object) == 0){
        counter.insert(object, weight);
    }else{
        counter.insert(object, counter.at(object)+weight);
    }
};
template<class T> void WeightedPoll<T>::decrement(T * object, double weight){
    double total = counter.at(object);
    counter.remove(object);
    if (total-weight > 0){
        counter.insert(object,total-weight);
    }
}
template<class T> void WeightedPoll<T>::remove(T * object){
    counter.remove(object);
}

template<class T>
double WeightedPoll<T>::get_total(){
    return total;
};

template<class T>
double WeightedPoll<T>::get_weighted_count(T * object){
    return counter.at(object);
};

template<class T>
double WeightedPoll<T>::get_contribution(T * object){
    if (counter.find(object) != counter.end()){
        return ((double)counter.at(object))/total;
    }
    return 0;
};
template<class T>
typename std::set<T*> WeightedPoll<T>::get_objects(){
    typename std::set<T*> items;
    for(typename std::map<T*,double>::iterator it = counter.begin(); it != counter.end(); ++it) {
        items.insert(it->first);
    }
    return items;
}

template<class T>
double WeightedPoll<T>::get_confidence(){
    double max_confidence = 0;
    for (T key : counter){
        if (counter.get(key) > max_confidence){
            max_confidence = counter.get(key);
        }
    }
    return max_confidence;
}
*/

typedef std::pair<Connection*,double> ConWeightPair;
void WeightedPoll::increment(Connection * object, double weight){
    total += weight;
    if (counter.count(object) == 0){
        counter.insert(ConWeightPair(object, weight));
    }else{
        counter.insert(ConWeightPair(object, counter.at(object)+weight));
    }
};
void WeightedPoll::decrement(Connection * object, double weight){
    double total = counter.at(object);
    counter.erase(object);
//    counter.remove(object);
    if (total-weight > 0){
        counter.insert(ConWeightPair(object,total-weight));
    }
}
void WeightedPoll::remove(Connection * object){
//    counter.remove(object);
    counter.erase(object);
}

double WeightedPoll::get_total(){
    return total;
};

double WeightedPoll::get_weighted_count(Connection * object){
    return counter.at(object);
};

double WeightedPoll::get_contribution(Connection * object){
    if (counter.find(object) != counter.end()){
        return ((double)counter.at(object))/total;
    }
    return 0;
};
typename std::set<Connection*> WeightedPoll::get_objects(){
    typename std::set<Connection*> items;
    for(typename std::map<Connection*,double>::iterator it = counter.begin(); it != counter.end(); ++it) {
        items.insert(it->first);
    }
    return items;
}

double WeightedPoll::get_confidence(){
    double max_confidence = 0;
    for (ConWeightPair obj_pair : counter){
        if (obj_pair.second > max_confidence){
            max_confidence = obj_pair.second;
        }
    }
/*    return max_confidence;
    for (Connection * key : counter){
        if (counter.get(key) > max_confidence){
            max_confidence = counter.getKey();
        }
    }*/
    return max_confidence;
}
