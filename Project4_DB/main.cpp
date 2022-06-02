#include "SQL.h"
#include "exerciser.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <pqxx/pqxx>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

bool insertStates(connection *C) {
  ifstream file("state.txt");
  if (file.is_open()) {
    string line = "";
    while (std::getline(file, line)) {
      stringstream ss(line);

      string id;
      string stateName;

      std::getline(ss, id, ' ');
      std::getline(ss, stateName);
      add_state(C, stateName);
    }
    file.close();
    return true;
  } else {
    return false;
  }
}

bool insertColors(connection *C) {
  ifstream file("color.txt");
  if (file.is_open()) {
    string line = "";
    while (std::getline(file, line)) {
      stringstream ss(line);

      string id;
      string colorName;

      std::getline(ss, id, ' ');
      std::getline(ss, colorName);
      add_color(C, colorName);
    }
    file.close();
    return true;
  } else {
    return false;
  }
}

bool insertTeams(connection *C) {
  ifstream file("team.txt");
  if (file.is_open()) {
    string line = "";
    while (std::getline(file, line)) {
      stringstream ss(line);

      string id;
      string teamName;
      string state_id;
      string color_id;
      string wins;
      string loses;

      std::getline(ss, id, ' ');
      std::getline(ss, teamName, ' ');
      std::getline(ss, state_id, ' ');
      std::getline(ss, color_id, ' ');
      std::getline(ss, wins, ' ');
      std::getline(ss, loses);

      add_team(C, teamName, std::stoi(state_id), std::stoi(color_id),
               std::stoi(wins), std::stoi(loses));
    }
    file.close();
    return true;
  } else {
    return false;
  }
}

bool insertPlayers(connection *C) {
  ifstream file("player.txt");
  if (file.is_open()) {
    string line = "";
    while (std::getline(file, line)) {
      stringstream ss(line);

      string id;
      string teamId;
      string jerseyNum;
      string f_name;
      string l_name;
      string mpg;
      string ppg;
      string rpg;
      string apg;
      string spg;
      string bpg;

      std::getline(ss, id, ' ');
      std::getline(ss, teamId, ' ');
      std::getline(ss, jerseyNum, ' ');
      std::getline(ss, f_name, ' ');
      std::getline(ss, l_name, ' ');
      std::getline(ss, mpg, ' ');
      std::getline(ss, ppg, ' ');
      std::getline(ss, rpg, ' ');
      std::getline(ss, apg, ' ');
      std::getline(ss, spg, ' ');
      std::getline(ss, bpg);

      add_player(C, std::stoi(teamId), std::stoi(jerseyNum), f_name, l_name,
                 std::stoi(mpg), std::stoi(ppg), std::stoi(rpg), std::stoi(apg),
                 std::stod(spg), std::stod(bpg));
    }
    file.close();
    return true;
  } else {
    return false;
  }
}

// Insert all tables
void insertTables(connection *C) {
  // insert state
  insertStates(C);
  // insert color
  insertColors(C);
  // insert team
  insertTeams(C);
  // insert player
  insertPlayers(C);
}

int main(int argc, char *argv[]) {
  try {
    std::string dbInfo = "dbname=ACC_BBALL user=postgres password=passw0rd";
    SQL sql(dbInfo);

    if (sql.getConnection()->is_open()) {
      std::cout << "Opened database successfully: "
                << sql.getConnection()->dbname() << std::endl;

      // if the Table exist, then drop table
      sql.dropAllTables();
      // Create the table
      sql.buildALLTables();
      // Insert table
      insertTables(sql.getConnection());
      // Do exercise
      exercise(sql.getConnection());
      // Disconect the table
      sql.getConnection()->disconnect();
      return EXIT_SUCCESS;

    } else {
      std::cout << "Can't open database" << std::endl;
      return EXIT_FAILURE;
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
