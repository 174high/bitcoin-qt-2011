#include <iostream>
#include <db_cxx.h>
#include <string.h>

using namespace std;

int main(void)
{

       Db db(NULL,0);
       u_int32_t oFlags = DB_CREATE; // Open flags;
       try
       {
              db.open(NULL,                // Transaction pointer
                     "my_db.db",          // Database file name
                      NULL,                // Optional logical database name
                      DB_BTREE,            // Database access method
                      oFlags,              // Open flags
                      0);                  // File mode (using defaults)

              db.truncate(NULL,0,0);

              float money = 122.45;

              char *description = "Grocery bill.";

              Dbt key(&money, sizeof(float));

              Dbt data(description, strlen(description)+1);

              int ret = db.put(NULL, &key, &data, DB_NOOVERWRITE);

              cout<<"put data--"<<description<<endl;

              ret = db.get(NULL, &key, &data, DB_GET_BOTH);

              cout<<"get key--"<<*((float*)key.get_data())<<endl;

              cout<<"get data--"<<(char *)data.get_data()<<endl;


              money = 111;

              description = "James--------------------"; 

             data.set_data(description);

              data.set_size(strlen(description)+1);

              db.put(NULL,&key,&data,DB_NOOVERWRITE);

              ret = db.get(NULL, &key, &data, DB_GET_BOTH);

              cout<<"get key--"<<*((float*)key.get_data())<<endl;

              cout<<"get data--"<<(char *)data.get_data()<<endl;

              money = 191;

              description = "Mike";

              data.set_data(description);

              data.set_size(strlen(description)+1);

              db.put(NULL,&key,&data,DB_NOOVERWRITE);

              ret = db.get(NULL, &key, &data, DB_GET_BOTH);

              cout<<"get key--"<<*((float*)key.get_data())<<endl;

              cout<<"get data--"<<(char *)data.get_data()<<endl;
 

              Dbc* cursor;

              db.cursor(NULL,&cursor,0);

              cout<<"open cursor"<<endl;

              while((ret = cursor->get(&key,&data,DB_PREV)) != DB_NOTFOUND)
              {

                     cout<<"get key--"<<*((float*)key.get_data())<<endl;

                     cout<<"get data--"<<(char *)data.get_data()<<endl;

              }

              if (cursor != NULL)
              {

                     cursor->close();

              }

              money = 191;

              description = "Mike";

              data.set_data(description);

              data.set_size(strlen(description)+1);

              db.cursor(NULL,&cursor,0);

              cout<<"delete 191..."<<endl;

              while((ret = cursor->get(&key,&data,DB_SET)) == 0 )
              {

                     cursor-> del(0);

              }

              if (cursor != NULL)
              {

                     cursor->close();

              }

              cout<<"after delete 191..."<<endl;

              db.cursor(NULL,&cursor,0);

              while((ret = cursor->get(&key,&data,DB_PREV)) != DB_NOTFOUND)
              {
                     cout<<"get key--"<<*((float*)key.get_data())<<endl;

                     cout<<"get data--"<<(char *)data.get_data()<<endl;

              }

              if (cursor != NULL)
              {

                     cursor->close();

              }

       }

       catch(DbException &e)
       {

              cerr<<"DBException:"<<e.what();

       }

       catch(std::exception &e)
       {

              cerr<<"DBException:"<<e.what();

       }


       return 0;
}




