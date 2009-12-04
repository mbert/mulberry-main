/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <iostream.h>
#include <string>

using namespace std;

void process();
void doString(const string& resid, const string& filename );
istream& eatwhite(istream&);

int main()
{
  string indicator;

  // Look for STRINGTABLE
  
  while(!cin.eof()) {
    cin >> indicator;
    if (indicator == "STRINGTABLE") {
      process();
    } else {
      getline(cin, indicator);
    }
  }
}

void process()
{
  string data;

  // Punt over line
  getline(cin, data);

  // Look for BEGIN
  while(!cin.eof()) {
    getline(cin, data);
    if (data == "BEGIN")
      break;
  }
  if (cin.eof()) return;

  // Get each string
  while(!cin.eof()) {
    string resid, resString;

    eatwhite(cin >> resid);
    bool do_it = true;
    if (resid == "END")
      break;
    else if (resid.substr(0, 4) == "AFX_")
      do_it = false;
    else if (resid.substr(0, 4) == "IDR_")
      do_it = false;
    else if (resid.substr(0, 3) == "ID_")
      do_it = false;
    getline(eatwhite(cin), resString);
    if (do_it)
      doString(resid, resString);
  }
}

void doString(const string& resid, const string& resString )
{
  string stringname;
  
  stringname =  resid.substr(2, resid.length() - 2);
  cout << "AddGlobalResource( " << resid << ", "<< stringname << "Data );" << endl;
}

istream& eatwhite(istream& is)
{
  char c;
  while (is.get(c)) {
    if (!isspace(c)) {
      is.putback(c);
      break;
    }
  }
  return is;
}
