#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>

static std::string old_acronym, old_acronym_lower, new_acronym, new_acronym_lower;

std::string str_to_lower(std::string& str) {
  std::string str_lower = "";
  for (char c : str) {
    str_lower += std::tolower(c);
  }
  return str_lower;
}

bool is_acronym(std::string& str, size_t pos, std::string& acronym) {
  size_t offset = pos + acronym.length();
  return  pos == 0 ||
          offset == str.length() ||
          !(std::isalpha(str[pos - 1]) ||
            std::isdigit(str[pos - 1]) ||
            std::isalpha(str[offset]) ||
            std::isdigit(str[offset]));
}

void replace_acronyms(std::string& str) {
  size_t pos = str.find(old_acronym);
  while (pos != std::string::npos) {
    if (is_acronym(str, pos, old_acronym))
      str.replace(pos, old_acronym.length(), new_acronym);
    pos = str.find(old_acronym, pos + new_acronym.length());
  }
}

void replace_acronyms_lower(std::string& str) {
  size_t pos = str.find(old_acronym_lower);
  while (pos != std::string::npos) {
    if (is_acronym(str, pos, old_acronym_lower))
      str.replace(pos, old_acronym_lower.length(), new_acronym_lower);
    pos = str.find(old_acronym_lower, pos + new_acronym_lower.length());
  }
}

std::string fix_acronyms_in_str(std::string& str) {
  std::string new_str = str;
  replace_acronyms(new_str);
  replace_acronyms_lower(new_str);
  return new_str;
}

void fix_folders(std::filesystem::path* input_path, std::filesystem::path* output_path) {
  for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(*input_path)) {
    if (entry.is_directory()) {
      std::filesystem::path local_input_path = std::filesystem::path(entry.path());
      std::string fixed_output_path = entry.path().filename().string();
      if (fixed_output_path.size() == 2 && fixed_output_path == old_acronym_lower)
        fixed_output_path = new_acronym_lower;
      else
        fixed_output_path = fix_acronyms_in_str(fixed_output_path);
      std::filesystem::path local_ouput_path = std::filesystem::path(*output_path / fixed_output_path);
      std::filesystem::create_directory(local_ouput_path);
      fix_folders(&local_input_path, &local_ouput_path);
    }
    else if (entry.is_regular_file()) {
      std::string fixed_output_path = entry.path().filename().string();
      fixed_output_path = (*output_path / fix_acronyms_in_str(fixed_output_path)).string();

      if (entry.path().extension() != ".txt") {
        std::filesystem::copy_file(entry.path(), fixed_output_path, std::filesystem::copy_options::overwrite_existing);
        continue;
      }

      std::ifstream file(entry.path());
      if (!file.is_open()) {
        std::cerr << "Error: Unable to open file for reading: " << entry.path() << std::endl;
        continue;
      }
      std::stringstream buffer;
      char c;
      while (file.get(c))
        buffer << c;
      file.close();
      std::string contents = buffer.str();

      if (entry.path().parent_path().filename().string() == old_acronym_lower) {
        contents = fix_acronyms_in_str(contents);
      }
      else {
        std::size_t pos = contents.find('\n');
        if (pos == std::string::npos)
          continue;
        std::string modified_line = contents.substr(0, pos);
        modified_line = fix_acronyms_in_str(modified_line);
        contents.replace(0, pos, modified_line);
      }

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
  std::string input_path, output_path = "output";
  std::cout << "Input folder for input(or drag and drop): ";
  std::getline(std::cin, input_path);
  std::cout << "Both of next acronyms should be in upper case.\n";
  std::cout << "Input old acronym: ";
  std::cin >> old_acronym;
  std::cout << "Input new acronym: ";
  std::cin >> new_acronym;

  old_acronym_lower = str_to_lower(old_acronym);
  new_acronym_lower = str_to_lower(new_acronym);

  std::filesystem::remove_all(output_path);
  std::filesystem::create_directory(output_path);
  std::filesystem::path __input_path(input_path), __output_path(output_path);
  fix_folders(&__input_path, &__output_path);

  std::cout << "Acronyms were fixed. Output is in the output folder, press any button to continue...\n";
  std::cin.get();

  return 0;
}