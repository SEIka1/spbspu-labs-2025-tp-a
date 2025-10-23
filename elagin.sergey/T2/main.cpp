#include <algorithm>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>
#include "data-struct.hpp"

int main()
{
  using namespace elagin;
  using input_it_t = std::istream_iterator<DataStruct>;
  using output_it_t = std::ostream_iterator<DataStruct>;

  std::vector<DataStruct> sdata;

  while (!std::cin.eof()) {
    if (std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::copy(input_it_t{std::cin}, input_it_t{}, std::back_inserter(sdata));
  }

  if (std::cin.bad()) {
    std::cerr << "Input error\n";
    return 1;
  }

  std::sort(sdata.begin(), sdata.end());
  std::copy(sdata.cbegin(), sdata.cend(), output_it_t{std::cout, "\n"});

  return 0;
}
