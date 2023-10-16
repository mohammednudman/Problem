#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

constexpr auto EQUALS_SYBL = '=';
constexpr auto SEMICOLON_SYBL = ';';

void parseLine(const std::string& line, std::unordered_map<std::string, std::string>& keyValueMap) {
    std::size_t pos = 0;
    while (pos < line.size()) {
        std::size_t eqPos = line.find(EQUALS_SYBL, pos);
        std::size_t scPos = line.find(SEMICOLON_SYBL, pos);

        if (eqPos == std::string::npos || scPos == std::string::npos)
            break;

        std::string key = line.substr(pos, eqPos - pos);
        std::string value = line.substr(eqPos + 1, scPos - eqPos - 1);

        pos = scPos + 1;

        keyValueMap[key] = value;
    }
}

int main() {
    std::ifstream inputFile("data.txt");

    if (!inputFile.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }

    std::string line;
    std::unordered_map<std::string, std::string> keyValueMap;

    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    std::queue<std::string> linesQueue;
    std::mutex queueMutex;
    std::condition_variable cv;

    // Function to process lines from the queue
    auto processLines = [&]() {
        while (true) {
            std::string line;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [&] { return !linesQueue.empty(); });
                line = std::move(linesQueue.front());
                linesQueue.pop();
            }
            if (line.empty()) {
                break;  // Thread termination signal
            }
            parseLine(line, keyValueMap);
        }
    };

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(processLines);
    }

    while (std::getline(inputFile, line)) {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            linesQueue.push(line);
        }
        cv.notify_one();
    }

    // Send termination signals to threads
    for (int i = 0; i < numThreads; ++i) {
        linesQueue.push("");  // Thread termination signal
        cv.notify_one();
    }

    for (std::thread& t : threads) {
        t.join();
    }

    inputFile.close();

    // Rest of your processing code
    for (const auto& pair : keyValueMap) {
        std::cout << "Key: " << pair.first << " Value: " << pair.second << std::endl;
    }

    return 0;
}