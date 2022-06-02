#include "SQL.h"
#include <pqxx/pqxx>
#include <string>
#include <vector>

void execute(pqxx::work &W, std::string sql) {
  W.exec(sql);
  W.commit();
}

pqxx::connection *SQL::getConnection() { return this->connection; }
// Start a transaction
void SQL::runSQL(const std::string &transaction, pqxx::connection *connection) {
  pqxx::work W(*connection);
  W.exec(transaction);
  W.commit();
}

void SQL::dropAllTables() {
  std::vector<std::string> rmList = {"PLAYER", "TEAM", "STATE", "COLOR"};
  for (auto toRm : rmList) {
    SQL::dropTable(toRm);
  }
}

void SQL::dropTable(std::string tbName) {
  std::string transaction = "DROP TABLE IF EXISTS " + tbName + " CASCADE;";
  SQL::runSQL(transaction, this->connection);
}

void SQL::buildALLTables() {
  std::string transaction = "CREATE TABLE STATE(\n"
                            "STATE_ID SERIAL,\n"
                            "NAME VARCHAR(256),\n"
                            "PRIMARY KEY (STATE_ID)\n"
                            ");\n"
                            "\n"
                            "CREATE TABLE COLOR(\n"
                            "COLOR_ID SERIAL,\n"
                            "NAME VARCHAR(256),\n"
                            "PRIMARY KEY (COLOR_ID)\n"
                            ");\n"
                            "\n"
                            "CREATE TABLE TEAM(\n"
                            "TEAM_ID SERIAL,\n"
                            "NAME VARCHAR(256),\n"
                            "STATE_ID INT,\n"
                            "COLOR_ID INT,\n"
                            "WINS INT,\n"
                            "LOSSES INT,\n"
                            "PRIMARY KEY (TEAM_ID),\n"
                            "FOREIGN KEY (STATE_ID) REFERENCES STATE(STATE_ID) "
                            "ON DELETE SET NULL ON UPDATE CASCADE,\n"
                            "FOREIGN KEY (COLOR_ID) REFERENCES COLOR(COLOR_ID) "
                            "ON DELETE SET NULL ON UPDATE CASCADE\n"
                            ");\n"
                            "\n"
                            "CREATE TABLE PLAYER(\n"
                            "PLAYER_ID SERIAL,\n"
                            "TEAM_ID INT,\n"
                            "UNIFORM_NUM INT,\n"
                            "FIRST_NAME VARCHAR(256),\n"
                            "LAST_NAME VARCHAR(256),\n"
                            "MPG INT,\n"
                            "PPG INT,\n"
                            "RPG INT,\n"
                            "APG INT,\n"
                            "SPG DOUBLE PRECISION,\n"
                            "BPG DOUBLE PRECISION,\n"
                            "PRIMARY KEY (PLAYER_ID),\n"
                            "FOREIGN KEY (TEAM_ID) REFERENCES TEAM(TEAM_ID) ON "
                            "DELETE SET NULL ON UPDATE CASCADE\n"
                            ");";

  SQL::runSQL(transaction, this->connection);
}
