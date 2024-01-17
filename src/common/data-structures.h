#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <string>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <latch>

#include "utils.h"

namespace mapreduce {

/**
 * A simple class that generates unique identifiers.
*/
class Identifier {
public:
    /**
     * Returns the next unique identifier. Thread-safe.
    */
    inline uint32_t get_next() { return next_id++; }

private:
    std::atomic<uint32_t> next_id{0};
};

/**
 * A queue of work items, held by a worker server to serve requests to client binaries.
*/
template<class V>
class ListenerWorkQueue {
public:
    /**
     * Adds a new work item to the queue. Items are grouped by execpath.
    */
    void add(std::string const& execpath, V const& item);

    /**
     * Returns any work item that can be processed by worker with a given execpath. 
     * Assumes that there is at least one item in the queue.
    */
    V get_one(std::string const& execpath);

private:
    std::unordered_map<std::string, std::queue<V>> work_queue;
    std::mutex mutex;
};

/**
 * Contains all information needed by the master to keep track of a single client request.
 * [TODO] Add proper documentation, and rewrite so that there is less code duplication.
*/
class ClientRequestInfo {
public:
    ClientRequestInfo(ClientRequest const& request_)
        : request(request_),
          map_latch(request.num_mappers()), reduce_latch(request.num_reducers()) {}

    void add_mapper_job(MapRequest const& request);

    void add_reducer_job(ReduceRequest const& request);

    void complete_mapper_job(uint32_t id);

    void complete_reducer_job(uint32_t id);

    void wait_for_map();

    void wait_for_reduce();

private:
    ClientRequest request;
    std::unordered_map<uint32_t, MapRequest> ongoing_map_requests;
    std::unordered_map<uint32_t, ReduceRequest> ongoing_reduce_requests;
    std::mutex mutex;
    std::latch map_latch;
    std::latch reduce_latch;
};

/**
 * A structure that hold all informations needed by the master to forward requests to mappers / reducers
 * and keep track of their status.
*/
class ClientRequestQueue {
public:
    ClientRequestInfo& add_request(uint32_t req_id, ClientRequest const& request);

    void complete_request(uint32_t req_id);

    ClientRequestInfo& get_request_info(uint32_t req_id);

private:
    std::unordered_map<uint32_t, ClientRequestInfo> ongoing_requests;
    std::mutex mutex;
};

} // mapreduce

// https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
// The more you know XD. Wygląda dziwnie ale działa.
#include "data-structures.cc"

#endif // DATA_STRUCTURES_H