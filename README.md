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

## Author

**Rigzin Wangmo**
[GitHub](https://github.com/Wangmo-inn/cpp-rate-limiter)
