#include <iostream>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <queue>
#include <thread>
#include <vector>

// ==========================================
// 1. BASE INTERFACE
// ==========================================
class RateLimiter {
protected:
    // Helper to get current time in milliseconds
    long long currentTimeMillis() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
public:
    virtual bool allowRequest(int clientId) = 0;
    virtual ~RateLimiter() = default;
};


// ==========================================
// 2. TOKEN BUCKET
// Allows bursts of traffic. Tokens are added at a constant rate.
// ==========================================
class TokenBucket : public RateLimiter {
    int capacity;
    int refillRatePerSecond;
    
    struct ClientState {
        double currentTokens;
        long long lastRefillTime;
    };
    
    std::unordered_map<int, ClientState> clients;
    std::mutex mtx; // Global lock for interview simplicity (shard this in production)

public:
    TokenBucket(int cap, int refillRate) : capacity(cap), refillRatePerSecond(refillRate) {}

    bool allowRequest(int clientId) override {
        std::lock_guard<std::mutex> lock(mtx);
        long long now = currentTimeMillis();
        
        if (clients.find(clientId) == clients.end()) {
            clients[clientId] = { (double)capacity, now };
        }

        ClientState& state = clients[clientId];
        double timeElapsed = (now - state.lastRefillTime) / 1000.0;
        
        // Refill tokens based on time passed
        state.currentTokens = std::min((double)capacity, state.currentTokens + timeElapsed * refillRatePerSecond);
        state.lastRefillTime = now;

        // Check if we have at least 1 token to process the request
        if (state.currentTokens >= 1.0) {
            state.currentTokens -= 1.0;
            return true;
        }
        return false;
    }
};


// ==========================================
// 3. LEAKY BUCKET (Metered)
// Smooths out traffic. Requests fill the bucket, which leaks at a constant rate.
// ==========================================
class LeakyBucket : public RateLimiter {
    int capacity; 
    int leakRatePerSecond; 
    
    struct ClientState {
        double currentWater; // Represents queued requests
        long long lastLeakTime;
    };
    std::unordered_map<int, ClientState> clients;
    std::mutex mtx;

public:
    LeakyBucket(int cap, int leakRate) : capacity(cap), leakRatePerSecond(leakRate) {}

    bool allowRequest(int clientId) override {
        std::lock_guard<std::mutex> lock(mtx);
        long long now = currentTimeMillis();

        if (clients.find(clientId) == clients.end()) {
            clients[clientId] = { 0.0, now };
        }

        ClientState& state = clients[clientId];
        double timeElapsed = (now - state.lastLeakTime) / 1000.0;
        
        // Water (requests) leaks out over time
        state.currentWater = std::max(0.0, state.currentWater - timeElapsed * leakRatePerSecond);
        state.lastLeakTime = now;

        // If bucket isn't full, accept the request (add water)
        if (state.currentWater + 1.0 <= capacity) {
            state.currentWater += 1.0;
            return true;
        }
        return false;
    }
};


// ==========================================
// 4. FIXED WINDOW COUNTER
// Resets count at fixed time intervals. Prone to edge-case bursts.
// ==========================================
class FixedWindowCounter : public RateLimiter {
    int maxRequests;
    long long windowSizeMillis;
    
    struct ClientState {
        int counter;
        long long windowStartTime;
    };
    std::unordered_map<int, ClientState> clients;
    std::mutex mtx;

public:
    FixedWindowCounter(int maxReq, long long windowSizeMs) 
        : maxRequests(maxReq), windowSizeMillis(windowSizeMs) {}

    bool allowRequest(int clientId) override {
        std::lock_guard<std::mutex> lock(mtx);
        long long now = currentTimeMillis();
        // Floor the current time to the nearest window boundary
        long long currentWindowStart = (now / windowSizeMillis) * windowSizeMillis;

        if (clients.find(clientId) == clients.end() || clients[clientId].windowStartTime != currentWindowStart) {
            clients[clientId] = {0, currentWindowStart};
        }

        if (clients[clientId].counter < maxRequests) {
            clients[clientId].counter++;
            return true;
        }
        return false;
    }
};


// ==========================================
// 5. SLIDING WINDOW LOG
// 100% accurate, but memory intensive. Keeps a timestamp per request.
// ==========================================
class SlidingWindowLog : public RateLimiter {
    int maxRequests;
    long long windowSizeMillis;
    std::unordered_map<int, std::queue<long long>> clients;
    std::mutex mtx;

public:
    SlidingWindowLog(int maxReq, long long windowSizeMs) 
        : maxRequests(maxReq), windowSizeMillis(windowSizeMs) {}

    bool allowRequest(int clientId) override {
        std::lock_guard<std::mutex> lock(mtx);
        long long now = currentTimeMillis();
        long long windowStart = now - windowSizeMillis;

        std::queue<long long>& logs = clients[clientId];

        // Clean up requests older than the sliding window
        while (!logs.empty() && logs.front() <= windowStart) {
            logs.pop();
        }

        if (logs.size() < maxRequests) {
            logs.push(now);
            return true;
        }
        return false;
    }
};


// ==========================================
// 6. SLIDING WINDOW COUNTER
// Hybrid approach: Memory efficient, highly accurate.
// ==========================================
class SlidingWindowCounter : public RateLimiter {
    int maxRequests;
    long long windowSizeMillis;
    
    struct WindowState {
        long long currentWindowStart;
        int currentCount;
        int previousCount;
    };
    std::unordered_map<int, WindowState> clients;
    std::mutex mtx;

public:
    SlidingWindowCounter(int maxReq, long long windowSizeMs) 
        : maxRequests(maxReq), windowSizeMillis(windowSizeMs) {}

    bool allowRequest(int clientId) override {
        std::lock_guard<std::mutex> lock(mtx);
        long long now = currentTimeMillis();
        long long currentWindowStart = (now / windowSizeMillis) * windowSizeMillis;

        if (clients.find(clientId) == clients.end()) {
            clients[clientId] = {currentWindowStart, 0, 0};
        }

        WindowState& state = clients[clientId];
        
        // If we crossed into a new window, shift counts
        if (state.currentWindowStart != currentWindowStart) {
            long long windowsPassed = (currentWindowStart - state.currentWindowStart) / windowSizeMillis;
            // If exactly 1 window passed, carry over count. Otherwise, they were idle too long.
            state.previousCount = (windowsPassed == 1) ? state.currentCount : 0;
            state.currentCount = 0;
            state.currentWindowStart = currentWindowStart;
        }

        // Calculate overlap weight of the previous window
        double overlapPercentage = (double)(windowSizeMillis - (now - currentWindowStart)) / windowSizeMillis;
        int estimatedTotalRequests = (state.previousCount * overlapPercentage) + state.currentCount;

        if (estimatedTotalRequests < maxRequests) {
            state.currentCount++;
            return true;
        }
        return false;
    }
};

// ==========================================
// 7. MAIN INTEGRATION TEST
// ==========================================
int main() {
    // Test setup: Allow 3 requests per second
    TokenLimiter* limiter = new TokenBucket(3, 3); 
    // You can easily swap this out:
    // RateLimiter* limiter = new SlidingWindowLog(3, 1000);
    
    int clientId = 101;

    std::cout << "--- Sending 5 rapid requests (Burst) ---" << std::endl;
    for (int i = 1; i <= 5; i++) {
        bool allowed = limiter->allowRequest(clientId);
        std::cout << "Request " << i << ": " << (allowed ? "ACCEPTED" : "DROPPED") << std::endl;
    }

    std::cout << "\n--- Waiting 1.1 seconds for refill/window shift ---" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    std::cout << "Request 6: " << (limiter->allowRequest(clientId) ? "ACCEPTED" : "DROPPED") << std::endl;

    delete limiter;
    return 0;
}