#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <stdexcept>

int stringToInt(const std::string &str)
{
    int result = 0;
    bool negative = false;
    size_t i = 0;

    while (i < str.size() && std::isspace(str[i]))
        i++;

    if (i < str.size() && (str[i] == '-' || str[i] == '+'))
    {
        if (str[i] == '-')
            negative = true;
        i++;
    }

    while (i < str.size() && std::isdigit(str[i]))
    {
        result = result * 10 + (str[i] - '0');
        i++;
    }

    if (negative)
        result = -result;

    return result;
}

void parseLine(const std::vector<std::string> &lines, std::unordered_map<std::string, int64_t> &keyValueMap, std::vector<int> args)
{
    for (const std::string &line : lines)
    {
        std::size_t pos = 0;
        while (pos < line.size())
        {
            std::size_t eqPos = line.find('=', pos);
            std::size_t scPos = line.find(';', pos);

            if (eqPos == std::string::npos || scPos == std::string::npos)
                break;

            std::string keyStr = line.substr(pos, eqPos - pos);
            std::string value = line.substr(eqPos + 1, scPos - eqPos - 1);

            pos = scPos + 1;

            int32_t key = stringToInt(keyStr);
            std::string concatenatedStr;

            for (size_t input = 0; input < args.size(); input++)
            {
                if (input == key)
                {
                    concatenatedStr += value;
                    if (input < args.size() - 1)
                        concatenatedStr += "-";
                    break;
                }
            }

            if (!concatenatedStr.empty())
                keyValueMap[concatenatedStr]++;
        }
    }
}

int main(int argc, char *argv[])
{
    std::ifstream inputFile("data.txt");

    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }

    std::string line;
    std::unordered_map<std::string, int64_t> keyValueMap;

    std::vector<std::thread> threads;
    std::vector<std::string> batch;
    const size_t batchSize = 1000;

    std::vector<int> args;
    for (int i = 1; i < argc; i++)
    {
        args.push_back(stringToInt(argv[i]));
    }

    while (std::getline(inputFile, line))
    {
        batch.push_back(line);

        if (batch.size() >= batchSize)
        {
            threads.emplace_back(parseLine, batch, std::ref(keyValueMap), args);
            batch.clear();
        }
    }

    if (!batch.empty())
    {
        threads.emplace_back(parseLine, batch, std::ref(keyValueMap), args);
        batch.clear();
    }

    for (std::thread &t : threads)
        t.join();

    inputFile.close();

    for (const auto &pair : keyValueMap)
        std::cout << "Key: " << pair.first << " Value: " << pair.second << std::endl;

    return 0;
}
