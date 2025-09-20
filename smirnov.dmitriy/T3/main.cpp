#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <limits>
#include <functional>
#include <iterator>
#include "polygon.hpp"
#include "commands.hpp"

int main(int argc, char* argv[])
{
    using namespace smirnov;
    if (argc != 2)
    {
    std::cerr << "not a file" << "/n";
    return 1;
    }
        std::string fname = argv[1];
        std::ifstream file(fname);
        std::vector< Polygon > polygons;
        using inputIt = std::istream_iterator< Polygon >;
        while (!file.eof())
        {
            std::copy(inputIt{ file }, inputIt{}, std::back_inserter(polygons));
            file.clear();
            file.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
        }
        std::map < std::string, std::function< void(std::istream&, std::ostream&) > > commands;
        {
            using namespace std::placeholders;
            commands["AREA"] = std::bind(doAreaCommand, std::cref(polygons), _1, _2);
            commands["MAX"] = std::bind(doMaxCommand, std::cref(polygons), _1, _2);
            commands["MIN"] = std::bind(doMinCommand, std::cref(polygons), _1, _2);
            commands["COUNT"] = std::bind(doCountCommand, std::cref(polygons), _1, _2);
            commands["RECTS"] = std::bind(doRectsCommand, std::cref(polygons), _2);
            commands["INTERSECTIONS"] = std::bind(doIntersections, std::cref(polygons), _1, _2);
        }

        std::string cmd;
        while (std::cin >> cmd)
        {
            try
            {
                commands.at(cmd)(std::cin, std::cout);
            }
            catch (...)
            {
                std::cout << "<INVALID COMMAND>\n";
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
        }
        file.close();
    return 0;
}
