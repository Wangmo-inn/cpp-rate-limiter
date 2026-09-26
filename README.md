\# Designing of Rate Limiter



A simple, thread-safe C++ implementation of the 5 core rate-limiting algorithms frequently discussed in System Design interviews.



\## Algorithms Implemented

1\. \*\*Token Bucket:\*\* Allows bursts of traffic. Tokens are added at a constant rate.

2\. \*\*Leaky Bucket:\*\* Smooths out bursty traffic into a steady stream.

3\. \*\*Fixed Window Counter:\*\* Simple but prone to edge-case burst spikes.

4\. \*\*Sliding Window Log:\*\* 100% accurate enforcement, but memory-intensive.

5\. \*\*Sliding Window Counter:\*\* Hybrid approach that fixes the Fixed Window flaw while remaining memory-efficient.



\## How to Run

This project uses standard C++ libraries and has no external dependencies. 



Compile and run using g++:

