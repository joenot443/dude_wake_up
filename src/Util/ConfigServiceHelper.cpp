#include "ConfigServiceHelper.hpp"
#include "ConfigService.hpp" // This is where the full ConfigService.hpp is included

std::string getExportsFolderPath() {
  return ConfigService::getService()->exportsFolderFilePath();
}
