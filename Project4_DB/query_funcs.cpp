#include "query_funcs.h"
#include "SQL.h"
#include <iomanip>
#include <string>

void add_player(pqxx::connection *C, int team_id, int jersey_num,
                string first_name, string last_name, int mpg, int ppg, int rpg,
                int apg, double spg, double bpg) {
  pqxx::work W(*C);
  std::string sqlQuery =
      "INSERT INTO PLAYER (TEAM_ID, UNIFORM_NUM, FIRST_NAME, LAST_NAME, MPG, "
      "PPG, RPG, APG, SPG, BPG) VALUES (" +
      std::to_string(team_id) + ", " + std::to_string(jersey_num) + ", " +
      W.quote(first_name) + ", " + W.quote(last_name) + ", " +
      std::to_string(mpg) + ", " + std::to_string(ppg) + ", " +
      std::to_string(rpg) + ", " + std::to_string(apg) + ", " +
      std::to_string(spg) + ", " + std::to_string(bpg) + ");";

  W.exec(sqlQuery);
  W.commit();
}

void add_team(connection *C, string name, int state_id, int color_id, int wins,
              int losses) {
  work W(*C);
  std::string sqlQuery =
      "INSERT INTO TEAM (NAME, STATE_ID, COLOR_ID, WINS, LOSSES) VALUES (" +
      W.quote(name) + ", " + std::to_string(state_id) + ", " +
      std::to_string(color_id) + ", " + std::to_string(wins) + ", " +
      std::to_string(losses) + ");";
  W.exec(sqlQuery);
  W.commit();
}

void add_state(connection *C, string name) {
  work W(*C);
  std::string sqlQuery =
      "INSERT INTO STATE (NAME) VALUES (" + W.quote(name) + ");";
  W.exec(sqlQuery);
  W.commit();
}

void add_color(connection *C, string name) {
  work W(*C);
  std::string sqlQuery =
      "INSERT INTO COLOR (NAME) VALUES (" + W.quote(name) + ");";
  W.exec(sqlQuery);
  W.commit();
}

void query1(connection *C, int use_mpg, int min_mpg, int max_mpg, int use_ppg,
            int min_ppg, int max_ppg, int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg, int use_spg, double min_spg,
            double max_spg, int use_bpg, double min_bpg, double max_bpg) {

  std::stringstream sql;
  sql << "SELECT * FROM PLAYER";
  std::vector<int> use_vec = {use_mpg, use_mpg, use_ppg, use_rpg,
                              use_apg, use_spg, use_bpg};

  bool where = false;
  for (int i = 0; i < use_vec.size(); i++) {
    if (use_vec[i] > 0)
      where = true;
  }

  std::vector<string> query_vec;
  if (where) {
    sql << " WHERE ";
    if (use_mpg) {
      string ans = "MPG >= " + std::to_string(min_mpg) + " AND " +
                   "MPG <= " + std::to_string(max_mpg);
      query_vec.push_back(ans);
    }
    if (use_ppg) {
      string ans = "PPG >= " + std::to_string(min_ppg) + " AND " +
                   "PPG <= " + std::to_string(max_ppg);
      query_vec.push_back(ans);
    }
    if (use_rpg) {
      string ans = "RPG >= " + std::to_string(min_rpg) + " AND " +
                   "RPG <= " + std::to_string(max_rpg);
      query_vec.push_back(ans);
    }
    if (use_apg) {
      string ans = "APG >= " + std::to_string(min_apg) + " AND " +
                   "APG <= " + std::to_string(max_apg);
      query_vec.push_back(ans);
    }
    if (use_spg) {
      string ans = "SPG >= " + std::to_string(min_spg) + " AND " +
                   "SPG <= " + std::to_string(max_spg);
      query_vec.push_back(ans);
    }
    if (use_bpg) {
      string ans = "BPG >= " + std::to_string(min_bpg) + " AND " +
                   "BPG <= " + std::to_string(max_bpg);
      query_vec.push_back(ans);
    }
  }

  for (int i = 0; i < query_vec.size(); i++) {
    sql << query_vec[i];
    if (i != query_vec.size() - 1)
      sql << " AND ";
  }

  sql << ";";

  nontransaction N(*C);
  result R(N.exec(sql.str()));
  cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG "
          "SPG BPG"
       << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<int>() << " " << c[1].as<int>() << " " << c[2].as<int>()
         << " " << c[3].as<string>() << " " << c[4].as<string>() << " "
         << c[5].as<int>() << " " << c[6].as<int>() << " " << c[7].as<int>()
         << " " << c[8].as<int>() << " " << fixed << setprecision(1)
         << c[9].as<double>() << " " << c[10].as<double>() << endl;
  }
}

void query2(connection *C, string team_color) {
  work W(*C);
  string query;
  query = "SELECT TEAM.NAME FROM TEAM, COLOR WHERE TEAM.COLOR_ID = "
          "COLOR.COLOR_ID AND COLOR.NAME = " +
          W.quote(team_color) + ";";
  W.commit();
  nontransaction N(*C);
  result R(N.exec(query));
  cout << "NAME" << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << endl;
  }
}

void query3(connection *C, string team_name) {
  work W(*C);
  string query;
  query = "SELECT FIRST_NAME, LAST_NAME FROM PLAYER, TEAM WHERE PLAYER.TEAM_ID "
          "= TEAM.TEAM_ID AND TEAM.NAME = " +
          W.quote(team_name) + " ORDER BY PPG DESC;";
  W.commit();
  nontransaction N(*C);
  result R(N.exec(query));
  cout << "FIRST_NAME LAST_NAME" << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << " " << c[1].as<string>() << endl;
  }
}

void query4(connection *C, string team_state, string team_color) {
  work W(*C);
  string query;
  query =
      "SELECT FIRST_NAME, LAST_NAME, UNIFORM_NUM FROM PLAYER, STATE, COLOR, "
      "TEAM WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND TEAM.COLOR_ID = "
      "COLOR.COLOR_ID AND TEAM.STATE_ID = STATE.STATE_ID AND STATE.NAME = " +
      W.quote(team_state) + " AND COLOR.NAME = " + W.quote(team_color) + ";";
  W.commit();
  nontransaction N(*C);
  result R(N.exec(query));
  cout << "FIRST_NAME LAST_NAME UNIFORM_NUM" << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << " " << c[1].as<string>() << " "
         << c[2].as<int>() << endl;
  }
}

void query5(connection *C, int num_wins) {
  work W(*C);
  string query;
  query = "SELECT FIRST_NAME, LAST_NAME, NAME, WINS FROM PLAYER, TEAM WHERE "
          "PLAYER.TEAM_ID = TEAM.TEAM_ID AND WINS >" +
          std::to_string(num_wins) + ";";
  W.commit();
  nontransaction N(*C);
  result R(N.exec(query));
  cout << "FIRST_NAME LAST_NAME NAME WINS" << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << " " << c[1].as<string>() << " "
         << c[2].as<string>() << " " << c[3].as<int>() << endl;
  }
}
