#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <stdexcept>

int stringToInt(const std::string& str) {
    int result = 0;
    int sign = 1;
    size_t i = 0;

    while (i < str.size() && std::isspace(str[i]))
        i++;

    if (i < str.size() && (str[i] == '-' || str[i] == '+')) {
        if (str[i] == '-') {
            sign = -1;
        }
        i++;
    }

    while (i < str.size() && std::isdigit(str[i])) {
        int digit = str[i] - '0';

        if (result > (INT_MAX - digit) / 10) 
            throw std::overflow_error("Integer overflow");
        
        result = result * 10 + digit;
        i++;
    }

    // Ensure that the entire string is consumed
    while (i < str.size() && std::isspace(str[i])) 
        i++;

    if (i < str.size()) 
        throw std::invalid_argument("Invalid input: " + str);

    return result * sign;
}

// Function to parse a batch of lines and update the map
void parseLine(const std::vector<std::string>& lines, std::unordered_map<std::string, int64_t>& keyValueMap, int argc, char* argv[]) {
    for (const std::string& line : lines) {
        std::size_t pos = 0;
        while (pos < line.size()) {
            std::size_t eqPos = line.find('=', pos);
            std::size_t scPos = line.find(';', pos);

            if (eqPos == std::string::npos || scPos == std::string::npos)
                break;

            std::string keyStr = line.substr(pos, eqPos - pos);
            std::string value = line.substr(eqPos + 1, scPos - eqPos - 1);

            pos = scPos + 1;

            int32_t key = stringToInt(keyStr);
            std::string concatenatedStr;

            for(size_t input = 0; input < argc; input++)
            {
                if(input == key)
                {
                    concatenatedStr += value;
                    concatenatedStr += (input < argc - 1) ? "-" : "";
                }
            }

            if(!concatenatedStr.empty())
                keyValueMap[concatenatedStr]++;
        }
    }
}

int main(int argc, char* argv[]) {
    std::ifstream inputFile("data.txt");

    if (!inputFile.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }

    std::string line;
    std::unordered_map<std::string, int64_t> keyValueMap;

    std::vector<std::thread> threads;
    std::vector<std::string> batch;
    const size_t batchSize = 1000;

    while (std::getline(inputFile, line)) {
        batch.push_back(line);

        if (batch.size() >= batchSize) {
            threads.emplace_back(parseLine, batch, std::ref(keyValueMap), argc, argv);
            batch.clear();
        }
    }

    if (!batch.empty()) {
        threads.emplace_back(parseLine, batch, std::ref(keyValueMap), argc, argv);
        batch.clear();
    }

    for (std::thread& t : threads)
        t.join();

    inputFile.close();

    for (const auto& pair : keyValueMap)
        std::cout << "Key: " << pair.first << " Value: " << pair.second << std::endl;

    return 0;
}
