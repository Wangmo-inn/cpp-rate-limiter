# Designing of Rate Limiter

A simple, thread-safe C++ implementation of the **5 core rate-limiting algorithms** frequently discussed in System Design interviews.

## Algorithms Implemented

### 1. Token Bucket
Allows bursts of traffic while maintaining an average request rate.

- Tokens are added at a constant rate.
- Each request consumes one token.
- Requests are rejected when no tokens are available.
- Supports controlled bursts up to the bucket capacity.

### 2. Leaky Bucket
Smooths out bursty traffic into a steady stream.

- Requests are added to a queue.
- Requests are processed at a fixed rate.
- Excess requests are rejected when the queue is full.
- Provides a predictable output rate.

### 3. Fixed Window Counter
Divides time into fixed intervals and limits the number of requests in each interval.

- Maintains a request counter for the current window.
- Counter resets when the window expires.
- Simple and memory efficient.
- Can allow bursts at window boundaries.

### 4. Sliding Window Log
Provides accurate rate limiting using individual request timestamps.

- Stores the timestamp of every request.
- Removes timestamps outside the current time window.
- Provides accurate enforcement.
- Requires more memory as request timestamps are stored.

### 5. Sliding Window Counter
A hybrid approach that combines the efficiency of Fixed Window Counter with the smoother behavior of a Sliding Window.

- Maintains request counts for the current and previous windows.
- Uses weighted counts to estimate requests in the current sliding window.
- More memory efficient than Sliding Window Log.
- Reduces boundary burst problems.

## Comparison

| Algorithm | Burst Handling | Accuracy | Memory Usage | Main Advantage |
|---|---|---|---|---|
| Token Bucket | Allows bursts | High | Low | Handles burst traffic |
| Leaky Bucket | Smooths bursts | High | Low/Medium | Constant output rate |
| Fixed Window Counter | Boundary bursts possible | Medium | Very Low | Simple and efficient |
| Sliding Window Log | Controlled | Very High | High | Accurate enforcement |
| Sliding Window Counter | Controlled | High | Low | Balance of accuracy and memory |

## Thread Safety

The implementation uses C++ synchronization primitives to make the rate limiters thread-safe.

Key components include:

- `std::mutex`
- `std::lock_guard`
- `std::chrono`
- STL containers such as `std::queue` and `std::deque`

This prevents race conditions when multiple threads access the rate limiter concurrently.

## How It Works

A typical rate-limiting flow looks like:

```text
Client
   |
   v
Rate Limiter
   |
   +---- Request Allowed ----> Backend Service
   |
   +---- Request Rejected ---> HTTP 429
                               Too Many Requests
```

When a request arrives, the rate limiter checks whether the request is allowed according to the selected algorithm.

If the limit has been exceeded, the request is rejected.

## Example

For a limit of **5 requests per second**:

```text
Request 1  -> Allowed
Request 2  -> Allowed
Request 3  -> Allowed
Request 4  -> Allowed
Request 5  -> Allowed
Request 6  -> Rejected
```

The exact behavior depends on the rate-limiting algorithm being used.

## Project Structure

```text
Rate-Limiter/
│
├── main.cpp
├── token_bucket.cpp
├── leaky_bucket.cpp
├── fixed_window.cpp
├── sliding_window_log.cpp
├── sliding_window_counter.cpp
│
└── README.md
```

> Update the project structure if your implementation uses different file names.

## How to Run

This project uses only standard C++ libraries and has **no external dependencies**.

### Compile

Using `g++`:

```bash
g++ -std=c++17 -pthread main.cpp -o rate_limiter
```

If the implementations are split across multiple `.cpp` files:

```bash
g++ -std=c++17 -pthread *.cpp -o rate_limiter
```

### Run

Linux/macOS:

```bash
./rate_limiter
```

Windows:

```bash
rate_limiter.exe
```

## Key Concepts

This project demonstrates:

* Rate Limiting
* Token Bucket
* Leaky Bucket
* Fixed Window Counter
* Sliding Window Log
* Sliding Window Counter
* Thread Safety
* Concurrency
* Mutex-based synchronization
* Time-based calculations
* Queue-based request management
* Traffic shaping
* Burst handling
* Memory vs. accuracy trade-offs

## Why Rate Limiting?

Rate limiting is commonly used to:

* Prevent API abuse
* Protect backend services from excessive traffic
* Control resource consumption
* Prevent sudden traffic spikes
* Enforce API quotas
* Maintain service availability

## System Design Trade-offs

```text
Token Bucket
    |
    +--> Good for handling bursts


Leaky Bucket
    |
    +--> Good for smooth and predictable traffic


Fixed Window Counter
    |
    +--> Simple and memory efficient
    +--> Can suffer from boundary bursts


Sliding Window Log
    |
    +--> Highly accurate
    +--> Higher memory usage


Sliding Window Counter
    |
    +--> Memory efficient
    +--> Better handling of boundary bursts
    +--> Good balance between accuracy and performance
```

## Technologies Used

* C++17
* STL
* Multithreading
* Mutex
* `std::chrono`
* `std::queue`
* `std::deque`

## Future Improvements

* Per-user rate limiting
* Per-IP rate limiting
* Configurable limits
* Redis-based distributed rate limiting
* HTTP API integration
* Unit testing
* Performance benchmarking
* Monitoring and metrics
* Distributed rate limiting across multiple servers

## Learning Objective

The goal of this project is to understand and implement commonly used rate-limiting algorithms while exploring their **performance, memory, accuracy, concurrency, and system-design trade-offs**.

It also serves as a practical implementation for **C++ and System Design interview preparation**.

## Author

**Rigzin Wangmo**
[GitHub](https://github.com/Wangmo-inn/cpp-rate-limiter)
