#include "CampusCompass.h"

#include <string>

using namespace std;

CampusCompass::CampusCompass() {
    // initialize your object

}

bool CampusCompass::ParseEdges(const string& edges_filepath) {
    ifstream edges_csv(edges_filepath);
    if (!edges_csv.is_open()) {
        return false;
    }

    std::string line;

    //flush first line of csv headers
    std::getline(edges_csv, line);

    while (std::getline(edges_csv, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        std::stringstream ss(line);
        std::string cell;
        vector<string> parts;
        while (std::getline(ss, cell, ',')) {
            parts.push_back(cell);
        }

        this->graph[stoi(parts[0])].push_back({stoi(parts[1]), stoi(parts[4]), true});
        this->graph[stoi(parts[1])].push_back({stoi(parts[0]), stoi(parts[4]), true});

        this->locations[stoi(parts[0])] = parts[2];
        this->locations[stoi(parts[1])] = parts[3];
    }
    edges_csv.close();
    return true;
}

bool CampusCompass::ParseClasses(const string& classes_filepath) {
    ifstream classes_csv(classes_filepath);
    if (!classes_csv.is_open()) {
        return false;
    }

    std::string line;

    //flush first line of csv headers
    std::getline(classes_csv, line);
    while (std::getline(classes_csv, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        std::stringstream ss(line);
        std::string cell;
        vector<string> parts;
        while (std::getline(ss, cell, ',')) {
            parts.push_back(cell);
        }

        //maps ClassCode -> LocationID
        this->classes[parts[0]] = stoi(parts[1]);

        //calculate minutes of class starting time
        int startTime = 0;
        string timePart = "";
        for (int i = 0; i < parts[2].size(); i++) {
            if (parts[2][i] != ':') timePart += parts[2][i];
            else {
                startTime += stoi(timePart)*60;
                timePart = "";
            }
        }
        startTime += stoi(timePart);

        //calculate minutes of class end time
        int endTime = 0;
        timePart = "";
        for (int i = 0; i < parts[3].size(); i++) {
            if (parts[3][i] != ':') timePart += parts[3][i];
            else {
                endTime += stoi(timePart)*60;
                timePart = "";
            }
        }
        endTime += stoi(timePart);

        //map ClassCode to start and end time
        this->times[parts[0]] = {startTime, endTime};
    }

    classes_csv.close();
    return true;
}

//code inspired by "Reading CSV Files in C++: How To Guide" reference we were given
bool CampusCompass::ParseCSV(const string &edges_filepath, const string &classes_filepath) {
    // return boolean based on whether parsing was successful or not

    //parsing edges csv
    if (!(ParseEdges(edges_filepath))) return false;

    //parsing classes csv
    if (!(ParseClasses(classes_filepath))) return false;

    return true;
}


bool CampusCompass::insert(string STUDENT_NAME, string STUDENT_ID, int RESIDENCE_LOCATION_ID, vector<string> CLASS_CODES) {
    if (this->students.count(STUDENT_ID)) return false;
    student newStudent;
    newStudent.name = STUDENT_NAME;
    newStudent.id = STUDENT_ID;
    newStudent.residence = RESIDENCE_LOCATION_ID;
    newStudent.classes = CLASS_CODES;
    this->students[STUDENT_ID] = newStudent;
    return true;
}

bool CampusCompass::remove(string STUDENT_ID) {
    if (!(this->students.count(STUDENT_ID))) return false;
    this->students.erase(STUDENT_ID);
    return true;
}

bool CampusCompass::dropClass(string STUDENT_ID, string CLASSCODE) {
    if (!(this->students.count(STUDENT_ID))) return false;
    auto index = find(students[STUDENT_ID].classes.begin(), students[STUDENT_ID].classes.end(), CLASSCODE);
    if (index == students[STUDENT_ID].classes.end()) return false;
    students[STUDENT_ID].classes.erase(index);

    if (students[STUDENT_ID].classes.size() == 0) remove(STUDENT_ID);

    return true;
}

bool CampusCompass::replaceClass(string STUDENT_ID, string CLASSCODE_1, string CLASSCODE_2) {
    if (!(this->students.count(STUDENT_ID))) return false;
    auto index = find(students[STUDENT_ID].classes.begin(), students[STUDENT_ID].classes.end(), CLASSCODE_2);
    if (index != students[STUDENT_ID].classes.end()) return false;

    index = find(students[STUDENT_ID].classes.begin(), students[STUDENT_ID].classes.end(), CLASSCODE_1);
    if (index == students[STUDENT_ID].classes.end()) return false;

    if (!(classes.count(CLASSCODE_2))) return false;

    //use pointer to change class 1 to class 2
    *index = CLASSCODE_2;
    return true;
}

int CampusCompass::removeClass(string CLASSCODE) {
    if (!(classes.count(CLASSCODE))) return -1;
    // classes.erase(CLASSCODE);
    int count = 0;

    vector<string> studentIDS;
    for (auto s : students) studentIDS.push_back(s.second.id);

    for (string s : studentIDS) {
        if (dropClass(s, CLASSCODE)) count += 1;
    }
    return count;
}

bool CampusCompass::toggleEdgesClosure(vector<pair<int, int>> edges) {
    for (pair<int, int> e : edges) {
        for (int i = 0; i < graph[e.first].size(); i++) {
            if (graph[e.first][i].id == e.second) {
                graph[e.first][i].open = !(graph[e.first][i].open);
            }
        }
        for (int i = 0; i < graph[e.second].size(); i++) {
            if (graph[e.second][i].id == e.first) {
                graph[e.second][i].open = !(graph[e.second][i].open);
            }
        }
    }
    return true;
}

string CampusCompass::checkEdgeStatus(int LOCATION_ID_X, int LOCATION_ID_Y) {
    for (int i = 0; i < graph[LOCATION_ID_X].size(); i++) {
        if (graph[LOCATION_ID_X][i].id == LOCATION_ID_Y) {
            if (graph[LOCATION_ID_X][i].open) {
                return "open";
            } else {
                return "closed";
            }
        }
    }
    return "DNE";
}

bool CampusCompass::isConnected(int LOCATION_ID_1, int LOCATION_ID_2) {
    if (!(this->graph.count(LOCATION_ID_1))) return false;
    if (!(this->graph.count(LOCATION_ID_2))) return false;

    unordered_set<int> visited;
    queue<int> locals;
    locals.push(LOCATION_ID_1);
    visited.insert(LOCATION_ID_1);
    while (!locals.empty()) {
        int n = locals.front();
        locals.pop();
        for (int i = 0; i < graph[n].size(); i++) {
            if (graph[n][i].id == LOCATION_ID_2 && graph[n][i].open) {
                return true;
            } else {
                if (graph[n][i].open && visited.count(graph[n][i].id) == 0) {
                    locals.push(graph[n][i].id);
                    visited.insert(graph[n][i].id);
                }
            }
        }
    }
    return false;
}

pair<unordered_map<int,int>, unordered_map<int,int>> CampusCompass::djikstrasHelper(int LOCATION_ID) {
    unordered_map<int, int> dist;
    unordered_map<int, int> parents;
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> locals;
    locals.push({0, LOCATION_ID});
    dist[LOCATION_ID] = 0;
    parents[LOCATION_ID] = -1;

    while (!(locals.empty())) {
        pair<int, int> curr = locals.top();
        locals.pop();
        for (node n : graph[curr.second]) {
            if (n.open) {
                if (dist.count(n.id) == 0) {
                    dist[n.id] = dist[curr.second] + n.weight;
                    parents[n.id] = curr.second;
                    locals.push({dist[n.id], n.id});
                } else {
                    if (dist[n.id] > dist[curr.second] + n.weight) {
                        dist[n.id] = dist[curr.second] + n.weight;
                        parents[n.id] = curr.second;
                        locals.push({dist[n.id], n.id});
                    }
                }
            }
        }
    }
    return {dist, parents};
}

bool CampusCompass::printShortestEdges(string STUDENT_ID) {
    pair<unordered_map<int,int>, unordered_map<int,int>> info = djikstrasHelper(students[STUDENT_ID].residence);
    unordered_map<int,int> dist = info.first;
    cout << "Time For Shortest Edges: " << students[STUDENT_ID].name << endl;
    sort(students[STUDENT_ID].classes. begin(), students[STUDENT_ID].classes.end());
    for (string course : students[STUDENT_ID].classes) {
        if (dist.count(classes[course]) != 0) cout << course << ": " << dist[classes[course]] << endl;
        else cout << course << ": " << -1 << endl;
    }

    return true;
}

int CampusCompass::printStudentZone(string STUDENT_ID) {
    pair<unordered_map<int,int>, unordered_map<int,int>> info = djikstrasHelper(students[STUDENT_ID].residence);
    unordered_map<int,int> dist = info.first;
    unordered_map<int,int> parents = info.second;
    queue<int> q;
    unordered_set<int> visited;
    visited.insert(students[STUDENT_ID].residence);
    for (string course : students[STUDENT_ID].classes) {
        visited.insert(classes[course]);
        q.push(classes[course]);
    }
    while (!q.empty()) {
        int n = q.front();
        q.pop();
        if (parents[n] != -1 && !(visited.count(parents[n]))) {
            visited.insert(parents[n]);
            q.push(parents[n]);
        }
    }
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> edges;
    int weight = 0;
    edges.push({0, students[STUDENT_ID].residence});
    while (!edges.empty()) {
        pair<int, int> curr = edges.top();
        edges.pop();

        if (visited.count(curr.second) == 0) continue;
        visited.erase(curr.second);
        weight += curr.first;

        for (node n : graph[curr.second]) {
            if (n.open && visited.count(n.id)) edges.push({n.weight, n.id});
        }
    }

    return weight;
}

bool CampusCompass::verifySchedule(string STUDENT_ID) {
    priority_queue<pair<int, string>, vector<pair<int, string>>, greater<pair<int, string>>> courses;

    for (string c : students[STUDENT_ID].classes) {
        courses.push({times[c].first, c});
    }

    if (courses.size() == 0) return false;

    string lastClass = courses.top().second;
    int last = classes[courses.top().second];
    int time = times[courses.top().second].second;
    courses.pop();

    if (courses.size() == 0) return false;

    cout << "Schedule Check for " << students[STUDENT_ID].name << ":" << endl;

    while (!courses.empty()) {
        pair<int, string> curr = courses.top();
        courses.pop();

        pair<unordered_map<int, int>, unordered_map<int, int>> info = djikstrasHelper(last);
        unordered_map<int, int> dist = info.first;

        if (dist.count(classes[curr.second]) && time + dist[classes[curr.second]] <= curr.first) cout << lastClass << " - " << curr.second << ": successful" << endl;
        else cout << lastClass << " - " << curr.second << ": unsuccessful" << endl;

            time = times[curr.second].second;
            lastClass = curr.second;
            last = classes[curr.second];
    }

    return true;
}


//returns true if course invalid
bool checkCourseCode(string course) {
    if (course.size() != 7) return true;
    for (int i = 0; i < course.length(); i++) {
        if (i < 3) {
            if (!(isupper(course[i]))) return true;
        } else {
            if (!(isdigit(course[i]))) return true;
        }
    }
    return false;
}

//returns true if name is invalid
bool checkName(string name) {
    for (char c : name) {
        if (!(isalpha(c)) && c != ' ') return true;
    }
    return false;
}

//returns true if id is invalid
bool checkID(string id) {
    if (id.size() != 8) return true;
    for (char c : id) {
        if (!isdigit(c)) return true;
    }
    return false;
}

bool failure() {
    cout << "unsuccessful" << endl;
    return false;
}

bool success() {
    cout << "successful" << endl;
    return true;
}


bool CampusCompass::ParseCommand(const string &command) {
    // do whatever regex you need to parse validity
    // hint: return a boolean for validation when testing. For example:

    stringstream stream(command);
    string function;
    stream >> function;

    if (function == "insert") {
        string STUDENT_NAME;
        string STUDENT_ID;
        int RESIDENCE_LOCATION_ID;
        int n;

        stream >> quoted(STUDENT_NAME) >> STUDENT_ID >> RESIDENCE_LOCATION_ID >> n;

        if (stream.fail()) return failure();
        if (checkName(STUDENT_NAME)) return failure();
        if (checkID(STUDENT_ID)) return failure();
        if (students.count(STUDENT_ID)) return failure();
        if (locations.count(RESIDENCE_LOCATION_ID) == 0) return failure();
        if (n > 6 || n < 1) return failure();

        vector<string> courses;
        for (int i = 0; i < n; i++) {
            string temp;
            stream >> temp;
            if (checkCourseCode(temp)) return failure();
            courses.push_back(temp);
        }

        if (stream.fail()) return failure();

        if (insert(STUDENT_NAME, STUDENT_ID, RESIDENCE_LOCATION_ID, courses)) return success();
        return failure();
    } if (function == "remove") {
        string STUDENT_ID;

        stream >> STUDENT_ID;

        if (stream.fail()) return failure();
        if (checkID(STUDENT_ID)) return failure();
        if (students.count(STUDENT_ID) == 0) return failure();

        if (remove(STUDENT_ID)) return success();
        return failure();
    } if (function == "dropClass") {
        string STUDENT_ID;
        string CLASSCODE;
        stream >> STUDENT_ID >> CLASSCODE;
        if (stream.fail()) return failure();
        if (checkID(STUDENT_ID)) return failure();
        if (students.count(STUDENT_ID) == 0) return failure();
        if (checkCourseCode(CLASSCODE)) return failure();

        if (dropClass(STUDENT_ID, CLASSCODE)) return success();
        return failure();
    } if (function == "replaceClass") {
        string STUDENT_ID;
        string CLASSCODE1;
        string CLASSCODE2;
        stream >> STUDENT_ID >> CLASSCODE1 >> CLASSCODE2;
        if (stream.fail()) return failure();
        if (checkID(STUDENT_ID)) return failure();
        if (students.count(STUDENT_ID) == 0) return failure();
        if (checkCourseCode(CLASSCODE1)) return failure();
        if (checkCourseCode(CLASSCODE2)) return failure();

        if (replaceClass(STUDENT_ID, CLASSCODE1, CLASSCODE2)) return success();
        return failure();
    } if (function == "removeClass") {
        string CLASSCODE;
        stream >> CLASSCODE;
        if (stream.fail()) return failure();
        if (checkCourseCode(CLASSCODE)) return failure();

        int count = removeClass( CLASSCODE);

        if ( count == -1) return failure();

        cout << count << endl;
        return true;
    } if (function == "toggleEdgesClosure") {
        int n;
        stream >> n;
        if (stream.fail()) return failure();
        if (n < 1) return failure();

        vector<pair<int, int>> edges;
        for (int i = 0; i < n; i++) {
            int a;
            int b;
            stream >> a >> b;
            if (stream.fail()) return failure();
            if (locations.count(a) == 0) return failure();
            if (locations.count(b) == 0) return failure();
            edges.push_back(make_pair(a, b));
        }
        toggleEdgesClosure(edges);
        return success();
    } if (function == "checkEdgeStatus") {
        int a;
        int b;
        stream >> a >> b;
        if (stream.fail()) return failure();
        if (locations.count(a) == 0) return failure();
        if (locations.count(b) == 0) return failure();
        string result = checkEdgeStatus(a, b);
        cout << result << endl;
        return true;
    } if (function == "isConnected") {
        int a;
        int b;
        stream >> a >> b;
        if (stream.fail()) return failure();
        if (locations.count(a) == 0) return failure();
        if (locations.count(b) == 0) return failure();
        if (isConnected(a, b)) return success();
        return failure();
    } if (function == "printShortestEdges") {
        string STUDENT_ID;

        stream >> STUDENT_ID;

        if (stream.fail()) return failure();
        if (checkID(STUDENT_ID)) return failure();
        if (students.count(STUDENT_ID) == 0) return failure();
        printShortestEdges(STUDENT_ID);
        return true;
    } if (function == "printStudentZone") {
        string STUDENT_ID;

        stream >> STUDENT_ID;

        if (stream.fail()) return failure();
        if (checkID(STUDENT_ID)) return failure();
        if (students.count(STUDENT_ID) == 0) return failure();

        int w = printStudentZone(STUDENT_ID);
        cout << "Student Zone Cost For " << students[STUDENT_ID].name << ": " << w << endl;
        return true;
    } if (function == "verifySchedule") {
        string STUDENT_ID;

        stream >> STUDENT_ID;

        if (stream.fail()) return failure();
        if (checkID(STUDENT_ID)) return failure();
        if (students.count(STUDENT_ID) == 0) return failure();

        verifySchedule(STUDENT_ID);
        return true;
    }

    return failure();
}