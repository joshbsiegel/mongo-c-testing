#include <stdio.h>
#include <mongoc/mongoc.h>
#include <pthread.h>

static pthread_mutex_t mutex;
static bool in_shutdown = false;

static void * threadScramAuth() {
  mongoc_client_t *client = mongoc_client_new ("mongodb://user,=:pass@127.0.0.1/test?appname=scram-example&authMechanism=SCRAM-SHA-1");
}

int main()
{
   mongoc_client_t *client = NULL;
   mongoc_database_t *database = NULL;
   mongoc_collection_t *collection = NULL;
   mongoc_cursor_t *cursor = NULL;
   bson_error_t error;
   const char *uri_string = "mongodb://127.0.0.1/";
   mongoc_uri_t *uri = NULL;
   const char *authuristr;
   bson_t roles;
   const bson_t *doc;
   int exit_code = EXIT_FAILURE;
   pthread_t threads[10000];

   mongoc_init ();

   bson_init (&roles);

   uri = mongoc_uri_new_with_error (uri_string, &error);
   if (!uri) {
      fprintf (stderr,
               "failed to parse URI: %s\n"
               "error message:       %s\n",
               uri_string,
               error.message);
      goto CLEANUP;
   }

   client = mongoc_client_new_from_uri (uri);
   if (!client) {
      goto CLEANUP;
   }

   mongoc_client_set_error_api (client, 2);

   database = mongoc_client_get_database (client, "test");

   BCON_APPEND (&roles, "0", "{", "role", "root", "db", "admin", "}");

   mongoc_database_add_user (database, "user,=", "pass", &roles, NULL, &error);

   mongoc_database_destroy (database);

   mongoc_client_destroy (client);

  
   for (int i = 0; i < 10000; ++i) {
        pthread_create(&threads[i], NULL, threadScramAuth, NULL);
   }
  
   CLEANUP:

   bson_destroy (&roles);

   if (uri) {
      mongoc_uri_destroy (uri);
   }

   mongoc_cleanup ();

   return exit_code;
}