//
// Created by xg on 3/7/22.
//

#ifndef ECE650P4_SQL_H
#define ECE650P4_SQL_H

#include <pqxx/pqxx>
#include <string>

class SQL {
private:
  pqxx::connection *connection;
  static void runSQL(const std::string &transaction,
                     pqxx::connection *connection);

public:
  explicit SQL(std::string dbInfo) {
    this->connection = new pqxx::connection(dbInfo);
  };
  void buildALLTables();
  void dropTable(std::string tbName);
  void dropAllTables();
  pqxx::connection *getConnection();
  static void execute(pqxx::work &W, std::string sql);
};

#endif // ECE650P4_SQL_H
