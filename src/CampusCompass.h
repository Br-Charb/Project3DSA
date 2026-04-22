#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <iomanip>

using namespace std;

class CampusCompass {
private:
    // Think about what member variables you need to initialize
    // perhaps some graph representation?

    //struct for a node
    struct node {
        int id;
        int weight;
        bool open;

        //inspired by https://www.geeksforgeeks.org/dsa/stl-priority-queue-for-structure-or-class/
        bool operator>(const node& other) const {
            return weight > other.weight;
        }
    };

    //graph representation of routes
    unordered_map<int, vector<node>> graph;

    //class code -> locationID
    unordered_map<string, int> classes;

    //locationID -> location name
    unordered_map<int, string> locations;

    //classCode -> times
    unordered_map<string, pair<int, int>> times;

    //struct for students
    struct student {
        string name;
        string id;
        int residence;
        vector<string> classes;
    };

    //studentID -> student info
    unordered_map<string, student> students;

public:
    // Think about what helper functions you will need in the algorithm
    CampusCompass(); // constructor
    bool ParseEdges(const string& edges_filepath);
    bool ParseClasses(const string& classes_filepath);
    bool ParseCSV(const string &edges_filepath, const string &classes_filepath);
    bool insert(string STUDENT_NAME, string STUDENT_ID, int RESIDENCE_LOCATION_ID, vector<string> CLASS_CODES);
    bool remove(string STUDENT_ID);
    bool dropClass(string STUDENT_ID, string CLASSCODE);
    bool replaceClass(string STUDENT_ID, string CLASSCODE_1, string CLASSCODE_2);
    int removeClass(string CLASSCODE);
    bool toggleEdgesClosure(vector<pair<int, int>> edges);
    string checkEdgeStatus(int LOCATION_ID_X, int LOCATION_ID_Y);
    bool isConnected(int LOCATION_ID_1, int LOCATION_ID_2);
    pair<unordered_map<int,int>, unordered_map<int,int>> djikstrasHelper(int LOCATION_ID);
    bool printShortestEdges(string STUDENT_ID);
    int printStudentZone(string STUDENT_ID);
    bool verifySchedule(string STUDENT_ID);


    bool ParseCommand(const string &command);

};
