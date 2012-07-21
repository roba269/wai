#include <stdio.h>
#include "mongo.h"
#include "bson.h"

int main(int argc, char **argv)
{
  mongo conn;
  int status = mongo_connect(&conn, "127.0.0.1", 27017);
  if (status != MONGO_OK) {
    printf("connection failed.\n");
    return -1;
  }
  bson query;
  bson_init(&query);
  bson_append_int(&query, "status", 0);
  bson_finish(&query);
  mongo_cursor cursor;
  mongo_cursor_init(&cursor, &conn, "wai.submits");
  mongo_cursor_set_query(&cursor, &query);
  while (mongo_cursor_next(&cursor) == MONGO_OK) {
    printf("===========================\n");
    bson_print(&(cursor.current));
  }
  bson_destroy(&query);
  mongo_cursor_destroy(&cursor);
  mongo_destroy(&conn);
    // 1. Create Deamon
    // 2. poll query in database, compiling uncompiled source file
    // 3. call Scheduler::GetMatch(), and build match, get result, fill in the db
    return 0;
}

