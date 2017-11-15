#include "lvmwrapper.hpp"
#include "config.hpp"
#include "error.hpp"

namespace detail {
std::string run_command_impl(const std::string &cmd) {
  piso_log("Running command: ", cmd);

  char line[1024];
  std::string result = "";

  FILE *proc = popen(cmd.c_str(), "r");
  if (proc == NULL) {
    piso_error("popen(): command failed: ", cmd);
  }

  while (fgets(line, sizeof(line), proc)) {
    result += line;
  }
  auto retcode = WEXITSTATUS(pclose(proc));

  if (retcode != 0) {
    piso_error("popen(): command returned non-zero: ", cmd);
  }

  return result;
}

std::string lvm_run_impl(const std::string &cmd) { return run_command(cmd); }

Json::Value lvm_run_json_impl(const std::string &cmd) {
  Json::Value root;
  Json::Reader reader;
  const auto &command_results = run_command(cmd + " --reportformat json");
  bool parsed = reader.parse(command_results, root);
  if (!parsed) {
    piso_error("Error while parsing json output of: ", cmd);
  }

  return root;
}
} // namespace detail

Json::Value lvm_lvs_report(std::string options, std::string volname) {
  if (options != "") {
    options = "-o " + options;
  }
  if (volname != "") {
    volname = std::string(VOLUME_GROUP_NAME) + "/" + volname;
  }
  auto res = lvm_run_json("lvs ", options, " ", volname)["report"][0]["lv"];

  // If a specific volume is requested, just return it
  if (volname != "") {
    return res[0];
  } else {
    return res;
  }
}

std::string lvm_lvs_volume_value(std::string field, std::string volname) {
  return lvm_lvs_report(field, volname)[field].asString();
}