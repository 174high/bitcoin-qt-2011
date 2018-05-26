#include <iostream>
#include <string.h>
#include <db_cxx.h>
#include <db.h>

using std::cout;
using std::endl;
using std::cerr;

int
main (int argc, char *argv[])
{
try
{
/* 1: create the database handle */
Db db (0, 0);

/* 2: open the database using the handle */
db.open (NULL, "./chap4_db", NULL, DB_BTREE, DB_CREATE, 0644);

/* 3: create the key and value Dbts */
char *first_key = "first_record";
u_int32_t key_len = (u_int32_t) strlen (first_key);

char * first_value = "Hello World - Berkeley DB style!!";
u_int32_t value_len = (u_int32_t) strlen (first_value);

Dbt key (first_key, key_len + 1);
Dbt value (first_value, value_len + 1);

/* 4: insert the key-value pair into the database */
int ret;
ret = db.put (0, &key, &value, DB_NOOVERWRITE);
if (ret == DB_KEYEXIST)
{
cout << "hello_world: " << first_key << " already exists in db" <<endl;
}

/* 5: read the value stored earlier in a Dbt object */
Dbt stored_value;
ret = db.get (0, &key, &stored_value, 0);

/* 6: print the value read from the database */
cout << (char *) stored_value.get_data () << endl;

/* 7: close the database handle */
db.close (0);
}
catch (DbException & dbex)
{
cerr << "hello_world: exception caught: " << dbex.what () << endl;
}
}


