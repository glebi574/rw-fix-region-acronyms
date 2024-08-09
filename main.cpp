#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>

static std::filesystem::path output_path = "output";
static std::filesystem::path input_path;
static std::string old_acronym, old_acronym_lower, new_acronym, new_acronym_lower;
static std::regex acronym_pattern1, acronym_pattern2, acronym_pattern_lower1, acronym_pattern_lower2;

std::string str_to_lower(std::string& str) {
  std::string str_lower = "";
  for (char c : str) {
    str_lower += std::tolower(c);
  }
  return str_lower;
}

std::string fix_acronym_in_str(std::string& str) {
  std::string new_str = std::regex_replace(str, acronym_pattern1, new_acronym + '_');
  new_str = std::regex_replace(new_str, acronym_pattern2, new_acronym + '-');
  new_str = std::regex_replace(new_str, acronym_pattern_lower1, new_acronym_lower + '_');
  new_str = std::regex_replace(new_str, acronym_pattern_lower2, new_acronym_lower + '-');
  return new_str;
}

void fix_folders(std::filesystem::path* input_path, std::filesystem::path* output_path) {
  for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(*input_path)) {
    if (entry.is_directory()) {
      std::filesystem::path local_input_path = std::filesystem::path(entry.path());
      if (local_input_path.filename() == ".git")
        continue;

      std::string fixed_output_path = entry.path().filename().string();
      if (fixed_output_path.size() == 2 && fixed_output_path == old_acronym_lower)
        fixed_output_path = new_acronym_lower;
      else
        fixed_output_path = fix_acronym_in_str(fixed_output_path);
      std::filesystem::path local_ouput_path = std::filesystem::path(*output_path / fixed_output_path);
      std::filesystem::create_directory(local_ouput_path);
      fix_folders(&local_input_path, &local_ouput_path);
    }
    else if (entry.is_regular_file()) {
      std::string fixed_output_path = entry.path().filename().string();
      fixed_output_path = (*output_path / fix_acronym_in_str(fixed_output_path)).string();
      std::filesystem::copy_file(entry.path(), fixed_output_path, std::filesystem::copy_options::overwrite_existing);

      if (entry.path().extension() != ".txt")
        continue;

      std::ifstream file(fixed_output_path);
      if (!file.is_open()) {
        std::cerr << "Error: Unable to open file for reading: " << fixed_output_path << std::endl;
        continue;
      }
      std::stringstream buffer;
      char c;
      while (file.get(c))
        buffer << c;
      file.close();

      std::string contents = buffer.str();
      std::size_t pos = contents.find('\n');
      if (pos == std::string::npos)
        continue;
      std::string modified_line = contents.substr(0, pos);
      modified_line = fix_acronym_in_str(modified_line);
      contents.replace(0, pos, modified_line);

      std::ofstream mfile(fixed_output_path);
      if (!mfile.is_open()) {
        std::cerr << "Error: Unable to open file for reading: " << fixed_output_path << std::endl;
        continue;
      }
      mfile << contents;
      mfile.close();
    }
  }
}

int main() {
  
  std::cout << "Input folder for input(or drag and drop): ";
  std::string __input_path;
  std::getline(std::cin, __input_path);
  input_path = __input_path;
  std::cout << "Both of next acronyms should be in upper case.\n";
  std::cout << "Input old acronym: ";
  std::cin >> old_acronym;
  std::cout << "Input new acronym: ";
  std::cin >> new_acronym;

  old_acronym_lower = str_to_lower(old_acronym);
  new_acronym_lower = str_to_lower(new_acronym);
  acronym_pattern1 = old_acronym + '_';
  acronym_pattern2 = old_acronym + '-';
  acronym_pattern_lower1 = old_acronym_lower + '_';
  acronym_pattern_lower2 = old_acronym_lower + '-';

  std::filesystem::create_directory(output_path);
  fix_folders(&input_path, &output_path);

  std::cout << "Acronyms were fixed. Output is in the output folder, press any button to continue...\n";
  std::cin.get();

  return 0;
}