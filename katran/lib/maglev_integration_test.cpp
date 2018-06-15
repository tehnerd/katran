/* Copyright (C) 2018-present, Facebook, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <gflags/gflags.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#include "CHHelpers.h"

DEFINE_int64(weight, 100, "weights per real");
DEFINE_int64(freq, 1, "how often real would have diff weight");
DEFINE_int64(difweight, 1, "diff weight for test");
DEFINE_int64(nreals, 400, "number of reals");
DEFINE_int64(pos, -1, "position to delete");
DEFINE_bool(all, false, "check all positions");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::vector<katran::Endpoint> endpoints;
  std::vector<uint32_t> freq(FLAGS_nreals, 0);
  katran::Endpoint endpoint;
  int index_to_delete;
  double n1 = 0;
  double n2 = 0;
  std::srand(std::time(nullptr));
  for (int i = 0; i < FLAGS_nreals; i++) {
    endpoint.num = i;
    endpoint.hash = 10 * i;
    if (i % FLAGS_freq == 0) {
      endpoint.weight = FLAGS_weight;
    } else {
      endpoint.weight = FLAGS_difweight;
    }
    endpoints.push_back(endpoint);
  }
  auto ch1 = katran::CHHelpers::GenerateMaglevHash(endpoints);
  for (int i = 0; i < ch1.size(); i++) {
    freq[ch1[i]]++;
  }
  std::vector<uint32_t> sorted_freq(freq);

  std::sort(sorted_freq.begin(), sorted_freq.end());

  std::cout << "min freq is " << sorted_freq[0] << " max freq is "
            << sorted_freq[sorted_freq.size() - 1] << std::endl;

  std::cout << "p95 w: " << sorted_freq[(sorted_freq.size() / 20) * 19]
            << "\np75 w: " << sorted_freq[(sorted_freq.size() / 20) * 15]
            << "\np50 w: " << sorted_freq[sorted_freq.size() / 2]
            << "\np25 w: " << sorted_freq[sorted_freq.size() / 4]
            << "\np5 w: " << sorted_freq[sorted_freq.size() / 20] << std::endl;




  if (!FLAGS_all) {
    if (FLAGS_pos >= 0) {
      index_to_delete = FLAGS_pos;
    } else {
      index_to_delete = std::rand()%endpoints.size();
    }
    endpoints.erase(endpoints.begin() + index_to_delete);
    auto ch2 = katran::CHHelpers::GenerateMaglevHash(endpoints);
    for (int i = 0; i < ch1.size(); i++) {
      if (ch1[i] != ch2[i]) {
        if (ch1[i] == index_to_delete) {
          n1++;
          continue;
        }
        n2++;
      }
    }
    std::cout << "changes for affected real: " << n1 << "; and for not affected "
              << n2 << " this is: " << n2 / ch1.size() * 100 << "\n";


  } else {
    std::vector<double> affected_pct;
    std::vector<int> pchanges;
    for (auto& elem: endpoints) {
      pchanges.push_back(0);
    }
    for (int i = 0; i < endpoints.size(); i++) {
      auto changes = pchanges;
      double n1 = 0;
      double n2 = 0;
      auto ep = endpoints;
      ep.erase(ep.begin() + i);
      auto ch2 = katran::CHHelpers::GenerateMaglevHash(ep);
      for (int j = 0; j < ch1.size(); j++) {
        if (ch1[j] != ch2[j]) {
          if (ch1[j] == i) {
            // changes related to deleted real
            n1++;
            continue;
          }
          // changes related to non deleted reals
          n2++;
          changes[ch1[j]]++;
        }
      }
      std::sort(changes.begin(), changes.end());
      std::cout << "max changes per real: " << changes.back()
                << " p50 changes: " << changes[changes.size() / 2]
                << " p75 changes: " << changes[((changes.size()) / 20) * 15]
                << " p95 changes: " << changes[((changes.size()) / 20) * 19]
                << std::endl;
      //affected_pct.push_back(n2/ch1.size()*100);
      affected_pct.push_back(n2);
    }
    std::sort(affected_pct.begin(), affected_pct.end());
    std::cout << "changes for non-affected real:\n"
              << "min: " << affected_pct[0] << " max: "
              << affected_pct[affected_pct.size() - 1]
              << "\n95 w: " << affected_pct[(sorted_freq.size() / 20) * 19]
              << "\np75 w: " << affected_pct[(sorted_freq.size() / 20) * 15]
              << "\np50 w: " << affected_pct[sorted_freq.size() / 2]
              << "\np25 w: " << affected_pct[sorted_freq.size() / 4]
              << "\np5 w: " << affected_pct[sorted_freq.size() / 20] << std::endl;
  }
  return 0;
}
