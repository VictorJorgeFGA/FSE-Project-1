#include <bits/stdc++.h>

using namespace std;

vector<pair<int,int>> hardwares;
vector<pair<int,int>> vms;
vector<string> vms_name;

vector<int> hw;
vector<vector<int>> vmhw;

int main() {
  int hardwares_count;
  cin >> hardwares_count;
  hw.resize(hardwares_count);
  iota(hw.begin(), hw.end(), 1);

  for (int i = 0; i < hardwares_count; i++) {
    int ram, cpu;
    cin >> ram >> cpu;
    hardwares.push_back({ram, cpu});
  }

  int vm_count;
  cin >> vm_count;
  int last = hardwares_count;
  vmhw.resize(vm_count);
  for (int i = 0; i < vm_count; i++) {
    string vm_name;
    int ram, cpu;
    cin >> vm_name >> ram >> cpu;
    vms.push_back({ram, cpu});
    vms_name.push_back(vm_name);

    for (int j = 0; j < hardwares_count; j++) {
      vmhw[i].push_back(++last);
    }
  }
  cout << "HW: ";
  for (auto e : hw) {
    cout << e << ' ';
  }
  cout << endl << "VM HW: " << endl;

  for (auto row : vmhw) {
    for (auto column : row) {
      cout << column << ' ';
    }
    cout << endl;
  }
  int restrictions = 0;

  int restr_08_hw_ram = 0, restr_09_hw_proc = 0;
  for (auto e : hardwares) {
    restr_08_hw_ram += e.first;
    restr_09_hw_proc += e.second;
  }

  int restr_08_vm_ram = 0, restr_09_vm_proc = 0;
  for (auto e : vms) {
    restr_08_vm_ram += e.first;
    restr_09_vm_proc += e.second;
  }

  restrictions = 2;
  return 0;
}