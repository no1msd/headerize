#include <iostream>
#include <fstream>
#include <functional>
#include <system_error>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

void open_file(
    const std::string& filename,
    std::ios_base::openmode mode,
    std::function<void(std::fstream&)> callback)
{
  std::fstream stream(filename, mode);
  if (!stream.is_open())
    throw std::system_error(
        EIO,
        std::system_category(),
        "Error opening file '" + filename + "'");

  try {
    callback(stream);
  } catch(...) {
    stream.close();
    throw;
  }
}

void wrap_string(
    std::istream& input,
    std::ostream& output,
    const std::string& variable_name)
{
  output << "const std::string " << variable_name << "{\n";
  std::string line;
  while (std::getline(input, line)) {
    boost::replace_all(line, "\\", "\\\\");
    boost::replace_all(line, "\"", "\\\"");
    output << "  \"" << line;
    if (!input.eof())
      output << "\\n";
    output << "\"\n";
  }
  output << "};\n";
}

std::string variable_name(const std::string& in) {
  std::string variable_name = boost::replace_all_copy(in, ".", "_");
  boost::replace_all_regex(
      variable_name,
      boost::regex("[^0-9a-zA-Z_]"),
      std::string{""});
  return variable_name;
}

void render_header(std::ostream& output, const po::variables_map& vm) {
  output << "#pragma once" << std::endl;

  if (vm.count("namespace"))
    output << "namespace " << vm["namespace"].as<std::string>() << " {\n";

  if (vm.count("input"))
    for (auto& name: vm["input"].as<std::vector<std::string>>())
      open_file(name, std::ios::in, [&output,&name](std::fstream& file) {
        wrap_string(file, output, variable_name(name));
      });

  if (vm.count("namespace"))
    output << "}\n";
}

int main(int argc, char* argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()
      ("help,h", "show help")
      ("output,o", po::value<std::string>(),
          "Output file. Leave empty to use stdout.")
      ("namespace,n", po::value<std::string>(),
          "Namespace to put the variables in.")
      ("input,i", po::value<std::vector<std::string>>(),
          "List of files to parse.");
  po::variables_map vm;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help") || !vm.count("input")) {
      std::cout << desc << std::endl;
      return 1;
    }

    if (vm.count("output")) {
      auto filename = vm["output"].as<std::string>();
      open_file(filename, std::ios::out, [&vm](std::fstream& file) {
        render_header(file, vm);
      });
    } else {
      render_header(std::cout, vm);
    }
  } catch(std::exception& error) {
    std::cerr << error.what() << std::endl;
    return 1;
  }

  return 0;
}
